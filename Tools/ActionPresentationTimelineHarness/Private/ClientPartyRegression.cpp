#include "AnimationSkillBindingDocument.h"
#include "ClientReplicationEvent.h"
#include "EncounterPatternReference.h"
#include "MouseButtonReleaseGate.h"
#include "PartyTransferNotice.h"
#include "ReplicatedPlayerHealth.h"
#include "Sound/TrackedSoundChannel.h"
#include "ValtanPatternSoundCueDocument.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>

namespace
{
	bool Require(const bool condition, const char* message)
	{
		if (!condition)
			std::cerr << "ClientPartyRegression: " << message << '\n';
		return condition;
	}

	bool VerifyMousePressOwnership()
	{
		Engine::CMouseButtonReleaseGate left, right;
		if (!Require(right.Observe(true, false), "fresh world RMB was suppressed"))
			return false;
		// A menu opens in Update, before ImGui capture has caught up. Both
		// physical buttons belong to it from that very frame.
		if (!Require(!right.Observe(true, true) && !left.Observe(true, true),
			"popup opening frame leaked RMB movement/LMB attack"))
			return false;
		for (int frame = 0; frame < 10; ++frame)
		{
			if (!Require(!right.Observe(true, false) && !left.Observe(true, false),
				"closing popup rearmed a still-held button"))
				return false;
		}
		if (!Require(!left.Observe(false, false) && left.Observe(true, false) &&
			!right.Observe(true, false), "LMB release incorrectly rearmed RMB"))
			return false;
		if (!Require(!right.Observe(false, false) && right.Observe(true, false),
			"physical RMB release did not permit the next world click"))
			return false;
		// Existing MapTool/UI ownership is additive: a later false contribution
		// must not clear a press another consumer already latched.
		return Require(!left.Observe(true, true) && !left.Observe(true, false),
			"one input consumer cleared another consumer's physical-press ownership");
	}

	struct FakeChannel
	{
		int stops = 0;
		void stop() { ++stops; }
	};

	bool VerifyIndependentLoopedSound()
	{
		Engine::CTrackedSoundChannel<FakeChannel> music, uiLoop;
		FakeChannel bgm, wait, replacement, failedStage;
		const auto start = [](auto& owner, FakeChannel& channel)
		{
			return owner.Try_Replace([&](FakeChannel*& staged) { staged = &channel; return true; });
		};
		if (!Require(start(music, bgm) && start(uiLoop, wait) && 0 == bgm.stops,
			"starting UI wait sound stopped BGM"))
			return false;
		if (!Require(!uiLoop.Try_Replace([](FakeChannel*&) { return false; }) &&
			0 == wait.stops && 0 == bgm.stops, "load failure stopped committed audio"))
			return false;
		if (!Require(!uiLoop.Try_Replace([&](FakeChannel*& staged)
			{ staged = &failedStage; return false; }) &&
			1 == failedStage.stops && 0 == wait.stops && 0 == bgm.stops,
			"volume/unpause failure did not roll back only its staged channel"))
			return false;
		if (!Require(start(uiLoop, replacement) && 1 == wait.stops && 0 == bgm.stops,
			"UI loop replacement touched the music owner"))
			return false;
		uiLoop.Stop(); // result, close, level transition and application teardown
		uiLoop.Stop();
		if (!Require(1 == replacement.stops && 0 == bgm.stops,
			"UI cleanup stopped BGM or stopped its own channel twice"))
			return false;
		music.Stop();
		return Require(1 == bgm.stops, "music owner failed to clean up its channel");
	}

