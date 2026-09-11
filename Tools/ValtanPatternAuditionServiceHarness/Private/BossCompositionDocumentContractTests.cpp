#include "BossCompositionDocument.h"
#include "KoukuSaydonActionWorkbench.h"
#include "Level_KakulSaydonArena.h"
#include "ProjectDataRoot.h"
#include "AnimationTargetService.h"
#include "CameraTool.h"

#include <Windows.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifdef LOSTARK_VALTAN_AUDITION_SERVICE_HARNESS
// CPU editor contracts never create a live Level or operate its camera UI.
// Keep link-only dependencies strict so a new live call fails the contract.
Client::CLevel_KakulSaydonArena* Client::CLevel_KakulSaydonArena::s_pActiveInstance = nullptr;

bool_t Client::CLevel_KakulSaydonArena::Ensure_CameraShotAuthoring(std::string&)
{
	throw std::runtime_error("CPU editor harness unexpectedly requested live Camera authoring.");
}

bool_t Client::CLevel_KakulSaydonArena::Create_CameraShot(
	std::string_view, std::string&, std::string&)
{
	throw std::runtime_error("CPU editor harness unexpectedly requested live Camera creation.");
}

bool_t Client::CLevel_KakulSaydonArena::Update_CameraShot(
	const KAKUL_CAMERA_SHOT&, std::string&)
{
	throw std::runtime_error("CPU editor harness unexpectedly requested live Camera update.");
}

bool_t Client::CLevel_KakulSaydonArena::Capture_CameraShot(std::string_view, std::string&)
{
	throw std::runtime_error("CPU editor harness unexpectedly requested live Camera capture.");
}

bool_t Client::CLevel_KakulSaydonArena::Save_CameraShots(std::string&)
{
	throw std::runtime_error("CPU editor harness unexpectedly requested live Camera Save.");
}

void Client::CLevel_KakulSaydonArena::Stop_CompositionCamera(bool_t)
{
	throw std::runtime_error("CPU editor harness unexpectedly stopped a live Camera.");
}

Client::VALTAN_CINEMATIC_CAMERA_CUE Client::CLevel_KakulSaydonArena::CameraShot_ToCue(const KAKUL_CAMERA_SHOT&)
{
	throw std::runtime_error("CPU editor harness unexpectedly requested a live Camera cue.");
}

bool_t Client::CCameraTool::Capture_ViewPose(VALTAN_CINEMATIC_CAMERA_POSE&)
{
	throw std::runtime_error("CPU editor harness unexpectedly captured a live Camera.");
}

std::string Client::CAnimationTargetService::Resolve_AssetName()
{
	throw std::runtime_error("CPU editor harness unexpectedly queried the live animation target.");
}

std::shared_ptr<Client::CCharacter> Client::CAnimationTargetService::Resolve_SceneCharacter()
{
	throw std::runtime_error("CPU editor harness unexpectedly queried the live Scene Character.");
}

bool_t Client::CAnimationTargetService::Resolve_ModelTarget(
	ANIMATION_BONE_TARGET, ANIMATION_MODEL_TARGET_VIEW&)
{
	throw std::runtime_error("CPU editor harness unexpectedly queried a live bone target.");
}
#endif

namespace
{
	class SCOPED_TEST_DIRECTORY final
	{
	public:
		explicit SCOPED_TEST_DIRECTORY(std::filesystem::path path)
			: m_Path(std::move(path))
		{
		}

		~SCOPED_TEST_DIRECTORY()
		{
			std::error_code error;
			std::filesystem::remove_all(m_Path, error);
		}

	private:
		std::filesystem::path m_Path;
	};

	class SCOPED_ENVIRONMENT_VARIABLE final
	{
	public:
		explicit SCOPED_ENVIRONMENT_VARIABLE(const wchar_t* name)
			: m_Name(name)
		{
			const DWORD required = GetEnvironmentVariableW(
				m_Name.c_str(), nullptr, 0u);
			if (0u == required)
				return;
			std::vector<wchar_t> value(required);
			const DWORD copied = GetEnvironmentVariableW(
				m_Name.c_str(), value.data(), required);
			if (0u != copied && copied < required)
			{
				m_HadValue = true;
				m_Previous.assign(value.data(), copied);
			}
		}

		~SCOPED_ENVIRONMENT_VARIABLE()
		{
			SetEnvironmentVariableW(m_Name.c_str(),
				m_HadValue ? m_Previous.c_str() : nullptr);
		}

		bool Set(const std::filesystem::path& value)
		{
			return FALSE != SetEnvironmentVariableW(
				m_Name.c_str(), value.c_str());
		}

	private:
		std::wstring m_Name;
		std::wstring m_Previous;
		bool m_HadValue = false;
	};

	void Require(const bool condition, const char* const message)
	{
		if (!condition)
			throw std::runtime_error(message);
	}

	std::string ReadText(const std::filesystem::path& path)
	{
		std::ifstream input(path, std::ios::binary);
		return { std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>() };
	}

	bool WriteText(const std::filesystem::path& path,
		const std::string_view text)
	{
		std::error_code error;
		std::filesystem::create_directories(path.parent_path(), error);
		if (error)
			return false;
		std::ofstream output(path, std::ios::binary | std::ios::trunc);
		output.write(text.data(), static_cast<std::streamsize>(text.size()));
		return output.good();
	}

	bool CopyFixture(const std::filesystem::path& source,
		const std::filesystem::path& destination)
	{
		std::error_code error;
		std::filesystem::create_directories(destination.parent_path(), error);
		if (error)
			return false;
		return std::filesystem::copy_file(source, destination,
			std::filesystem::copy_options::overwrite_existing, error) && !error;
	}

	bool ReplaceOnce(std::string& text,
		const std::string_view needle,
		const std::string_view replacement)
	{
		const std::size_t offset = text.find(needle);
		if (std::string::npos == offset)
			return false;
		text.replace(offset, needle.size(), replacement);
		return true;
	}

	struct CATALOG_SNAPSHOT final
	{
		std::string compositionId;
		std::string sequencerId;
		uint32_t bossRevision = 0u;
		uint32_t arenaRevision = 0u;
		std::size_t profileCount = 0u;
		std::size_t trackCount = 0u;

		bool operator==(const CATALOG_SNAPSHOT&) const = default;
	};

	CATALOG_SNAPSHOT Snapshot(
		const Client::CCompositionDocumentCatalog& catalog,
		const std::string& compositionId,
		const std::string& sequencerId)
	{
		const Client::CBossCompositionDocument* boss =
			catalog.Find_Boss(compositionId);
		const Client::CArenaSequencerDocument* arena =
			catalog.Find_Arena(sequencerId);
		Require(nullptr != boss && nullptr != arena,
			"selected descriptor pair is missing from the catalog");
		return {
			boss->Get_CompositionId(), arena->Get_SequencerId(),
			boss->Get_Revision(), arena->Get_Revision(),
			boss->Get_Coverage().profiles.size(), arena->Get_Tracks().size()
		};
	}

	struct FIXTURE final
	{
		std::filesystem::path testRoot;
		std::filesystem::path dataRoot;
		std::filesystem::path koukuSaydonBossPath;
		std::filesystem::path koukuSaydonArenaPath;
		std::filesystem::path valtanBossPath;
		std::filesystem::path valtanArenaPath;
		std::string koukuSaydonBossBaseline;
		std::string koukuSaydonArenaBaseline;
		std::string valtanBossBaseline;
		std::string valtanArenaBaseline;
		Client::CCompositionDocumentCatalog catalog;
		CATALOG_SNAPSHOT committed;
	};

	FIXTURE BuildFixture()
	{
		const std::filesystem::path repositoryDataRoot =
			Client::CProjectDataRoot::Get();
		FIXTURE fixture;
		fixture.testRoot = std::filesystem::temp_directory_path() /
			("LostArkBossCompositionDocumentHarness-" +
			 std::to_string(GetCurrentProcessId()) + "-" +
			 std::to_string(GetTickCount64()));
		fixture.dataRoot = fixture.testRoot / "Data";
		fixture.koukuSaydonBossPath = fixture.dataRoot /
			"Compositions/Bosses/KoukuSaydonGate1.bosscomposition.json";
		fixture.koukuSaydonArenaPath = fixture.dataRoot /
			"Compositions/Sequences/KoukuSaydonArena.sequencer.json";
		fixture.valtanBossPath = fixture.dataRoot /
			"Compositions/Bosses/Valtan.bosscomposition.json";
		fixture.valtanArenaPath = fixture.dataRoot /
			"Compositions/Sequences/ValtanArena.sequencer.json";
		Require(CopyFixture(repositoryDataRoot /
				"Compositions/Bosses/KoukuSaydonGate1.bosscomposition.json",
			fixture.koukuSaydonBossPath) &&
			CopyFixture(repositoryDataRoot /
				"Compositions/Sequences/KoukuSaydonArena.sequencer.json",
				fixture.koukuSaydonArenaPath) &&
			CopyFixture(repositoryDataRoot /
				"Compositions/Bosses/Valtan.bosscomposition.json",
				fixture.valtanBossPath) &&
			CopyFixture(repositoryDataRoot /
				"Compositions/Sequences/ValtanArena.sequencer.json",
				fixture.valtanArenaPath),
			"could not copy Composition descriptor fixtures");

		const std::vector<std::filesystem::path> sourceFiles{
			"Valtan/Valtan.gameplay.json",
			"Valtan/Valtan.presentation.json",
			"Valtan/Valtan.combatobjects.json",
			"Valtan/Valtan.worldeventsets.json",
			"Animation/Authored/Valtan/Valtan.patternbindings.json",
			"Animation/Authored/Valtan/Valtan.patterneffectcues.json",
			"Animation/Authored/Valtan/Valtan.patterneffectv1aliases.json",
			"Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json",
			"Animation/Authored/Valtan/Valtan.patternsoundcues.json",
			"Animation/Authored/Valtan/Valtan.patternshakecues.json",
			"Animation/Authored/Valtan/Valtan.combatobjectsoundcues.json",
			"Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.mapeffects.json",
			"Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.maplights.json",
			"Animation/Reference/KoukuSaydon/MN_RPCZ_00.actionreference.json",
			"Animation/Authored/KoukuSaydon/MN_RPCZ_00.actionbindings.json",
			"Animation/Authored/KoukuSaydon/MN_RPCZ_00.patternbindings.json",
			"Animation/Reference/KoukuSaydon/MN_RPCT_05.actionreference.json",
			"Animation/Authored/KoukuSaydon/MN_RPCT_05.actionbindings.json",
			"Animation/Authored/KoukuSaydon/MN_RPCT_05.patternbindings.json",
			"Animation/Reference/KoukuSaydon/MN_RPCT_06.actionreference.json",
			"Animation/Authored/KoukuSaydon/MN_RPCT_06.actionbindings.json",
			"Animation/Authored/KoukuSaydon/MN_RPCT_06.patternbindings.json",
			"Animation/Reference/KoukuSaydon/MN_RPCT_07.actionreference.json",
			"Animation/Authored/KoukuSaydon/MN_RPCT_07.actionbindings.json",
			"Animation/Authored/KoukuSaydon/MN_RPCT_07.patternbindings.json",
			"Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json",
			"Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json",
			"Rendering/Authored/RenderingProfiles.json",
			"Items/ItemCatalog.json",
		};
		for (const std::filesystem::path& relative : sourceFiles)
		{
			Require(WriteText(fixture.dataRoot / relative, "{}\n"),
				"could not create a Composition owner placeholder");
		}
		fixture.koukuSaydonBossBaseline = ReadText(fixture.koukuSaydonBossPath);
		fixture.koukuSaydonArenaBaseline = ReadText(fixture.koukuSaydonArenaPath);
		fixture.valtanBossBaseline = ReadText(fixture.valtanBossPath);
		fixture.valtanArenaBaseline = ReadText(fixture.valtanArenaPath);
		return fixture;
	}

	void RequirePreserved(
		const Client::CCompositionDocumentCatalog& catalog,
		const CATALOG_SNAPSHOT& expected,
		const std::string& compositionId,
		const std::string& sequencerId)
	{
		Require(Snapshot(catalog, compositionId, sequencerId) == expected,
			"failed pair load replaced the last committed descriptor pair");
	}

	void VerifyFixedSourceClosureAndRollback()
	{
		FIXTURE fixture = BuildFixture();
		SCOPED_TEST_DIRECTORY cleanup(fixture.testRoot);
		SCOPED_ENVIRONMENT_VARIABLE dataEnvironment(
			L"LOSTARK_PROJECT_DATA_ROOT");
		Require(dataEnvironment.Set(fixture.dataRoot),
			"could not redirect the Composition Data root");

		std::string status;
		Require(fixture.catalog.Load_Pair(
				"boss.composition.kakulsaydon",
				"arena.sequencer.kakulsaydon", status),
			("valid KoukuSaydon descriptor pair failed: " + status).c_str());
		fixture.committed = Snapshot(fixture.catalog,
			"boss.composition.kakulsaydon",
			"arena.sequencer.kakulsaydon");

		std::string wrongArenaPath = fixture.koukuSaydonArenaBaseline;
		Require(ReplaceOnce(wrongArenaPath,
				"Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json",
				"Data/Items/ItemCatalog.json") &&
			WriteText(fixture.koukuSaydonArenaPath, wrongArenaPath),
			"could not stage the wrong Arena source path fixture");
		Require(!fixture.catalog.Load_Pair(
				"boss.composition.kakulsaydon",
				"arena.sequencer.kakulsaydon", status) &&
			status.find("source role/path closure") != std::string::npos,
			"native Arena parser accepted a Python-rejected source path");
		RequirePreserved(fixture.catalog, fixture.committed,
			"boss.composition.kakulsaydon",
			"arena.sequencer.kakulsaydon");

		Require(WriteText(fixture.koukuSaydonArenaPath, fixture.koukuSaydonArenaBaseline),
			"could not restore the Arena descriptor fixture");
		std::string wrongBossRole = fixture.koukuSaydonBossBaseline;
		Require(ReplaceOnce(wrongBossRole,
				"ACTION_REFERENCE_MN_RPCZ_00",
				"ACTION_REFERENCE_WRONG") &&
			WriteText(fixture.koukuSaydonBossPath, wrongBossRole),
			"could not stage the wrong Boss source role fixture");
		Require(!fixture.catalog.Load_Pair(
				"boss.composition.kakulsaydon",
				"arena.sequencer.kakulsaydon", status) &&
			status.find("source role/path closure") != std::string::npos,
			"native Boss parser accepted a Python-rejected source role");
		RequirePreserved(fixture.catalog, fixture.committed,
			"boss.composition.kakulsaydon",
			"arena.sequencer.kakulsaydon");

		Require(WriteText(fixture.koukuSaydonBossPath, fixture.koukuSaydonBossBaseline),
			"could not restore the Boss descriptor fixture");
		std::string extraArenaSource = fixture.koukuSaydonArenaBaseline;
		const std::size_t sourceStart =
			extraArenaSource.find("\"sourceDocuments\"");
		const std::size_t sourceEnd = extraArenaSource.find("\n  ],", sourceStart);
		Require(std::string::npos != sourceStart &&
			std::string::npos != sourceEnd,
			"could not locate the Arena sourceDocuments fixture");
		extraArenaSource.insert(sourceEnd,
			",\n    {\n      \"role\": \"UNUSED\",\n"
			"      \"path\": \"Data/Items/ItemCatalog.json\"\n    }");
		Require(WriteText(fixture.koukuSaydonArenaPath, extraArenaSource),
			"could not stage the extra Arena source fixture");
		Require(!fixture.catalog.Load_Pair(
				"boss.composition.kakulsaydon",
				"arena.sequencer.kakulsaydon", status) &&
			status.find("source role/path closure") != std::string::npos,
			"native Arena parser accepted a Python-rejected extra source");
		RequirePreserved(fixture.catalog, fixture.committed,
			"boss.composition.kakulsaydon",
			"arena.sequencer.kakulsaydon");

		Require(WriteText(fixture.koukuSaydonArenaPath,
			fixture.koukuSaydonArenaBaseline),
			"could not restore the extra Arena source fixture");
		Client::CCompositionDocumentCatalog valtanCatalog;
		Require(valtanCatalog.Load_Pair(
				"boss.composition.valtan", "arena.sequencer.valtan", status),
			("valid Valtan descriptor pair failed: " + status).c_str());
		const CATALOG_SNAPSHOT valtanCommitted = Snapshot(valtanCatalog,
			"boss.composition.valtan", "arena.sequencer.valtan");

		std::string wrongValtanBossRole = fixture.valtanBossBaseline;
		Require(ReplaceOnce(wrongValtanBossRole,
				"\"role\": \"GAMEPLAY\"", "\"role\": \"WRONG_GAMEPLAY\"") &&
			WriteText(fixture.valtanBossPath, wrongValtanBossRole),
			"could not stage the wrong Valtan Boss source role fixture");
		Require(!valtanCatalog.Load_Pair(
				"boss.composition.valtan", "arena.sequencer.valtan", status) &&
			status.find("source role/path closure") != std::string::npos,
			"native Valtan Boss parser accepted a Python-rejected source role");
		RequirePreserved(valtanCatalog, valtanCommitted,
			"boss.composition.valtan", "arena.sequencer.valtan");

		Require(WriteText(fixture.valtanBossPath,
			fixture.valtanBossBaseline),
			"could not restore the Valtan Boss descriptor fixture");
		std::string wrongValtanArenaPath = fixture.valtanArenaBaseline;
		Require(ReplaceOnce(wrongValtanArenaPath,
				"Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.mapeffects.json",
				"Data/Items/ItemCatalog.json") &&
			WriteText(fixture.valtanArenaPath, wrongValtanArenaPath),
			"could not stage the wrong Valtan Arena source path fixture");
		Require(!valtanCatalog.Load_Pair(
				"boss.composition.valtan", "arena.sequencer.valtan", status) &&
			status.find("source role/path closure") != std::string::npos,
			"native Valtan Arena parser accepted a Python-rejected source path");
		RequirePreserved(valtanCatalog, valtanCommitted,
			"boss.composition.valtan", "arena.sequencer.valtan");
	}
}

int Run_BossCompositionDocumentContractTests()
{
	try
	{
		VerifyFixedSourceClosureAndRollback();
		std::cout << "BossCompositionDocumentContractTests: 7/7 passed\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << "BossCompositionDocumentContractTests: FAIL: " <<
			error.what() << '\n';
		return 1;
	}
}


namespace
{
	void RequireEditorStep(const bool succeeded, const std::string& status,
		const char* const step)
	{
		if (!succeeded)
			throw std::runtime_error(std::string(step) + ": " + status);
	}

	const Client::KOUKU_SAYDON_COMPOSITION_PATTERN& EditorPattern(
		const Client::CKoukuSaydonActionWorkbench& workbench,
		const std::string& patternId)
	{
		const auto& patterns = workbench.Get_Composition().Patterns;
		const auto found = std::find_if(patterns.begin(), patterns.end(),
			[&](const auto& pattern) { return pattern.strPatternId == patternId; });
		Require(found != patterns.end(), "editor Pattern was lost");
		return *found;
	}

	void RequireEditorRoundtrip(Client::CKoukuSaydonActionWorkbench& workbench)
	{
		std::string status;
		RequireEditorStep(workbench.Save(status), status, "save editor draft");
		Require(!workbench.Is_Dirty(), "successful Save left the draft dirty");
		const auto saved = workbench.Get_Composition();
		RequireEditorStep(workbench.Reload(status), status, "reload saved editor draft");
		Require(workbench.Get_Composition() == saved,
			"Save/Reload changed Stage, occurrence, profile, timing or stable ID");
	}

	void VerifyKoukuSequenceDocumentIsolation(const std::filesystem::path& actionPath,
		const std::filesystem::path& sequencePath)
	{
		using namespace Client;
		const auto actionBytes = ReadText(actionPath);
		const auto sequenceBytes = ReadText(sequencePath);
		std::string status;
		CKoukuSaydonCompositionDocument action(actionPath), sequence(sequencePath);
		Require(CKoukuSaydonCompositionDocument::Resolve_Path() == actionPath &&
			CKoukuSaydonCompositionDocument::Resolve_SequencePath() == sequencePath,
			"Action and Sequence resolvers do not select their independent Data files");
		RequireEditorStep(action.Reload(status), status, "load Action owner for isolation");
		RequireEditorStep(sequence.Reload(status), status, "load independent Sequence seed");
		const auto actionGood = action.Get_LastGood();
		const auto sequenceGood = sequence.Get_LastGood();
		std::vector<const KOUKU_SAYDON_COMPOSITION_PATTERN*> gateOneSequences;
		for (const auto& row : sequenceGood.Patterns)
		{
			RequireEditorStep(row.strLoadError.empty(), row.strLoadError,
				("admit Sequence seed " + row.strPatternId).c_str());
			if (row.strGateId == "GATE1") gateOneSequences.push_back(&row);
		}
		Require(sequenceGood.strCompositionId == "boss.composition.kakulsaydon.sequencer" &&
			sequenceGood.iRevision >= 2u && gateOneSequences.size() == 2u &&
			std::all_of(sequenceGood.Patterns.begin(), sequenceGood.Patterns.end(), [](const auto& row) {
				return row.strLoadError.empty() && row.strAuthoringStatus == "DRAFT" &&
					!row.WorldOccurrences.empty() && !row.PresentationOccurrences.empty(); }),
			"Sequence seed did not retain both valid World/Camera DRAFT timelines");
		const auto& opening = *gateOneSequences.front();
		Require(opening.WorldOccurrences.size() == 7u && opening.Stages.size() == 1u &&
			opening.Stages.front().iDurationMs == 37800u &&
			std::all_of(opening.WorldOccurrences.begin(), opening.WorldOccurrences.begin() + 5,
				[](const auto& box) { return box.iStartMs == 0u && box.iDurationMs == 4507u && box.fPlaybackSpeed == 1.f; }) &&
			opening.WorldOccurrences[5].iDurationMs == 37800u && opening.WorldOccurrences[6].iDurationMs == 37800u,
			"opening map must finish unfolding at native 4507ms while book and Saydon keep the 37800ms scene");
		CKoukuSaydonActionWorkbench sequenceWorkbench(true);
		RequireEditorStep(sequenceWorkbench.Reload(status), status, "load Sequence playback workspace");
		const auto firstSequenceId = gateOneSequences[0]->strPatternId;
		const auto secondSequenceId = gateOneSequences[1]->strPatternId;
		const auto consumeSequence = [&](const std::string& expectedId, const bool expectedPaused = false) {
			KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
			std::uint32_t clockMs = 999u;
			bool_t paused = false;
			std::string target;
			Require(sequenceWorkbench.Consume_PatternPreviewRequest(pattern, clockMs, paused, target) &&
				pattern.strPatternId == expectedId && clockMs == 0u && paused == expectedPaused && target.empty(),
				"Complete Play lost the source order, zero start, pause state or existing preview route");
			Require(!sequenceWorkbench.Consume_PatternPreviewRequest(pattern, clockMs, paused, target),
				"Complete Play submitted the same sequence twice");
		};
		RequireEditorStep(sequenceWorkbench.Select_PatternById(secondSequenceId, status), status,
			"select finale before Complete Play");
		RequireEditorStep(sequenceWorkbench.Request_CompleteSequencePlay(status), status,
			"Complete Play always begins at the first Gate sequence");
		Require(sequenceWorkbench.Get_SelectedPatternId() == firstSequenceId &&
			!sequenceWorkbench.Advance_CompleteSequencePlay(firstSequenceId),
			"unadmitted sequence advanced or Complete Play kept the finale selection");
		Require(sequenceWorkbench.Request_PreviewPause(), "queued Complete Play could not pause");
		consumeSequence(firstSequenceId, true);
		sequenceWorkbench.Notify_SequencePreviewAdmission(true, "paused at zero");
		Require(!sequenceWorkbench.Advance_CompleteSequencePlay(secondSequenceId),
			"unrelated completion advanced the active sequence");
		Require(sequenceWorkbench.Advance_CompleteSequencePlay(firstSequenceId) &&
			sequenceWorkbench.Get_SelectedPatternId() == secondSequenceId &&
			!sequenceWorkbench.Advance_CompleteSequencePlay(firstSequenceId),
			"natural completion failed to queue the finale exactly once");
		consumeSequence(secondSequenceId);
		sequenceWorkbench.Notify_SequencePreviewAdmission(false, "WORLD source unavailable");
		Require(!sequenceWorkbench.Is_CompleteSequencePlaying() &&
			!sequenceWorkbench.Advance_CompleteSequencePlay(secondSequenceId) &&
			sequenceWorkbench.Get_Status().find("WORLD source unavailable") != std::string::npos,
			"failed admission advanced Complete Play or discarded the failure reason");
		RequireEditorStep(sequenceWorkbench.Request_CompleteSequencePlay(status), status, "restart complete run");
		consumeSequence(firstSequenceId);
		sequenceWorkbench.Notify_SequencePreviewAdmission(true, "first ready");
		Require(sequenceWorkbench.Advance_CompleteSequencePlay(firstSequenceId), "first sequence did not finish");
		consumeSequence(secondSequenceId);
		sequenceWorkbench.Notify_SequencePreviewAdmission(true, "second ready");
		std::string completedGate = "previous";
		Require(sequenceWorkbench.Advance_CompleteSequencePlay(secondSequenceId, &completedGate) &&
			!sequenceWorkbench.Is_CompleteSequencePlaying() && completedGate == "GATE1",
			"Complete Play looped after the finale or did not hand off its exact Gate");
		Require(!sequenceWorkbench.Advance_CompleteSequencePlay(secondSequenceId, &completedGate) && completedGate.empty(),
			"duplicate completion handed the Gate to battle twice");
		std::vector<std::string> gateTwoSequences;
		for (const auto& row : sequenceGood.Patterns)
			if (row.strGateId == "GATE2") gateTwoSequences.push_back(row.strPatternId);
		Require(!gateTwoSequences.empty(), "Sequence seed has no Gate 2 intro");
		RequireEditorStep(sequenceWorkbench.Request_CompleteSequencePlay(status, "GATE2"), status,
			"explicit Gate 2 Complete Play from Gate 1 selection");
		for (std::size_t i = 0u; i < gateTwoSequences.size(); ++i)
		{
			consumeSequence(gateTwoSequences[i]);
			sequenceWorkbench.Notify_SequencePreviewAdmission(true, "Gate 2 ready");
			Require(sequenceWorkbench.Advance_CompleteSequencePlay(gateTwoSequences[i], &completedGate),
				"Gate 2 sequence did not finish");
			Require(completedGate == (i + 1u == gateTwoSequences.size() ? "GATE2" : ""),
				"Gate 2 completion handed off another Gate or started battle before its finale");
		}
		RequireEditorStep(sequenceWorkbench.Select_PatternById(firstSequenceId, status), status,
			"return to Gate 1 after Gate 2 Complete Play");
		sequenceWorkbench.Set_CompleteSequenceAdmission([](std::string_view gate, std::string& reason) {
			reason = "saved flow unavailable for " + std::string(gate); return false;
		});
		Require(!sequenceWorkbench.Request_CompleteSequencePlay(status, "GATE1") &&
			!sequenceWorkbench.Is_CompleteSequencePlaying() && status == "saved flow unavailable for GATE1",
			"failed battle preflight started the intro or discarded its failure");
		sequenceWorkbench.Set_CompleteSequenceAdmission({});
		RequireEditorStep(sequenceWorkbench.Request_CompleteSequencePlay(status), status, "queue before owner loss");
		sequenceWorkbench.Cancel_CompleteSequencePlay();
		KOUKU_SAYDON_COMPOSITION_PATTERN cancelled;
		std::uint32_t cancelledClock = 0u; bool_t cancelledPaused = false; std::string cancelledTarget;
		Require(!sequenceWorkbench.Is_CompleteSequencePlaying() &&
			!sequenceWorkbench.Consume_PatternPreviewRequest(cancelled, cancelledClock, cancelledPaused, cancelledTarget),
			"owner loss retained an automatic next-sequence request");
		sequenceWorkbench.Select_WorkbenchBoss(COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_GATE3);
		Require(!sequenceWorkbench.Request_CompleteSequencePlay(status), "empty Gate replayed another Gate's sequences");
		CKoukuSaydonActionWorkbench actionWorkbench;
		Require(!actionWorkbench.Request_CompleteSequencePlay(status), "Action workspace admitted Sequence Complete Play");
		Require(!sequenceWorkbench.Is_Dirty() && ReadText(sequencePath) == sequenceBytes && ReadText(actionPath) == actionBytes,
			"Complete Play or its failure path changed the authored documents");
		auto wrongAction = actionGood;
		wrongAction.strCompositionId = sequenceGood.strCompositionId;
		Require(!action.Save_Atomic(wrongAction, status) && action.Get_LastGood() == actionGood &&
			action.Is_Fresh() && ReadText(actionPath) == actionBytes,
			"Sequence identity overwrote the Action source or invalidated a valid baseline");
		auto wrongSequence = sequenceGood;
		wrongSequence.strCompositionId = actionGood.strCompositionId;
		Require(!sequence.Save_Atomic(wrongSequence, status) && sequence.Get_LastGood() == sequenceGood &&
			sequence.Is_Fresh() && ReadText(sequencePath) == sequenceBytes,
			"Action identity overwrote the independent Sequence source");

		auto candidate = sequenceGood;
		candidate.Patterns.front().strDisplayName = "Native independent Sequence save";
		RequireEditorStep(sequence.Save_Atomic(candidate, status), status, "save independent Sequence");
		const auto saved = sequence.Get_LastGood();
		const auto savedBytes = ReadText(sequencePath);
		Require(saved.iRevision == sequenceGood.iRevision + 1u && ReadText(actionPath) == actionBytes,
			"Sequence Save changed the Action source or failed to advance only its own revision");
		const auto externalBytes = savedBytes + "\n";
		Require(WriteText(sequencePath, externalBytes), "could not stage Sequence CAS conflict");
		Require(!sequence.Save_Atomic(saved, status) && !sequence.Is_Fresh() &&
			sequence.Get_LastGood() == saved && ReadText(sequencePath) == externalBytes &&
			ReadText(actionPath) == actionBytes,
			"Sequence CAS conflict did not preserve the external file and last-good state");
		RequireEditorStep(sequence.Reload(status), status, "reload Sequence after CAS conflict");
		Require(sequence.Get_LastGood() == saved, "Sequence CAS recovery changed the saved timeline");

		Require(WriteText(sequencePath, actionBytes), "could not stage wrong identity at Sequence path");
		Require(!sequence.Reload(status) && sequence.Get_LastGood() == saved &&
			ReadText(sequencePath) == actionBytes && ReadText(actionPath) == actionBytes,
			"Sequence Reload admitted Action identity or overwrote either source");
		Require(WriteText(actionPath, sequenceBytes), "could not stage wrong identity at Action path");
		Require(!action.Reload(status) && action.Get_LastGood() == actionGood &&
			ReadText(actionPath) == sequenceBytes,
			"Action Reload admitted Sequence identity or replaced its last-good state");
		Require(WriteText(actionPath, actionBytes) && WriteText(sequencePath, sequenceBytes),
			"could not restore isolated scratch documents");
	}

