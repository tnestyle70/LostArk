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
	player.bRestoreAvailable = false;
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
		session, request.iRequestSequence, player))
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
	if (!Is_NewerSequence(request.iRequestSequence, player.iLastItemUseSequence)) return;
	player.iLastItemUseSequence = request.iRequestSequence;
	const SERVER_ITEM_DEFINITION* itemDefinition = m_ItemCatalog.Find_Item(request.strItemId);
	if (!itemDefinition || !player.iCurrentHp ||
		(!itemDefinition->iHealPercent && itemDefinition->BattleUse.eKind == BATTLE_ITEM_KIND::NONE)) return;
	const auto existing = std::find_if(player.Inventory.begin(), player.Inventory.end(),
		[&request](const INVENTORY_ITEM_SNAPSHOT& item)
		{ return item.strItemId == request.strItemId && EQUIPMENT_SLOT::NONE == item.eEquippedSlot; });
	if (existing == player.Inventory.end() || !existing->iQuantity) return;
	const auto& use = itemDefinition->BattleUse;
	if (use.eKind != BATTLE_ITEM_KIND::NONE)
	{
		const auto tick = m_iServerTick == (std::numeric_limits<std::uint32_t>::max)() ? 1u : m_iServerTick + 1u;
		const auto cooldown = player.ItemCooldownEndTicks.find(request.strItemId);
		const bool isThrown = use.eKind == BATTLE_ITEM_KIND::DESTRUCTION || use.eKind == BATTLE_ITEM_KIND::WHIRLWIND;
		const bool isAllyTarget = use.eKind == BATTLE_ITEM_KIND::CLEANSE;
		if (Is_KoukuRaidInputBlocked() || player.Has_TimeStop(tick) || player.iMarioStage || player.bPatternBound ||
			player.TriggerMove.isActive || player.iVehicleId != INVALID_VEHICLE_ID ||
			player.eMadnessForm != PLAYER_MADNESS_FORM::NORMAL || player.eKoukuHudMode != KOUKU_HUD_MODE::NONE ||
			player.eAction != PLAYER_ACTION_STATE::NONE || player.fKnockbackRemainingSeconds > 0.f ||
			(cooldown != player.ItemCooldownEndTicks.end() && !Has_ReachedServerTick(tick, cooldown->second)) ||
			request.hasGroundTarget != isThrown ||
			(request.iTargetPlayerNetEntityId != INVALID_NET_ENTITY_ID) != isAllyTarget ||
			!std::isfinite(request.fTargetX) || !std::isfinite(request.fTargetZ) ||
			(!request.hasGroundTarget && (request.fTargetX != 0.f || request.fTargetZ != 0.f))) return;
		SERVER_NAV_POINT target{player.fPositionX, player.fPositionY, player.fPositionZ};
		if (request.hasGroundTarget)
		{
			const float dx = request.fTargetX - player.fPositionX, dz = request.fTargetZ - player.fPositionZ;
			const float range = static_cast<float>(use.iRangeCm) * .01f;
			if (dx * dx + dz * dz > range * range + .001f) return;
			target.x = request.fTargetX; target.z = request.fTargetZ;
		}
		if (use.eKind == BATTLE_ITEM_KIND::DESTRUCTION || use.eKind == BATTLE_ITEM_KIND::WHIRLWIND)
		{
			if (!m_ServerNavigation.Sample_SurfacePosition(target.x, target.z, target) ||
				std::abs(target.y - player.fPositionY) > 3.f) return;
			auto transaction = m_CombatObjectRuntime.Begin_Transaction();
			std::string status;
			if (!m_CombatObjectRuntime.Stage_BattleItemProjectile(transaction, player, use,
				target.x, target.y, target.z, m_GameplayCatalog, tick, status) ||
				!m_CombatObjectRuntime.Commit(std::move(transaction))) return;
			if (!Broadcast_CombatObjectLifecycle()) Mark_RuntimeFailure("battle.item.projectile-spawn");
		}
		else
		{
			SERVER_PLAYER* recipient = &player;
			if (use.eKind == BATTLE_ITEM_KIND::CLEANSE)
			{
				recipient = nullptr;
				const auto sourceParty = m_PartyIdByPlayerId.find(player.iPlayerId);
				if (sourceParty == m_PartyIdByPlayerId.end() || !sourceParty->second) return;
				for (auto& [id, candidate] : m_Players)
				{
					if (candidate.iNetEntityId != request.iTargetPlayerNetEntityId) continue;
					const auto targetParty = m_PartyIdByPlayerId.find(id);
					const float dx = candidate.fPositionX - player.fPositionX;
					const float dz = candidate.fPositionZ - player.fPositionZ;
					const float range = static_cast<float>(use.iRangeCm) * .01f;
					if (id == player.iPlayerId || !candidate.Is_Human() || !candidate.iCurrentHp ||
						targetParty == m_PartyIdByPlayerId.end() || targetParty->second != sourceParty->second ||
						candidate.iMarioStage || candidate.bPatternBound || candidate.TriggerMove.isActive ||
						candidate.eKoukuHudMode != KOUKU_HUD_MODE::NONE || candidate.eAttachmentSlot != PLAYER_ATTACHMENT_SLOT::NONE ||
						candidate.eAction == PLAYER_ACTION_STATE::DEAD || candidate.eAction == PLAYER_ACTION_STATE::FALLING ||
						std::abs(candidate.fPositionY - player.fPositionY) > 3.f || dx * dx + dz * dz > range * range + .001f) return;
					recipient = &candidate;
					break;
				}
				if (!recipient) return;
			}
			S2C_WORLD_SEQUENCE_PLAY presentation;
			presentation.strSequenceInstanceId = "world.item.use." + request.strItemId + "." +
				std::to_string(player.iNetEntityId) + "." + std::to_string(request.iRequestSequence) + "." +
				std::to_string(recipient->iNetEntityId);
			presentation.iStartTick = presentation.iServerTick = tick;
			presentation.iDurationMs = use.iDurationMs;
			presentation.fPositionOffsetX = recipient->fPositionX;
			presentation.fPositionOffsetY = recipient->fPositionY;
			presentation.fPositionOffsetZ = recipient->fPositionZ;
			CPacketWriter writer;
			if (!Write_Message(writer, presentation)) return;
			const auto endTick = tick + (use.iDurationMs * SERVER_TICK_HZ + 999u) / 1000u;
			if (use.eKind == BATTLE_ITEM_KIND::CLEANSE)
			{
				recipient->iHolyCharmProtectionEndTick = endTick;
				if (recipient->eAction == PLAYER_ACTION_STATE::FEAR)
				{
					recipient->iFearEndTick = tick;
					(void)CKoukuSaydonLogicRuntime::Update_PlayerFear(*recipient, tick);
				}
			}
			else
			{
				player.iTimeStopEndTick = endTick;
				player.hasMoveGoal = false; player.MovePath.clear(); player.iMovePathIndex = 0u;
				player.PendingCommand.Clear(); player.hasBufferedComboInput = false;
			}
			for (const auto& [id, observer] : m_Players)
			{
				(void)id;
				if (auto connection = Find_Session(observer.iSessionId); connection &&
					!connection->Send_Frame(PACKET_TYPE::S2C_WORLD_SEQUENCE_PLAY, writer.Get_Buffer())) connection->Request_Close();
			}
		}
		player.ItemCooldownEndTicks[request.strItemId] = tick + (use.iCooldownMs * SERVER_TICK_HZ + 999u) / 1000u;
	}
	else if (request.hasGroundTarget || request.iTargetPlayerNetEntityId != INVALID_NET_ENTITY_ID) return;

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
	player.bRestoreAvailable = false;

	if (!Send_InventorySnapshot(
		session, request.iRequestSequence, player))
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
		equipped.iDurabilityPercent = bagEntry->iDurabilityPercent;
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
	if (Apply_SetEquipment(playerIter->second, request))
		playerIter->second.bRestoreAvailable = false;
	if (!Send_InventorySnapshot(
		session, request.iRequestSequence, playerIter->second))
	{
		session->Request_Close();
	}
}

