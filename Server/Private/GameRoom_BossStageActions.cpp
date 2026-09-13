#include "GameRoom.h"

#include "ClientSession.h"
#include "ServerCombatHitRuntime.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <set>
#include <string_view>
#include <utility>

#include "GameRoom_Internal.h"

using namespace GameRoomDetail;

bool LostArk::Server::CGameRoom::Stage_BossPatternStageActions(
	const SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const std::string& patternId,
	const std::string& actionId,
	const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
	const std::uint32_t serverTick,
	SERVER_BOSS_COMBAT_STATE& stagedCombat,
	std::uint8_t& stagedGameplayPhase,
	SERVER_COMBAT_OBJECT_TRANSACTION& combatObjectTransaction,
	const std::uint32_t spawnWaveOrdinal,
	const bool scheduledSpawnWave)
{
	if (patternId.empty() || actionId.empty())
		return true;
	const auto* patterns = catalog.Find_BossPatterns(boss.strEncounterId);
	if (nullptr == patterns)
	{
		m_strStatus = "Boss stage action encounter is missing";
		return false;
	}
	const auto pattern = std::find_if(patterns->begin(), patterns->end(),
		[&patternId](const BOSS_PATTERN_DEFINITION& candidate)
		{
			return candidate.strPatternId == patternId;
		});
	if (patterns->end() == pattern)
	{
		m_strStatus = "Boss stage action pattern is missing: " + patternId;
		return false;
	}
	const auto stage = std::find_if(
		pattern->Stages.begin(), pattern->Stages.end(),
		[&actionId](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
		{
			return candidate.strActionId == actionId;
		});
	if (pattern->Stages.end() == stage)
	{
		m_strStatus = "Boss stage action owner is missing: " + actionId;
		return false;
	}
	if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
		stage == pattern->Stages.begin() && pattern->Motion.bMoveToAnchorBeforeTakeoff)
	{
		SERVER_NAV_POINT anchor{};
		if (!m_ServerNavigation.Is_PointWalkableExact(
			pattern->Motion.fLandingX, pattern->Motion.fLandingZ) ||
			!m_ServerNavigation.Sample_Position(
				pattern->Motion.fLandingX, pattern->Motion.fLandingZ, anchor) ||
			std::fabs(anchor.y - pattern->Motion.fLandingY) > 1.5f)
		{
			m_strStatus = "Pre-takeoff arena anchor is not on the live arena deck";
			return false;
		}
	}
	const BOSS_PATTERN_STAGE_ACTION* scheduledVolleyClock = nullptr;
	for (const BOSS_PATTERN_STAGE_ACTION& action : stage->Actions)
	{
		if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != action.eTrigger ||
			BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY !=
				action.eKind)
		{
			continue;
		}
		const std::uint64_t lastSpawnOffsetMs =
			static_cast<std::uint64_t>(action.Volley.iFirstSpawnOffsetMs) +
			static_cast<std::uint64_t>(0u == action.Volley.iSpawnCount ? 0u :
				action.Volley.iSpawnCount - 1u) *
				action.Volley.iSpawnIntervalMs;
		if (0u == action.Volley.iSpawnCount ||
			lastSpawnOffsetMs >= stage->iDurationMs)
		{
			m_strStatus = "Boss scheduled volley leaves its stage clock";
			return false;
		}
		if (0u == action.Volley.iFirstSpawnOffsetMs &&
			action.Volley.iSpawnCount <= 1u)
		{
			continue;
		}
		if (nullptr != scheduledVolleyClock &&
			(scheduledVolleyClock->Volley.iFirstSpawnOffsetMs !=
				action.Volley.iFirstSpawnOffsetMs ||
			 scheduledVolleyClock->Volley.iSpawnCount !=
				action.Volley.iSpawnCount ||
			 scheduledVolleyClock->Volley.iSpawnIntervalMs !=
				action.Volley.iSpawnIntervalMs))
		{
			m_strStatus = "Boss scheduled volleys do not share one clock";
			return false;
		}
		scheduledVolleyClock = &action;
	}
	for (const BOSS_PATTERN_STAGE_ACTION& action : stage->Actions)
	{
		if (action.eTrigger != trigger)
			continue;
		const bool isTypedVolley =
			BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY ==
				action.eKind;
		const bool isScheduledVolley = isTypedVolley &&
			(0u != action.Volley.iFirstSpawnOffsetMs ||
			 action.Volley.iSpawnCount > 1u);
		if (scheduledSpawnWave &&
			(!isScheduledVolley ||
			 BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != trigger ||
			 spawnWaveOrdinal >= action.Volley.iSpawnCount))
		{
			continue;
		}
		if (!scheduledSpawnWave && isTypedVolley &&
			BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
			0u != action.Volley.iFirstSpawnOffsetMs)
		{
			continue;
		}
		if (0u != action.iDurationMs &&
			BOSS_PATTERN_STAGE_ACTION_KIND::RELEASE_GRABBED_PLAYERS !=
				action.eKind &&
			BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_BIND != action.eKind &&
			BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_SILENCE != action.eKind)
		{
			m_strStatus = "Unsupported boss stage action: " + action.strTargetId;
			return false;
		}
		switch (action.eKind)
		{
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_BOSS_FLAG:
		{
			if (action.iValue > 1u)
			{
				m_strStatus = "Boss flag stage action value is invalid: " +
					action.strTargetId;
				return false;
			}
			SERVER_BOSS_COMBAT_FLAG flag{};
			if ("boss.flag.groggy" == action.strTargetId)
				flag = SERVER_BOSS_COMBAT_FLAG::GROGGY;
			else if ("boss.flag.invulnerable" == action.strTargetId)
				flag = SERVER_BOSS_COMBAT_FLAG::INVULNERABLE;
			else if ("boss.flag.counterable" == action.strTargetId)
				flag = SERVER_BOSS_COMBAT_FLAG::COUNTERABLE;
			else
			{
				m_strStatus = "Unknown boss flag stage action: " +
					action.strTargetId;
				return false;
			}
			(void)CBossCombatRuntime::Set_Flag(
				stagedCombat, flag, 0u != action.iValue);
			break;
		}
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_STAGGER_GAUGE:
			if ("boss.gauge.stagger" != action.strTargetId)
			{
				m_strStatus = "Unknown boss stagger gauge target: " +
					action.strTargetId;
				return false;
			}
			(void)CBossCombatRuntime::Set_StaggerGauge(
				stagedCombat, action.iValue);
			break;
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_SHIELD:
			if ("boss.gauge.shield" != action.strTargetId)
			{
				m_strStatus = "Unknown boss shield gauge target: " +
					action.strTargetId;
				return false;
			}
			(void)CBossCombatRuntime::Set_Shield(
				stagedCombat, action.iValue);
			break;
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_BIND:
		{
			const bool entering =
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger;
			if ("player.status.bind" != action.strTargetId ||
				(entering ?
					(5000u != action.iValue || action.iDurationMs < 100u ||
					 action.iDurationMs > 120000u) :
					(0u != action.iValue || 0u != action.iDurationMs)))
			{
				m_strStatus = "Boss player-bind stage action is invalid";
				return false;
			}
			if (!entering)
				break;
			const auto target = std::find_if(
				m_Players.begin(), m_Players.end(),
				[&boss](const auto& entry)
				{
					return entry.second.iNetEntityId ==
						boss.iPatternTargetEntityId;
				});
			if (m_Players.end() == target ||
				INVALID_NET_ENTITY_ID == boss.iPatternTargetEntityId ||
				0u == target->second.iCurrentHp ||
				!target->second.isCombatReady || target->second.bPatternBound ||
				PLAYER_ACTION_STATE::DEAD == target->second.eAction ||
				PLAYER_ACTION_STATE::FALLING == target->second.eAction ||
				PLAYER_ACTION_STATE::GRABBED == target->second.eAction ||
				!m_ServerNavigation.Is_Loaded() ||
				!m_ServerNavigation.Is_PointWalkableExact(
					target->second.fPositionX, target->second.fPositionZ))
			{
				m_strStatus = "Boss player-bind target is not an admitted alive target";
				return false;
			}
			SERVER_NAV_POINT ground{};
			if (!m_ServerNavigation.Sample_Position(
				target->second.fPositionX, target->second.fPositionZ, ground) ||
				std::abs(ground.y - target->second.fPositionY) > 1.5f ||
				!std::isfinite(target->second.fPositionY + 5.f))
			{
				m_strStatus = "Boss player-bind restore pose is not navigable";
				return false;
			}
			break;
		}
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_SILENCE:
		{
			if ("player.status.silence" != action.strTargetId ||
				BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != trigger ||
				1u != action.iValue || action.iDurationMs < stage->iDurationMs ||
				action.iDurationMs < 100u || action.iDurationMs > 120000u)
			{
				m_strStatus = "Boss player-silence stage action is invalid";
				return false;
			}
			break;
		}
		case BOSS_PATTERN_STAGE_ACTION_KIND::SET_GAMEPLAY_PHASE:
		{
			const BOSS_RUNTIME_PROFILE* profile =
				catalog.Find_Boss(boss.strArchetypeId);
			const bool isValtan = "BOSS_VALTAN" == boss.strArchetypeId &&
				"ENCOUNTER_VALTAN" == boss.strEncounterId;
			if (nullptr == profile ||
				BOSS_PHASE_POLICY_KIND::AUTHORED_PATTERN_EVENT !=
					profile->PhasePolicy.eKind ||
				"boss.phase.gameplay" != action.strTargetId ||
				action.iValue < 2u || action.iValue > 3u ||
				(isValtan &&
					BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
					!(("VALTAN_ARENA_BREAK_109" == patternId &&
					   "valtan.mechanic.arena-break-109.impact" == actionId &&
					   2u == action.iValue) ||
					  ("VALTAN_GHOST_RESPAWN_AUDITION" == patternId &&
					   "valtan.sequence.respawn.step-01" == actionId &&
					   3u == action.iValue))) ||
				(isValtan &&
					BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != trigger) ||
				action.iValue < stagedGameplayPhase)
			{
				m_strStatus = "Boss gameplay phase stage action is invalid";
				return false;
			}
			SERVER_WORLD_ENTITY stagedPhaseBoss{};
			stagedPhaseBoss.iPhase = stagedGameplayPhase;
			stagedPhaseBoss.BossCombat = stagedCombat;
			(void)CBossCombatRuntime::Set_GameplayPhase(
				stagedPhaseBoss, static_cast<std::uint8_t>(action.iValue));
			stagedGameplayPhase = stagedPhaseBoss.iPhase;
			stagedCombat = std::move(stagedPhaseBoss.BossCombat);
			break;
		}
		case BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT:
		case BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY:
		{
			const bool isTypedVolley =
				BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY ==
					action.eKind;
			const BOSS_COMBAT_OBJECT_DEFINITION* definition =
				catalog.Find_BossCombatObject(action.strTargetId);
			if (nullptr == definition ||
				definition->strEncounterId != boss.strEncounterId ||
				definition->strOwnerPatternId != patternId ||
				definition->strOwnerStageActionId != actionId)
			{
				m_strStatus = "Boss combat object owner is invalid: " +
					action.strTargetId;
				return false;
			}
			const bool perAlivePlayerVolley = isTypedVolley &&
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::PER_ALIVE_PLAYER ==
					action.Volley.ePolicy;
			const bool bossRelativeVolley = isTypedVolley &&
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::BOSS_RELATIVE ==
					action.Volley.ePolicy;
			const bool arenaCenterVolley = isTypedVolley &&
				BOSS_COMBAT_OBJECT_VOLLEY_POLICY::ARENA_CENTER ==
					action.Volley.ePolicy;
			const bool navIndependentPortalVolley =
				"BOSS_VALTAN" == boss.strArchetypeId &&
				bossRelativeVolley &&
				"VALTAN_GHOST_PORTAL_ONCE" == patternId &&
				"valtan.ghost.portal-once.active" == actionId &&
				"combatobject.valtan.ghost.portal-charge" ==
					definition->strCombatObjectArchetypeId;
			if (isTypedVolley &&
				((!perAlivePlayerVolley && !bossRelativeVolley &&
				  !arenaCenterVolley) ||
				 (perAlivePlayerVolley &&
					BOSS_COMBAT_OBJECT_ORIGIN_POLICY::
						LOCKED_TARGET_PER_ALIVE_PLAYER !=
						definition->eOriginPolicy) ||
				 ((bossRelativeVolley || arenaCenterVolley) &&
					BOSS_COMBAT_OBJECT_ORIGIN_POLICY::BOSS_POSITION !=
						definition->eOriginPolicy) ||
				 action.Volley.iSpawnCount < 1u ||
				 spawnWaveOrdinal >= action.Volley.iSpawnCount ||
				 action.Volley.iMaximumTotalObjects > 64u))
			{
				m_strStatus = "Boss combat object volley policy is invalid: " +
					action.strTargetId;
				return false;
			}
			SERVER_COMBAT_OBJECT_LOCKED_TARGET lockedTarget{};
			const SERVER_COMBAT_OBJECT_LOCKED_TARGET* resolvedLockedTarget =
				nullptr;
			if (BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_UNTIL_FIRST_PULSE ==
				definition->eOriginPolicy)
			{
				for (const auto& [playerId, player] : m_Players)
				{
					(void)playerId;
					if (player.iNetEntityId == boss.iPatternTargetEntityId)
					{
						if (0u != player.iCurrentHp && player.isCombatReady &&
							LostArk::Shared::PLAYER_ACTION_STATE::DEAD !=
								player.eAction &&
							LostArk::Shared::PLAYER_ACTION_STATE::FALLING !=
								player.eAction)
						{
							lockedTarget.iNetEntityId = player.iNetEntityId;
							lockedTarget.fPositionX = player.fPositionX;
							lockedTarget.fPositionY = player.fPositionY;
							lockedTarget.fPositionZ = player.fPositionZ;
							resolvedLockedTarget = &lockedTarget;
						}
						break;
					}
				}
				if (nullptr == resolvedLockedTarget &&
					boss.bHasPatternTargetLastPosition &&
					LostArk::Shared::INVALID_NET_ENTITY_ID !=
						boss.iPatternTargetEntityId)
				{
					lockedTarget.iNetEntityId = boss.iPatternTargetEntityId;
					lockedTarget.fPositionX =
						boss.fPatternTargetLastPositionX;
					lockedTarget.fPositionY =
						boss.fPatternTargetLastPositionY;
					lockedTarget.fPositionZ =
						boss.fPatternTargetLastPositionZ;
					resolvedLockedTarget = &lockedTarget;
				}
				/* A pattern that never acquired a valid target still enters without
				creating an object. A vanished locked target uses the cached pose. */
				if (nullptr == resolvedLockedTarget)
				{
					break;
				}
			}
			if (bossRelativeVolley || arenaCenterVolley)
			{
				const std::uint32_t count =
					action.Volley.iCountPerResolvedTarget;
				/* ARENA_CENTER anchors on the boss spawn placement (the authored arena
				   centre) with world-absolute angles; BOSS_RELATIVE keeps the live boss
				   pose and yaw. Stage_BossCombatObject resolves the same origin. */
				const float volleyOriginX =
					arenaCenterVolley ? boss.fSpawnPositionX : boss.fPositionX;
				const float volleyOriginZ =
					arenaCenterVolley ? boss.fSpawnPositionZ : boss.fPositionZ;
				const float volleyYawBasisDegrees =
					arenaCenterVolley ? 0.f : boss.fYawDegrees;
				/* The four-rock presentation volleys may straddle the arena boundary when
				   an owner starts its authored radial set near an edge. Part Break is
				   expected to begin at the charge wall, while Six Pizza and Struggling
				   intentionally preserve their centered corner layouts. Keep the exception
				   tied to the exact owner IDs and an empty damage list; adding gameplay hits
				   makes navigation admission strict again. */
				const bool visualCardinalRocksMayStartOffNavigation =
					BOSS_COMBAT_OBJECT_KIND::FIXED_AREA == definition->eKind &&
					BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NONE ==
						definition->eDirectionPolicy && definition->Hits.empty() &&
					!definition->PresentationPulses.empty() &&
					(("combatobject.valtan.ground-roar.rock" ==
							definition->strCombatObjectArchetypeId &&
					  "VALTAN_GROUND_ROAR" == patternId &&
					  "valtan.sequence.sequence.400440.0.step-01" == actionId) ||
					 ("combatobject.valtan.part-break.rock" ==
							definition->strCombatObjectArchetypeId &&
					  "VALTAN_PART_BREAK" == patternId &&
					  "valtan.reaction.part-break.recovery" == actionId) ||
					 ("combatobject.valtan.six-pizza.rock-pillar" ==
							definition->strCombatObjectArchetypeId &&
					  "VALTAN_SIX_PIZZA_106" == patternId &&
					  "valtan.sequence.center-six-pizza-charge.step-01" == actionId) ||
					 ("combatobject.valtan.struggling.rock-pillar" ==
							definition->strCombatObjectArchetypeId &&
					  "VALTAN_STRUGGLING" == patternId &&
					  "valtan.sequence.warp-jump-four-hand-twohand-roar-roar-dead.step-04" ==
						actionId)) &&
					4u == count;
				/* The exact ghost-portal owner is a world-transform rush rather than
				a walking actor. Unlike the presentation-only rock exception, it may
				own gameplay hits while its authored vertices cross missing nav. */
				const bool authoredVolleyMayStartOffNavigation =
					visualCardinalRocksMayStartOffNavigation ||
					navIndependentPortalVolley;
				const bool damagingCoverVolleyMayProject =
					BOSS_COMBAT_OBJECT_KIND::FIXED_AREA == definition->eKind &&
					BOSS_COMBAT_OBJECT_DIRECTION_POLICY::NONE ==
						definition->eDirectionPolicy &&
					definition->fCoverRadiusM > 0.f && !definition->Hits.empty();
				if (count < 2u || count > 8u || action.iValue != count ||
					BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL !=
						action.Volley.eLayout ||
					action.Volley.fRadiusM <= 0.f ||
					action.Volley.bAllowOverlap ||
					1u != action.Volley.iSpawnCount ||
					0u != action.Volley.iArenaRandomCount ||
					(!navIndependentPortalVolley &&
					 !m_ServerNavigation.Is_Loaded()))
				{
					m_strStatus =
						"Boss-relative combat object volley is invalid";
					return false;
				}
				std::array<std::pair<float, float>, 8u> points{};
				std::array<SERVER_NAV_POINT, 8u> resolvedPoints{};
				bool skipVolley = false;
				for (std::uint32_t ordinal = 0u; ordinal < count; ++ordinal)
				{
					const float degrees = volleyYawBasisDegrees +
						action.Volley.fStartAngleDegrees +
						action.Volley.fAngleStepDegrees *
							static_cast<float>(ordinal);
					const float radians = degrees * DEGREES_TO_RADIANS;
					const float x = volleyOriginX +
						std::sin(radians) * action.Volley.fRadiusM;
					const float z = volleyOriginZ +
						std::cos(radians) * action.Volley.fRadiusM;
					SERVER_NAV_POINT resolvedPoint{
						x, arenaCenterVolley ? boss.fSpawnPositionY : boss.fPositionY, z };
					if (std::isfinite(x) && std::isfinite(z) &&
						damagingCoverVolleyMayProject &&
						!m_ServerNavigation.Is_PointWalkableExact(x, z))
					{
						SERVER_NAV_POINT projected{};
						const bool hasProjection =
							m_ServerNavigation.Project_PointOnSameLevel(x, z, projected);
						float projectionDistance = hasProjection ?
							std::hypot(projected.x - x, projected.z - z) :
							(std::numeric_limits<float>::max)();
						constexpr float MAX_COVER_PROJECTION_METERS = 2.f;
						/* Project_PointOnSameLevel returns a walkable cell centre. When
						   the centre is just beyond the authored two-metre cap, retain
						   the same cell but move toward the authored point until the
						   world displacement is exactly two metres. This removes grid
						   centre quantization without admitting a farther cell. */
						if (hasProjection &&
							projectionDistance > MAX_COVER_PROJECTION_METERS)
						{
							const float ratio = MAX_COVER_PROJECTION_METERS /
								projectionDistance;
							const float boundedX = x + (projected.x - x) * ratio;
							const float boundedZ = z + (projected.z - z) * ratio;
							if (m_ServerNavigation.Is_PointWalkableExact(boundedX, boundedZ))
							{
								projected.x = boundedX;
								projected.z = boundedZ;
								projectionDistance = MAX_COVER_PROJECTION_METERS;
							}
						}
						if (!hasProjection || projectionDistance > 2.f)
						{
							m_strStatus =
								"Damaging cover volley has no nearby navigation projection: patternId=" +
								patternId + " actionId=" + actionId + " combatObject=" +
								definition->strCombatObjectArchetypeId + " ordinal=" +
								std::to_string(ordinal) + " authoredX=" +
								std::to_string(x) + " authoredZ=" + std::to_string(z) +
								" projectedX=" + std::to_string(projected.x) +
								" projectedZ=" + std::to_string(projected.z) +
								" projectionDistance=" +
								std::to_string(projectionDistance);
							std::cerr << "[DamagingCoverVolleySkipped] " << m_strStatus
								<< '\n';
							skipVolley = true;
							break;
						}
						resolvedPoint = projected;
					}
					if (!std::isfinite(resolvedPoint.x) ||
						!std::isfinite(resolvedPoint.y) ||
						!std::isfinite(resolvedPoint.z) ||
						(!authoredVolleyMayStartOffNavigation &&
						 !m_ServerNavigation.Is_PointWalkableExact(
							 resolvedPoint.x, resolvedPoint.z)))
					{
						m_strStatus =
							"Boss-relative combat object leaves navigable arena: patternId=" +
							patternId + " actionId=" + actionId + " combatObject=" +
							definition->strCombatObjectArchetypeId + " ordinal=" +
							std::to_string(ordinal) + " bossX=" +
							std::to_string(boss.fPositionX) + " bossZ=" +
							std::to_string(boss.fPositionZ) + " spawnX=" +
							std::to_string(x) + " spawnZ=" + std::to_string(z);
						/* A presentation-only object (no gameplay hit, only pulses) never
						   owns damage or navigation authority, so one authored root that
						   lands inside a still-standing wall must not latch the whole room
						   into a runtime failure and close every session. The wave is
						   skipped without staging anything, the exact diagnostic stays in
						   m_strStatus and the Server log, and the wave counter still
						   advances so the skip is not retried every tick. Definitions with
						   any hit keep the strict room-failure path. */
						if (std::isfinite(resolvedPoint.x) &&
							std::isfinite(resolvedPoint.z) &&
							definition->Hits.empty() &&
							!definition->PresentationPulses.empty())
						{
							std::cerr << "[PresentationVolleySkipped] " << m_strStatus
								<< '\n';
							skipVolley = true;
							break;
						}
						return false;
					}
					for (std::uint32_t existingOrdinal = 0u;
						existingOrdinal < ordinal; ++existingOrdinal)
					{
						const auto& [existingX, existingZ] =
							points[existingOrdinal];
						const float deltaX = resolvedPoint.x - existingX;
						const float deltaZ = resolvedPoint.z - existingZ;
						if (deltaX * deltaX + deltaZ * deltaZ <=
							VOLLEY_SPACING_EPSILON)
						{
							m_strStatus =
								"Boss-relative combat object positions overlap";
							return false;
						}
					}
					points[ordinal] = { resolvedPoint.x, resolvedPoint.z };
					resolvedPoints[ordinal] = resolvedPoint;
				}
				if (skipVolley)
				{
					break;
				}
				const std::size_t firstStagedObject =
					combatObjectTransaction.Objects.size();
				const std::size_t firstStagedSpawn =
					combatObjectTransaction.Spawned.size();
				if (!m_CombatObjectRuntime.Stage_BossCombatObject(
					combatObjectTransaction, boss, nullptr, *definition,
					&action.Volley, catalog, count, serverTick, m_strStatus))
				{
					return false;
				}
				if (combatObjectTransaction.Objects.size() !=
						firstStagedObject + count ||
					combatObjectTransaction.Spawned.size() !=
						firstStagedSpawn + count)
				{
					m_strStatus = "Boss-relative combat object staging count is invalid";
					return false;
				}
				for (std::uint32_t ordinal = 0u; ordinal < count; ++ordinal)
				{
					SERVER_COMBAT_OBJECT& staged =
						combatObjectTransaction.Objects[firstStagedObject + ordinal];
					staged.LiveState.CurrentPose.fPositionX =
						resolvedPoints[ordinal].x;
					staged.LiveState.CurrentPose.fPositionY =
						resolvedPoints[ordinal].y;
					staged.LiveState.CurrentPose.fPositionZ =
						resolvedPoints[ordinal].z;
					staged.LiveState.PreviousPose = staged.LiveState.CurrentPose;
					auto& spawned =
						combatObjectTransaction.Spawned[firstStagedSpawn + ordinal];
					spawned.fPositionX = resolvedPoints[ordinal].x;
					spawned.fPositionY = resolvedPoints[ordinal].y;
					spawned.fPositionZ = resolvedPoints[ordinal].z;
				}
				break;
			}
			if (BOSS_COMBAT_OBJECT_ORIGIN_POLICY::LOCKED_TARGET_PER_ALIVE_PLAYER ==
				definition->eOriginPolicy)
			{
				if (!isTypedVolley && action.iValue > 1u)
				{
					m_strStatus =
						"Multi-object per-player spawn requires typed volley layout";
					return false;
				}
				/* The volley is dealt once: every player alive at this edge gets
				one object locked to where they stand, and the whole set shares the
				staging transaction so a single failure drops all of it. */
				std::vector<const SERVER_PLAYER*> aliveTargets;
				for (const auto& [volleyPlayerId, volleyPlayer] : m_Players)
				{
					(void)volleyPlayerId;
					if (0u == volleyPlayer.iCurrentHp ||
						!volleyPlayer.isCombatReady ||
						LostArk::Shared::PLAYER_ACTION_STATE::DEAD ==
							volleyPlayer.eAction ||
						LostArk::Shared::PLAYER_ACTION_STATE::FALLING ==
							volleyPlayer.eAction)
					{
						continue;
					}
					aliveTargets.push_back(&volleyPlayer);
				}
				const std::uint32_t countPerTarget = isTypedVolley ?
					action.Volley.iCountPerResolvedTarget : action.iValue;
				if (0u == countPerTarget || countPerTarget > 8u ||
					(isTypedVolley && action.iValue != countPerTarget))
				{
					m_strStatus =
						"Boss combat object volley count is out of range";
					return false;
				}
				const std::uint32_t maximumTotal = isTypedVolley ?
					action.Volley.iMaximumTotalObjects : 32u;
				const std::uint64_t requestedTotal =
					static_cast<std::uint64_t>(aliveTargets.size()) * countPerTarget +
					(isTypedVolley ? action.Volley.iArenaRandomCount : 0u);
				if (maximumTotal < countPerTarget ||
					requestedTotal > maximumTotal || requestedTotal > 64u)
				{
					m_strStatus = "Boss combat object volley total is out of range";
					return false;
				}
				struct VOLLEY_POINT final
				{
					float fX = 0.f;
					float fZ = 0.f;
				};
				std::vector<VOLLEY_POINT> resolvedPoints;
				resolvedPoints.reserve(static_cast<std::size_t>(requestedTotal));
				if (isTypedVolley && !m_ServerNavigation.Is_Loaded())
				{
					m_strStatus =
						"Boss combat object volley navigation is unavailable";
					return false;
				}
				if (isTypedVolley)
				{
					for (const SERVER_PLAYER* volleyPlayer : aliveTargets)
					{
						const std::size_t targetPointBegin = resolvedPoints.size();
						for (std::uint32_t ordinal = 0u;
							ordinal < countPerTarget; ++ordinal)
						{
							float x = volleyPlayer->fPositionX;
							float z = volleyPlayer->fPositionZ;
							if (BOSS_COMBAT_OBJECT_LAYOUT_KIND::RADIAL ==
								action.Volley.eLayout)
							{
								const float degrees =
									action.Volley.fStartAngleDegrees +
									action.Volley.fAngleStepDegrees *
										static_cast<float>(ordinal);
								const float radians = degrees * DEGREES_TO_RADIANS;
								x += std::sin(radians) * action.Volley.fRadiusM;
								z += std::cos(radians) * action.Volley.fRadiusM;
							}
							if (!std::isfinite(x) || !std::isfinite(z) ||
								!m_ServerNavigation.Is_PointWalkableExact(x, z))
							{
								m_strStatus =
									"Boss combat object volley leaves navigable arena";
								return false;
							}
							resolvedPoints.push_back({ x, z });
						}
						/* Layout belongs to one resolved target. Players may legitimately
						stack, so overlap admission compares only the ordinals dealt around
						that same target. */
						if (!action.Volley.bAllowOverlap)
						{
							const float minimumSpacing =
								Resolve_VolleyMinimumSpacing(*definition);
							const float minimumSpacingSquared =
								minimumSpacing * minimumSpacing;
							for (std::size_t left = targetPointBegin;
								left < resolvedPoints.size(); ++left)
							{
								for (std::size_t right = left + 1u;
								right < resolvedPoints.size(); ++right)
								{
									const float deltaX = resolvedPoints[left].fX -
										resolvedPoints[right].fX;
									const float deltaZ = resolvedPoints[left].fZ -
										resolvedPoints[right].fZ;
									if (deltaX * deltaX + deltaZ * deltaZ +
										VOLLEY_SPACING_EPSILON < minimumSpacingSquared)
									{
										m_strStatus =
											"Boss combat object volley spacing is invalid";
										return false;
									}
								}
							}
						}
					}
				}
				std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET> arenaOrigins;
				if (isTypedVolley &&
					!Resolve_ArenaRandomVolleyOrigins(
						boss, action, *definition, spawnWaveOrdinal,
						arenaOrigins))
				{
					return false;
				}
				for (const SERVER_PLAYER* volleyPlayer : aliveTargets)
				{
					SERVER_COMBAT_OBJECT_LOCKED_TARGET volleyTarget{};
					volleyTarget.iNetEntityId = volleyPlayer->iNetEntityId;
					volleyTarget.fPositionX = volleyPlayer->fPositionX;
					volleyTarget.fPositionY = volleyPlayer->fPositionY;
					volleyTarget.fPositionZ = volleyPlayer->fPositionZ;
					volleyTarget.bTrackUntilFirstPulse = false;
					if (!m_CombatObjectRuntime.Stage_BossCombatObject(
						combatObjectTransaction, boss, &volleyTarget, *definition,
						isTypedVolley ? &action.Volley : nullptr,
						catalog, countPerTarget, serverTick, m_strStatus))
					{
						return false;
					}
				}
				for (const SERVER_COMBAT_OBJECT_LOCKED_TARGET& arenaOrigin :
					arenaOrigins)
				{
					if (!m_CombatObjectRuntime.Stage_BossCombatObject(
						combatObjectTransaction, boss, &arenaOrigin, *definition,
						nullptr, catalog, 1u, serverTick, m_strStatus))
					{
						return false;
					}
				}
				/* An empty arena spawns nothing rather than falling back to the
				boss position. The authored arena supplement remains independent of
				the player count. */
				break;
			}
			if (!m_CombatObjectRuntime.Stage_BossCombatObject(
				combatObjectTransaction, boss, resolvedLockedTarget, *definition,
				nullptr, catalog, action.iValue, serverTick, m_strStatus))
			{
				return false;
			}
			break;
		}
		case BOSS_PATTERN_STAGE_ACTION_KIND::RETURN_TO_ARENA_CENTER:
		{
			SERVER_NAV_POINT center{};
			if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != trigger ||
				"boss.arena.center" != action.strTargetId || 1u != action.iValue ||
				!Resolve_ArenaCenter(boss, center))
				return false;
			break;
		}
		case BOSS_PATTERN_STAGE_ACTION_KIND::RETARGET_RANDOM_ALIVE:
			if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != trigger ||
				"boss.target.pattern" != action.strTargetId ||
				1u != action.iValue || 0u != action.iDurationMs ||
				BOSS_GRABBED_RELEASE_MODE::NONE != action.eReleaseMode ||
				0.f != action.fReleaseSpeedMps ||
				0.f != action.fReleaseYawOffsetDegrees)
			{
				m_strStatus = "Boss retarget stage action is invalid";
				return false;
			}
			break;
		case BOSS_PATTERN_STAGE_ACTION_KIND::SUPPRESS_INTER_STEP_PURSUIT:
			if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT != trigger ||
				"boss.sequence.inter-step-pursuit" != action.strTargetId ||
				0u != action.iValue || 0u != action.iDurationMs ||
				"VALTAN_GHOST_DEATH_AUDITION" != patternId ||
				"valtan.sequence.dead.step-01" != actionId ||
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED !=
					boss.PatternTerminalReceipt.eResult ||
				0u == boss.iPatternSequence ||
				boss.PatternTerminalReceipt.iPatternSequence != boss.iPatternSequence)
			{
				m_strStatus =
					"Boss inter-step pursuit suppression edge is invalid";
				return false;
			}
			break;
		case BOSS_PATTERN_STAGE_ACTION_KIND::DAMAGE_GRABBED_PLAYERS:
		case BOSS_PATTERN_STAGE_ACTION_KIND::EXECUTE_GRABBED_PLAYERS:
		{
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER> stagedPlayers;
			std::vector<LostArk::Shared::DAMAGE_EVENT> stagedDamageEvents;
			if (!Prepare_GrabbedPlayerImpact(
				boss, catalog, action, serverTick, stagedPlayers, stagedDamageEvents))
			{
				return false;
			}
			break;
		}
		case BOSS_PATTERN_STAGE_ACTION_KIND::RELEASE_GRABBED_PLAYERS:
		{
			const bool hold = BOSS_GRABBED_RELEASE_MODE::HOLD ==
				action.eReleaseMode && 0.f == action.fReleaseSpeedMps &&
				0u == action.iDurationMs &&
				0.f == action.fReleaseYawOffsetDegrees;
			const bool knockback =
				(BOSS_GRABBED_RELEASE_MODE::OPPOSITE_KNOCKBACK ==
					action.eReleaseMode ||
				 BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION == action.eReleaseMode) &&
					action.fReleaseSpeedMps > 0.f &&
					action.fReleaseSpeedMps <= 50.f &&
					action.iDurationMs > 0u && action.iDurationMs <= 5000u &&
					(BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION ==
						action.eReleaseMode || 0.f == action.fReleaseYawOffsetDegrees);
			if ("boss.attachment.left-hand" != action.strTargetId ||
				0u != action.iValue || !std::isfinite(action.fReleaseSpeedMps) ||
				!std::isfinite(action.fReleaseYawOffsetDegrees) ||
				std::abs(action.fReleaseYawOffsetDegrees) > 180.f ||
				(!hold && !knockback))
			{
				m_strStatus = "Boss grabbed-player release action is invalid";
				return false;
			}
			for (const auto& [playerId, player] : m_Players)
			{
				(void)playerId;
				if (player.iAttachmentOwnerNetEntityId != boss.iNetEntityId) continue;
				SERVER_PLAYER staged = player;
				const float distance = knockback ? action.fReleaseSpeedMps *
					(static_cast<float>(action.iDurationMs) / 1000.f) : 0.f;
				const bool prepared =
					BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION == action.eReleaseMode ?
					Prepare_ArenaEjection(staged, boss, action, serverTick) :
					Release_PlayerAttachment(staged, boss.iNetEntityId, distance,
						action.iDurationMs, false, 0u, serverTick);
				if (!prepared)
				{
					m_strStatus = "Boss grabbed-player release target is invalid";
					return false;
				}
			}
			break;
		}
		default:
			m_strStatus = "Unsupported boss stage action kind";
			return false;
		}
	}
	return true;
}

