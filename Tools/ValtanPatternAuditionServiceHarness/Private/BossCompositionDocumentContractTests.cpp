#include "BossCompositionDocument.h"
#include "ProjectDataRoot.h"

#include <Windows.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
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
