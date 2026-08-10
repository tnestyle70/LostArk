#include "Client_Defines.h"
#include "ActionPresentationTimeline.h"
#include "LobbyCommandService.h"
#include "NetObjectRegistry.h"
#include "AnimationSkillBindingDocument.h"
#include "CharacterSelectionState.h"
#include "DataJson.h"
#include "Effect_CascadeCompiler.h"
#include "Effect_DocumentCodec.h"
#include "Effect_Distribution.h"
#include "Effect_Catalog.h"
#include "Effect_MaterialTemplate.h"
#include "Effect_Playback.h"
#include "Effect_RuntimeAuthority.h"
#include "PlayerSkillCatalog.h"
#include "PresentationProvider.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
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

	struct EFFECT_BUILD_ROW final
	{
		std::size_t iComponentCount = 0u;
		std::size_t iEmitterCount = 0u;
	};

	struct EFFECT_BUILD_CONTRACT final
	{
		std::size_t iEffectCount = 0u;
		std::size_t iComponentCount = 0u;
		std::size_t iEmitterCount = 0u;
		std::map<std::string, EFFECT_BUILD_ROW, std::less<>> Effects;
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

	bool_t Load_EffectBuildReceipt(
		const wchar_t* pRelativePath,
		const char_t* pExpectedCharacterClass,
		EFFECT_BUILD_CONTRACT& outContract,
		std::string& outStatus)
	{
		using namespace Client;
		outContract = {};
		const std::filesystem::path path = CProjectDataRoot::Resolve(pRelativePath);
		std::ifstream input(path, std::ios::binary);
		if (path.empty() || !input)
		{
			outStatus = std::string(pExpectedCharacterClass) +
				" component build receipt is missing.";
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
			characterClass->Get_String() != pExpectedCharacterClass ||
			nullptr == compileIdentity || !compileIdentity->Is_Boolean() ||
			!compileIdentity->Get_Boolean() ||
			nullptr == effects || !effects->Is_Array() ||
			!Read_ContractSize(root, "effectCount", outContract.iEffectCount) ||
			!Read_ContractSize(root, "componentCount", outContract.iComponentCount) ||
			!Read_ContractSize(root, "emitterCount", outContract.iEmitterCount))
		{
			outStatus = std::string(pExpectedCharacterClass) +
				" component build receipt contract is invalid.";
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
			EFFECT_BUILD_ROW row;
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
				outStatus = std::string(pExpectedCharacterClass) +
					" component build receipt effect row is invalid.";
				return false;
			}
			componentTotal += row.iComponentCount;
			emitterTotal += row.iEmitterCount;
		}

		if (outContract.Effects.size() != outContract.iEffectCount ||
			componentTotal != outContract.iComponentCount ||
			emitterTotal != outContract.iEmitterCount)
		{
			outStatus = std::string(pExpectedCharacterClass) +
				" component build receipt totals disagree with its rows.";
			return false;
		}
		return true;
	}

	bool_t Load_FourClassEffectBuildContract(
		EFFECT_BUILD_CONTRACT& outContract,
		std::string& outStatus)
	{
		struct RECEIPT_SOURCE final
		{
			const wchar_t* pRelativePath = nullptr;
			const char_t* pCharacterClass = nullptr;
		};
		const RECEIPT_SOURCE Sources[] =
		{
			{ L"Effects/AuthoredCorrections/Generated/ComponentBuild/"
				L"Artist.component-build.receipt.json", "ARTIST" },
			{ L"Effects/AuthoredCorrections/Generated/ComponentBuild/"
				L"DimensionMaster.component-build.receipt.json", "DIMENSIONMASTER" },
			{ L"Effects/AuthoredCorrections/Generated/ComponentBuild/"
				L"LanceMaster.component-build.receipt.json", "LANCE_MASTER" },
			{ L"Effects/AuthoredCorrections/Generated/ComponentBuild/"
				L"Warlord.component-build.receipt.json", "WARLORD" }
		};

		outContract = {};
		for (const RECEIPT_SOURCE& source : Sources)
		{
			EFFECT_BUILD_CONTRACT receipt;
			if (!Load_EffectBuildReceipt(source.pRelativePath,
				source.pCharacterClass, receipt, outStatus))
			{
				outContract = {};
				return false;
			}
			outContract.iEffectCount += receipt.iEffectCount;
			outContract.iComponentCount += receipt.iComponentCount;
			outContract.iEmitterCount += receipt.iEmitterCount;
			for (const auto& entry : receipt.Effects)
			{
				if (!outContract.Effects.emplace(entry).second)
				{
					outStatus = "Four-class component build receipts contain a duplicate Effect ID.";
					outContract = {};
					return false;
				}
			}
		}

		if (101u != outContract.iEffectCount ||
			555u != outContract.iComponentCount ||
			2160u != outContract.iEmitterCount ||
			outContract.Effects.size() != outContract.iEffectCount)
		{
			outStatus = "Four-class component build receipt totals must be exactly 101/555/2160.";
			outContract = {};
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

	void Test_CharacterSelectAuthorizedSelection(TEST_RUNNER& runner)
	{
		using namespace Client;
		using namespace LostArk::Shared;
		runner.Require(CCharacterSelectionState::Select(
			CHARACTER_CLASS_ID::DIMENSIONMASTER),
			"Commit Server-approved Character Select class");
		CHARACTER_CLASS_ID selected = CHARACTER_CLASS_ID::END;
		runner.Require(CCharacterSelectionState::Try_Get_SelectedClass(selected) &&
			CHARACTER_CLASS_ID::DIMENSIONMASTER == selected,
			"Persist Server-approved Character Select class");

		LOBBY_COMMAND_TOKEN token = INVALID_LOBBY_COMMAND_TOKEN;
		runner.Require(
			CLobbyCommandService::Request(
				LOBBY_STAGE::CHARACTER_SELECT,
				token) &&
			INVALID_LOBBY_COMMAND_TOKEN != token,
			"Character Select stages tokenized Server entry");

		LOBBY_COMMAND command{};
		runner.Require(
			CLobbyCommandService::Try_Consume(command) &&
			LOBBY_STAGE::CHARACTER_SELECT == command.eStage &&
			LOBBY_COMMAND_PURPOSE::GAMEPLAY == command.ePurpose &&
			token == command.iToken,
			"Lobby consumes exact Character Select Server entry");
		Require_NoPendingCommand(
			runner,
			"Character Select entry leaves no stale Lobby command");
	}

	void Test_NetObjectRegistryClassReplacement(TEST_RUNNER& runner)
	{
		using namespace Client;
		using namespace LostArk::Shared;
		const auto noDelete = [](CCharacter*) {};
		const std::shared_ptr<CCharacter> first{
			reinterpret_cast<CCharacter*>(static_cast<std::uintptr_t>(1u)), noDelete };
		const std::shared_ptr<CCharacter> second{
			reinterpret_cast<CCharacter*>(static_cast<std::uintptr_t>(2u)), noDelete };

		CNetObjectRegistry registry;
		NET_PLAYER_RECORD record{};
		record.iPlayerId = 7u;
		record.iNetEntityId = 107u;
		record.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		record.strNickName = "RegistryClassSwitch";
		OBJECT_HANDLE originalHandle{};
		runner.Require(registry.Register(record, first, originalHandle),
			"Registry stages original Character Select entity");

		NET_PLAYER_RECORD replacement = record;
		replacement.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		OBJECT_HANDLE replacementHandle{};
		runner.Require(
			registry.Replace(
				record.iNetEntityId, replacement, second, replacementHandle) &&
			originalHandle.iSlotIndex == replacementHandle.iSlotIndex &&
			originalHandle.iGeneration == replacementHandle.iGeneration &&
			registry.Resolve(replacementHandle) == second &&
			nullptr != registry.Find_Record(record.iNetEntityId) &&
			CHARACTER_CLASS_ID::ARTIST ==
				registry.Find_Record(record.iNetEntityId)->eCharacterClass,
			"Registry atomically replaces same-entity class presentation");

		NET_PLAYER_RECORD invalid = replacement;
		invalid.eCharacterClass = CHARACTER_CLASS_ID::END;
		OBJECT_HANDLE ignored{};
		runner.Require(
			!registry.Replace(record.iNetEntityId, invalid, first, ignored) &&
			registry.Resolve(replacementHandle) == second &&
			CHARACTER_CLASS_ID::ARTIST ==
				registry.Find_Record(record.iNetEntityId)->eCharacterClass,
			"Rejected registry replacement preserves committed presentation");
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
			{ 101u, { { { { "Active_A", 0u, 1.f }, { "Active_B", 0u, 1.f } } } } },
			{ 102u, { { { { "BA_1", 0u, 1.f } } },
				{ { { "BA_2", 0u, 1.f } } },
				{ { { "BA_3", 0u, 1.f } } } } }
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
			"\"formatVersion\":3,\"animationAssetId\":\"LanceMaster\","
			"\"characterClass\":\"LANCE_MASTER\",\"bindings\":["
			"{\"skillId\":101,\"clips\":[\"Active_A\","
			"{\"clip\":\"Active_B\",\"playMs\":250,\"playRate\":1.25}]},"
			"{\"skillId\":102,\"clips\":[[\"BA_1\"],[\"BA_2\"],[\"BA_3\"]]}]}";
		ANIMATION_SKILL_BINDING_DOCUMENT output;
		std::string status;
		runner.Require(
			CAnimationSkillBindingDocument::Parse_Text(valid, output, status) &&
			CAnimationSkillBindingDocument::Validate(output, "LanceMaster",
				CHARACTER_CLASS_ID::LANCE_MASTER, skills, clips, status),
			"Skill Binding Valid Parse And Catalog Join");

		runner.Require(
			1u == output.Bindings[0].Stages.size() &&
			2u == output.Bindings[0].Stages[0].Clips.size() &&
			3u == output.Bindings[1].Stages.size() &&
			1u == output.Bindings[1].Stages[0].Clips.size(),
			"Skill Binding Flat Clips Are One Stage And Nested Clips Are Many");

		const std::string wrongVersion =
			"{\"schema\":\"lostark.animation-skill-bindings\","
			"\"formatVersion\":2,\"animationAssetId\":\"LanceMaster\","
			"\"characterClass\":\"LANCE_MASTER\",\"bindings\":["
			"{\"skillId\":101,\"clips\":[\"Active_A\"]}]}";
		const std::string badPlayMs =
			"{\"schema\":\"lostark.animation-skill-bindings\","
			"\"formatVersion\":3,\"animationAssetId\":\"LanceMaster\","
			"\"characterClass\":\"LANCE_MASTER\",\"bindings\":["
			"{\"skillId\":101,\"clips\":["
			"{\"clip\":\"Active_A\",\"playMs\":0}]}]}";
		const std::string traversal =
			"{\"schema\":\"lostark.animation-skill-bindings\","
			"\"formatVersion\":3,\"animationAssetId\":\"../Escape\","
			"\"characterClass\":\"LANCE_MASTER\",\"bindings\":["
			"{\"skillId\":101,\"clips\":[\"Active_A\"]}]}";
		const std::string mixedShape =
			"{\"schema\":\"lostark.animation-skill-bindings\","
			"\"formatVersion\":3,\"animationAssetId\":\"LanceMaster\","
			"\"characterClass\":\"LANCE_MASTER\",\"bindings\":["
			"{\"skillId\":102,\"clips\":[[\"BA_1\"],\"BA_2\",[\"BA_3\"]]}]}";
		const std::string emptyStage =
			"{\"schema\":\"lostark.animation-skill-bindings\","
			"\"formatVersion\":3,\"animationAssetId\":\"LanceMaster\","
			"\"characterClass\":\"LANCE_MASTER\",\"bindings\":["
			"{\"skillId\":102,\"clips\":[[\"BA_1\"],[],[\"BA_3\"]]}]}";
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
		runner.Require(!CAnimationSkillBindingDocument::Parse_Text(
			mixedShape, output, status), "Skill Binding Rejects Mixed Stage Shape");
		runner.Require(!CAnimationSkillBindingDocument::Parse_Text(
			emptyStage, output, status), "Skill Binding Rejects Empty Stage");

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
		invalid.Bindings[0].Stages[0].Clips[0].strClipName = "Missing_Clip";
		runner.Require(!CAnimationSkillBindingDocument::Validate(invalid,
			"LanceMaster", CHARACTER_CLASS_ID::LANCE_MASTER, skills, clips, status),
			"Skill Binding Rejects Unknown Model Clip");
		invalid = Make_BindingFixture();
		invalid.Bindings[1].Stages.pop_back();
		runner.Require(!CAnimationSkillBindingDocument::Validate(invalid,
			"LanceMaster", CHARACTER_CLASS_ID::LANCE_MASTER, skills, clips, status),
			"Skill Binding Rejects Combo Count Mismatch");
		/* An ACTIVE chain is one stage by contract: splitting it would silently
		make the client wait for a stage the Server never sends. */
		invalid = Make_BindingFixture();
		invalid.Bindings[0].Stages.push_back(
			{ { { "Active_B", 0u, 1.f } } });
		runner.Require(!CAnimationSkillBindingDocument::Validate(invalid,
			"LanceMaster", CHARACTER_CLASS_ID::LANCE_MASTER, skills, clips, status),
			"Skill Binding Rejects Staged Active Skill");

		ANIMATION_SKILL_BINDING_DOCUMENT preserved = Make_BindingFixture();
		runner.Require(!CAnimationSkillBindingDocument::Load_FromPath(
			L"Z:/definitely/missing/skillbindings.json", "LanceMaster",
			CHARACTER_CLASS_ID::LANCE_MASTER, skills, clips, preserved, status) &&
			2u == preserved.Bindings.size() &&
			"Active_A" == preserved.Bindings[0].Stages[0].Clips[0].strClipName,
			"Skill Binding Failed Staged Load Preserves Prior Output");
	}

	void Test_ActionPresentationTimeline(TEST_RUNNER& runner)
	{
		using namespace Client;
		std::vector<ACTION_PRESENTATION_CLIP_TIMING> clips =
		{
			{ 2.f, 500u, 0.5f, false },
			{ 1.f, 0u, 2.f, false }
		};
		f32_t sourceDuration = 0.f;
		f32_t wallDuration = 0.f;
		runner.Require(
			CActionPresentationTimeline::Resolve_ClipDuration(
				clips[0], sourceDuration, wallDuration) &&
			std::abs(sourceDuration - 0.5f) < 0.000001f &&
			std::abs(wallDuration - 1.f) < 0.000001f,
			"Action Timeline Applies PlayMs Before PlayRate");

		ACTION_PRESENTATION_SAMPLE sample;
		runner.Require(
			CActionPresentationTimeline::Resolve_Sample(clips, 1.f, sample) &&
			1u == sample.iClipIndex &&
			std::abs(sample.fClipSourceTimeSeconds) < 0.000001f,
			"Action Timeline Exact Boundary Selects Next Sequential Clip");
		runner.Require(
			CActionPresentationTimeline::Resolve_Sample(clips, 1.125f, sample) &&
			1u == sample.iClipIndex &&
			std::abs(sample.fClipSourceTimeSeconds - 0.25f) < 0.000001f,
			"Action Timeline Converts Wall Overshoot By Current Clip Rate");

		const std::vector<ACTION_PRESENTATION_CLIP_TIMING> hold =
			{ { 1.f, 400u, 2.f, true } };
		runner.Require(
			CActionPresentationTimeline::Resolve_Sample(hold, 0.65f, sample) &&
			0u == sample.iClipIndex && 3u == sample.iLoopEpoch &&
			std::abs(sample.fClipSourceTimeSeconds - 0.1f) < 0.00001f,
			"Action Timeline HOLD Resolves Loop Epoch And Source Phase");
		f32_t cueWallOffset = 0.f;
		runner.Require(
			CActionPresentationTimeline::Resolve_CueWallOffset(
				hold, 0u, 0.1f, 2u, cueWallOffset) &&
			std::abs(cueWallOffset - 0.45f) < 0.00001f,
			"Action Timeline Cue Identity Includes HOLD Loop Epoch");

		f32_t actionAge = 0.f;
		runner.Require(
			CActionPresentationTimeline::Is_ForwardTick(1u, UINT32_MAX) &&
			!CActionPresentationTimeline::Is_ForwardTick(UINT32_MAX, 1u) &&
			!CActionPresentationTimeline::Is_ForwardTick(1u, 1u) &&
			CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
				1u, UINT32_MAX, 30.f, actionAge) &&
			std::abs(actionAge - 1.f / 30.f) < 0.000001f,
			"Action Timeline Skips Reserved Zero Tick Across Wrap");
		runner.Require(
			CActionPresentationTimeline::Try_ResolveActionAgeSeconds(
				1u, UINT32_MAX - 1u, 30.f, actionAge) &&
			std::abs(actionAge - 2.f / 30.f) < 0.000001f,
			"Action Timeline Counts Two Forward Ticks Across Reserved Zero");

		ANIMATION_SKILL_BINDING_DOCUMENT dimensionMaster;
		std::string status;
		const std::filesystem::path bindingPath =
			CAnimationSkillBindingDocument::Resolve_Path("DimensionMaster");
		const bool_t bindingParsed =
			CAnimationSkillBindingDocument::Parse_Text(
				Read_Text(bindingPath), dimensionMaster, status);
		const auto binding = std::find_if(
			dimensionMaster.Bindings.begin(), dimensionMaster.Bindings.end(),
			[](const ANIMATION_SKILL_BINDING& candidate)
			{
				return 2050240u == candidate.iSkillId;
			});
		const std::filesystem::path cuePath = CProjectDataRoot::Resolve(
			"Animation/Authored/DimensionMaster/DimensionMaster.animevents");
		const std::string cueText = Read_Text(cuePath);
		const bool_t exactPriorClipProductCue = std::string::npos != cueText.find(
			"\"pc_sp_m_00_sk_sk_telekinesisthrust_01\" EFFECT startms=0 "
			"payload=\"effect.dimensionmaster.skill.2050240.authored-baseline.clip1\" "
			"effectref=asset");
		bool_t exactBinding = bindingParsed &&
			binding != dimensionMaster.Bindings.end() &&
			1u == binding->Stages.size() &&
			2u == binding->Stages[0].Clips.size();
		if (exactBinding)
		{
			exactBinding =
				binding->Stages[0].Clips[0].strClipName ==
					"pc_sp_m_00_sk_sk_telekinesisthrust_01" &&
				binding->Stages[0].Clips[1].strClipName ==
					"pc_sp_m_00_sk_sk_telekinesisthrust_04";
		}
		const std::vector<ACTION_PRESENTATION_CLIP_TIMING> dm2050240 =
		{
			{ 0.25f, exactBinding ?
				binding->Stages[0].Clips[0].iPlayMs : 0u,
				exactBinding ? binding->Stages[0].Clips[0].fPlayRate : 1.f,
				false },
			{ 1.f, exactBinding ?
				binding->Stages[0].Clips[1].iPlayMs : 0u,
				exactBinding ? binding->Stages[0].Clips[1].fPlayRate : 1.f,
				false }
		};
		f32_t dmCueWallOffset = -1.f;
		const bool_t delayedPriorClipCue =
			CActionPresentationTimeline::Resolve_Sample(
				dm2050240, 0.5f, sample) &&
			1u == sample.iClipIndex &&
			CActionPresentationTimeline::Resolve_CueWallOffset(
				dm2050240, 0u, 0.f, 0u, dmCueWallOffset) &&
			std::abs(dmCueWallOffset) < 0.000001f &&
			std::abs((0.5f - dmCueWallOffset) *
				dm2050240[0].fPlayRate - 0.5f) < 0.000001f;
		runner.Require(exactBinding && exactPriorClipProductCue &&
			delayedPriorClipCue,
			"DimensionMaster 2050240 Delayed First Snapshot Retains Prior Clip Product Cue");
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
				for (const ANIMATION_SKILL_STAGE& stage : binding.Stages)
				{
					for (const ANIMATION_SKILL_CLIP& clip : stage.Clips)
						available.push_back(clip.strClipName);
				}
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
					std::vector<std::string> boundClips;
					if (binding != parsed.Bindings.end())
					{
						for (const ANIMATION_SKILL_STAGE& stage : binding->Stages)
						{
							for (const ANIMATION_SKILL_CLIP& clip : stage.Clips)
								boundClips.push_back(clip.strClipName);
						}
					}
					bool_t clipsMatch = boundClips.size() == row.Clips.size();
					if (clipsMatch)
					{
						for (size_t iClip = 0u; iClip < row.Clips.size(); ++iClip)
						{
							clipsMatch = clipsMatch &&
								boundClips[iClip] == row.Clips[iClip];
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
		changed.Bindings[0].Stages = { { { { "Active_B", 0u, 1.f } } } };
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
		const std::string at3 = simulate(3u);
		if (at3 != at30 || at30 != at60 || at60 != at144)
		{
			std::cout << "[DETAIL] Effect signatures 3/30/60/144 bytes="
				<< at3.size() << '/' << at30.size() << '/' << at60.size() << '/'
				<< at144.size() << " hash=" << std::hash<std::string>{}(at3) << '/'
				<< std::hash<std::string>{}(at30) << '/'
				<< std::hash<std::string>{}(at60) << '/'
				<< std::hash<std::string>{}(at144) << '\n';
		}
		runner.Require(at3 == at30 && at30 == at60 && at60 == at144,
			"Effect Playback Is Deterministic At 3 30 60 And 144 FPS");

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

		EFFECT_DOCUMENT_DESC lerpDocument;
		lerpDocument.strEffectAssetId = "effect.manual.mesh.lerp.harness";
		lerpDocument.strDisplayName = "Manual Mesh Lerp Harness";
		EFFECT_ELEMENT_DESC lerpMesh;
		lerpMesh.strElementId = "mesh";
		lerpMesh.strDisplayName = "Mesh";
		lerpMesh.eKind = EFFECT_ELEMENT_KIND::MESH;
		lerpMesh.Detail.Timing.fLifeTimeSeconds = 2.f;
		lerpMesh.Detail.Transform.vPosition = { 0.f, 0.f, 0.f };
		lerpMesh.Detail.Transform.vRotationDegrees = { 0.f, 0.f, 0.f };
		lerpMesh.Detail.Transform.vScale = { 1.f, 1.f, 1.f };
		lerpMesh.Detail.Transform.vVelocityPerSecond = { 1.f, 0.f, 0.f };
		lerpMesh.Detail.LinearLerp.bPosition = true;
		lerpMesh.Detail.LinearLerp.vEndPosition = { 10.f, 0.f, 0.f };
		lerpMesh.Detail.LinearLerp.bRotation = true;
		lerpMesh.Detail.LinearLerp.vEndRotationDegrees = { 0.f, 45.f, 0.f };
		lerpMesh.Detail.LinearLerp.bScale = true;
		lerpMesh.Detail.LinearLerp.vEndScale = { 2.f, 2.f, 2.f };
		lerpMesh.Detail.LinearLerp.bVelocity = true;
		lerpMesh.Detail.LinearLerp.vEndVelocityPerSecond = { 3.f, 0.f, 0.f };
		lerpDocument.Elements.push_back(lerpMesh);
		CEffectPlayback lerpPlayback;
		const bool_t bLerpStaged =
			lerpPlayback.Stage_Document(lerpDocument, status);
		lerpPlayback.Seek(1.f, root);
		const EFFECT_EVALUATED_FRAME& lerpFrame = lerpPlayback.Get_Frame();
		bool_t bLerpEvaluated = bLerpStaged &&
			lerpFrame.Elements.size() == 1u;
		if (bLerpEvaluated)
		{
			const float4x4_t& world = lerpFrame.Elements.front().World;
			const f32_t basisLength = std::sqrt(
				world._11 * world._11 + world._12 * world._12 +
				world._13 * world._13);
			bLerpEvaluated =
				std::abs(world._41 - 6.5f) < 0.0001f &&
				std::abs(basisLength - 1.5f) < 0.0001f &&
				std::abs(world._13) > 0.5f;
		}
		runner.Require(bLerpEvaluated,
			"Manual Mesh Lerp Evaluates Position Rotation Scale And Velocity Over Lifetime");

		EFFECT_DOCUMENT_DESC snapshotDocument;
		snapshotDocument.strEffectAssetId =
			"effect.player.root.snapshot.occurrences.harness";
		snapshotDocument.strDisplayName =
			"Player Root Snapshot Occurrences Harness";
		auto snapshotSlash = [](const std::string& id, const f32_t start)
		{
			EFFECT_ELEMENT_DESC element;
			element.strElementId = id;
			element.strDisplayName = id;
			element.eKind = EFFECT_ELEMENT_KIND::MESH;
			element.Detail.Timing.fStartDelaySeconds = start;
			element.Detail.Timing.fLifeTimeSeconds = 0.75f;
			element.ActionCueAttachment.bEnabled = true;
			element.ActionCueAttachment.bFollow = false;
			element.ActionCueAttachment.strSourceAnchorSlotId = "root";
			element.ActionCueAttachment.strRuntimeAnchorSlotId = "root";
			return element;
		};
		snapshotDocument.Elements.push_back(snapshotSlash("slash.0", 0.25f));
		snapshotDocument.Elements.push_back(snapshotSlash("slash.1", 0.60f));
		snapshotDocument.Elements.push_back(snapshotSlash("slash.2", 0.90f));
		snapshotDocument.Elements.push_back(snapshotSlash("slash.3", 1.30f));
		CEffectPlayback snapshotPlayback;
		const bool_t bSnapshotStaged =
			snapshotPlayback.Stage_Document(snapshotDocument, status);
		float4x4_t rootAtZero{};
		float4x4_t rootAtFirstSlash{};
		float4x4_t rootBetweenFirstSecond{};
		float4x4_t rootAtSecondSlash{};
		float4x4_t rootBetweenSecondThird{};
		float4x4_t rootAtThirdSlash{};
		float4x4_t rootBetweenThirdFourth{};
		float4x4_t rootAtFourthSlash{};
		float4x4_t rootAfterFourthSlash{};
		XMStoreFloat4x4(&rootAtZero, XMMatrixIdentity());
		XMStoreFloat4x4(&rootAtFirstSlash, XMMatrixTranslation(1.f, 0.f, 0.f));
		XMStoreFloat4x4(&rootBetweenFirstSecond, XMMatrixTranslation(2.f, 0.f, 0.f));
		XMStoreFloat4x4(&rootAtSecondSlash, XMMatrixTranslation(3.f, 0.f, 0.f));
		XMStoreFloat4x4(&rootBetweenSecondThird, XMMatrixTranslation(4.f, 0.f, 0.f));
		XMStoreFloat4x4(&rootAtThirdSlash, XMMatrixTranslation(5.f, 0.f, 0.f));
		XMStoreFloat4x4(&rootBetweenThirdFourth, XMMatrixTranslation(6.f, 0.f, 0.f));
		XMStoreFloat4x4(&rootAtFourthSlash, XMMatrixTranslation(7.f, 0.f, 0.f));
		XMStoreFloat4x4(&rootAfterFourthSlash, XMMatrixTranslation(8.f, 0.f, 0.f));
		auto findSnapshotX = [&snapshotPlayback](const char* elementId)
		{
			for (const EFFECT_EVALUATED_ELEMENT& element :
				snapshotPlayback.Get_Frame().Elements)
			{
				if (element.pElement->strElementId == elementId)
					return element.World._41;
			}
			return -1.f;
		};
		auto hasActiveSlash = [&snapshotPlayback](const char* elementId)
		{
			for (const EFFECT_EVALUATED_ELEMENT& element :
				snapshotPlayback.Get_Frame().Elements)
			{
				if (element.pElement->strElementId == elementId)
					return true;
			}
			return false;
		};
		f32_t firstSnapshotX = -1.f;
		f32_t secondSnapshotX = -1.f;
		f32_t thirdSnapshotX = -1.f;
		f32_t fourthSnapshotX = -1.f;
		bool_t bExpiredInAuthoredOrder = false;
		if (bSnapshotStaged)
		{
			snapshotPlayback.Update(0.20f, rootAtZero);
			snapshotPlayback.Update(0.10f, rootAtFirstSlash);
			firstSnapshotX = findSnapshotX("slash.0");
			snapshotPlayback.Update(0.25f, rootBetweenFirstSecond);
			snapshotPlayback.Update(0.10f, rootAtSecondSlash);
			secondSnapshotX = findSnapshotX("slash.1");
			snapshotPlayback.Update(0.20f, rootBetweenSecondThird);
			snapshotPlayback.Update(0.10f, rootAtThirdSlash);
			thirdSnapshotX = findSnapshotX("slash.2");
			snapshotPlayback.Update(0.30f, rootBetweenThirdFourth);
			snapshotPlayback.Update(0.10f, rootAtFourthSlash);
			fourthSnapshotX = findSnapshotX("slash.3");

			snapshotPlayback.Update(0.05f, rootAfterFourthSlash);
			const bool_t bFirstTwoExpired =
				snapshotPlayback.Get_Frame().Elements.size() == 2u &&
				hasActiveSlash("slash.2") && hasActiveSlash("slash.3");
			snapshotPlayback.Update(0.30f, rootAfterFourthSlash);
			const bool_t bThirdExpired =
				snapshotPlayback.Get_Frame().Elements.size() == 1u &&
				hasActiveSlash("slash.3");
			snapshotPlayback.Update(0.40f, rootAfterFourthSlash);
			bExpiredInAuthoredOrder = bFirstTwoExpired && bThirdExpired &&
				snapshotPlayback.Get_Frame().Elements.empty();
		}
		runner.Require(bSnapshotStaged &&
			std::abs(firstSnapshotX - 1.f) < 0.0001f &&
			std::abs(secondSnapshotX - 3.f) < 0.0001f &&
			std::abs(thirdSnapshotX - 5.f) < 0.0001f &&
			std::abs(fourthSnapshotX - 7.f) < 0.0001f,
			"Effect Slash Occurrences Snapshot Player Root At Their Own Start Times");
		runner.Require(bSnapshotStaged && bExpiredInAuthoredOrder,
			"Effect Slash Occurrences Expire From Their Authored Start And Lifetime");
	}

	void Test_EffectTypedPresentation(TEST_RUNNER& runner)
	{
		using namespace Client;
		using namespace Engine;
		EFFECT_DOCUMENT_DESC document;
		document.strEffectAssetId = "effect.presentation.harness";
		document.strDisplayName = "Typed Presentation Harness";

		EFFECT_ELEMENT_DESC light;
		light.strElementId = "light.point";
		light.strDisplayName = "Point Light";
		light.eKind = EFFECT_ELEMENT_KIND::LIGHT;
		light.Detail.Transform.vPosition = { 1.f, 2.f, 3.f };
		light.Detail.Timing.fLifeTimeSeconds = 1.f;
		light.Detail.Light.bEnabled = true;
		light.Detail.Light.eProfile =
			EFFECT_LIGHT_PROFILE::POINT_RECONSTRUCTED_V1;
		light.Detail.Light.eStatus =
			EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE;
		light.Detail.Light.fRange = 3.f;
		light.Detail.Light.fIntensity = 10.f;
		light.Detail.Light.vColor = { 3.f, 2.f, 1.f, 1.f };
		light.SourcePresentation.bEnabled = true;
		light.SourcePresentation.strSchema =
			"lostark.effect-source-presentation";
		light.SourcePresentation.iVersion = 1u;
		light.SourcePresentation.strProfileId =
			"light.point.reconstructed.v1";
		light.SourcePresentation.eStatus =
			EFFECT_SOURCE_PRESENTATION_STATUS::RECONSTRUCTED;
		light.SourcePresentation.strSourceObjectPath =
			"Harness.ParticleModuleLight";
		light.SourcePresentation.strSourceActionCueId = "cue.light";
		light.SourcePresentation.strSourceEventId = "event.light";
		light.SourcePresentation.iSourceOccurrenceIndex = 7u;
		light.SourcePresentation.fSourceTimeSeconds = 0.25f;
		EFFECT_SOURCE_PRESENTATION_PARAMETER_DESC intensity;
		intensity.strName = "intensity";
		intensity.eKind =
			EFFECT_SOURCE_PRESENTATION_PARAMETER_KIND::NUMBER;
		intensity.eStatus =
			EFFECT_SOURCE_PRESENTATION_PARAMETER_STATUS::SOURCE_EXPLICIT;
		intensity.strSourcePropertyPath = "brightness";
		intensity.fNumberValue = 10.0;
		light.SourcePresentation.Parameters.push_back(intensity);
		document.Elements.push_back(light);

		for (size_t iPost = 0u; iPost < 2u; ++iPost)
		{
			EFFECT_ELEMENT_DESC post;
			post.strElementId = iPost == 0u ? "post.rgb" : "post.zoom";
			post.strDisplayName = iPost == 0u ? "RGB Noise" : "Zoom Blur";
			post.eKind = EFFECT_ELEMENT_KIND::SCREEN_POST;
			post.Detail.Timing.fLifeTimeSeconds = 1.f;
			post.Detail.ScreenPost.bEnabled = true;
			post.Detail.ScreenPost.eProfile = iPost == 0u ?
				EFFECT_SCREEN_POST_PROFILE::RGB_NOISE_RECONSTRUCTED_V1 :
				EFFECT_SCREEN_POST_PROFILE::ZOOM_BLUR_RECONSTRUCTED_V1;
			post.Detail.ScreenPost.eStatus =
				EFFECT_PRESENTATION_RUNTIME_STATUS::RECONSTRUCTED_PROFILE;
			post.Detail.ScreenPost.fIntensity = 0.5f +
				static_cast<f32_t>(iPost);
			post.Detail.ScreenPost.fSecondaryIntensity = 0.25f;
			post.Detail.ScreenPost.fFrequency = 2.f;
			post.Detail.ScreenPost.iRandomSeed =
				static_cast<uint32_t>(101u + iPost);
			document.Elements.push_back(post);
		}
		EFFECT_ELEMENT_DESC disabled = document.Elements.back();
		disabled.strElementId = "post.disabled";
		disabled.strDisplayName = "Disabled Post";
		disabled.Detail.ScreenPost.bEnabled = false;
		disabled.Detail.ScreenPost.eProfile = EFFECT_SCREEN_POST_PROFILE::END;
		disabled.Detail.ScreenPost.eStatus =
			EFFECT_PRESENTATION_RUNTIME_STATUS::END;
		document.Elements.push_back(disabled);

		std::string status;
		EFFECT_DOCUMENT_DESC roundTrip;
		const bool_t codecExact =
			CEffectDocumentCodec::Validate(document, status) &&
			CEffectDocumentCodec::Parse(
				CEffectDocumentCodec::Serialize(document), roundTrip, status) &&
			roundTrip.iFormatVersion == EFFECT_AUTHORING_FORMAT_VERSION &&
			roundTrip.Elements.front().Detail.Light.bEnabled &&
			roundTrip.Elements.front().SourcePresentation.Parameters.size() == 1u &&
			roundTrip.Elements.front().SourcePresentation.Parameters.front().
				fNumberValue == 10.0;
		runner.Require(codecExact,
			"Effect V12 Typed Presentation Codec Round Trips Losslessly");

		EFFECT_DOCUMENT_DESC invalidKind = document;
		invalidKind.Elements.front().eKind = EFFECT_ELEMENT_KIND::SPRITE;
		runner.Require(!CEffectDocumentCodec::Validate(invalidKind, status),
			"Effect Typed Light Cannot Execute On Non Light Element");

		float4x4_t root{};
		XMStoreFloat4x4(&root, XMMatrixTranslation(5.f, 0.f, 0.f));
		CEffectPlayback playback;
		const bool_t staged = playback.Stage_Document(document, status);
		if (staged)
			playback.Seek(0.5f, root);
		const EFFECT_EVALUATED_FRAME& frame = playback.Get_Frame();
		const bool_t evaluated = staged && frame.Elements.empty() &&
			frame.Lights.size() == 1u && frame.ScreenPosts.size() == 2u &&
			std::abs(frame.Lights.front().vWorldPosition.x - 6.f) < 0.0001f &&
			frame.ScreenPosts[0].eProfile ==
				EFFECT_SCREEN_POST_PROFILE::RGB_NOISE_RECONSTRUCTED_V1 &&
			frame.ScreenPosts[1].eProfile ==
				EFFECT_SCREEN_POST_PROFILE::ZOOM_BLUR_RECONSTRUCTED_V1 &&
			frame.ScreenPosts[0].iSourceOrder <
				frame.ScreenPosts[1].iSourceOrder;
		runner.Require(evaluated,
			"Effect Playback Separates Typed Light And Ordered Screen Post Cues");
		if (staged)
			playback.Seek(2.f, root);
		runner.Require(staged && playback.Get_Frame().Lights.empty() &&
			playback.Get_Frame().ScreenPosts.empty(),
			"Effect Typed Presentation Stops And Disabled Cues Never Execute");

		const PRESENTATION_SCREEN_POST_PLAN_STEP first =
			Build_PresentationScreenPostPlanStep(0u);
		const PRESENTATION_SCREEN_POST_PLAN_STEP second =
			Build_PresentationScreenPostPlanStep(1u);
		runner.Require(first.iSourceTarget == 0u &&
			first.iDestinationTarget == 1u &&
			second.iSourceTarget == 1u &&
			second.iDestinationTarget == 0u &&
			PresentationScreenPostFinalTarget(0u) == 0u &&
			PresentationScreenPostFinalTarget(1u) == 1u &&
			PresentationScreenPostFinalTarget(2u) == 0u,
			"Effect Screen Post Plan Has No Alias And Handles Zero Odd Even");
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
		auto literalBoolean = [](const std::string& path, const bool_t value)
		{
			EFFECT_SOURCE_LITERAL_DESC result;
			result.strPropertyPath = path;
			result.eKind = EFFECT_SOURCE_LITERAL_KIND::BOOLEAN;
			result.bBoolean = value;
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
				ueFractions.z) < 0.00001f &&
			std::abs(seededPlayback.Get_Frame().Particles.front().World._43 -
				(-ueFractions.y)) < 0.00001f;
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
				(-expectedLockedPosition)) < 0.00001f;
		runner.Require(lockedExact,
			"Effect Raw Type XYZ Lock Reuses First UE Random Fraction");

		EFFECT_DOCUMENT_DESC basisDocument;
		basisDocument.strEffectAssetId = "effect.ue3.basis.harness";
		basisDocument.strDisplayName = "UE3 Basis Harness";
		EFFECT_ELEMENT_DESC basis = sourceElement("basis");
		basis.SourceRecipe.Bursts.push_back({ 0.f, 1u, 1u });
		basis.SourceRecipe.Modules.push_back(lifetime());
		EFFECT_SOURCE_MODULE_DESC basisLocation;
		basisLocation.strStableId = "location.basis";
		basisLocation.strClassName = "particlemodulelocation";
		basisLocation.strObjectPath = "Harness.LocationBasis";
		basisLocation.Distributions.push_back(distribution(
			"startlocation", { 100.f, 200.f, 300.f, 0.f },
			{ 100.f, 200.f, 300.f, 0.f }, 3u));
		basis.SourceRecipe.Modules.push_back(basisLocation);
		EFFECT_SOURCE_MODULE_DESC basisAxisLock;
		basisAxisLock.strStableId = "axis.basis";
		basisAxisLock.strClassName = "particlemoduleorientationaxislock";
		basisAxisLock.strObjectPath = "Harness.AxisBasis";
		basisAxisLock.Literals.push_back(literalString(
			"lockaxisflags", "epal_z"));
		basis.SourceRecipe.Modules.push_back(basisAxisLock);
		basisDocument.Elements.push_back(basis);
		CEffectPlayback basisPlayback;
		const bool_t basisStaged = basisPlayback.Stage_Document(
			basisDocument, status);
		if (basisStaged)
			basisPlayback.Seek(1.f / 60.f, identity);
		const bool_t basisExact = basisStaged &&
			basisPlayback.Get_Frame().Particles.size() == 1u &&
			std::abs(basisPlayback.Get_Frame().Particles.front().World._41 -
				1.f) < 0.00001f &&
			std::abs(basisPlayback.Get_Frame().Particles.front().World._42 -
				3.f) < 0.00001f &&
			std::abs(basisPlayback.Get_Frame().Particles.front().World._43 +
				2.f) < 0.00001f &&
			basisPlayback.Get_Frame().Particles.front().eSpriteAlignment ==
				EFFECT_PARTICLE_SPRITE_ALIGNMENT::AXIS_POSITIVE_Y;
		runner.Require(basisExact,
			"Effect Source Position And EPAL Z Use UE3 To Client Basis Once");

		EFFECT_DOCUMENT_DESC cylinderSpinDocument;
		cylinderSpinDocument.strEffectAssetId =
			"effect.cylinder.spin.source.harness";
		cylinderSpinDocument.strDisplayName = "Cylinder Spin Source Harness";
		EFFECT_ELEMENT_DESC cylinderSpin = sourceElement("cylinder.spin");
		cylinderSpin.SourceRecipe.Bursts.push_back({ 0.f, 1u, 1u });
		cylinderSpin.SourceRecipe.Modules.push_back(lifetime());
		EFFECT_SOURCE_MODULE_DESC cylinderSpinModule;
		cylinderSpinModule.strStableId = "cylinder.spin.module";
		cylinderSpinModule.strClassName =
			"efparticlemodulelocationprimitivecylinderspin";
		cylinderSpinModule.strObjectPath = "Harness.CylinderSpin";
		cylinderSpinModule.Literals.push_back(literalBoolean(
			"negative_x", false));
		cylinderSpinModule.Literals.push_back(literalBoolean(
			"surfaceonly", true));
		cylinderSpinModule.Literals.push_back(literalBoolean(
			"velocity", true));
		cylinderSpinModule.Distributions.push_back(distribution(
			"startradius", { 200.f, 0.f, 0.f, 0.f },
			{ 200.f, 0.f, 0.f, 0.f }, 1u));
		cylinderSpinModule.Distributions.push_back(distribution(
			"startheight", {}, {}, 1u));
		cylinderSpinModule.Distributions.push_back(distribution(
			"startcylinderrot", {}, {}, 1u));
		cylinderSpinModule.Distributions.push_back(distribution(
			"spinangle", {}, {}, 1u));
		cylinderSpinModule.Distributions.push_back(distribution(
			"startlocation", {}, {}, 3u));
		cylinderSpinModule.Distributions.push_back(distribution(
			"velocityscale", { 1.f, 0.f, 0.f, 0.f },
			{ 1.f, 0.f, 0.f, 0.f }, 1u));
		cylinderSpin.SourceRecipe.Modules.push_back(cylinderSpinModule);
		cylinderSpinDocument.Elements.push_back(cylinderSpin);
		CEffectPlayback cylinderSpinPlayback;
		const bool_t cylinderSpinStaged = cylinderSpinPlayback.Stage_Document(
			cylinderSpinDocument, status);
		if (cylinderSpinStaged)
			cylinderSpinPlayback.Seek(1.f / 60.f, identity);
		const EFFECT_EVALUATED_PARTICLE* pCylinderSpin =
			cylinderSpinStaged &&
			cylinderSpinPlayback.Get_Frame().Particles.size() == 1u ?
			&cylinderSpinPlayback.Get_Frame().Particles.front() : nullptr;
		const f32_t fCylinderRadius = nullptr == pCylinderSpin ? 0.f :
			std::sqrt(pCylinderSpin->World._41 * pCylinderSpin->World._41 +
				pCylinderSpin->World._43 * pCylinderSpin->World._43);
		const f32_t fCylinderSpeed = nullptr == pCylinderSpin ? 0.f :
			std::sqrt(pCylinderSpin->vWorldVelocity.x *
				pCylinderSpin->vWorldVelocity.x +
				pCylinderSpin->vWorldVelocity.z *
				pCylinderSpin->vWorldVelocity.z);
		runner.Require(nullptr != pCylinderSpin &&
			std::abs(pCylinderSpin->World._42) < 0.00001f &&
			fCylinderRadius >= 2.f && fCylinderRadius < 2.1f &&
			std::abs(fCylinderSpeed - 2.f) < 0.0001f,
			"Effect Source Cylinder Spin Adds Radial Position And Velocity");

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
		EFFECT_PARTICLE_RUNTIME_PROBE colorProbe;
		const bool_t colorProbeExact = colorStaged &&
			colorPlayback.Query_ParticleRuntimeProbe("colored", colorProbe) &&
			colorProbe.iActiveParticleCount == 1u &&
			!colorProbe.bMeshRenderer &&
			std::abs(colorProbe.fFirstAlpha - 0.25f) < 0.0001f &&
			std::abs(colorProbe.fMinAlpha - 0.25f) < 0.0001f &&
			std::abs(colorProbe.fMaxAlpha - 0.25f) < 0.0001f;
		runner.Require(colorProbeExact,
			"Effect Runtime Probe Reports Evaluated Particle Alpha");

		EFFECT_DOCUMENT_DESC implicitAlphaDocument;
		implicitAlphaDocument.strEffectAssetId =
			"effect.alpha.implicit.identity.harness";
		implicitAlphaDocument.strDisplayName =
			"Implicit UE3 Alpha Identity Harness";
		EFFECT_ELEMENT_DESC implicitAlpha = sourceElement("implicit.alpha");
		implicitAlpha.SourceRecipe.Bursts.push_back({ 0.f, 1u, 1u });
		implicitAlpha.SourceRecipe.Modules.push_back(lifetime());
		EFFECT_SOURCE_MODULE_DESC implicitColor;
		implicitColor.strStableId = "color.implicit.alpha";
		implicitColor.strClassName = "particlemodulecolor";
		implicitColor.strObjectPath = "Harness.ImplicitColor";
		implicitColor.Distributions.push_back(distribution(
			"startcolor", { 1.f, 1.f, 1.f, 0.f },
			{ 1.f, 1.f, 1.f, 0.f }, 3u));
		implicitColor.Distributions.push_back(distribution(
			"startalpha", {}, {}, 1u));
		implicitAlpha.SourceRecipe.Modules.push_back(implicitColor);
		EFFECT_SOURCE_MODULE_DESC implicitScale;
		implicitScale.strStableId = "color.scale.implicit.alpha";
		implicitScale.strClassName = "particlemodulecolorscaleoverlife";
		implicitScale.strObjectPath = "Harness.ImplicitColorScale";
		implicitScale.Distributions.push_back(distribution(
			"colorscaleoverlife", { 1.f, 1.f, 1.f, 0.f },
			{ 1.f, 1.f, 1.f, 0.f }, 3u));
		implicitScale.Distributions.push_back(distribution(
			"alphascaleoverlife", {}, {}, 1u));
		implicitAlpha.SourceRecipe.Modules.push_back(implicitScale);
		implicitAlphaDocument.Elements.push_back(implicitAlpha);
		CEffectPlayback implicitAlphaPlayback;
		const bool_t implicitAlphaStaged =
			implicitAlphaPlayback.Stage_Document(implicitAlphaDocument, status);
		if (implicitAlphaStaged)
			implicitAlphaPlayback.Seek(1.f / 60.f, identity);
		EFFECT_PARTICLE_RUNTIME_PROBE implicitAlphaProbe;
		runner.Require(
			implicitAlphaStaged &&
			implicitAlphaPlayback.Query_ParticleRuntimeProbe(
				"implicit.alpha", implicitAlphaProbe) &&
			implicitAlphaProbe.iActiveParticleCount == 1u &&
			std::abs(implicitAlphaProbe.fFirstAlpha - 1.f) < 0.0001f,
			"Effect Empty UE3 Alpha Distributions Preserve Multiplicative Identity");

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
			std::abs(sizePlayback.Get_Frame().Particles.front().World._22 - 3.f) >= 0.0001f ||
			std::abs(sizePlayback.Get_Frame().Particles.front().World._33 - 2.f) >= 0.0001f)
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
			std::abs(sizePlayback.Get_Frame().Particles.front().World._22 - 3.f) < 0.0001f &&
			std::abs(sizePlayback.Get_Frame().Particles.front().World._33 - 2.f) < 0.0001f,
			"Effect Source Mesh Size Converts UE Units And Axis Basis Once");
		resourceRootEnvironment.Restore();

		EFFECT_DOCUMENT_DESC worldAccelerationDocument;
		worldAccelerationDocument.strEffectAssetId =
			"effect.acceleration.world.basis.harness";
		worldAccelerationDocument.strDisplayName =
			"World Acceleration Basis Harness";
		EFFECT_ELEMENT_DESC worldAcceleration =
			sourceElement("world.acceleration");
		worldAcceleration.Detail.Transform.vRotationDegrees = { 0.f, 90.f, 0.f };
		worldAcceleration.Detail.Particle.bLocalSpace = true;
		worldAcceleration.SourceRecipe.Bursts.push_back({ 0.f, 1u, 1u });
		worldAcceleration.SourceRecipe.Modules.push_back(lifetime());
		EFFECT_SOURCE_MODULE_DESC accelerationModule;
		accelerationModule.strStableId = "acceleration.world";
		accelerationModule.strClassName = "particlemoduleacceleration";
		accelerationModule.strObjectPath = "Harness.WorldAcceleration";
		accelerationModule.Literals.push_back(literalBoolean(
			"balwaysinworldspace", true));
		accelerationModule.Distributions.push_back(distribution(
			"acceleration", { 0.f, 100.f, 0.f, 0.f },
			{ 0.f, 100.f, 0.f, 0.f }, 3u));
		worldAcceleration.SourceRecipe.Modules.push_back(accelerationModule);
		worldAccelerationDocument.Elements.push_back(worldAcceleration);
		CEffectPlayback worldAccelerationPlayback;
		const bool_t worldAccelerationStaged =
			worldAccelerationPlayback.Stage_Document(
				worldAccelerationDocument, status);
		if (worldAccelerationStaged)
			worldAccelerationPlayback.Seek(1.f / 60.f, identity);
		const EFFECT_EVALUATED_PARTICLE* pWorldAcceleration =
			worldAccelerationStaged &&
			worldAccelerationPlayback.Get_Frame().Particles.size() == 1u ?
				&worldAccelerationPlayback.Get_Frame().Particles.front() : nullptr;
		runner.Require(nullptr != pWorldAcceleration &&
			std::abs(pWorldAcceleration->vWorldVelocity.x) < 0.0001f &&
			std::abs(pWorldAcceleration->vWorldVelocity.y) < 0.0001f &&
			std::abs(pWorldAcceleration->vWorldVelocity.z + 1.f / 60.f) <
				0.0001f,
			"Effect Always World Acceleration Converts UE Basis And Ignores Root");

		EFFECT_DOCUMENT_DESC worldVelocityDocument;
		worldVelocityDocument.strEffectAssetId =
			"effect.initial.velocity.world.basis.harness";
		worldVelocityDocument.strDisplayName =
			"World Initial Velocity Basis Harness";
		EFFECT_ELEMENT_DESC worldVelocity =
			sourceElement("world.initial.velocity");
		worldVelocity.Detail.Transform.vRotationDegrees = { 0.f, 90.f, 0.f };
		worldVelocity.Detail.Particle.bLocalSpace = true;
		worldVelocity.SourceRecipe.Bursts.push_back({ 0.f, 1u, 1u });
		worldVelocity.SourceRecipe.Modules.push_back(lifetime());
		EFFECT_SOURCE_MODULE_DESC velocityModule;
		velocityModule.strStableId = "velocity.world";
		velocityModule.strClassName = "particlemodulevelocity";
		velocityModule.strObjectPath = "Harness.WorldInitialVelocity";
		velocityModule.Literals.push_back(literalBoolean(
			"binworldspace", true));
		velocityModule.Distributions.push_back(distribution(
			"startvelocity", { 0.f, 100.f, 0.f, 0.f },
			{ 0.f, 100.f, 0.f, 0.f }, 3u));
		worldVelocity.SourceRecipe.Modules.push_back(velocityModule);
		worldVelocityDocument.Elements.push_back(worldVelocity);
		CEffectPlayback worldVelocityPlayback;
		const bool_t worldVelocityStaged =
			worldVelocityPlayback.Stage_Document(worldVelocityDocument, status);
		if (worldVelocityStaged)
			worldVelocityPlayback.Seek(1.f / 60.f, identity);
		const EFFECT_EVALUATED_PARTICLE* pWorldVelocity =
			worldVelocityStaged &&
			worldVelocityPlayback.Get_Frame().Particles.size() == 1u ?
				&worldVelocityPlayback.Get_Frame().Particles.front() : nullptr;
		runner.Require(nullptr != pWorldVelocity &&
			std::abs(pWorldVelocity->vWorldVelocity.x) < 0.0001f &&
			std::abs(pWorldVelocity->vWorldVelocity.y) < 0.0001f &&
			std::abs(pWorldVelocity->vWorldVelocity.z + 1.f) < 0.0001f,
			"Effect World Initial Velocity Converts UE Basis And Ignores Root");

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
		const auto WriteConstantVectorField =
			[&vectorPath](const float3_t& sample)
		{
			std::ofstream output(vectorPath, std::ios::binary | std::ios::trunc);
			const char magic[4] = { 'W', 'V', 'F', '1' };
			const uint32_t header[5] = { 1u, 2u, 2u, 2u, 8u };
			output.write(magic, sizeof(magic));
			output.write(reinterpret_cast<const char*>(header), sizeof(header));
			for (uint32_t index = 0u; index < 8u; ++index)
			{
				output.write(reinterpret_cast<const char*>(&sample), sizeof(sample));
			}
			return output.good();
		};
		const bool_t vectorWritten = WriteConstantVectorField(
			{ 100.f, 0.f, 0.f });
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
		const bool_t vectorStaged = vectorWritten && !vectorError &&
			vectorPlayback.Stage_Document(vectorDocument, status);
		if (vectorStaged)
			vectorPlayback.Seek(1.f / 60.f, identity);
		const bool_t vectorExecuted = vectorStaged &&
			vectorPlayback.Get_Frame().Particles.size() == 1u &&
			std::abs(vectorPlayback.Get_Frame().Particles.front().World._41 -
				(61.f / 3600.f)) < 0.00001f;
		runner.Require(vectorExecuted,
			"Effect Local Vector Field Loads Volume And Applies Trilinear Force");

		const uint64_t vectorLoadsBeforeRevisionBundles =
			CEffectPlayback::Get_VectorFieldDiskLoadCount();
		std::shared_ptr<const CEffectPlayback::PREPARED_RESOURCES>
			revisionOneBundle;
		CEffectPlayback revisionOnePlayback;
		const bool_t revisionOnePrepared =
			CEffectPlayback::Prepare_DocumentResources(
				vectorDocument, revisionOneBundle, status) &&
			revisionOnePlayback.Stage_PrevalidatedDocument(
				vectorDocument, revisionOneBundle, status);
		if (revisionOnePrepared)
			revisionOnePlayback.Seek(1.f / 60.f, identity);
		const f32_t revisionOneX = revisionOnePrepared &&
			!revisionOnePlayback.Get_Frame().Particles.empty() ?
			revisionOnePlayback.Get_Frame().Particles.front().World._41 : 0.f;

		const bool_t revisionTwoWritten = WriteConstantVectorField(
			{ -100.f, 0.f, 0.f });
		std::shared_ptr<const CEffectPlayback::PREPARED_RESOURCES>
			revisionTwoBundle;
		CEffectPlayback revisionTwoPlayback;
		const bool_t revisionTwoPrepared = revisionTwoWritten &&
			CEffectPlayback::Prepare_DocumentResources(
				vectorDocument, revisionTwoBundle, status) &&
			revisionTwoPlayback.Stage_PrevalidatedDocument(
				vectorDocument, revisionTwoBundle, status);
		if (revisionTwoPrepared)
			revisionTwoPlayback.Seek(1.f / 60.f, identity);
		const f32_t revisionTwoX = revisionTwoPrepared &&
			!revisionTwoPlayback.Get_Frame().Particles.empty() ?
			revisionTwoPlayback.Get_Frame().Particles.front().World._41 : 0.f;
		revisionOnePlayback.Seek(1.f / 60.f, identity);
		const f32_t retainedRevisionOneX = revisionOnePrepared &&
			!revisionOnePlayback.Get_Frame().Particles.empty() ?
			revisionOnePlayback.Get_Frame().Particles.front().World._41 : 0.f;
		runner.Require(
			revisionOnePrepared && revisionTwoPrepared &&
			revisionOneBundle != revisionTwoBundle &&
			CEffectPlayback::Get_VectorFieldDiskLoadCount() ==
				vectorLoadsBeforeRevisionBundles + 2u &&
			std::abs(revisionOneX - retainedRevisionOneX) < 0.000001f &&
			revisionTwoX < revisionOneX - 0.01f,
			"Effect Vector Field Same Asset Id Rebuilds Per Revision And Active Bundle Retains Old Samples");

		EFFECT_DOCUMENT_DESC missingVector = vectorDocument;
		missingVector.strEffectAssetId = "effect.vector.missing.harness";
		for (EFFECT_SOURCE_LITERAL_DESC& literal :
			missingVector.Elements.front().SourceRecipe.Modules.back().Literals)
		{
			if (literal.strPropertyPath == "vectorfield.assetid")
				literal.strString = "Effect/Harness/missing.wvectorfield";
		}
		std::shared_ptr<const CEffectPlayback::PREPARED_RESOURCES>
			preservedBundle = revisionOneBundle;
		const bool_t failedPreparationPreserved =
			!CEffectPlayback::Prepare_DocumentResources(
				missingVector, preservedBundle, status) &&
			preservedBundle == revisionOneBundle;
		CEffectPlayback afterFailedPreparation;
		const bool_t oldBundleStillStages = failedPreparationPreserved &&
			afterFailedPreparation.Stage_PrevalidatedDocument(
				vectorDocument, preservedBundle, status);
		if (oldBundleStillStages)
			afterFailedPreparation.Seek(1.f / 60.f, identity);
		runner.Require(
			!vectorPlayback.Stage_Document(missingVector, status) &&
			vectorPlayback.Get_Frame().Particles.size() == 1u &&
			std::abs(vectorPlayback.Get_Frame().Particles.front().World._41 -
				(61.f / 3600.f)) < 0.00001f &&
			oldBundleStillStages &&
			afterFailedPreparation.Get_Frame().Particles.size() == 1u &&
			std::abs(afterFailedPreparation.Get_Frame().Particles.front().World._41 -
				revisionOneX) < 0.000001f,
			"Effect Failed Vector Field Prewarm Leaves No Residue And Preserves Old Bundle");
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
				L"Effects/Authored/effect.dimensionmaster.skill." +
				std::to_wstring(skillId) + L".effect.json");
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
			"Current DimensionMaster Authored Documents Preserve Exact Semantic Module Counts");
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

		const std::filesystem::path hitAPath = CProjectDataRoot::Resolve(
			L"Effects/Authored/effect.dimensionmaster.skill.2050210.effect.json");
		EFFECT_DOCUMENT_DESC hitADocument;
		CEffectPlayback hitAPlayback;
		const bool_t hitAStaged = !hitAPath.empty() &&
			CEffectDocumentCodec::Load(hitAPath, hitADocument, status) &&
			hitAPlayback.Stage_Document(hitADocument, status);
		float4x4_t hitAIdentity{};
		XMStoreFloat4x4(&hitAIdentity, XMMatrixIdentity());
		if (hitAStaged)
			hitAPlayback.Seek(0.282f, hitAIdentity);
		bool_t bHitAImplicitAlphaVisible = hitAStaged;
		for (const std::string_view emitterId : {
			"fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1."
				"particlespriteemitter_14",
			"fx_pc_swp_00.par_j_swp_willowrend_swinghit_00_1."
				"particlespriteemitter_20" })
		{
			EFFECT_PARTICLE_RUNTIME_PROBE probe;
			bHitAImplicitAlphaVisible = bHitAImplicitAlphaVisible &&
				hitAPlayback.Query_ParticleRuntimeProbe(emitterId, probe) &&
				probe.iActiveParticleCount > 0u && probe.fMaxAlpha > 0.f;
		}
		runner.Require(bHitAImplicitAlphaVisible,
			"DimensionMaster A Hit Mesh 14 And 20 Preserve Implicit Alpha Identity");

		const std::filesystem::path dPath = CProjectDataRoot::Resolve(
			L"Effects/Authored/effect.dimensionmaster.skill.2050240.effect.json");
		EFFECT_DOCUMENT_DESC aDocument;
		EFFECT_DOCUMENT_DESC aRoundTrip;
		const bool_t aLoaded = !dPath.empty() &&
			CEffectDocumentCodec::Load(dPath, aDocument, status);
		std::set<std::string> aProfileIds;
		size_t aParticleCount = 0u;
		size_t aEnabledProfileCount = 0u;
		size_t aReconstructedProfileCount = 0u;
		size_t aGroupedShaderCount = 0u;
		size_t aFallbackBlockedShaderCount = 0u;
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
			if (source.strRuntimeShaderProfileId ==
				"effect.ue3.grouped-translucent.v1")
			{
				++aGroupedShaderCount;
			}
			else if (source.strRuntimeShaderProfileId ==
				"effect.ue3.fallback-blocked.v1")
			{
				++aFallbackBlockedShaderCount;
			}
			else if (source.strRuntimeShaderProfileId !=
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
		runner.Require(aLoaded &&
			aDocument.iFormatVersion == EFFECT_AUTHORING_FORMAT_VERSION &&
			aDocument.Elements.size() == 51u && aParticleCount == 46u &&
			aEnabledProfileCount == 46u &&
			aReconstructedProfileCount == 46u &&
			aProfileIds.size() == 21u && aGroupedShaderCount == 30u &&
			aFallbackBlockedShaderCount == 2u &&
			aSpecializedShaderCount == 14u &&
			aDynamicSemanticElementCount == 32u &&
			aSubUVElementCount == 2u &&
			aSourceModuleOccurrenceCount == 518u &&
			aSourceModuleClasses.size() == 28u && aRoundTripped,
			"DimensionMaster D 2050240 V12 Source Material Profiles Round Trip Losslessly");
		runner.Require(aCookedLookupCount == 604u &&
			aMalformedLookupCount == 0u && aEmitter17SizeExact &&
			aEmitter32SizeExact && aEmitter64SizeExact &&
			aXyzRandomLockCount == 13u,
			"DimensionMaster D 2050240 Cooked Distribution Payloads And Sprite Sizes Are Exact");
		runner.Require(aSourceColorElementCount == 46u &&
			aSourceColorOverrideViolationCount == 0u &&
			aSourceSubUVOverrideViolationCount == 0u,
			"DimensionMaster D 2050240 Source Color And SubUV Baselines Execute Once");
		resourceRootEnvironment.Restore();
	}

	void Test_DimensionMasterImportedPortalDraft(TEST_RUNNER& runner)
	{
		using namespace Client;
		SCOPED_ENVIRONMENT_VARIABLE resourceRootEnvironment(
			L"LOSTARK_RESOURCE_ROOT");
		const std::filesystem::path resourceRoot =
			CProjectDataRoot::Get().parent_path() /
			L"Client" / L"Bin" / L"Resources";
		resourceRootEnvironment.Set(resourceRoot.c_str());
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
		SCOPED_ENVIRONMENT_VARIABLE resourceRootEnvironment(
			L"LOSTARK_RESOURCE_ROOT");
		const std::filesystem::path resourceRoot =
			CProjectDataRoot::Get().parent_path() /
			L"Client" / L"Bin" / L"Resources";
		resourceRootEnvironment.Set(resourceRoot.c_str());
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
		SCOPED_ENVIRONMENT_VARIABLE resourceRootEnvironment(
			L"LOSTARK_RESOURCE_ROOT");
		const std::filesystem::path resourceRoot =
			CProjectDataRoot::Get().parent_path() /
			L"Client" / L"Bin" / L"Resources";
		resourceRootEnvironment.Set(resourceRoot.c_str());
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
		sprite.Detail.Sprite.bBillboard = true;
		sprite.Detail.Sprite.fBillboardRollDegrees = -90.f;
		document.Elements.push_back(sprite);

		std::string status;
		EFFECT_DOCUMENT_DESC spriteRoundTrip;
		runner.Require(
			CEffectDocumentCodec::Parse(
				CEffectDocumentCodec::Serialize(document),
				spriteRoundTrip, status) &&
			spriteRoundTrip.Elements.front().Detail.Sprite.
				fBillboardRollDegrees == -90.f,
			"Effect Authored Sprite Billboard Roll Round Trips Without Global Yaw");
		EFFECT_DOCUMENT_DESC invalidSpriteRoll = document;
		invalidSpriteRoll.Elements.front().Detail.Sprite.
			fBillboardRollDegrees = 3601.f;
		runner.Require(
			!CEffectDocumentCodec::Validate(invalidSpriteRoll, status),
			"Effect Authored Sprite Billboard Roll Rejects Invalid Range");
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
		runner.Require(
			Is_UnsafeEffectBaseTextureAssetId("") &&
			Is_UnsafeEffectBaseTextureAssetId(
				"Effect/Test/T_blankwhite_00.dds") &&
			Is_UnsafeEffectBaseTextureAssetId(
				"Effect/Test/T_NormalMap_00.dds") &&
			Is_UnsafeEffectBaseTextureAssetId(
				"Effect/Test/T_BUMP_00.dds") &&
			Is_UnsafeEffectBaseTextureAssetId(
				"Effect/Test/fx_surface_n.dds") &&
			Is_UnsafeEffectBaseTextureAssetId(
				"Effect/Test/fx_surface_n_01.dds") &&
			!Is_UnsafeEffectBaseTextureAssetId(
				"Effect/Test/T_BaseColor_00.dds"),
			"Effect Runtime And Tool Share Unsafe Base Texture Classification");
		runner.Require(
			!Is_EffectManualMeshAuthoringContractSatisfied(
				false, true, false) &&
			!Is_EffectManualMeshAuthoringContractSatisfied(
				true, false, false) &&
			!Is_EffectManualMeshAuthoringContractSatisfied(
				true, true, true) &&
			Is_EffectManualMeshAuthoringContractSatisfied(
				true, true, false),
			"Manual Mesh Authoring Requires Mesh And Safe Base Only");
		runner.Require(
			!Is_EffectManualMeshCreateReady("", true, true, false) &&
			Is_EffectManualMeshCreateReady(
				"effect.manual.mesh.named", true, true, false),
			"Manual Mesh Create Requires Name But Not A Preselected Data File");

		EFFECT_DOCUMENT_DESC manualMeshDocument;
		manualMeshDocument.strEffectAssetId = "effect.manual.mesh.harness";
		manualMeshDocument.strDisplayName = "Manual Mesh Harness";
		EFFECT_ELEMENT_DESC manualMesh;
		manualMesh.strElementId = "mesh_layer_1";
		manualMesh.strDisplayName = "Mesh Layer 1";
		manualMesh.eKind = EFFECT_ELEMENT_KIND::MESH;
		manualMesh.Material.strTemplateId = "effect.standard";
		manualMesh.Material.eRenderProfile =
			EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ;
		manualMesh.Detail.Mesh.bUseModelMaterial = false;
		manualMesh.Detail.Transform.vScale = {
			EFFECT_MANUAL_MESH_DEFAULT_SCALE,
			EFFECT_MANUAL_MESH_DEFAULT_SCALE,
			EFFECT_MANUAL_MESH_DEFAULT_SCALE };
		manualMesh.ResourceBindings = {
			{ "meshModel",
				"Effect/DimensionMaster/Meshes/bfm_q_crack_01.wmodel" },
			{ "base",
				"Effect/DimensionMaster/Textures/EFMASTER_MATERIAL_PROLOGUE/fx_a_ice_003.dds" }
		};
		manualMeshDocument.Elements.push_back(manualMesh);
		EFFECT_DOCUMENT_DESC manualMeshRoundTrip;
		runner.Require(
			CEffectDocumentCodec::Validate_Drawable(
				manualMeshDocument, status) &&
			CEffectDocumentCodec::Parse(
				CEffectDocumentCodec::Serialize(manualMeshDocument),
				manualMeshRoundTrip, status) &&
			manualMeshRoundTrip.Elements.size() == 1u &&
			manualMeshRoundTrip.Elements.front().eKind ==
				EFFECT_ELEMENT_KIND::MESH &&
			!manualMeshRoundTrip.Elements.front().Detail.Mesh.
				bUseModelMaterial &&
			manualMeshRoundTrip.Elements.front().Detail.Transform.vScale.x ==
				EFFECT_MANUAL_MESH_DEFAULT_SCALE &&
			manualMeshRoundTrip.Elements.front().Detail.Transform.vScale.y ==
				EFFECT_MANUAL_MESH_DEFAULT_SCALE &&
			manualMeshRoundTrip.Elements.front().Detail.Transform.vScale.z ==
				EFFECT_MANUAL_MESH_DEFAULT_SCALE &&
			manualMeshRoundTrip.Elements.front().ResourceBindings.size() == 2u,
			"Manual Mesh Create Contract Round Trips One Percent Scale Carrier Layer");

		EFFECT_DOCUMENT_DESC sourceMaterialDocument = document;
		EFFECT_MATERIAL_DESC& sourceMaterial =
			sourceMaterialDocument.Elements.front().Material;
		sourceMaterial.strTemplateId = "effect.source_material";
		sourceMaterial.strSourceMaterialPath = "harness.material.source";
		sourceMaterial.SourceMaterial.bEnabled = true;
		sourceMaterial.SourceMaterial.strProfileId =
			"harness.material.profile";
		sourceMaterial.SourceMaterial.strRuntimeShaderProfileId =
			"effect.ue3.reconstructed-standard.v1";
		sourceMaterial.SourceMaterial.strParentMaterialPath =
			"harness.material.parent";
		sourceMaterial.SourceMaterial.eStatus =
			EFFECT_SOURCE_MATERIAL_STATUS::RECONSTRUCTED_PROFILE;
		runner.Require(
			CEffectDocumentCodec::Validate(sourceMaterialDocument, status),
			"Effect Authoring Accepts Registered Source Material Runtime Contract");
		EFFECT_DOCUMENT_DESC groupedMaterialDocument = sourceMaterialDocument;
		EFFECT_SOURCE_MATERIAL_DESC& groupedSource =
			groupedMaterialDocument.Elements.front().Material.SourceMaterial;
		groupedSource.strRuntimeShaderProfileId =
			"effect.ue3.grouped-translucent.v1";
		groupedSource.Scalars = {
			{ "02.Map_A_UVScale_R", "01_Alpha", 2.f },
			{ "03.mAP_a_UvScAlE_g", "01_ALPHA", 3.f },
			{ "04.Map_A_Panning_X", "01_aLpHa", 0.25f },
			{ "05.map_A_pAnNiNg_Y", "01_Alpha", -0.5f },
			{ "36.Str", "01_Alpha", 0.5f },
			{ "37.POWER", "01_Alpha", 2.f },
			{ "92.Emissiion_Str", "02_Emission", 4.f },
			{ "93.EMISSIION_POWER", "02_EMISSION", 1.5f },
			{ "05.Distort_Str", "08_UVDistort", 0.2f },
			{ "Dissolve_Hardness", "07_Dissolve", 10.f }
		};
		groupedSource.Vectors = {
			{ "94.Emissiion_Color", "02_Emission",
				{ 0.25f, 0.5f, 1.f, 1.f } }
		};
		const EFFECT_GROUPED_TRANSLUCENT_CONSTANTS groupedConstants =
			Build_EffectGroupedTranslucentConstants(groupedSource);
		EFFECT_DOCUMENT_DESC groupedRoundTrip;
		const bool_t groupedRoundTripExact =
			CEffectDocumentCodec::Validate(groupedMaterialDocument, status) &&
			CEffectDocumentCodec::Parse(
				CEffectDocumentCodec::Serialize(groupedMaterialDocument),
				groupedRoundTrip, status) &&
			groupedRoundTrip.Elements.front().Material.SourceMaterial.
				Scalars.front().strGroup == "01_Alpha" &&
			groupedRoundTrip.Elements.front().Material.SourceMaterial.
				Vectors.front().strGroup == "02_Emission";
		runner.Require(groupedRoundTripExact &&
			groupedConstants.vUVScalePan.x == 2.f &&
			groupedConstants.vUVScalePan.y == 3.f &&
			groupedConstants.vUVScalePan.z == 0.25f &&
			groupedConstants.vUVScalePan.w == -0.5f &&
			groupedConstants.vAlphaEmissive.x == 0.5f &&
			groupedConstants.vAlphaEmissive.y == 2.f &&
			groupedConstants.vAlphaEmissive.z == 4.f &&
			groupedConstants.vAlphaEmissive.w == 1.5f &&
			groupedConstants.vNoiseDissolve.y == 0.2f &&
			groupedConstants.vNoiseDissolve.w == 10.f &&
			groupedConstants.vTint.z == 1.f,
			"Effect Grouped Translucent Packs Named Parameters By Name And Group");
		runner.Require(
			!Is_EffectGroupedTranslucentResourceContractSatisfied(
				groupedSource, false, false, false, false) &&
			Is_EffectGroupedTranslucentResourceContractSatisfied(
				groupedSource, true, false, false, false),
			"Effect Grouped Translucent Fails Closed Without Alpha Or Emission Resource");
		EFFECT_SOURCE_MATERIAL_DESC emissionOnlySource;
		emissionOnlySource.Scalars = {
			{ "Emission_Strength", "Emission", 3.f }
		};
		runner.Require(
			!Is_EffectGroupedTranslucentResourceContractSatisfied(
				emissionOnlySource, false, true, false, false) &&
			Is_EffectGroupedTranslucentResourceContractSatisfied(
				emissionOnlySource, false, false, true, false),
			"Effect Grouped Translucent Requires Emission Carrier For Emission Profile");
		runner.Require(
			Is_EffectFiniteProfileResourceContractSatisfied(
				"effect.ue3.shine.v1", true, true, false, false) &&
			!Is_EffectFiniteProfileResourceContractSatisfied(
				"effect.ue3.shine.v1", true, false, false, false) &&
			Is_EffectFiniteProfileResourceContractSatisfied(
				"effect.ue3.blackline-aura.v1", false, true, true, true) &&
			!Is_EffectFiniteProfileResourceContractSatisfied(
				"effect.ue3.local-crack.v1", true, true, true, true) &&
			Is_EffectFiniteProfileResourceContractSatisfied(
				"effect.ue3.missiletrail-01.v1", true, true, true, true) &&
			!Is_EffectFiniteProfileResourceContractSatisfied(
				"effect.ue3.missiletrail-01.v1", true, false, true, true) &&
			Is_EffectFiniteProfileResourceContractSatisfied(
				"effect.ue3.procedural-center-glow.v1",
				false, false, false, false),
			"Effect Finite Source Profiles Require Their Decoded Runtime Carriers");
		runner.Require(
			Is_EffectLocalCrackResourceContractSatisfied(
				true, true, true, true) &&
			!Is_EffectLocalCrackResourceContractSatisfied(
				false, true, true, true) &&
			!Is_EffectLocalCrackResourceContractSatisfied(
				true, false, true, true) &&
			!Is_EffectLocalCrackResourceContractSatisfied(
				true, true, false, true) &&
			!Is_EffectLocalCrackResourceContractSatisfied(
				true, true, true, false),
			"Effect Local Crack Requires Named Normal Reflection Dissolve And Mesh");
		EFFECT_DOCUMENT_DESC legacyLocalCrackDocument = sourceMaterialDocument;
		EFFECT_ELEMENT_DESC& legacyLocalCrackElement =
			legacyLocalCrackDocument.Elements.front();
		legacyLocalCrackElement.eKind = EFFECT_ELEMENT_KIND::PARTICLE;
		legacyLocalCrackElement.ResourceBindings = {
			{ "meshModel",
				"Effect/DimensionMaster/Meshes/fm_a_broken_012.wmodel" },
			{ "dissolve",
				"Effect/DimensionMaster/Textures/FX_TEX_04/fx_h_atypical_01_1.dds" }
		};
		EFFECT_SOURCE_MATERIAL_DESC& legacyLocalCrackSource =
			legacyLocalCrackElement.Material.SourceMaterial;
		legacyLocalCrackSource.strRuntimeShaderProfileId =
			"effect.ue3.local-crack.v1";
		legacyLocalCrackSource.Textures.clear();
		EFFECT_DOCUMENT_DESC partialLocalCrackDocument =
			legacyLocalCrackDocument;
		partialLocalCrackDocument.Elements.front().Material.SourceMaterial.Textures = {
			{ "normal_tex", "", "harness.localcrack.normal",
				"Effect/DimensionMaster/Textures/FX_TEX_06/fx_j_normal_bc5_09.dds",
				EFFECT_TEXTURE_ADDRESS_MODE::WRAP,
				EFFECT_TEXTURE_ADDRESS_MODE::WRAP,
				EFFECT_TEXTURE_COLOR_SPACE::LINEAR, "upk_props" }
		};
		EFFECT_DOCUMENT_DESC completeLocalCrackDocument =
			partialLocalCrackDocument;
		completeLocalCrackDocument.Elements.front().Material.SourceMaterial.Textures.
			push_back({ "refle_tex", "", "harness.localcrack.reflection",
				"Effect/DimensionMaster/Textures/FX_TEX_00/fx_b_atypical_004_cube.dds",
				EFFECT_TEXTURE_ADDRESS_MODE::WRAP,
				EFFECT_TEXTURE_ADDRESS_MODE::WRAP,
				EFFECT_TEXTURE_COLOR_SPACE::SRGB, "upk_props" });
		completeLocalCrackDocument.Elements.front().Material.SourceMaterial.Textures.
			push_back({ "dissolve_tex", "", "harness.localcrack.dissolve",
				"Effect/DimensionMaster/Textures/FX_TEX_04/fx_h_atypical_01_1.dds",
				EFFECT_TEXTURE_ADDRESS_MODE::WRAP,
				EFFECT_TEXTURE_ADDRESS_MODE::WRAP,
				EFFECT_TEXTURE_COLOR_SPACE::SRGB, "upk_props" });
		runner.Require(
			CEffectDocumentCodec::Validate_Drawable(
				legacyLocalCrackDocument, status) &&
			!CEffectDocumentCodec::Validate_Drawable(
				partialLocalCrackDocument, status) &&
			CEffectDocumentCodec::Validate_Drawable(
				completeLocalCrackDocument, status),
			"Effect Local Crack Admits Legacy Carrier But Rejects Partial Named Contract");
		EFFECT_SOURCE_MATERIAL_DESC stagedSourceSignature = groupedSource;
		EFFECT_SOURCE_MATERIAL_DESC changedSourceSignature = groupedSource;
		changedSourceSignature.Scalars.front().fValue += 1.f;
		runner.Require(
			Is_EffectSourceMaterialStagingSignatureEqual(
				stagedSourceSignature, stagedSourceSignature) &&
			!Is_EffectSourceMaterialStagingSignatureEqual(
				stagedSourceSignature, changedSourceSignature),
			"Effect Material Value Edits Invalidate Renderer Staging Constants");
		EFFECT_DOCUMENT_DESC fallbackBlockedMaterial = sourceMaterialDocument;
		fallbackBlockedMaterial.Elements.front().Material.SourceMaterial.
			strRuntimeShaderProfileId = "effect.ue3.fallback-blocked.v1";
		fallbackBlockedMaterial.Elements.front().Material.eRenderProfile =
			EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ;
		EFFECT_DOCUMENT_DESC fallbackBlockedRoundTrip;
		runner.Require(
			CEffectDocumentCodec::Validate(fallbackBlockedMaterial, status) &&
			CEffectDocumentCodec::Parse(
				CEffectDocumentCodec::Serialize(fallbackBlockedMaterial),
				fallbackBlockedRoundTrip, status) &&
			fallbackBlockedRoundTrip.Elements.front().Material.
				SourceMaterial.strRuntimeShaderProfileId ==
					"effect.ue3.fallback-blocked.v1" &&
			fallbackBlockedRoundTrip.Elements.front().Material.eRenderProfile ==
				EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ,
			"Effect Authoring Round Trips Fail-Closed One-Sided Material Profile");
		EFFECT_DOCUMENT_DESC fallbackBlockedMesh = fallbackBlockedMaterial;
		EFFECT_ELEMENT_DESC& fallbackMeshElement =
			fallbackBlockedMesh.Elements.front();
		fallbackMeshElement.bVisible = true;
		fallbackMeshElement.eKind = EFFECT_ELEMENT_KIND::PARTICLE;
		fallbackMeshElement.Material.strTemplateId = "effect.standard";
		fallbackMeshElement.Detail.Mesh.bUseModelMaterial = false;
		fallbackMeshElement.ResourceBindings = {
			{ "meshModel",
				"Effect/DimensionMaster/Meshes/fm_d_crack_037.wmodel" },
			{ "dissolve",
				"Effect/DimensionMaster/Textures/FX_TEX_04/fx_h_atypical_01_1.dds" }
		};
		runner.Require(
			CEffectDocumentCodec::Validate_Drawable(
				fallbackBlockedMesh, status),
			"Effect Fail-Closed Mesh Without Base Stages For Hidden Rendering");
		EFFECT_DOCUMENT_DESC unprotectedMesh = fallbackBlockedMesh;
		unprotectedMesh.Elements.front().Material.SourceMaterial.
			strRuntimeShaderProfileId =
				"effect.ue3.reconstructed-standard.v1";
		runner.Require(
			!CEffectDocumentCodec::Validate_Drawable(unprotectedMesh, status),
			"Effect Unprotected Mesh Without Base Still Fails Drawable Validation");
		EFFECT_DOCUMENT_DESC unknownShaderProfile = sourceMaterialDocument;
		unknownShaderProfile.Elements.front().Material.SourceMaterial.
			strRuntimeShaderProfileId = "effect.ue3.merge-typo.v1";
		runner.Require(
			!CEffectDocumentCodec::Validate(unknownShaderProfile, status),
			"Effect Authoring Rejects Unknown Source Material Shader Profile Before Draw");
		EFFECT_DOCUMENT_DESC unknownDynamicSemantic = sourceMaterialDocument;
		unknownDynamicSemantic.Elements.front().Material.SourceMaterial.
			DynamicParameterSemantics[0] = "merge_typo";
		runner.Require(
			!CEffectDocumentCodec::Validate(unknownDynamicSemantic, status),
			"Effect Authoring Rejects Unknown Dynamic Parameter Semantic Before Draw");
		EFFECT_DOCUMENT_DESC unknownSubUVMode = sourceMaterialDocument;
		unknownSubUVMode.Elements.front().Material.SourceMaterial.strSubUVMode =
			"psuvim_merge_typo";
		runner.Require(
			!CEffectDocumentCodec::Validate(unknownSubUVMode, status),
			"Effect Authoring Rejects Unknown Source Material SubUV Mode Before Draw");

		EFFECT_DOCUMENT_DESC sourceRecipeDocument = document;
		EFFECT_ELEMENT_DESC sourceDraft =
			sourceRecipeDocument.Elements.front();
		sourceDraft.strDisplayName = "Applied SourceRecipe Draft";
		sourceDraft.SourceRecipe.bEnabled = true;
		sourceDraft.SourceRecipe.strRendererShape = "sprite";
		sourceDraft.SourceRecipe.fEmitterDurationSeconds = 1.f;
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
		sourceDraft.SourceRecipe.Modules.push_back(sourceModule);
		Apply_EffectElementDetailDraft(
			sourceRecipeDocument.Elements.front(), sourceDraft);
		EFFECT_DOCUMENT_DESC sourceRoundTrip;
		const float4_t evaluatedRate = CEffectDistribution::Evaluate(
			sourceRate, 0.5f, 0.25f);
		runner.Require(
			CEffectDocumentCodec::Validate(sourceRecipeDocument, status) &&
			CEffectDocumentCodec::Parse(
				CEffectDocumentCodec::Serialize(sourceRecipeDocument),
				sourceRoundTrip, status) &&
			sourceRecipeDocument.Elements.front().strElementId ==
				"unbound_sprite" &&
			sourceRoundTrip.Elements.front().strDisplayName ==
				"Applied SourceRecipe Draft" &&
			sourceRoundTrip.Elements.front().SourceRecipe.bEnabled &&
			sourceRoundTrip.Elements.front().SourceRecipe.Modules.size() == 1u &&
			evaluatedRate.x == 12.f,
			"Effect Detail Apply And Source Recipe Round Trip Losslessly");

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
			CEffectDocumentCodec::Save_AtomicIfUnchanged(
				path, sourceRecipeDocument, std::string_view{}, status) &&
			CEffectDocumentCodec::Load(path, loaded, status) &&
			loaded.strEffectAssetId == document.strEffectAssetId &&
			loaded.Elements.size() == 1u &&
			loaded.Elements.front().strDisplayName ==
				"Applied SourceRecipe Draft" &&
			loaded.Elements.front().strGroupId == "harness_group" &&
			loaded.Elements.front().strSourceNode == "Harness/Node/0" &&
			!loaded.Elements.front().bVisible &&
		loaded.Elements.front().Material.strTemplateId == "effect.standard" &&
		loaded.Elements.front().SourceRecipe.bEnabled &&
		loaded.Elements.front().SourceRecipe.Modules.size() == 1u &&
		loaded.Elements.front().SourceRecipe.Modules.front().strStableId ==
			"harness:module:spawn" &&
		loaded.Elements.front().Detail.Particle.vInitialPositionMin.x == 0.f &&
		loaded.Elements.front().Detail.Particle.vInitialPositionMax.x == 0.f &&
		loaded.Elements.front().ResourceBindings.empty(),
			"Effect Authoring Atomically Saves And Reloads V12 Partial SourceRecipe Draft");

		const std::string loadedCanonical =
			CEffectDocumentCodec::Serialize(loaded);
		EFFECT_DOCUMENT_DESC externalEdit = loaded;
		externalEdit.strDisplayName = "External Writer Won";
		const bool_t externalSaved = CEffectDocumentCodec::Save_Atomic(
			path, externalEdit, status);
		EFFECT_DOCUMENT_DESC staleDraft = loaded;
		staleDraft.strDisplayName = "Stale Tool Lost";
		const bool_t staleRejected = externalSaved &&
			!CEffectDocumentCodec::Save_AtomicIfUnchanged(
				path, staleDraft, loadedCanonical, status);
		EFFECT_DOCUMENT_DESC preservedExternal;
		const bool_t externalPreserved = staleRejected &&
			CEffectDocumentCodec::Load(path, preservedExternal, status) &&
			preservedExternal.strDisplayName == "External Writer Won";
		runner.Require(externalPreserved,
			"Effect Authoring Rejects Stale Save And Preserves External Writer");
		std::filesystem::remove(path, error);

		const std::filesystem::path qPath = CProjectDataRoot::Resolve(
			L"Effects/Authored/effect.dimensionmaster.skill.2050100.effect.json");
		EFFECT_DOCUMENT_DESC qDocument;
		const std::string qElementId =
			"fx_pc_swp_00.par_j_swp_nailstrike_00_1.particlespriteemitter_8";
		const bool_t qLoaded = !qPath.empty() &&
			CEffectDocumentCodec::Load(qPath, qDocument, status);
		auto qElement = qLoaded ? std::find_if(
			qDocument.Elements.begin(), qDocument.Elements.end(),
			[&qElementId](const EFFECT_ELEMENT_DESC& element)
			{
				return element.strElementId == qElementId;
			}) : qDocument.Elements.end();
		const std::filesystem::path qDraftPath =
			std::filesystem::temp_directory_path() /
			"lostark-effect-q-material-draft.effect.json";
		std::filesystem::remove(qDraftPath, error);
		bool_t qMaterialRoundTrip = false;
		if (qLoaded && qElement != qDocument.Elements.end() &&
			!qElement->Material.SourceMaterial.Scalars.empty())
		{
			EFFECT_NAMED_FLOAT_DESC& scalar =
				qElement->Material.SourceMaterial.Scalars.front();
			const std::string scalarName = scalar.strName;
			const f32_t authoredValue = scalar.fValue + 0.125f;
			scalar.fValue = authoredValue;
			qElement->Material.SourceMaterial.eStatus =
				EFFECT_SOURCE_MATERIAL_STATUS::RECONSTRUCTED_PROFILE;
			EFFECT_DOCUMENT_DESC qReloaded;
			qMaterialRoundTrip =
				CEffectDocumentCodec::Save_AtomicIfUnchanged(
					qDraftPath, qDocument, std::string_view{}, status) &&
				CEffectDocumentCodec::Load(qDraftPath, qReloaded, status) &&
				CEffectDocumentCodec::Serialize(qReloaded) ==
					CEffectDocumentCodec::Serialize(qDocument) &&
				CEffectDocumentCodec::Validate_Drawable(qReloaded, status);
			if (qMaterialRoundTrip)
			{
				const auto reloadedElement = std::find_if(
					qReloaded.Elements.begin(), qReloaded.Elements.end(),
					[&qElementId](const EFFECT_ELEMENT_DESC& element)
					{
						return element.strElementId == qElementId;
					});
				qMaterialRoundTrip =
					reloadedElement != qReloaded.Elements.end() &&
					!reloadedElement->Material.SourceMaterial.Scalars.empty() &&
					reloadedElement->Material.SourceMaterial.Scalars.front().strName ==
						scalarName &&
					reloadedElement->Material.SourceMaterial.Scalars.front().fValue ==
						authoredValue &&
					reloadedElement->Material.SourceMaterial.eStatus ==
						EFFECT_SOURCE_MATERIAL_STATUS::RECONSTRUCTED_PROFILE;
			}
		}
		runner.Require(qMaterialRoundTrip,
			"DimensionMaster Q Stable Emitter Material Tuning Saves And Reloads Losslessly");
		std::filesystem::remove(qDraftPath, error);
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
		EFFECT_BUILD_CONTRACT buildContract;
		std::string buildContractStatus;
		const bool_t buildContractLoaded =
			Load_FourClassEffectBuildContract(
				buildContract, buildContractStatus);
		const bool_t loaded = !error && CEffectCatalog::Load(status);
		if (!loaded)
			std::cout << "[DETAIL] Effect runtime catalog: " << status << '\n';
		const std::vector<std::string> effectIds =
			CEffectCatalog::Get_EffectAssetIds();
		const std::set<std::string> actualEffectIdSet(
			effectIds.begin(), effectIds.end());
		std::set<std::string> contractEffectIds;
		for (const auto& entry : buildContract.Effects)
			contractEffectIds.insert(entry.first);
		bool_t hierarchyComplete = loaded && buildContractLoaded &&
			actualEffectIdSet == contractEffectIds &&
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
			assemblyEmitterCount == buildContract.iEmitterCount,
			"Four-Class Effect Runtime Matches Generated Assembly Component Emitter Contract");
		runner.Require(playbackStageComplete &&
			stagedEffectCount == buildContract.iEffectCount,
			"Effect Runtime Stages Every Four-Class Product Document");
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
			/* Which effect survives does not matter; that the committed set
			survives does. Naming one here ties the rollback check to whatever
			the authored rollout happens to call that skill. */
			const std::string committedId = effectIds.empty() ?
				std::string{} : effectIds.front();
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
				!committedId.empty() && CEffectCatalog::Contains(committedId),
				"Effect Runtime Invalid Catalog Preserves Committed Assembly State");
		}

		CEffectCatalog::Clear();
		error.clear();
		std::filesystem::remove(stagedCatalog, error);
		resourceRootEnvironment.Restore();
	}

	std::string Build_RuntimeAuthorityFixtureEntry()
	{
		using namespace Client;
		const std::string HashA(64u, 'a');
		const std::string HashB(64u, 'b');
		const std::string HashC(64u, 'c');
		const std::string HashD(64u, 'd');
		const std::string HashE(64u, 'e');
		const std::string HashF(64u, 'f');
		const std::string Hash1(64u, '1');
		const std::string Hash2(64u, '2');
		const std::string Hash3(64u, '3');
		const std::string Hash4(64u, '4');
		const std::string Hash5(64u, '5');
		const std::string Hash6(64u, '6');
		const std::string Hash9(64u, '9');
		const std::string Identity =
			"{\"schema\":\"lostark.effect-derived-identity\","
			"\"formatVersion\":1,\"sourceContractHash\":\"" + HashA +
			"\",\"sourceSemanticClosureHash\":\"" + HashB +
			"\",\"geometryContractHash\":\"" + HashC +
			"\",\"materialContractHash\":\"" + HashD +
			"\",\"resourceBindingHash\":\"" + HashE +
			"\",\"compilerInputHash\":\"" + HashF + "\"}";
		const std::string Contract =
			"{\"artifactBindingBlockerSet\":[],"
			"\"artifactBindingBlockerCount\":0,"
			"\"executionBlockerSet\":[],\"executionBlockerCount\":0,"
			"\"executionAdmission\":true}";
		const std::string Ir =
			"{\"schema\":\"lostark.effect-compiled-ir\","
			"\"formatVersion\":1,"
			"\"effectAssetId\":\"effect.fixture.runtime.authority\","
			"\"artifactRevision\":1,"
			"\"compilerRevision\":\"cascade.runtime.fixture.v1\","
			"\"runtimeSemanticAuthority\":\"IMMUTABLE_COMPILED_IR\","
			"\"derivedIdentity\":" + Identity +
			",\"executionContract\":" + Contract +
			",\"program\":{\"opcodes\":[],\"resourceBindings\":[],"
			"\"handlerReceipts\":[]}}";
		DATA_JSON_VALUE IrValue;
		std::string Error;
		if (!CDataJson::Parse(Ir, IrValue, Error))
			return {};
		const std::string IrSha =
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(IrValue));
		const std::string Artifact =
			"{\"schema\":\"lostark.effect-compiled-artifact\","
			"\"formatVersion\":1,"
			"\"effectAssetId\":\"effect.fixture.runtime.authority\","
			"\"artifactRevision\":1,"
			"\"compilerRevision\":\"cascade.runtime.fixture.v1\","
			"\"runtimeSemanticAuthority\":\"IMMUTABLE_COMPILED_IR\","
			"\"derivedIdentity\":" + Identity +
			",\"compiledIrSha256\":\"" + IrSha +
			"\",\"compilerReceiptTokenSha256\":\"" + Hash9 +
			"\",\"compiledIr\":" + Ir +
			",\"executionAdmission\":true,\"productAdmission\":false}";
		DATA_JSON_VALUE ArtifactValue;
		if (!CDataJson::Parse(Artifact, ArtifactValue, Error))
			return {};
		const std::string ArtifactSha =
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				CEffectRuntimeAuthorityCodec::Serialize_PrettyJson(ArtifactValue));
		const std::string ToolDependencies =
			"[{\"role\":\"DERIVED_ARTIFACT_GENERATOR\","
			"\"path\":\"Tools/EffectPipeline/build_effect_derived_artifact.py\","
			"\"rawSha256\":\"" + Hash1 +
			"\",\"canonicalSha256\":\"" + Hash2 +
			"\",\"hashDomain\":\"TRACKED_SOURCE_EOL_CANONICAL_TEXT\","
			"\"verificationRole\":\"CANONICAL_REQUIRED_RAW_OBSERVED\"},"
			"{\"role\":\"DERIVED_ARTIFACT_SCHEMA\","
			"\"path\":\"Tools/EffectPipeline/Schemas/contract.json\","
			"\"rawSha256\":\"" + Hash2 +
			"\",\"canonicalSha256\":\"" + Hash3 +
			"\",\"hashDomain\":\"CANONICAL_JSON\","
			"\"verificationRole\":\"CANONICAL_REQUIRED_RAW_OBSERVED\"},"
			"{\"role\":\"EFFECT_PUBLISHER\","
			"\"path\":\"Tools/EffectPipeline/Publish-Effects.ps1\","
			"\"rawSha256\":\"" + Hash3 +
			"\",\"canonicalSha256\":\"" + Hash4 +
			"\",\"hashDomain\":\"TRACKED_SOURCE_EOL_CANONICAL_TEXT\","
			"\"verificationRole\":\"CANONICAL_REQUIRED_RAW_OBSERVED\"}]";
		const std::string Receipt =
			"{\"schema\":\"lostark.effect-compiled-artifact-receipt\","
			"\"formatVersion\":1,"
			"\"effectAssetId\":\"effect.fixture.runtime.authority\","
			"\"artifactRevision\":1,"
			"\"compilerRevision\":\"cascade.runtime.fixture.v1\","
			"\"runtimeSemanticAuthority\":\"IMMUTABLE_COMPILED_IR\","
			"\"derivedIdentity\":" + Identity +
			",\"sourceContractVersion\":14,"
			"\"authoringCarrierSha256\":\"" + Hash4 +
			"\",\"assemblySha256\":\"" + Hash5 +
			"\",\"compiledArtifactSha256\":\"" + ArtifactSha +
			"\",\"compiledIrSha256\":\"" + IrSha +
			"\",\"compilerReceiptRawSha256\":\"" + Hash5 +
			"\",\"compilerReceiptCanonicalSha256\":\"" + Hash6 +
			"\",\"compilerReceiptTokenSha256\":\"" + Hash9 +
			"\",\"toolDependencies\":" + ToolDependencies +
			",\"artifactBindingBlockerSet\":[],"
			"\"artifactBindingBlockerCount\":0,\"executionBlockerSet\":[],"
			"\"executionBlockerCount\":0,\"executionAdmission\":true,"
			"\"productAdmission\":false,"
			"\"publicationState\":\"CODE_ONLY_NOT_ADMITTED\"}";
		DATA_JSON_VALUE ReceiptValue;
		if (!CDataJson::Parse(Receipt, ReceiptValue, Error))
			return {};
		const std::string ReceiptSha =
			CEffectRuntimeAuthorityCodec::Compute_Sha256Hex(
				CEffectRuntimeAuthorityCodec::Serialize_PrettyJson(ReceiptValue));
		return
			"{\"payloadKind\":\"IMMUTABLE_COMPILED_IR\","
			"\"effectAssetId\":\"effect.fixture.runtime.authority\","
			"\"authoringFormatVersion\":13,"
			"\"runtimeSemanticAuthority\":\"IMMUTABLE_COMPILED_IR\","
			"\"derivedIdentity\":" + Identity +
			",\"authoringCarrierSha256\":\"" + Hash4 +
			"\",\"assemblySha256\":\"" + Hash5 +
			"\",\"compiledArtifactSha256\":\"" + ArtifactSha +
			"\",\"compiledReceiptSha256\":\"" + ReceiptSha +
			"\",\"artifactRevision\":1,"
			"\"compilerRevision\":\"cascade.runtime.fixture.v1\","
			"\"compiledIrSha256\":\"" + IrSha +
			"\",\"compilerReceiptTokenSha256\":\"" + Hash9 +
			"\",\"executionAdmission\":true,\"productAdmission\":false,"
			"\"compiledArtifact\":" + Artifact +
			",\"compiledReceipt\":" + Receipt + "}";
	}

	void Test_EffectRuntimeAuthorityCatalog(TEST_RUNNER& runner)
	{
		using namespace Client;
		DATA_JSON_VALUE NumberTokens;
		std::string NumberStatus;
		const bool_t CanonicalNumberTokens = CDataJson::Parse(
			"{\"d\":1e0,\"c\":-0.0,\"b\":1,\"a\":1.0}",
			NumberTokens, NumberStatus) &&
			CEffectRuntimeAuthorityCodec::Serialize_CanonicalJson(NumberTokens) ==
				"{\"a\":1.0,\"b\":1,\"c\":-0.0,\"d\":1.0}";
		runner.Require(CanonicalNumberTokens,
			"Runtime Authority Canonical JSON Preserves Integer And Float Token Domains");
		const std::string EntryText = Build_RuntimeAuthorityFixtureEntry();
		DATA_JSON_VALUE Entry;
		std::string Status;
		std::shared_ptr<const EFFECT_COMPILED_RUNTIME_DOCUMENT> Parsed;
		const bool_t ParsedFixture = !EntryText.empty() &&
			CDataJson::Parse(EntryText, Entry, Status) &&
			CEffectRuntimeAuthorityCodec::Parse_DerivedEntry(
				Entry, Parsed, Status);
		runner.Require(ParsedFixture && nullptr != Parsed &&
			Parsed->Identity.strEffectAssetId ==
				"effect.fixture.runtime.authority" &&
			Parsed->Identity.iArtifactRevision == 1u &&
			Parsed->bArtifactBindingSelfConsistent &&
			!Parsed->bExternalIdentityAuthenticated &&
			Parsed->bArtifactExecutionAdmission &&
			!Parsed->bTypedProgramMaterialized &&
			!Parsed->bRuntimeExecutionAdmission &&
			!Parsed->bProductAdmission && Parsed->iOpcodeCount == 0u &&
			Parsed->iResourceBindingCount == 0u &&
			Parsed->RuntimeBlockers == std::vector<std::string>{
				"COMPILED_AUTHORITY_EXTERNAL_AUTHENTICATION_PENDING",
				"TYPED_RUNTIME_PROGRAM_ADAPTER_PENDING" },
			"Format3 Compiled Authority Parses As Immutable Non-Executable Runtime Input");

		std::string ProductMutation = EntryText;
		const std::string ProductField = "\"productAdmission\":false";
		const size_t ProductOffset = ProductMutation.find(ProductField);
		if (ProductOffset != std::string::npos)
			ProductMutation.replace(
				ProductOffset, ProductField.size(), "\"productAdmission\":true");
		DATA_JSON_VALUE MutatedEntry;
		std::shared_ptr<const EFFECT_COMPILED_RUNTIME_DOCUMENT> Preserved = Parsed;
		const bool_t ProductRejected =
			std::string::npos != ProductOffset &&
			CDataJson::Parse(ProductMutation, MutatedEntry, Status) &&
			!CEffectRuntimeAuthorityCodec::Parse_DerivedEntry(
				MutatedEntry, Preserved, Status) && Preserved == Parsed;
		runner.Require(ProductRejected,
			"Format3 Product Promotion Rejects Before Replacing Parsed Authority");

		std::string HashMutation = EntryText;
		const std::string IrHashField = "\"compiledIrSha256\":\"" +
			Parsed->Identity.strCompiledIrSha256 + "\"";
		const size_t HashOffset = HashMutation.find(IrHashField);
		if (HashOffset != std::string::npos)
			HashMutation[HashOffset + IrHashField.size() - 2u] =
				HashMutation[HashOffset + IrHashField.size() - 2u] == '0' ? '1' : '0';
		const bool_t HashRejected = std::string::npos != HashOffset &&
			CDataJson::Parse(HashMutation, MutatedEntry, Status) &&
			!CEffectRuntimeAuthorityCodec::Parse_DerivedEntry(
				MutatedEntry, Preserved, Status) && Preserved == Parsed;
		runner.Require(HashRejected,
			"Format3 Cross-Layer Compiled IR Hash Mutation Rejects Transactionally");

		wchar_t ModuleBuffer[32768]{};
		const DWORD ModuleLength = GetModuleFileNameW(
			nullptr, ModuleBuffer, static_cast<DWORD>(std::size(ModuleBuffer)));
		const std::filesystem::path ModuleDirectory =
			0u == ModuleLength || ModuleLength >= std::size(ModuleBuffer) ?
			std::filesystem::path{} :
			std::filesystem::path(ModuleBuffer).parent_path();
		const std::filesystem::path CatalogPath = ModuleDirectory /
			L"DataFiles" / L"Effect" / L"EffectCatalog.runtime.json";
		std::error_code Error;
		std::filesystem::create_directories(CatalogPath.parent_path(), Error);
		std::vector<char> PriorBytes;
		const bool_t HadPrior = std::filesystem::is_regular_file(CatalogPath);
		if (HadPrior)
		{
			std::ifstream Prior(CatalogPath, std::ios::binary);
			PriorBytes.assign(std::istreambuf_iterator<char>(Prior),
				std::istreambuf_iterator<char>());
		}
		const std::string CatalogText =
			"{\"schema\":\"lostark.effect-runtime-catalog\","
			"\"formatVersion\":3,\"components\":[],\"effects\":[" +
			EntryText + "]}";
		{
			std::ofstream Output(CatalogPath, std::ios::binary | std::ios::trunc);
			Output.write(CatalogText.data(),
				static_cast<std::streamsize>(CatalogText.size()));
		}
		CEffectCatalog::Clear();
		const bool_t CatalogLoaded = CEffectCatalog::Load(Status);
		const uint64_t Revision = CEffectCatalog::Get_RuntimeRevision();
		const std::shared_ptr<const EFFECT_COMPILED_RUNTIME_DOCUMENT>
			CatalogAuthority = CEffectCatalog::Find_RuntimeAuthority(
				"effect.fixture.runtime.authority");
		runner.Require(CatalogLoaded && Revision != 0u &&
			nullptr != CatalogAuthority &&
			CatalogAuthority->Identity.strCompiledIrSha256 ==
				Parsed->Identity.strCompiledIrSha256 &&
			CEffectCatalog::Contains_RuntimeAuthority(
				"effect.fixture.runtime.authority") &&
			!CEffectCatalog::Contains("effect.fixture.runtime.authority") &&
			nullptr == CEffectCatalog::Find("effect.fixture.runtime.authority") &&
			CEffectCatalog::Get_RuntimeAuthorityAssetIds() ==
				std::vector<std::string>{ "effect.fixture.runtime.authority" },
			"Format3 Catalog Commits Compiled Authority Without Raw Drawable Document");

		std::string FloatingVersionCatalog = CatalogText;
		const std::string CatalogVersion = "\"formatVersion\":3";
		const size_t CatalogVersionOffset =
			FloatingVersionCatalog.find(CatalogVersion);
		if (CatalogVersionOffset != std::string::npos)
		{
			FloatingVersionCatalog.replace(CatalogVersionOffset,
				CatalogVersion.size(), "\"formatVersion\":3.0");
		}
		{
			std::ofstream Output(CatalogPath, std::ios::binary | std::ios::trunc);
			Output.write(FloatingVersionCatalog.data(),
				static_cast<std::streamsize>(FloatingVersionCatalog.size()));
		}
		const bool_t FloatingVersionRejected =
			!CEffectCatalog::Load(Status);
		runner.Require(std::string::npos != CatalogVersionOffset &&
			FloatingVersionRejected &&
			CEffectCatalog::Get_RuntimeRevision() == Revision &&
			CEffectCatalog::Find_RuntimeAuthority(
				"effect.fixture.runtime.authority") == CatalogAuthority,
			"Format3 Floating Point Version Rejects And Preserves Prior Catalog");

		const std::string InvalidCatalog =
			"{\"schema\":\"lostark.effect-runtime-catalog\","
			"\"formatVersion\":3,\"components\":[],\"effects\":[" +
			ProductMutation + "]}";
		{
			std::ofstream Output(CatalogPath, std::ios::binary | std::ios::trunc);
			Output.write(InvalidCatalog.data(),
				static_cast<std::streamsize>(InvalidCatalog.size()));
		}
		const bool_t ReloadRejected = !CEffectCatalog::Load(Status);
		runner.Require(ReloadRejected &&
			CEffectCatalog::Get_RuntimeRevision() == Revision &&
			CEffectCatalog::Find_RuntimeAuthority(
				"effect.fixture.runtime.authority") == CatalogAuthority,
			"Format3 Catalog Failed Reload Preserves Prior Revision And Pointer");

		CEffectCatalog::Clear();
		if (HadPrior)
		{
			std::ofstream Output(CatalogPath, std::ios::binary | std::ios::trunc);
			Output.write(PriorBytes.data(),
				static_cast<std::streamsize>(PriorBytes.size()));
		}
		else
		{
			std::filesystem::remove(CatalogPath, Error);
		}
	}

	void Test_Artist31470SourceContractRoundTrip(
		TEST_RUNNER& runner,
		const std::filesystem::path& path)
	{
		using namespace Client;
		EFFECT_DOCUMENT_DESC document;
		std::string status;
		const bool_t loaded = CEffectDocumentCodec::Load(path, document, status) &&
			CEffectDocumentCodec::Validate_SourceContract(document, status);
		const std::string serialized = loaded ?
			CEffectDocumentCodec::Serialize(document) : std::string{};
		EFFECT_DOCUMENT_DESC roundTrip;
		const bool_t parsed = loaded &&
			CEffectDocumentCodec::Parse(serialized, roundTrip, status) &&
			CEffectDocumentCodec::Validate_SourceContract(roundTrip, status) &&
			CEffectDocumentCodec::Serialize(roundTrip) == serialized;
		const std::string inputIdentity =
			CEffectCascadeCompiler::Build_CanonicalDocumentIdentity(roundTrip);
		EFFECT_CASCADE_EXPECTED_SOURCE_IDENTITY expectedIdentity;
		expectedIdentity.eKind =
			EFFECT_CASCADE_EXTERNAL_SOURCE_IDENTITY_KIND::CANDIDATE_SHA256;
		expectedIdentity.strExpectedCanonicalDocumentIdentity = inputIdentity;
		expectedIdentity.strExpectedExternalIdentityToken =
			"sha256:aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
		const auto ExpectedIdentityFor = [&expectedIdentity](
			const EFFECT_DOCUMENT_DESC& candidate)
		{
			EFFECT_CASCADE_EXPECTED_SOURCE_IDENTITY result = expectedIdentity;
			result.strExpectedCanonicalDocumentIdentity =
				CEffectCascadeCompiler::Build_CanonicalDocumentIdentity(candidate);
			return result;
		};
		const EFFECT_CASCADE_INSPECTION_COMPILER_PROBE probeBefore =
			CEffectCascadeCompiler::Get_Probe();
		std::shared_ptr<const EFFECT_CASCADE_INSPECTION_IR> inspectionA;
		std::shared_ptr<const EFFECT_CASCADE_INSPECTION_IR> inspectionB;
		const bool_t inspectedTwice = parsed &&
			CEffectCascadeCompiler::Compile_SourceInspection(
				roundTrip, expectedIdentity, inspectionA, status) &&
			CEffectCascadeCompiler::Compile_SourceInspection(
				roundTrip, expectedIdentity, inspectionB, status);
		const EFFECT_CASCADE_INSPECTION_COMPILER_PROBE probeAfter =
			CEffectCascadeCompiler::Get_Probe();
		if (!inspectedTwice)
			std::cerr << "Cascade inspection compile error: " << status << '\n';
		const auto RendererCount = [&inspectionA](const EFFECT_RENDERER_TYPE type)
		{
			return nullptr == inspectionA ? 0u :
				inspectionA->Consumption.RendererCounts[
					static_cast<size_t>(type)];
		};
		bool_t stableInspectionIdentity = inspectedTwice;
		bool_t handlerReceiptsComplete = inspectedTwice;
		bool_t propertyBlockersBound = inspectedTwice;
		bool_t rawDistributionsIsolated = inspectedTwice;
		bool_t fidelityBlocked = inspectedTwice;
		size_t geometryEvidenceCount = 0u;
		size_t baselineExactClassCount = 0u;
		size_t baselineQuarantinedClassCount = 0u;
		for (const EFFECT_CASCADE_INSPECTION_SYSTEM& system :
			inspectedTwice ? inspectionA->Systems :
			std::vector<EFFECT_CASCADE_INSPECTION_SYSTEM>{})
		{
			stableInspectionIdentity = stableInspectionIdentity &&
				!system.strSourceSystemId.empty() && system.iStableSemantic != 0u;
			for (const EFFECT_CASCADE_INSPECTION_EMITTER& emitter : system.Emitters)
			{
				stableInspectionIdentity = stableInspectionIdentity &&
					!emitter.Identity.strSourceSystemId.empty() &&
					!emitter.Identity.strSourceOccurrenceId.empty() &&
					!emitter.Identity.strCanonicalId.empty() &&
					emitter.Identity.iStableReference != 0u;
				fidelityBlocked = fidelityBlocked &&
					emitter.SelectedLOD.bIdentityPreserved &&
					!emitter.SelectedLOD.bExecutionFidelityProven &&
					!emitter.SelectedLOD.strCanonicalLineageId.empty() &&
					emitter.SelectedLOD.iStableReference != 0u &&
					!emitter.SelectedLOD.Blockers.empty();
				if (emitter.Geometry.has_value())
				{
					++geometryEvidenceCount;
					fidelityBlocked = fidelityBlocked &&
						!emitter.Geometry->bPayloadIntegrityValid &&
						!emitter.Geometry->bRuntimeConsumerReady &&
						!emitter.Geometry->ChannelConsumptionBlockers.empty();
				}
				for (const EFFECT_CASCADE_INSPECTION_OPCODE& opcode :
					emitter.OrderedOpcodes)
				{
					const bool_t bQuarantinedExactClass =
						opcode.eOpcode == EFFECT_CASCADE_OPCODE::
							UNKNOWN_EXACT_CLASS_QUARANTINE;
					const bool_t bExactKnownClass =
						opcode.HandlerReceipt.eClassLineageStatus ==
							EFFECT_CASCADE_CLASS_LINEAGE_STATUS::EXACT_SOURCE_CLASS;
					if (bExactKnownClass)
						++baselineExactClassCount;
					if (bQuarantinedExactClass)
						++baselineQuarantinedClassCount;
					handlerReceiptsComplete = handlerReceiptsComplete &&
						opcode.eOpcode != EFFECT_CASCADE_OPCODE::END &&
						opcode.Reference.eRole != EFFECT_CASCADE_MODULE_ROLE::END &&
						!opcode.Reference.strReceiptRole.empty() &&
						!opcode.Reference.strCanonicalId.empty() &&
						opcode.Reference.iStableReference != 0u &&
						opcode.HandlerReceipt.eResult ==
							EFFECT_CASCADE_HANDLER_RESULT::
								STRUCTURE_CONSUMED_EXECUTION_BLOCKED &&
						opcode.HandlerReceipt.eModuleCoverageStatus !=
							EFFECT_SOURCE_COVERAGE_STATUS::END &&
						opcode.HandlerReceipt.eAggregateBlockerRequirement !=
							EFFECT_CASCADE_BLOCKER_REQUIREMENT::END &&
						!opcode.HandlerReceipt.strReceiptNormalizedClass.empty() &&
						!opcode.HandlerReceipt.strExactSourceClass.empty() &&
						opcode.HandlerReceipt.strAliasId.empty() &&
						opcode.HandlerReceipt.strExactSourceClass ==
							opcode.HandlerReceipt.strReceiptNormalizedClass &&
						(bExactKnownClass ||
						 (bQuarantinedExactClass &&
						  opcode.HandlerReceipt.eClassLineageStatus ==
							EFFECT_CASCADE_CLASS_LINEAGE_STATUS::
								EXACT_CLASS_HANDLER_QUARANTINED)) &&
						!opcode.HandlerReceipt.strOpcodeSchemaId.empty() &&
						opcode.HandlerReceipt.bExactClassLineagePreserved &&
						!opcode.HandlerReceipt.bPayloadAccessAllowed &&
						opcode.HandlerReceipt.PropertyConsumption.size() ==
							opcode.Properties.size() &&
						opcode.HandlerReceipt.ConsumedPropertyReferenceIds.size() ==
							opcode.Properties.size() &&
						(bQuarantinedExactClass ||
						 !opcode.HandlerReceipt.RequiredPropertyReferenceIds.empty());
					for (const EFFECT_CASCADE_PROPERTY_EVIDENCE& property :
						opcode.Properties)
					{
						const size_t propertyIndex = static_cast<size_t>(
							&property - opcode.Properties.data());
						const EFFECT_CASCADE_PROPERTY_HANDLER_RECEIPT& receipt =
							opcode.HandlerReceipt.PropertyConsumption[propertyIndex];
						stableInspectionIdentity = stableInspectionIdentity &&
							!property.Property.strCanonicalPath.empty() &&
							!property.Property.strCanonicalReferenceId.empty() &&
							property.Property.iStableSemantic != 0u &&
							property.Property.iStableReference != 0u &&
							property.eBlockerRequirement !=
								EFFECT_CASCADE_BLOCKER_REQUIREMENT::END &&
							receipt.Property.strCanonicalReferenceId ==
								property.Property.strCanonicalReferenceId &&
							receipt.eStorage == property.eStorage &&
							receipt.eResult ==
								(bQuarantinedExactClass ?
								 EFFECT_CASCADE_PROPERTY_HANDLER_RESULT::
									QUARANTINED_FIELD_PRESERVED_EXECUTION_BLOCKED :
								 EFFECT_CASCADE_PROPERTY_HANDLER_RESULT::
									SCHEMA_FIELD_CONSUMED_EXECUTION_BLOCKED) &&
							!receipt.strHandlerFieldId.empty();
						propertyBlockersBound = propertyBlockersBound &&
							((property.eBlockerRequirement ==
								EFFECT_CASCADE_BLOCKER_REQUIREMENT::
									PROPERTY_BLOCKERS_REQUIRED &&
							  !property.Blockers.empty()) ||
							 (property.eBlockerRequirement ==
								EFFECT_CASCADE_BLOCKER_REQUIREMENT::
									BLOCKERS_PROHIBITED &&
							  property.Blockers.empty()));
					}
				}
				for (const EFFECT_CASCADE_DISTRIBUTION_EVIDENCE& distribution :
					emitter.Distributions)
				{
					rawDistributionsIsolated = rawDistributionsIsolated &&
						!distribution.bRawPayloadRead &&
						!distribution.bExecutionAllowed &&
						!distribution.Blockers.empty();
				}
			}
		}
		runner.Require(inspectedTwice && nullptr != inspectionA &&
			inspectionA->iCompilerRevision ==
				EFFECT_CASCADE_INSPECTION_COMPILER_REVISION &&
			!inspectionA->bExecutable && !inspectionA->bProductAdmission &&
			inspectionA->eExternalIdentityKind ==
				EFFECT_CASCADE_EXTERNAL_SOURCE_IDENTITY_KIND::CANDIDATE_SHA256 &&
			inspectionA->eExternalIdentityStatus ==
				EFFECT_CASCADE_EXTERNAL_IDENTITY_STATUS::
					SELF_CONSISTENT_UNAUTHENTICATED &&
			inspectionA->strExpectedExternalIdentityToken ==
				expectedIdentity.strExpectedExternalIdentityToken &&
			!inspectionA->bExternalIdentityAuthenticated &&
			std::find(inspectionA->Blockers.begin(), inspectionA->Blockers.end(),
				"SELF_CONSISTENT_UNAUTHENTICATED") !=
					inspectionA->Blockers.end() &&
			std::find(inspectionA->Blockers.begin(), inspectionA->Blockers.end(),
				"SOURCE_EXTERNAL_IDENTITY_ADAPTER_PENDING") !=
					inspectionA->Blockers.end() &&
			inspectionA->Consumption.iSystemCount == 7u &&
			inspectionA->Consumption.iDeclaredSourceElementCount == 35u &&
			inspectionA->Consumption.iDisabledSourceRecipeCount == 0u &&
			inspectionA->Consumption.iEmitterCount == 35u &&
			inspectionA->Consumption.iOrderedOpcodeCount == 399u &&
			inspectionA->Consumption.iDistributionEvidenceCount == 629u &&
			inspectionA->Consumption.iUnknownClassCount == 0u &&
			inspectionA->Consumption.iQuarantinedExactClassCount == 26u &&
			baselineExactClassCount == 373u &&
			baselineQuarantinedClassCount == 26u &&
			inspectionA->Consumption.iUnconsumedRequiredPropertyCount == 0u &&
			inspectionA->Consumption.iHandlerPropertyReceiptCount ==
				inspectionA->Consumption.iConsumedPropertyCount &&
			inspectionA->Consumption.iRawPayloadReadCount == 0u &&
			inspectionA->Consumption.iExecutableOpcodeCount == 0u &&
			inspectionA->Consumption.iConsumedPropertyCount >=
				inspectionA->Consumption.iRequiredPropertyCount &&
			RendererCount(EFFECT_RENDERER_TYPE::MESH_PARTICLE) == 13u &&
			RendererCount(EFFECT_RENDERER_TYPE::SPRITE_PARTICLE) == 16u &&
			RendererCount(EFFECT_RENDERER_TYPE::DECAL_PARTICLE) == 3u &&
			RendererCount(EFFECT_RENDERER_TYPE::CASCADE_RIBBON) == 1u &&
			RendererCount(EFFECT_RENDERER_TYPE::LIGHT_PARTICLE) == 1u &&
			RendererCount(EFFECT_RENDERER_TYPE::SCREEN_POST) == 1u &&
			stableInspectionIdentity && handlerReceiptsComplete &&
			propertyBlockersBound &&
			rawDistributionsIsolated && fidelityBlocked &&
			geometryEvidenceCount == 13u,
			"Artist F Inspection IR Preserves Fixture 7 Systems 35 Declared Emitters 399 Opcodes 629 Isolated Distributions And Renderer Denominator");
		runner.Require(inspectedTwice &&
			inspectionA->strCanonicalDocumentIdentity == inputIdentity &&
			inspectionA->strInspectionHash == inspectionB->strInspectionHash &&
			CEffectCascadeCompiler::Matches_InputIdentity(
				*inspectionA, expectedIdentity) &&
			probeAfter.iCompileAttemptCount ==
				probeBefore.iCompileAttemptCount + 2u &&
			probeAfter.iCompileSuccessCount ==
				probeBefore.iCompileSuccessCount + 2u,
			"Cascade Inspection Hash Is Deterministic And Input Identity Bound");

		const auto CompileMutation = [&](const EFFECT_DOCUMENT_DESC& candidate)
		{
			std::shared_ptr<const EFFECT_CASCADE_INSPECTION_IR> rejected;
			return CEffectCascadeCompiler::Compile_SourceInspection(
				candidate, ExpectedIdentityFor(candidate), rejected, status);
		};
		const auto FixedExpectedIdentityRejects = [&](
			const EFFECT_DOCUMENT_DESC& candidate)
		{
			std::shared_ptr<const EFFECT_CASCADE_INSPECTION_IR> fixedOutput =
				inspectionA;
			const auto* pCommitted = fixedOutput.get();
			const bool_t fixedRejected =
				!CEffectCascadeCompiler::Compile_SourceInspection(
					candidate, expectedIdentity, fixedOutput, status) &&
				fixedOutput.get() == pCommitted &&
				fixedOutput->strInspectionHash == inspectionA->strInspectionHash;
			const EFFECT_CASCADE_EXPECTED_SOURCE_IDENTITY resealedExpected =
				ExpectedIdentityFor(candidate);
			std::shared_ptr<const EFFECT_CASCADE_INSPECTION_IR>
				selfConsistentInspection;
			const bool_t selfConsistentOnly =
				CEffectCascadeCompiler::Compile_SourceInspection(
					candidate, resealedExpected, selfConsistentInspection, status) &&
				nullptr != selfConsistentInspection &&
				selfConsistentInspection->eExternalIdentityStatus ==
					EFFECT_CASCADE_EXTERNAL_IDENTITY_STATUS::
						SELF_CONSISTENT_UNAUTHENTICATED &&
				!selfConsistentInspection->bExternalIdentityAuthenticated &&
				!selfConsistentInspection->bExecutable &&
				!selfConsistentInspection->bProductAdmission;
			return fixedRejected && selfConsistentOnly;
		};

		EFFECT_DOCUMENT_DESC disabledRecipe = roundTrip;
		const std::string disabledElementId =
			disabledRecipe.Elements.front().strElementId;
		disabledRecipe.Elements.front().SourceRecipe.bEnabled = false;
		std::shared_ptr<const EFFECT_CASCADE_INSPECTION_IR> disabledInspection;
		const bool_t disabledInspected =
			CEffectCascadeCompiler::Compile_SourceInspection(
				disabledRecipe, ExpectedIdentityFor(disabledRecipe),
				disabledInspection, status);
		const EFFECT_CASCADE_INSPECTION_EMITTER* pDisabledEmitter = nullptr;
		if (disabledInspected && nullptr != disabledInspection)
		{
			for (const EFFECT_CASCADE_INSPECTION_SYSTEM& system :
				disabledInspection->Systems)
			{
				const auto found = std::find_if(system.Emitters.begin(),
					system.Emitters.end(), [&disabledElementId](const auto& emitter)
					{
						return emitter.Identity.strElementId == disabledElementId;
					});
				if (found != system.Emitters.end())
				{
					pDisabledEmitter = &*found;
					break;
				}
			}
		}
		runner.Require(disabledInspected && nullptr != disabledInspection &&
			disabledInspection->Consumption.iDeclaredSourceElementCount == 35u &&
			disabledInspection->Consumption.iEmitterCount == 35u &&
			disabledInspection->Consumption.iDisabledSourceRecipeCount == 1u &&
			disabledInspection->Consumption.iOrderedOpcodeCount == 399u &&
			nullptr != pDisabledEmitter && !pDisabledEmitter->bSourceRecipeEnabled &&
			std::find(pDisabledEmitter->Blockers.begin(),
				pDisabledEmitter->Blockers.end(),
				"SOURCE_RECIPE_DISABLED_QUARANTINED") !=
					pDisabledEmitter->Blockers.end() &&
			!disabledInspection->bExecutable &&
			!disabledInspection->bProductAdmission,
			"Disabled Source Recipe Remains One Of 35 Declared Emitters As Explicit Quarantine");

		EFFECT_DOCUMENT_DESC fixedBlockerAttack = roundTrip;
		bool_t changedFixedBlocker = !fixedBlockerAttack.Elements.front().
			SourceRecipe.CompiledExecutionAdmission.Blockers.empty();
		if (changedFixedBlocker)
		{
			fixedBlockerAttack.Elements.front().SourceRecipe.
				CompiledExecutionAdmission.Blockers.front() =
					"FORGED_EXTERNAL_BLOCKER";
		}
		runner.Require(changedFixedBlocker &&
			FixedExpectedIdentityRejects(fixedBlockerAttack),
			"Fixed External Source Identity Rejects Coordinated Blocker Reseal");

		EFFECT_DOCUMENT_DESC fixedModuleIdAttack = roundTrip;
		auto& attackedReference = fixedModuleIdAttack.Elements.front().SourceRecipe.
			CompilerEvidence.ModuleReferenceOrder.front();
		auto& attackedCoverage = fixedModuleIdAttack.Elements.front().SourceRecipe.
			ModuleCoverage.front();
		attackedReference.strSourceObjectId = "FX_FIXED_ATTACK:export:999";
		attackedCoverage.strModuleStableId =
			attackedReference.strSourceObjectId + "@ref:" +
			std::to_string(attackedReference.iSourceReferenceIndex);
		runner.Require(FixedExpectedIdentityRejects(fixedModuleIdAttack),
			"Fixed External Source Identity Rejects Coordinated Module ID Reseal");

		EFFECT_DOCUMENT_DESC fixedRecordAttack = roundTrip;
		fixedRecordAttack.Elements.front().SourceRecipe.CompilerEvidence.
			ModuleReferenceOrder.front().strSourceRecordSha256 =
				std::string(64u, 'b');
		runner.Require(FixedExpectedIdentityRejects(fixedRecordAttack),
			"Fixed External Source Identity Rejects Coordinated Record SHA Reseal");

		EFFECT_DOCUMENT_DESC fixedProvenanceAttack = roundTrip;
		EFFECT_SOURCE_PROPERTY_COVERAGE_DESC* pMetadataProvenance = nullptr;
		for (EFFECT_ELEMENT_DESC& element : fixedProvenanceAttack.Elements)
		{
			for (EFFECT_SOURCE_MODULE_COVERAGE_DESC& coverage :
				element.SourceRecipe.ModuleCoverage)
			{
				const auto found = std::find_if(coverage.Properties.begin(),
					coverage.Properties.end(), [](const auto& property)
					{
						return property.eStatus ==
							EFFECT_SOURCE_COVERAGE_STATUS::METADATA_ONLY &&
							property.Blockers.empty() &&
							(property.strProvenance ==
								"DETERMINISTIC_REFERENCE_METADATA_JOIN" ||
							 property.strProvenance ==
								"OPAQUE_HEX_METADATA_ONLY");
					});
				if (found != coverage.Properties.end())
				{
					pMetadataProvenance = &*found;
					break;
				}
			}
			if (nullptr != pMetadataProvenance)
				break;
		}
		if (nullptr != pMetadataProvenance)
		{
			pMetadataProvenance->strProvenance =
				pMetadataProvenance->strProvenance ==
					"DETERMINISTIC_REFERENCE_METADATA_JOIN" ?
					"OPAQUE_HEX_METADATA_ONLY" :
					"DETERMINISTIC_REFERENCE_METADATA_JOIN";
		}
		runner.Require(nullptr != pMetadataProvenance &&
			FixedExpectedIdentityRejects(fixedProvenanceAttack),
			"Fixed External Source Identity Rejects Coordinated Provenance Reseal");

		EFFECT_DOCUMENT_DESC rawPayloadMutation = roundTrip;
		rawPayloadMutation.Elements.front().SourceRecipe.Modules.front().strClassName =
			"raw-payload-mutation-must-not-drive-inspection";
		const EFFECT_CASCADE_EXPECTED_SOURCE_IDENTITY rawMutationExpected =
			ExpectedIdentityFor(rawPayloadMutation);
		std::shared_ptr<const EFFECT_CASCADE_INSPECTION_IR> rawMutationInspection;
		std::shared_ptr<const EFFECT_CASCADE_INSPECTION_IR>
			reusedIdentityInspection = inspectionA;
		const auto* pCommittedBeforeFailure = reusedIdentityInspection.get();
		const std::string committedHashBeforeFailure =
			reusedIdentityInspection->strInspectionHash;
		const bool_t reusedIdentityRejected =
			!CEffectCascadeCompiler::Compile_SourceInspection(
				rawPayloadMutation, expectedIdentity,
				reusedIdentityInspection, status);
		const bool_t rawPayloadNotMaterialized =
			CEffectCascadeCompiler::Compile_SourceInspection(
				rawPayloadMutation, rawMutationExpected,
				rawMutationInspection, status);
		runner.Require(reusedIdentityRejected && rawPayloadNotMaterialized &&
			nullptr != rawMutationInspection &&
			reusedIdentityInspection.get() == pCommittedBeforeFailure &&
			reusedIdentityInspection->strInspectionHash ==
				committedHashBeforeFailure &&
			!CEffectCascadeCompiler::Matches_InputIdentity(
				*inspectionA, rawMutationExpected) &&
			rawMutationInspection->strInspectionHash !=
				inspectionA->strInspectionHash &&
			rawMutationInspection->Consumption.iOrderedOpcodeCount ==
				inspectionA->Consumption.iOrderedOpcodeCount,
			"Raw B Reusing A External Identity Is Rejected And Preserves Committed Inspection While Values Stay Unmaterialized");

		EFFECT_CASCADE_EXPECTED_SOURCE_IDENTITY malformedExpected =
			expectedIdentity;
		malformedExpected.strExpectedExternalIdentityToken = "sha256:broken";
		std::shared_ptr<const EFFECT_CASCADE_INSPECTION_IR>
			malformedIdentityOutput = inspectionA;
		const auto* pMalformedCommitted = malformedIdentityOutput.get();
		runner.Require(!CEffectCascadeCompiler::Compile_SourceInspection(
			roundTrip, malformedExpected, malformedIdentityOutput, status) &&
			malformedIdentityOutput.get() == pMalformedCommitted &&
			malformedIdentityOutput->strInspectionHash ==
				inspectionA->strInspectionHash,
			"Malformed External Source Identity Rejects Before Compile And Preserves Caller Commit");

		EFFECT_CASCADE_INSPECTION_IR fabricatedInspection;
		fabricatedInspection.strCanonicalDocumentIdentity = inputIdentity;
		fabricatedInspection.strInspectionHash = "fnv1a64:0123456789abcdef";
		runner.Require(!CEffectCascadeCompiler::Matches_InputIdentity(
			fabricatedInspection, expectedIdentity),
			"Fabricated Default Inspection Cannot Match A Valid Looking Identity And Hash");

		EFFECT_CASCADE_INSPECTION_IR tamperedInspection = *inspectionA;
		tamperedInspection.Systems.front().Emitters.front().SelectedLOD.strEmitterNodeId =
			"FX_PC_SDM_07:export:99999";
		runner.Require(!CEffectCascadeCompiler::Matches_InputIdentity(
			tamperedInspection, expectedIdentity),
			"Inspection Hash Binds Source Emitter Node And LOD Lineage");

		EFFECT_CASCADE_INSPECTION_IR forgedHandlerReceipt = *inspectionA;
		forgedHandlerReceipt.Systems.front().Emitters.front().OrderedOpcodes.
			front().HandlerReceipt.PropertyConsumption.front().strHandlerFieldId =
				"ue3.cascade.forged-handler-field.v1";
		runner.Require(!CEffectCascadeCompiler::Matches_InputIdentity(
			forgedHandlerReceipt, expectedIdentity),
			"Inspection Rejects Forged Opcode Handler Field Consumption Receipt");

		EFFECT_CASCADE_INSPECTION_IR strippedPropertyBlocker = *inspectionA;
		EFFECT_CASCADE_PROPERTY_EVIDENCE* pBlockedInspectionProperty = nullptr;
		for (EFFECT_CASCADE_INSPECTION_SYSTEM& system :
			strippedPropertyBlocker.Systems)
		{
			for (EFFECT_CASCADE_INSPECTION_EMITTER& emitter : system.Emitters)
			{
				for (EFFECT_CASCADE_INSPECTION_OPCODE& opcode :
					emitter.OrderedOpcodes)
				{
					const auto blocked = std::find_if(opcode.Properties.begin(),
						opcode.Properties.end(), [](const auto& property)
						{
							return !property.Blockers.empty();
						});
					if (blocked != opcode.Properties.end())
					{
						pBlockedInspectionProperty = &*blocked;
						break;
					}
				}
				if (nullptr != pBlockedInspectionProperty)
					break;
			}
			if (nullptr != pBlockedInspectionProperty)
				break;
		}
		if (nullptr != pBlockedInspectionProperty)
			pBlockedInspectionProperty->Blockers.clear();
		runner.Require(nullptr != pBlockedInspectionProperty &&
			!CEffectCascadeCompiler::Matches_InputIdentity(
				strippedPropertyBlocker, expectedIdentity),
			"Inspection Rejects Stripped Property Fidelity Blocker");

		EFFECT_DOCUMENT_DESC simultaneousUnknown = roundTrip;
		EFFECT_SOURCE_LITERAL_DESC unknownRaw;
		unknownRaw.strPropertyPath = "unknownsimultaneous";
		unknownRaw.eKind = EFFECT_SOURCE_LITERAL_KIND::NUMBER;
		simultaneousUnknown.Elements.front().SourceRecipe.Modules.front().Literals.push_back(
			unknownRaw);
		EFFECT_SOURCE_PROPERTY_COVERAGE_DESC unknownCoverage;
		unknownCoverage.strPropertyPath = "unknownsimultaneous";
		unknownCoverage.strStorage = "literal";
		unknownCoverage.strProvenance = "SOURCE_TAGGED_PRIMITIVE";
		unknownCoverage.eStatus = EFFECT_SOURCE_COVERAGE_STATUS::SOURCE_DECODED;
		simultaneousUnknown.Elements.front().SourceRecipe.ModuleCoverage.front().Properties.push_back(
			unknownCoverage);
		runner.Require(!CompileMutation(simultaneousUnknown),
			"Cascade Inspection Rejects Simultaneous Source And Coverage Unknown Property");

		EFFECT_DOCUMENT_DESC efClassMutation = roundTrip;
		efClassMutation.Elements.front().SourceRecipe.ModuleCoverage.front().strNormalizedClass =
			"efparticlemodulerequired";
		runner.Require(!CompileMutation(efClassMutation),
			"Cascade Inspection Rejects EF Class Mutation Without Blanket Normalization");

		EFFECT_DOCUMENT_DESC exactClassLineage = roundTrip;
		EFFECT_SOURCE_MODULE_COVERAGE_DESC& exactCoverage =
			exactClassLineage.Elements.front().SourceRecipe.ModuleCoverage.front();
		exactCoverage.strExactSourceClass = exactCoverage.strNormalizedClass;
		std::shared_ptr<const EFFECT_CASCADE_INSPECTION_IR> exactInspection;
		const bool_t exactClassAccepted =
			CEffectCascadeCompiler::Compile_SourceInspection(
				exactClassLineage, ExpectedIdentityFor(exactClassLineage),
				exactInspection, status);
		const EFFECT_CASCADE_HANDLER_RECEIPT* pExactReceipt =
			exactClassAccepted && nullptr != exactInspection ?
				&exactInspection->Systems.front().Emitters.front().
					OrderedOpcodes.front().HandlerReceipt : nullptr;
		runner.Require(nullptr != pExactReceipt &&
			pExactReceipt->eClassLineageStatus ==
				EFFECT_CASCADE_CLASS_LINEAGE_STATUS::EXACT_SOURCE_CLASS &&
			pExactReceipt->bExactClassLineagePreserved &&
			pExactReceipt->strExactSourceClass ==
				pExactReceipt->strReceiptNormalizedClass &&
			pExactReceipt->strAliasId.empty(),
			"Cascade Inspection Accepts Only Self Identical Exact Source Class Without Alias");

		EFFECT_DOCUMENT_DESC receiptBoundClasses = roundTrip;
		for (EFFECT_ELEMENT_DESC& element : receiptBoundClasses.Elements)
		{
			std::map<std::string, std::string> sourceClassByStableId;
			for (const EFFECT_SOURCE_MODULE_DESC& module :
				element.SourceRecipe.Modules)
			{
				sourceClassByStableId.emplace(
					module.strStableId, module.strClassName);
			}
			for (EFFECT_SOURCE_MODULE_COVERAGE_DESC& coverage :
				element.SourceRecipe.ModuleCoverage)
			{
				const auto sourceClass = sourceClassByStableId.find(
					coverage.strModuleStableId);
				if (sourceClass != sourceClassByStableId.end())
				{
					coverage.strNormalizedClass = sourceClass->second;
					coverage.strExactSourceClass = sourceClass->second;
				}
				coverage.strAliasId.clear();
			}
		}
		std::shared_ptr<const EFFECT_CASCADE_INSPECTION_IR>
			receiptBoundInspection;
		const bool_t receiptBoundInspected =
			CEffectCascadeCompiler::Compile_SourceInspection(
				receiptBoundClasses, ExpectedIdentityFor(receiptBoundClasses),
				receiptBoundInspection, status);
		size_t exactClassCount = 0u;
		size_t exactClassQuarantinedCount = 0u;
		size_t inspectedClassCount = 0u;
		bool_t quarantinedClassesPreserved = true;
		if (receiptBoundInspected && nullptr != receiptBoundInspection)
		{
			for (const EFFECT_CASCADE_INSPECTION_SYSTEM& system :
				receiptBoundInspection->Systems)
			{
				for (const EFFECT_CASCADE_INSPECTION_EMITTER& emitter :
					system.Emitters)
				{
					for (const EFFECT_CASCADE_INSPECTION_OPCODE& opcode :
						emitter.OrderedOpcodes)
					{
						++inspectedClassCount;
						if (opcode.HandlerReceipt.eClassLineageStatus ==
							EFFECT_CASCADE_CLASS_LINEAGE_STATUS::EXACT_SOURCE_CLASS)
						{
							++exactClassCount;
						}
						else if (opcode.HandlerReceipt.eClassLineageStatus ==
							EFFECT_CASCADE_CLASS_LINEAGE_STATUS::
								EXACT_CLASS_HANDLER_QUARANTINED)
						{
							++exactClassQuarantinedCount;
							quarantinedClassesPreserved =
								quarantinedClassesPreserved &&
								opcode.HandlerReceipt.bExactClassLineagePreserved &&
								opcode.HandlerReceipt.strAliasId.empty() &&
								opcode.HandlerReceipt.strExactSourceClass ==
									opcode.HandlerReceipt.strReceiptNormalizedClass &&
								opcode.eOpcode == EFFECT_CASCADE_OPCODE::
									UNKNOWN_EXACT_CLASS_QUARANTINE &&
								(opcode.HandlerReceipt.strExactSourceClass.
									starts_with("ef") ||
								 opcode.HandlerReceipt.strExactSourceClass.find(
									"_seeded") != std::string::npos) &&
								std::find(opcode.HandlerReceipt.Blockers.begin(),
									opcode.HandlerReceipt.Blockers.end(),
									"EXACT_SOURCE_CLASS_HANDLER_UNAVAILABLE") !=
										opcode.HandlerReceipt.Blockers.end() &&
								std::find(opcode.HandlerReceipt.Blockers.begin(),
									opcode.HandlerReceipt.Blockers.end(),
									"UNKNOWN_EXACT_CLASS_OPCODE_QUARANTINED") !=
										opcode.HandlerReceipt.Blockers.end() &&
								std::all_of(
									opcode.HandlerReceipt.PropertyConsumption.begin(),
									opcode.HandlerReceipt.PropertyConsumption.end(),
									[](const auto& receipt)
									{
										return receipt.eResult ==
											EFFECT_CASCADE_PROPERTY_HANDLER_RESULT::
												QUARANTINED_FIELD_PRESERVED_EXECUTION_BLOCKED &&
											!receipt.bRequired;
									});
						}
					}
				}
			}
		}
		runner.Require(receiptBoundInspected && nullptr != receiptBoundInspection &&
			inspectedClassCount == 399u && exactClassCount == 373u &&
			exactClassQuarantinedCount == 26u &&
			quarantinedClassesPreserved &&
			receiptBoundInspection->Consumption.iOrderedOpcodeCount == 399u &&
			receiptBoundInspection->Consumption.iQuarantinedExactClassCount ==
				26u &&
			receiptBoundInspection->Consumption.iUnknownClassCount == 0u &&
			!receiptBoundInspection->bExecutable &&
			!receiptBoundInspection->bProductAdmission,
			"Artist F Gate1 Normalized Exact Classes Preserve 373 Typed And 26 Schema Independent Custom Quarantined References");

		EFFECT_DOCUMENT_DESC forgedAlias = roundTrip;
		EFFECT_SOURCE_MODULE_COVERAGE_DESC& aliasCoverage =
			forgedAlias.Elements.front().SourceRecipe.ModuleCoverage.front();
		aliasCoverage.strExactSourceClass =
			"ef" + aliasCoverage.strNormalizedClass;
		aliasCoverage.strAliasId = "ue3.cascade.alias.forged.v1";
		std::shared_ptr<const EFFECT_CASCADE_INSPECTION_IR> aliasInspection;
		const bool_t aliasInspected = CEffectCascadeCompiler::Compile_SourceInspection(
			forgedAlias, ExpectedIdentityFor(forgedAlias),
			aliasInspection, status);
		const EFFECT_CASCADE_HANDLER_RECEIPT* pAliasReceipt =
			aliasInspected && nullptr != aliasInspection ?
				&aliasInspection->Systems.front().Emitters.front().
					OrderedOpcodes.front().HandlerReceipt : nullptr;
		runner.Require(nullptr != pAliasReceipt &&
			pAliasReceipt->eClassLineageStatus ==
				EFFECT_CASCADE_CLASS_LINEAGE_STATUS::
					EXPLICIT_ALIAS_EXECUTION_UNAPPROVED &&
			!aliasInspection->bExecutable &&
			std::find(pAliasReceipt->Blockers.begin(),
				pAliasReceipt->Blockers.end(),
				"SOURCE_CLASS_ALIAS_UNAPPROVED") !=
					pAliasReceipt->Blockers.end(),
			"Cascade Inspection Preserves Free Form Alias As Non Executable Evidence");

		EFFECT_DOCUMENT_DESC exactClassMismatch = roundTrip;
		EFFECT_SOURCE_MODULE_COVERAGE_DESC& mismatchCoverage =
			exactClassMismatch.Elements.front().SourceRecipe.ModuleCoverage.front();
		mismatchCoverage.strNormalizedClass =
			"ef" + mismatchCoverage.strNormalizedClass;
		mismatchCoverage.strExactSourceClass =
			mismatchCoverage.strNormalizedClass;
		std::shared_ptr<const EFFECT_CASCADE_INSPECTION_IR> mismatchInspection;
		const bool_t mismatchInspected =
			CEffectCascadeCompiler::Compile_SourceInspection(
				exactClassMismatch, ExpectedIdentityFor(exactClassMismatch),
				mismatchInspection, status);
		const EFFECT_CASCADE_HANDLER_RECEIPT* pMismatchReceipt =
			mismatchInspected && nullptr != mismatchInspection ?
				&mismatchInspection->Systems.front().Emitters.front().
					OrderedOpcodes.front().HandlerReceipt : nullptr;
		const EFFECT_CASCADE_INSPECTION_OPCODE* pMismatchOpcode =
			mismatchInspected && nullptr != mismatchInspection ?
				&mismatchInspection->Systems.front().Emitters.front().
					OrderedOpcodes.front() : nullptr;
		runner.Require(nullptr != pMismatchReceipt &&
			nullptr != pMismatchOpcode &&
			pMismatchOpcode->eOpcode ==
				EFFECT_CASCADE_OPCODE::UNKNOWN_EXACT_CLASS_QUARANTINE &&
			pMismatchReceipt->eClassLineageStatus ==
				EFFECT_CASCADE_CLASS_LINEAGE_STATUS::
					EXACT_CLASS_HANDLER_QUARANTINED &&
			pMismatchReceipt->bExactClassLineagePreserved &&
			pMismatchReceipt->strAliasId.empty() &&
			!mismatchInspection->bExecutable &&
			std::find(pMismatchReceipt->Blockers.begin(),
				pMismatchReceipt->Blockers.end(),
				"EXACT_SOURCE_CLASS_HANDLER_UNAVAILABLE") !=
					pMismatchReceipt->Blockers.end() &&
			std::find(pMismatchReceipt->Blockers.begin(),
				pMismatchReceipt->Blockers.end(),
				"UNKNOWN_EXACT_CLASS_OPCODE_QUARANTINED") !=
					pMismatchReceipt->Blockers.end() &&
			std::all_of(pMismatchReceipt->PropertyConsumption.begin(),
				pMismatchReceipt->PropertyConsumption.end(), [](const auto& receipt)
				{
					return receipt.eResult ==
						EFFECT_CASCADE_PROPERTY_HANDLER_RESULT::
							QUARANTINED_FIELD_PRESERVED_EXECUTION_BLOCKED;
				}),
			"Cascade Inspection Preserves Exact Custom Class As Handler Quarantined Non Executable Evidence");

		EFFECT_DOCUMENT_DESC provenancePromotion = roundTrip;
		provenancePromotion.Elements.front().SourceRecipe.ModuleCoverage.front().Properties.front().strProvenance =
			"SOURCE_EXACT";
		runner.Require(!CompileMutation(provenancePromotion),
			"Cascade Inspection Rejects SOURCE_TAGGED To SOURCE_EXACT Provenance Promotion");

		EFFECT_DOCUMENT_DESC strippedPropertyBlockerInput = roundTrip;
		EFFECT_SOURCE_PROPERTY_COVERAGE_DESC* pRequiredBlockedProperty = nullptr;
		for (EFFECT_ELEMENT_DESC& element : strippedPropertyBlockerInput.Elements)
		{
			for (EFFECT_SOURCE_MODULE_COVERAGE_DESC& coverage :
				element.SourceRecipe.ModuleCoverage)
			{
				const auto blocked = std::find_if(coverage.Properties.begin(),
					coverage.Properties.end(), [](const auto& property)
					{
						return !property.Blockers.empty();
					});
				if (blocked != coverage.Properties.end())
				{
					pRequiredBlockedProperty = &*blocked;
					break;
				}
			}
			if (nullptr != pRequiredBlockedProperty)
				break;
		}
		if (nullptr != pRequiredBlockedProperty)
			pRequiredBlockedProperty->Blockers.clear();
		runner.Require(nullptr != pRequiredBlockedProperty &&
			!CompileMutation(strippedPropertyBlockerInput),
			"Cascade Inspection Rejects Required Property Blocker Loss");

		EFFECT_DOCUMENT_DESC fabricatedPropertyBlockerInput = roundTrip;
		EFFECT_SOURCE_PROPERTY_COVERAGE_DESC* pUnblockedProperty = nullptr;
		for (EFFECT_ELEMENT_DESC& element : fabricatedPropertyBlockerInput.Elements)
		{
			for (EFFECT_SOURCE_MODULE_COVERAGE_DESC& coverage :
				element.SourceRecipe.ModuleCoverage)
			{
				const auto unblocked = std::find_if(
					coverage.Properties.begin(), coverage.Properties.end(),
					[](const EFFECT_SOURCE_PROPERTY_COVERAGE_DESC& property)
					{
						return property.Blockers.empty();
					});
				if (unblocked != coverage.Properties.end())
				{
					pUnblockedProperty = &*unblocked;
					break;
				}
			}
			if (nullptr != pUnblockedProperty)
				break;
		}
		if (nullptr != pUnblockedProperty)
			pUnblockedProperty->Blockers.emplace_back("FORGED_PROPERTY_BLOCKER");
		runner.Require(nullptr != pUnblockedProperty &&
			!CompileMutation(fabricatedPropertyBlockerInput),
			"Cascade Inspection Rejects Fabricated Property Fidelity Blocker");

		EFFECT_DOCUMENT_DESC prematureAdmission = roundTrip;
		prematureAdmission.Elements.front().SourceRecipe.
			CompiledExecutionAdmission.bAllowed = true;
		prematureAdmission.Elements.front().SourceRecipe.
			CompiledExecutionAdmission.Blockers.clear();
		runner.Require(!CompileMutation(prematureAdmission),
			"Cascade Inspection Rejects Execution Admission Before Payload And Handler Closure");

		EFFECT_DOCUMENT_DESC aggregatePromotion = roundTrip;
		EFFECT_SOURCE_MODULE_COVERAGE_DESC* pPromotedModule = nullptr;
		EFFECT_SOURCE_PROPERTY_COVERAGE_DESC* pPromotedProperty = nullptr;
		for (EFFECT_ELEMENT_DESC& element : aggregatePromotion.Elements)
		{
			for (EFFECT_SOURCE_MODULE_COVERAGE_DESC& coverage :
				element.SourceRecipe.ModuleCoverage)
			{
				if (coverage.strNormalizedClass !=
						"particlemodulecolorscaleoverlife" ||
					coverage.eStatus != EFFECT_SOURCE_COVERAGE_STATUS::UNRESOLVED)
				{
					continue;
				}
				const auto unresolved = std::find_if(
					coverage.Properties.begin(), coverage.Properties.end(),
					[](const EFFECT_SOURCE_PROPERTY_COVERAGE_DESC& property)
					{
						return property.eStatus ==
							EFFECT_SOURCE_COVERAGE_STATUS::UNRESOLVED;
					});
				if (unresolved != coverage.Properties.end())
				{
					pPromotedModule = &coverage;
					pPromotedProperty = &*unresolved;
					break;
				}
			}
			if (nullptr != pPromotedModule)
				break;
		}
		if (nullptr != pPromotedModule && nullptr != pPromotedProperty)
		{
			pPromotedModule->eStatus =
				EFFECT_SOURCE_COVERAGE_STATUS::SOURCE_DECODED;
			pPromotedModule->Blockers.clear();
			pPromotedProperty->eStatus =
				EFFECT_SOURCE_COVERAGE_STATUS::SOURCE_DECODED;
			pPromotedProperty->strProvenance = "SOURCE_TAGGED_PRIMITIVE";
		}
		runner.Require(nullptr != pPromotedModule && nullptr != pPromotedProperty &&
			!CompileMutation(aggregatePromotion),
			"Cascade Inspection Rejects Unresolved ColorScale Module Property Aggregate Promotion");

		EFFECT_DOCUMENT_DESC forgedLod = roundTrip;
		forgedLod.Elements.front().SourceRecipe.CompilerEvidence.strSelectedLodPath =
			"forged.particlelodlevel_0";
		runner.Require(!CompileMutation(forgedLod),
			"Cascade Inspection Rejects Forged Selected LOD Path");

		EFFECT_DOCUMENT_DESC noncanonicalLodSuffix = roundTrip;
		noncanonicalLodSuffix.Elements.front().SourceRecipe.CompilerEvidence.
			strSelectedLodPath = noncanonicalLodSuffix.Elements.front().SourceRecipe.
			CompilerEvidence.strSourceEmitterPath + ".particlelodlevel_01";
		runner.Require(!CompileMutation(noncanonicalLodSuffix),
			"Cascade Inspection Rejects Noncanonical Selected LOD Object Suffix");

		EFFECT_DOCUMENT_DESC reusedLodNode = roundTrip;
		reusedLodNode.Elements.front().SourceRecipe.CompilerEvidence.
			strSelectedLodNodeId = reusedLodNode.Elements.front().SourceRecipe.
			CompilerEvidence.strSourceEmitterNodeId;
		runner.Require(!CompileMutation(reusedLodNode),
			"Cascade Inspection Rejects Emitter Node Reused As Selected LOD Node");

		EFFECT_DOCUMENT_DESC forgedEmitterNode = roundTrip;
		forgedEmitterNode.Elements.front().SourceRecipe.CompilerEvidence.strSourceEmitterNodeId =
			"FX_VALID_LOOKING_OTHER:export:1392";
		runner.Require(!CompileMutation(forgedEmitterNode),
			"Cascade Inspection Rejects Valid Looking Emitter Node Package Mismatch");

		EFFECT_DOCUMENT_DESC aliasLineage = roundTrip;
		aliasLineage.Elements.front().SourceRecipe.ModuleCoverage.front().strModuleStableId +=
			".forged";
		runner.Require(!CompileMutation(aliasLineage),
			"Cascade Inspection Rejects Alias ID And Source Reference Lineage Drift");

		EFFECT_DOCUMENT_DESC validLookingAliasMismatch = roundTrip;
		EFFECT_SOURCE_MODULE_COVERAGE_DESC* pAliasCoverage = nullptr;
		for (EFFECT_ELEMENT_DESC& element : validLookingAliasMismatch.Elements)
		{
			const auto lifetime = std::find_if(
				element.SourceRecipe.ModuleCoverage.begin(),
				element.SourceRecipe.ModuleCoverage.end(),
				[](const EFFECT_SOURCE_MODULE_COVERAGE_DESC& coverage)
				{
					return coverage.strNormalizedClass ==
						"particlemodulelifetime";
				});
			if (lifetime != element.SourceRecipe.ModuleCoverage.end())
			{
				pAliasCoverage = &*lifetime;
				break;
			}
		}
		if (nullptr != pAliasCoverage)
			pAliasCoverage->strNormalizedClass = "particlemodulecolor";
		runner.Require(nullptr != pAliasCoverage &&
			!CompileMutation(validLookingAliasMismatch),
			"Cascade Inspection Rejects Valid Looking Opcode Alias Schema Mismatch");

		EFFECT_DOCUMENT_DESC wrongTypedRole = roundTrip;
		EFFECT_SOURCE_MODULE_REFERENCE_DESC& firstRole =
			wrongTypedRole.Elements.front().SourceRecipe.CompilerEvidence.
				ModuleReferenceOrder.front();
		const bool_t wasRequiredRole = firstRole.strRole == "REQUIRED";
		firstRole.strRole = "MODULE";
		runner.Require(wasRequiredRole && !CompileMutation(wrongTypedRole),
			"Cascade Inspection Rejects REQUIRED Opcode With Valid Looking MODULE Role");

		EFFECT_DOCUMENT_DESC duplicateProperty = roundTrip;
		duplicateProperty.Elements.front().SourceRecipe.ModuleCoverage.front().Properties.push_back(
			duplicateProperty.Elements.front().SourceRecipe.ModuleCoverage.front().Properties.front());
		std::shared_ptr<const EFFECT_CASCADE_INSPECTION_IR>
			lateFailureRollback = inspectionA;
		const auto* pLateFailureCommitted = lateFailureRollback.get();
		runner.Require(!CEffectCascadeCompiler::Compile_SourceInspection(
			duplicateProperty, ExpectedIdentityFor(duplicateProperty),
			lateFailureRollback, status) &&
			lateFailureRollback.get() == pLateFailureCommitted &&
			lateFailureRollback->strInspectionHash ==
				inspectionA->strInspectionHash,
			"Cascade Inspection Rejects Late Duplicate Property Path And Preserves Caller Commit");

		EFFECT_DOCUMENT_DESC unknownStorage = roundTrip;
		unknownStorage.Elements.front().SourceRecipe.ModuleCoverage.front().Properties.front().strStorage =
			"unknown";
		runner.Require(!CompileMutation(unknownStorage),
			"Cascade Inspection Rejects Unknown Property Storage");

		EFFECT_DOCUMENT_DESC duplicateReference = roundTrip;
		if (duplicateReference.Elements.front().SourceRecipe.CompilerEvidence.ModuleReferenceOrder.size() > 1u)
		{
			duplicateReference.Elements.front().SourceRecipe.CompilerEvidence.ModuleReferenceOrder[1u].iSourceReferenceIndex =
				duplicateReference.Elements.front().SourceRecipe.CompilerEvidence.ModuleReferenceOrder.front().iSourceReferenceIndex;
		}
		runner.Require(
			duplicateReference.Elements.front().SourceRecipe.CompilerEvidence.ModuleReferenceOrder.size() > 1u &&
			!CompileMutation(duplicateReference),
			"Cascade Inspection Rejects Duplicate Module Reference Index");

		EFFECT_DOCUMENT_DESC rendererIdentityMismatch = roundTrip;
		rendererIdentityMismatch.Elements.front().eKind =
			EFFECT_ELEMENT_KIND::SCREEN_POST;
		runner.Require(!CompileMutation(rendererIdentityMismatch),
			"Cascade Inspection Rejects Element And Renderer Identity Mismatch");

		EFFECT_DOCUMENT_DESC nonfiniteGeometry = roundTrip;
		EFFECT_SOURCE_GEOMETRY_BINDING_DESC* pGeometry = nullptr;
		for (EFFECT_ELEMENT_DESC& element : nonfiniteGeometry.Elements)
		{
			if (element.SourceRecipe.GeometryBinding.bEnabled)
			{
				pGeometry = &element.SourceRecipe.GeometryBinding;
				break;
			}
		}
		if (nullptr != pGeometry)
			pGeometry->fCarrierGeometryPreScale =
				(std::numeric_limits<f32_t>::infinity)();
		runner.Require(nullptr != pGeometry && !CompileMutation(nonfiniteGeometry),
			"Cascade Inspection Rejects Nonfinite Typed Evidence");

		EFFECT_DOCUMENT_DESC geometryAssetMismatch = roundTrip;
		EFFECT_SOURCE_GEOMETRY_BINDING_DESC* pMismatchedGeometry = nullptr;
		for (EFFECT_ELEMENT_DESC& element : geometryAssetMismatch.Elements)
		{
			if (element.SourceRecipe.GeometryBinding.bEnabled)
			{
				pMismatchedGeometry = &element.SourceRecipe.GeometryBinding;
				break;
			}
		}
		if (nullptr != pMismatchedGeometry)
		{
			pMismatchedGeometry->strAssetId =
				"Effect/Artist/Meshes/valid_looking_mismatch.wmodel";
		}
		runner.Require(nullptr != pMismatchedGeometry &&
			!CompileMutation(geometryAssetMismatch),
			"Cascade Inspection Rejects Safe Looking Geometry And Resource Binding Mismatch");

		EFFECT_DOCUMENT_DESC missingMeshGeometry = roundTrip;
		bool_t removedMeshGeometry = false;
		for (EFFECT_ELEMENT_DESC& element : missingMeshGeometry.Elements)
		{
			if (element.Renderer.eType == EFFECT_RENDERER_TYPE::MESH_PARTICLE &&
				element.SourceRecipe.GeometryBinding.bEnabled)
			{
				element.SourceRecipe.GeometryBinding.bEnabled = false;
				removedMeshGeometry = true;
				break;
			}
		}
		runner.Require(removedMeshGeometry && !CompileMutation(missingMeshGeometry),
			"Cascade Inspection Rejects Mesh Renderer Geometry Binding Removal");

		const std::array<std::string_view, 5u> legacyMigrationClasses = {
			"particlemodulecollision",
			"particlemodulesizemultiplyvelocity",
			"particlemodulesubuvmovie",
			"distributionfloatsoundparameter",
			"distributionvectorconstant"
		};
		bool_t legacyGapsClassified = true;
		for (const std::string_view sourceClass : legacyMigrationClasses)
		{
			const EFFECT_CASCADE_CLASS_REPORT report =
				CEffectCascadeCompiler::Classify_ReceiptClass(sourceClass);
			legacyGapsClassified = legacyGapsClassified &&
				report.eClassification ==
					EFFECT_CASCADE_CLASS_CLASSIFICATION::
						KNOWN_LEGACY_MIGRATION_GAP &&
				report.strOpcodeSchemaId.empty() &&
				!report.strReasonCode.empty();
		}
		runner.Require(legacyGapsClassified,
			"Legacy Collision SizeMultiplyVelocity SubUVMovie FloatSoundParameter VectorConstant Stay Explicit Migration Gaps");
		const auto SameFloat3 = [](const float3_t& left, const float3_t& right)
		{
			return left.x == right.x && left.y == right.y && left.z == right.z;
		};
		const auto SameFloat4 = [](const float4_t& left, const float4_t& right)
		{
			return left.x == right.x && left.y == right.y &&
				left.z == right.z && left.w == right.w;
		};
		const auto SameTypedField = [&SameFloat4](
			const EFFECT_SOURCE_TYPED_FIELD_DESC& left,
			const EFFECT_SOURCE_TYPED_FIELD_DESC& right)
		{
			return left.strPropertyPath == right.strPropertyPath &&
				left.eKind == right.eKind &&
				left.bBoolean == right.bBoolean &&
				left.fNumber == right.fNumber &&
				left.strString == right.strString &&
				SameFloat4(left.vVector, right.vVector);
		};
		const auto SameTypedFields = [&SameTypedField](const auto& left,
			const auto& right)
		{
			return left.size() == right.size() &&
				std::equal(left.begin(), left.end(), right.begin(), SameTypedField);
		};
		const auto NearFloat3 = [](const float3_t& actual,
			const float3_t& expected)
		{
			return std::abs(actual.x - expected.x) <= 0.000001f &&
				std::abs(actual.y - expected.y) <= 0.000001f &&
				std::abs(actual.z - expected.z) <= 0.000001f;
		};
		struct EXPECTED_CUE final
		{
			float3_t vPosition;
			float3_t vScale;
			size_t iParameterOverrideCount = 0u;
		};
		const std::map<std::string, EXPECTED_CUE> expectedCues = {
			{ "skill-31470/clip-000/notify-000",
				{ { 0.3f, 0.f, 0.f }, { 1.f, 1.f, 1.f }, 1u } },
			{ "skill-31470/clip-000/notify-014",
				{ { 0.f, 0.f, 0.f }, { 3.f, 3.f, 3.f }, 0u } },
			{ "skill-31470/clip-000/notify-018",
				{ { 1.f, 0.f, 1.f }, { 3.f, 3.f, 3.f }, 0u } },
			{ "skill-31470/clip-000/notify-022",
				{ { 0.6f, 0.f, 0.f },
					{ 1.2999999523162842f, 1.2999999523162842f,
						1.2999999523162842f }, 1u } },
			{ "skill-31470/clip-000/notify-026",
				{ { 1.f, 0.f, 0.f },
					{ 0.4000000059604645f, 0.4000000059604645f,
						0.4000000059604645f }, 1u } },
			{ "skill-31470/clip-000/notify-028",
				{ { 0.f, 0.6f, 0.f }, { 1.f, 1.f, 1.f }, 0u } },
			{ "skill-31470/clip-000/notify-029",
				{ { 2.f, 1.f, 0.f }, { 1.f, 1.f, 1.f }, 6u } }
		};

		bool_t preserved = parsed && document.Elements.size() == 35u &&
			roundTrip.Elements.size() == document.Elements.size();
		size_t moduleReferenceCount = 0u;
		size_t localReferenceBindingCount = 0u;
		size_t linkedDistributionOccurrenceCount = 0u;
		size_t pointLightBindingCount = 0u;
		bool_t pointLightTypedPayloadPreserved = false;
		size_t meshGeometryCount = 0u;
		size_t nonMeshGeometryIdentityCount = 0u;
		std::set<std::string> evidenceIds;
		std::set<std::string> cueIds;
		std::map<std::string, size_t> cueParameterOverrideCounts;
		std::set<std::string> evidenceFileHashes;
		std::set<std::string> evidenceSelfHashes;
		std::set<std::string> localReferenceFileHashes;
		std::set<std::string> localReferenceSelfHashes;
		std::set<std::string> geometryFileHashes;
		std::set<std::string> geometrySelfHashes;
		for (size_t iElement = 0u;
			preserved && iElement < document.Elements.size(); ++iElement)
		{
			const EFFECT_CASCADE_RECIPE_DESC& before =
				document.Elements[iElement].SourceRecipe;
			const EFFECT_CASCADE_RECIPE_DESC& after =
				roundTrip.Elements[iElement].SourceRecipe;
			const EFFECT_SOURCE_COMPILER_EVIDENCE_DESC& left =
				before.CompilerEvidence;
			const EFFECT_SOURCE_COMPILER_EVIDENCE_DESC& right =
				after.CompilerEvidence;
			evidenceIds.insert(left.strEvidenceId);
			cueIds.insert(left.strSourceCueId);
			evidenceFileHashes.insert(left.strArtifactFileSha256);
			evidenceSelfHashes.insert(left.strArtifactSelfSha256);
			localReferenceFileHashes.insert(
				left.strLocalReferenceClosureFileSha256);
			localReferenceSelfHashes.insert(
				left.strLocalReferenceClosureSelfSha256);
			geometryFileHashes.insert(left.strGeometryParityFileSha256);
			geometrySelfHashes.insert(left.strGeometryParitySelfSha256);
			moduleReferenceCount += left.ModuleReferenceOrder.size();
			localReferenceBindingCount += before.LocalReferenceBindings.size();
			if (before.GeometryBinding.bEnabled)
			{
				++meshGeometryCount;
				if (std::abs(before.GeometryBinding.fCarrierGeometryPreScale -
						0.01f) > 0.0000001f ||
					before.GeometryBinding.strParticleScaleSemantics !=
						"DIMENSIONLESS_AXIS_REORDER_ONLY" ||
					before.GeometryBinding.strStatus !=
						"GLTF_TO_WMODEL_PARITY_PROVEN_UPK_TO_GLTF_UNRESOLVED" ||
					before.GeometryBinding.Blockers.empty())
				{
					preserved = false;
					break;
				}
			}
			else if (before.GeometryBinding.fCarrierGeometryPreScale == 1.f)
				++nonMeshGeometryIdentityCount;
			const auto cue = expectedCues.find(left.strSourceCueId);
			if (cue == expectedCues.end() ||
				!NearFloat3(left.CueLocalTransform.vPosition,
					cue->second.vPosition) ||
				!NearFloat3(left.CueLocalTransform.vRotationDegrees,
					{ 0.f, 0.f, 0.f }) ||
				!NearFloat3(left.CueLocalTransform.vScale,
					cue->second.vScale) ||
				left.ParameterOverrides.size() !=
					cue->second.iParameterOverrideCount)
			{
				preserved = false;
				break;
			}
			const auto insertedOverrideCount = cueParameterOverrideCounts.emplace(
				left.strSourceCueId, left.ParameterOverrides.size());
			if (!insertedOverrideCount.second &&
				insertedOverrideCount.first->second != left.ParameterOverrides.size())
			{
				preserved = false;
				break;
			}
			preserved = left.strArtifactFileSha256 == right.strArtifactFileSha256 &&
				left.strArtifactSelfSha256 == right.strArtifactSelfSha256 &&
				left.strEvidenceId == right.strEvidenceId &&
				left.strSourceCueId == right.strSourceCueId &&
				left.strSourceOccurrenceId == right.strSourceOccurrenceId &&
				left.strSelectedLodPath == right.strSelectedLodPath &&
				left.ModuleReferenceOrder.size() ==
					right.ModuleReferenceOrder.size() &&
				SameFloat3(left.vCueSourcePositionUeUnits,
					right.vCueSourcePositionUeUnits) &&
				SameFloat3(left.CueLocalTransform.vPosition,
					right.CueLocalTransform.vPosition) &&
				SameFloat3(left.CueLocalTransform.vRotationDegrees,
					right.CueLocalTransform.vRotationDegrees) &&
				SameFloat3(left.CueLocalTransform.vScale,
					right.CueLocalTransform.vScale) &&
				left.ParameterOverrides.size() == right.ParameterOverrides.size() &&
				left.CompositionOrder == right.CompositionOrder &&
				left.strLocalReferenceClosureFileSha256 ==
					right.strLocalReferenceClosureFileSha256 &&
				left.strLocalReferenceClosureSelfSha256 ==
					right.strLocalReferenceClosureSelfSha256 &&
				left.strGeometryParityFileSha256 ==
					right.strGeometryParityFileSha256 &&
				left.strGeometryParitySelfSha256 ==
					right.strGeometryParitySelfSha256 &&
				before.CompiledExecutionAdmission.bAllowed ==
					after.CompiledExecutionAdmission.bAllowed &&
				before.CompiledExecutionAdmission.Blockers ==
					after.CompiledExecutionAdmission.Blockers &&
				before.MaterialAdmission.strStatus == after.MaterialAdmission.strStatus &&
				before.MaterialAdmission.Blockers == after.MaterialAdmission.Blockers &&
				before.GeometryBinding.strReceiptFileSha256 ==
					after.GeometryBinding.strReceiptFileSha256 &&
				before.GeometryBinding.strReceiptSelfSha256 ==
					after.GeometryBinding.strReceiptSelfSha256 &&
				before.GeometryBinding.fCarrierGeometryPreScale ==
					after.GeometryBinding.fCarrierGeometryPreScale &&
				before.GeometryBinding.strParticleScaleSemantics ==
					after.GeometryBinding.strParticleScaleSemantics &&
				before.LocalReferenceBindings.size() ==
					after.LocalReferenceBindings.size() &&
				before.ModuleCoverage.size() == after.ModuleCoverage.size();
			for (size_t iBinding = 0u;
				preserved && iBinding < before.LocalReferenceBindings.size();
				++iBinding)
			{
				const EFFECT_SOURCE_LOCAL_REFERENCE_BINDING_DESC& a =
					before.LocalReferenceBindings[iBinding];
				const EFFECT_SOURCE_LOCAL_REFERENCE_BINDING_DESC& b =
					after.LocalReferenceBindings[iBinding];
				preserved = a.strReferenceKind == b.strReferenceKind &&
					a.strReferenceId == b.strReferenceId &&
					a.strDefinitionId == b.strDefinitionId &&
					a.strOccurrenceId == b.strOccurrenceId &&
					a.strModuleStableId == b.strModuleStableId &&
					a.strPropertyPath == b.strPropertyPath &&
					a.strProvenance == b.strProvenance &&
					SameTypedFields(a.ExactPayload, b.ExactPayload) &&
					SameTypedFields(a.CurrentDefaultEvidence,
						b.CurrentDefaultEvidence) &&
					a.ExecutionAdmission.bAllowed ==
						b.ExecutionAdmission.bAllowed &&
					a.ExecutionAdmission.Blockers ==
						b.ExecutionAdmission.Blockers;
				if (a.strReferenceKind == "TYPEDATA_COMPONENT")
				{
					++pointLightBindingCount;
					const auto FindField = [](const auto& Fields,
						const std::string_view PropertyPath)
					{
						return std::find_if(Fields.begin(), Fields.end(),
							[PropertyPath](const EFFECT_SOURCE_TYPED_FIELD_DESC& Field)
							{
								return Field.strPropertyPath == PropertyPath;
							});
					};
					const auto Brightness = FindField(
						a.ExactPayload, "brightness");
					const auto CastComposite = FindField(
						a.ExactPayload, "bcastcompositeshadow");
					const auto AffectComposite = FindField(
						a.ExactPayload, "baffectcompositeshadowdirection");
					const auto Radius = FindField(
						a.CurrentDefaultEvidence, "radius");
					const auto Falloff = FindField(
						a.CurrentDefaultEvidence, "falloffexponent");
					const auto LightColor = FindField(
						a.CurrentDefaultEvidence, "lightcolor");
					pointLightTypedPayloadPreserved =
						Brightness != a.ExactPayload.end() &&
						Brightness->eKind ==
							EFFECT_SOURCE_TYPED_FIELD_KIND::NUMBER &&
						Brightness->fNumber == 10.0 &&
						CastComposite != a.ExactPayload.end() &&
						CastComposite->eKind ==
							EFFECT_SOURCE_TYPED_FIELD_KIND::BOOLEAN &&
						!CastComposite->bBoolean &&
						AffectComposite != a.ExactPayload.end() &&
						AffectComposite->eKind ==
							EFFECT_SOURCE_TYPED_FIELD_KIND::BOOLEAN &&
						!AffectComposite->bBoolean &&
						Radius != a.CurrentDefaultEvidence.end() &&
						Radius->eKind ==
							EFFECT_SOURCE_TYPED_FIELD_KIND::NUMBER &&
						Radius->fNumber == 200.0 &&
						Falloff != a.CurrentDefaultEvidence.end() &&
						Falloff->eKind ==
							EFFECT_SOURCE_TYPED_FIELD_KIND::NUMBER &&
						Falloff->fNumber == 2.0 &&
						LightColor != a.CurrentDefaultEvidence.end() &&
						LightColor->eKind ==
							EFFECT_SOURCE_TYPED_FIELD_KIND::VECTOR &&
						LightColor->vVector.x == 255.f &&
						LightColor->vVector.y == 255.f &&
						LightColor->vVector.z == 255.f;
				}
			}
			preserved = preserved && before.Modules.size() == after.Modules.size();
			for (size_t iModule = 0u;
				preserved && iModule < before.Modules.size(); ++iModule)
			{
				const EFFECT_SOURCE_MODULE_DESC& a = before.Modules[iModule];
				const EFFECT_SOURCE_MODULE_DESC& b = after.Modules[iModule];
				preserved = a.strStableId == b.strStableId &&
					a.Distributions.size() == b.Distributions.size();
				for (size_t iDistribution = 0u;
					preserved && iDistribution < a.Distributions.size();
					++iDistribution)
				{
					const EFFECT_DISTRIBUTION_DESC& x =
						a.Distributions[iDistribution];
					const EFFECT_DISTRIBUTION_DESC& y =
						b.Distributions[iDistribution];
					preserved = x.strReferenceId == y.strReferenceId &&
						x.strOccurrenceId == y.strOccurrenceId &&
						x.strPayloadStatus == y.strPayloadStatus &&
						x.strFidelity == y.strFidelity &&
						x.ExecutionAdmission.bAllowed ==
							y.ExecutionAdmission.bAllowed &&
						x.ExecutionAdmission.Blockers ==
							y.ExecutionAdmission.Blockers;
					if (!x.strOccurrenceId.empty())
						++linkedDistributionOccurrenceCount;
				}
			}
			for (size_t iModule = 0u;
				preserved && iModule < left.ModuleReferenceOrder.size(); ++iModule)
			{
				const EFFECT_SOURCE_MODULE_REFERENCE_DESC& a =
					left.ModuleReferenceOrder[iModule];
				const EFFECT_SOURCE_MODULE_REFERENCE_DESC& b =
					right.ModuleReferenceOrder[iModule];
				preserved = a.iOrder == b.iOrder &&
					a.iSourceReferenceIndex == b.iSourceReferenceIndex &&
					a.strRole == b.strRole &&
					a.strSourceObjectId == b.strSourceObjectId &&
					a.strSourceRecordSha256 == b.strSourceRecordSha256;
			}
			for (size_t iCoverage = 0u;
				preserved && iCoverage < before.ModuleCoverage.size(); ++iCoverage)
			{
				const EFFECT_SOURCE_MODULE_COVERAGE_DESC& a =
					before.ModuleCoverage[iCoverage];
				const EFFECT_SOURCE_MODULE_COVERAGE_DESC& b =
					after.ModuleCoverage[iCoverage];
				preserved = a.Blockers == b.Blockers &&
					a.Properties.size() == b.Properties.size();
				for (size_t iProperty = 0u;
					preserved && iProperty < a.Properties.size(); ++iProperty)
				{
					preserved = a.Properties[iProperty].strProvenance ==
						b.Properties[iProperty].strProvenance &&
						a.Properties[iProperty].Blockers ==
						b.Properties[iProperty].Blockers;
				}
			}
		}
		size_t uniqueParameterOverrideCount = 0u;
		for (const auto& cue : cueParameterOverrideCounts)
			uniqueParameterOverrideCount += cue.second;
		preserved = preserved && evidenceIds.size() == 35u &&
			cueIds.size() == 7u && moduleReferenceCount == 399u &&
			localReferenceBindingCount == 18u &&
			linkedDistributionOccurrenceCount == 17u &&
			pointLightBindingCount == 1u &&
			pointLightTypedPayloadPreserved &&
			cueParameterOverrideCounts.size() == 7u &&
			uniqueParameterOverrideCount == 9u &&
			meshGeometryCount == 13u && nonMeshGeometryIdentityCount == 22u &&
			evidenceFileHashes.size() == 1u && evidenceSelfHashes.size() == 1u &&
			localReferenceFileHashes.size() == 1u &&
			localReferenceSelfHashes.size() == 1u &&
			geometryFileHashes.size() == 1u && geometrySelfHashes.size() == 1u;
		runner.Require(preserved,
			"Artist F Source Contract Preserves 35 Occurrences 18 Local References Typed PointLight Evidence And Receipt Hashes");

		EFFECT_DOCUMENT_DESC highPrecisionLiteral = roundTrip;
		size_t highPrecisionElementIndex = highPrecisionLiteral.Elements.size();
		size_t highPrecisionModuleIndex = 0u;
		size_t highPrecisionLiteralIndex = 0u;
		constexpr f64_t highPrecisionValue = 0.12345678901234566;
		for (size_t iElement = 0u;
			iElement < highPrecisionLiteral.Elements.size(); ++iElement)
		{
			auto& modules =
				highPrecisionLiteral.Elements[iElement].SourceRecipe.Modules;
			for (size_t iModule = 0u; iModule < modules.size(); ++iModule)
			{
				const auto literal = std::find_if(modules[iModule].Literals.begin(),
					modules[iModule].Literals.end(),
					[](const EFFECT_SOURCE_LITERAL_DESC& value)
					{
						return value.eKind == EFFECT_SOURCE_LITERAL_KIND::NUMBER;
					});
				if (literal == modules[iModule].Literals.end())
					continue;
				highPrecisionElementIndex = iElement;
				highPrecisionModuleIndex = iModule;
				highPrecisionLiteralIndex = static_cast<size_t>(
					std::distance(modules[iModule].Literals.begin(), literal));
				literal->fNumber = highPrecisionValue;
				break;
			}
			if (highPrecisionElementIndex != highPrecisionLiteral.Elements.size())
				break;
		}
		const bool_t foundHighPrecisionLiteral =
			highPrecisionElementIndex < highPrecisionLiteral.Elements.size();
		const std::string highPrecisionJson = foundHighPrecisionLiteral ?
			CEffectDocumentCodec::Serialize(highPrecisionLiteral) : std::string{};
		EFFECT_DOCUMENT_DESC highPrecisionRoundTrip;
		const bool_t parsedHighPrecision = foundHighPrecisionLiteral &&
			CEffectDocumentCodec::Parse(
				highPrecisionJson, highPrecisionRoundTrip, status);
		const bool_t preservedHighPrecision = parsedHighPrecision &&
			highPrecisionElementIndex < highPrecisionRoundTrip.Elements.size() &&
			highPrecisionModuleIndex < highPrecisionRoundTrip.Elements[
				highPrecisionElementIndex].SourceRecipe.Modules.size() &&
			highPrecisionLiteralIndex < highPrecisionRoundTrip.Elements[
				highPrecisionElementIndex].SourceRecipe.Modules[
					highPrecisionModuleIndex].Literals.size() &&
			highPrecisionRoundTrip.Elements[highPrecisionElementIndex].SourceRecipe.
				Modules[highPrecisionModuleIndex].Literals[
					highPrecisionLiteralIndex].fNumber == highPrecisionValue &&
			CEffectDocumentCodec::Serialize(highPrecisionRoundTrip) ==
				highPrecisionJson;
		runner.Require(preservedHighPrecision,
			"Source Literal Number Round Trip Preserves Double Precision");

		std::string legacy = serialized;
		const std::string version14 = "\"version\": 14";
		const size_t versionOffset = legacy.find(version14);
		if (std::string::npos != versionOffset)
			legacy.replace(versionOffset, version14.size(), "\"version\": 13");
		const std::string purpose = "  \"purpose\": \"source_contract\",\n";
		const size_t purposeOffset = legacy.find(purpose);
		if (std::string::npos != purposeOffset)
			legacy.erase(purposeOffset, purpose.size());
		EFFECT_DOCUMENT_DESC rejected;
		runner.Require(std::string::npos != versionOffset &&
			std::string::npos != purposeOffset &&
			!CEffectDocumentCodec::Parse(legacy, rejected, status),
			"Legacy Effect Rejects Native V14 Source Evidence Instead Of Erasing It");

		std::string badOrder = serialized;
		const std::string sourceOrder =
			"\"moduleReferenceOrder\": [{ \"order\": 0";
		const size_t orderOffset = badOrder.find(sourceOrder);
		if (std::string::npos != orderOffset)
			badOrder.replace(orderOffset, sourceOrder.size(),
				"\"moduleReferenceOrder\": [{ \"order\": 99");
		runner.Require(std::string::npos != orderOffset &&
			!CEffectDocumentCodec::Parse(badOrder, rejected, status),
			"Artist F Source Contract Fails Closed When Module Reference Order Changes");

		std::string missingRandomLockAxes = serialized;
		const std::string randomLockAxesField = ", \"randomLockAxes\": ";
		const size_t randomLockAxesOffset =
			missingRandomLockAxes.find(randomLockAxesField);
		const size_t randomLockAxesEnd = std::string::npos == randomLockAxesOffset ?
			std::string::npos : missingRandomLockAxes.find(
				',', randomLockAxesOffset + randomLockAxesField.size());
		if (std::string::npos != randomLockAxesOffset &&
			std::string::npos != randomLockAxesEnd)
		{
			missingRandomLockAxes.erase(randomLockAxesOffset,
				randomLockAxesEnd - randomLockAxesOffset);
		}
		runner.Require(std::string::npos != randomLockAxesOffset &&
			std::string::npos != randomLockAxesEnd &&
			!CEffectDocumentCodec::Parse(
				missingRandomLockAxes, rejected, status),
			"Source Contract Requires Explicit Distribution Random Lock Axes");

		EFFECT_DOCUMENT_DESC badLocalReference = roundTrip;
		EFFECT_DISTRIBUTION_DESC* pLinkedDistribution = nullptr;
		for (EFFECT_ELEMENT_DESC& element : badLocalReference.Elements)
		{
			for (EFFECT_SOURCE_MODULE_DESC& module :
				element.SourceRecipe.Modules)
			{
				const auto linked = std::find_if(module.Distributions.begin(),
					module.Distributions.end(),
					[](const EFFECT_DISTRIBUTION_DESC& distribution)
					{
						return !distribution.strOccurrenceId.empty();
					});
				if (linked != module.Distributions.end())
				{
					pLinkedDistribution = &*linked;
					break;
				}
			}
			if (nullptr != pLinkedDistribution)
				break;
		}
		if (nullptr != pLinkedDistribution)
			pLinkedDistribution->strReferenceId += ".mismatch";
		runner.Require(nullptr != pLinkedDistribution &&
			!CEffectDocumentCodec::Validate_SourceContract(
				badLocalReference, status),
			"Source Distribution Rejects A Local Reference Identity Mismatch");

		const auto FindFirstDistributionCoverage = [](
			EFFECT_DOCUMENT_DESC& candidate)
		{
			using Result = std::pair<EFFECT_DISTRIBUTION_DESC*,
				EFFECT_SOURCE_PROPERTY_COVERAGE_DESC*>;
			for (EFFECT_ELEMENT_DESC& element : candidate.Elements)
			{
				for (EFFECT_SOURCE_MODULE_DESC& module :
					element.SourceRecipe.Modules)
				{
					if (module.Distributions.empty())
						continue;
					EFFECT_DISTRIBUTION_DESC* pDistribution =
						&module.Distributions.front();
					const auto moduleCoverage = std::find_if(
						element.SourceRecipe.ModuleCoverage.begin(),
						element.SourceRecipe.ModuleCoverage.end(),
						[&module](
							const EFFECT_SOURCE_MODULE_COVERAGE_DESC& coverage)
						{
							return coverage.strModuleStableId == module.strStableId;
						});
					if (moduleCoverage ==
						element.SourceRecipe.ModuleCoverage.end())
					{
						return Result{ pDistribution, nullptr };
					}
					const auto propertyCoverage = std::find_if(
						moduleCoverage->Properties.begin(),
						moduleCoverage->Properties.end(),
						[pDistribution](
							const EFFECT_SOURCE_PROPERTY_COVERAGE_DESC& property)
						{
							return property.strStorage == "distribution" &&
								property.strPropertyPath ==
									pDistribution->strPropertyPath;
						});
					return Result{ pDistribution,
						propertyCoverage == moduleCoverage->Properties.end() ?
							nullptr : &*propertyCoverage };
				}
			}
			return Result{ nullptr, nullptr };
		};
		const auto ScrubAsUnresolved = [](
			EFFECT_DISTRIBUTION_DESC& distribution,
			EFFECT_SOURCE_PROPERTY_COVERAGE_DESC& coverage)
		{
			distribution.strPayloadStatus =
				"UNRESOLVED_SYNTHETIC_ADMISSION_ORACLE";
			distribution.strFidelity =
				"UNRESOLVED_SYNTHETIC_ADMISSION_ORACLE";
			coverage.eStatus = EFFECT_SOURCE_COVERAGE_STATUS::UNRESOLVED;
			distribution.eParameterBinding =
				EFFECT_DISTRIBUTION_PARAMETER_BINDING::NONE;
			distribution.strParameterName.clear();
			distribution.iOperation = 0u;
			distribution.iRandomLockAxes = 0u;
			distribution.iLookupTableChunkSize = 0u;
			distribution.iLookupTableNumElements = 0u;
			distribution.fLookupTableTimeScale = 0.f;
			distribution.fLookupTableStartTime = 0.f;
			distribution.vDefaultMinimum = { 0.f, 0.f, 0.f, 0.f };
			distribution.vDefaultMaximum = { 0.f, 0.f, 0.f, 0.f };
			distribution.LookupTable.clear();
			distribution.Keys.clear();
		};

		EFFECT_DOCUMENT_DESC badUnresolvedAdmission = roundTrip;
		auto [pUnresolvedDistribution, pUnresolvedCoverage] =
			FindFirstDistributionCoverage(badUnresolvedAdmission);
		if (nullptr != pUnresolvedDistribution && nullptr != pUnresolvedCoverage)
		{
			ScrubAsUnresolved(
				*pUnresolvedDistribution, *pUnresolvedCoverage);
			pUnresolvedDistribution->ExecutionAdmission.Blockers.clear();
			pUnresolvedDistribution->vDefaultMinimum.x = 1.f;
		}
		status.clear();
		runner.Require(nullptr != pUnresolvedDistribution &&
			nullptr != pUnresolvedCoverage &&
			!CEffectDocumentCodec::Validate_SourceContract(
				badUnresolvedAdmission, status) &&
			status == "Source-contract distribution admission is invalid.",
			"Unresolved Distribution Admission Rejects Before Executable Payload Validation");

		EFFECT_DOCUMENT_DESC badUnresolvedPayload = roundTrip;
		auto [pPayloadDistribution, pPayloadCoverage] =
			FindFirstDistributionCoverage(badUnresolvedPayload);
		if (nullptr != pPayloadDistribution && nullptr != pPayloadCoverage)
		{
			ScrubAsUnresolved(*pPayloadDistribution, *pPayloadCoverage);
			pPayloadDistribution->iOperation = 1u;
		}
		status.clear();
		runner.Require(nullptr != pPayloadDistribution &&
			nullptr != pPayloadCoverage &&
			!CEffectDocumentCodec::Validate_SourceContract(
				badUnresolvedPayload, status) &&
			status ==
				"Unresolved source distribution carries executable payload.",
			"Unresolved Distribution Scrubs Every Evaluator Input");

		EFFECT_DOCUMENT_DESC badBlockerPropagation = roundTrip;
		bool_t removedRequiredBlocker = false;
		for (EFFECT_ELEMENT_DESC& element : badBlockerPropagation.Elements)
		{
			for (const EFFECT_SOURCE_MODULE_DESC& module :
				element.SourceRecipe.Modules)
			{
				const auto blocked = std::find_if(module.Distributions.begin(),
					module.Distributions.end(),
					[](const EFFECT_DISTRIBUTION_DESC& distribution)
					{
						return !distribution.ExecutionAdmission.Blockers.empty();
					});
				if (blocked == module.Distributions.end())
					continue;
				const std::string blocker =
					blocked->ExecutionAdmission.Blockers.front();
				auto& productBlockers =
					element.SourceRecipe.CompiledExecutionAdmission.Blockers;
				const size_t priorSize = productBlockers.size();
				productBlockers.erase(std::remove(productBlockers.begin(),
					productBlockers.end(), blocker), productBlockers.end());
				removedRequiredBlocker = priorSize != productBlockers.size();
				break;
			}
			if (removedRequiredBlocker)
				break;
		}
		runner.Require(removedRequiredBlocker &&
			!CEffectDocumentCodec::Validate_SourceContract(
				badBlockerPropagation, status),
			"Source Distribution Blockers Cannot Be Laundered At Product Admission");

		EFFECT_DOCUMENT_DESC badBindingBlockerPropagation = roundTrip;
		bool_t removedBindingOnlyBlocker = false;
		for (EFFECT_ELEMENT_DESC& element :
			badBindingBlockerPropagation.Elements)
		{
			const auto binding = std::find_if(
				element.SourceRecipe.LocalReferenceBindings.begin(),
				element.SourceRecipe.LocalReferenceBindings.end(),
				[](const EFFECT_SOURCE_LOCAL_REFERENCE_BINDING_DESC& value)
				{
					return value.strReferenceKind == "DISTRIBUTION_TARGET" &&
						value.ExecutionAdmission.Blockers.size() > 1u;
				});
			if (binding == element.SourceRecipe.LocalReferenceBindings.end())
				continue;
			binding->ExecutionAdmission.Blockers.erase(
				binding->ExecutionAdmission.Blockers.begin());
			removedBindingOnlyBlocker = true;
			break;
		}
		runner.Require(removedBindingOnlyBlocker &&
			!CEffectDocumentCodec::Validate_SourceContract(
				badBindingBlockerPropagation, status),
			"Source Distribution And Local Binding Blockers Cannot Diverge");

		EFFECT_DOCUMENT_DESC legacyInMemory;
		legacyInMemory.strEffectAssetId = "effect.legacy.native.field.guard";
		legacyInMemory.strDisplayName = "Legacy Native Field Guard";
		EFFECT_ELEMENT_DESC legacyElement;
		legacyElement.strElementId = "sprite";
		legacyElement.strDisplayName = "Sprite";
		legacyElement.eKind = EFFECT_ELEMENT_KIND::SPRITE;
		legacyElement.bVisible = false;
		legacyInMemory.Elements.push_back(legacyElement);
		const bool_t legacyBaselineValid =
			CEffectDocumentCodec::Validate(legacyInMemory, status);
		const auto RejectsLegacyNativeFields = [&](EFFECT_DOCUMENT_DESC candidate)
		{
			return !CEffectDocumentCodec::Validate(candidate, status);
		};

		const std::string legacyJson =
			CEffectDocumentCodec::Serialize(legacyInMemory);
		EFFECT_DOCUMENT_DESC legacyJsonBaseline;
		const bool_t legacyJsonBaselineValid =
			CEffectDocumentCodec::Parse(legacyJson, legacyJsonBaseline, status);
		std::string legacyRendererJson = legacyJson;
		const std::string resourcesField = "      \"resources\": [";
		const size_t resourcesOffset = legacyRendererJson.find(resourcesField);
		if (std::string::npos != resourcesOffset)
		{
			legacyRendererJson.insert(resourcesOffset,
				"      \"renderer\": { \"type\": \"spriteParticle\", "
				"\"sourceSpace\": \"ue3CascadeV1\" },\n");
		}
		runner.Require(legacyBaselineValid && legacyJsonBaselineValid &&
			std::string::npos != resourcesOffset &&
			!CEffectDocumentCodec::Parse(
				legacyRendererJson, rejected, status),
			"Legacy JSON Rejects Isolated Native V14 Renderer Evidence");

		EFFECT_DOCUMENT_DESC legacyRendererEvidence = legacyInMemory;
		legacyRendererEvidence.Elements.front().Renderer.eType =
			EFFECT_RENDERER_TYPE::SPRITE_PARTICLE;
		legacyRendererEvidence.Elements.front().Renderer.eSourceSpace =
			EFFECT_SOURCE_SPACE::UE3_CASCADE_V1;
		runner.Require(RejectsLegacyNativeFields(legacyRendererEvidence),
			"Legacy In-Memory Document Rejects Native V14 Renderer Evidence");

		EFFECT_DOCUMENT_DESC legacyDistributionBaseline = legacyInMemory;
		auto& legacyRecipe =
			legacyDistributionBaseline.Elements.front().SourceRecipe;
		legacyRecipe.bEnabled = true;
		legacyRecipe.strRendererShape = "sprite";
		EFFECT_SOURCE_MODULE_DESC legacyDistributionModule;
		legacyDistributionModule.strStableId = "legacy.module@ref:0";
		legacyDistributionModule.strClassName = "ParticleModuleSize";
		legacyDistributionModule.strObjectPath = "legacy.module.size";
		EFFECT_DISTRIBUTION_DESC legacyDistribution;
		legacyDistribution.strPropertyPath = "StartSize";
		legacyDistribution.strSourceClass = "DistributionFloatConstant";
		legacyDistribution.strSourceObjectPath = "legacy.distribution.constant";
		legacyDistributionModule.Distributions.push_back(legacyDistribution);
		legacyRecipe.Modules.push_back(legacyDistributionModule);
		const bool_t legacyDistributionBaselineValid =
			CEffectDocumentCodec::Validate(legacyDistributionBaseline, status);
		const std::string legacyDistributionJson =
			legacyDistributionBaselineValid ?
			CEffectDocumentCodec::Serialize(legacyDistributionBaseline) :
			std::string{};
		EFFECT_DOCUMENT_DESC legacyDistributionJsonBaseline;
		const bool_t legacyDistributionJsonBaselineValid =
			legacyDistributionBaselineValid && CEffectDocumentCodec::Parse(
				legacyDistributionJson, legacyDistributionJsonBaseline, status);
		const std::string sourceObjectPathField =
			"\"sourceObjectPath\": \"legacy.distribution.constant\"";
		const size_t sourceObjectPathOffset =
			legacyDistributionJson.find(sourceObjectPathField);
		std::string legacyParameterBindingJson = legacyDistributionJson;
		std::string legacyParameterNameJson = legacyDistributionJson;
		if (std::string::npos != sourceObjectPathOffset)
		{
			const size_t insertionOffset =
				sourceObjectPathOffset + sourceObjectPathField.size();
			legacyParameterBindingJson.insert(insertionOffset,
				", \"parameterBinding\": \"actionCue\"");
			legacyParameterNameJson.insert(insertionOffset,
				", \"parameterName\": \"Scale\"");
		}
		runner.Require(legacyDistributionJsonBaselineValid &&
			std::string::npos != sourceObjectPathOffset &&
			!CEffectDocumentCodec::Parse(
				legacyParameterBindingJson, rejected, status) &&
			!CEffectDocumentCodec::Parse(
				legacyParameterNameJson, rejected, status),
			"Legacy JSON Rejects Isolated Distribution Parameter Evidence");

		EFFECT_DOCUMENT_DESC legacyParameterBindingEvidence =
			legacyDistributionBaseline;
		legacyParameterBindingEvidence.Elements.front().SourceRecipe.Modules.front().
			Distributions.front().eParameterBinding =
				EFFECT_DISTRIBUTION_PARAMETER_BINDING::ACTION_CUE;
		EFFECT_DOCUMENT_DESC legacyParameterNameEvidence =
			legacyDistributionBaseline;
		legacyParameterNameEvidence.Elements.front().SourceRecipe.Modules.front().
			Distributions.front().strParameterName = "Scale";
		runner.Require(
			RejectsLegacyNativeFields(legacyParameterBindingEvidence) &&
			RejectsLegacyNativeFields(legacyParameterNameEvidence),
			"Legacy In-Memory Document Rejects Distribution Parameter Evidence");

		EFFECT_DOCUMENT_DESC legacyCompilerEvidence = legacyInMemory;
		legacyCompilerEvidence.Elements.front().SourceRecipe.CompilerEvidence.
			strEvidenceId = "forbidden.native.evidence";
		EFFECT_DOCUMENT_DESC legacyCompiledAdmission = legacyInMemory;
		legacyCompiledAdmission.Elements.front().SourceRecipe.
			CompiledExecutionAdmission.Blockers.push_back("FORBIDDEN_NATIVE_BLOCKER");
		EFFECT_DOCUMENT_DESC legacyMaterialAdmission = legacyInMemory;
		legacyMaterialAdmission.Elements.front().SourceRecipe.MaterialAdmission.
			strStatus = "BLOCKED_MATERIAL_RECIPE_MISSING";
		EFFECT_DOCUMENT_DESC legacyGeometryBinding = legacyInMemory;
		legacyGeometryBinding.Elements.front().SourceRecipe.GeometryBinding.
			strReceiptFileSha256 = std::string(64u, '0');
		EFFECT_DOCUMENT_DESC legacyCoverage = legacyInMemory;
		EFFECT_SOURCE_MODULE_COVERAGE_DESC legacyModuleCoverage;
		legacyModuleCoverage.strModuleStableId = "forbidden.native.coverage";
		legacyModuleCoverage.Blockers.push_back("FORBIDDEN_NATIVE_BLOCKER");
		EFFECT_SOURCE_PROPERTY_COVERAGE_DESC legacyPropertyCoverage;
		legacyPropertyCoverage.strPropertyPath = "forbidden";
		legacyPropertyCoverage.strProvenance = "UNRESOLVED";
		legacyModuleCoverage.Properties.push_back(legacyPropertyCoverage);
		legacyCoverage.Elements.front().SourceRecipe.ModuleCoverage.push_back(
			legacyModuleCoverage);
		EFFECT_DOCUMENT_DESC legacyLocalReference = legacyInMemory;
		EFFECT_SOURCE_LOCAL_REFERENCE_BINDING_DESC legacyBinding;
		legacyBinding.strReferenceId = "forbidden.native.local-reference";
		legacyLocalReference.Elements.front().SourceRecipe.LocalReferenceBindings.
			push_back(legacyBinding);
		EFFECT_DOCUMENT_DESC legacyDistributionEvidence = legacyInMemory;
		EFFECT_SOURCE_MODULE_DESC legacySourceModule;
		EFFECT_DISTRIBUTION_DESC legacySourceDistribution;
		legacySourceDistribution.strPayloadStatus = "FORBIDDEN_NATIVE_STATUS";
		legacySourceModule.Distributions.push_back(legacySourceDistribution);
		legacyDistributionEvidence.Elements.front().SourceRecipe.Modules.push_back(
			legacySourceModule);
		runner.Require(legacyBaselineValid &&
			RejectsLegacyNativeFields(legacyRendererEvidence) &&
			RejectsLegacyNativeFields(legacyParameterBindingEvidence) &&
			RejectsLegacyNativeFields(legacyParameterNameEvidence) &&
			RejectsLegacyNativeFields(legacyCompilerEvidence) &&
			RejectsLegacyNativeFields(legacyCompiledAdmission) &&
			RejectsLegacyNativeFields(legacyMaterialAdmission) &&
			RejectsLegacyNativeFields(legacyGeometryBinding) &&
			RejectsLegacyNativeFields(legacyCoverage) &&
			RejectsLegacyNativeFields(legacyLocalReference) &&
			RejectsLegacyNativeFields(legacyDistributionEvidence),
			"Legacy In-Memory Document Rejects Every Native V14 Evidence Family Before Serialization");
	}
}