void LostArk::Server::CGameRoom::Handle_RepairEquipment(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_REPAIR_EQUIPMENT& request)
{
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (nullptr == session || sessionIter == m_PlayerIdBySessionId.end())
		return;
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;
	/* The window's two buttons: "repair equipped" mends the worn weapon and armor, "repair all"
	   mends every damaged piece, worn or in the bag. Only gear ever drops below 100, so the
	   percent alone says what is damaged. Each piece costs silver in proportion to its wear:
	   1000 silver at 0 percent, rounded up. A purse that cannot cover the whole bill repairs
	   nothing; the answer below still goes out so the window sees the unchanged state. */
	SERVER_PLAYER& player = playerIter->second;
	std::uint32_t cost = 0u;
	for (const LostArk::Shared::INVENTORY_ITEM_SNAPSHOT& item : player.Inventory)
	{
		if (item.iDurabilityPercent >= 100u ||
			(!request.bAllSlots && !LostArk::Shared::Is_Durable_Slot(item.eEquippedSlot)))
			continue;
		cost += LostArk::Shared::Repair_Silver_Cost(item.iDurabilityPercent);
	}
	if (player.Purse.iSilver >= cost)
	{
		player.Purse.iSilver -= cost;
		for (LostArk::Shared::INVENTORY_ITEM_SNAPSHOT& item : player.Inventory)
		{
			if (item.iDurabilityPercent < 100u &&
				(request.bAllSlots || LostArk::Shared::Is_Durable_Slot(item.eEquippedSlot)))
				item.iDurabilityPercent = 100u;
		}
		player.bDurabilityDirty = false;
		if (0u != cost)
			player.bRestoreAvailable = false;
	}
	if (!Send_InventorySnapshot(
		session, request.iRequestSequence, playerIter->second))
	{
		session->Request_Close();
	}
}

