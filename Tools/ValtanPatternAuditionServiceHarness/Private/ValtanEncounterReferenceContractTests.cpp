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
		std::size_t committedGripStageCount = 0u;
		for (const char* const patternId : {
			"VALTAN_TRASH", "VALTAN_TRASH_CATCH_IF", "VALTAN_CATCH_BREATH" })
		{
			const auto* capturePattern = reference.Find_Pattern(patternId);
			if (!Require(nullptr != capturePattern,
				"encounter Product lost a left-hand capture Pattern"))
				return false;
			for (const auto& stage : capturePattern->stages)
			{
				if (!stage.gripLocalOffset.has_value())
					continue;
				++committedGripStageCount;
				if (!Require(std::abs(stage.gripLocalOffset->fForwardM) <= 1.e-6f &&
					std::abs(stage.gripLocalOffset->fUpM + 0.9f) <= 1.e-6f &&
					std::abs(stage.gripLocalOffset->fRightM) <= 1.e-6f,
					"encounter Product changed the authored wrist-local correction"))
					return false;
			}
		}
		if (!Require(7u == committedGripStageCount,
			"encounter Product did not retain all seven stable capture actions"))
			return false;
		const auto* committedStagger =
			reference.Find_Pattern("VALTAN_STAGGER_SLOT");
		const ENCOUNTER_STAGE_REFERENCE* committedStaggerChannel = nullptr;
		if (nullptr != committedStagger)
		{
			const auto channel = std::find_if(committedStagger->stages.begin(),
				committedStagger->stages.end(),
				[](const ENCOUNTER_STAGE_REFERENCE& stage)
				{ return "CHANNEL" == stage.stageId; });
			if (channel != committedStagger->stages.end())
				committedStaggerChannel = &(*channel);
		}
		if (!Require(nullptr != committedStagger &&
			nullptr != committedStaggerChannel &&
			std::abs(committedStaggerChannel->fVerticalOffsetM - 0.5f) <= 1.e-6f,
			"encounter Product did not retain the Stage-owned magic-orb offset"))
		{
			return false;
		}
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
		const char* stageMotionError =
			"Encounter stage v4 motion is invalid:";
		const char* stageActionsError =
			"Encounter stage v4 actions are invalid:";
		const char* stageBranchesError =
			"Encounter stage v4 branches are invalid:";
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
		const auto firstGripKey = original.find("\"gripLocalOffset\"");
		const auto firstGripObject = original.find('{', firstGripKey);
		const auto firstGripEnd = original.find('}', firstGripObject);
		const auto firstGripComma = original.rfind(',', firstGripKey);
		if (!Require(firstGripKey != std::string::npos &&
			firstGripObject != std::string::npos &&
			firstGripEnd != std::string::npos &&
			firstGripComma != std::string::npos,
			"encounter Product has no gripLocalOffset fixture"))
			return false;
		auto missingGrip = original;
		missingGrip.erase(firstGripComma, firstGripEnd - firstGripComma + 1u);
		if (!rejectedWithoutCommit(missingGrip, "missing capture gripLocalOffset",
			"Encounter capture hit fields are incomplete:"))
			return false;
		const auto firstGripUp = original.find("\"upM\"", firstGripObject);
		const auto firstGripUpColon = original.find(':', firstGripUp);
		const auto firstGripUpValue = original.find_first_not_of(
			" \t\r\n", firstGripUpColon + 1u);
		const auto firstGripUpEnd = original.find_first_of(
			",}\r\n", firstGripUpValue);
		if (!Require(firstGripUp != std::string::npos &&
			firstGripUpColon != std::string::npos &&
			firstGripUpValue != std::string::npos &&
			firstGripUpEnd != std::string::npos,
			"encounter Product grip up component could not be located"))
			return false;
		auto invalidGrip = original;
		invalidGrip.replace(firstGripUpValue,
			firstGripUpEnd - firstGripUpValue, "10.01");
		if (!rejectedWithoutCommit(invalidGrip, "out-of-range capture gripLocalOffset",
			"Encounter capture hit contract is invalid:"))
			return false;
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
			if (!rejectedWithoutCommit(replaceAction(actionText(kind, "EXIT", target, 0u, 0u)), "typed impact EXIT", stageActionsError) ||
				!rejectedWithoutCommit(replaceAction(actionText(kind, "ENTER", "boss.attachment.right-hand", 0u, 0u)), "typed impact target", stageActionsError) ||
				!rejectedWithoutCommit(replaceAction(actionText(kind, "ENTER", target, 1u, 0u)), "typed impact value", stageActionsError) ||
				!rejectedWithoutCommit(replaceAction(actionText(kind, "ENTER", target, 0u, 1u)), "typed impact duration", stageActionsError) ||
				!rejectedWithoutCommit(replaceAction(actionText("UNKNOWN_GRABBED_ACTION", "ENTER", target, 0u, 0u)), "unknown typed impact", stageActionsError) ||
				!rejectedWithoutCommit(replaceAction(actionText(kind, "ENTER", target, 0u, 0u) + ',' +
					actionText("RETARGET_RANDOM_ALIVE", "ENTER", "boss.target.pattern", 1u, 0u)), "shared typed impact transaction", stageActionsError))
				return false;
			const auto stageAt = original.rfind("\"stageId\"", begin);
			const auto hitAt = original.find("\"hitShape\"", stageAt);
			const auto noneAt = original.find("\"NONE\"", hitAt);
			if (!Require(stageAt != std::string::npos && hitAt < begin && noneAt < begin,
				"typed impact fixture does not have its own NONE hit stage"))
				return false;
			auto hitConflict = original;
			hitConflict.replace(noneAt, 6u, "\"BOX\"");
			if (!rejectedWithoutCommit(hitConflict, "typed impact with ordinary hit shape",
				stageActionsError))
				return false;
		}
		const auto branchAt = original.find("\"ANY_PLAYER_GRABBED\"");
		if (!Require(branchAt != std::string::npos, "latest Trash has no ANY_PLAYER_GRABBED branch"))
			return false;
		auto unknownBranch = original;
		unknownBranch.replace(branchAt, std::string("\"ANY_PLAYER_GRABBED\"").size(),
			"\"UNKNOWN_PLAYER_GRABBED\"");
		if (!rejectedWithoutCommit(unknownBranch, "unknown grabbed-player branch",
			stageBranchesError))
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
			if (mutation.replacement == nullptr)
			{
				const auto separator = text.find_last_not_of(" \t\r\n", fieldAt - 1u);
				if (separator == std::string::npos || text[separator] != ',')
					return std::string{};
				text.erase(separator, end - separator);
			}
			else
				text.replace(begin, end - begin, mutation.replacement);
			return text;
		};
		const auto invalidGameplayPhase = replaceField(original,
			{ "VALTAN_GHOST_RESPAWN_AUDITION", "STEP_01", "value", "4" });
		if (!Require(!invalidGameplayPhase.empty(),
				"ghost-respawn gameplay phase fixture was not staged") ||
			!rejectedWithoutCommit(invalidGameplayPhase,
				"ghost-respawn gameplay phase 4", stageActionsError))
		{
			return false;
		}
		auto dynamicFinale = replaceField(original,
			{ "VALTAN_GHOST_FINALE", "finale", "maximumActiveGhosts", "2" });
		dynamicFinale = replaceField(std::move(dynamicFinale),
			{ "VALTAN_GHOST_FINALE", "finale", "ghostPatternIds",
			  "[\"VALTAN_FOUR_SLASH\",\"VALTAN_WHIRLWIND\"]" });
		const std::string canonicalFinaleId = "\"VALTAN_GHOST_FINALE\"";
		const std::string genericFinaleId = "\"VALTAN_FIXTURE_DYNAMIC_FINALE\"";
		for (size_t at = dynamicFinale.find(canonicalFinaleId); at != std::string::npos;
			at = dynamicFinale.find(canonicalFinaleId, at + genericFinaleId.size()))
			dynamicFinale.replace(at, canonicalFinaleId.size(), genericFinaleId);
		CEncounterPatternReference dynamicFinaleReference;
		if (!Require(!dynamicFinale.empty() && writeFixture(dynamicFinale) &&
			dynamicFinaleReference.Load(fixture.File, status) &&
			nullptr != dynamicFinaleReference.Find_Pattern("VALTAN_FIXTURE_DYNAMIC_FINALE"),
			"data-driven two-child reordered finale was rejected"))
			return false;
		const char* extensionError = "Encounter pattern extensions are invalid:";
		// Exercise the actual whole-Encounter entry reader, including legacy omission.
		const auto setFinaleInterval = [](std::string text, const char* key, const char* value)
		{
			const auto finaleAt = text.find("\"finale\"");
			const auto begin = text.find('{', finaleAt);
			const auto end = text.find('}', begin);
			if (finaleAt == std::string::npos || begin == std::string::npos ||
				end == std::string::npos) return std::string{};
			const std::string token = '"' + std::string(key) + '"';
			const auto keyAt = text.find(token, begin);
			if (keyAt == std::string::npos || keyAt >= end)
			{
				if (nullptr != value) text.insert(end, "," + token + ":" + value);
				return text;
			}
			const auto colon = text.find(':', keyAt);
			const auto valueAt = text.find_first_not_of(" \t\r\n", colon + 1u);
			const auto valueEnd = text.find_first_of(",}\r\n", valueAt);
			if (colon >= end || valueAt >= end || valueEnd == std::string::npos)
				return std::string{};
			if (nullptr != value) text.replace(valueAt, valueEnd - valueAt, value);
			else
			{
				const auto commaBefore = text.rfind(',', keyAt);
				if (commaBefore == std::string::npos || commaBefore < begin)
					return std::string{};
				text.erase(commaBefore, valueEnd - commaBefore);
			}
			return text;
		};
		auto legacyIntervals = setFinaleInterval(original, "auxiliarySpawnIntervalMs", nullptr);
		legacyIntervals = setFinaleInterval(std::move(legacyIntervals), "portalSpawnIntervalMs", nullptr);
		if (!Require(!legacyIntervals.empty(), "legacy finale interval fixture was not staged"))
			return false;
		size_t intervalAdmissionCount = 0u;
		for (const auto& intervals : std::array<std::array<const char*, 2u>, 5u>{
			std::array<const char*, 2u>{ nullptr, nullptr },
			std::array<const char*, 2u>{ "5000", nullptr },
			std::array<const char*, 2u>{ nullptr, "10000" },
			std::array<const char*, 2u>{ "1", "600000" },
			std::array<const char*, 2u>{ "5000", "10000" } })
		{
			auto text = setFinaleInterval(legacyIntervals, "auxiliarySpawnIntervalMs", intervals[0]);
			text = setFinaleInterval(std::move(text), "portalSpawnIntervalMs", intervals[1]);
			CEncounterPatternReference accepted;
			if (!Require(!text.empty() && writeFixture(text) && accepted.Load(fixture.File, status) &&
				accepted.Get_Patterns().size() == committedCount &&
				nullptr != accepted.Find_Pattern("VALTAN_GHOST_FINALE"),
				"whole-Encounter optional finale interval admission failed"))
				return false;
			++intervalAdmissionCount;
		}
		for (const char* key : { "auxiliarySpawnIntervalMs", "portalSpawnIntervalMs" })
		{
			for (const char* invalid : { "0", "-1", "600001", "1.5", "null", "true", "\"5000\"" })
			{
				const auto text = setFinaleInterval(legacyIntervals, key, invalid);
				const auto label = std::string("finale interval ") + key + "=" + invalid;
				if (!Require(!text.empty(), "invalid finale interval fixture was not staged") ||
					!rejectedWithoutCommit(text, label.c_str(), extensionError))
					return false;
			}
		}
		const auto unknownInterval = setFinaleInterval(legacyIntervals, "spawnIntervalMs", "5000");
		if (!rejectedWithoutCommit(unknownInterval, "unknown finale interval property", extensionError))
			return false;
		const auto legacySixPool = replaceField(legacyIntervals,
			{ "VALTAN_GHOST_FINALE", "finale", "ghostPatternIds",
			  "[\"VALTAN_WHIRLWIND\",\"VALTAN_FOUR_SLASH\",\"VALTAN_SEQUENCE_FOUR\",\"VALTAN_CROSS\",\"VALTAN_CHARGE\",\"VALTAN_CHARGE_2\"]" });
		CEncounterPatternReference legacySixReference;
		if (!Require(!legacySixPool.empty() && writeFixture(legacySixPool) &&
			legacySixReference.Load(fixture.File, status), "legacy six-child canonical finale was rejected"))
			return false;
		// The current death stage transfers to ghost respawn. Keep the older
		// terminal suppression admission checks on an explicit compatibility fixture.
		auto legacyGhostDeath = replaceField(original,
			{ "VALTAN_GHOST_DEATH_AUDITION", "STEP_01", "branches", nullptr });
		const auto legacySuppression = std::string("23000,\"actions\":[") +
			actionText("SUPPRESS_INTER_STEP_PURSUIT", "EXIT", "boss.sequence.inter-step-pursuit", 0u, 0u) + "]";
		legacyGhostDeath = replaceField(std::move(legacyGhostDeath),
			{ "VALTAN_GHOST_DEATH_AUDITION", "STEP_01", "durationMs", legacySuppression.c_str() });
		CEncounterPatternReference legacyGhostDeathReference;
		if (!Require(!legacyGhostDeath.empty() && writeFixture(legacyGhostDeath) &&
			legacyGhostDeathReference.Load(fixture.File, status),
			(std::string("legacy terminal suppression fixture was rejected: ") + status).c_str()))
			return false;
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
				"[\"VALTAN_WHIRLWIND\",\"VALTAN_FOUR_SLASH\",\"VALTAN_SEQUENCE_FOUR\",\"VALTAN_CROSS\",\"VALTAN_CHARGE\"]", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "ghostPatternIds",
				"[\"VALTAN_FOUR_SLASH\",\"VALTAN_WHIRLWIND\",\"VALTAN_SEQUENCE_FOUR\",\"VALTAN_CROSS\"]", extensionError },
			{ "VALTAN_GHOST_FINALE", "finale", "ghostPatternIds",
				"[\"VALTAN_FOUR_SLASH\",\"VALTAN_WHIRLWIND\"]", extensionError },
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
			{ "VALTAN_GHOST_FINALE", "STEP_02", "cornerIndex", "4", stageMotionError },
			{ "VALTAN_GHOST_FINALE", "STEP_02", "cornerIndex", "-1", stageMotionError },
			{ "VALTAN_GHOST_FINALE", "STEP_02", "cornerIndex", "true", stageMotionError },
			{ "VALTAN_GHOST_FINALE", "STEP_02", "cornerIndex", "1.5", stageMotionError },
			{ "VALTAN_GHOST_FINALE", "STEP_02", "halfExtentsM", "[0,22]", stageMotionError },
			{ "VALTAN_GHOST_FINALE", "STEP_02", "halfExtentsM", "[22,101]", stageMotionError },
			{ "VALTAN_GHOST_FINALE", "STEP_02", "halfExtentsM", "[22]", stageMotionError },
			{ "VALTAN_WARP", "STEP_02", "kind", "\"UNKNOWN_MOTION\"", stageMotionError },
			{ "VALTAN_WARP", "STEP_02", "kind",
				"\"PORTAL_TARGET_RUSH\",\"cornerIndex\":0", stageMotionError },
			{ "VALTAN_WARP", "STEP_02", "kind",
				"\"PORTAL_TARGET_RUSH\",\"halfExtentsM\":[22,22]", stageMotionError },
			{ "VALTAN_WARP", "STEP_02", "retargetDelayMs", "2301", stageMotionError },
			{ "VALTAN_WARP", "STEP_02", "retargetDelayMs", "true", stageMotionError },
			{ "VALTAN_WARP", "STEP_02", "speedMps", "0", stageMotionError },
			{ "VALTAN_WARP", "STEP_02", "speedMps", "1001", stageMotionError },
			{ "VALTAN_WARP", "STEP_02", "distanceM", "0", stageMotionError },
			{ "VALTAN_WARP", "STEP_02", "distanceM", "1001", stageMotionError },
			{ "VALTAN_CATCH_BREATH", "RELEASE_GRABBED_PLAYERS", "releaseMode", "\"UNKNOWN_EJECTION\"", stageActionsError },
			{ "VALTAN_CATCH_BREATH", "RELEASE_GRABBED_PLAYERS", "speedMps", "0", stageActionsError },
			{ "VALTAN_CATCH_BREATH", "RELEASE_GRABBED_PLAYERS", "durationMs", "0", stageActionsError },
			{ "VALTAN_FIST_IN_OUT", "SPAWN_COMBAT_OBJECT", "value", "2", stageActionsError },
			{ "VALTAN_HIGH_JUMP", "AIRBORNE", "spawnCount", "9", stageActionsError },
			{ "VALTAN_HIGH_JUMP", "AIRBORNE", "firstSpawnOffsetMs", "8000", stageActionsError },
			{ "VALTAN_HIGH_JUMP", "AIRBORNE", "firstSpawnOffsetMs", "true", stageActionsError },
			{ "VALTAN_STAGGER_SLOT", "CHANNEL", "verticalOffsetM", "0" },
			{ "VALTAN_STAGGER_SLOT", "CHANNEL", "verticalOffsetM", "100.1" },
			{ "VALTAN_STAGGER_SLOT", "CHANNEL", "verticalOffsetM", "true" },
			{ "VALTAN_STAGGER_SLOT", "CHANNEL", "verticalOffsetM",
				"0.5,\"motion\":{\"kind\":\"FORWARD\",\"distance\":1}" },
			{ "VALTAN_GHOST_DEATH_AUDITION", "STEP_01", "downMs",
				"0,\"verticalOffsetM\":0.5" },
			{ "VALTAN_GHOST_DEATH_AUDITION", "STEP_01", "actionId",
				"\"valtan.sequence.dead.step-other\"", stageActionsError },
			{ "VALTAN_GHOST_DEATH_AUDITION", "STEP_01", "trigger", "\"ENTER\"", stageActionsError },
			{ "VALTAN_GHOST_DEATH_AUDITION", "SUPPRESS_INTER_STEP_PURSUIT",
				"targetId", "\"boss.target.pattern\"", stageActionsError },
			{ "VALTAN_GHOST_DEATH_AUDITION", "SUPPRESS_INTER_STEP_PURSUIT",
				"value", "1", stageActionsError },
			{ "VALTAN_GHOST_DEATH_AUDITION", "SUPPRESS_INTER_STEP_PURSUIT",
				"durationMs", "1", stageActionsError },
			{ "VALTAN_GHOST_PORTAL_ONCE", "ACTIVE", "angleStepDegrees", "100.0", stageActionsError },
			{ "VALTAN_BIND_SLOT", "STEP_01", "value", "10000", stageActionsError },
			{ "VALTAN_SILENCE_SLOT", "STEP_01", "trigger", "\"EXIT\"", stageActionsError },
			{ "VALTAN_SILENCE_SLOT", "STEP_01", "value", "0", stageActionsError },
			{ "VALTAN_SILENCE_SLOT", "SET_PLAYER_SILENCE", "durationMs", "99", stageActionsError },
			{ "VALTAN_TRASH_CATCH_IF", "STEP_07", "outcome", "\"NAVIGATION_BLOCKED\"", stageBranchesError },
			{ "VALTAN_TRASH", "GROGGY", "nextActionId", "\"valtan.sequence.center-trash-rush-if.step-07\"", extensionError },
			{ "VALTAN_TRASH_CATCH_IF", "GROGGY", "nextActionId", "\"valtan.sequence.rush-if.step-07\"", extensionError },
			{ "VALTAN_TRASH_CATCH_SUCCESS", "EXECUTE_TAIL", "nextActionId", "\"valtan.sequence.rush-success.catch-pre-impact\"", extensionError },
			{ "VALTAN_TRASH_CATCH_FAIL", "RUSH_MISS", "nextActionId", "\"valtan.sequence.rush-fail.rush-miss\"", stageBranchesError },
			{ "VALTAN_GHOST_FINALE", "STEP_10", "durationMs",
				"1667,\"branches\":[{\"outcome\":\"TIMEOUT\",\"nextActionId\":\"valtan.sequence.ghost-finale.step-01\"}]", extensionError },
			{ "VALTAN_WHIRLWIND", "RECOVERY", "durationMs",
				"1467,\"branches\":[{\"outcome\":\"TIMEOUT\",\"nextActionId\":\"valtan.attack.whirlwind.windup\"}]", extensionError }
		};
		for (const auto& mutation : malformedFields)
		{
			const bool legacySuppressionMutation =
				std::string(mutation.pattern) == "VALTAN_GHOST_DEATH_AUDITION";
			const auto text = replaceField(legacySuppressionMutation ? legacyGhostDeath : original, mutation);
			const std::string label = std::string(mutation.pattern) + "/" + mutation.field;
			if (!Require(!text.empty(), ("encounter mutation field not found: " + label).c_str()) ||
				!rejectedWithoutCommit(text, label.c_str(), mutation.errorPrefix))
				return false;
		}
		// TIMEOUT null ends the entry stage. Cycles in the now-unreachable
		// tail of a current finale child still cannot be admitted.
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
			if (!rejectedWithoutCommit(text, "invalid arena-center recovery action",
				stageActionsError))
				return false;
		}
		std::cout << "EncounterPatternReference: " << intervalAdmissionCount <<
			" optional interval admissions; latest Trash/portal/finale and " << rejectionCount << " rejection/rollback cases PASS\n";
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
