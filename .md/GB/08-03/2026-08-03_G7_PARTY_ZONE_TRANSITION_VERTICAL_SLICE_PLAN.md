```cpp
// FILE: Shared/Public/Party/PartyContracts.h

#pragma once

#include "Network/NetworkIds.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace LostArk::Shared
{
	using PARTY_ID = std::uint32_t;
	using PARTY_INVITE_ID = std::uint32_t;
	using ZONE_TRANSITION_ID = std::uint32_t;

	inline constexpr PARTY_ID INVALID_PARTY_ID = 0;
	inline constexpr PARTY_INVITE_ID INVALID_PARTY_INVITE_ID = 0;
	inline constexpr ZONE_TRANSITION_ID INVALID_ZONE_TRANSITION_ID = 0;
	inline constexpr std::size_t MAX_PARTY_MEMBERS = 4;

	enum class ZONE_ID : std::uint8_t
	{
		BAREN,
		CHAOS_DUNGEON,
		VALTAN_ARENA,
		END
	};

	struct PARTY_MEMBER_SNAPSHOT
	{
		PLAYER_ID iPlayerId = INVALID_PLAYER_ID;
		std::string strNickname;
		bool isOnline = false;
	};

	struct PARTY_STATE_SNAPSHOT
	{
		PARTY_ID iPartyId = INVALID_PARTY_ID;
		PLAYER_ID iLeaderPlayerId = INVALID_PLAYER_ID;
		std::vector<PARTY_MEMBER_SNAPSHOT> Members;
	};

	struct PARTY_INVITE_COMMAND
	{
		PLAYER_ID iTargetPlayerId = INVALID_PLAYER_ID;
	};

	struct PARTY_INVITE_RESPONSE_COMMAND
	{
		PARTY_INVITE_ID iInviteId = INVALID_PARTY_INVITE_ID;
		bool isAccepted = false;
	};

	struct PARTY_LEAVE_COMMAND {};

	struct START_ZONE_TRANSITION_COMMAND
	{
		ZONE_ID eZoneId = ZONE_ID::END;
	};

	struct ZONE_READY_COMMAND
	{
		ZONE_TRANSITION_ID iTransitionId = INVALID_ZONE_TRANSITION_ID;
		bool isReady = false;
	};

	using PARTY_COMMAND_PAYLOAD = std::variant<
		PARTY_INVITE_COMMAND,
		PARTY_INVITE_RESPONSE_COMMAND,
		PARTY_LEAVE_COMMAND,
		START_ZONE_TRANSITION_COMMAND,
		ZONE_READY_COMMAND>;

	struct C2S_PARTY_COMMAND
	{
		PARTY_COMMAND_PAYLOAD Payload = PARTY_LEAVE_COMMAND{};
	};

	struct PARTY_INVITE_EVENT
	{
		PARTY_INVITE_ID iInviteId = INVALID_PARTY_INVITE_ID;
		PLAYER_ID iInviterPlayerId = INVALID_PLAYER_ID;
		std::string strInviterNickname;
	};

	struct PARTY_STATE_EVENT
	{
		PARTY_STATE_SNAPSHOT State;
	};

	struct ZONE_TRANSITION_BEGIN_EVENT
	{
		ZONE_TRANSITION_ID iTransitionId = INVALID_ZONE_TRANSITION_ID;
		PARTY_ID iPartyId = INVALID_PARTY_ID;
		ZONE_ID eZoneId = ZONE_ID::END;
	};

	struct ZONE_TRANSITION_COMMIT_EVENT
	{
		ZONE_TRANSITION_ID iTransitionId = INVALID_ZONE_TRANSITION_ID;
		ZONE_ID eZoneId = ZONE_ID::END;
	};

	enum class ZONE_TRANSITION_ABORT_REASON : std::uint8_t
	{
		MEMBER_REJECTED,
		MEMBER_DISCONNECTED,
		LOAD_FAILED,
		TIMED_OUT,
		END
	};

	struct ZONE_TRANSITION_ABORT_EVENT
	{
		ZONE_TRANSITION_ID iTransitionId = INVALID_ZONE_TRANSITION_ID;
		ZONE_TRANSITION_ABORT_REASON eReason =
			ZONE_TRANSITION_ABORT_REASON::END;
	};

	using PARTY_EVENT_PAYLOAD = std::variant<
		PARTY_INVITE_EVENT,
		PARTY_STATE_EVENT,
		ZONE_TRANSITION_BEGIN_EVENT,
		ZONE_TRANSITION_COMMIT_EVENT,
		ZONE_TRANSITION_ABORT_EVENT>;

	struct S2C_PARTY_EVENT
	{
		std::uint32_t iEventId = 0;
		PARTY_EVENT_PAYLOAD Payload = PARTY_STATE_EVENT{};
	};
}
```

```cpp
// FILE: Shared/Public/Party/PartyMessages.h

#pragma once

#include "Party/PartyContracts.h"

namespace LostArk::Shared
{
	class CPacketReader;
	class CPacketWriter;

	bool Write_Message(CPacketWriter& writer, const C2S_PARTY_COMMAND& message);
	bool Read_Message(CPacketReader& reader, C2S_PARTY_COMMAND& message);
	bool Write_Message(CPacketWriter& writer, const S2C_PARTY_EVENT& message);
	bool Read_Message(CPacketReader& reader, S2C_PARTY_EVENT& message);
}
```

```cpp
// FILE: Shared/Public/Network/PacketType.h
// ADD ENUMERATORS AND Is_Known_Packet_Type() CASES

C2S_PARTY_COMMAND,
S2C_PARTY_EVENT,
```

