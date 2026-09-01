#include <Windows.h>

#include "EncounterPatternReference.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>

namespace
{
	bool Require(const bool condition, const char* message)
	{
		if (!condition)
			std::cerr << "ValtanEncounterReferenceContracts: " << message << '\n';
		return condition;
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
		const auto* committedPizza =
			reference.Find_Pattern("VALTAN_SIX_PIZZA_106");
		if (!Require(nullptr != committedPizza &&
				committedPizza->targetPolicy ==
					"LOCK_RANDOM_ALIVE_ON_START" &&
				committedPizza->aimPolicy == "TRACK_TARGET_EACH_TICK" &&
				committedPizza->serverMotion.has_value() &&
				committedPizza->serverMotion->kind == "LEAP_TO_ANCHOR" &&
				committedPizza->serverMotion->bMoveToAnchorBeforeTakeoff &&
				committedPizza->serverMotion->anchorId ==
					"anchor.valtan.six-pizza-106.landing" &&
				std::all_of(
					committedPizza->serverMotion->landingPosition.begin(),
					committedPizza->serverMotion->landingPosition.end(),
					[](const f32_t value) { return std::isfinite(value); }),
			"encounter Product did not retain six-pizza target/motion authority"))
		{
			return false;
		}
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
		const auto invalidGameplayPhase = replaceField(original,
			{ "VALTAN_GHOST_RESPAWN_AUDITION", "STEP_01", "value", "4" });
		if (!Require(!invalidGameplayPhase.empty(),
				"ghost-respawn gameplay phase fixture was not staged") ||
			!rejectedWithoutCommit(invalidGameplayPhase,
				"ghost-respawn gameplay phase 4"))
		{
			return false;
		}
		auto dynamicFinale = replaceField(original,
			{ "VALTAN_GHOST_FINALE", "finale", "maximumActiveGhosts", "2" });
		dynamicFinale = replaceField(std::move(dynamicFinale),
			{ "VALTAN_GHOST_FINALE", "finale", "ghostPatternIds",
			  "[\"VALTAN_FOUR_SLASH\",\"VALTAN_WHIRLWIND\"]" });
		CEncounterPatternReference dynamicFinaleReference;
		if (!Require(!dynamicFinale.empty() && writeFixture(dynamicFinale) &&
			dynamicFinaleReference.Load(fixture.File, status) &&
			nullptr != dynamicFinaleReference.Find_Pattern("VALTAN_GHOST_FINALE"),
			"data-driven two-child reordered finale was rejected"))
			return false;
		const char* extensionError = "Encounter pattern extensions are invalid:";
		const FieldMutation malformedFields[] =
		{
			{ "VALTAN_GHOST_FINALE", "finale", "kind", "\"UNKNOWN_FINALE\"", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "ghostArchetypeId", "\"BOSS_VALTAN\"", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "maximumActiveGhosts", "0", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "maximumActiveGhosts", "65", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "maximumActiveGhosts", "true", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "maximumActiveGhosts", "1,\"unsupported\":1", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "spawnHalfExtentsM", "[0,10]", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "spawnHalfExtentsM", "[10,101]", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "spawnHalfExtentsM", "[10]", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "ghostPatternIds",
				"[]", extensionError },
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
			{ "VALTAN_WARP", "STEP_02", "retargetDelayMs", "2301" },
			{ "VALTAN_WARP", "STEP_02", "retargetDelayMs", "true" },
			{ "VALTAN_WARP", "STEP_02", "speedMps", "0" },
			{ "VALTAN_WARP", "STEP_02", "speedMps", "1001" },
			{ "VALTAN_WARP", "STEP_02", "distanceM", "0" },
			{ "VALTAN_WARP", "STEP_02", "distanceM", "1001" },
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
			{ "VALTAN_SIX_PIZZA_106", "STEP_11", "durationMs",
				"1300,\"branches\":[{\"outcome\":\"TIMEOUT\",\"nextActionId\":\"valtan.sequence.center-six-pizza-charge.step-01\"}]", extensionError }
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
			FieldMutation{ "VALTAN_SIX_PIZZA_106", "STEP_01", "durationMs",
				"1200,\"branches\":[{\"outcome\":\"TIMEOUT\",\"nextActionId\":null}]" },
			FieldMutation{ "VALTAN_SIX_PIZZA_106", "STEP_02", "durationMs",
				"1000,\"branches\":[{\"outcome\":\"TIMEOUT\",\"nextActionId\":\"valtan.sequence.center-six-pizza-charge.step-03\"}]" },
			FieldMutation{ "VALTAN_SIX_PIZZA_106", "STEP_03", "durationMs",
				"1200,\"branches\":[{\"outcome\":\"TIMEOUT\",\"nextActionId\":\"valtan.sequence.center-six-pizza-charge.step-02\"}]" } })
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
}

int Run_ValtanEncounterReferenceContractTests()
{
	if (!VerifyEncounterTrashActionAdmission(std::filesystem::current_path()))
		return 1;
	std::cout << "Valtan encounter reference contracts: PASS\n";
	return 0;
}
