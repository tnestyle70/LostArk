#include "ServerTriggerSystem.h"

#include "Gameplay/WorldCollisionContract.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <utility>

namespace
{
	constexpr float DEGREES_TO_RADIANS = 0.0174532925f;
	constexpr float RADIANS_TO_DEGREES = 57.2957795f;

	/* Which trigger actions still fire the moment a player steps into the box.
	   EVERYTHING ELSE waits for the player to press G inside the box: the Server
	   offers the prompt on entry and the G request fires it (Activate_Interact /
	   Activate_Here), re-checking the volume. A row is one (world, action kind,
	   optional id prefix); WORLD_ID::END matches every world, and a null prefix
	   matches every placement id of that kind. These are scripted flows whose entry IS
	   the flow, so making them wait for G would stall the raid or the cutscene:
		PLAY_SEQUENCE, any world   cutscene / mechanic beats (Kouku Mario intro, paper, showtime)
		CLAIM_CARD_MAZE_TELESCOPE  card maze strike volume (never fired by entry or G anyway)
		VALTAN_ARENA spawn group   corridor waves (Stage_1, Stage_2, Stage_MiniBoss_Spawn)
		VALTAN_ARENA encounter     boss start (Stage_Boss)
		KAKULSAYDON_ARENA spawn group  start-area book waves (ids Book1_Monsters, Book2_Monsters)
	   Everything the PLAYER does to move -- Mario crossings (jump down, climb, cross a gap),
	   the Mario terminal exits, jump.*, the Valtan start box and the other Valtan movePlayer
	   boxes -- is deliberately NOT here: it waits for G. A Mario lane box still goes through
	   the room's Mario admission (SERVER_TRIGGER_MOVE_ENTRY_HANDLER) when G runs it.
	   Delete a row to make that kind G-only. Add a row to make one fire on entry.
	   An authored requiresInteract flag always wins over this table. */
	struct AUTO_ENTRY_RULE final
	{
		LostArk::Shared::WORLD_ID eWorld;
		LostArk::Server::WORLD_TRIGGER_ACTION_KIND eKind;
		const char* pIdPrefix;
	};
	constexpr AUTO_ENTRY_RULE AUTO_ENTRY_RULES[] =
	{
		{ LostArk::Shared::WORLD_ID::END, LostArk::Server::WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE, nullptr },
		{ LostArk::Shared::WORLD_ID::END, LostArk::Server::WORLD_TRIGGER_ACTION_KIND::CLAIM_CARD_MAZE_TELESCOPE, nullptr },
		{ LostArk::Shared::WORLD_ID::VALTAN_ARENA, LostArk::Server::WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP, nullptr },
		{ LostArk::Shared::WORLD_ID::VALTAN_ARENA, LostArk::Server::WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER, nullptr },
		{ LostArk::Shared::WORLD_ID::KAKULSAYDON_ARENA, LostArk::Server::WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP, "Book" },
	};

	/* Valtan boss start. The Stage_Boss box starts the boss encounter and also sends
	   the player who fired it to the Stage_Boss_ArenaEntry box, at that box's centre.
	   Every player who fires it is sent, including ones who arrive after the boss is
	   already up, so a raid does not have to walk in one by one. */
	constexpr const char* VALTAN_BOSS_START_TRIGGER_ID = "Stage_Boss";
	constexpr const char* VALTAN_ARENA_ENTRY_TRIGGER_ID = "Stage_Boss_ArenaEntry";
}