bool LostArk::Server::CGameRoom::Prepare_GrabbedPlayerImpact(
	const SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const BOSS_PATTERN_STAGE_ACTION& action,
	const std::uint32_t serverTick,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& stagedPlayers,
	std::vector<LostArk::Shared::DAMAGE_EVENT>& stagedDamageEvents)
{
	using namespace LostArk::Shared;
	stagedPlayers.clear();
	stagedDamageEvents.clear();
	const bool execution = BOSS_PATTERN_STAGE_ACTION_KIND::
		EXECUTE_GRABBED_PLAYERS == action.eKind;
	const bool damage = BOSS_PATTERN_STAGE_ACTION_KIND::
		DAMAGE_GRABBED_PLAYERS == action.eKind;
	if ((!execution && !damage) || 0u == serverTick ||
		0u == boss.iPatternSequence || 0u == boss.iCurrentHp ||
		SERVER_ENTITY_ACTION::DEAD == boss.eAction ||
		BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != action.eTrigger ||
		0u != action.iValue || 0u != action.iDurationMs ||
		BOSS_GRABBED_RELEASE_MODE::NONE != action.eReleaseMode ||
		0.f != action.fReleaseSpeedMps ||
		0.f != action.fReleaseYawOffsetDegrees ||
		(execution && "boss.attachment.left-hand" != action.strTargetId))
	{
		m_strStatus = "Grabbed-player impact action or owner is invalid";
		return false;
	}
	const SERVER_BOSS_GRAB_ROSTER roster =
		CValtanBrain::Classify_GrabbedPlayers(boss, m_Players);
	if (execution && SERVER_BOSS_GRAB_CLASSIFICATION::ALL != roster.eClassification)
	{
		m_strStatus = "Grab execution requires every living participant in this occurrence";
		return false;
	}
	if (m_TickDamageEvents.size() > MAX_DAMAGE_EVENTS ||
		roster.iGrabbedCount > MAX_DAMAGE_EVENTS - m_TickDamageEvents.size())
	{
		m_strStatus = "Grabbed-player impact damage event capacity is exhausted";
		return false;
	}
	const BOSS_RUNTIME_PROFILE* bossProfile = catalog.Find_Boss(boss.strArchetypeId);
	const std::uint32_t rate = damage ?
		catalog.Find_DamageRatePercent(action.strTargetId) : 0u;
	if (damage && (nullptr == bossProfile || 0u == rate))
	{
		m_strStatus = "Grabbed-player impact damage definition is missing";
		return false;
	}
	const std::uint32_t rawDamage = damage ?
		CGameplayCatalog::Resolve_Damage(bossProfile->iAttackPower, rate) : 0u;
	for (const auto& [playerId, player] : m_Players)
	{
		if (PLAYER_ACTION_STATE::GRABBED != player.eAction ||
			player.iAttachmentOwnerNetEntityId != boss.iNetEntityId ||
			PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND != player.eAttachmentSlot ||
			player.iAttachmentPatternSequence != boss.iPatternSequence ||
			0u == player.iCurrentHp)
		{
			continue;
		}
		const auto identity = m_PlayerIdByEntityId.find(player.iNetEntityId);
		const PLAYER_RUNTIME_PROFILE* profile = catalog.Find_Player(player.eCharacterClass);
		if (INVALID_NET_ENTITY_ID == player.iNetEntityId ||
			m_PlayerIdByEntityId.end() == identity || identity->second != playerId ||
			player.iPlayerId != playerId || nullptr == profile ||
			!std::isfinite(player.fPositionX) || !std::isfinite(player.fPositionY) ||
			!std::isfinite(player.fPositionZ) || !std::isfinite(player.fYawDegrees))
		{
			m_strStatus = "Grabbed-player impact target identity or transform is invalid";
			return false;
		}
		SERVER_PLAYER staged = player;
		const std::uint32_t amount = execution ? player.iCurrentHp :
			(std::min)(player.iCurrentHp,
				CGameplayCatalog::Apply_Defense(rawDamage, profile->iDefense));
		staged.iCurrentHp -= amount;
		if (!Release_PlayerAttachment(staged, boss.iNetEntityId,
			0.f, 0u, false, 0u, serverTick))
		{
			m_strStatus = "Grabbed-player impact detach could not be prepared";
			return false;
		}
		staged.iActionStartTick = 0u == staged.iCurrentHp ? serverTick : 0u;
		staged.Projectiles.clear();
		staged.iSpawnedProjectileMask = 0u;
		staged.iAppliedHitMask = 0u;
		staged.hasAppliedSkillDamage = false;
		staged.fFallVelocityY = 0.f;
		staged.iFallDeathTick = 0u;
		stagedPlayers.emplace(playerId, std::move(staged));
		DAMAGE_EVENT event{};
		event.iTargetNetEntityId = player.iNetEntityId;
		event.iAmount = amount;
		event.fPositionX = player.fPositionX;
		event.fPositionY = player.fPositionY;
		event.fPositionZ = player.fPositionZ;
		event.isOutgoing = false;
		stagedDamageEvents.push_back(event);
	}
	if (stagedPlayers.size() != roster.iGrabbedCount)
	{
		m_strStatus = "Grabbed-player impact roster changed during preparation";
		return false;
	}
	return true;
}