	void VerifyLegacyEditorMigration(const std::filesystem::path& sourcePath)
	{
		using namespace Client;
		const std::string original = ReadText(sourcePath);
		KOUKU_SAYDON_COMPOSITION_DOCUMENT source;
		std::string status;
		RequireEditorStep(CKoukuSaydonCompositionDocument::Parse_Text(original, source, status),
			status, "parse current composition for legacy fixture");
		const auto legacyOwner = source.Patterns.front().strActorProfileId;
		const auto fixture = std::find_if(source.Patterns.begin(), source.Patterns.end(),
			[](const auto& pattern) { return pattern.strLoadError.empty() && pattern.Stages.size() >= 2u &&
				std::all_of(pattern.Stages.begin(), pattern.Stages.end(), [](const auto& stage) { return !stage.AnimationOccurrences.empty(); }); });
		Require(fixture != source.Patterns.end(), "real multi-Stage fixture is missing");
		auto mixed = *fixture;
		mixed.strPatternId = "KAKULSAYDON_G1_NATIVE_MIXED_LEGACY";
		mixed.strAuthoringStatus = "DRAFT";
		for (std::size_t index = 0u; index < mixed.Stages.size(); ++index)
		{
			mixed.Stages[index].strActionId = mixed.strPatternId + ".stage." +
				std::to_string(index + 1u);
			mixed.Stages[index].AnimationOccurrences.at(0u).strOccurrenceId =
				mixed.strPatternId + ".animation." + std::to_string(index + 1u);
		}
		mixed.Stages[0].AnimationOccurrences[0].strProfileId = fixture->strActorProfileId == "MN_RPCT_05" ? "MN_RPCZ_00" : "MN_RPCT_05";
		source.Patterns.push_back(mixed);
		std::string serialized = CKoukuSaydonCompositionDocument::Serialize(source);
		Require(ReplaceOnce(serialized, "\"formatVersion\": 3", "\"formatVersion\": 1"),
			"could not form a legacy v1 document");
		std::istringstream lines(serialized);
		std::string line;
		std::string legacy;
		while (std::getline(lines, line))
			if (line.find("\"actorProfileId\"") == std::string::npos)
				legacy += line + "\n";
		Require(WriteText(sourcePath, legacy), "could not write legacy scratch fixture");
		CKoukuSaydonCompositionDocument document;
		RequireEditorStep(document.Reload_FromPath(sourcePath, status), status,
			"load legacy document with mixed-model quarantine");
		const auto candidate = document.Get_LastGood();
		Require(candidate.iFormatVersion == 3u &&
			candidate.Patterns[0].strActorProfileId == legacyOwner &&
			!candidate.Patterns.back().strLoadError.empty() &&
			!candidate.Patterns.back().strPreservedJson.empty(),
			"v1 migration lost deterministic owner or mixed-model raw quarantine");
		const std::string preserved = candidate.Patterns.back().strPreservedJson;
		RequireEditorStep(document.Save_Atomic(candidate, status), status,
			"save migrated v3 document with quarantine");
		CKoukuSaydonCompositionDocument reopened;
		RequireEditorStep(reopened.Reload_FromPath(sourcePath, status), status,
			"reopen migrated v3 document");
		Require(reopened.Get_LastGood().iFormatVersion == 3u &&
			reopened.Get_LastGood().Patterns.back().strPreservedJson == preserved &&
			ReadText(sourcePath).find("\"formatVersion\": 3") != std::string::npos,
			"v3 Save/Reload lost the quarantined legacy source JSON");
		Require(WriteText(sourcePath, original), "could not restore scratch source for editor test");
	}

	void VerifyKoukuGateBundleStorage(const std::filesystem::path& sourcePath)
	{
		using namespace Client;
		const auto original = ReadText(sourcePath);
		std::string status;
		CKoukuSaydonCompositionDocument owner;
		RequireEditorStep(owner.Reload_FromPath(sourcePath, status), status, "load hierarchy scratch source");
		auto candidate = owner.Get_LastGood();
		KOUKU_SAYDON_COMPOSITION_FOLDER folder;
		folder.strFolderId = "kakulsaydon.folder." + std::to_string(candidate.iNextFolderOrdinal++);
		folder.strGateId = "GATE2"; folder.strDisplayName = "Native Gate 2 folder";
		candidate.Folders.push_back(folder);
		auto siblingFolder = folder;
		siblingFolder.strFolderId = "kakulsaydon.folder." + std::to_string(candidate.iNextFolderOrdinal++);
		siblingFolder.strDisplayName = "Native second Gate 2 folder";
		candidate.Folders.push_back(siblingFolder);
		auto foreignFolder = folder;
		foreignFolder.strFolderId = "kakulsaydon.folder." + std::to_string(candidate.iNextFolderOrdinal++);
		foreignFolder.strGateId = "GATE1";
		foreignFolder.strDisplayName = "Native other Gate folder";
		candidate.Folders.push_back(foreignFolder);
		KOUKU_SAYDON_COMPOSITION_BUNDLE bundle;
		bundle.strBundleId = "kakulsaydon.bundle." + std::to_string(candidate.iNextBundleOrdinal++);
		bundle.strGateId = folder.strGateId; bundle.strFolderId = folder.strFolderId;
		bundle.strDisplayName = "Native parallel bundle";
		auto emptyBundle = bundle;
		emptyBundle.strBundleId = "kakulsaydon.bundle." + std::to_string(candidate.iNextBundleOrdinal++);
		emptyBundle.strFolderId = siblingFolder.strFolderId;
		emptyBundle.strDisplayName = "Native empty bundle";
		candidate.Bundles.push_back(emptyBundle);
		KOUKU_SAYDON_ACTION_REFERENCE_SET references;
		RequireEditorStep(CKoukuSaydonCompositionDocument::Load_ImmutableActionReferences(references, status), status, "load hierarchy references");
		candidate.Bundles.push_back(bundle);
		RequireEditorStep(CKoukuSaydonCompositionDocument::Validate(candidate, references, status), status, "validate empty DRAFT folder and bundle");
		for (const auto* actor : {"MN_RPCZ_00", "MN_RPCT_06"})
		{
			KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
			pattern.strPatternId = "KAKULSAYDON_G1_PATTERN_" + std::to_string(candidate.iNextPatternOrdinal++);
			pattern.strDisplayName = std::string("Native child ") + actor; pattern.strAuthoringStatus = "DRAFT";
			pattern.strCategory = "NORMAL"; pattern.strActorProfileId = actor; pattern.strGateId = "GATE2";
			pattern.strTargetBossPlacementId = CKoukuSaydonCompositionDocument::Resolve_DefaultPlacementId(pattern.strGateId, actor);
			auto& target = candidate.Bundles.back();
			target.Members.push_back({target.strBundleId + ".member." + std::to_string(target.iNextMemberOrdinal++), pattern.strPatternId, 0u});
			candidate.Patterns.push_back(pattern);
		}
		RequireEditorStep(CKoukuSaydonCompositionDocument::Validate(candidate, references, status), status, "validate distinct Gate 2 targets");
		KOUKU_SAYDON_COMPOSITION_DOCUMENT parsed;
		RequireEditorStep(CKoukuSaydonCompositionDocument::Parse_Text(CKoukuSaydonCompositionDocument::Serialize(candidate), parsed, status), status, "roundtrip Gate 2 hierarchy");
		Require(parsed == candidate, "hierarchy roundtrip changed IDs, targets, references or counters");
		RequireEditorStep(owner.Save_Atomic(candidate, status), status, "save Gate 2 hierarchy");
		const auto saved = owner.Get_LastGood();
		CKoukuSaydonCompositionDocument reopened;
		RequireEditorStep(reopened.Reload_FromPath(sourcePath, status), status, "reload saved Gate 2 hierarchy");
		Require(reopened.Get_LastGood() == saved, "saved Gate 2 hierarchy failed exact reload");
		auto invalid = saved;
		auto duplicateTarget = invalid.Patterns[invalid.Patterns.size() - 2u];
		duplicateTarget.strPatternId = "KAKULSAYDON_G1_PATTERN_" + std::to_string(invalid.iNextPatternOrdinal++);
		invalid.Patterns.push_back(duplicateTarget);
		invalid.Bundles.back().Members.back().strPatternId = duplicateTarget.strPatternId;
		Require(!CKoukuSaydonCompositionDocument::Validate(invalid, references, status), "distinct Patterns targeting one boss were admitted to a bundle");
		const auto savedBytes = ReadText(sourcePath);
		Require(!reopened.Save_Atomic(invalid, status) && reopened.Get_LastGood() == saved && ReadText(sourcePath) == savedBytes,
			"invalid duplicate target changed saved hierarchy or admitted state");

		CKoukuSaydonActionWorkbench workbench;
		RequireEditorStep(workbench.Reload(status), status, "load hierarchy for native Pattern creation");
		workbench.Select_WorkbenchBoss(COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_GATE2);
		RequireEditorStep(workbench.Select_ActorProfile("MN_RPCT_06", status), status,
			"select Gate 2 Kouku for direct Parent creation");
		RequireEditorStep(workbench.Set_PatternCreationDestination(folder.strFolderId, {}, status), status,
			"select a Parent with Bundle None");
		const auto beforeCreation = workbench.Get_Composition();
		const auto generationBeforeDestination = workbench.Get_DraftGeneration();
		for (const auto& rejectedDestination : std::vector<std::pair<std::string, std::string>>{
			{ "kakulsaydon.folder.999999", {} }, { foreignFolder.strFolderId, {} },
			{ folder.strFolderId, emptyBundle.strBundleId }, { {}, bundle.strBundleId },
			{ folder.strFolderId, "kakulsaydon.bundle.999999" } })
		{
			Require(!workbench.Set_PatternCreationDestination(rejectedDestination.first,
				rejectedDestination.second, status) && !status.empty() &&
				workbench.Get_Composition() == beforeCreation && !workbench.Is_Dirty() &&
				workbench.Get_DraftGeneration() == generationBeforeDestination && ReadText(sourcePath) == savedBytes,
				"invalid Parent or mismatched Bundle changed the document or saved source");
		}
		std::string directPatternId;
		RequireEditorStep(workbench.Create_Pattern("Native direct Parent child", "NORMAL",
			directPatternId, status), status, "create a Pattern under the retained Parent without a Bundle");
		const auto& directPattern = EditorPattern(workbench, directPatternId);
		Require(directPattern.strGateId == "GATE2" && directPattern.strActorProfileId == "MN_RPCT_06" &&
			directPattern.strFolderId == folder.strFolderId && directPattern.Stages.empty() &&
			workbench.Get_Composition().Folders == beforeCreation.Folders &&
			workbench.Get_Composition().Bundles == beforeCreation.Bundles &&
			workbench.Get_Composition().iNextBundleOrdinal == beforeCreation.iNextBundleOrdinal,
			"Bundle None lost its Parent after rejection or created an unwanted Bundle/member");
		for (const auto& previous : beforeCreation.Patterns)
			Require(EditorPattern(workbench, previous.strPatternId) == previous,
				"direct Parent creation changed an existing Gate or Bundle Pattern");
		RequireEditorRoundtrip(workbench);
		Require(EditorPattern(workbench, directPatternId).strFolderId == folder.strFolderId,
			"Save/Reload lost direct Parent placement");
		CKoukuSaydonCompositionDocument directOwner;
		RequireEditorStep(directOwner.Reload_FromPath(sourcePath, status), status,
			"load saved direct Parent placement for invalid-reference checks");
		const auto directSaved = directOwner.Get_LastGood();
		const auto directSavedBytes = ReadText(sourcePath);
		for (const auto& rejectedParent : { std::string("kakulsaydon.folder.999999"), foreignFolder.strFolderId })
		{
			auto invalidParent = directSaved;
			invalidParent.Patterns.back().strFolderId = rejectedParent;
			Require(!CKoukuSaydonCompositionDocument::Validate(invalidParent, references, status),
				"missing or cross-Gate direct Parent was admitted by document validation");
			KOUKU_SAYDON_COMPOSITION_DOCUMENT isolated;
			RequireEditorStep(CKoukuSaydonCompositionDocument::Parse_Text(
				CKoukuSaydonCompositionDocument::Serialize(invalidParent), isolated, status), status,
				"parse an invalid direct Parent while retaining the other hierarchy rows");
			Require(isolated.Patterns.back().strPatternId == directPatternId &&
				!isolated.Patterns.back().strLoadError.empty() &&
				isolated.Patterns.back().strPreservedJson.find(rejectedParent) != std::string::npos &&
				isolated.Folders == directSaved.Folders && isolated.Bundles == directSaved.Bundles,
				"invalid direct Parent lost its raw Pattern or damaged valid hierarchy rows");
			Require(!directOwner.Save_Atomic(invalidParent, status) &&
				directOwner.Get_LastGood() == directSaved && ReadText(sourcePath) == directSavedBytes,
				"invalid direct Parent overwrote the saved hierarchy or admitted state");
		}

		RequireEditorStep(workbench.Set_PatternCreationDestination({}, {}, status), status,
			"select legacy Gate-root creation with Parent and Bundle None");
		std::string gatePatternId;
		RequireEditorStep(workbench.Create_Pattern("Native Gate-root child", "NORMAL",
			gatePatternId, status), status, "create a compatible Gate-root Pattern");
		Require(EditorPattern(workbench, gatePatternId).strFolderId.empty() &&
			EditorPattern(workbench, gatePatternId).strGateId == "GATE2" &&
			workbench.Get_Composition().Bundles == beforeCreation.Bundles,
			"Gate-root creation retained a previous Parent or created a Bundle member");
		RequireEditorRoundtrip(workbench);

		const auto beforeBundleCreation = workbench.Get_Composition();
		RequireEditorStep(workbench.Set_PatternCreationDestination(siblingFolder.strFolderId,
			emptyBundle.strBundleId, status), status, "select a Bundle under its matching Parent");
		std::string bundlePatternId;
		RequireEditorStep(workbench.Create_Pattern("Native bundled child", "NORMAL",
			bundlePatternId, status), status, "create a Bundle member through the destination API");
		Require(EditorPattern(workbench, bundlePatternId).strFolderId.empty(),
			"Bundle member also acquired direct Parent ownership");
		for (const auto& previous : beforeBundleCreation.Bundles)
		{
			auto expectedBundle = previous;
			if (previous.strBundleId == emptyBundle.strBundleId)
				expectedBundle.Members.push_back({ expectedBundle.strBundleId + ".member." +
					std::to_string(expectedBundle.iNextMemberOrdinal++), bundlePatternId, 0u });
			const auto& bundles = workbench.Get_Composition().Bundles;
			const auto actual = std::find_if(bundles.begin(), bundles.end(), [&](const auto& row) {
				return row.strBundleId == previous.strBundleId; });
			Require(actual != bundles.end() && *actual == expectedBundle,
				"Bundle creation changed another Bundle or lost its exact member identity");
		}
		RequireEditorRoundtrip(workbench);
		RequireEditorStep(workbench.Append_ActionAsStages(bundlePatternId, "MN_RPCT_06", 4221801u, status),
			status, "give the existing reparent target real Stage and animation content");
		RequireEditorRoundtrip(workbench);
		for (const auto& destination : { folder.strFolderId, std::string{} })
		{
			auto expectedReparent = workbench.Get_Composition();
			const auto target = std::find_if(expectedReparent.Patterns.begin(), expectedReparent.Patterns.end(),
				[&](const auto& row) { return row.strPatternId == bundlePatternId; });
			Require(target != expectedReparent.Patterns.end() && !target->Stages.empty() &&
				!target->Stages.front().AnimationOccurrences.empty(), "reparent fixture lost authored clips");
			target->strFolderId = destination;
			RequireEditorStep(workbench.Set_PatternFolder(bundlePatternId, destination, status), status,
				destination.empty() ? "move an existing Pattern to Gate root" : "move an existing Pattern to another Parent");
			Require(workbench.Get_Composition() == expectedReparent,
				"Pattern reparent changed Stage/action/clip content, Bundle references or unrelated metadata");
			RequireEditorRoundtrip(workbench);
			Require(EditorPattern(workbench, bundlePatternId).strFolderId == destination,
				"Save/Reload lost the existing Pattern's selected Parent");
		}
		const auto reparentSaved = workbench.Get_Composition();
		const auto reparentGeneration = workbench.Get_DraftGeneration();
		const auto reparentBytes = ReadText(sourcePath);
		for (const auto& rejectedParent : { std::string("kakulsaydon.folder.999999"), foreignFolder.strFolderId })
			Require(!workbench.Set_PatternFolder(bundlePatternId, rejectedParent, status) && !status.empty() &&
				workbench.Get_Composition() == reparentSaved && !workbench.Is_Dirty() &&
				workbench.Get_DraftGeneration() == reparentGeneration && ReadText(sourcePath) == reparentBytes,
				"invalid existing-Pattern Parent changed saved content or dirty state");
		Require(!workbench.Set_PatternFolder("MISSING_PATTERN", folder.strFolderId, status) &&
			workbench.Get_Composition() == reparentSaved && !workbench.Is_Dirty() &&
			workbench.Get_DraftGeneration() == reparentGeneration && ReadText(sourcePath) == reparentBytes,
			"reparenting a missing Pattern changed the admitted document");
		Require(WriteText(sourcePath, original), "could not restore hierarchy scratch fixture");
	}

	void VerifyKoukuObjectContactAuthoring(Client::CKoukuSaydonActionWorkbench& workbench,
		const std::filesystem::path& sourcePath, const std::string& patternId,
		const std::string& firstColliderId, const std::string& secondColliderId)
	{
		using namespace Client;
		RequireEditorRoundtrip(workbench);
		const auto beforeContact = workbench.Get_Composition();
		const auto originalPattern = EditorPattern(workbench, patternId);
		std::string status, worldId, firstTarget, secondTarget;
		RequireEditorStep(workbench.Create_World("Native contact cards", "world.object.instance.kouku.card",
			worldId, status), status, "create a saved card definition for contact targets");
		RequireEditorStep(workbench.Append_WorldBox(patternId, worldId, 0u, 1000u, firstTarget, status),
			status, "place the first contact card");
		RequireEditorStep(workbench.Append_WorldBox(patternId, worldId, 0u, 1000u, secondTarget, status),
			status, "place the same saved card again with an independent occurrence identity");
		Require(firstTarget != secondTarget, "contact cards reused one WORLD occurrence identity");
		const auto createLogic = [&](KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION values)
		{
			std::string id;
			RequireEditorStep(workbench.Create_Logic(values.strDisplayName, values.strLogicType, id, status),
				status, "create contact authoring Logic");
			RequireEditorStep(workbench.Set_LogicDefinitionValues(id, values, status), status,
				"apply typed contact authoring values");
			return id;
		};
		KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION signal;
		signal.strDisplayName = "Native external completion";
		signal.strLogicType = "DURATION"; signal.strJudgementKind = "EXTERNAL_SIGNAL";
		signal.bEndsPatternOnSuccess = true;
		const auto signalId = createLogic(signal);
		std::string signalWindowId;
		RequireEditorStep(workbench.Append_LogicBox(patternId, signalId, 0u, 1000u, signalWindowId, status),
			status, "place an externally completed Logic window");
		KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION contact;
		contact.strDisplayName = "Native shared card contact"; contact.strLogicType = "TRIGGER";
		contact.strTriggerKind = "OBJECT_CONTACT";
		contact.TargetWorldOccurrenceIds = { firstTarget, secondTarget };
		contact.fTargetRadiusM = 1.25;
		contact.strContactGroupId = "native.contact.cards"; contact.iContactPriority = 10u;
		const auto contactId = createLogic(contact);
		const auto findCollider = [&](const std::string& id)
		{
			const auto& rows = EditorPattern(workbench, patternId).PresentationOccurrences;
			const auto found = std::find_if(rows.begin(), rows.end(), [&](const auto& row) { return row.strOccurrenceId == id; });
			Require(found != rows.end(), "contact Collider was lost");
			return *found;
		};
		const auto findWindow = [&](const std::string& id)
		{
			const auto& rows = EditorPattern(workbench, patternId).LogicOccurrences;
			const auto found = std::find_if(rows.begin(), rows.end(), [&](const auto& row) { return row.strOccurrenceId == id; });
			Require(found != rows.end(), "contact Logic window was lost");
			return *found;
		};
		RequireEditorStep(workbench.Connect_ColliderLogic(patternId, findCollider(firstColliderId), contactId, status),
			status, "connect the first Collider to OBJECT_CONTACT");
		const auto contactWindowId = findCollider(firstColliderId).strLogicOccurrenceId;
		const auto windowOrdinal = EditorPattern(workbench, patternId).iNextLogicOccurrenceOrdinal;
		auto secondCollider = findCollider(secondColliderId);
		secondCollider.iStartMs = 100u; secondCollider.iDurationMs = 400u;
		secondCollider.strLogicOccurrenceId = contactWindowId; // Explicit Shared selection owns this clock.
		RequireEditorStep(workbench.Connect_ColliderLogic(patternId, secondCollider, contactId, status),
			status, "reuse the contact window for a second Collider");
		Require(findCollider(secondColliderId).strLogicOccurrenceId == contactWindowId &&
			findCollider(secondColliderId).iStartMs == 0u && findCollider(secondColliderId).iDurationMs == 1000u &&
			EditorPattern(workbench, patternId).iNextLogicOccurrenceOrdinal == windowOrdinal,
			"shared contact connection duplicated its window or overwrote the shared clock");
		KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION motions;
		motions.strDisplayName = "Native flip contacted card"; motions.strLogicType = "RESULT";
		motions.strOutcomeKind = "PLAY_CONTACT_WORLD_OBJECT_MOTION";
		motions.ContactMotions = { { firstTarget, "world.object.instance.kouku.card_flip" },
			{ secondTarget, "world.object.instance.kouku.card_flip" } };
		const auto motionsId = createLogic(motions);
		KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION complete;
		complete.strDisplayName = "Native complete on first card"; complete.strLogicType = "RESULT";
		complete.strOutcomeKind = "COMPLETE_LOGIC_WINDOW";
		complete.strTargetLogicOccurrenceId = signalWindowId;
		complete.strContactTargetWorldOccurrenceId = firstTarget;
		const auto completeId = createLogic(complete);
		RequireEditorStep(workbench.Set_LogicBoxOutcomes(patternId, contactWindowId,
			KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS, { motionsId, completeId }, status), status,
			"wire target-specific Motion and external completion Results");
		auto firstCollider = findCollider(firstColliderId);
		firstCollider.iStartMs = 100u; firstCollider.iDurationMs = 800u;
		RequireEditorStep(workbench.Set_PresentationBox(patternId, firstCollider, status), status,
			"retime contact through a linked Collider");
		Require(findWindow(contactWindowId).iStartMs == 100u && findWindow(contactWindowId).iDurationMs == 800u &&
			findCollider(secondColliderId).iStartMs == 100u && findCollider(secondColliderId).iDurationMs == 800u,
			"Collider timing edit did not update the shared window and second Collider");
		RequireEditorStep(workbench.Set_LogicBoxWindow(patternId, contactWindowId, 200u, 600u, status), status,
			"retime both Colliders through the contact window");
		for (const auto& id : { firstColliderId, secondColliderId })
			Require(findCollider(id).iStartMs == 200u && findCollider(id).iDurationMs == 600u &&
				findCollider(id).strLogicOccurrenceId == contactWindowId,
				"shared window edit lost a Collider link or left stale timing");
		Require(findWindow(contactWindowId).OnSuccessLogicIds == std::vector<std::string>{ motionsId, completeId } &&
			findWindow(signalWindowId).iStartMs == 0u && findWindow(signalWindowId).iDurationMs == 1000u,
			"contact retiming rewired its Results or changed the externally completed window");
		auto boneCollider = findCollider(firstColliderId);
		boneCollider.strAnchorKind = "BOSS";
		boneCollider.strWorldId.clear();
		boneCollider.strBoneTarget = "WEAPON";
		boneCollider.strBone = "b_rpct_03";
		boneCollider.bFollowBoss = true;
		RequireEditorStep(workbench.Set_PresentationBox(patternId, boneCollider, status), status,
			"anchor a contact Collider to an explicit weapon Bone");
		RequireEditorRoundtrip(workbench);
		Require(findCollider(firstColliderId) == boneCollider,
			"Save/Reload lost the weapon Bone or modified its linked Collider window");
		const auto requireRejectedEdit = [&](const auto& edit, const char* const message)
		{
			const auto saved = workbench.Get_Composition();
			const auto generation = workbench.Get_DraftGeneration();
			const auto bytes = ReadText(sourcePath);
			Require(!edit() && !status.empty() && workbench.Get_Composition() == saved &&
				!workbench.Is_Dirty() && workbench.Get_DraftGeneration() == generation && ReadText(sourcePath) == bytes,
				message);
		};
		for (const auto invalidKind : { "UNKNOWN", "WORLD", "EMPTY_BONE", "NOT_FOLLOWING" })
		{
			auto invalidBone = boneCollider;
			if (std::string(invalidKind) == "UNKNOWN") invalidBone.strBoneTarget = "UNKNOWN";
			else if (std::string(invalidKind) == "WORLD") invalidBone.strAnchorKind = "WORLD";
			else if (std::string(invalidKind) == "EMPTY_BONE") invalidBone.strBone.clear();
			else invalidBone.bFollowBoss = false;
			requireRejectedEdit([&]() { return workbench.Set_PresentationBox(patternId, invalidBone, status); },
				"invalid weapon Bone anchor changed the saved Collider or shared Logic");
		}
		auto invalidContact = contact;
		invalidContact.TargetWorldOccurrenceIds = { firstTarget, firstTarget };
		requireRejectedEdit([&]() { return workbench.Set_LogicDefinitionValues(contactId, invalidContact, status); },
			"duplicate contact target partially replaced the saved definition");
		invalidContact.TargetWorldOccurrenceIds = { firstTarget, patternId + ".world.999999" };
		requireRejectedEdit([&]() { return workbench.Set_LogicDefinitionValues(contactId, invalidContact, status); },
			"missing contact target changed the existing card mapping");
		auto invalidMotions = motions;
		invalidMotions.ContactMotions.pop_back();
		requireRejectedEdit([&]() { return workbench.Set_LogicDefinitionValues(motionsId, invalidMotions, status); },
			"incomplete per-target Motion map was accepted by a shared contact window");
		invalidMotions.ContactMotions = { motions.ContactMotions[0], motions.ContactMotions[0] };
		requireRejectedEdit([&]() { return workbench.Set_LogicDefinitionValues(motionsId, invalidMotions, status); },
			"duplicate Motion target overwrote the saved contact map");
		auto invalidComplete = complete;
		invalidComplete.strTargetLogicOccurrenceId = contactWindowId;
		requireRejectedEdit([&]() { return workbench.Set_LogicDefinitionValues(completeId, invalidComplete, status); },
			"external completion was allowed to target its contact Trigger window");
		invalidComplete = complete;
		invalidComplete.strContactTargetWorldOccurrenceId = patternId + ".world.999999";
		requireRejectedEdit([&]() { return workbench.Set_LogicDefinitionValues(completeId, invalidComplete, status); },
			"external completion accepted an unrelated contact-target filter");
		for (const auto& previous : originalPattern.LogicOccurrences)
			Require(findWindow(previous.strOccurrenceId) == previous,
				"contact authoring changed an existing ENTER_AREA or Duration window");
		for (const auto& previous : originalPattern.PresentationOccurrences)
			if (previous.strOccurrenceId != firstColliderId && previous.strOccurrenceId != secondColliderId)
				Require(findCollider(previous.strOccurrenceId) == previous,
					"contact authoring changed an existing damage Collider or presentation");
		for (const auto& previous : beforeContact.Logics)
		{
			const auto& logics = workbench.Get_Composition().Logics;
			const auto found = std::find_if(logics.begin(), logics.end(), [&](const auto& row) { return row.strLogicId == previous.strLogicId; });
			Require(found != logics.end() && *found == previous,
				"contact authoring rewrote a shared ENTER_AREA damage Result or other Logic");
		}
	}

