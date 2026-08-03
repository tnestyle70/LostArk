```cpp
// FILE: Shared/Public/Gameplay/SkillContracts.h

#pragma once

#include "Gameplay/GameplayContracts.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace LostArk::Shared
{
	inline constexpr std::size_t MAX_SKILL_SLOTS = 8;

	struct SKILL_COOLDOWN_SNAPSHOT
	{
		SKILL_ID iSkillId = INVALID_SKILL_ID;
		std::uint32_t iCooldownEndTick = 0;
		bool isUsable = false;
	};

	struct PLAYER_COMBAT_SNAPSHOT
	{
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
		std::uint32_t iHp = 0;
		std::uint32_t iMaxHp = 0;
		std::uint32_t iResource = 0;
		std::uint32_t iMaxResource = 0;
		std::uint8_t iIdentityState = 0;
		std::vector<SKILL_COOLDOWN_SNAPSHOT> Skills;
	};

	struct S2C_PLAYER_COMBAT_SNAPSHOT
	{
		std::uint32_t iServerTick = 0;
		std::vector<PLAYER_COMBAT_SNAPSHOT> Players;
	};
}
```

```cpp
// FILE: Shared/Public/Gameplay/SkillMessages.h

#pragma once

#include "Gameplay/SkillContracts.h"

namespace LostArk::Shared
{
	class CPacketReader;
	class CPacketWriter;

	bool Write_Message(
		CPacketWriter& writer,
		const S2C_PLAYER_COMBAT_SNAPSHOT& message);

	bool Read_Message(
		CPacketReader& reader,
		S2C_PLAYER_COMBAT_SNAPSHOT& message);
}
```

```cpp
// FILE: Shared/Public/Network/PacketType.h
// ADD ENUMERATOR AND Is_Known_Packet_Type() CASE

S2C_PLAYER_COMBAT_SNAPSHOT,
```

```cpp
// FILE: Shared/Private/Gameplay/SkillMessages.cpp

#include "Gameplay/SkillMessages.h"

#include "Network/PacketMessages.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <utility>

namespace
{
	bool Is_Valid(
		const LostArk::Shared::PLAYER_COMBAT_SNAPSHOT& player)
	{
		using namespace LostArk::Shared;
		if (player.iNetEntityId == INVALID_NET_ENTITY_ID ||
			0 == player.iMaxHp ||
			player.iHp > player.iMaxHp ||
			0 == player.iMaxResource ||
			player.iResource > player.iMaxResource ||
			player.Skills.size() > MAX_SKILL_SLOTS)
		{
			return false;
		}

		for (const SKILL_COOLDOWN_SNAPSHOT& skill : player.Skills)
		{
			if (skill.iSkillId == INVALID_SKILL_ID)
				return false;
		}

		return true;
	}
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_PLAYER_COMBAT_SNAPSHOT& message)
{
	if (0 == message.iServerTick ||
		message.Players.empty() ||
		message.Players.size() > MAX_WORLD_SNAPSHOT_PLAYERS)
	{
		return false;
	}

	for (const PLAYER_COMBAT_SNAPSHOT& player : message.Players)
		if (!Is_Valid(player)) return false;

	writer.Write_U32(message.iServerTick);
	writer.Write_U16(static_cast<std::uint16_t>(message.Players.size()));
	for (const PLAYER_COMBAT_SNAPSHOT& player : message.Players)
	{
		writer.Write_U32(player.iNetEntityId);
		writer.Write_U32(player.iHp);
		writer.Write_U32(player.iMaxHp);
		writer.Write_U32(player.iResource);
		writer.Write_U32(player.iMaxResource);
		writer.Write_U8(player.iIdentityState);
		writer.Write_U8(static_cast<std::uint8_t>(player.Skills.size()));
		for (const SKILL_COOLDOWN_SNAPSHOT& skill : player.Skills)
		{
			writer.Write_U32(skill.iSkillId);
			writer.Write_U32(skill.iCooldownEndTick);
			writer.Write_U8(skill.isUsable ? 1u : 0u);
		}
	}
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_PLAYER_COMBAT_SNAPSHOT& message)
{
	std::uint32_t serverTick = 0;
	std::uint16_t playerCount = 0;
	if (!reader.Read_U32(serverTick) ||
		!reader.Read_U16(playerCount) ||
		0 == serverTick ||
		0 == playerCount ||
		playerCount > MAX_WORLD_SNAPSHOT_PLAYERS)
	{
		return false;
	}

	S2C_PLAYER_COMBAT_SNAPSHOT decoded{};
	decoded.iServerTick = serverTick;
	decoded.Players.reserve(playerCount);

	for (std::uint16_t index = 0; index < playerCount; ++index)
	{
		PLAYER_COMBAT_SNAPSHOT player{};
		std::uint8_t skillCount = 0;
		if (!reader.Read_U32(player.iNetEntityId) ||
			!reader.Read_U32(player.iHp) ||
			!reader.Read_U32(player.iMaxHp) ||
			!reader.Read_U32(player.iResource) ||
			!reader.Read_U32(player.iMaxResource) ||
			!reader.Read_U8(player.iIdentityState) ||
			!reader.Read_U8(skillCount) ||
			skillCount > MAX_SKILL_SLOTS)
		{
			return false;
		}

		player.Skills.reserve(skillCount);
		for (std::uint8_t skillIndex = 0;
			skillIndex < skillCount;
			++skillIndex)
		{
			SKILL_COOLDOWN_SNAPSHOT skill{};
			std::uint8_t rawUsable = 0;
			if (!reader.Read_U32(skill.iSkillId) ||
				!reader.Read_U32(skill.iCooldownEndTick) ||
				!reader.Read_U8(rawUsable) ||
				rawUsable > 1u)
			{
				return false;
			}
			skill.isUsable = 1u == rawUsable;
			player.Skills.push_back(skill);
		}

		if (!Is_Valid(player)) return false;
		decoded.Players.push_back(std::move(player));
	}

	message = std::move(decoded);
	return true;
}
```

