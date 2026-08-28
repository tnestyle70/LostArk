#pragma once

#include "ServerIds.h"
#include "Network/PacketMessages.h"

#include <memory>
#include <vector>

namespace LostArk::Server
{
	//server는 여러 개의 존재하는 client를 가져야 한다.
	class CClientSession;
	// Room은 같은 World 상태와 Tick을 공유하고 서로의 Event를 Broadcast 받는 세션 집합이다.
	// 현재는 베른성 하나이므로 Server에 CGameRoom 하나만 존재한다.
	enum class ROOM_COMMAND_TYPE
	{
		REGISTER_SESSION,
		ENTER_WORLD,
		MOVE,
		USE_SKILL,
		RELEASE_SKILL,
		UPDATE_SKILL_AIM,
		USE_ESTHER_SKILL,
		REVIVE_PLAYER,
		DEBUG_KILL_SELF,
		CHANGE_CHARACTER_CLASS,
		SPAWN_WORLD_ENTITY,
		VALTAN_AUDITION,
		VALTAN_PATTERN_FLOW_START,
		VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT,
		DEBUG_GIVE_ITEM,
		USE_ITEM,
		DESPAWN_ALL_WORLD_ENTITIES,
		CONFIRM_NPC_ENTRY,
		RETURN_TO_BERN,
		PARTY_INVITE,
		PARTY_INVITE_RESPOND,
		CHAT,
		LEAVE
	};

	struct ROOM_COMMAND
	{
		ROOM_COMMAND_TYPE eType =
			ROOM_COMMAND_TYPE::REGISTER_SESSION;

		SESSION_ID iSessionId = INVALID_SESSION_ID;

		std::shared_ptr<CClientSession> pSession;

		LostArk::Shared::C2S_ENTER_WORLD EnterWorld;
		/* Server-internal only, never part of the wire message -- see
		SERVER_WORLD_TRANSFER_REQUEST::strSpawnPlacementOverrideId. Set only when
		CServerApp::Transfer_SessionWorld builds this ENTER_WORLD command itself
		(a real Client-sent C2S_ENTER_WORLD never populates it). */
		std::string strSpawnPlacementOverrideId;
		/* Server-internal only, never part of the wire message -- see
		SERVER_WORLD_TRANSFER_REQUEST::CarriedInventory. Empty means grant the
		default fresh-entry loadout. */
		std::vector<LostArk::Shared::INVENTORY_ITEM_SNAPSHOT> CarriedInventory;

		LostArk::Shared::C2S_MOVE Move;

		LostArk::Shared::C2S_USE_SKILL UseSkill;

		LostArk::Shared::C2S_RELEASE_SKILL ReleaseSkill;
		LostArk::Shared::C2S_UPDATE_SKILL_AIM UpdateSkillAim;
		LostArk::Shared::C2S_USE_ESTHER_SKILL UseEstherSkill;
		LostArk::Shared::C2S_REVIVE_PLAYER RevivePlayer;
		LostArk::Shared::C2S_DEBUG_KILL_SELF DebugKillSelf;
		LostArk::Shared::C2S_CHANGE_CHARACTER_CLASS ChangeCharacterClass;

		LostArk::Shared::C2S_SPAWN_WORLD_ENTITY SpawnWorldEntity;

		LostArk::Shared::C2S_VALTAN_AUDITION_REQUEST ValtanAudition;
		LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_START
			ValtanPatternFlowStart;
		LostArk::Shared::C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT
			ValtanPatternFlowStopAfterCurrent;

		LostArk::Shared::C2S_DEBUG_GIVE_ITEM DebugGiveItem;
		LostArk::Shared::C2S_USE_ITEM UseItem;
		LostArk::Shared::C2S_DESPAWN_ALL_WORLD_ENTITIES DespawnAllWorldEntities;
		LostArk::Shared::C2S_CONFIRM_NPC_ENTRY ConfirmNpcEntry;
		LostArk::Shared::C2S_RETURN_TO_BERN ReturnToBern;
		LostArk::Shared::C2S_PARTY_INVITE PartyInvite;
		LostArk::Shared::C2S_PARTY_INVITE_RESPOND PartyInviteRespond;
		LostArk::Shared::C2S_CHAT Chat;

		LostArk::Shared::PLAYER_DESPAWN_REASON eLeaveReason =
			LostArk::Shared::PLAYER_DESPAWN_REASON::DISCONNECTED;
	};
}