	void VerifyKoukuPresentationDocument(const std::filesystem::path& sourcePath)
	{
		using namespace Client;
		KOUKU_SAYDON_COMPOSITION_DOCUMENT source;
		std::string status;
		RequireEditorStep(CKoukuSaydonCompositionDocument::Parse_Text(ReadText(sourcePath), source, status),
			status, "parse presentation source");
		auto pattern = std::find_if(source.Patterns.begin(), source.Patterns.end(), [](const auto& row) {
			return row.strLoadError.empty() && !row.Stages.empty() && row.Stages.front().iDurationMs >= 1000u; });
		Require(pattern != source.Patterns.end() && !source.Worlds.empty(), "presentation test needs an authored Pattern and World");
		const auto patternId = pattern->strPatternId;
		pattern->strAuthoringStatus = "DRAFT";
		pattern->bResetBossToSpawn = true;
		pattern->PresentationOccurrences.clear();
		source.Worlds.front().strAnchorKind = "BOSS_SPAWN";
		source.Worlds.front().AnchorPosition = { -0.319, 1.9, 737.531 };
		source.Worlds.front().PositionOffset = { 0.0, 0.58, 0.0 };
		KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION area;
		area.strLogicId = "kakulsaydon.g1.logic." + std::to_string(source.iNextLogicOrdinal++);
		area.strDisplayName = "Native Area Overlap";
		area.strLogicType = "DURATION";
		area.strJudgementKind = "AREA_OVERLAP";
		area.strInsideOutcome = "FAIL";
		source.Logics.push_back(area);
		KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE window;
		window.strOccurrenceId = patternId + ".logic." + std::to_string(pattern->iNextLogicOccurrenceOrdinal++);
		window.strLogicId = area.strLogicId;
		window.iDurationMs = 1000u;
		pattern->LogicOccurrences.push_back(window);
		for (std::size_t i = 0u; i < 4u; ++i)
		{
			KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE resource;
			resource.strResourceId = "kakulsaydon.g1.presentation." + std::to_string(source.iNextPresentationResourceOrdinal++);
			resource.strDisplayName = "Native presentation " + std::to_string(i);
			resource.eKind = static_cast<KOUKU_SAYDON_PRESENTATION_KIND>(i);
			resource.strAssetId = i == 0u ? "boss.kouku.disarm" : i == 1u ? "Sound/native/test.wav" : i == 2u ? "native.camera.shot" : "";
			if (i == 3u)
			{
				resource.strColliderKind = "ROULETTE_CARD_REGION";
				resource.strShape = "SECTOR";
				resource.fHalfAngleDegrees = 22.5;
			}
			source.PresentationResources.push_back(resource);
			KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE box;
			box.strOccurrenceId = patternId + ".presentation." + std::to_string(pattern->iNextPresentationOccurrenceOrdinal++);
			box.strResourceId = resource.strResourceId;
			box.iDurationMs = 1000u;
			box.PositionOffset = { 1.0, 0.5, -2.0 };
			box.RotationDegrees = { 10.0, 22.5, 30.0 };
			box.Scale = { 1.5, 2.0, 1.5 };
			box.iFadeInMs = 100u;
			box.iFadeOutMs = 200u;
			box.fDissolveStart = 0.25;
			box.fDissolveEnd = 0.75;
			if (i == 3u)
			{
				box.strRegionId = "native.roulette.spade.black";
				box.strCardSymbol = "SPADE";
				box.strCardColor = "BLACK";
				box.strAnchorKind = "WORLD";
				box.strWorldId = source.Worlds.front().strWorldId;
				box.strLogicOccurrenceId = window.strOccurrenceId;
			}
			pattern->PresentationOccurrences.push_back(box);
		}
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE circle;
		circle.strResourceId = "kakulsaydon.g1.presentation." + std::to_string(source.iNextPresentationResourceOrdinal++);
		circle.strDisplayName = "Native Circle"; circle.eKind = KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER;
		circle.strShape = "CIRCLE"; circle.fRadiusM = 4.25;
		source.PresentationResources.push_back(circle);
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE circleBox;
		circleBox.strOccurrenceId = patternId + ".presentation." + std::to_string(pattern->iNextPresentationOccurrenceOrdinal++);
		circleBox.strResourceId = circle.strResourceId; circleBox.iDurationMs = 1000u;
		circleBox.bDebugRender = false;
		pattern->PresentationOccurrences.push_back(circleBox);
		auto secondCircle = circleBox;
		secondCircle.strOccurrenceId = patternId + ".presentation." + std::to_string(pattern->iNextPresentationOccurrenceOrdinal++);
		pattern->PresentationOccurrences.push_back(secondCircle);
		auto firstContactCircle = circleBox;
		firstContactCircle.strOccurrenceId = patternId + ".presentation." + std::to_string(pattern->iNextPresentationOccurrenceOrdinal++);
		pattern->PresentationOccurrences.push_back(firstContactCircle);
		auto secondContactCircle = circleBox;
		secondContactCircle.strOccurrenceId = patternId + ".presentation." + std::to_string(pattern->iNextPresentationOccurrenceOrdinal++);
		pattern->PresentationOccurrences.push_back(secondContactCircle);
		KOUKU_SAYDON_COMPOSITION_DOCUMENT parsed;
		RequireEditorStep(CKoukuSaydonCompositionDocument::Parse_Text(CKoukuSaydonCompositionDocument::Serialize(source), parsed, status),
			status, "roundtrip presentation resources and windows");
		Require(parsed == source, "presentation roundtrip changed transforms, fades, card/World/Logic identities or spawn reset");
		auto damaged = source;
		auto& damagedPattern = *std::find_if(damaged.Patterns.begin(), damaged.Patterns.end(), [&](const auto& row) { return row.strPatternId == patternId; });
		damagedPattern.PresentationOccurrences.front().iFadeInMs = 900u;
		RequireEditorStep(CKoukuSaydonCompositionDocument::Parse_Text(CKoukuSaydonCompositionDocument::Serialize(damaged), parsed, status),
			status, "isolate malformed presentation window");
		const auto isolated = std::find_if(parsed.Patterns.begin(), parsed.Patterns.end(), [&](const auto& row) { return row.strPatternId == patternId; });
		Require(isolated != parsed.Patterns.end() && !isolated->strLoadError.empty() && !isolated->strPreservedJson.empty(),
			"invalid fade sum did not preserve the broken Pattern JSON");
		damaged = source;
		damaged.PresentationResources.back().strAssetId = "Sound/../outside.wav";
		const auto lastGood = parsed;
		Require(!CKoukuSaydonCompositionDocument::Parse_Text(CKoukuSaydonCompositionDocument::Serialize(damaged), parsed, status) && parsed == lastGood,
			"invalid presentation resource partially replaced the last-good document");
		Require(Kouku_IsOutcomeSlotAllowed("ROULETTE_CARD_MATCH", KOUKU_SAYDON_OUTCOME_SLOT::TIMEOUT) &&
			Kouku_IsOutcomeSlotAllowed("AREA_OVERLAP", KOUKU_SAYDON_OUTCOME_SLOT::FAIL) &&
			!Kouku_IsOutcomeSlotAllowed("ENTER_AREA", KOUKU_SAYDON_OUTCOME_SLOT::FAIL),
			"roulette/area result slots differ from their Server judgement contracts");
		const auto originalBytes = ReadText(sourcePath);
		Require(WriteText(sourcePath, CKoukuSaydonCompositionDocument::Serialize(source)), "could not stage Circle editor scratch source");
		CKoukuSaydonActionWorkbench workbench;
		RequireEditorStep(workbench.Reload(status), status, "reload Circle Trigger fixture");
		RequireEditorStep(workbench.Set_ColliderTriggerDamage(patternId, circleBox, 15u, status), status, "create Trigger damage atomically");
		const auto findBox = [&](const std::string& id) {
			const auto& document = workbench.Get_Composition();
			const auto owner = std::find_if(document.Patterns.begin(), document.Patterns.end(), [&](const auto& row) { return row.strPatternId == patternId; });
			const auto box = std::find_if(owner->PresentationOccurrences.begin(), owner->PresentationOccurrences.end(), [&](const auto& row) { return row.strOccurrenceId == id; });
			return *box;
		};
		const auto findWindow = [&](const std::string& id) {
			const auto& document = workbench.Get_Composition();
			const auto owner = std::find_if(document.Patterns.begin(), document.Patterns.end(), [&](const auto& row) { return row.strPatternId == patternId; });
			const auto window = std::find_if(owner->LogicOccurrences.begin(), owner->LogicOccurrences.end(), [&](const auto& row) { return row.strOccurrenceId == id; });
			Require(window != owner->LogicOccurrences.end(), "Collider Trigger has no real Logic window");
			return *window;
		};
		const auto first = findBox(circleBox.strOccurrenceId);
		const auto firstWindow = findWindow(first.strLogicOccurrenceId);
		const auto& damageLogics = workbench.Get_Composition().Logics;
		const auto damageTrigger = std::find_if(damageLogics.begin(), damageLogics.end(),
			[&](const auto& logic) { return logic.strLogicId == firstWindow.strLogicId; });
		Require(damageTrigger != damageLogics.end() && damageTrigger->fBossChargeDistanceM == 0.0,
			"automatic Collider damage inherited another Pattern's boss charge");
		Require(!first.bDebugRender && firstWindow.OnSuccessLogicIds.size() == 1u, "Circle debug flag or Trigger damage Result was lost");
		const auto firstDamage = firstWindow.OnSuccessLogicIds.front();
		const auto ordinal = workbench.Get_Composition().iNextLogicOrdinal;
		RequireEditorStep(workbench.Set_ColliderTriggerDamage(patternId, first, 15u, status), status, "reuse existing Trigger damage");
		Require(workbench.Get_Composition().iNextLogicOrdinal == ordinal && findBox(first.strOccurrenceId).strLogicOccurrenceId == first.strLogicOccurrenceId,
			"repeated Trigger Apply created duplicate Logic or Result");
		RequireEditorStep(workbench.Set_ColliderTriggerDamage(patternId, secondCircle, 15u, status), status, "reuse damage definition with independent Trigger window");
		const auto second = findBox(secondCircle.strOccurrenceId);
		Require(second.strLogicOccurrenceId != first.strLogicOccurrenceId && findWindow(second.strLogicOccurrenceId).OnSuccessLogicIds.front() == firstDamage,
			"independent Colliders unexpectedly shared a window or failed to reuse their Result");
		RequireEditorStep(workbench.Set_ColliderTriggerDamage(patternId, first, 20u, status), status, "tune one Trigger damage without mutating shared Result");
		Require(findWindow(second.strLogicOccurrenceId).OnSuccessLogicIds.front() == firstDamage &&
			findWindow(first.strLogicOccurrenceId).OnSuccessLogicIds.front() != firstDamage,
			"damage tuning rewrote another Collider's Result");
		KOUKU_COLLIDER_DAMAGE_SETTINGS pushDamage;
		pushDamage.iPercent = 20; pushDamage.bRepeatAfterKnockback = true;
		pushDamage.fPushRangeM = 2.0; pushDamage.iPushMs = 242u;
		pushDamage.strPushDirection = "BOSS_FORWARD";
		const auto secondWindowBeforePush = findWindow(second.strLogicOccurrenceId);
		RequireEditorStep(workbench.Set_ColliderTriggerDamage(patternId, first, pushDamage, status), status,
			"edit damage, repeat and boss-forward push directly on Collider");
		const auto pushedWindow = findWindow(first.strLogicOccurrenceId);
		const auto& pushedLogics = workbench.Get_Composition().Logics;
		const auto pushedResult = std::find_if(pushedLogics.begin(), pushedLogics.end(),
			[&](const auto& row) { return row.strLogicId == pushedWindow.OnSuccessLogicIds.front(); });
		const auto pushedTrigger = std::find_if(pushedLogics.begin(), pushedLogics.end(),
			[&](const auto& row) { return row.strLogicId == pushedWindow.strLogicId; });
		Require(pushedResult != pushedLogics.end() && pushedResult->iPercent == 20u &&
			pushedResult->fPushRangeM == 2.0 && pushedResult->iPushMs == 242u && pushedResult->strPushDirection == "BOSS_FORWARD" &&
			pushedTrigger != pushedLogics.end() && pushedTrigger->bRepeatAfterKnockback && !pushedTrigger->bRearmOnExit &&
			findWindow(second.strLogicOccurrenceId) == secondWindowBeforePush,
			"direct Collider damage lost push settings or changed another Collider's shared Trigger/Result");
		const auto pushOrdinal = workbench.Get_Composition().iNextLogicOrdinal;
		RequireEditorStep(workbench.Set_ColliderTriggerDamage(patternId, first, pushDamage, status), status,
			"reapply identical Collider damage settings");
		Require(workbench.Get_Composition().iNextLogicOrdinal == pushOrdinal,
			"identical direct damage settings created duplicate definitions");
		RequireEditorRoundtrip(workbench);
		const auto lastPushGood = workbench.Get_Composition();
		auto invalidPush = pushDamage; invalidPush.iPushMs = 0u;
		Require(!workbench.Set_ColliderTriggerDamage(patternId, first, invalidPush, status) && workbench.Get_Composition() == lastPushGood,
			"unpaired push values changed the saved Collider draft");
		invalidPush = pushDamage; invalidPush.bRearmOnExit = true;
		Require(!workbench.Set_ColliderTriggerDamage(patternId, first, invalidPush, status) && workbench.Get_Composition() == lastPushGood,
			"incompatible contact repeat modes changed the saved Collider draft");
		const auto beforeDebug = workbench.Get_Composition();
		const auto beforeDebugGeneration = workbench.Get_DraftGeneration();
		auto expectedDebug = beforeDebug;
		for (auto& owner : expectedDebug.Patterns)
			if (owner.strPatternId == patternId)
				for (auto& box : owner.PresentationOccurrences)
					if (box.strOccurrenceId == first.strOccurrenceId) box.bDebugRender = true;
		RequireEditorStep(workbench.Set_PresentationBoxDebugRender(patternId, first.strOccurrenceId, true, status), status, "apply only Collider debug flag");
		Require(workbench.Get_Composition() == expectedDebug && workbench.Get_DraftGeneration() == beforeDebugGeneration + 1u,
			"Debug Render changed geometry, timing, outcomes or failed to advance draft generation");
		Require(!workbench.Set_PresentationBoxDebugRender(patternId, "absent.collider", false, status) &&
			workbench.Get_Composition() == expectedDebug && workbench.Get_DraftGeneration() == beforeDebugGeneration + 1u,
			"missing Collider debug target mutated the draft or generation");
		const auto lastTriggerGood = workbench.Get_Composition();
		Require(!workbench.Set_ColliderTriggerDamage(patternId, first, 0u, status) && workbench.Get_Composition() == lastTriggerGood,
			"invalid Trigger damage partially committed Logic or geometry");
		RequireEditorStep(CKoukuSaydonCompositionDocument::Parse_Text(CKoukuSaydonCompositionDocument::Serialize(lastTriggerGood), parsed, status),
			status, "roundtrip Circle/debugRender/insideOutcome and atomic Trigger wiring");
		Require(parsed == lastTriggerGood, "Trigger damage save/reload changed its explicit contracts");
		VerifyKoukuObjectContactAuthoring(workbench, sourcePath, patternId,
			firstContactCircle.strOccurrenceId, secondContactCircle.strOccurrenceId);
		std::string companionPatternId, companionWorldId, companionWorldBoxId;
		RequireEditorStep(workbench.Create_Pattern("Native World companion", "NORMAL", companionPatternId, status), status, "create companion Pattern");
		RequireEditorStep(workbench.Set_PatternDuration(companionPatternId, 2000u, status), status, "set companion Pattern lifetime");
		RequireEditorStep(workbench.Create_World("Native companion World", source.Worlds.front().strSequenceInstanceId, companionWorldId, status), status, "create companion World");
		const auto effectResource = std::find_if(source.PresentationResources.begin(), source.PresentationResources.end(),
			[](const auto& row) { return row.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT; });
		Require(effectResource != source.PresentationResources.end(), "companion test has no Effect resource");
		RequireEditorStep(workbench.Set_WorldCompanionEffect(companionWorldId, effectResource->strResourceId, status), status, "select World companion Effect");
		RequireEditorStep(workbench.Append_WorldBox(companionPatternId, companionWorldId, 100u, 900u, companionWorldBoxId, status), status, "append World with one explicit companion Effect");
		const auto companionPattern = [&]() {
			const auto& document = workbench.Get_Composition();
			return *std::find_if(document.Patterns.begin(), document.Patterns.end(), [&](const auto& row) { return row.strPatternId == companionPatternId; });
		};
		auto paired = companionPattern();
		Require(paired.WorldOccurrences.size() == 1u && paired.PresentationOccurrences.size() == 1u &&
			paired.PresentationOccurrences.front().strWorldOccurrenceId == companionWorldBoxId &&
			paired.PresentationOccurrences.front().strResourceId == effectResource->strResourceId &&
			paired.PresentationOccurrences.front().iStartMs == 100u && paired.PresentationOccurrences.front().iDurationMs == 900u,
			"World Append did not create one matching Effect box");
		RequireEditorStep(workbench.Set_WorldBoxWindow(companionPatternId, companionWorldBoxId, 300u, 400u, 1.f, status), status, "move and trim World companion together");
		paired = companionPattern();
		Require(paired.PresentationOccurrences.front().iStartMs == 300u && paired.PresentationOccurrences.front().iDurationMs == 400u,
			"World timing edit left its companion at the old window");
		auto independentTiming = workbench.Get_Composition();
		auto timingPattern = std::find_if(independentTiming.Patterns.begin(), independentTiming.Patterns.end(), [&](const auto& row) { return row.strPatternId == companionPatternId; });
		timingPattern->PresentationOccurrences.front().iStartMs = 320u;
		timingPattern->PresentationOccurrences.front().iDurationMs = 100u;
		RequireEditorStep(CKoukuSaydonCompositionDocument::Parse_Text(CKoukuSaydonCompositionDocument::Serialize(independentTiming), parsed, status), status, "allow independent companion Effect timing");
		Require(parsed == independentTiming, "World link forced Effect timing equality");
		auto duplicateEffect = timingPattern->PresentationOccurrences.front();
		duplicateEffect.strOccurrenceId = companionPatternId + ".presentation." + std::to_string(timingPattern->iNextPresentationOccurrenceOrdinal++);
		timingPattern->PresentationOccurrences.push_back(duplicateEffect);
		RequireEditorStep(CKoukuSaydonCompositionDocument::Parse_Text(CKoukuSaydonCompositionDocument::Serialize(independentTiming), parsed, status), status, "isolate duplicate World companion");
		const auto duplicateOwner = std::find_if(parsed.Patterns.begin(), parsed.Patterns.end(), [&](const auto& row) { return row.strPatternId == companionPatternId; });
		Require(duplicateOwner != parsed.Patterns.end() && !duplicateOwner->strLoadError.empty(), "duplicate World companion was admitted");
		RequireEditorStep(workbench.Set_WorldCompanionEffect(companionWorldId, "", status), status, "unlink World while retaining explicit Effect");
		Require(companionPattern().PresentationOccurrences.size() == 1u && companionPattern().PresentationOccurrences.front().strWorldOccurrenceId.empty(),
			"unlinking World destroyed its independently editable Effect");
		RequireEditorStep(workbench.Set_WorldCompanionEffect(companionWorldId, effectResource->strResourceId, status), status, "relink World with a new owned Effect");
		RequireEditorStep(workbench.Delete_WorldBox(companionPatternId, companionWorldBoxId, status), status, "delete World and only its linked Effect");
		paired = companionPattern();
		Require(paired.WorldOccurrences.empty() && paired.PresentationOccurrences.size() == 1u && paired.PresentationOccurrences.front().strWorldOccurrenceId.empty(),
			"World delete removed an independent Effect or left a dangling companion");
		RequireEditorStep(CKoukuSaydonCompositionDocument::Parse_Text(CKoukuSaydonCompositionDocument::Serialize(workbench.Get_Composition()), parsed, status), status, "roundtrip World companion edit transactions");
		Require(parsed == workbench.Get_Composition(), "World companion roundtrip changed definitions or occurrences");
		Require(WriteText(sourcePath, originalBytes), "could not restore Circle editor scratch source");
	}

	void VerifyKoukuRootVerticalScale(const std::filesystem::path& sourcePath)
	{
		using namespace Client;
		CKoukuSaydonCompositionDocument document(sourcePath);
		std::string status;
		RequireEditorStep(document.Reload(status), status, "load root height source");
		auto candidate = document.Get_LastGood();
		const auto found = std::find_if(candidate.Patterns.begin(), candidate.Patterns.end(), [](const auto& pattern) {
			return pattern.strLoadError.empty() && !pattern.Stages.empty(); });
		Require(found != candidate.Patterns.end(), "root height test needs an authored Pattern");
		const auto index = static_cast<std::size_t>(found - candidate.Patterns.begin());
		for (const double scale : {0.0, 0.8, 1.0})
		{
			candidate.Patterns[index].fAnimationRootVerticalScale = scale;
			RequireEditorStep(document.Save_Atomic(candidate, status), status, "save independent root height");
			RequireEditorStep(document.Reload(status), status, "reload independent root height");
			Require(document.Get_LastGood().Patterns[index].fAnimationRootVerticalScale == scale,
				"root height lost its authored scalar through Save/Reload");
			candidate = document.Get_LastGood();
		}
		const auto good = document.Get_LastGood();
		const auto bytes = ReadText(sourcePath);
		for (const double scale : {-0.1, 1.1, (std::numeric_limits<double>::infinity)(), (std::numeric_limits<double>::quiet_NaN)()})
		{
			candidate = good; candidate.Patterns[index].fAnimationRootVerticalScale = scale;
			Require(!document.Save_Atomic(candidate, status) && document.Get_LastGood() == good && ReadText(sourcePath) == bytes,
				"invalid root height replaced the prior saved document");
		}
	}

	void VerifyKoukuBoneColliderPreview(const std::filesystem::path& sourcePath)
	{
		using namespace Client;
		const auto originalBytes = ReadText(sourcePath);
		KOUKU_SAYDON_COMPOSITION_DOCUMENT source;
		std::string status;
		RequireEditorStep(CKoukuSaydonCompositionDocument::Parse_Text(originalBytes, source, status),
			status, "parse Bone preview fixture source");
		auto owner = std::find_if(source.Patterns.begin(), source.Patterns.end(), [](const auto& pattern) {
			return pattern.strLoadError.empty() && pattern.strActorProfileId == "MN_RPCT_06" &&
				!pattern.Stages.empty() && pattern.Stages.front().iDurationMs >= 1000u &&
				!pattern.Stages.front().AnimationOccurrences.empty(); });
		const auto resource = std::find_if(source.PresentationResources.begin(), source.PresentationResources.end(),
			[](const auto& row) { return row.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER && row.strColliderKind == "GEOMETRY"; });
		Require(owner != source.Patterns.end() && resource != source.PresentationResources.end(),
			"Bone preview fixture needs the real hammer actor animation and a Collider resource");
		const auto patternId = owner->strPatternId;
		owner->strAuthoringStatus = "DRAFT";
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE collider;
		collider.strOccurrenceId = patternId + ".presentation." + std::to_string(owner->iNextPresentationOccurrenceOrdinal++);
		collider.strResourceId = resource->strResourceId;
		collider.iStartMs = 100u; collider.iDurationMs = 600u;
		collider.strBoneTarget = "WEAPON"; collider.strBone = "b_rpct_01";
		collider.bFollowBoss = true;
		owner->PresentationOccurrences.push_back(collider);
		auto edge = collider;
		edge.strOccurrenceId = patternId + ".presentation." + std::to_string(owner->iNextPresentationOccurrenceOrdinal++);
		edge.Scale = { 2.0, 1.0, 2.0 };
		owner->PresentationOccurrences.push_back(edge);
		Require(WriteText(sourcePath, CKoukuSaydonCompositionDocument::Serialize(source)), "could not write Bone preview fixture");
		CKoukuSaydonActionWorkbench workbench;
		RequireEditorStep(workbench.Reload(status), status, "reload Bone preview fixture");
		const auto saved = workbench.Get_Composition();
		const auto savedBytes = ReadText(sourcePath);
		const auto generation = workbench.Get_DraftGeneration();
		auto edited = collider;
		edited.iStartMs = 250u;
		edited.PositionOffset = { 0.25, -0.5, 1.5 };
		edited.RotationDegrees = { 0.0, 25.0, 0.0 };
		edited.Scale = { 0.5, 0.75, 1.25 };
		auto expected = EditorPattern(workbench, patternId);
		for (auto& row : expected.PresentationOccurrences) if (row.strOccurrenceId == edited.strOccurrenceId) row = edited;
		KOUKU_SAYDON_COMPOSITION_PATTERN preview;
		std::uint32_t clock = 0u; bool_t paused = true; std::string target;
		const auto consumeExpected = [&]() {
			Require(workbench.Consume_PatternPreviewRequest(preview, clock, paused, target) &&
				preview == expected && clock == edited.iStartMs && !paused && target.empty(),
				"Bone preview lost its source actor/clips, edited transform, other Collider or Pattern clock");
			KOUKU_PRESENTATION_PREVIEW_REQUEST standalone;
			Require(!workbench.Consume_PresentationPreviewRequest(standalone), "Bone preview also queued the player-root resource route");
			Require(workbench.Get_Composition() == saved && !workbench.Is_Dirty() &&
				workbench.Get_DraftGeneration() == generation && ReadText(sourcePath) == savedBytes,
				"Bone preview committed un-applied Detail values to the draft or disk");
		};
		RequireEditorStep(workbench.Request_ColliderBoxPreview(patternId, edited, status), status, "preview edited Bone Collider");
		consumeExpected();
		for (const auto invalidKind : { "PATTERN", "BOX", "RESOURCE", "BONE", "BONE_ID", "TARGET", "ANCHOR", "FOLLOW", "TIME", "TRANSFORM" })
		{
			RequireEditorStep(workbench.Request_ColliderBoxPreview(patternId, edited, status), status, "retain valid queued Bone preview");
			auto invalid = edited;
			std::string invalidPattern = patternId;
			const std::string kind = invalidKind;
			if (kind == "PATTERN") invalidPattern = "absent.pattern";
			else if (kind == "BOX") invalid.strOccurrenceId = "absent.box";
			else if (kind == "RESOURCE") invalid.strResourceId = "absent.resource";
			else if (kind == "BONE") invalid.strBone.clear();
			else if (kind == "BONE_ID") invalid.strBone = "../bone";
			else if (kind == "TARGET") invalid.strBoneTarget = "UNKNOWN";
			else if (kind == "ANCHOR") invalid.strAnchorKind = "WORLD";
			else if (kind == "FOLLOW") invalid.bFollowBoss = false;
			else if (kind == "TIME") invalid.iDurationMs = 600000u;
			else invalid.PositionOffset[0] = (std::numeric_limits<double>::quiet_NaN)();
			Require(!workbench.Request_ColliderBoxPreview(invalidPattern, invalid, status) && !status.empty(),
				"invalid Bone preview unexpectedly replaced the pending valid request");
			consumeExpected();
		}
		Require(WriteText(sourcePath, originalBytes), "could not restore source after Bone preview fixture");
	}

