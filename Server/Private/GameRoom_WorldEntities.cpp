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

void LostArk::Server::CGameRoom::Rollback_Join(const SESSION_ID sessionId)
{
	using namespace LostArk::Shared;
	const auto sessionPlayerIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionPlayerIter == m_PlayerIdBySessionId.end())
		return;
	const PLAYER_ID playerId = sessionPlayerIter->second;
	const auto playerIter = m_Players.find(playerId);
	if (playerIter != m_Players.end())
	{
		m_PlayerIdByEntityId.erase(playerIter->second.iNetEntityId);
		m_Players.erase(playerIter);
	}
	m_PlayerIdBySessionId.erase(sessionPlayerIter);
	if (const std::shared_ptr<CClientSession> session = Find_Session(sessionId))
		session->Bind_PlayerId(INVALID_PLAYER_ID);
}

bool LostArk::Server::CGameRoom::Is_PlayerAdmissionFull() const
{
	if (LostArk::Shared::WORLD_ID::VALTAN_ARENA == m_eWorldId &&
		m_Players.size() >= LostArk::Shared::MAX_VALTAN_RAID_PLAYERS)
	{
		return true;
	}
	return nullptr == Find_AvailablePlayerSpawn();
}

const LostArk::Server::WORLD_BOOTSTRAP_PLACEMENT*
LostArk::Server::CGameRoom::Find_AvailablePlayerSpawn() const
{
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
		m_WorldBootstrap.Get_Placements())
	{
		if (!placement.isEnabled ||
			placement.eKind != WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN)
		{
			continue;
		}
		const bool isUsed = std::any_of(
			m_Players.begin(), m_Players.end(),
			[&placement](const auto& playerEntry)
			{
				return playerEntry.second.strSpawnPlacementId ==
					placement.strPlacementId;
			});
		if (!isUsed)
			return &placement;
	}
	return nullptr;
}

const LostArk::Server::WORLD_BOOTSTRAP_PLACEMENT*
LostArk::Server::CGameRoom::Find_Placement(
	const std::string& placementId) const
{
	const auto& placements = m_WorldBootstrap.Get_Placements();
	const auto iter = std::find_if(
		placements.begin(),
		placements.end(),
		[&placementId](const WORLD_BOOTSTRAP_PLACEMENT& placement)
		{
			return placement.strPlacementId == placementId;
		});
	return placements.end() != iter ? &*iter : nullptr;
}