bool LostArk::Server::CServerTriggerSystem::Initialize(
	const std::vector<WORLD_BOOTSTRAP_PLACEMENT>& placements,
	std::string& outStatus,
	const bool enableDebugValtanStageBypass)
{
	std::vector<RUNTIME_TRIGGER> staged;
	for (const WORLD_BOOTSTRAP_PLACEMENT& placement : placements)
	{
		if (WORLD_BOOTSTRAP_KIND::TRIGGER_BOX != placement.eKind ||
			!placement.isEnabled)
		{
			continue;
		}
		if (1u != placement.TriggerActions.size() ||
			(WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER !=
				placement.TriggerActions.front().eKind &&
			WORLD_TRIGGER_ACTION_KIND::CHANGE_LEVEL !=
				placement.TriggerActions.front().eKind &&
			WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP !=
				placement.TriggerActions.front().eKind &&
			WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER !=
				placement.TriggerActions.front().eKind &&
			WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE !=
				placement.TriggerActions.front().eKind &&
			WORLD_TRIGGER_ACTION_KIND::CLAIM_CARD_MAZE_TELESCOPE !=
				placement.TriggerActions.front().eKind))
		{
			outStatus = "Enabled trigger requires one supported action: " +
				placement.strPlacementId;
			return false;
		}
		RUNTIME_TRIGGER runtime{ placement };
		/* Product decision (2026-09-19): every authored trigger fires again for
		   every player, any number of times. The authored triggerOnce flag stays
		   in the data and in MapTool, but it used to become a room-wide latch: the
		   first player who fired the box spent it for everyone after them, and it
		   only cleared when the room emptied (Character Select, Valtan, Kouku; Bern
		   and the training ground never reset). Only a test opts back in. */
		if (!m_bHonourTriggerOnce)
			runtime.Definition.isTriggerOnce = false;
		staged.push_back(std::move(runtime));
	}
	m_Triggers = std::move(staged);
#ifdef _DEBUG
	m_bDebugValtanStageBypass = enableDebugValtanStageBypass;
#else
	(void)enableDebugValtanStageBypass;
	m_bDebugValtanStageBypass = false;
#endif
	outStatus = "Initialized server triggers: " +
		std::to_string(m_Triggers.size()) +
		(m_bDebugValtanStageBypass ? ", ValtanStageBypass=1" : "");
	return true;
}

bool LostArk::Server::CServerTriggerSystem::Update_PlayerMotion(
	SERVER_PLAYER& player,
	const float fixedDeltaSeconds) const
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::TRIGGER_MOVE != player.eAction)
	{
		return false;
	}
	if (!player.TriggerMove.isActive ||
		player.TriggerMove.fDurationSeconds <= 0.f)
	{
		player.TriggerMove = {};
		player.eAction = PLAYER_ACTION_STATE::NONE;
		player.iActionStartTick = 0;
		player.PendingCommand.Clear();
		return false;
	}
	if (0u == player.iCurrentHp)
	{
		player.TriggerMove = {};
		return false;
	}

	SERVER_TRIGGER_MOVE& move = player.TriggerMove;
	move.fElapsedSeconds = (std::min)(
		move.fDurationSeconds,
		move.fElapsedSeconds + fixedDeltaSeconds);
	const float ratio = move.fElapsedSeconds / move.fDurationSeconds;
	player.fPositionX = move.fStartX +
		(move.fTargetX - move.fStartX) * ratio;
	player.fPositionY = move.fStartY +
		(move.fTargetY - move.fStartY) * ratio +
		4.f * move.fArcHeight * ratio * (1.f - ratio);
	player.fPositionZ = move.fStartZ +
		(move.fTargetZ - move.fStartZ) * ratio;

	if (ratio >= 1.f)
	{
		player.fPositionX = move.fTargetX;
		player.fPositionY = move.fTargetY;
		player.fPositionZ = move.fTargetZ;
		if (KOUKU_HUD_MODE::END != move.eKoukuHudModeOnArrival)
		{
			player.Clear_KoukuInteractionState();
			player.eKoukuAreaHudMode = move.eKoukuHudModeOnArrival;
			player.eMadnessForm = KOUKU_HUD_MODE::NONE == move.eKoukuHudModeOnArrival ?
				PLAYER_MADNESS_FORM::NORMAL : PLAYER_MADNESS_FORM::CLOWN;
		}
		move = {};
		player.eAction = PLAYER_ACTION_STATE::NONE;
		player.iActionStartTick = 0;
		player.PendingCommand.Clear();
	}
	return true;
}