```cpp
// FILE: Shared/Private/Party/PartyMessages.cpp

#include "Party/PartyMessages.h"

#include "Network/PacketMessages.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <type_traits>
#include <utility>

namespace
{
	using namespace LostArk::Shared;

	enum class COMMAND_WIRE_TYPE : std::uint8_t
	{
		INVITE, INVITE_RESPONSE, LEAVE, START_TRANSITION, READY, END
	};

	enum class EVENT_WIRE_TYPE : std::uint8_t
	{
		INVITE, PARTY_STATE, TRANSITION_BEGIN, TRANSITION_COMMIT,
		TRANSITION_ABORT, END
	};

	bool Is_Valid_Zone(ZONE_ID value)
	{
		return static_cast<std::uint8_t>(value) <
			static_cast<std::uint8_t>(ZONE_ID::END);
	}

	bool Write_PartyState(CPacketWriter& writer, const PARTY_STATE_SNAPSHOT& state)
	{
		if (state.iPartyId == INVALID_PARTY_ID)
		{
			if (state.iLeaderPlayerId != INVALID_PLAYER_ID || !state.Members.empty())
				return false;
			writer.Write_U32(INVALID_PARTY_ID);
			writer.Write_U32(INVALID_PLAYER_ID);
			writer.Write_U8(0);
			return true;
		}

		if (state.iLeaderPlayerId == INVALID_PLAYER_ID ||
			state.Members.empty() || state.Members.size() > MAX_PARTY_MEMBERS)
			return false;

		bool foundLeader = false;
		for (const auto& member : state.Members)
		{
			if (member.iPlayerId == INVALID_PLAYER_ID ||
				member.strNickname.empty() ||
				member.strNickname.size() > MAX_NICKNAME_BYTES)
				return false;
			foundLeader = foundLeader || member.iPlayerId == state.iLeaderPlayerId;
		}
		if (!foundLeader) return false;

		writer.Write_U32(state.iPartyId);
		writer.Write_U32(state.iLeaderPlayerId);
		writer.Write_U8(static_cast<std::uint8_t>(state.Members.size()));
		for (const auto& member : state.Members)
		{
			writer.Write_U32(member.iPlayerId);
			if (!writer.Write_String(member.strNickname, MAX_NICKNAME_BYTES)) return false;
			writer.Write_U8(member.isOnline ? 1u : 0u);
		}
		return true;
	}

	bool Read_PartyState(CPacketReader& reader, PARTY_STATE_SNAPSHOT& state)
	{
		PARTY_STATE_SNAPSHOT decoded{};
		std::uint8_t count = 0;
		if (!reader.Read_U32(decoded.iPartyId) ||
			!reader.Read_U32(decoded.iLeaderPlayerId) ||
			!reader.Read_U8(count) || count > MAX_PARTY_MEMBERS)
			return false;

		decoded.Members.reserve(count);
		for (std::uint8_t index = 0; index < count; ++index)
		{
			PARTY_MEMBER_SNAPSHOT member{};
			std::uint8_t online = 0;
			if (!reader.Read_U32(member.iPlayerId) ||
				!reader.Read_String(member.strNickname, MAX_NICKNAME_BYTES) ||
				!reader.Read_U8(online) || online > 1u)
				return false;
			member.isOnline = 1u == online;
			decoded.Members.push_back(std::move(member));
		}

		CPacketWriter validator;
		if (!Write_PartyState(validator, decoded)) return false;
		state = std::move(decoded);
		return true;
	}
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer, const C2S_PARTY_COMMAND& message)
{
	return std::visit([&writer](const auto& value) -> bool
	{
		using T = std::decay_t<decltype(value)>;
		if constexpr (std::is_same_v<T, PARTY_INVITE_COMMAND>)
		{
			if (value.iTargetPlayerId == INVALID_PLAYER_ID) return false;
			writer.Write_U8(static_cast<std::uint8_t>(COMMAND_WIRE_TYPE::INVITE));
			writer.Write_U32(value.iTargetPlayerId);
		}
		else if constexpr (std::is_same_v<T, PARTY_INVITE_RESPONSE_COMMAND>)
		{
			if (value.iInviteId == INVALID_PARTY_INVITE_ID) return false;
			writer.Write_U8(static_cast<std::uint8_t>(COMMAND_WIRE_TYPE::INVITE_RESPONSE));
			writer.Write_U32(value.iInviteId);
			writer.Write_U8(value.isAccepted ? 1u : 0u);
		}
		else if constexpr (std::is_same_v<T, PARTY_LEAVE_COMMAND>)
		{
			writer.Write_U8(static_cast<std::uint8_t>(COMMAND_WIRE_TYPE::LEAVE));
		}
		else if constexpr (std::is_same_v<T, START_ZONE_TRANSITION_COMMAND>)
		{
			if (!Is_Valid_Zone(value.eZoneId)) return false;
			writer.Write_U8(static_cast<std::uint8_t>(COMMAND_WIRE_TYPE::START_TRANSITION));
			writer.Write_U8(static_cast<std::uint8_t>(value.eZoneId));
		}
		else
		{
			if (value.iTransitionId == INVALID_ZONE_TRANSITION_ID) return false;
			writer.Write_U8(static_cast<std::uint8_t>(COMMAND_WIRE_TYPE::READY));
			writer.Write_U32(value.iTransitionId);
			writer.Write_U8(value.isReady ? 1u : 0u);
		}
		return true;
	}, message.Payload);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader, C2S_PARTY_COMMAND& message)
{
	std::uint8_t rawType = 0;
	if (!reader.Read_U8(rawType) || rawType >= static_cast<std::uint8_t>(COMMAND_WIRE_TYPE::END))
		return false;

	C2S_PARTY_COMMAND decoded{};
	switch (static_cast<COMMAND_WIRE_TYPE>(rawType))
	{
	case COMMAND_WIRE_TYPE::INVITE:
	{
		PARTY_INVITE_COMMAND value{};
		if (!reader.Read_U32(value.iTargetPlayerId) || value.iTargetPlayerId == INVALID_PLAYER_ID) return false;
		decoded.Payload = value; break;
	}
	case COMMAND_WIRE_TYPE::INVITE_RESPONSE:
	{
		PARTY_INVITE_RESPONSE_COMMAND value{}; std::uint8_t accepted = 0;
		if (!reader.Read_U32(value.iInviteId) || !reader.Read_U8(accepted) ||
			value.iInviteId == INVALID_PARTY_INVITE_ID || accepted > 1u) return false;
		value.isAccepted = 1u == accepted; decoded.Payload = value; break;
	}
	case COMMAND_WIRE_TYPE::LEAVE: decoded.Payload = PARTY_LEAVE_COMMAND{}; break;
	case COMMAND_WIRE_TYPE::START_TRANSITION:
	{
		std::uint8_t zone = 0; if (!reader.Read_U8(zone) || zone >= static_cast<std::uint8_t>(ZONE_ID::END)) return false;
		START_ZONE_TRANSITION_COMMAND value{}; value.eZoneId = static_cast<ZONE_ID>(zone); decoded.Payload = value; break;
	}
	case COMMAND_WIRE_TYPE::READY:
	{
		ZONE_READY_COMMAND value{}; std::uint8_t ready = 0;
		if (!reader.Read_U32(value.iTransitionId) || !reader.Read_U8(ready) ||
			value.iTransitionId == INVALID_ZONE_TRANSITION_ID || ready > 1u) return false;
		value.isReady = 1u == ready; decoded.Payload = value; break;
	}
	default: return false;
	}
	message = std::move(decoded); return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer, const S2C_PARTY_EVENT& message)
{
	if (0 == message.iEventId) return false;
	writer.Write_U32(message.iEventId);
	return std::visit([&writer](const auto& value) -> bool
	{
		using T = std::decay_t<decltype(value)>;
		if constexpr (std::is_same_v<T, PARTY_INVITE_EVENT>)
		{
			if (value.iInviteId == INVALID_PARTY_INVITE_ID ||
				value.iInviterPlayerId == INVALID_PLAYER_ID ||
				value.strInviterNickname.empty()) return false;
			writer.Write_U8(static_cast<std::uint8_t>(EVENT_WIRE_TYPE::INVITE));
			writer.Write_U32(value.iInviteId); writer.Write_U32(value.iInviterPlayerId);
			return writer.Write_String(value.strInviterNickname, MAX_NICKNAME_BYTES);
		}
		else if constexpr (std::is_same_v<T, PARTY_STATE_EVENT>)
		{
			writer.Write_U8(static_cast<std::uint8_t>(EVENT_WIRE_TYPE::PARTY_STATE));
			return Write_PartyState(writer, value.State);
		}
		else if constexpr (std::is_same_v<T, ZONE_TRANSITION_BEGIN_EVENT>)
		{
			if (value.iTransitionId == INVALID_ZONE_TRANSITION_ID ||
				value.iPartyId == INVALID_PARTY_ID || !Is_Valid_Zone(value.eZoneId)) return false;
			writer.Write_U8(static_cast<std::uint8_t>(EVENT_WIRE_TYPE::TRANSITION_BEGIN));
			writer.Write_U32(value.iTransitionId); writer.Write_U32(value.iPartyId);
			writer.Write_U8(static_cast<std::uint8_t>(value.eZoneId)); return true;
		}
		else if constexpr (std::is_same_v<T, ZONE_TRANSITION_COMMIT_EVENT>)
		{
			if (value.iTransitionId == INVALID_ZONE_TRANSITION_ID || !Is_Valid_Zone(value.eZoneId)) return false;
			writer.Write_U8(static_cast<std::uint8_t>(EVENT_WIRE_TYPE::TRANSITION_COMMIT));
			writer.Write_U32(value.iTransitionId); writer.Write_U8(static_cast<std::uint8_t>(value.eZoneId)); return true;
		}
		else
		{
			if (value.iTransitionId == INVALID_ZONE_TRANSITION_ID ||
				static_cast<std::uint8_t>(value.eReason) >= static_cast<std::uint8_t>(ZONE_TRANSITION_ABORT_REASON::END)) return false;
			writer.Write_U8(static_cast<std::uint8_t>(EVENT_WIRE_TYPE::TRANSITION_ABORT));
			writer.Write_U32(value.iTransitionId); writer.Write_U8(static_cast<std::uint8_t>(value.eReason)); return true;
		}
	}, message.Payload);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader, S2C_PARTY_EVENT& message)
{
	S2C_PARTY_EVENT decoded{}; std::uint8_t rawType = 0;
	if (!reader.Read_U32(decoded.iEventId) || !reader.Read_U8(rawType) ||
		0 == decoded.iEventId || rawType >= static_cast<std::uint8_t>(EVENT_WIRE_TYPE::END)) return false;

	switch (static_cast<EVENT_WIRE_TYPE>(rawType))
	{
	case EVENT_WIRE_TYPE::INVITE:
	{
		PARTY_INVITE_EVENT value{};
		if (!reader.Read_U32(value.iInviteId) || !reader.Read_U32(value.iInviterPlayerId) ||
			!reader.Read_String(value.strInviterNickname, MAX_NICKNAME_BYTES)) return false;
		decoded.Payload = std::move(value); break;
	}
	case EVENT_WIRE_TYPE::PARTY_STATE:
	{
		PARTY_STATE_EVENT value{}; if (!Read_PartyState(reader, value.State)) return false;
		decoded.Payload = std::move(value); break;
	}
	case EVENT_WIRE_TYPE::TRANSITION_BEGIN:
	{
		ZONE_TRANSITION_BEGIN_EVENT value{}; std::uint8_t zone = 0;
		if (!reader.Read_U32(value.iTransitionId) || !reader.Read_U32(value.iPartyId) || !reader.Read_U8(zone) ||
			zone >= static_cast<std::uint8_t>(ZONE_ID::END)) return false;
		value.eZoneId = static_cast<ZONE_ID>(zone); decoded.Payload = value; break;
	}
	case EVENT_WIRE_TYPE::TRANSITION_COMMIT:
	{
		ZONE_TRANSITION_COMMIT_EVENT value{}; std::uint8_t zone = 0;
		if (!reader.Read_U32(value.iTransitionId) || !reader.Read_U8(zone) || zone >= static_cast<std::uint8_t>(ZONE_ID::END)) return false;
		value.eZoneId = static_cast<ZONE_ID>(zone); decoded.Payload = value; break;
	}
	case EVENT_WIRE_TYPE::TRANSITION_ABORT:
	{
		ZONE_TRANSITION_ABORT_EVENT value{}; std::uint8_t reason = 0;
		if (!reader.Read_U32(value.iTransitionId) || !reader.Read_U8(reason) ||
			reason >= static_cast<std::uint8_t>(ZONE_TRANSITION_ABORT_REASON::END)) return false;
		value.eReason = static_cast<ZONE_TRANSITION_ABORT_REASON>(reason); decoded.Payload = value; break;
	}
	default: return false;
	}

	CPacketWriter validator; if (!Write_Message(validator, decoded)) return false;
	message = std::move(decoded); return true;
}
```