bool LostArk::Server::CGameRoom::Build_WorldEntity(
	const WORLD_BOOTSTRAP_PLACEMENT& placement,
	const LostArk::Shared::NET_ENTITY_ID netEntityId,
	SERVER_WORLD_ENTITY& outEntity,
	const CGameplayCatalog* definitionCatalog,
	const LostArk::Shared::NET_ENTITY_ID ownerBossNetEntityId)
{
	const CGameplayCatalog& catalog = nullptr == definitionCatalog ?
		m_GameplayCatalog.Active() : *definitionCatalog;
	if (LostArk::Shared::INVALID_NET_ENTITY_ID == netEntityId ||
		WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::TRIGGER_BOX == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::COLLISION_BOX == placement.eKind ||
		WORLD_BOOTSTRAP_KIND::END == placement.eKind ||
		(LostArk::Shared::INVALID_NET_ENTITY_ID != ownerBossNetEntityId &&
			(WORLD_BOOTSTRAP_KIND::BOSS != placement.eKind ||
			 ownerBossNetEntityId == netEntityId)))
	{
		m_strStatus = "World entity placement is invalid";
		return false;
	}

	SERVER_WORLD_ENTITY staged{};
	staged.iNetEntityId = netEntityId;
	staged.iOwnerBossNetEntityId = ownerBossNetEntityId;
	staged.PinnedDefinitionRevision =
		catalog.Get_ActiveRevision();
	if (!staged.PinnedDefinitionRevision.Is_Valid())
	{
		m_strStatus = "Active gameplay revision is unavailable";
		return false;
	}
	staged.strPlacementId = placement.strPlacementId;
	staged.strArchetypeId = placement.strArchetypeId;
	staged.strEncounterId = placement.strEncounterId;
	staged.eKind = placement.eKind;
	staged.fPositionX = placement.fPositionX;
	staged.fPositionY = placement.fPositionY;
	staged.fPositionZ = placement.fPositionZ;
	staged.fYawDegrees = placement.fYawDegrees;
	if (WORLD_BOOTSTRAP_KIND::NPC == staged.eKind)
	{
		/* NPCs are non-combat living bodies. Keep their liveness explicit so
		the body rebuild cannot silently drop them through a zero HP default.
		Their wire collision radius stays zero by Shared contract; the Server's
		blocking-body rebuild owns the separate player-sized NPC body. */
		staged.iCurrentHp = 1u;
		staged.iMaximumHp = 1u;
		staged.fCollisionRadius = 0.f;
		staged.strActionId = CNpcBehaviorRuntime::IDLE_ACTION_ID;
		staged.iActionStartTick = 0u == m_iServerTick ? 1u : m_iServerTick;
		if (placement.bHasNpcBehavior &&
			!m_NpcBehaviorRuntime.Initialize(
				placement, m_ServerNavigation,
				staged.iActionStartTick, staged, m_strStatus))
		{
			return false;
		}
	}
	if (WORLD_BOOTSTRAP_KIND::BOSS == staged.eKind)
	{
		const BOSS_RUNTIME_PROFILE* profile =
			catalog.Find_Boss(staged.strArchetypeId);
		const auto* patterns =
			catalog.Find_BossPatterns(staged.strEncounterId);
		if (nullptr == profile ||
			profile->strEncounterId != staged.strEncounterId ||
			nullptr == patterns || patterns->empty())
		{
			m_strStatus = "Boss gameplay profile or damage profile is missing";
			return false;
		}
		const bool isDependentArchetype = std::any_of(patterns->begin(), patterns->end(),
			[&staged](const BOSS_PATTERN_DEFINITION& pattern)
			{
				return BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP == pattern.Finale.eKind &&
					pattern.Finale.strGhostArchetypeId == staged.strArchetypeId;
			});
		const bool isKoukuClone = LostArk::Shared::INVALID_NET_ENTITY_ID != ownerBossNetEntityId &&
			LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId &&
			staged.strArchetypeId.starts_with(KOUKUSAYDON_ARENA_BOSS_ARCHETYPE_PREFIX);
		if ((isDependentArchetype || isKoukuClone) !=
			(LostArk::Shared::INVALID_NET_ENTITY_ID != ownerBossNetEntityId))
		{
			m_strStatus = "Boss archetype requires its declared primary/dependent spawn role";
			return false;
		}
		if (isDependentArchetype || isKoukuClone)
		{
			const auto owner = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
				[ownerBossNetEntityId](const SERVER_WORLD_ENTITY& candidate)
				{ return candidate.iNetEntityId == ownerBossNetEntityId; });
			if (owner == m_WorldEntities.end() || WORLD_BOOTSTRAP_KIND::BOSS != owner->eKind ||
				LostArk::Shared::INVALID_NET_ENTITY_ID != owner->iOwnerBossNetEntityId ||
				0u == owner->iCurrentHp || SERVER_ENTITY_ACTION::DEAD == owner->eAction ||
				owner->bMechanicLedgerRequiresReset || owner->strEncounterId != staged.strEncounterId ||
				owner->PinnedDefinitionRevision != catalog.Get_ActiveRevision())
			{
				m_strStatus = "Dependent boss requires a live primary in its pinned encounter";
				return false;
			}
			const bool ownerRunsFinale = std::any_of(patterns->begin(), patterns->end(),
				[&owner, &staged, isKoukuClone](const BOSS_PATTERN_DEFINITION& pattern)
				{
					if (isKoukuClone)
						return pattern.strPatternId == owner->strPatternId &&
							owner->strArchetypeId == staged.strArchetypeId &&
							std::any_of(pattern.MechanicTriggers.begin(), pattern.MechanicTriggers.end(),
								[](const BOSS_PATTERN_MECHANIC_TRIGGER& trigger)
								{ return BOSS_PATTERN_MECHANIC_TRIGGER_KIND::REAL_GAZE_TELEPORT == trigger.eKind; });
					const bool directFinaleOccurrence =
						pattern.strPatternId == owner->strPatternId;
					const bool phaseThreeFinaleController =
						owner->bGhostPhasePatternLoopActive && 3u == owner->iPhase &&
						"BOSS_VALTAN" == owner->strArchetypeId &&
						"boss.valtan.center" == owner->strPlacementId &&
						"VALTAN_GHOST_FINALE" == pattern.strPatternId;
					return (directFinaleOccurrence || phaseThreeFinaleController) &&
						BOSS_PATTERN_FINALE_KIND::GHOST_PORTAL_LOOP == pattern.Finale.eKind &&
						pattern.Finale.strGhostArchetypeId == staged.strArchetypeId;
				});
			if (!ownerRunsFinale)
			{
				m_strStatus = "Dependent boss owner is not running its declared finale";
				return false;
			}
		}
		const std::vector<BOSS_PART_DEFINITION>* bossParts =
			catalog.Find_BossParts(staged.strArchetypeId);
		const std::vector<BOSS_PART_DEFINITION> noBossParts;
		std::string combatStatus;
		if (!CBossCombatRuntime::Initialize(
			staged.BossCombat,
			nullptr == bossParts ? noBossParts : *bossParts,
			combatStatus))
		{
			m_strStatus = std::move(combatStatus);
			return false;
		}
		staged.iCurrentHp = profile->iMaximumHp;
		staged.iMaximumHp = profile->iMaximumHp;
		staged.iMaximumHealthBars = profile->iMaximumHealthBars;
		staged.iLastEvaluatedHealthBar = profile->iMaximumHealthBars;
		staged.iAttackPower = profile->iAttackPower;
		staged.fCollisionRadius = profile->fCollisionRadius;
		staged.fEngageDistance = profile->fEngageDistance;
		staged.fMoveSpeed = profile->fMoveSpeed;
		staged.PhasePolicy = profile->PhasePolicy;
		staged.iPhaseTwoHpPercent = profile->PhasePolicy.iThresholdPercent;
		/* Every plate starts intact. A boss with no authored plates keeps an
		empty list and therefore no mitigation, which is the pre-armour rule. */
		staged.ArmorPlates.clear();
		staged.ArmorPlates.reserve(profile->ArmorPlates.size());
		for (const BOSS_ARMOR_PLATE& plate : profile->ArmorPlates)
		{
			SERVER_BOSS_ARMOR_PLATE_STATE state{};
			state.iPlateIndex = plate.iPlateIndex;
			state.iRemainingDurability = plate.iDurability;
			state.iDefense = plate.iDefense;
			staged.ArmorPlates.push_back(state);
		}
		if (m_ServerNavigation.Is_Loaded())
		{
			/* An authored placement on a walkable cell keeps its exact XZ and
			only takes the grid height. Snapping it to the cell centre moved the
			KoukuSaydon gate bosses up to 2.8 m on that area's 4 m grid, onto
			the fixed player position the gate teleports to. Off-grid or
			blocked placements still project to the nearest walkable cell. */
			/* Big Saydon stands above the G2 floor. Its saved transform owns
			the height while navigation still admits its XZ footprint. */
			const bool preserveAuthoredHeight =
				LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId &&
				staged.strArchetypeId == "BOSS_KAKULSAYDON_G2_BIG_SAYDON";
			SERVER_NAV_POINT projected{};
			if (m_ServerNavigation.Is_PointWalkableExact(
					staged.fPositionX, staged.fPositionZ) &&
				m_ServerNavigation.Sample_Position(
					staged.fPositionX, staged.fPositionZ, projected))
			{
				if (!preserveAuthoredHeight)
					staged.fPositionY = projected.y;
			}
			else if (m_ServerNavigation.Project_Point(
				staged.fPositionX,
				staged.fPositionZ,
				projected))
			{
				staged.fPositionX = projected.x;
				if (!preserveAuthoredHeight)
					staged.fPositionY = projected.y;
				staged.fPositionZ = projected.z;
			}
			else
			{
				m_strStatus = "Boss placement is outside server navigation";
				return false;
			}
		}
	}
	staged.fSpawnPositionX = staged.fPositionX;
	staged.fSpawnPositionY = staged.fPositionY;
	staged.fSpawnPositionZ = staged.fPositionZ;
	outEntity = std::move(staged);
	return true;
}