bool LostArk::Server::CServerTriggerSystem::Run_Action(
	RUNTIME_TRIGGER& trigger,
	SERVER_PLAYER& player,
	const std::uint32_t actionStartTick,
	std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
	const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
		const std::string&)>& activateTarget) const
{
	const WORLD_TRIGGER_ACTION& action =
		trigger.Definition.TriggerActions.front();
	bool fired = false;
	if (WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER == action.eKind)
	{
		fired = Begin_MovePlayer(player, action, actionStartTick);
		if (fired)
			player.TriggerMove.strSourcePlacementId = trigger.Definition.strPlacementId;
	}
	else if (WORLD_TRIGGER_ACTION_KIND::CHANGE_LEVEL == action.eKind)
	{
		SERVER_WORLD_TRANSFER_REQUEST transfer{};
		fired = Build_WorldTransfer(player, action, transfer);
		if (fired)
			outTransfers.push_back(std::move(transfer));
	}
	else if (WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER == action.eKind &&
		LostArk::Shared::WORLD_ID::VALTAN_ARENA == m_eWorldId &&
		VALTAN_BOSS_START_TRIGGER_ID == trigger.Definition.strPlacementId &&
		activateTarget)
	{
		/* Start the boss, then send this player to the arena entrance. The start is
		   refused once the boss is up, which is fine: a later player is still sent. The
		   trigger counts as fired when the player was sent. A player who is busy (skill,
		   hit reaction) is not sent; the boss start is unaffected and stepping out and
		   back in sends them. Without an entrance box only the start runs. */
		fired = activateTarget(action.eKind, action.strTargetId);
		if (Place_PlayerAtValtanArenaEntry(player, actionStartTick))
		{
			player.TriggerMove.strSourcePlacementId = trigger.Definition.strPlacementId;
			fired = true;
		}
	}
	else if ((WORLD_TRIGGER_ACTION_KIND::ACTIVATE_SPAWN_GROUP == action.eKind ||
		WORLD_TRIGGER_ACTION_KIND::ACTIVATE_ENCOUNTER == action.eKind ||
		WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE == action.eKind ||
		WORLD_TRIGGER_ACTION_KIND::CLAIM_CARD_MAZE_TELESCOPE == action.eKind) &&
		activateTarget)
	{
		fired = activateTarget(action.eKind, action.strTargetId);
	}
	if (fired && trigger.Definition.isTriggerOnce)
		trigger.hasFired = true;
	return fired;
}

bool LostArk::Server::CServerTriggerSystem::Place_PlayerAtValtanArenaEntry(
	SERVER_PLAYER& player,
	const std::uint32_t actionStartTick) const
{
	const auto entry = std::find_if(m_Triggers.begin(), m_Triggers.end(),
		[](const RUNTIME_TRIGGER& trigger)
		{
			return VALTAN_ARENA_ENTRY_TRIGGER_ID == trigger.Definition.strPlacementId;
		});
	if (m_Triggers.end() == entry)
		return false;
	const WORLD_BOOTSTRAP_PLACEMENT& box = entry->Definition;

	WORLD_TRIGGER_ACTION move{};
	move.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
	move.fTargetX = box.fPositionX;
	move.fTargetY = box.fPositionY;
	move.fTargetZ = box.fPositionZ;
	/* Same hop the entrance box itself authors, so both moves feel alike. */
	move.fDurationSeconds = 0.8f;
	move.fArcHeight = 0.f;
	if (!box.TriggerActions.empty() &&
		WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER == box.TriggerActions.front().eKind &&
		box.TriggerActions.front().fDurationSeconds > 0.f)
	{
		move.fDurationSeconds = box.TriggerActions.front().fDurationSeconds;
		move.fArcHeight = box.TriggerActions.front().fArcHeight;
	}
	/* The box height is authored by hand and can sit off the floor; land on the
	   floor the room's navigation reports under the box centre when it can. */
	float groundY = 0.f;
	if (m_GroundSampler &&
		m_GroundSampler(box.fPositionX, box.fPositionZ, groundY))
	{
		move.fTargetY = groundY;
	}
	return Begin_MovePlayer(player, move, actionStartTick);
}

void LostArk::Server::CServerTriggerSystem::Reset_SequenceActivation(const std::string& instanceId)
{
	for (auto& trigger : m_Triggers)
		if (std::any_of(trigger.Definition.TriggerActions.begin(), trigger.Definition.TriggerActions.end(),
			[&](const auto& action) { return action.eKind == WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE && action.strTargetId == instanceId; }))
			trigger.hasFired = false;
}

