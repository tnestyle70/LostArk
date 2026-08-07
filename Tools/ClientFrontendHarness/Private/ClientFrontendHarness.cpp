#include "Client_Defines.h"
#include "LobbyCommandService.h"
#include "AnimationSkillBindingDocument.h"
#include "CharacterSelectionState.h"
#include "DataJson.h"
#include "Effect_DocumentCodec.h"
#include "Effect_Distribution.h"
#include "Effect_Catalog.h"
#include "Effect_Playback.h"
#include "PlayerSkillCatalog.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <Windows.h>

namespace
{
	class SCOPED_ENVIRONMENT_VARIABLE final
	{
	public:
		explicit SCOPED_ENVIRONMENT_VARIABLE(const wchar_t* pName) :
			m_strName(pName)
		{
			SetLastError(ERROR_SUCCESS);
			const DWORD required = GetEnvironmentVariableW(
				m_strName.c_str(), nullptr, 0u);
			m_bWasDefined = 0u != required ||
				GetLastError() != ERROR_ENVVAR_NOT_FOUND;
			if (0u == required)
				return;
			std::vector<wchar_t> value(required);
			const DWORD length = GetEnvironmentVariableW(
				m_strName.c_str(), value.data(), required);
			if (0u != length && length < required)
				m_strOriginalValue.assign(value.data(), length);
		}

		~SCOPED_ENVIRONMENT_VARIABLE()
		{
			Restore();
		}

		SCOPED_ENVIRONMENT_VARIABLE(const SCOPED_ENVIRONMENT_VARIABLE&) = delete;
		SCOPED_ENVIRONMENT_VARIABLE& operator=(
			const SCOPED_ENVIRONMENT_VARIABLE&) = delete;

		bool_t Set(const wchar_t* pValue)
		{
			m_bRestored = false;
			return FALSE != SetEnvironmentVariableW(m_strName.c_str(), pValue);
		}

		bool_t Was_Defined() const { return m_bWasDefined; }
		const std::wstring& Get_OriginalValue() const { return m_strOriginalValue; }

		void Restore()
		{
			if (m_bRestored)
				return;
			SetEnvironmentVariableW(m_strName.c_str(),
				m_bWasDefined ? m_strOriginalValue.c_str() : nullptr);
			m_bRestored = true;
		}

	private:
		std::wstring m_strName;
		std::wstring m_strOriginalValue;
		bool_t m_bWasDefined = false;
		bool_t m_bRestored = false;
	};

	bool_t Collect_DimensionMasterEffectRoster(
		std::vector<uint32_t>& outSkillIds,
		std::vector<std::string>& outEffectIds,
		std::string& outStatus)
	{
		using namespace Client;
		using namespace LostArk::Shared;
		outSkillIds.clear();
		outEffectIds.clear();
		if (!CPlayerSkillCatalog::Load(outStatus))
			return false;

		for (const PLAYER_SKILL_DEFINITION& skill :
			CPlayerSkillCatalog::Get_Skills())
		{
			if (skill.eCharacterClass != CHARACTER_CLASS_ID::DIMENSIONMASTER ||
				skill.strInputSlot == "SPACE")
			{
				continue;
			}

			const std::string expectedEffectId =
				"effect.dimensionmaster.skill." +
				std::to_string(skill.iSkillId);
			if (skill.strEffectId != expectedEffectId)
			{
				outStatus = "DimensionMaster slot " + skill.strInputSlot +
					" must map skill " + std::to_string(skill.iSkillId) +
					" to " + expectedEffectId;
				return false;
			}

			outSkillIds.push_back(skill.iSkillId);
			outEffectIds.push_back(skill.strEffectId);
			for (std::size_t stage = 1u;
				stage <= skill.iComboStageCount; ++stage)
			{
				outEffectIds.push_back(skill.strEffectId + ".ba" +
					std::to_string(stage));
			}
		}

		if (outSkillIds.empty())
		{
			outStatus = "DimensionMaster trial effect roster is empty.";
			return false;
		}
		return true;
	}

	struct DIMENSIONMASTER_EFFECT_BUILD_ROW final
	{
		std::size_t iComponentCount = 0u;
		std::size_t iEmitterCount = 0u;
	};

	struct DIMENSIONMASTER_EFFECT_BUILD_CONTRACT final
	{
		std::size_t iEffectCount = 0u;
		std::size_t iComponentCount = 0u;
		std::size_t iEmitterCount = 0u;
		std::map<std::string, DIMENSIONMASTER_EFFECT_BUILD_ROW, std::less<>> Effects;
	};

	bool_t Read_ContractSize(
		const Client::DATA_JSON_VALUE& object,
		const char_t* pName,
		std::size_t& outValue)
	{
		const Client::DATA_JSON_VALUE* value = object.Find(pName);
		if (nullptr == value || !value->Is_Number() ||
			!std::isfinite(value->Get_Number()) ||
			value->Get_Number() != std::floor(value->Get_Number()) ||
			value->Get_Number() < 0.0 ||
			value->Get_Number() > static_cast<double>(
				(std::numeric_limits<std::size_t>::max)()))
		{
			return false;
		}
		outValue = static_cast<std::size_t>(value->Get_Number());
		return true;
	}

	bool_t Load_DimensionMasterEffectBuildContract(
		DIMENSIONMASTER_EFFECT_BUILD_CONTRACT& outContract,
		std::string& outStatus)
	{
		using namespace Client;
		outContract = {};
		const std::filesystem::path path = CProjectDataRoot::Resolve(
			L"Effects/Imported/DimensionMaster/"
			L"DimensionMaster.component-build.receipt.json");
		std::ifstream input(path, std::ios::binary);
		if (path.empty() || !input)
		{
			outStatus = "DimensionMaster component build receipt is missing.";
			return false;
		}
		std::string text{
			std::istreambuf_iterator<char_t>(input),
			std::istreambuf_iterator<char_t>() };
		if (text.starts_with("\xEF\xBB\xBF"))
			text.erase(0u, 3u);
		DATA_JSON_VALUE root;
		if (!CDataJson::Parse(text, root, outStatus) || !root.Is_Object())
			return false;

		const DATA_JSON_VALUE* schema = root.Find("schema");
		const DATA_JSON_VALUE* version = root.Find("version");
		const DATA_JSON_VALUE* characterClass = root.Find("characterClass");
		const DATA_JSON_VALUE* compileIdentity = root.Find("compileIdentityComplete");
		const DATA_JSON_VALUE* effects = root.Find("effects");
		if (nullptr == schema || !schema->Is_String() ||
			schema->Get_String() != "lostark.effect-component-build-receipt" ||
			nullptr == version || !version->Is_Number() ||
			version->Get_Number() != 1.0 ||
			nullptr == characterClass || !characterClass->Is_String() ||
			characterClass->Get_String() != "DIMENSIONMASTER" ||
			nullptr == compileIdentity || !compileIdentity->Is_Boolean() ||
			!compileIdentity->Get_Boolean() ||
			nullptr == effects || !effects->Is_Array() ||
			!Read_ContractSize(root, "effectCount", outContract.iEffectCount) ||
			!Read_ContractSize(root, "componentCount", outContract.iComponentCount) ||
			!Read_ContractSize(root, "emitterCount", outContract.iEmitterCount))
		{
			outStatus = "DimensionMaster component build receipt contract is invalid.";
			return false;
		}

		std::size_t componentTotal = 0u;
		std::size_t emitterTotal = 0u;
		for (const DATA_JSON_VALUE& effect : effects->Get_Array())
		{
			const DATA_JSON_VALUE* effectAssetId = effect.Find("effectAssetId");
			const DATA_JSON_VALUE* effectIdentity = effect.Find("compileIdentity");
			const DATA_JSON_VALUE* sourceSha = effect.Find("sourceDocumentSha256");
			const DATA_JSON_VALUE* compiledSha = effect.Find("compiledDocumentSha256");
			DIMENSIONMASTER_EFFECT_BUILD_ROW row;
			if (!effect.Is_Object() || nullptr == effectAssetId ||
				!effectAssetId->Is_String() || effectAssetId->Get_String().empty() ||
				nullptr == effectIdentity || !effectIdentity->Is_Boolean() ||
				!effectIdentity->Get_Boolean() || nullptr == sourceSha ||
				!sourceSha->Is_String() || nullptr == compiledSha ||
				!compiledSha->Is_String() ||
				sourceSha->Get_String() != compiledSha->Get_String() ||
				!Read_ContractSize(effect, "componentCount", row.iComponentCount) ||
				!Read_ContractSize(effect, "emitterCount", row.iEmitterCount) ||
				0u == row.iComponentCount || 0u == row.iEmitterCount ||
				!outContract.Effects.emplace(
					effectAssetId->Get_String(), row).second)
			{
				outStatus = "DimensionMaster component build receipt effect row is invalid.";
				return false;
			}
			componentTotal += row.iComponentCount;
			emitterTotal += row.iEmitterCount;
		}

		if (outContract.Effects.size() != outContract.iEffectCount ||
			componentTotal != outContract.iComponentCount ||
			emitterTotal != outContract.iEmitterCount)
		{
			outStatus = "DimensionMaster component build receipt totals disagree with its rows.";
			return false;
		}
		return true;
	}

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

	void Test_CharacterSelectPreviewReturnCommand(TEST_RUNNER& runner)
	{
		using namespace Client;
		LOBBY_COMMAND_TOKEN token = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			CLobbyCommandService::Request(
				LOBBY_STAGE::CHARACTER_SELECT,
				token) &&
			INVALID_LOBBY_COMMAND_TOKEN != token,
			"Server Arena Preview Selection Stages Tokenized Return");

