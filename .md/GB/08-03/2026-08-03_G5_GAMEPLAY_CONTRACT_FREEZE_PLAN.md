```cpp
// FILE: Shared/Public/Gameplay/GameplayContracts.h

#pragma once

#include "Network/NetworkIds.h"

#include <cstddef>
#include <cstdint>
#include <variant>
#include <vector>

namespace LostArk::Shared
{
	using ACTION_ID = std::uint32_t;
	using SKILL_ID = std::uint32_t;
	using EFFECT_ASSET_ID = std::uint32_t;

	inline constexpr ACTION_ID INVALID_ACTION_ID = 0;
	inline constexpr SKILL_ID INVALID_SKILL_ID = 0;
	inline constexpr EFFECT_ASSET_ID INVALID_EFFECT_ASSET_ID = 0;
	inline constexpr std::size_t MAX_ENTITY_SNAPSHOTS = 128;

	enum class ENTITY_TYPE : std::uint8_t
	{
		PLAYER,
		MONSTER,
		BOSS,
		PROJECTILE,
		END
	};

	enum class ENTITY_STATE : std::uint8_t
	{
		IDLE,
		MOVING,
		ACTING,
		STAGGERED,
		DEAD,
		END
	};

	struct TRANSFORM_SNAPSHOT
	{
		float fPositionX = 0.f;
		float fPositionY = 0.f;
		float fPositionZ = 0.f;
		float fYawDegrees = 0.f;
	};

	struct ENTITY_SNAPSHOT
	{
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
		ENTITY_TYPE eEntityType = ENTITY_TYPE::END;
		TRANSFORM_SNAPSHOT Transform;
		std::uint32_t iHp = 0;
		std::uint32_t iMaxHp = 0;
		ENTITY_STATE eState = ENTITY_STATE::END;
		ACTION_ID iActionId = INVALID_ACTION_ID;
		std::uint32_t iActionStartTick = 0;
	};

	struct MOVE_COMMAND
	{
		std::uint32_t iSequence = 0;
		float fGoalX = 0.f;
		float fGoalZ = 0.f;
	};

	struct USE_SKILL_COMMAND
	{
		std::uint32_t iSequence = 0;
		SKILL_ID iSkillId = INVALID_SKILL_ID;
		float fAimX = 0.f;
		float fAimY = 0.f;
		float fAimZ = 0.f;
	};

	struct CHANGE_IDENTITY_COMMAND
	{
		std::uint32_t iSequence = 0;
		std::uint8_t iRequestedState = 0;
	};

	struct INTERACT_COMMAND
	{
		std::uint32_t iSequence = 0;
		NET_ENTITY_ID iTargetNetEntityId = INVALID_NET_ENTITY_ID;
	};

	struct ENTITY_SPAWNED_EVENT
	{
		ENTITY_SNAPSHOT InitialState;
	};

	enum class ENTITY_DESPAWN_REASON : std::uint8_t
	{
		OUT_OF_SCOPE,
		LEVEL_CHANGED,
		DESTROYED,
		END
	};

	struct ENTITY_DESPAWNED_EVENT
	{
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
		ENTITY_DESPAWN_REASON eReason = ENTITY_DESPAWN_REASON::END;
	};

	struct ACTION_STARTED_EVENT
	{
		NET_ENTITY_ID iSourceNetEntityId = INVALID_NET_ENTITY_ID;
		ACTION_ID iActionId = INVALID_ACTION_ID;
		std::uint32_t iActionStartTick = 0;
		float fDirectionX = 0.f;
		float fDirectionZ = 1.f;
	};

	struct DAMAGE_APPLIED_EVENT
	{
		NET_ENTITY_ID iSourceNetEntityId = INVALID_NET_ENTITY_ID;
		NET_ENTITY_ID iTargetNetEntityId = INVALID_NET_ENTITY_ID;
		std::uint32_t iDamage = 0;
		std::uint32_t iRemainingHp = 0;
	};

	struct ENTITY_DIED_EVENT
	{
		NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
		NET_ENTITY_ID iKillerNetEntityId = INVALID_NET_ENTITY_ID;
	};

	enum class EFFECT_ATTACH_SOCKET : std::uint8_t
	{
		WORLD,
		ROOT,
		WEAPON_RIGHT,
		TARGET_CENTER,
		END
	};

	struct EFFECT_CUE
	{
		EFFECT_ASSET_ID iEffectAssetId = INVALID_EFFECT_ASSET_ID;
		NET_ENTITY_ID iSourceNetEntityId = INVALID_NET_ENTITY_ID;
		NET_ENTITY_ID iTargetNetEntityId = INVALID_NET_ENTITY_ID;
		EFFECT_ATTACH_SOCKET eAttachSocket = EFFECT_ATTACH_SOCKET::END;
		float fWorldX = 0.f;
		float fWorldY = 0.f;
		float fWorldZ = 0.f;
		float fDirectionX = 0.f;
		float fDirectionY = 0.f;
		float fDirectionZ = 1.f;
		std::uint32_t iServerStartTick = 0;
	};

	enum class CAMERA_CUE_TYPE : std::uint8_t
	{
		SHAKE,
		FOCUS,
		RESET,
		END
	};

	struct CAMERA_CUE
	{
		CAMERA_CUE_TYPE eType = CAMERA_CUE_TYPE::END;
		NET_ENTITY_ID iTargetNetEntityId = INVALID_NET_ENTITY_ID;
		float fMagnitude = 0.f;
		float fDurationSeconds = 0.f;
	};

	using GAMEPLAY_EVENT_PAYLOAD = std::variant<
		ENTITY_SPAWNED_EVENT,
		ENTITY_DESPAWNED_EVENT,
		ACTION_STARTED_EVENT,
		DAMAGE_APPLIED_EVENT,
		ENTITY_DIED_EVENT,
		EFFECT_CUE,
		CAMERA_CUE>;

	struct GAMEPLAY_EVENT
	{
		std::uint32_t iEventId = 0;
		std::uint32_t iServerTick = 0;
		GAMEPLAY_EVENT_PAYLOAD Payload = ENTITY_SPAWNED_EVENT{};
	};

	struct S2C_ENTITY_SNAPSHOT
	{
		std::uint32_t iServerTick = 0;
		std::vector<ENTITY_SNAPSHOT> Entities;
	};

	struct S2C_GAMEPLAY_EVENT
	{
		GAMEPLAY_EVENT Event;
	};

	using C2S_USE_SKILL = USE_SKILL_COMMAND;
	using C2S_CHANGE_IDENTITY = CHANGE_IDENTITY_COMMAND;
	using C2S_INTERACT = INTERACT_COMMAND;
}
```

