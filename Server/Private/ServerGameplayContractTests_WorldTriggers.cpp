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
					"LOSTARK_WORLD_BOOTSTRAP\t8\tVALTAN_ARENA"
					"\tLV_LUT_HEARTRB_ED\t3\t" <<
					(referenceDisabledNpc ? 5 : 4) << "\n"
					"player.spawn.contract\tplayerSpawn\t-\t-\t0\t0\t0\t0\t1\n"
					"trigger.contract.jump\ttriggerBox\t-\t-\t0\t0\t0\t0\t1"
					"\t2\t2\t2\t0\t1\t0\tmovePlayer\t5\t10\t0\t0\t"
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
			"Parse trigger, collision and logical NPC behavior from world bootstrap v8");

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
		trigger.strPlacementId = "movement.contract.arc";
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
		triggerSystem.Set_WorldId(WORLD_ID::KAKULSAYDON_ARENA);
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
		triggerSystem.Evaluate_Entries(triggerPlayers, 9, transfers, {}, promptEdges);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == triggerPlayers.begin()->second.eAction &&
			!triggerPlayers.begin()->second.TriggerMove.isActive &&
			1u == promptEdges.size() && promptEdges.front().bAvailable &&
			promptEdges.front().strTriggerPlacementId == trigger.strPlacementId,
			"A movement box only offers itself when the player steps into its OBB");
		tests.Require(
			1u == triggerSystem.Activate_Here(1u, triggerPlayers, 10, transfers, {}) &&
			PLAYER_ACTION_STATE::TRIGGER_MOVE ==
				triggerPlayers.begin()->second.eAction &&
			triggerPlayers.begin()->second.TriggerMove.isActive &&
			10u == triggerPlayers.begin()->second.iActionStartTick,
			"G inside the OBB fires the movement box");
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
		tests.Require(
			0u == triggerSystem.Activate_Here(1u, triggerPlayers, 12, transfers, {}),
			"A second G press inside the per-player debounce runs nothing");
		tests.Require(
			1u == triggerSystem.Activate_Here(1u, triggerPlayers, 20, transfers, {}) &&
			PLAYER_ACTION_STATE::TRIGGER_MOVE ==
				triggerPlayers.begin()->second.eAction &&
			20u == triggerPlayers.begin()->second.iActionStartTick,
			"A non-once movement box fires again for the same player after the debounce");
	}

	{
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "Mario0_Contract_RoomOwned";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.isTriggerOnce = true;
		trigger.fHalfExtentX = trigger.fHalfExtentY = trigger.fHalfExtentZ = 1.f;
		WORLD_TRIGGER_ACTION move{};
		move.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
		move.fTargetX = 8.f;
		move.fDurationSeconds = 0.6f;
		move.fArcHeight = 1.5f;
		trigger.TriggerActions.push_back(move);
		CServerTriggerSystem triggerSystem;
		triggerSystem.Set_HonourTriggerOnce(true);
		triggerSystem.Set_WorldId(WORLD_ID::KAKULSAYDON_ARENA);
		std::string status;
		tests.Require(triggerSystem.Initialize({trigger}, status),
			"Initialize room-owned movement entry fixture");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		auto& player = players[1u];
		player.iPlayerId = 1u;
		player.iCurrentHp = player.iMaximumHp = 100u;
		player.eAction = PLAYER_ACTION_STATE::SKILL;
		player.iCurrentSkillId = 34090u;
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::vector<SERVER_INTERACT_PROMPT_EDGE> prompts;
		unsigned attempts = 0u;
		bool admit = false;
		const SERVER_TRIGGER_MOVE_ENTRY_HANDLER roomEntry =
			[&](const WORLD_BOOTSTRAP_PLACEMENT& placement, SERVER_PLAYER& target, const std::uint32_t tick)
		{
			++attempts;
			if (!admit) return SERVER_TRIGGER_MOVE_ENTRY_RESULT::RETRY_WHILE_INSIDE;
			SERVER_PLAYER candidate = target;
			candidate.eAction = PLAYER_ACTION_STATE::NONE;
			if (!CServerTriggerSystem::Begin_MovePlayer(candidate, placement.TriggerActions.front(), tick))
				return SERVER_TRIGGER_MOVE_ENTRY_RESULT::RETRY_WHILE_INSIDE;
			target = std::move(candidate);
			return SERVER_TRIGGER_MOVE_ENTRY_RESULT::STARTED;
		};
		(void)triggerSystem.Activate_Interact(1u, trigger.strPlacementId, players, 10u,
			transfers, {}, roomEntry);
		(void)triggerSystem.Activate_Interact(1u, trigger.strPlacementId, players, 11u,
			transfers, {}, roomEntry);
		tests.Require(attempts == 2u && player.eAction == PLAYER_ACTION_STATE::SKILL &&
			player.iCurrentSkillId == 34090u && !player.TriggerMove.isActive,
			"Rejected owned move remains eligible while overlapping without changing player action");
		admit = true;
		tests.Require(triggerSystem.Activate_Interact(1u, trigger.strPlacementId, players, 12u,
				transfers, {}, roomEntry),
			"A room-admitted G press reports the movement as fired");
		tests.Require(attempts == 3u && player.eAction == PLAYER_ACTION_STATE::TRIGGER_MOVE &&
			player.TriggerMove.isActive && player.iActionStartTick == 12u &&
			player.TriggerMove.strSourcePlacementId == trigger.strPlacementId &&
			player.iCurrentSkillId == INVALID_SKILL_ID && player.fPositionX == 0.f,
			"Owned move starts on the first admitted G press without leaving and preserves authored source");
		(void)triggerSystem.Activate_Interact(1u, trigger.strPlacementId, players, 13u,
			transfers, {}, roomEntry);
		tests.Require(attempts == 3u && player.iActionStartTick == 12u,
			"A repeat G press cannot restart a successful owned move");
		triggerSystem.Update_PlayerMotion(player, move.fDurationSeconds);
		player.fPositionX = 0.f;
		(void)triggerSystem.Activate_Interact(1u, trigger.strPlacementId, players, 30u,
			transfers, {}, roomEntry);
		tests.Require(attempts == 3u && !player.TriggerMove.isActive,
			"Owned one-shot move commits its latch only after successful admission");

		(void)triggerSystem.Initialize({trigger}, status);
		player.eAction = PLAYER_ACTION_STATE::SKILL;
		(void)triggerSystem.Activate_Interact(1u, trigger.strPlacementId, players, 40u,
			transfers, {},
			[](const WORLD_BOOTSTRAP_PLACEMENT&, SERVER_PLAYER&, std::uint32_t)
			{ return SERVER_TRIGGER_MOVE_ENTRY_RESULT::USE_DEFAULT; });
		tests.Require(player.eAction == PLAYER_ACTION_STATE::SKILL && !player.TriggerMove.isActive,
			"Unowned move keeps ordinary action admission and does not interrupt skills");

		(void)triggerSystem.Initialize({trigger}, status);
		player.iCurrentHp = 0u;
		const unsigned beforeDead = attempts;
		(void)triggerSystem.Activate_Interact(1u, trigger.strPlacementId, players, 41u,
			transfers, {}, roomEntry);
		tests.Require(attempts == beforeDead && !player.TriggerMove.isActive,
			"Dead players never invoke room-owned movement entry");

		player.iCurrentHp = 100u;
		trigger.requiresInteract = true;
		(void)triggerSystem.Initialize({trigger}, status);
		triggerSystem.Evaluate_Entries(players, 22u, transfers, {}, prompts, roomEntry);
		tests.Require(attempts == beforeDead && prompts.size() == 1u && prompts.front().bAvailable &&
			!player.TriggerMove.isActive,
			"Room-owned automatic entry cannot bypass an authored interaction requirement");

		trigger.requiresInteract = false;
		trigger.isTriggerOnce = false;
		(void)triggerSystem.Initialize({trigger}, status);
		(void)triggerSystem.Activate_Interact(1u, trigger.strPlacementId, players, 50u,
			transfers, {}, roomEntry);
		triggerSystem.Update_PlayerMotion(player, move.fDurationSeconds);
		player.fPositionX = 0.f;
		(void)triggerSystem.Activate_Interact(1u, trigger.strPlacementId, players, 60u,
			transfers, {}, roomEntry);
		tests.Require(attempts == beforeDead + 2u && player.TriggerMove.isActive &&
			player.iActionStartTick == 60u && player.TriggerMove.strSourcePlacementId == trigger.strPlacementId,
			"Repeatable owned move admits a later G press again after the motion");
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
		triggerSystem.Set_HonourTriggerOnce(true);
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
			transfers.empty() && 1u == promptEdges.size() &&
			promptEdges.front().bAvailable &&
			"trigger.contract.change-level" == promptEdges.front().strTriggerPlacementId,
			"Stepping into a changeLevel box only offers it; no world transfer is emitted");
		tests.Require(
			triggerSystem.Activate_Interact(
				3, "trigger.contract.change-level", players, 21, transfers, {}) &&
			1u == transfers.size() &&
			7u == transfers.front().iSessionId &&
			WORLD_ID::VALTAN_ARENA == transfers.front().eTargetWorldId &&
			CHARACTER_CLASS_ID::LANCE_MASTER ==
				transfers.front().eCharacterClass &&
			"TriggerTransfer" == transfers.front().strNickName,
			"G inside the offered changeLevel box emits one typed Server world transfer request");
		transfers.clear();
		tests.Require(
			!triggerSystem.Activate_Interact(
				3, "trigger.contract.change-level", players, 60, transfers, {}) &&
			transfers.empty(),
			"Do not repeat a triggerOnce world transfer once it is spent");
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
		triggerSystem.Set_HonourTriggerOnce(true);
		triggerSystem.Set_WorldId(WORLD_ID::VALTAN_ARENA);
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
		/* Stage_1, Stage_2 and Stage_MiniBoss build their waves or author the Lugaru
		entrance move, and Stage_3 authors the cliff move, so all four run their
		real actions in Debug as well. Only Stage_Boss keeps the shortcut that still
		carries boss work to Valtan without clearing the corridor. */
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "Stage_3";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION authoredMove{};
		authoredMove.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
		authoredMove.fTargetX = 100.42f;
		authoredMove.fTargetY = 20.53f;
		authoredMove.fTargetZ = -86.95f;
		authoredMove.fDurationSeconds = 0.8f;
		authoredMove.fArcHeight = 0.f;
		trigger.TriggerActions.push_back(authoredMove);

		CServerTriggerSystem triggerSystem;
		triggerSystem.Set_WorldId(WORLD_ID::VALTAN_ARENA);
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus, true),
			"Initialize the Debug Valtan stage-route bypass with Stage_3 present");
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
		const SERVER_PLAYER& moving = players.begin()->second;
		tests.Require(
			0u == activationCount &&
			PLAYER_ACTION_STATE::NONE == moving.eAction &&
			!moving.TriggerMove.isActive &&
			1u == promptEdges.size() && promptEdges.front().bAvailable,
			"Stepping into Stage_3 only offers its authored move, Debug included; the player stays put");
		tests.Require(
			1u == triggerSystem.Activate_Here(404u, players, 42u, transfers,
				[&activationCount](WORLD_TRIGGER_ACTION_KIND, const std::string&)
				{
					++activationCount;
					return true;
				}) &&
			0u == activationCount &&
			PLAYER_ACTION_STATE::TRIGGER_MOVE == moving.eAction &&
			moving.TriggerMove.isActive &&
			std::abs(moving.TriggerMove.fTargetX - 100.42f) < 0.001f &&
			std::abs(moving.TriggerMove.fTargetY - 20.53f) < 0.001f &&
			std::abs(moving.TriggerMove.fTargetZ + 86.95f) < 0.001f,
			"G runs the authored Stage_3 cliff move in Debug too, not the boss-approach shortcut hop");
		triggerSystem.Update_PlayerMotion(players.begin()->second, 1.f);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == players.begin()->second.eAction &&
			std::abs(players.begin()->second.fPositionX - 100.42f) < 0.001f &&
			std::abs(players.begin()->second.fPositionZ + 86.95f) < 0.001f,
			"Complete the authored Stage_3 move at the requested cliff point");
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
		triggerSystem.Set_WorldId(WORLD_ID::VALTAN_ARENA);
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
		/* Stage_2 is a wave as well, so it is exempt from the Debug shortcut like
		Stage_1: stepping in runs spawn.valtan.stage03 instead of hopping the
		player toward Stage_3, with the bypass switched on. */
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "Stage_2";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = 2.f;
		trigger.fHalfExtentY = 2.f;
		trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION activate{};
		activate.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP;
		activate.strTargetId = "spawn.valtan.stage03";
		trigger.TriggerActions.push_back(activate);

		CServerTriggerSystem triggerSystem;
		triggerSystem.Set_WorldId(WORLD_ID::VALTAN_ARENA);
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ trigger }, triggerStatus, true),
			"Initialize the Debug bypass with Stage_2 present");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER player{};
		player.iPlayerId = 406u;
		player.iCurrentHp = 100u;
		player.iMaximumHp = 100u;
		players.emplace(player.iPlayerId, player);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::vector<SERVER_INTERACT_PROMPT_EDGE> promptEdges;
		std::string activatedTargetId;
		std::size_t activationCount = 0u;
		triggerSystem.Evaluate_Entries(
			players, 43u, transfers,
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
			"spawn.valtan.stage03" == activatedTargetId &&
			PLAYER_ACTION_STATE::NONE == players.begin()->second.eAction &&
			!players.begin()->second.TriggerMove.isActive &&
			promptEdges.empty(),
			"Activate the Stage_2 wave instead of bypassing it, Debug included");
	}

	{
		/* Stage_Boss starts the boss and sends every player who fires it to the
		   Stage_Boss_ArenaEntry box centre, landing on the floor the room's navigation
		   reports there. The start is refused once the boss is up and a later player is
		   still sent; a busy player is not; a missing entrance box only starts the boss. */
		const auto makeBox = [](const char* id, const float x, const float y, const float z,
			const float halfX, const float halfY, const float halfZ)
		{
			WORLD_BOOTSTRAP_PLACEMENT box{};
			box.strPlacementId = id;
			box.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
			box.isEnabled = true;
			box.fPositionX = x;
			box.fPositionY = y;
			box.fPositionZ = z;
			box.fHalfExtentX = halfX;
			box.fHalfExtentY = halfY;
			box.fHalfExtentZ = halfZ;
			box.isTriggerOnce = true;
			return box;
		};
		WORLD_BOOTSTRAP_PLACEMENT bossStart =
			makeBox("Stage_Boss", 129.65f, 23.02f, -96.85f, 2.f, 1.5f, 2.f);
		WORLD_TRIGGER_ACTION startAction{};
		startAction.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER;
		startAction.strTargetId = "boss.valtan.center";
		bossStart.TriggerActions.push_back(startAction);
		WORLD_BOOTSTRAP_PLACEMENT entrance =
			makeBox("Stage_Boss_ArenaEntry", 141.85f, 25.73f, -107.55f, 2.5f, 1.5f, 2.5f);
		WORLD_TRIGGER_ACTION inward{};
		inward.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
		inward.fTargetX = 147.75f;
		inward.fTargetY = 23.02f;
		inward.fTargetZ = -117.25f;
		inward.fDurationSeconds = 0.8f;
		inward.fArcHeight = 0.f;
		entrance.TriggerActions.push_back(inward);

		const auto makePlayer = [&bossStart](const PLAYER_ID id)
		{
			SERVER_PLAYER player{};
			player.iPlayerId = id;
			player.iCurrentHp = 100u;
			player.iMaximumHp = 100u;
			player.fPositionX = bossStart.fPositionX;
			player.fPositionY = bossStart.fPositionY;
			player.fPositionZ = bossStart.fPositionZ;
			return player;
		};
		const auto sentTo = [](const SERVER_PLAYER& player, const float y)
		{
			return PLAYER_ACTION_STATE::TRIGGER_MOVE == player.eAction &&
				player.TriggerMove.isActive &&
				std::abs(player.TriggerMove.fTargetX - 141.85f) < 0.001f &&
				std::abs(player.TriggerMove.fTargetY - y) < 0.001f &&
				std::abs(player.TriggerMove.fTargetZ + 107.55f) < 0.001f &&
				"Stage_Boss" == player.TriggerMove.strSourcePlacementId;
		};

		CServerTriggerSystem triggerSystem;
		triggerSystem.Set_WorldId(WORLD_ID::VALTAN_ARENA);
		triggerSystem.Set_GroundSampler(
			[](const float, const float, float& outY) { outY = 22.836f; return true; });
		std::string triggerStatus;
		tests.Require(
			triggerSystem.Initialize({ bossStart, entrance }, triggerStatus),
			"Initialize the Valtan boss start beside its arena entrance box");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		players.emplace(501u, makePlayer(501u));
		players.emplace(502u, makePlayer(502u));
		SERVER_PLAYER busyPlayer = makePlayer(503u);
		busyPlayer.eAction = PLAYER_ACTION_STATE::KNOCKDOWN;
		players.emplace(503u, busyPlayer);
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::vector<SERVER_INTERACT_PROMPT_EDGE> promptEdges;
		std::size_t startCount = 0u;
		triggerSystem.Evaluate_Entries(
			players, 60u, transfers,
			[&startCount](const WORLD_TRIGGER_ACTION_KIND kind, const std::string&)
			{
				if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER != kind)
					return false;
				return 1u == ++startCount;
			},
			promptEdges);
		tests.Require(
			3u == startCount &&
			sentTo(players.at(501u), 22.836f) &&
			sentTo(players.at(502u), 22.836f),
			"Stepping into Stage_Boss sends each player to the arena entrance box centre on the navigation floor, including one who arrives after the boss is already up");
		tests.Require(
			PLAYER_ACTION_STATE::KNOCKDOWN == players.at(503u).eAction &&
			!players.at(503u).TriggerMove.isActive,
			"A busy player is not sent to the arena entrance and the boss start is unaffected");
		triggerSystem.Update_PlayerMotion(players.at(501u), 1.f);
		tests.Require(
			PLAYER_ACTION_STATE::NONE == players.at(501u).eAction &&
			std::abs(players.at(501u).fPositionX - 141.85f) < 0.001f &&
			std::abs(players.at(501u).fPositionY - 22.836f) < 0.001f &&
			std::abs(players.at(501u).fPositionZ + 107.55f) < 0.001f,
			"Complete the Stage_Boss placement at the arena entrance box centre");

		{
			CServerTriggerSystem withoutGround;
			withoutGround.Set_WorldId(WORLD_ID::VALTAN_ARENA);
			std::string status;
			tests.Require(
				withoutGround.Initialize({ bossStart, entrance }, status),
				"Initialize the Valtan boss start without a floor sampler");
			std::map<PLAYER_ID, SERVER_PLAYER> lone;
			lone.emplace(511u, makePlayer(511u));
			std::vector<SERVER_WORLD_TRANSFER_REQUEST> loneTransfers;
			std::vector<SERVER_INTERACT_PROMPT_EDGE> loneEdges;
			withoutGround.Evaluate_Entries(
				lone, 61u, loneTransfers,
				[](const WORLD_TRIGGER_ACTION_KIND, const std::string&) { return true; },
				loneEdges);
			tests.Require(
				sentTo(lone.at(511u), 25.73f),
				"Without a floor sampler the Stage_Boss placement uses the entrance box height");
		}

		{
			CServerTriggerSystem withoutEntrance;
			withoutEntrance.Set_WorldId(WORLD_ID::VALTAN_ARENA);
			std::string status;
			tests.Require(
				withoutEntrance.Initialize({ bossStart }, status),
				"Initialize the Valtan boss start without an arena entrance box");
			std::map<PLAYER_ID, SERVER_PLAYER> lone;
			lone.emplace(512u, makePlayer(512u));
			std::vector<SERVER_WORLD_TRANSFER_REQUEST> loneTransfers;
			std::vector<SERVER_INTERACT_PROMPT_EDGE> loneEdges;
			std::size_t loneStarts = 0u;
			withoutEntrance.Evaluate_Entries(
				lone, 62u, loneTransfers,
				[&loneStarts](const WORLD_TRIGGER_ACTION_KIND, const std::string&)
				{
					++loneStarts;
					return true;
				},
				loneEdges);
			tests.Require(
				1u == loneStarts &&
				PLAYER_ACTION_STATE::NONE == lone.at(512u).eAction &&
				!lone.at(512u).TriggerMove.isActive,
				"Without an arena entrance box Stage_Boss only starts the boss");
		}