bool LostArk::Server::CServerTriggerSystem::Activate_Interact(
	const LostArk::Shared::PLAYER_ID playerId,
	const std::string& triggerPlacementId,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const std::uint32_t actionStartTick,
	std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
	const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
		const std::string&)>& activateTarget,
	const SERVER_TRIGGER_MOVE_ENTRY_HANDLER& moveEntry)
{
	const auto found = players.find(playerId);
	if (players.end() == found || 0u == found->second.iCurrentHp ||
		Is_KeyDebounced(playerId, actionStartTick))
	{
		return false;
	}
	for (RUNTIME_TRIGGER& trigger : m_Triggers)
	{
		if (trigger.Definition.strPlacementId != triggerPlacementId ||
			trigger.Definition.TriggerActions.empty() ||
			Fires_OnEntry(trigger) ||
			WORLD_TRIGGER_ACTION_KIND::CLAIM_CARD_MAZE_TELESCOPE ==
				trigger.Definition.TriggerActions.front().eKind ||
			(trigger.Definition.isTriggerOnce && trigger.hasFired))
		{
			continue;
		}
		/* The offer was sent when they entered; they may have walked out since,
		   so membership is tested again rather than trusted. */
		if (!Contains(trigger, found->second))
			return false;
		const bool fired = Run_KeyTrigger(trigger, found->second, actionStartTick,
			outTransfers, activateTarget, moveEntry);
		if (fired)
		{
			Log_Fire(trigger, playerId, "KEY");
			m_LastKeyActivationTick[playerId] = actionStartTick;
		}
		return fired;
	}
	return false;
}

std::uint32_t LostArk::Server::CServerTriggerSystem::Activate_Here(
	const LostArk::Shared::PLAYER_ID playerId,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const std::uint32_t actionStartTick,
	std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
	const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
		const std::string&)>& activateTarget,
	const SERVER_TRIGGER_MOVE_ENTRY_HANDLER& moveEntry)
{
	const auto found = players.find(playerId);
	if (players.end() == found || 0u == found->second.iCurrentHp ||
		Is_KeyDebounced(playerId, actionStartTick))
	{
		return 0u;
	}
	std::uint32_t used = 0u;
	for (RUNTIME_TRIGGER& trigger : m_Triggers)
	{
		/* The telescope box is a strike volume the room measures the hammer
		   against; G inside it runs nothing, exactly like walking in. A box that
		   fires on entry is not G's to run either. */
		if (trigger.Definition.TriggerActions.empty() ||
			WORLD_TRIGGER_ACTION_KIND::CLAIM_CARD_MAZE_TELESCOPE ==
				trigger.Definition.TriggerActions.front().eKind ||
			Fires_OnEntry(trigger) ||
			(trigger.Definition.isTriggerOnce && trigger.hasFired) ||
			!Contains(trigger, found->second))
		{
			continue;
		}
		if (Run_KeyTrigger(trigger, found->second, actionStartTick,
			outTransfers, activateTarget, moveEntry))
		{
			Log_Fire(trigger, playerId, "KEY");
			++used;
		}
	}
	if (0u != used)
		m_LastKeyActivationTick[playerId] = actionStartTick;
	return used;
}

void LostArk::Server::CServerTriggerSystem::Log_Fire(
	const RUNTIME_TRIGGER& trigger,
	const LostArk::Shared::PLAYER_ID playerId,
	const char* const pSource) const
{
	if (!m_FireLog)
		return;
	m_FireLog("[Trigger] Fire Trigger=" + trigger.Definition.strPlacementId +
		" Player=" + std::to_string(playerId) + " Source=" + pSource);
}

bool LostArk::Server::CServerTriggerSystem::Fires_OnEntry(
	const RUNTIME_TRIGGER& trigger) const
{
	if (trigger.Definition.requiresInteract ||
		trigger.Definition.TriggerActions.empty())
	{
		return false;
	}
#ifdef _DEBUG
	/* The Debug Valtan corridor shortcut hops the player toward the next stage, so
	   it waits for G like any other movement trigger. */
	WORLD_TRIGGER_ACTION shortcut{};
	if (m_bDebugValtanStageBypass &&
		Build_ValtanStageBypassMove(trigger.Definition.strPlacementId, shortcut))
	{
		return false;
	}
#endif
	const WORLD_TRIGGER_ACTION_KIND kind =
		trigger.Definition.TriggerActions.front().eKind;
	const std::string& placementId = trigger.Definition.strPlacementId;
	return std::any_of(std::begin(AUTO_ENTRY_RULES), std::end(AUTO_ENTRY_RULES),
		[this, kind, &placementId](const AUTO_ENTRY_RULE& rule)
		{
			return rule.eKind == kind &&
				(LostArk::Shared::WORLD_ID::END == rule.eWorld ||
					m_eWorldId == rule.eWorld) &&
				(nullptr == rule.pIdPrefix ||
					placementId.starts_with(rule.pIdPrefix));
		});
}

