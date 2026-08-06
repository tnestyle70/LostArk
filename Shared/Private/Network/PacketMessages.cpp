#include "Network/PacketMessages.h"

#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <utility>

//writer가 일부만 기록한 뒤 실패하지 않도록 snapshot의 모든 state를 먼저 검증한다.
namespace
{
    //유효한 애니메이션인지 검증
    bool Is_Valid_Locomotion(
        LostArk::Shared::PLAYER_LOCOMOTION_STATE state)
    {
        return static_cast<std::uint8_t>(state) <
            static_cast<std::uint8_t>(
                LostArk::Shared::PLAYER_LOCOMOTION_STATE::END);
    }

	bool Is_Valid_PlayerAction(
		const LostArk::Shared::PLAYER_ACTION_STATE state)
	{
		return static_cast<std::uint8_t>(state) <
			static_cast<std::uint8_t>(
				LostArk::Shared::PLAYER_ACTION_STATE::END);
	}

	bool Is_Valid_Stance(
		const LostArk::Shared::PLAYER_STANCE_ID stance)
	{
		return static_cast<std::uint8_t>(stance) <
			static_cast<std::uint8_t>(
				LostArk::Shared::PLAYER_STANCE_ID::END);
	}

	bool Is_Valid_Cooldowns(
		const std::vector<LostArk::Shared::SKILL_COOLDOWN_SNAPSHOT>& cooldowns)
	{
		if (cooldowns.size() > LostArk::Shared::MAX_PLAYER_COOLDOWNS)
			return false;
		for (std::size_t i = 0; i < cooldowns.size(); ++i)
		{
			if (cooldowns[i].iSkillId == LostArk::Shared::INVALID_SKILL_ID ||
				0 == cooldowns[i].iCooldownEndTick)
			{
				return false;
			}
			for (std::size_t j = i + 1; j < cooldowns.size(); ++j)
			{
				if (cooldowns[i].iSkillId == cooldowns[j].iSkillId)
					return false;
			}
		}
		return true;
	}
    //유효한 플레이어 스냅샷인지 검증 - netentityid, position x y z, locomotion state
    bool Is_Valid_PlayerSnapshot(
        const LostArk::Shared::PLAYER_SNAPSHOT& snapshot)
    {
        return
            snapshot.iNetEntityId != LostArk::Shared::INVALID_NET_ENTITY_ID &&
            std::isfinite(snapshot.fPositionX) &&
            std::isfinite(snapshot.fPositionY) &&
            std::isfinite(snapshot.fPositionZ) &&
            std::isfinite(snapshot.fYawDegrees) &&
            Is_Valid_Locomotion(snapshot.eLocomotionState) &&
			Is_Valid_PlayerAction(snapshot.eAction) &&
			Is_Valid_Stance(snapshot.eStance) &&
			0 != snapshot.iMaximumHp &&
			snapshot.iCurrentHp <= snapshot.iMaximumHp &&
			0 != snapshot.iMaximumResource &&
			snapshot.iCurrentResource <= snapshot.iMaximumResource &&
			Is_Valid_Cooldowns(snapshot.Cooldowns) &&
			snapshot.iComboStage <= LostArk::Shared::MAX_COMBO_STAGES &&
			(0 == snapshot.iComboStage ||
				LostArk::Shared::PLAYER_ACTION_STATE::SKILL == snapshot.eAction) &&
			((LostArk::Shared::PLAYER_ACTION_STATE::SKILL == snapshot.eAction &&
				snapshot.iSkillId != LostArk::Shared::INVALID_SKILL_ID &&
				0 != snapshot.iActionStartTick) ||
			 (LostArk::Shared::PLAYER_ACTION_STATE::TRIGGER_MOVE == snapshot.eAction &&
				snapshot.iSkillId == LostArk::Shared::INVALID_SKILL_ID &&
				0 != snapshot.iActionStartTick) ||
			 ((LostArk::Shared::PLAYER_ACTION_STATE::SKILL != snapshot.eAction &&
				LostArk::Shared::PLAYER_ACTION_STATE::TRIGGER_MOVE != snapshot.eAction) &&
				snapshot.iSkillId == LostArk::Shared::INVALID_SKILL_ID));
    }