	void VerifyKoukuTimelineControls(Client::CKoukuSaydonActionWorkbench& workbench,
		const std::filesystem::path& sourcePath)
	{
		using namespace Client;
		std::string status;
		RequireEditorStep(workbench.Select_ActorProfile("MN_RPCT_05", status), status,
			"select Saydon for timeline controls");
		std::string patternId;
		RequireEditorStep(workbench.Create_Pattern("Timeline control roundtrip", "NORMAL",
			patternId, status), status, "create timeline control Pattern");
		RequireEditorRoundtrip(workbench);
		RequireEditorStep(workbench.Set_PatternDuration(patternId, 10000u, status), status,
			"author a presentation-only Pattern clock");
		const auto clockOnly = EditorPattern(workbench, patternId);
		Require(clockOnly.Stages.size() == 1u && clockOnly.Stages.front().iDurationMs == 10000u &&
			clockOnly.Stages.front().AnimationOccurrences.empty(), "presentation-only lifetime lost the existing Stage clock contract");
		RequireEditorStep(workbench.Request_PatternPreview(patternId, 0u, status), status,
			"request presentation-only Pattern preview");
		KOUKU_SAYDON_COMPOSITION_PATTERN clockPreview;
		std::uint32_t clockStart = 0u;
		bool_t clockPaused = false;
		std::string clockTarget;
		Require(workbench.Consume_PatternPreviewRequest(clockPreview, clockStart, clockPaused, clockTarget) &&
			clockPreview.strPatternId == patternId, "presentation-only preview command was not consumable");
		RequireEditorRoundtrip(workbench);
		RequireEditorStep(workbench.Delete_TimelineSelection(patternId, { clockOnly.Stages.front().strStageId }, {}, status), status,
			"remove the clock-only fixture Stage before animation timing checks");
		RequireEditorRoundtrip(workbench);
		RequireEditorStep(workbench.Append_ActionAsStages(patternId, "MN_RPCT_05",
			4219811u, status), status, "append first timeline Stage");
		RequireEditorStep(workbench.Append_ActionAsStages(patternId, "MN_RPCT_05",
			4219811u, status), status, "append second timeline Stage");
		const auto occupied = EditorPattern(workbench, patternId);
		RequireEditorStep(workbench.Set_PatternDuration(patternId, 10000u, status), status,
			"extend total timeline duration");
		auto expectedDuration = occupied;
		expectedDuration.Stages.back().iDurationMs = 5667u;
		Require(EditorPattern(workbench, patternId) == expectedDuration,
			"total duration retimed earlier Stages, boxes, owners or IDs");
		RequireEditorRoundtrip(workbench);
		const auto durationBaseline = workbench.Get_Composition();
		const auto durationBytes = ReadText(sourcePath);
		for (const auto rejectedDuration : { 0u, 8665u, 600001u })
		{
			Require(!workbench.Set_PatternDuration(patternId, rejectedDuration, status) &&
				!status.empty() && workbench.Get_Composition() == durationBaseline &&
				!workbench.Is_Dirty() && ReadText(sourcePath) == durationBytes,
				"duration below occupied content or outside bounds changed saved state");
		}

		const auto documentBeforeDuplicate = workbench.Get_Composition();
		const auto beforeDuplicate = EditorPattern(workbench, patternId);
		const auto firstStage = beforeDuplicate.Stages[0];
		const auto secondStage = beforeDuplicate.Stages[1];
		const std::string firstBox = firstStage.AnimationOccurrences[0].strOccurrenceId;
		const std::string secondBox = secondStage.AnimationOccurrences[0].strOccurrenceId;
		RequireEditorStep(workbench.Duplicate_TimelineSelection(patternId,
			{ firstStage.strStageId }, { firstBox, secondBox }, status), status,
			"duplicate Stage and standalone box without duplicating its selected child twice");
		const auto duplicated = EditorPattern(workbench, patternId);
		Require(duplicated.strActorProfileId == "MN_RPCT_05" &&
			duplicated.Stages.size() == 4u && duplicated.Stages[0] == firstStage &&
			duplicated.Stages[1] == secondStage &&
			duplicated.iNextStageOrdinal == beforeDuplicate.iNextStageOrdinal + 2u &&
			duplicated.iNextAnimationOrdinal == beforeDuplicate.iNextAnimationOrdinal + 2u,
			"multi-duplicate changed owner, original Stage or consumed wrong ID counts");
		for (const auto& untouched : documentBeforeDuplicate.Patterns)
			if (untouched.strPatternId != patternId)
				Require(EditorPattern(workbench, untouched.strPatternId) == untouched,
					"duplicate changed another actor's Pattern");
		const auto& clonedStage = duplicated.Stages[2];
		auto expectedStage = firstStage;
		expectedStage.strStageId = clonedStage.strStageId;
		expectedStage.strActionId = clonedStage.strActionId;
		expectedStage.AnimationOccurrences[0].strOccurrenceId =
			clonedStage.AnimationOccurrences[0].strOccurrenceId;
		Require(clonedStage == expectedStage &&
			clonedStage.strStageId != firstStage.strStageId &&
			clonedStage.strActionId != firstStage.strActionId &&
			clonedStage.AnimationOccurrences[0].strOccurrenceId != firstBox,
			"Stage clone lost source identity/timing or reused stable IDs");
		auto expectedSecond = secondStage;
		const auto& clonedSecond = duplicated.Stages[3];
		expectedSecond.strStageId = clonedSecond.strStageId;
		expectedSecond.strActionId = clonedSecond.strActionId;
		expectedSecond.iDurationMs = secondStage.AnimationOccurrences[0].iPlayMs;
		expectedSecond.AnimationOccurrences[0].iStartOffsetMs = 0u;
		expectedSecond.AnimationOccurrences[0].strOccurrenceId =
			clonedSecond.AnimationOccurrences[0].strOccurrenceId;
		Require(clonedSecond == expectedSecond && clonedSecond.strStageId != secondStage.strStageId &&
			clonedSecond.strActionId != secondStage.strActionId &&
			clonedSecond.AnimationOccurrences[0].strOccurrenceId != secondBox,
			"standalone box copy was not appended after the selected block in its own trimmed Stage");
		RequireEditorStep(workbench.Validate_Draft(status), status,
			"validate duplicate stable IDs and ownership");
		RequireEditorRoundtrip(workbench);
		const auto requireRejectedMove = [&](const std::string_view target, const int direction,
			const char* const message)
		{
			const auto before = workbench.Get_Composition();
			const auto generation = workbench.Get_DraftGeneration();
			const bool dirty = workbench.Is_Dirty();
			const auto bytes = ReadText(sourcePath);
			Require(!workbench.Move_SelectedStage(target, direction, status) && !status.empty() &&
				workbench.Get_Composition() == before && workbench.Get_DraftGeneration() == generation &&
				workbench.Is_Dirty() == dirty && ReadText(sourcePath) == bytes, message);
		};
		const auto requireMixedOrder = [&](std::vector<KOUKU_SAYDON_COMPOSITION_STAGE> stages,
			const char* const message)
		{
			auto expected = duplicated;
			expected.Stages = std::move(stages);
			Require(EditorPattern(workbench, patternId) == expected, message);
		};
		requireRejectedMove(patternId, 1, "right-edge multi move changed saved state");
		requireRejectedMove(patternId, 0, "zero-direction multi move changed saved state");
		requireRejectedMove("MISSING_PATTERN", -1, "missing Pattern multi move changed saved state");
		RequireEditorStep(workbench.Move_SelectedStage(patternId, -1, status), status,
			"move the selected Stage and animation owner earlier together");
		requireMixedOrder({ firstStage, clonedStage, clonedSecond, secondStage },
			"multi move changed selected order, Stage clocks or stable IDs");
		RequireEditorStep(workbench.Move_SelectedStage(patternId, -1, status), status,
			"repeat Earlier without selecting the copied boxes again");
		requireMixedOrder({ clonedStage, clonedSecond, firstStage, secondStage },
			"repeated Earlier lost the mixed Stage and animation selection");
		requireRejectedMove(patternId, -1, "left-edge multi move partially moved the selection");
		RequireEditorStep(workbench.Move_SelectedStage(patternId, 1, status), status,
			"move the same selected block later after an edge rejection");
		requireMixedOrder({ firstStage, clonedStage, clonedSecond, secondStage },
			"Later lost the selection after an edge rejection");
		RequireEditorStep(workbench.Move_SelectedStage(patternId, 1, status), status,
			"repeat Later without selecting the copied boxes again");
		Require(EditorPattern(workbench, patternId) == duplicated,
			"Earlier/Later roundtrip changed the selected block or unselected Stages");
		RequireEditorRoundtrip(workbench);

		const auto duplicateBaseline = workbench.Get_Composition();
		const auto duplicateBytes = ReadText(sourcePath);
		Require(!workbench.Duplicate_TimelineSelection(patternId,
			{ firstStage.strStageId, "STALE_STAGE" }, { secondBox }, status),
			"stale Stage selection partially duplicated valid entries");
		Require(!workbench.Duplicate_TimelineSelection(patternId,
			{ firstStage.strStageId }, { "STALE_BOX" }, status),
			"stale box selection partially duplicated a valid Stage");
		const auto& foreignPatterns = workbench.Get_Composition().Patterns;
		const auto foreignPattern = std::find_if(foreignPatterns.begin(), foreignPatterns.end(), [&](const auto& candidate) {
			return candidate.strPatternId != patternId && candidate.strLoadError.empty() && !candidate.Stages.empty() &&
				!candidate.Stages.front().AnimationOccurrences.empty(); });
		Require(foreignPattern != foreignPatterns.end(), "foreign Pattern fixture is missing");
		const auto foreignBox = foreignPattern->Stages.front().AnimationOccurrences.front().strOccurrenceId;
		requireRejectedMove(foreignPattern->strPatternId, -1,
			"multi move applied the current selection to another Pattern");
		Require(!workbench.Duplicate_TimelineSelection(patternId, {}, { foreignBox }, status),
			"duplicate imported a box owned by another Pattern");
		Require(workbench.Get_Composition() == duplicateBaseline && !workbench.Is_Dirty() &&
			ReadText(sourcePath) == duplicateBytes,
			"stale selection consumed ordinals, changed ownership or saved source");

		KOUKU_SAYDON_COMPOSITION_PATTERN preview;
		std::uint32_t previewStart = 0u;
		bool_t previewPaused = true;
		std::string previewTarget;
		RequireEditorStep(workbench.Request_PatternPreview(patternId, 1000u, status), status,
			"queue timeline Play from cursor");
		Require(workbench.Consume_PatternPreviewRequest(preview, previewStart,
			previewPaused, previewTarget) && preview == EditorPattern(workbench, patternId) &&
			previewStart == 1000u && !previewPaused && previewTarget.empty(),
			"Play request lost cursor, owner, source boxes or running state");
		Require(!workbench.Consume_PatternPreviewRequest(preview, previewStart,
			previewPaused, previewTarget) && workbench.Get_Composition() == duplicateBaseline &&
			!workbench.Is_Dirty(), "Play request was not one-shot or edited source state");

		RequireEditorStep(workbench.Set_PatternDuration(patternId, 600000u, status), status,
			"extend timeline to authoring duration bound");
		RequireEditorRoundtrip(workbench);
		const auto bounded = workbench.Get_Composition();
		const auto boundedBytes = ReadText(sourcePath);
		Require(!workbench.Duplicate_TimelineSelection(patternId,
			{ firstStage.strStageId }, { secondBox }, status) &&
			workbench.Get_Composition() == bounded && !workbench.Is_Dirty() &&
			ReadText(sourcePath) == boundedBytes,
			"overlong multi-duplicate partially committed or consumed ID ordinals");
		RequireEditorStep(workbench.Request_PatternPreview(patternId, 600000u, status), status,
			"restart timeline Play from its end");
		Require(workbench.Consume_PatternPreviewRequest(preview, previewStart,
			previewPaused, previewTarget) && previewStart == 0u && !previewPaused,
			"Play at total duration did not restart from zero");

		std::string blockPatternId;
		RequireEditorStep(workbench.Create_Pattern("Selected block ordering", "NORMAL",
			blockPatternId, status), status, "create noncontiguous selection fixture");
		auto shortClip = firstStage.AnimationOccurrences[0];
		shortClip.iPlayMs = 1000u;
		std::vector<std::string> blockStageIds, blockBoxIds;
		for (int index = 0; index < 4; ++index)
		{
			std::string stageId, boxId;
			RequireEditorStep(workbench.Append_AnimationAsStage(blockPatternId, shortClip,
				stageId, boxId, status), status, "append a selected block fixture Stage");
			blockStageIds.push_back(stageId);
			blockBoxIds.push_back(boxId);
		}
		RequireEditorStep(workbench.Set_StageDuration(blockPatternId, blockStageIds[0], 2000u, status),
			status, "retain the first source Stage tail");
		RequireEditorStep(workbench.Move_Animation(blockPatternId, blockBoxIds[0], 250u, status),
			status, "offset the first source animation");
		RequireEditorStep(workbench.Set_StageDuration(blockPatternId, blockStageIds[1], 8000u, status),
			status, "retain the multi-row source Stage tail");
		RequireEditorStep(workbench.Move_Animation(blockPatternId, blockBoxIds[1], 100u, status),
			status, "offset the second source animation");
		std::string unselectedBoxId, laterBoxId;
		RequireEditorStep(workbench.Bind_Animation(blockPatternId, blockStageIds[1], shortClip,
			3000u, unselectedBoxId, status), status, "append an unselected middle animation");
		RequireEditorStep(workbench.Bind_Animation(blockPatternId, blockStageIds[1], shortClip,
			6000u, laterBoxId, status), status, "append a second selected animation in the same Stage");
		RequireEditorStep(workbench.Set_StageDuration(blockPatternId, blockStageIds[2], 3000u, status),
			status, "give the third source Stage a distinct clock");
		RequireEditorRoundtrip(workbench);
		const auto blockSource = EditorPattern(workbench, blockPatternId);
		RequireEditorStep(workbench.Duplicate_TimelineSelection(blockPatternId,
			{ blockStageIds[2], blockStageIds[0], blockStageIds[2] }, {}, status), status,
			"duplicate noncontiguous Stages selected in reverse order with a repeated ID");
		const auto stageBlock = EditorPattern(workbench, blockPatternId);
		Require(stageBlock.Stages.size() == 6u &&
			stageBlock.iNextStageOrdinal == blockSource.iNextStageOrdinal + 2u &&
			stageBlock.iNextAnimationOrdinal == blockSource.iNextAnimationOrdinal + 2u,
			"Stage multi selection duplicated an unselected owner or consumed repeated IDs");
		auto firstCopy = blockSource.Stages[0];
		firstCopy.strStageId = stageBlock.Stages[3].strStageId;
		firstCopy.strActionId = stageBlock.Stages[3].strActionId;
		firstCopy.AnimationOccurrences[0].strOccurrenceId =
			stageBlock.Stages[3].AnimationOccurrences[0].strOccurrenceId;
		auto thirdCopy = blockSource.Stages[2];
		thirdCopy.strStageId = stageBlock.Stages[4].strStageId;
		thirdCopy.strActionId = stageBlock.Stages[4].strActionId;
		thirdCopy.AnimationOccurrences[0].strOccurrenceId =
			stageBlock.Stages[4].AnimationOccurrences[0].strOccurrenceId;
		auto expectedBlock = blockSource;
		expectedBlock.iNextStageOrdinal += 2u;
		expectedBlock.iNextAnimationOrdinal += 2u;
		expectedBlock.Stages.insert(expectedBlock.Stages.begin() + 3, { firstCopy, thirdCopy });
		Require(stageBlock == expectedBlock,
			"noncontiguous Stage copies were interleaved or followed click order instead of source order");
		RequireEditorStep(workbench.Move_Stage(blockPatternId, firstCopy.strStageId, -1, status),
			status, "separate the selected copied Stages with an unselected Stage");
		RequireEditorStep(workbench.Move_SelectedStage(blockPatternId, -1, status), status,
			"move noncontiguous selected Stages earlier together");
		expectedBlock.Stages = { blockSource.Stages[0], firstCopy, blockSource.Stages[1],
			thirdCopy, blockSource.Stages[2], blockSource.Stages[3] };
		Require(EditorPattern(workbench, blockPatternId) == expectedBlock,
			"noncontiguous Earlier changed spacing, source order or unselected Stage contents");
		RequireEditorStep(workbench.Move_SelectedStage(blockPatternId, 1, status), status,
			"move the same noncontiguous Stage selection later");
		RequireEditorStep(workbench.Move_Stage(blockPatternId, firstCopy.strStageId, 1, status),
			status, "restore adjacent copied Stages after noncontiguous movement");
		Require(EditorPattern(workbench, blockPatternId) == stageBlock,
			"noncontiguous movement did not preserve both selected Stage IDs");
		RequireEditorRoundtrip(workbench);

		RequireEditorStep(workbench.Duplicate_TimelineSelection(blockPatternId, {},
			{ laterBoxId, blockBoxIds[0], blockBoxIds[1], laterBoxId }, status), status,
			"duplicate animation-only selection across owners with an unselected middle row");
		const auto animationBlock = EditorPattern(workbench, blockPatternId);
		Require(animationBlock.Stages.size() == stageBlock.Stages.size() + 2u &&
			animationBlock.iNextStageOrdinal == stageBlock.iNextStageOrdinal + 2u &&
			animationBlock.iNextAnimationOrdinal == stageBlock.iNextAnimationOrdinal + 3u,
			"animation-only duplication reused owners or duplicated a selected animation twice");
		auto firstAnimationCopy = blockSource.Stages[0];
		firstAnimationCopy.strStageId = animationBlock.Stages[2].strStageId;
		firstAnimationCopy.strActionId = animationBlock.Stages[2].strActionId;
		firstAnimationCopy.iDurationMs = 1000u;
		firstAnimationCopy.AnimationOccurrences[0].iStartOffsetMs = 0u;
		firstAnimationCopy.AnimationOccurrences[0].strOccurrenceId =
			animationBlock.Stages[2].AnimationOccurrences[0].strOccurrenceId;
		auto secondAnimationCopy = blockSource.Stages[1];
		secondAnimationCopy.strStageId = animationBlock.Stages[3].strStageId;
		secondAnimationCopy.strActionId = animationBlock.Stages[3].strActionId;
		secondAnimationCopy.iDurationMs = 6900u;
		secondAnimationCopy.AnimationOccurrences.erase(secondAnimationCopy.AnimationOccurrences.begin() + 1);
		for (std::size_t index = 0; index < secondAnimationCopy.AnimationOccurrences.size(); ++index)
		{
			secondAnimationCopy.AnimationOccurrences[index].iStartOffsetMs -= 100u;
			secondAnimationCopy.AnimationOccurrences[index].strOccurrenceId =
				animationBlock.Stages[3].AnimationOccurrences[index].strOccurrenceId;
		}
		auto expectedAnimationBlock = stageBlock;
		expectedAnimationBlock.iNextStageOrdinal += 2u;
		expectedAnimationBlock.iNextAnimationOrdinal += 3u;
		expectedAnimationBlock.Stages.insert(expectedAnimationBlock.Stages.begin() + 2,
			{ firstAnimationCopy, secondAnimationCopy });
		Require(animationBlock == expectedAnimationBlock,
			"animation block did not trim outer gaps, preserve selected spacing or retain original Stage tails");
		RequireEditorStep(workbench.Move_SelectedStage(blockPatternId, -1, status), status,
			"move multiple selected animations through each owner Stage once");
		expectedAnimationBlock.Stages = { blockSource.Stages[0], firstAnimationCopy, secondAnimationCopy,
			blockSource.Stages[1], blockSource.Stages[2], firstCopy, thirdCopy, blockSource.Stages[3] };
		Require(EditorPattern(workbench, blockPatternId) == expectedAnimationBlock,
			"animation selection moved its shared owner more than once or collapsed the selection");
		RequireEditorStep(workbench.Move_SelectedStage(blockPatternId, 1, status), status,
			"move the same animation-only selection later");
		Require(EditorPattern(workbench, blockPatternId) == animationBlock,
			"animation-only Earlier/Later did not preserve selection, timing or source content");
		RequireEditorRoundtrip(workbench);
	}

	void VerifyKoukuLogicControls(Client::CKoukuSaydonActionWorkbench& workbench,
		const std::filesystem::path& sourcePath)
	{
		using namespace Client;
		const auto originalDraft = workbench.Get_Composition();
		const std::string originalBytes = ReadText(sourcePath);
		Require(!workbench.Is_Dirty(), "Logic fixture requires a saved editor draft");
		std::string status;
		RequireEditorStep(workbench.Select_ActorProfile("MN_RPCT_05", status), status,
			"select Saydon for Logic controls");
		std::string patternId;
		RequireEditorStep(workbench.Create_Pattern("Logic resource roundtrip", "NORMAL",
			patternId, status), status, "create Logic fixture Pattern");
		RequireEditorStep(workbench.Append_ActionAsStages(patternId, "MN_RPCT_05",
			4219811u, status), status, "append Logic fixture animation");
		const auto animationStages = EditorPattern(workbench, patternId).Stages;
		std::string referencedLogicId, unrelatedLogicId, boxId;
		RequireEditorStep(workbench.Create_Logic("Referenced duration", "DURATION",
			referencedLogicId, status), status, "create duration Logic");
		RequireEditorStep(workbench.Create_Logic("Unrelated trigger", "TRIGGER",
			unrelatedLogicId, status), status, "create unreferenced trigger Logic");
		const auto& definitions = workbench.Get_Composition().Logics;
		Require(definitions.size() == originalDraft.Logics.size() + 2u &&
			referencedLogicId != unrelatedLogicId &&
			definitions[definitions.size() - 2u].strLogicId == referencedLogicId &&
			definitions[definitions.size() - 2u].strDisplayName == "Referenced duration" &&
			definitions[definitions.size() - 2u].strLogicType == "DURATION" &&
			definitions.back().strLogicId == unrelatedLogicId &&
			definitions.back().strLogicType == "TRIGGER",
			"Logic creation lost definition identity, name or type");
		RequireEditorStep(workbench.Append_LogicBox(patternId, referencedLogicId,
			1000u, 1000u, boxId, status), status, "append Logic box");
		RequireEditorStep(workbench.Set_LogicBoxWindow(patternId, boxId,
			750u, 1250u, status), status, "move and trim Logic box");
		const auto edited = EditorPattern(workbench, patternId);
		Require(edited.Stages == animationStages && edited.LogicOccurrences.size() == 1u &&
			edited.LogicOccurrences[0].strOccurrenceId == boxId &&
			edited.LogicOccurrences[0].strLogicId == referencedLogicId &&
			edited.LogicOccurrences[0].iStartMs == 750u &&
			edited.LogicOccurrences[0].iDurationMs == 1250u,
			"Logic editing changed animation content or lost box identity/window");
		RequireEditorRoundtrip(workbench);
		const auto nameOnlyDefinition = *std::find_if(workbench.Get_Composition().Logics.begin(),
			workbench.Get_Composition().Logics.end(), [&](const auto& logic) { return logic.strLogicId == referencedLogicId; });
		auto playableDefinition = nameOnlyDefinition;
		playableDefinition.strJudgementKind = "EXTERNAL_SIGNAL";
		RequireEditorStep(workbench.Set_LogicDefinitionValues(referencedLogicId, playableDefinition, status),
			status, "complete the referenced duration judgement");
		RequireEditorStep(workbench.Set_PatternAuthoringStatus(patternId, "PRODUCT", status),
			status, "retain an existing PRODUCT Pattern for Logic re-editing");
		RequireEditorRoundtrip(workbench);
		const auto beforeNameOnlyEdit = workbench.Get_Composition();
		RequireEditorStep(workbench.Set_LogicDefinitionValues(referencedLogicId, nameOnlyDefinition, status),
			status, "apply name-only Logic to a previously PRODUCT Pattern");
		Require(workbench.Is_Dirty() && EditorPattern(workbench, patternId).strAuthoringStatus == "DRAFT" &&
			EditorPattern(workbench, patternId).Stages == animationStages &&
			EditorPattern(workbench, patternId).LogicOccurrences == edited.LogicOccurrences,
			"name-only Logic edit retained PRODUCT restrictions or changed its saved sequence");
		for (const auto& previous : beforeNameOnlyEdit.Patterns)
			if (previous.strPatternId != patternId)
				Require(EditorPattern(workbench, previous.strPatternId) == previous,
					"referenced Logic edit changed an unrelated Pattern");
		RequireEditorRoundtrip(workbench);
		Require(std::find(workbench.Get_Composition().Logics.begin(), workbench.Get_Composition().Logics.end(),
			nameOnlyDefinition) != workbench.Get_Composition().Logics.end() &&
			EditorPattern(workbench, patternId).strAuthoringStatus == "DRAFT" && !workbench.Is_PublishRunning(),
			"name-only Logic Save/Reload lost the draft or started publication");
		const auto validDraft = workbench.Get_Composition();
		const std::string validBytes = ReadText(sourcePath);
		Require(!workbench.Set_LogicBoxWindow(patternId, boxId, 4000u, 1000u, status) &&
			!workbench.Delete_Logic(referencedLogicId, status) &&
			workbench.Get_Composition() == validDraft && !workbench.Is_Dirty() &&
			ReadText(sourcePath) == validBytes,
			"invalid Logic window or referenced definition delete changed saved state");

		// An unrelated Stage error must not hide a preserved Logic reference.
		auto damaged = validDraft;
		Require(damaged.Patterns.back().strPatternId == patternId,
			"Logic fixture Pattern is not the last authored Pattern");
		damaged.Patterns.back().Stages[0].iDurationMs = 0u;
		const std::string damagedBytes = CKoukuSaydonCompositionDocument::Serialize(damaged);
		Require(WriteText(sourcePath, damagedBytes), "could not damage scratch Stage duration");
		RequireEditorStep(workbench.Reload(status), status, "isolate Pattern with bad Stage");
		const auto isolatedDraft = workbench.Get_Composition();
		const auto badPattern = EditorPattern(workbench, patternId);
		Require(!badPattern.strLoadError.empty() && badPattern.LogicOccurrences.empty() &&
			badPattern.strPreservedJson.find(referencedLogicId) != std::string::npos,
			"bad Stage fixture did not isolate the Pattern with its raw Logic reference");
		Require(!workbench.Delete_Logic(referencedLogicId, status) && !status.empty() &&
			workbench.Get_Composition() == isolatedDraft && !workbench.Is_Dirty() &&
			ReadText(sourcePath) == damagedBytes,
			"deleting a quarantined Pattern's Logic changed draft or source bytes");
		RequireEditorStep(workbench.Delete_Logic(unrelatedLogicId, status), status,
			"delete unrelated Logic while a Pattern is isolated");
		Require(workbench.Get_Composition().Logics.size() == isolatedDraft.Logics.size() - 1u &&
			std::none_of(workbench.Get_Composition().Logics.begin(),
				workbench.Get_Composition().Logics.end(), [&](const auto& logic) {
					return logic.strLogicId == unrelatedLogicId;
				}) && EditorPattern(workbench, patternId) == badPattern &&
			ReadText(sourcePath) == damagedBytes,
			"unrelated Logic deletion changed the isolated Pattern or saved source");
		RequireEditorRoundtrip(workbench);
		Require(EditorPattern(workbench, patternId) == badPattern,
			"saving an unrelated Logic deletion changed preserved Pattern JSON");

		// An unreadable reference list blocks definition deletion, not other edits or Save.
		std::string unresolvedBytes = damagedBytes;
		const std::size_t patternOffset = unresolvedBytes.find("\"patternId\": \"" + patternId + "\"");
		const std::size_t listOffset = unresolvedBytes.find("\"logicOccurrences\": [", patternOffset);
		const std::size_t listStart = unresolvedBytes.find('[', listOffset);
		std::size_t listEnd = std::string::npos;
		std::size_t arrayDepth = 0u;
		bool quoted = false, escaped = false;
		for (std::size_t i = listStart; i < unresolvedBytes.size(); ++i)
		{
			const char c = unresolvedBytes[i];
			if (quoted)
			{
				if (escaped) escaped = false;
				else if (c == '\\') escaped = true;
				else if (c == '"') quoted = false;
				continue;
			}
			if (c == '"') quoted = true;
			else if (c == '[') ++arrayDepth;
			else if (c == ']' && --arrayDepth == 0u) { listEnd = i; break; }
		}
		Require(patternOffset != std::string::npos && listOffset != std::string::npos &&
			listStart != std::string::npos && listEnd != std::string::npos,
			"could not locate the scratch Logic reference list");
		unresolvedBytes.replace(listStart, listEnd - listStart + 1u, "{}");
		Require(WriteText(sourcePath, unresolvedBytes), "could not damage scratch Logic list");
		RequireEditorStep(workbench.Reload(status), status, "isolate unreadable Logic list");
		const auto unresolvedDraft = workbench.Get_Composition();
		const auto unresolvedPattern = EditorPattern(workbench, patternId);
		Require(!unresolvedPattern.strLoadError.empty() &&
			!workbench.Delete_Logic(unrelatedLogicId, status) && !status.empty() &&
			workbench.Get_Composition() == unresolvedDraft && !workbench.Is_Dirty() &&
			ReadText(sourcePath) == unresolvedBytes,
			"unresolved Logic references allowed deletion or changed draft/source bytes");
		std::string additionalLogicId;
		RequireEditorStep(workbench.Create_Logic("Still editable result", "RESULT",
			additionalLogicId, status), status, "create Logic beside an unresolved Pattern");
		Require(workbench.Get_Composition().Logics.size() == unresolvedDraft.Logics.size() + 1u &&
			EditorPattern(workbench, patternId) == unresolvedPattern,
			"unresolved references blocked a normal edit or replaced preserved Pattern JSON");
		RequireEditorRoundtrip(workbench);
		Require(EditorPattern(workbench, patternId) == unresolvedPattern,
			"Save changed an unresolved Pattern's preserved JSON");

		Require(WriteText(sourcePath, validBytes), "could not restore valid Logic fixture");
		RequireEditorStep(workbench.Reload(status), status, "reload repaired Logic fixture");
		Require(workbench.Get_Composition() == validDraft,
			"Logic fixture did not recover after restoring its valid source");
		RequireEditorStep(workbench.Delete_LogicBox(patternId, boxId, status), status,
			"delete Logic box before its definition");
		RequireEditorStep(workbench.Delete_Logic(referencedLogicId, status), status,
			"delete unlinked Logic definition");
		Require(EditorPattern(workbench, patternId).LogicOccurrences.empty() &&
			EditorPattern(workbench, patternId).Stages == animationStages,
			"Logic deletion changed animation content or retained the removed box");
		RequireEditorRoundtrip(workbench);

		Require(WriteText(sourcePath, originalBytes), "could not restore pre-Logic scratch source");
		RequireEditorStep(workbench.Reload(status), status, "restore editor state after Logic tests");
		Require(workbench.Get_Composition() == originalDraft && !workbench.Is_Dirty() &&
			ReadText(sourcePath) == originalBytes,
			"Logic tests changed the fixture for subsequent editor contracts");
	}