#ifdef _DEBUG
		{
			/* Debug makes Stage_Boss wait for G (the corridor shortcut table still lists
			   it for the pattern audition), and G sends the player to the same entrance
			   box, not to the old 159-bar bait point. */
			CServerTriggerSystem debugSystem;
			debugSystem.Set_WorldId(WORLD_ID::VALTAN_ARENA);
			debugSystem.Set_GroundSampler(
				[](const float, const float, float& outY) { outY = 22.836f; return true; });
			std::string status;
			tests.Require(
				debugSystem.Initialize({ bossStart, entrance }, status, true),
				"Initialize the Debug Valtan bypass with Stage_Boss and its entrance box");
			std::map<PLAYER_ID, SERVER_PLAYER> lone;
			lone.emplace(521u, makePlayer(521u));
			std::vector<SERVER_WORLD_TRANSFER_REQUEST> loneTransfers;
			std::vector<SERVER_INTERACT_PROMPT_EDGE> loneEdges;
			std::size_t loneStarts = 0u;
			const auto countStart =
				[&loneStarts](const WORLD_TRIGGER_ACTION_KIND, const std::string&)
				{
					++loneStarts;
					return true;
				};
			debugSystem.Evaluate_Entries(lone, 63u, loneTransfers, countStart, loneEdges);
			tests.Require(
				0u == loneStarts &&
				PLAYER_ACTION_STATE::NONE == lone.at(521u).eAction &&
				1u == loneEdges.size() && loneEdges.front().bAvailable,
				"Debug: stepping into Stage_Boss only offers it and does not start the boss");
			tests.Require(
				1u == debugSystem.Activate_Here(
					521u, lone, 64u, loneTransfers, countStart) &&
				1u == loneStarts &&
				sentTo(lone.at(521u), 22.836f),
				"Debug: G at Stage_Boss starts the boss and sends the player to the arena entrance box, not the audition bait point");
		}
