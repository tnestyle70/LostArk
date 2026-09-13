#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "Gameplay/WorldCollisionContract.h"
#include "GameplayCatalog.h"
#include "NpcBehaviorRuntime.h"
#include "PlayerSkillSystem.h"
#include "ServerNavigation.h"
#include "ServerCollisionSystem.h"
#include "ServerTriggerSystem.h"
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

void LostArk::Server::CServerGameplayContractRunner::Run_WorldTriggers(TESTS& tests, CGameplayCatalog& catalog)
{


	{
		SERVER_PLAYER meleePlayer{};
		meleePlayer.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		meleePlayer.eStance = PLAYER_STANCE_ID::LANCE_MASTER_LONG_SPEAR;
		meleePlayer.iCurrentHp = 1000;
		meleePlayer.iMaximumHp = 1000;
		meleePlayer.iCurrentResource = 1000;
		meleePlayer.iMaximumResource = 1000;
		meleePlayer.fPositionX = 0.f;
		meleePlayer.fPositionZ = 0.f;
		SERVER_WORLD_ENTITY meleeBoss{};
		meleeBoss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
		meleeBoss.eAction = SERVER_ENTITY_ACTION::IDLE;
		meleeBoss.strArchetypeId = "BOSS_VALTAN";
		meleeBoss.iCurrentHp = 20000;
		meleeBoss.iMaximumHp = 20000;
		meleeBoss.fPositionX = 0.f;
		/* 34090 reaches 2.8 on its own; 3.5 is inside reach only because the
		boss's profile body radius extends the centre-to-centre test. */
		meleeBoss.fPositionZ = 3.5f;
		std::vector<SERVER_WORLD_ENTITY> meleeEntities{ meleeBoss };
		C2S_USE_SKILL melee{};
		melee.iClientSequence = 1;
		melee.iSkillId = 34090;
		melee.fAimX = 0.f;
		melee.fAimZ = 3.5f;
		CPlayerSkillSystem meleeSkills;
		std::vector<DAMAGE_EVENT> meleeDamageEvents;
		tests.Require(meleeSkills.Try_Start(meleePlayer, melee, catalog, 10),
			"Approve melee skill command");
		for (std::uint32_t tick = 11; tick < 60; ++tick)
		{
			meleeSkills.Update(
				meleePlayer, meleeEntities, catalog, nullptr, nullptr,
				1.f / 30.f, tick, meleeDamageEvents);
		}
		tests.Require(20000u - 10500u == meleeEntities[0].iCurrentHp,
			"Reach the boss through its collision radius");
	}

	{
		namespace fs = std::filesystem;
		const fs::path triggerRoot =
			fs::temp_directory_path() / L"LostArkWorldTriggerContractTest";
		std::error_code prepareError;
		fs::remove_all(triggerRoot, prepareError);
		fs::create_directories(triggerRoot / L"World");
		const fs::path bootstrapPath =
			triggerRoot / L"World" / L"VALTAN_ARENA.worldbootstrap";
		const auto writeTriggerBootstrap =
			[&bootstrapPath](
				const float durationSeconds,
				const std::uint32_t npcActionDurationMs = 100u,
				const float npcLookYawDegrees = 90.f,
				const std::uint32_t npcActionWeight = 1u,
				const bool referenceDisabledNpc = false,
				const char* npcActionId = "npc.ambient.look")
			{
				std::ofstream bootstrap(bootstrapPath, std::ios::binary);
				bootstrap <<
					"LOSTARK_WORLD_BOOTSTRAP\t7\tVALTAN_ARENA"
					"\tLV_LUT_HEARTRB_ED\t3\t" <<
					(referenceDisabledNpc ? 5 : 4) << "\n"
					"player.spawn.contract\tplayerSpawn\t-\t-\t0\t0\t0\t0\t1\n"
					"trigger.contract.jump\ttriggerBox\t-\t-\t0\t0\t0\t0\t1"
					"\t2\t2\t2\t0\t1\tmovePlayer\t5\t10\t0\t0\t"
					<< durationSeconds << "\t13\n"
					"collision.contract.wall\tcollisionBox\t-\t-\t4\t1\t0\t0\t1"
					"\t0.5\t1\t2\n"
					"npc.contract.ambient\tnpc\tNPC_BEDA\t-\t1\t0\t1\t0\t1"
					"\t1\tpatrol\tloop\tsequence\t1\t0\t7\t0\t0\t0\t" <<
					(referenceDisabledNpc ? "npc.contract.disabled-target" : "-") <<
					"\t2"
					"\tnpc.contract.wp.0\t1\t0\t1\t0\t1\t" <<
					npcLookYawDegrees <<
					"\tnpc.contract.wp.1\t2\t0\t1\t0\t0\t0"
					"\t1\t" << npcActionId << "\t" << npcActionDurationMs <<
					"\t0\t" << npcActionWeight << "\n";
				if (referenceDisabledNpc)
				{
					bootstrap <<
						"npc.contract.disabled-target\tnpc\tNPC_BEDA\t-"
						"\t3\t0\t1\t0\t0\t0\n";
				}
			};
		writeTriggerBootstrap(1.f);

		wchar_t previousRoot[32768]{};
		const DWORD previousLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", previousRoot,
			static_cast<DWORD>(std::size(previousRoot)));
		SetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", triggerRoot.c_str());
		CWorldBootstrap triggerBootstrap;
		const bool loadedTriggerBootstrap = triggerBootstrap.Load(
			WORLD_ID::VALTAN_ARENA);
		tests.Require(
			loadedTriggerBootstrap &&
			4u == triggerBootstrap.Get_Placements().size() &&
			WORLD_BOOTSTRAP_KIND::TRIGGER_BOX ==
				triggerBootstrap.Get_Placements()[1].eKind &&
			WORLD_BOOTSTRAP_KIND::COLLISION_BOX ==
				triggerBootstrap.Get_Placements()[2].eKind &&
			1u == triggerBootstrap.Get_Placements()[1].TriggerActions.size() &&
			triggerBootstrap.Get_Placements()[3].bHasNpcBehavior &&
			2u == triggerBootstrap.Get_Placements()[3].NpcBehavior.Waypoints.size() &&
			1u == triggerBootstrap.Get_Placements()[3].NpcBehavior.Actions.size(),
			"Parse trigger, collision and logical NPC behavior from world bootstrap v7");

		writeTriggerBootstrap(1.f, 0u);
		tests.Require(
			!triggerBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			4u == triggerBootstrap.Get_Placements().size(),
			"Reject a zero-duration NPC action without replacing committed world");

		writeTriggerBootstrap(1.f, 100u, 360.01f);
		tests.Require(
			!triggerBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			4u == triggerBootstrap.Get_Placements().size(),
			"Reject an NPC waypoint look yaw outside minus 360 to 360 degrees");

		writeTriggerBootstrap(1.f, 100u, 90.f, 100001u);
		tests.Require(
			!triggerBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			4u == triggerBootstrap.Get_Placements().size(),
			"Reject an NPC action weight above the 100000 contract maximum");

		writeTriggerBootstrap(1.f, 100u, 90.f, 1u, true);
		tests.Require(
			!triggerBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			4u == triggerBootstrap.Get_Placements().size(),
			"Reject a logical NPC look target that is disabled");

		writeTriggerBootstrap(
			1.f, 100u, 90.f, 1u, false,
			CNpcBehaviorRuntime::IDLE_ACTION_ID);
		tests.Require(
			!triggerBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			4u == triggerBootstrap.Get_Placements().size(),
			"Reject authored npc.idle because the Server runtime owns that action ID");

		writeTriggerBootstrap(
			1.f, 100u, 90.f, 1u, false,
			CNpcBehaviorRuntime::WALK_ACTION_ID);
		tests.Require(
			!triggerBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			4u == triggerBootstrap.Get_Placements().size(),
			"Reject authored npc.move.walk because the Server runtime owns that action ID");

		writeTriggerBootstrap(-1.f);
		tests.Require(
			!triggerBootstrap.Load(WORLD_ID::VALTAN_ARENA) &&
			4u == triggerBootstrap.Get_Placements().size(),
			"Reject invalid trigger bootstrap without replacing committed world");
		SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
			0u == previousLength || previousLength >= std::size(previousRoot) ?
				nullptr : previousRoot);
		std::error_code cleanupError;
		fs::remove_all(triggerRoot, cleanupError);
	}

	{
		CServerNavigation npcNavigation;
		SERVER_NAV_POINT patrolStart{};
		SERVER_NAV_POINT patrolEnd{};
		std::vector<SERVER_NAV_POINT> candidatePath;
		bool foundPatrolSegment =
			npcNavigation.Load("LV_BER_BERNCASTLE") &&
			npcNavigation.Project_Point(
				137.586334f, -22.4640217f, patrolStart);
		if (foundPatrolSegment)
		{
			foundPatrolSegment = false;
			for (int radius = 2; radius <= 20 && !foundPatrolSegment; ++radius)
			{
				for (int dz = -radius; dz <= radius && !foundPatrolSegment; ++dz)
				{
					for (int dx = -radius; dx <= radius; ++dx)
					{
						if (std::abs(dx) != radius && std::abs(dz) != radius)
							continue;
						SERVER_NAV_POINT projected{};
						if (!npcNavigation.Project_Point(
							patrolStart.x + static_cast<float>(dx) * 0.5f,
							patrolStart.z + static_cast<float>(dz) * 0.5f,
							projected))
						{
							continue;
						}
						const float distance = std::hypot(
							projected.x - patrolStart.x,
							projected.z - patrolStart.z);
						if (distance >= 1.f && npcNavigation.Find_Path(
							patrolStart.x, patrolStart.z,
							projected.x, projected.z, candidatePath))
						{
							patrolEnd = projected;
							foundPatrolSegment = true;
							break;
						}
					}
				}
			}
		}
		tests.Require(
			foundPatrolSegment,
			"Find a reachable Bern navigation segment for NPC behavior contracts");

		WORLD_BOOTSTRAP_PLACEMENT patrol{};
		patrol.strPlacementId = "npc.contract.patrol";
		patrol.strArchetypeId = "NPC_AYLARA";
		patrol.eKind = WORLD_BOOTSTRAP_KIND::NPC;
		patrol.isEnabled = true;
		patrol.bHasNpcBehavior = true;
		patrol.fPositionX = patrolStart.x;
		patrol.fPositionY = patrolStart.y;
		patrol.fPositionZ = patrolStart.z;
		patrol.NpcBehavior.eMode = NPC_BEHAVIOR_MODE::PATROL;
		patrol.NpcBehavior.eRouteMode = NPC_ROUTE_MODE::ONCE;
		patrol.NpcBehavior.eActionSelection = NPC_ACTION_SELECTION::SEQUENCE;
		patrol.NpcBehavior.fMoveSpeed = 4.f;
		patrol.NpcBehavior.iRandomSeed = 17u;
		patrol.NpcBehavior.Waypoints = {
			{ "npc.contract.wp.0", patrolStart.x, patrolStart.y,
				patrolStart.z, 0u, false, 0.f },
			{ "npc.contract.wp.1", patrolEnd.x, patrolEnd.y,
				patrolEnd.z, 0u, true, 90.f }
		};

		CNpcBehaviorRuntime npcRuntime;
		std::string npcStatus;
		CServerCollisionSystem npcCollision;
		const bool npcCollisionInitialized =
			npcCollision.Initialize({}, npcStatus);
		std::vector<WORLD_BOOTSTRAP_PLACEMENT> patrolPlacements{ patrol };
		SERVER_WORLD_ENTITY patrolEntity{};
		patrolEntity.strPlacementId = patrol.strPlacementId;
		patrolEntity.strArchetypeId = patrol.strArchetypeId;
		patrolEntity.eKind = WORLD_BOOTSTRAP_KIND::NPC;
		patrolEntity.fPositionX = patrol.fPositionX;
		patrolEntity.fPositionY = patrol.fPositionY;
		patrolEntity.fPositionZ = patrol.fPositionZ;
		bool patrolAdvanced = foundPatrolSegment && npcCollisionInitialized &&
			npcRuntime.Validate_Admission(
				patrolPlacements, npcNavigation, npcStatus) &&
			npcRuntime.Initialize(
				patrol, npcNavigation, 1u, patrolEntity, npcStatus);
		bool sawPatrolWalk = false;
		for (std::uint32_t tick = 2u;
			patrolAdvanced && tick < 2000u &&
			patrolEntity.NpcBehavior.ePhase !=
				SERVER_NPC_BEHAVIOR_PHASE::COMPLETE;
			++tick)
		{
			patrolAdvanced = npcRuntime.Update(
				patrol, nullptr, npcNavigation, npcCollision, 1.f / 30.f,
				tick, patrolEntity, npcStatus);
			sawPatrolWalk = sawPatrolWalk ||
				patrolEntity.strActionId == CNpcBehaviorRuntime::WALK_ACTION_ID;
		}
		tests.Require(
			patrolAdvanced && sawPatrolWalk &&
			patrolEntity.NpcBehavior.ePhase ==
				SERVER_NPC_BEHAVIOR_PHASE::COMPLETE &&
			std::hypot(
				patrolEntity.fPositionX - patrolEnd.x,
				patrolEntity.fPositionZ - patrolEnd.z) < 0.01f &&
			std::abs(patrolEntity.fYawDegrees - 90.f) < 0.01f,
			"Advance an authored once patrol through walk, arrival yaw and completion");

		WORLD_BOOTSTRAP_PLACEMENT loopPatrol = patrol;
		loopPatrol.strPlacementId = "npc.contract.loop";
		loopPatrol.NpcBehavior.eRouteMode = NPC_ROUTE_MODE::LOOP;
		loopPatrol.NpcBehavior.fMoveSpeed = 12.f;
		loopPatrol.NpcBehavior.Waypoints[0].iWaitMs = 100u;
		loopPatrol.NpcBehavior.Waypoints[1].iWaitMs = 0u;
		SERVER_WORLD_ENTITY loopEntity{};
		loopEntity.iNetEntityId = 7101u;
		loopEntity.strPlacementId = loopPatrol.strPlacementId;
		loopEntity.strArchetypeId = loopPatrol.strArchetypeId;
		loopEntity.eKind = WORLD_BOOTSTRAP_KIND::NPC;
		loopEntity.fPositionX = loopPatrol.fPositionX;
		loopEntity.fPositionY = loopPatrol.fPositionY;
		loopEntity.fPositionZ = loopPatrol.fPositionZ;
		bool loopAdvanced = npcRuntime.Initialize(
			loopPatrol, npcNavigation, 1u, loopEntity, npcStatus);
		std::vector<std::size_t> loopArrivalNextIndices;
		std::uint32_t firstLoopArrivalTick = 0u;
		std::uint32_t firstLoopWaitUntilTick = 0u;
		bool walkedBeforeWaitDeadline = false;
		bool walkedOnWaitDeadline = false;
		for (std::uint32_t tick = 2u;
			loopAdvanced && tick < 2000u &&
			loopArrivalNextIndices.size() < 3u; ++tick)
		{
			loopAdvanced = npcRuntime.Update(
				loopPatrol, nullptr, npcNavigation, npcCollision,
				1.f / 30.f, tick, loopEntity, npcStatus);
			if (0u != firstLoopArrivalTick &&
				tick < firstLoopWaitUntilTick &&
				loopEntity.strActionId == CNpcBehaviorRuntime::WALK_ACTION_ID)
			{
				walkedBeforeWaitDeadline = true;
			}
			if (0u != firstLoopArrivalTick &&
				tick == firstLoopWaitUntilTick &&
				loopEntity.strActionId == CNpcBehaviorRuntime::WALK_ACTION_ID)
			{
				walkedOnWaitDeadline = true;
			}
			if (loopEntity.NpcBehavior.ePhase ==
					SERVER_NPC_BEHAVIOR_PHASE::IDLE_WAIT &&
				loopEntity.strActionId == CNpcBehaviorRuntime::IDLE_ACTION_ID &&
				loopEntity.iActionStartTick == tick)
			{
				loopArrivalNextIndices.push_back(
					loopEntity.NpcBehavior.iWaypointIndex);
				if (0u == firstLoopArrivalTick)
				{
					firstLoopArrivalTick = tick;
					firstLoopWaitUntilTick =
						loopEntity.NpcBehavior.iWaitUntilTick;
				}
			}
		}
		tests.Require(
			loopAdvanced && loopArrivalNextIndices.size() == 3u &&
			loopArrivalNextIndices[0] == 1u &&
			loopArrivalNextIndices[1] == 0u &&
			loopArrivalNextIndices[2] == 1u &&
			firstLoopWaitUntilTick == firstLoopArrivalTick + 3u &&
			!walkedBeforeWaitDeadline && walkedOnWaitDeadline,
			"Loop patrol wraps waypoint indices and honors a 100ms wait for exactly three 30Hz ticks");

		WORLD_BOOTSTRAP_PLACEMENT pingPongPatrol = loopPatrol;
		pingPongPatrol.strPlacementId = "npc.contract.ping-pong";
		pingPongPatrol.NpcBehavior.eRouteMode = NPC_ROUTE_MODE::PING_PONG;
		pingPongPatrol.NpcBehavior.Waypoints = {
			{ "npc.contract.ping.0", patrolStart.x, patrolStart.y,
				patrolStart.z, 0u, false, 0.f },
			{ "npc.contract.ping.1", patrolEnd.x, patrolEnd.y,
				patrolEnd.z, 0u, false, 0.f },
			{ "npc.contract.ping.2", patrolStart.x, patrolStart.y,
				patrolStart.z, 0u, false, 0.f }
		};
		SERVER_WORLD_ENTITY pingPongEntity{};
		pingPongEntity.iNetEntityId = 7102u;
		pingPongEntity.strPlacementId = pingPongPatrol.strPlacementId;
		pingPongEntity.strArchetypeId = pingPongPatrol.strArchetypeId;
		pingPongEntity.eKind = WORLD_BOOTSTRAP_KIND::NPC;
		pingPongEntity.fPositionX = pingPongPatrol.fPositionX;
		pingPongEntity.fPositionY = pingPongPatrol.fPositionY;
		pingPongEntity.fPositionZ = pingPongPatrol.fPositionZ;
		bool pingPongAdvanced = npcRuntime.Initialize(
			pingPongPatrol, npcNavigation, 1u, pingPongEntity, npcStatus);
		std::vector<std::size_t> pingPongNextIndices;
		for (std::uint32_t tick = 2u;
			pingPongAdvanced && tick < 3000u &&
			pingPongNextIndices.size() < 4u; ++tick)
		{
			pingPongAdvanced = npcRuntime.Update(
				pingPongPatrol, nullptr, npcNavigation, npcCollision,
				1.f / 30.f, tick, pingPongEntity, npcStatus);
			if (pingPongEntity.NpcBehavior.ePhase ==
					SERVER_NPC_BEHAVIOR_PHASE::IDLE_WAIT &&
				pingPongEntity.strActionId == CNpcBehaviorRuntime::IDLE_ACTION_ID &&
				pingPongEntity.iActionStartTick == tick)
			{
				pingPongNextIndices.push_back(
					pingPongEntity.NpcBehavior.iWaypointIndex);
			}
		}
		tests.Require(
			pingPongAdvanced && pingPongNextIndices.size() == 4u &&
			pingPongNextIndices[0] == 1u &&
			pingPongNextIndices[1] == 2u &&
			pingPongNextIndices[2] == 1u &&
			pingPongNextIndices[3] == 0u &&
			!pingPongEntity.NpcBehavior.bRouteForward,
			"Ping-pong patrol reverses at the authored end instead of wrapping");

		using namespace LostArk::Shared::WorldCollision;
		const float segmentMiddleX = (patrolStart.x + patrolEnd.x) * 0.5f;
		const float segmentMiddleY = (patrolStart.y + patrolEnd.y) * 0.5f;
		const float segmentMiddleZ = (patrolStart.z + patrolEnd.z) * 0.5f;
		CServerCollisionSystem npcBodyCollision;
		const bool bodyCollisionInitialized =
			npcBodyCollision.Initialize({}, npcStatus);
		npcBodyCollision.Set_BlockingBodies({ SERVER_BLOCKING_BODY{
			segmentMiddleX, segmentMiddleZ, PLAYER_HALF_EXTENT_X,
			segmentMiddleY + PLAYER_CENTER_OFFSET_Y,
			PLAYER_HALF_EXTENT_Y, 7202u } });
		float bodyResolvedX = patrolStart.x;
		float bodyResolvedY = patrolStart.y;
		float bodyResolvedZ = patrolStart.z;
		bool bodyWasBlocked = false;
		const bool sameFloorBodyResolved = bodyCollisionInitialized &&
			npcBodyCollision.Resolve_CircleMove(
				patrolStart.x, patrolStart.y, patrolStart.z,
				patrolEnd.x, patrolEnd.y, patrolEnd.z,
				PLAYER_HALF_EXTENT_X, PLAYER_HALF_EXTENT_Y,
				PLAYER_CENTER_OFFSET_Y,
				bodyResolvedX, bodyResolvedY, bodyResolvedZ,
				bodyWasBlocked, 7201u);
		tests.Require(
			sameFloorBodyResolved &&
			std::hypot(
				bodyResolvedX - segmentMiddleX,
				bodyResolvedZ - segmentMiddleZ) >=
				PLAYER_HALF_EXTENT_X * 2.f - 0.002f,
			"Resolve a same-floor NPC circle without entering another living NPC body");

		npcBodyCollision.Set_BlockingBodies({ SERVER_BLOCKING_BODY{
			segmentMiddleX, segmentMiddleZ, PLAYER_HALF_EXTENT_X,
			segmentMiddleY + PLAYER_CENTER_OFFSET_Y + 20.f,
			PLAYER_HALF_EXTENT_Y, 7202u } });
		bodyResolvedX = patrolStart.x;
		bodyResolvedY = patrolStart.y;
		bodyResolvedZ = patrolStart.z;
		bodyWasBlocked = false;
		const bool otherFloorIgnored = npcBodyCollision.Resolve_CircleMove(
			patrolStart.x, patrolStart.y, patrolStart.z,
			patrolEnd.x, patrolEnd.y, patrolEnd.z,
			PLAYER_HALF_EXTENT_X, PLAYER_HALF_EXTENT_Y,
			PLAYER_CENTER_OFFSET_Y,
			bodyResolvedX, bodyResolvedY, bodyResolvedZ,
			bodyWasBlocked, 7201u) &&
			!bodyWasBlocked &&
			std::hypot(
				bodyResolvedX - patrolEnd.x,
				bodyResolvedZ - patrolEnd.z) < 0.001f;
		tests.Require(
			otherFloorIgnored,
			"Do not collide NPC bodies whose authoritative vertical spans are on different floors");

		npcBodyCollision.Set_BlockingBodies({ SERVER_BLOCKING_BODY{
			segmentMiddleX, segmentMiddleZ, PLAYER_HALF_EXTENT_X,
			segmentMiddleY + PLAYER_CENTER_OFFSET_Y,
			PLAYER_HALF_EXTENT_Y, 7201u } });
		bodyResolvedX = patrolStart.x;
		bodyResolvedY = patrolStart.y;
		bodyResolvedZ = patrolStart.z;
		bodyWasBlocked = false;
		tests.Require(
			npcBodyCollision.Resolve_CircleMove(
				patrolStart.x, patrolStart.y, patrolStart.z,
				patrolEnd.x, patrolEnd.y, patrolEnd.z,
				PLAYER_HALF_EXTENT_X, PLAYER_HALF_EXTENT_Y,
				PLAYER_CENTER_OFFSET_Y,
				bodyResolvedX, bodyResolvedY, bodyResolvedZ,
				bodyWasBlocked, 7201u) &&
			!bodyWasBlocked &&
			std::hypot(
				bodyResolvedX - patrolEnd.x,
				bodyResolvedZ - patrolEnd.z) < 0.001f,
			"Exclude the moving NPC from dynamic bodies by stable NetEntityId");

		WORLD_BOOTSTRAP_PLACEMENT npcWall{};
		npcWall.strPlacementId = "collision.contract.npc-wall";
		npcWall.eKind = WORLD_BOOTSTRAP_KIND::COLLISION_BOX;
		npcWall.isEnabled = true;
		npcWall.fPositionX = segmentMiddleX;
		npcWall.fPositionY = segmentMiddleY + PLAYER_CENTER_OFFSET_Y;
		npcWall.fPositionZ = segmentMiddleZ;
		npcWall.fHalfExtentX = 0.1f;
		npcWall.fHalfExtentY = 2.f;
		npcWall.fHalfExtentZ = 0.1f;
		CServerCollisionSystem npcWallCollision;
		const bool wallCollisionInitialized =
			npcWallCollision.Initialize({ npcWall }, npcStatus);
		float wallResolvedX = patrolStart.x;
		float wallResolvedY = patrolStart.y;
		float wallResolvedZ = patrolStart.z;
		bool wallWasBlocked = false;
		tests.Require(
			wallCollisionInitialized &&
			npcWallCollision.Resolve_CircleMove(
				patrolStart.x, patrolStart.y, patrolStart.z,
				patrolEnd.x, patrolEnd.y, patrolEnd.z,
				PLAYER_HALF_EXTENT_X, PLAYER_HALF_EXTENT_Y,
				PLAYER_CENTER_OFFSET_Y,
				wallResolvedX, wallResolvedY, wallResolvedZ,
				wallWasBlocked, 7201u) && wallWasBlocked &&
			std::hypot(
				wallResolvedX - patrolEnd.x,
				wallResolvedZ - patrolEnd.z) > 0.01f,
			"Stop an NPC circle at an authoritative collisionBox instead of crossing it");

		WORLD_BOOTSTRAP_PLACEMENT unreachable = patrol;
		unreachable.strPlacementId = "npc.contract.unreachable";
		unreachable.NpcBehavior.Waypoints[1].fPositionX = 100000.f;
		tests.Require(
			!npcRuntime.Validate_Admission(
				std::vector<WORLD_BOOTSTRAP_PLACEMENT>{ unreachable },
				npcNavigation, npcStatus),
			"Reject an NPC patrol waypoint outside authoritative navigation");

		WORLD_BOOTSTRAP_PLACEMENT ambient = patrol;
		ambient.strPlacementId = "npc.contract.ambient-runtime";
		ambient.NpcBehavior.eMode = NPC_BEHAVIOR_MODE::STATIONARY;
		ambient.NpcBehavior.eRouteMode = NPC_ROUTE_MODE::LOOP;
		ambient.NpcBehavior.Waypoints.clear();
		ambient.NpcBehavior.Actions = {
			{ "npc.ambient.look", 34u, 0u, 1u },
			{ "npc.ambient.greet", 34u, 0u, 1u }
		};
		CServerNavigation unloadedNpcNavigation;
		tests.Require(
			npcRuntime.Validate_Admission(
				std::vector<WORLD_BOOTSTRAP_PLACEMENT>{ ambient },
				unloadedNpcNavigation, npcStatus) &&
			!npcRuntime.Validate_Admission(
				std::vector<WORLD_BOOTSTRAP_PLACEMENT>{ patrol },
				unloadedNpcNavigation, npcStatus),
			"Allow stationary ambient NPCs without navigation but reject moving behaviors");
		SERVER_WORLD_ENTITY ambientEntity{};
		ambientEntity.strPlacementId = ambient.strPlacementId;
		ambientEntity.strArchetypeId = ambient.strArchetypeId;
		ambientEntity.eKind = WORLD_BOOTSTRAP_KIND::NPC;
		ambientEntity.fPositionX = ambient.fPositionX;
		ambientEntity.fPositionY = ambient.fPositionY;
		ambientEntity.fPositionZ = ambient.fPositionZ;
		bool ambientAdvanced = npcRuntime.Initialize(
			ambient, npcNavigation, 1u, ambientEntity, npcStatus);
		bool sawLook = false;
		bool sawGreet = false;
		std::uint32_t firstLookTick = 0u;
		std::uint32_t repeatedLookTick = 0u;
		for (std::uint32_t tick = 2u; ambientAdvanced && tick < 40u; ++tick)
		{
			ambientAdvanced = npcRuntime.Update(
				ambient, nullptr, npcNavigation, npcCollision, 1.f / 30.f,
				tick, ambientEntity, npcStatus);
			if (ambientEntity.strActionId == "npc.ambient.look")
			{
				sawLook = true;
				if (0u == firstLookTick)
					firstLookTick = ambientEntity.iActionStartTick;
				else if (firstLookTick != ambientEntity.iActionStartTick)
					repeatedLookTick = ambientEntity.iActionStartTick;
			}
			if (ambientEntity.strActionId == "npc.ambient.greet")
				sawGreet = true;
		}
		tests.Require(
			ambientAdvanced && sawLook && sawGreet &&
			0u != repeatedLookTick,
			"Sequence ambient actions and restart the same clip through actionStartTick edges");

		WORLD_BOOTSTRAP_PLACEMENT wander = ambient;
		wander.strPlacementId = "npc.contract.wander";
		wander.NpcBehavior.eMode = NPC_BEHAVIOR_MODE::WANDER;
		wander.NpcBehavior.eActionSelection = NPC_ACTION_SELECTION::WEIGHTED;
		wander.NpcBehavior.fWanderRadius = 3.f;
		wander.NpcBehavior.iRandomSeed = 991u;
		wander.NpcBehavior.Actions.clear();
		std::vector<SERVER_NAV_POINT> boundedWanderPath;
		const bool foundBoundedWanderPath =
			npcNavigation.Find_PathToReachablePointWithinRadius(
				patrolStart.x, patrolStart.z,
				patrolStart.x, patrolStart.z, 3.f, 0.05f,
				boundedWanderPath);
		const bool boundedWanderPathStayedInside =
			foundBoundedWanderPath && !boundedWanderPath.empty() &&
			std::all_of(
				boundedWanderPath.begin(), boundedWanderPath.end(),
				[&patrolStart](const SERVER_NAV_POINT& point)
				{
					return std::hypot(
						point.x - patrolStart.x,
						point.z - patrolStart.z) <= 3.001f;
				});
		tests.Require(
			boundedWanderPathStayedInside,
			"Find a deterministic nontrivial wander path whose every cell stays inside the authored radius");
		SERVER_WORLD_ENTITY wanderA{};
		wanderA.strPlacementId = wander.strPlacementId;
		wanderA.strArchetypeId = wander.strArchetypeId;
		wanderA.eKind = WORLD_BOOTSTRAP_KIND::NPC;
		wanderA.fPositionX = wander.fPositionX;
		wanderA.fPositionY = wander.fPositionY;
		wanderA.fPositionZ = wander.fPositionZ;
		SERVER_WORLD_ENTITY wanderB = wanderA;
		bool deterministicWander = npcRuntime.Initialize(
			wander, npcNavigation, 1u, wanderA, npcStatus) &&
			npcRuntime.Initialize(
				wander, npcNavigation, 1u, wanderB, npcStatus);
		bool sawWanderWalk = false;
		float maximumWanderDisplacement = 0.f;
		for (std::uint32_t tick = 2u;
			deterministicWander && tick < 300u; ++tick)
		{
				deterministicWander = npcRuntime.Update(
				wander, nullptr, npcNavigation, npcCollision, 1.f / 30.f,
				tick, wanderA, npcStatus) &&
				npcRuntime.Update(
					wander, nullptr, npcNavigation, npcCollision, 1.f / 30.f,
					tick, wanderB, npcStatus) &&
				wanderA.fPositionX == wanderB.fPositionX &&
				wanderA.fPositionZ == wanderB.fPositionZ &&
				std::hypot(
					wanderA.fPositionX - wanderA.fSpawnPositionX,
					wanderA.fPositionZ - wanderA.fSpawnPositionZ) <= 3.001f;
			sawWanderWalk = sawWanderWalk ||
				wanderA.strActionId == CNpcBehaviorRuntime::WALK_ACTION_ID;
			maximumWanderDisplacement = (std::max)(
				maximumWanderDisplacement,
				std::hypot(
					wanderA.fPositionX - wanderA.fSpawnPositionX,
					wanderA.fPositionZ - wanderA.fSpawnPositionZ));
		}
		tests.Require(
			deterministicWander && sawWanderWalk &&
			maximumWanderDisplacement > 0.1f,
			"Keep seeded NPC wander deterministic, walking, displaced and inside its authored radius");

		WORLD_BOOTSTRAP_PLACEMENT trivialWander = wander;
		trivialWander.strPlacementId = "npc.contract.wander-trivial";
		trivialWander.NpcBehavior.fWanderRadius = 0.001f;
		tests.Require(
			!npcRuntime.Validate_Admission(
				std::vector<WORLD_BOOTSTRAP_PLACEMENT>{ trivialWander },
				npcNavigation, npcStatus),
			"Reject wander admission when no nontrivial reachable destination exists");

		SERVER_WORLD_ENTITY exhaustedWander{};
		exhaustedWander.iNetEntityId = 7103u;
		exhaustedWander.strPlacementId = wander.strPlacementId;
		exhaustedWander.strArchetypeId = wander.strArchetypeId;
		exhaustedWander.eKind = WORLD_BOOTSTRAP_KIND::NPC;
		exhaustedWander.fPositionX = wander.fPositionX;
		exhaustedWander.fPositionY = wander.fPositionY;
		exhaustedWander.fPositionZ = wander.fPositionZ;
		const bool initializedExhaustedWander = npcRuntime.Initialize(
			wander, npcNavigation, 1u, exhaustedWander, npcStatus);
		WORLD_BOOTSTRAP_PLACEMENT exhaustedDescriptor = wander;
		exhaustedDescriptor.NpcBehavior.fWanderRadius = 0.001f;
		tests.Require(
			initializedExhaustedWander &&
			!npcRuntime.Update(
				exhaustedDescriptor, nullptr, npcNavigation, npcCollision,
				1.f / 30.f, 2u, exhaustedWander, npcStatus) &&
			npcStatus.find("destination became unavailable") != std::string::npos,
			"Surface exhausted wander destination search instead of reporting a successful permanent idle");
	}

	{
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "trigger.contract.jump";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = false;
		WORLD_TRIGGER_ACTION move{};
		move.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
		move.fTargetX = 10.f;
		move.fTargetY = 0.f;
		move.fTargetZ = 0.f;
		move.fDurationSeconds = 1.f;
		move.fArcHeight = 4.f;
		move.eKoukuHudModeOnArrival = KOUKU_HUD_MODE::MARIO;
		trigger.TriggerActions.push_back(move);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus) &&
			1u == triggerSystem.Get_TriggerCount(),
			"Initialize enabled movePlayer trigger");
		std::map<PLAYER_ID, SERVER_PLAYER> triggerPlayers;
		SERVER_PLAYER triggerPlayer{};
		triggerPlayer.iPlayerId = 1;
		triggerPlayer.fPositionX = 2.4f;
		triggerPlayer.iCurrentHp = 100;
		triggerPlayer.iMaximumHp = 100;
		triggerPlayers.emplace(1, triggerPlayer);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::vector<SERVER_INTERACT_PROMPT_EDGE> promptEdges;
		triggerSystem.Evaluate_Entries(triggerPlayers, 10, transfers, {}, promptEdges);
		tests.Require(
			PLAYER_ACTION_STATE::TRIGGER_MOVE ==
				triggerPlayers.begin()->second.eAction &&
			triggerPlayers.begin()->second.TriggerMove.isActive &&
			10u == triggerPlayers.begin()->second.iActionStartTick,
			"Fire trigger on OBB entry");
		triggerSystem.Update_PlayerMotion(triggerPlayers.begin()->second, 0.5f);
		tests.Require(
			std::abs(triggerPlayers.begin()->second.fPositionX - 6.2f) < 0.001f &&
			std::abs(triggerPlayers.begin()->second.fPositionY - 4.f) < 0.001f &&
			PLAYER_MADNESS_FORM::NORMAL == triggerPlayers.begin()->second.eMadnessForm,
			"Advance movePlayer with authored parabolic arc");
		triggerSystem.Update_PlayerMotion(triggerPlayers.begin()->second, 0.5f);
		tests.Require(
			std::abs(triggerPlayers.begin()->second.fPositionX - 10.f) < 0.001f &&
			std::abs(triggerPlayers.begin()->second.fPositionY) < 0.001f &&
			PLAYER_ACTION_STATE::NONE == triggerPlayers.begin()->second.eAction &&
			!triggerPlayers.begin()->second.TriggerMove.isActive &&
			KOUKU_HUD_MODE::MARIO == triggerPlayers.begin()->second.eKoukuAreaHudMode &&
			PLAYER_MADNESS_FORM::CLOWN == triggerPlayers.begin()->second.eMadnessForm,
			"Complete movePlayer at exact authored destination");
		triggerSystem.Evaluate_Entries(triggerPlayers, 11, transfers, {}, promptEdges);
		triggerPlayers.begin()->second.fPositionX = 0.f;
		triggerSystem.Evaluate_Entries(triggerPlayers, 12, transfers, {}, promptEdges);
		tests.Require(
			PLAYER_ACTION_STATE::TRIGGER_MOVE ==
				triggerPlayers.begin()->second.eAction &&
			12u == triggerPlayers.begin()->second.iActionStartTick,
			"Rearm non-once trigger after player exits");
	}

	{
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "trigger.contract.change-level";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION changeLevel{};
		changeLevel.eKind = WORLD_TRIGGER_ACTION_KIND::CHANGE_LEVEL;
		changeLevel.eTargetWorldId = WORLD_ID::VALTAN_ARENA;
		trigger.TriggerActions.push_back(changeLevel);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus),
			"Initialize enabled changeLevel trigger");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iSessionId = 7;
		player.iPlayerId = 3;
		player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
		player.strNickName = "TriggerTransfer";
		player.iCurrentHp = 100;
		player.iMaximumHp = 100;
		players.emplace(player.iPlayerId, player);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::vector<SERVER_INTERACT_PROMPT_EDGE> promptEdges;
		triggerSystem.Evaluate_Entries(players, 20, transfers, {}, promptEdges);
		tests.Require(
			1u == transfers.size() &&
			7u == transfers.front().iSessionId &&
			WORLD_ID::VALTAN_ARENA == transfers.front().eTargetWorldId &&
			CHARACTER_CLASS_ID::LANCE_MASTER ==
				transfers.front().eCharacterClass &&
			"TriggerTransfer" == transfers.front().strNickName,
			"Emit one typed Server world transfer request on OBB entry");
		triggerSystem.Evaluate_Entries(players, 21, transfers, {}, promptEdges);
			tests.Require(
			transfers.empty(),
			"Do not repeat a triggerOnce world transfer while occupied");
	}

	{
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "trigger.contract.activate-spawn-group";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION activate{};
		activate.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP;
		activate.strTargetId = "spawn.valtan.stage01";
		trigger.TriggerActions.push_back(activate);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus),
			"Initialize enabled activateSpawnGroup trigger");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iPlayerId = 4;
		player.iCurrentHp = 100;
		player.iMaximumHp = 100;
		players.emplace(player.iPlayerId, player);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::vector<SERVER_INTERACT_PROMPT_EDGE> promptEdges;
		std::size_t activationCount = 0u;
		triggerSystem.Evaluate_Entries(
			players,
			30,
			transfers,
			[&activationCount](
				WORLD_TRIGGER_ACTION_KIND kind,
				const std::string& targetId)
			{
				if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP != kind ||
					"spawn.valtan.stage01" != targetId)
				{
					return false;
				}
				++activationCount;
				return true;
			},
				promptEdges);
		tests.Require(
			1u == activationCount && transfers.empty(),
			"Dispatch typed activateSpawnGroup target on OBB entry");
		triggerSystem.Evaluate_Entries(
			players,
			31,
			transfers,
			[&activationCount](WORLD_TRIGGER_ACTION_KIND, const std::string&)
			{
				++activationCount;
				return true;
			},
				promptEdges);
		tests.Require(
			1u == activationCount,
			"Do not repeat a triggerOnce spawn-group activation while occupied");
	}

	{
		/* Stage_1 is the stage whose wave is being built, so it runs its real
		activateSpawnGroup action in Debug as well. The later stages keep the
		shortcut, which is what still carries boss work to Valtan without
		clearing the corridor. Stage_MiniBoss stands in for those here. */
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "Stage_MiniBoss";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION activate{};
		activate.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP;
		activate.strTargetId = "spawn.valtan.stage02.miniboss";
		trigger.TriggerActions.push_back(activate);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus, true),
			"Initialize the Debug Valtan stage-route bypass");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iPlayerId = 404u;
		player.iCurrentHp = 100u;
		player.iMaximumHp = 100u;
		players.emplace(player.iPlayerId, player);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::vector<SERVER_INTERACT_PROMPT_EDGE> promptEdges;
		std::size_t activationCount = 0u;
		triggerSystem.Evaluate_Entries(
			players, 41u, transfers,
			[&activationCount](WORLD_TRIGGER_ACTION_KIND, const std::string&)
			{
				++activationCount;
				return true;
			},
				promptEdges);