```cpp
// FILE: Server/Public/ServerPlayer.h
// ADD INCLUDE AND MEMBERS

#include "Gameplay/GameplayContracts.h"
#include <unordered_map>

std::uint32_t iHp = 1000;
std::uint32_t iMaxHp = 1000;
std::uint32_t iResource = 100;
std::uint32_t iMaxResource = 100;
std::uint8_t iIdentityState = 0;
std::uint32_t iLastSkillSequence = 0;
LostArk::Shared::ACTION_ID iActionId =
	LostArk::Shared::INVALID_ACTION_ID;
std::uint32_t iActionStartTick = 0;
std::uint32_t iActionEndTick = 0;
std::unordered_map<LostArk::Shared::SKILL_ID, std::uint32_t>
	CooldownEndTickBySkillId;
```

```cpp
// FILE: Server/Public/SkillSystem.h

#pragma once

#include "ServerPlayer.h"

#include "Gameplay/GameplayContracts.h"

#include <cstdint>
#include <map>
#include <vector>

namespace LostArk::Server
{
	struct SKILL_DEFINITION
	{
		LostArk::Shared::SKILL_ID iSkillId =
			LostArk::Shared::INVALID_SKILL_ID;
		LostArk::Shared::ACTION_ID iActionId =
			LostArk::Shared::INVALID_ACTION_ID;
		LostArk::Shared::EFFECT_ASSET_ID iEffectAssetId =
			LostArk::Shared::INVALID_EFFECT_ASSET_ID;
		std::uint32_t iCooldownTicks = 0;
		std::uint32_t iActionTicks = 0;
		std::uint32_t iResourceCost = 0;
		std::uint32_t iDamage = 0;
		float fCastRange = 0.f;
		float fTargetRadius = 0.f;
	};

	enum class SKILL_ACTIVATION_RESULT
	{
		ACCEPTED,
		REJECTED,
		INVALID_COMMAND
	};

	struct SKILL_ACTIVATION_OUTPUT
	{
		std::vector<LostArk::Shared::GAMEPLAY_EVENT_PAYLOAD> Events;
	};

	class CSkillSystem final
	{
	public:
		SKILL_ACTIVATION_RESULT Try_Activate(
			SERVER_PLAYER& source,
			std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
			const LostArk::Shared::C2S_USE_SKILL& command,
			std::uint32_t serverTick,
			SKILL_ACTIVATION_OUTPUT& output) const;

		void Update_ActionState(
			SERVER_PLAYER& player,
			std::uint32_t serverTick) const;

		static const SKILL_DEFINITION* Find_Definition(
			LostArk::Shared::SKILL_ID skillId);
	};
}
```