bool LostArk::Server::CGameRoom::Apply_BuyItems(
	SERVER_PLAYER& player, const LostArk::Shared::C2S_BUY_ITEMS& request) const
{
	using namespace LostArk::Shared;
	/* The window stays open while the player walks a little, so the check is a few metres
	   wider than the 3 m the client stops at. */
	constexpr float SHOP_INTERACTION_RADIUS = 6.f;
	if (WORLD_ID::BERN != m_eWorldId || 0u == player.iCurrentHp)
		return false;
	const auto npc = std::find_if(m_WorldEntities.begin(), m_WorldEntities.end(),
		[&request](const SERVER_WORLD_ENTITY& entity)
		{
			return WORLD_BOOTSTRAP_KIND::NPC == entity.eKind &&
				entity.strPlacementId == request.strNpcPlacementId;
		});
	if (m_WorldEntities.end() == npc)
		return false;
	const float deltaX = player.fPositionX - npc->fPositionX;
	const float deltaZ = player.fPositionZ - npc->fPositionZ;
	if (deltaX * deltaX + deltaZ * deltaZ > SHOP_INTERACTION_RADIUS * SHOP_INTERACTION_RADIUS)
		return false;

	/* Price every line first; the basket is bought whole or not at all. */
	std::vector<INVENTORY_ITEM_SNAPSHOT> staged = player.Inventory;
	SERVER_PURSE stagedPurse = player.Purse;
	const auto bagEntry = [&staged](const std::string& itemId)
	{
		return std::find_if(staged.begin(), staged.end(), [&itemId](const INVENTORY_ITEM_SNAPSHOT& item)
			{ return item.strItemId == itemId && EQUIPMENT_SLOT::NONE == item.eEquippedSlot; });
	};
	for (const SHOP_BASKET_ENTRY& entry : request.Entries)
	{
		const SERVER_SHOP_ITEM* stock = m_ItemCatalog.Find_ShopItem(request.strNpcPlacementId, entry.strItemId);
		const SERVER_ITEM_DEFINITION* definition = m_ItemCatalog.Find_Item(entry.strItemId);
		if (nullptr == stock || nullptr == definition)
			return false;
		/* A shop template is one icon for every class: the buyer receives the variant of the
		   buyer's class. A class the template has no variant for cannot buy it. */
		const SERVER_ITEM_DEFINITION* granted = definition;
		if (!definition->ClassVariants.empty())
		{
			const auto variant = definition->ClassVariants.find(Item_ClassName(player.eCharacterClass));
			if (definition->ClassVariants.end() == variant)
				return false;
			granted = m_ItemCatalog.Find_Item(variant->second);
			if (nullptr == granted)
				return false;
		}
		/* An avatar is bought once: worn or in the bag, owning it refuses another copy. */
		if (0 == granted->strEquipSlot.compare(0, 6, "avatar") &&
			std::any_of(staged.begin(), staged.end(), [granted](const INVENTORY_ITEM_SNAPSHOT& item)
				{ return item.strItemId == granted->strItemId; }))
			return false;
		const std::uint64_t cost = static_cast<std::uint64_t>(stock->iPrice) * entry.iQuantity;
		std::uint32_t& purse = stagedPurse.Amount(stock->eCurrency);
		if (purse < cost)
			return false;
		purse -= static_cast<std::uint32_t>(cost);

		/* No silent cap: a line that would overflow the stack refuses the basket. */
		const auto owned = bagEntry(granted->strItemId);
		if (staged.end() == owned)
		{
			if (staged.size() >= MAX_INVENTORY_ITEMS || entry.iQuantity > granted->iMaxStack)
				return false;
			INVENTORY_ITEM_SNAPSHOT item{};
			item.strItemId = granted->strItemId;
			item.iQuantity = entry.iQuantity;
			staged.push_back(std::move(item));
		}
		else
		{
			if (static_cast<std::uint64_t>(owned->iQuantity) + entry.iQuantity > granted->iMaxStack)
				return false;
			owned->iQuantity += entry.iQuantity;
		}
	}
	player.Inventory = std::move(staged);
	player.Purse = stagedPurse;
	player.bRestoreAvailable = false;
	return true;
}

