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
			candidate.Patterns[0].strActorProfileId == legacyOwner &&
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
		const auto& foreignPatterns = workbench.Get_Composition().Patterns;
		const auto foreignPattern = std::find_if(foreignPatterns.begin(), foreignPatterns.end(), [&](const auto& candidate) {
			return candidate.strPatternId != patternId && candidate.strLoadError.empty() && !candidate.Stages.empty() &&
				!candidate.Stages.front().AnimationOccurrences.empty(); });
		Require(foreignPattern != foreignPatterns.end(), "foreign Pattern fixture is missing");
		const auto foreignBox = foreignPattern->Stages.front().AnimationOccurrences.front().strOccurrenceId;
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
		VerifyKoukuPresentationDocument(sourcePath);
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
		VerifyKoukuLogicControls(workbench, sourcePath);

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
			"249-stage/batch-delete/duration/multi-duplicate/preview-request/logic/presentation/region-link/quarantine/save-reload/CAS passed\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << "KoukuCompositionEditorContractTests: FAIL: " << error.what() << '\n';
		return 1;
	}
}
