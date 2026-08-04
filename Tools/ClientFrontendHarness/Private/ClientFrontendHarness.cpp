#include "Client_Defines.h"
#include "LobbyCommandService.h"
#include "AnimationSkillBindingDocument.h"
#include "CharacterSelectionState.h"
#include "PlayerSkillCatalog.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace
{
	struct TEST_RUNNER final
	{
		void Require(const bool_t condition, const char_t* pName)
		{
			if (condition)
			{
				std::cout << "[PASS] " << pName << '\n';
				return;
			}

			++iFailureCount;
			std::cout << "[FAILURE] " << pName << '\n';
		}

		std::size_t iFailureCount = 0;
	};

	void Require_NoPendingCommand(
		TEST_RUNNER& runner,
		const char_t* pName)
	{
		Client::LOBBY_COMMAND command{};
		runner.Require(
			!Client::CLobbyCommandService::Try_Consume(command),
			pName);
	}

	void Test_NormalHandoff(TEST_RUNNER& runner)
	{
		using namespace Client;
		LOBBY_COMMAND_TOKEN token = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			CLobbyCommandService::Request(LOBBY_STAGE::TEST, token) &&
			INVALID_LOBBY_COMMAND_TOKEN != token,
			"Normal Handoff Stages Tokenized Test Command");

		LOBBY_COMMAND_TOKEN duplicateToken = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			!CLobbyCommandService::Request(
				LOBBY_STAGE::BERN,
				duplicateToken) &&
			INVALID_LOBBY_COMMAND_TOKEN == duplicateToken,
			"Duplicate Enter Does Not Replace Pending Command");

		LOBBY_COMMAND command{};
		runner.Require(
			CLobbyCommandService::Try_Consume(command) &&
			LOBBY_STAGE::TEST == command.eStage &&
			LOBBY_COMMAND_PURPOSE::GAMEPLAY == command.ePurpose &&
			token == command.iToken,
			"Lobby Consumes Exact Handoff Command Once");
		Require_NoPendingCommand(
			runner,
			"Consumed Handoff Leaves No Stale Command");
	}

	void Test_EntryPurpose(TEST_RUNNER& runner)
	{
		using namespace Client;
		LOBBY_COMMAND_TOKEN token = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			CLobbyCommandService::Request(
				LOBBY_STAGE::TEST,
				LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE,
				token),
			"Map Editor Test Purpose Is Staged Explicitly");

		LOBBY_COMMAND command{};
		runner.Require(
			CLobbyCommandService::Try_Consume(command) &&
			LOBBY_STAGE::TEST == command.eStage &&
			LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE == command.ePurpose &&
			token == command.iToken,
			"Lobby Preserves Map Editor Purpose Through Handoff");

		token = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			!CLobbyCommandService::Request(
				LOBBY_STAGE::BERN,
				LOBBY_COMMAND_PURPOSE::MAP_EDITOR_WORKSPACE,
				token) &&
			INVALID_LOBBY_COMMAND_TOKEN == token,
			"Map Editor Purpose Rejects Non-Test Stage");
	}

	void Test_ExactCancellation(TEST_RUNNER& runner)
	{
		using namespace Client;
		LOBBY_COMMAND_TOKEN token = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			CLobbyCommandService::Request(LOBBY_STAGE::TEST, token),
			"Cancellation Fixture Stages Test Command");
		runner.Require(
			CLobbyCommandService::Cancel(token, "handoff owner failed"),
			"Exact Token Cancels Pending Command");
		Require_NoPendingCommand(
			runner,
			"Exact Cancellation Leaves No Stale Command");
	}

	void Test_StaleTokenCannotCancelNewCommand(TEST_RUNNER& runner)
	{
		using namespace Client;
		LOBBY_COMMAND_TOKEN oldToken = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			CLobbyCommandService::Request(LOBBY_STAGE::TEST, oldToken) &&
			CLobbyCommandService::Cancel(oldToken, "old handoff cancelled"),
			"Old Handoff Is Cancelled");

		LOBBY_COMMAND_TOKEN newToken = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			CLobbyCommandService::Request(LOBBY_STAGE::BERN, newToken) &&
			newToken > oldToken,
			"New Handoff Receives New Token");
		runner.Require(
			!CLobbyCommandService::Cancel(oldToken, "stale failure"),
			"Stale Failure Cannot Cancel New Handoff");

		LOBBY_COMMAND command{};
		runner.Require(
			CLobbyCommandService::Try_Consume(command) &&
			LOBBY_STAGE::BERN == command.eStage &&
			newToken == command.iToken,
			"New Handoff Survives Stale Cancellation");
	}

	void Test_InvalidRequestsPreservePendingCommand(TEST_RUNNER& runner)
	{
		using namespace Client;
		LOBBY_COMMAND_TOKEN token = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			!CLobbyCommandService::Request(LOBBY_STAGE::END, token) &&
			INVALID_LOBBY_COMMAND_TOKEN == token,
			"Invalid Stage Is Rejected");

		runner.Require(
			CLobbyCommandService::Request(LOBBY_STAGE::VALTAN, token),
			"Valid Command Is Staged After Invalid Stage");
		runner.Require(
			!CLobbyCommandService::Cancel(
				INVALID_LOBBY_COMMAND_TOKEN,
				"invalid token"),
			"Invalid Token Is Rejected");

		LOBBY_COMMAND command{};
		runner.Require(
			CLobbyCommandService::Try_Consume(command) &&
			LOBBY_STAGE::VALTAN == command.eStage &&
			token == command.iToken,
			"Invalid Cancellation Preserves Pending Command");
	}

	void Test_CharacterSelectServerHandoff(TEST_RUNNER& runner)
	{
		using namespace Client;
		using namespace LostArk::Shared;
		CCharacterSelectionState::Clear_TestEntryMode();
		runner.Require(!CCharacterSelectionState::Stage_TestEntryMode(
			CHARACTER_TEST_ENTRY_MODE::SERVER_GAMEPLAY),
			"Server Gameplay Handoff Requires Selected Class");
		runner.Require(CCharacterSelectionState::Select(
			CHARACTER_CLASS_ID::DIMENSIONMASTER) &&
			CCharacterSelectionState::Stage_TestEntryMode(
				CHARACTER_TEST_ENTRY_MODE::SERVER_GAMEPLAY),
			"Lobby Stages Approved Character Select Server Handoff");
		CHARACTER_TEST_ENTRY_MODE consumed = CHARACTER_TEST_ENTRY_MODE::NONE;
		runner.Require(CCharacterSelectionState::Try_Consume_TestEntryMode(consumed) &&
			CHARACTER_TEST_ENTRY_MODE::SERVER_GAMEPLAY == consumed,
			"Character Select Consumes Approved Server Handoff Once");
		runner.Require(!CCharacterSelectionState::Try_Consume_TestEntryMode(consumed),
			"Consumed Server Handoff Cannot Be Replayed");
		runner.Require(CCharacterSelectionState::Stage_TestEntryMode(
			CHARACTER_TEST_ENTRY_MODE::SERVER_GAMEPLAY) &&
			CCharacterSelectionState::Select(CHARACTER_CLASS_ID::ARTIST) &&
			!CCharacterSelectionState::Try_Consume_TestEntryMode(consumed),
			"Changing Class Clears Stale Server Handoff");
	}

	std::string Read_Text(const std::filesystem::path& path)
	{
		std::ifstream input(path, std::ios::binary);
		return std::string(std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>());
	}

	std::vector<Client::PLAYER_SKILL_DEFINITION> Make_SkillFixture()
	{
		using namespace Client;
		using namespace LostArk::Shared;
		PLAYER_SKILL_DEFINITION active{};
		active.iSkillId = 101u;
		active.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		active.strInputSlot = "Q";
		active.eSkillKind = PLAYER_SKILL_KIND::ACTIVE;
		PLAYER_SKILL_DEFINITION combo{};
		combo.iSkillId = 102u;
		combo.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		combo.strInputSlot = "LMB";
		combo.eSkillKind = PLAYER_SKILL_KIND::COMBO;
		combo.iComboStageCount = 3u;
		return { active, combo };
	}

	Client::ANIMATION_SKILL_BINDING_DOCUMENT Make_BindingFixture()
	{
		using namespace Client;
		using namespace LostArk::Shared;
		ANIMATION_SKILL_BINDING_DOCUMENT document;
		document.strAnimationAssetId = "LanceMaster";
		document.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		document.Bindings =
		{
			{ 101u, { "Active_A", "Active_B" } },
			{ 102u, { "BA_1", "BA_2", "BA_3" } }
		};
		return document;
	}

	void Test_SkillBindingSchema(TEST_RUNNER& runner)
	{
		using namespace Client;
		using namespace LostArk::Shared;
		const auto skills = Make_SkillFixture();
		const std::vector<std::string> clips =
			{ "Active_A", "Active_B", "BA_1", "BA_2", "BA_3" };
		const std::string valid =
			"{\"schema\":\"lostark.animation-skill-bindings\","
			"\"formatVersion\":1,\"animationAssetId\":\"LanceMaster\","
			"\"characterClass\":\"LANCE_MASTER\",\"bindings\":["
			"{\"skillId\":101,\"clips\":[\"Active_A\",\"Active_B\"]},"
			"{\"skillId\":102,\"clips\":[\"BA_1\",\"BA_2\",\"BA_3\"]}]}";
		ANIMATION_SKILL_BINDING_DOCUMENT output;
		std::string status;
		runner.Require(
			CAnimationSkillBindingDocument::Parse_Text(valid, output, status) &&
			CAnimationSkillBindingDocument::Validate(output, "LanceMaster",
				CHARACTER_CLASS_ID::LANCE_MASTER, skills, clips, status),
			"Skill Binding Valid Parse And Catalog Join");

		const std::string wrongVersion =
			"{\"schema\":\"lostark.animation-skill-bindings\","
			"\"formatVersion\":2,\"animationAssetId\":\"LanceMaster\","
			"\"characterClass\":\"LANCE_MASTER\",\"bindings\":["
			"{\"skillId\":101,\"clips\":[\"Active_A\"]}]}";
		const std::string traversal =
			"{\"schema\":\"lostark.animation-skill-bindings\","
			"\"formatVersion\":1,\"animationAssetId\":\"../Escape\","
			"\"characterClass\":\"LANCE_MASTER\",\"bindings\":["
			"{\"skillId\":101,\"clips\":[\"Active_A\"]}]}";
		const std::string extraField = valid.substr(0u, valid.size() - 1u) +
			",\"unexpected\":true}";
		runner.Require(!CAnimationSkillBindingDocument::Parse_Text(
			wrongVersion, output, status), "Skill Binding Rejects Wrong Version");
		runner.Require(!CAnimationSkillBindingDocument::Parse_Text(
			traversal, output, status), "Skill Binding Rejects Traversal Asset");
		runner.Require(!CAnimationSkillBindingDocument::Parse_Text(
			extraField, output, status), "Skill Binding Rejects Extra Field");

		auto invalid = Make_BindingFixture();
		runner.Require(!CAnimationSkillBindingDocument::Validate(invalid,
			"Artist", CHARACTER_CLASS_ID::LANCE_MASTER, skills, clips, status),
			"Skill Binding Rejects Wrong Owner");
		invalid = Make_BindingFixture();
		invalid.Bindings[1].iSkillId = 101u;
		runner.Require(!CAnimationSkillBindingDocument::Validate(invalid,
			"LanceMaster", CHARACTER_CLASS_ID::LANCE_MASTER, skills, clips, status),
			"Skill Binding Rejects Duplicate Skill");
		invalid = Make_BindingFixture();
		invalid.Bindings.pop_back();
		runner.Require(!CAnimationSkillBindingDocument::Validate(invalid,
			"LanceMaster", CHARACTER_CLASS_ID::LANCE_MASTER, skills, clips, status),
			"Skill Binding Rejects Missing Skill");
		invalid = Make_BindingFixture();
		invalid.Bindings[0].iSkillId = 999u;
		runner.Require(!CAnimationSkillBindingDocument::Validate(invalid,
			"LanceMaster", CHARACTER_CLASS_ID::LANCE_MASTER, skills, clips, status),
			"Skill Binding Rejects Unknown Skill");
		invalid = Make_BindingFixture();
		invalid.Bindings[0].Clips[0] = "Missing_Clip";
		runner.Require(!CAnimationSkillBindingDocument::Validate(invalid,
			"LanceMaster", CHARACTER_CLASS_ID::LANCE_MASTER, skills, clips, status),
			"Skill Binding Rejects Unknown Model Clip");
		invalid = Make_BindingFixture();
		invalid.Bindings[1].Clips.pop_back();
		runner.Require(!CAnimationSkillBindingDocument::Validate(invalid,
			"LanceMaster", CHARACTER_CLASS_ID::LANCE_MASTER, skills, clips, status),
			"Skill Binding Rejects Combo Count Mismatch");

		ANIMATION_SKILL_BINDING_DOCUMENT preserved = Make_BindingFixture();
		runner.Require(!CAnimationSkillBindingDocument::Load_FromPath(
			L"Z:/definitely/missing/skillbindings.json", "LanceMaster",
			CHARACTER_CLASS_ID::LANCE_MASTER, skills, clips, preserved, status) &&
			2u == preserved.Bindings.size() &&
			"Active_A" == preserved.Bindings[0].Clips[0],
			"Skill Binding Failed Staged Load Preserves Prior Output");
	}

	void Test_RealSkillBindingDocuments(TEST_RUNNER& runner)
	{
		using namespace Client;
		using namespace LostArk::Shared;
		std::string status;
		if (!CPlayerSkillCatalog::Load(status))
		{
			runner.Require(false, "Real Player Skill Catalog Loads");
			return;
		}
		struct OWNER { const char_t* asset; CHARACTER_CLASS_ID characterClass; };
		constexpr OWNER owners[] =
		{
			{ "LanceMaster", CHARACTER_CLASS_ID::LANCE_MASTER },
			{ "GunSlinger", CHARACTER_CLASS_ID::GUNSLINGER },
			{ "Slayer", CHARACTER_CLASS_ID::SLAYER },
			{ "Artist", CHARACTER_CLASS_ID::ARTIST },
			{ "DimensionMaster", CHARACTER_CLASS_ID::DIMENSIONMASTER }
		};
		std::size_t total = 0u;
		for (const OWNER& owner : owners)
		{
			ANIMATION_SKILL_BINDING_DOCUMENT parsed;
			const std::filesystem::path path =
				CAnimationSkillBindingDocument::Resolve_Path(owner.asset);
			if (!CAnimationSkillBindingDocument::Parse_Text(
				Read_Text(path), parsed, status))
			{
				runner.Require(false, "Real Skill Binding Document Parses");
				continue;
			}
			std::vector<std::string> available;
			for (const ANIMATION_SKILL_BINDING& binding : parsed.Bindings)
				available.insert(available.end(), binding.Clips.begin(), binding.Clips.end());
			runner.Require(CAnimationSkillBindingDocument::Validate(parsed,
				owner.asset, owner.characterClass, CPlayerSkillCatalog::Get_Skills(),
				available, status),
				"Real Skill Binding Covers Current Class Catalog");
			total += parsed.Bindings.size();
		}
		runner.Require(53u == total,
			"Five Real Skill Binding Documents Cover 53 Skill Definitions");
	}

	void Test_SkillBindingAtomicSave(TEST_RUNNER& runner)
	{
		using namespace Client;
		using namespace LostArk::Shared;
		wchar_t tempBase[MAX_PATH]{};
		GetTempPathW(MAX_PATH, tempBase);
		const std::filesystem::path root = std::filesystem::path(tempBase) /
			("LostArkSkillBindingHarness_" + std::to_string(GetCurrentProcessId()));
		std::error_code error;
		std::filesystem::create_directories(root, error);
		SetEnvironmentVariableW(L"LOSTARK_PROJECT_DATA_ROOT", root.c_str());
		const auto skills = Make_SkillFixture();
		const std::vector<std::string> clips =
			{ "Active_A", "Active_B", "BA_1", "BA_2", "BA_3" };
		const auto document = Make_BindingFixture();
		std::string status;
		runner.Require(CAnimationSkillBindingDocument::Save_Atomic(document,
			"LanceMaster", CHARACTER_CLASS_ID::LANCE_MASTER,
			skills, clips, status),
			"Skill Binding Atomic Save Writes Verified Destination");
		const std::filesystem::path destination =
			CAnimationSkillBindingDocument::Resolve_Path("LanceMaster");
		const std::string before = Read_Text(destination);
		SetFileAttributesW(destination.c_str(), FILE_ATTRIBUTE_READONLY);
		auto changed = document;
		changed.Bindings[0].Clips = { "Active_B" };
		runner.Require(!CAnimationSkillBindingDocument::Save_Atomic(changed,
			"LanceMaster", CHARACTER_CLASS_ID::LANCE_MASTER,
			skills, clips, status) && before == Read_Text(destination),
			"Skill Binding Replace Failure Preserves Destination");
		SetFileAttributesW(destination.c_str(), FILE_ATTRIBUTE_NORMAL);
		SetEnvironmentVariableW(L"LOSTARK_PROJECT_DATA_ROOT", nullptr);
		std::filesystem::remove_all(root, error);
	}
}

int main()
{
	TEST_RUNNER runner{};

	Test_NormalHandoff(runner);
	Test_CharacterSelectServerHandoff(runner);
	Test_EntryPurpose(runner);
	Test_ExactCancellation(runner);
	Test_StaleTokenCannotCancelNewCommand(runner);
	Test_InvalidRequestsPreservePendingCommand(runner);
	Test_SkillBindingSchema(runner);
	Test_RealSkillBindingDocuments(runner);
	Test_SkillBindingAtomicSave(runner);

	std::cout << "failures : " << runner.iFailureCount << '\n';
	return 0 == runner.iFailureCount ? 0 : 1;
}