	void VerifyKoukuObjectPlacement(Client::CKoukuSaydonActionWorkbench& workbench,
		const std::filesystem::path& sourcePath)
	{
		using namespace Client;
		const auto original = workbench.Get_Composition();
		const auto originalBytes = ReadText(sourcePath);
		std::string status, patternId;
		RequireEditorStep(workbench.Select_ActorProfile("MN_RPCT_05", status), status, "select Object fixture owner");
		RequireEditorStep(workbench.Create_Pattern("Object placement fixture", "NORMAL", patternId, status), status, "create Object fixture");
		RequireEditorStep(workbench.Append_ActionAsStages(patternId, "MN_RPCT_05", 4219811u, status), status, "set Object lifetime");
		RequireEditorStep(workbench.Set_PatternAuthoringStatus(patternId, "PRODUCT", status), status, "admit Object fixture before placement editing");
		KOUKU_WORLD_SEQUENCE_RESOURCE card;
		card.strInstanceId = "world.object.instance.editor_fixture.card";
		card.strObjectResourceId = "world.object.editor_fixture.card";
		card.strDisplayName = "Card Idle"; card.strObjectDisplayName = "Card";
		card.bDefaultMotion = true; card.bSupportsPlacement = true; card.iDurationMs = 1000u;
		auto joker = card;
		joker.strInstanceId = "world.object.instance.editor_fixture.joker_card";
		joker.strObjectResourceId = "world.object.editor_fixture.joker_card";
		joker.strDisplayName = "Joker Idle"; joker.strObjectDisplayName = "Joker";
		auto hop = card;
		hop.strInstanceId = "world.object.instance.editor_fixture.card_hop"; hop.bDefaultMotion = false;
		workbench.Set_WorldSequenceResources({ hop, card, joker }, "Object fixture");
		const auto beforeMissingAnchor = workbench.Get_Composition();
		Require(!workbench.Append_WorldObject(card.strObjectResourceId, status) &&
			workbench.Get_Composition() == beforeMissingAnchor,
			"Append without a player reference consumed IDs or changed the draft");
		KOUKU_SAYDON_WORLD_PLACEMENT seed;
		seed.Position = { 10.0, 2.0, 30.0 };
		workbench.Set_WorldPlacementResolver([&](auto& placement, auto&) { placement = seed; return true; });
		RequireEditorStep(workbench.Append_WorldObject(card.strObjectResourceId, status), status, "append Card Object");
		const auto first = EditorPattern(workbench, patternId).WorldOccurrences.back();
		seed.Position = { 16.0, 2.0, 30.0 };
		RequireEditorStep(workbench.Append_WorldObject(card.strObjectResourceId, status), status, "append second Card Object");
		RequireEditorStep(workbench.Append_WorldObject(joker.strObjectResourceId, status), status, "append Joker Object");
		const auto placed = EditorPattern(workbench, patternId).WorldOccurrences;
		Require(placed.size() == 3u && placed[0].strWorldId == placed[1].strWorldId &&
			placed[0].strOccurrenceId != placed[1].strOccurrenceId && placed[0].strWorldId != placed[2].strWorldId &&
			placed[0].Placement->Position == first.Placement->Position && placed[1].Placement->Position == seed.Position &&
			placed[0].iDurationMs == 4333u && placed[1].iDurationMs == 4333u,
			"Object Append did not preserve independent placements or full Pattern lifetime");
		const auto& worlds = workbench.Get_Composition().Worlds;
		const auto definition = std::find_if(worlds.begin(), worlds.end(), [&](const auto& world) { return world.strWorldId == first.strWorldId; });
		Require(definition != worlds.end() && definition->strObjectResourceId == card.strObjectResourceId &&
			definition->strSequenceInstanceId == card.strInstanceId && definition->strDisplayName == "Card",
			"Object Append selected a non-default state or lost parent identity");
		auto transform = *first.Placement;
		transform.Position = { 40.0, 3.0, 70.0 }; transform.RotationDegrees = { 15.0, 90.0, -20.0 }; transform.Scale = { 2.0, 3.0, 4.0 };
		RequireEditorStep(workbench.Set_WorldBoxPlacement(patternId, first.strOccurrenceId, transform, status), status, "edit one Card Transform");
		const auto& changed = EditorPattern(workbench, patternId).WorldOccurrences;
		Require(changed[0].Placement == transform && changed[1] == placed[1] && changed[2] == placed[2],
			"one Object Transform changed another card or its state");
		KOUKU_PRESENTATION_PREVIEW_REQUEST preview;
		Require(workbench.Consume_PresentationPreviewRequest(preview) && preview.WorldBoxes.size() == 3u &&
			preview.WorldBoxes[0].Placement == transform && preview.WorldBoxes[1].Placement == placed[1].Placement &&
			preview.WorldBoxes[0].strOccurrenceId == first.strOccurrenceId,
			"Append/Transform preview omitted another card or changed its exact identity");
		// An unrelated playing preview previously swallowed every placement request,
		// although the numeric draft and Save both changed successfully.
		for (const auto& previewId : { std::string("unrelated.pattern"), patternId,
			std::string("preview.kouku.resource"), std::string("unrelated.bundle") })
		{
			KOUKU_PREVIEW_STATE state;
			state.bPlaying = true; state.bPaused = true; state.strPatternId = previewId;
			workbench.Set_PreviewState(state);
			transform.Position[0] += 1.0;
			RequireEditorStep(workbench.Set_WorldBoxPlacement(patternId, first.strOccurrenceId, transform, status),
				status, "move Card while another preview owns the clock");
			Require(workbench.Consume_PresentationPreviewRequest(preview) &&
				preview.strEditedOccurrenceId == first.strOccurrenceId && preview.WorldBoxes.size() == 3u &&
				preview.WorldBoxes[0].Placement == transform && preview.WorldBoxes[1].Placement == placed[1].Placement &&
				!workbench.Consume_PresentationPreviewRequest(preview),
				"playing/paused preview swallowed or duplicated the Card position update");
		}
		workbench.Set_PreviewState({});
		const auto good = workbench.Get_Composition();
		Require(EditorPattern(workbench, patternId).strAuthoringStatus == "PRODUCT" &&
			std::find(good.PlayAllPatternIds.begin(), good.PlayAllPatternIds.end(), patternId) != good.PlayAllPatternIds.end(),
			"valid Object Append/Transform removed an admitted Pattern from Product publication");
		transform.Scale[0] = 0.0;
		Require(!workbench.Set_WorldBoxPlacement(patternId, first.strOccurrenceId, transform, status) &&
			workbench.Get_Composition() == good, "invalid Object Transform changed the saved draft");
		RequireEditorRoundtrip(workbench);
		Require(EditorPattern(workbench, patternId).WorldOccurrences == good.Patterns.back().WorldOccurrences,
			"Map Transform changed during Save/Reload");
		workbench.Set_WorldSequenceResources({}, {});
		workbench.Set_WorldPlacementResolver({});
		Require(WriteText(sourcePath, originalBytes), "could not restore pre-Object fixture");
		RequireEditorStep(workbench.Reload(status), status, "restore editor after Object test");
		Require(workbench.Get_Composition() == original && !workbench.Is_Dirty(), "Object fixture leaked saved edits");
	}

	void VerifyKoukuAllLaneDuplicateContracts()
	{
		using namespace Client;
		const auto sourceRoot = CProjectDataRoot::Get();
		const auto scratchRoot = std::filesystem::temp_directory_path() /
			("LostArkKoukuAllLaneDuplicate-" + std::to_string(GetCurrentProcessId()) + "-" + std::to_string(GetTickCount64()));
		SCOPED_TEST_DIRECTORY cleanup(scratchRoot);
		const auto dataRoot = scratchRoot / "Data";
		const auto relativeSource = std::filesystem::path("KoukuSaydon/Gate1/KoukuSaydonComposition.json");
		const auto sourcePath = dataRoot / relativeSource;
		for (const char* profile : { "MN_RPCT_05", "MN_RPCT_06", "MN_RPCT_07", "MN_RPCZ_00" })
		{
			const auto relative = std::filesystem::path("Animation/Reference/KoukuSaydon") /
				(std::string(profile) + ".actionreference.json");
			Require(CopyFixture(sourceRoot / relative, dataRoot / relative), "could not copy all-lane action reference");
		}
		KOUKU_SAYDON_COMPOSITION_DOCUMENT source;
		std::string status;
		RequireEditorStep(CKoukuSaydonCompositionDocument::Parse_Text(ReadText(sourceRoot / relativeSource), source, status),
			status, "parse all-lane fixture source");
		const auto owner = std::find_if(source.Patterns.begin(), source.Patterns.end(), [](const auto& pattern) {
			return pattern.strLoadError.empty() && !pattern.Stages.empty() &&
				!pattern.Stages.front().AnimationOccurrences.empty() && pattern.Stages.front().AnimationOccurrences.front().iPlayMs >= 1000u; });
		Require(owner != source.Patterns.end(), "all-lane fixture needs one finite authored animation");
		KOUKU_SAYDON_COMPOSITION_PATTERN pattern;
		pattern.strPatternId = owner->strPatternId; pattern.strActorProfileId = owner->strActorProfileId;
		pattern.strGateId = owner->strGateId; pattern.strTargetBossPlacementId = owner->strTargetBossPlacementId;
		pattern.strDisplayName = "Native all-lane segment"; pattern.strAuthoringStatus = "DRAFT"; pattern.strCategory = "NORMAL";
		const auto patternId = pattern.strPatternId;
		auto animation = owner->Stages.front().AnimationOccurrences.front();
		animation.iStartOffsetMs = 0u; animation.iPlayMs = 1000u;
		for (unsigned i = 0u; i < 3u; ++i)
		{
			auto stage = owner->Stages.front();
			const auto ordinal = std::to_string(pattern.iNextStageOrdinal++);
			stage.strStageId = "STAGE_" + ordinal; stage.strActionId = patternId + ".stage." + ordinal;
			stage.iDurationMs = 1000u; stage.bRetargetOnEnter = false;
			animation.strOccurrenceId = patternId + ".animation." + std::to_string(pattern.iNextAnimationOrdinal++);
			stage.AnimationOccurrences = { animation }; pattern.Stages.push_back(stage);
		}
		source.Patterns.clear(); source.Folders.clear(); source.Bundles.clear(); source.PlayAllPatternIds.clear();
		source.PatternFlows.clear();
		source.Logics.clear(); source.Summons.clear(); source.Worlds.clear(); source.SceneProfiles.clear(); source.PresentationResources.clear();
		const std::array<KOUKU_SAYDON_PRESENTATION_KIND, 5u> kinds = {
			KOUKU_SAYDON_PRESENTATION_KIND::EFFECT, KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER,
			KOUKU_SAYDON_PRESENTATION_KIND::SOUND, KOUKU_SAYDON_PRESENTATION_KIND::CAMERA, KOUKU_SAYDON_PRESENTATION_KIND::LIGHT };
		const std::array<std::uint32_t, 5u> starts = { 100u, 250u, 500u, 50u, 400u };
		const std::array<std::uint32_t, 5u> lengths = { 700u, 300u, 150u, 800u, 350u };
		for (std::size_t i = 0u; i < kinds.size(); ++i)
		{
			KOUKU_SAYDON_COMPOSITION_PRESENTATION_RESOURCE resource;
			resource.strResourceId = "kakulsaydon.g1.presentation." + std::to_string(source.iNextPresentationResourceOrdinal++);
			resource.strDisplayName = "Native lane " + std::to_string(i); resource.eKind = kinds[i];
			resource.strAssetId = i == 1u ? "" : (i == 2u ? "Sound/Native/duplicate.wav" : "native.duplicate.asset." + std::to_string(i));
			if (i == 4u) resource.strResourceKind.clear();
			source.PresentationResources.push_back(resource);
			KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE box;
			box.strOccurrenceId = patternId + ".presentation." + std::to_string(pattern.iNextPresentationOccurrenceOrdinal++);
			box.strResourceId = resource.strResourceId; box.iStartMs = starts[i]; box.iDurationMs = lengths[i];
			box.PositionOffset = { .25, -.5, 1.5 }; box.RotationDegrees = { 0., 15., 0. };
			if (i != 4u) box.Scale = { .75, 1.25, 1.5 };
			if (i == 1u) box.strRegionId = "native.duplicate.region";
			pattern.PresentationOccurrences.push_back(box);
		}
		KOUKU_SAYDON_COMPOSITION_WORLD_DEFINITION world;
		world.strWorldId = "kakulsaydon.g1.world." + std::to_string(source.iNextWorldOrdinal++);
		world.strDisplayName = "Native placed card"; world.strSequenceInstanceId = "native.duplicate.world";
		world.strCompanionEffectResourceId = source.PresentationResources.front().strResourceId;
		source.Worlds.push_back(world);
		KOUKU_SAYDON_COMPOSITION_WORLD_OCCURRENCE worldBox;
		worldBox.strOccurrenceId = patternId + ".world." + std::to_string(pattern.iNextWorldOccurrenceOrdinal++);
		worldBox.strWorldId = world.strWorldId; worldBox.iStartMs = 100u; worldBox.iDurationMs = 700u;
		worldBox.fPlaybackSpeed = .75f;
		worldBox.Placement = KOUKU_SAYDON_WORLD_PLACEMENT{ { 3., 4., 5. }, { 0., 35., 0. }, { .5, 1.5, 2. } };
		pattern.WorldOccurrences.push_back(worldBox);
		pattern.PresentationOccurrences.front().strWorldOccurrenceId = worldBox.strOccurrenceId;
		auto persistentWorld = worldBox;
		persistentWorld.strOccurrenceId = patternId + ".world." + std::to_string(pattern.iNextWorldOccurrenceOrdinal++);
		persistentWorld.iStartMs = 0u; persistentWorld.iDurationMs = 3000u; pattern.WorldOccurrences.push_back(persistentWorld);
		KOUKU_SAYDON_COMPOSITION_SUMMON_DEFINITION summon;
		summon.strSummonId = "kakulsaydon.g1.summon." + std::to_string(source.iNextSummonOrdinal++);
		summon.strDisplayName = "Native summon"; source.Summons.push_back(summon);
		pattern.SummonOccurrences.push_back({ patternId + ".summon." + std::to_string(pattern.iNextSummonOccurrenceOrdinal++), summon.strSummonId, 300u, 200u });
		KOUKU_SAYDON_COMPOSITION_SCENE_PROFILE_DEFINITION scene;
		scene.strSceneProfileId = "kakulsaydon.g1.sceneprofile." + std::to_string(source.iNextSceneProfileOrdinal++);
		scene.strDisplayName = "Native scene"; scene.strRenderingProfileId = "native.duplicate.scene"; source.SceneProfiles.push_back(scene);
		pattern.SceneProfileOccurrences.push_back({ patternId + ".sceneprofile." + std::to_string(pattern.iNextSceneProfileOccurrenceOrdinal++), scene.strSceneProfileId, 350u, 400u, 75u });
		const auto addLogic = [&](const char* type, const char* name) {
			KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION logic;
			logic.strLogicId = "kakulsaydon.g1.logic." + std::to_string(source.iNextLogicOrdinal++);
			logic.strLogicType = type; logic.strDisplayName = name; return logic;
		};
		auto trigger = addLogic("TRIGGER", "Native contact");
		trigger.strTriggerKind = "OBJECT_CONTACT"; trigger.fTargetRadiusM = 1.25;
		trigger.TargetWorldOccurrenceIds = { worldBox.strOccurrenceId };
		auto motion = addLogic("RESULT", "Native card motion");
		motion.strOutcomeKind = "PLAY_CONTACT_WORLD_OBJECT_MOTION";
		motion.ContactMotions = { { worldBox.strOccurrenceId, "native.duplicate.card.flip" } };
		auto master = addLogic("DURATION", "Native persistent deadline"); master.strJudgementKind = "EXTERNAL_SIGNAL";
		auto wipe = addLogic("RESULT", "Native delayed wipe"); wipe.strOutcomeKind = "INSTANT_DEATH";
		KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE contactBox, masterBox, wipeBox;
		contactBox.strOccurrenceId = patternId + ".logic." + std::to_string(pattern.iNextLogicOccurrenceOrdinal++);
		contactBox.strLogicId = trigger.strLogicId; contactBox.iStartMs = 250u; contactBox.iDurationMs = 300u;
		masterBox.strOccurrenceId = patternId + ".logic." + std::to_string(pattern.iNextLogicOccurrenceOrdinal++);
		masterBox.strLogicId = master.strLogicId; masterBox.iDurationMs = 3000u; masterBox.OnTimeoutLogicIds = { wipe.strLogicId };
		wipeBox.strOccurrenceId = patternId + ".logic." + std::to_string(pattern.iNextLogicOccurrenceOrdinal++);
		wipeBox.strLogicId = wipe.strLogicId; wipeBox.iStartMs = 2200u; wipeBox.iDurationMs = 100u;
		auto signal = addLogic("RESULT", "Native exact completion"); signal.strOutcomeKind = "COMPLETE_LOGIC_WINDOW";
		signal.strTargetLogicOccurrenceId = masterBox.strOccurrenceId; signal.strContactTargetWorldOccurrenceId = worldBox.strOccurrenceId;
		contactBox.OnSuccessLogicIds = { motion.strLogicId, signal.strLogicId };
		source.Logics = { trigger, motion, master, wipe, signal };
		pattern.LogicOccurrences = { contactBox, masterBox, wipeBox };
		pattern.PresentationOccurrences[1].strLogicOccurrenceId = contactBox.strOccurrenceId;
		source.Patterns.push_back(pattern);
		SCOPED_ENVIRONMENT_VARIABLE environment(L"LOSTARK_PROJECT_DATA_ROOT");
		Require(environment.Set(dataRoot), "could not select all-lane scratch root");
		CKoukuSaydonActionWorkbench workbench;
		const auto reset = [&](const KOUKU_SAYDON_COMPOSITION_DOCUMENT& fixture) {
			Require(WriteText(sourcePath, CKoukuSaydonCompositionDocument::Serialize(fixture)), "could not write all-lane fixture");
			RequireEditorStep(workbench.Reload(status), status, "reload all-lane fixture");
			RequireEditorStep(EditorPattern(workbench, patternId).strLoadError.empty(),
				EditorPattern(workbench, patternId).strLoadError, "admit all-lane fixture without quarantine");
		};
		const auto findById = [](const auto& rows, const std::string& id) -> const auto& {
			const auto found = std::find_if(rows.begin(), rows.end(), [&](const auto& row) { return row.strOccurrenceId == id; });
			Require(found != rows.end(), "duplicate lost an original occurrence"); return *found;
		};
		const auto findLogic = [&](const std::string& id) -> const KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION& {
			const auto& rows = workbench.Get_Composition().Logics;
			const auto found = std::find_if(rows.begin(), rows.end(), [&](const auto& row) { return row.strLogicId == id; });
			Require(found != rows.end(), "duplicate lost a Logic definition"); return *found;
		};
		std::vector<std::string> laneSelection = { pattern.SummonOccurrences.front().strOccurrenceId, pattern.SceneProfileOccurrences.front().strOccurrenceId };
		for (const auto& box : pattern.PresentationOccurrences) laneSelection.push_back(box.strOccurrenceId);
		laneSelection.push_back(pattern.PresentationOccurrences.front().strOccurrenceId); // Repeated selection consumes no extra ID.
		const auto verifyLaneCopies = [&](const std::uint32_t delta, const bool spliced) {
			const auto& actual = EditorPattern(workbench, patternId);
			Require(actual.PresentationOccurrences.size() == 10u && actual.LogicOccurrences.size() == 4u &&
				actual.WorldOccurrences.size() == 3u && actual.SummonOccurrences.size() == 2u && actual.SceneProfileOccurrences.size() == 2u,
				"selection closure lost a lane, cloned the persistent owner, or duplicated a repeated ID");
			const auto copiedWorld = std::find_if(actual.WorldOccurrences.begin(), actual.WorldOccurrences.end(),
				[&](const auto& row) { return row.iStartMs == worldBox.iStartMs + delta; });
			const auto copiedContact = std::find_if(actual.LogicOccurrences.begin(), actual.LogicOccurrences.end(),
				[&](const auto& row) { return row.iStartMs == contactBox.iStartMs + delta; });
			Require(copiedWorld != actual.WorldOccurrences.end() && copiedContact != actual.LogicOccurrences.end(), "dependency closure did not copy World and Collider Logic");
			auto expectedWorld = worldBox; expectedWorld.strOccurrenceId = copiedWorld->strOccurrenceId; expectedWorld.iStartMs += delta;
			Require(*copiedWorld == expectedWorld && copiedWorld->strOccurrenceId != worldBox.strOccurrenceId, "World copy lost placement, speed, window or stable identity");
			for (const auto& original : pattern.PresentationOccurrences)
			{
				Require(findById(actual.PresentationOccurrences, original.strOccurrenceId) == original, "duplicate retuned an original presentation box");
				const auto copied = std::find_if(actual.PresentationOccurrences.begin(), actual.PresentationOccurrences.end(), [&](const auto& row) {
					return row.strResourceId == original.strResourceId && row.iStartMs == original.iStartMs + delta; });
				Require(copied != actual.PresentationOccurrences.end() && copied->strOccurrenceId != original.strOccurrenceId, "one of the five presentation lanes did not receive a fresh occurrence");
				auto expected = original; expected.strOccurrenceId = copied->strOccurrenceId; expected.iStartMs += delta;
				if (!expected.strWorldOccurrenceId.empty()) expected.strWorldOccurrenceId = copiedWorld->strOccurrenceId;
				if (!expected.strLogicOccurrenceId.empty()) expected.strLogicOccurrenceId = copiedContact->strOccurrenceId;
				if (!expected.strRegionId.empty()) { Require(!copied->strRegionId.empty() && copied->strRegionId != expected.strRegionId, "Collider copy reused its region ID"); expected.strRegionId = copied->strRegionId; }
				Require(*copied == expected, "copied presentation lost relative time, P/R/S, fades or its remapped owner");
			}
			auto expectedSummon = pattern.SummonOccurrences.front(); expectedSummon.iStartMs += delta;
			const auto& copiedSummon = actual.SummonOccurrences.back(); expectedSummon.strOccurrenceId = copiedSummon.strOccurrenceId;
			Require(copiedSummon == expectedSummon && copiedSummon.strOccurrenceId != pattern.SummonOccurrences.front().strOccurrenceId, "Summon copy lost its resource or lifetime");
			auto expectedScene = pattern.SceneProfileOccurrences.front(); expectedScene.iStartMs += delta;
			const auto& copiedScene = actual.SceneProfileOccurrences.back(); expectedScene.strOccurrenceId = copiedScene.strOccurrenceId;
			Require(copiedScene == expectedScene && copiedScene.strOccurrenceId != pattern.SceneProfileOccurrences.front().strOccurrenceId, "Scene copy lost its profile, blend or relative time");
			Require(findById(actual.LogicOccurrences, contactBox.strOccurrenceId) == contactBox && findById(actual.WorldOccurrences, worldBox.strOccurrenceId) == worldBox,
				"copy-on-write changed the original linked pair");
			auto expectedMaster = masterBox; auto expectedPersistent = persistentWorld; auto expectedWipe = wipeBox;
			if (spliced) { expectedMaster.iDurationMs += delta; expectedPersistent.iDurationMs += delta; expectedWipe.iStartMs += delta; }
			Require(findById(actual.LogicOccurrences, masterBox.strOccurrenceId) == expectedMaster &&
				findById(actual.LogicOccurrences, wipeBox.strOccurrenceId) == expectedWipe &&
				findById(actual.WorldOccurrences, persistentWorld.strOccurrenceId) == expectedPersistent,
				"segment insertion did not stretch the persistent deadline/World and delay the original wipe exactly once");
			for (const auto& original : source.Logics) Require(findLogic(original.strLogicId) == original, "copy-on-write modified a shared original Logic definition");
			const auto& copiedTrigger = findLogic(copiedContact->strLogicId);
			auto expectedTrigger = trigger; expectedTrigger.strLogicId = copiedTrigger.strLogicId; expectedTrigger.strDisplayName = copiedTrigger.strDisplayName;
			expectedTrigger.TargetWorldOccurrenceIds = { copiedWorld->strOccurrenceId };
			Require(copiedTrigger == expectedTrigger && copiedTrigger.strLogicId != trigger.strLogicId, "copied Contact did not isolate its selected World target");
			Require(copiedContact->OnSuccessLogicIds.size() == 2u, "copied Contact lost its ordered outcomes");
			auto expectedContact = contactBox; expectedContact.strOccurrenceId = copiedContact->strOccurrenceId;
			expectedContact.strLogicId = copiedContact->strLogicId; expectedContact.iStartMs += delta;
			expectedContact.OnSuccessLogicIds = copiedContact->OnSuccessLogicIds;
			Require(*copiedContact == expectedContact && copiedContact->strOccurrenceId != contactBox.strOccurrenceId,
				"copied Logic lost its window, enabled state or occurrence identity");
			const auto& copiedMotion = findLogic(copiedContact->OnSuccessLogicIds[0]);
			auto expectedMotion = motion; expectedMotion.strLogicId = copiedMotion.strLogicId; expectedMotion.strDisplayName = copiedMotion.strDisplayName;
			expectedMotion.ContactMotions.front().strTargetWorldOccurrenceId = copiedWorld->strOccurrenceId;
			const auto& copiedSignal = findLogic(copiedContact->OnSuccessLogicIds[1]);
			auto expectedSignal = signal; expectedSignal.strLogicId = copiedSignal.strLogicId; expectedSignal.strDisplayName = copiedSignal.strDisplayName;
			expectedSignal.strContactTargetWorldOccurrenceId = copiedWorld->strOccurrenceId;
			Require(copiedMotion == expectedMotion && copiedSignal == expectedSignal && copiedMotion.strLogicId != motion.strLogicId && copiedSignal.strLogicId != signal.strLogicId,
				"copied outcomes lost their exact card or rewired the unselected persistent master");
			Require(workbench.Get_Composition().PresentationResources == source.PresentationResources && workbench.Get_Composition().Worlds == source.Worlds &&
				workbench.Get_Composition().Summons == source.Summons && workbench.Get_Composition().SceneProfiles == source.SceneProfiles,
				"segment duplication cloned or modified reusable resource definitions");
		};
		reset(source);
		auto mixedSelection = laneSelection; mixedSelection.push_back(pattern.Stages.front().AnimationOccurrences.front().strOccurrenceId);
		RequireEditorStep(workbench.Duplicate_TimelineSelection(patternId, { pattern.Stages.front().strStageId, pattern.Stages.front().strStageId }, mixedSelection, status),
			status, "duplicate one mixed segment with linked-lane closure");
		const auto mixed = EditorPattern(workbench, patternId);
		Require(mixed.Stages.size() == 4u && mixed.Stages[0] == pattern.Stages[0] && mixed.Stages[2] == pattern.Stages[1] && mixed.Stages[3] == pattern.Stages[2],
			"mixed segment did not insert once after the selected owner or altered later Stage content");
		auto expectedStage = pattern.Stages.front(); expectedStage.strStageId = mixed.Stages[1].strStageId; expectedStage.strActionId = mixed.Stages[1].strActionId;
		expectedStage.AnimationOccurrences.front().strOccurrenceId = mixed.Stages[1].AnimationOccurrences.front().strOccurrenceId;
		Require(mixed.Stages[1] == expectedStage && mixed.iNextStageOrdinal == pattern.iNextStageOrdinal + 1u && mixed.iNextAnimationOrdinal == pattern.iNextAnimationOrdinal + 1u,
			"selected Stage plus selected child duplicated its animation twice or lost its source clip");
		verifyLaneCopies(1000u, true); RequireEditorRoundtrip(workbench);
		reset(source);
		RequireEditorStep(workbench.Duplicate_TimelineSelection(patternId, {}, laneSelection, status), status, "duplicate lane-only envelope next to itself");
		Require(EditorPattern(workbench, patternId).Stages == pattern.Stages, "lane-only duplication spliced the Stage timeline");
		verifyLaneCopies(800u, false); RequireEditorRoundtrip(workbench);
		auto logicOnly = source;
		for (auto& definition : logicOnly.Logics)
		{
			if (definition.strLogicId == trigger.strLogicId) definition.TargetWorldOccurrenceIds = { persistentWorld.strOccurrenceId };
			if (definition.strLogicId == motion.strLogicId) definition.ContactMotions.front().strTargetWorldOccurrenceId = persistentWorld.strOccurrenceId;
			if (definition.strLogicId == signal.strLogicId) definition.strContactTargetWorldOccurrenceId = persistentWorld.strOccurrenceId;
		}
		auto& logicPattern = logicOnly.Patterns.front();
		auto secondLinkedCollider = pattern.PresentationOccurrences[1];
		secondLinkedCollider.strOccurrenceId = patternId + ".presentation." + std::to_string(logicPattern.iNextPresentationOccurrenceOrdinal++);
		secondLinkedCollider.strRegionId = "native.duplicate.second.region"; secondLinkedCollider.PositionOffset[0] = 1.75;
		logicPattern.PresentationOccurrences.push_back(secondLinkedCollider);
		reset(logicOnly);
		RequireEditorStep(workbench.Duplicate_TimelineSelection(patternId, {}, { contactBox.strOccurrenceId }, status),
			status, "duplicate a Logic selection with every linked Collider");
		const auto logicCopy = EditorPattern(workbench, patternId);
		const auto copiedWindow = std::find_if(logicCopy.LogicOccurrences.begin(), logicCopy.LogicOccurrences.end(),
			[&](const auto& row) { return row.strOccurrenceId != contactBox.strOccurrenceId && row.strLogicId == contactBox.strLogicId && row.iStartMs == 550u && row.iDurationMs == 300u; });
		Require(copiedWindow != logicCopy.LogicOccurrences.end() && logicCopy.LogicOccurrences.size() == 4u &&
			logicCopy.PresentationOccurrences.size() == 8u && logicCopy.Stages == pattern.Stages && logicCopy.WorldOccurrences == pattern.WorldOccurrences &&
			workbench.Get_Composition().Logics == logicOnly.Logics && findById(logicCopy.LogicOccurrences, masterBox.strOccurrenceId) == masterBox &&
			findById(logicCopy.LogicOccurrences, wipeBox.strOccurrenceId) == wipeBox,
			"Logic-only closure lost a Collider, cloned shared targets/outcomes or moved the original master/wipe");
		for (const auto& original : { pattern.PresentationOccurrences[1], secondLinkedCollider })
		{
			Require(findById(logicCopy.PresentationOccurrences, original.strOccurrenceId) == original, "Logic-only closure altered an original Collider");
			const auto copied = std::find_if(logicCopy.PresentationOccurrences.begin(), logicCopy.PresentationOccurrences.end(), [&](const auto& row) {
				return row.strLogicOccurrenceId == copiedWindow->strOccurrenceId && row.PositionOffset == original.PositionOffset; });
			Require(copied != logicCopy.PresentationOccurrences.end() && copied->strOccurrenceId != original.strOccurrenceId &&
				!copied->strRegionId.empty() && copied->strRegionId != original.strRegionId, "Logic-only copy did not give each Collider its own IDs and copied owner");
			auto expected = original; expected.strOccurrenceId = copied->strOccurrenceId; expected.strRegionId = copied->strRegionId;
			expected.strLogicOccurrenceId = copiedWindow->strOccurrenceId; expected.iStartMs = 550u;
			Require(*copied == expected, "Logic-only copy changed a linked Collider's geometry or resource");
		}
		RequireEditorRoundtrip(workbench);
		reset(source);
		auto lateCamera = pattern.PresentationOccurrences[3]; lateCamera.iStartMs = 2200u; lateCamera.iDurationMs = 700u;
		RequireEditorStep(workbench.Set_PresentationBox(patternId, lateCamera, status), status, "place a late camera for lane-only extension");
		RequireEditorStep(workbench.Duplicate_TimelineSelection(patternId, {}, { lateCamera.strOccurrenceId }, status), status, "extend only the final Stage for an adjacent lane copy");
		const auto extended = EditorPattern(workbench, patternId);
		Require(extended.Stages.size() == 3u && extended.Stages[0] == pattern.Stages[0] && extended.Stages[1] == pattern.Stages[1] &&
			extended.Stages.back().iDurationMs == 1600u && findById(extended.LogicOccurrences, wipeBox.strOccurrenceId) == wipeBox,
			"lane-only tail extension moved an original event or earlier Stage");
		Require(std::any_of(extended.PresentationOccurrences.begin(), extended.PresentationOccurrences.end(), [&](const auto& row) {
			return row.strResourceId == lateCamera.strResourceId && row.strOccurrenceId != lateCamera.strOccurrenceId && row.iStartMs == 2900u && row.iDurationMs == 700u; }),
			"lane-only copy did not start exactly at its source endpoint");
		RequireEditorRoundtrip(workbench);
		const auto reject = [&](const std::vector<std::string>& stages, const std::vector<std::string>& boxes, const char* message) {
			const auto before = workbench.Get_Composition(); const auto generation = workbench.Get_DraftGeneration();
			const auto dirty = workbench.Is_Dirty(); const auto bytes = ReadText(sourcePath);
			Require(!workbench.Duplicate_TimelineSelection(patternId, stages, boxes, status) && !status.empty() &&
				workbench.Get_Composition() == before && workbench.Get_DraftGeneration() == generation && workbench.Is_Dirty() == dirty && ReadText(sourcePath) == bytes, message);
		};
		reset(source);
		reject({ pattern.Stages.front().strStageId }, { pattern.PresentationOccurrences.front().strOccurrenceId, "MISSING_OCCURRENCE" }, "invalid generic occurrence partially committed a mixed copy");
		reject({}, { pattern.Stages.front().strStageId }, "generic occurrence selection accepted a Stage ID");
		RequireEditorStep(workbench.Set_PatternDuration(patternId, 600000u, status), status, "fill the bounded authoring timeline");
		RequireEditorRoundtrip(workbench);
		reject({ pattern.Stages.front().strStageId }, mixedSelection, "mixed time overflow changed source, dirty state, generation or ordinals");
		auto exhausted = source; exhausted.iNextLogicOrdinal = 1000000u; reset(exhausted);
		reject({ pattern.Stages.front().strStageId }, mixedSelection, "copy-on-write Logic ordinal exhaustion partially inserted a segment");
		exhausted = source; exhausted.Patterns.front().iNextPresentationOccurrenceOrdinal = 1000000u; reset(exhausted);
		reject({ pattern.Stages.front().strStageId }, mixedSelection, "presentation ordinal exhaustion partially inserted a segment");
	}