bool LostArk::Server::CGameRoom::Initialize_WorldEntities()
{
	m_WorldEntities.clear();
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement :
		m_WorldBootstrap.Get_Placements())
	{
		if (!placement.isEnabled ||
			placement.eKind == WORLD_BOOTSTRAP_KIND::PLAYER_SPAWN ||
			placement.eKind == WORLD_BOOTSTRAP_KIND::TRIGGER_BOX ||
			placement.eKind == WORLD_BOOTSTRAP_KIND::COLLISION_BOX)
		{
			continue;
		}
		if (m_iNextNetEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID)
		{
			m_strStatus = "World entity ID space exhausted";
			return false;
		}
		SERVER_WORLD_ENTITY entity{};
		if (!Build_WorldEntity(placement, m_iNextNetEntityId, entity))
			return false;
		++m_iNextNetEntityId;
		m_WorldEntities.push_back(std::move(entity));
	}
	return true;
}

bool LostArk::Server::CGameRoom::Reset_ReplayableArenaWhenEmpty()
{
	using LostArk::Shared::WORLD_ID;
	if ((WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId &&
		WORLD_ID::VALTAN_ARENA != m_eWorldId &&
		WORLD_ID::KAKULSAYDON_ARENA != m_eWorldId) || !m_Players.empty())
		return true;

#ifdef _DEBUG
	Clear_KoukuSaydonPatternAudition();
#endif
	std::string resetStatus;
	if (!m_ServerTriggerSystem.Initialize(
		m_WorldBootstrap.Get_Placements(), resetStatus))
	{
		m_strStatus = std::move(resetStatus);
		Mark_RuntimeFailure("empty-arena-reset.trigger-system");
		return false;
	}
	if (!m_SpawnGroupRuntime.Initialize(m_SpawnGroupBootstrap, resetStatus))
	{
		m_strStatus = std::move(resetStatus);
		Mark_RuntimeFailure("empty-arena-reset.spawn-groups");
		return false;
	}
	if (!Initialize_WorldEntities())
	{
		Mark_RuntimeFailure("empty-arena-reset.world-entities");
		return false;
	}
	m_CombatObjectRuntime.Reset();
	m_CombatObjectRuntime.Discard_PendingLifecycle();
	m_TickDamageEvents.clear();
	m_TickBossCombatEvents.clear();
	m_iNextBossCombatEventSequence = 1u;
	m_ValtanPatternIdAuditionSequenceBySessionId.clear();
	m_ValtanPatternFlowStartSequenceBySessionId.clear();
	m_ValtanPatternFlowControlSequenceBySessionId.clear();
#ifdef _DEBUG
	m_KoukuSaydonPatternAuditionReceiptBySessionId.clear();
	m_PendingKoukuSaydonPatternAuditionLifecycle.clear();
	Cancel_ValtanPatternIdAudition("room reset after the last player left");
	m_ValtanNextPatternReceiptBySessionId.clear();
	m_ValtanPatternFlowAudition = {};
	m_ValtanFightPageStart = {};
#endif
	m_strStatus = "Replayable arena reset after the room became empty";
	return true;
}