```cpp
// FILE: Server/Public/PartySystem.h

#pragma once

#include "Party/PartyContracts.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace LostArk::Server
{
	struct PARTY_PLAYER_VIEW
	{
		LostArk::Shared::PLAYER_ID iPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
		std::string strNickname;
		bool isOnline = false;
	};

	struct PARTY_DELIVERY
	{
		std::vector<LostArk::Shared::PLAYER_ID> Recipients;
		LostArk::Shared::PARTY_EVENT_PAYLOAD Payload;
	};

	class CPartySystem final
	{
	public:
		bool Handle_Command(
			LostArk::Shared::PLAYER_ID senderPlayerId,
			const LostArk::Shared::PARTY_COMMAND_PAYLOAD& command,
			const std::unordered_map<LostArk::Shared::PLAYER_ID, PARTY_PLAYER_VIEW>& players,
			std::uint32_t serverTick,
			std::vector<PARTY_DELIVERY>& deliveries);
		void On_PlayerDisconnected(
			LostArk::Shared::PLAYER_ID playerId,
			const std::unordered_map<LostArk::Shared::PLAYER_ID, PARTY_PLAYER_VIEW>& players,
			std::vector<PARTY_DELIVERY>& deliveries);
		void Tick(std::uint32_t serverTick, std::vector<PARTY_DELIVERY>& deliveries);
		bool Is_Leader(LostArk::Shared::PLAYER_ID playerId) const;

	private:
		struct PARTY_RECORD
		{
			LostArk::Shared::PARTY_ID iPartyId = LostArk::Shared::INVALID_PARTY_ID;
			LostArk::Shared::PLAYER_ID iLeaderPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
			std::vector<LostArk::Shared::PLAYER_ID> Members;
		};

		struct INVITE_RECORD
		{
			LostArk::Shared::PARTY_INVITE_ID iInviteId = LostArk::Shared::INVALID_PARTY_INVITE_ID;
			LostArk::Shared::PLAYER_ID iInviterPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
			LostArk::Shared::PLAYER_ID iTargetPlayerId = LostArk::Shared::INVALID_PLAYER_ID;
		};

		struct TRANSITION_RECORD
		{
			LostArk::Shared::ZONE_TRANSITION_ID iTransitionId = LostArk::Shared::INVALID_ZONE_TRANSITION_ID;
			LostArk::Shared::PARTY_ID iPartyId = LostArk::Shared::INVALID_PARTY_ID;
			LostArk::Shared::ZONE_ID eZoneId = LostArk::Shared::ZONE_ID::END;
			std::uint32_t iDeadlineTick = 0;
			std::unordered_map<LostArk::Shared::PLAYER_ID, bool> ReadyByPlayerId;
		};

		PARTY_RECORD* Find_PartyByPlayer(LostArk::Shared::PLAYER_ID playerId);
		PARTY_RECORD* Find_Party(LostArk::Shared::PARTY_ID partyId);
		void Emit_State(const PARTY_RECORD& party,
			const std::unordered_map<LostArk::Shared::PLAYER_ID, PARTY_PLAYER_VIEW>& players,
			std::vector<PARTY_DELIVERY>& deliveries) const;
		void Abort_Transition(LostArk::Shared::ZONE_TRANSITION_ABORT_REASON reason,
			std::vector<PARTY_DELIVERY>& deliveries);

	private:
		std::unordered_map<LostArk::Shared::PARTY_ID, PARTY_RECORD> m_Parties;
		std::unordered_map<LostArk::Shared::PARTY_INVITE_ID, INVITE_RECORD> m_Invites;
		TRANSITION_RECORD m_Transition;
		LostArk::Shared::PARTY_ID m_iNextPartyId = 1;
		LostArk::Shared::PARTY_INVITE_ID m_iNextInviteId = 1;
		LostArk::Shared::ZONE_TRANSITION_ID m_iNextTransitionId = 1;
	};
}
```