	void VerifyKoukuPreviewTransportContracts()
	{
		using namespace Client;
		const auto sourceRoot = CProjectDataRoot::Get();
		const auto scratchRoot = std::filesystem::temp_directory_path() /
			("LostArkKoukuPreviewTransport-" + std::to_string(GetCurrentProcessId()) +
			 "-" + std::to_string(GetTickCount64()));
		SCOPED_TEST_DIRECTORY cleanup(scratchRoot);
		const auto dataRoot = scratchRoot / "Data";
		const auto relativeSource = std::filesystem::path("KoukuSaydon/Gate1/KoukuSaydonComposition.json");
		const auto sourcePath = dataRoot / relativeSource;
		for (const char* profile : { "MN_RPCT_05", "MN_RPCT_06", "MN_RPCT_07", "MN_RPCZ_00" })
		{
			const auto relative = std::filesystem::path("Animation/Reference/KoukuSaydon") /
				(std::string(profile) + ".actionreference.json");
			Require(CopyFixture(sourceRoot / relative, dataRoot / relative), "could not copy preview transport action reference");
		}
		KOUKU_SAYDON_COMPOSITION_DOCUMENT source;
		std::string status;
		RequireEditorStep(CKoukuSaydonCompositionDocument::Parse_Text(ReadText(sourceRoot / relativeSource), source, status),
			status, "parse preview transport source");
		const auto owner = std::find_if(source.Patterns.begin(), source.Patterns.end(), [](const auto& pattern) {
			return pattern.strLoadError.empty() && pattern.strActorProfileId == "MN_RPCT_06" &&
				!pattern.Stages.empty() && pattern.Stages.front().iDurationMs >= 1000u &&
				!pattern.Stages.front().AnimationOccurrences.empty(); });
		const auto resource = std::find_if(source.PresentationResources.begin(), source.PresentationResources.end(),
			[](const auto& row) { return row.eKind == KOUKU_SAYDON_PRESENTATION_KIND::COLLIDER && row.strColliderKind == "GEOMETRY"; });
		Require(owner != source.Patterns.end() && resource != source.PresentationResources.end(),
			"preview transport fixture needs a real hammer actor and Collider resource");
		const auto patternId = owner->strPatternId;
		// Use a finite real animation, with no unrelated lanes extending the endpoint.
		owner->Stages.resize(1u);
		owner->Stages.front().bRetargetOnEnter = false;
		owner->BossMotion.reset();
		for (auto& pattern : source.Patterns) pattern.strAuthoringStatus = "DRAFT";
		for (auto& existingBundle : source.Bundles) existingBundle.strAuthoringStatus = "DRAFT";
		source.PlayAllPatternIds.clear();
		owner->PresentationOccurrences.clear(); owner->WorldOccurrences.clear();
		owner->SceneProfileOccurrences.clear(); owner->LogicOccurrences.clear(); owner->SummonOccurrences.clear();
		const auto duration = owner->Stages.front().iDurationMs;
		KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE collider;
		collider.strOccurrenceId = patternId + ".presentation." + std::to_string(owner->iNextPresentationOccurrenceOrdinal++);
		collider.strResourceId = resource->strResourceId;
		collider.iStartMs = 100u; collider.iDurationMs = 600u;
		collider.strBoneTarget = "WEAPON"; collider.strBone = "b_rpct_01"; collider.bFollowBoss = true;
		owner->PresentationOccurrences.push_back(collider);
		auto laterCollider = collider;
		laterCollider.strOccurrenceId = patternId + ".presentation." + std::to_string(owner->iNextPresentationOccurrenceOrdinal++);
		laterCollider.iStartMs = 800u; laterCollider.iDurationMs = 200u;
		owner->PresentationOccurrences.push_back(laterCollider);
		auto sharedCollider = collider;
		sharedCollider.strOccurrenceId = patternId + ".presentation." + std::to_string(owner->iNextPresentationOccurrenceOrdinal++);
		owner->PresentationOccurrences.push_back(sharedCollider);
		std::array<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE, 2u> effects;
		for (std::size_t i = 0u; i < effects.size(); ++i)
		{
			const auto effectResource = std::find_if(source.PresentationResources.begin(), source.PresentationResources.end(),
				[&](const auto& row) { return row.eKind == KOUKU_SAYDON_PRESENTATION_KIND::EFFECT && row.strResourceKind == (i == 0u ? "LEAF" : "GROUP"); });
			Require(effectResource != source.PresentationResources.end(), "geometry fixture needs V2 LEAF and GROUP resources");
			auto& effect = effects[i];
			effect.strOccurrenceId = patternId + ".presentation." + std::to_string(owner->iNextPresentationOccurrenceOrdinal++);
			effect.strResourceId = effectResource->strResourceId;
			effect.iStartMs = 100u; effect.iDurationMs = 600u; effect.bFollowBoss = true;
			owner->PresentationOccurrences.push_back(effect);
		}
		KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION contact;
		contact.strLogicId = "kakulsaydon.g1.logic." + std::to_string(source.iNextLogicOrdinal++);
		contact.strDisplayName = "Native Collider Apply contact"; contact.strLogicType = "TRIGGER";
		source.Logics.push_back(contact);
		KOUKU_SAYDON_COMPOSITION_FOLDER folder;
		folder.strFolderId = "kakulsaydon.folder." + std::to_string(source.iNextFolderOrdinal++);
		folder.strGateId = owner->strGateId; folder.strDisplayName = "Preview transport fixture";
		source.Folders.push_back(folder);
		KOUKU_SAYDON_COMPOSITION_BUNDLE bundle;
		bundle.strBundleId = "kakulsaydon.bundle." + std::to_string(source.iNextBundleOrdinal++);
		bundle.strGateId = folder.strGateId; bundle.strFolderId = folder.strFolderId;
		bundle.strDisplayName = "Preview transport fixture";
		bundle.Members.push_back({bundle.strBundleId + ".member." + std::to_string(bundle.iNextMemberOrdinal++), patternId, 100u});
		source.Bundles.push_back(bundle);
		Require(WriteText(sourcePath, CKoukuSaydonCompositionDocument::Serialize(source)), "could not write preview transport fixture");
		SCOPED_ENVIRONMENT_VARIABLE environment(L"LOSTARK_PROJECT_DATA_ROOT");
		Require(environment.Set(dataRoot), "could not select preview transport scratch root");
		CKoukuSaydonActionWorkbench workbench;
		RequireEditorStep(workbench.Reload(status), status, "load preview transport fixture");
		const auto baseline = workbench.Get_Composition();
		const auto bytes = ReadText(sourcePath);
		const auto generation = workbench.Get_DraftGeneration();
		const auto expectPattern = [&](const std::uint32_t expectedClock, const bool_t expectedPaused,
			const KOUKU_SAYDON_COMPOSITION_PATTERN& expected) {
			KOUKU_SAYDON_COMPOSITION_PATTERN actual;
			std::uint32_t clock = 0u; bool_t paused = false; std::string target;
			Require(workbench.Consume_PatternPreviewRequest(actual, clock, paused, target) &&
				actual == expected && clock == expectedClock && paused == expectedPaused && target.empty(),
				"Pattern transport lost its exact clock, pause flag or authored content");
			Require(!workbench.Consume_PatternPreviewRequest(actual, clock, paused, target), "Pattern preview request was not one-shot");
		};
		const auto expectTransport = [&](const KOUKU_PREVIEW_TRANSPORT expected, const std::uint32_t expectedClock) {
			KOUKU_PREVIEW_TRANSPORT command = KOUKU_PREVIEW_TRANSPORT::NONE; std::uint32_t clock = 0u;
			Require(workbench.Consume_PreviewTransportRequest(command, clock) && command == expected && clock == expectedClock,
				"Preview transport changed the requested operation or clock");
			Require(!workbench.Consume_PreviewTransportRequest(command, clock), "Preview transport request was not one-shot");
		};
		const auto expected = EditorPattern(workbench, patternId);
		KOUKU_PREVIEW_STATE repeatedFailure;
		repeatedFailure.strStatus = "Actor preview could not stage its selected model.";
		for (unsigned retry = 0u; retry < 2u; ++retry)
		{
			RequireEditorStep(workbench.Request_PatternPreview(patternId, 0u, status), status,
				"request Pattern after the same staging failure");
			expectPattern(0u, false, expected);
			workbench.Set_PreviewState(repeatedFailure);
			Require(workbench.Get_Status() == repeatedFailure.strStatus,
				"repeated Pattern failure left the request acknowledgement visible");
		}
		KOUKU_PREVIEW_STATE steadyState;
		steadyState.strStatus = "Pattern preview ready.";
		workbench.Set_PreviewState(steadyState);
		Require(!workbench.Set_PatternDuration("absent.pattern", 1000u, status),
			"invalid edit fixture unexpectedly succeeded");
		const auto editStatus = workbench.Get_Status();
		for (unsigned frame = 0u; frame < 3u; ++frame) workbench.Set_PreviewState(steadyState);
		Require(workbench.Get_Status() == editStatus,
			"unchanged frame status erased the unrelated edit diagnostic");
		workbench.Set_PreviewState({});
		Require(!workbench.Request_PreviewPause(), "inactive preview accepted Pause");
		RequireEditorStep(workbench.Request_PatternScrub(patternId, 321u, status), status, "cold Pattern ruler seek");
		expectPattern(321u, true, expected);
		RequireEditorStep(workbench.Request_PatternScrub(patternId, 600000u, status), status, "cold Pattern endpoint seek");
		expectPattern(duration, true, expected);
		RequireEditorStep(workbench.Request_PatternPreview(patternId, duration, status), status, "Play from Pattern endpoint");
		expectPattern(0u, false, expected);
		RequireEditorStep(workbench.Request_PatternPreview(patternId, 250u, status, true), status, "explicit paused Pattern start");
		expectPattern(250u, true, expected);
		RequireEditorStep(workbench.Request_PatternPreview(patternId, 654u, status), status, "queue Pattern before its first frame");
		Require(workbench.Request_PreviewPause(), "pending Pattern refused Pause before its first frame");
		expectPattern(654u, true, expected);

		KOUKU_PREVIEW_STATE state;
		state.bPlaying = true; state.strPatternId = patternId; state.iDurationMs = duration; state.iClockMs = 432u;
		workbench.Set_PreviewState(state);
		Require(workbench.Request_PreviewPause(), "running Pattern refused Pause");
		expectTransport(KOUKU_PREVIEW_TRANSPORT::PAUSE, 432u);
		for (const bool_t paused : {false, true})
		{
			state.bPaused = paused; workbench.Set_PreviewState(state);
			RequireEditorStep(workbench.Request_PatternScrub(patternId, 543u, status), status, "live Pattern ruler seek");
			expectTransport(KOUKU_PREVIEW_TRANSPORT::SEEK, 543u);
			RequireEditorStep(workbench.Request_PatternScrub(patternId, 600000u, status), status, "live Pattern endpoint seek");
			expectTransport(KOUKU_PREVIEW_TRANSPORT::SEEK, duration);
		}
		state.bPaused = true; state.iClockMs = 543u; workbench.Set_PreviewState(state);
		auto edited = collider; edited.PositionOffset = {0.25, -0.5, 1.5}; edited.Scale = {0.5, 0.75, 1.25};
		auto editedPattern = expected;
		for (auto& row : editedPattern.PresentationOccurrences) if (row.strOccurrenceId == edited.strOccurrenceId) row = edited;
		RequireEditorStep(workbench.Request_ColliderBoxPreview(patternId, edited, status), status, "paused Collider Detail refresh");
		expectPattern(543u, true, editedPattern);
		state.iClockMs = duration; workbench.Set_PreviewState(state);
		RequireEditorStep(workbench.Request_ColliderBoxPreview(patternId, edited, status), status, "paused endpoint Collider Detail refresh");
		expectPattern(duration, true, editedPattern);
		state.strPatternId = "other.preview"; workbench.Set_PreviewState(state);
		RequireEditorStep(workbench.Request_ColliderBoxPreview(patternId, edited, status), status, "Collider preview from another paused owner");
		expectPattern(collider.iStartMs, false, editedPattern);
		workbench.Set_PreviewState({});
		RequireEditorStep(workbench.Request_PatternScrub(patternId, 321u, status), status, "retain valid pending scrub");
		Require(!workbench.Request_PatternScrub("absent.pattern", 654u, status), "unknown Pattern scrub was accepted");
		expectPattern(321u, true, expected);

		RequireEditorStep(workbench.Select_BundleById(bundle.strBundleId, status), status, "select preview transport bundle");
		const auto bundleDuration = duration + 100u;
		const auto expectBundle = [&](const std::uint32_t expectedClock) {
			std::string id; std::uint32_t clock = 0u; bool_t paused = false;
			Require(workbench.Consume_BundlePreviewRequest(id, clock, paused) &&
				id == bundle.strBundleId && clock == expectedClock && paused, "Bundle scrub lost its identity, endpoint or paused state");
			Require(!workbench.Consume_BundlePreviewRequest(id, clock, paused), "Bundle scrub request was not one-shot");
		};
		for (unsigned retry = 0u; retry < 2u; ++retry)
		{
			Require(workbench.Request_BundleScrub(0u), "request Bundle after the same staging failure");
			expectBundle(0u);
			workbench.Set_PreviewState(repeatedFailure);
			Require(workbench.Get_Status() == repeatedFailure.strStatus,
				"repeated Bundle failure left the request acknowledgement visible");
		}
		Require(workbench.Request_BundleScrub(321u), "cold Bundle ruler seek failed"); expectBundle(321u);
		Require(workbench.Request_BundleScrub(600000u), "cold Bundle endpoint seek failed"); expectBundle(bundleDuration);
		state.strPatternId = bundle.strBundleId; state.iDurationMs = bundleDuration; state.iClockMs = 432u;
		for (const bool_t paused : {false, true})
		{
			state.bPlaying = true; state.bPaused = paused; workbench.Set_PreviewState(state);
			Require(workbench.Request_PreviewPause(), "Bundle refused Pause");
			expectTransport(KOUKU_PREVIEW_TRANSPORT::PAUSE, 432u);
			Require(workbench.Request_BundleScrub(543u), "live Bundle ruler seek failed");
			expectTransport(KOUKU_PREVIEW_TRANSPORT::SEEK, 543u);
			Require(workbench.Request_BundleScrub(600000u), "live Bundle endpoint seek failed");
			expectTransport(KOUKU_PREVIEW_TRANSPORT::SEEK, bundleDuration);
		}
		// Live Detail geometry uses an overlay, preserving the existing playback session and authored data.
		std::uint32_t geometryCheck = 0u;
		const auto expectGeometry = [&](const KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE& expectedBox) {
			const auto authored = std::find_if(expected.PresentationOccurrences.begin(), expected.PresentationOccurrences.end(),
				[&](const auto& row) { return row.strOccurrenceId == expectedBox.strOccurrenceId; });
			Require(authored != expected.PresentationOccurrences.end(), "geometry fixture occurrence is missing");
			auto expectedOverlay = *authored;
			expectedOverlay.PositionOffset = expectedBox.PositionOffset;
			expectedOverlay.RotationDegrees = expectedBox.RotationDegrees;
			expectedOverlay.Scale = expectedBox.Scale;
			KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST request;
			Require(workbench.Consume_PresentationGeometryPreviewRequest(request) && request.strPatternId == patternId &&
				request.Occurrence == expectedOverlay, ("live Collider geometry lost its identity or changed unrelated fields at check " + std::to_string(++geometryCheck)).c_str());
		};
		const auto expectNoRestart = [&]() {
			KOUKU_SAYDON_COMPOSITION_PATTERN pending; KOUKU_PREVIEW_TRANSPORT command;
			std::string id, target; std::uint32_t clock = 0u; bool_t paused = false;
			Require(!workbench.Consume_PatternPreviewRequest(pending, clock, paused, target) &&
				!workbench.Consume_BundlePreviewRequest(id, clock, paused) &&
				!workbench.Consume_PreviewTransportRequest(command, clock), "live Collider edit restarted, sought or stopped its preview");
		};
		KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST noGeometry;
		auto latestGeometry = edited; latestGeometry.RotationDegrees = {12.0, 34.0, -56.0};
		for (const bool_t paused : {false, true})
		{
			state.strPatternId = patternId; state.iDurationMs = duration; state.iClockMs = 543u; state.bPaused = paused;
			workbench.Set_PreviewState(state);
			RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, edited, status), status, "first live Collider drag");
			auto detail = latestGeometry; detail.iStartMs = 999999u; detail.iDurationMs = 0u;
			detail.strBone = "unapplied_bone"; detail.strLogicOccurrenceId = "unapplied.logic";
			RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, detail, status), status, "coalesced live Collider drag");
			auto invalid = edited; invalid.Scale[0] = 0.0;
			Require(!workbench.Request_PresentationGeometryPreview(patternId, invalid, status), "invalid live Collider scale was accepted");
			invalid = edited; invalid.PositionOffset[0] = std::numeric_limits<double>::quiet_NaN();
			Require(!workbench.Request_PresentationGeometryPreview(patternId, invalid, status), "nonfinite live Collider position was accepted");
			invalid = edited; invalid.strResourceId = "missing.resource";
			Require(!workbench.Request_PresentationGeometryPreview(patternId, invalid, status), "wrong live Collider resource was accepted");
			expectGeometry(latestGeometry);
			Require(!workbench.Consume_PresentationGeometryPreviewRequest(noGeometry), "live drag was not coalesced");
			expectNoRestart();
			Require(workbench.Request_PreviewPause(), "live Collider edit lost its active preview");
			expectTransport(KOUKU_PREVIEW_TRANSPORT::PAUSE, 543u);
			workbench.Cancel_PresentationGeometryPreview(); expectGeometry(collider); expectNoRestart();
		}
		state.strPatternId = bundle.strBundleId; state.iDurationMs = bundleDuration; state.iClockMs = bundleDuration;
		state.bPaused = true; workbench.Set_PreviewState(state);
		RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, edited, status), status, "Bundle member geometry at endpoint");
		expectGeometry(edited); expectNoRestart();
		Require(workbench.Request_PreviewPause(), "member geometry lost its Bundle preview");
		expectTransport(KOUKU_PREVIEW_TRANSPORT::PAUSE, bundleDuration);
		auto laterGeometry = laterCollider; laterGeometry.Scale = {2.0, 3.0, 4.0};
		RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, laterGeometry, status), status, "switch live Collider selection");
		expectGeometry(laterGeometry); expectNoRestart();
		RequireEditorStep(workbench.Select_PatternById(patternId, status), status, "switch Bundle hierarchy to Pattern");
		Require(!workbench.Consume_PresentationGeometryPreviewRequest(noGeometry), "hierarchy Reset kept a stale geometry request");
		expectTransport(KOUKU_PREVIEW_TRANSPORT::STOP, 0u);
		state.strPatternId = patternId; state.iDurationMs = duration; state.iClockMs = 543u;
		workbench.Set_PreviewState(state);
		RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, laterGeometry, status), status, "geometry before Detail selection sync");
		expectGeometry(laterGeometry);
		RequireEditorStep(workbench.Select_PatternById(patternId, status), status, "selection synchronization preserves pending Collider geometry");
		expectNoRestart();
		Require(workbench.Is_Dirty() && !workbench.Consume_PresentationGeometryPreviewRequest(noGeometry),
			"selection discarded pending Collider geometry or queued a revert");
		RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, laterGeometry, status), status, "select later Collider before explicit Revert");
		expectGeometry(laterGeometry); workbench.Cancel_PresentationGeometryPreview(); expectGeometry(laterCollider);
		RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, edited, status), status, "select first Collider before explicit Revert");
		expectGeometry(edited); workbench.Cancel_PresentationGeometryPreview(); expectGeometry(collider);
		Require(!workbench.Is_Dirty(), "explicit Collider Revert left pending geometry");

		workbench.Set_PreviewState({});
		RequireEditorStep(workbench.Request_PatternScrub(patternId, 321u, status), status, "set inactive geometry cursor");
		expectPattern(321u, true, expected);
		RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, edited, status), status, "cold geometry preview at cursor");
		RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, latestGeometry, status), status, "update pending cold geometry preview");
		auto coldGeometryPattern = expected;
		for (auto& row : coldGeometryPattern.PresentationOccurrences)
			if (row.strOccurrenceId == latestGeometry.strOccurrenceId)
			{
				row.PositionOffset = latestGeometry.PositionOffset;
				row.RotationDegrees = latestGeometry.RotationDegrees;
				row.Scale = latestGeometry.Scale;
			}
		expectPattern(321u, true, coldGeometryPattern); expectGeometry(latestGeometry);
		workbench.Cancel_PresentationGeometryPreview(); expectGeometry(collider);
		RequireEditorStep(workbench.Request_PatternScrub(patternId, duration, status), status, "set inactive geometry endpoint");
		expectPattern(duration, true, expected);
		RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, edited, status), status, "cold endpoint geometry preview");
		workbench.Cancel_PresentationGeometryPreview();
		expectPattern(duration, true, expected); expectGeometry(collider);
		Require(!workbench.Consume_PresentationGeometryPreviewRequest(noGeometry), "cancel left stale live geometry");
		expectNoRestart();
		// V2 LEAF/GROUP geometry stays on the actor timeline and is staged for Save.
		state.strPatternId = patternId; state.bPlaying = true; state.iClockMs = 543u; state.iDurationMs = duration;
		for (const auto& effect : effects)
			for (const bool_t paused : {false, true})
			{
				state.bPaused = paused; workbench.Set_PreviewState(state);
				auto live = effect; live.PositionOffset = {1.25, -2.5, 3.75}; live.RotationDegrees = {15.0, 90.0, -10.0}; live.Scale = {2.0, 0.5, 1.5};
				RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, live, status), status, "live V2 geometry on Pattern actor");
				expectGeometry(live); expectNoRestart();
				Require(workbench.Is_Dirty() && workbench.Get_Composition() == baseline && workbench.Get_DraftGeneration() == generation &&
					ReadText(sourcePath) == bytes, "Effect geometry did not enable Save or prematurely committed authored data");
				Require(workbench.Request_PreviewPause(), "Effect geometry lost actor playback");
				expectTransport(KOUKU_PREVIEW_TRANSPORT::PAUSE, 543u);
				workbench.Cancel_PresentationGeometryPreview(); expectGeometry(effect); expectNoRestart();
				Require(!workbench.Is_Dirty(), "Effect Revert left a geometry save candidate");
			}
		state.strPatternId = bundle.strBundleId; state.iClockMs = bundleDuration; state.bPaused = true;
		workbench.Set_PreviewState(state);
		auto liveEffect = effects.front(); liveEffect.PositionOffset = {2.0, 3.0, 4.0}; liveEffect.RotationDegrees = {0.0, 35.0, 0.0}; liveEffect.Scale = {1.5, 2.5, 3.5};
		RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, liveEffect, status), status, "V2 Bundle member endpoint geometry");
		expectGeometry(liveEffect); expectNoRestart();
		workbench.Cancel_PresentationGeometryPreview(); expectGeometry(effects.front());
		workbench.Set_PreviewState({});
		RequireEditorStep(workbench.Request_PatternScrub(patternId, 321u, status), status, "V2 inactive cursor");
		expectPattern(321u, true, expected);
		RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, liveEffect, status), status, "V2 cold geometry actor Preview");
		auto coldEffectPattern = expected;
		for (auto& row : coldEffectPattern.PresentationOccurrences) if (row.strOccurrenceId == liveEffect.strOccurrenceId)
		{ row.PositionOffset = liveEffect.PositionOffset; row.RotationDegrees = liveEffect.RotationDegrees; row.Scale = liveEffect.Scale; }
		expectPattern(321u, true, coldEffectPattern); expectGeometry(liveEffect);
		workbench.Cancel_PresentationGeometryPreview(); expectGeometry(effects.front());
		Require(!workbench.Is_Dirty(), "cold V2 Revert left pending geometry");
		KOUKU_PRESENTATION_PREVIEW_REQUEST standalone;
		Require(!workbench.Consume_PresentationPreviewRequest(standalone), "transport unexpectedly queued standalone presentation");
		Require(workbench.Get_Composition() == baseline && !workbench.Is_Dirty() &&
			workbench.Get_DraftGeneration() == generation && ReadText(sourcePath) == bytes,
			"Preview transport mutated or saved authored data");

		// Collider Detail must commit the selected definition and its link together.
		workbench.Set_PreviewState({});
		Require(!workbench.Connect_ColliderLogic(patternId, collider, contact.strLogicId, status) && !status.empty(),
			"name-only Trigger silently created a Collider link");
		contact.strTriggerKind = "OBJECT_CONTACT"; contact.fTargetRadiusM = 1.0;
		Require(!workbench.Set_ColliderLogicValues(patternId, collider, contact, status) && !status.empty(),
			"contact without target cards was accepted");
		Require(workbench.Get_Composition() == baseline && !workbench.Is_Dirty() &&
			workbench.Get_DraftGeneration() == generation && ReadText(sourcePath) == bytes,
			"failed Collider Apply consumed a window ID, changed the definition, draft or saved source");
		std::string worldId, targetId;
		RequireEditorStep(workbench.Create_World("Native contact target", "world.object.instance.kouku.card", worldId, status),
			status, "create contact target definition");
		RequireEditorStep(workbench.Append_WorldBox(patternId, worldId, 0u, duration, targetId, status),
			status, "place a contact target covering both strikes");
		contact.TargetWorldOccurrenceIds = {targetId};
		const auto findCollider = [&](const std::string& id) {
			const auto& rows = EditorPattern(workbench, patternId).PresentationOccurrences;
			const auto found = std::find_if(rows.begin(), rows.end(), [&](const auto& row) { return row.strOccurrenceId == id; });
			Require(found != rows.end(), "Collider Apply lost its occurrence"); return *found;
		};
		const auto initialOrdinal = EditorPattern(workbench, patternId).iNextLogicOccurrenceOrdinal;
		RequireEditorStep(workbench.Set_ColliderLogicValues(patternId, edited, contact, status), status,
			"apply typed Trigger, Collider values and window link atomically");
		const auto firstWindow = findCollider(collider.strOccurrenceId).strLogicOccurrenceId;
		auto expectedCollider = edited; expectedCollider.strLogicOccurrenceId = firstWindow;
		Require(!firstWindow.empty() && findCollider(collider.strOccurrenceId) == expectedCollider &&
			EditorPattern(workbench, patternId).iNextLogicOccurrenceOrdinal == initialOrdinal + 1u,
			"Apply Values failed to preserve the Collider pose, timing or one new link");
		RequireEditorStep(workbench.Connect_ColliderLogic(patternId, laterCollider, contact.strLogicId, status),
			status, "connect a later strike without reusing the first strike clock");
		const auto secondWindow = findCollider(laterCollider.strOccurrenceId).strLogicOccurrenceId;
		Require(secondWindow != firstWindow && findCollider(laterCollider.strOccurrenceId).iStartMs == 800u &&
			findCollider(laterCollider.strOccurrenceId).iDurationMs == 200u &&
			EditorPattern(workbench, patternId).iNextLogicOccurrenceOrdinal == initialOrdinal + 2u,
			"later Collider connection moved to the first window or reused its link");
		RequireEditorStep(workbench.Connect_ColliderLogic(patternId, sharedCollider, contact.strLogicId, status),
			status, "reuse the exact same contact interval");
		Require(findCollider(sharedCollider.strOccurrenceId).strLogicOccurrenceId == firstWindow &&
			EditorPattern(workbench, patternId).iNextLogicOccurrenceOrdinal == initialOrdinal + 2u,
			"matching contact interval duplicated its window");
		sharedCollider.strLogicOccurrenceId = secondWindow;
		RequireEditorStep(workbench.Connect_ColliderLogic(patternId, sharedCollider, contact.strLogicId, status),
			status, "explicit Shared selection uses the selected later window");
		Require(findCollider(sharedCollider.strOccurrenceId).strLogicOccurrenceId == secondWindow &&
			findCollider(sharedCollider.strOccurrenceId).iStartMs == 800u &&
			findCollider(sharedCollider.strOccurrenceId).iDurationMs == 200u,
			"explicit Shared selection failed to preserve the selected window clock");
		auto resizedTrigger = findCollider(laterCollider.strOccurrenceId);
		resizedTrigger.iStartMs = 700u; resizedTrigger.iDurationMs = 300u;
		resizedTrigger.Scale = {2.0, 3.0, 2.0};
		auto expectedShared = findCollider(sharedCollider.strOccurrenceId);
		expectedShared.iStartMs = resizedTrigger.iStartMs; expectedShared.iDurationMs = resizedTrigger.iDurationMs;
		RequireEditorStep(workbench.Set_ColliderLogicValues(patternId, resizedTrigger, contact, status), status,
			"resize an already linked Trigger from Collider Apply Values");
		const auto& resizedWindows = EditorPattern(workbench, patternId).LogicOccurrences;
		const auto resizedWindow = std::find_if(resizedWindows.begin(), resizedWindows.end(),
			[&](const auto& row) { return row.strOccurrenceId == secondWindow; });
		Require(resizedWindow != resizedWindows.end() && resizedWindow->iStartMs == 700u && resizedWindow->iDurationMs == 300u &&
			findCollider(laterCollider.strOccurrenceId) == resizedTrigger && findCollider(sharedCollider.strOccurrenceId) == expectedShared &&
			findCollider(collider.strOccurrenceId) == expectedCollider &&
			EditorPattern(workbench, patternId).iNextLogicOccurrenceOrdinal == initialOrdinal + 2u,
			"Trigger Apply Values discarded Collider timing/size, failed to synchronize its shared window, or moved another strike");
		const auto beforeRejectedResize = workbench.Get_Composition();
		auto invalidResize = resizedTrigger; invalidResize.iDurationMs = duration + 1u;
		Require(!workbench.Set_ColliderLogicValues(patternId, invalidResize, contact, status) &&
			workbench.Get_Composition() == beforeRejectedResize,
			"invalid Trigger resize changed the linked window or another Collider");

		const auto stageId = EditorPattern(workbench, patternId).Stages.front().strStageId;
		Require(!EditorPattern(workbench, patternId).Stages.front().bRetargetOnEnter,
			"absent retargetOnEnter did not default to false");
		RequireEditorStep(workbench.Set_StageRetargetOnEnter(patternId, stageId, true, status), status,
			"enable player retarget at Stage entry");
		Require(EditorPattern(workbench, patternId).Stages.front().bRetargetOnEnter,
			"stage retarget edit was ignored by the authoring document");
		RequireEditorRoundtrip(workbench);
		Require(EditorPattern(workbench, patternId).Stages.front().bRetargetOnEnter &&
			findCollider(collider.strOccurrenceId) == expectedCollider &&
			findCollider(laterCollider.strOccurrenceId) == resizedTrigger && findCollider(sharedCollider.strOccurrenceId) == expectedShared,
			"Save/Reload dropped Stage retarget or the Collider definition/window timing and geometry");
		for (const auto& previous : baseline.Patterns)
			if (previous.strPatternId != patternId)
				Require(EditorPattern(workbench, previous.strPatternId) == previous, "Collider Apply changed an unrelated Pattern");

		// Malformed optional flags are isolated with their original JSON; never silently false.
		auto schemaFixture = workbench.Get_Composition();
		for (auto& pattern : schemaFixture.Patterns)
			for (auto& stage : pattern.Stages) stage.bRetargetOnEnter = pattern.strPatternId == patternId;
		const auto schemaText = CKoukuSaydonCompositionDocument::Serialize(schemaFixture);
		const auto assertQuarantined = [&](const std::string& invalidText, const char* message) {
			KOUKU_SAYDON_COMPOSITION_DOCUMENT parsed;
			RequireEditorStep(CKoukuSaydonCompositionDocument::Parse_Text(invalidText, parsed, status), status, message);
			const auto found = std::find_if(parsed.Patterns.begin(), parsed.Patterns.end(),
				[&](const auto& pattern) { return pattern.strPatternId == patternId; });
			Require(found != parsed.Patterns.end() && !found->strLoadError.empty() && found->Stages.empty() &&
				!found->strPreservedJson.empty(), message);
			for (const auto& previous : schemaFixture.Patterns)
				if (previous.strPatternId != patternId) {
					const auto intact = std::find_if(parsed.Patterns.begin(), parsed.Patterns.end(),
						[&](const auto& pattern) { return pattern.strPatternId == previous.strPatternId; });
					Require(intact != parsed.Patterns.end() && *intact == previous, "invalid Stage flag damaged another Pattern");
				}
		};
		for (const auto* invalid : {"1", "\"true\"", "null"}) {
			auto invalidText = schemaText;
			Require(ReplaceOnce(invalidText, "\"retargetOnEnter\": true", std::string("\"retargetOnEnter\": ") + invalid),
				"retarget field missing from serialized Stage");
			assertQuarantined(invalidText, "non-boolean Stage retarget silently fell back to false");
		}
		for (auto& pattern : schemaFixture.Patterns) if (pattern.strPatternId == patternId) {
			pattern.bResetBossToSpawn = false; pattern.ResetBossYawDegrees.reset();
			pattern.BossMotion = KOUKU_SAYDON_BOSS_MOTION{}; pattern.BossMotion->iEndMs = duration;
		}
		assertQuarantined(CKoukuSaydonCompositionDocument::Serialize(schemaFixture),
			"retarget plus fixed-yaw Boss Motion admitted competing yaw writers");
		RequireEditorStep(workbench.Set_StageRetargetOnEnter(patternId, stageId, false, status), status,
			"disable Stage entry retarget");
		RequireEditorRoundtrip(workbench);
		Require(!EditorPattern(workbench, patternId).Stages.front().bRetargetOnEnter &&
			findCollider(collider.strOccurrenceId).strLogicOccurrenceId == firstWindow,
			"false retarget roundtrip dropped a saved Collider connection");
		// Collider P/R/S survives selection and another committed edit, then Save without Apply.
		const auto beforeColliderSave = workbench.Get_Composition();
		const auto beforeColliderBytes = ReadText(sourcePath);
		const auto beforeColliderGeneration = workbench.Get_DraftGeneration();
		state.strPatternId = patternId; state.bPlaying = true; state.bPaused = true; state.iClockMs = 543u;
		workbench.Set_PreviewState(state);
		auto firstColliderEdit = findCollider(collider.strOccurrenceId);
		firstColliderEdit.PositionOffset = {1.25, -2.5, 3.75};
		firstColliderEdit.RotationDegrees = {0.0, 45.0, 0.0}; firstColliderEdit.Scale = {2.0, 3.0, 2.0};
		auto secondColliderEdit = findCollider(laterCollider.strOccurrenceId);
		secondColliderEdit.PositionOffset = {-4.0, 5.0, 6.0};
		secondColliderEdit.RotationDegrees = {0.0, -30.0, 0.0}; secondColliderEdit.Scale = {4.0, 5.0, 4.0};
		auto colliderDetail = firstColliderEdit; colliderDetail.iStartMs = 999999u; colliderDetail.iDurationMs = 0u;
		colliderDetail.strBone = "unapplied_bone"; colliderDetail.strLogicOccurrenceId = "unapplied.logic";
		RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, colliderDetail, status), status,
			"stage Collider geometry without applying unrelated Detail fields");
		Require(workbench.Is_Dirty() && workbench.Get_Composition() == beforeColliderSave &&
			workbench.Get_DraftGeneration() == beforeColliderGeneration && ReadText(sourcePath) == beforeColliderBytes,
			"Collider geometry failed to enable Save or prematurely changed applied source");
		RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, secondColliderEdit, status), status,
			"stage a second Collider geometry without Apply");
		RequireEditorStep(workbench.Select_BundleById(bundle.strBundleId, status), status, "switch hierarchy with pending Collider geometry");
		RequireEditorStep(workbench.Select_PatternById(patternId, status), status, "return to edited Collider Pattern");
		RequireEditorStep(workbench.Set_StageRetargetOnEnter(patternId, stageId, true, status), status,
			"commit a separate Stage edit while both Collider geometries are pending");
		Require(ReadText(sourcePath) == beforeColliderBytes, "a separate Stage edit wrote pending Collider geometry to disk");
		auto expectedColliderSave = workbench.Get_Composition(); ++expectedColliderSave.iRevision;
		for (auto& pattern : expectedColliderSave.Patterns) if (pattern.strPatternId == patternId)
			for (auto& row : pattern.PresentationOccurrences)
				for (const auto* value : {&firstColliderEdit, &secondColliderEdit})
					if (row.strOccurrenceId == value->strOccurrenceId)
					{ row.PositionOffset = value->PositionOffset; row.RotationDegrees = value->RotationDegrees; row.Scale = value->Scale; }
		RequireEditorStep(workbench.Save(status), status, "Save all Collider geometry without Apply after selection and separate commit");
		Require(!workbench.Is_Dirty() && workbench.Get_Composition() == expectedColliderSave,
			"Collider Save lost P/R/S or changed unapplied timing, Bone or Logic fields");
		CKoukuSaydonActionWorkbench reopenedCollider;
		RequireEditorStep(reopenedCollider.Reload(status), status, "open saved Collider geometry in a new Workbench instance");
		Require(reopenedCollider.Get_Composition() == expectedColliderSave,
			"new Workbench lost saved Collider geometry or the separately committed Stage edit");

		const auto colliderSavedBytes = ReadText(sourcePath);
		const auto colliderSavedGeneration = workbench.Get_DraftGeneration();
		workbench.Set_PreviewState(state);
		auto invalidCollider = firstColliderEdit; invalidCollider.Scale[0] = 0.0;
		Require(!workbench.Request_PresentationGeometryPreview(patternId, invalidCollider, status),
			"zero Collider scale was admitted for preview");
		Require(!workbench.Save(status) && !status.empty() && workbench.Is_Dirty() &&
			workbench.Get_Composition() == expectedColliderSave && workbench.Get_DraftGeneration() == colliderSavedGeneration &&
			ReadText(sourcePath) == colliderSavedBytes, "invalid Collider Save changed applied draft or last saved source");
		CKoukuSaydonActionWorkbench reopenedAfterRejectedCollider;
		RequireEditorStep(reopenedAfterRejectedCollider.Reload(status), status, "reopen last saved source after invalid Collider Save");
		Require(reopenedAfterRejectedCollider.Get_Composition() == expectedColliderSave,
			"invalid Collider Save corrupted the source seen by a new Workbench");
		RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, firstColliderEdit, status), status,
			"correct invalid Collider geometry to the saved values");
		RequireEditorStep(workbench.Save(status), status, "Save after correcting rejected Collider geometry");
		Require(!workbench.Is_Dirty() && ReadText(sourcePath) == colliderSavedBytes,
			"correcting Collider geometry to saved values unnecessarily rewrote the source");
		while (workbench.Consume_PresentationGeometryPreviewRequest(noGeometry)) {}

		// First connection reuses a placed name-only Trigger even when its interval differs.
		{
			const auto triggerDataRoot = scratchRoot / "FirstTrigger/Data";
			const auto triggerSourcePath = triggerDataRoot / relativeSource;
			for (const char* profile : { "MN_RPCT_05", "MN_RPCT_06", "MN_RPCT_07", "MN_RPCZ_00" })
			{
				const auto relative = std::filesystem::path("Animation/Reference/KoukuSaydon") /
					(std::string(profile) + ".actionreference.json");
				Require(CopyFixture(dataRoot / relative, triggerDataRoot / relative), "could not copy first Trigger fixture reference");
			}
			auto triggerSource = expectedColliderSave;
			KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION firstContact;
			firstContact.strLogicId = "kakulsaydon.g1.logic." + std::to_string(triggerSource.iNextLogicOrdinal++);
			firstContact.strDisplayName = "Native first Trigger connection"; firstContact.strLogicType = "TRIGGER";
			triggerSource.Logics.push_back(firstContact);
			KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION result;
			result.strLogicId = "kakulsaydon.g1.logic." + std::to_string(triggerSource.iNextLogicOrdinal++);
			result.strDisplayName = "Native preserved contact result"; result.strLogicType = "RESULT";
			result.strOutcomeKind = "MAX_HP_PERCENT_DAMAGE"; result.iPercent = 10u;
			triggerSource.Logics.push_back(result);
			KOUKU_SAYDON_COMPOSITION_LOGIC_OCCURRENCE placed;
			placed.strLogicId = firstContact.strLogicId; placed.iStartMs = 3992u; placed.iDurationMs = 301u;
			auto firstBox = collider; firstBox.iStartMs = 3992u; firstBox.iDurationMs = 316u;
			firstBox.PositionOffset = {1.0, 2.0, 3.0}; firstBox.Scale = {2.0, 3.0, 2.0};
			for (auto& pattern : triggerSource.Patterns) if (pattern.strPatternId == patternId)
			{
				pattern.Stages.front().iDurationMs = (std::max)(pattern.Stages.front().iDurationMs, 5000u);
				pattern.LogicOccurrences.clear(); pattern.PresentationOccurrences = {firstBox};
				pattern.WorldOccurrences.clear(); pattern.SceneProfileOccurrences.clear(); pattern.SummonOccurrences.clear();
				placed.strOccurrenceId = patternId + ".logic." + std::to_string(pattern.iNextLogicOccurrenceOrdinal++);
				pattern.LogicOccurrences.push_back(placed);
			}
			Require(WriteText(triggerSourcePath, CKoukuSaydonCompositionDocument::Serialize(triggerSource)),
				"could not write first Trigger connection fixture");
			SCOPED_ENVIRONMENT_VARIABLE triggerEnvironment(L"LOSTARK_PROJECT_DATA_ROOT");
			Require(triggerEnvironment.Set(triggerDataRoot), "could not select first Trigger connection fixture");
			CKoukuSaydonActionWorkbench firstTriggerWorkbench;
			RequireEditorStep(firstTriggerWorkbench.Reload(status), status, "load name-only placed Trigger with unlinked Collider");
			const auto triggerBaseline = firstTriggerWorkbench.Get_Composition();
			const auto triggerBytes = ReadText(triggerSourcePath);
			const auto nextWindowOrdinal = EditorPattern(firstTriggerWorkbench, patternId).iNextLogicOccurrenceOrdinal;
			firstContact.strTriggerKind = "ENTER_AREA"; firstContact.bRearmOnExit = true;
			auto invalidFirstBox = firstBox; invalidFirstBox.iDurationMs = 600001u;
			Require(!firstTriggerWorkbench.Set_ColliderLogicValues(patternId, invalidFirstBox, firstContact, status) &&
				firstTriggerWorkbench.Get_Composition() == triggerBaseline && ReadText(triggerSourcePath) == triggerBytes,
				"rejected first Trigger Apply changed its name-only definition, unconnected window or source");
			RequireEditorStep(firstTriggerWorkbench.Set_ColliderLogicValues(patternId, firstBox, firstContact, status), status,
				"apply selected Trigger kind and connect its placed window using the Collider interval");
			auto expectedFirstBox = firstBox; expectedFirstBox.strLogicOccurrenceId = placed.strOccurrenceId;
			const auto& connected = EditorPattern(firstTriggerWorkbench, patternId);
			Require(connected.LogicOccurrences.size() == 1u && connected.iNextLogicOccurrenceOrdinal == nextWindowOrdinal &&
				connected.LogicOccurrences.front().strOccurrenceId == placed.strOccurrenceId &&
				connected.LogicOccurrences.front().iStartMs == 3992u && connected.LogicOccurrences.front().iDurationMs == 316u &&
				connected.PresentationOccurrences == std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE>{expectedFirstBox},
				"first Trigger connection duplicated the placed window or lost Collider time/geometry");
			result.fPushRangeM = 2.0; result.iPushMs = 242u;
			RequireEditorStep(firstTriggerWorkbench.Set_LogicDefinitionValues(result.strLogicId, result, status), status,
				"set contact damage with knockback");
			RequireEditorStep(firstTriggerWorkbench.Set_LogicBoxOutcomes(patternId, placed.strOccurrenceId,
				KOUKU_SAYDON_OUTCOME_SLOT::SUCCESS, {result.strLogicId}, status), status, "connect first Trigger Success result");
			auto resizedFirstBox = expectedFirstBox; resizedFirstBox.iStartMs = 3800u; resizedFirstBox.iDurationMs = 600u;
			RequireEditorStep(firstTriggerWorkbench.Set_ColliderLogicValues(patternId, resizedFirstBox, firstContact, status), status,
				"resize first connected Trigger while preserving its Result");
			RequireEditorRoundtrip(firstTriggerWorkbench);
			const auto& reopened = EditorPattern(firstTriggerWorkbench, patternId);
			Require(reopened.LogicOccurrences.size() == 1u && reopened.iNextLogicOccurrenceOrdinal == nextWindowOrdinal &&
				reopened.LogicOccurrences.front().strOccurrenceId == placed.strOccurrenceId &&
				reopened.LogicOccurrences.front().iStartMs == resizedFirstBox.iStartMs &&
				reopened.LogicOccurrences.front().iDurationMs == resizedFirstBox.iDurationMs &&
				reopened.LogicOccurrences.front().OnSuccessLogicIds == std::vector<std::string>{result.strLogicId} &&
				reopened.PresentationOccurrences == std::vector<KOUKU_SAYDON_COMPOSITION_PRESENTATION_OCCURRENCE>{resizedFirstBox},
				"first Trigger Save/Reload lost its link, interval, Collider pose or Success result");
			const auto savedTrigger = firstTriggerWorkbench.Get_Composition();
			const auto savedTriggerBytes = ReadText(triggerSourcePath);
			const auto savedTriggerGeneration = firstTriggerWorkbench.Get_DraftGeneration();
			const auto findSavedLogic = [&](const std::string& id) {
				const auto found = std::find_if(savedTrigger.Logics.begin(), savedTrigger.Logics.end(),
					[&](const auto& logic) { return logic.strLogicId == id; });
				Require(found != savedTrigger.Logics.end(), "first Trigger Save dropped a Logic definition");
				return *found;
			};
			Require(findSavedLogic(firstContact.strLogicId) == firstContact && findSavedLogic(result.strLogicId) == result,
				"Save/Reload lost ENTER_AREA rearmOnExit or 2 m / 242 ms damage knockback");
			std::vector<KOUKU_SAYDON_COMPOSITION_LOGIC_DEFINITION> invalidResults(8u, result);
			invalidResults[0].fPushRangeM = 0.0;
			invalidResults[1].iPushMs = 0u;
			invalidResults[2].fPushRangeM = std::numeric_limits<double>::quiet_NaN();
			invalidResults[3].fPushRangeM = std::numeric_limits<double>::infinity();
			invalidResults[4].fPushRangeM = 20.01;
			invalidResults[5].iPushMs = 600001u;
			invalidResults[6].strOutcomeKind = "MADNESS_GAUGE_ADD_PERCENT";
			invalidResults[7].bRearmOnExit = true;
			for (const auto& invalidResult : invalidResults)
				Require(!firstTriggerWorkbench.Set_LogicDefinitionValues(result.strLogicId, invalidResult, status) && !status.empty() &&
					firstTriggerWorkbench.Get_Composition() == savedTrigger && !firstTriggerWorkbench.Is_Dirty() &&
					firstTriggerWorkbench.Get_DraftGeneration() == savedTriggerGeneration && ReadText(triggerSourcePath) == savedTriggerBytes,
					"invalid knockback fields changed the damage Result, linked Trigger or saved source");
			auto invalidContact = firstContact; invalidContact.strTriggerKind = "HUD_ENTER"; invalidContact.strHudMode = "NONE";
			Require(!firstTriggerWorkbench.Set_LogicDefinitionValues(firstContact.strLogicId, invalidContact, status) &&
				firstTriggerWorkbench.Get_Composition() == savedTrigger && !firstTriggerWorkbench.Is_Dirty(),
				"non-contact Trigger accepted rearmOnExit or changed the saved definition");
			const auto rejectFieldText = [&](const std::string& logicId, const std::string& from, const std::string& to) {
				auto invalidText = CKoukuSaydonCompositionDocument::Serialize(savedTrigger);
				const auto definitionStart = invalidText.find("\"logicId\": \"" + logicId + "\"");
				Require(definitionStart != std::string::npos, "serialized Trigger fixture lost its definition");
				const auto position = invalidText.find(from, definitionStart);
				Require(position != std::string::npos && position < invalidText.find("\n    }", definitionStart),
					"serialized Trigger fixture lost a knockback/rearm field");
				invalidText.replace(position, from.size(), to);
				auto parsed = savedTrigger;
				Require(!CKoukuSaydonCompositionDocument::Parse_Text(invalidText, parsed, status) && !status.empty() && parsed == savedTrigger,
					"malformed knockback/rearm JSON was accepted or replaced the previous parsed document");
			};
			rejectFieldText(result.strLogicId, ",\n      \"pushRangeM\": 2", "");
			rejectFieldText(result.strLogicId, ",\n      \"pushMs\": 242", "");
			rejectFieldText(result.strLogicId, "\"pushRangeM\": 2", "\"pushRangeM\": 1e309");
			rejectFieldText(result.strLogicId, "\"outcomeKind\": \"MAX_HP_PERCENT_DAMAGE\"", "\"outcomeKind\": \"MADNESS_GAUGE_ADD_PERCENT\"");
			rejectFieldText(firstContact.strLogicId, "\"rearmOnExit\": true", "\"rearmOnExit\": 1");
			Require(firstTriggerWorkbench.Get_Composition() == savedTrigger && ReadText(triggerSourcePath) == savedTriggerBytes,
				"knockback/rearm rejection checks changed the applied draft or saved source");
			auto continuousContact = firstContact; continuousContact.bRearmOnExit = false; continuousContact.bRepeatAfterKnockback = true;
			RequireEditorStep(firstTriggerWorkbench.Set_LogicDefinitionValues(firstContact.strLogicId, continuousContact, status), status,
				"select repeated contact after knockback finishes");
			RequireEditorRoundtrip(firstTriggerWorkbench);
			const auto& continuousSaved = firstTriggerWorkbench.Get_Composition();
			const auto continuousFound = std::find_if(continuousSaved.Logics.begin(), continuousSaved.Logics.end(),
				[&](const auto& row) { return row.strLogicId == continuousContact.strLogicId; });
			Require(continuousFound != continuousSaved.Logics.end() && *continuousFound == continuousContact,
				"Save/Reload lost repeat after knockback mode");
			continuousContact.bRearmOnExit = true;
			Require(!firstTriggerWorkbench.Set_LogicDefinitionValues(continuousContact.strLogicId, continuousContact, status),
				"mutually exclusive contact modes were accepted");
		}
		Require(ReadText(sourcePath) == colliderSavedBytes, "first Trigger fixture changed the separate Collider source");

		// Linked Sector geometry supports independent X/Z scales in preview, Save and gameplay.
		{
			const auto sectorDataRoot = scratchRoot / "Sector/Data";
			const auto sectorSourcePath = sectorDataRoot / relativeSource;
			for (const char* profile : { "MN_RPCT_05", "MN_RPCT_06", "MN_RPCT_07", "MN_RPCZ_00" })
			{
				const auto relative = std::filesystem::path("Animation/Reference/KoukuSaydon") /
					(std::string(profile) + ".actionreference.json");
				Require(CopyFixture(dataRoot / relative, sectorDataRoot / relative), "could not copy Sector fixture action reference");
			}
			auto sectorSource = expectedColliderSave;
			const auto originalResource = std::find_if(sectorSource.PresentationResources.begin(), sectorSource.PresentationResources.end(),
				[&](const auto& row) { return row.strResourceId == firstColliderEdit.strResourceId; });
			Require(originalResource != sectorSource.PresentationResources.end(), "Sector fixture lost its source Collider resource");
			auto sectorResource = *originalResource;
			sectorResource.strResourceId = "kakulsaydon.g1.presentation." + std::to_string(sectorSource.iNextPresentationResourceOrdinal++);
			sectorResource.strDisplayName = "Native linked Sector save"; sectorResource.strShape = "SECTOR";
			sectorSource.PresentationResources.push_back(sectorResource);
			auto sectorBox = firstColliderEdit; sectorBox.strResourceId = sectorResource.strResourceId;
			for (auto& pattern : sectorSource.Patterns) if (pattern.strPatternId == patternId)
				for (auto& row : pattern.PresentationOccurrences) if (row.strOccurrenceId == sectorBox.strOccurrenceId) row = sectorBox;
			Require(!sectorBox.strLogicOccurrenceId.empty(), "Sector save fixture needs an applied Logic link");
			Require(WriteText(sectorSourcePath, CKoukuSaydonCompositionDocument::Serialize(sectorSource)), "could not write isolated Sector save fixture");
			SCOPED_ENVIRONMENT_VARIABLE sectorEnvironment(L"LOSTARK_PROJECT_DATA_ROOT");
			Require(sectorEnvironment.Set(sectorDataRoot), "could not select isolated Sector save fixture");
			CKoukuSaydonActionWorkbench sectorWorkbench;
			RequireEditorStep(sectorWorkbench.Reload(status), status, "load linked Sector save fixture");
			const auto sectorBaseline = sectorWorkbench.Get_Composition();
			const auto sectorBytes = ReadText(sectorSourcePath);
			sectorWorkbench.Set_PreviewState(state);
			auto ellipse = sectorBox; ellipse.Scale = {3.0, 1.0, 1.0};
			ellipse.strLogicOccurrenceId.clear(); // Detail preview changes geometry only.
			RequireEditorStep(sectorWorkbench.Request_PresentationGeometryPreview(patternId, ellipse, status), status,
				"preview independent Sector X/Z scale while retaining its applied link");
			KOUKU_PRESENTATION_GEOMETRY_PREVIEW_REQUEST sectorPreview;
			Require(sectorWorkbench.Consume_PresentationGeometryPreviewRequest(sectorPreview), "Sector ellipse did not reach geometry preview");
			Require(sectorWorkbench.Get_Composition() == sectorBaseline && ReadText(sectorSourcePath) == sectorBytes,
				"Sector geometry preview prematurely replaced the applied source");
			RequireEditorRoundtrip(sectorWorkbench);
			const auto expectedSector = sectorWorkbench.Get_Composition();
			const auto& savedPattern = EditorPattern(sectorWorkbench, patternId);
			const auto savedBox = std::find_if(savedPattern.PresentationOccurrences.begin(), savedPattern.PresentationOccurrences.end(),
				[&](const auto& row) { return row.strOccurrenceId == ellipse.strOccurrenceId; });
			Require(savedBox != savedPattern.PresentationOccurrences.end() && savedBox->Scale == ellipse.Scale &&
				savedBox->strLogicOccurrenceId == sectorBox.strLogicOccurrenceId,
				"Sector Save/Reload lost independent X/Z scale or its applied Logic link");
			auto appliedEllipse = *savedBox;
			RequireEditorStep(sectorWorkbench.Set_PresentationBox(patternId, appliedEllipse, status), status, "apply elliptical Collider geometry");
			RequireEditorStep(sectorWorkbench.Set_ColliderLogicValues(patternId, appliedEllipse, contact, status), status, "apply elliptical Collider Logic");
			RequireEditorStep(sectorWorkbench.Set_ColliderTriggerDamage(patternId, appliedEllipse, 25u, status), status, "apply elliptical damage Collider");
			RequireEditorRoundtrip(sectorWorkbench);

			// A newly connected ellipse and its reverse use the same authoring path.
			for (const bool reverse : {false, true})
			{
				auto unlinkedSource = expectedSector;
				auto unlinkedEllipse = ellipse;
				for (auto& resource : unlinkedSource.PresentationResources)
					if (resource.strResourceId == sectorResource.strResourceId)
					{ resource.strShape = reverse ? "REVERSE_SECTOR" : "SECTOR"; resource.fHalfAngleDegrees = reverse ? 0.0 : 30.0; }
				for (auto& pattern : unlinkedSource.Patterns) if (pattern.strPatternId == patternId)
					for (auto& row : pattern.PresentationOccurrences) if (row.strOccurrenceId == unlinkedEllipse.strOccurrenceId) row = unlinkedEllipse;
				Require(WriteText(sectorSourcePath, CKoukuSaydonCompositionDocument::Serialize(unlinkedSource)),
					"could not write unlinked elliptic Sector fixture");
				CKoukuSaydonActionWorkbench unlinkedSector;
				RequireEditorStep(unlinkedSector.Reload(status), status, "load regular/reverse elliptic Sector");
				RequireEditorStep(unlinkedSector.Connect_ColliderLogic(patternId, unlinkedEllipse, contact.strLogicId, status), status,
					"connect first Logic window to elliptic Sector");
				RequireEditorRoundtrip(unlinkedSector);
				const auto& connected = EditorPattern(unlinkedSector, patternId);
				const auto box = std::find_if(connected.PresentationOccurrences.begin(), connected.PresentationOccurrences.end(),
					[&](const auto& row) { return row.strOccurrenceId == unlinkedEllipse.strOccurrenceId; });
				Require(box != connected.PresentationOccurrences.end() && box->Scale == ellipse.Scale && !box->strLogicOccurrenceId.empty(),
					"regular/reverse Sector lost elliptical scale during connection");
				const auto boxCopy = *box;
				RequireEditorStep(unlinkedSector.Set_ColliderTriggerDamage(patternId, boxCopy, 25u, status), status,
					"connect damage directly to regular/reverse elliptic Sector");
				RequireEditorRoundtrip(unlinkedSector);
				const auto lastGood = unlinkedSector.Get_Composition();
				auto invalid = boxCopy; invalid.Scale[0] = 0.0;
				Require(!unlinkedSector.Set_ColliderTriggerDamage(patternId, invalid, 25u, status) &&
					unlinkedSector.Get_Composition() == lastGood, "invalid ellipse size replaced the last-good damage draft");
			}
		}
		Require(ReadText(sourcePath) == colliderSavedBytes, "Sector fixture changed the separate Collider source");

		// Save collects geometry from every edited Effect box, without a separate Apply.
		const auto beforeEffectSave = workbench.Get_Composition();
		const auto beforeEffectBytes = ReadText(sourcePath);
		const auto beforeEffectGeneration = workbench.Get_DraftGeneration();
		state.strPatternId = patternId; state.bPlaying = true; state.bPaused = true; state.iClockMs = 543u;
		workbench.Set_PreviewState(state);
		auto firstEffectEdit = effects[0]; firstEffectEdit.PositionOffset = {1.1, 2.2, 3.3};
		firstEffectEdit.RotationDegrees = {10.0, 75.0, -20.0}; firstEffectEdit.Scale = {0.5, 1.5, 2.5};
		auto secondEffectEdit = effects[1]; secondEffectEdit.PositionOffset = {-3.5, 1.25, 7.0};
		secondEffectEdit.RotationDegrees = {25.0, -45.0, 5.0}; secondEffectEdit.Scale = {2.0, 3.0, 4.0};
		auto unrelatedDetail = firstEffectEdit; unrelatedDetail.iStartMs = 999999u; unrelatedDetail.iDurationMs = 0u;
		unrelatedDetail.strBone = "unapplied_bone"; unrelatedDetail.strLogicOccurrenceId = "unapplied.logic";
		RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, unrelatedDetail, status), status, "stage first Effect geometry only");
		expectGeometry(firstEffectEdit);
		RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, secondEffectEdit, status), status, "stage second Effect geometry");
		expectGeometry(secondEffectEdit);
		RequireEditorStep(workbench.Select_PatternById(patternId, status), status, "preserve all Effect geometry across selection sync");
		Require(!workbench.Consume_PresentationGeometryPreviewRequest(noGeometry), "Effect selection reverted a staged box");
		Require(workbench.Is_Dirty() && workbench.Get_Composition() == beforeEffectSave &&
			workbench.Get_DraftGeneration() == beforeEffectGeneration && ReadText(sourcePath) == beforeEffectBytes,
			"multi-box Effect staging prematurely applied geometry or lost Save enablement");
		RequireEditorStep(workbench.Save(status), status, "Save all Effect geometry without Apply");
		auto expectedEffectSave = beforeEffectSave; ++expectedEffectSave.iRevision;
		for (auto& pattern : expectedEffectSave.Patterns) if (pattern.strPatternId == patternId)
			for (auto& row : pattern.PresentationOccurrences)
				for (const auto* value : {&firstEffectEdit, &secondEffectEdit})
					if (row.strOccurrenceId == value->strOccurrenceId)
					{ row.PositionOffset = value->PositionOffset; row.RotationDegrees = value->RotationDegrees; row.Scale = value->Scale; }
		Require(!workbench.Is_Dirty() && workbench.Get_Composition() == expectedEffectSave,
			"Effect Save lost a box or applied unrelated timing, Bone or Logic fields");
		RequireEditorStep(workbench.Reload(status), status, "reload saved Effect geometry");
		Require(workbench.Get_Composition() == expectedEffectSave, "Effect geometry Save/Reload changed its P/R/S");

		// Invalid geometry and CAS conflicts leave the applied draft and source intact.
		const auto effectSavedBytes = ReadText(sourcePath);
		const auto effectSavedGeneration = workbench.Get_DraftGeneration();
		auto corrected = firstEffectEdit; corrected.Scale = {4.0, 5.0, 6.0};
		workbench.Set_PreviewState(state);
		RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, corrected, status), status, "valid Effect preview before rejected edit");
		auto invalidEffect = corrected; invalidEffect.Scale[0] = std::numeric_limits<double>::quiet_NaN();
		Require(!workbench.Request_PresentationGeometryPreview(patternId, invalidEffect, status), "invalid Effect geometry was previewed");
		expectGeometry(corrected);
		Require(!workbench.Save(status) && !status.empty() && workbench.Is_Dirty() &&
			workbench.Get_Composition() == expectedEffectSave && workbench.Get_DraftGeneration() == effectSavedGeneration &&
			ReadText(sourcePath) == effectSavedBytes, "invalid Effect Save changed applied draft or source");
		Require(!workbench.Consume_PresentationGeometryPreviewRequest(noGeometry), "invalid Effect Save replaced last valid preview");
		RequireEditorStep(workbench.Request_PresentationGeometryPreview(patternId, corrected, status), status, "correct rejected Effect geometry");
		expectGeometry(corrected);
		const auto externalBytes = effectSavedBytes + "\n";
		Require(WriteText(sourcePath, externalBytes), "could not simulate concurrent Effect source save");
		Require(!workbench.Save(status) && !status.empty() && workbench.Is_Dirty() &&
			workbench.Get_Composition() == expectedEffectSave && workbench.Get_DraftGeneration() == effectSavedGeneration &&
			ReadText(sourcePath) == externalBytes, "CAS rejected Effect Save overwrote source or committed pending draft");

		// Save preserves editable source; only Publish All launches the hidden child and refreshes after success.
		CKoukuSaydonActionWorkbench promoter;
		Require(!promoter.Publish_AllPatterns(status) && !status.empty() && !promoter.Is_PublishRunning(),
			"Publish All accepted an unloaded Composition");
		RequireEditorStep(promoter.Reload(status), status, "reload source for separate Save and Publish All contract");
		const auto& promotionBoxes = EditorPattern(promoter, patternId).PresentationOccurrences;
		const auto promotionCollider = *std::find_if(promotionBoxes.begin(), promotionBoxes.end(),
			[&](const auto& box) { return box.strOccurrenceId == collider.strOccurrenceId; });
		auto invalidRotation = promotionCollider; invalidRotation.RotationDegrees[2] = -0.25;
		RequireEditorStep(promoter.Set_PresentationBox(patternId, invalidRotation, status), status, "stage unsupported gameplay Collider roll");
		RequireEditorStep(promoter.Set_PatternAuthoringStatus(patternId, "PRODUCT", status), status, "retain PRODUCT metadata while saving editable roll");
		auto savedRoll = promoter.Get_Composition(); ++savedRoll.iRevision;
		RequireEditorStep(promoter.Save(status), status, "Save preserves roll for later runtime eligibility assessment");
		Require(promoter.Get_Composition() == savedRoll && !promoter.Is_Dirty() &&
			!promoter.Is_PublishRunning() && !promoter.Consume_ProductInventoryRefreshRequest(),
			"source-only Save rejected editable roll, launched publish, or changed authored metadata");
		RequireEditorStep(promoter.Reload(status), status, "reload saved Collider roll");
		Require(promoter.Get_Composition() == savedRoll, "Save/Reload lost the authored Collider roll");
		RequireEditorStep(promoter.Set_PresentationBox(patternId, promotionCollider, status), status, "restore supported Collider geometry");
		RequireEditorStep(promoter.Set_PatternAuthoringStatus(patternId, "DRAFT", status), status, "leave every Pattern as DRAFT for full-tree publish");
		const auto publishScript = scratchRoot / "Tools/Build/Invoke-BuildDomainOwner.ps1";
		const std::string successScript = "param([string]$Owner,[uint32]$ExpectedKoukuSaydonSourceRevision)\n"
			"Write-Output ('Owner=' + $Owner + '; Revision=' + $ExpectedKoukuSaydonSourceRevision)\nexit 0\n";
		Require(WriteText(publishScript, successScript), "could not write isolated publisher process fixture");
		Require(!promoter.Consume_ProductInventoryRefreshRequest(), "fresh Workbench requested an inventory refresh");
		const auto unsavedPublishDraft = promoter.Get_Composition();
		const auto unsavedPublishBytes = ReadText(sourcePath);
		Require(promoter.Is_Dirty() && !promoter.Publish_AllPatterns(status) && !status.empty() &&
			!promoter.Is_PublishRunning() && promoter.Get_Composition() == unsavedPublishDraft &&
			ReadText(sourcePath) == unsavedPublishBytes && !promoter.Consume_ProductInventoryRefreshRequest(),
			"Publish All accepted unsaved edits or changed the source while rejecting them");
		RequireEditorStep(promoter.Save(status), status, "Save source without starting the available publisher");
		const auto savedPublishDraft = promoter.Get_Composition();
		const auto savedPublishBytes = ReadText(sourcePath);
		Require(!promoter.Is_PublishRunning() && !promoter.Is_Dirty() && savedPublishDraft.PlayAllPatternIds.empty() &&
			!promoter.Consume_ProductInventoryRefreshRequest(),
			"Save launched publish, changed DRAFT eligibility, or refreshed inventory");
		RequireEditorStep(promoter.Save(status), status, "repeat source-only Save with a clean draft");
		Require(promoter.Get_Composition() == savedPublishDraft && ReadText(sourcePath) == savedPublishBytes &&
			!promoter.Is_PublishRunning() && !promoter.Consume_ProductInventoryRefreshRequest(),
			"clean Save rewrote the source or started publish");
		RequireEditorStep(promoter.Publish_AllPatterns(status), status, "Publish All starts actual isolated process with an all-DRAFT tree");
		Require(promoter.Is_PublishRunning() && !promoter.Is_Dirty() && !promoter.Consume_ProductInventoryRefreshRequest() &&
			promoter.Get_Composition() == savedPublishDraft && ReadText(sourcePath) == savedPublishBytes,
			"Publish All failed to launch, rewrote source statuses, or refreshed inventory before success");
		Require(!promoter.Publish_AllPatterns(status) && !status.empty() && promoter.Is_PublishRunning(),
			"Publish All started a second process while the first was still observed");
		Require(!promoter.Save(status) && promoter.Get_Composition() == savedPublishDraft &&
			ReadText(sourcePath) == savedPublishBytes, "Save wrote during the observed publication");
		const auto waitForPublish = [&]() {
			const auto deadline = GetTickCount64() + 10000u;
			while (promoter.Is_PublishRunning() && GetTickCount64() < deadline) { Sleep(10u); promoter.Tick_Background(); }
			Require(!promoter.Is_PublishRunning(), "bounded isolated publisher process did not finish");
		};
		waitForPublish();
		Require(promoter.Consume_ProductInventoryRefreshRequest() && !promoter.Consume_ProductInventoryRefreshRequest(),
			"successful publisher completion did not emit exactly one Product inventory refresh");
		bool capturedPublisherOutput = false;
		for (const auto& entry : std::filesystem::directory_iterator(scratchRoot / "out/KoukuSaydon"))
			if (entry.is_regular_file()) capturedPublisherOutput |= ReadText(entry.path()).find(
				"Owner=KoukuSaydon; Revision=" + std::to_string(promoter.Get_Composition().iRevision)) != std::string::npos;
		Require(capturedPublisherOutput, "Publish All log did not capture the owner and saved source revision");
		Require(promoter.Get_Composition() == savedPublishDraft && ReadText(sourcePath) == savedPublishBytes,
			"successful Publish All rewrote authored statuses or source bytes");
		Require(WriteText(publishScript, "param([string]$Owner,[uint32]$ExpectedKoukuSaydonSourceRevision)\nWrite-Output 'fixture publish rejected'\nexit 7\n"),
			"could not write isolated publisher rejection fixture");
		RequireEditorStep(promoter.Publish_AllPatterns(status), status, "retry actual isolated Publish All process");
		waitForPublish();
		Require(!promoter.Consume_ProductInventoryRefreshRequest() && promoter.Get_Composition() == savedPublishDraft &&
			ReadText(sourcePath) == savedPublishBytes, "failed publisher completion refreshed inventory or changed source");
		Require(WriteText(sourcePath, "{invalid source"), "could not stage a failed Composition reload");
		Require(!promoter.Reload(status) && !promoter.Is_Dirty() && promoter.Get_Composition() == savedPublishDraft,
			"failed reload did not retain the clean last-good Composition");
		Require(WriteText(sourcePath, savedPublishBytes), "could not restore the saved Composition fixture");
		Require(!promoter.Publish_AllPatterns(status) && !status.empty() && !promoter.Is_PublishRunning() &&
			!promoter.Consume_ProductInventoryRefreshRequest() && ReadText(sourcePath) == savedPublishBytes,
			"Publish All accepted a stale last-good Composition without a successful reload");
		RequireEditorStep(promoter.Reload(status), status, "reopen the saved source after stale publish rejection");
	}

	void VerifyKoukuEditorRoundtrip()
	{
		const auto sourceRoot = Client::CProjectDataRoot::Get();
		const auto scratchRoot = std::filesystem::temp_directory_path() /
			("LostArkKoukuCompositionEditor-" + std::to_string(GetCurrentProcessId()) +
			 "-" + std::to_string(GetTickCount64()));
		const auto dataRoot = scratchRoot / "Data";
		const auto relativeSource =
			std::filesystem::path("KoukuSaydon/Gate1/KoukuSaydonComposition.json");
		const auto sourcePath = dataRoot / relativeSource;
		Require(CopyFixture(sourceRoot / relativeSource, sourcePath),
			"could not copy real Kouku composition source");
		const auto relativeSequence = std::filesystem::path("Compositions/Sequences/KoukuSaydonSequenceComposition.json");
		const auto sequencePath = dataRoot / relativeSequence;
		Require(CopyFixture(sourceRoot / relativeSequence, sequencePath),
			"could not copy independent Sequence composition seed");
		for (const char* profile : { "MN_RPCT_05", "MN_RPCT_06",
			"MN_RPCT_07", "MN_RPCZ_00" })
		{
			const auto relative = std::filesystem::path("Animation/Reference/KoukuSaydon") /
				(std::string(profile) + ".actionreference.json");
			Require(CopyFixture(sourceRoot / relative, dataRoot / relative),
				"could not copy real Kouku Action reference");
		}
		SCOPED_ENVIRONMENT_VARIABLE environment(L"LOSTARK_PROJECT_DATA_ROOT");
		Require(environment.Set(dataRoot), "could not select the scratch Data root");
		std::cout << "Kouku editor fixture: " << sourcePath.string() << '\n';

		VerifyKoukuSequenceDocumentIsolation(sourcePath, sequencePath);
		VerifyLegacyEditorMigration(sourcePath);
		VerifyKoukuGateBundleStorage(sourcePath);
		VerifyKoukuPresentationDocument(sourcePath);
		VerifyKoukuBoneColliderPreview(sourcePath);
		VerifyKoukuRootVerticalScale(sourcePath);
		Client::CKoukuSaydonActionWorkbench workbench;
		std::string status;
		RequireEditorStep(workbench.Reload(status), status, "load real source");
		RequireEditorStep(workbench.Select_ActorProfile("MN_RPCT_05", status),
			status, "select Saydon Pattern category");
		std::string patternId;
		RequireEditorStep(workbench.Create_Pattern("Native editor roundtrip", "NORMAL",
			patternId, status), status, "create DRAFT Pattern");
		RequireEditorStep(workbench.Append_ActionAsStages(patternId, "MN_RPCT_05",
			4219811u, status), status, "append Saydon Action 4219811 as Stages");
		const auto firstAppend = EditorPattern(workbench, patternId);
		Require(firstAppend.strActorProfileId == "MN_RPCT_05" &&
			firstAppend.Stages.size() == 1u &&
			firstAppend.Stages[0].AnimationOccurrences.size() == 1u &&
			firstAppend.Stages[0].iDurationMs == 4333u &&
			firstAppend.Stages[0].AnimationOccurrences[0].strProfileId == "MN_RPCT_05" &&
			firstAppend.Stages[0].AnimationOccurrences[0].strRuntimeClip ==
				"rpct00_att_battle_12_06",
			"Saydon Action did not preserve its one 4333ms source Stage");
		const std::string firstStageId = firstAppend.Stages[0].strStageId;
		RequireEditorStep(workbench.Append_ActionToStage(patternId, firstStageId,
			"MN_RPCT_05", 4219811u, status), status, "append Action into selected Stage");
		const auto& combined = EditorPattern(workbench, patternId).Stages[0];
		Require(combined.AnimationOccurrences.size() == 2u &&
			combined.AnimationOccurrences[1].iStartOffsetMs == 4333u &&
			combined.iDurationMs == 8666u,
			"Append Action to Stage did not preserve ordered source windows");

		RequireEditorStep(workbench.Append_ActionAsStages(patternId, "MN_RPCT_05",
			0u, status), status, "append real Action ID zero");
		const auto withIdle = EditorPattern(workbench, patternId);
		Require(withIdle.Stages.size() == 3u,
			"Action ID zero did not append exactly its two populated Stages");
		for (std::size_t index = 1u; index < 3u; ++index)
		{
			const auto& row = withIdle.Stages[index].AnimationOccurrences.at(0u);
			Require(row.iSourceActionId == 0u && row.strProfileId == "MN_RPCT_05" &&
				row.strSourceStageId == (index == 1u ? "stage-003" : "stage-006") &&
				row.strSourceSlotId == "animation-000" && row.iPlayMs == 3000u &&
				!row.strReferenceRevision.empty(),
				"Action zero was replaced by RAW identity or lost its source Stage");
		}
		RequireEditorRoundtrip(workbench);

		RequireEditorStep(workbench.Append_ActionAsStages(patternId, "MN_RPCT_07",
			4219905u, status), status, "append Saydon profile 07 physical-model alias");
		const auto saydon = EditorPattern(workbench, patternId);
		Require(saydon.strActorProfileId == "MN_RPCT_05" && saydon.Stages.size() == 5u &&
			saydon.Stages[3].AnimationOccurrences[0].strProfileId == "MN_RPCT_07",
			"Saydon alias did not preserve source profile 07 and physical owner 05");
		RequireEditorRoundtrip(workbench);

		const auto beforeCrossModel = workbench.Get_Composition();
		const std::string savedBeforeCrossModel = ReadText(sourcePath);
		Require(!workbench.Append_ActionAsStages(patternId, "MN_RPCZ_00", 4219714u, status),
			"Kouku Action appended to a Saydon-owned Pattern");
		Require(workbench.Get_Composition() == beforeCrossModel && !workbench.Is_Dirty() &&
			ReadText(sourcePath) == savedBeforeCrossModel,
			"cross-model rejection changed draft, owner or saved source");

		RequireEditorStep(workbench.Set_PatternAuthoringStatus(patternId, "PRODUCT", status), status,
			"admit the supported Saydon arena body as PRODUCT");
		RequireEditorRoundtrip(workbench);
		RequireEditorStep(workbench.Set_PatternAuthoringStatus(patternId, "DRAFT", status), status,
			"return the native editor fixture to DRAFT");
		RequireEditorRoundtrip(workbench);

		RequireEditorStep(workbench.Select_ActorProfile("MN_RPCZ_00", status), status,
			"select Kouku Pattern category");
		std::string koukuPatternId;
		RequireEditorStep(workbench.Create_Pattern("Separate Kouku pattern", "NORMAL",
			koukuPatternId, status), status, "create separate Kouku-owned Pattern");
		RequireEditorStep(workbench.Append_ActionAsStages(koukuPatternId, "MN_RPCZ_00",
			4219714u, status), status, "append Kouku six-Stage Pizza");
		Require(EditorPattern(workbench, koukuPatternId).strActorProfileId == "MN_RPCZ_00" &&
			EditorPattern(workbench, koukuPatternId).Stages.size() == 6u,
			"separate Kouku Pattern lost its owner or source Stages");
		RequireEditorRoundtrip(workbench);
		const auto koukuBeforeDelete = EditorPattern(workbench, koukuPatternId);
		workbench.Select_WorkbenchBoss(Client::COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON_GATE2);
		RequireEditorStep(workbench.Select_ActorProfile("MN_RPCT_06", status), status,
			"select Gate 2 Large Saydon Model View");
		const auto beforeAutoCreate = workbench.Get_Composition();
		Require(!workbench.Append_ActionAsStages("", "MN_RPCT_06", 999999999u, status),
			"unknown Action created an automatic Pattern");
		Require(workbench.Get_Composition() == beforeAutoCreate && !workbench.Is_Dirty(),
			"failed automatic Append consumed an ordinal or mutated the draft");
		Require(!workbench.Append_ActionAsStages("", "MN_RPCT_06", 4221801u, status),
			"Append without explicit Pattern selection created an automatic timeline");
		Require(workbench.Get_Composition() == beforeAutoCreate && !workbench.Is_Dirty(),
			"rejected Append changed draft or counters");
		std::string largePatternId;
		RequireEditorStep(workbench.Create_Pattern("Separate Gate 2 Large Saydon", "NORMAL",
			largePatternId, status), status, "explicitly create Large Saydon Pattern");
		RequireEditorStep(workbench.Append_ActionAsStages(largePatternId, "MN_RPCT_06", 4221801u,
			status), status, "append Large Saydon Action to the explicit Pattern");
		Require(!largePatternId.empty() &&
			EditorPattern(workbench, largePatternId).strActorProfileId == "MN_RPCT_06" &&
			EditorPattern(workbench, largePatternId).Stages.size() == 6u &&
			workbench.Get_Composition().Patterns.size() == beforeAutoCreate.Patterns.size() + 1u &&
			workbench.Get_Composition().iNextPatternOrdinal == beforeAutoCreate.iNextPatternOrdinal + 1u,
			"explicit Create and Append did not preserve one owned Pattern");
		RequireEditorRoundtrip(workbench);
		const auto largeBeforeDelete = EditorPattern(workbench, largePatternId);
		workbench.Select_WorkbenchBoss(Client::COMPOSITION_WORKBENCH_BOSS::KOUKU_SAYDON);
		RequireEditorStep(workbench.Select_ActorProfile("MN_RPCT_05", status), status,
			"return to Saydon Pattern category");
		std::string longPatternId;
		RequireEditorStep(workbench.Create_Pattern("Real 249 Stage Saydon Action", "NORMAL",
			longPatternId, status), status, "create long reference-action Pattern");
		RequireEditorStep(workbench.Append_ActionAsStages(longPatternId, "MN_RPCT_05",
			4219880u, status), status, "append real 249-Stage Saydon Action");
		const auto longPattern = EditorPattern(workbench, longPatternId);
		std::uint32_t longDurationMs = 0u;
		for (const auto& stage : longPattern.Stages)
		{
			longDurationMs += stage.iDurationMs;
			Require(stage.AnimationOccurrences.size() == 1u &&
				stage.AnimationOccurrences[0].iSourceActionId == 4219880u,
				"long Action append dropped or duplicated a source slot");
		}
		Require(longPattern.Stages.size() == 249u && longDurationMs == 273134u,
			"long Action was truncated to the old 64-Stage Product capacity");
		RequireEditorRoundtrip(workbench);
		const auto longBeforeDelete = EditorPattern(workbench, longPatternId);
		RequireEditorStep(workbench.Select_PatternById(patternId, status), status,
			"select Saydon Pattern for batch deletion");

		const auto beforeDelete = EditorPattern(workbench, patternId);
		const std::string removedStage = beforeDelete.Stages[0].strStageId;
		const std::string overlappingBox =
			beforeDelete.Stages[0].AnimationOccurrences[0].strOccurrenceId;
		const std::string removedBox =
			beforeDelete.Stages[1].AnimationOccurrences[0].strOccurrenceId;
		RequireEditorStep(workbench.Delete_TimelineSelection(patternId,
			{ removedStage }, { overlappingBox, removedBox }, status), status,
			"delete selected Stage and a box in another Stage");
		auto expected = beforeDelete;
		expected.Stages.erase(expected.Stages.begin());
		expected.Stages[0].AnimationOccurrences.clear();
		Require(EditorPattern(workbench, patternId) == expected &&
			EditorPattern(workbench, koukuPatternId) == koukuBeforeDelete &&
			EditorPattern(workbench, largePatternId) == largeBeforeDelete &&
			EditorPattern(workbench, longPatternId) == longBeforeDelete,
			"batch delete changed unselected Stages, counters, timing or the other model");
		RequireEditorRoundtrip(workbench);

		VerifyKoukuTimelineControls(workbench, sourcePath);
		VerifyKoukuLogicControls(workbench, sourcePath);
		VerifyKoukuObjectPlacement(workbench, sourcePath);

		const auto beforeInvalid = workbench.Get_Composition();
		Require(!workbench.Append_ActionAsStages(patternId, "MN_RPCT_05",
			999999999u, status), "unknown Action was accepted");
		Require(!workbench.Delete_TimelineSelection("MISSING_PATTERN",
			{ removedStage }, {}, status), "missing Pattern accepted batch delete");
		Require(workbench.Get_Composition() == beforeInvalid && !workbench.Is_Dirty(),
			"rejected edit changed the admitted draft");

		RequireEditorStep(workbench.Rename_Pattern(patternId, "Unsaved local edit", status),
			status, "stage edit before stale Save");
		const auto pending = workbench.Get_Composition();
		const std::string externalBytes = ReadText(sourcePath) + "\n";
		Require(WriteText(sourcePath, externalBytes), "could not stage concurrent source edit");
		Require(!workbench.Save(status), "stale Save overwrote external source edit");
		Require(ReadText(sourcePath) == externalBytes &&
			workbench.Get_Composition() == pending && workbench.Is_Dirty(),
			"stale Save failed to preserve external bytes and unsaved local draft");
	}
}