```cpp
// FILE: Shared/Public/Gameplay/GameplayMessages.h

#pragma once

#include "Gameplay/GameplayContracts.h"

namespace LostArk::Shared
{
	class CPacketReader;
	class CPacketWriter;

	[[nodiscard]] bool Is_Valid_EntitySnapshot(
		const ENTITY_SNAPSHOT& snapshot);

	bool Write_Message(CPacketWriter& writer, const C2S_USE_SKILL& message);
	bool Read_Message(CPacketReader& reader, C2S_USE_SKILL& message);

	bool Write_Message(CPacketWriter& writer, const C2S_CHANGE_IDENTITY& message);
	bool Read_Message(CPacketReader& reader, C2S_CHANGE_IDENTITY& message);

	bool Write_Message(CPacketWriter& writer, const C2S_INTERACT& message);
	bool Read_Message(CPacketReader& reader, C2S_INTERACT& message);

	bool Write_Message(CPacketWriter& writer, const S2C_ENTITY_SNAPSHOT& message);
	bool Read_Message(CPacketReader& reader, S2C_ENTITY_SNAPSHOT& message);

	bool Write_Message(CPacketWriter& writer, const S2C_GAMEPLAY_EVENT& message);
	bool Read_Message(CPacketReader& reader, S2C_GAMEPLAY_EVENT& message);
}
```

```cpp
// FILE: Shared/Public/Network/PacketType.h
// INSERT BEFORE S2C_PLAYER_DESPAWNED AND ADD TO Is_Known_Packet_Type()

C2S_USE_SKILL,
C2S_CHANGE_IDENTITY,
C2S_INTERACT,
S2C_ENTITY_SNAPSHOT,
S2C_GAMEPLAY_EVENT,
```

