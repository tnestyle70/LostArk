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

bool LostArk::Server::CGameRoom::Grant_Item(
	SERVER_PLAYER& player,
	const std::string& itemId,
	const std::uint32_t quantity)
{
	using namespace LostArk::Shared;
	const SERVER_ITEM_DEFINITION* itemDefinition =
		m_ItemCatalog.Find_Item(itemId);
	// An unknown item ID is rejected rather than silently dropped or
	// substituted; the inventory is only ever a replace-in-full send, so a
	// no-op reply carries no result to give a caller.
	if (nullptr == itemDefinition)
		return false;

	/* Stacks only with the bag entry; an equipped copy stays where it is. */
	const auto existing = std::find_if(
		player.Inventory.begin(), player.Inventory.end(),
		[&itemId](const INVENTORY_ITEM_SNAPSHOT& item)
		{
			return item.strItemId == itemId &&
				EQUIPMENT_SLOT::NONE == item.eEquippedSlot;
		});
	if (existing == player.Inventory.end())
	{
		if (player.Inventory.size() >= MAX_INVENTORY_ITEMS)
			return false;
		INVENTORY_ITEM_SNAPSHOT item{};
		item.strItemId = itemId;
		item.iQuantity = (std::min)(quantity, itemDefinition->iMaxStack);
		player.Inventory.push_back(std::move(item));
	}
	else
	{
		const std::uint64_t stacked =
			static_cast<std::uint64_t>(existing->iQuantity) +
			static_cast<std::uint64_t>(quantity);
		existing->iQuantity = static_cast<std::uint32_t>(
			(std::min)(stacked, static_cast<std::uint64_t>(
				itemDefinition->iMaxStack)));
	}
	return true;
}