```cpp
// FILE: Server/Private/PartySystem.cpp

#include "PartySystem.h"

#include <algorithm>
#include <type_traits>

LostArk::Server::CPartySystem::PARTY_RECORD*
LostArk::Server::CPartySystem::Find_PartyByPlayer(
	LostArk::Shared::PLAYER_ID playerId)
{
	for (auto& [partyId, party] : m_Parties)
	{
		(void)partyId;
		if (std::find(party.Members.begin(), party.Members.end(), playerId) !=
			party.Members.end()) return &party;
	}
	return nullptr;
}

LostArk::Server::CPartySystem::PARTY_RECORD*
LostArk::Server::CPartySystem::Find_Party(LostArk::Shared::PARTY_ID partyId)
{
	const auto iter = m_Parties.find(partyId);
	return iter == m_Parties.end() ? nullptr : &iter->second;
}

void LostArk::Server::CPartySystem::Emit_State(
	const PARTY_RECORD& party,
	const std::unordered_map<LostArk::Shared::PLAYER_ID, PARTY_PLAYER_VIEW>& players,
	std::vector<PARTY_DELIVERY>& deliveries) const
{
	LostArk::Shared::PARTY_STATE_EVENT stateEvent{};
	stateEvent.State.iPartyId = party.iPartyId;
	stateEvent.State.iLeaderPlayerId = party.iLeaderPlayerId;
	for (const LostArk::Shared::PLAYER_ID memberId : party.Members)
	{
		const auto playerIter = players.find(memberId);
		if (playerIter == players.end()) continue;
		LostArk::Shared::PARTY_MEMBER_SNAPSHOT member{};
		member.iPlayerId = memberId;
		member.strNickname = playerIter->second.strNickname;
		member.isOnline = playerIter->second.isOnline;
		stateEvent.State.Members.push_back(std::move(member));
	}
	PARTY_DELIVERY delivery{};
	delivery.Recipients = party.Members;
	delivery.Payload = std::move(stateEvent);
	deliveries.push_back(std::move(delivery));
}

void LostArk::Server::CPartySystem::Abort_Transition(
	LostArk::Shared::ZONE_TRANSITION_ABORT_REASON reason,
	std::vector<PARTY_DELIVERY>& deliveries)
{
	using namespace LostArk::Shared;
	if (m_Transition.iTransitionId == INVALID_ZONE_TRANSITION_ID) return;
	PARTY_DELIVERY delivery{};
	for (const auto& [playerId, ready] : m_Transition.ReadyByPlayerId)
	{
		(void)ready;
		delivery.Recipients.push_back(playerId);
	}
	ZONE_TRANSITION_ABORT_EVENT event{};
	event.iTransitionId = m_Transition.iTransitionId;
	event.eReason = reason;
	delivery.Payload = event;
	deliveries.push_back(std::move(delivery));
	m_Transition = {};
}

bool LostArk::Server::CPartySystem::Handle_Command(
	LostArk::Shared::PLAYER_ID senderPlayerId,
	const LostArk::Shared::PARTY_COMMAND_PAYLOAD& command,
	const std::unordered_map<LostArk::Shared::PLAYER_ID, PARTY_PLAYER_VIEW>& players,
	std::uint32_t serverTick,
	std::vector<PARTY_DELIVERY>& deliveries)
{
	using namespace LostArk::Shared;
	deliveries.clear();
	const auto senderView = players.find(senderPlayerId);
	if (senderPlayerId == INVALID_PLAYER_ID || senderView == players.end() ||
		!senderView->second.isOnline) return false;

	if (const auto* invite = std::get_if<PARTY_INVITE_COMMAND>(&command))
	{
		const auto targetView = players.find(invite->iTargetPlayerId);
		PARTY_RECORD* senderParty = Find_PartyByPlayer(senderPlayerId);
		if (invite->iTargetPlayerId == senderPlayerId || targetView == players.end() ||
			!targetView->second.isOnline || nullptr != Find_PartyByPlayer(invite->iTargetPlayerId) ||
			(nullptr != senderParty && senderParty->Members.size() >= MAX_PARTY_MEMBERS)) return true;

		if (0 == m_iNextInviteId) ++m_iNextInviteId;
		INVITE_RECORD record{};
		record.iInviteId = m_iNextInviteId++;
		record.iInviterPlayerId = senderPlayerId;
		record.iTargetPlayerId = invite->iTargetPlayerId;
		m_Invites.emplace(record.iInviteId, record);

		PARTY_INVITE_EVENT event{};
		event.iInviteId = record.iInviteId;
		event.iInviterPlayerId = senderPlayerId;
		event.strInviterNickname = senderView->second.strNickname;
		PARTY_DELIVERY delivery{};
		delivery.Recipients.push_back(record.iTargetPlayerId);
		delivery.Payload = std::move(event);
		deliveries.push_back(std::move(delivery));
		return true;
	}

	if (const auto* response = std::get_if<PARTY_INVITE_RESPONSE_COMMAND>(&command))
	{
		const auto inviteIter = m_Invites.find(response->iInviteId);
		if (inviteIter == m_Invites.end() || inviteIter->second.iTargetPlayerId != senderPlayerId)
			return true;
		const INVITE_RECORD invite = inviteIter->second;
		m_Invites.erase(inviteIter);
		if (!response->isAccepted) return true;
		const auto inviterView = players.find(invite.iInviterPlayerId);
		if (inviterView == players.end() || !inviterView->second.isOnline ||
			nullptr != Find_PartyByPlayer(senderPlayerId)) return true;

		PARTY_RECORD* party = Find_PartyByPlayer(invite.iInviterPlayerId);
		if (nullptr == party)
		{
			if (0 == m_iNextPartyId) ++m_iNextPartyId;
			PARTY_RECORD created{};
			created.iPartyId = m_iNextPartyId++;
			created.iLeaderPlayerId = invite.iInviterPlayerId;
			created.Members.push_back(invite.iInviterPlayerId);
			const PARTY_ID partyId = created.iPartyId;
			m_Parties.emplace(partyId, std::move(created));
			party = Find_Party(partyId);
		}
		if (nullptr == party || party->Members.size() >= MAX_PARTY_MEMBERS) return true;
		party->Members.push_back(senderPlayerId);
		Emit_State(*party, players, deliveries);
		return true;
	}

	if (std::holds_alternative<PARTY_LEAVE_COMMAND>(command))
	{
		PARTY_RECORD* party = Find_PartyByPlayer(senderPlayerId);
		if (nullptr == party) return true;
		const PARTY_ID partyId = party->iPartyId;
		if (m_Transition.iPartyId == partyId)
			Abort_Transition(ZONE_TRANSITION_ABORT_REASON::MEMBER_REJECTED, deliveries);
		party = Find_Party(partyId);
		if (nullptr == party) return true;
		party->Members.erase(std::remove(
			party->Members.begin(), party->Members.end(), senderPlayerId),
			party->Members.end());
		PARTY_STATE_EVENT empty{};
		PARTY_DELIVERY leaveDelivery{};
		leaveDelivery.Recipients.push_back(senderPlayerId);
		leaveDelivery.Payload = empty;
		deliveries.push_back(std::move(leaveDelivery));
		if (party->Members.empty())
		{
			m_Parties.erase(partyId);
			return true;
		}
		if (party->iLeaderPlayerId == senderPlayerId)
			party->iLeaderPlayerId = party->Members.front();
		Emit_State(*party, players, deliveries);
		return true;
	}

	if (const auto* start = std::get_if<START_ZONE_TRANSITION_COMMAND>(&command))
	{
		PARTY_RECORD* party = Find_PartyByPlayer(senderPlayerId);
		if (nullptr == party || party->iLeaderPlayerId != senderPlayerId ||
			m_Transition.iTransitionId != INVALID_ZONE_TRANSITION_ID ||
			static_cast<std::uint8_t>(start->eZoneId) >= static_cast<std::uint8_t>(ZONE_ID::END))
			return true;
		if (0 == m_iNextTransitionId) ++m_iNextTransitionId;
		m_Transition.iTransitionId = m_iNextTransitionId++;
		m_Transition.iPartyId = party->iPartyId;
		m_Transition.eZoneId = start->eZoneId;
		m_Transition.iDeadlineTick = serverTick + 900;
		for (const PLAYER_ID memberId : party->Members)
			m_Transition.ReadyByPlayerId.emplace(memberId, false);
		ZONE_TRANSITION_BEGIN_EVENT event{};
		event.iTransitionId = m_Transition.iTransitionId;
		event.iPartyId = party->iPartyId;
		event.eZoneId = start->eZoneId;
		PARTY_DELIVERY delivery{};
		delivery.Recipients = party->Members;
		delivery.Payload = event;
		deliveries.push_back(std::move(delivery));
		return true;
	}

	const auto* ready = std::get_if<ZONE_READY_COMMAND>(&command);
	if (nullptr == ready || ready->iTransitionId != m_Transition.iTransitionId)
		return true;
	const auto readyIter = m_Transition.ReadyByPlayerId.find(senderPlayerId);
	if (readyIter == m_Transition.ReadyByPlayerId.end()) return true;
	if (!ready->isReady)
	{
		Abort_Transition(ZONE_TRANSITION_ABORT_REASON::LOAD_FAILED, deliveries);
		return true;
	}
	readyIter->second = true;
	const bool allReady = std::all_of(
		m_Transition.ReadyByPlayerId.begin(), m_Transition.ReadyByPlayerId.end(),
		[](const auto& pair) { return pair.second; });
	if (!allReady) return true;
	PARTY_DELIVERY delivery{};
	for (const auto& [playerId, isReady] : m_Transition.ReadyByPlayerId)
	{
		(void)isReady; delivery.Recipients.push_back(playerId);
	}
	ZONE_TRANSITION_COMMIT_EVENT event{};
	event.iTransitionId = m_Transition.iTransitionId;
	event.eZoneId = m_Transition.eZoneId;
	delivery.Payload = event;
	deliveries.push_back(std::move(delivery));
	m_Transition = {};
	return true;
}

void LostArk::Server::CPartySystem::On_PlayerDisconnected(
	LostArk::Shared::PLAYER_ID playerId,
	const std::unordered_map<LostArk::Shared::PLAYER_ID, PARTY_PLAYER_VIEW>& players,
	std::vector<PARTY_DELIVERY>& deliveries)
{
	using namespace LostArk::Shared;
	deliveries.clear();
	if (m_Transition.ReadyByPlayerId.contains(playerId))
		Abort_Transition(ZONE_TRANSITION_ABORT_REASON::MEMBER_DISCONNECTED, deliveries);
	PARTY_RECORD* party = Find_PartyByPlayer(playerId);
	if (nullptr == party) return;
	const PARTY_ID partyId = party->iPartyId;
	party->Members.erase(std::remove(party->Members.begin(), party->Members.end(), playerId),
		party->Members.end());
	if (party->Members.empty()) { m_Parties.erase(partyId); return; }
	if (party->iLeaderPlayerId == playerId) party->iLeaderPlayerId = party->Members.front();
	Emit_State(*party, players, deliveries);
}

void LostArk::Server::CPartySystem::Tick(
	std::uint32_t serverTick,
	std::vector<PARTY_DELIVERY>& deliveries)
{
	deliveries.clear();
	if (m_Transition.iTransitionId != LostArk::Shared::INVALID_ZONE_TRANSITION_ID &&
		serverTick >= m_Transition.iDeadlineTick)
	{
		Abort_Transition(LostArk::Shared::ZONE_TRANSITION_ABORT_REASON::TIMED_OUT,
			deliveries);
	}
}

bool LostArk::Server::CPartySystem::Is_Leader(
	LostArk::Shared::PLAYER_ID playerId) const
{
	for (const auto& [partyId, party] : m_Parties)
	{
		(void)partyId;
		if (party.iLeaderPlayerId == playerId) return true;
	}
	return false;
}
```