```cpp
// FILE: Shared/Private/Gameplay/GameplayMessages.cpp

#include "Gameplay/GameplayMessages.h"

#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <cmath>
#include <type_traits>
#include <utility>

namespace
{
	using namespace LostArk::Shared;

	template<typename TEnum>
	bool Is_Enum_Value(TEnum value, TEnum end)
	{
		return static_cast<std::underlying_type_t<TEnum>>(value) <
			static_cast<std::underlying_type_t<TEnum>>(end);
	}

	bool Is_Finite_Transform(const TRANSFORM_SNAPSHOT& value)
	{
		return std::isfinite(value.fPositionX) &&
			std::isfinite(value.fPositionY) &&
			std::isfinite(value.fPositionZ) &&
			std::isfinite(value.fYawDegrees);
	}

	void Write_Transform(CPacketWriter& writer, const TRANSFORM_SNAPSHOT& value)
	{
		writer.Write_F32(value.fPositionX);
		writer.Write_F32(value.fPositionY);
		writer.Write_F32(value.fPositionZ);
		writer.Write_F32(value.fYawDegrees);
	}

	bool Read_Transform(CPacketReader& reader, TRANSFORM_SNAPSHOT& value)
	{
		TRANSFORM_SNAPSHOT decoded{};
		if (!reader.Read_F32(decoded.fPositionX) ||
			!reader.Read_F32(decoded.fPositionY) ||
			!reader.Read_F32(decoded.fPositionZ) ||
			!reader.Read_F32(decoded.fYawDegrees) ||
			!Is_Finite_Transform(decoded))
		{
			return false;
		}

		value = decoded;
		return true;
	}

	bool Write_Entity(CPacketWriter& writer, const ENTITY_SNAPSHOT& value)
	{
		if (!Is_Valid_EntitySnapshot(value))
			return false;

		writer.Write_U32(value.iNetEntityId);
		writer.Write_U8(static_cast<std::uint8_t>(value.eEntityType));
		Write_Transform(writer, value.Transform);
		writer.Write_U32(value.iHp);
		writer.Write_U32(value.iMaxHp);
		writer.Write_U8(static_cast<std::uint8_t>(value.eState));
		writer.Write_U32(value.iActionId);
		writer.Write_U32(value.iActionStartTick);
		return true;
	}

	bool Read_Entity(CPacketReader& reader, ENTITY_SNAPSHOT& value)
	{
		ENTITY_SNAPSHOT decoded{};
		std::uint8_t rawType = 0;
		std::uint8_t rawState = 0;

		if (!reader.Read_U32(decoded.iNetEntityId) ||
			!reader.Read_U8(rawType) ||
			!Read_Transform(reader, decoded.Transform) ||
			!reader.Read_U32(decoded.iHp) ||
			!reader.Read_U32(decoded.iMaxHp) ||
			!reader.Read_U8(rawState) ||
			!reader.Read_U32(decoded.iActionId) ||
			!reader.Read_U32(decoded.iActionStartTick))
		{
			return false;
		}

		decoded.eEntityType = static_cast<ENTITY_TYPE>(rawType);
		decoded.eState = static_cast<ENTITY_STATE>(rawState);
		if (!Is_Valid_EntitySnapshot(decoded))
			return false;

		value = decoded;
		return true;
	}

	enum class WIRE_EVENT_TYPE : std::uint8_t
	{
		ENTITY_SPAWNED,
		ENTITY_DESPAWNED,
		ACTION_STARTED,
		DAMAGE_APPLIED,
		ENTITY_DIED,
		EFFECT_CUE,
		CAMERA_CUE,
		END
	};

	WIRE_EVENT_TYPE Get_WireType(const GAMEPLAY_EVENT_PAYLOAD& payload)
	{
		return std::visit([](const auto& value)
		{
			using TValue = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<TValue, ENTITY_SPAWNED_EVENT>)
				return WIRE_EVENT_TYPE::ENTITY_SPAWNED;
			else if constexpr (std::is_same_v<TValue, ENTITY_DESPAWNED_EVENT>)
				return WIRE_EVENT_TYPE::ENTITY_DESPAWNED;
			else if constexpr (std::is_same_v<TValue, ACTION_STARTED_EVENT>)
				return WIRE_EVENT_TYPE::ACTION_STARTED;
			else if constexpr (std::is_same_v<TValue, DAMAGE_APPLIED_EVENT>)
				return WIRE_EVENT_TYPE::DAMAGE_APPLIED;
			else if constexpr (std::is_same_v<TValue, ENTITY_DIED_EVENT>)
				return WIRE_EVENT_TYPE::ENTITY_DIED;
			else if constexpr (std::is_same_v<TValue, EFFECT_CUE>)
				return WIRE_EVENT_TYPE::EFFECT_CUE;
			else
				return WIRE_EVENT_TYPE::CAMERA_CUE;
		}, payload);
	}
}

bool LostArk::Shared::Is_Valid_EntitySnapshot(
	const ENTITY_SNAPSHOT& snapshot)
{
	return snapshot.iNetEntityId != INVALID_NET_ENTITY_ID &&
		Is_Enum_Value(snapshot.eEntityType, ENTITY_TYPE::END) &&
		Is_Finite_Transform(snapshot.Transform) &&
		snapshot.iMaxHp > 0 &&
		snapshot.iHp <= snapshot.iMaxHp &&
		Is_Enum_Value(snapshot.eState, ENTITY_STATE::END) &&
		(snapshot.iActionId == INVALID_ACTION_ID ||
		 snapshot.iActionStartTick != 0);
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_USE_SKILL& message)
{
	if (0 == message.iSequence ||
		message.iSkillId == INVALID_SKILL_ID ||
		!std::isfinite(message.fAimX) ||
		!std::isfinite(message.fAimY) ||
		!std::isfinite(message.fAimZ))
	{
		return false;
	}

	writer.Write_U32(message.iSequence);
	writer.Write_U32(message.iSkillId);
	writer.Write_F32(message.fAimX);
	writer.Write_F32(message.fAimY);
	writer.Write_F32(message.fAimZ);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_USE_SKILL& message)
{
	C2S_USE_SKILL decoded{};
	if (!reader.Read_U32(decoded.iSequence) ||
		!reader.Read_U32(decoded.iSkillId) ||
		!reader.Read_F32(decoded.fAimX) ||
		!reader.Read_F32(decoded.fAimY) ||
		!reader.Read_F32(decoded.fAimZ))
	{
		return false;
	}

	CPacketWriter validator;
	if (!Write_Message(validator, decoded))
		return false;

	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_CHANGE_IDENTITY& message)
{
	if (0 == message.iSequence || message.iRequestedState > 7)
		return false;

	writer.Write_U32(message.iSequence);
	writer.Write_U8(message.iRequestedState);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_CHANGE_IDENTITY& message)
{
	C2S_CHANGE_IDENTITY decoded{};
	if (!reader.Read_U32(decoded.iSequence) ||
		!reader.Read_U8(decoded.iRequestedState) ||
		0 == decoded.iSequence ||
		decoded.iRequestedState > 7)
	{
		return false;
	}

	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_INTERACT& message)
{
	if (0 == message.iSequence ||
		message.iTargetNetEntityId == INVALID_NET_ENTITY_ID)
	{
		return false;
	}

	writer.Write_U32(message.iSequence);
	writer.Write_U32(message.iTargetNetEntityId);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_INTERACT& message)
{
	C2S_INTERACT decoded{};
	if (!reader.Read_U32(decoded.iSequence) ||
		!reader.Read_U32(decoded.iTargetNetEntityId) ||
		0 == decoded.iSequence ||
		decoded.iTargetNetEntityId == INVALID_NET_ENTITY_ID)
	{
		return false;
	}

	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_ENTITY_SNAPSHOT& message)
{
	if (0 == message.iServerTick ||
		message.Entities.empty() ||
		message.Entities.size() > MAX_ENTITY_SNAPSHOTS)
	{
		return false;
	}

	for (const ENTITY_SNAPSHOT& entity : message.Entities)
	{
		if (!Is_Valid_EntitySnapshot(entity))
			return false;
	}

	writer.Write_U32(message.iServerTick);
	writer.Write_U16(static_cast<std::uint16_t>(message.Entities.size()));
	for (const ENTITY_SNAPSHOT& entity : message.Entities)
	{
		if (!Write_Entity(writer, entity))
			return false;
	}
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_ENTITY_SNAPSHOT& message)
{
	std::uint32_t serverTick = 0;
	std::uint16_t count = 0;
	if (!reader.Read_U32(serverTick) ||
		!reader.Read_U16(count) ||
		0 == serverTick ||
		0 == count ||
		count > MAX_ENTITY_SNAPSHOTS)
	{
		return false;
	}

	S2C_ENTITY_SNAPSHOT decoded{};
	decoded.iServerTick = serverTick;
	decoded.Entities.reserve(count);
	for (std::uint16_t index = 0; index < count; ++index)
	{
		ENTITY_SNAPSHOT entity{};
		if (!Read_Entity(reader, entity))
			return false;
		decoded.Entities.push_back(entity);
	}

	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_GAMEPLAY_EVENT& message)
{
	const GAMEPLAY_EVENT& event = message.Event;
	if (0 == event.iEventId || 0 == event.iServerTick)
		return false;

	const WIRE_EVENT_TYPE wireType = Get_WireType(event.Payload);
	writer.Write_U32(event.iEventId);
	writer.Write_U32(event.iServerTick);
	writer.Write_U8(static_cast<std::uint8_t>(wireType));

	return std::visit([&writer](const auto& value) -> bool
	{
		using TValue = std::decay_t<decltype(value)>;
		if constexpr (std::is_same_v<TValue, ENTITY_SPAWNED_EVENT>)
		{
			return Write_Entity(writer, value.InitialState);
		}
		else if constexpr (std::is_same_v<TValue, ENTITY_DESPAWNED_EVENT>)
		{
			if (value.iNetEntityId == INVALID_NET_ENTITY_ID ||
				!Is_Enum_Value(value.eReason, ENTITY_DESPAWN_REASON::END))
				return false;
			writer.Write_U32(value.iNetEntityId);
			writer.Write_U8(static_cast<std::uint8_t>(value.eReason));
			return true;
		}
		else if constexpr (std::is_same_v<TValue, ACTION_STARTED_EVENT>)
		{
			if (value.iSourceNetEntityId == INVALID_NET_ENTITY_ID ||
				value.iActionId == INVALID_ACTION_ID ||
				0 == value.iActionStartTick ||
				!std::isfinite(value.fDirectionX) ||
				!std::isfinite(value.fDirectionZ))
				return false;
			writer.Write_U32(value.iSourceNetEntityId);
			writer.Write_U32(value.iActionId);
			writer.Write_U32(value.iActionStartTick);
			writer.Write_F32(value.fDirectionX);
			writer.Write_F32(value.fDirectionZ);
			return true;
		}
		else if constexpr (std::is_same_v<TValue, DAMAGE_APPLIED_EVENT>)
		{
			if (value.iSourceNetEntityId == INVALID_NET_ENTITY_ID ||
				value.iTargetNetEntityId == INVALID_NET_ENTITY_ID ||
				0 == value.iDamage)
				return false;
			writer.Write_U32(value.iSourceNetEntityId);
			writer.Write_U32(value.iTargetNetEntityId);
			writer.Write_U32(value.iDamage);
			writer.Write_U32(value.iRemainingHp);
			return true;
		}
		else if constexpr (std::is_same_v<TValue, ENTITY_DIED_EVENT>)
		{
			if (value.iNetEntityId == INVALID_NET_ENTITY_ID)
				return false;
			writer.Write_U32(value.iNetEntityId);
			writer.Write_U32(value.iKillerNetEntityId);
			return true;
		}
		else if constexpr (std::is_same_v<TValue, EFFECT_CUE>)
		{
			if (value.iEffectAssetId == INVALID_EFFECT_ASSET_ID ||
				!Is_Enum_Value(value.eAttachSocket, EFFECT_ATTACH_SOCKET::END) ||
				0 == value.iServerStartTick ||
				!std::isfinite(value.fWorldX) ||
				!std::isfinite(value.fWorldY) ||
				!std::isfinite(value.fWorldZ) ||
				!std::isfinite(value.fDirectionX) ||
				!std::isfinite(value.fDirectionY) ||
				!std::isfinite(value.fDirectionZ))
				return false;
			writer.Write_U32(value.iEffectAssetId);
			writer.Write_U32(value.iSourceNetEntityId);
			writer.Write_U32(value.iTargetNetEntityId);
			writer.Write_U8(static_cast<std::uint8_t>(value.eAttachSocket));
			writer.Write_F32(value.fWorldX);
			writer.Write_F32(value.fWorldY);
			writer.Write_F32(value.fWorldZ);
			writer.Write_F32(value.fDirectionX);
			writer.Write_F32(value.fDirectionY);
			writer.Write_F32(value.fDirectionZ);
			writer.Write_U32(value.iServerStartTick);
			return true;
		}
		else
		{
			if (!Is_Enum_Value(value.eType, CAMERA_CUE_TYPE::END) ||
				!std::isfinite(value.fMagnitude) ||
				!std::isfinite(value.fDurationSeconds) ||
				value.fMagnitude < 0.f ||
				value.fDurationSeconds < 0.f)
				return false;
			writer.Write_U8(static_cast<std::uint8_t>(value.eType));
			writer.Write_U32(value.iTargetNetEntityId);
			writer.Write_F32(value.fMagnitude);
			writer.Write_F32(value.fDurationSeconds);
			return true;
		}
	}, event.Payload);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_GAMEPLAY_EVENT& message)
{
	GAMEPLAY_EVENT decodedEvent{};
	std::uint8_t rawType = 0;
	if (!reader.Read_U32(decodedEvent.iEventId) ||
		!reader.Read_U32(decodedEvent.iServerTick) ||
		!reader.Read_U8(rawType) ||
		0 == decodedEvent.iEventId ||
		0 == decodedEvent.iServerTick ||
		rawType >= static_cast<std::uint8_t>(WIRE_EVENT_TYPE::END))
	{
		return false;
	}

	switch (static_cast<WIRE_EVENT_TYPE>(rawType))
	{
	case WIRE_EVENT_TYPE::ENTITY_SPAWNED:
	{
		ENTITY_SPAWNED_EVENT value{};
		if (!Read_Entity(reader, value.InitialState)) return false;
		decodedEvent.Payload = value;
		break;
	}
	case WIRE_EVENT_TYPE::ENTITY_DESPAWNED:
	{
		ENTITY_DESPAWNED_EVENT value{};
		std::uint8_t rawReason = 0;
		if (!reader.Read_U32(value.iNetEntityId) ||
			!reader.Read_U8(rawReason) ||
			value.iNetEntityId == INVALID_NET_ENTITY_ID ||
			rawReason >= static_cast<std::uint8_t>(ENTITY_DESPAWN_REASON::END))
			return false;
		value.eReason = static_cast<ENTITY_DESPAWN_REASON>(rawReason);
		decodedEvent.Payload = value;
		break;
	}
	case WIRE_EVENT_TYPE::ACTION_STARTED:
	{
		ACTION_STARTED_EVENT value{};
		if (!reader.Read_U32(value.iSourceNetEntityId) ||
			!reader.Read_U32(value.iActionId) ||
			!reader.Read_U32(value.iActionStartTick) ||
			!reader.Read_F32(value.fDirectionX) ||
			!reader.Read_F32(value.fDirectionZ)) return false;
		decodedEvent.Payload = value;
		break;
	}
	case WIRE_EVENT_TYPE::DAMAGE_APPLIED:
	{
		DAMAGE_APPLIED_EVENT value{};
		if (!reader.Read_U32(value.iSourceNetEntityId) ||
			!reader.Read_U32(value.iTargetNetEntityId) ||
			!reader.Read_U32(value.iDamage) ||
			!reader.Read_U32(value.iRemainingHp)) return false;
		decodedEvent.Payload = value;
		break;
	}
	case WIRE_EVENT_TYPE::ENTITY_DIED:
	{
		ENTITY_DIED_EVENT value{};
		if (!reader.Read_U32(value.iNetEntityId) ||
			!reader.Read_U32(value.iKillerNetEntityId)) return false;
		decodedEvent.Payload = value;
		break;
	}
	case WIRE_EVENT_TYPE::EFFECT_CUE:
	{
		EFFECT_CUE value{};
		std::uint8_t rawSocket = 0;
		if (!reader.Read_U32(value.iEffectAssetId) ||
			!reader.Read_U32(value.iSourceNetEntityId) ||
			!reader.Read_U32(value.iTargetNetEntityId) ||
			!reader.Read_U8(rawSocket) ||
			!reader.Read_F32(value.fWorldX) ||
			!reader.Read_F32(value.fWorldY) ||
			!reader.Read_F32(value.fWorldZ) ||
			!reader.Read_F32(value.fDirectionX) ||
			!reader.Read_F32(value.fDirectionY) ||
			!reader.Read_F32(value.fDirectionZ) ||
			!reader.Read_U32(value.iServerStartTick) ||
			rawSocket >= static_cast<std::uint8_t>(EFFECT_ATTACH_SOCKET::END))
			return false;
		value.eAttachSocket = static_cast<EFFECT_ATTACH_SOCKET>(rawSocket);
		decodedEvent.Payload = value;
		break;
	}
	case WIRE_EVENT_TYPE::CAMERA_CUE:
	{
		CAMERA_CUE value{};
		std::uint8_t rawCue = 0;
		if (!reader.Read_U8(rawCue) ||
			!reader.Read_U32(value.iTargetNetEntityId) ||
			!reader.Read_F32(value.fMagnitude) ||
			!reader.Read_F32(value.fDurationSeconds) ||
			rawCue >= static_cast<std::uint8_t>(CAMERA_CUE_TYPE::END))
			return false;
		value.eType = static_cast<CAMERA_CUE_TYPE>(rawCue);
		decodedEvent.Payload = value;
		break;
	}
	default:
		return false;
	}

	S2C_GAMEPLAY_EVENT validation{};
	validation.Event = decodedEvent;
	CPacketWriter validator;
	if (!Write_Message(validator, validation))
		return false;

	message.Event = std::move(decodedEvent);
	return true;
}
```