bool LostArk::Server::CGameRoom::Reset_ValtanArenaWhenEmpty()
{
	using LostArk::Shared::WORLD_ID;
	if (WORLD_ID::VALTAN_ARENA != m_eWorldId || !m_Players.empty())
		return true;

	std::string resetStatus;
	const std::uint32_t resetTick = 0u == m_iServerTick ? 1u : m_iServerTick;
	if (!m_SpawnGroupRuntime.Initialize(m_SpawnGroupBootstrap, resetStatus))
	{
		m_strStatus = std::move(resetStatus);
		Mark_RuntimeFailure("valtan-empty-reset.spawn-groups");
		return false;
	}
	m_iPillarAuditionBreakTick = 0u;
	m_bPillarAuditionCycleArmed = false;
#ifdef _DEBUG
	m_ValtanTimelineAudition = {};
	m_ValtanFightPageStart = {};
#endif
	if (m_EncounterPropRuntime.Is_Initialized() &&
		!m_EncounterPropRuntime.Reset(resetStatus, resetTick))
	{
		m_strStatus = std::move(resetStatus);
		Mark_RuntimeFailure("valtan-empty-reset.encounter-props");
		return false;
	}
	if (!m_WorldDestructionRuntime.Reset(resetStatus, resetTick))
	{
		m_strStatus = std::move(resetStatus);
		Mark_RuntimeFailure("valtan-empty-reset.world-destruction");
		return false;
	}
	m_ServerCollisionSystem.Reset_RuntimeStates();
	m_ServerNavigation.Reset_RuntimeBlockers();
	m_iNextWorldDestructionEventSequence = 1u;
	m_iNextBossCombatEventSequence = 1u;
	if (!Initialize_WorldEntities())
	{
		Mark_RuntimeFailure("valtan-empty-reset.world-entities");
		return false;
	}
	m_TickDamageEvents.clear();
	m_TickBossCombatEvents.clear();
	/* The encounter is fresh, so a bar armed by the previous occupants no
	longer describes any live boss. Leave already dropped their sequences. */
	m_iValtanAuditionArmedHealthBar = 0u;
	// The next party charges its Esther gauge from zero; any live summon was
	// already discarded with the entity rebuild above, and a summon still in
	// its landing delay has no party left to land for.
	m_PendingEstherSummons.clear();
	m_EstherSkillSystem.Reset();
	m_bValtanRaidCleared = false;
	m_strStatus = "Valtan arena reset after the room became empty";
	return true;
}

bool LostArk::Server::CGameRoom::Apply_EncounterPropStageEntry(
	const SERVER_WORLD_ENTITY& boss,
	const std::uint32_t serverTick)
{
	if (!m_EncounterPropRuntime.Is_Initialized() ||
		0u == boss.iPatternSequence || boss.strPatternId.empty() ||
		boss.strPatternStageId.empty())
	{
		return true;
	}
	/* A later pattern owns each shatter, because the stele set outlives the
	   pattern that raised it. The stage edge names the pair it breaks, so the
	   pair that goes leaves the opposite pair standing as the cover the raid
	   moves to. */
	const CGameplayCatalog* occurrenceCatalog =
		Resolve_ValtanGameplayCatalog(boss);
	if (nullptr == occurrenceCatalog)
	{
		m_strStatus = "Encounter prop pinned gameplay generation is missing";
		return false;
	}
	if (const std::vector<BOSS_PATTERN_DEFINITION>* propBreakPatterns =
		occurrenceCatalog->Find_BossPatterns(boss.strEncounterId))
	{
		const auto propBreakPattern = std::find_if(
			propBreakPatterns->begin(), propBreakPatterns->end(),
			[&boss](const BOSS_PATTERN_DEFINITION& candidate)
			{ return candidate.strPatternId == boss.strPatternId; });
		if (propBreakPatterns->end() != propBreakPattern &&
			boss.iPatternStageIndex < propBreakPattern->Stages.size())
		{
			const BOSS_PATTERN_STAGE_DEFINITION& propBreakStage =
				propBreakPattern->Stages[boss.iPatternStageIndex];
			const auto& propSlots = m_EncounterPropRuntime.Get_SlotStates();
			bool allowScriptedPropBreak = false;
#ifdef _DEBUG
			allowScriptedPropBreak = boss.bScriptedPatternPlayback &&
				VALTAN_TIMELINE_AUDITION_PHASE::INACTIVE !=
					m_ValtanTimelineAudition.ePhase &&
				boss.iNetEntityId == m_ValtanTimelineAudition.iBossEntityId &&
				m_ValtanTimelineAudition.bAllowProductPropBreak;
#endif
			/* Only a raised pair can shatter. The wave is an ordinary rotation
			   pattern that also runs when no stele stands, and asking to break a
			   hidden slot is a rejection, not a no-op. */
			const bool everyNamedSlotRaised =
				!propBreakStage.PropBreakSlotIds.empty() &&
				/* Generic scripted playback suppresses incidental prop breaks. A
				   timeline row that explicitly prepared four intact pillars is the
				   one exception: its real red-blade stages own the two pairs. */
				(!boss.bScriptedPatternPlayback || allowScriptedPropBreak) &&
				propBreakStage.strPropBreakSetId ==
					m_EncounterPropRuntime.Get_PropSetId() &&
				propBreakStage.strActionId == boss.strActionId &&
				std::all_of(
					propBreakStage.PropBreakSlotIds.begin(),
					propBreakStage.PropBreakSlotIds.end(),
					[&propSlots](const std::string& slotId)
					{
						const auto found = std::find_if(
							propSlots.begin(), propSlots.end(),
							[&slotId](const ENCOUNTER_PROP_SLOT_STATE& candidate)
							{ return candidate.strSlotId == slotId; });
						return propSlots.end() != found &&
							LostArk::Shared::ENCOUNTER_PROP_STATE::INTACT ==
								found->eState;
					});
			if (everyNamedSlotRaised)
			{
				ENCOUNTER_PROP_TRANSACTION breakTransaction{};
				std::string breakStatus;
				const ENCOUNTER_PROP_PREPARE_RESULT breakResult =
					m_EncounterPropRuntime.Prepare_BreakSlots(
						propBreakStage.PropBreakSlotIds,
						m_EncounterPropRuntime.Get_OccurrenceSequence(),
						serverTick, breakTransaction, breakStatus);
				if (ENCOUNTER_PROP_PREPARE_RESULT::READY == breakResult)
				{
					if (!m_EncounterPropRuntime.Commit(breakTransaction, breakStatus))
					{
						m_strStatus = std::move(breakStatus);
						return false;
					}
					Broadcast_EncounterPropSync();
				}
				else if (ENCOUNTER_PROP_PREPARE_RESULT::NO_CHANGE != breakResult)
				{
					m_strStatus = std::move(breakStatus);
					return false;
				}
			}
		}
	}
	if (PILLAR_PATTERN_ID != boss.strPatternId ||
		PILLAR_SPAWN_STAGE_ID != boss.strPatternStageId)
	{
		return true;
	}

	ENCOUNTER_PROP_TRANSACTION transaction{};
	std::string status;
	const ENCOUNTER_PROP_PREPARE_RESULT result =
		m_EncounterPropRuntime.Prepare_Spawn(
			boss.iPatternSequence, serverTick, transaction, status);
	if (ENCOUNTER_PROP_PREPARE_RESULT::NO_CHANGE == result)
		return true;
	if (ENCOUNTER_PROP_PREPARE_RESULT::READY != result ||
		!m_EncounterPropRuntime.Commit(transaction, status))
	{
		/* A new occurrence cannot claim slots that the previous one still owns.
		   Preserve the prop failure so this tick cannot publish completion or
		   promote Next after an uncommitted stage entry. */
		m_strStatus = std::move(status);
		return false;
	}
	/* The Debug audition asked for a whole cycle, so the raise it just observed
	   schedules the shatter the product path has no owner for yet. */
	if (m_bPillarAuditionCycleArmed)
	{
		m_iPillarAuditionBreakTick =
			Add_ServerTicksSkippingReservedZero(
				serverTick, PILLAR_AUDITION_DWELL_TICKS);
		m_bPillarAuditionCycleArmed = false;
	}
	Broadcast_EncounterPropSync();
	return true;
}