```cpp
// FILE: Server/Private/SkillSystem.cpp

#include "SkillSystem.h"

#include <cmath>
#include <limits>

namespace
{
	constexpr LostArk::Server::SKILL_DEFINITION TEST_SKILL
	{
		1001,
		1001,
		10001,
		90,
		20,
		10,
		100,
		10.f,
		2.f
	};

	float DistanceSquared(float leftX, float leftZ, float rightX, float rightZ)
	{
		const float x = leftX - rightX;
		const float z = leftZ - rightZ;
		return x * x + z * z;
	}
}

const LostArk::Server::SKILL_DEFINITION*
LostArk::Server::CSkillSystem::Find_Definition(
	LostArk::Shared::SKILL_ID skillId)
{
	return skillId == TEST_SKILL.iSkillId ? &TEST_SKILL : nullptr;
}

LostArk::Server::SKILL_ACTIVATION_RESULT
LostArk::Server::CSkillSystem::Try_Activate(
	SERVER_PLAYER& source,
	std::map<LostArk::Shared::PLAYER_ID, SERVER_PLAYER>& players,
	const LostArk::Shared::C2S_USE_SKILL& command,
	std::uint32_t serverTick,
	SKILL_ACTIVATION_OUTPUT& output) const
{
	using namespace LostArk::Shared;
	output = {};

	if (0 == command.iSequence ||
		!std::isfinite(command.fAimX) ||
		!std::isfinite(command.fAimY) ||
		!std::isfinite(command.fAimZ))
	{
		return SKILL_ACTIVATION_RESULT::INVALID_COMMAND;
	}

	if (command.iSequence <= source.iLastSkillSequence)
		return SKILL_ACTIVATION_RESULT::REJECTED;

	source.iLastSkillSequence = command.iSequence;
	const SKILL_DEFINITION* definition =
		Find_Definition(command.iSkillId);
	if (nullptr == definition ||
		0 == source.iHp ||
		source.iActionId != INVALID_ACTION_ID ||
		source.iResource < definition->iResourceCost)
	{
		return SKILL_ACTIVATION_RESULT::REJECTED;
	}

	const auto cooldownIter =
		source.CooldownEndTickBySkillId.find(command.iSkillId);
	if (cooldownIter != source.CooldownEndTickBySkillId.end() &&
		serverTick < cooldownIter->second)
	{
		return SKILL_ACTIVATION_RESULT::REJECTED;
	}

	const float castRangeSquared =
		definition->fCastRange * definition->fCastRange;
	if (DistanceSquared(
		source.fPositionX, source.fPositionZ,
		command.fAimX, command.fAimZ) > castRangeSquared)
	{
		return SKILL_ACTIVATION_RESULT::REJECTED;
	}

	source.iResource -= definition->iResourceCost;
	source.CooldownEndTickBySkillId[command.iSkillId] =
		serverTick + definition->iCooldownTicks;
	source.iActionId = definition->iActionId;
	source.iActionStartTick = serverTick;
	source.iActionEndTick = serverTick + definition->iActionTicks;
	source.hasMoveGoal = false;

	ACTION_STARTED_EVENT action{};
	action.iSourceNetEntityId = source.iNetEntityId;
	action.iActionId = definition->iActionId;
	action.iActionStartTick = serverTick;
	action.fDirectionX = command.fAimX - source.fPositionX;
	action.fDirectionZ = command.fAimZ - source.fPositionZ;
	output.Events.emplace_back(action);

	EFFECT_CUE effect{};
	effect.iEffectAssetId = definition->iEffectAssetId;
	effect.iSourceNetEntityId = source.iNetEntityId;
	effect.eAttachSocket = EFFECT_ATTACH_SOCKET::WEAPON_RIGHT;
	effect.fWorldX = source.fPositionX;
	effect.fWorldY = source.fPositionY;
	effect.fWorldZ = source.fPositionZ;
	effect.fDirectionX = action.fDirectionX;
	effect.fDirectionZ = action.fDirectionZ;
	effect.iServerStartTick = serverTick;

	SERVER_PLAYER* target = nullptr;
	float bestDistanceSquared = (std::numeric_limits<float>::max)();
	const float targetRadiusSquared =
		definition->fTargetRadius * definition->fTargetRadius;
	for (auto& [playerId, candidate] : players)
	{
		(void)playerId;
		if (candidate.iPlayerId == source.iPlayerId || 0 == candidate.iHp)
			continue;
		const float distanceSquared = DistanceSquared(
			candidate.fPositionX, candidate.fPositionZ,
			command.fAimX, command.fAimZ);
		if (distanceSquared <= targetRadiusSquared &&
			distanceSquared < bestDistanceSquared)
		{
			target = &candidate;
			bestDistanceSquared = distanceSquared;
		}
	}

	if (nullptr != target)
	{
		effect.iTargetNetEntityId = target->iNetEntityId;
		const std::uint32_t damage =
			(definition->iDamage < target->iHp) ?
			definition->iDamage : target->iHp;
		target->iHp -= damage;

		DAMAGE_APPLIED_EVENT damageEvent{};
		damageEvent.iSourceNetEntityId = source.iNetEntityId;
		damageEvent.iTargetNetEntityId = target->iNetEntityId;
		damageEvent.iDamage = damage;
		damageEvent.iRemainingHp = target->iHp;
		output.Events.emplace_back(damageEvent);

		if (0 == target->iHp)
		{
			target->hasMoveGoal = false;
			target->iActionId = INVALID_ACTION_ID;
			ENTITY_DIED_EVENT died{};
			died.iNetEntityId = target->iNetEntityId;
			died.iKillerNetEntityId = source.iNetEntityId;
			output.Events.emplace_back(died);
		}
	}

	output.Events.emplace_back(effect);
	return SKILL_ACTIVATION_RESULT::ACCEPTED;
}

void LostArk::Server::CSkillSystem::Update_ActionState(
	SERVER_PLAYER& player,
	std::uint32_t serverTick) const
{
	if (player.iActionId != LostArk::Shared::INVALID_ACTION_ID &&
		serverTick >= player.iActionEndTick)
	{
		player.iActionId = LostArk::Shared::INVALID_ACTION_ID;
		player.iActionStartTick = 0;
		player.iActionEndTick = 0;
	}
}
```