bool LostArk::Server::CServerTriggerSystem::Run_Trigger(
	RUNTIME_TRIGGER& trigger,
	SERVER_PLAYER& player,
	const std::uint32_t actionStartTick,
	std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
	const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
		const std::string&)>& activateTarget) const
{
#ifdef _DEBUG
	/* Debug Valtan shortcut: the trigger's own action is replaced by a short hop
	   toward the next stage. Stage_Boss is exempt: it keeps its real activateEncounter
	   action, and Run_Action then sends the player to the arena entrance box exactly as
	   Release does. Both run only on G. */
	WORLD_TRIGGER_ACTION bypassMove{};
	if (m_bDebugValtanStageBypass &&
		"Stage_Boss" != trigger.Definition.strPlacementId &&
		Build_ValtanStageBypassMove(
			trigger.Definition.strPlacementId, bypassMove))
	{
		const bool moved = Begin_MovePlayer(player, bypassMove, actionStartTick);
		if (moved && trigger.Definition.isTriggerOnce)
			trigger.hasFired = true;
		return moved;
	}
#endif
	/* Stage_Boss's entrance placement lives in Run_Action, so Debug and Release move
	   the player the same way. Place_PlayerAtValtanAuditionBait is now only used by
	   the Debug pattern audition. */
	return Run_Action(
		trigger, player, actionStartTick, outTransfers, activateTarget);
}

bool LostArk::Server::CServerTriggerSystem::Run_KeyTrigger(
	RUNTIME_TRIGGER& trigger,
	SERVER_PLAYER& player,
	const std::uint32_t actionStartTick,
	std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
	const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
		const std::string&)>& activateTarget,
	const SERVER_TRIGGER_MOVE_ENTRY_HANDLER& moveEntry) const
{
	/* A movement box the room owns (a Kouku Mario lane) is admitted by the same room
	   handler whether the player stepped in or pressed G: the stage match, authority
	   locks, contact-action interruption and the terminal exit's return destination
	   all live there, so a G press can neither bypass them nor land the player on the
	   raw authored point. Anything else runs the authored action unchanged. */
	if (moveEntry && !trigger.Definition.TriggerActions.empty() &&
		WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER ==
			trigger.Definition.TriggerActions.front().eKind)
	{
		const SERVER_TRIGGER_MOVE_ENTRY_RESULT owned =
			moveEntry(trigger.Definition, player, actionStartTick);
		if (SERVER_TRIGGER_MOVE_ENTRY_RESULT::RETRY_WHILE_INSIDE == owned)
			return false;
		if (SERVER_TRIGGER_MOVE_ENTRY_RESULT::STARTED == owned)
		{
			player.TriggerMove.strSourcePlacementId = trigger.Definition.strPlacementId;
			if (trigger.Definition.isTriggerOnce)
				trigger.hasFired = true;
			return true;
		}
	}
	return Run_Trigger(trigger, player, actionStartTick, outTransfers, activateTarget);
}

bool LostArk::Server::CServerTriggerSystem::Is_KeyDebounced(
	const LostArk::Shared::PLAYER_ID playerId,
	const std::uint32_t tick) const
{
	const auto last = m_LastKeyActivationTick.find(playerId);
	return m_LastKeyActivationTick.end() != last &&
		tick - last->second < KEY_ACTIVATION_DEBOUNCE_TICKS;
}

LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT LostArk::Server::CServerTriggerSystem::Debug_Activate(
	const LostArk::Shared::PLAYER_ID playerId, const std::string& triggerId, const bool replay,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players, const std::uint32_t tick,
	std::vector<SERVER_WORLD_TRANSFER_REQUEST>& transfers,
	const std::function<bool(WORLD_TRIGGER_ACTION_KIND, const std::string&)>& activateTarget)
{
	using Result = LostArk::Shared::DEBUG_WORLD_PLAYBACK_RESULT;
#ifndef _DEBUG
	(void)playerId; (void)triggerId; (void)replay; (void)players; (void)tick; (void)transfers; (void)activateTarget;
	return Result::DISABLED;
#else
	const auto player = players.find(playerId);
	if (player == players.end() || !player->second.iCurrentHp || player->second.TriggerMove.isActive)
		return Result::INVALID_PLAYER;
	const auto trigger = std::find_if(m_Triggers.begin(), m_Triggers.end(),
		[&](const RUNTIME_TRIGGER& value) { return value.Definition.strPlacementId == triggerId; });
	if (trigger == m_Triggers.end()) return Result::INVALID_TARGET;
	if (!replay && trigger->Definition.isTriggerOnce && trigger->hasFired) return Result::ALREADY_USED;
	// Debug bypasses only entry/G-key/one-shot admission. The authored action is shared.
	return Run_Action(*trigger, player->second, tick, transfers, activateTarget) ?
		Result::ACCEPTED : Result::ACTION_REJECTED;
#endif
}