```cpp
// FILE: Client/Public/PartyViewModel.h

#pragma once

#include "Party/PartyContracts.h"

#include <cstdint>
#include <optional>

namespace Client
{
	class CPartyViewModel final
	{
	public:
		bool Apply(const LostArk::Shared::S2C_PARTY_EVENT& event);
		void Reset();
		const LostArk::Shared::PARTY_STATE_SNAPSHOT& Get_State() const;
		const std::optional<LostArk::Shared::PARTY_INVITE_EVENT>& Get_Invite() const;
	private:
		LostArk::Shared::PARTY_STATE_SNAPSHOT m_State;
		std::optional<LostArk::Shared::PARTY_INVITE_EVENT> m_Invite;
		std::uint32_t m_iLastEventId = 0;
	};
}
```

```cpp
// FILE: Client/Private/PartyViewModel.cpp

#include "PartyViewModel.h"

bool Client::CPartyViewModel::Apply(
	const LostArk::Shared::S2C_PARTY_EVENT& event)
{
	if (0 == event.iEventId || event.iEventId <= m_iLastEventId) return false;
	if (const auto* invite = std::get_if<LostArk::Shared::PARTY_INVITE_EVENT>(&event.Payload))
		m_Invite = *invite;
	else if (const auto* state = std::get_if<LostArk::Shared::PARTY_STATE_EVENT>(&event.Payload))
		m_State = state->State;
	m_iLastEventId = event.iEventId;
	return true;
}

void Client::CPartyViewModel::Reset()
{
	m_State = {}; m_Invite.reset(); m_iLastEventId = 0;
}

const LostArk::Shared::PARTY_STATE_SNAPSHOT&
Client::CPartyViewModel::Get_State() const
{
	return m_State;
}

const std::optional<LostArk::Shared::PARTY_INVITE_EVENT>&
Client::CPartyViewModel::Get_Invite() const
{
	return m_Invite;
}
```