	bool Is_Valid_StableId(const std::string& value, const bool allowEmpty)
	{
		return (allowEmpty && value.empty()) ||
			(!value.empty() && value.size() <=
				LostArk::Shared::MAX_STABLE_NETWORK_ID_BYTES &&
				std::all_of(value.begin(), value.end(), [](const unsigned char character)
				{
					return 0 != std::isalnum(character) || character == '_' ||
						character == '-' || character == '.';
				}));
	}

	bool Is_Valid_WorldEntityKind(
		const LostArk::Shared::WORLD_ENTITY_KIND kind)
	{
		return static_cast<std::uint8_t>(kind) <
			static_cast<std::uint8_t>(LostArk::Shared::WORLD_ENTITY_KIND::END);
	}

	bool Is_Valid_WorldEntityAction(
		const LostArk::Shared::WORLD_ENTITY_ACTION action)
	{
		return static_cast<std::uint8_t>(action) <
			static_cast<std::uint8_t>(LostArk::Shared::WORLD_ENTITY_ACTION::END);
	}

	bool Is_Valid_WorldEntitySnapshot(
		const LostArk::Shared::WORLD_ENTITY_SNAPSHOT& snapshot)
	{
		return snapshot.iNetEntityId != LostArk::Shared::INVALID_NET_ENTITY_ID &&
			Is_Valid_WorldEntityAction(snapshot.eAction) &&
			Is_Valid_StableId(snapshot.strActionId, true) &&
			std::isfinite(snapshot.fPositionX) &&
			std::isfinite(snapshot.fPositionY) &&
			std::isfinite(snapshot.fPositionZ) &&
			std::isfinite(snapshot.fYawDegrees) &&
			0 != snapshot.iMaximumHp &&
			snapshot.iCurrentHp <= snapshot.iMaximumHp &&
			0 != snapshot.iPhase;
	}

	// A zero amount is not a hit, so it must not reach presentation as one; the
	// server clamps every resolved hit to at least 1.
	bool Is_Valid_DamageEvent(
		const LostArk::Shared::DAMAGE_EVENT& damage)
	{
		return
			damage.iTargetNetEntityId !=
				LostArk::Shared::INVALID_NET_ENTITY_ID &&
			0 != damage.iAmount &&
			std::isfinite(damage.fPositionX) &&
			std::isfinite(damage.fPositionY) &&
			std::isfinite(damage.fPositionZ);
	}
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, const C2S_ENTER_WORLD& message)
{
	if (NETWORK_PROTOCOL_VERSION != message.iProtocolVersion ||
		!Is_Known_World_Id(message.eWorldId))
	{
		return false;
	}

    //character class write
    const std::uint8_t rawCharacterClass =
        static_cast<std::uint8_t>(
            message.eCharacterClass);

    if (rawCharacterClass >= static_cast<std::uint8_t>(
        CHARACTER_CLASS_ID::END))
        return false;

    //message의 nickname 검증
    if (message.strNickName.empty())
        return false;

    if (message.strNickName.size() >
        MAX_NICKNAME_BYTES)
        return false;

	writer.Write_U16(message.iProtocolVersion);
	writer.Write_U16(static_cast<std::uint16_t>(message.eWorldId));

	//character class write
	writer.Write_U8(rawCharacterClass);

    //nickname write
    return writer.Write_String(
        message.strNickName,
        MAX_NICKNAME_BYTES);
}

bool LostArk::Shared::Read_Message(CPacketReader& reader, C2S_ENTER_WORLD& message)
{
	std::uint16_t protocolVersion = {};
	std::uint16_t rawWorldId = {};
	std::uint8_t rawCharacterClass = {};
	std::string nickName;

	if (!reader.Read_U16(protocolVersion) ||
		!reader.Read_U16(rawWorldId) ||
		NETWORK_PROTOCOL_VERSION != protocolVersion ||
		!Is_Known_World_Id(static_cast<WORLD_ID>(rawWorldId)))
	{
		return false;
	}

    //character class 예외처리
    if (!reader.Read_U8(rawCharacterClass))
        return false;

    if (rawCharacterClass >= static_cast<std::uint8_t>(
        CHARACTER_CLASS_ID::END))
        return false;

    //nickname read
    if (!reader.Read_String(
        nickName,
        MAX_NICKNAME_BYTES))
        return false;

    if (nickName.empty())
        return false;

	C2S_ENTER_WORLD decoded{};
	decoded.iProtocolVersion = protocolVersion;
	decoded.eWorldId = static_cast<WORLD_ID>(rawWorldId);

    decoded.eCharacterClass =
        static_cast<CHARACTER_CLASS_ID>(
            rawCharacterClass);

    decoded.strNickName = std::move(nickName);

    //const라서 대입이 안 되는 상황?
    message = std::move(decoded);

    return true;
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, 
    const S2C_ENTER_ACCEPTED& message)
{
	if (NETWORK_PROTOCOL_VERSION != message.iProtocolVersion ||
		!Is_Known_World_Id(message.eWorldId))
	{
		return false;
	}

    //playerid가 유효한지 검사
    if (message.iPlayerId == INVALID_PLAYER_ID)
        return false;

    //netid가 유효한지 검사
    if (message.iNetEntityId == INVALID_NET_ENTITY_ID)
        return false;

	writer.Write_U16(message.iProtocolVersion);
	writer.Write_U16(static_cast<std::uint16_t>(message.eWorldId));

	//playerid, netentityid를 u32로 기록
    writer.Write_U32(message.iPlayerId);
    writer.Write_U32(message.iNetEntityId);

    return true;
}