bool LostArk::Server::CGameRoom::Apply_BossPatternStageActions(
	SERVER_WORLD_ENTITY& boss,
	const std::string& patternId,
	const std::string& actionId,
	const BOSS_PATTERN_STAGE_ACTION_TRIGGER trigger,
	const std::uint32_t serverTick,
	const std::uint32_t spawnWaveOrdinal,
	const bool scheduledSpawnWave)
{
	const CGameplayCatalog* occurrenceCatalog =
		Resolve_ValtanGameplayCatalog(boss);
	if (nullptr == occurrenceCatalog)
	{
		m_strStatus = "Boss stage action pinned gameplay generation is missing";
		return false;
	}
	SERVER_BOSS_COMBAT_STATE stagedCombat = boss.BossCombat;
	std::uint8_t stagedGameplayPhase = boss.iPhase;
	SERVER_COMBAT_OBJECT_TRANSACTION transaction =
		m_CombatObjectRuntime.Begin_Transaction();
	if (!Stage_BossPatternStageActions(
		boss, *occurrenceCatalog, patternId, actionId, trigger, serverTick,
		stagedCombat, stagedGameplayPhase, transaction, spawnWaveOrdinal,
		scheduledSpawnWave))
	{
		CValtanBrain::Fail_Mechanic(
			boss, patternId,
			SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_PREFLIGHT, serverTick);
		return false;
	}
	if (!m_CombatObjectRuntime.Commit(std::move(transaction)))
	{
		m_strStatus = "Boss stage combat object transaction changed";
		CValtanBrain::Fail_Mechanic(
			boss, patternId,
			SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_COMMIT, serverTick);
		return false;
	}
	boss.BossCombat = std::move(stagedCombat);
	boss.iPhase = stagedGameplayPhase;
	if (!scheduledSpawnWave && !Commit_BossPatternPlayerStageActions(
		boss, *occurrenceCatalog, patternId, actionId, trigger,
		serverTick, spawnWaveOrdinal))
	{
		m_strStatus = "Boss player stage action commit failed";
		return false;
	}
	return true;
}