void LostArk::Server::CGameRoom::Handle_BuyItems(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_BUY_ITEMS& request)
{
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (nullptr == session || sessionIter == m_PlayerIdBySessionId.end())
		return;
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end())
		return;
	/* A refused basket still answers, so the window shows the unchanged purse. */
	(void)Apply_BuyItems(playerIter->second, request);
	if (!Send_InventorySnapshot(
		session, request.iRequestSequence, playerIter->second))
	{
		session->Request_Close();
	}
}

bool LostArk::Server::CGameRoom::Validate_RestoreCharacter(
	const SERVER_PLAYER& player, const LostArk::Shared::C2S_RESTORE_CHARACTER& request) const
{
	using namespace LostArk::Shared;
	/* Wire decoding already rejected unknown slots, empty ids, zero counts, two items in one
	   slot and a stack listed twice; what is left is what only the Server's catalogs know. */
	if (request.Items.size() > MAX_INVENTORY_ITEMS)
		return false;
	for (const INVENTORY_ITEM_SNAPSHOT& item : request.Items)
	{
		const SERVER_ITEM_DEFINITION* definition = m_ItemCatalog.Find_Item(item.strItemId);
		if (nullptr == definition || 0u == item.iQuantity || item.iQuantity > definition->iMaxStack)
			return false;
		if (EQUIPMENT_SLOT::NONE == item.eEquippedSlot)
			continue;
		/* An equipped entry is one item in a slot of its own kind that this class can wear. */
		const char* slotKind = Equipment_SlotKind(item.eEquippedSlot);
		if (1u != item.iQuantity || nullptr == slotKind || definition->strEquipSlot != slotKind ||
			!Is_UsableByClass(*definition, player.eCharacterClass))
			return false;
	}
	return INVALID_HONOR_TITLE_ID == request.iHonorTitleId ||
		m_HonorTitleCatalog.Has_Title(request.iHonorTitleId);
}