```cpp
// FILE: Client/Public/PlayerCommandSink.h

#pragma once

#include "Gameplay/GameplayContracts.h"

namespace Client
{
	class IPlayerCommandSink
	{
	public:
		virtual ~IPlayerCommandSink() = default;

		virtual bool Submit_Move(
			const LostArk::Shared::MOVE_COMMAND& command) = 0;
		virtual bool Submit_UseSkill(
			const LostArk::Shared::USE_SKILL_COMMAND& command) = 0;
		virtual bool Submit_ChangeIdentity(
			const LostArk::Shared::CHANGE_IDENTITY_COMMAND& command) = 0;
		virtual bool Submit_Interact(
			const LostArk::Shared::INTERACT_COMMAND& command) = 0;
	};

	class CNetworkPlayerCommandSink final : public IPlayerCommandSink
	{
	public:
		bool Submit_Move(
			const LostArk::Shared::MOVE_COMMAND& command) override;
		bool Submit_UseSkill(
			const LostArk::Shared::USE_SKILL_COMMAND& command) override;
		bool Submit_ChangeIdentity(
			const LostArk::Shared::CHANGE_IDENTITY_COMMAND& command) override;
		bool Submit_Interact(
			const LostArk::Shared::INTERACT_COMMAND& command) override;
	};
}
```