```cpp
// FILE: Server/Public/RoomCommand.h
// ADD ENUMERATOR AND MEMBER

USE_SKILL,

LostArk::Shared::C2S_USE_SKILL UseSkill;
```

```cpp
// FILE: Server/Public/GameRoom.h
// ADD INCLUDE, FUNCTIONS, MEMBERS

#include "SkillSystem.h"

bool Handle_UseSkill(
	SESSION_ID sessionId,
	const LostArk::Shared::C2S_USE_SKILL& command);
bool Broadcast_GameplayEvent(
	const LostArk::Shared::GAMEPLAY_EVENT_PAYLOAD& payload);
bool Broadcast_PlayerCombatSnapshot();
void Update_CombatState();

CSkillSystem m_SkillSystem;
std::uint32_t m_iNextGameplayEventId = 1;
```

```cpp
// FILE: Server/Private/GameRoom.cpp
// ADD TO Tick() COMMAND SWITCH, THEN CALL AFTER Update_Players()

case ROOM_COMMAND_TYPE::USE_SKILL:
	if (!Handle_UseSkill(command.iSessionId, command.UseSkill))
	{
		if (const auto session = Find_Session(command.iSessionId))
			session->Request_Close();
	}
	break;

Update_CombatState();
Broadcast_PlayerCombatSnapshot();
```