```cpp
// FILE: Client/Public/ZoneTransitionCoordinator.h

#pragma once

#include "Party/PartyContracts.h"

namespace Client
{
	class IZoneLoadStage
	{
	public:
		virtual ~IZoneLoadStage() = default;
		virtual bool Begin_Stage(LostArk::Shared::ZONE_ID zoneId) = 0;
		virtual bool Commit_Stage(LostArk::Shared::ZONE_ID zoneId) = 0;
		virtual void Abort_Stage() = 0;
	};

	class CZoneTransitionCoordinator final
	{
	public:
		void Set_LoadStage(IZoneLoadStage* stage);
		bool Apply(const LostArk::Shared::S2C_PARTY_EVENT& event);
		void Reset();
		bool Has_PendingTransition() const;
	private:
		IZoneLoadStage* m_pLoadStage = nullptr;
		LostArk::Shared::ZONE_TRANSITION_ID m_iTransitionId =
			LostArk::Shared::INVALID_ZONE_TRANSITION_ID;
		LostArk::Shared::ZONE_ID m_eZoneId = LostArk::Shared::ZONE_ID::END;
	};
}
```

```cpp
// FILE: Client/Private/ZoneTransitionCoordinator.cpp

#include "ZoneTransitionCoordinator.h"
#include "NetworkManager.h"

void Client::CZoneTransitionCoordinator::Set_LoadStage(IZoneLoadStage* stage)
{
	m_pLoadStage = stage;
}

bool Client::CZoneTransitionCoordinator::Apply(
	const LostArk::Shared::S2C_PARTY_EVENT& event)
{
	using namespace LostArk::Shared;
	if (const auto* begin = std::get_if<ZONE_TRANSITION_BEGIN_EVENT>(&event.Payload))
	{
		if (nullptr == m_pLoadStage || Has_PendingTransition()) return false;
		m_iTransitionId = begin->iTransitionId;
		m_eZoneId = begin->eZoneId;
		const bool staged = m_pLoadStage->Begin_Stage(m_eZoneId);
		ZONE_READY_COMMAND ready{};
		ready.iTransitionId = m_iTransitionId;
		ready.isReady = staged;
		C2S_PARTY_COMMAND command{}; command.Payload = ready;
		return CNetworkManager::Get().Send_PartyCommand(command);
	}
	if (const auto* commit = std::get_if<ZONE_TRANSITION_COMMIT_EVENT>(&event.Payload))
	{
		if (commit->iTransitionId != m_iTransitionId || nullptr == m_pLoadStage) return false;
		const bool result = m_pLoadStage->Commit_Stage(commit->eZoneId);
		Reset(); return result;
	}
	if (const auto* abort = std::get_if<ZONE_TRANSITION_ABORT_EVENT>(&event.Payload))
	{
		if (abort->iTransitionId != m_iTransitionId) return false;
		if (nullptr != m_pLoadStage) m_pLoadStage->Abort_Stage();
		Reset(); return true;
	}
	return true;
}

void Client::CZoneTransitionCoordinator::Reset()
{
	m_iTransitionId = LostArk::Shared::INVALID_ZONE_TRANSITION_ID;
	m_eZoneId = LostArk::Shared::ZONE_ID::END;
}

bool Client::CZoneTransitionCoordinator::Has_PendingTransition() const
{
	return m_iTransitionId != LostArk::Shared::INVALID_ZONE_TRANSITION_ID;
}
```