#ifdef _DEBUG
		const SERVER_PLAYER& moving = players.begin()->second;
		tests.Require(
			0u == activationCount &&
			PLAYER_ACTION_STATE::TRIGGER_MOVE == moving.eAction &&
			moving.TriggerMove.isActive &&
			std::abs(moving.TriggerMove.fTargetX - 86.110f) < 0.001f &&
			std::abs(moving.TriggerMove.fTargetZ + 93.033f) < 0.001f,
			"Bypass a later Valtan stage group and move toward the next trigger in Debug");
		triggerSystem.Update_PlayerMotion(players.begin()->second, 1.f);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == players.begin()->second.eAction &&
			std::abs(players.begin()->second.fPositionX - 86.110f) < 0.001f &&
			std::abs(players.begin()->second.fPositionZ + 93.033f) < 0.001f,
			"Complete the Debug stage bypass at the authored next-stage approach point");
#else
		tests.Require(
			1u == activationCount &&
			PLAYER_ACTION_STATE::NONE == players.begin()->second.eAction,
			"Keep the original Valtan spawn-group trigger unchanged in Release");
#endif
	}

	{
		/* Stage_1 is exempt from the Debug shortcut on purpose: it is the wave
		being built, so stepping into it has to run the real activation even with
		the bypass switched on. Without this the corridor's first fight is
		unreachable in a Debug session. */
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "Stage_1";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION activate{};
		activate.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP;
		activate.strTargetId = "spawn.valtan.stage01";
		trigger.TriggerActions.push_back(activate);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus, true),
			"Initialize the Debug bypass with Stage_1 present");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iPlayerId = 405u;
		player.iCurrentHp = 100u;
		player.iMaximumHp = 100u;
		players.emplace(player.iPlayerId, player);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::vector<SERVER_INTERACT_PROMPT_EDGE> promptEdges;
		std::string activatedTargetId;
		std::size_t activationCount = 0u;
		triggerSystem.Evaluate_Entries(
			players, 42u, transfers,
			[&activationCount, &activatedTargetId](
				const WORLD_TRIGGER_ACTION_KIND kind, const std::string& targetId)
			{
				if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP != kind)
					return false;
				++activationCount;
				activatedTargetId = targetId;
				return true;
			},
				promptEdges);
		tests.Require(
			1u == activationCount &&
			"spawn.valtan.stage01" == activatedTargetId &&
			PLAYER_ACTION_STATE::NONE == players.begin()->second.eAction,
			"Activate the Stage_1 wave instead of bypassing it, Debug included");
	}

	{
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "trigger.contract.activate-encounter";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION activate{};
		activate.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER;
		activate.strTargetId = "boss.valtan.center";
		trigger.TriggerActions.push_back(activate);

		CServerTriggerSystem triggerSystem;
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus),
			"Initialize enabled activateEncounter trigger");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iPlayerId = 5;
		player.iCurrentHp = 100;
		player.iMaximumHp = 100;
		players.emplace(player.iPlayerId, player);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::vector<SERVER_INTERACT_PROMPT_EDGE> promptEdges;
		std::size_t activationCount = 0u;
		triggerSystem.Evaluate_Entries(
			players,
			40,
			transfers,
			[&activationCount](
				WORLD_TRIGGER_ACTION_KIND kind,
				const std::string& targetId)
			{
				if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER != kind ||
					"boss.valtan.center" != targetId)
				{
					return false;
				}
				++activationCount;
				return true;
			},
				promptEdges);
		tests.Require(
			1u == activationCount && transfers.empty(),
			"Dispatch typed activateEncounter target on OBB entry");
	}

	{
		/* A bootstrap whose skill cost exceeds every class pool must fail load:
		the publisher enforces the same bound, so acceptance here would mean the
		two sides disagree about the same document. */
		namespace fs = std::filesystem;
		const fs::path overCostRoot =
			fs::temp_directory_path() / L"LostArkBalanceContractTest";
		std::error_code prepareError;
		fs::remove_all(overCostRoot, prepareError);
		fs::create_directories(overCostRoot / L"Gameplay");
		{
			std::ofstream bootstrap(
				overCostRoot / L"Gameplay" / L"Gameplay.bootstrap",
				std::ios::binary);
			bootstrap <<
				"LOSTARK_GAMEPLAY_BOOTSTRAP\t" << GAMEPLAY_BOOTSTRAP_VERSION <<
				"\t20\n"
				"PATTERNPRESENTATIONGENERATION\tENCOUNTER_VALTAN\t1111111111111111111111111111111111111111111111111111111111111111\n"
				"BOSS\tBOSS_VALTAN\tENCOUNTER_VALTAN\t60000\t160\t100\t3\t20\t2.6\tHEALTH_PERCENT_THRESHOLD\t50\n"
				"BOSSPART\tBOSS_VALTAN\tboss.part.valtan.arm-armor\t2\t1000\t15\tGROGGY_ONLY\n"
				"DAMAGE\tdamage.player.34120\t361\n"
				"PATTERN\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test\tNORMAL\t1\t160\t0\t0\t1\t1\t0\t8\t2\tANY\tANY\t0\n"
				"PATTERNPOLICY\tENCOUNTER_VALTAN\tVALTAN_TEST\tNORMAL\t1\t3\tLOCK_NEAREST_ON_START\tLOCK_FACING_ON_START\n"
				"PATTERNSOURCE\tENCOUNTER_VALTAN\tVALTAN_TEST\t420601\t12\t5000\t150\t350\t300\t180\n"
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t0\tACTIVE\tvaltan.test.active\tACTIVE\t1000\tCIRCLE\t8\t0\t0\t0\t0\t1\t0\t0\tdamage.player.34120\t2\t242\t1\t2000\n"
				"PATTERNSTAGE\tENCOUNTER_VALTAN\tVALTAN_TEST\t1\tSPAWN\tvaltan.test.spawn\tWINDUP\t500\tNONE\t0\t0\t0\t0\t0\t0\t0\t0\t-\t0\t0\t0\t0\n"
				"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.active\tTIMEOUT\tvaltan.test.spawn\n"
				"PATTERNSTAGEBRANCH\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\tTIMEOUT\t-\n"
				"BOSSCOMBATOBJECT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\tcombatobject.visual.valtan.test.v1\tVALTAN_TEST\tvaltan.test.spawn\tFIXED_AREA\tLOCKED_TARGET_UNTIL_FIRST_PULSE\tNONE\t0\t0\t0\t0\t1000\t1\n"
				"BOSSCOMBATOBJECTHIT\tENCOUNTER_VALTAN\tcombatobject.valtan.test\t0\thit.valtan.test.01\tTIMED\t100\t1\t0\tCIRCLE\t4\t0\t0\t0\t0\tdamage.player.34120\t0\t0\t0\t0\n"
				"PATTERNSTAGEACTION\tENCOUNTER_VALTAN\tVALTAN_TEST\tvaltan.test.spawn\t0\tENTER\tSPAWN_COMBAT_OBJECT\tcombatobject.valtan.test\t1\t0\n"
				"PLAYER\tLANCE_MASTER\t5500\t1000\t25\t100\t105\t2.95\t1\t0\t0\t0\t0\t0\tLANCE_MASTER_LONG_SPEAR\n"
				"SKILL\t34120\tLANCE_MASTER\tQ\tlancemaster.skill.34120\t10000\t2266"
				"\t1510\t2000\t0\t0\t8\tdamage.player.34120\tACTIVE\tLANCE_MASTER_LONG_SPEAR\tNONE\n"
				"SKILLCOMBATTRAITS\t34120\t0\t0\t0\n";
			Write_ValidValtanTimelineRows(bootstrap);
		}
		wchar_t previousRoot[32768]{};
		const DWORD previousLength = GetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", previousRoot,
			static_cast<DWORD>(std::size(previousRoot)));
		CGameplayCatalog rollbackCatalog;
		tests.Require(rollbackCatalog.Load(),
			"Stage a valid gameplay catalog before rollback test");
		const auto* committedPatterns =
			rollbackCatalog.Find_BossPatterns("ENCOUNTER_VALTAN");
		const size_t committedPatternCount =
			nullptr != committedPatterns ? committedPatterns->size() : 0u;
		const VALTAN_TIMELINE_DEFINITION* committedTimeline =
			rollbackCatalog.Find_ValtanTimeline("ENCOUNTER_VALTAN");
		const size_t committedTimelineRowCount =
			nullptr != committedTimeline ? committedTimeline->Rows.size() : 0u;
		const std::string committedIntroPatternId =
			rollbackCatalog.Find_IntroPatternId("ENCOUNTER_VALTAN");
		SetEnvironmentVariableW(
			L"LOSTARK_SERVER_DATA_ROOT", overCostRoot.c_str());
		CGameplayCatalog overCostCatalog;
		tests.Require(!overCostCatalog.Load(),
			"Reject bootstrap skill cost above every class pool");
		tests.Require(
			!rollbackCatalog.Load() &&
			nullptr != rollbackCatalog.Find_Skill(34010) &&
			nullptr != rollbackCatalog.Find_BossPatterns("ENCOUNTER_VALTAN") &&
			0u != committedPatternCount &&
			committedPatternCount ==
				rollbackCatalog.Find_BossPatterns("ENCOUNTER_VALTAN")->size() &&
			nullptr != rollbackCatalog.Find_ValtanTimeline(
				"ENCOUNTER_VALTAN") &&
			52u == committedTimelineRowCount &&
			committedTimelineRowCount ==
				rollbackCatalog.Find_ValtanTimeline(
					"ENCOUNTER_VALTAN")->Rows.size() &&
			!committedIntroPatternId.empty() &&
			committedIntroPatternId == rollbackCatalog.Find_IntroPatternId(
				"ENCOUNTER_VALTAN"),
			"Preserve the committed catalog after a corrupt replacement fails");
		SetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT",
			0u == previousLength || previousLength >= std::size(previousRoot) ?
				nullptr : previousRoot);
		std::error_code cleanupError;
		fs::remove_all(overCostRoot, cleanupError);
	}
}