		LOBBY_COMMAND command{};
		runner.Require(
			CLobbyCommandService::Try_Consume(command) &&
			LOBBY_STAGE::CHARACTER_SELECT == command.eStage &&
			LOBBY_COMMAND_PURPOSE::GAMEPLAY == command.ePurpose &&
			token == command.iToken,
			"Lobby Consumes Exact Character Select Preview Return");
		Require_NoPendingCommand(
			runner,
			"Preview Return Leaves No Stale Lobby Command");
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
			{ 101u, { { "Active_A", 0u, 1.f }, { "Active_B", 0u, 1.f } } },
			{ 102u, { { "BA_1", 0u, 1.f }, { "BA_2", 0u, 1.f },
				{ "BA_3", 0u, 1.f } } }
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
			"\"formatVersion\":2,\"animationAssetId\":\"LanceMaster\","
			"\"characterClass\":\"LANCE_MASTER\",\"bindings\":["
			"{\"skillId\":101,\"clips\":[\"Active_A\","
			"{\"clip\":\"Active_B\",\"playMs\":250,\"playRate\":1.25}]},"
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
			"\"formatVersion\":3,\"animationAssetId\":\"LanceMaster\","
			"\"characterClass\":\"LANCE_MASTER\",\"bindings\":["
			"{\"skillId\":101,\"clips\":[\"Active_A\"]}]}";
		const std::string badPlayMs =
			"{\"schema\":\"lostark.animation-skill-bindings\","
			"\"formatVersion\":2,\"animationAssetId\":\"LanceMaster\","
			"\"characterClass\":\"LANCE_MASTER\",\"bindings\":["
			"{\"skillId\":101,\"clips\":["
			"{\"clip\":\"Active_A\",\"playMs\":0}]}]}";
		const std::string traversal =
			"{\"schema\":\"lostark.animation-skill-bindings\","
			"\"formatVersion\":2,\"animationAssetId\":\"../Escape\","
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
		runner.Require(!CAnimationSkillBindingDocument::Parse_Text(
			badPlayMs, output, status), "Skill Binding Rejects Zero Clip Play Length");

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
		invalid.Bindings[0].Clips[0].strClipName = "Missing_Clip";
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
			"Active_A" == preserved.Bindings[0].Clips[0].strClipName,
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
			{ "DimensionMaster", CHARACTER_CLASS_ID::DIMENSIONMASTER },
			{ "Warlord", CHARACTER_CLASS_ID::WARLORD }
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
			{
				for (const ANIMATION_SKILL_CLIP& clip : binding.Clips)
					available.push_back(clip.strClipName);
			}
			runner.Require(CAnimationSkillBindingDocument::Validate(parsed,
				owner.asset, owner.characterClass, CPlayerSkillCatalog::Get_Skills(),
				available, status),
				"Real Skill Binding Covers Current Class Catalog");
			if (CHARACTER_CLASS_ID::DIMENSIONMASTER == owner.characterClass)
			{
				struct EXPECTED_BINDING final
				{
					const char_t* pSlot;
					LostArk::Shared::SKILL_ID iSkillId;
					std::vector<const char_t*> Clips;
				};
				const std::vector<EXPECTED_BINDING> expected =
				{
					{ "LMB", 2050010u, {
						"pc_sp_m_00_sk_att_battle_1_01",
						"pc_sp_m_00_sk_att_battle_1_02",
						"pc_sp_m_00_sk_att_battle_1_03",
						"pc_sp_m_00_sk_att_battle_1_04" } },
					{ "Q", 2050100u, { "pc_sp_m_00_sk_sk_nailstrike_01" } },
					{ "W", 2050120u, {
						"pc_sp_m_00_sk_sk_dimensionalleap_01",
						"pc_sp_m_00_sk_sk_dimensionalleap_02",
						"pc_sp_m_00_sk_sk_dimensionalleap_03" } },
					{ "E", 2050160u, {
						"pc_sp_m_00_sk_sk_overslash_01",
						"pc_sp_m_00_sk_sk_overslash_02",
						"pc_sp_m_00_sk_sk_overslash_03",
						"pc_sp_m_00_sk_sk_overslash_04",
						"pc_sp_m_00_sk_sk_overslash_05" } },
					{ "R", 2050180u, { "pc_sp_m_00_sk_sk_foldcut" } },
					{ "A", 2050210u, { "pc_sp_m_00_sk_sk_willowrend" } },
					{ "S", 2050220u, { "pc_sp_m_00_sk_sk_momentaryrift" } },
					{ "F", 2050230u, { "pc_sp_m_00_sk_sk_chronorecoil" } },
					{ "D", 2050240u, {
						"pc_sp_m_00_sk_sk_telekinesisthrust_01",
						"pc_sp_m_00_sk_sk_telekinesisthrust_04" } },
					{ "T", 2050500u, { "pc_sp_m_00_sk_sk_dimensionprison" } },
					{ "V", 2050520u, { "pc_sp_m_00_sk_sk_timewave" } },
					{ "ALT_V", 2050540u, { "pc_sp_m_00_sk_sk_super_timewave" } },
					/* jump_04 carries the same notify timeline as
					moving_normal_1_04, so binding both replayed one leap twice. */
					{ "SPACE", 2050020u, {
						"pc_sp_m_00_sk_sk_moving_normal_1_04" } }
				};
				bool_t exactRoster = expected.size() == parsed.Bindings.size();
				for (const EXPECTED_BINDING& row : expected)
				{
					const PLAYER_SKILL_DEFINITION* skill =
						CPlayerSkillCatalog::Find_BySlot(
							CHARACTER_CLASS_ID::DIMENSIONMASTER, row.pSlot);
					const auto binding = std::find_if(
						parsed.Bindings.begin(), parsed.Bindings.end(),
						[&row](const ANIMATION_SKILL_BINDING& candidate)
						{
							return row.iSkillId == candidate.iSkillId;
						});
					bool_t clipsMatch = binding != parsed.Bindings.end() &&
						binding->Clips.size() == row.Clips.size();
					if (clipsMatch)
					{
						for (size_t iClip = 0u; iClip < row.Clips.size(); ++iClip)
						{
							clipsMatch = clipsMatch &&
								binding->Clips[iClip].strClipName == row.Clips[iClip];
						}
					}
					const std::string expectedEffectId =
						std::string(row.pSlot) == "SPACE" ? std::string{} :
						"effect.dimensionmaster.skill." +
						std::to_string(row.iSkillId);
					const bool_t effectIdentityMatches = nullptr != skill &&
						skill->strEffectId == expectedEffectId;
					exactRoster = exactRoster && nullptr != skill &&
						skill->iSkillId == row.iSkillId && clipsMatch &&
						effectIdentityMatches;
				}
				runner.Require(exactRoster,
					"DimensionMaster Trial Roster Joins Exact Slots Skills Clips And Effects");
			}
			total += parsed.Bindings.size();
		}
		runner.Require(CPlayerSkillCatalog::Get_Skills().size() == total,
			"Every Real Skill Binding Document Covers Every Skill Definition");
	}

	void Test_SkillBindingAtomicSave(TEST_RUNNER& runner)
	{
		using namespace Client;
		using namespace LostArk::Shared;
		SCOPED_ENVIRONMENT_VARIABLE projectDataRootEnvironment(
			L"LOSTARK_PROJECT_DATA_ROOT");
		wchar_t tempBase[MAX_PATH]{};
		GetTempPathW(MAX_PATH, tempBase);
		const std::filesystem::path root = std::filesystem::path(tempBase) /
			("LostArkSkillBindingHarness_" + std::to_string(GetCurrentProcessId()));
		std::error_code error;
		std::filesystem::create_directories(root, error);
		projectDataRootEnvironment.Set(root.c_str());
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
		changed.Bindings[0].Clips = { { "Active_B", 0u, 1.f } };
		runner.Require(!CAnimationSkillBindingDocument::Save_Atomic(changed,
			"LanceMaster", CHARACTER_CLASS_ID::LANCE_MASTER,
			skills, clips, status) && before == Read_Text(destination),
			"Skill Binding Replace Failure Preserves Destination");
		SetFileAttributesW(destination.c_str(), FILE_ATTRIBUTE_NORMAL);
		projectDataRootEnvironment.Restore();
		std::filesystem::remove_all(root, error);
	}

	std::string Snapshot_EffectFrame(
		const Client::EFFECT_EVALUATED_FRAME& frame)
	{
		std::ostringstream output;
		output << std::hexfloat << frame.fSampleTimeSeconds << '|';
		auto appendMatrix = [&output](const float4x4_t& value)
		{
			const f32_t* values = &value._11;
			for (size_t index = 0u; index < 16u; ++index)
				output << values[index] << ',';
		};
		for (const Client::EFFECT_EVALUATED_ELEMENT& element : frame.Elements)
		{
			output << 'E' << element.pElement->strElementId << ':';
			appendMatrix(element.World);
			output << element.Color.vColorOffset.x << ','
				<< element.Color.vColorMultiply.x << ','
				<< element.Color.fEmissiveIntensity << ','
				<< element.fLocalTimeSeconds << ','
				<< element.fNormalizedLife << '|';
		}
		for (const Client::EFFECT_EVALUATED_PARTICLE& particle : frame.Particles)
		{
			output << 'P' << particle.pElement->strElementId << ':';
			appendMatrix(particle.World);
			output << particle.Color.x << ',' << particle.Color.y << ','
				<< particle.Color.z << ',' << particle.Color.w << '|';
		}
		for (const Client::EFFECT_EVALUATED_TRAIL& trail : frame.Trails)
		{
			output << 'T' << trail.pElement->strElementId << ':';
			for (const Client::EFFECT_EVALUATED_TRAIL_POINT& point : trail.Points)
			{
				output << point.vWorldPosition.x << ','
					<< point.vWorldPosition.y << ','
					<< point.vWorldPosition.z << ','
					<< point.fNormalizedAge << ';';
			}
			output << '|';
		}
		for (const Client::EFFECT_EVALUATED_AFTERIMAGE& image : frame.AfterImages)
		{
			output << 'A' << image.pElement->strElementId << ':';
			appendMatrix(image.World);
			output << image.fAlpha << '|';
		}
		return output.str();
	}

	void Test_EffectPlaybackDeterminism(TEST_RUNNER& runner)
	{
		using namespace Client;
		EFFECT_DOCUMENT_DESC document;
		document.strEffectAssetId = "effect.playback.harness";
		document.strDisplayName = "Playback Harness";

		EFFECT_ELEMENT_DESC particle;
		particle.strElementId = "particle";
		particle.strDisplayName = "Particle";
		particle.eKind = EFFECT_ELEMENT_KIND::PARTICLE;
		particle.Detail.Timing.fLifeTimeSeconds = 3.f;
		particle.Detail.Particle.iMaxParticles = 64u;
		particle.Detail.Particle.fSpawnRatePerSecond = 12.f;
		particle.Detail.Particle.iBurstCount = 3u;
		particle.Detail.Particle.iRandomSeed = 77u;
		particle.Detail.Particle.vLifeTimeSeconds = { 0.25f, 0.5f };
		particle.Detail.Particle.vInitialPositionMin = { -1.f, -0.5f, 0.f };
		particle.Detail.Particle.vInitialPositionMax = { 1.f, 0.5f, 0.f };
		document.Elements.push_back(particle);

		EFFECT_ELEMENT_DESC trail;
		trail.strElementId = "trail";
		trail.strDisplayName = "Trail";
		trail.eKind = EFFECT_ELEMENT_KIND::TRAIL;
		trail.Detail.Timing.fLifeTimeSeconds = 3.f;
		trail.Detail.Transform.vVelocityPerSecond = { 1.f, 0.f, 0.f };
		trail.Detail.Trail.fPointLifeTimeSeconds = 0.35f;
		trail.Detail.Trail.fMinimumDistance = 0.f;
		document.Elements.push_back(trail);

		EFFECT_ELEMENT_DESC sprite;
		sprite.strElementId = "sprite";
		sprite.strDisplayName = "Sprite";
		sprite.eKind = EFFECT_ELEMENT_KIND::SPRITE;
		sprite.Detail.Timing.fLifeTimeSeconds = 3.f;
		sprite.Detail.Timing.fAfterImageSeconds = 0.3f;
		sprite.Detail.AfterImage.fSampleIntervalSeconds = 0.1f;
		sprite.Detail.AfterImage.iMaxCopies = 4u;
		document.Elements.push_back(sprite);

		EFFECT_ELEMENT_DESC hidden;
		hidden.strElementId = "hidden";
		hidden.strDisplayName = "Hidden";
		hidden.eKind = EFFECT_ELEMENT_KIND::SPRITE;
		hidden.bVisible = false;
		hidden.Detail.Timing.fLifeTimeSeconds = 20.f;
		document.Elements.push_back(hidden);

		float4x4_t root{};
		XMStoreFloat4x4(&root, XMMatrixIdentity());
		auto simulate = [&](const uint32_t framesPerSecond)
		{
			CEffectPlayback playback;
			std::string status;
			if (!playback.Stage_Document(document, status))
				return std::string("STAGE-FAILED:") + status;
			const f32_t delta = 1.f / static_cast<f32_t>(framesPerSecond);
			for (uint32_t frame = 0u; frame < framesPerSecond * 2u; ++frame)
				playback.Update(delta, root);
			return Snapshot_EffectFrame(playback.Get_Frame());
		};

		const std::string at30 = simulate(30u);
		const std::string at60 = simulate(60u);
		const std::string at144 = simulate(144u);
		if (at30 != at60 || at60 != at144)
		{
			std::cout << "[DETAIL] Effect signatures 30/60/144 bytes="
				<< at30.size() << '/' << at60.size() << '/' << at144.size()
				<< " hash=" << std::hash<std::string>{}(at30) << '/'
				<< std::hash<std::string>{}(at60) << '/'
				<< std::hash<std::string>{}(at144) << '\n';
		}
		runner.Require(at30 == at60 && at60 == at144,
			"Effect Playback Is Deterministic At 30 60 And 144 FPS");

		CEffectPlayback seeked;
		std::string status;
		runner.Require(seeked.Stage_Document(document, status),
			"Effect Playback Stages Valid Document");
		seeked.Seek(2.f, root);
		const std::string seekSignature =
			Snapshot_EffectFrame(seeked.Get_Frame());
		if (at60 != seekSignature)
		{
			std::cout << "[DETAIL] Effect sequential/seek bytes="
				<< at60.size() << '/' << seekSignature.size() << " hash="
				<< std::hash<std::string>{}(at60) << '/'
				<< std::hash<std::string>{}(seekSignature) << '\n';
		}
		runner.Require(at60 == seekSignature,
			"Effect Playback Sequential Two Seconds Equals Seek");
		runner.Require(std::abs(seeked.Get_DurationSeconds() - 3.5f) < 0.0001f,
			"Effect Playback Duration Includes Particle Tail And Excludes Hidden Element");

		seeked.Seek(1.f, root);
		const std::string beforeInvalid =
			Snapshot_EffectFrame(seeked.Get_Frame());
		const f32_t durationBeforeInvalid = seeked.Get_DurationSeconds();
		EFFECT_DOCUMENT_DESC invalid = document;
		invalid.Elements[0].Detail.Particle.iMaxParticles = 4096u;
		runner.Require(!seeked.Stage_Document(invalid, status) &&
			beforeInvalid == Snapshot_EffectFrame(seeked.Get_Frame()) &&
			durationBeforeInvalid == seeked.Get_DurationSeconds(),
			"Effect Playback Invalid Stage Preserves Committed State");
		invalid = document;
		invalid.Elements[0].Detail.Particle.vInitialPositionMin.x = 2.f;
		invalid.Elements[0].Detail.Particle.vInitialPositionMax.x = -2.f;
		runner.Require(!seeked.Stage_Document(invalid, status) &&
			beforeInvalid == Snapshot_EffectFrame(seeked.Get_Frame()) &&
			durationBeforeInvalid == seeked.Get_DurationSeconds(),
			"Effect Playback Rejects Reversed Initial Position Range");
		invalid = document;
		invalid.ParticleSystem.fUniformScaleMultiplier = 0.f;
		runner.Require(!seeked.Stage_Document(invalid, status) &&
			beforeInvalid == Snapshot_EffectFrame(seeked.Get_Frame()) &&
			durationBeforeInvalid == seeked.Get_DurationSeconds(),
			"Effect Playback Rejects Invalid Particle System Without Replacing State");

		EFFECT_DOCUMENT_DESC modifierDocument;
		modifierDocument.strEffectAssetId = "effect.particle.system.harness";
		modifierDocument.strDisplayName = "Particle System Harness";
		EFFECT_ELEMENT_DESC modifierParticle;
		modifierParticle.strElementId = "particle";
		modifierParticle.strDisplayName = "Particle";
		modifierParticle.eKind = EFFECT_ELEMENT_KIND::PARTICLE;
		modifierParticle.Detail.Timing.fLifeTimeSeconds = 2.f;
		modifierParticle.Detail.Particle.iMaxParticles = 1u;
		modifierParticle.Detail.Particle.fSpawnRatePerSecond = 0.f;
		modifierParticle.Detail.Particle.iBurstCount = 1u;
		modifierParticle.Detail.Particle.vLifeTimeSeconds = { 1.f, 1.f };
		modifierParticle.Detail.Particle.vInitialPositionMin = { 1.f, 0.f, 0.f };
		modifierParticle.Detail.Particle.vInitialPositionMax = { 1.f, 0.f, 0.f };
		modifierParticle.Detail.Particle.vInitialVelocityMin = { 1.f, 0.f, 0.f };
		modifierParticle.Detail.Particle.vInitialVelocityMax = { 1.f, 0.f, 0.f };
		modifierParticle.Detail.Particle.vAcceleration = { 0.f, 0.f, 0.f };
		modifierParticle.Detail.Particle.vStartSize = { 1.f, 1.f };
		modifierParticle.Detail.Particle.vEndSize = { 1.f, 1.f };
		modifierDocument.Elements.push_back(modifierParticle);
		EFFECT_ELEMENT_DESC unaffectedSprite;
		unaffectedSprite.strElementId = "sprite";
		unaffectedSprite.strDisplayName = "Sprite";
		unaffectedSprite.eKind = EFFECT_ELEMENT_KIND::SPRITE;
		unaffectedSprite.Detail.Timing.fLifeTimeSeconds = 2.f;
		unaffectedSprite.Detail.Transform.vPosition = { 2.f, 0.f, 0.f };
		modifierDocument.Elements.push_back(unaffectedSprite);

		CEffectPlayback identitySystem;
		runner.Require(identitySystem.Stage_Document(modifierDocument, status),
			"Identity Particle System Stages");
		identitySystem.Seek(1.f / 60.f, root);
		const EFFECT_EVALUATED_FRAME identityFrame = identitySystem.Get_Frame();
		modifierDocument.ParticleSystem.fUniformScaleMultiplier = 2.f;
		modifierDocument.ParticleSystem.fYawOffsetDegrees = 90.f;
		modifierDocument.ParticleSystem.fDirectionYawDegrees = 90.f;
		modifierDocument.ParticleSystem.fInitialSpeedMultiplier = 3.f;
		CEffectPlayback modifiedSystem;
		runner.Require(modifiedSystem.Stage_Document(modifierDocument, status),
			"Modified Particle System Stages");
		modifiedSystem.Seek(1.f / 60.f, root);
		const EFFECT_EVALUATED_FRAME modifiedFrame = modifiedSystem.Get_Frame();
		const bool_t bFrameShape = identityFrame.Particles.size() == 1u &&
			modifiedFrame.Particles.size() == 1u &&
			identityFrame.Elements.size() == 1u &&
			modifiedFrame.Elements.size() == 1u;
		bool_t bParticleModifierMatches = false;
		bool_t bNonParticleUnchanged = false;
		if (bFrameShape)
		{
			float3_t velocity{};
			XMStoreFloat3(&velocity, XMVectorScale(
				XMVector3TransformNormal(
					XMVectorSet(1.f, 0.f, 0.f, 0.f),
					XMMatrixRotationY(XMConvertToRadians(90.f))),
				3.f));
			const float3_t position = {
				1.f + velocity.x / 60.f,
				velocity.y / 60.f,
				velocity.z / 60.f };
			float4x4_t expectedWorld{};
			XMStoreFloat4x4(&expectedWorld,
				XMMatrixTranslation(position.x, position.y, position.z) *
				XMMatrixScaling(2.f, 2.f, 2.f) *
				XMMatrixRotationY(XMConvertToRadians(90.f)));
			const float4x4_t& actualWorld = modifiedFrame.Particles.front().World;
			bParticleModifierMatches =
				std::abs(actualWorld._41 - expectedWorld._41) < 0.0001f &&
				std::abs(actualWorld._42 - expectedWorld._42) < 0.0001f &&
				std::abs(actualWorld._43 - expectedWorld._43) < 0.0001f &&
				std::abs(actualWorld._11 - expectedWorld._11) < 0.0001f &&
				std::abs(actualWorld._13 - expectedWorld._13) < 0.0001f;
			const float4x4_t& identitySprite = identityFrame.Elements.front().World;
			const float4x4_t& modifiedSprite = modifiedFrame.Elements.front().World;
			bNonParticleUnchanged =
				std::abs(identitySprite._41 - modifiedSprite._41) < 0.0001f &&
				std::abs(identitySprite._42 - modifiedSprite._42) < 0.0001f &&
				std::abs(identitySprite._43 - modifiedSprite._43) < 0.0001f &&
				std::abs(identitySprite._11 - modifiedSprite._11) < 0.0001f;
		}
		runner.Require(bFrameShape && bParticleModifierMatches,
			"Particle System Applies Layout Scale Yaw And Emission Direction Speed");
		runner.Require(bFrameShape && bNonParticleUnchanged,
			"Particle System Leaves Non Particle Elements Unchanged");
	}

	void Test_EffectSourceModuleOccurrenceOrder(TEST_RUNNER& runner)
	{
		using namespace Client;
		auto distribution = [](const std::string& path,
			const float3_t& value, const uint32_t components)
		{
			EFFECT_DISTRIBUTION_DESC result;
			result.strPropertyPath = path;
			result.iComponentCount = components;
			result.iOperation = 1u;
			result.vDefaultMinimum = { value.x, value.y, value.z, 0.f };
			result.vDefaultMaximum = result.vDefaultMinimum;
			return result;
		};
		auto module = [&distribution](const std::string& id,
			const std::string& className, const std::string& path,
			const float3_t& value, const uint32_t components)
		{
			EFFECT_SOURCE_MODULE_DESC result;
			result.strStableId = id;
			result.strClassName = className;
			result.strObjectPath = "Harness." + id;
			result.Distributions.push_back(
				distribution(path, value, components));
			return result;
		};

		EFFECT_DOCUMENT_DESC document;
		document.strEffectAssetId = "effect.module.order.harness";
		document.strDisplayName = "Module Order Harness";
		EFFECT_ELEMENT_DESC element;
		element.strElementId = "source_emitter";
		element.strDisplayName = "Source Emitter";
		element.eKind = EFFECT_ELEMENT_KIND::PARTICLE;
		element.Detail.Timing.fLifeTimeSeconds = 1.f;
		element.Detail.Particle.iMaxParticles = 1u;
		element.Detail.Particle.fSpawnRatePerSecond = 0.f;
		element.Detail.Particle.vLifeTimeSeconds = { 1.f, 1.f };
		element.SourceRecipe.bEnabled = true;
		element.SourceRecipe.strRendererShape = "sprite";
		element.SourceRecipe.fEmitterDurationSeconds = 1.f;
		element.SourceRecipe.iEmitterLoopCount = 1u;
		element.SourceRecipe.Bursts.push_back({ 0.f, 1u, 1u });
		element.SourceRecipe.Modules.push_back(module(
			"lifetime", "particlemodulelifetime", "lifetime",
			{ 1.f, 0.f, 0.f }, 1u));
		const EFFECT_SOURCE_MODULE_DESC accelerationA = module(
			"acceleration.a", "particlemoduleacceleration", "acceleration",
			{ 100.f, 0.f, 0.f }, 3u);
		EFFECT_SOURCE_MODULE_DESC absoluteVelocity = module(
			"velocity.absolute", "particlemodulevelocityoverlifetime",
			"veloverlife", { 100.f, 0.f, 0.f }, 3u);
		EFFECT_SOURCE_LITERAL_DESC absolute;
		absolute.strPropertyPath = "absolute";
		absolute.eKind = EFFECT_SOURCE_LITERAL_KIND::BOOLEAN;
		absolute.bBoolean = true;
		absoluteVelocity.Literals.push_back(absolute);
		const EFFECT_SOURCE_MODULE_DESC accelerationB = module(
			"acceleration.b", "particlemoduleacceleration", "acceleration",
			{ 100.f, 0.f, 0.f }, 3u);
		element.SourceRecipe.Modules.push_back(accelerationA);
		element.SourceRecipe.Modules.push_back(absoluteVelocity);
		element.SourceRecipe.Modules.push_back(accelerationB);
		document.Elements.push_back(element);

		float4x4_t root{};
		XMStoreFloat4x4(&root, XMMatrixIdentity());
		auto sampleX = [&root](const EFFECT_DOCUMENT_DESC& value)
		{
			CEffectPlayback playback;
			std::string status;
			if (!playback.Stage_Document(value, status))
				return -1.f;
			playback.Seek(1.f / 60.f, root);
			return playback.Get_Frame().Particles.empty() ? -1.f :
				playback.Get_Frame().Particles.front().World._41;
		};
		const f32_t sourceOrderX = sampleX(document);
		EFFECT_DOCUMENT_DESC reordered = document;
		auto& modules = reordered.Elements.front().SourceRecipe.Modules;
		std::swap(modules[1], modules[2]);
		const f32_t reorderedX = sampleX(reordered);
		const f32_t expectedSourceOrder =
			(1.f + 1.f / 60.f) / 60.f;
		const f32_t expectedReordered =
			(1.f + 2.f / 60.f) / 60.f;
		runner.Require(
			std::abs(sourceOrderX - expectedSourceOrder) < 0.00001f &&
			std::abs(reorderedX - expectedReordered) < 0.00001f &&
			sourceOrderX != reorderedX,
			"Effect Playback Preserves Source Module Order And Duplicate Occurrences");
	}

	void Test_EffectExactSourceSemantics(TEST_RUNNER& runner)
	{
		using namespace Client;
		SCOPED_ENVIRONMENT_VARIABLE resourceRootEnvironment(
			L"LOSTARK_RESOURCE_ROOT");
		auto literalNumber = [](const std::string& path, const f64_t value)
		{
			EFFECT_SOURCE_LITERAL_DESC result;
			result.strPropertyPath = path;
			result.eKind = EFFECT_SOURCE_LITERAL_KIND::NUMBER;
			result.fNumber = value;
			return result;
		};
		auto literalString = [](const std::string& path, const std::string& value)
		{
			EFFECT_SOURCE_LITERAL_DESC result;
			result.strPropertyPath = path;
			result.eKind = EFFECT_SOURCE_LITERAL_KIND::STRING;
			result.strString = value;
			return result;
		};
		auto distribution = [](const std::string& path,
			const float4_t& minimum, const float4_t& maximum,
			const uint32_t components, const uint32_t operation = 1u)
		{
			EFFECT_DISTRIBUTION_DESC result;
			result.strPropertyPath = path;
			result.iComponentCount = components;
			result.iOperation = operation;
			result.vDefaultMinimum = minimum;
			result.vDefaultMaximum = maximum;
			return result;
		};
		auto sourceElement = [](const std::string& id)
		{
			EFFECT_ELEMENT_DESC element;
			element.strElementId = id;
			element.strDisplayName = id;
			element.eKind = EFFECT_ELEMENT_KIND::PARTICLE;
			element.Detail.Timing.fLifeTimeSeconds = 2.f;
			element.Detail.Particle.iMaxParticles = 16u;
			element.Detail.Particle.fSpawnRatePerSecond = 0.f;
			element.Detail.Particle.vLifeTimeSeconds = { 1.f, 1.f };
			element.SourceRecipe.bEnabled = true;
			element.SourceRecipe.strRendererShape = "sprite";
			element.SourceRecipe.fEmitterDurationSeconds = 1.f;
			element.SourceRecipe.iEmitterLoopCount = 1u;
			return element;
		};
		auto lifetime = [&distribution]()
		{
			EFFECT_SOURCE_MODULE_DESC module;
			module.strStableId = "lifetime";
			module.strClassName = "particlemodulelifetime";
			module.strObjectPath = "Harness.Lifetime";
			module.Distributions.push_back(distribution(
				"lifetime", { 1.f, 0.f, 0.f, 0.f },
				{ 1.f, 0.f, 0.f, 0.f }, 1u));
			return module;
		};

		float4x4_t identity{};
		XMStoreFloat4x4(&identity, XMMatrixIdentity());
		std::string status;

		EFFECT_DISTRIBUTION_DESC cookedSquareSize;
		cookedSquareSize.strPropertyPath = "startsize";
		cookedSquareSize.iComponentCount = 3u;
		cookedSquareSize.iOperation = 1u;
		cookedSquareSize.LookupTable = {
			0.f, 500.f,
			500.f, 0.f, 0.f,
			500.f, 0.f, 0.f
		};
		const float4_t squareSize = CEffectDistribution::Evaluate(
			cookedSquareSize, 0.f, float4_t{});
		runner.Require(CEffectDistribution::Validate(cookedSquareSize, status) &&
			std::abs(squareSize.x - 500.f) < 0.0001f &&
			std::abs(squareSize.y) < 0.0001f &&
			std::abs(squareSize.z) < 0.0001f,
			"Effect Cooked Vector Lookup Skips Range Cache And Uses XYZ Stride");

		EFFECT_DISTRIBUTION_DESC cookedRandom;
		cookedRandom.strPropertyPath = "startvelocity";
		cookedRandom.iComponentCount = 3u;
		cookedRandom.iOperation = 2u;
		cookedRandom.iLookupTableChunkSize = 6u;
		cookedRandom.fLookupTableTimeScale = 1.f;
		cookedRandom.LookupTable = {
			0.f, 20.f,
			0.f, 2.f, 4.f, 10.f, 12.f, 14.f,
			2.f, 4.f, 6.f, 12.f, 14.f, 16.f
		};
		const float4_t randomMiddle = CEffectDistribution::Evaluate(
			cookedRandom, 0.5f, float4_t(0.5f, 0.5f, 0.5f, 0.5f));
		runner.Require(CEffectDistribution::Validate(cookedRandom, status) &&
			std::abs(randomMiddle.x - 6.f) < 0.0001f &&
			std::abs(randomMiddle.y - 8.f) < 0.0001f &&
			std::abs(randomMiddle.z - 10.f) < 0.0001f,
			"Effect Cooked Random Lookup Interpolates Payload Entries Only");
		EFFECT_DISTRIBUTION_DESC malformed = cookedSquareSize;
		malformed.LookupTable.pop_back();
		runner.Require(!CEffectDistribution::Validate(malformed, status),
			"Effect Malformed Cooked Lookup Fails Closed");
		EFFECT_DISTRIBUTION_DESC malformedMetadata = cookedRandom;
		malformedMetadata.iLookupTableNumElements = 1u;
		runner.Require(!CEffectDistribution::Validate(
				malformedMetadata, status),
			"Effect Cooked Lookup Rejects Noncanonical Element Count");

		const EFFECT_SUBUV_FRAME_DESC subUV =
			CEffectPlayback::Resolve_SourceSubUVFrame(
				8u, 4u, 10.5f, false, false, 0.f, true);
		runner.Require(
			std::abs(subUV.Current.x - 0.125f) < 0.0001f &&
			std::abs(subUV.Current.y - 0.25f) < 0.0001f &&
			std::abs(subUV.Current.z - 0.25f) < 0.0001f &&
			std::abs(subUV.Current.w - 0.25f) < 0.0001f &&
			std::abs(subUV.Next.z - 0.375f) < 0.0001f &&
			std::abs(subUV.Next.w - 0.25f) < 0.0001f &&
			std::abs(subUV.fBlend - 0.5f) < 0.0001f,
			"Effect Source SubUV Eight By Four Transform Executes Once");

		EFFECT_DOCUMENT_DESC seededDocument;
		seededDocument.strEffectAssetId = "effect.seeded.exact.harness";
		seededDocument.strDisplayName = "Seeded Exact Harness";
		EFFECT_ELEMENT_DESC seeded = sourceElement("seeded");
		seeded.SourceRecipe.Bursts.push_back({ 0.f, 1u, 1u });
		seeded.SourceRecipe.Modules.push_back(lifetime());
		EFFECT_SOURCE_MODULE_DESC seededLocation;
		seededLocation.strStableId = "location.seeded";
		seededLocation.strClassName = "particlemodulelocation_seeded";
		seededLocation.strObjectPath = "Harness.LocationSeeded";
		seededLocation.Literals.push_back(literalNumber(
			"randomseedinfo.randomseeds[0]", 123));
		seededLocation.Distributions.push_back(distribution(
			"startlocation", { 0.f, 0.f, 0.f, 0.f },
			{ 100.f, 100.f, 100.f, 0.f }, 3u, 2u));
		seeded.SourceRecipe.Modules.push_back(seededLocation);
		seededDocument.Elements.push_back(seeded);
		CEffectPlayback seededPlayback;
		runner.Require(seededPlayback.Stage_Document(seededDocument, status),
			"Effect Seeded Source Document Stages");
		seededPlayback.Seek(1.f / 60.f, identity);
		uint32_t ueSeed = 123u;
		auto nextUeFraction = [&ueSeed]()
		{
			ueSeed = ueSeed * 196314165u + 907633515u;
			const uint32_t ueBits = 0x3f800000u | (ueSeed >> 9u);
			f32_t value = 0.f;
			std::memcpy(&value, &ueBits, sizeof(value));
			return value - 1.f;
		};
		const float3_t ueFractions = {
			nextUeFraction(), nextUeFraction(), nextUeFraction()
		};
		const bool_t seededExact =
			seededPlayback.Get_Frame().Particles.size() == 1u &&
			std::abs(seededPlayback.Get_Frame().Particles.front().World._41 -
				ueFractions.x) < 0.00001f &&
			std::abs(seededPlayback.Get_Frame().Particles.front().World._42 -
				ueFractions.y) < 0.00001f &&
			std::abs(seededPlayback.Get_Frame().Particles.front().World._43 -
				ueFractions.z) < 0.00001f;
		seededPlayback.Reset();
		seededPlayback.Seek(1.f / 60.f, identity);
		runner.Require(seededExact &&
			std::abs(seededPlayback.Get_Frame().Particles.front().World._41 -
				ueFractions.x) < 0.00001f,
			"Effect Seeded Vector Uses UE Axis Random Order And Reset Replays It");

		EFFECT_DOCUMENT_DESC lockedDocument;
		lockedDocument.strEffectAssetId = "effect.random.lock.exact.harness";
		lockedDocument.strDisplayName = "Random Lock Exact Harness";
		EFFECT_ELEMENT_DESC locked = sourceElement("locked");
		locked.SourceRecipe.Bursts.push_back({ 0.f, 1u, 1u });
		locked.SourceRecipe.Modules.push_back(lifetime());
		EFFECT_SOURCE_MODULE_DESC lockedLocation;
		lockedLocation.strStableId = "location.locked";
		lockedLocation.strClassName = "particlemodulelocation_seeded";
		lockedLocation.strObjectPath = "Harness.LocationLocked";
		lockedLocation.Literals.push_back(literalNumber(
			"randomseedinfo.randomseeds[0]", 313));
		EFFECT_DISTRIBUTION_DESC lockedDistribution;
		lockedDistribution.strPropertyPath = "startlocation";
		lockedDistribution.iComponentCount = 3u;
		lockedDistribution.iOperation = 2u;
		lockedDistribution.iRandomLockAxes = 4u;
		lockedDistribution.iLookupTableChunkSize = 6u;
		lockedDistribution.iLookupTableNumElements = 2u;
		lockedDistribution.LookupTable = {
			6.f, 7.f,
			6.f, 6.f, 6.f, 7.f, 7.f, 7.f
		};
		lockedLocation.Distributions.push_back(lockedDistribution);
		locked.SourceRecipe.Modules.push_back(lockedLocation);
		lockedDocument.Elements.push_back(locked);
		CEffectPlayback lockedPlayback;
		const bool_t lockedStaged =
			lockedPlayback.Stage_Document(lockedDocument, status);
		if (lockedStaged)
			lockedPlayback.Seek(1.f / 60.f, identity);
		uint32_t lockedSeed = 313u;
		lockedSeed = lockedSeed * 196314165u + 907633515u;
		const uint32_t lockedBits = 0x3f800000u | (lockedSeed >> 9u);
		f32_t lockedFraction = 0.f;
		std::memcpy(&lockedFraction, &lockedBits, sizeof(lockedFraction));
		lockedFraction -= 1.f;
		const f32_t expectedLockedPosition =
			(6.f + lockedFraction) * 0.01f;
		const bool_t lockedExact = lockedStaged &&
			lockedPlayback.Get_Frame().Particles.size() == 1u &&
			std::abs(lockedPlayback.Get_Frame().Particles.front().World._41 -
				expectedLockedPosition) < 0.00001f &&
			std::abs(lockedPlayback.Get_Frame().Particles.front().World._42 -
				expectedLockedPosition) < 0.00001f &&
			std::abs(lockedPlayback.Get_Frame().Particles.front().World._43 -
				expectedLockedPosition) < 0.00001f;
		runner.Require(lockedExact,
			"Effect Raw Type XYZ Lock Reuses First UE Random Fraction");

		EFFECT_DOCUMENT_DESC colorDocument;
		colorDocument.strEffectAssetId = "effect.color.ownership.harness";
		colorDocument.strDisplayName = "Source Color Ownership Harness";
		EFFECT_ELEMENT_DESC colored = sourceElement("colored");
		colored.Detail.Color.vColorMultiply = { 0.5f, 2.f, 0.25f, 0.5f };
		colored.SourceRecipe.Bursts.push_back({ 0.f, 1u, 1u });
		colored.SourceRecipe.Modules.push_back(lifetime());
		EFFECT_SOURCE_MODULE_DESC colorModule;
		colorModule.strStableId = "color.start";
		colorModule.strClassName = "particlemodulecolor";
		colorModule.strObjectPath = "Harness.Color";
		colorModule.Distributions.push_back(distribution(
			"startcolor", { 2.f, 3.f, 4.f, 0.f },
			{ 2.f, 3.f, 4.f, 0.f }, 3u));
		colorModule.Distributions.push_back(distribution(
			"startalpha", { 0.5f, 0.f, 0.f, 0.f },
			{ 0.5f, 0.f, 0.f, 0.f }, 1u));
		colored.SourceRecipe.Modules.push_back(colorModule);
		colorDocument.Elements.push_back(colored);
		CEffectPlayback colorPlayback;
		const bool_t colorStaged =
			colorPlayback.Stage_Document(colorDocument, status);
		if (colorStaged)
			colorPlayback.Seek(1.f / 60.f, identity);
		const bool_t colorExact = colorStaged &&
			colorPlayback.Get_Frame().Particles.size() == 1u &&
			std::abs(colorPlayback.Get_Frame().Particles.front().Color.x - 1.f) <
				0.0001f &&
			std::abs(colorPlayback.Get_Frame().Particles.front().Color.y - 6.f) <
				0.0001f &&
			std::abs(colorPlayback.Get_Frame().Particles.front().Color.z - 1.f) <
				0.0001f &&
			std::abs(colorPlayback.Get_Frame().Particles.front().Color.w - 0.25f) <
				0.0001f;
		runner.Require(colorExact,
			"Effect Source Color And Detail Override Each Execute Once");

		EFFECT_DOCUMENT_DESC sizeDocument;
		sizeDocument.strEffectAssetId = "effect.size.exact.harness";
		sizeDocument.strDisplayName = "Source Size Unit Harness";
		EFFECT_ELEMENT_DESC sized = sourceElement("sized.mesh");
		sized.SourceRecipe.strRendererShape = "mesh";
		sized.SourceRecipe.Bursts.push_back({ 0.f, 1u, 1u });
		sized.SourceRecipe.Modules.push_back(lifetime());
		EFFECT_SOURCE_MODULE_DESC sizeModule;
		sizeModule.strStableId = "size.start";
		sizeModule.strClassName = "particlemodulesize";
		sizeModule.strObjectPath = "Harness.Size";
		sizeModule.Distributions.push_back(distribution(
			"startsize", { 100.f, 200.f, 300.f, 0.f },
			{ 100.f, 200.f, 300.f, 0.f }, 3u));
		sized.SourceRecipe.Modules.push_back(sizeModule);
		sized.ResourceBindings.push_back({
			"meshModel", "Effect/DimensionMaster/Meshes/bfm_q_crack_01.wmodel" });
		sizeDocument.Elements.push_back(sized);
		const std::filesystem::path sizeResourceRoot =
			resourceRootEnvironment.Was_Defined() &&
			!resourceRootEnvironment.Get_OriginalValue().empty() ?
			std::filesystem::path(resourceRootEnvironment.Get_OriginalValue()) :
			CProjectDataRoot::Get().parent_path() /
				L"Client" / L"Bin" / L"Resources";
		resourceRootEnvironment.Set(sizeResourceRoot.c_str());
		CEffectPlayback sizePlayback;
		const bool_t sizeStaged = sizePlayback.Stage_Document(sizeDocument, status);
		if (sizeStaged)
			sizePlayback.Seek(1.f / 60.f, identity);
		if (!sizeStaged || sizePlayback.Get_Frame().Particles.size() != 1u ||
			std::abs(sizePlayback.Get_Frame().Particles.front().World._11 - 1.f) >= 0.0001f ||
			std::abs(sizePlayback.Get_Frame().Particles.front().World._22 - 2.f) >= 0.0001f ||
			std::abs(sizePlayback.Get_Frame().Particles.front().World._33 - 3.f) >= 0.0001f)
		{
			std::cout << "[DETAIL] source size stage=" << sizeStaged
				<< " status=" << status
				<< " particles=" << sizePlayback.Get_Frame().Particles.size();
			if (!sizePlayback.Get_Frame().Particles.empty())
			{
				const float4x4_t& world =
					sizePlayback.Get_Frame().Particles.front().World;
				std::cout << " scale=" << world._11 << ',' << world._22
					<< ',' << world._33;
			}
			std::cout << '\n';
		}
		runner.Require(sizeStaged &&
			sizePlayback.Get_Frame().Particles.size() == 1u &&
			std::abs(sizePlayback.Get_Frame().Particles.front().World._11 - 1.f) < 0.0001f &&
			std::abs(sizePlayback.Get_Frame().Particles.front().World._22 - 2.f) < 0.0001f &&
			std::abs(sizePlayback.Get_Frame().Particles.front().World._33 - 3.f) < 0.0001f,
			"Effect Source Mesh Size Converts UE Units By Project 0.01 Scale");
		resourceRootEnvironment.Restore();

		EFFECT_DOCUMENT_DESC spaceDocument;
		spaceDocument.strEffectAssetId = "effect.space.exact.harness";
		spaceDocument.strDisplayName = "Space Exact Harness";
		for (const bool_t local : { true, false })
		{
			EFFECT_ELEMENT_DESC element = sourceElement(local ? "local" : "world");
			element.Detail.Particle.bLocalSpace = local;
			element.SourceRecipe.Bursts.push_back({ 0.f, 1u, 1u });
			element.SourceRecipe.Modules.push_back(lifetime());
			spaceDocument.Elements.push_back(element);
		}
		CEffectPlayback spacePlayback;
		runner.Require(spacePlayback.Stage_Document(spaceDocument, status),
			"Effect Local And World Source Document Stages");
		spacePlayback.Update(1.f / 60.f, identity);
		float4x4_t moved{};
		XMStoreFloat4x4(&moved, XMMatrixTranslation(10.f, 0.f, 0.f));
		spacePlayback.Update(1.f / 60.f, moved);
		f32_t localX = -1.f;
		f32_t worldX = -1.f;
		for (const EFFECT_EVALUATED_PARTICLE& particle :
			spacePlayback.Get_Frame().Particles)
		{
			if (particle.pElement->strElementId == "local") localX = particle.World._41;
			if (particle.pElement->strElementId == "world") worldX = particle.World._41;
		}
		runner.Require(std::abs(localX - 10.f) < 0.0001f &&
			std::abs(worldX) < 0.0001f,
			"Effect Local Particles Follow Current Root And World Particles Keep Spawn Root");

		EFFECT_DOCUMENT_DESC boneDocument;
		boneDocument.strEffectAssetId = "effect.bone.exact.harness";
		boneDocument.strDisplayName = "Bone Exact Harness";
		EFFECT_ELEMENT_DESC bone = sourceElement("bone");
		bone.SourceRecipe.Bursts.push_back({ 0.f, 1u, 1u });
		bone.SourceRecipe.Modules.push_back(lifetime());
		EFFECT_SOURCE_MODULE_DESC boneModule;
		boneModule.strStableId = "bone.socket";
		boneModule.strClassName = "particlemodulelocationbonesocket";
		boneModule.strObjectPath = "Harness.BoneSocket";
		boneModule.Literals.push_back(literalString(
			"sourcelocations[0].bonesocketname", "fx_r_hand"));
		bone.SourceRecipe.Modules.push_back(boneModule);
		boneDocument.Elements.push_back(bone);
		CEffectPlayback bonePlayback;
		runner.Require(bonePlayback.Stage_Document(boneDocument, status),
			"Effect Bone Socket Source Document Stages");
		float4x4_t hand{};
		XMStoreFloat4x4(&hand, XMMatrixTranslation(3.f, 2.f, 1.f));
		bonePlayback.Set_SourceAnchorWorlds({ { "fx_r_hand", hand } });
		bonePlayback.Seek(1.f / 60.f, identity);
		runner.Require(bonePlayback.Get_Frame().Particles.size() == 1u &&
			std::abs(bonePlayback.Get_Frame().Particles.front().World._41 - 3.f) < 0.0001f &&
			std::abs(bonePlayback.Get_Frame().Particles.front().World._42 - 2.f) < 0.0001f,
			"Effect Bone Socket Module Consumes Live Source Anchor Matrix");

		EFFECT_DOCUMENT_DESC eventDocument;
		eventDocument.strEffectAssetId = "effect.event.exact.harness";
		eventDocument.strDisplayName = "Event Exact Harness";
		EFFECT_ELEMENT_DESC generator = sourceElement("generator");
		generator.SourceRecipe.Bursts.push_back({ 0.f, 1u, 1u });
		generator.SourceRecipe.Modules.push_back(lifetime());
		EFFECT_SOURCE_MODULE_DESC eventGenerator;
		eventGenerator.strStableId = "event.generator";
		eventGenerator.strClassName = "particlemoduleeventgenerator";
		eventGenerator.strObjectPath = "Harness.EventGenerator";
		eventGenerator.Literals.push_back(literalString("events[0].type", "epet_spawn"));
		eventGenerator.Literals.push_back(literalString("events[0].customname", "aaa"));
		eventGenerator.Literals.push_back(literalNumber("events[0].frequency", 1));
		generator.SourceRecipe.Modules.push_back(eventGenerator);
		eventDocument.Elements.push_back(generator);
		EFFECT_ELEMENT_DESC receiver = sourceElement("receiver");
		receiver.SourceRecipe.Modules.push_back(lifetime());
		EFFECT_SOURCE_MODULE_DESC eventReceiver;
		eventReceiver.strStableId = "event.receiver";
		eventReceiver.strClassName = "particlemoduleeventreceiverspawn";
		eventReceiver.strObjectPath = "Harness.EventReceiver";
		eventReceiver.Literals.push_back(literalString("eventgeneratortype", "epet_spawn"));
		eventReceiver.Literals.push_back(literalString("eventname", "aaa"));
		eventReceiver.Distributions.push_back(distribution(
			"spawncount", { 2.f, 0.f, 0.f, 0.f },
			{ 2.f, 0.f, 0.f, 0.f }, 1u));
		receiver.SourceRecipe.Modules.push_back(eventReceiver);
		eventDocument.Elements.push_back(receiver);
		CEffectPlayback eventPlayback;
		runner.Require(eventPlayback.Stage_Document(eventDocument, status),
			"Effect Event Source Document Stages");
		eventPlayback.Seek(1.f / 60.f, identity);
		const size_t receiverCount = std::count_if(
			eventPlayback.Get_Frame().Particles.begin(),
			eventPlayback.Get_Frame().Particles.end(),
			[](const EFFECT_EVALUATED_PARTICLE& particle)
			{
				return particle.pElement->strElementId == "receiver";
			});
		runner.Require(receiverCount == 2u,
			"Effect Spawn Event Routes By Name And Type Into Receiver Spawn Count");

		const std::filesystem::path vectorRoot =
			std::filesystem::temp_directory_path() /
			"lostark-effect-vector-field-harness";
		const std::filesystem::path vectorPath = vectorRoot /
			"Effect" / "Harness" / "constant.wvectorfield";
		std::error_code vectorError;
		std::filesystem::create_directories(vectorPath.parent_path(), vectorError);
		{
			std::ofstream output(vectorPath, std::ios::binary | std::ios::trunc);
			const char magic[4] = { 'W', 'V', 'F', '1' };
			const uint32_t header[5] = { 1u, 2u, 2u, 2u, 8u };
			const float3_t sample = { 100.f, 0.f, 0.f };
			output.write(magic, sizeof(magic));
			output.write(reinterpret_cast<const char*>(header), sizeof(header));
			for (uint32_t index = 0u; index < 8u; ++index)
			{
				output.write(reinterpret_cast<const char*>(&sample), sizeof(sample));
			}
		}
		resourceRootEnvironment.Set(vectorRoot.c_str());
		EFFECT_DOCUMENT_DESC vectorDocument;
		vectorDocument.strEffectAssetId = "effect.vector.exact.harness";
		vectorDocument.strDisplayName = "Vector Exact Harness";
		EFFECT_ELEMENT_DESC vectorElement = sourceElement("vector");
		vectorElement.SourceRecipe.Bursts.push_back({ 0.f, 1u, 1u });
		vectorElement.SourceRecipe.Modules.push_back(lifetime());
		EFFECT_SOURCE_MODULE_DESC vectorModule;
		vectorModule.strStableId = "vector.field";
		vectorModule.strClassName = "particlemodulelocalvectorfield";
		vectorModule.strObjectPath = "Harness.VectorField";
		vectorModule.Literals.push_back(literalString(
			"vectorfield.assetid", "Effect/Harness/constant.wvectorfield"));
		for (const char axis : { 'x', 'y', 'z' })
		{
			vectorModule.Literals.push_back(literalNumber(
				std::string("relativescale3d.") + axis, 100.0));
		}
		vectorModule.Literals.push_back(literalNumber("intensity", 1.0));
		vectorModule.Literals.push_back(literalNumber("tightness", 1.0));
		vectorElement.SourceRecipe.Modules.push_back(vectorModule);
		vectorDocument.Elements.push_back(vectorElement);
		CEffectPlayback vectorPlayback;
		const bool_t vectorStaged = !vectorError &&
			vectorPlayback.Stage_Document(vectorDocument, status);
		if (vectorStaged)
			vectorPlayback.Seek(1.f / 60.f, identity);
		const bool_t vectorExecuted = vectorStaged &&
			vectorPlayback.Get_Frame().Particles.size() == 1u &&
			std::abs(vectorPlayback.Get_Frame().Particles.front().World._41 -
				(61.f / 3600.f)) < 0.00001f;
		runner.Require(vectorExecuted,
			"Effect Local Vector Field Loads Volume And Applies Trilinear Force");

		EFFECT_DOCUMENT_DESC missingVector = vectorDocument;
		missingVector.strEffectAssetId = "effect.vector.missing.harness";
		for (EFFECT_SOURCE_LITERAL_DESC& literal :
			missingVector.Elements.front().SourceRecipe.Modules.back().Literals)
		{
			if (literal.strPropertyPath == "vectorfield.assetid")
				literal.strString = "Effect/Harness/missing.wvectorfield";
		}
		runner.Require(
			!vectorPlayback.Stage_Document(missingVector, status) &&
			vectorPlayback.Get_Frame().Particles.size() == 1u &&
			std::abs(vectorPlayback.Get_Frame().Particles.front().World._41 -
				(61.f / 3600.f)) < 0.00001f,
			"Effect Invalid Vector Field Fails Staging And Preserves Commit");
		resourceRootEnvironment.Restore();
		std::filesystem::remove(vectorPath, vectorError);
		std::filesystem::remove(vectorPath.parent_path(), vectorError);
		std::filesystem::remove(vectorPath.parent_path().parent_path(), vectorError);
		std::filesystem::remove(vectorRoot, vectorError);
	}

	void Test_DimensionMasterSourceSemanticAssets(TEST_RUNNER& runner)
	{
		using namespace Client;
		SCOPED_ENVIRONMENT_VARIABLE resourceRootEnvironment(
			L"LOSTARK_RESOURCE_ROOT");
		constexpr std::size_t iExpectedSeededModules = 162u;
		constexpr std::size_t iExpectedSeedMetadataDeclared = 103u;
		constexpr std::size_t iExpectedVectorModules = 4u;
		constexpr std::size_t iExpectedVectorAssets = 2u;
		constexpr std::size_t iExpectedBoneSocketModules = 0u;
		constexpr std::size_t iExpectedEventGenerators = 0u;
		constexpr std::size_t iExpectedEventReceivers = 0u;
		const std::filesystem::path repositoryRoot =
			CProjectDataRoot::Get().parent_path();
		const std::filesystem::path resourceRoot =
			resourceRootEnvironment.Was_Defined() &&
			!resourceRootEnvironment.Get_OriginalValue().empty() ?
			std::filesystem::path(resourceRootEnvironment.Get_OriginalValue()) :
			repositoryRoot / L"Client" / L"Bin" / L"Resources";
		resourceRootEnvironment.Set(resourceRoot.c_str());
		std::vector<uint32_t> skillIds;
		std::vector<std::string> expectedEffectIds;
		std::string status;
		const bool_t rosterLoaded = Collect_DimensionMasterEffectRoster(
			skillIds, expectedEffectIds, status);
		bool_t bLoadedAndStaged = true;
		size_t iSeededModules = 0u;
		size_t iSeedMetadataDeclared = 0u;
		size_t iSeedMetadataComplete = 0u;
		size_t iVectorModules = 0u;
		size_t iBoundVectorModules = 0u;
		size_t iBoneSocketModules = 0u;
		size_t iEventGenerators = 0u;
		size_t iEventReceivers = 0u;
		std::set<std::string> vectorAssetIds;
		for (const uint32_t skillId : skillIds)
		{
			const std::filesystem::path path = CProjectDataRoot::Resolve(
				L"Effects/Imported/DimensionMaster/Converted/"
				L"effect.dimensionmaster.skill." + std::to_wstring(skillId) +
				L".imported.effect.json");
			EFFECT_DOCUMENT_DESC document;
			CEffectPlayback playback;
			const bool_t loaded = !path.empty() &&
				CEffectDocumentCodec::Load(path, document, status);
			const bool_t staged = loaded &&
				playback.Stage_Document(document, status);
			bLoadedAndStaged = bLoadedAndStaged && staged;
			if (!staged)
			{
				std::cout << "[DETAIL] DimensionMaster semantic stage " <<
					skillId << ": " << path.string() << " status=" << status << '\n';
			}
			if (!loaded)
				continue;
			for (const EFFECT_ELEMENT_DESC& element : document.Elements)
			{
				for (const EFFECT_SOURCE_MODULE_DESC& module :
					element.SourceRecipe.Modules)
				{
					if (module.strClassName.ends_with("_seeded"))
					{
						++iSeededModules;
						std::set<std::string> seedFlags;
						for (const EFFECT_SOURCE_LITERAL_DESC& literal : module.Literals)
						{
							if (literal.strPropertyPath.starts_with(
								"randomseedinfo.b"))
							{
								seedFlags.insert(literal.strPropertyPath);
							}
						}
						if (!seedFlags.empty())
							++iSeedMetadataDeclared;
						if (seedFlags.contains(
							"randomseedinfo.bresetseedonemitterlooping") &&
							seedFlags.contains(
								"randomseedinfo.brandomlyselectseedarray") &&
							seedFlags.contains(
								"randomseedinfo.bgetseedfrominstance") &&
							seedFlags.contains(
								"randomseedinfo.binstanceseedisindex"))
						{
							++iSeedMetadataComplete;
						}
					}
					if (module.strClassName == "particlemodulelocationbonesocket")
						++iBoneSocketModules;
					if (module.strClassName == "particlemoduleeventgenerator")
						++iEventGenerators;
					if (module.strClassName == "particlemoduleeventreceiverspawn")
						++iEventReceivers;
					if (module.strClassName != "particlemodulelocalvectorfield")
						continue;
					++iVectorModules;
					for (const EFFECT_SOURCE_LITERAL_DESC& literal : module.Literals)
					{
						if (literal.strPropertyPath != "vectorfield.assetid" ||
							literal.eKind != EFFECT_SOURCE_LITERAL_KIND::STRING)
						{
							continue;
						}
						const std::filesystem::path resolved =
							resourceRoot / std::filesystem::path(literal.strString);
						if (std::filesystem::is_regular_file(resolved))
							++iBoundVectorModules;
						vectorAssetIds.insert(literal.strString);
					}
				}
			}
		}
		if (!rosterLoaded || !bLoadedAndStaged ||
			iSeededModules != iExpectedSeededModules ||
			iSeedMetadataDeclared != iExpectedSeedMetadataDeclared ||
			iSeedMetadataComplete != iExpectedSeedMetadataDeclared ||
			iBoneSocketModules != iExpectedBoneSocketModules ||
			iEventGenerators != iExpectedEventGenerators ||
			iEventReceivers != iExpectedEventReceivers ||
			iVectorModules != iExpectedVectorModules ||
			iBoundVectorModules != iExpectedVectorModules ||
			vectorAssetIds.size() != iExpectedVectorAssets)
		{
			std::cout << "[DETAIL] DimensionMaster semantic counts seeded=" <<
				iSeededModules << " seedDeclared=" << iSeedMetadataDeclared <<
				" seedComplete=" << iSeedMetadataComplete <<
				" bone=" << iBoneSocketModules << " eventGenerator=" <<
				iEventGenerators << " eventReceiver=" << iEventReceivers <<
				" vector=" << iVectorModules << " vectorBound=" <<
				iBoundVectorModules << " vectorAssets=" << vectorAssetIds.size() << '\n';
		}
		runner.Require(rosterLoaded &&
			expectedEffectIds.size() >= skillIds.size() && bLoadedAndStaged &&
			iSeededModules == iExpectedSeededModules &&
			iSeedMetadataDeclared == iExpectedSeedMetadataDeclared &&
			iSeedMetadataComplete == iExpectedSeedMetadataDeclared &&
			iBoneSocketModules == iExpectedBoneSocketModules &&
			iEventGenerators == iExpectedEventGenerators &&
			iEventReceivers == iExpectedEventReceivers,
			"Current DimensionMaster Source Documents Preserve Exact Semantic Module Counts");
		runner.Require(iVectorModules == iExpectedVectorModules &&
			iBoundVectorModules == iExpectedVectorModules &&
			vectorAssetIds.size() == iExpectedVectorAssets,
			"Current DimensionMaster Vector Field Occurrences Resolve Exact Source Volumes");
		bool_t bAllEffectsAuthoredStage = true;
		size_t iAuthoredStageCount = 0u;
		for (const uint32_t skillId : skillIds)
		{
			const std::filesystem::path path = CProjectDataRoot::Resolve(
				L"Effects/Authored/effect.dimensionmaster.skill." +
				std::to_wstring(skillId) + L".effect.json");
			EFFECT_DOCUMENT_DESC document;
			CEffectPlayback playback;
			const bool_t staged = !path.empty() &&
				CEffectDocumentCodec::Load(path, document, status) &&
				CEffectDocumentCodec::Validate_Drawable(document, status) &&
				playback.Stage_Document(document, status);
			bAllEffectsAuthoredStage = bAllEffectsAuthoredStage && staged;
			if (staged)
				++iAuthoredStageCount;
			else
				std::cout << "[DETAIL] All Effects authored stage " << skillId
					<< ": " << status << '\n';
		}
		runner.Require(bAllEffectsAuthoredStage &&
			iAuthoredStageCount == skillIds.size(),
			"All Effects Stages Every Current DimensionMaster Authored Document");

		const std::filesystem::path aPath = CProjectDataRoot::Resolve(
			L"Effects/Authored/effect.dimensionmaster.skill.2050240.effect.json");
		EFFECT_DOCUMENT_DESC aDocument;
		EFFECT_DOCUMENT_DESC aRoundTrip;
		const bool_t aLoaded = !aPath.empty() &&
			CEffectDocumentCodec::Load(aPath, aDocument, status);
		std::set<std::string> aProfileIds;
		size_t aParticleCount = 0u;
		size_t aEnabledProfileCount = 0u;
		size_t aReconstructedProfileCount = 0u;
		size_t aSpecializedShaderCount = 0u;
		size_t aDynamicSemanticElementCount = 0u;
		size_t aSubUVElementCount = 0u;
		size_t aSourceModuleOccurrenceCount = 0u;
		size_t aCookedLookupCount = 0u;
		size_t aMalformedLookupCount = 0u;
		size_t aXyzRandomLockCount = 0u;
		size_t aSourceColorElementCount = 0u;
		size_t aSourceColorOverrideViolationCount = 0u;
		size_t aSourceSubUVOverrideViolationCount = 0u;
		bool_t aEmitter17SizeExact = false;
		bool_t aEmitter32SizeExact = false;
		bool_t aEmitter64SizeExact = false;
		std::set<std::string> aSourceModuleClasses;
		for (const EFFECT_ELEMENT_DESC& element : aDocument.Elements)
		{
			if (element.eKind != EFFECT_ELEMENT_KIND::PARTICLE)
				continue;
			++aParticleCount;
			aSourceModuleOccurrenceCount += element.SourceRecipe.Modules.size();
			bool_t bHasSourceColor = false;
			bool_t bHasSourceSubUV = false;
			for (const EFFECT_SOURCE_MODULE_DESC& module :
				element.SourceRecipe.Modules)
			{
				aSourceModuleClasses.insert(module.strClassName);
				bHasSourceColor = bHasSourceColor ||
					module.strClassName.starts_with("particlemodulecolor");
				bHasSourceSubUV = bHasSourceSubUV ||
					module.strClassName == "particlemodulesubuv";
				for (const EFFECT_DISTRIBUTION_DESC& distribution :
					module.Distributions)
				{
					if (distribution.iRandomLockAxes == 4u)
						++aXyzRandomLockCount;
					if (distribution.LookupTable.empty())
						continue;
					++aCookedLookupCount;
					std::string distributionStatus;
					if (!CEffectDistribution::Validate(
							distribution, distributionStatus))
					{
						++aMalformedLookupCount;
					}
					if (distribution.strPropertyPath != "startsize")
						continue;
					const float4_t value = CEffectDistribution::Evaluate(
						distribution, 0.f, float4_t{});
					if (element.strElementId.ends_with("particlespriteemitter_17"))
					{
						aEmitter17SizeExact = std::abs(value.x - 500.f) < 0.0001f &&
							std::abs(value.y) < 0.0001f;
					}
					else if (element.strElementId.ends_with("particlespriteemitter_32"))
					{
						aEmitter32SizeExact = std::abs(value.x - 250.f) < 0.0001f &&
							std::abs(value.y + 100.f) < 0.0001f;
					}
					else if (element.strElementId.ends_with("particlespriteemitter_64"))
					{
						aEmitter64SizeExact = std::abs(value.x - 60.f) < 0.0001f &&
							std::abs(value.y - 180.f) < 0.0001f;
					}
				}
			}
			if (bHasSourceColor)
			{
				++aSourceColorElementCount;
				const float4_t& multiply = element.Detail.Color.vColorMultiply;
				if (std::abs(multiply.x - 1.f) >= 0.0001f ||
					std::abs(multiply.y - 1.f) >= 0.0001f ||
					std::abs(multiply.z - 1.f) >= 0.0001f ||
					std::abs(multiply.w - 1.f) >= 0.0001f ||
					element.Detail.LinearLerp.bColorMultiply)
				{
					++aSourceColorOverrideViolationCount;
				}
			}
			if (bHasSourceSubUV &&
				(element.Detail.UV.bSequence ||
					element.Detail.UV.iTileColumns != 1 ||
					element.Detail.UV.iTileRows != 1 ||
					element.Detail.UV.iTileIndex != 0))
			{
				++aSourceSubUVOverrideViolationCount;
			}
			const EFFECT_SOURCE_MATERIAL_DESC& source =
				element.Material.SourceMaterial;
			if (!source.bEnabled)
				continue;
			++aEnabledProfileCount;
			aProfileIds.insert(source.strProfileId);
			if (source.eStatus ==
				EFFECT_SOURCE_MATERIAL_STATUS::RECONSTRUCTED_PROFILE)
			{
				++aReconstructedProfileCount;
			}
			if (source.strRuntimeShaderProfileId !=
				"effect.ue3.reconstructed-standard.v1")
			{
				++aSpecializedShaderCount;
			}
			if (std::any_of(source.DynamicParameterSemantics.begin(),
				source.DynamicParameterSemantics.end(),
				[](const std::string& semantic)
				{
					return semantic != "unbound";
				}))
			{
				++aDynamicSemanticElementCount;
			}
			if (source.strSubUVMode != "none")
				++aSubUVElementCount;
		}
		const std::string aSerialized = aLoaded ?
			CEffectDocumentCodec::Serialize(aDocument) : std::string{};
		const bool_t aRoundTripped = aLoaded &&
			CEffectDocumentCodec::Parse(aSerialized, aRoundTrip, status) &&
			CEffectDocumentCodec::Serialize(aRoundTrip) == aSerialized;
		runner.Require(aLoaded && aDocument.iFormatVersion == 11u &&
			aDocument.Elements.size() == 51u && aParticleCount == 46u &&
			aEnabledProfileCount == 46u &&
			aReconstructedProfileCount == 46u &&
			aProfileIds.size() == 21u && aSpecializedShaderCount == 14u &&
			aDynamicSemanticElementCount == 32u &&
			aSubUVElementCount == 2u &&
			aSourceModuleOccurrenceCount == 518u &&
			aSourceModuleClasses.size() == 28u && aRoundTripped,
			"DimensionMaster A V11 Source Material Profiles Round Trip Losslessly");
		runner.Require(aCookedLookupCount == 604u &&
			aMalformedLookupCount == 0u && aEmitter17SizeExact &&
			aEmitter32SizeExact && aEmitter64SizeExact &&
			aXyzRandomLockCount == 13u,
			"DimensionMaster A Cooked Distribution Payloads And Sprite Sizes Are Exact");
		runner.Require(aSourceColorElementCount == 46u &&
			aSourceColorOverrideViolationCount == 0u &&
			aSourceSubUVOverrideViolationCount == 0u,
			"DimensionMaster A Source Color And SubUV Baselines Execute Once");
		resourceRootEnvironment.Restore();
	}

	void Test_DimensionMasterImportedPortalDraft(TEST_RUNNER& runner)
	{
		using namespace Client;
		std::string status;
		std::vector<uint32_t> skillIds;
		std::vector<std::string> expectedEffectIds;
		const bool_t rosterLoaded = Collect_DimensionMasterEffectRoster(
			skillIds, expectedEffectIds, status);
		bool_t importedContractsHold = rosterLoaded;
		std::size_t importedContractCount = 0u;
		std::size_t pendingProfileCount = 0u;
		for (const uint32_t skillId : skillIds)
		{
			status.clear();
			const std::filesystem::path path = CProjectDataRoot::Resolve(
				L"Effects/Imported/DimensionMaster/Converted/"
				L"effect.dimensionmaster.skill." + std::to_wstring(skillId) +
				L".imported.effect.json");
			const std::string rawText = Read_Text(path);
			DATA_JSON_VALUE rawDocument;
			DATA_JSON_PARSE_LIMITS rawLimits;
			rawLimits.iMaximumBytes = 64u * 1024u * 1024u;
			rawLimits.iMaximumDepth = 64u;
			rawLimits.iMaximumValues = 3'000'000u;
			const DATA_JSON_VALUE* rawElements = nullptr;
			const DATA_JSON_VALUE* rawEffectId = nullptr;
			const bool_t rawCaptured = !rawText.empty() &&
				CDataJson::Parse(rawText, rawDocument, status, rawLimits) &&
				rawDocument.Is_Object() &&
				nullptr != (rawElements = rawDocument.Find("elements")) &&
				rawElements->Is_Array() && !rawElements->Get_Array().empty() &&
				nullptr != (rawEffectId = rawDocument.Find("effectAssetId")) &&
				rawEffectId->Is_String() && rawEffectId->Get_String() ==
					"effect.dimensionmaster.skill." + std::to_string(skillId) +
					".imported";
			bool_t hasPendingSourceProfile = false;
			if (rawCaptured)
			{
				for (const DATA_JSON_VALUE& rawElement : rawElements->Get_Array())
				{
					if (!rawElement.Is_Object())
						continue;
					const DATA_JSON_VALUE* material = rawElement.Find("material");
					if (nullptr == material || !material->Is_Object())
						continue;
					const DATA_JSON_VALUE* templateId = material->Find("templateId");
					if (nullptr == templateId || !templateId->Is_String() ||
						templateId->Get_String() != "effect.source_material")
					{
						continue;
					}
					const DATA_JSON_VALUE* sourceProfile =
						material->Find("sourceProfile");
					const DATA_JSON_VALUE* enabled =
						nullptr != sourceProfile && sourceProfile->Is_Object() ?
						sourceProfile->Find("enabled") : nullptr;
					hasPendingSourceProfile = nullptr == enabled ||
						!enabled->Is_Boolean() || !enabled->Get_Boolean();
					if (hasPendingSourceProfile)
						break;
				}
			}

			EFFECT_DOCUMENT_DESC importedDocument;
			const bool_t importedLoaded = rawCaptured &&
				CEffectDocumentCodec::Load(path, importedDocument, status);
			bool_t contractHolds = false;
			if (importedLoaded)
			{
				EFFECT_DOCUMENT_DESC roundTrip;
				const std::string serialized =
					CEffectDocumentCodec::Serialize(importedDocument);
				const bool_t roundTripped =
					!importedDocument.Elements.empty() &&
					CEffectDocumentCodec::Parse(serialized, roundTrip, status) &&
					CEffectDocumentCodec::Serialize(roundTrip) == serialized;
				if (hasPendingSourceProfile)
				{
					contractHolds = roundTripped ||
						status.find("requires a staged profile") !=
							std::string::npos;
					if (contractHolds)
						++pendingProfileCount;
				}
				else
				{
					contractHolds = roundTripped;
				}
			}
			else if (rawCaptured && hasPendingSourceProfile &&
				status.find("requires a staged profile") != std::string::npos)
			{
				contractHolds = true;
				++pendingProfileCount;
			}
			importedContractsHold = importedContractsHold && contractHolds;
			if (contractHolds)
				++importedContractCount;
			else
				std::cout << "[DETAIL] DimensionMaster imported document " <<
					skillId << ": " << path.string() << " rawCaptured=" <<
					rawCaptured << " status=" << status << '\n';
		}
		runner.Require(importedContractsHold && pendingProfileCount > 0u &&
			expectedEffectIds.size() >= skillIds.size() &&
			importedContractCount == skillIds.size(),
			"Every Current Imported Baseline Round Trips And Preserves Pending Materialization State");

		const std::filesystem::path authoredTPath = CProjectDataRoot::Resolve(
			L"Effects/Authored/effect.dimensionmaster.skill.2050500.effect.json");
		EFFECT_DOCUMENT_DESC authoredT;
		const bool_t authoredTLoaded = !authoredTPath.empty() &&
			CEffectDocumentCodec::Load(authoredTPath, authoredT, status);
		std::vector<std::string> resourceIds;
		CEffectDocumentCodec::Collect_ResourceAssetIds(authoredT, resourceIds);
		const bool_t hasSummonDependency = std::find(resourceIds.begin(),
			resourceIds.end(),
			"Character/DimensionMaster/DimensionMaster_DimensionSummon.wmodel") !=
			resourceIds.end();
		CEffectPlayback completePlayback;
		const bool_t playbackStaged = authoredTLoaded &&
			completePlayback.Stage_Document(authoredT, status);
		EFFECT_DOCUMENT_DESC roundTrip;
		const bool_t roundTripped = authoredTLoaded &&
			CEffectDocumentCodec::Parse(
				CEffectDocumentCodec::Serialize(authoredT), roundTrip, status);
		runner.Require(authoredTLoaded && authoredT.ModelCues.size() == 1u &&
			hasSummonDependency &&
			playbackStaged && completePlayback.Get_DurationSeconds() >= 5.4583f &&
			roundTripped && roundTrip.ModelCues.size() == 1u &&
			roundTrip.ParticleSystem.fUniformScaleMultiplier == 1.f &&
			roundTrip.ParticleSystem.fInitialSpeedMultiplier == 1.f &&
			roundTrip.ModelCues.front().strClipName ==
				"sk_swp_dms_00_sk_sk_dimensionprison",
			"Canonical DimensionMaster T Round Trips Summon Model Cue");

		if (authoredTLoaded)
		{
			EFFECT_DOCUMENT_DESC invalidCue = authoredT;
			invalidCue.ModelCues.push_back(invalidCue.ModelCues.front());
			const f32_t committedDuration =
				completePlayback.Get_DurationSeconds();
			runner.Require(
				!completePlayback.Stage_Document(invalidCue, status) &&
				completePlayback.Get_DurationSeconds() == committedDuration,
				"Duplicate Model Cue Stage Preserves Committed Effect Playback");
		}
	}

	void Test_EffectAllEffectsAuthoringJoin(TEST_RUNNER& runner)
	{
		using namespace Client;
		std::string status;
		bool_t valid = CPlayerSkillCatalog::Load(status);
		if (!valid)
			std::cout << "[DETAIL] PlayerSkills load: " << status << '\n';
		std::size_t mappedCount = 0u;
		for (const PLAYER_SKILL_DEFINITION& skill :
			CPlayerSkillCatalog::Get_Skills())
		{
			if (skill.strEffectId.empty())
				continue;
			++mappedCount;
			const std::filesystem::path path = CProjectDataRoot::Resolve(
				std::filesystem::path(L"Effects") / L"Authored" /
				(std::filesystem::path(skill.strEffectId).wstring() +
					L".effect.json"));
			EFFECT_DOCUMENT_DESC document;
			const bool_t joined = !path.empty() &&
				CEffectDocumentCodec::Load(path, document, status) &&
				document.strEffectAssetId == skill.strEffectId;
			if (!joined)
			{
				std::cout << "[DETAIL] Effect join: " << skill.strEffectId
					<< " path=" << path.string() << " status=" << status << '\n';
			}
			valid = valid && joined;
		}
		runner.Require(valid && 0u != mappedCount,
			"All Effects Joins Every PlayerSkills EffectId To Valid Authored Document");
	}

	void Test_EffectDraftAtomicSave(TEST_RUNNER& runner)
	{
		using namespace Client;
		EFFECT_DOCUMENT_DESC document;
		document.strEffectAssetId = "effect.draft.harness";
		document.strDisplayName = "Draft Harness";
		EFFECT_ELEMENT_DESC sprite;
		sprite.strElementId = "unbound_sprite";
		sprite.strDisplayName = "Unbound Sprite";
		sprite.strGroupId = "harness_group";
		sprite.strSourceNode = "Harness/Node/0";
		sprite.bVisible = false;
		sprite.eKind = EFFECT_ELEMENT_KIND::SPRITE;
		document.Elements.push_back(sprite);

		std::string status;
		runner.Require(
			CEffectDocumentCodec::Validate(document, status) &&
			!CEffectDocumentCodec::Validate_Drawable(document, status),
			"Effect Authoring Distinguishes Valid Draft From Drawable Document");
		EFFECT_DOCUMENT_DESC unknownTemplate = document;
		unknownTemplate.Elements.front().Material.strTemplateId =
			"effect.unproven.custom";
		runner.Require(
			!CEffectDocumentCodec::Validate(unknownTemplate, status),
			"Effect Authoring Rejects Unregistered Material Template");

		EFFECT_DOCUMENT_DESC sourceRecipeDocument = document;
		EFFECT_ELEMENT_DESC& sourceElement =
			sourceRecipeDocument.Elements.front();
		sourceElement.SourceRecipe.bEnabled = true;
		sourceElement.SourceRecipe.strRendererShape = "sprite";
		sourceElement.SourceRecipe.fEmitterDurationSeconds = 1.f;
		EFFECT_SOURCE_MODULE_DESC sourceModule;
		sourceModule.strStableId = "harness:module:spawn";
		sourceModule.strClassName = "particlemodulespawn";
		sourceModule.strObjectPath = "Harness.ParticleModuleSpawn";
		EFFECT_DISTRIBUTION_DESC sourceRate;
		sourceRate.strPropertyPath = "rate";
		sourceRate.iComponentCount = 1u;
		sourceRate.iOperation = 1u;
		sourceRate.vDefaultMinimum.x = 12.f;
		sourceRate.vDefaultMaximum.x = 12.f;
		sourceModule.Distributions.push_back(sourceRate);
		sourceElement.SourceRecipe.Modules.push_back(sourceModule);
		EFFECT_DOCUMENT_DESC sourceRoundTrip;
		const float4_t evaluatedRate = CEffectDistribution::Evaluate(
			sourceRate, 0.5f, 0.25f);
		runner.Require(
			CEffectDocumentCodec::Validate(sourceRecipeDocument, status) &&
			CEffectDocumentCodec::Parse(
				CEffectDocumentCodec::Serialize(sourceRecipeDocument),
				sourceRoundTrip, status) &&
			sourceRoundTrip.Elements.front().SourceRecipe.bEnabled &&
			sourceRoundTrip.Elements.front().SourceRecipe.Modules.size() == 1u &&
			evaluatedRate.x == 12.f,
			"Effect V9 Source Recipe And Distribution Round Trip Losslessly");

		std::string legacyV7 = CEffectDocumentCodec::Serialize(document);
		const std::string currentVersion = "\"version\": " +
			std::to_string(EFFECT_AUTHORING_FORMAT_VERSION);
		const size_t versionOffset = legacyV7.find(currentVersion);
		if (std::string::npos != versionOffset)
			legacyV7.replace(versionOffset, currentVersion.size(), "\"version\": 7");
		const size_t particleSystemOffset = legacyV7.find(
			"  \"particleSystem\":");
		if (std::string::npos != particleSystemOffset)
		{
			const size_t nextField = legacyV7.find(
				"  \"modelCues\":", particleSystemOffset);
			if (std::string::npos != nextField)
				legacyV7.erase(particleSystemOffset,
					nextField - particleSystemOffset);
		}
		EFFECT_DOCUMENT_DESC upgradedV7;
		runner.Require(std::string::npos != versionOffset &&
			CEffectDocumentCodec::Parse(legacyV7, upgradedV7, status) &&
			upgradedV7.iFormatVersion == EFFECT_AUTHORING_FORMAT_VERSION &&
			upgradedV7.ParticleSystem.fUniformScaleMultiplier == 1.f &&
			upgradedV7.ParticleSystem.fYawOffsetDegrees == 0.f &&
			upgradedV7.ParticleSystem.fDirectionYawDegrees == 0.f &&
			upgradedV7.ParticleSystem.fInitialSpeedMultiplier == 1.f,
			"Effect Authoring Upgrades V7 To Identity Particle System");

		const std::filesystem::path path =
			std::filesystem::temp_directory_path() /
			"lostark-effect-draft-harness.effect.json";
		std::error_code error;
		std::filesystem::remove(path, error);
		EFFECT_DOCUMENT_DESC loaded;
		runner.Require(
			CEffectDocumentCodec::Save_Atomic(path, document, status) &&
			CEffectDocumentCodec::Load(path, loaded, status) &&
			loaded.strEffectAssetId == document.strEffectAssetId &&
			loaded.Elements.size() == 1u &&
			loaded.Elements.front().strDisplayName == "Unbound Sprite" &&
			loaded.Elements.front().strGroupId == "harness_group" &&
			loaded.Elements.front().strSourceNode == "Harness/Node/0" &&
			!loaded.Elements.front().bVisible &&
		loaded.Elements.front().Material.strTemplateId == "effect.standard" &&
		loaded.Elements.front().Detail.Particle.vInitialPositionMin.x == 0.f &&
		loaded.Elements.front().Detail.Particle.vInitialPositionMax.x == 0.f &&
		loaded.Elements.front().ResourceBindings.empty(),
			"Effect Authoring Atomically Saves And Reloads V9 Metadata Draft");
		std::filesystem::remove(path, error);
	}

	void Test_EffectAssemblyRuntimeCatalog(
		TEST_RUNNER& runner,
		const bool_t bCheckFailedReloadRollback = true)
	{
		using namespace Client;
		SCOPED_ENVIRONMENT_VARIABLE resourceRootEnvironment(
			L"LOSTARK_RESOURCE_ROOT");
		wchar_t moduleBuffer[32768]{};
		const DWORD moduleLength = GetModuleFileNameW(
			nullptr, moduleBuffer, static_cast<DWORD>(std::size(moduleBuffer)));
		const std::filesystem::path moduleDirectory =
			0u == moduleLength || moduleLength >= std::size(moduleBuffer) ?
			std::filesystem::path{} :
			std::filesystem::path(moduleBuffer).parent_path();
		const std::filesystem::path repositoryRoot =
			CProjectDataRoot::Get().parent_path();
		const std::filesystem::path sourceCatalog = repositoryRoot /
			L"Client" / L"Bin" / L"DataFiles" / L"Effect" /
			L"EffectCatalog.runtime.json";
		const std::filesystem::path stagedCatalog = moduleDirectory /
			L"DataFiles" / L"Effect" / L"EffectCatalog.runtime.json";
		std::error_code error;
		std::filesystem::create_directories(stagedCatalog.parent_path(), error);
		error.clear();
		std::filesystem::copy_file(sourceCatalog, stagedCatalog,
			std::filesystem::copy_options::overwrite_existing, error);
		const std::filesystem::path resourceRoot =
			resourceRootEnvironment.Was_Defined() &&
			!resourceRootEnvironment.Get_OriginalValue().empty() ?
			std::filesystem::path(resourceRootEnvironment.Get_OriginalValue()) :
			repositoryRoot / L"Client" / L"Bin" / L"Resources";
		resourceRootEnvironment.Set(resourceRoot.c_str());

		CEffectCatalog::Clear();
		std::string status;
		DIMENSIONMASTER_EFFECT_BUILD_CONTRACT buildContract;
		std::string buildContractStatus;
		const bool_t buildContractLoaded =
			Load_DimensionMasterEffectBuildContract(
				buildContract, buildContractStatus);
		std::vector<uint32_t> currentSkillIds;
		std::vector<std::string> expectedEffectIds;
		const bool_t rosterLoaded = Collect_DimensionMasterEffectRoster(
			currentSkillIds, expectedEffectIds, status);
		const bool_t loaded = !error && CEffectCatalog::Load(status);
		if (!loaded)
			std::cout << "[DETAIL] Effect runtime catalog: " << status << '\n';
		const std::vector<std::string> effectIds =
			CEffectCatalog::Get_EffectAssetIds();
		const std::set<std::string> expectedEffectIdSet(
			expectedEffectIds.begin(), expectedEffectIds.end());
		const std::set<std::string> actualEffectIdSet(
			effectIds.begin(), effectIds.end());
		std::set<std::string> contractEffectIds;
		for (const auto& entry : buildContract.Effects)
			contractEffectIds.insert(entry.first);
		bool_t hierarchyComplete = rosterLoaded && !currentSkillIds.empty() && loaded &&
			buildContractLoaded && expectedEffectIdSet == actualEffectIdSet &&
			expectedEffectIdSet == contractEffectIds &&
			effectIds.size() == buildContract.iEffectCount;
		bool_t playbackStageComplete = loaded;
		size_t stagedEffectCount = 0u;
		size_t assemblyEmitterCount = 0u;
		std::set<std::string> componentIds;
		for (const std::string& effectId : effectIds)
		{
			const std::shared_ptr<const EFFECT_ASSEMBLY_DESC> assembly =
				CEffectCatalog::Find_Assembly(effectId);
			const std::shared_ptr<const EFFECT_DOCUMENT_DESC> document =
				CEffectCatalog::Find(effectId);
			hierarchyComplete = hierarchyComplete && nullptr != assembly &&
				nullptr != document && !assembly->ComponentCues.empty();
			const auto expectedBuild = buildContract.Effects.find(effectId);
			hierarchyComplete = hierarchyComplete &&
				expectedBuild != buildContract.Effects.end() &&
				nullptr != assembly && expectedBuild != buildContract.Effects.end() &&
				assembly->ComponentCues.size() ==
					expectedBuild->second.iComponentCount;
			size_t effectEmitterCount = 0u;
			if (nullptr != document)
			{
				CEffectPlayback playback;
				std::string playbackStatus;
				const bool_t staged =
					playback.Stage_Document(*document, playbackStatus);
				playbackStageComplete = playbackStageComplete && staged;
				if (staged)
					++stagedEffectCount;
				else
					std::cout << "[DETAIL] Runtime playback stage " << effectId
						<< ": " << playbackStatus << '\n';
			}
			if (nullptr == assembly)
				continue;
			for (const EFFECT_COMPONENT_CUE_DESC& cue : assembly->ComponentCues)
			{
				const std::shared_ptr<const EFFECT_COMPONENT_DESC> component =
					CEffectCatalog::Find_Component(cue.strComponentAssetId);
				hierarchyComplete = hierarchyComplete && nullptr != component &&
					!component->Emitters.empty() &&
					component->Emitters.size() ==
						component->Document.Elements.size();
				if (nullptr != component)
					effectEmitterCount += component->Emitters.size();
				hierarchyComplete = hierarchyComplete &&
					componentIds.insert(cue.strComponentAssetId).second;
			}
			hierarchyComplete = hierarchyComplete &&
				expectedBuild != buildContract.Effects.end() &&
				effectEmitterCount == expectedBuild->second.iEmitterCount;
			assemblyEmitterCount += effectEmitterCount;
		}
		const std::vector<std::string> catalogComponentIds =
			CEffectCatalog::Get_ComponentAssetIds();
		const std::set<std::string> catalogComponentIdSet(
			catalogComponentIds.begin(), catalogComponentIds.end());
		const std::shared_ptr<const EFFECT_DOCUMENT_DESC> tDocument =
			CEffectCatalog::Find("effect.dimensionmaster.skill.2050500");
		const bool_t canonicalTModelCue = nullptr != tDocument &&
			std::any_of(tDocument->ModelCues.begin(), tDocument->ModelCues.end(),
				[](const EFFECT_MODEL_CUE_DESC& cue)
				{
					return cue.strClipName ==
						"sk_swp_dms_00_sk_sk_dimensionprison";
				});
		if (!buildContractLoaded ||
			effectIds.size() != buildContract.iEffectCount ||
			catalogComponentIds.size() != buildContract.iComponentCount ||
			assemblyEmitterCount != buildContract.iEmitterCount)
		{
			std::cout << "[DETAIL] Effect build contract: " <<
				buildContractStatus << " effects=" << effectIds.size() << '/' <<
				buildContract.iEffectCount << " components=" <<
				catalogComponentIds.size() << '/' << buildContract.iComponentCount <<
				" emitters=" << assemblyEmitterCount << '/' <<
				buildContract.iEmitterCount << '\n';
		}
		runner.Require(
			hierarchyComplete && componentIds == catalogComponentIdSet &&
			catalogComponentIds.size() == buildContract.iComponentCount &&
			assemblyEmitterCount == buildContract.iEmitterCount && canonicalTModelCue,
			"Effect Runtime Matches Generated Assembly Component Emitter Contract And Canonical T Cue");
		runner.Require(playbackStageComplete &&
			stagedEffectCount == expectedEffectIds.size(),
			"Effect Runtime Stages Every Current Compiled DimensionMaster Document");
		bool_t componentStageComplete = true;
		size_t stagedComponentCount = 0u;
		for (const std::string& componentId : catalogComponentIds)
		{
			const std::shared_ptr<const EFFECT_COMPONENT_DESC> component =
				CEffectCatalog::Find_Component(componentId);
			CEffectPlayback playback;
			std::string playbackStatus;
			const bool_t staged = nullptr != component &&
				playback.Stage_Document(component->Document, playbackStatus);
			componentStageComplete = componentStageComplete && staged;
			if (staged)
				++stagedComponentCount;
			else
				std::cout << "[DETAIL] WFX component stage " << componentId
					<< ": " << playbackStatus << '\n';
		}
		runner.Require(!catalogComponentIds.empty() && componentStageComplete &&
			stagedComponentCount == catalogComponentIds.size(),
			"Data Files Stages Every Current WFX Component");

		if (bCheckFailedReloadRollback)
		{
			const uint64_t revision = CEffectCatalog::Get_RuntimeRevision();
			std::ifstream input(stagedCatalog, std::ios::binary);
			std::string invalid{
				std::istreambuf_iterator<char>(input),
				std::istreambuf_iterator<char>() };
			const std::string marker = "\"formatVersion\":2";
			const size_t markerIndex = invalid.find(marker);
			if (std::string::npos != markerIndex)
				invalid[markerIndex + marker.size() - 1u] = '9';
			std::ofstream output(stagedCatalog,
				std::ios::binary | std::ios::trunc);
			output.write(invalid.data(), static_cast<std::streamsize>(invalid.size()));
			output.close();
			const bool_t rejected = !CEffectCatalog::Load(status);
			runner.Require(
				std::string::npos != markerIndex && rejected &&
				revision == CEffectCatalog::Get_RuntimeRevision() &&
				CEffectCatalog::Contains("effect.dimensionmaster.skill.2050500"),
				"Effect Runtime Invalid Catalog Preserves Committed Assembly State");
		}

		CEffectCatalog::Clear();
		error.clear();
		std::filesystem::remove(stagedCatalog, error);
		resourceRootEnvironment.Restore();
	}
}