	bool VerifyReplicatedPartyHealth()
	{
		using namespace LostArk::Shared;
		Client::CReplicatedPlayerHealth health;
		S2C_WORLD_SNAPSHOT snapshot{};
		snapshot.iServerTick = 10u;
		PLAYER_SNAPSHOT first{}, second{};
		first.iNetEntityId = 101u; first.iCurrentHp = 25; first.iMaximumHp = 100;
		second.iNetEntityId = 202u; second.iCurrentHp = 0; second.iMaximumHp = 200;
		snapshot.Players = { second, first }; // roster order is deliberately different
		if (!Require(!health.Find(101u).hasSnapshot && health.Apply_Snapshot(snapshot) &&
			health.Find(101u).Get_Ratio() == 0.25f && health.Find(202u).hasSnapshot &&
			health.Find(202u).Get_Ratio() == 0.f && !health.Find(999u).hasSnapshot,
			"HP join fabricated 100%, used row order, or hid real zero HP"))
			return false;
		snapshot.Players[1].iCurrentHp = 90;
		if (!Require(health.Apply_Snapshot(snapshot) && health.Find(101u).Get_Ratio() == 0.25f,
			"duplicate tick replaced current HP"))
			return false;
		snapshot.iServerTick = 9u;
		if (!Require(health.Apply_Snapshot(snapshot) && health.Find(101u).Get_Ratio() == 0.25f,
			"older tick replaced current HP"))
			return false;
		snapshot.iServerTick = 11u;
		snapshot.Players.push_back(first);
		if (!Require(!health.Apply_Snapshot(snapshot) && health.Find(101u).Get_Ratio() == 0.25f,
			"duplicate entity partially committed HP"))
			return false;
		snapshot.Players.pop_back();
		snapshot.Players[1].iMaximumHp = 0;
		if (!Require(!health.Apply_Snapshot(snapshot) && health.Find(101u).Get_Ratio() == 0.25f,
			"invalid HP partially committed the snapshot"))
			return false;
		snapshot.Players = { second };
		if (!Require(health.Apply_Snapshot(snapshot) && !health.Find(101u).hasSnapshot,
			"out-of-world member retained a stale HP bar"))
			return false;
		health.Erase(202u);
		if (!Require(!health.Find(202u).hasSnapshot, "despawn retained HP"))
			return false;
		health.Reset();
		snapshot.iServerTick = 1u;
		if (!Require(health.Apply_Snapshot(snapshot) && health.Find(202u).hasSnapshot,
			"new-world tick origin was rejected after reset"))
			return false;
		health.Reset();
		return Require(!health.Find(202u).hasSnapshot, "disconnect reset retained party HP");
	}

	std::string ReadText(const std::filesystem::path& path)
	{
		std::ifstream stream(path, std::ios::binary);
		return { std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };
	}