bool LostArk::Server::CGameRoom::Commit_BossPatternPlayerStageActions(
	SERVER_WORLD_ENTITY& boss,
	const CGameplayCatalog& catalog,
	const std::string& patternId,
	const std::string& actionId,
	const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
	const std::uint32_t serverTick,
	const std::uint32_t spawnWaveOrdinal)
{
	using namespace LostArk::Shared;
	if (patternId.empty() || actionId.empty() || spawnWaveOrdinal > 0u)
		return true;
	const auto* patterns = catalog.Find_BossPatterns(boss.strEncounterId);
	if (nullptr == patterns)
		return false;
	const auto pattern = std::find_if(
		patterns->begin(), patterns->end(),
		[&patternId](const BOSS_PATTERN_DEFINITION& candidate)
		{
			return candidate.strPatternId == patternId;
		});
	if (patterns->end() == pattern)
		return false;
	const auto stage = std::find_if(
		pattern->Stages.begin(), pattern->Stages.end(),
		[&actionId](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
		{
			return candidate.strActionId == actionId;
		});
	if (pattern->Stages.end() == stage)
		return false;

	for (const BOSS_PATTERN_STAGE_ACTION& action : stage->Actions)
	{
		if (action.eTrigger != trigger)
			continue;
		if (BOSS_PATTERN_STAGE_ACTION_KIND::SUPPRESS_INTER_STEP_PURSUIT ==
			action.eKind)
		{
			if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT != trigger ||
				"boss.sequence.inter-step-pursuit" != action.strTargetId ||
				0u != action.iValue || 0u != action.iDurationMs ||
				"VALTAN_GHOST_DEATH_AUDITION" != patternId ||
				"valtan.sequence.dead.step-01" != actionId ||
				SERVER_BOSS_PATTERN_TERMINAL_RESULT::COMPLETED !=
					boss.PatternTerminalReceipt.eResult ||
				boss.PatternTerminalReceipt.iPatternSequence != boss.iPatternSequence)
			{
				return false;
			}
			/* FinishPattern already advanced the ordered cursor and reserved the
			default delay. Consume only that delay; Debug COMPLETED_HOLD and the
			pattern-flow lifecycle remain owned by their existing state machines. */
			boss.iAutomaticPatternSequencePursuitTicksRemaining = 0u;
			continue;
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::RETURN_TO_ARENA_CENTER == action.eKind)
		{
			SERVER_NAV_POINT center{};
			if (!Resolve_ArenaCenter(boss, center))
				return false;
			boss.fPositionX = center.x;
			boss.fPositionY = center.y;
			boss.fPositionZ = center.z;
			boss.MovePath.clear();
			boss.PatternStageRootMotion.clear();
			boss.fPatternForcedMotionSpeed = 0.f;
			continue;
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::RETARGET_RANDOM_ALIVE ==
			action.eKind)
		{
			std::vector<SERVER_PLAYER*> candidates;
			candidates.reserve(m_Players.size());
			for (auto& [playerId, player] : m_Players)
			{
				(void)playerId;
				if (0u == player.iCurrentHp || !player.isCombatReady ||
					PLAYER_ACTION_STATE::GRABBED == player.eAction ||
					PLAYER_ACTION_STATE::DEAD == player.eAction ||
					PLAYER_ACTION_STATE::FALLING == player.eAction)
				{
					continue;
				}
				candidates.push_back(&player);
			}
			if (candidates.empty())
			{
				boss.iPatternTargetEntityId = INVALID_NET_ENTITY_ID;
				boss.bHasPatternTargetLastPosition = false;
				continue;
			}
			const std::uint64_t seed = Mix_DeterministicRandom(
				Hash_StableId(actionId) ^ Hash_StableId(action.strTargetId) ^
				(static_cast<std::uint64_t>(boss.iNetEntityId) << 32u) ^
				static_cast<std::uint64_t>(boss.iPatternSequence) ^
				(static_cast<std::uint64_t>(serverTick) << 1u));
			SERVER_PLAYER& selected = *candidates[
				static_cast<std::size_t>(seed % candidates.size())];
			boss.iTargetEntityId = selected.iNetEntityId;
			boss.iPatternTargetEntityId = selected.iNetEntityId;
			boss.bHasPatternTargetLastPosition = true;
			boss.fPatternTargetLastPositionX = selected.fPositionX;
			boss.fPatternTargetLastPositionY = selected.fPositionY;
			boss.fPatternTargetLastPositionZ = selected.fPositionZ;
			if (BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH !=
				stage->Motion.eKind)
			{
				const float deltaX = selected.fPositionX - boss.fPositionX;
				const float deltaZ = selected.fPositionZ - boss.fPositionZ;
				if (deltaX * deltaX + deltaZ * deltaZ > 0.000001f)
				{
					boss.fYawDegrees =
						std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
					// The catalog Big Saydon face/hammer forward is measured +X, while yaw faces +Z.
					if (boss.strArchetypeId == "BOSS_KAKULSAYDON_G2_BIG_SAYDON")
						boss.fYawDegrees -= 90.f;
				}
			}
			continue;
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_BIND == action.eKind)
		{
			std::map<PLAYER_ID, SERVER_PLAYER> stagedPlayers;
			if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger)
			{
				const auto target = std::find_if(
					m_Players.begin(), m_Players.end(),
					[&boss](const auto& entry)
					{
						return entry.second.iNetEntityId ==
							boss.iPatternTargetEntityId;
					});
				if (m_Players.end() == target)
					return false;
				SERVER_PLAYER staged = target->second;
				const float restoreX = staged.fPositionX;
				const float restoreY = staged.fPositionY;
				const float restoreZ = staged.fPositionZ;
				const float restoreYaw = staged.fYawDegrees;
				const bool restoreCombatReady = staged.isCombatReady;
				Cancel_PlayerActionForPatternStatus(staged);
				staged.bPatternBound = true;
				staged.iPatternBindOwnerNetEntityId = boss.iNetEntityId;
				staged.iPatternBindSequence = boss.iPatternSequence;
				staged.iPatternBindEndTick = Add_ServerTicksSkippingReservedZero(
					0u == serverTick ? 1u : serverTick,
					DurationMillisecondsToServerTicks(action.iDurationMs));
				staged.fPatternBindRestoreX = restoreX;
				staged.fPatternBindRestoreY = restoreY;
				staged.fPatternBindRestoreZ = restoreZ;
				staged.fPatternBindRestoreYawDegrees = restoreYaw;
				staged.bPatternBindRestoreCombatReady = restoreCombatReady;
				staged.fPositionY = restoreY +
					static_cast<float>(action.iValue) / 1000.f;
				stagedPlayers.emplace(target->first, std::move(staged));
			}
			else
			{
				for (const auto& [playerId, player] : m_Players)
				{
					if (!player.bPatternBound ||
						player.iPatternBindOwnerNetEntityId != boss.iNetEntityId ||
						player.iPatternBindSequence != boss.iPatternSequence)
					{
						continue;
					}
					SERVER_PLAYER staged = player;
					if (!Restore_PatternBoundPlayer(staged))
						return false;
					stagedPlayers.emplace(playerId, std::move(staged));
				}
			}
			for (auto& [playerId, staged] : stagedPlayers)
			{
				m_CombatObjectRuntime.Cancel_Source(staged.iNetEntityId);
				m_Players.at(playerId) = std::move(staged);
			}
			continue;
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::SET_PLAYER_SILENCE == action.eKind)
		{
			std::map<PLAYER_ID, SERVER_PLAYER> stagedPlayers;
			for (const auto& [playerId, player] : m_Players)
			{
				if (0u == player.iCurrentHp ||
					PLAYER_ACTION_STATE::DEAD == player.eAction)
				{
					continue;
				}
				SERVER_PLAYER staged = player;
				staged.iSilenceOwnerNetEntityId = boss.iNetEntityId;
				staged.iSilencePatternSequence = boss.iPatternSequence;
				staged.iSilenceEndTick = Add_ServerTicksSkippingReservedZero(
					0u == serverTick ? 1u : serverTick,
					DurationMillisecondsToServerTicks(action.iDurationMs));
				staged.iSilenceDurationTicks =
					DurationMillisecondsToServerTicks(action.iDurationMs);
				if (PLAYER_PENDING_COMMAND_KIND::SKILL ==
					staged.PendingCommand.eKind)
				{
					staged.PendingCommand.Clear();
				}
				stagedPlayers.emplace(playerId, std::move(staged));
			}
			for (auto& [playerId, staged] : stagedPlayers)
				m_Players.at(playerId) = std::move(staged);
			continue;
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::DAMAGE_GRABBED_PLAYERS == action.eKind ||
			BOSS_PATTERN_STAGE_ACTION_KIND::EXECUTE_GRABBED_PLAYERS == action.eKind)
		{
			std::map<PLAYER_ID, SERVER_PLAYER> stagedPlayers;
			std::vector<DAMAGE_EVENT> stagedDamageEvents;
			if (!Prepare_GrabbedPlayerImpact(
				boss, catalog, action, serverTick, stagedPlayers, stagedDamageEvents))
			{
				return false;
			}
			/* Allocate before publishing any player copy. The remaining moves and
			POD event copies cannot expose a partially executed roster. */
			m_TickDamageEvents.reserve(m_TickDamageEvents.size() + stagedDamageEvents.size());
			for (auto& [playerId, staged] : stagedPlayers)
			{
				m_CombatObjectRuntime.Cancel_Source(staged.iNetEntityId);
				m_Players.at(playerId) = std::move(staged);
			}
			m_TickDamageEvents.insert(m_TickDamageEvents.end(),
				stagedDamageEvents.begin(), stagedDamageEvents.end());
			if (BOSS_PATTERN_STAGE_ACTION_KIND::EXECUTE_GRABBED_PLAYERS == action.eKind)
			{
				boss.iGrabExecutionCommittedPatternSequence = boss.iPatternSequence;
				boss.iGrabExecutionCommittedStageIndex = boss.iPatternStageIndex;
			}
			continue;
		}
		if (BOSS_PATTERN_STAGE_ACTION_KIND::RELEASE_GRABBED_PLAYERS ==
			action.eKind)
		{
			const float distance =
				BOSS_GRABBED_RELEASE_MODE::OPPOSITE_KNOCKBACK ==
					action.eReleaseMode ?
				action.fReleaseSpeedMps *
					(static_cast<float>(action.iDurationMs) / 1000.f) : 0.f;
			std::map<PLAYER_ID, SERVER_PLAYER> stagedPlayers;
			for (const auto& [playerId, player] : m_Players)
			{
				if (player.iAttachmentOwnerNetEntityId != boss.iNetEntityId)
					continue;
				SERVER_PLAYER staged = player;
				const bool prepared =
					BOSS_GRABBED_RELEASE_MODE::ARENA_EJECTION == action.eReleaseMode ?
					Prepare_ArenaEjection(staged, boss, action, serverTick) :
					Release_PlayerAttachment(staged, boss.iNetEntityId, distance,
						action.iDurationMs, false, 0u, serverTick);
				if (!prepared)
					return false;
				stagedPlayers.emplace(playerId, std::move(staged));
			}
			for (auto& [playerId, staged] : stagedPlayers)
				m_Players.at(playerId) = std::move(staged);
		}
	}
	if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
		BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_TARGET_RUSH == stage->Motion.eKind)
	{
		if (!CValtanBrain::Lock_PortalTargetRushAtStageStart(boss))
		{
			m_strStatus = "Portal target rush Stage entered without a valid locked route";
			return false;
		}
	}
	else if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER == trigger &&
		BOSS_PATTERN_STAGE_MOTION_KIND::PORTAL_CROSS_ARENA == stage->Motion.eKind)
		CValtanBrain::Configure_PortalMotion(boss, *stage);
	return true;
}