void LostArk::Server::CServerTriggerSystem::Evaluate_Entries(
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const std::uint32_t actionStartTick,
	std::vector<SERVER_WORLD_TRANSFER_REQUEST>& outTransfers,
	const std::function<bool(WORLD_TRIGGER_ACTION_KIND,
		const std::string&)>& activateTarget,
	std::vector<SERVER_INTERACT_PROMPT_EDGE>& outPromptEdges,
	const SERVER_TRIGGER_MOVE_ENTRY_HANDLER& moveEntry)
{
	outTransfers.clear();
	outPromptEdges.clear();
	for (RUNTIME_TRIGGER& trigger : m_Triggers)
	{
		/* The telescope box is a strike volume the room measures the hammer
		   against; walking into it runs nothing and offers nothing. */
		if (!trigger.Definition.TriggerActions.empty() &&
			WORLD_TRIGGER_ACTION_KIND::CLAIM_CARD_MAZE_TELESCOPE ==
				trigger.Definition.TriggerActions.front().eKind)
		{
			continue;
		}
		/* A box that does not fire on entry only offers itself: the player presses
		   G inside it and the Server checks the volume again. */
		const bool firesOnEntry = Fires_OnEntry(trigger);
		std::unordered_set<LostArk::Shared::PLAYER_ID> currentInside;
		for (auto& [playerId, player] : players)
		{
			if (0u == player.iCurrentHp || !Contains(trigger, player))
				continue;
			currentInside.insert(playerId);
			const bool wasInside = trigger.PlayersInside.contains(playerId);
			if (!firesOnEntry)
			{
				/* It offers, once, on the edge. A spent one-shot (a test that kept
				   the latch, see Set_HonourTriggerOnce) has nothing left to offer. */
				if (!wasInside && !(trigger.Definition.isTriggerOnce &&
					trigger.hasFired))
				{
					outPromptEdges.push_back({ playerId,
						trigger.Definition.strPlacementId, true });
				}
				continue;
			}
			if (wasInside ||
				(trigger.Definition.isTriggerOnce && trigger.hasFired))
			{
				continue;
			}
			bool fired = false;
			const auto ownedEntry = moveEntry &&
				WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER == trigger.Definition.TriggerActions.front().eKind ?
				moveEntry(trigger.Definition, player, actionStartTick) :
				SERVER_TRIGGER_MOVE_ENTRY_RESULT::USE_DEFAULT;
			if (SERVER_TRIGGER_MOVE_ENTRY_RESULT::RETRY_WHILE_INSIDE == ownedEntry)
			{
				/* Contact was detected but not admitted. Do not turn a
				   temporary authority lock into a consumed entry edge. */
				currentInside.erase(playerId);
				continue;
			}
			if (SERVER_TRIGGER_MOVE_ENTRY_RESULT::STARTED == ownedEntry)
			{
				fired = true;
				player.TriggerMove.strSourcePlacementId = trigger.Definition.strPlacementId;
			}
			else
			{
				fired = Run_Action(trigger, player, actionStartTick,
					outTransfers, activateTarget);
			}
			if (fired)
			{
				Log_Fire(trigger, playerId, "ENTER");
				/* An owned entry fires without going through Run_Action, so the
				   one-shot latch is still set here for that path. It is a no-op
				   unless Set_HonourTriggerOnce kept isTriggerOnce. */
				if (trigger.Definition.isTriggerOnce)
				{
					trigger.hasFired = true;
				}
			}
			else if (LostArk::Shared::PLAYER_ACTION_STATE::NONE != player.eAction &&
				LostArk::Shared::PLAYER_ACTION_STATE::TRIGGER_MOVE != player.eAction &&
				(WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER ==
					trigger.Definition.TriggerActions.front().eKind ||
				WORLD_TRIGGER_ACTION_KIND::CHANGE_LEVEL ==
					trigger.Definition.TriggerActions.front().eKind))
			{
				/* Only a box that fires on entry gets here. Contact was made while
				   the player was busy (skill, hit reaction), so the entry edge is
				   not spent: the next tick tries again until the action ends or they
				   leave. A trigger move in flight is excluded so a box crossed
				   mid-arc never chains into another teleport. */
				currentInside.erase(playerId);
			}
		}
		/* Leaving withdraws the offer, so a Client never keeps a prompt for a
		   box it has walked out of. */
		if (!firesOnEntry)
		{
			for (const LostArk::Shared::PLAYER_ID previous : trigger.PlayersInside)
			{
				if (!currentInside.contains(previous))
				{
					outPromptEdges.push_back({ previous,
						trigger.Definition.strPlacementId, false });
				}
			}
		}
		trigger.PlayersInside = std::move(currentInside);
	}
}