	bool VerifyEncounterTrashActionAdmission(const std::filesystem::path& root)
	{
		using namespace Client;
		const auto sourcePath = root / "Data/Encounters/Valtan/ValtanEncounter.json";
		const std::string original = ReadText(sourcePath);
		CEncounterPatternReference reference;
		std::string status;
		const bool loaded = reference.Load(sourcePath, status);
		if (!Require(loaded, status.c_str()))
			return false;
		const auto* committedPatterns = reference.Get_Patterns().data();
		const auto committedCount = reference.Get_Patterns().size();
		const auto committedEncounter = reference.Get_EncounterId();
		const auto committedBoss = reference.Get_BossArchetypeId();
		const auto committedTickHz = reference.Get_FixedTickHz();
		const auto* committedTrash = reference.Find_Pattern("VALTAN_TRASH");
		if (!Require(nullptr != committedTrash, "latest encounter did not load Trash"))
			return false;
		const auto committedDuration = committedTrash->iTotalDurationMs;
		struct ScopedEncounterFixture
		{
			std::filesystem::path Directory, File;
			bool ownsDirectory = false;
			~ScopedEncounterFixture()
			{
				if (!ownsDirectory) return;
				std::error_code ignored;
				std::filesystem::remove(File, ignored);
				std::filesystem::remove(Directory, ignored);
			}
		} fixture;
		std::error_code fileError;
		fixture.Directory = std::filesystem::temp_directory_path(fileError) /
			("LostArkEncounterReference-" + std::to_string(GetCurrentProcessId()) +
				"-" + std::to_string(GetTickCount64()));
		fixture.File = fixture.Directory / "Encounter.json";
		fixture.ownsDirectory = !fileError &&
			std::filesystem::create_directory(fixture.Directory, fileError);
		if (!Require(fixture.ownsDirectory && !fileError,
			"encounter reference fixture directory could not be created"))
			return false;
		const auto writeFixture = [&](const std::string& text)
		{
			std::ofstream output(fixture.File, std::ios::binary | std::ios::trunc);
			output << text;
			output.flush();
			return output.good();
		};
		size_t rejectionCount = 0u;
		const auto rejectedWithoutCommit = [&](const std::string& text, const char* label,
			const char* errorPrefix = "Encounter stage v4 field is invalid:")
		{
			if (!Require(writeFixture(text), "encounter fixture write failed"))
				return false;
			const bool rejected = !reference.Load(fixture.File, status);
			const bool preserved = reference.Is_Ready() &&
				reference.Get_Patterns().data() == committedPatterns &&
				reference.Get_Patterns().size() == committedCount &&
				reference.Get_EncounterId() == committedEncounter &&
				reference.Get_BossArchetypeId() == committedBoss &&
				reference.Get_FixedTickHz() == committedTickHz &&
				reference.Find_Pattern("VALTAN_TRASH") == committedTrash &&
				committedTrash->iTotalDurationMs == committedDuration;
			if (!Require(rejected && preserved && status.find(errorPrefix) == 0u,
				(std::string("encounter reference accepted or partially committed ") +
					label + ": " + status).c_str()))
				return false;
			++rejectionCount;
			return true;
		};
		const auto actionText = [](const std::string& kind, const std::string& trigger,
			const std::string& target, const uint32_t value, const uint32_t duration)
		{
			return "{\"trigger\":\"" + trigger + "\",\"kind\":\"" + kind +
				"\",\"targetId\":\"" + target + "\",\"value\":" +
				std::to_string(value) + ",\"durationMs\":" + std::to_string(duration) + "}";
		};
		for (const auto& [kind, target] : std::array<std::pair<std::string, std::string>, 2>{
			std::pair{ "DAMAGE_GRABBED_PLAYERS", "damage.valtan.charge-grab-roar" },
			std::pair{ "EXECUTE_GRABBED_PLAYERS", "boss.attachment.left-hand" } })
		{
			const auto kindAt = original.find('"' + kind + '"');
			const auto begin = original.rfind('{', kindAt);
			const auto end = original.find('}', kindAt);
			if (!Require(kindAt != std::string::npos && begin != std::string::npos &&
				end != std::string::npos, "latest encounter lost a typed grabbed-player action"))
				return false;
			const auto replaceAction = [&](const std::string& action)
			{
				auto text = original;
				text.replace(begin, end - begin + 1u, action);
				return text;
			};
			CEncounterPatternReference validFixture;
			if (!Require(writeFixture(replaceAction(actionText(kind, "ENTER", target, 0u, 0u))) &&
				validFixture.Load(fixture.File, status), "valid typed action fixture was rejected"))
				return false;
			if (!rejectedWithoutCommit(replaceAction(actionText(kind, "EXIT", target, 0u, 0u)), "typed impact EXIT") ||
				!rejectedWithoutCommit(replaceAction(actionText(kind, "ENTER", "boss.attachment.right-hand", 0u, 0u)), "typed impact target") ||
				!rejectedWithoutCommit(replaceAction(actionText(kind, "ENTER", target, 1u, 0u)), "typed impact value") ||
				!rejectedWithoutCommit(replaceAction(actionText(kind, "ENTER", target, 0u, 1u)), "typed impact duration") ||
				!rejectedWithoutCommit(replaceAction(actionText("UNKNOWN_GRABBED_ACTION", "ENTER", target, 0u, 0u)), "unknown typed impact") ||
				!rejectedWithoutCommit(replaceAction(actionText(kind, "ENTER", target, 0u, 0u) + ',' +
					actionText("RETARGET_RANDOM_ALIVE", "ENTER", "boss.target.pattern", 1u, 0u)), "shared typed impact transaction"))
				return false;
			const auto stageAt = original.rfind("\"stageId\"", begin);
			const auto hitAt = original.find("\"hitShape\"", stageAt);
			const auto noneAt = original.find("\"NONE\"", hitAt);
			if (!Require(stageAt != std::string::npos && hitAt < begin && noneAt < begin,
				"typed impact fixture does not have its own NONE hit stage"))
				return false;
			auto hitConflict = original;
			hitConflict.replace(noneAt, 6u, "\"BOX\"");
			if (!rejectedWithoutCommit(hitConflict, "typed impact with ordinary hit shape"))
				return false;
		}
		const auto branchAt = original.find("\"ANY_PLAYER_GRABBED\"");
		if (!Require(branchAt != std::string::npos, "latest Trash has no ANY_PLAYER_GRABBED branch"))
			return false;
		auto unknownBranch = original;
		unknownBranch.replace(branchAt, std::string("\"ANY_PLAYER_GRABBED\"").size(),
			"\"UNKNOWN_PLAYER_GRABBED\"");
		if (!rejectedWithoutCommit(unknownBranch, "unknown grabbed-player branch"))
			return false;
		if (!Require(nullptr != reference.Find_Pattern("VALTAN_GHOST_FINALE") &&
			nullptr != reference.Find_Pattern("VALTAN_WARP") &&
			nullptr != reference.Find_Pattern("VALTAN_FIST_IN_OUT"),
			"latest encounter lost the finale, portal, or independent donut owner"))
			return false;
		struct FieldMutation
		{
			const char* pattern;
			const char* scope;
			const char* field;
			const char* replacement;
			const char* errorPrefix = "Encounter stage v4 field is invalid:";
		};
		const auto replaceField = [](std::string text, const FieldMutation& mutation)
		{
			const std::string patternId = '"' + std::string(mutation.pattern) + '"';
			size_t cursor = text.find("\"patterns\"");
			size_t patternAt = std::string::npos;
			while (cursor != std::string::npos)
			{
				cursor = text.find("\"patternId\"", cursor);
				if (cursor == std::string::npos) break;
				const auto colon = text.find(':', cursor);
				const auto value = text.find_first_not_of(" \t\r\n", colon + 1u);
				if (value != std::string::npos && text.compare(value, patternId.size(), patternId) == 0)
				{
					patternAt = value;
					break;
				}
				++cursor;
			}
			if (patternAt == std::string::npos) return std::string{};
			const auto nextPatternAt = text.find("\"patternId\"", patternAt + patternId.size());
			cursor = patternAt;
			const std::string scope = mutation.scope;
			if (!scope.empty())
			{
				if (scope != "finale" && scope != "serverMotion")
					cursor = text.find("\"stages\"", cursor);
				cursor = text.find('"' + scope + '"', cursor);
			}
			if (cursor == std::string::npos || cursor >= nextPatternAt) return std::string{};
			const auto fieldAt = text.find('"' + std::string(mutation.field) + '"', cursor);
			if (fieldAt == std::string::npos || fieldAt >= nextPatternAt) return std::string{};
			const auto colon = text.find(':', fieldAt);
			const auto begin = text.find_first_not_of(" \t\r\n", colon + 1u);
			if (begin == std::string::npos || begin >= nextPatternAt) return std::string{};
			size_t end = text.find_first_of(",}\r\n", begin);
			if (text[begin] == '[') end = text.find(']', begin) + 1u;
			else if (text[begin] == '"') end = text.find('"', begin + 1u) + 1u;
			if (end == std::string::npos || end <= begin || end > nextPatternAt)
				return std::string{};
			text.replace(begin, end - begin, mutation.replacement);
			return text;
		};
		const char* extensionError = "Encounter pattern extensions are invalid:";
		const FieldMutation malformedFields[] =
		{
			{ "VALTAN_GHOST_FINALE", "finale", "kind", "\"UNKNOWN_FINALE\"", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "ghostArchetypeId", "\"BOSS_VALTAN\"", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "maximumActiveGhosts", "0", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "maximumActiveGhosts", "2", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "maximumActiveGhosts", "true", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "maximumActiveGhosts", "1,\"unsupported\":1", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "spawnHalfExtentsM", "[0,10]", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "spawnHalfExtentsM", "[10,101]", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "spawnHalfExtentsM", "[10]", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "ghostPatternIds",
				"[\"VALTAN_FOUR_SLASH\",\"VALTAN_WHIRLWIND\",\"VALTAN_SEQUENCE_FOUR\"]", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "ghostPatternIds",
				"[\"VALTAN_WHIRLWIND\",\"VALTAN_WHIRLWIND\",\"VALTAN_SEQUENCE_FOUR\"]", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "ghostPatternIds",
				"[\"VALTAN_WHIRLWIND\",\"VALTAN_FOUR_SLASH\",\"VALTAN_GHOST_FINALE\"]", extensionError },
			{ "VALTAN_GHOST_FINALE", "", "invulnerableWhileRunning", "true", extensionError },
			{ "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK", "serverMotion", "moveToAnchorBeforeTakeoff", "\"true\"", extensionError },
			{ "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK", "serverMotion", "takeoffStartMs", "0", extensionError },
			{ "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK", "serverMotion", "travelStageId", "\"MISSING\"", extensionError },
			{ "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK", "serverMotion", "landingPosition", "[0,0,100001]", extensionError },
			{ "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK", "serverMotion", "apexHeight", "0", extensionError },
			{ "VALTAN_TERRAIN_DESTRUCTION_3_OCLOCK", "serverMotion", "moveToAnchorBeforeTakeoff", "true,\"unsupported\":1", extensionError },
			{ "VALTAN_GHOST_FINALE", "STEP_02", "cornerIndex", "4" },
			{ "VALTAN_GHOST_FINALE", "STEP_02", "cornerIndex", "-1" },
			{ "VALTAN_GHOST_FINALE", "STEP_02", "cornerIndex", "true" },
			{ "VALTAN_GHOST_FINALE", "STEP_02", "cornerIndex", "1.5" },
			{ "VALTAN_GHOST_FINALE", "STEP_02", "halfExtentsM", "[0,22]" },
			{ "VALTAN_GHOST_FINALE", "STEP_02", "halfExtentsM", "[22,101]" },
			{ "VALTAN_GHOST_FINALE", "STEP_02", "halfExtentsM", "[22]" },
			{ "VALTAN_WARP", "STEP_02", "kind", "\"UNKNOWN_MOTION\"" },
			{ "VALTAN_WARP", "STEP_02", "kind",
				"\"PORTAL_TARGET_RUSH\",\"cornerIndex\":0" },
			{ "VALTAN_WARP", "STEP_02", "kind",
				"\"PORTAL_TARGET_RUSH\",\"halfExtentsM\":[22,22]" },
			{ "VALTAN_CATCH_BREATH", "RELEASE_GRABBED_PLAYERS", "releaseMode", "\"UNKNOWN_EJECTION\"" },
			{ "VALTAN_CATCH_BREATH", "RELEASE_GRABBED_PLAYERS", "speedMps", "0" },
			{ "VALTAN_CATCH_BREATH", "RELEASE_GRABBED_PLAYERS", "durationMs", "0" },
			{ "VALTAN_FIST_IN_OUT", "SPAWN_COMBAT_OBJECT", "value", "2" },
			{ "VALTAN_TRASH_CATCH_IF", "STEP_07", "outcome", "\"NAVIGATION_BLOCKED\"" },
			{ "VALTAN_TRASH", "GROGGY", "nextActionId", "\"valtan.sequence.center-trash-rush-if.step-07\"", extensionError },
			{ "VALTAN_TRASH_CATCH_IF", "GROGGY", "nextActionId", "\"valtan.sequence.rush-if.step-07\"", extensionError },
			{ "VALTAN_TRASH_CATCH_SUCCESS", "EXECUTE_TAIL", "nextActionId", "\"valtan.sequence.rush-success.catch-pre-impact\"", extensionError },
			{ "VALTAN_TRASH_CATCH_FAIL", "RUSH_MISS", "nextActionId", "\"valtan.sequence.rush-fail.rush-miss\"" },
			{ "VALTAN_GHOST_FINALE", "STEP_10", "durationMs",
				"1667,\"branches\":[{\"outcome\":\"TIMEOUT\",\"nextActionId\":\"valtan.sequence.ghost-finale.step-01\"}]", extensionError },
			{ "VALTAN_WHIRLWIND", "RECOVERY", "durationMs",
				"1467,\"branches\":[{\"outcome\":\"TIMEOUT\",\"nextActionId\":\"valtan.attack.whirlwind.windup\"}]", extensionError },
			{ "VALTAN_FOUR_SLASH", "RECOVERY", "durationMs",
				"800,\"branches\":[{\"outcome\":\"TIMEOUT\",\"nextActionId\":\"valtan.attack.four-slash.windup\"}]", extensionError },
			{ "VALTAN_SEQUENCE_FOUR", "STEP_01", "durationMs",
				"5000,\"branches\":[{\"outcome\":\"TIMEOUT\",\"nextActionId\":\"valtan.sequence.four.step-01\"}]" }
		};
		for (const auto& mutation : malformedFields)
		{
			const auto text = replaceField(original, mutation);
			const std::string label = std::string(mutation.pattern) + "/" + mutation.field;
			if (!Require(!text.empty(), ("encounter mutation field not found: " + label).c_str()) ||
				!rejectedWithoutCommit(text, label.c_str(), mutation.errorPrefix))
				return false;
		}
		// TIMEOUT null ends the entry stage. Cycles in the now-unreachable
		// tail still cannot be admitted as a finite ghost animation graph.
		auto unreachableCycle = original;
		for (const auto& mutation : std::array<FieldMutation, 3>{
			FieldMutation{ "VALTAN_WHIRLWIND", "WINDUP", "durationMs",
				"1333,\"branches\":[{\"outcome\":\"TIMEOUT\",\"nextActionId\":null}]" },
			FieldMutation{ "VALTAN_WHIRLWIND", "SPIN", "durationMs",
				"1200,\"branches\":[{\"outcome\":\"TIMEOUT\",\"nextActionId\":\"valtan.attack.whirlwind.recovery\"}]" },
			FieldMutation{ "VALTAN_WHIRLWIND", "RECOVERY", "durationMs",
				"1467,\"branches\":[{\"outcome\":\"TIMEOUT\",\"nextActionId\":\"valtan.attack.whirlwind.active\"}]" } })
		{
			unreachableCycle = replaceField(std::move(unreachableCycle), mutation);
			if (!Require(!unreachableCycle.empty(), "unreachable-cycle fixture was not staged"))
				return false;
		}
		if (!rejectedWithoutCommit(unreachableCycle, "unreachable ghost tail cycle", extensionError))
			return false;
		const auto returnAt = original.find("\"RETURN_TO_ARENA_CENTER\"");
		const auto returnBegin = original.rfind('{', returnAt);
		const auto returnEnd = original.find('}', returnAt);
		if (!Require(returnAt != std::string::npos && returnBegin != std::string::npos &&
			returnEnd != std::string::npos, "portal recovery has no typed arena-center action"))
			return false;
		for (const auto& action : std::array<std::string, 4>{
			actionText("RETURN_TO_ARENA_CENTER", "EXIT", "boss.arena.center", 0u, 0u),
			actionText("RETURN_TO_ARENA_CENTER", "ENTER", "boss.target.pattern", 1u, 0u),
			actionText("RETURN_TO_ARENA_CENTER", "ENTER", "boss.arena.center", 2u, 0u),
			actionText("RETURN_TO_ARENA_CENTER", "ENTER", "boss.arena.center", 1u, 1u) })
		{
			auto text = original;
			text.replace(returnBegin, returnEnd - returnBegin + 1u, action);
			if (!rejectedWithoutCommit(text, "invalid arena-center recovery action"))
				return false;
		}
		std::cout << "EncounterPatternReference: latest Trash/portal/finale and " <<
			rejectionCount << " rejection/rollback cases PASS\n";
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
		const bool loaded = encounter.Load(root / "Data/Encounters/Valtan/ValtanEncounter.json", status) &&
			CValtanPatternAnimationBindingDocument::Parse_Text(
				ReadText(authored / "Valtan.patternbindings.json"), animation, status) &&
			CValtanPatternSoundCueDocument::Parse_Text(
				ReadText(authored / "Valtan.patternsoundcues.json"), encounter, animation, document, status);
		if (!Require(loaded, status.c_str()))
			return false;
		if (!Require(511u == document.Cues.size() &&
			status.find("0 explicitly suppressed-animation") != std::string::npos &&
			status.find("0 not-yet-implemented-pattern") != std::string::npos,
			"real 511-row sound document did not admit every cue with zero NONE/unimplemented rows"))
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

	bool VerifyPartyTransferNotice()
	{
		using namespace LostArk::Shared;
		for (const auto result : { PARTY_TRANSFER_RESULT::REJECTED_NOT_LEADER,
			PARTY_TRANSFER_RESULT::REJECTED_ROOM_FULL, PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE,
			PARTY_TRANSFER_RESULT::REJECTED_ADMISSION_FAILED, PARTY_TRANSFER_RESULT::REJECTED_OUTBOUND_BUSY })
		{
			if (!Require(nullptr != Client::Get_PartyTransferFailureText(result),
				"server transfer failure has no product notice"))
				return false;
		}
		using Event = Client::CLIENT_REPLICATION_EVENT_TYPE;
		return Require(nullptr == Client::Get_PartyTransferFailureText(static_cast<PARTY_TRANSFER_RESULT>(255)) &&
			!Client::Can_CoalesceAdjacentReplicationEvents(Event::WORLD_SNAPSHOT, Event::PARTY_TRANSFER_RESULT) &&
			!Client::Can_CoalesceAdjacentReplicationEvents(Event::PARTY_TRANSFER_RESULT, Event::WORLD_SNAPSHOT),
			"unknown transfer result was normalized or reliable notice lost its ordering barrier");
	}
}

bool VerifyClientPartyRegression(const std::filesystem::path& root)
{
	return VerifyMousePressOwnership() && VerifyIndependentLoopedSound() &&
		VerifyReplicatedPartyHealth() && VerifyEncounterTrashActionAdmission(root) &&
		VerifySoundCueIsolation(root) && VerifyPartyTransferNotice();
}