```cpp
// FILE: Client/Public/NetworkManager.h
// ADD INCLUDE AND FUNCTION

#include "Party/PartyMessages.h"

bool Send_PartyCommand(const LostArk::Shared::C2S_PARTY_COMMAND& command);
```

```cpp
// FILE: Client/Private/NetworkManager.cpp
// ADD Send_PartyCommand() AND Handle_Frame() CASE

bool CNetworkManager::Send_PartyCommand(
	const LostArk::Shared::C2S_PARTY_COMMAND& command)
{
	using namespace LostArk::Shared;
	if (!Is_Connected()) return false;
	CPacketWriter writer; if (!Write_Message(writer, command)) return false;
	std::vector<std::uint8_t> frame;
	if (!Build_Packet_Frame(PACKET_TYPE::C2S_PARTY_COMMAND, writer.Get_Buffer(), frame)) return false;
	return Send_All(frame);
}

case PACKET_TYPE::S2C_PARTY_EVENT:
{
	S2C_PARTY_EVENT message{};
	if (!Read_Message(reader, message) || 0 != reader.Get_RemainingSize())
	{ m_iLastErrorCode.store(WSAEINVAL); return; }
	Client::CLIENT_REPLICATION_EVENT event{};
	event.eType = Client::CLIENT_REPLICATION_EVENT_TYPE::PARTY_EVENT;
	event.PartyEvent = std::move(message);
	m_ReplicationEvents.push_back(std::move(event));
	break;
}
```

```cpp
// FILE: Client/Public/ClientReplicationEvent.h
// ADD ENUMERATOR AND MEMBER

PARTY_EVENT,
LostArk::Shared::S2C_PARTY_EVENT PartyEvent;
```

```cpp
// FILE: Client/Public/ClientReplication.h
// ADD MEMBERS/GETTERS AND APPLY IN THE ORDERED EVENT SWITCH

#include "PartyViewModel.h"
#include "ZoneTransitionCoordinator.h"

CPartyViewModel m_PartyViewModel;
CZoneTransitionCoordinator m_ZoneTransitionCoordinator;

case CLIENT_REPLICATION_EVENT_TYPE::PARTY_EVENT:
	allSucceeded = m_PartyViewModel.Apply(event.PartyEvent) &&
		m_ZoneTransitionCoordinator.Apply(event.PartyEvent) && allSucceeded;
	break;
```

```cpp
// FILE: Server/Public/RoomCommand.h
// ADD ENUMERATOR AND MEMBER

PARTY_COMMAND,
LostArk::Shared::C2S_PARTY_COMMAND PartyCommand;
```

```cpp
// FILE: Server/Private/ServerApp.cpp
// ADD PACKET CASE

case PACKET_TYPE::C2S_PARTY_COMMAND:
{
	C2S_PARTY_COMMAND party{};
	if (!Read_Message(reader, party) || 0 != reader.Get_RemainingSize())
	{ Request_SessionClose(sessionId); return; }
	command.eType = ROOM_COMMAND_TYPE::PARTY_COMMAND;
	command.PartyCommand = std::move(party);
	break;
}
```

```cpp
// FILE: Server/Public/GameRoom.h
// ADD MEMBER/FUNCTIONS

#include "PartySystem.h"

CPartySystem m_PartySystem;
std::uint32_t m_iNextPartyEventId = 1;

bool Handle_PartyCommand(SESSION_ID sessionId,
	const LostArk::Shared::C2S_PARTY_COMMAND& command);
void Deliver_PartyEvents(std::vector<PARTY_DELIVERY>& deliveries);
std::unordered_map<LostArk::Shared::PLAYER_ID, PARTY_PLAYER_VIEW>
	Build_PartyPlayerViews() const;
```