```cpp
// FILE: Client/Private/PlayerCommandSink.cpp

#include "PlayerCommandSink.h"

#include "NetworkManager.h"

bool Client::CNetworkPlayerCommandSink::Submit_Move(
	const LostArk::Shared::MOVE_COMMAND& command)
{
	return CNetworkManager::Get().Send_MoveGoal(
		command.iSequence,
		command.fGoalX,
		command.fGoalZ);
}

bool Client::CNetworkPlayerCommandSink::Submit_UseSkill(
	const LostArk::Shared::USE_SKILL_COMMAND& command)
{
	return CNetworkManager::Get().Send_UseSkill(command);
}

bool Client::CNetworkPlayerCommandSink::Submit_ChangeIdentity(
	const LostArk::Shared::CHANGE_IDENTITY_COMMAND& command)
{
	return CNetworkManager::Get().Send_ChangeIdentity(command);
}

bool Client::CNetworkPlayerCommandSink::Submit_Interact(
	const LostArk::Shared::INTERACT_COMMAND& command)
{
	return CNetworkManager::Get().Send_Interact(command);
}
```

```cpp
// FILE: Client/Public/NetworkManager.h
// ADD INCLUDE AND PUBLIC FUNCTIONS

#include "Gameplay/GameplayMessages.h"

bool Send_UseSkill(const LostArk::Shared::C2S_USE_SKILL& command);
bool Send_ChangeIdentity(const LostArk::Shared::C2S_CHANGE_IDENTITY& command);
bool Send_Interact(const LostArk::Shared::C2S_INTERACT& command);
```