bool LostArk::Server::CGameRoom::Resolve_ArenaRandomVolleyOrigins(
	const SERVER_WORLD_ENTITY& boss,
	const BOSS_PATTERN_STAGE_ACTION& action,
	const BOSS_COMBAT_OBJECT_DEFINITION& definition,
	const std::uint32_t spawnWaveOrdinal,
	std::vector<SERVER_COMBAT_OBJECT_LOCKED_TARGET>& outOrigins)
{
	outOrigins.clear();
	const BOSS_COMBAT_OBJECT_VOLLEY& volley = action.Volley;
	if (0u == volley.iArenaRandomCount)
		return true;
	if (!m_ServerNavigation.Is_Loaded() ||
		BOSS_COMBAT_OBJECT_ARENA_ANCHOR_POLICY::BOSS_SPAWN_POSITION !=
			volley.eArenaAnchorPolicy ||
		0u == boss.iNetEntityId || 0u == boss.iPatternSequence ||
		!std::isfinite(boss.fSpawnPositionX) ||
		!std::isfinite(boss.fSpawnPositionY) ||
		!std::isfinite(boss.fSpawnPositionZ) ||
		!std::isfinite(volley.fArenaRandomRadiusM) ||
		volley.fArenaRandomRadiusM <= 0.f ||
		!std::isfinite(volley.fArenaHeightToleranceM) ||
		volley.fArenaHeightToleranceM <= 0.f)
	{
		m_strStatus = "Boss arena-random volley contract is invalid";
		return false;
	}

	const float minimumSpacing = Resolve_VolleyMinimumSpacing(definition);
	if (!std::isfinite(minimumSpacing) || minimumSpacing <= 0.f)
	{
		m_strStatus = "Boss arena-random volley spacing is invalid";
		return false;
	}
	const float minimumSpacingSquared = minimumSpacing * minimumSpacing;
	/* Arena-random authoring defines a valid origin, not a guarantee that every
	   cell under the 3.5m damage circle is walkable. The Valtan nav paint has
	   intentional seams inside the arena, so admission pins the exact centre to
	   authoritative navigation/height and keeps whole circles apart by their
	   damage diameter. */
	const auto IsSpawnPointWalkable =
		[this, &boss, &volley](
			const float centerX, const float centerZ, float& outY)
		{
			SERVER_NAV_POINT center{};
			if (!m_ServerNavigation.Is_PointWalkableExact(centerX, centerZ) ||
				!m_ServerNavigation.Sample_Position(centerX, centerZ, center) ||
				std::abs(center.y - boss.fSpawnPositionY) >
					volley.fArenaHeightToleranceM)
			{
				return false;
			}
			outY = center.y;
			return true;
		};

	try
	{
		outOrigins.reserve(volley.iArenaRandomCount);
	}
	catch (const std::bad_alloc&)
	{
		m_strStatus = "Boss arena-random volley allocation failed";
		return false;
	}
	const std::uint64_t baseSeed = Mix_DeterministicRandom(
		Hash_StableId(action.strTargetId) ^
		(static_cast<std::uint64_t>(boss.iNetEntityId) << 32u) ^
		static_cast<std::uint64_t>(boss.iPatternSequence) ^
		(static_cast<std::uint64_t>(spawnWaveOrdinal) << 48u));
	for (std::uint32_t attempt = 0u;
		attempt < ARENA_RANDOM_MAXIMUM_ATTEMPTS &&
		outOrigins.size() < volley.iArenaRandomCount;
		++attempt)
	{
		const std::uint64_t attemptSeed = baseSeed ^
			(static_cast<std::uint64_t>(attempt + 1u) *
				0xd6e8feb86659fd93ull);
		const float radius = std::sqrt(
			DeterministicUnitFloat(attemptSeed)) *
			volley.fArenaRandomRadiusM;
		const float angle = DeterministicUnitFloat(
			attemptSeed ^ 0xa0761d6478bd642full) * TWO_PI;
		const float x = boss.fSpawnPositionX + std::cos(angle) * radius;
		const float z = boss.fSpawnPositionZ + std::sin(angle) * radius;
		float y = 0.f;
		if (!IsSpawnPointWalkable(x, z, y))
			continue;
		const bool overlaps = std::any_of(
			outOrigins.begin(), outOrigins.end(),
			[x, z, minimumSpacingSquared](
				const SERVER_COMBAT_OBJECT_LOCKED_TARGET& existing)
			{
				const float deltaX = x - existing.fPositionX;
				const float deltaZ = z - existing.fPositionZ;
				return deltaX * deltaX + deltaZ * deltaZ +
					VOLLEY_SPACING_EPSILON < minimumSpacingSquared;
			});
		if (overlaps)
			continue;
		SERVER_COMBAT_OBJECT_LOCKED_TARGET origin{};
		origin.fPositionX = x;
		origin.fPositionY = y;
		origin.fPositionZ = z;
		origin.bTrackUntilFirstPulse = false;
		outOrigins.push_back(origin);
	}
	if (outOrigins.size() != volley.iArenaRandomCount)
	{
		outOrigins.clear();
		m_strStatus = "Boss arena-random volley has no valid point set";
		return false;
	}
	return true;
}