#ifdef _DEBUG
bool LostArk::Server::CServerTriggerSystem::Place_PlayerAtValtanAuditionBait(
	SERVER_PLAYER& player,
	const std::uint32_t actionStartTick) const
{
	WORLD_TRIGGER_ACTION move{};
	if (!Build_ValtanStageBypassMove("Stage_Boss", move) ||
		!Begin_MovePlayer(player, move, actionStartTick))
	{
		return false;
	}
	return Update_PlayerMotion(player, move.fDurationSeconds) &&
		LostArk::Shared::PLAYER_ACTION_STATE::NONE == player.eAction;
}

bool LostArk::Server::CServerTriggerSystem::Build_ValtanStageBypassMove(
	const std::string& triggerPlacementId,
	WORLD_TRIGGER_ACTION& outAction)
{
	/* These destinations stop just before the next authored trigger. The player
	still walks into every next stage deliberately, while the long blocked route
	and its unkillable audition monsters no longer prevent reaching Valtan. */
	struct BYPASS_DESTINATION final
	{
		const char* pTriggerPlacementId;
		float x;
		float y;
		float z;
		float duration;
		float arcHeight;
	};
	/* Stage_1, Stage_2 and Stage_MiniBoss are deliberately absent. Stage_1 and
	Stage_2 build their spawn-group waves and Stage_MiniBoss now authors the
	Lugaru entrance move that places the party under the entrance camera shot,
	so all three have to run their real actions. Stage_2 used to hop 7 m toward
	Stage_3 instead of spawning spawn.valtan.stage03. Stage_3 is absent as well:
	it authors the cliff move in Gameplay.world.json, so that authored action is
	the only truth. The remaining rows keep the shortcut, which is still how boss
	work reaches Valtan without clearing the whole corridor. */
	static constexpr BYPASS_DESTINATION DESTINATIONS[] =
	{
		{ "Stage_Boss", 154.296f, 22.970f, -125.219f, 0.01f, 0.f }
	};
	const auto found = std::find_if(
		std::begin(DESTINATIONS), std::end(DESTINATIONS),
		[&triggerPlacementId](const BYPASS_DESTINATION& destination)
		{
			return triggerPlacementId == destination.pTriggerPlacementId;
		});
	if (std::end(DESTINATIONS) == found)
		return false;

	WORLD_TRIGGER_ACTION staged{};
	staged.eKind = WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER;
	staged.fTargetX = found->x;
	staged.fTargetY = found->y;
	staged.fTargetZ = found->z;
	staged.fDurationSeconds = found->duration;
	staged.fArcHeight = found->arcHeight;
	outAction = staged;
	return true;
}
#endif

void LostArk::Server::CServerTriggerSystem::Remove_Player(
	const LostArk::Shared::PLAYER_ID playerId)
{
	for (RUNTIME_TRIGGER& trigger : m_Triggers)
	{
		trigger.PlayersInside.erase(playerId);
	}
	m_LastKeyActivationTick.erase(playerId);
}

bool LostArk::Server::CServerTriggerSystem::Contains(
	const RUNTIME_TRIGGER& trigger,
	const SERVER_PLAYER& player)
{
	return Contains_Placement(trigger.Definition, player);
}