```cpp
// FILE: Client/Private/NetworkManager.cpp
// APPEND

bool CNetworkManager::Send_UseSkill(
	const LostArk::Shared::C2S_USE_SKILL& command)
{
	using namespace LostArk::Shared;
	if (!Is_Connected()) return false;
	CPacketWriter writer;
	if (!Write_Message(writer, command)) return false;
	std::vector<std::uint8_t> frame;
	if (!Build_Packet_Frame(
		PACKET_TYPE::C2S_USE_SKILL,
		writer.Get_Buffer(), frame)) return false;
	return Send_All(frame);
}

bool CNetworkManager::Send_ChangeIdentity(
	const LostArk::Shared::C2S_CHANGE_IDENTITY& command)
{
	using namespace LostArk::Shared;
	if (!Is_Connected()) return false;
	CPacketWriter writer;
	if (!Write_Message(writer, command)) return false;
	std::vector<std::uint8_t> frame;
	if (!Build_Packet_Frame(
		PACKET_TYPE::C2S_CHANGE_IDENTITY,
		writer.Get_Buffer(), frame)) return false;
	return Send_All(frame);
}

bool CNetworkManager::Send_Interact(
	const LostArk::Shared::C2S_INTERACT& command)
{
	using namespace LostArk::Shared;
	if (!Is_Connected()) return false;
	CPacketWriter writer;
	if (!Write_Message(writer, command)) return false;
	std::vector<std::uint8_t> frame;
	if (!Build_Packet_Frame(
		PACKET_TYPE::C2S_INTERACT,
		writer.Get_Buffer(), frame)) return false;
	return Send_All(frame);
}
```