```cpp
// FILE: Server/Private/GameRoom.cpp
// APPEND

#include "Gameplay/GameplayMessages.h"
#include "Gameplay/SkillMessages.h"

bool LostArk::Server::CGameRoom::Handle_UseSkill(
	SESSION_ID sessionId,
	const LostArk::Shared::C2S_USE_SKILL& command)
{
	const auto sessionIter = m_PlayerIdBySessionId.find(sessionId);
	if (sessionIter == m_PlayerIdBySessionId.end()) return false;
	const auto playerIter = m_Players.find(sessionIter->second);
	if (playerIter == m_Players.end()) return false;

	SKILL_ACTIVATION_OUTPUT output{};
	const SKILL_ACTIVATION_RESULT result = m_SkillSystem.Try_Activate(
		playerIter->second,
		m_Players,
		command,
		m_iServerTick,
		output);
	if (result == SKILL_ACTIVATION_RESULT::INVALID_COMMAND) return false;
	if (result == SKILL_ACTIVATION_RESULT::REJECTED) return true;

	for (const auto& event : output.Events)
		if (!Broadcast_GameplayEvent(event)) return false;
	return true;
}

bool LostArk::Server::CGameRoom::Broadcast_GameplayEvent(
	const LostArk::Shared::GAMEPLAY_EVENT_PAYLOAD& payload)
{
	using namespace LostArk::Shared;
	if (0 == m_iNextGameplayEventId) ++m_iNextGameplayEventId;
	S2C_GAMEPLAY_EVENT message{};
	message.Event.iEventId = m_iNextGameplayEventId++;
	message.Event.iServerTick = m_iServerTick;
	message.Event.Payload = payload;

	CPacketWriter writer;
	if (!Write_Message(writer, message)) return false;
	bool allSucceeded = true;
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const auto session = Find_Session(sessionId);
		if (nullptr == session || !session->Send_Frame(
			PACKET_TYPE::S2C_GAMEPLAY_EVENT, writer.Get_Buffer()))
		{
			allSucceeded = false;
			if (nullptr != session) session->Request_Close();
		}
	}
	return allSucceeded;
}

void LostArk::Server::CGameRoom::Update_CombatState()
{
	for (auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		m_SkillSystem.Update_ActionState(player, m_iServerTick);
	}
}

bool LostArk::Server::CGameRoom::Broadcast_PlayerCombatSnapshot()
{
	using namespace LostArk::Shared;
	if (m_Players.empty()) return true;
	S2C_PLAYER_COMBAT_SNAPSHOT message{};
	message.iServerTick = m_iServerTick;
	for (const auto& [playerId, player] : m_Players)
	{
		(void)playerId;
		PLAYER_COMBAT_SNAPSHOT snapshot{};
		snapshot.iNetEntityId = player.iNetEntityId;
		snapshot.iHp = player.iHp;
		snapshot.iMaxHp = player.iMaxHp;
		snapshot.iResource = player.iResource;
		snapshot.iMaxResource = player.iMaxResource;
		snapshot.iIdentityState = player.iIdentityState;
		const SKILL_DEFINITION* skill = CSkillSystem::Find_Definition(1001);
		if (nullptr != skill)
		{
			SKILL_COOLDOWN_SNAPSHOT cooldown{};
			cooldown.iSkillId = skill->iSkillId;
			const auto iter = player.CooldownEndTickBySkillId.find(skill->iSkillId);
			cooldown.iCooldownEndTick = iter == player.CooldownEndTickBySkillId.end() ? 0 : iter->second;
			cooldown.isUsable = player.iHp > 0 &&
				player.iResource >= skill->iResourceCost &&
				m_iServerTick >= cooldown.iCooldownEndTick;
			snapshot.Skills.push_back(cooldown);
		}
		message.Players.push_back(std::move(snapshot));
	}

	CPacketWriter writer;
	if (!Write_Message(writer, message)) return false;
	bool allSucceeded = true;
	for (const auto& [sessionId, playerId] : m_PlayerIdBySessionId)
	{
		(void)playerId;
		const auto session = Find_Session(sessionId);
		if (nullptr == session || !session->Send_Frame(
			PACKET_TYPE::S2C_PLAYER_COMBAT_SNAPSHOT,
			writer.Get_Buffer()))
		{
			allSucceeded = false;
			if (nullptr != session) session->Request_Close();
		}
	}
	return allSucceeded;
}
```

```cpp
// FILE: Server/Private/ServerApp.cpp
// ADD INCLUDE AND PACKET SWITCH CASE

#include "Gameplay/GameplayMessages.h"

case PACKET_TYPE::C2S_USE_SKILL:
{
	C2S_USE_SKILL useSkill{};
	if (!Read_Message(reader, useSkill) ||
		0 != reader.Get_RemainingSize())
	{
		Request_SessionClose(sessionId);
		return;
	}
	command.eType = ROOM_COMMAND_TYPE::USE_SKILL;
	command.UseSkill = useSkill;
	break;
}
```

```cpp
// FILE: Client/Public/PlayerCombatViewModel.h

#pragma once

#include "Gameplay/SkillContracts.h"

#include <cstdint>
#include <unordered_map>

namespace Client
{
	class CPlayerCombatViewModel final
	{
	public:
		bool Apply(
			const LostArk::Shared::S2C_PLAYER_COMBAT_SNAPSHOT& snapshot,
			LostArk::Shared::NET_ENTITY_ID localEntityId);
		void Reset();
		[[nodiscard]] const LostArk::Shared::PLAYER_COMBAT_SNAPSHOT*
			Get_LocalPlayer() const;
		[[nodiscard]] std::uint32_t Get_LastServerTick() const;
		[[nodiscard]] std::uint32_t Get_RemainingCooldownTicks(
			LostArk::Shared::SKILL_ID skillId) const;

	private:
		LostArk::Shared::PLAYER_COMBAT_SNAPSHOT m_LocalPlayer;
		std::uint32_t m_iLastServerTick = 0;
		bool m_hasLocalPlayer = false;
	};
}
```