void LostArk::Server::CGameRoom::Handle_DebugGiveItem(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DEBUG_GIVE_ITEM& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (nullptr == session || sessionIter == m_PlayerIdBySessionId.end())
		return;
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;

	SERVER_PLAYER& player = playerIter->second;
	if (!Grant_Item(player, request.strItemId, request.iQuantity))
		return;

	if (!Send_InventorySnapshot(
		session, request.iRequestSequence, player.Inventory))
	{
		session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Handle_UseItem(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_USE_ITEM& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (nullptr == session || sessionIter == m_PlayerIdBySessionId.end())
		return;
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;

	SERVER_PLAYER& player = playerIter->second;
	const SERVER_ITEM_DEFINITION* itemDefinition =
		m_ItemCatalog.Find_Item(request.strItemId);
	// Not owned, not a consumable, or already dead: no-op. A dead player using
	// a heal potion would just resurrect them for free outside the real
	// revive path, so this is deliberately excluded too.
	if (nullptr == itemDefinition || 0u == itemDefinition->iHealPercent ||
		0u == player.iCurrentHp)
	{
		return;
	}

	const auto existing = std::find_if(
		player.Inventory.begin(), player.Inventory.end(),
		[&request](const INVENTORY_ITEM_SNAPSHOT& item)
		{
			return item.strItemId == request.strItemId &&
				EQUIPMENT_SLOT::NONE == item.eEquippedSlot;
		});
	if (existing == player.Inventory.end() || 0u == existing->iQuantity)
		return;

	const std::uint64_t healAmount =
		(static_cast<std::uint64_t>(player.iMaximumHp) *
			static_cast<std::uint64_t>(itemDefinition->iHealPercent)) / 100u;
	const std::uint32_t healedFromHp = player.iCurrentHp;
	player.iCurrentHp = static_cast<std::uint32_t>((std::min)(
		static_cast<std::uint64_t>(player.iMaximumHp),
		static_cast<std::uint64_t>(player.iCurrentHp) + healAmount));

	/* Retail floats the restored amount over the drinker in COLOR_HEAL. The number is
	what the potion actually restored, so a nearly full bar shows the smaller figure. */
	const std::uint32_t healedAmount = player.iCurrentHp - healedFromHp;
	if (0u != healedAmount &&
		m_PendingCommandDamageEvents.size() < MAX_DAMAGE_EVENTS)
	{
		DAMAGE_EVENT healEvent{};
		healEvent.iTargetNetEntityId = player.iNetEntityId;
		healEvent.iAmount = healedAmount;
		healEvent.fPositionX = player.fPositionX;
		healEvent.fPositionY = player.fPositionY;
		healEvent.fPositionZ = player.fPositionZ;
		healEvent.isOutgoing = true;
		healEvent.iSourcePlayerId = playerIter->first;
		healEvent.eHitFlag = DAMAGE_HIT_FLAG::HEAL;
		m_PendingCommandDamageEvents.push_back(healEvent);
	}

	--existing->iQuantity;
	if (0u == existing->iQuantity)
		player.Inventory.erase(existing);

	if (!Send_InventorySnapshot(
		session, request.iRequestSequence, player.Inventory))
	{
		session->Request_Close();
	}
}

namespace
{
	/* ItemCatalog.json characterClass for a class; "" for a class with no items. */
	const char* Item_ClassName(const LostArk::Shared::CHARACTER_CLASS_ID eClass)
	{
		using LostArk::Shared::CHARACTER_CLASS_ID;
		switch (eClass)
		{
		case CHARACTER_CLASS_ID::LANCE_MASTER: return "LanceMaster";
		case CHARACTER_CLASS_ID::GUNSLINGER: return "Gunslinger";
		case CHARACTER_CLASS_ID::SLAYER: return "Slayer";
		case CHARACTER_CLASS_ID::ARTIST: return "Artist";
		case CHARACTER_CLASS_ID::DIMENSIONMASTER: return "DimensionMaster";
		case CHARACTER_CLASS_ID::WARLORD: return "Warlord";
		case CHARACTER_CLASS_ID::GUARDIANKNIGHT: return "GuardianKnight";
		default: return "";
		}
	}

	bool Is_UsableByClass(const LostArk::Server::SERVER_ITEM_DEFINITION& item,
		const LostArk::Shared::CHARACTER_CLASS_ID eClass)
	{
		return item.strCharacterClass.empty() || item.strCharacterClass == Item_ClassName(eClass);
	}

	/* A bag entry of this item already exists, so an equipped copy cannot go back to the
	   bag without breaking the one-bag-entry-per-item rule. */
	bool Has_BagEntry(const std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT>& inventory,
		const std::string& itemId)
	{
		for (const LostArk::Shared::INVENTORY_ITEM_SNAPSHOT& item : inventory)
			if (item.strItemId == itemId && LostArk::Shared::EQUIPMENT_SLOT::NONE == item.eEquippedSlot)
				return true;
		return false;
	}
}

bool LostArk::Server::CGameRoom::Apply_SetEquipment(
	SERVER_PLAYER& player, const LostArk::Shared::C2S_SET_EQUIPMENT& request) const
{
	using namespace LostArk::Shared;
	std::vector<INVENTORY_ITEM_SNAPSHOT>& inventory = player.Inventory;
	const auto occupant = std::find_if(inventory.begin(), inventory.end(),
		[&request](const INVENTORY_ITEM_SNAPSHOT& item) { return item.eEquippedSlot == request.eSlot; });

	if (!request.bEquip)
	{
		if (occupant == inventory.end() || Has_BagEntry(inventory, occupant->strItemId))
			return false;
		occupant->eEquippedSlot = EQUIPMENT_SLOT::NONE;
		return true;
	}

	const SERVER_ITEM_DEFINITION* definition = m_ItemCatalog.Find_Item(request.strItemId);
	const char* slotKind = Equipment_SlotKind(request.eSlot);
	if (nullptr == definition || nullptr == slotKind || definition->strEquipSlot != slotKind ||
		!Is_UsableByClass(*definition, player.eCharacterClass))
		return false;
	const auto bagEntry = std::find_if(inventory.begin(), inventory.end(),
		[&request](const INVENTORY_ITEM_SNAPSHOT& item)
		{
			return item.strItemId == request.strItemId && EQUIPMENT_SLOT::NONE == item.eEquippedSlot;
		});
	if (bagEntry == inventory.end() || 0u == bagEntry->iQuantity)
		return false;
	/* The item the slot held goes back to the bag. */
	if (occupant != inventory.end())
	{
		if (occupant->strItemId != request.strItemId && Has_BagEntry(inventory, occupant->strItemId))
			return false;
		if (occupant->strItemId == request.strItemId)
			return false;
	}
	const bool splitsStack = 1u < bagEntry->iQuantity;
	if (splitsStack && inventory.size() >= MAX_INVENTORY_ITEMS)
		return false;

	if (occupant != inventory.end())
		occupant->eEquippedSlot = EQUIPMENT_SLOT::NONE;
	if (splitsStack)
	{
		--bagEntry->iQuantity;
		INVENTORY_ITEM_SNAPSHOT equipped{};
		equipped.strItemId = request.strItemId;
		equipped.iQuantity = 1u;
		equipped.eEquippedSlot = request.eSlot;
		inventory.push_back(std::move(equipped));
	}
	else
	{
		bagEntry->eEquippedSlot = request.eSlot;
	}
	return true;
}

bool LostArk::Server::CGameRoom::Unequip_OtherClassItems(SERVER_PLAYER& player) const
{
	using namespace LostArk::Shared;
	bool changed = false;
	for (INVENTORY_ITEM_SNAPSHOT& item : player.Inventory)
	{
		if (EQUIPMENT_SLOT::NONE == item.eEquippedSlot)
			continue;
		const SERVER_ITEM_DEFINITION* definition = m_ItemCatalog.Find_Item(item.strItemId);
		if (nullptr == definition || Is_UsableByClass(*definition, player.eCharacterClass) ||
			Has_BagEntry(player.Inventory, item.strItemId))
			continue;
		item.eEquippedSlot = EQUIPMENT_SLOT::NONE;
		changed = true;
	}
	return changed;
}

void LostArk::Server::CGameRoom::Handle_SetEquipment(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_SET_EQUIPMENT& request)
{
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (nullptr == session || sessionIter == m_PlayerIdBySessionId.end())
		return;
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;
	/* A refused move still answers, so the window drops any optimistic state. */
	(void)Apply_SetEquipment(playerIter->second, request);
	if (!Send_InventorySnapshot(
		session, request.iRequestSequence, playerIter->second.Inventory))
	{
		session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Handle_DespawnAllWorldEntities(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_DESPAWN_ALL_WORLD_ENTITIES& request)
{
	using namespace LostArk::Shared;
	(void)request;
	// Same room gating as Handle_SpawnWorldEntity -- this debug revert only makes
	// sense for the Character Select Arena's own spawn buttons and the
	// KoukuSaydon arena's Debug gate buttons.
#ifdef _DEBUG
	if (Is_KoukuRaidRunning()) return;
	const bool koukuGateWorld = WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId;
    const bool valtanWorld = WORLD_ID::VALTAN_ARENA == m_eWorldId;
#else
	const bool koukuGateWorld = false;
    const bool valtanWorld = false;
#endif
	if ((WORLD_ID::CHARACTER_SELECT_ARENA != m_eWorldId && !koukuGateWorld && !valtanWorld) ||
		!m_PlayerIdBySessionId.contains(sessionId))
	{
		return;
	}
	if (koukuGateWorld)
	{
		Reset_CardMaze();
		if (!Despawn_KoukuSaydonArenaDebugEntities())
			Mark_RuntimeFailure("despawn-all.koukusaydon-arena");
		else if (auto player = m_Players.find(m_PlayerIdBySessionId.at(sessionId)); player != m_Players.end())
			player->second.Clear_KoukuAssignedCard();
		return;
	}

#ifdef _DEBUG
    if (valtanWorld)
    {
        std::vector<NET_ENTITY_ID> removed;
        for (const auto& entity : m_WorldEntities)
            if (entity.eKind == WORLD_BOOTSTRAP_KIND::BOSS &&
                entity.strArchetypeId == "BOSS_VALTAN" && entity.strEncounterId == "ENCOUNTER_VALTAN" &&
                entity.iOwnerBossNetEntityId == INVALID_NET_ENTITY_ID)
                removed.push_back(entity.iNetEntityId);
        for (bool changed = true; changed;)
        {
            changed = false;
            for (const auto& entity : m_WorldEntities)
                if (std::find(removed.begin(), removed.end(), entity.iNetEntityId) == removed.end() &&
                    std::find(removed.begin(), removed.end(), entity.iOwnerBossNetEntityId) != removed.end())
                { removed.push_back(entity.iNetEntityId); changed = true; }
        }
        if (removed.empty()) return;
        // Preserve wave monsters, NPCs, player effects, arena destruction and
        // per-session request receipts. Only the Valtan ownership tree retires.
        Cancel_ValtanPatternIdAudition("Valtan despawned by the arena editor");
        if (Is_ValtanPatternFlowRunning())
            Queue_ValtanPatternFlowLifecycle(VALTAN_PATTERN_FLOW_LIFECYCLE_STATE::ABORTED,
                Find_AuditionBoss(), "Valtan despawned by the arena editor");
        m_ValtanPatternFlowAudition = {};
        (void)Stop_ValtanTimelineRow(false);
        m_ValtanFightPageStart = {};
        m_iValtanAuditionArmedHealthBar = 0u;
        const uint32_t despawnTick = m_iServerTick ? m_iServerTick : 1u;
        for (auto id = removed.rbegin(); id != removed.rend(); ++id)
        {
            const auto entity = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
                [id](const auto& candidate) { return candidate.iNetEntityId == *id; });
            if (entity == m_WorldEntities.end()) continue;
            if (entity->eKind == WORLD_BOOTSTRAP_KIND::BOSS)
            {
                Clear_ValtanGhostRelocationState(*entity);
                (void)Release_PlayerAttachments(*id, 0.f, 0u, false, 0u, despawnTick);
            }
            m_CombatObjectRuntime.Cancel_Source(*id);
            if (!Broadcast_CombatObjectLifecycle())
            { Mark_RuntimeFailure("despawn-valtan.combat-object-lifecycle"); return; }
            Broadcast_WorldEntityDespawned(*id);
            m_WorldEntities.erase(entity);
        }
        if (!Flush_ValtanPatternIdAuditionLifecycle() || !Flush_ValtanPatternFlowLifecycle())
            Mark_RuntimeFailure("despawn-valtan.audition-lifecycle");
        m_strStatus = "Valtan and its dependents despawned: " + std::to_string(removed.size());
        return;
    }
#endif

	// Character Select Arena's own placements have no statically-enabled
	// MONSTER/BOSS entries (confirmed: only 4 disabled playerSpawn + one disabled
	// BOSS_VALTAN), so everything currently in m_WorldEntities here was created by
	// the debug spawn buttons -- safe to despawn all of it unconditionally.
	const std::uint32_t despawnTick =
		0u == m_iServerTick ? 1u : m_iServerTick;
	for (auto entity = m_WorldEntities.rbegin(); entity != m_WorldEntities.rend(); ++entity)
	{
		if (WORLD_BOOTSTRAP_KIND::BOSS == entity->eKind)
		{
			(void)Release_PlayerAttachments(
				entity->iNetEntityId, 0.f, 0u, false, 0u, despawnTick);
		}
		m_CombatObjectRuntime.Cancel_Source(entity->iNetEntityId);
		if (!Broadcast_CombatObjectLifecycle())
		{
			Mark_RuntimeFailure("despawn-all.combat-object-lifecycle");
			return;
		}
		Broadcast_WorldEntityDespawned(entity->iNetEntityId);
	}
	m_WorldEntities.clear();

	// Reset spawn group state (DORMANT) too, so the same group can be activated
	// again -- Handle_SpawnWorldEntity's Is_ActiveOrCompleted check would otherwise
	// keep refusing a re-spawn after this revert. Same call
	// Reset_ReplayableArenaWhenEmpty already uses for the equivalent "room is
	// empty" reset, just without that gate.
	std::string resetStatus;
	if (!m_SpawnGroupRuntime.Initialize(m_SpawnGroupBootstrap, resetStatus))
		m_strStatus = std::move(resetStatus);
}

bool LostArk::Server::CGameRoom::Despawn_KoukuSaydonArenaDebugEntities(const bool allArenaBosses)
{
	return Despawn_KoukuSaydonArenaDebugEntities(allArenaBosses, false);
}

bool LostArk::Server::CGameRoom::Despawn_KoukuSaydonArenaDebugEntities(const bool allArenaBosses, const bool preflightOnly)
{
	/* The KoukuSaydon arena keeps its statically enabled Gate 1 Kouku and any
	Esther summon. Only entities raised from a disabled bootstrap placement
	(the Debug gate buttons) and the dependents they own are removed. An
	audition owned by one of those removed bosses is aborted explicitly. */
	std::vector<LostArk::Shared::NET_ENTITY_ID> removedIds;
	for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		if (entity.isEstherSummon)
			continue;
		const WORLD_BOOTSTRAP_PLACEMENT* placement =
			Find_Placement(entity.strPlacementId);
		const bool debugActivated =
			nullptr == placement || !placement->isEnabled;
		const bool ownedByRemoved =
			LostArk::Shared::INVALID_NET_ENTITY_ID != entity.iOwnerBossNetEntityId &&
			removedIds.end() != std::find(removedIds.begin(), removedIds.end(),
				entity.iOwnerBossNetEntityId);
		if ((allArenaBosses ? entity.eKind == WORLD_BOOTSTRAP_KIND::BOSS : debugActivated) || ownedByRemoved)
			removedIds.push_back(entity.iNetEntityId);
	}
	if (allArenaBosses)
	{
		// Ownership can precede the owner in storage. Close the dependency set
		// without removing unrelated NPCs, monsters or Esther summons.
		bool added = true;
		while (added)
		{
			added = false;
			for (const auto& entity : m_WorldEntities)
				if (!entity.isEstherSummon && std::find(removedIds.begin(), removedIds.end(), entity.iNetEntityId) == removedIds.end() &&
					std::find(removedIds.begin(), removedIds.end(), entity.iOwnerBossNetEntityId) != removedIds.end())
				{ removedIds.push_back(entity.iNetEntityId); added = true; }
		}
	}
	if (preflightOnly)
	{
		// Entry admission validates pending lifecycle encoding before resetting the
		// arena. Cancel_Source-generated messages are checked on a private copy.
		auto staged = m_CombatObjectRuntime;
		for (const auto id : removedIds) staged.Cancel_Source(id);
		std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_SPAWNED> spawned;
		std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_PRESENTATION_EVENT> events;
		std::vector<LostArk::Shared::S2C_COMBAT_OBJECT_DESPAWNED> despawned;
		staged.Drain_Lifecycle(spawned, events, despawned);
		const auto valid = [](const auto& messages) {
			for (const auto& message : messages)
			{
				LostArk::Shared::CPacketWriter writer;
				if (!LostArk::Shared::Write_Message(writer, message)) return false;
			}
			return true;
		};
		return valid(spawned) && valid(events) && valid(despawned);
	}
	const std::uint32_t despawnTick =
		0u == m_iServerTick ? 1u : m_iServerTick;
	for (auto id = removedIds.rbegin(); id != removedIds.rend(); ++id)
	{
		const auto entity = std::find_if(
			m_WorldEntities.begin(), m_WorldEntities.end(),
			[removedId = *id](const SERVER_WORLD_ENTITY& candidate)
			{
				return candidate.iNetEntityId == removedId;
			});
		if (m_WorldEntities.end() == entity)
			continue;
		if (WORLD_BOOTSTRAP_KIND::BOSS == entity->eKind)
		{
			(void)Release_PlayerAttachments(
				entity->iNetEntityId, 0.f, 0u, false, 0u, despawnTick);
#ifdef _DEBUG
			if (Find_KoukuAuditionMember(entity->iNetEntityId))
			{ m_strStatus = "KoukuSaydon audition target was despawned by a gate change"; Clear_KoukuSaydonPatternAudition(); }

#endif
		}
		m_CombatObjectRuntime.Cancel_Source(entity->iNetEntityId);
		if (!Broadcast_CombatObjectLifecycle())
			return false;
		Broadcast_WorldEntityDespawned(entity->iNetEntityId);
		m_WorldEntities.erase(entity);
	}
	m_strStatus = "KoukuSaydon arena Debug entities despawned: " +
		std::to_string(removedIds.size());
	return true;
}

void LostArk::Server::CGameRoom::Handle_ConfirmNpcEntry(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_CONFIRM_NPC_ENTRY& request)
{
	using namespace LostArk::Shared;
	// Mirrors the two disabled changeLevel triggerBox placements
	// (trigger.bern.to-valtan / valtan) in Data/Worlds/LV_BER_BERNCASTLE/
	// Gameplay.world.json -- both guide NPCs currently lead to the same target.
	struct VALTAN_ENTRY_GUIDE_NPC
	{
		const char* pNpcPlacementId;
		WORLD_ID eTargetWorldId;
	};
	static constexpr VALTAN_ENTRY_GUIDE_NPC VALTAN_ENTRY_GUIDE_NPCS[] =
	{
		{ "npc.bern.beda.guide", WORLD_ID::VALTAN_ARENA },
		{ "npc.bern.aylara", WORLD_ID::VALTAN_ARENA },
	};
	// Same footprint as the old trigger boxes' largest half extent (2m), so
	// standing where the box used to be still reaches the NPC.
	constexpr float INTERACTION_RADIUS = 3.f;

	if (WORLD_ID::BERN != m_eWorldId)
		return;

	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end())
		return;
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;
	const SERVER_PLAYER& player = playerIter->second;
	if (0u == player.iCurrentHp || PLAYER_ACTION_STATE::NONE != player.eAction)
		return;

	const auto guideIter = std::find_if(
		std::begin(VALTAN_ENTRY_GUIDE_NPCS), std::end(VALTAN_ENTRY_GUIDE_NPCS),
		[&request](const VALTAN_ENTRY_GUIDE_NPC& guide)
		{
			return request.strNpcPlacementId == guide.pNpcPlacementId;
		});
	if (std::end(VALTAN_ENTRY_GUIDE_NPCS) == guideIter)
		return;

	const auto entityIter = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[&request](const SERVER_WORLD_ENTITY& entity)
		{
			return WORLD_BOOTSTRAP_KIND::NPC == entity.eKind &&
				entity.strPlacementId == request.strNpcPlacementId;
		});
	if (m_WorldEntities.end() == entityIter)
		return;

	const float deltaX = player.fPositionX - entityIter->fPositionX;
	const float deltaZ = player.fPositionZ - entityIter->fPositionZ;
	if (deltaX * deltaX + deltaZ * deltaZ >
		INTERACTION_RADIUS * INTERACTION_RADIUS)
	{
		return;
	}

	if (INVALID_SESSION_ID == player.iSessionId ||
		CHARACTER_CLASS_ID::END == player.eCharacterClass ||
		player.strNickName.empty())
	{
		return;
	}

	const auto isAlreadyStaged = [this](SESSION_ID sid)
	{
		return std::any_of(
			m_PendingWorldTransfers.begin(), m_PendingWorldTransfers.end(),
			[sid](const SERVER_WORLD_TRANSFER_REQUEST& pending)
			{
				return pending.iSessionId == sid ||
					std::find(pending.PartyBatchSessionIds.begin(),
						pending.PartyBatchSessionIds.end(), sid) != pending.PartyBatchSessionIds.end();
			});
	};
	if (isAlreadyStaged(player.iSessionId))
		return;

	/* Solo by default; becomes the whole party's member list only when the
	   confirming player is a partied leader (members.front(), the original
	   inviter -- see "Same-room party state" in GameRoom.h). A non-leader
	   member's solo confirm is rejected instead of splitting the party across
	   two rooms, so a formed party never breaks on a Valtan entry. */
	std::vector<PLAYER_ID> batchMemberIds{ playerIter->first };
	const auto partyIdIter = m_PartyIdByPlayerId.find(playerIter->first);
	if (partyIdIter != m_PartyIdByPlayerId.end())
	{
		const auto membersIter =
			m_PartyMembersByPartyId.find(partyIdIter->second);
		if (membersIter != m_PartyMembersByPartyId.end() &&
			membersIter->second.size() > 1)
		{
			if (membersIter->second.front() != playerIter->first)
			{
				Notify_PartyTransferFailure(sessionId, request.iRequestSequence,
					guideIter->eTargetWorldId, PARTY_TRANSFER_RESULT::REJECTED_NOT_LEADER);
				return;
			}
			batchMemberIds = membersIter->second;
		}
	}

	SERVER_WORLD_TRANSFER_REQUEST transfer{};
	transfer.iSessionId = player.iSessionId;
	transfer.eTargetWorldId = guideIter->eTargetWorldId;
	transfer.strRaidReturnNpcPlacementId = request.strNpcPlacementId;
	transfer.eCharacterClass = player.eCharacterClass;
	transfer.strNickName = player.strNickName;
	transfer.iHonorTitleId = player.iHonorTitleId;
	transfer.iPartyRequestSequence = request.iRequestSequence;
	for (const PLAYER_ID memberId : batchMemberIds)
	{
		const auto memberIter = m_Players.find(memberId);
		if (memberIter == m_Players.end() ||
			CHARACTER_CLASS_ID::END == memberIter->second.eCharacterClass ||
			memberIter->second.strNickName.empty() ||
			isAlreadyStaged(memberIter->second.iSessionId))
		{
			Notify_PartyTransferFailure(sessionId, request.iRequestSequence,
				guideIter->eTargetWorldId, PARTY_TRANSFER_RESULT::REJECTED_MEMBER_UNAVAILABLE);
			return;
		}
		if (batchMemberIds.size() > 1u)
			transfer.PartyBatchSessionIds.push_back(memberIter->second.iSessionId);
	}
	m_PendingWorldTransfers.push_back(std::move(transfer));
}