```cpp
// FILE: Client/Public/PlayerController.h
// ADD FORWARD DECLARATION, SETTER, MEMBER

class IPlayerCommandSink;

void Set_CommandSink(IPlayerCommandSink* commandSink);

IPlayerCommandSink* m_pCommandSink = nullptr;
```

```cpp
// FILE: Client/Private/PlayerController.cpp
// REPLACE NetworkManager INCLUDE

#include "PlayerCommandSink.h"
```

```cpp
// FILE: Client/Private/PlayerController.cpp
// ADD SETTER

void Client::CPlayerController::Set_CommandSink(
	IPlayerCommandSink* commandSink)
{
	m_pCommandSink = commandSink;
}
```

```cpp
// FILE: Client/Private/PlayerController.cpp
// REPLACE Send_MoveGoal() CALL

LostArk::Shared::MOVE_COMMAND command{};
command.iSequence = m_iNextMoveSequence;
command.fGoalX = goal.x;
command.fGoalZ = goal.z;

if (nullptr != m_pCommandSink &&
	m_pCommandSink->Submit_Move(command))
{
	++m_iNextMoveSequence;
	if (0 == m_iNextMoveSequence)
		m_iNextMoveSequence = 1;
}
```

```cpp
// FILE: Client/Public/Level_Baren.h
// ADD INCLUDE AND MEMBER BEFORE CPlayerController

#include "PlayerCommandSink.h"

CNetworkPlayerCommandSink m_PlayerCommandSink;
```

```cpp
// FILE: Client/Private/Level_Baren.cpp
// ADD TO Initialize() AFTER m_Replication.Initialize()

m_PlayerController.Set_CommandSink(&m_PlayerCommandSink);
```

```xml
<!-- FILE: Shared/Default/Shared.vcxproj -->
<ClInclude Include="..\Public\Gameplay\GameplayContracts.h" />
<ClInclude Include="..\Public\Gameplay\GameplayMessages.h" />
<ClCompile Include="..\Private\Gameplay\GameplayMessages.cpp" />
```

```xml
<!-- FILE: Shared/Default/Shared.vcxproj.filters -->
<Filter Include="Public\Gameplay">
  <UniqueIdentifier>{8F56491E-5F5C-4E52-A6D7-A3063FE4D67D}</UniqueIdentifier>
</Filter>
<Filter Include="Private\Gameplay">
  <UniqueIdentifier>{2A451130-46B4-4636-9A16-52532CA5EE65}</UniqueIdentifier>
</Filter>
<ClInclude Include="..\Public\Gameplay\GameplayContracts.h">
  <Filter>Public\Gameplay</Filter>
</ClInclude>
<ClInclude Include="..\Public\Gameplay\GameplayMessages.h">
  <Filter>Public\Gameplay</Filter>
</ClInclude>
<ClCompile Include="..\Private\Gameplay\GameplayMessages.cpp">
  <Filter>Private\Gameplay</Filter>
</ClCompile>
```

```xml
<!-- FILE: Client/Default/Client.vcxproj -->
<ClInclude Include="..\Public\PlayerCommandSink.h" />
<ClCompile Include="..\Private\PlayerCommandSink.cpp" />
```

```xml
<!-- FILE: Client/Default/Client.vcxproj.filters -->
<ClInclude Include="..\Public\PlayerCommandSink.h">
  <Filter>04. Network</Filter>
</ClInclude>
<ClCompile Include="..\Private\PlayerCommandSink.cpp">
  <Filter>04. Network</Filter>
</ClCompile>
```