bool LostArk::Shared::Read_Message(CPacketReader& reader,
   S2C_ENTER_ACCEPTED& message)
{
	std::uint16_t protocolVersion = {};
	std::uint16_t rawWorldId = {};
	PLAYER_ID playerId =
        INVALID_PLAYER_ID;

    NET_ENTITY_ID netEntityId =
        INVALID_NET_ENTITY_ID;

	if (!reader.Read_U16(protocolVersion) ||
		!reader.Read_U16(rawWorldId) ||
		NETWORK_PROTOCOL_VERSION != protocolVersion ||
		!Is_Known_World_Id(static_cast<WORLD_ID>(rawWorldId)))
	{
		return false;
	}

	if (!reader.Read_U32(playerId))
        return false;
    if (!reader.Read_U32(netEntityId))
        return false;

    if (playerId == 0 || netEntityId == 0)
        return false;

	S2C_ENTER_ACCEPTED decoded {};

	decoded.iProtocolVersion = protocolVersion;
	decoded.eWorldId = static_cast<WORLD_ID>(rawWorldId);
	decoded.iPlayerId = playerId;
    decoded.iNetEntityId = netEntityId;
   
    message = decoded;

    return true;
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, 
    const S2C_PLAYER_SPAWNED& spawned)
{
    //playerid가 유효한지 검사
    if (spawned.iPlayerId == INVALID_PLAYER_ID)
        return false;

    //netid가 유효한지 검사
    if (spawned.iNetEntityId == INVALID_NET_ENTITY_ID)
        return false;

    //character class가 end인지 검사
    const std::uint8_t rawCharacterClass =
        static_cast<std::uint8_t>(spawned.eCharacterClass);

    if (rawCharacterClass >=
        static_cast<std::uint8_t>(CHARACTER_CLASS_ID::END))
        return false;

    //Nickname이 비어있는지 검사
    if (spawned.strNickName.empty() ||
        spawned.strNickName.length() > MAX_NICKNAME_BYTES)
        return false;

    //position X/Y/Z가 finite인지 검사
    if (!std::isfinite(spawned.fPositionX) ||
        !std::isfinite(spawned.fPositionY) ||
        !std::isfinite(spawned.fPositionZ))
        return false;

    //yawDegrees가 finite인지 검사
    if (!std::isfinite(spawned.fYawDegrees))
        return false;

    //playerid, net entity, character class, nickname
    //position x y z, yawdegrees를 u32로 기록
    writer.Write_U32(spawned.iPlayerId);
    writer.Write_U32(spawned.iNetEntityId);

    writer.Write_U8(rawCharacterClass);

    if (!writer.Write_String(
        spawned.strNickName,
        MAX_NICKNAME_BYTES))
    {
        return false;
    }
    
    writer.Write_F32(spawned.fPositionX);
    writer.Write_F32(spawned.fPositionY);
    writer.Write_F32(spawned.fPositionZ);
    writer.Write_F32(spawned.fYawDegrees);

    return true;
}

