#include "ServerGameplayContractTests_Runner.h"
#include "KoukuSaydonLogicRuntime.h"
#include "ServerGameplayContractTests.h"
#include "BossCombatRuntime.h"
#include "GameplayCatalog.h"
#include "KoukuSaydonBrain.h"
#include "PlayerSkillSystem.h"
#include "ServerCombatHitRuntime.h"
#include "WorldBootstrap.h"
#include "WorldDestructionBootstrapContractTests.h"
#include <Windows.h>
#include <process.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>


using namespace LostArk::Server;
using namespace LostArk::Shared;

namespace ServerGameplayContractDetail
{

	void Run_KoukuObjectOverlapContracts(
		TESTS& tests, const LostArk::Server::CGameplayCatalog& catalog)
	{
		using namespace LostArk::Shared;
		using namespace LostArk::Server;
		const auto makeWindow = []()
		{
			BOSS_PATTERN_LOGIC_WINDOW window{};
			window.strWindowId = "object.contact";
			window.eKind = BOSS_PATTERN_LOGIC_KIND::OBJECT_OVERLAP;
			window.iStartMs = 100u; window.iDurationMs = 1000u;
			window.strTargetWorldInstanceId = "world.card.idle";
			window.fTargetRadiusM = .5f;
			BOSS_PATTERN_LOGIC_RESULT success{};
			success.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_WORLD_OBJECT_MOTION;
			success.strTargetWorldInstanceId = window.strTargetWorldInstanceId;
			success.strMotionInstanceId = "world.card.hop";
			window.OnSuccess = { success, success }; // Duplicate authoring cannot replay a motion.
			success.strMotionInstanceId = "world.card.flip";
			window.OnFail = { success, success };
			BOSS_PATTERN_LOGIC_RESULT timeout{};
			timeout.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_WORLD_OBJECT_MOTION;
			timeout.strTargetWorldInstanceId = window.strTargetWorldInstanceId;
			timeout.strMotionInstanceId = "world.card.timeout";
			window.OnTimeout.push_back(timeout);
			return window;
		};
		const auto judgeGeometry = [&](const BOSS_LOGIC_REGION& region, SERVER_WORLD_ENTITY& boss,
			const float x, const float z, const float radius, const bool hit,
			const bool insideIsFail, const char* label)
		{
			BOSS_PATTERN_DEFINITION pattern{}; pattern.strPatternId = "KAKULSAYDON_TEST_OBJECT_CONTACT";
			auto window = makeWindow(); window.fTargetWorldX = x; window.fTargetWorldZ = z;
			window.fTargetRadiusM = radius; window.bInsideIsFail = insideIsFail;
			window.CardRegions.push_back(region); pattern.LogicWindows.push_back(window);
			std::map<PLAYER_ID, SERVER_PLAYER> players;
			for (PLAYER_ID id = 1u; id <= 4u; ++id)
			{
				auto& player = players[id]; player.iPlayerId = id; player.isCombatReady = true;
				player.iCurrentHp = player.iMaximumHp = 100u;
				player.fPositionX = player.fPositionZ = 999.f;
			}
			KOUKUSAYDON_LOGIC_LEDGER ledger; KOUKUSAYDON_LOGIC_OUTPUT output;
			std::vector<DAMAGE_EVENT> events;
			CKoukuSaydonLogicRuntime::Build(pattern, boss, 100u, ledger);
			const auto update = [&](const std::uint32_t tick) {
				CKoukuSaydonLogicRuntime::Update(boss, pattern, ledger, players, catalog, nullptr, tick, events, output);
			};
			update(102u);
			const bool beforeStart = !ledger.Windows.front().bOpened && output.WorldSequencePlays.empty();
			update(103u); update(104u); update(132u);
			const bool beforeEnd = output.FollowupPatternIds.empty() &&
				output.WorldSequencePlays.size() == (hit ? 1u : 0u) && ledger.Windows.front().bClosed == hit;
			update(133u); update(134u);
			const bool results = output.WorldSequencePlays.size() == 1u && output.FollowupPatternIds.empty() &&
				output.WorldSequencePlays.front().strTargetSequenceInstanceId == window.strTargetWorldInstanceId &&
				output.WorldSequencePlays.front().strInstanceId == (hit ?
					(insideIsFail ? "world.card.flip" : "world.card.hop") : "world.card.timeout");
			tests.Require(beforeStart && beforeEnd && results && ledger.Windows.front().bClosed &&
				ledger.Windows.front().Answers.empty() && events.empty(), label);
		};
		auto boss = std::make_unique<SERVER_WORLD_ENTITY>();
		boss->iNetEntityId = 4242u; boss->iPatternSequence = 1u;
		boss->fPositionX = 10.f; boss->fPositionZ = 20.f; boss->fYawDegrees = 90.f;
		BOSS_LOGIC_REGION box{}; box.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT;
		box.fCenterZ = 3.f; box.fHalfX = 1.f; box.fHalfZ = 2.f;
		judgeGeometry(box, *boss, 15.5f, 20.f, .5f, true, false,
			"Object circle touches the rotated boss box outside its centre and emits one Motion for four players");
		judgeGeometry(box, *boss, 15.4f, 21.4f, .5f, false, false,
			"Object circle misses the rotated box corner despite axis inflation and times out once");
		judgeGeometry(box, *boss, 15.35f, 21.35f, .5f, true, true,
			"Object circle overlaps the rounded box corner and Inside Is Fail emits only the Fail Motion");
		BOSS_LOGIC_REGION circle{}; circle.bCircle = true;
		circle.fCenterX = 4.f; circle.fCenterZ = 5.f; circle.fRadiusM = 2.f;
		judgeGeometry(circle, *boss, 6.5f, 5.f, .5f, true, false,
			"World object circles include exact radius-sum contact");
		judgeGeometry(circle, *boss, 6.501f, 5.f, .5f, false, false,
			"World object circles reject a gap beyond the shared contact epsilon");
		BOSS_LOGIC_REGION cone{}; cone.bSector = true;
		cone.fYawDegrees = 90.f; cone.fRadiusM = 4.f; cone.fHalfAngleDegrees = 45.f;
		judgeGeometry(cone, *boss, 2.f, 2.5f, .36f, true, false,
			"Object circle intersects a rotated sector radial edge while its centre lies outside");
		judgeGeometry(cone, *boss, 2.f, 2.6f, .36f, false, false,
			"Object circle misses a rotated sector radial edge beyond its own radius");
		judgeGeometry(cone, *boss, 4.5f, 0.f, .5f, true, false,
			"Object circle includes tangency to the sector outer arc");
		judgeGeometry(cone, *boss, 0.f, 0.f, .5f, true, false,
			"An object circle overlapping the sector origin counts as physical contact");

		// Same transform sampler as player regions: delayed, visible, scaled and
		// quaternion-rotated WORLD motion, evaluated on the Server clock.
		for (const bool testVisibility : { true, false })
		{
			BOSS_PATTERN_DEFINITION pattern{}; pattern.strPatternId = "KAKULSAYDON_TEST_OBJECT_WORLD_TRACK";
			auto window = makeWindow(); window.iStartMs = 200u; window.iDurationMs = 1300u;
			window.fTargetWorldX = testVisibility ? 102.f : 112.2f;
			window.fTargetWorldZ = testVisibility ? 205.f : 205.12132f; window.fTargetRadiusM = .25f;
			BOSS_LOGIC_REGION region{}; region.bCircle = true; region.fCenterZ = 1.f; region.fRadiusM = 1.f;
			region.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_SPAWN;
			auto& track = region.WorldTrack; track.bEnabled = true;
			track.iStartMs = 200u; track.iStartDelayMs = 100u; track.iDurationMs = 1100u;
			track.fPlaybackSpeed = 2.f; track.bSmoothStep = true;
			track.fBaselineX = 2.f; track.fBaselineY = -1000.f; track.fBaselineZ = 3.f;
			track.fBaselineScaleX = track.fBaselineScaleY = track.fBaselineScaleZ = 2.f;
			BOSS_LOGIC_WORLD_TRANSFORM_KEY key{}; key.bVisible = false; track.Keys.push_back(key);
			key.iTimeMs = 100u; key.bVisible = true; track.Keys.push_back(key);
			key.iTimeMs = 1100u; key.fOffsetX = 10.f; key.fOffsetY = 1000.f;
			key.fRotationY = key.fRotationW = .7071067811865475f;
			key.fScaleX = key.fScaleY = key.fScaleZ = 2.f; track.Keys.push_back(key);
			window.CardRegions.push_back(region); pattern.LogicWindows.push_back(window);
			boss->fSpawnPositionX = 100.f; boss->fSpawnPositionZ = 200.f;
			std::map<PLAYER_ID, SERVER_PLAYER> players; // Object contact needs no player proxy.
			KOUKUSAYDON_LOGIC_LEDGER ledger; KOUKUSAYDON_LOGIC_OUTPUT output;
			std::vector<DAMAGE_EVENT> events;
			CKoukuSaydonLogicRuntime::Build(pattern, *boss, 1000u, ledger);
			const auto update = [&](const std::uint32_t tick) {
				CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, tick, events, output);
			};
			update(1006u); update(testVisibility ? 1010u : 1015u);
			const bool waited = output.WorldSequencePlays.empty() && !ledger.Windows.front().bClosed;
			update(testVisibility ? 1011u : 1018u);
			const bool contacted = ledger.Windows.front().bClosed && output.WorldSequencePlays.size() == 1u &&
				output.WorldSequencePlays.front().strInstanceId == "world.card.hop";
			update(1021u); update(1045u);
			tests.Require(waited && contacted && output.WorldSequencePlays.size() == 1u && output.FollowupPatternIds.empty() &&
				ledger.Windows.front().bClosed && ledger.Windows.front().Answers.empty(), testVisibility ?
				"Object overlap waits through WORLD start delay and invisible keys before firing once" :
				"Object overlap samples WORLD translation, quaternion, scale, smooth clock and spawn anchor in XZ with no players");
		}
		{
			BOSS_PATTERN_DEFINITION pattern{}; pattern.strPatternId = "KAKULSAYDON_TEST_POINT_BOUNDARY_PRESERVED";
			BOSS_PATTERN_LOGIC_WINDOW window{}; window.eKind = BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP;
			window.iDurationMs = 100u;
			cone.fYawDegrees = 0.f; window.CardRegions.push_back(cone); pattern.LogicWindows.push_back(window);
			std::map<PLAYER_ID, SERVER_PLAYER> players;
			for (PLAYER_ID id = 1u; id <= 3u; ++id)
			{
				auto& player = players[id]; player.iPlayerId = id; player.isCombatReady = true; player.iCurrentHp = 100u;
				player.fPositionX = id == 1u ? -1.f : id == 2u ? 1.f : 0.f;
				player.fPositionZ = id == 3u ? 0.f : 1.f;
			}
			KOUKUSAYDON_LOGIC_LEDGER ledger; KOUKUSAYDON_LOGIC_OUTPUT output; std::vector<DAMAGE_EVENT> events;
			CKoukuSaydonLogicRuntime::Build(pattern, *boss, 2000u, ledger);
			CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, 2003u, events, output);
			const auto& answers = ledger.Windows.front().Answers;
			tests.Require(answers.at(1u) == KOUKUSAYDON_LOGIC_ANSWER::SUCCESS &&
				answers.at(2u) == KOUKUSAYDON_LOGIC_ANSWER::TIMEOUT && answers.at(3u) == KOUKUSAYDON_LOGIC_ANSWER::TIMEOUT,
				"Player point sectors retain the negative-inclusive positive-exclusive radial edge and excluded origin");
		}
	}

	void Run_KoukuObjectContactContracts(TESTS& tests, const LostArk::Server::CGameplayCatalog& catalog)
	{
		using namespace LostArk::Shared;
		using namespace LostArk::Server;
		const auto makeDeadline = [](const bool endsPattern)
		{
			BOSS_PATTERN_LOGIC_WINDOW window{}; window.strWindowId = "joker.deadline";
			window.eKind = BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL; window.iDurationMs = 1000u;
			window.bEndsPatternOnSuccess = endsPattern;
			BOSS_PATTERN_LOGIC_RESULT result{}; result.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN;
			result.strPatternId = "joker.success"; window.OnSuccess.push_back(result);
			result.strPatternId = "joker.timeout"; window.OnTimeout.push_back(result);
			return window;
		};
		const auto makeContact = [](const std::string& id, const unsigned priority, const unsigned start,
			const std::string& motion, const bool signal)
		{
			BOSS_PATTERN_LOGIC_WINDOW window{}; window.strWindowId = id;
			window.eKind = BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT; window.iStartMs = start; window.iDurationMs = 100u;
			window.strContactGroupId = "hammer.strike"; window.iContactPriority = priority; window.bHasContactGroup = true;
			window.ContactTargets = { { "card.normal", "card.shared.idle", -1.f, 0.f, .1f },
				{ "card.joker", "card.shared.idle", 1.f, 0.f, .1f } };
			BOSS_LOGIC_REGION region{}; region.bCircle = true; region.fRadiusM = 2.f;
			region.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT; window.CardRegions.push_back(region);
			BOSS_PATTERN_LOGIC_RESULT result{}; result.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_CONTACT_WORLD_OBJECT_MOTION;
			result.ContactMotions = { { "card.normal", motion }, { "card.joker", motion } }; window.OnSuccess.push_back(result);
			if (signal)
			{
				result = {}; result.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::COMPLETE_LOGIC_WINDOW;
				result.strTargetLogicOccurrenceId = "joker.deadline"; result.strContactTargetWorldOccurrenceId = "card.joker";
				window.OnSuccess.push_back(result);
			}
			return window;
		};
		for (const unsigned scenario : { 0u, 1u, 2u, 3u, 4u, 5u })
		{
			BOSS_PATTERN_DEFINITION pattern{}; pattern.strPatternId = "contact.contract";
			pattern.LogicWindows = { makeDeadline(scenario != 0u),
				makeContact("edge", 10u, 100u, "card.hop", false),
				makeContact("centre", 100u, 100u, "card.flip", true) };
			if (scenario == 2u)
			{
				pattern.LogicWindows.erase(pattern.LogicWindows.begin() + 1);
				pattern.LogicWindows[1].CardRegions[0].fRadiusM = .2f;
			}
			else if (scenario == 3u)
			{
				pattern.LogicWindows = { makeDeadline(true), makeContact("last.strike", 100u, 900u, "card.flip", true) };
				pattern.LogicWindows[1].OnSuccess[1].strContactTargetWorldOccurrenceId.clear();
			}
			else if (scenario >= 4u)
				pattern.LogicWindows = { makeContact("strike.one", 100u, 100u, "card.flip", false),
					makeContact("strike.two", scenario == 5u ? 10u : 100u, 500u, "card.hop", false) };
			auto boss = std::make_unique<SERVER_WORLD_ENTITY>(); boss->iNetEntityId = 1u;
			boss->fPositionX = scenario == 2u ? -1.f : scenario == 3u ? 20.f : 0.f;
			std::map<PLAYER_ID, SERVER_PLAYER> players; std::vector<DAMAGE_EVENT> events;
			KOUKUSAYDON_LOGIC_LEDGER ledger; KOUKUSAYDON_LOGIC_OUTPUT output;
			pattern.strEncounterId = "ENCOUNTER_KAKULSAYDON_G1"; pattern.eSelection = BOSS_PATTERN_SELECTION::AUDITION_ONLY;
			BOSS_PATTERN_STAGE_DEFINITION stage{}; stage.strStageId = "contact.stage"; stage.strActionId = "contact.action";
			stage.eStageKind = BOSS_PATTERN_STAGE_KIND::ACTIVE; stage.iDurationMs = 1000u;
			pattern.Stages.push_back(stage); pattern.iExpectedStageCount = 1u;
			boss->strEncounterId = pattern.strEncounterId; boss->iCurrentHp = 100u;
			GameplayDataRevision revision{}; revision.Bytes.front() = 1u;
			CKoukuSaydonBrain brain; std::string admissionStatus;
			tests.Require(brain.Begin_Pattern(*boss, pattern, revision, 100u, admissionStatus),
				"Contact and external signal fixtures pass the actual animation-only Brain admission");
			CKoukuSaydonLogicRuntime::Build(pattern, *boss, 100u, ledger);
			for (const auto tick : { 102u, 103u, 104u, 106u, 115u, 116u, 118u, 129u })
				CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, tick, events, output);
			const bool waitedForDeadline = output.FollowupPatternIds.empty() && !ledger.Windows.front().bClosed;
			if (scenario == 3u) boss->fPositionX = 0.f;
			for (const auto tick : { 130u, 131u })
				CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, tick, events, output);
			if (scenario < 2u || scenario == 3u)
			{
				const bool exactCards = output.WorldSequencePlays.size() == 2u &&
					output.WorldSequencePlays[0].strTargetWorldOccurrenceId == "card.normal" &&
					output.WorldSequencePlays[1].strTargetWorldOccurrenceId == "card.joker" &&
					std::all_of(output.WorldSequencePlays.begin(), output.WorldSequencePlays.end(), [](const auto& play) {
						return play.strInstanceId == "card.flip" && play.strTargetSequenceInstanceId == "card.shared.idle";
					});
				tests.Require(exactCards && output.FollowupPatternIds == std::vector<std::string>{ "joker.success" } &&
					output.bEndPatternEarly == (scenario != 0u) && events.empty() && (scenario != 3u || waitedForDeadline), scenario == 3u ?
					"Two unfiltered final-tick contacts complete once before the deadline timeout" : scenario == 1u ?
					"Higher-priority contact wins per occurrence and completes the whole deadline once with early end" :
					"Cards sharing a saved Idle receive one centre Motion each and success can keep the pattern running");
			}
			else if (scenario == 2u)
				tests.Require(waitedForDeadline && output.WorldSequencePlays.size() == 1u &&
					output.WorldSequencePlays.front().strTargetWorldOccurrenceId == "card.normal" && !output.bEndPatternEarly &&
					output.FollowupPatternIds == std::vector<std::string>{ "joker.timeout" },
					"A normal-card hit does not complete the Joker signal and missing a short strike waits for the whole deadline");
			else if (scenario == 5u)
				tests.Require(output.WorldSequencePlays.size() == 2u && ledger.AppliedContactMotionPriorities.size() == 2u,
					"A lower-priority edge reaction cannot turn an already flipped card back to Idle on a later strike");
			else
				tests.Require(output.WorldSequencePlays.size() == 4u && output.WorldSequencePlays[0].strInstanceId == "card.flip" &&
					output.WorldSequencePlays[2].strInstanceId == "card.hop" && ledger.ConsumedContactGroups.size() == 4u,
					"A later strike can reuse its contact group and hit each card once again");
		}
	}

	void Run_KoukuWorldPlacementContracts(TESTS& tests, const LostArk::Server::CGameplayCatalog& catalog)
	{
		using namespace LostArk::Shared;
		using namespace LostArk::Server;
		BOSS_PATTERN_DEFINITION pattern{}; pattern.strPatternId = "placement.contract";
		pattern.strEncounterId = "ENCOUNTER_KAKULSAYDON_G1"; pattern.eSelection = BOSS_PATTERN_SELECTION::AUDITION_ONLY;
		BOSS_PATTERN_STAGE_DEFINITION stage{}; stage.strStageId = "placement.stage"; stage.strActionId = "placement.action";
		stage.eStageKind = BOSS_PATTERN_STAGE_KIND::ACTIVE; stage.iDurationMs = 1000u;
		pattern.Stages.push_back(stage); pattern.iExpectedStageCount = 1u;
		for (unsigned index = 0u; index < 7u; ++index)
		{
			BOSS_PATTERN_WORLD_SEQUENCE cue; cue.strInstanceId = index == 6u ? "world.joker.idle" : "world.card.idle";
			cue.strOccurrenceId = "world.card." + std::to_string(index); cue.iDurationMs = 1000u;
			BOSS_PATTERN_WORLD_PLACEMENT placement;
			placement.fPositionX = static_cast<float>(index * 10u); placement.fPositionY = -2.f; placement.fPositionZ = 30.f;
			placement.fRotationXDegrees = 15.f; placement.fRotationYDegrees = 30.f; placement.fRotationZDegrees = -45.f;
			placement.fScaleX = .5f; placement.fScaleY = 2.f; placement.fScaleZ = 3.f;
			cue.Placement = placement; pattern.WorldSequences.push_back(cue);
		}
		auto boss = std::make_unique<SERVER_WORLD_ENTITY>(); boss->iNetEntityId = 1u;
		boss->strEncounterId = pattern.strEncounterId; boss->iCurrentHp = 100u;
		boss->fSpawnPositionX = 999.f; boss->fSpawnPositionY = 888.f; boss->fSpawnPositionZ = 777.f;
		GameplayDataRevision revision{}; revision.Bytes.front() = 1u;
		CKoukuSaydonBrain brain; std::string status;
		tests.Require(brain.Begin_Pattern(*boss, pattern, revision, 100u, status), "Seven independently placed Object cues pass actual Brain admission");
		std::map<PLAYER_ID, SERVER_PLAYER> players; std::vector<DAMAGE_EVENT> events;
		KOUKUSAYDON_LOGIC_LEDGER ledger; KOUKUSAYDON_LOGIC_OUTPUT output;
		CKoukuSaydonLogicRuntime::Build(pattern, *boss, 100u, ledger);
		CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, 100u, events, output);
		bool independent = output.WorldSequencePlays.size() == 7u;
		for (std::size_t index = 0u; independent && index < output.WorldSequencePlays.size(); ++index)
		{
			const auto& play = output.WorldSequencePlays[index];
			independent = play.Placement && play.strOccurrenceId == pattern.WorldSequences[index].strOccurrenceId &&
				play.strInstanceId == pattern.WorldSequences[index].strInstanceId && play.fPositionOffsetX == 0.f &&
				play.fPositionOffsetY == 0.f && play.fPositionOffsetZ == 0.f &&
				play.Placement->fPositionX == static_cast<float>(index * 10u) && play.Placement->fPositionY == -2.f &&
				play.Placement->fPositionZ == 30.f && play.Placement->fRotationXDegrees == 15.f &&
				play.Placement->fRotationYDegrees == 30.f && play.Placement->fRotationZDegrees == -45.f &&
				play.Placement->fScaleX == .5f && play.Placement->fScaleY == 2.f && play.Placement->fScaleZ == 3.f;
		}
		output = {};
		CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, 101u, events, output);
		tests.Require(independent && output.WorldSequencePlays.empty(), "Seven Object occurrences keep full absolute TRS, ignore boss spawn and emit only once");
		for (unsigned invalid = 0u; invalid < 6u; ++invalid)
		{
			auto rejected = pattern; auto& cue = rejected.WorldSequences.front();
			switch (invalid)
			{
			case 0u: cue.Placement->fScaleY = 0.f; break;
			case 1u: cue.Placement->fRotationXDegrees = 36001.f; break;
			case 2u: cue.fPositionOffsetX = 1.f; break;
			case 3u: cue.bAnchorBossSpawn = true; break;
			case 4u: cue.strOccurrenceId.clear(); break;
			case 5u: cue.Placement->fPositionZ = std::numeric_limits<float>::infinity(); break;
			}
			CKoukuSaydonBrain rejectedBrain;
			tests.Require(!rejectedBrain.Begin_Pattern(*boss, rejected, revision, 100u, status), "Brain rejects malformed or conflicting absolute Object placement");
		}
	}

	void Run_KoukuBoneContactContracts(TESTS& tests, const LostArk::Server::CGameplayCatalog& catalog)
	{
		using namespace LostArk::Shared;
		using namespace LostArk::Server;
		for (const unsigned scenario : { 0u, 1u, 2u })
		{
			BOSS_PATTERN_DEFINITION pattern{}; pattern.strPatternId = "bone.contact.contract";
			BOSS_PATTERN_LOGIC_WINDOW window{}; window.strWindowId = "hammer.tip";
			window.eKind = BOSS_PATTERN_LOGIC_KIND::OBJECT_CONTACT; window.iStartMs = scenario == 2u ? 101u : 100u;
			window.iDurationMs = 100u; window.bHasContactGroup = true;
			const float targetX = scenario == 0u ? 12.f : scenario == 1u ? 35.5f : 4000.f / 30.f - 101.f;
			window.ContactTargets = { { "tip.card", "card.idle", targetX, scenario == 0u ? 18.5f : scenario == 1u ? 48.f : 0.f, .01f } };
			BOSS_PATTERN_LOGIC_RESULT result{}; result.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::PLAY_CONTACT_WORLD_OBJECT_MOTION;
			result.ContactMotions = { { "tip.card", "card.flip" } }; window.OnSuccess.push_back(result);
			BOSS_LOGIC_REGION region{}; region.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT;
			region.fCenterX = scenario == 2u ? 0.f : .5f; region.fYawDegrees = 90.f;
			region.fHalfX = .1f; region.fHalfZ = 1.2f; region.bCircle = scenario == 2u; region.fRadiusM = .01f;
			auto& track = region.WorldTrack; track.bEnabled = true; track.iStartMs = window.iStartMs; track.iDurationMs = 100u;
			BOSS_LOGIC_WORLD_TRANSFORM_KEY key{}; key.fOffsetZ = scenario == 2u ? 0.f : 2.f; track.Keys.push_back(key);
			key.iTimeMs = 100u; key.fOffsetX = scenario == 2u ? 100.f : 3.f; track.Keys.push_back(key);
			window.CardRegions.push_back(region); pattern.LogicWindows.push_back(window);
			auto boss = std::make_unique<SERVER_WORLD_ENTITY>(); boss->iNetEntityId = 1u;
			boss->fPositionX = scenario == 2u ? 0.f : 10.f; boss->fPositionZ = scenario == 2u ? 0.f : 20.f;
			boss->fYawDegrees = scenario == 2u ? 0.f : 90.f;
			std::map<PLAYER_ID, SERVER_PLAYER> players; std::vector<DAMAGE_EVENT> events;
			KOUKUSAYDON_LOGIC_LEDGER ledger; KOUKUSAYDON_LOGIC_OUTPUT output;
			pattern.strEncounterId = "ENCOUNTER_KAKULSAYDON_G1"; pattern.eSelection = BOSS_PATTERN_SELECTION::AUDITION_ONLY;
			BOSS_PATTERN_STAGE_DEFINITION stage{}; stage.strStageId = "contact.stage"; stage.strActionId = "contact.action";
			stage.eStageKind = BOSS_PATTERN_STAGE_KIND::ACTIVE; stage.iDurationMs = 1000u;
			pattern.Stages.push_back(stage); pattern.iExpectedStageCount = 1u;
			boss->strEncounterId = pattern.strEncounterId; boss->iCurrentHp = 100u;
			GameplayDataRevision revision{}; revision.Bytes.front() = 1u;
			CKoukuSaydonBrain brain; std::string admissionStatus;
			tests.Require(brain.Begin_Pattern(*boss, pattern, revision, 100u, admissionStatus),
				"Contact and external signal fixtures pass the actual animation-only Brain admission");
			CKoukuSaydonLogicRuntime::Build(pattern, *boss, 100u, ledger);
			const auto update = [&](const unsigned tick) {
				CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, tick, events, output);
			};
			update(scenario == 2u ? 103u : 102u); const bool beforeStart = output.WorldSequencePlays.empty();
			update(scenario == 2u ? 104u : 103u);
			const bool beforeMovedRoot = scenario != 1u || output.WorldSequencePlays.empty();
			if (scenario == 1u) { boss->fPositionX = 40.f; boss->fPositionZ = 50.f; boss->fYawDegrees = 180.f; }
			update(106u); update(107u); update(108u);
			tests.Require(beforeStart && beforeMovedRoot && output.WorldSequencePlays.size() == 1u &&
				output.WorldSequencePlays.front().strTargetWorldOccurrenceId == "tip.card" && events.empty(), scenario == 2u ?
				"Bone contact uses the exact authored millisecond clock after a fractional-tick trigger start" : scenario == 1u ?
				"Baked bone motion composes the current Server boss position and yaw once at the final tick" :
				"Bone tip plus authored offset follows the boss while collision shape keeps authored TARGET_YAW orientation");
		}
	}

	/* KoukuSaydon Logic runtime: synthetic windows judged on a fixed clock so
	the verdict rules never depend on which pattern the composition authors.
	A function of its own keeps these rooms and players off the contract
	frame, which already sits close to the 1 MiB production stack. */
    void Run_KoukuFearAndCounterContracts(TESTS& tests, const LostArk::Server::CGameplayCatalog& catalog)
    {
        using namespace LostArk::Shared;
        using namespace LostArk::Server;
        auto boss = std::make_unique<SERVER_WORLD_ENTITY>();
        boss->iNetEntityId = 4000u; boss->iPatternSequence = 5u;
        boss->eKind = WORLD_BOOTSTRAP_KIND::BOSS;
        boss->strPatternId = "fear.pattern"; boss->strActionId = "fear.action";
        boss->iCurrentHp = boss->iMaximumHp = 10000u;
        SERVER_PLAYER player{};
        player.iPlayerId = 1u; player.iNetEntityId = 100u;
        player.iCurrentHp = player.iMaximumHp = 1000u;
        player.fPositionZ = -5.f; player.fYawDegrees = 180.f;
        player.hasMoveGoal = true; player.iComboStage = 2u;
        player.hasBufferedComboInput = true;
        std::map<PLAYER_ID, SERVER_PLAYER> players{{1u, player}};
        players[2u] = player; players[2u].iPlayerId = 2u;
        players[2u].fYawDegrees = 0.f;
        BOSS_PATTERN_LOGIC_RESULT fear{};
        fear.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::FEAR;
        fear.iDurationMs = 3000u; fear.strFearPresentationId = "fear.result";
        BOSS_PATTERN_DEFINITION pattern{}; pattern.strPatternId = boss->strPatternId;
        BOSS_PATTERN_LOGIC_WINDOW gaze{};
        gaze.strWindowId = "gaze.1"; gaze.eKind = BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS;
        gaze.iDurationMs = 1000u; gaze.fHalfAngleDegrees = 45.f; gaze.fMaxDistanceM = 50.f;
        gaze.OnFail = {fear}; pattern.LogicWindows = {gaze};
        KOUKUSAYDON_LOGIC_LEDGER ledger;
        KOUKUSAYDON_LOGIC_OUTPUT output;
        std::vector<DAMAGE_EVENT> events;
        CKoukuSaydonLogicRuntime::Build(pattern, *boss, 100u, ledger);
        CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, 130u, events, output);
        auto& afraid = players.at(1u);
        tests.Require(afraid.eAction == PLAYER_ACTION_STATE::FEAR && afraid.iActionStartTick == 130u &&
            afraid.iFearEndTick == 220u && afraid.strFearPresentationId == "fear.result" &&
            afraid.iCurrentHp == 1000u && !afraid.hasMoveGoal && afraid.iComboStage == 0u &&
            !afraid.hasBufferedComboInput && players.at(2u).eAction == PLAYER_ACTION_STATE::NONE,
            "A failed boss gaze fears only the failing player for 90 ticks and cancels movement/combo without killing");
        CKoukuSaydonLogicRuntime::Apply_Result(afraid, fear, *boss, catalog, nullptr, 150u, events);
        CPlayerSkillSystem::Arm_PlayerHitReaction(afraid, 0.f, 0.f, 2.f, 500u, true, 1000u, 151u);
        tests.Require(afraid.iFearEndTick == 220u && afraid.eAction == PLAYER_ACTION_STATE::FEAR &&
            afraid.fKnockbackRemainingSeconds == 0.f &&
            CKoukuSaydonLogicRuntime::Update_PlayerFear(afraid, 219u) &&
            !CKoukuSaydonLogicRuntime::Update_PlayerFear(afraid, 220u) &&
            afraid.eAction == PLAYER_ACTION_STATE::NONE && afraid.strFearPresentationId.empty(),
            "Overlapping fear/hit does not rearm its deadline; the exact deadline clears the lock and presentation identity");
        CKoukuSaydonLogicRuntime::Apply_Result(afraid, fear, *boss, catalog, nullptr, 300u, events);
        afraid.iCurrentHp = 0u;
        tests.Require(!CKoukuSaydonLogicRuntime::Update_PlayerFear(afraid, 301u) &&
            afraid.eAction == PLAYER_ACTION_STATE::DEAD && afraid.iFearEndTick == 0u,
            "Death cancels fear without reviving the player");

        std::map<PLAYER_ID, SERVER_PLAYER> invertedGazePlayers;
        for (PLAYER_ID id = 1u; id <= 6u; ++id)
        {
            auto& target = invertedGazePlayers[id];
            target = player; target.iPlayerId = id; target.fYawDegrees = 0.f;
        }
        invertedGazePlayers.at(2u).fYawDegrees = 45.f;
        invertedGazePlayers.at(3u).fYawDegrees = 45.1f;
        invertedGazePlayers.at(4u).fYawDegrees = 180.f;
        invertedGazePlayers.at(5u).fPositionZ = -51.f;
        invertedGazePlayers.at(6u).fPositionZ = 0.f;
        gaze.bInsideIsFail = true; pattern.LogicWindows = {gaze};
        CKoukuSaydonLogicRuntime::Build(pattern, *boss, 300u, ledger);
        output = {};
        CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, invertedGazePlayers,
            catalog, nullptr, 330u, events, output);
        tests.Require(invertedGazePlayers.at(1u).eAction == PLAYER_ACTION_STATE::FEAR &&
            invertedGazePlayers.at(2u).eAction == PLAYER_ACTION_STATE::FEAR &&
            invertedGazePlayers.at(3u).eAction == PLAYER_ACTION_STATE::NONE &&
            invertedGazePlayers.at(4u).eAction == PLAYER_ACTION_STATE::NONE &&
            invertedGazePlayers.at(5u).eAction == PLAYER_ACTION_STATE::NONE &&
            invertedGazePlayers.at(6u).eAction == PLAYER_ACTION_STATE::FEAR,
            "Facing-is-fail gaze fears inside and boundary players, including co-location, while outside angle or range stays safe");

        BOSS_PATTERN_LOGIC_WINDOW counter{};
        counter.strWindowId = "counter.1"; counter.eKind = BOSS_PATTERN_LOGIC_KIND::COUNTER_WINDOW;
        counter.iDurationMs = 1000u; counter.bEndsPatternOnSuccess = true;
        BOSS_PATTERN_LOGIC_RESULT followup{};
        followup.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN;
        followup.strPatternId = "groggy.pattern"; counter.OnSuccess = {followup};
        pattern.LogicWindows = {counter};
        CKoukuSaydonLogicRuntime::Build(pattern, *boss, 400u, ledger);
        CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, 400u, events, output);
        SERVER_PLAYER_TO_WORLD_HIT hit{};
        hit.iSourcePlayerId = 2u; hit.iSkillId = 34010u; hit.iRawDamage = 1u; hit.iServerTick = 400u;
        (void)CServerCombatHitRuntime::Apply_PlayerToWorld(*boss, hit, events);
        CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, 400u, events, output);
        tests.Require(!output.bEndPatternEarly && output.FollowupPatternIds.empty(),
            "Ordinary HP damage cannot succeed a counter window");
        hit.iCounterPower = 1u; hit.iServerTick = 401u;
        (void)CServerCombatHitRuntime::Apply_PlayerToWorld(*boss, hit, events);
        CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, 401u, events, output);
        tests.Require(output.bEndPatternEarly && output.FollowupPatternIds == std::vector<std::string>{"groggy.pattern"} &&
            !CBossCombatRuntime::Has_Flag(boss->BossCombat, SERVER_BOSS_COMBAT_FLAG::COUNTERABLE),
            "A counter-power hit consumes the Server counter outcome once, ends spider, and queues groggy");
        output = {};
        CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, 402u, events, output);
        tests.Require(output.FollowupPatternIds.empty(), "A closed counter window cannot queue groggy twice");

        // A Parent deadline cancels the child verdict and keeps a longer Parent window alive.
        BOSS_PATTERN_LOGIC_WINDOW parentWindow{};
        parentWindow.strWindowId = "fear.pattern.logic.1";
        parentWindow.eKind = BOSS_PATTERN_LOGIC_KIND::EXTERNAL_SIGNAL;
        parentWindow.iDurationMs = 2000u;
        BOSS_PATTERN_LOGIC_WINDOW childWindow = counter;
        childWindow.strWindowId = "fear.pattern.pattern.1.r0.logic.1";
        childWindow.bEndsPatternOnSuccess = false;
        childWindow.bCancelAtEnd = true;
        childWindow.OnTimeout = {followup};
        pattern.LogicWindows = {parentWindow, childWindow};
        CKoukuSaydonLogicRuntime::Build(pattern, *boss, 450u, ledger);
        output = {};
        CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, 450u, events, output);
        CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, 480u, events, output);
        tests.Require(!ledger.Windows[0].bClosed && ledger.Windows[1].bClosed &&
            output.FollowupPatternIds.empty() && !output.bEndPatternEarly,
            "Scheduled child deadline cancels timeout outcomes while Parent judgement remains active");

        BOSS_PATTERN_LOGIC_WINDOW scheduledPose{};
        scheduledPose.strWindowId = "fear.pattern.pattern.2.r0.logic.1";
        scheduledPose.eKind = BOSS_PATTERN_LOGIC_KIND::POSE_INPUT;
        scheduledPose.iStartMs = 1000u; scheduledPose.iDurationMs = 1000u;
        pattern.LogicWindows = {scheduledPose};
        CKoukuSaydonLogicRuntime::Build(pattern, *boss, 500u, ledger);
        players.clear(); players.emplace(1u, player);
        CKoukuSaydonLogicRuntime::Update_PlayerModes(players, &ledger, nullptr, 529u);
        const auto beforeDance = players.at(1u).eKoukuHudMode;
        CKoukuSaydonLogicRuntime::Update_PlayerModes(players, &ledger, nullptr, 530u);
        const auto duringDance = players.at(1u).eKoukuHudMode;
        CKoukuSaydonLogicRuntime::Update_PlayerModes(players, &ledger, nullptr, 560u);
        tests.Require(beforeDance != KOUKU_HUD_MODE::DANCE && duringDance == KOUKU_HUD_MODE::DANCE &&
            players.at(1u).eKoukuHudMode != KOUKU_HUD_MODE::DANCE,
            "Scheduled child dance HUD opens on its own interval and releases at the deadline");

        BOSS_PATTERN_LOGIC_WINDOW charge{};
        charge.strWindowId = "charge.1"; charge.eKind = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA;
        tests.Require(charge.fChargeYawOffsetDegrees == 0.f, "Legacy charge facing offset defaults to zero");
        charge.iStartMs = 101u; charge.iDurationMs = 1001u; charge.fBossChargeDistanceM = 7.f;
        charge.fChargeYawOffsetDegrees = 90.f;
        BOSS_LOGIC_REGION body{};
        body.strRegionId = "charge.body"; body.bCircle = true;
        body.fRadiusM = 1.f; body.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT;
        charge.CardRegions = {body}; charge.OnSuccess = {fear};
        pattern.LogicWindows = {charge};
        players.clear(); player.eAction = PLAYER_ACTION_STATE::NONE;
        player.fPositionX = 0.f; player.fPositionZ = 10.f;
        players.emplace(1u, player);
        boss->fPositionX = boss->fPositionZ = 0.f;
        CKoukuSaydonLogicRuntime::Build(pattern, *boss, 500u, ledger);
        output = {};
        CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, 504u, events, output);
        players.at(1u).fPositionX = 10.f; players.at(1u).fPositionZ = 0.f;
        output = {};
        CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, 519u, events, output);
        tests.Require(std::abs(boss->fPositionX) < .001f && boss->fPositionZ > 3.f && boss->fPositionZ < 4.f &&
            std::abs(boss->fYawDegrees - 90.f) < .001f && boss->fPatternTargetLastPositionZ == 10.f,
            "Boss charge keeps its captured travel direction after the player moves while rotating body yaw by ninety degrees");
        players.at(1u).fPositionX = 0.f; players.at(1u).fPositionZ = 7.f;
        output = {};
        CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, 534u, events, output);
        tests.Require(std::abs(boss->fPositionZ - 7.f) < .001f && std::abs(boss->fPositionX) < .001f &&
            std::abs(boss->fYawDegrees - 90.f) < .001f && players.at(1u).eAction == PLAYER_ACTION_STATE::FEAR && players.at(1u).iActionStartTick == 534u,
            "Boss charge reaches exactly seven metres on its pattern-clock deadline and its body collider follows before fear overlap");
        output = {};
        CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, players, catalog, nullptr, 550u, events, output);
        tests.Require(std::abs(boss->fPositionZ - 7.f) < .001f && players.at(1u).iActionStartTick == 534u,
            "A completed charge neither moves nor reapplies its fear again");
    }

	void Run_KoukuSaydonLogicRuntimeContracts(
		TESTS& tests, const LostArk::Server::CGameplayCatalog& catalog)
	{
		using namespace LostArk::Shared;
		using namespace LostArk::Server;
		/* KoukuSaydon Logic runtime: synthetic windows judged on a fixed clock so
		the verdict rules never depend on which pattern the composition authors. */
		const auto makePlayer = [](const PLAYER_ID id, const float x, const float z,
			const float yaw) -> SERVER_PLAYER
		{
			SERVER_PLAYER player{};
			player.iPlayerId = id;
			player.iNetEntityId = static_cast<NET_ENTITY_ID>(1000u + id);
			player.iCurrentHp = player.iMaximumHp = 1000u;
			player.iCurrentMadness = 0u;
			player.iMaximumMadness = 100u;
			player.fPositionX = x;
			player.fPositionZ = z;
			player.fYawDegrees = yaw;
			return player;
		};
		SERVER_WORLD_ENTITY logicBoss{};
		logicBoss.iNetEntityId = 4242u;
		logicBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		logicBoss.strEncounterId = "ENCOUNTER_KAKULSAYDON_G1";
		logicBoss.strArchetypeId = "BOSS_KAKULSAYDON_G1_SAYDON";
		logicBoss.iCurrentHp = logicBoss.iMaximumHp = 100000u;
		logicBoss.iPatternSequence = 7u;
		logicBoss.fYawDegrees = 0.f;
		BOSS_ENCOUNTER_MADNESS_POLICY policy{};
		policy.iMaximum = 100u;
		policy.iClownHoldMs = 15000u;
		std::vector<DAMAGE_EVENT> logicEvents;
		{
			BOSS_PATTERN_DEFINITION anchoredPattern{};
			anchoredPattern.strPatternId = "KAKULSAYDON_TEST_ANCHORED_WORLD";
			BOSS_PATTERN_WORLD_SEQUENCE world{};
			world.strInstanceId = "world.sequence.instance.8";
			world.bAnchorBossSpawn = true;
			world.fAnchorPositionX = -.319f;
			world.fAnchorPositionY = 1.9f;
			world.fAnchorPositionZ = 737.531f;
			world.fPositionOffsetY = .58f;
			anchoredPattern.WorldSequences.push_back(world);
			auto anchorBoss = std::make_unique<SERVER_WORLD_ENTITY>(logicBoss);
			anchorBoss->fSpawnPositionX = -.07f;
			anchorBoss->fSpawnPositionY = 1.32f;
			anchorBoss->fSpawnPositionZ = 942.33f;
			anchorBoss->fPositionX = 99.f;
			std::map<PLAYER_ID, SERVER_PLAYER> players;
			KOUKUSAYDON_LOGIC_LEDGER ledger;
			KOUKUSAYDON_LOGIC_OUTPUT output;
			CKoukuSaydonLogicRuntime::Build(anchoredPattern, *anchorBoss, 1u, ledger);
			CKoukuSaydonLogicRuntime::Update(*anchorBoss, anchoredPattern, ledger, players,
				catalog, &policy, 1u, logicEvents, output);
			tests.Require(output.WorldSequencePlays.size() == 1u &&
				std::abs(output.WorldSequencePlays.front().fPositionOffsetX - .249f) < .001f &&
				std::abs(output.WorldSequencePlays.front().fPositionOffsetY) < .001f &&
				std::abs(output.WorldSequencePlays.front().fPositionOffsetZ - 204.799f) < .001f,
				"Anchor the roulette World cue to the owning boss spawn and preserve its authored height offset");
		}

		{
			std::uint32_t cardMask = 0u;
			bool stable = true;
			for (PLAYER_ID id = 1u; id <= 128u; ++id)
			{
				auto player = makePlayer(id, 0.f, 0.f, 0.f);
				CKoukuSaydonLogicRuntime::Assign_EncounterCard(player, logicBoss.iNetEntityId, 42u);
				const auto suit = player.eMechanicCardSymbol;
				const auto color = player.eMechanicCardColor;
				const std::uint32_t index = static_cast<std::uint32_t>(suit) - 1u +
					(MECHANIC_CARD_COLOR::BLACK == color ? 4u : 0u);
				cardMask |= 1u << index;
				player.Clear_KoukuInteractionState();
				CKoukuSaydonLogicRuntime::Assign_EncounterCard(player, logicBoss.iNetEntityId, 43u);
				stable = stable && suit == player.eMechanicCardSymbol && color == player.eMechanicCardColor;
			}
			tests.Require(0xffu == cardMask && stable,
				"Deal all eight suit/color cards on Server and preserve the chosen card across form and HUD changes");
		}


		{
			BOSS_PATTERN_DEFINITION pattern{};
			pattern.strPatternId = "KAKULSAYDON_TEST_ROULETTE";
			BOSS_PATTERN_LOGIC_WINDOW window{};
			window.strWindowId = "roulette.1";
			window.eKind = BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH;
			window.iStartMs = 0u;
			window.iDurationMs = 1000u;
			window.iSectorCount = 4u;
			window.SectorSymbols = { MECHANIC_CARD_SYMBOL::HEART, MECHANIC_CARD_SYMBOL::SPADE,
				MECHANIC_CARD_SYMBOL::CLUB, MECHANIC_CARD_SYMBOL::DIAMOND };
			window.fOuterRadiusM = 20.f;
			window.OnFail.push_back({ BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT, 50u, 0u, {} });
			pattern.LogicWindows.push_back(window);
			std::map<PLAYER_ID, SERVER_PLAYER> players;
			players.emplace(1u, makePlayer(1u, 0.f, 5.f, 0.f));   // sector 0 = HEART
			players.emplace(2u, makePlayer(2u, 5.f, 0.f, 0.f));   // sector 1 = SPADE
			players.emplace(3u, makePlayer(3u, 0.f, 50.f, 0.f));  // outside the wheel
			for (auto& [id, player] : players)
				CKoukuSaydonLogicRuntime::Assign_EncounterCard(player, logicBoss.iNetEntityId, 99u);
			KOUKUSAYDON_LOGIC_LEDGER ledger;
			CKoukuSaydonLogicRuntime::Build(pattern, logicBoss, 100u, ledger);
			KOUKUSAYDON_LOGIC_OUTPUT output;
			CKoukuSaydonLogicRuntime::Update(logicBoss, pattern, ledger, players, catalog,
				&policy, 100u, logicEvents, output);
			const bool dealt = ledger.Windows.front().bOpened &&
				MECHANIC_CARD_SYMBOL::NONE != players.at(1u).eMechanicCardSymbol &&
				MECHANIC_CARD_SYMBOL::NONE != players.at(2u).eMechanicCardSymbol;
			for (auto& [id, player] : players)
			{
				(void)id;
				player.eMechanicCardSymbol = MECHANIC_CARD_SYMBOL::HEART;
				player.eMechanicCardColor = MECHANIC_CARD_COLOR::RED;
			}
			CKoukuSaydonLogicRuntime::Update(logicBoss, pattern, ledger, players, catalog,
				&policy, 129u, logicEvents, output);
			const bool notYet = !ledger.Windows.front().bClosed &&
				0u == players.at(2u).iCurrentMadness;
			CKoukuSaydonLogicRuntime::Update(logicBoss, pattern, ledger, players, catalog,
				&policy, 130u, logicEvents, output);
			tests.Require(dealt && notYet && ledger.Windows.front().bClosed &&
				0u == players.at(1u).iCurrentMadness &&
				50u == players.at(2u).iCurrentMadness &&
				50u == players.at(3u).iCurrentMadness &&
				MECHANIC_CARD_SYMBOL::HEART == players.at(1u).eMechanicCardSymbol &&
				MECHANIC_CARD_COLOR::RED == players.at(1u).eMechanicCardColor,
				"Judge the roulette card against the sector under each player at the window end tick only");
		}

		{
			BOSS_PATTERN_DEFINITION pattern{};
			pattern.strPatternId = "KAKULSAYDON_TEST_CARD_REGIONS";
			BOSS_PATTERN_LOGIC_WINDOW window{};
			window.strWindowId = "regions.1";
			window.eKind = BOSS_PATTERN_LOGIC_KIND::ROULETTE_CARD_MATCH;
			window.iDurationMs = 1000u;
			window.OnFail.push_back({BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE,10u,0u,{}});
			window.OnTimeout.push_back({BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE,20u,0u,{}});
			for (std::uint32_t index = 0u; index < 8u; ++index)
			{
				BOSS_LOGIC_REGION region{};
				region.strRegionId = "region." + std::to_string(index);
				region.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_SPAWN;
				region.bSector = true;
				region.fRadiusM = 8.f;
				region.fHalfAngleDegrees = 22.5f;
				region.fYawDegrees = 22.5f + 45.f * index;
				region.eCardSymbol = static_cast<MECHANIC_CARD_SYMBOL>(1u + index / 2u);
				region.eCardColor = index % 2u == 0u ? MECHANIC_CARD_COLOR::RED : MECHANIC_CARD_COLOR::BLACK;
				window.CardRegions.push_back(region);
			}
			pattern.LogicWindows.push_back(window);
			auto boss = logicBoss;
			boss.fSpawnPositionX = 100.f; boss.fSpawnPositionZ = 200.f;
			boss.fPositionX = -10.f; boss.fPositionZ = -20.f;
			std::map<PLAYER_ID,SERVER_PLAYER> players;
			for (PLAYER_ID id = 1u; id <= 4u; ++id)
			{
				auto player = makePlayer(id, 102.f, 205.f, 0.f);
				player.eMechanicCardSymbol = MECHANIC_CARD_SYMBOL::HEART;
				player.eMechanicCardColor = id == 2u ? MECHANIC_CARD_COLOR::BLACK : MECHANIC_CARD_COLOR::RED;
				if (id == 3u) player.eMechanicCardSymbol = MECHANIC_CARD_SYMBOL::SPADE;
				if (id == 4u) player.fPositionZ = 250.f;
				players.emplace(id,player);
			}
			KOUKUSAYDON_LOGIC_LEDGER ledger;
			KOUKUSAYDON_LOGIC_OUTPUT output;
			CKoukuSaydonLogicRuntime::Build(pattern,boss,600u,ledger);
			CKoukuSaydonLogicRuntime::Update(boss,pattern,ledger,players,catalog,&policy,629u,logicEvents,output);
			const bool untouched = std::all_of(players.begin(),players.end(),[](const auto& pair){return pair.second.iCurrentHp == 1000u;});
			CKoukuSaydonLogicRuntime::Update(boss,pattern,ledger,players,catalog,&policy,630u,logicEvents,output);
			tests.Require(untouched && players.at(1u).iCurrentHp == 1000u && players.at(2u).iCurrentHp == 900u &&
				players.at(3u).iCurrentHp == 900u && players.at(4u).iCurrentHp == 800u &&
				ledger.Windows[0].Answers.at(4u) == KOUKUSAYDON_LOGIC_ANSWER::TIMEOUT,
				"Judge explicit roulette suit and color at the end only, anchored to spawn: correct success, wrong fail, outside timeout");
		}
		for (const auto kind : { BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP, BOSS_PATTERN_LOGIC_KIND::ENTER_AREA })
		{
			BOSS_PATTERN_DEFINITION pattern{};
			pattern.strPatternId = "KAKULSAYDON_TEST_COLLIDER_RESULT";
			BOSS_PATTERN_LOGIC_WINDOW window{};
			window.strWindowId = "geometry.1";
			window.eKind = kind;
			window.iDurationMs = 1000u;
			window.OnSuccess.push_back({BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE,10u,0u,{}});
			window.OnTimeout.push_back({BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE,20u,0u,{}});
			BOSS_LOGIC_REGION region{};
			region.strRegionId = "geometry.box";
			region.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT;
			region.fCenterZ = 5.f;
			region.fHalfX = region.fHalfZ = 1.f;
			window.CardRegions.push_back(region);
			pattern.LogicWindows.push_back(window);
			auto boss = logicBoss; boss.fPositionX = 10.f; boss.fPositionZ = 20.f; boss.fYawDegrees = 90.f;
			std::map<PLAYER_ID,SERVER_PLAYER> players;
			players.emplace(1u,makePlayer(1u,15.f,20.f,0.f));
			players.emplace(2u,makePlayer(2u,50.f,50.f,0.f));
			KOUKUSAYDON_LOGIC_LEDGER ledger; KOUKUSAYDON_LOGIC_OUTPUT output;
			CKoukuSaydonLogicRuntime::Build(pattern,boss,700u,ledger);
			CKoukuSaydonLogicRuntime::Update(boss,pattern,ledger,players,catalog,&policy,700u,logicEvents,output);
			const bool early = players.at(1u).iCurrentHp == (kind == BOSS_PATTERN_LOGIC_KIND::ENTER_AREA ? 900u : 1000u);
			CKoukuSaydonLogicRuntime::Update(boss,pattern,ledger,players,catalog,&policy,701u,logicEvents,output);
			CKoukuSaydonLogicRuntime::Update(boss,pattern,ledger,players,catalog,&policy,730u,logicEvents,output);
			tests.Require(early && players.at(1u).iCurrentHp == 900u && players.at(2u).iCurrentHp == 800u,
				"Reuse existing Result slots for linked Collider Duration or first-enter Trigger exactly once and timeout outside");
		}
		{
			auto boss = logicBoss; boss.bKoukuShieldActive = true; boss.fKoukuShieldArcDegrees = 90.f;
			boss.fKoukuShieldNormalYawOffsetDegrees = 90.f;
			tests.Require(CKoukuSaydonLogicRuntime::Is_ShieldReflected(boss,boss.fPositionX+10.f,boss.fPositionZ) &&
				!CKoukuSaydonLogicRuntime::Is_ShieldReflected(boss,boss.fPositionX,boss.fPositionZ+10.f),
				"Apply the authored shield normal yaw correction to the same visual front");
		}
		{
			BOSS_PATTERN_DEFINITION pattern{};
			pattern.strPatternId = "KAKULSAYDON_TEST_TWO_SHIELDS";
			BOSS_PATTERN_LOGIC_WINDOW window{};
			window.strWindowId = "shield.pair";
			window.eKind = BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW;
			window.iDurationMs = 1000u;
			window.iThreshold = 1000u;
			window.fShieldArcDegrees = 71.737272f;
			BOSS_LOGIC_REGION front{};
			front.strRegionId = "shield.front";
			front.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_CURRENT;
			front.bSector = true;
			front.fHalfAngleDegrees = 35.868636f;
			front.fRadiusM = 2.148847f;
			auto back = front;
			back.strRegionId = "shield.back";
			back.fCenterZ = 0.5f;
			back.fYawDegrees = 180.f;
			window.CardRegions = {front, back};
			pattern.LogicWindows.push_back(window);
			auto boss = logicBoss;
			boss.fPositionX = 10.f; boss.fPositionZ = 20.f; boss.fYawDegrees = 90.f;
			std::map<PLAYER_ID, SERVER_PLAYER> players;
			KOUKUSAYDON_LOGIC_LEDGER ledger; KOUKUSAYDON_LOGIC_OUTPUT output;
			CKoukuSaydonLogicRuntime::Build(pattern, boss, 900u, ledger);
			CKoukuSaydonLogicRuntime::Update(boss, pattern, ledger, players, catalog, &policy, 900u, logicEvents, output);
			tests.Require(boss.KoukuShieldRegions.size() == 2u &&
				CKoukuSaydonLogicRuntime::Is_ShieldReflected(boss, 30.f, 20.f) &&
				CKoukuSaydonLogicRuntime::Is_ShieldReflected(boss, -10.f, 20.f) &&
				!CKoukuSaydonLogicRuntime::Is_ShieldReflected(boss, 10.f, 30.f),
				"Reflect both authored shield directions at a translated and rotated boss, including ranged attacks");
			// The rear shield's half-metre centre shift changes this near-edge answer.
			tests.Require(CKoukuSaydonLogicRuntime::Is_ShieldReflected(boss, 10.1f, 20.25f),
				"Use the rear shield local centre rather than the boss pivot for reflection");
			const float inside = 35.f * 0.017453292519943295f;
			const float outside = 40.f * 0.017453292519943295f;
			tests.Require(CKoukuSaydonLogicRuntime::Is_ShieldReflected(boss,
				10.f + 10.f * std::cos(inside), 20.f - 10.f * std::sin(inside)) &&
				!CKoukuSaydonLogicRuntime::Is_ShieldReflected(boss,
				10.f + 10.f * std::cos(outside), 20.f - 10.f * std::sin(outside)),
				"Match the native shield mesh arc: 35 degrees reflects and 40 degrees remains open");
			CKoukuSaydonLogicRuntime::Update(boss, pattern, ledger, players, catalog, &policy, 930u, logicEvents, output);
			tests.Require(!boss.bKoukuShieldActive && boss.KoukuShieldRegions.empty() &&
				!CKoukuSaydonLogicRuntime::Is_ShieldReflected(boss, 30.f, 20.f),
				"Remove both shield regions when the stagger window closes");
		}

		{
			BOSS_PATTERN_DEFINITION pattern{};
			pattern.strPatternId = "KAKULSAYDON_TEST_CIRCLE_FAIL";
			BOSS_PATTERN_LOGIC_WINDOW window{};
			window.strWindowId = "circle.fail"; window.eKind = BOSS_PATTERN_LOGIC_KIND::AREA_OVERLAP;
			window.bInsideIsFail = true; window.iDurationMs = 1000u;
			window.OnFail.push_back({BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE,50u,0u,{}});
			window.OnFail.push_back({BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT,50u,0u,{}});
			BOSS_LOGIC_REGION region{}; region.strRegionId = "circle.1"; region.bCircle = true;
			region.fCenterX = 10.f; region.fCenterZ = 20.f; region.fRadiusM = 8.f;
			window.CardRegions.push_back(region); pattern.LogicWindows.push_back(window);
			std::map<PLAYER_ID,SERVER_PLAYER> players;
			players.emplace(1u,makePlayer(1u,10.f,20.f,0.f));
			players.emplace(2u,makePlayer(2u,20.f,20.f,0.f));
			KOUKUSAYDON_LOGIC_LEDGER ledger; KOUKUSAYDON_LOGIC_OUTPUT output;
			CKoukuSaydonLogicRuntime::Build(pattern,logicBoss,800u,ledger);
			CKoukuSaydonLogicRuntime::Update(logicBoss,pattern,ledger,players,catalog,&policy,829u,logicEvents,output);
			const bool unchanged = players.at(1u).iCurrentHp == 1000u;
			CKoukuSaydonLogicRuntime::Update(logicBoss,pattern,ledger,players,catalog,&policy,830u,logicEvents,output);
			tests.Require(unchanged && players.at(1u).iCurrentHp == 500u && players.at(1u).iCurrentMadness == 50u &&
				players.at(2u).iCurrentHp == 1000u && ledger.Windows.front().Answers.at(1u) == KOUKUSAYDON_LOGIC_ANSWER::FAIL &&
				ledger.Windows.front().Answers.at(2u) == KOUKUSAYDON_LOGIC_ANSWER::TIMEOUT,
				"Execute the authored Fail Result at Circle Duration end, including the centre, and Timeout outside");
		}
		{
			BOSS_PATTERN_DEFINITION pattern{};
			pattern.strPatternId = "KAKULSAYDON_TEST_ANIMATED_WORLD_TRIGGER";
			BOSS_PATTERN_LOGIC_WINDOW window{};
			window.strWindowId = "world.trigger"; window.eKind = BOSS_PATTERN_LOGIC_KIND::ENTER_AREA;
			window.iStartMs = 200u; window.iDurationMs = 1300u;
			window.OnSuccess.push_back({BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE,10u,0u,{}});
			window.OnTimeout.push_back({BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE,20u,0u,{}});
			BOSS_LOGIC_REGION region{}; region.strRegionId = "world.moving.circle"; region.bCircle = true;
			region.eAnchor = BOSS_LOGIC_REGION_ANCHOR::BOSS_SPAWN; region.fCenterZ = 1.f; region.fRadiusM = 1.f;
			auto& track = region.WorldTrack; track.bEnabled = true;
			track.iStartMs = 200u; track.iStartDelayMs = 100u; track.iDurationMs = 1100u;
			track.fPlaybackSpeed = 2.f; track.bSmoothStep = true;
			track.fBaselineX = 2.f; track.fBaselineZ = 3.f;
			track.fBaselineScaleX = track.fBaselineScaleY = track.fBaselineScaleZ = 2.f;
			BOSS_LOGIC_WORLD_TRANSFORM_KEY key{}; key.bVisible = false; track.Keys.push_back(key);
			key.iTimeMs = 100u; key.bVisible = true; track.Keys.push_back(key);
			key.iTimeMs = 1100u; key.fOffsetX = 10.f; key.fRotationY = key.fRotationW = 0.7071067811865475f;
			key.fScaleX = key.fScaleY = key.fScaleZ = 2.f; track.Keys.push_back(key);
			window.CardRegions.push_back(region); pattern.LogicWindows.push_back(window);
			auto boss = logicBoss; boss.fSpawnPositionX = 100.f; boss.fSpawnPositionZ = 200.f;
			std::map<PLAYER_ID,SERVER_PLAYER> players;
			players.emplace(1u,makePlayer(1u,109.12132f,205.12132f,0.f));
			players.emplace(2u,makePlayer(2u,999.f,999.f,0.f));
			KOUKUSAYDON_LOGIC_LEDGER ledger; KOUKUSAYDON_LOGIC_OUTPUT output;
			CKoukuSaydonLogicRuntime::Build(pattern,boss,1000u,ledger);
			CKoukuSaydonLogicRuntime::Update(boss,pattern,ledger,players,catalog,&policy,1006u,logicEvents,output);
			CKoukuSaydonLogicRuntime::Update(boss,pattern,ledger,players,catalog,&policy,1015u,logicEvents,output);
			const bool waiting = players.at(1u).iCurrentHp == 1000u;
			CKoukuSaydonLogicRuntime::Update(boss,pattern,ledger,players,catalog,&policy,1018u,logicEvents,output);
			const bool sweptThrough = players.at(1u).iCurrentHp == 900u;
			CKoukuSaydonLogicRuntime::Update(boss,pattern,ledger,players,catalog,&policy,1021u,logicEvents,output);
			CKoukuSaydonLogicRuntime::Update(boss,pattern,ledger,players,catalog,&policy,1045u,logicEvents,output);
			tests.Require(waiting && sweptThrough && players.at(1u).iCurrentHp == 900u && players.at(2u).iCurrentHp == 800u,
				"Sample the actual WORLD Trigger translation, quaternion, scale, visibility and clock every tick; fire damage once before its final pose");
		}

		{
			BOSS_PATTERN_DEFINITION pattern{};
			pattern.strPatternId = "KAKULSAYDON_TEST_GAZE";
			BOSS_PATTERN_LOGIC_WINDOW window{};
			window.strWindowId = "gaze.1";
			window.eKind = BOSS_PATTERN_LOGIC_KIND::GAZE_REAL_BOSS;
			window.iStartMs = 0u;
			window.iDurationMs = 1000u;
			window.fHalfAngleDegrees = 45.f;
			window.fMaxDistanceM = 30.f;
			window.OnFail.push_back({ BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH, 0u, 0u, {} });
			pattern.LogicWindows.push_back(window);
			std::map<PLAYER_ID, SERVER_PLAYER> players;
			players.emplace(1u, makePlayer(1u, 0.f, -10.f, 0.f));    // south of the boss, facing +Z toward it
			players.emplace(2u, makePlayer(2u, 0.f, -10.f, 180.f));  // same spot, back turned
			players.emplace(3u, makePlayer(3u, 10.f, 0.f, 0.f));     // east of the boss, facing +Z (90 degrees off)
			players.emplace(4u, makePlayer(4u, 0.f, -40.f, 0.f));    // facing it but beyond 30 m
			KOUKUSAYDON_LOGIC_LEDGER ledger;
			CKoukuSaydonLogicRuntime::Build(pattern, logicBoss, 200u, ledger);
			KOUKUSAYDON_LOGIC_OUTPUT output;
			CKoukuSaydonLogicRuntime::Update(logicBoss, pattern, ledger, players, catalog,
				&policy, 200u, logicEvents, output);
			CKoukuSaydonLogicRuntime::Update(logicBoss, pattern, ledger, players, catalog,
				&policy, 230u, logicEvents, output);
			tests.Require(ledger.Windows.front().bClosed &&
				1000u == players.at(1u).iCurrentHp &&
				0u == players.at(2u).iCurrentHp && PLAYER_ACTION_STATE::DEAD == players.at(2u).eAction &&
				0u == players.at(3u).iCurrentHp && 0u == players.at(4u).iCurrentHp,
				"Judge the real-boss gaze by player facing toward the boss inside the cone at the window end tick");
		}

		{
			BOSS_PATTERN_DEFINITION pattern{};
			pattern.strPatternId = "KAKULSAYDON_TEST_DANCE";
			BOSS_PATTERN_LOGIC_WINDOW window{};
			window.strWindowId = "pose.1";
			window.eKind = BOSS_PATTERN_LOGIC_KIND::POSE_INPUT;
			window.iStartMs = 0u;
			window.iDurationMs = 1000u;
			window.iPoseIndex = 2u;
			window.OnFail.push_back({ BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE, 10u, 0u, {} });
			window.OnTimeout.push_back({ BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE, 20u, 0u, {} });
			pattern.LogicWindows.push_back(window);
			std::map<PLAYER_ID, SERVER_PLAYER> players;
			players.emplace(1u, makePlayer(1u, 0.f, 0.f, 0.f));
			players.emplace(2u, makePlayer(2u, 0.f, 0.f, 0.f));
			players.emplace(3u, makePlayer(3u, 0.f, 0.f, 0.f));
			KOUKUSAYDON_LOGIC_LEDGER ledger;
			CKoukuSaydonLogicRuntime::Build(pattern, logicBoss, 300u, ledger);
			CKoukuSaydonLogicRuntime::Update_PlayerModes(players, &ledger, &policy, 300u);
			const auto slotOfPose = [](const SERVER_PLAYER& player, const std::int8_t pose)
			{
				for (std::size_t slot = 0u; slot < 4u; ++slot)
				{
					if (player.ModeSkillIndexBySlot[slot] == pose)
						return static_cast<INTERACTION_SLOT>(slot);
				}
				return INTERACTION_SLOT::END;
			};
			bool layoutDealt = true;
			for (const auto& [id, player] : players)
			{
				(void)id;
				bool seen[4] = { false, false, false, false };
				for (std::size_t slot = 0u; slot < 4u; ++slot)
				{
					const std::int8_t pose = player.ModeSkillIndexBySlot[slot];
					if (pose < 0 || pose > 3 || seen[pose] || pose != static_cast<std::int8_t>(slot))
						layoutDealt = false;
					else
						seen[pose] = true;
				}
				layoutDealt = layoutDealt && KOUKU_HUD_MODE::DANCE == player.eKoukuHudMode &&
					-1 == player.ModeSkillIndexBySlot[4];
			}
			KOUKUSAYDON_LOGIC_OUTPUT output;
			CKoukuSaydonLogicRuntime::Update(logicBoss, pattern, ledger, players, catalog,
				&policy, 300u, logicEvents, output);
			std::string answerStatus;
			const INTERACTION_SLOT rightSlot = slotOfPose(players.at(1u), 2);
			const INTERACTION_SLOT wrongSlot = slotOfPose(players.at(2u), 0);
			const bool rightRecorded = CKoukuSaydonLogicRuntime::Record_InteractionSlot(
				ledger, pattern, players.at(1u), rightSlot, answerStatus);
			const bool wrongRecorded = CKoukuSaydonLogicRuntime::Record_InteractionSlot(
				ledger, pattern, players.at(2u), wrongSlot, answerStatus);
			const bool secondAnswerRefused = !CKoukuSaydonLogicRuntime::Record_InteractionSlot(
				ledger, pattern, players.at(2u), rightSlot, answerStatus);
			CKoukuSaydonLogicRuntime::Update(logicBoss, pattern, ledger, players, catalog,
				&policy, 330u, logicEvents, output);
			tests.Require(layoutDealt && rightRecorded && wrongRecorded && secondAnswerRefused &&
				ledger.Windows.front().bClosed &&
				1000u == players.at(1u).iCurrentHp &&
				900u == players.at(2u).iCurrentHp &&
				800u == players.at(3u).iCurrentHp,
				"Keep fixed QWER pose order and judge the first answer as success, fail or timeout");
			CKoukuSaydonLogicRuntime::Update_PlayerModes(players, nullptr, &policy, 331u);
			tests.Require(KOUKU_HUD_MODE::NONE == players.at(1u).eKoukuHudMode &&
				-1 == players.at(1u).ModeSkillIndexBySlot[0],
				"Return the class HUD when no dance pattern runs");
		}

		{
			BOSS_PATTERN_DEFINITION pattern{};
			pattern.strPatternId = "KAKULSAYDON_TEST_STAGGER";
			BOSS_PATTERN_LOGIC_WINDOW window{};
			window.strWindowId = "stagger.1";
			window.eKind = BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW;
			window.iStartMs = 0u;
			window.iDurationMs = 2000u;
			window.iThreshold = 1000u;
			window.fShieldArcDegrees = 90.f;
			window.bEndsPatternOnSuccess = true;
			window.OnSuccess.push_back({ BOSS_PATTERN_LOGIC_RESULT_KIND::FOLLOWUP_PATTERN, 0u, 0u, "KAKULSAYDON_TEST_GROGGY" });
			window.OnTimeout.push_back({ BOSS_PATTERN_LOGIC_RESULT_KIND::INSTANT_DEATH, 0u, 0u, {} });
			pattern.LogicWindows.push_back(window);
			std::map<PLAYER_ID, SERVER_PLAYER> players;
			players.emplace(1u, makePlayer(1u, 0.f, 10.f, 180.f));
			SERVER_WORLD_ENTITY staggerBoss = logicBoss;
			staggerBoss.iCurrentHp = 5000u;
			KOUKUSAYDON_LOGIC_LEDGER ledger;
			CKoukuSaydonLogicRuntime::Build(pattern, staggerBoss, 400u, ledger);
			KOUKUSAYDON_LOGIC_OUTPUT output;
			CKoukuSaydonLogicRuntime::Update(staggerBoss, pattern, ledger, players, catalog,
				&policy, 400u, logicEvents, output);
			const bool shieldRaised = staggerBoss.bKoukuShieldActive &&
				CKoukuSaydonLogicRuntime::Is_ShieldReflected(staggerBoss, 0.f, 10.f) &&
				CKoukuSaydonLogicRuntime::Is_ShieldReflected(staggerBoss, 4.f, 10.f) &&
				!CKoukuSaydonLogicRuntime::Is_ShieldReflected(staggerBoss, 10.f, 0.f) &&
				!CKoukuSaydonLogicRuntime::Is_ShieldReflected(staggerBoss, 0.f, -10.f);
			staggerBoss.iCurrentHp -= 999u;
			CKoukuSaydonLogicRuntime::Update(staggerBoss, pattern, ledger, players, catalog,
				&policy, 410u, logicEvents, output);
			const bool belowThreshold = !ledger.Windows.front().bClosed && output.FollowupPatternIds.empty();
			staggerBoss.iCurrentHp -= 1u;
			CKoukuSaydonLogicRuntime::Update(staggerBoss, pattern, ledger, players, catalog,
				&policy, 411u, logicEvents, output);
			tests.Require(shieldRaised && belowThreshold && ledger.Windows.front().bClosed &&
				!staggerBoss.bKoukuShieldActive && output.bEndPatternEarly &&
				1u == output.FollowupPatternIds.size() &&
				"KAKULSAYDON_TEST_GROGGY" == output.FollowupPatternIds.front() &&
				1000u == players.at(1u).iCurrentHp,
				"Raise the frontal shield for the stagger window and hand the follow-up to the audition once the lost health reaches the threshold");

			KOUKUSAYDON_LOGIC_LEDGER timeoutLedger;
			SERVER_WORLD_ENTITY timeoutBoss = logicBoss;
			CKoukuSaydonLogicRuntime::Build(pattern, timeoutBoss, 500u, timeoutLedger);
			KOUKUSAYDON_LOGIC_OUTPUT timeoutOutput;
			CKoukuSaydonLogicRuntime::Update(timeoutBoss, pattern, timeoutLedger, players, catalog,
				&policy, 500u, logicEvents, timeoutOutput);
			CKoukuSaydonLogicRuntime::Update(timeoutBoss, pattern, timeoutLedger, players, catalog,
				&policy, 560u, logicEvents, timeoutOutput);
			tests.Require(timeoutLedger.Windows.front().bClosed && !timeoutOutput.bEndPatternEarly &&
				0u == players.at(1u).iCurrentHp,
				"Wipe the living raid when the stagger window times out");
		}

		{
			std::map<PLAYER_ID, SERVER_PLAYER> players;
			players.emplace(1u, makePlayer(1u, 0.f, 0.f, 0.f));
			SERVER_PLAYER& clown = players.at(1u);
			clown.iCurrentMadness = 60u;
			BOSS_PATTERN_LOGIC_RESULT gauge{ BOSS_PATTERN_LOGIC_RESULT_KIND::MADNESS_GAUGE_ADD_PERCENT, 50u, 0u, {} };
			CKoukuSaydonLogicRuntime::Apply_Result(clown, gauge, logicBoss, catalog, &policy, 600u, logicEvents);
			const bool transformed = PLAYER_MADNESS_FORM::CLOWN == clown.eMadnessForm &&
				0u == clown.iCurrentMadness && 0u != clown.iMadnessFormEndTick;
			CKoukuSaydonLogicRuntime::Update_PlayerModes(players, nullptr, &policy, 601u);
			const bool polymorphHud = KOUKU_HUD_MODE::POLYMORPH == clown.eKoukuHudMode &&
				0 == clown.ModeSkillIndexBySlot[0] && 1 == clown.ModeSkillIndexBySlot[1] &&
				2 == clown.ModeSkillIndexBySlot[2] && -1 == clown.ModeSkillIndexBySlot[3];
			CKoukuSaydonLogicRuntime::Update_PlayerModes(players, nullptr, &policy, 600u + 451u);
			const bool restored = PLAYER_MADNESS_FORM::NORMAL == clown.eMadnessForm &&
				KOUKU_HUD_MODE::NONE == clown.eKoukuHudMode;
			clown.eDebugKoukuHudModeOverride = KOUKU_HUD_MODE::MARIO;
			CKoukuSaydonLogicRuntime::Update_PlayerModes(players, nullptr, &policy, 1100u);
			const bool mario = KOUKU_HUD_MODE::MARIO == clown.eKoukuHudMode &&
				0 == clown.ModeSkillIndexBySlot[0] && 1 == clown.ModeSkillIndexBySlot[1] &&
				-1 == clown.ModeSkillIndexBySlot[2];
			/* Debug dance keeps the same fixed QWER order as the pattern. */
			clown.eDebugKoukuHudModeOverride = KOUKU_HUD_MODE::DANCE;
			CKoukuSaydonLogicRuntime::Update_PlayerModes(players, nullptr, &policy, 1101u);
			std::uint32_t dealtPoseMask = 0u;
			for (std::uint32_t slot = 0u; slot < 4u; ++slot)
			{
				if (clown.ModeSkillIndexBySlot[slot] >= 0 && clown.ModeSkillIndexBySlot[slot] < 4)
					dealtPoseMask |= 1u << static_cast<std::uint32_t>(clown.ModeSkillIndexBySlot[slot]);
			}
			std::int8_t firstDeal[4] = {};
			std::copy(std::begin(clown.ModeSkillIndexBySlot), std::begin(clown.ModeSkillIndexBySlot) + 4, firstDeal);
			CKoukuSaydonLogicRuntime::Update_PlayerModes(players, nullptr, &policy, 1102u);
			const bool danceParty = KOUKU_HUD_MODE::DANCE == clown.eKoukuHudMode &&
				0xFu == dealtPoseMask && -1 == clown.ModeSkillIndexBySlot[4] &&
				std::equal(std::begin(firstDeal), std::end(firstDeal), std::begin(clown.ModeSkillIndexBySlot));
			clown.eDebugKoukuHudModeOverride = KOUKU_HUD_MODE::NONE;
			CKoukuSaydonLogicRuntime::Update_PlayerModes(players, nullptr, &policy, 1103u);
			const bool overrideCleared = KOUKU_HUD_MODE::NONE == clown.eKoukuHudMode;
			clown.eKoukuAreaHudMode = KOUKU_HUD_MODE::MARIO;
			clown.eMadnessForm = PLAYER_MADNESS_FORM::CLOWN;
			clown.iMadnessFormEndTick = 1104u;
			CKoukuSaydonLogicRuntime::Update_PlayerModes(players, nullptr, &policy, 1104u);
			const bool areaSurvivesTimedForm = PLAYER_MADNESS_FORM::CLOWN == clown.eMadnessForm &&
				KOUKU_HUD_MODE::MARIO == clown.eKoukuHudMode && 0u == clown.iMadnessFormEndTick;
			tests.Require(transformed && polymorphHud && restored && mario && danceParty &&
				overrideCleared && areaSurvivesTimedForm,
				"Fill the gauge into a timed clown form, restore it, and keep Mario active when that timer ends");
		}
		{
			BOSS_PATTERN_DEFINITION pattern{};
			pattern.strPatternId = "KAKULSAYDON_TEST_TRIGGERS";
			BOSS_PATTERN_MECHANIC_TRIGGER hud{};
			hud.strTriggerId = "hud.mario";
			hud.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::HUD_ENTER;
			hud.iStartMs = 1000u;
			hud.iDurationMs = 1000u;
			hud.eHudMode = KOUKU_HUD_MODE::MARIO;
			pattern.MechanicTriggers.push_back(hud);
			BOSS_PATTERN_MECHANIC_TRIGGER teleport{};
			teleport.strTriggerId = "teleport.real";
			teleport.eKind = BOSS_PATTERN_MECHANIC_TRIGGER_KIND::REAL_GAZE_TELEPORT;
			teleport.iStartMs = 1000u;
			teleport.iDurationMs = 1000u;
			teleport.fTeleportX = -6.36f;
			teleport.fTeleportY = 1.3f;
			teleport.fTeleportZ = 937.92f;
			teleport.strClonePatternId = "KAKULSAYDON_TEST_CLONE";
			teleport.ClockHours = { 4u, 7u, 10u };
			pattern.MechanicTriggers.push_back(teleport);
			std::map<PLAYER_ID, SERVER_PLAYER> players;
			players.emplace(1u, makePlayer(1u, 0.f, 0.f, 0.f));
			KOUKUSAYDON_LOGIC_LEDGER ledger;
			CKoukuSaydonLogicRuntime::Build(pattern, logicBoss, 1200u, ledger);
			KOUKUSAYDON_LOGIC_OUTPUT output;
			CKoukuSaydonLogicRuntime::Update(logicBoss, pattern, ledger, players, catalog,
				&policy, 1229u, logicEvents, output);
			const bool pending = output.MechanicTriggers.empty() && ledger.eHudMode == KOUKU_HUD_MODE::NONE;
			CKoukuSaydonLogicRuntime::Update(logicBoss, pattern, ledger, players, catalog,
				&policy, 1230u, logicEvents, output);
			CKoukuSaydonLogicRuntime::Update_PlayerModes(players, &ledger, &policy, 1230u);
			const bool entered = KOUKU_HUD_MODE::MARIO == players.at(1u).eKoukuHudMode &&
				PLAYER_MADNESS_FORM::CLOWN == players.at(1u).eMadnessForm && output.MechanicTriggers.size() == 1u;
			output = {};
			CKoukuSaydonLogicRuntime::Update(logicBoss, pattern, ledger, players, catalog,
				&policy, 1231u, logicEvents, output);
			CKoukuSaydonLogicRuntime::Update_PlayerModes(players, nullptr, &policy, 1231u);
			tests.Require(pending && entered && output.MechanicTriggers.empty() &&
				PLAYER_MADNESS_FORM::NORMAL == players.at(1u).eMadnessForm &&
				Kouku_InteractionCooldownSkillId(KOUKU_HUD_MODE::DANCE, 0u) !=
				Kouku_InteractionCooldownSkillId(KOUKU_HUD_MODE::POLYMORPH, 0u),
				"Fire a teleport once on the pattern clock; enter Mario with a clown and restore the player at exit");
		}

	}
}