```cpp
// FILE: Client/Private/PlayerCombatViewModel.cpp

#include "PlayerCombatViewModel.h"

bool Client::CPlayerCombatViewModel::Apply(
	const LostArk::Shared::S2C_PLAYER_COMBAT_SNAPSHOT& snapshot,
	LostArk::Shared::NET_ENTITY_ID localEntityId)
{
	if (0 == snapshot.iServerTick ||
		snapshot.iServerTick <= m_iLastServerTick ||
		localEntityId == LostArk::Shared::INVALID_NET_ENTITY_ID)
		return false;

	for (const auto& player : snapshot.Players)
	{
		if (player.iNetEntityId == localEntityId)
		{
			m_LocalPlayer = player;
			m_hasLocalPlayer = true;
			break;
		}
	}
	m_iLastServerTick = snapshot.iServerTick;
	return true;
}

void Client::CPlayerCombatViewModel::Reset()
{
	m_LocalPlayer = {};
	m_iLastServerTick = 0;
	m_hasLocalPlayer = false;
}

const LostArk::Shared::PLAYER_COMBAT_SNAPSHOT*
Client::CPlayerCombatViewModel::Get_LocalPlayer() const
{
	return m_hasLocalPlayer ? &m_LocalPlayer : nullptr;
}

std::uint32_t Client::CPlayerCombatViewModel::Get_LastServerTick() const
{
	return m_iLastServerTick;
}

std::uint32_t Client::CPlayerCombatViewModel::Get_RemainingCooldownTicks(
	LostArk::Shared::SKILL_ID skillId) const
{
	if (!m_hasLocalPlayer) return 0;
	for (const auto& skill : m_LocalPlayer.Skills)
	{
		if (skill.iSkillId == skillId)
			return skill.iCooldownEndTick > m_iLastServerTick ?
				skill.iCooldownEndTick - m_iLastServerTick : 0;
	}
	return 0;
}
```

```cpp
// FILE: Client/Public/EffectCueSink.h

#pragma once

#include "Gameplay/GameplayContracts.h"

#include <cstdint>
#include <unordered_set>

namespace Client
{
	class IEffectCueSink
	{
	public:
		virtual ~IEffectCueSink() = default;
		virtual bool Play(
			std::uint32_t eventId,
			const LostArk::Shared::EFFECT_CUE& cue) = 0;
	};

	class CDebugEffectCueSink final : public IEffectCueSink
	{
	public:
		bool Play(
			std::uint32_t eventId,
			const LostArk::Shared::EFFECT_CUE& cue) override;
		[[nodiscard]] std::uint32_t Get_PlayCount() const;
	private:
		std::unordered_set<std::uint32_t> m_PlayedEventIds;
		std::uint32_t m_iPlayCount = 0;
	};
}
```

```cpp
// FILE: Client/Private/EffectCueSink.cpp

#include "EffectCueSink.h"

#include <Windows.h>
#include <string>

bool Client::CDebugEffectCueSink::Play(
	std::uint32_t eventId,
	const LostArk::Shared::EFFECT_CUE& cue)
{
	if (0 == eventId ||
		cue.iEffectAssetId == LostArk::Shared::INVALID_EFFECT_ASSET_ID ||
		!m_PlayedEventIds.emplace(eventId).second)
		return false;
	++m_iPlayCount;
	const std::string line = "[EffectCue] EventId=" +
		std::to_string(eventId) + " AssetId=" +
		std::to_string(cue.iEffectAssetId) + "\n";
	OutputDebugStringA(line.c_str());
	return true;
}

std::uint32_t Client::CDebugEffectCueSink::Get_PlayCount() const
{
	return m_iPlayCount;
}
```

```cpp
// FILE: Client/Public/ClientReplicationEvent.h
// ADD ENUMERATORS AND MEMBERS

PLAYER_COMBAT_SNAPSHOT,
GAMEPLAY_EVENT,

LostArk::Shared::S2C_PLAYER_COMBAT_SNAPSHOT PlayerCombatSnapshot;
LostArk::Shared::S2C_GAMEPLAY_EVENT GameplayEvent;
```