bool LostArk::Server::CGameRoom::Apply_BossPatternScheduledSpawnWave(
	SERVER_WORLD_ENTITY& boss,
	const std::uint32_t serverTick)
{
	if (boss.strPatternId.empty() || boss.strActionId.empty() ||
		0u == boss.iActionStartTick)
	{
		return true;
	}
	const CGameplayCatalog* catalog = Resolve_ValtanGameplayCatalog(boss);
	if (nullptr == catalog)
	{
		m_strStatus = "Boss scheduled volley pinned gameplay generation is missing";
		return false;
	}
	const auto* patterns = catalog->Find_BossPatterns(boss.strEncounterId);
	if (nullptr == patterns)
	{
		m_strStatus = "Boss scheduled volley encounter is missing";
		return false;
	}
	const auto pattern = std::find_if(
		patterns->begin(), patterns->end(),
		[&boss](const BOSS_PATTERN_DEFINITION& candidate)
		{ return candidate.strPatternId == boss.strPatternId; });
	if (patterns->end() == pattern)
	{
		m_strStatus = "Boss scheduled volley pattern is missing";
		return false;
	}
	const auto stage = std::find_if(
		pattern->Stages.begin(), pattern->Stages.end(),
		[&boss](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
		{ return candidate.strActionId == boss.strActionId; });
	if (pattern->Stages.end() == stage)
	{
		m_strStatus = "Boss scheduled volley stage is missing";
		return false;
	}
	const BOSS_PATTERN_STAGE_ACTION* scheduledAction = nullptr;
	for (const BOSS_PATTERN_STAGE_ACTION& action : stage->Actions)
	{
		if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != action.eTrigger ||
			BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY !=
				action.eKind ||
			(0u == action.Volley.iFirstSpawnOffsetMs &&
			 action.Volley.iSpawnCount <= 1u))
		{
			continue;
		}
		if (nullptr != scheduledAction &&
			(scheduledAction->Volley.iFirstSpawnOffsetMs !=
				action.Volley.iFirstSpawnOffsetMs ||
			 scheduledAction->Volley.iSpawnCount != action.Volley.iSpawnCount ||
			 scheduledAction->Volley.iSpawnIntervalMs !=
				action.Volley.iSpawnIntervalMs))
		{
			m_strStatus = "Boss scheduled volleys do not share one clock";
			return false;
		}
		scheduledAction = &action;
	}
	if (nullptr == scheduledAction)
		return true;
	if (0u == boss.iAppliedPatternStageSpawnWaveCount &&
		0u == scheduledAction->Volley.iFirstSpawnOffsetMs)
	{
		m_strStatus = "Boss scheduled volley ENTER wave was not committed";
		return false;
	}
	if (boss.iAppliedPatternStageSpawnWaveCount >=
		scheduledAction->Volley.iSpawnCount)
	{
		return true;
	}
	const std::uint32_t waveOrdinal =
		boss.iAppliedPatternStageSpawnWaveCount;
	const std::uint64_t dueMilliseconds =
		static_cast<std::uint64_t>(scheduledAction->Volley.iFirstSpawnOffsetMs) +
		static_cast<std::uint64_t>(waveOrdinal) *
			scheduledAction->Volley.iSpawnIntervalMs;
	const std::uint64_t elapsedTicks =
		Elapsed_ServerTicksSkippingReservedZero(
			boss.iActionStartTick, serverTick);
	if (elapsedTicks * 1000ull <
		dueMilliseconds * static_cast<std::uint64_t>(SERVER_TICK_HZ))
	{
		return true;
	}
	if (!Apply_BossPatternStageActions(
		boss, boss.strPatternId, boss.strActionId,
		BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER,
		serverTick, waveOrdinal, true))
	{
		return false;
	}
	++boss.iAppliedPatternStageSpawnWaveCount;
	return true;
}