#endif
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
		triggerSystem.Set_WorldId(WORLD_ID::VALTAN_ARENA);
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
		/* triggerOnce stays authored, but the Server no longer turns it into a
		   room-wide latch: the first player's entry must not spend the box for the
		   next player, nor for the same player on a return trip. A Valtan corridor
		   wave is a scripted-flow kind, so it still fires on entry. */
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "trigger.contract.repeat-spawn-group";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = trigger.fHalfExtentY = trigger.fHalfExtentZ = 2.f;
		trigger.isTriggerOnce = true;
		WORLD_TRIGGER_ACTION activate{};
		activate.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP;
		activate.strTargetId = "spawn.contract.repeat";
		trigger.TriggerActions.push_back(activate);

		CServerTriggerSystem triggerSystem;
		triggerSystem.Set_WorldId(WORLD_ID::VALTAN_ARENA);
		std::string status;
		tests.Require(triggerSystem.Initialize({ trigger }, status),
			"Initialize an authored triggerOnce trigger under the repeatable Product policy");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		for (const PLAYER_ID id : { 10u, 11u })
		{
			SERVER_PLAYER& entry = players[id];
			entry.iPlayerId = id;
			entry.iCurrentHp = entry.iMaximumHp = 100u;
			entry.fPositionX = 50.f;
		}
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::vector<SERVER_INTERACT_PROMPT_EDGE> prompts;
		std::size_t activations = 0u;
		const auto count = [&activations](WORLD_TRIGGER_ACTION_KIND kind, const std::string& id)
		{
			if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP != kind ||
				"spawn.contract.repeat" != id)
			{
				return false;
			}
			++activations;
			return true;
		};
		players.at(10u).fPositionX = 0.f;
		triggerSystem.Evaluate_Entries(players, 50u, transfers, count, prompts);
		triggerSystem.Evaluate_Entries(players, 51u, transfers, count, prompts);
		tests.Require(1u == activations,
			"The first player fires the trigger once and staying inside does not repeat it");
		players.at(11u).fPositionX = 0.f;
		triggerSystem.Evaluate_Entries(players, 52u, transfers, count, prompts);
		tests.Require(2u == activations,
			"A second player's own entry fires the same trigger again");
		players.at(10u).fPositionX = 50.f;
		triggerSystem.Evaluate_Entries(players, 53u, transfers, count, prompts);
		players.at(10u).fPositionX = 0.f;
		triggerSystem.Evaluate_Entries(players, 54u, transfers, count, prompts);
		tests.Require(3u == activations,
			"The first player fires it again on a return trip");
	}

	{
		/* A G press made while a skill or hit reaction owns the player changes
		nothing and costs no debounce, so the same press works again the moment
		the action ends (a Kouku Mario crossing is a G box). */
		WORLD_BOOTSTRAP_PLACEMENT trigger{};
		trigger.strPlacementId = "Mario0_Contract_BusyRetry";
		trigger.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		trigger.isEnabled = true;
		trigger.fHalfExtentX = trigger.fHalfExtentY = trigger.fHalfExtentZ = 1.f;
		WORLD_TRIGGER_ACTION move{};
		move.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
		move.fTargetX = 8.f;
		move.fDurationSeconds = 0.6f;
		trigger.TriggerActions.push_back(move);
		CServerTriggerSystem triggerSystem;
		triggerSystem.Set_WorldId(WORLD_ID::KAKULSAYDON_ARENA);
		std::string status;
		tests.Require(triggerSystem.Initialize({ trigger }, status),
			"Initialize the busy-retry movement fixture");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		SERVER_PLAYER& player = players[1u];
		player.iPlayerId = 1u;
		player.iCurrentHp = player.iMaximumHp = 100u;
		player.eAction = PLAYER_ACTION_STATE::SKILL;
		player.iCurrentSkillId = 34090u;
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::vector<SERVER_INTERACT_PROMPT_EDGE> prompts;
		tests.Require(!triggerSystem.Activate_Interact(1u, trigger.strPlacementId, players, 60u,
				transfers, {}) &&
			PLAYER_ACTION_STATE::SKILL == player.eAction && !player.TriggerMove.isActive,
			"A busy player is not interrupted by a movement trigger");
		player.eAction = PLAYER_ACTION_STATE::NONE;
		player.iCurrentSkillId = INVALID_SKILL_ID;
		tests.Require(triggerSystem.Activate_Interact(1u, trigger.strPlacementId, players, 61u,
				transfers, {}) &&
			PLAYER_ACTION_STATE::TRIGGER_MOVE == player.eAction &&
			player.TriggerMove.isActive && 61u == player.iActionStartTick,
			"The same G press starts the movement once the action ends, without leaving and re-entering");
	}

	{
		/* Outside the scripted flows a trigger never fires by stepping in. Entry
		   only offers it, and G pressed inside the volume fires it, any number of
		   times, for any player. Bern has no scripted-flow row in AUTO_ENTRY_RULES. */
		WORLD_BOOTSTRAP_PLACEMENT spawn{};
		spawn.strPlacementId = "trigger.contract.gonly-spawn";
		spawn.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
		spawn.isEnabled = true;
		spawn.fHalfExtentX = spawn.fHalfExtentY = spawn.fHalfExtentZ = 2.f;
		WORLD_TRIGGER_ACTION activate{};
		activate.eKind = WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP;
		activate.strTargetId = "spawn.contract.gonly";
		spawn.TriggerActions.push_back(activate);
		WORLD_BOOTSTRAP_PLACEMENT hop = spawn;
		hop.strPlacementId = "trigger.contract.gonly-move";
		hop.fPositionX = 20.f;
		hop.TriggerActions.front().eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
		hop.TriggerActions.front().fTargetX = 30.f;
		hop.TriggerActions.front().fDurationSeconds = 0.5f;
		WORLD_BOOTSTRAP_PLACEMENT cutscene = spawn;
		cutscene.strPlacementId = "trigger.contract.auto-sequence";
		cutscene.fPositionX = 40.f;
		cutscene.TriggerActions.front().eKind = WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE;
		cutscene.TriggerActions.front().strTargetId = "sequence.contract.auto";

		CServerTriggerSystem triggerSystem;
		triggerSystem.Set_WorldId(WORLD_ID::BERN);
		std::vector<std::string> fireLog;
		triggerSystem.Set_FireLog([&fireLog](const std::string& line) { fireLog.push_back(line); });
		std::string status;
		tests.Require(triggerSystem.Initialize({ spawn, hop, cutscene }, status),
			"Initialize a spawn box, a movement box and a scripted sequence box in Bern");
		std::map<PLAYER_ID, SERVER_PLAYER> players;
		for (const PLAYER_ID id : { 9u, 10u })
		{
			SERVER_PLAYER& entry = players[id];
			entry.iPlayerId = id;
			entry.iCurrentHp = entry.iMaximumHp = 100u;
			entry.fPositionX = 50.f;
		}
		std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
		std::vector<SERVER_INTERACT_PROMPT_EDGE> prompts;
		std::size_t spawnCount = 0u;
		std::size_t sequenceCount = 0u;
		const auto run = [&](WORLD_TRIGGER_ACTION_KIND kind, const std::string& id)
		{
			if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP == kind &&
				"spawn.contract.gonly" == id)
			{
				++spawnCount;
				return true;
			}
			if (WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE == kind &&
				"sequence.contract.auto" == id)
			{
				++sequenceCount;
				return true;
			}
			return false;
		};
		const auto offered = [&prompts](const char* id, const PLAYER_ID player, const bool available)
		{
			return std::any_of(prompts.begin(), prompts.end(),
				[&](const SERVER_INTERACT_PROMPT_EDGE& edge)
				{
					return edge.strTriggerPlacementId == id &&
						edge.iPlayerId == player && edge.bAvailable == available;
				});
		};
		const std::uint32_t debounce = CServerTriggerSystem::KEY_ACTIVATION_DEBOUNCE_TICKS;

		tests.Require(0u == triggerSystem.Activate_Here(9u, players, 100u, transfers, run) &&
			0u == spawnCount,
			"G outside every trigger box runs nothing");
		players.at(9u).fPositionX = 0.f;
		triggerSystem.Evaluate_Entries(players, 101u, transfers, run, prompts);
		tests.Require(0u == spawnCount && offered("trigger.contract.gonly-spawn", 9u, true),
			"Stepping into a box only offers it; the action does not run");
		triggerSystem.Evaluate_Entries(players, 102u, transfers, run, prompts);
		tests.Require(0u == spawnCount && prompts.empty(),
			"The offer is sent once on the entry edge, and standing inside still runs nothing");
		tests.Require(1u == triggerSystem.Activate_Here(9u, players, 103u, transfers, run) &&
			1u == spawnCount,
			"G inside the box runs it");
		tests.Require(0u == triggerSystem.Activate_Here(9u, players, 105u, transfers, run) &&
			1u == spawnCount,
			"A second G inside the per-player debounce is ignored");
		tests.Require(1u == triggerSystem.Activate_Here(9u, players, 103u + debounce, transfers, run) &&
			2u == spawnCount,
			"G fires the same box again once the debounce has elapsed, any number of times");
		players.at(10u).fPositionX = 0.f;
		triggerSystem.Evaluate_Entries(players, 120u, transfers, run, prompts);
		tests.Require(2u == spawnCount && offered("trigger.contract.gonly-spawn", 10u, true),
			"A second player stepping in is offered the box and fires nothing");
		tests.Require(1u == triggerSystem.Activate_Here(10u, players, 121u, transfers, run) &&
			3u == spawnCount,
			"Another player's G is independent of the first player's debounce");

		players.at(9u).fPositionX = 20.f;
		triggerSystem.Evaluate_Entries(players, 130u, transfers, run, prompts);
		tests.Require(!players.at(9u).TriggerMove.isActive &&
			PLAYER_ACTION_STATE::NONE == players.at(9u).eAction &&
			offered("trigger.contract.gonly-move", 9u, true) &&
			offered("trigger.contract.gonly-spawn", 9u, false),
			"Stepping into a movement box does not move the player; leaving the first box withdraws its offer");
		tests.Require(triggerSystem.Activate_Interact(9u, "trigger.contract.gonly-move",
				players, 140u, transfers, run) &&
			PLAYER_ACTION_STATE::TRIGGER_MOVE == players.at(9u).eAction &&
			players.at(9u).TriggerMove.isActive && 140u == players.at(9u).iActionStartTick,
			"G on the offered movement box moves the player");

		players.at(10u).fPositionX = 40.f;
		triggerSystem.Evaluate_Entries(players, 150u, transfers, run, prompts);
		tests.Require(1u == sequenceCount,
			"A scripted sequence box still fires the moment a player steps in");
		tests.Require(!triggerSystem.Activate_Interact(10u, "trigger.contract.auto-sequence",
				players, 160u, transfers, run) &&
			0u == triggerSystem.Activate_Here(10u, players, 160u, transfers, run) &&
			1u == sequenceCount,
			"G does not run a box that fires on entry");
		tests.Require(!triggerSystem.Activate_Interact(9u, "trigger.contract.missing",
				players, 170u, transfers, run),
			"A G request naming no such box is rejected");
		players.at(9u).fPositionX = 0.f;
		players.at(9u).eAction = PLAYER_ACTION_STATE::NONE;
		players.at(9u).TriggerMove = {};
		players.at(9u).iCurrentHp = 0u;
		tests.Require(0u == triggerSystem.Activate_Here(9u, players, 200u, transfers, run) &&
			3u == spawnCount,
			"A dead player cannot fire a trigger with G");
		tests.Require(0u == triggerSystem.Activate_Here(77u, players, 210u, transfers, run),
			"G from an unknown player id runs nothing");
		const auto fromKey = std::count_if(fireLog.begin(), fireLog.end(),
			[](const std::string& line) { return std::string::npos != line.find("Source=KEY"); });
		tests.Require(5u == fireLog.size() && 4 == fromKey &&
			std::string::npos != fireLog.front().find("Trigger=trigger.contract.gonly-spawn") &&
			std::string::npos != fireLog.front().find("Player=9") &&
			std::string::npos != fireLog.back().find("Trigger=trigger.contract.auto-sequence") &&
			std::string::npos != fireLog.back().find("Source=ENTER"),
			"Every fired trigger is logged once with its id, player and how it fired; G-only boxes log KEY only");
	}

	{
		/* AUTO_ENTRY_RULES by world and id: no player-movement box fires on entry, a
		Mario* id in Kouku included. Crossings, jumps, exits and every changeLevel
		box wait for G in every world. */
		for (const WORLD_ID world : { WORLD_ID::KAKULSAYDON_ARENA, WORLD_ID::VALTAN_ARENA, WORLD_ID::BERN })
		for (const char* const moveId : { "Mario0_Contract_Rule", "jump.contract.rule" })
		{
			WORLD_BOOTSTRAP_PLACEMENT move{};
			move.strPlacementId = moveId;
			move.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
			move.isEnabled = true;
			move.fHalfExtentX = move.fHalfExtentY = move.fHalfExtentZ = 2.f;
			WORLD_TRIGGER_ACTION hop{};
			hop.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
			hop.fTargetX = 8.f;
			hop.fDurationSeconds = 0.6f;
			move.TriggerActions.push_back(hop);
			WORLD_BOOTSTRAP_PLACEMENT transfer = move;
			transfer.strPlacementId = "trigger.contract.rule-transfer";
			transfer.TriggerActions.front().eKind = WORLD_TRIGGER_ACTION_KIND::CHANGE_LEVEL;
			transfer.TriggerActions.front().eTargetWorldId = WORLD_ID::VALTAN_ARENA;
			CServerTriggerSystem triggerSystem;
			triggerSystem.Set_WorldId(world);
			std::string status;
			tests.Require(triggerSystem.Initialize({ move, transfer }, status),
				"Initialize the entry-rule fixture");
			std::map<PLAYER_ID, SERVER_PLAYER> players;
			SERVER_PLAYER& player = players[1u];
			player.iPlayerId = 1u;
			player.iSessionId = 5u;
			player.eCharacterClass = CHARACTER_CLASS_ID::LANCE_MASTER;
			player.strNickName = "RuleProbe";
			player.iCurrentHp = player.iMaximumHp = 100u;
			std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
			std::vector<SERVER_INTERACT_PROMPT_EDGE> prompts;
			triggerSystem.Evaluate_Entries(players, 300u, transfers, {}, prompts);
			tests.Require(!player.TriggerMove.isActive && transfers.empty(),
				"No movement box fires on entry -- a Mario id in Kouku included -- and a changeLevel box never does");
			tests.Require(triggerSystem.Activate_Interact(1u, moveId, players, 301u, transfers, {}) &&
				player.TriggerMove.isActive && transfers.empty(),
				"G fires that same movement box in every world");
		}
	}

	{
		/* A player-movement box is per player. Two players standing in the same box each
		move with their own G, in the Kouku Mario world and in the Valtan corridor alike,
		the first mover does not spend the box for the second, and each can use it again
		after the motion. A room-owned (Mario) box is admitted per player as well, so one
		player's rejection never blocks the other. */
		for (const WORLD_ID world : { WORLD_ID::KAKULSAYDON_ARENA, WORLD_ID::VALTAN_ARENA })
		{
			WORLD_BOOTSTRAP_PLACEMENT crossing{};
			crossing.strPlacementId = WORLD_ID::KAKULSAYDON_ARENA == world ?
				"Mario0_Contract_Crossing" : "Stage_Contract_Crossing";
			crossing.eKind = WORLD_BOOTSTRAP_KIND::TRIGGER_BOX;
			crossing.isEnabled = true;
			crossing.isTriggerOnce = true;
			crossing.fHalfExtentX = crossing.fHalfExtentY = crossing.fHalfExtentZ = 2.f;
			WORLD_TRIGGER_ACTION hop{};
			hop.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
			hop.fTargetX = 8.f;
			hop.fDurationSeconds = 0.6f;
			crossing.TriggerActions.push_back(hop);
			CServerTriggerSystem triggerSystem;
			triggerSystem.Set_WorldId(world);
			std::string status;
			tests.Require(triggerSystem.Initialize({ crossing }, status),
				"Initialize the two-player crossing fixture");
			std::map<PLAYER_ID, SERVER_PLAYER> players;
			for (const PLAYER_ID id : { 1u, 2u })
			{
				SERVER_PLAYER& entry = players[id];
				entry.iPlayerId = id;
				entry.iCurrentHp = entry.iMaximumHp = 100u;
			}
			std::vector<SERVER_WORLD_TRANSFER_REQUEST> transfers;
			std::vector<SERVER_INTERACT_PROMPT_EDGE> prompts;
			triggerSystem.Evaluate_Entries(players, 400u, transfers, {}, prompts);
			tests.Require(2u == prompts.size() && !players.at(1u).TriggerMove.isActive &&
				!players.at(2u).TriggerMove.isActive,
				"Two players standing in a crossing box are both offered it and neither moves by stepping in");
			tests.Require(
				triggerSystem.Activate_Interact(1u, crossing.strPlacementId, players, 401u, transfers, {}) &&
				triggerSystem.Activate_Interact(2u, crossing.strPlacementId, players, 401u, transfers, {}) &&
				players.at(1u).TriggerMove.isActive && players.at(2u).TriggerMove.isActive,
				"Each player's own G moves that player; the first mover does not spend the box for the second");
			for (const PLAYER_ID id : { 1u, 2u })
			{
				(void)triggerSystem.Update_PlayerMotion(players.at(id), hop.fDurationSeconds);
				players.at(id).fPositionX = 0.f;
			}
			tests.Require(
				triggerSystem.Activate_Interact(1u, crossing.strPlacementId, players, 420u, transfers, {}) &&
				triggerSystem.Activate_Interact(2u, crossing.strPlacementId, players, 420u, transfers, {}),
				"Both players use the same crossing again after the motion, any number of times");

			if (WORLD_ID::KAKULSAYDON_ARENA != world)
				continue;
			const SERVER_TRIGGER_MOVE_ENTRY_HANDLER roomEntry =
				[](const WORLD_BOOTSTRAP_PLACEMENT& placement, SERVER_PLAYER& target,
					const std::uint32_t tick)
			{
				if (1u == target.iPlayerId)
					return SERVER_TRIGGER_MOVE_ENTRY_RESULT::RETRY_WHILE_INSIDE;
				return CServerTriggerSystem::Begin_MovePlayer(
						target, placement.TriggerActions.front(), tick) ?
					SERVER_TRIGGER_MOVE_ENTRY_RESULT::STARTED :
					SERVER_TRIGGER_MOVE_ENTRY_RESULT::RETRY_WHILE_INSIDE;
			};
			(void)triggerSystem.Initialize({ crossing }, status);
			for (const PLAYER_ID id : { 1u, 2u })
			{
				players.at(id).TriggerMove = {};
				players.at(id).eAction = PLAYER_ACTION_STATE::NONE;
				players.at(id).fPositionX = 0.f;
			}
			tests.Require(
				!triggerSystem.Activate_Interact(1u, crossing.strPlacementId, players, 500u,
					transfers, {}, roomEntry) &&
				triggerSystem.Activate_Interact(2u, crossing.strPlacementId, players, 500u,
					transfers, {}, roomEntry) &&
				!players.at(1u).TriggerMove.isActive && players.at(2u).TriggerMove.isActive &&
				players.at(2u).TriggerMove.strSourcePlacementId == crossing.strPlacementId,
				"A Mario box the room rejects for one player still moves the other player through the room admission");
		}

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