```cpp
// FILE: Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp

#include <limits>
#include <variant>

bool Test_GameplayContracts(TEST_CONTEXT& context)
{
	using namespace LostArk::Shared;

	C2S_USE_SKILL useSkill{};
	useSkill.iSequence = 10;
	useSkill.iSkillId = 1001;
	useSkill.fAimX = 3.f;
	useSkill.fAimY = 0.f;
	useSkill.fAimZ = 7.f;
	CPacketWriter skillWriter;
	context.Expect(Write_Message(skillWriter, useSkill),
		"C2S_USE_SKILL write succeeds");
	C2S_USE_SKILL decodedSkill{};
	CPacketReader skillReader{ skillWriter.Get_Buffer() };
	context.Expect(Read_Message(skillReader, decodedSkill) &&
		0 == skillReader.Get_RemainingSize() &&
		decodedSkill.iSequence == useSkill.iSequence &&
		decodedSkill.iSkillId == useSkill.iSkillId &&
		decodedSkill.fAimX == useSkill.fAimX &&
		decodedSkill.fAimY == useSkill.fAimY &&
		decodedSkill.fAimZ == useSkill.fAimZ,
		"C2S_USE_SKILL round trip");

	S2C_ENTITY_SNAPSHOT snapshot{};
	snapshot.iServerTick = 30;
	ENTITY_SNAPSHOT player{};
	player.iNetEntityId = 100;
	player.eEntityType = ENTITY_TYPE::PLAYER;
	player.Transform.fPositionX = 3.f;
	player.Transform.fPositionY = 0.f;
	player.Transform.fPositionZ = 7.f;
	player.iHp = 900;
	player.iMaxHp = 1000;
	player.eState = ENTITY_STATE::ACTING;
	player.iActionId = 1001;
	player.iActionStartTick = 30;
	snapshot.Entities.push_back(player);
	CPacketWriter snapshotWriter;
	context.Expect(Write_Message(snapshotWriter, snapshot),
		"S2C_ENTITY_SNAPSHOT write succeeds");
	S2C_ENTITY_SNAPSHOT decodedSnapshot{};
	CPacketReader snapshotReader{ snapshotWriter.Get_Buffer() };
	context.Expect(Read_Message(snapshotReader, decodedSnapshot) &&
		0 == snapshotReader.Get_RemainingSize() &&
		1 == decodedSnapshot.Entities.size() &&
		decodedSnapshot.iServerTick == 30 &&
		decodedSnapshot.Entities[0].iNetEntityId == 100 &&
		decodedSnapshot.Entities[0].iHp == 900 &&
		decodedSnapshot.Entities[0].iActionId == 1001,
		"S2C_ENTITY_SNAPSHOT round trip");

	S2C_GAMEPLAY_EVENT gameplayEvent{};
	gameplayEvent.Event.iEventId = 1;
	gameplayEvent.Event.iServerTick = 30;
	DAMAGE_APPLIED_EVENT damage{};
	damage.iSourceNetEntityId = 100;
	damage.iTargetNetEntityId = 200;
	damage.iDamage = 100;
	damage.iRemainingHp = 900;
	gameplayEvent.Event.Payload = damage;
	CPacketWriter eventWriter;
	context.Expect(Write_Message(eventWriter, gameplayEvent),
		"S2C_GAMEPLAY_EVENT write succeeds");
	S2C_GAMEPLAY_EVENT decodedEvent{};
	CPacketReader eventReader{ eventWriter.Get_Buffer() };
	const auto* decodedDamage = std::get_if<DAMAGE_APPLIED_EVENT>(
		&decodedEvent.Event.Payload);
	context.Expect(Read_Message(eventReader, decodedEvent) &&
		0 == eventReader.Get_RemainingSize(),
		"S2C_GAMEPLAY_EVENT read succeeds");
	decodedDamage = std::get_if<DAMAGE_APPLIED_EVENT>(
		&decodedEvent.Event.Payload);
	context.Expect(nullptr != decodedDamage &&
		decodedEvent.Event.iEventId == 1 &&
		decodedDamage->iSourceNetEntityId == 100 &&
		decodedDamage->iTargetNetEntityId == 200 &&
		decodedDamage->iDamage == 100 &&
		decodedDamage->iRemainingHp == 900,
		"S2C_GAMEPLAY_EVENT payload round trip");

	S2C_ENTITY_SNAPSHOT invalid = snapshot;
	invalid.iServerTick = 0;
	CPacketWriter invalidWriter;
	context.Expect(!Write_Message(invalidWriter, invalid) &&
		invalidWriter.Get_Buffer().empty(),
		"entity snapshot rejects zero tick before write");

	invalid = snapshot;
	invalid.Entities[0].iHp = invalid.Entities[0].iMaxHp + 1;
	CPacketWriter hpWriter;
	context.Expect(!Write_Message(hpWriter, invalid) &&
		hpWriter.Get_Buffer().empty(),
		"entity snapshot rejects hp over max before write");

	invalid = snapshot;
	invalid.Entities[0].Transform.fPositionX =
		(std::numeric_limits<float>::quiet_NaN)();
	CPacketWriter nanWriter;
	context.Expect(!Write_Message(nanWriter, invalid) &&
		nanWriter.Get_Buffer().empty(),
		"entity snapshot rejects NaN before write");

	return 0 == context.Get_FailureCount();
}
```

```powershell
$msbuild = "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
& $msbuild Shared\Default\Shared.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
& $msbuild Tools\NetworkProtocolHarness\Default\NetworkProtocolHarness.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
& Tools\NetworkProtocolHarness\Bin\Debug\NetworkProtocolHarness.exe
& $msbuild Client\Default\Client.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
```

```text
NetworkProtocolHarness failures: 0
IPlayerCommandSink is the only PlayerController output boundary
Command contains intent and sequence, never authoritative PlayerId
EntitySnapshot contains current server truth
GameplayEvent contains one-time occurrence with EventId
Client/Server never include each other's implementation headers
```