bool LostArk::Shared::Read_Message(CPacketReader& reader, 
    S2C_PLAYER_SPAWNED& spawned)
{
    PLAYER_ID iPlayerId =
        INVALID_PLAYER_ID;

    NET_ENTITY_ID iNetEntityId =
        INVALID_NET_ENTITY_ID;

    std::uint8_t rawCharacterClass = {};
    std::string nickName;

    float positionX = 0.f;
    float positionY = 0.f;
    float positionZ = 0.f;
    float yawDegrees = 0.f;

    if (!reader.Read_U32(iPlayerId))
        return false;

    if (!reader.Read_U32(iNetEntityId))
        return false;

    if (!reader.Read_U8(rawCharacterClass))
        return false;

    if (!reader.Read_String(
        nickName,
        MAX_NICKNAME_BYTES))
    {
        return false;
    }

    if (!reader.Read_F32(positionX))
        return false;

    if (!reader.Read_F32(positionY))
        return false;

    if (!reader.Read_F32(positionZ))
        return false;

    if (!reader.Read_F32(yawDegrees))
        return false;

    if (iPlayerId == INVALID_PLAYER_ID)
        return false;

    if (iNetEntityId == INVALID_NET_ENTITY_ID)
        return false;

    if (rawCharacterClass >=
        static_cast<std::uint8_t>(
            CHARACTER_CLASS_ID::END))
    {
        return false;
    }

    if (nickName.empty())
        return false;

    if (!std::isfinite(positionX) ||
        !std::isfinite(positionY) ||
        !std::isfinite(positionZ) ||
        !std::isfinite(yawDegrees))
    {
        return false;
    }



    S2C_PLAYER_SPAWNED decoded{};

    decoded.iPlayerId = iPlayerId;
    decoded.iNetEntityId = iNetEntityId;

    decoded.eCharacterClass = static_cast<CHARACTER_CLASS_ID>(
        rawCharacterClass);

    decoded.strNickName =
        std::move(nickName);

    decoded.fPositionX = positionX;
    decoded.fPositionY = positionY;
    decoded.fPositionZ = positionZ;
    decoded.fYawDegrees = yawDegrees;

    spawned = std::move(decoded);

    return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_WORLD_ENTITY_SPAWNED& spawned)
{
	if (spawned.iNetEntityId == INVALID_NET_ENTITY_ID ||
		!Is_Valid_WorldEntityKind(spawned.eKind) ||
		!Is_Valid_StableId(spawned.strArchetypeId, false) ||
		!Is_Valid_StableId(spawned.strEncounterId, true) ||
		!std::isfinite(spawned.fPositionX) ||
		!std::isfinite(spawned.fPositionY) ||
		!std::isfinite(spawned.fPositionZ) ||
		!std::isfinite(spawned.fYawDegrees))
	{
		return false;
	}
	writer.Write_U32(spawned.iNetEntityId);
	writer.Write_U8(static_cast<std::uint8_t>(spawned.eKind));
	if (!writer.Write_String(
		spawned.strArchetypeId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!writer.Write_String(
		spawned.strEncounterId, MAX_STABLE_NETWORK_ID_BYTES))
	{
		return false;
	}
	writer.Write_F32(spawned.fPositionX);
	writer.Write_F32(spawned.fPositionY);
	writer.Write_F32(spawned.fPositionZ);
	writer.Write_F32(spawned.fYawDegrees);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_WORLD_ENTITY_SPAWNED& spawned)
{
	S2C_WORLD_ENTITY_SPAWNED decoded{};
	std::uint8_t rawKind = 0;
	if (!reader.Read_U32(decoded.iNetEntityId) ||
		!reader.Read_U8(rawKind) ||
		!reader.Read_String(
			decoded.strArchetypeId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_String(
			decoded.strEncounterId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_F32(decoded.fPositionX) ||
		!reader.Read_F32(decoded.fPositionY) ||
		!reader.Read_F32(decoded.fPositionZ) ||
		!reader.Read_F32(decoded.fYawDegrees))
	{
		return false;
	}
	decoded.eKind = static_cast<WORLD_ENTITY_KIND>(rawKind);
	if (decoded.iNetEntityId == INVALID_NET_ENTITY_ID ||
		!Is_Valid_WorldEntityKind(decoded.eKind) ||
		!Is_Valid_StableId(decoded.strArchetypeId, false) ||
		!Is_Valid_StableId(decoded.strEncounterId, true) ||
		!std::isfinite(decoded.fPositionX) ||
		!std::isfinite(decoded.fPositionY) ||
		!std::isfinite(decoded.fPositionZ) ||
		!std::isfinite(decoded.fYawDegrees))
	{
		return false;
	}
	spawned = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_WORLD_ENTITY_DESPAWNED& despawned)
{
	if (INVALID_NET_ENTITY_ID == despawned.iNetEntityId)
		return false;

	writer.Write_U32(despawned.iNetEntityId);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_WORLD_ENTITY_DESPAWNED& despawned)
{
	S2C_WORLD_ENTITY_DESPAWNED decoded{};
	if (!reader.Read_U32(decoded.iNetEntityId) ||
		INVALID_NET_ENTITY_ID == decoded.iNetEntityId)
	{
		return false;
	}

	despawned = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_SPAWN_WORLD_ENTITY& message)
{
	return Is_Valid_StableId(message.strPlacementId, false) &&
		writer.Write_String(
			message.strPlacementId,
			MAX_STABLE_NETWORK_ID_BYTES);
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_SPAWN_WORLD_ENTITY& message)
{
	std::string placementId;
	if (!reader.Read_String(placementId, MAX_STABLE_NETWORK_ID_BYTES) ||
		!Is_Valid_StableId(placementId, false))
	{
		return false;
	}

	C2S_SPAWN_WORLD_ENTITY decoded{};
	decoded.strPlacementId = std::move(placementId);
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const S2C_WORLD_ENTITY_SPAWN_RESULT& message)
{
	const std::uint8_t rawResult =
		static_cast<std::uint8_t>(message.eResult);
	const bool hasEntity =
		WORLD_ENTITY_SPAWN_RESULT::SPAWNED == message.eResult ||
		WORLD_ENTITY_SPAWN_RESULT::ALREADY_EXISTS == message.eResult;
	if (!Is_Valid_StableId(message.strPlacementId, false) ||
		rawResult >= static_cast<std::uint8_t>(
			WORLD_ENTITY_SPAWN_RESULT::END) ||
		(hasEntity && INVALID_NET_ENTITY_ID == message.iNetEntityId) ||
		(!hasEntity && INVALID_NET_ENTITY_ID != message.iNetEntityId))
	{
		return false;
	}
	if (!writer.Write_String(
		message.strPlacementId,
		MAX_STABLE_NETWORK_ID_BYTES))
	{
		return false;
	}
	writer.Write_U8(rawResult);
	writer.Write_U32(message.iNetEntityId);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	S2C_WORLD_ENTITY_SPAWN_RESULT& message)
{
	S2C_WORLD_ENTITY_SPAWN_RESULT decoded{};
	std::uint8_t rawResult = 0;
	if (!reader.Read_String(
		decoded.strPlacementId,
		MAX_STABLE_NETWORK_ID_BYTES) ||
		!reader.Read_U8(rawResult) ||
		!reader.Read_U32(decoded.iNetEntityId))
	{
		return false;
	}
	decoded.eResult = static_cast<WORLD_ENTITY_SPAWN_RESULT>(rawResult);
	const bool hasEntity =
		WORLD_ENTITY_SPAWN_RESULT::SPAWNED == decoded.eResult ||
		WORLD_ENTITY_SPAWN_RESULT::ALREADY_EXISTS == decoded.eResult;
	if (!Is_Valid_StableId(decoded.strPlacementId, false) ||
		rawResult >= static_cast<std::uint8_t>(
			WORLD_ENTITY_SPAWN_RESULT::END) ||
		(hasEntity && INVALID_NET_ENTITY_ID == decoded.iNetEntityId) ||
		(!hasEntity && INVALID_NET_ENTITY_ID != decoded.iNetEntityId))
	{
		return false;
	}
	message = std::move(decoded);
	return true;
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, const S2C_PLAYER_DESPAWNED& message)
{
    //검증
    if (message.iNetEntityId == INVALID_NET_ENTITY_ID)
        return false;

    const std::uint8_t rawReason =
        static_cast<std::uint8_t>(message.eReason);

    if (rawReason >= static_cast<std::uint8_t>(
        PLAYER_DESPAWN_REASON::END))
    {
        return false;
    }

    //정보 쓰기
    writer.Write_U32(message.iNetEntityId);
    writer.Write_U8(rawReason);

    return true;
}

bool LostArk::Shared::Read_Message(CPacketReader& reader, S2C_PLAYER_DESPAWNED& message)
{
    NET_ENTITY_ID netEntityId = INVALID_NET_ENTITY_ID;
    std::uint8_t rawReason = {};

    if (!reader.Read_U32(netEntityId))
        return false;

    if (!reader.Read_U8(rawReason))
        return false;

    if (netEntityId == INVALID_NET_ENTITY_ID)
        return false;

    if (rawReason >= static_cast<std::uint8_t>(
        PLAYER_DESPAWN_REASON::END))
        return false;

    S2C_PLAYER_DESPAWNED decoded{};

    decoded.iNetEntityId = netEntityId;
    decoded.eReason = static_cast<PLAYER_DESPAWN_REASON>(
        rawReason);

    message = decoded;

    return true;
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, const C2S_MOVE& message)
{
    if (0 == message.iClientSequence ||
        !std::isfinite(message.fGoalX) ||
        !std::isfinite(message.fGoalZ))
    {
        return false;
    }

    writer.Write_U32(message.iClientSequence);
    writer.Write_F32(message.fGoalX);
    writer.Write_F32(message.fGoalZ);

    return true;
}

bool LostArk::Shared::Read_Message(CPacketReader& reader, C2S_MOVE& message)
{
    std::uint32_t clientSequence = 0;
    float fGoalX = 0.f;
    float fGoalZ = 0.f;

    if (!reader.Read_U32(clientSequence) ||
        !reader.Read_F32(fGoalX) ||
        !reader.Read_F32(fGoalZ))
    {
        return false;
    }

    if (0 == clientSequence ||
        !std::isfinite(fGoalX) ||
        !std::isfinite(fGoalZ))
    {
        return false;
    }

    C2S_MOVE decoded{};
    decoded.iClientSequence = clientSequence;
    decoded.fGoalX = fGoalX;
    decoded.fGoalZ = fGoalZ;

    message = decoded;
    return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_USE_SKILL& message)
{
	if (0 == message.iClientSequence ||
		INVALID_SKILL_ID == message.iSkillId ||
		!std::isfinite(message.fAimX) ||
		!std::isfinite(message.fAimZ))
	{
		return false;
	}

	writer.Write_U32(message.iClientSequence);
	writer.Write_U32(message.iSkillId);
	writer.Write_F32(message.fAimX);
	writer.Write_F32(message.fAimZ);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_USE_SKILL& message)
{
	C2S_USE_SKILL decoded{};
	if (!reader.Read_U32(decoded.iClientSequence) ||
		!reader.Read_U32(decoded.iSkillId) ||
		!reader.Read_F32(decoded.fAimX) ||
		!reader.Read_F32(decoded.fAimZ) ||
		0 == decoded.iClientSequence ||
		INVALID_SKILL_ID == decoded.iSkillId ||
		!std::isfinite(decoded.fAimX) ||
		!std::isfinite(decoded.fAimZ))
	{
		return false;
	}

	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(
	CPacketWriter& writer,
	const C2S_RELEASE_SKILL& message)
{
	if (0 == message.iClientSequence ||
		INVALID_SKILL_ID == message.iSkillId)
	{
		return false;
	}

	writer.Write_U32(message.iClientSequence);
	writer.Write_U32(message.iSkillId);
	return true;
}

bool LostArk::Shared::Read_Message(
	CPacketReader& reader,
	C2S_RELEASE_SKILL& message)
{
	C2S_RELEASE_SKILL decoded{};
	if (!reader.Read_U32(decoded.iClientSequence) ||
		!reader.Read_U32(decoded.iSkillId) ||
		0 == decoded.iClientSequence ||
		INVALID_SKILL_ID == decoded.iSkillId)
	{
		return false;
	}

	message = decoded;
	return true;
}

bool LostArk::Shared::Write_Message(CPacketWriter& writer, const S2C_WORLD_SNAPSHOT& message)
{
    //world의 snapshot write, servertick과 player 정보
	if (0 == message.iServerTick ||
		!Is_Known_World_Id(message.eWorldId) ||
        message.Players.empty() ||
        message.Players.size() >
        MAX_WORLD_SNAPSHOT_PLAYERS ||
		message.Entities.size() > MAX_WORLD_SNAPSHOT_ENTITIES ||
		message.DamageEvents.size() > MAX_DAMAGE_EVENTS)
    {
        return false;
    }
    //유효하지 않은 플레이어 스냅샷 검사
    for (const PLAYER_SNAPSHOT& player : message.Players)
    {
        if (!Is_Valid_PlayerSnapshot(player))
            return false;
    }
	for (const WORLD_ENTITY_SNAPSHOT& entity : message.Entities)
	{
		if (!Is_Valid_WorldEntitySnapshot(entity))
			return false;
	}
	for (const DAMAGE_EVENT& damage : message.DamageEvents)
	{
		if (!Is_Valid_DamageEvent(damage))
			return false;
	}
    //server tick과 player size 넣기
	writer.Write_U32(message.iServerTick);
	writer.Write_U16(
		static_cast<std::uint16_t>(message.eWorldId));
	writer.Write_U16(
        static_cast<std::uint16_t>(
            message.Players.size()));
	writer.Write_U16(
		static_cast<std::uint16_t>(message.Entities.size()));
	// U8 is enough: MAX_DAMAGE_EVENTS bounds one tick, far under 255.
	writer.Write_U8(
		static_cast<std::uint8_t>(message.DamageEvents.size()));

    for (const PLAYER_SNAPSHOT& player : message.Players)
    {
        writer.Write_U32(player.iNetEntityId);
        writer.Write_F32(player.fPositionX);
        writer.Write_F32(player.fPositionY);
        writer.Write_F32(player.fPositionZ);
        writer.Write_F32(player.fYawDegrees);
        writer.Write_U8(
            static_cast<std::uint8_t>(
                player.eLocomotionState));
		writer.Write_U8(static_cast<std::uint8_t>(player.eAction));
		writer.Write_U8(static_cast<std::uint8_t>(player.eStance));
		writer.Write_U32(player.iSkillId);
		writer.Write_U32(player.iActionStartTick);
		writer.Write_U32(player.iCurrentHp);
		writer.Write_U32(player.iMaximumHp);
		writer.Write_U32(player.iCurrentResource);
		writer.Write_U32(player.iMaximumResource);
		writer.Write_U8(player.iComboStage);
		writer.Write_U8(static_cast<std::uint8_t>(player.Cooldowns.size()));
		for (const SKILL_COOLDOWN_SNAPSHOT& cooldown : player.Cooldowns)
		{
			writer.Write_U32(cooldown.iSkillId);
			writer.Write_U32(cooldown.iCooldownEndTick);
		}
    }
	for (const WORLD_ENTITY_SNAPSHOT& entity : message.Entities)
	{
		writer.Write_U32(entity.iNetEntityId);
		writer.Write_U8(static_cast<std::uint8_t>(entity.eAction));
		if (!writer.Write_String(
			entity.strActionId, MAX_STABLE_NETWORK_ID_BYTES))
		{
			return false;
		}
		writer.Write_F32(entity.fPositionX);
		writer.Write_F32(entity.fPositionY);
		writer.Write_F32(entity.fPositionZ);
		writer.Write_F32(entity.fYawDegrees);
		writer.Write_U32(entity.iActionStartTick);
		writer.Write_U32(entity.iCurrentHp);
		writer.Write_U32(entity.iMaximumHp);
		writer.Write_U8(entity.iPhase);
	}
	for (const DAMAGE_EVENT& damage : message.DamageEvents)
	{
		writer.Write_U32(damage.iTargetNetEntityId);
		writer.Write_U32(damage.iAmount);
		writer.Write_F32(damage.fPositionX);
		writer.Write_F32(damage.fPositionY);
		writer.Write_F32(damage.fPositionZ);
		writer.Write_U8(damage.isOutgoing ? 1u : 0u);
	}

    return true;
}

bool LostArk::Shared::Read_Message(CPacketReader& reader, S2C_WORLD_SNAPSHOT& message)
{
	std::uint32_t serverTick = 0;
	std::uint16_t rawWorldId = 0;
	std::uint16_t playerCount = 0;
	std::uint16_t entityCount = 0;
	std::uint8_t damageEventCount = 0;

	if (!reader.Read_U32(serverTick) ||
		!reader.Read_U16(rawWorldId) ||
		!reader.Read_U16(playerCount) ||
		!reader.Read_U16(entityCount) ||
		!reader.Read_U8(damageEventCount))
    {
        return false;
    }

	if (0 == serverTick ||
		!Is_Known_World_Id(static_cast<WORLD_ID>(rawWorldId)) ||
        0 == playerCount ||
        playerCount > MAX_WORLD_SNAPSHOT_PLAYERS ||
		entityCount > MAX_WORLD_SNAPSHOT_ENTITIES ||
		damageEventCount > MAX_DAMAGE_EVENTS)
    {
        return false;
    }

	S2C_WORLD_SNAPSHOT decoded{};
	decoded.iServerTick = serverTick;
	decoded.eWorldId = static_cast<WORLD_ID>(rawWorldId);
    decoded.Players.reserve(playerCount);
	decoded.Entities.reserve(entityCount);
	decoded.DamageEvents.reserve(damageEventCount);

    for (std::uint16_t i = 0; i < playerCount; ++i)
    {
        PLAYER_SNAPSHOT player{};
        std::uint8_t rawLocomotion = 0;
		std::uint8_t rawAction = 0;
		std::uint8_t rawStance = 0;
		std::uint8_t cooldownCount = 0;

        if (!reader.Read_U32(player.iNetEntityId) ||
            !reader.Read_F32(player.fPositionX) ||
            !reader.Read_F32(player.fPositionY) ||
            !reader.Read_F32(player.fPositionZ) ||
            !reader.Read_F32(player.fYawDegrees) ||
            !reader.Read_U8(rawLocomotion) ||
			!reader.Read_U8(rawAction) ||
			!reader.Read_U8(rawStance) ||
			!reader.Read_U32(player.iSkillId) ||
			!reader.Read_U32(player.iActionStartTick) ||
			!reader.Read_U32(player.iCurrentHp) ||
			!reader.Read_U32(player.iMaximumHp) ||
			!reader.Read_U32(player.iCurrentResource) ||
			!reader.Read_U32(player.iMaximumResource) ||
			!reader.Read_U8(player.iComboStage) ||
			player.iComboStage > MAX_COMBO_STAGES ||
			!reader.Read_U8(cooldownCount) ||
			cooldownCount > MAX_PLAYER_COOLDOWNS)
        {
            return false;
        }

        player.eLocomotionState =
            static_cast<PLAYER_LOCOMOTION_STATE>(
                rawLocomotion);
		player.eAction = static_cast<PLAYER_ACTION_STATE>(rawAction);
		player.eStance = static_cast<PLAYER_STANCE_ID>(rawStance);
		player.Cooldowns.reserve(cooldownCount);
		for (std::uint8_t cooldownIndex = 0;
			cooldownIndex < cooldownCount;
			++cooldownIndex)
		{
			SKILL_COOLDOWN_SNAPSHOT cooldown{};
			if (!reader.Read_U32(cooldown.iSkillId) ||
				!reader.Read_U32(cooldown.iCooldownEndTick))
			{
				return false;
			}
			player.Cooldowns.push_back(cooldown);
		}

        if (!Is_Valid_PlayerSnapshot(player))
            return false;

        decoded.Players.push_back(player);
    }
	for (std::uint16_t i = 0; i < entityCount; ++i)
	{
		WORLD_ENTITY_SNAPSHOT entity{};
		std::uint8_t rawAction = 0;
		if (!reader.Read_U32(entity.iNetEntityId) ||
			!reader.Read_U8(rawAction) ||
			!reader.Read_String(
				entity.strActionId, MAX_STABLE_NETWORK_ID_BYTES) ||
			!reader.Read_F32(entity.fPositionX) ||
			!reader.Read_F32(entity.fPositionY) ||
			!reader.Read_F32(entity.fPositionZ) ||
			!reader.Read_F32(entity.fYawDegrees) ||
			!reader.Read_U32(entity.iActionStartTick) ||
			!reader.Read_U32(entity.iCurrentHp) ||
			!reader.Read_U32(entity.iMaximumHp) ||
			!reader.Read_U8(entity.iPhase))
		{
			return false;
		}
		entity.eAction = static_cast<WORLD_ENTITY_ACTION>(rawAction);
		if (!Is_Valid_WorldEntitySnapshot(entity))
			return false;
		decoded.Entities.push_back(std::move(entity));
	}
	for (std::uint8_t i = 0; i < damageEventCount; ++i)
	{
		DAMAGE_EVENT damage{};
		std::uint8_t rawOutgoing = 0;
		if (!reader.Read_U32(damage.iTargetNetEntityId) ||
			!reader.Read_U32(damage.iAmount) ||
			!reader.Read_F32(damage.fPositionX) ||
			!reader.Read_F32(damage.fPositionY) ||
			!reader.Read_F32(damage.fPositionZ) ||
			!reader.Read_U8(rawOutgoing) ||
			rawOutgoing > 1u)
		{
			return false;
		}
		damage.isOutgoing = 0u != rawOutgoing;
		if (!Is_Valid_DamageEvent(damage))
			return false;
		decoded.DamageEvents.push_back(damage);
	}

    message = std::move(decoded);
    return true;
}