void LostArk::Server::CGameRoom::Drain_BossCombatEvents()
{
	using namespace LostArk::Shared;
	for (SERVER_WORLD_ENTITY& boss : m_WorldEntities)
	{
		if (WORLD_BOOTSTRAP_KIND::BOSS != boss.eKind)
			continue;
		auto& pending = boss.BossCombat.PendingPartBreakEdges;
		while (!pending.empty() &&
			m_TickBossCombatEvents.size() < MAX_BOSS_COMBAT_EVENTS)
		{
			const SERVER_BOSS_PART_BREAK_EDGE edge = pending.front();
			BOSS_COMBAT_EVENT event{};
			event.iEventSequence = m_iNextBossCombatEventSequence;
			event.iEventTick = edge.iServerTick;
			event.iBossNetEntityId = boss.iNetEntityId;
			event.eKind = BOSS_COMBAT_EVENT_KIND::PART_BROKEN;
			event.iPartMask = edge.iPartMask;
			m_TickBossCombatEvents.push_back(event);
			pending.erase(pending.begin());
			m_iNextBossCombatEventSequence =
				(std::numeric_limits<std::uint64_t>::max)() ==
					m_iNextBossCombatEventSequence ?
				1u : m_iNextBossCombatEventSequence + 1u;
		}
	}
}