```cpp
// FILE: Server/Private/GameRoom.cpp
// ADD TO Tick(), Leave(), AND SEND PATH

case ROOM_COMMAND_TYPE::PARTY_COMMAND:
	if (!Handle_PartyCommand(command.iSessionId, command.PartyCommand))
		if (const auto session = Find_Session(command.iSessionId)) session->Request_Close();
	break;

std::vector<PARTY_DELIVERY> partyTickDeliveries;
m_PartySystem.Tick(m_iServerTick, partyTickDeliveries);
Deliver_PartyEvents(partyTickDeliveries);

// IN Leave(), BEFORE ERASING THE PLAYER:
std::vector<PARTY_DELIVERY> partyLeaveDeliveries;
m_PartySystem.On_PlayerDisconnected(playerId, Build_PartyPlayerViews(), partyLeaveDeliveries);
Deliver_PartyEvents(partyLeaveDeliveries);
```

```cpp
// FILE: Server/Private/GameRoom.cpp

std::unordered_map<LostArk::Shared::PLAYER_ID, LostArk::Server::PARTY_PLAYER_VIEW>
LostArk::Server::CGameRoom::Build_PartyPlayerViews() const
{
	std::unordered_map<LostArk::Shared::PLAYER_ID, PARTY_PLAYER_VIEW> views;
	for (const auto& [playerId, player] : m_Players)
	{
		PARTY_PLAYER_VIEW view{};
		view.iPlayerId = playerId;
		view.strNickname = player.strNickName;
		view.isOnline = nullptr != Find_Session(player.iSessionId);
		views.emplace(playerId, std::move(view));
	}
	return views;
}

bool LostArk::Server::CGameRoom::Handle_PartyCommand(
	SESSION_ID sessionId,
	const LostArk::Shared::C2S_PARTY_COMMAND& command)
{
	const auto playerIdIter = m_PlayerIdBySessionId.find(sessionId);
	if (playerIdIter == m_PlayerIdBySessionId.end()) return false;
	std::vector<PARTY_DELIVERY> deliveries;
	if (!m_PartySystem.Handle_Command(
		playerIdIter->second,
		command.Payload,
		Build_PartyPlayerViews(),
		m_iServerTick,
		deliveries)) return false;
	Deliver_PartyEvents(deliveries);
	return true;
}

void LostArk::Server::CGameRoom::Deliver_PartyEvents(
	std::vector<PARTY_DELIVERY>& deliveries)
{
	using namespace LostArk::Shared;
	for (PARTY_DELIVERY& delivery : deliveries)
	{
		if (0 == m_iNextPartyEventId) ++m_iNextPartyEventId;
		S2C_PARTY_EVENT message{};
		message.iEventId = m_iNextPartyEventId++;
		message.Payload = std::move(delivery.Payload);
		CPacketWriter writer;
		if (!Write_Message(writer, message)) continue;
		for (const PLAYER_ID recipientId : delivery.Recipients)
		{
			const auto playerIter = m_Players.find(recipientId);
			if (playerIter == m_Players.end()) continue;
			const auto session = Find_Session(playerIter->second.iSessionId);
			if (nullptr != session && !session->Send_Frame(
				PACKET_TYPE::S2C_PARTY_EVENT, writer.Get_Buffer()))
				session->Request_Close();
		}
	}
	deliveries.clear();
}
```

```xml
<!-- PROJECT ITEMS -->
<ClInclude Include="..\Public\Party\PartyContracts.h" />
<ClInclude Include="..\Public\Party\PartyMessages.h" />
<ClCompile Include="..\Private\Party\PartyMessages.cpp" />
<ClInclude Include="..\Public\PartySystem.h" />
<ClCompile Include="..\Private\PartySystem.cpp" />
<ClInclude Include="..\Public\PartyViewModel.h" />
<ClInclude Include="..\Public\ZoneTransitionCoordinator.h" />
<ClCompile Include="..\Private\PartyViewModel.cpp" />
<ClCompile Include="..\Private\ZoneTransitionCoordinator.cpp" />
```

```xml
<!-- FILE: Shared/Default/Shared.vcxproj.filters -->
<Filter Include="Public\Party"><UniqueIdentifier>{47B5731A-135A-44D0-825B-F2FA9E741EF2}</UniqueIdentifier></Filter>
<Filter Include="Private\Party"><UniqueIdentifier>{FB04D26D-67C5-4D79-B821-36878DB0F4B2}</UniqueIdentifier></Filter>
<ClInclude Include="..\Public\Party\PartyContracts.h"><Filter>Public\Party</Filter></ClInclude>
<ClInclude Include="..\Public\Party\PartyMessages.h"><Filter>Public\Party</Filter></ClInclude>
<ClCompile Include="..\Private\Party\PartyMessages.cpp"><Filter>Private\Party</Filter></ClCompile>

<!-- FILE: Server/Default/Server.vcxproj.filters -->
<ClInclude Include="..\Public\PartySystem.h"><Filter>Public</Filter></ClInclude>
<ClCompile Include="..\Private\PartySystem.cpp"><Filter>Private</Filter></ClCompile>

<!-- FILE: Client/Default/Client.vcxproj.filters -->
<ClInclude Include="..\Public\PartyViewModel.h"><Filter>04. Network</Filter></ClInclude>
<ClInclude Include="..\Public\ZoneTransitionCoordinator.h"><Filter>04. Network</Filter></ClInclude>
<ClCompile Include="..\Private\PartyViewModel.cpp"><Filter>04. Network</Filter></ClCompile>
<ClCompile Include="..\Private\ZoneTransitionCoordinator.cpp"><Filter>04. Network</Filter></ClCompile>
```

```text
A invites B -> InviteId nonzero -> B receives once
B accepts -> PartyId nonzero -> A/B state has same leader and members
non-leader StartZoneTransition -> rejected without Begin
leader StartZoneTransition -> A/B same TransitionId and ZoneId
A Ready=true, B pending -> no Commit
B Ready=true -> one Commit to A/B
B Ready=false/load failure/disconnect/timeout -> one Abort to A/B
Abort -> staged resources rollback, current level preserved
Commit -> both clients enter the same zone
```
