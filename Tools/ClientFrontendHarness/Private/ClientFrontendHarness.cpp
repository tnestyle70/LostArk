#include "Client_Defines.h"
#include "LobbyCommandService.h"
#include "AnimationSkillBindingDocument.h"
#include "CharacterSelectionState.h"
#include "Effect_DocumentCodec.h"
#include "Effect_Playback.h"
#include "PlayerSkillCatalog.h"
#include "ProjectDataRoot.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
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
			{
				for (const ANIMATION_SKILL_CLIP& clip : binding.Clips)
					available.push_back(clip.strClipName);
			}
			runner.Require(CAnimationSkillBindingDocument::Validate(parsed,
				owner.asset, owner.characterClass, CPlayerSkillCatalog::Get_Skills(),
				available, status),
				"Real Skill Binding Covers Current Class Catalog");
			total += parsed.Bindings.size();
		}
		runner.Require(CPlayerSkillCatalog::Get_Skills().size() == total,
			"Five Real Skill Binding Documents Cover Every Skill Definition");
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
		changed.Bindings[0].Clips = { { "Active_B", 0u, 1.f } };
		runner.Require(!CAnimationSkillBindingDocument::Save_Atomic(changed,
			"LanceMaster", CHARACTER_CLASS_ID::LANCE_MASTER,
			skills, clips, status) && before == Read_Text(destination),
			"Skill Binding Replace Failure Preserves Destination");
		SetFileAttributesW(destination.c_str(), FILE_ATTRIBUTE_NORMAL);
		SetEnvironmentVariableW(L"LOSTARK_PROJECT_DATA_ROOT", nullptr);
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
	}

	void Test_DimensionMasterImportedPortalDraft(TEST_RUNNER& runner)
	{
		using namespace Client;
		const std::filesystem::path path = CProjectDataRoot::Resolve(
			L"Effects/Imported/DimensionMaster/Converted/"
			L"effect.dimensionmaster.skill.2050500.imported.effect.json");
		EFFECT_DOCUMENT_DESC document;
		std::string status;
		const bool_t loaded = !path.empty() &&
			CEffectDocumentCodec::Load(path, document, status) &&
			CEffectDocumentCodec::Validate_Drawable(document, status);
		std::size_t meshParticleCount = 0u;
		for (const EFFECT_ELEMENT_DESC& element : document.Elements)
		{
			if (element.eKind != EFFECT_ELEMENT_KIND::PARTICLE)
				continue;
			meshParticleCount += std::count_if(
				element.ResourceBindings.begin(),
				element.ResourceBindings.end(),
				[](const EFFECT_RESOURCE_BINDING_DESC& binding)
				{
					return binding.strSlotId == "meshModel";
				});
		}
		if (!loaded)
		{
			std::cout << "[DETAIL] DimensionMaster imported portal: "
				<< path.string() << " status=" << status << '\n';
		}
		runner.Require(loaded && document.Elements.size() == 97u &&
			meshParticleCount >= 28u,
			"DimensionMaster F Imported Portal Is Drawable With Mesh Particles");
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
			"Effect Authoring Atomically Saves And Reloads V6 Metadata Draft");
		std::filesystem::remove(path, error);
	}
}

int main()
{
	TEST_RUNNER runner{};

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
	Test_DimensionMasterImportedPortalDraft(runner);
	Test_EffectAllEffectsAuthoringJoin(runner);
	Test_EffectDraftAtomicSave(runner);

	std::cout << "failures : " << runner.iFailureCount << '\n';
	return 0 == runner.iFailureCount ? 0 : 1;
}