```cpp
// FILE: Client/Private/NetworkManager.cpp
// ADD INCLUDES AND Handle_Frame() CASES

#include "Gameplay/GameplayMessages.h"
#include "Gameplay/SkillMessages.h"

case PACKET_TYPE::S2C_PLAYER_COMBAT_SNAPSHOT:
{
	S2C_PLAYER_COMBAT_SNAPSHOT snapshot{};
	if (!Read_Message(reader, snapshot) || 0 != reader.Get_RemainingSize())
	{
		m_iLastErrorCode.store(WSAEINVAL); return;
	}
	Client::CLIENT_REPLICATION_EVENT event{};
	event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::PLAYER_COMBAT_SNAPSHOT;
	event.PlayerCombatSnapshot = std::move(snapshot);
	m_ReplicationEvents.push_back(std::move(event));
	break;
}

case PACKET_TYPE::S2C_GAMEPLAY_EVENT:
{
	S2C_GAMEPLAY_EVENT message{};
	if (!Read_Message(reader, message) || 0 != reader.Get_RemainingSize())
	{
		m_iLastErrorCode.store(WSAEINVAL); return;
	}
	Client::CLIENT_REPLICATION_EVENT event{};
	event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::GAMEPLAY_EVENT;
	event.GameplayEvent = std::move(message);
	m_ReplicationEvents.push_back(std::move(event));
	break;
}
```

```cpp
// FILE: Client/Public/ClientReplication.h
// ADD INCLUDES, PUBLIC API, PRIVATE FUNCTIONS, MEMBERS

#include "EffectCueSink.h"
#include "PlayerCombatViewModel.h"

void Set_EffectCueSink(IEffectCueSink* sink);
const CPlayerCombatViewModel& Get_PlayerCombatViewModel() const;

bool Apply_PlayerCombatSnapshot(
	const LostArk::Shared::S2C_PLAYER_COMBAT_SNAPSHOT& snapshot);
bool Apply_GameplayEvent(
	const LostArk::Shared::S2C_GAMEPLAY_EVENT& message);

CPlayerCombatViewModel m_PlayerCombatViewModel;
IEffectCueSink* m_pEffectCueSink = nullptr;
std::uint32_t m_iLastGameplayEventId = 0;
```

```cpp
// FILE: Client/Private/ClientReplication.cpp
// ADD TO Update() SWITCH

case CLIENT_REPLICATION_EVENT_TYPE::PLAYER_COMBAT_SNAPSHOT:
	allSucceeded = Apply_PlayerCombatSnapshot(
		event.PlayerCombatSnapshot) && allSucceeded;
	break;
case CLIENT_REPLICATION_EVENT_TYPE::GAMEPLAY_EVENT:
	allSucceeded = Apply_GameplayEvent(
		event.GameplayEvent) && allSucceeded;
	break;
```

```cpp
// FILE: Client/Private/ClientReplication.cpp
// APPEND

void Client::CClientReplication::Set_EffectCueSink(IEffectCueSink* sink)
{
	m_pEffectCueSink = sink;
}

const Client::CPlayerCombatViewModel&
Client::CClientReplication::Get_PlayerCombatViewModel() const
{
	return m_PlayerCombatViewModel;
}

bool Client::CClientReplication::Apply_PlayerCombatSnapshot(
	const LostArk::Shared::S2C_PLAYER_COMBAT_SNAPSHOT& snapshot)
{
	return m_PlayerCombatViewModel.Apply(
		snapshot,
		CNetworkManager::Get().Get_LocalEntityId());
}

bool Client::CClientReplication::Apply_GameplayEvent(
	const LostArk::Shared::S2C_GAMEPLAY_EVENT& message)
{
	using namespace LostArk::Shared;
	const GAMEPLAY_EVENT& event = message.Event;
	if (0 == event.iEventId || event.iEventId <= m_iLastGameplayEventId)
		return false;

	bool succeeded = true;
	if (const auto* action = std::get_if<ACTION_STARTED_EVENT>(&event.Payload))
	{
		OBJECT_HANDLE handle{};
		if (m_Registry.Find_Handle(action->iSourceNetEntityId, handle))
		{
			if (const auto character = m_Registry.Resolve(handle))
				succeeded = character->Play_Skill(
					static_cast<int32_t>(action->iActionId));
		}
	}
	else if (const auto* cue = std::get_if<EFFECT_CUE>(&event.Payload))
	{
		succeeded = nullptr != m_pEffectCueSink &&
			m_pEffectCueSink->Play(event.iEventId, *cue);
	}

	m_iLastGameplayEventId = event.iEventId;
	return succeeded;
}

// ADD TO Reset_World()
m_PlayerCombatViewModel.Reset();
m_iLastGameplayEventId = 0;
```

```cpp
// FILE: Client/Public/PlayerController.h
// ADD MEMBER

std::uint32_t m_iNextSkillSequence = 1;
```

