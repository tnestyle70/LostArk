#include "BossCompositionDocument.h"
#include "KoukuSaydonActionWorkbench.h"
#include "ProjectDataRoot.h"

#include <Windows.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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

	void VerifyLegacyEditorMigration(const std::filesystem::path& sourcePath)
	{
		using namespace Client;
		const std::string original = ReadText(sourcePath);
		KOUKU_SAYDON_COMPOSITION_DOCUMENT source;
		std::string status;
		RequireEditorStep(CKoukuSaydonCompositionDocument::Parse_Text(original, source, status),
			status, "parse current composition for legacy fixture");
		const auto pizza = std::find_if(source.Patterns.begin(), source.Patterns.end(),
			[](const auto& pattern) { return pattern.strPatternId == "KAKULSAYDON_G1_PIZZA"; });
		Require(pizza != source.Patterns.end() && pizza->Stages.size() >= 2u,
			"real Pizza fixture is missing");
		auto mixed = *pizza;
		mixed.strPatternId = "KAKULSAYDON_G1_NATIVE_MIXED_LEGACY";
		mixed.strAuthoringStatus = "DRAFT";
		for (std::size_t index = 0u; index < mixed.Stages.size(); ++index)
		{
			mixed.Stages[index].strActionId = mixed.strPatternId + ".stage." +
				std::to_string(index + 1u);
			mixed.Stages[index].AnimationOccurrences.at(0u).strOccurrenceId =
				mixed.strPatternId + ".animation." + std::to_string(index + 1u);
		}
		mixed.Stages[0].AnimationOccurrences[0].strProfileId = "MN_RPCT_05";
		source.Patterns.push_back(mixed);
		std::string serialized = CKoukuSaydonCompositionDocument::Serialize(source);
		Require(ReplaceOnce(serialized, "\"formatVersion\": 2", "\"formatVersion\": 1"),
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
		Require(candidate.iFormatVersion == 2u &&
			candidate.Patterns[0].strActorProfileId == "MN_RPCZ_00" &&
			!candidate.Patterns.back().strLoadError.empty() &&
			!candidate.Patterns.back().strPreservedJson.empty(),
			"v1 migration lost deterministic owner or mixed-model raw quarantine");
		const std::string preserved = candidate.Patterns.back().strPreservedJson;
		RequireEditorStep(document.Save_Atomic(candidate, status), status,
			"save migrated v2 document with quarantine");
		CKoukuSaydonCompositionDocument reopened;
		RequireEditorStep(reopened.Reload_FromPath(sourcePath, status), status,
			"reopen migrated v2 document");
		Require(reopened.Get_LastGood().iFormatVersion == 2u &&
			reopened.Get_LastGood().Patterns.back().strPreservedJson == preserved &&
			ReadText(sourcePath).find("\"formatVersion\": 2") != std::string::npos,
			"v2 Save/Reload lost the quarantined legacy source JSON");
		Require(WriteText(sourcePath, original), "could not restore scratch source for editor test");
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
		const auto empty = workbench.Get_Composition();
		Require(!workbench.Set_PatternDuration(patternId, 10000u, status) &&
			workbench.Get_Composition() == empty && !workbench.Is_Dirty(),
			"empty Pattern duration fabricated a Stage or changed its counters");
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
			duplicated.Stages.size() == 3u && duplicated.Stages[0] == firstStage &&
			duplicated.iNextStageOrdinal == beforeDuplicate.iNextStageOrdinal + 1u &&
			duplicated.iNextAnimationOrdinal == beforeDuplicate.iNextAnimationOrdinal + 2u,
			"multi-duplicate changed owner, original Stage or consumed wrong ID counts");
		for (const auto& untouched : documentBeforeDuplicate.Patterns)
			if (untouched.strPatternId != patternId)
				Require(EditorPattern(workbench, untouched.strPatternId) == untouched,
					"duplicate changed another actor's Pattern");
		const auto& clonedStage = duplicated.Stages[1];
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
		auto expectedBox = secondStage.AnimationOccurrences[0];
		expectedBox.strOccurrenceId = duplicated.Stages[2].AnimationOccurrences.back().strOccurrenceId;
		expectedSecond.AnimationOccurrences.push_back(expectedBox);
		Require(duplicated.Stages[2] == expectedSecond && expectedBox.strOccurrenceId != secondBox,
			"standalone box clone changed its Stage clock, offset, profile or original box");
		RequireEditorStep(workbench.Validate_Draft(status), status,
			"validate duplicate stable IDs and ownership");
		RequireEditorRoundtrip(workbench);

		const auto duplicateBaseline = workbench.Get_Composition();
		const auto duplicateBytes = ReadText(sourcePath);
		Require(!workbench.Duplicate_TimelineSelection(patternId,
			{ firstStage.strStageId, "STALE_STAGE" }, { secondBox }, status),
			"stale Stage selection partially duplicated valid entries");
		Require(!workbench.Duplicate_TimelineSelection(patternId,
			{ firstStage.strStageId }, { "STALE_BOX" }, status),
			"stale box selection partially duplicated a valid Stage");
		const auto foreignBox = EditorPattern(workbench, "KAKULSAYDON_G1_PIZZA").Stages[0].
			AnimationOccurrences[0].strOccurrenceId;
		Require(!workbench.Duplicate_TimelineSelection(patternId, {}, { foreignBox }, status),
			"duplicate imported a box owned by another actor Pattern");
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

		VerifyLegacyEditorMigration(sourcePath);
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

		Require(!workbench.Set_PatternAuthoringStatus(patternId, "PRODUCT", status),
			"Saydon authoring Pattern was admitted as the Server Kouku PRODUCT");
		Require(workbench.Get_Composition() == beforeCrossModel && !workbench.Is_Dirty(),
			"rejected non-boss PRODUCT changed the saved Saydon draft");

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
		RequireEditorStep(workbench.Select_ActorProfile("MN_RPCT_06", status), status,
			"select empty Large Saydon category");
		const auto beforeAutoCreate = workbench.Get_Composition();
		Require(!workbench.Append_ActionAsStages("", "MN_RPCT_06", 999999999u, status),
			"unknown Action created an automatic Pattern");
		Require(workbench.Get_Composition() == beforeAutoCreate && !workbench.Is_Dirty(),
			"failed automatic Append consumed an ordinal or mutated the draft");
		RequireEditorStep(workbench.Append_ActionAsStages("", "MN_RPCT_06", 4221801u,
			status), status, "auto-create Large Saydon Pattern with Action append");
		const std::string largePatternId = workbench.Get_SelectedPatternId();
		Require(!largePatternId.empty() &&
			EditorPattern(workbench, largePatternId).strActorProfileId == "MN_RPCT_06" &&
			EditorPattern(workbench, largePatternId).Stages.size() == 6u &&
			workbench.Get_Composition().Patterns.size() == beforeAutoCreate.Patterns.size() + 1u &&
			workbench.Get_Composition().iNextPatternOrdinal == beforeAutoCreate.iNextPatternOrdinal + 1u,
			"automatic Action Append did not commit one owned Pattern atomically");
		RequireEditorRoundtrip(workbench);
		const auto largeBeforeDelete = EditorPattern(workbench, largePatternId);
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

int Run_KoukuCompositionEditorContractTests()
{
	try
	{
		VerifyKoukuEditorRoundtrip();
		std::cout << "KoukuCompositionEditorContractTests: append/action-zero/model-owners/"
			"249-stage/batch-delete/duration/multi-duplicate/preview-request/save-reload/CAS passed\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << "KoukuCompositionEditorContractTests: FAIL: " << error.what() << '\n';
		return 1;
	}
}