bool LostArk::Server::CServerTriggerSystem::Contains_Placement(
	const WORLD_BOOTSTRAP_PLACEMENT& box,
	const SERVER_PLAYER& player)
{
	const float deltaX = player.fPositionX - box.fPositionX;
	const float deltaZ = player.fPositionZ - box.fPositionZ;
	const float yaw = box.fYawDegrees * DEGREES_TO_RADIANS;
	const float cosine = std::cos(yaw);
	const float sine = std::sin(yaw);
	const float localX = cosine * deltaX - sine * deltaZ;
	const float localZ = sine * deltaX + cosine * deltaZ;
	const float playerCenterY = player.fPositionY +
		LostArk::Shared::WorldCollision::PLAYER_CENTER_OFFSET_Y;
	return std::abs(localX) <= box.fHalfExtentX +
			LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_X &&
		std::abs(playerCenterY - box.fPositionY) <= box.fHalfExtentY +
			LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_Y &&
		std::abs(localZ) <= box.fHalfExtentZ +
			LostArk::Shared::WorldCollision::PLAYER_HALF_EXTENT_Z;
}

bool LostArk::Server::CServerTriggerSystem::Begin_MovePlayer(
	SERVER_PLAYER& player,
	const WORLD_TRIGGER_ACTION& action,
	const std::uint32_t actionStartTick)
{
	using namespace LostArk::Shared;
	if (PLAYER_ACTION_STATE::NONE != player.eAction ||
		0u == player.iCurrentHp || 0u == actionStartTick ||
		WORLD_TRIGGER_ACTION_KIND::MOVE_PLAYER != action.eKind)
	{
		return false;
	}

	player.hasMoveGoal = false;
	player.MovePath.clear();
	player.iMovePathIndex = 0;
	player.iCurrentSkillId = INVALID_SKILL_ID;
	player.Clear_SkillTarget();
	player.fActionElapsedSeconds = 0.f;
	player.iComboStage = 0;
	player.hasBufferedComboInput = false;
	player.PendingCommand.Clear();
	player.TriggerMove = {};
	player.TriggerMove.fStartX = player.fPositionX;
	player.TriggerMove.fStartY = player.fPositionY;
	player.TriggerMove.fStartZ = player.fPositionZ;
	player.TriggerMove.fTargetX = action.fTargetX;
	player.TriggerMove.fTargetY = action.fTargetY;
	player.TriggerMove.fTargetZ = action.fTargetZ;
	player.TriggerMove.fDurationSeconds = action.fDurationSeconds;
	player.TriggerMove.fElapsedSeconds = 0.f;
	player.TriggerMove.fArcHeight = action.fArcHeight;
	player.TriggerMove.eKoukuHudModeOnArrival = action.eKoukuHudModeOnArrival;
	player.TriggerMove.isActive = true;
	const float deltaX = action.fTargetX - player.fPositionX;
	const float deltaZ = action.fTargetZ - player.fPositionZ;
	if (deltaX * deltaX + deltaZ * deltaZ > 0.000001f)
		player.fYawDegrees = std::atan2(deltaX, deltaZ) * RADIANS_TO_DEGREES;
	player.eAction = PLAYER_ACTION_STATE::TRIGGER_MOVE;
	player.iActionStartTick = actionStartTick;
	return true;
}

bool LostArk::Server::CServerTriggerSystem::Build_WorldTransfer(
	const SERVER_PLAYER& player,
	const WORLD_TRIGGER_ACTION& action,
	SERVER_WORLD_TRANSFER_REQUEST& outTransfer)
{
	using namespace LostArk::Shared;
	if (WORLD_TRIGGER_ACTION_KIND::CHANGE_LEVEL != action.eKind ||
		(WORLD_ID::BERN != action.eTargetWorldId &&
			WORLD_ID::VALTAN_ARENA != action.eTargetWorldId) ||
		INVALID_SESSION_ID == player.iSessionId ||
		CHARACTER_CLASS_ID::END == player.eCharacterClass ||
		player.strNickName.empty() || 0u == player.iCurrentHp ||
		PLAYER_ACTION_STATE::NONE != player.eAction)
	{
		return false;
	}

	outTransfer.iSessionId = player.iSessionId;
	outTransfer.eTargetWorldId = action.eTargetWorldId;
	outTransfer.eCharacterClass = player.eCharacterClass;
	outTransfer.strNickName = player.strNickName;
	return true;
}