int Run_KoukuSequenceDocumentContractTests()
{
	try
	{
		const auto sourceRoot = Client::CProjectDataRoot::Get();
		const auto scratchRoot = std::filesystem::temp_directory_path() /
			("LostArkKoukuSequenceDocument-" + std::to_string(GetCurrentProcessId()) +
			 "-" + std::to_string(GetTickCount64()));
		const auto dataRoot = scratchRoot / "Data";
		const auto relativeAction = std::filesystem::path("KoukuSaydon/Gate1/KoukuSaydonComposition.json");
		const auto relativeSequence = std::filesystem::path("Compositions/Sequences/KoukuSaydonSequenceComposition.json");
		Require(CopyFixture(sourceRoot / relativeAction, dataRoot / relativeAction) &&
			CopyFixture(sourceRoot / relativeSequence, dataRoot / relativeSequence),
			"could not copy the real Action and Sequence authoring documents");
		SCOPED_ENVIRONMENT_VARIABLE environment(L"LOSTARK_PROJECT_DATA_ROOT");
		Require(environment.Set(dataRoot), "could not select the isolated Sequence test Data root");
		VerifyKoukuSequenceDocumentIsolation(dataRoot / relativeAction, dataRoot / relativeSequence);
		std::cout << "KoukuSequenceDocumentContractTests: real-seed/isolated-path/identity-rejection/atomic-save/reload/CAS/action-source-preservation/complete-sequence-order-zero-start-pause-failure-cancel passed\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << "KoukuSequenceDocumentContractTests: FAIL: " << error.what() << '\n';
		return 1;
	}
}

int Run_KoukuCompositionEditorContractTests()
{
	try
	{
		VerifyKoukuEditorRoundtrip();
		std::cout << "KoukuCompositionEditorContractTests: append/action-zero/model-owners/"
			"sequence-path-identity/sequence-CAS/action-source-preservation/gate-bundle-storage/249-stage/batch-delete/duration/multi-duplicate/preview-request/logic/presentation/region-link/quarantine/save-reload/CAS passed\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << "KoukuCompositionEditorContractTests: FAIL: " << error.what() << '\n';
		return 1;
	}
}

int Run_KoukuPreviewTransportContractTests()
{
	try
	{
		VerifyKoukuPreviewTransportContracts();
		VerifyKoukuAllLaneDuplicateContracts();
		std::cout << "KoukuPreviewTransportContractTests: pause-clock/cold-live-end-scrub/Play-restart/paused-Collider/one-shot/Collider-Apply-window-link/Collider-save-without-Apply/linked-Sector-scale/Stage-retarget/source-preservation/all-lane-segment-copy/linked-definition-COW passed\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << "KoukuPreviewTransportContractTests: FAIL: " << error.what() << '\n';
		return 1;
	}
}