int main(const int argc, char* argv[])
{
	TEST_RUNNER runner{};
	const std::string Mode = argc > 1 && nullptr != argv[1] ? argv[1] : "";
	if (Mode == "--effect-document" && argc > 2 && nullptr != argv[2])
	{
		Client::EFFECT_DOCUMENT_DESC document;
		std::string status;
		const bool_t loaded = Client::CEffectDocumentCodec::Load(
			std::filesystem::path(argv[2]), document, status);
		std::cout << (loaded ? "[PASS] " : "[FAILURE] ")
			<< "Effect Document V12 Parse: " << status << '\n';
		return loaded ? 0 : 1;
	}
	if (Mode == "--effect-source-contract" && argc > 2 && nullptr != argv[2])
	{
		Test_Artist31470SourceContractRoundTrip(
			runner, std::filesystem::path(argv[2]));
		std::cout << "failures : " << runner.iFailureCount << '\n';
		return 0 == runner.iFailureCount ? 0 : 1;
	}
	if (Mode == "--skill-binding-fast")
	{
		Test_ActionPresentationTimeline(runner);
		Test_RealSkillBindingDocuments(runner);
		std::cout << "failures : " << runner.iFailureCount << '\n';
		return 0 == runner.iFailureCount ? 0 : 1;
	}
	if (Mode == "--effect-executor-fast")
	{
		Test_ActionPresentationTimeline(runner);
		Test_RealSkillBindingDocuments(runner);
		Test_EffectPlaybackDeterminism(runner);
		Test_EffectTypedPresentation(runner);
		Test_EffectSourceModuleOccurrenceOrder(runner);
		Test_EffectExactSourceSemantics(runner);
		Test_DimensionMasterSourceSemanticAssets(runner);
		std::cout << "failures : " << runner.iFailureCount << '\n';
		return 0 == runner.iFailureCount ? 0 : 1;
	}
	if (Mode == "--effect-runtime-fast")
	{
		Test_ActionPresentationTimeline(runner);
		Test_RealSkillBindingDocuments(runner);
		Test_EffectAssemblyRuntimeCatalog(runner, false);
		std::cout << "failures : " << runner.iFailureCount << '\n';
		return 0 == runner.iFailureCount ? 0 : 1;
	}
	if (Mode == "--effect-runtime-authority")
	{
		Test_EffectRuntimeAuthorityCatalog(runner);
		std::cout << "failures : " << runner.iFailureCount << '\n';
		return 0 == runner.iFailureCount ? 0 : 1;
	}
	if (Mode == "--effect-imported-fast")
	{
		Test_DimensionMasterImportedPortalDraft(runner);
		std::cout << "failures : " << runner.iFailureCount << '\n';
		return 0 == runner.iFailureCount ? 0 : 1;
	}
	if (Mode == "--effect-authoring-fast")
	{
		Test_EffectDraftAtomicSave(runner);
		Test_EffectTypedPresentation(runner);
		std::cout << "failures : " << runner.iFailureCount << '\n';
		return 0 == runner.iFailureCount ? 0 : 1;
	}

	Test_NormalHandoff(runner);
	Test_CharacterSelectAuthorizedSelection(runner);
	Test_NetObjectRegistryClassReplacement(runner);
	Test_EntryPurpose(runner);
	Test_ExactCancellation(runner);
	Test_StaleTokenCannotCancelNewCommand(runner);
	Test_InvalidRequestsPreservePendingCommand(runner);
	Test_SkillBindingSchema(runner);
	Test_ActionPresentationTimeline(runner);
	Test_RealSkillBindingDocuments(runner);
	Test_SkillBindingAtomicSave(runner);
	Test_EffectPlaybackDeterminism(runner);
	Test_EffectTypedPresentation(runner);
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