bool LostArk::Server::CGameRoom::Commit_DueEncounterProps(
	const std::uint32_t serverTick)
{
	if (!m_EncounterPropRuntime.Is_Initialized() || 0u == serverTick)
		return true;
	CEncounterPropRuntime staged = m_EncounterPropRuntime;
	bool broadcast = false;
	std::string status;
	const bool breakDue = 0u != m_iPillarAuditionBreakTick &&
		Has_ReachedServerTick(serverTick, m_iPillarAuditionBreakTick);
	if (breakDue)
	{
		ENCOUNTER_PROP_TRANSACTION breakTransaction{};
		const auto prepared = staged.Prepare_Break(
			staged.Get_OccurrenceSequence(), serverTick, breakTransaction, status);
		if (ENCOUNTER_PROP_PREPARE_RESULT::REJECTED == prepared ||
			(ENCOUNTER_PROP_PREPARE_RESULT::READY == prepared &&
			 !staged.Commit(breakTransaction, status)))
		{
			m_strStatus = "Encounter prop break commit failed: " + status;
			return false;
		}
		if (ENCOUNTER_PROP_PREPARE_RESULT::READY == prepared)
		{
			broadcast = true;
		}
	}
	ENCOUNTER_PROP_TRANSACTION transaction{};
	const auto prepared = staged.Prepare_DueRemoval(
		serverTick, PILLAR_BREAKING_TICKS, transaction, status);
	if (ENCOUNTER_PROP_PREPARE_RESULT::REJECTED == prepared ||
		(ENCOUNTER_PROP_PREPARE_RESULT::READY == prepared &&
		 !staged.Commit(transaction, status)))
	{
		m_strStatus = "Encounter prop removal commit failed: " + status;
		return false;
	}
	broadcast = broadcast || ENCOUNTER_PROP_PREPARE_RESULT::READY == prepared;
	if (breakDue)
		m_iPillarAuditionBreakTick = 0u;
	if (broadcast)
	{
		m_EncounterPropRuntime = std::move(staged);
		Broadcast_EncounterPropSync();
	}
	return true;
}