int main(const int argc, char* argv[])
{
	TEST_RUNNER runner{};
	const std::string Mode = argc > 1 && nullptr != argv[1] ? argv[1] : "";
	if (Mode == "--skill-binding-fast")
	{
		Test_RealSkillBindingDocuments(runner);
		std::cout << "failures : " << runner.iFailureCount << '\n';
		return 0 == runner.iFailureCount ? 0 : 1;
	}
	if (Mode == "--effect-executor-fast")
	{
		Test_EffectPlaybackDeterminism(runner);
		Test_EffectSourceModuleOccurrenceOrder(runner);
		Test_EffectExactSourceSemantics(runner);
		Test_DimensionMasterSourceSemanticAssets(runner);
		std::cout << "failures : " << runner.iFailureCount << '\n';
		return 0 == runner.iFailureCount ? 0 : 1;
	}
	if (Mode == "--effect-runtime-fast")
	{
		Test_EffectAssemblyRuntimeCatalog(runner, false);
		std::cout << "failures : " << runner.iFailureCount << '\n';
		return 0 == runner.iFailureCount ? 0 : 1;
	}
	if (Mode == "--effect-imported-fast")
	{
		Test_DimensionMasterImportedPortalDraft(runner);
		std::cout << "failures : " << runner.iFailureCount << '\n';
		return 0 == runner.iFailureCount ? 0 : 1;
	}

	Test_NormalHandoff(runner);
	Test_CharacterSelectServerHandoff(runner);
	Test_CharacterSelectPreviewReturnCommand(runner);
	Test_EntryPurpose(runner);
	Test_ExactCancellation(runner);
	Test_StaleTokenCannotCancelNewCommand(runner);
	Test_InvalidRequestsPreservePendingCommand(runner);
	Test_SkillBindingSchema(runner);
	Test_RealSkillBindingDocuments(runner);
	Test_SkillBindingAtomicSave(runner);
	Test_EffectPlaybackDeterminism(runner);
	Test_EffectSourceModuleOccurrenceOrder(runner);
	Test_EffectExactSourceSemantics(runner);
	Test_DimensionMasterSourceSemanticAssets(runner);
	Test_DimensionMasterImportedPortalDraft(runner);
	Test_EffectAllEffectsAuthoringJoin(runner);
	Test_EffectDraftAtomicSave(runner);
	Test_EffectAssemblyRuntimeCatalog(runner);

	std::cout << "failures : " << runner.iFailureCount << '\n';
	return 0 == runner.iFailureCount ? 0 : 1;
}
