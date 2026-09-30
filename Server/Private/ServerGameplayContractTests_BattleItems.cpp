#include "ServerGameplayContractTests_Runner.h"
#include "GameRoom.h"
#include "ClientSession.h"
#include "ItemCatalog.h"
#include "ServerCombatHitRuntime.h"
#include "KoukuSaydonLogicRuntime.h"
#include "WorldBootstrap.h"
#include "ValtanBrain.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <algorithm>
#include <memory>
#include <vector>

using namespace LostArk::Server;
using namespace LostArk::Shared;

int CServerGameplayContractRunner::Run_BattleItemsOnly()
{
    TESTS tests;
    {
        // Use real admission and typed F1/restore handlers. Each connection owns
        // an independent saved character and receives its own inventory only.
        auto bern = std::make_unique<CGameRoom>(WORLD_ID::BERN);
        std::vector<std::shared_ptr<CClientSession>> peers;
        bool admitted = bern->Is_Ready();
        for (unsigned index = 0; index < 4u && admitted; ++index)
        {
            auto peer = std::make_shared<CClientSession>(93000u + index, INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
            peer->m_isSendRunning.store(true);
            bern->Handle_Register(peer);
            C2S_ENTER_WORLD entry;
            entry.eWorldId = WORLD_ID::BERN;
            entry.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
            entry.strNickName = "SavedBattleItems" + std::to_string(index);
            admitted = bern->Join(peer->Get_SessionId(), entry);
            peers.push_back(peer);
        }
        tests.Require(admitted && bern->Count_HumanPlayers() == 4u,
            "Four real Bern admissions provide independent character restore slots");
        const auto clearFrames = [&]() {
            for (const auto& peer : peers)
            { peer->m_OutboundFrames.clear(); peer->m_iQueuedOutboundBytes = 0u; }
        };
        const auto sameItems = [](const auto& left, const auto& right) {
            return left.size() == right.size() && std::equal(left.begin(), left.end(), right.begin(),
                [](const auto& a, const auto& b) { return a.strItemId == b.strItemId &&
                    a.iQuantity == b.iQuantity && a.eEquippedSlot == b.eEquippedSlot; });
        };
        const auto hasRestoreResult = [](const auto& peer, const auto expected, const unsigned sequence) {
            for (const auto& frame : peer->m_OutboundFrames)
                if (frame.ePacketType == PACKET_TYPE::S2C_RESTORE_CHARACTER_RESULT)
                {
                    CPacketReader reader{std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES)};
                    S2C_RESTORE_CHARACTER_RESULT result;
                    if (Read_Message(reader, result) && result.eResult == expected &&
                        result.iRequestSequence == sequence) return true;
                }
            return false;
        };
        if (admitted)
        {
            for (unsigned index = 0; index < peers.size(); ++index)
            {
                const auto& peer = peers[index];
                auto& player = bern->m_Players.at(peer->Get_PlayerId());
                C2S_RESTORE_CHARACTER saved;
                saved.iRequestSequence = 1u;
                saved.Items = {{"POTION_HP_SMALL", 10u + index, EQUIPMENT_SLOT::NONE},
                    {"EQUIP_WARLORD_HONORWHISPER_WEAPON", 1u, EQUIPMENT_SLOT::WEAPON}};
                if (index == 3u) saved.Items.clear(); // A deliberately empty saved bag is also authoritative.
                saved.iSilver = 1000u + index; saved.iGold = 100u + index; saved.iHonorTitleId = 30001u;
                CPacketWriter writer; C2S_RESTORE_CHARACTER decoded;
                const bool encoded = Write_Message(writer, saved);
                CPacketReader reader{writer.Get_Buffer()};
                const bool validWire = encoded && Read_Message(reader, decoded);
                clearFrames();
                if (validWire) bern->Handle_RestoreCharacter(peer->Get_SessionId(), decoded);
                bool ownSnapshot = false;
                for (const auto& frame : peer->m_OutboundFrames)
                    if (frame.ePacketType == PACKET_TYPE::S2C_INVENTORY_SNAPSHOT)
                    {
                        CPacketReader packet{std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES)};
                        S2C_INVENTORY_SNAPSHOT snapshot;
                        ownSnapshot = Read_Message(packet, snapshot) && sameItems(snapshot.Items, saved.Items) &&
                            snapshot.iSilver == saved.iSilver && snapshot.iGold == saved.iGold;
                    }
                tests.Require(validWire && ownSnapshot && sameItems(player.Inventory, saved.Items) &&
                    player.Purse.iSilver == saved.iSilver && player.Purse.iGold == saved.iGold &&
                    player.iHonorTitleId == saved.iHonorTitleId && !player.bRestoreAvailable &&
                    peer->m_OutboundFrames.size() == 2u &&
                    peer->m_OutboundFrames.front().ePacketType == PACKET_TYPE::S2C_INVENTORY_SNAPSHOT &&
                    hasRestoreResult(peer, CHARACTER_RESTORE_RESULT::APPLIED, 1u) &&
                    std::all_of(peers.begin(), peers.end(), [&](const auto& other) {
                        return other == peer || other->m_OutboundFrames.empty(); }),
                    "Saved equipment, purse and title restore atomically before APPLIED for only their owner");
                clearFrames();
                decoded.iRequestSequence = 2u; decoded.iGold = 999999u; decoded.Items.clear();
                bern->Handle_RestoreCharacter(peer->Get_SessionId(), decoded);
                tests.Require(hasRestoreResult(peer, CHARACTER_RESTORE_RESULT::REJECTED_UNAVAILABLE, 2u) &&
                    sameItems(player.Inventory, saved.Items) && player.Purse.iGold == saved.iGold &&
                    player.iHonorTitleId == saved.iHonorTitleId,
                    "Repeated restore rejects changed content without overwriting the active character");
                player.isCombatReady = true;
                bern->m_PartyIdByPlayerId[player.iPlayerId] = 1u;
                bern->m_PartyMembersByPartyId[1u].push_back(player.iPlayerId);
            }
            constexpr const char* itemIds[] = {"BATTLE_DESTRUCTION_BOMB", "BATTLE_WHIRLWIND_GRENADE",
                "BATTLE_HOLY_CHARM", "BATTLE_TIME_STOP_POTION"};
            for (const auto& peer : peers)
            {
                auto& player = bern->m_Players.at(peer->Get_PlayerId());
                unsigned sequence = 10u;
                for (const auto* itemId : itemIds)
                {
                    clearFrames();
                    C2S_DEBUG_GIVE_ITEM request{sequence++, itemId, 3u};
                    CPacketWriter writer; C2S_DEBUG_GIVE_ITEM decoded;
                    const bool encoded = Write_Message(writer, request);
                    CPacketReader packet{writer.Get_Buffer()};
                    const bool validWire = encoded && Read_Message(packet, decoded);
                    if (validWire) bern->Handle_DebugGiveItem(peer->Get_SessionId(), decoded);
                    bool delivered = false;
                    for (const auto& frame : peer->m_OutboundFrames)
                        if (frame.ePacketType == PACKET_TYPE::S2C_INVENTORY_SNAPSHOT)
                        {
                            CPacketReader reader{std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES)};
                            S2C_INVENTORY_SNAPSHOT snapshot;
                            delivered = Read_Message(reader, snapshot) && snapshot.iRequestSequence == request.iRequestSequence &&
                                sameItems(snapshot.Items, player.Inventory) && snapshot.iGold == player.Purse.iGold &&
                                std::any_of(snapshot.Items.begin(), snapshot.Items.end(), [&](const auto& item) {
                                    return item.strItemId == itemId && item.iQuantity == 3u; });
                        }
                    tests.Require(validWire && delivered && std::all_of(peers.begin(), peers.end(), [&](const auto& other) {
                        return other == peer || other->m_OutboundFrames.empty(); }),
                        "F1 typed grant returns the correct private inventory for each of four clients");
                }
            }
            bern->m_iServerTick = 100u;
            for (unsigned kind = 0; kind < 4u; ++kind)
                for (unsigned index = 0; index < peers.size(); ++index)
                {
                    auto& player = bern->m_Players.at(peers[index]->Get_PlayerId());
                    C2S_USE_ITEM request;
                    request.iRequestSequence = 100u + kind;
                    request.strItemId = itemIds[kind];
                    request.hasGroundTarget = kind < 2u;
                    if (request.hasGroundTarget)
                    { request.fTargetX = player.fPositionX; request.fTargetZ = player.fPositionZ; }
                    if (kind == 2u)
                    {
                        auto& target = bern->m_Players.at(peers[(index + 1u) % peers.size()]->Get_PlayerId());
                        target.fPositionX = player.fPositionX; target.fPositionY = player.fPositionY;
                        target.fPositionZ = player.fPositionZ;
                        target.eAction = PLAYER_ACTION_STATE::FEAR; target.iFearEndTick = 500u;
                        request.iTargetPlayerNetEntityId = target.iNetEntityId;
                    }
                    clearFrames();
                    bern->Handle_UseItem(peers[index]->Get_SessionId(), request);
                    const auto remaining = std::find_if(player.Inventory.begin(), player.Inventory.end(),
                        [&](const auto& item) { return item.strItemId == request.strItemId; });
                    tests.Require(remaining != player.Inventory.end() && remaining->iQuantity == 2u &&
                        (kind != 3u || player.Has_TimeStop(101u)),
                        "Every admitted client can use each F1-granted battle item exactly once");
                    bern->Handle_UseItem(peers[index]->Get_SessionId(), request);
                    tests.Require(remaining != player.Inventory.end() && remaining->iQuantity == 2u,
                        "Repeated item sequence preserves all four clients' consumed quantities");
                }
            // The protocol harness covers avatar wire fields; this block covers the
            // actual shop -> inventory -> equipment -> replicated observer path.
            const auto avatarMerchant = std::find_if(bern->m_WorldEntities.begin(), bern->m_WorldEntities.end(),
                [](const auto& entity) { return entity.eKind == WORLD_BOOTSTRAP_KIND::NPC &&
                    entity.strPlacementId == "npc.bern.plaza.17"; });
            tests.Require(avatarMerchant != bern->m_WorldEntities.end(), "Published Bern avatar merchant is available");
            if (avatarMerchant != bern->m_WorldEntities.end())
            {
                auto& avatarPlayer = bern->m_Players.at(peers.front()->Get_PlayerId());
                const auto beforeAvatar = avatarPlayer;
                struct AVATAR_CASE { CHARACTER_CLASS_ID eClass; const char* prefix; };
                const AVATAR_CASE avatarCases[] = {
                    {CHARACTER_CLASS_ID::LANCE_MASTER, "LANCEMASTER"},
                    {CHARACTER_CLASS_ID::WARLORD, "WARLORD"},
                    {CHARACTER_CLASS_ID::ARTIST, "ARTIST"},
                    {CHARACTER_CLASS_ID::DIMENSIONMASTER, "DIMENSIONMASTER"},
                    {CHARACTER_CLASS_ID::GUARDIANKNIGHT, "GUARDIANKNIGHT"},
                    {CHARACTER_CLASS_ID::GUNSLINGER, nullptr},
                    {CHARACTER_CLASS_ID::SLAYER, nullptr}};
                C2S_BUY_ITEMS basket;
                basket.iRequestSequence = 1000u;
                basket.strNpcPlacementId = "npc.bern.plaza.17";
                basket.Entries = {{"AVATAR_MOKOKO_036_HEAD", 1u}, {"AVATAR_MOKOKO_036_OUTFIT", 1u}};
                const auto atSlot = [&avatarPlayer](const EQUIPMENT_SLOT slot, const std::string& id) {
                    return std::any_of(avatarPlayer.Inventory.begin(), avatarPlayer.Inventory.end(),
                        [&](const auto& item) { return item.eEquippedSlot == slot &&
                            item.strItemId == id && item.iQuantity == 1u; });
                };
                const auto privateInventoryMatches = [&](const unsigned sequence) {
                    bool found = false;
                    for (const auto& frame : peers.front()->m_OutboundFrames)
                        if (frame.ePacketType == PACKET_TYPE::S2C_INVENTORY_SNAPSHOT)
                        {
                            CPacketReader reader{std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES)};
                            S2C_INVENTORY_SNAPSHOT snapshot;
                            found = Read_Message(reader, snapshot) && snapshot.iRequestSequence == sequence &&
                                sameItems(snapshot.Items, avatarPlayer.Inventory) &&
                                snapshot.iGold == avatarPlayer.Purse.iGold && snapshot.iSilver == avatarPlayer.Purse.iSilver;
                        }
                    return found && std::all_of(peers.begin() + 1, peers.end(),
                        [](const auto& peer) { return peer->m_OutboundFrames.empty(); });
                };
                for (const auto& avatarCase : avatarCases)
                {
                    avatarPlayer = beforeAvatar;
                    avatarPlayer.eCharacterClass = avatarCase.eClass;
                    avatarPlayer.fPositionX = avatarMerchant->fPositionX;
                    avatarPlayer.fPositionZ = avatarMerchant->fPositionZ;
                    avatarPlayer.Inventory.clear();
                    avatarPlayer.Purse.iGold = 100u; avatarPlayer.Purse.iSilver = 77u;
                    avatarPlayer.bRestoreAvailable = true;
                    clearFrames(); ++basket.iRequestSequence;
                    bern->Handle_BuyItems(peers.front()->Get_SessionId(), basket);
                    bool correct = privateInventoryMatches(basket.iRequestSequence) && avatarPlayer.Purse.iSilver == 77u;
                    if (avatarCase.prefix != nullptr)
                    {
                        const std::string base = std::string("AVATAR_") + avatarCase.prefix + "_MOKOKO_036_";
                        correct &= avatarPlayer.Inventory.size() == 2u && avatarPlayer.Purse.iGold == 80u &&
                            !avatarPlayer.bRestoreAvailable && atSlot(EQUIPMENT_SLOT::NONE, base + "HEAD") &&
                            atSlot(EQUIPMENT_SLOT::NONE, base + "OUTFIT");
                    }
                    else
                        correct &= avatarPlayer.Inventory.empty() && avatarPlayer.Purse.iGold == 100u &&
                            avatarPlayer.bRestoreAvailable;
                    tests.Require(correct, avatarCase.prefix != nullptr ?
                        "Avatar shop grants both exact class variants privately and charges the template prices" :
                        "Unsupported avatar classes reject the entire basket without consuming currency or restore state");
                }
                avatarPlayer = beforeAvatar;
                avatarPlayer.Inventory.clear(); avatarPlayer.Purse.iGold = 100u;
                avatarPlayer.fPositionX = avatarMerchant->fPositionX;
                avatarPlayer.fPositionZ = avatarMerchant->fPositionZ;
                bool normalEquipped = true;
                for (const auto slot : {EQUIPMENT_SLOT::HELMET, EQUIPMENT_SLOT::TOP})
                {
                    const std::string id = slot == EQUIPMENT_SLOT::HELMET ?
                        "EQUIP_WARLORD_HONORWHISPER_HELMET" : "EQUIP_WARLORD_HONORWHISPER_TOP";
                    C2S_SET_EQUIPMENT equip{1100u, slot, true, id};
                    normalEquipped &= bern->Grant_Item(avatarPlayer, id, 1u) && bern->Apply_SetEquipment(avatarPlayer, equip);
                }
                tests.Require(normalEquipped, "Normal helmet and top use the real equipment path before avatar dressing");
                clearFrames(); ++basket.iRequestSequence;
                bern->Handle_BuyItems(peers.front()->Get_SessionId(), basket);
                tests.Require(privateInventoryMatches(basket.iRequestSequence) && avatarPlayer.Inventory.size() == 4u,
                    "Avatar purchase adds its two variants beside existing normal equipment");
                C2S_SET_EQUIPMENT head{1101u, EQUIPMENT_SLOT::AVATAR_HEAD, true, "AVATAR_WARLORD_MOKOKO_036_HEAD"};
                C2S_SET_EQUIPMENT outfit{1102u, EQUIPMENT_SLOT::AVATAR_OUTFIT, true, "AVATAR_WARLORD_MOKOKO_036_OUTFIT"};
                for (const auto& equip : {head, outfit})
                {
                    clearFrames(); bern->Handle_SetEquipment(peers.front()->Get_SessionId(), equip);
                    tests.Require(privateInventoryMatches(equip.iRequestSequence) && atSlot(equip.eSlot, equip.strItemId),
                        "Avatar equipment handler commits the requested class item to its separate slot");
                }
                const auto normalPreserved = [&]() {
                    return atSlot(EQUIPMENT_SLOT::HELMET, "EQUIP_WARLORD_HONORWHISPER_HELMET") &&
                        atSlot(EQUIPMENT_SLOT::TOP, "EQUIP_WARLORD_HONORWHISPER_TOP");
                };
                tests.Require(normalPreserved(), "Avatar head and outfit leave normal helmet and top equipped");
                const bool foreignGranted = bern->Grant_Item(avatarPlayer, "AVATAR_LANCEMASTER_MOKOKO_036_HEAD", 1u);
                const auto beforeRejectedEquip = avatarPlayer.Inventory;
                C2S_SET_EQUIPMENT foreign{1103u, EQUIPMENT_SLOT::AVATAR_HEAD, true, "AVATAR_LANCEMASTER_MOKOKO_036_HEAD"};
                tests.Require(foreignGranted && !bern->Apply_SetEquipment(avatarPlayer, foreign) &&
                    sameItems(beforeRejectedEquip, avatarPlayer.Inventory),
                    "A wrong-class avatar in the bag cannot replace the currently worn head");
                const auto allObserversMatch = [&](const std::string& expectedHead) {
                    clearFrames(); bern->Broadcast_WorldSnapshot();
                    return std::all_of(peers.begin(), peers.end(), [&](const auto& peer) {
                        bool found = false;
                        for (const auto& frame : peer->m_OutboundFrames)
                            if (frame.ePacketType == PACKET_TYPE::S2C_WORLD_SNAPSHOT)
                            {
                                CPacketReader reader{std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES)};
                                S2C_WORLD_SNAPSHOT snapshot;
                                if (!Read_Message(reader, snapshot)) return false;
                                const auto player = std::find_if(snapshot.Players.begin(), snapshot.Players.end(),
                                    [&](const auto& value) { return value.iNetEntityId == avatarPlayer.iNetEntityId; });
                                found = player != snapshot.Players.end() && player->strAvatarHeadItemId == expectedHead &&
                                    player->strAvatarOutfitItemId == outfit.strItemId;
                            }
                        return found;
                    });
                };
                tests.Require(allObserversMatch(head.strItemId),
                    "All four world observers receive both equipped avatar IDs on the owner's exact entity");
                C2S_SET_EQUIPMENT removeHead{1104u, EQUIPMENT_SLOT::AVATAR_HEAD, false, {}};
                clearFrames(); bern->Handle_SetEquipment(peers.front()->Get_SessionId(), removeHead);
                tests.Require(privateInventoryMatches(removeHead.iRequestSequence) &&
                    atSlot(EQUIPMENT_SLOT::NONE, head.strItemId) && atSlot(EQUIPMENT_SLOT::AVATAR_OUTFIT, outfit.strItemId) &&
                    normalPreserved() && allObserversMatch({}),
                    "Removing the avatar head clears only its replicated ID while outfit and normal equipment remain");
                avatarPlayer = beforeAvatar;
                clearFrames();
            }
        }
        for (const auto& peer : peers) peer->Request_Close();
        // Catalog-only errors pass the wire contract, then preserve fresh-entry state.
        auto rejected = std::make_unique<CGameRoom>(WORLD_ID::BERN);
        for (unsigned index = 0; index < 4u; ++index)
        {
            auto peer = std::make_shared<CClientSession>(94000u + index, INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
            peer->m_isSendRunning.store(true); rejected->Handle_Register(peer);
            C2S_ENTER_WORLD entry; entry.eWorldId = WORLD_ID::BERN;
            entry.eCharacterClass = CHARACTER_CLASS_ID::WARLORD; entry.strNickName = "RestoreRejected";
            const bool joined = rejected->Join(peer->Get_SessionId(), entry);
            tests.Require(joined, "Restore rejection fixture admits a fresh real Bern character");
            if (!joined) { peer->Request_Close(); continue; }
            auto& player = rejected->m_Players.at(peer->Get_PlayerId());
            const auto before = player.Inventory; const auto purse = player.Purse; const auto title = player.iHonorTitleId;
            C2S_RESTORE_CHARACTER invalid;
            invalid.iRequestSequence = 1u; invalid.iGold = 777u;
            if (index == 0u) invalid.Items = {{"UNKNOWN_CATALOG_ITEM", 1u, EQUIPMENT_SLOT::NONE}};
            if (index == 1u) invalid.Items = {{"POTION_HP_SMALL",
                rejected->m_ItemCatalog.Find_Item("POTION_HP_SMALL")->iMaxStack + 1u, EQUIPMENT_SLOT::NONE}};
            if (index == 2u) invalid.Items = {{"EQUIP_DESTINYBLAZE_WEAPON", 1u, EQUIPMENT_SLOT::WEAPON}};
            if (index == 3u) invalid.iHonorTitleId = 999999u;
            peer->m_OutboundFrames.clear(); peer->m_iQueuedOutboundBytes = 0u;
            CPacketWriter writer; C2S_RESTORE_CHARACTER decoded;
            const bool encoded = Write_Message(writer, invalid);
            CPacketReader reader{writer.Get_Buffer()};
            const bool validWire = encoded && Read_Message(reader, decoded);
            if (validWire) rejected->Handle_RestoreCharacter(peer->Get_SessionId(), decoded);
            tests.Require(validWire && hasRestoreResult(peer, CHARACTER_RESTORE_RESULT::REJECTED_CATALOG, 1u) &&
                sameItems(before, player.Inventory) && player.Purse.iSilver == purse.iSilver &&
                player.Purse.iGold == purse.iGold && player.iHonorTitleId == title && !player.bRestoreAvailable,
                "Unknown item, oversized stack, wrong-class equipment or title preserves existing character state");
            invalid.iRequestSequence = 2u; invalid.Items.clear(); invalid.iHonorTitleId = INVALID_HONOR_TITLE_ID;
            rejected->Handle_RestoreCharacter(peer->Get_SessionId(), invalid);
            tests.Require(hasRestoreResult(peer, CHARACTER_RESTORE_RESULT::REJECTED_UNAVAILABLE, 2u) &&
                sameItems(before, player.Inventory) && player.Purse.iGold == purse.iGold,
                "A rejected restore cannot be retried to replace the live character with different content");
            peer->Request_Close();
        }
    }
    auto room = std::make_unique<CGameRoom>(WORLD_ID::BERN);
    tests.Require(room->Is_Ready(), "Battle items load published catalogs in a real room");
    if (!room->Is_Ready()) { std::cout << room->Get_Status() << '\n'; return 1; }
    std::vector<std::shared_ptr<CClientSession>> sessions;
    for (unsigned id = 1; id <= 3; ++id)
    {
        auto& player = room->m_Players[id];
        player.iPlayerId = id; player.iNetEntityId = 100u + id; player.iSessionId = 9000u + id;
        player.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
        player.iCurrentHp = player.iMaximumHp = 10000u;
        player.isCombatReady = true; player.fPositionX = static_cast<float>(id);
        room->m_PlayerIdBySessionId[player.iSessionId] = id;
        auto session = std::make_shared<CClientSession>(player.iSessionId, INVALID_SOCKET,
            CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
        session->m_isSendRunning.store(true);
        room->m_Sessions[player.iSessionId] = session;
        sessions.push_back(std::move(session));
    }
    room->m_PartyIdByPlayerId[1u] = room->m_PartyIdByPlayerId[2u] = 1u;
    room->m_PartyMembersByPartyId[1u] = {1u, 2u};
    auto& caster = room->m_Players.at(1u);
    auto& ally = room->m_Players.at(2u);
    for (const char* id : {"BATTLE_DESTRUCTION_BOMB", "BATTLE_WHIRLWIND_GRENADE", "BATTLE_HOLY_CHARM", "BATTLE_TIME_STOP_POTION"})
        tests.Require(room->Grant_Item(caster, id, 5u), "Every battle item can enter the authoritative inventory");
    const auto quantity = [&](const char* id) {
        const auto found = std::find_if(caster.Inventory.begin(), caster.Inventory.end(),
            [&](const auto& item) { return item.strItemId == id; });
        return found == caster.Inventory.end() ? 0u : found->iQuantity;
    };
    room->m_iServerTick = 100u;
    C2S_USE_ITEM charm; charm.iRequestSequence = 1u; charm.strItemId = "BATTLE_HOLY_CHARM";
    charm.iTargetPlayerNetEntityId = 103u;
    room->Handle_UseItem(caster.iSessionId, charm);
    tests.Require(quantity("BATTLE_HOLY_CHARM") == 5u, "Holy charm rejects another party without consuming an item");
    ally.eAction = PLAYER_ACTION_STATE::FEAR; ally.iFearEndTick = 500u;
    charm.iRequestSequence = 2u; charm.iTargetPlayerNetEntityId = ally.iNetEntityId;
    room->Handle_UseItem(caster.iSessionId, charm);
    tests.Require(quantity("BATTLE_HOLY_CHARM") == 4u && ally.eAction != PLAYER_ACTION_STATE::FEAR &&
        ally.Has_HolyCharmProtection(101u) && !ally.Has_HolyCharmProtection(191u),
        "Holy charm immediately cleanses its chosen ally and grants exactly three seconds");
    room->Handle_UseItem(caster.iSessionId, charm);
    ++charm.iRequestSequence;
    room->Handle_UseItem(caster.iSessionId, charm);
    tests.Require(quantity("BATTLE_HOLY_CHARM") == 4u, "Duplicate and cooldown uses cannot spend twice");

    std::vector<DAMAGE_EVENT> damageEvents;
    SERVER_WORLD_TO_PLAYER_HIT lethal;
    lethal.iRawDamage = 100000u; lethal.iServerTick = 102u;
    lethal.bIgnoreDefense = lethal.bIgnoreCounter = true;
    (void)CServerCombatHitRuntime::Apply_WorldToPlayer(ally, lethal, room->m_GameplayCatalog, damageEvents);
    tests.Require(ally.iCurrentHp == 10000u && damageEvents.empty(), "Holy protection blocks lethal collision damage");
    auto expiredAlly = ally; lethal.iServerTick = 191u;
    (void)CServerCombatHitRuntime::Apply_WorldToPlayer(expiredAlly, lethal, room->m_GameplayCatalog, damageEvents);
    tests.Require(!expiredAlly.iCurrentHp, "Holy protection ends at the authoritative expiry tick");

    C2S_USE_ITEM stop; stop.iRequestSequence = 4u; stop.strItemId = "BATTLE_TIME_STOP_POTION";
    room->Handle_UseItem(caster.iSessionId, stop);
    tests.Require(quantity("BATTLE_TIME_STOP_POTION") == 4u && caster.Has_TimeStop(101u) && !caster.Has_TimeStop(191u),
        "Time stop consumes one item and protects only its caster for three seconds");
    damageEvents.clear(); lethal.iServerTick = 102u;
    const auto avoided = CServerCombatHitRuntime::Apply_WorldToPlayer(caster, lethal, room->m_GameplayCatalog, damageEvents);
    tests.Require(avoided == SERVER_COMBAT_HIT_RESULT::NOT_ADMITTED && caster.iCurrentHp == 10000u && damageEvents.empty(),
        "Time stop removes its caster from lethal collision hit admission");
    auto stoppedWipe = caster; auto protectedWipe = ally; lethal.bEncounterWipe = true;
    (void)CServerCombatHitRuntime::Apply_WorldToPlayer(stoppedWipe, lethal, room->m_GameplayCatalog, damageEvents);
    (void)CServerCombatHitRuntime::Apply_WorldToPlayer(protectedWipe, lethal, room->m_GameplayCatalog, damageEvents);
    tests.Require(!stoppedWipe.iCurrentHp && !protectedWipe.iCurrentHp,
        "Explicit encounter failure wipes bypass both battle-item protections");

    const auto* whirlwind = room->m_ItemCatalog.Find_Item("BATTLE_WHIRLWIND_GRENADE");
    tests.Require(whirlwind && whirlwind->BattleUse.iDamageRatePercent == 0u,
        "Whirlwind catalog gives no HP damage");
    if (whirlwind)
    {
        caster.iTimeStopEndTick = 0u;
        caster.fPositionX = caster.fPositionY = caster.fPositionZ = 0.f;
        auto entities = std::make_unique<std::vector<SERVER_WORLD_ENTITY>>(1u);
        auto& boss = entities->front();
        boss.iNetEntityId = 500u; boss.eKind = WORLD_BOOTSTRAP_KIND::BOSS;
        boss.strArchetypeId = "BOSS_VALTAN";
        boss.iCurrentHp = boss.iMaximumHp = 100000u;
        boss.fPositionZ = 2.f; boss.fCollisionRadius = 1.f;
        boss.BossCombat.iStaggerMaximum = 1000u;
        CCombatObjectRuntime runtime;
        for (unsigned throwIndex = 0; throwIndex < 3u; ++throwIndex)
        {
            auto transaction = runtime.Begin_Transaction(); std::string status;
            const auto tick = 200u + throwIndex * 40u;
            const bool staged = runtime.Stage_BattleItemProjectile(transaction, caster, whirlwind->BattleUse,
                0.f, 0.f, 4.f, room->m_GameplayCatalog, tick, status);
            tests.Require(staged && runtime.Commit(std::move(transaction)), "Whirlwind stages a real replicated projectile");
            std::vector<S2C_COMBAT_OBJECT_SPAWNED> spawned;
            std::vector<S2C_COMBAT_OBJECT_PRESENTATION_EVENT> pulses;
            std::vector<S2C_COMBAT_OBJECT_DESPAWNED> despawned;
            runtime.Drain_Lifecycle(spawned, pulses, despawned);
            tests.Require(spawned.size() == 1u && spawned.front().strClientVisualId == "battle.item.whirlwind_grenade",
                "Observers receive the canonical flight identity");
            const auto previous = boss.BossCombat.iStaggerCurrent;
            damageEvents.clear();
            for (unsigned step = 1; step <= 35u && !runtime.Get_LiveObjects().empty(); ++step)
                runtime.Update(room->m_Players, *entities, room->m_GameplayCatalog, 1.f / 30.f, tick + step, damageEvents);
            runtime.Drain_Lifecycle(spawned, pulses, despawned);
            tests.Require(runtime.Get_LiveObjects().empty() && pulses.size() == 1u && despawned.size() == 1u &&
                pulses.front().strHitId == "battle.item.impact", "Bomb contact emits one impact then removes its projectile");
            tests.Require(boss.iCurrentHp == 100000u && boss.BossCombat.iStaggerCurrent == (std::min)(1000u, previous + 334u),
                "Each whirlwind contributes one third of maximum stagger with zero HP loss");
            tests.Require(std::all_of(damageEvents.begin(), damageEvents.end(), [](const auto& event) { return event.iAmount == 0u; }),
                "Whirlwind combat events never report fabricated HP damage");
        }
        tests.Require(boss.BossCombat.iStaggerCurrent == 1000u, "Three whirlwinds fill a non-divisible stagger maximum exactly");
    }
    {
        auto boss = std::make_unique<SERVER_WORLD_ENTITY>();
        boss->iNetEntityId = 600u; boss->eKind = WORLD_BOOTSTRAP_KIND::BOSS;
        boss->strArchetypeId = "BOSS_KAKULSAYDON_G1_SAYDON";
        boss->iCurrentHp = boss->iMaximumHp = 100000u;
        boss->strPatternId = "battle.item.stagger.window"; boss->iPatternSequence = 1u;
        BOSS_PATTERN_DEFINITION pattern; pattern.strPatternId = boss->strPatternId;
        BOSS_PATTERN_LOGIC_WINDOW window;
        window.strWindowId = "stagger"; window.eKind = BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW;
        const auto commonMaximum = room->m_GameplayCatalog.Active().Get_RaidStaggerMaximum();
        window.iDurationMs = 5000u;
        window.iThreshold = commonMaximum == 1u ? 2u : 1u; // Prove the active policy overrides a different authored fallback.
        window.bEndsPatternOnSuccess = true;
        pattern.LogicWindows.push_back(window);
        KOUKUSAYDON_LOGIC_LEDGER ledger; KOUKUSAYDON_LOGIC_OUTPUT output;
        CKoukuSaydonLogicRuntime::Build(pattern, *boss, 500u, ledger);
        CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, room->m_Players,
            room->m_GameplayCatalog, nullptr, 500u, damageEvents, output);
        const auto third = commonMaximum / 3u + (commonMaximum % 3u != 0u ? 1u : 0u);
        const auto* grenade = room->m_ItemCatalog.Find_Item("BATTLE_WHIRLWIND_GRENADE");
        BOSS_COMBAT_SNAPSHOT openedGauge;
        CKoukuSaydonLogicRuntime::Project_MechanicGauge(*boss, pattern, ledger, 500u, openedGauge);
        tests.Require(commonMaximum != 0u && boss->iKoukuItemStaggerMaximum == commonMaximum &&
            openedGauge.iMaximumMechanicGauge == commonMaximum && openedGauge.iCurrentMechanicGauge == commonMaximum &&
            grenade != nullptr && grenade->BattleUse.iStaggerMaximumDivisor == 3u,
            "Kouku stagger window consumes the active common policy and the published whirlwind grants one third");
        SERVER_PLAYER_TO_WORLD_HIT hit;
        hit.iSourcePlayerId = caster.iPlayerId;
        hit.iSkillId = grenade != nullptr ? grenade->BattleUse.iSkillId : 0u;
        hit.iStaggerMaximumDivisor = grenade != nullptr ? grenade->BattleUse.iStaggerMaximumDivisor : 0u;
        for (unsigned count = 1u; count <= 3u; ++count)
        {
            hit.iServerTick = 500u + count;
            (void)CServerCombatHitRuntime::Apply_PlayerToWorld(*boss, hit, damageEvents);
            BOSS_COMBAT_SNAPSHOT gauge;
            CKoukuSaydonLogicRuntime::Project_MechanicGauge(*boss, pattern, ledger, hit.iServerTick, gauge);
            tests.Require(boss->iCurrentHp == 100000u && gauge.iMaximumMechanicGauge == commonMaximum &&
                gauge.iCurrentMechanicGauge == commonMaximum - (std::min)(commonMaximum, count * third),
                "Kouku HUD decreases by a maximum-third while boss HP stays unchanged");
            CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, room->m_Players,
                room->m_GameplayCatalog, nullptr, hit.iServerTick, damageEvents, output);
        }
        tests.Require(ledger.Windows.front().bClosed && output.bStaggerSuccess && output.bEndPatternEarly &&
            boss->iCurrentHp == 100000u && !boss->iKoukuItemStaggerMaximum && !boss->iKoukuItemStaggerCredit,
            "Third whirlwind completes the actual Kouku window and clears only that window's credit");
        ++boss->iPatternSequence;
        CKoukuSaydonLogicRuntime::Build(pattern, *boss, 700u, ledger);
        CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, room->m_Players,
            room->m_GameplayCatalog, nullptr, 700u, damageEvents, output);
        auto skill = hit; skill.iStaggerMaximumDivisor = 0u; skill.iRawDamage = 100u; skill.iServerTick = 701u;
        (void)CServerCombatHitRuntime::Apply_PlayerToWorld(*boss, skill, damageEvents);
        hit.iServerTick = 702u;
        (void)CServerCombatHitRuntime::Apply_PlayerToWorld(*boss, hit, damageEvents);
        BOSS_COMBAT_SNAPSHOT mixed;
        CKoukuSaydonLogicRuntime::Project_MechanicGauge(*boss, pattern, ledger, 702u, mixed);
        tests.Require(boss->iCurrentHp == 99900u && mixed.iCurrentMechanicGauge == commonMaximum - third,
            "Sub-1000 ordinary HP damage does not become stagger and whirlwind credit remains independent");
        BOSS_PATTERN_LOGIC_RESULT fear;
        fear.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::FEAR; fear.iDurationMs = 3000u;
        fear.strFearPresentationId = "battle.item.fear";
        ally.iHolyCharmProtectionEndTick = 900u; ally.eAction = PLAYER_ACTION_STATE::NONE;
        CKoukuSaydonLogicRuntime::Apply_Result(ally, fear, *boss, room->m_GameplayCatalog, nullptr, 800u, damageEvents);
        tests.Require(ally.eAction != PLAYER_ACTION_STATE::FEAR, "Holy charm blocks new Kouku fear during protection");
        CKoukuSaydonLogicRuntime::Apply_Result(ally, fear, *boss, room->m_GameplayCatalog, nullptr, 900u, damageEvents);
        tests.Require(ally.eAction == PLAYER_ACTION_STATE::FEAR, "Kouku fear resumes exactly after holy protection expires");
    }
    {
        // Exercise the real use command, trajectory, part authority and four observer
        // packets. Only the current recovery window and cooldown clock are fixture inputs.
        auto arena = std::make_unique<CGameRoom>(WORLD_ID::VALTAN_ARENA);
        std::vector<std::shared_ptr<CClientSession>> observers;
        bool admitted = arena->Is_Ready();
        for (unsigned index = 0; index < 4u && admitted; ++index)
        {
            auto session = std::make_shared<CClientSession>(91000u + index, INVALID_SOCKET,
                CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
            session->m_isSendRunning.store(true);
            arena->Handle_Register(session);
            observers.push_back(session);
            C2S_ENTER_WORLD entry;
            entry.eWorldId = WORLD_ID::VALTAN_ARENA;
            entry.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
            entry.strNickName = "BombArmor" + std::to_string(index + 1u);
            admitted = arena->Join(session->Get_SessionId(), entry);
        }
        const auto* placement = arena->Find_Placement("boss.valtan.center");
        const auto* patterns = arena->m_GameplayCatalog.Find_BossPatterns("ENCOUNTER_VALTAN");
        const BOSS_PATTERN_STAGE_DEFINITION* recovery = nullptr;
        const BOSS_PATTERN_STAGE_DEFINITION* charge = nullptr;
        std::uint32_t recoveryIndex = 0u, chargeIndex = 0u;
        if (patterns) for (const auto& pattern : *patterns)
            if (pattern.strPatternId == "VALTAN_DASH_CHARGE")
                for (std::size_t index = 0u; index < pattern.Stages.size(); ++index)
                {
                    if (pattern.Stages[index].strActionId == "valtan.attack.dash-charge.recovery")
                    { recovery = &pattern.Stages[index]; recoveryIndex = static_cast<std::uint32_t>(index); }
                    if (pattern.Stages[index].bChargeImpact)
                    { charge = &pattern.Stages[index]; chargeIndex = static_cast<std::uint32_t>(index); }
                }
        auto stagedBoss = std::make_unique<SERVER_WORLD_ENTITY>();
        const bool built = admitted && placement && recovery && charge &&
            arena->Build_WorldEntity(*placement, 500001u, *stagedBoss);
        tests.Require(built && arena->Count_HumanPlayers() == 4u &&
            stagedBoss->BossCombat.iAlivePartMask == 3u && stagedBoss->ArmorPlates.size() == 2u &&
            recovery->ePartDamagePolicy == BOSS_PATTERN_PART_DAMAGE_POLICY::DESTROY_FIRST_ELIGIBLE,
            "Four admitted players use the published Valtan parts and dash recovery policy");
        if (built)
        {
            arena->m_WorldEntities.clear();
            arena->m_WorldEntities.push_back(std::move(*stagedBoss));
            auto& boss = arena->m_WorldEntities.front();
            auto& thrower = arena->m_Players.at(observers.front()->Get_PlayerId());
            SERVER_NAV_POINT source;
            const bool positioned = arena->m_ServerNavigation.Sample_SurfacePosition(
                boss.fPositionX, boss.fPositionZ + 4.f, source);
            thrower.fPositionX = source.x; thrower.fPositionY = source.y; thrower.fPositionZ = source.z;
            thrower.isCombatReady = true;
            tests.Require(positioned && arena->Grant_Item(thrower, "BATTLE_DESTRUCTION_BOMB", 5u),
                "Destruction bombs enter a real inventory at a navigable throwing position");
            const auto remaining = [&]() {
                const auto item = std::find_if(thrower.Inventory.begin(), thrower.Inventory.end(),
                    [](const auto& row) { return row.strItemId == "BATTLE_DESTRUCTION_BOMB"; });
                return item == thrower.Inventory.end() ? 0u : item->iQuantity;
            };
            const auto initialQuantity = remaining();
            std::uint32_t sequence = 1u;
            for (unsigned attempt = 0u; attempt < 4u && positioned; ++attempt)
            {
                for (const auto& observer : observers)
                { observer->m_OutboundFrames.clear(); observer->m_iQueuedOutboundBytes = 0u; }
                arena->m_iServerTick = 100u + attempt * 1000u;
                arena->m_TickBossCombatEvents.clear();
                boss.BossCombat.PendingOutcomes.clear();
                boss.bPendingArmorBreakReaction = false;
                boss.strPatternId = "VALTAN_DASH_CHARGE";
                boss.strPatternStageId = recovery->strStageId;
                boss.strActionId = recovery->strActionId;
                boss.iPatternSequence = attempt + 1u;
                boss.iPatternStageIndex = recoveryIndex;
                boss.iPatternStageDurationMs = recovery->iDurationMs;
                boss.iActionStartTick = arena->m_iServerTick;
                boss.eAction = SERVER_ENTITY_ACTION::PATTERN_RECOVERY;
                boss.ePatternPartDamagePolicy = recovery->ePartDamagePolicy;
                boss.bPatternGroggy = attempt != 0u;
                (void)CBossCombatRuntime::Set_Flag(boss.BossCombat, SERVER_BOSS_COMBAT_FLAG::GROGGY, attempt != 0u);
                const auto beforeMask = boss.BossCombat.iAlivePartMask;
                {
                    auto wallBoss = std::make_unique<SERVER_WORLD_ENTITY>(boss);
                    wallBoss->strPatternStageId = charge->strStageId;
                    wallBoss->strActionId = charge->strActionId;
                    wallBoss->iPatternStageIndex = chargeIndex;
                    wallBoss->bPatternChargeImpact = true;
                    const bool enteredGroggy = CValtanBrain{}.Complete_ImpactStage(
                        *wallBoss, arena->m_GameplayCatalog, arena->m_iServerTick);
                    tests.Require(enteredGroggy && wallBoss->strActionId == recovery->strActionId &&
                        wallBoss->BossCombat.iAlivePartMask == beforeMask &&
                        wallBoss->BossCombat.PendingPartBreakEdges.empty() &&
                        !wallBoss->bPendingArmorBreakReaction &&
                        std::equal(boss.BossCombat.Parts.begin(), boss.BossCombat.Parts.end(), wallBoss->BossCombat.Parts.begin(),
                            [](const auto& left, const auto& right) { return left.iCurrentDurability == right.iCurrentDurability; }) &&
                        std::equal(boss.ArmorPlates.begin(), boss.ArmorPlates.end(), wallBoss->ArmorPlates.begin(),
                            [](const auto& left, const auto& right) { return left.iRemainingDurability == right.iRemainingDurability; }),
                        "Repeated authoritative wall impacts open DASH groggy without removing or restoring armor");
                }
                if (attempt == 1u)
                {
                    const auto partsBeforeSkills = boss.BossCombat.Parts;
                    const auto armorBeforeSkills = boss.ArmorPlates;
                    const auto hpBeforeSkills = boss.iCurrentHp;
                    SERVER_PLAYER_TO_WORLD_HIT ordinary;
                    ordinary.iSourcePlayerId = thrower.iPlayerId;
                    ordinary.iRawDamage = 100u; ordinary.iPartDamage = 1000000u;
                    ordinary.iServerTick = arena->m_iServerTick; ordinary.bStaggerDisabled = true;
                    // Even the item's skill number carries no authority without its server projectile.
                    for (const auto skillId : { 34010u, 34040u, 32141u })
                    {
                        ordinary.iSkillId = skillId;
                        (void)CServerCombatHitRuntime::Apply_PlayerToWorld(boss, ordinary, damageEvents);
                    }
                    tests.Require(boss.iCurrentHp < hpBeforeSkills && boss.BossCombat.iAlivePartMask == beforeMask &&
                        boss.BossCombat.PendingPartBreakEdges.empty() && boss.BossCombat.PendingOutcomes.empty() &&
                        !boss.bPendingArmorBreakReaction && boss.bPatternGroggy &&
                        std::equal(partsBeforeSkills.begin(), partsBeforeSkills.end(), boss.BossCombat.Parts.begin(),
                            [](const auto& left, const auto& right) { return left.iCurrentDurability == right.iCurrentDurability; }) &&
                        std::equal(armorBeforeSkills.begin(), armorBeforeSkills.end(), boss.ArmorPlates.begin(),
                            [](const auto& left, const auto& right) { return left.iRemainingDurability == right.iRemainingDurability; }),
                        "Normal attacks, skills and a bare bomb skill ID deal HP damage without breaking Valtan armor");
                    auto laterPhase = std::make_unique<SERVER_WORLD_ENTITY>(boss);
                    laterPhase->iPhase = 2u;
                    (void)CServerCombatHitRuntime::Apply_PlayerToWorld(*laterPhase, ordinary, damageEvents);
                    tests.Require(laterPhase->BossCombat.iAlivePartMask == beforeMask &&
                        laterPhase->BossCombat.PendingPartBreakEdges.empty(),
                        "Unbroken primary Valtan armor remains bomb-only after the normal-body phase transition");
                    auto otherBoss = std::make_unique<SERVER_WORLD_ENTITY>(boss);
                    otherBoss->strArchetypeId = "MINIBOSS_LUGARU";
                    (void)CServerCombatHitRuntime::Apply_PlayerToWorld(*otherBoss, ordinary, damageEvents);
                    tests.Require(otherBoss->BossCombat.iAlivePartMask != beforeMask &&
                        otherBoss->BossCombat.PendingPartBreakEdges.size() == 1u,
                        "Bomb-only Valtan armor does not change another boss's generic part-damage contract");
                }
                std::uint32_t expectedBroken = 0u;
                if (attempt) for (const auto& part : boss.BossCombat.Parts)
                    if (part.iStateMask & beforeMask) { expectedBroken = part.iStateMask; break; }
                C2S_USE_ITEM request;
                request.strItemId = "BATTLE_DESTRUCTION_BOMB";
                request.hasGroundTarget = true;
                request.iRequestSequence = sequence++;
                request.fTargetX = thrower.fPositionX + 100.f; request.fTargetZ = thrower.fPositionZ;
                arena->Handle_UseItem(thrower.iSessionId, request);
                tests.Require(remaining() == initialQuantity - attempt && arena->m_CombatObjectRuntime.Get_LiveObjects().empty(),
                    "Out-of-range destruction throws preserve inventory and create no projectile");
                request.iRequestSequence = sequence++;
                request.fTargetX = boss.fPositionX; request.fTargetZ = boss.fPositionZ;
                arena->Handle_UseItem(thrower.iSessionId, request);
                tests.Require(remaining() == initialQuantity - attempt - 1u && arena->m_CombatObjectRuntime.Get_LiveObjects().size() == 1u,
                    "Accepted destruction command consumes one item and launches one server projectile");
                arena->Handle_UseItem(thrower.iSessionId, request);
                request.iRequestSequence = sequence++;
                arena->Handle_UseItem(thrower.iSessionId, request);
                tests.Require(remaining() == initialQuantity - attempt - 1u && arena->m_CombatObjectRuntime.Get_LiveObjects().size() == 1u,
                    "Duplicate and cooldown destruction requests cannot duplicate the throw");
                damageEvents.clear();
                for (unsigned step = 0u; step < 60u && !arena->m_CombatObjectRuntime.Get_LiveObjects().empty(); ++step)
                {
                    ++arena->m_iServerTick;
                    arena->m_CombatObjectRuntime.Update(arena->m_Players, arena->m_WorldEntities,
                        arena->m_GameplayCatalog, 1.f / 30.f, arena->m_iServerTick, damageEvents);
                }
                tests.Require(arena->m_CombatObjectRuntime.Get_LiveObjects().empty() &&
                    boss.BossCombat.iAlivePartMask == (beforeMask & ~expectedBroken) &&
                    boss.bPendingArmorBreakReaction == (expectedBroken != 0u),
                    "Destruction contact breaks exactly one eligible plate only while groggy");
                if (beforeMask == 0u)
                    tests.Require(!boss.BossCombat.iAlivePartMask &&
                        boss.BossCombat.PendingPartBreakEdges.empty() && boss.BossCombat.PendingOutcomes.empty() &&
                        !boss.bPendingArmorBreakReaction &&
                        std::all_of(boss.BossCombat.Parts.begin(), boss.BossCombat.Parts.end(),
                            [](const auto& part) { return part.iCurrentDurability == 0u; }) &&
                        std::all_of(boss.ArmorPlates.begin(), boss.ArmorPlates.end(),
                            [](const auto& plate) { return plate.iRemainingDurability == 0u; }),
                        "A fourth real destruction throw in another dash recovery cannot re-break or restore either removed armor plate");
                arena->Drain_BossCombatEvents();
                const bool broadcast = arena->Broadcast_CombatObjectLifecycle();
                arena->Broadcast_WorldSnapshot();
                bool parity = broadcast;
                for (const auto& observer : observers)
                {
                    unsigned spawns = 0u, impacts = 0u, despawns = 0u, snapshots = 0u;
                    for (const auto& frame : observer->m_OutboundFrames)
                    {
                        CPacketReader reader{std::span<const std::uint8_t>(frame.Bytes).subspan(PACKET_HEADER_BYTES)};
                        if (frame.ePacketType == PACKET_TYPE::S2C_COMBAT_OBJECT_SPAWNED)
                        {
                            S2C_COMBAT_OBJECT_SPAWNED message;
                            parity &= Read_Message(reader, message) && message.strClientVisualId == "battle.item.destruction_bomb";
                            ++spawns;
                        }
                        else if (frame.ePacketType == PACKET_TYPE::S2C_COMBAT_OBJECT_PRESENTATION_EVENT)
                        {
                            S2C_COMBAT_OBJECT_PRESENTATION_EVENT message;
                            parity &= Read_Message(reader, message) && message.strHitId == "battle.item.impact" &&
                                message.eKind == COMBAT_OBJECT_PRESENTATION_EVENT_KIND::HIT_PULSE;
                            ++impacts;
                        }
                        else if (frame.ePacketType == PACKET_TYPE::S2C_COMBAT_OBJECT_DESPAWNED)
                        {
                            S2C_COMBAT_OBJECT_DESPAWNED message;
                            parity &= Read_Message(reader, message);
                            ++despawns;
                        }
                        else if (frame.ePacketType == PACKET_TYPE::S2C_WORLD_SNAPSHOT)
                        {
                            S2C_WORLD_SNAPSHOT message;
                            parity &= Read_Message(reader, message);
                            const auto entity = std::find_if(message.Entities.begin(), message.Entities.end(),
                                [&](const auto& row) { return row.iNetEntityId == boss.iNetEntityId; });
                            parity &= entity != message.Entities.end() && entity->BossCombat.iAlivePartMask == boss.BossCombat.iAlivePartMask &&
                                entity->iBrokenArmorMask == static_cast<std::uint8_t>(3u & ~boss.BossCombat.iAlivePartMask) &&
                                message.BossCombatEvents.size() == (expectedBroken ? 1u : 0u);
                            if (expectedBroken && message.BossCombatEvents.size() == 1u)
                                parity &= message.BossCombatEvents.front().eKind == BOSS_COMBAT_EVENT_KIND::PART_BROKEN &&
                                    message.BossCombatEvents.front().iPartMask == expectedBroken;
                            ++snapshots;
                        }
                    }
                    parity &= spawns == 1u && impacts == 1u && despawns == 1u && snapshots == 1u;
                }
                tests.Require(parity, "Four observers decode one flight, impact, removal and matching armor break snapshot");
            }
            tests.Require(!boss.BossCombat.iAlivePartMask && std::all_of(boss.ArmorPlates.begin(), boss.ArmorPlates.end(),
                [](const auto& plate) { return !plate.iRemainingDurability; }),
                "Two separate groggy windows remove both typed and legacy armor plates");
        }
        for (const auto& observer : observers) observer->Request_Close();
    }
    {
        auto item = std::find_if(caster.Inventory.begin(), caster.Inventory.end(),
            [](const auto& row) { return row.strItemId == "BATTLE_HOLY_CHARM"; });
        item->iQuantity = 2u; caster.eAction = PLAYER_ACTION_STATE::NONE;
        caster.iTimeStopEndTick = 0u; caster.ItemCooldownEndTicks.clear();
        room->m_iServerTick = 5000u;
        charm.iRequestSequence = 20000u;
        room->Handle_UseItem(caster.iSessionId, charm);
        const auto deadline = caster.ItemCooldownEndTicks.at(charm.strItemId);
        tests.Require(quantity("BATTLE_HOLY_CHARM") == 1u,
            "An equipped item consumes the live inventory stack on the first use");
        ++charm.iRequestSequence; ++room->m_iServerTick;
        room->Handle_UseItem(caster.iSessionId, charm);
        tests.Require(quantity("BATTLE_HOLY_CHARM") == 1u,
            "A new key press during cooldown preserves the remaining inventory item");
        ++charm.iRequestSequence; room->m_iServerTick = deadline - 1u;
        room->Handle_UseItem(caster.iSessionId, charm);
        tests.Require(quantity("BATTLE_HOLY_CHARM") == 0u,
            "The same equipped item can consume the final stack exactly at cooldown expiry");
    }
    {
        auto protectedPlayer = caster;
        protectedPlayer.iCurrentHp = protectedPlayer.iMaximumHp = 10000u;
        protectedPlayer.iShield = 100000u;
        protectedPlayer.iTimeStopEndTick = 9999u;
        protectedPlayer.iHolyCharmProtectionEndTick = 9999u;
        protectedPlayer.iInvulnerableEndTick = 9999u;
        protectedPlayer.iEstherGuardEndTick = 9999u;
        protectedPlayer.bRonaunGuard = true;
        protectedPlayer.ActiveBuffs = {{319303u, 9999u}};
        SERVER_WORLD_TO_PLAYER_HIT instant;
        instant.bInstantDeath = true; instant.iServerTick = 900u;
        instant.iRawDamage = 1u; instant.bEstherGuardBlockable = true;
        for (const auto action : {PLAYER_ACTION_STATE::NONE, PLAYER_ACTION_STATE::GRABBED, PLAYER_ACTION_STATE::FALLING})
        {
            auto victim = protectedPlayer; victim.eAction = action; victim.isCombatReady = false;
            const auto result = CServerCombatHitRuntime::Apply_WorldToPlayer(victim, instant,
                room->m_GameplayCatalog, damageEvents);
            tests.Require(result == SERVER_COMBAT_HIT_RESULT::KILLED && !victim.iCurrentHp &&
                !victim.iShield && victim.eAction == PLAYER_ACTION_STATE::DEAD && victim.ActiveBuffs.empty(),
                "Explicit instant death bypasses shield, time stop, charm, Esther, death-deny and capture/falling state");
        }
        SERVER_WORLD_ENTITY source;
        BOSS_PATTERN_LOGIC_RESULT percent;
        percent.eKind = BOSS_PATTERN_LOGIC_RESULT_KIND::MAX_HP_PERCENT_DAMAGE; percent.iPercent = 100u;
        auto victim = protectedPlayer;
        CKoukuSaydonLogicRuntime::Apply_Result(victim, percent, source, room->m_GameplayCatalog,
            nullptr, 900u, damageEvents);
        tests.Require(!victim.iCurrentHp && victim.eAction == PLAYER_ACTION_STATE::DEAD,
            "Authored maximum-HP 100 percent is unconditional death rather than mitigated damage");
    }
    {
        auto boss = std::make_shared<SERVER_WORLD_ENTITY>();
        boss->iNetEntityId = 601u; boss->eKind = WORLD_BOOTSTRAP_KIND::BOSS;
        boss->strArchetypeId = "BOSS_KAKULSAYDON_G1_SAYDON";
        boss->iCurrentHp = boss->iMaximumHp = 100000000u;
        boss->strPatternId = "damage.reduction.duration"; boss->iPatternSequence = 1u;
        BOSS_PATTERN_DEFINITION pattern; pattern.strPatternId = boss->strPatternId;
        BOSS_PATTERN_LOGIC_WINDOW reduction;
        reduction.strWindowId = "reduction"; reduction.eKind = BOSS_PATTERN_LOGIC_KIND::BOSS_DAMAGE_REDUCTION;
        reduction.iDurationMs = 1000u; pattern.LogicWindows.push_back(reduction);
        auto stagger = reduction; stagger.strWindowId = "stagger";
        stagger.eKind = BOSS_PATTERN_LOGIC_KIND::STAGGER_WINDOW; stagger.iThreshold = 40000u;
        stagger.iDurationMs = 5000u; pattern.LogicWindows.push_back(stagger);
        KOUKUSAYDON_LOGIC_LEDGER ledger; KOUKUSAYDON_LOGIC_OUTPUT output;
        CKoukuSaydonLogicRuntime::Build(pattern, *boss, 1000u, ledger);
        CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, room->m_Players,
            room->m_GameplayCatalog, nullptr, 1000u, damageEvents, output);
        SERVER_PLAYER_TO_WORLD_HIT hit;
        hit.iSourcePlayerId = caster.iPlayerId; hit.iSkillId = 17000u; hit.iRawDamage = 1000000u;
        hit.iServerTick = 1001u;
        const auto before = boss->iCurrentHp; damageEvents.clear();
        CServerCombatHitRuntime::Apply_PlayerToWorld(*boss, hit, damageEvents);
        tests.Require(before - boss->iCurrentHp == 1000u && boss->iKoukuItemStaggerCredit == 1000u &&
            damageEvents.size() == 1u && damageEvents.front().eHitFlag == DAMAGE_HIT_FLAG::DAMAGE_REDUCED,
            "Duration applies exactly one /1000 to HP and independent stagger and flags its damage font");
        auto liveBoss = *boss; liveBoss.iKoukuDamageReductionWindows = 0u;
        liveBoss.iKoukuItemStaggerMaximum = 0u; liveBoss.KoukuRetainedLogicOwners.push_back(boss);
        const auto retainedBefore = liveBoss.iCurrentHp;
        CServerCombatHitRuntime::Apply_PlayerToWorld(liveBoss, hit, damageEvents);
        tests.Require(retainedBefore - liveBoss.iCurrentHp == 1000u && boss->iKoukuItemStaggerCredit == 2000u,
            "Retained parent duration and stagger apply to the current phase boss once");
        CKoukuSaydonLogicRuntime::Update(*boss, pattern, ledger, room->m_Players,
            room->m_GameplayCatalog, nullptr, 1030u, damageEvents, output);
        const auto normalBefore = boss->iCurrentHp; damageEvents.clear();
        CServerCombatHitRuntime::Apply_PlayerToWorld(*boss, hit, damageEvents);
        tests.Require(!boss->iKoukuDamageReductionWindows && normalBefore - boss->iCurrentHp == 1000000u &&
            boss->iKoukuItemStaggerCredit == 3000u && damageEvents.front().eHitFlag == DAMAGE_HIT_FLAG::NORMAL,
            "Duration expires on its end tick: full HP damage returns while stagger remains /1000");
    }
    {
        SERVER_PLAYER victim;
        victim.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
        victim.iNetEntityId = 7001u; victim.isCombatReady = true;
        victim.iCurrentHp = victim.iMaximumHp = 1000000u;
        SERVER_WORLD_TO_PLAYER_HIT incoming;
        incoming.iRawDamage = 1320u; incoming.iServerTick = 1200u;
        incoming.bIgnoreDefense = incoming.bIgnoreCounter = true;
        std::uint64_t total = 0u;
        std::uint32_t lowest = 2000u, highest = 0u;
        bool valid = true;
        for (unsigned sample = 0u; sample < 20000u; ++sample)
        {
            victim.iCurrentHp = victim.iMaximumHp; victim.eAction = PLAYER_ACTION_STATE::NONE;
            damageEvents.clear();
            CServerCombatHitRuntime::Apply_WorldToPlayer(victim, incoming, room->m_GameplayCatalog, damageEvents);
            const auto amount = victim.iMaximumHp - victim.iCurrentHp;
            lowest = (std::min)(lowest, amount); highest = (std::max)(highest, amount); total += amount;
            valid &= amount >= 1188u && amount <= 1452u && damageEvents.size() == 1u &&
                damageEvents.front().iAmount == amount && !damageEvents.front().isOutgoing;
        }
        tests.Require(valid && lowest == 1188u && highest == 1452u && victim.iIncomingDamageSampleSerial == 20000u,
            "Same-tick incoming contacts sample independent 1188..1452 integer damage and replicate actual HP loss");
        tests.Require(total >= 26347200ull && total <= 26452800ull,
            "Deterministic 20000-hit incoming sample mean stays within 0.2 percent of the 1320 baseline");
        incoming.iRawDamage = 100u; lowest = 200u; highest = 0u;
        for (unsigned sample = 0u; sample < 256u; ++sample)
        {
            victim.iCurrentHp = victim.iMaximumHp; damageEvents.clear();
            CServerCombatHitRuntime::Apply_WorldToPlayer(victim, incoming, room->m_GameplayCatalog, damageEvents);
            const auto amount = damageEvents.front().iAmount;
            lowest = (std::min)(lowest, amount); highest = (std::max)(highest, amount);
        }
        tests.Require(lowest == 90u && highest == 110u, "A 100 damage hit uses the inclusive 90..110 range");
        auto replay = victim;
        victim.iCurrentHp = replay.iCurrentHp = victim.iMaximumHp;
        damageEvents.clear();
        CServerCombatHitRuntime::Apply_WorldToPlayer(victim, incoming, room->m_GameplayCatalog, damageEvents);
        damageEvents.clear();
        CServerCombatHitRuntime::Apply_WorldToPlayer(replay, incoming, room->m_GameplayCatalog, damageEvents);
        tests.Require(victim.iCurrentHp == replay.iCurrentHp &&
            victim.iIncomingDamageSampleSerial == replay.iIncomingDamageSampleSerial,
            "The same server player state replays the same incoming roll without client calculation");
        incoming.iRawDamage = 1320u; incoming.bIgnoreDefense = false;
        victim.ActiveBuffs = {{171702u, 9999u}};
        const auto* profile = room->m_GameplayCatalog.Find_Player(victim.eCharacterClass);
        const auto base = CServerBuffRuntime::Scale_Damage(CGameplayCatalog::Apply_Defense(1320u,
            profile ? profile->iDefense : 0u),
            CServerBuffRuntime::Damage_TakenPercent(room->m_GameplayCatalog, victim.ActiveBuffs));
        valid = true;
        for (unsigned sample = 0u; sample < 128u; ++sample)
        {
            victim.iCurrentHp = victim.iMaximumHp; damageEvents.clear();
            CServerCombatHitRuntime::Apply_WorldToPlayer(victim, incoming, room->m_GameplayCatalog, damageEvents);
            const auto amount = victim.iMaximumHp - victim.iCurrentHp;
            valid &= amount >= base - base / 10u && amount <= base + base / 10u;
        }
        tests.Require(valid && base < 1320u, "Incoming variation uses the final defense-and-buff-mitigated baseline");
        incoming.bIgnoreDefense = true; victim.ActiveBuffs.clear();
        victim.iCurrentHp = victim.iMaximumHp; victim.iShield = 500u; damageEvents.clear();
        CServerCombatHitRuntime::Apply_WorldToPlayer(victim, incoming, room->m_GameplayCatalog, damageEvents);
        const auto hpLoss = victim.iMaximumHp - victim.iCurrentHp;
        tests.Require(!victim.iShield && damageEvents.size() == 2u &&
            damageEvents[0].eHitFlag == DAMAGE_HIT_FLAG::ABSORB && damageEvents[0].iAmount == 500u &&
            damageEvents[1].iAmount == hpLoss && hpLoss + 500u >= 1188u && hpLoss + 500u <= 1452u,
            "One incoming roll is split between shield absorption and the exact replicated HP loss");
        const auto serialBeforeExcluded = victim.iIncomingDamageSampleSerial;
        incoming.iRawDamage = 0u; damageEvents.clear();
        CServerCombatHitRuntime::Apply_WorldToPlayer(victim, incoming, room->m_GameplayCatalog, damageEvents);
        incoming.iRawDamage = 1320u; victim.iInvulnerableEndTick = 9999u;
        CServerCombatHitRuntime::Apply_WorldToPlayer(victim, incoming, room->m_GameplayCatalog, damageEvents);
        tests.Require(damageEvents.empty() && victim.iIncomingDamageSampleSerial == serialBeforeExcluded,
            "Zero damage and invulnerability do not advance the incoming variation stream");
        for (const bool wipe : {false, true})
        {
            auto doomed = victim; doomed.iCurrentHp = 100u; doomed.iShield = 10000u;
            incoming.bInstantDeath = !wipe; incoming.bEncounterWipe = wipe; damageEvents.clear();
            const auto result = CServerCombatHitRuntime::Apply_WorldToPlayer(doomed, incoming,
                room->m_GameplayCatalog, damageEvents);
            tests.Require(result == SERVER_COMBAT_HIT_RESULT::KILLED && !doomed.iCurrentHp &&
                doomed.iIncomingDamageSampleSerial == serialBeforeExcluded &&
                damageEvents.size() == 1u && damageEvents.front().iAmount == 100u,
                "Instant death and encounter wipe bypass random variation and retain exact actual-HP events");
        }
        incoming.bInstantDeath = incoming.bEncounterWipe = false; victim.iInvulnerableEndTick = 0u;
        victim.iCurrentHp = 50u; damageEvents.clear();
        CServerCombatHitRuntime::Apply_WorldToPlayer(victim, incoming, room->m_GameplayCatalog, damageEvents);
        tests.Require(!victim.iCurrentHp && damageEvents.size() == 1u && damageEvents.front().iAmount == 50u,
            "A randomized overkill event is capped to the HP actually removed");
    }
    std::cout << "failures : " << tests.failures << '\n';
    return tests.failures ? 1 : 0;
}