void LostArk::Server::CGameRoom::Handle_RestoreCharacter(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_RESTORE_CHARACTER& request)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (nullptr == session) return;
	const auto reply = [&](const CHARACTER_RESTORE_RESULT verdict, const HONOR_TITLE_ID title = INVALID_HONOR_TITLE_ID)
	{
		const S2C_RESTORE_CHARACTER_RESULT result{request.iRequestSequence, verdict, title};
		CPacketWriter writer;
		if (!Write_Message(writer, result) || !session->Send_Frame(
			PACKET_TYPE::S2C_RESTORE_CHARACTER_RESULT, writer.Get_Buffer())) session->Request_Close();
	};
	const auto playerIter = sessionIter == m_PlayerIdBySessionId.end() ? m_Players.end() : m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end() || playerIter->second.iSessionId != sessionId)
	{ reply(CHARACTER_RESTORE_RESULT::REJECTED_SESSION); return; }
	SERVER_PLAYER& player = playerIter->second;
	/* Once per fresh Bern entry, before the player changed anything. The chance is spent even
	   by a request that fails validation, so a client cannot probe with variants. */
	if (WORLD_ID::BERN != m_eWorldId || !player.bRestoreAvailable)
	{ reply(CHARACTER_RESTORE_RESULT::REJECTED_UNAVAILABLE); return; }
	player.bRestoreAvailable = false;
	if (!Validate_RestoreCharacter(player, request))
	{
		m_strStatus = "A saved character restore was refused by the Server catalogs.";
		reply(CHARACTER_RESTORE_RESULT::REJECTED_CATALOG);
		return;
	}
	player.Inventory = request.Items;
	player.Purse.iSilver = request.iSilver;
	player.Purse.iGold = request.iGold;
	player.iHonorTitleId = request.iHonorTitleId;
	if (!Send_InventorySnapshot(session, request.iRequestSequence, player))
	{ session->Request_Close(); return; }
	reply(CHARACTER_RESTORE_RESULT::APPLIED, player.iHonorTitleId);
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
	if (Is_KoukuRaidRunning()) return;
	const bool koukuGateWorld = WORLD_ID::KAKULSAYDON_ARENA == m_eWorldId;
    const bool valtanWorld = WORLD_ID::VALTAN_ARENA == m_eWorldId;
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
	/* Esther summons now carry damage and protection. A gate reset must end
	them along with the old encounter, including delayed summons and zones.
	Preflight below only validates this removal; it never changes live state. */
	std::vector<LostArk::Shared::NET_ENTITY_ID> removedIds;
	for (const SERVER_WORLD_ENTITY& entity : m_WorldEntities)
	{
		if (entity.isEstherSummon)
		{
			removedIds.push_back(entity.iNetEntityId);
			continue;
		}
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
		// without removing unrelated NPCs or monsters.
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
	m_PendingEstherSummons.clear();
	m_EstherZones.clear();
	for (auto& [id, player] : m_Players)
	{
		(void)id;
		player.bRonaunGuard = false; player.iRonaunGrantTick = 0u;
		player.iEstherGuardEndTick = 0u;
		player.iEstherGuardDamageTakenPercent = 0;
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
	transfer.iVoiceType = player.iVoiceType;
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

namespace
{
	// The Colosseum NPC inside the Bern castle; the queue request names it and the Server re-tests
	// that the player stands next to it.
	constexpr const char* COLOSSEUM_QUEUE_NPC_PLACEMENT_ID = "npc.bern.25184_1.2";
	// Product matchmaking always requires four humans in both configurations.
	constexpr std::uint8_t COLOSSEUM_MATCH_REQUIRED_PLAYERS = 4u;
}

void LostArk::Server::CGameRoom::Send_ColosseumQueueState(
	const SESSION_ID sessionId, const LostArk::Shared::COLOSSEUM_QUEUE_STATE state)
{
	using namespace LostArk::Shared;
	const std::shared_ptr<CClientSession> session = Find_Session(sessionId);
	if (nullptr == session)
		return;
	S2C_COLOSSEUM_QUEUE_STATE message{};
	message.eState = state;
	message.iQueuedCount = static_cast<std::uint8_t>(
		(std::min)(m_ColosseumQueue.size(), MAX_COLOSSEUM_MATCH_PLAYERS));
	message.iRequiredCount = COLOSSEUM_MATCH_REQUIRED_PLAYERS;
	CPacketWriter writer;
	if (!Write_Message(writer, message))
		return;
	if (!session->Send_Frame(PACKET_TYPE::S2C_COLOSSEUM_QUEUE_STATE, writer.Get_Buffer()))
		session->Request_Close();
}

void LostArk::Server::CGameRoom::Handle_ColosseumQueueJoin(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_COLOSSEUM_QUEUE_JOIN& request)
{
	using namespace LostArk::Shared;
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

	// A second accept from a queued player changes nothing; the wait window just keeps waiting.
	if (std::any_of(m_ColosseumQueue.begin(), m_ColosseumQueue.end(),
		[sessionId](const COLOSSEUM_QUEUE_ENTRY& entry) { return entry.iSessionId == sessionId; }))
	{
		Send_ColosseumQueueState(sessionId, COLOSSEUM_QUEUE_STATE::WAITING);
		return;
	}

	// Every refusal is answered, so the Client's wait window closes instead of waiting forever.
	const auto Fn_Reject = [this, sessionId]()
	{
		Send_ColosseumQueueState(sessionId, COLOSSEUM_QUEUE_STATE::REJECTED);
	};
	if (request.strNpcPlacementId != COLOSSEUM_QUEUE_NPC_PLACEMENT_ID ||
		0u == player.iCurrentHp || PLAYER_ACTION_STATE::NONE != player.eAction ||
		INVALID_SESSION_ID == player.iSessionId ||
		CHARACTER_CLASS_ID::END == player.eCharacterClass || player.strNickName.empty())
	{
		Fn_Reject();
		return;
	}

	const auto entityIter = std::find_if(
		m_WorldEntities.begin(), m_WorldEntities.end(),
		[&request](const SERVER_WORLD_ENTITY& entity)
		{
			return WORLD_BOOTSTRAP_KIND::NPC == entity.eKind &&
				entity.strPlacementId == request.strNpcPlacementId;
		});
	if (m_WorldEntities.end() == entityIter)
	{
		Fn_Reject();
		return;
	}
	const float deltaX = player.fPositionX - entityIter->fPositionX;
	const float deltaZ = player.fPositionZ - entityIter->fPositionZ;
	if (deltaX * deltaX + deltaZ * deltaZ > INTERACTION_RADIUS * INTERACTION_RADIUS)
	{
		Fn_Reject();
		return;
	}

	const bool alreadyStaged = std::any_of(
		m_PendingWorldTransfers.begin(), m_PendingWorldTransfers.end(),
		[sessionId](const SERVER_WORLD_TRANSFER_REQUEST& pending)
		{
			return pending.iSessionId == sessionId ||
				std::find(pending.PartyBatchSessionIds.begin(),
					pending.PartyBatchSessionIds.end(), sessionId) != pending.PartyBatchSessionIds.end();
		});
	if (alreadyStaged)
	{
		Fn_Reject();
		return;
	}

	// A randomized four-human match must not split an existing human party.
	// Personal guides wait in Bern and do not occupy queue or team slots.
	if (const auto partyIdIter = m_PartyIdByPlayerId.find(playerIter->first);
		partyIdIter != m_PartyIdByPlayerId.end())
	{
		const auto membersIter = m_PartyMembersByPartyId.find(partyIdIter->second);
		if (membersIter != m_PartyMembersByPartyId.end() && membersIter->second.size() > 1u)
		{
			Fn_Reject();
			return;
		}
	}

	m_ColosseumQueue.push_back(COLOSSEUM_QUEUE_ENTRY{ sessionId, request.iRequestSequence });
	Send_ColosseumQueueState(sessionId, COLOSSEUM_QUEUE_STATE::WAITING);
	Try_FormColosseumMatch();
}

void LostArk::Server::CGameRoom::Handle_ColosseumQueueLeave(
	const SESSION_ID sessionId,
	const LostArk::Shared::C2S_COLOSSEUM_QUEUE_LEAVE& request)
{
	(void)request;
	const std::size_t removed = std::erase_if(m_ColosseumQueue,
		[sessionId](const COLOSSEUM_QUEUE_ENTRY& entry) { return entry.iSessionId == sessionId; });
	if (0u != removed)
		Send_ColosseumQueueState(sessionId, LostArk::Shared::COLOSSEUM_QUEUE_STATE::LEFT);
}

void LostArk::Server::CGameRoom::Try_FormColosseumMatch()
{
	using namespace LostArk::Shared;
	if (WORLD_ID::BERN != m_eWorldId || m_bColosseumTransferPending)
		return;

	const auto isStaged = [this](const SESSION_ID sessionId)
	{
		return std::any_of(
			m_PendingWorldTransfers.begin(), m_PendingWorldTransfers.end(),
			[sessionId](const SERVER_WORLD_TRANSFER_REQUEST& pending)
			{
				return pending.iSessionId == sessionId ||
					std::find(pending.PartyBatchSessionIds.begin(),
						pending.PartyBatchSessionIds.end(), sessionId) != pending.PartyBatchSessionIds.end();
			});
	};

	// Whoever can no longer enter (dead, already staged elsewhere) leaves the queue and is told so.
	std::vector<SESSION_ID> dropped;
	for (auto entryIter = m_ColosseumQueue.begin(); entryIter != m_ColosseumQueue.end();)
	{
		const auto sessionIter = m_PlayerIdBySessionId.find(entryIter->iSessionId);
		const auto playerIter = sessionIter == m_PlayerIdBySessionId.end()
			? m_Players.end() : m_Players.find(sessionIter->second);
        const bool conflictingParty = playerIter != m_Players.end() && [&]()
        {
            const auto party = m_PartyIdByPlayerId.find(playerIter->first);
            if (party == m_PartyIdByPlayerId.end()) return false;
            const auto members = m_PartyMembersByPartyId.find(party->second);
            return members == m_PartyMembersByPartyId.end() || members->second.size() > 1u;
        }();
        const bool pendingVote = playerIter != m_Players.end() && std::any_of(
            m_RaidEntryProposals.begin(), m_RaidEntryProposals.end(), [&playerIter](const auto& vote)
            { return std::find(vote.Voters.begin(), vote.Voters.end(), playerIter->first) != vote.Voters.end(); });
		if (playerIter == m_Players.end() || 0u == playerIter->second.iCurrentHp ||
			isStaged(entryIter->iSessionId) || conflictingParty || pendingVote)
		{
			dropped.push_back(entryIter->iSessionId);
			entryIter = m_ColosseumQueue.erase(entryIter);
		}
		else
			++entryIter;
	}
	for (const SESSION_ID sessionId : dropped)
		Send_ColosseumQueueState(sessionId, COLOSSEUM_QUEUE_STATE::LEFT);

	if (m_ColosseumQueue.size() < COLOSSEUM_MATCH_REQUIRED_PLAYERS)
		return;
	std::vector<SESSION_ID> seats;
	for (std::size_t i = 0; i < MAX_COLOSSEUM_MATCH_PLAYERS; ++i)
		seats.push_back(m_ColosseumQueue[i].iSessionId);
	static thread_local std::mt19937 generator{ std::random_device{}() };
	std::shuffle(seats.begin(), seats.end(), generator);
	const auto& leader = m_Players.at(m_PlayerIdBySessionId.at(seats.front()));
	SERVER_WORLD_TRANSFER_REQUEST transfer;
	transfer.iSessionId = seats.front();
	transfer.eTargetWorldId = WORLD_ID::COLOSSEUM;
	transfer.eCharacterClass = leader.eCharacterClass;
	transfer.strNickName = leader.strNickName;
	transfer.PartyBatchSessionIds = std::move(seats);
	transfer.bColosseumMatch = true;
	m_PendingWorldTransfers.push_back(std::move(transfer));
	m_bColosseumTransferPending = true;
}