bool LostArk::Server::CGameRoom::Apply_BossPatternStageTransition(
	SERVER_WORLD_ENTITY& boss,
	const std::string& previousPatternId,
	const std::string& previousActionId,
	const std::string& nextPatternId,
	const std::string& nextActionId,
	const LostArk::Shared::GameplayDataRevision& previousDefinitionRevision,
	const LostArk::Shared::GameplayDataRevision& nextDefinitionRevision,
	const std::uint32_t serverTick)
{
	const std::string& failurePatternId =
		nextPatternId.empty() ? previousPatternId : nextPatternId;
	SERVER_BOSS_COMBAT_STATE stagedCombat = boss.BossCombat;
	std::uint8_t stagedGameplayPhase = boss.iPhase;
	SERVER_COMBAT_OBJECT_TRANSACTION transaction =
		m_CombatObjectRuntime.Begin_Transaction();
	const CGameplayCatalog* previousCatalog = previousPatternId.empty() ?
		&m_GameplayCatalog.Active() :
		m_GameplayCatalog.Resolve(previousDefinitionRevision);
	const CGameplayCatalog* nextCatalog = nextPatternId.empty() ?
		&m_GameplayCatalog.Active() :
		m_GameplayCatalog.Resolve(nextDefinitionRevision);
	if (nullptr == previousCatalog || nullptr == nextCatalog)
	{
		m_strStatus = "Boss stage transition pinned gameplay generation is missing";
		return false;
	}
	if (!Stage_BossPatternStageActions(
		boss, *previousCatalog, previousPatternId, previousActionId,
		BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT, serverTick,
		stagedCombat, stagedGameplayPhase, transaction) ||
		!Stage_BossPatternStageActions(
			boss, *nextCatalog, nextPatternId, nextActionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, serverTick,
			stagedCombat, stagedGameplayPhase, transaction))
	{
		CValtanBrain::Fail_Mechanic(
			boss, failurePatternId,
			SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_PREFLIGHT, serverTick);
		return false;
	}
	std::uint32_t nextStageAppliedSpawnWaveCount =
		nextActionId.empty() ? 0u : 1u;
	if (!nextActionId.empty())
	{
		const auto* nextPatterns = nextCatalog->Find_BossPatterns(
			boss.strEncounterId);
		if (nullptr == nextPatterns)
		{
			m_strStatus = "Boss next-stage scheduled volley encounter is missing";
			return false;
		}
		const auto nextPattern = std::find_if(
			nextPatterns->begin(), nextPatterns->end(),
			[&nextPatternId](const BOSS_PATTERN_DEFINITION& candidate)
			{ return candidate.strPatternId == nextPatternId; });
		if (nextPatterns->end() == nextPattern)
		{
			m_strStatus = "Boss next-stage scheduled volley pattern is missing";
			return false;
		}
		const auto nextStage = std::find_if(
			nextPattern->Stages.begin(), nextPattern->Stages.end(),
			[&nextActionId](const BOSS_PATTERN_STAGE_DEFINITION& candidate)
			{ return candidate.strActionId == nextActionId; });
		if (nextPattern->Stages.end() == nextStage)
		{
			m_strStatus = "Boss next-stage scheduled volley owner is missing";
			return false;
		}
		for (const BOSS_PATTERN_STAGE_ACTION& action : nextStage->Actions)
		{
			if (BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER != action.eTrigger ||
				BOSS_PATTERN_STAGE_ACTION_KIND::SPAWN_COMBAT_OBJECT_VOLLEY !=
					action.eKind ||
				(0u == action.Volley.iFirstSpawnOffsetMs &&
				 action.Volley.iSpawnCount <= 1u))
			{
				continue;
			}
			nextStageAppliedSpawnWaveCount =
				0u == action.Volley.iFirstSpawnOffsetMs ? 1u : 0u;
			break;
		}
	}
	WORLD_DESTRUCTION_TRANSACTION worldTransaction{};
	std::vector<LostArk::Shared::WORLD_DESTRUCTION_EVENT_WIRE> worldEvents;
	bool hasWorldTransaction = false;
	if (m_WorldDestructionRuntime.Is_Initialized() &&
		0u != boss.iNetEntityId && 0u != boss.iPatternSequence &&
		!boss.strPatternId.empty() && !boss.strPatternStageId.empty() &&
		!boss.strActionId.empty())
	{
		WORLD_DESTRUCTION_ACTION_TUPLE worldAction{};
		worldAction.strPatternId = boss.strPatternId;
		worldAction.strStageId = boss.strPatternStageId;
		worldAction.strActionId = boss.strActionId;
		worldAction.iStageIndex = boss.iPatternStageIndex;
		std::string worldStatus;
		const WORLD_DESTRUCTION_PREPARE_RESULT worldResult =
			m_WorldDestructionRuntime.Prepare_StageTrigger(
				worldAction, boss.iNetEntityId, boss.iPatternSequence,
				serverTick, worldTransaction, worldStatus);
		if (WORLD_DESTRUCTION_PREPARE_RESULT::READY == worldResult)
		{
			std::vector<SERVER_COLLISION_STATE_CHANGE> collisionChanges;
			std::vector<SERVER_NAVIGATION_CONDITION_CHANGE> navigationChanges;
			SERVER_COLLISION_STATE_STAGE collisionStage{};
			SERVER_NAVIGATION_CONDITION_STAGE navigationStage{};
			Build_WorldDestructionStateChanges(
				worldTransaction, collisionChanges, navigationChanges);
			if (!Build_WorldDestructionLiveEvents(
					worldTransaction, boss, worldEvents, worldStatus) ||
				!m_ServerCollisionSystem.Prepare_StateChanges(
					collisionChanges, collisionStage, worldStatus) ||
				!m_ServerNavigation.Prepare_ConditionChanges(
					navigationChanges, navigationStage, worldStatus))
			{
				m_strStatus = std::move(worldStatus);
				CValtanBrain::Fail_Mechanic(
					boss, failurePatternId,
					SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_PREFLIGHT,
					serverTick);
				return false;
			}
			hasWorldTransaction = true;
		}
		else if (WORLD_DESTRUCTION_PREPARE_RESULT::NO_MATCH != worldResult &&
			WORLD_DESTRUCTION_PREPARE_RESULT::DUPLICATE_REQUEST != worldResult &&
			WORLD_DESTRUCTION_PREPARE_RESULT::NO_CHANGE != worldResult)
		{
			m_strStatus = std::move(worldStatus);
			CValtanBrain::Fail_Mechanic(
				boss, failurePatternId,
				SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_PREFLIGHT,
				serverTick);
			return false;
		}
	}
	if (!m_CombatObjectRuntime.Commit(std::move(transaction)))
	{
		m_strStatus = "Boss stage transition combat object transaction changed";
		CValtanBrain::Fail_Mechanic(
			boss, failurePatternId,
			SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_COMMIT, serverTick);
		return false;
	}
	boss.BossCombat = std::move(stagedCombat);
	boss.iPhase = stagedGameplayPhase;
	/* A delayed first wave is intentionally absent from the ENTER transaction;
	   ordinal zero remains pending until its exact fixed-tick due time. */
	boss.iAppliedPatternStageSpawnWaveCount = nextStageAppliedSpawnWaveCount;
	if (hasWorldTransaction)
	{
		std::string worldStatus;
		if (!Commit_WorldDestructionTransaction(
			worldTransaction, worldEvents, serverTick, worldStatus))
		{
			m_strStatus = std::move(worldStatus);
			CValtanBrain::Fail_Mechanic(
				boss, failurePatternId,
				SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_COMMIT,
				serverTick);
			return false;
		}
		if (!worldEvents.empty())
		{
			const std::uint64_t lastSequence =
				worldEvents.back().iEventSequence;
			m_iNextWorldDestructionEventSequence =
				(std::numeric_limits<std::uint64_t>::max)() == lastSequence ?
					0u : lastSequence + 1u;
		}
	}
	if (!Commit_BossPatternPlayerStageActions(
			boss, *previousCatalog, previousPatternId, previousActionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER::EXIT, serverTick) ||
		!Commit_BossPatternPlayerStageActions(
			boss, *nextCatalog, nextPatternId, nextActionId,
			BOSS_PATTERN_STAGE_ACTION_TRIGGER::ENTER, serverTick))
	{
		m_strStatus = "Boss player stage action commit failed";
		CValtanBrain::Fail_Mechanic(
			boss, failurePatternId,
			SERVER_BOSS_MECHANIC_FAILURE::STAGE_TRANSITION_COMMIT, serverTick);
		return false;
	}
	return true;
}