bool LostArk::Server::CGameRoom::Send_EncounterPropSync(
	const std::shared_ptr<CClientSession>& session)
{
	using namespace LostArk::Shared;
	/* A late joiner is told the live slot states and nothing else. The raise and
	   shatter one-shots already happened for the players who were here. */
	if (!m_EncounterPropRuntime.Is_Initialized() || nullptr == session)
		return true;
	S2C_ENCOUNTER_PROP_SYNC message{};
	message.strPropSetId = m_EncounterPropRuntime.Get_PropSetId();
	message.iServerTick = 0u == m_iServerTick ? 1u : m_iServerTick;
	message.iEncounterEpoch = m_EncounterPropRuntime.Get_EncounterEpoch();
	for (const ENCOUNTER_PROP_SLOT_STATE& slot :
		m_EncounterPropRuntime.Get_SlotStates())
	{
		ENCOUNTER_PROP_SLOT_WIRE wire{};
		wire.strSlotId = slot.strSlotId;
		wire.eState = slot.eState;
		wire.iStateVersion = slot.iStateVersion;
		wire.iStateStartTick = slot.iStateStartTick;
		wire.iOccurrenceSequence = slot.iOccurrenceSequence;
		message.Slots.push_back(std::move(wire));
	}
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return false;
	return session->Send_Frame(
		PACKET_TYPE::S2C_ENCOUNTER_PROP_SYNC, writer.Get_Buffer());
}

void LostArk::Server::CGameRoom::Broadcast_EncounterPropSync()
{
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
		if (nullptr != session && !Send_EncounterPropSync(session))
			session->Request_Close();
	}
}