```cpp
// FILE: Client/Private/PlayerController.cpp
// ADD TO Update() AFTER MOVE INPUT

if (!CGameInstance::Get().IsKeyboardInputBlocked() &&
	CGameInstance::Get().Get_DIKeyPressed(DIK_Q) &&
	nullptr != character &&
	nullptr != m_pCommandSink)
{
	const auto transform = character->Get_Transform();
	if (nullptr != transform)
	{
		float3_t aim{};
		XMStoreFloat3(&aim, transform->Get_State(STATE::POSITION));
		Try_PickGroundPlane(aim.y, aim);
		LostArk::Shared::USE_SKILL_COMMAND command{};
		command.iSequence = m_iNextSkillSequence;
		command.iSkillId = 1001;
		command.fAimX = aim.x;
		command.fAimY = aim.y;
		command.fAimZ = aim.z;
		if (m_pCommandSink->Submit_UseSkill(command))
		{
			++m_iNextSkillSequence;
			if (0 == m_iNextSkillSequence) m_iNextSkillSequence = 1;
		}
	}
}
```

```cpp
// FILE: Client/Public/Level_Baren.h
// ADD MEMBER

CDebugEffectCueSink m_EffectCueSink;
```

```cpp
// FILE: Client/Private/Level_Baren.cpp
// ADD AFTER REPLICATION INITIALIZE

m_Replication.Set_EffectCueSink(&m_EffectCueSink);
```

```xml
<!-- FILE: Shared/Default/Shared.vcxproj -->
<ClInclude Include="..\Public\Gameplay\SkillContracts.h" />
<ClInclude Include="..\Public\Gameplay\SkillMessages.h" />
<ClCompile Include="..\Private\Gameplay\SkillMessages.cpp" />
```

```xml
<!-- FILE: Server/Default/Server.vcxproj -->
<ClInclude Include="..\Public\SkillSystem.h" />
<ClCompile Include="..\Private\SkillSystem.cpp" />
```

```xml
<!-- FILE: Client/Default/Client.vcxproj -->
<ClInclude Include="..\Public\PlayerCombatViewModel.h" />
<ClInclude Include="..\Public\EffectCueSink.h" />
<ClCompile Include="..\Private\PlayerCombatViewModel.cpp" />
<ClCompile Include="..\Private\EffectCueSink.cpp" />
```

```xml
<!-- FILE: Shared/Default/Shared.vcxproj.filters -->
<ClInclude Include="..\Public\Gameplay\SkillContracts.h">
  <Filter>Public\Gameplay</Filter>
</ClInclude>
<ClInclude Include="..\Public\Gameplay\SkillMessages.h">
  <Filter>Public\Gameplay</Filter>
</ClInclude>
<ClCompile Include="..\Private\Gameplay\SkillMessages.cpp">
  <Filter>Private\Gameplay</Filter>
</ClCompile>

<!-- FILE: Server/Default/Server.vcxproj.filters -->
<ClInclude Include="..\Public\SkillSystem.h"><Filter>Public</Filter></ClInclude>
<ClCompile Include="..\Private\SkillSystem.cpp"><Filter>Private</Filter></ClCompile>

<!-- FILE: Client/Default/Client.vcxproj.filters -->
<ClInclude Include="..\Public\PlayerCombatViewModel.h"><Filter>04. Network</Filter></ClInclude>
<ClInclude Include="..\Public\EffectCueSink.h"><Filter>04. Network</Filter></ClInclude>
<ClCompile Include="..\Private\PlayerCombatViewModel.cpp"><Filter>04. Network</Filter></ClCompile>
<ClCompile Include="..\Private\EffectCueSink.cpp"><Filter>04. Network</Filter></ClCompile>
```

```powershell
$msbuild = "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
& $msbuild Shared\Default\Shared.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
& $msbuild Tools\NetworkProtocolHarness\Default\NetworkProtocolHarness.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
& Tools\NetworkProtocolHarness\Bin\Debug\NetworkProtocolHarness.exe
& $msbuild Server\Default\Server.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
& $msbuild Client\Default\Client.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
```

```text
Q -> UseSkillCommand(sequence=1, skillId=1001, aim)
Server SessionId -> source player
accepted cast -> resource 100 to 90, cooldownEndTick=serverTick+90
ActionStarted exactly once on Client A and B
EffectCue eventId exactly once on Client A and B
valid target -> DamageApplied and identical remainingHp
recast before cooldown -> no ActionStarted, DamageApplied, EffectCue
PlayerCombatViewModel remaining cooldown converges to zero
```
