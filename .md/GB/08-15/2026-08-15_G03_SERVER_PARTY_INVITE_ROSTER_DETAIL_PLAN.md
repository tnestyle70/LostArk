# 2026-08-15 G03 Server Party Invite / Roster 코드 상세 계획서

## 0. 적용 상태와 사용법

- 이 문서는 `C:\Users\user\Desktop\LostArk`에서 G02가 적용되고 검증됐다는 전제로 작성한 G03 직접 반영용 코드 계획서다.
- 이 문서의 코드는 아직 제품 소스에 적용하지 않는다. G02 RESULT를 닫은 뒤 아래 순서대로 적용한다.
- 실제 적용 직전 각 파일의 `git diff`를 다시 읽고 다른 세션의 Effect 변경과 겹치는 block은 수동 병합한다.
- 신규 H/CPP는 UTF-8 BOM 없이 만들고 `.vcxproj`와 `.filters`를 같은 변경 단위에 등록한다.
- 기존 대형 H/CPP는 아래의 선언/함수 전체 교체 block을 기준점에 적용한다. 해당 block 밖의 기존 코드를 삭제하거나 정렬하지 않는다.
- `...`, TODO, 임시 fake roster, hardcoded player list는 적용 코드에 넣지 않는다.

### 0.1 기존 UI shell 보존 계약

- UI 변경 `768d8a8d`가 만든 `CChatWindowView`와 `CPartyWindowView`는 실제 물리 소스와 `Client.vcxproj/.filters`에 이미 존재한다. 같은 이름의 파일이나 두 번째 view를 만들거나 기존 project item을 재등록하지 않는다.
- Chat은 Enter/Escape, IME, local echo, fade까지만 구현된 presentation shell이다. G03에서는 `ChatWindowView.*`를 수정하지 않고 `MainApp.cpp`의 Chat 생성/input/render/capture block을 그대로 보존한다.
- Party는 `PANEL_X=20`, `PANEL_Y=720/3`인 왼쪽 foreground panel이며 constructor의 가짜 4명만 제거한다. title, native art size, row 간격, `CUITextureCache` 경로는 보존하고 data owner만 `CPartyViewModel`로 교체한다.
- `UI/Chat` 5개와 `UI/Party` 8개는 Git 제외 `Client/Bin/Resources`의 물리 runtime 입력이다. `Data/UI`나 프로젝트별 리소스 복사본을 만들지 않는다.
- Invite/Cancel popup과 RMB hover는 기존 UI 작업물에 없다. §6의 `CWorldPlayerSocialInteractionView`가 G03에서 새로 추가하는 임시 ImGui command surface다.

## 1. G03 적용 순서

| 순서 | 검증 가능한 계약 | 주요 파일 |
|---:|---|---|
| G03-01 | Shared party wire + codec | `PartyContracts`, `PartyMessages`, `PacketType` |
| G03-02 | Server social identity + immediate party authority | `GameRoom`, `ServerPartyService`, `ServerApp` |
| G03-03 | Client packet boundary + roster VM | `NetworkManager`, `PartyCommandSink`, `PartyViewModel` |
| G03-04 | remote hover/RMB + Party UI | `WorldPlayerSocialInteractionView`, Bern/Valtan, `PartyWindowView`, `MainApp` |
| G03-05 | protocol/server/frontend/live regression | 기존 세 harness와 RESULT |

## 2. Shared 전체 계약

### 2.1 `Shared/Public/Party/PartyContracts.h` 신규 전체 파일

```cpp
#pragma once

#include "Network/NetworkIds.h"
#include "Network/PacketType.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace LostArk::Shared
{
    using SOCIAL_PLAYER_ID = std::uint64_t;
    using PARTY_ID = std::uint64_t;

    inline constexpr SOCIAL_PLAYER_ID INVALID_SOCIAL_PLAYER_ID = 0u;
    inline constexpr PARTY_ID INVALID_PARTY_ID = 0u;
    inline constexpr std::size_t MAX_PARTY_MEMBERS = 4u;

    enum class PARTY_COMMAND_RESULT : std::uint8_t
    {
        ACCEPTED,
        REJECTED_WRONG_WORLD,
        REJECTED_SELF,
        REJECTED_TARGET_NOT_FOUND,
        REJECTED_NOT_LEADER,
        REJECTED_ALREADY_MEMBER,
        REJECTED_TARGET_IN_OTHER_PARTY,
        REJECTED_PARTY_FULL,
        REJECTED_STALE_REQUEST,
        END
    };

    [[nodiscard]] constexpr bool Is_Known_PartyCommandResult(
        const PARTY_COMMAND_RESULT result)
    {
        return static_cast<std::uint8_t>(result) <
            static_cast<std::uint8_t>(PARTY_COMMAND_RESULT::END);
    }

    struct C2S_PARTY_INVITE final
    {
        std::uint32_t iRequestSequence = 0u;
        WORLD_ID eTargetWorldId = WORLD_ID::END;
        NET_ENTITY_ID iTargetNetEntityId = INVALID_NET_ENTITY_ID;
    };

    struct S2C_PARTY_COMMAND_RESULT final
    {
        std::uint32_t iRequestSequence = 0u;
        PARTY_COMMAND_RESULT eResult = PARTY_COMMAND_RESULT::END;
        SOCIAL_PLAYER_ID iTargetSocialPlayerId = INVALID_SOCIAL_PLAYER_ID;
    };

    struct PARTY_MEMBER_WIRE final
    {
        SOCIAL_PLAYER_ID iSocialPlayerId = INVALID_SOCIAL_PLAYER_ID;
        WORLD_ID eWorldId = WORLD_ID::END;
        NET_ENTITY_ID iNetEntityId = INVALID_NET_ENTITY_ID;
        CHARACTER_CLASS_ID eCharacterClass = CHARACTER_CLASS_ID::END;
        std::string strNickname;
        std::uint32_t iCurrentHp = 0u;
        std::uint32_t iMaximumHp = 0u;
        std::uint8_t iPartySlot = 0u;
        bool isLeader = false;
        bool isPresent = false;
    };

    struct S2C_PARTY_ROSTER final
    {
        std::uint64_t iRosterRevision = 0u;
        PARTY_ID iPartyId = INVALID_PARTY_ID;
        SOCIAL_PLAYER_ID iSelfSocialPlayerId = INVALID_SOCIAL_PLAYER_ID;
        std::vector<PARTY_MEMBER_WIRE> Members;
    };

    [[nodiscard]] bool Is_Valid_PartyRoster(
        const S2C_PARTY_ROSTER& roster) noexcept;
}
```

### 2.2 `Shared/Public/Party/PartyMessages.h` 신규 전체 파일

```cpp
#pragma once

#include "Party/PartyContracts.h"

namespace LostArk::Shared
{
    class CPacketReader;
    class CPacketWriter;

    bool Write_Message(
        CPacketWriter& writer,
        const C2S_PARTY_INVITE& message);
    bool Read_Message(
        CPacketReader& reader,
        C2S_PARTY_INVITE& message);

    bool Write_Message(
        CPacketWriter& writer,
        const S2C_PARTY_COMMAND_RESULT& message);
    bool Read_Message(
        CPacketReader& reader,
        S2C_PARTY_COMMAND_RESULT& message);

    bool Write_Message(
        CPacketWriter& writer,
        const S2C_PARTY_ROSTER& message);
    bool Read_Message(
        CPacketReader& reader,
        S2C_PARTY_ROSTER& message);
}
```

### 2.3 Packet Reader/Writer U64 추가

`Shared/Public/Network/PacketWriter.h`의 `Write_U32` 다음에 추가한다.

```cpp
        void Write_U64(std::uint64_t value);
```

`Shared/Private/Network/PacketWriter.cpp`의 `Write_U32` 다음에 추가한다.

```cpp
void LostArk::Shared::CPacketWriter::Write_U64(
    const std::uint64_t value)
{
    Write_U32(static_cast<std::uint32_t>(value & 0xFFFFFFFFull));
    Write_U32(static_cast<std::uint32_t>(value >> 32u));
}
```

`Shared/Public/Network/PacketReader.h`의 `Read_U32` 다음에 추가한다.

```cpp
        bool Read_U64(std::uint64_t& value);
```

`Shared/Private/Network/PacketReader.cpp`의 `Read_U32` 다음에 추가한다. 두 U32를 모두 읽은 뒤에만 output을 commit한다.

```cpp
bool LostArk::Shared::CPacketReader::Read_U64(std::uint64_t& value)
{
    std::uint32_t low = 0u;
    std::uint32_t high = 0u;
    if (!Read_U32(low) || !Read_U32(high))
        return false;
    value = static_cast<std::uint64_t>(low) |
        (static_cast<std::uint64_t>(high) << 32u);
    return true;
}
```

### 2.4 `Shared/Private/Party/PartyMessages.cpp` 신규 전체 파일

```cpp
#include "Party/PartyMessages.h"

#include "Network/PacketMessages.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"

#include <algorithm>
#include <array>
#include <unordered_set>
#include <utility>

namespace
{
    using namespace LostArk::Shared;

    bool Is_Valid_PresentBinding(const PARTY_MEMBER_WIRE& member)
    {
        if (!member.isPresent)
        {
            return WORLD_ID::END == member.eWorldId &&
                INVALID_NET_ENTITY_ID == member.iNetEntityId;
        }
        return Is_Known_World_Id(member.eWorldId) &&
            WORLD_ID::CHARACTER_SELECT_ARENA != member.eWorldId &&
            WORLD_ID::TRAINING_GROUND != member.eWorldId &&
            INVALID_NET_ENTITY_ID != member.iNetEntityId;
    }
}

bool LostArk::Shared::Is_Valid_PartyRoster(
    const S2C_PARTY_ROSTER& roster) noexcept
{
    if (0u == roster.iRosterRevision ||
        INVALID_SOCIAL_PLAYER_ID == roster.iSelfSocialPlayerId)
    {
        return false;
    }

    if (roster.Members.empty())
        return INVALID_PARTY_ID == roster.iPartyId;

    if (INVALID_PARTY_ID == roster.iPartyId ||
        roster.Members.size() < 2u ||
        roster.Members.size() > MAX_PARTY_MEMBERS)
    {
        return false;
    }

    std::array<bool, MAX_PARTY_MEMBERS + 1u> seenPartySlots{};
    std::unordered_set<SOCIAL_PLAYER_ID> seenSocialPlayers;
    bool foundSelf = false;
    std::size_t leaderCount = 0u;
    for (const PARTY_MEMBER_WIRE& member : roster.Members)
    {
        if (INVALID_SOCIAL_PLAYER_ID == member.iSocialPlayerId ||
            !Is_Supported_Playable_Character_Class(member.eCharacterClass) ||
            !Is_Valid_PlayerNickname(member.strNickname) ||
            0u == member.iMaximumHp ||
            member.iCurrentHp > member.iMaximumHp ||
            0u == member.iPartySlot ||
            member.iPartySlot > MAX_PARTY_MEMBERS ||
            seenPartySlots[member.iPartySlot] ||
            member.isLeader != (1u == member.iPartySlot) ||
            !seenSocialPlayers.insert(member.iSocialPlayerId).second ||
            !Is_Valid_PresentBinding(member))
        {
            return false;
        }
        seenPartySlots[member.iPartySlot] = true;
        foundSelf = foundSelf ||
            member.iSocialPlayerId == roster.iSelfSocialPlayerId;
        leaderCount += member.isLeader ? 1u : 0u;
    }
    if (!foundSelf || 1u != leaderCount)
        return false;

    for (std::size_t slot = 1u; slot <= roster.Members.size(); ++slot)
    {
        if (!seenPartySlots[slot])
            return false;
    }
    return true;
}

bool LostArk::Shared::Write_Message(
    CPacketWriter& writer,
    const C2S_PARTY_INVITE& message)
{
    if (0u == message.iRequestSequence ||
        !Is_Known_World_Id(message.eTargetWorldId) ||
        (WORLD_ID::BERN != message.eTargetWorldId &&
            WORLD_ID::VALTAN_ARENA != message.eTargetWorldId) ||
        INVALID_NET_ENTITY_ID == message.iTargetNetEntityId)
    {
        return false;
    }
    writer.Write_U32(message.iRequestSequence);
    writer.Write_U16(static_cast<std::uint16_t>(message.eTargetWorldId));
    writer.Write_U32(message.iTargetNetEntityId);
    return true;
}

bool LostArk::Shared::Read_Message(
    CPacketReader& reader,
    C2S_PARTY_INVITE& message)
{
    C2S_PARTY_INVITE decoded{};
    std::uint16_t rawWorld = 0u;
    if (!reader.Read_U32(decoded.iRequestSequence) ||
        !reader.Read_U16(rawWorld) ||
        !reader.Read_U32(decoded.iTargetNetEntityId))
    {
        return false;
    }
    decoded.eTargetWorldId = static_cast<WORLD_ID>(rawWorld);
    CPacketWriter validator;
    if (!Write_Message(validator, decoded))
        return false;
    message = decoded;
    return true;
}

bool LostArk::Shared::Write_Message(
    CPacketWriter& writer,
    const S2C_PARTY_COMMAND_RESULT& message)
{
    if (0u == message.iRequestSequence ||
        !Is_Known_PartyCommandResult(message.eResult) ||
        (PARTY_COMMAND_RESULT::ACCEPTED == message.eResult &&
            INVALID_SOCIAL_PLAYER_ID == message.iTargetSocialPlayerId))
    {
        return false;
    }
    writer.Write_U32(message.iRequestSequence);
    writer.Write_U8(static_cast<std::uint8_t>(message.eResult));
    writer.Write_U64(message.iTargetSocialPlayerId);
    return true;
}

bool LostArk::Shared::Read_Message(
    CPacketReader& reader,
    S2C_PARTY_COMMAND_RESULT& message)
{
    S2C_PARTY_COMMAND_RESULT decoded{};
    std::uint8_t rawResult = 0u;
    if (!reader.Read_U32(decoded.iRequestSequence) ||
        !reader.Read_U8(rawResult) ||
        !reader.Read_U64(decoded.iTargetSocialPlayerId))
    {
        return false;
    }
    decoded.eResult = static_cast<PARTY_COMMAND_RESULT>(rawResult);
    CPacketWriter validator;
    if (!Write_Message(validator, decoded))
        return false;
    message = decoded;
    return true;
}

bool LostArk::Shared::Write_Message(
    CPacketWriter& writer,
    const S2C_PARTY_ROSTER& message)
{
    if (!Is_Valid_PartyRoster(message))
        return false;

    writer.Write_U64(message.iRosterRevision);
    writer.Write_U64(message.iPartyId);
    writer.Write_U64(message.iSelfSocialPlayerId);
    writer.Write_U8(static_cast<std::uint8_t>(message.Members.size()));
    for (const PARTY_MEMBER_WIRE& member : message.Members)
    {
        writer.Write_U64(member.iSocialPlayerId);
        writer.Write_U16(static_cast<std::uint16_t>(member.eWorldId));
        writer.Write_U32(member.iNetEntityId);
        writer.Write_U8(static_cast<std::uint8_t>(member.eCharacterClass));
        if (!writer.Write_String(member.strNickname, MAX_NICKNAME_BYTES))
            return false;
        writer.Write_U32(member.iCurrentHp);
        writer.Write_U32(member.iMaximumHp);
        writer.Write_U8(member.iPartySlot);
        writer.Write_U8(member.isLeader ? 1u : 0u);
        writer.Write_U8(member.isPresent ? 1u : 0u);
    }
    return true;
}

bool LostArk::Shared::Read_Message(
    CPacketReader& reader,
    S2C_PARTY_ROSTER& message)
{
    S2C_PARTY_ROSTER decoded{};
    std::uint8_t count = 0u;
    if (!reader.Read_U64(decoded.iRosterRevision) ||
        !reader.Read_U64(decoded.iPartyId) ||
        !reader.Read_U64(decoded.iSelfSocialPlayerId) ||
        !reader.Read_U8(count) || count > MAX_PARTY_MEMBERS)
    {
        return false;
    }

    decoded.Members.reserve(count);
    for (std::uint8_t index = 0u; index < count; ++index)
    {
        PARTY_MEMBER_WIRE member{};
        std::uint16_t rawWorld = 0u;
        std::uint8_t rawClass = 0u;
        std::uint8_t leader = 0u;
        std::uint8_t present = 0u;
        if (!reader.Read_U64(member.iSocialPlayerId) ||
            !reader.Read_U16(rawWorld) ||
            !reader.Read_U32(member.iNetEntityId) ||
            !reader.Read_U8(rawClass) ||
            !reader.Read_String(member.strNickname, MAX_NICKNAME_BYTES) ||
            !reader.Read_U32(member.iCurrentHp) ||
            !reader.Read_U32(member.iMaximumHp) ||
            !reader.Read_U8(member.iPartySlot) ||
            !reader.Read_U8(leader) ||
            !reader.Read_U8(present) ||
            leader > 1u || present > 1u)
        {
            return false;
        }
        member.eWorldId = static_cast<WORLD_ID>(rawWorld);
        member.eCharacterClass = static_cast<CHARACTER_CLASS_ID>(rawClass);
        member.isLeader = 1u == leader;
        member.isPresent = 1u == present;
        decoded.Members.push_back(std::move(member));
    }
    if (!Is_Valid_PartyRoster(decoded))
        return false;
    message = std::move(decoded);
    return true;
}
```

### 2.5 `PacketType.h` 교체 block

`NETWORK_PROTOCOL_VERSION`을 다음으로 교체한다.

```cpp
inline constexpr std::uint16_t NETWORK_PROTOCOL_VERSION = 20;
```

`S2C_WORLD_SNAPSHOT` 뒤, chat packet 앞에 추가한다.

```cpp
        C2S_PARTY_INVITE,
        S2C_PARTY_COMMAND_RESULT,
        S2C_PARTY_ROSTER,
```

`Is_Known_Packet_Type()`에도 같은 세 case를 추가한다.

```cpp
        case PACKET_TYPE::C2S_PARTY_INVITE:
        case PACKET_TYPE::S2C_PARTY_COMMAND_RESULT:
        case PACKET_TYPE::S2C_PARTY_ROSTER:
```

## 3. Server 전체 public 계약

### 3.1 `Server/Public/RoomSocialPresence.h` 신규 전체 파일

```cpp
#pragma once

#include "ServerIds.h"
#include "Network/NetworkIds.h"
#include "Network/PacketType.h"

#include <cstdint>
#include <string>

namespace LostArk::Server
{
    struct ROOM_SOCIAL_PRESENCE final
    {
        SESSION_ID iSessionId = INVALID_SESSION_ID;
        LostArk::Shared::WORLD_ID eWorldId = LostArk::Shared::WORLD_ID::END;
        LostArk::Shared::NET_ENTITY_ID iNetEntityId =
            LostArk::Shared::INVALID_NET_ENTITY_ID;
        LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
            LostArk::Shared::CHARACTER_CLASS_ID::END;
        std::string strNickname;
        std::uint32_t iCurrentHp = 0u;
        std::uint32_t iMaximumHp = 0u;
    };
}
```

### 3.2 `Server/Public/ServerPartyService.h` 신규 전체 파일

```cpp
#pragma once

#include "RoomSocialPresence.h"
#include "ServerIds.h"
#include "Party/PartyContracts.h"

#include <cstdint>
#include <map>
#include <span>
#include <unordered_map>
#include <vector>

namespace LostArk::Server
{
    struct SERVER_PARTY_OUTBOUND final
    {
        SESSION_ID iTargetSessionId = INVALID_SESSION_ID;
        LostArk::Shared::PACKET_TYPE ePacketType =
            LostArk::Shared::PACKET_TYPE::INVALID;
        LostArk::Shared::S2C_PARTY_COMMAND_RESULT CommandResult;
        LostArk::Shared::S2C_PARTY_ROSTER Roster;
    };

    class CServerPartyService final
    {
    public:
        void Synchronize_Presence(
            std::span<const ROOM_SOCIAL_PRESENCE> presence,
            std::span<const SESSION_ID> connectedSessions,
            std::vector<SERVER_PARTY_OUTBOUND>& outOutbound);
        void Process_Invite(
            SESSION_ID inviterSessionId,
            const LostArk::Shared::C2S_PARTY_INVITE& invite,
            std::vector<SERVER_PARTY_OUTBOUND>& outOutbound);
        void On_SessionClosed(
            SESSION_ID sessionId,
            std::vector<SERVER_PARTY_OUTBOUND>& outOutbound);

        [[nodiscard]] std::size_t Get_PartyCount() const
        {
            return m_Parties.size();
        }

    private:
        struct SOCIAL_PLAYER_STATE final
        {
            SESSION_ID iSessionId = INVALID_SESSION_ID;
            LostArk::Shared::SOCIAL_PLAYER_ID iSocialPlayerId =
                LostArk::Shared::INVALID_SOCIAL_PLAYER_ID;
            ROOM_SOCIAL_PRESENCE Presence;
            std::uint32_t iLastInviteSequence = 0u;
            LostArk::Shared::PARTY_COMMAND_RESULT eCachedInviteResult =
                LostArk::Shared::PARTY_COMMAND_RESULT::END;
            LostArk::Shared::SOCIAL_PLAYER_ID iCachedTargetSocialPlayerId =
                LostArk::Shared::INVALID_SOCIAL_PLAYER_ID;
            bool isConnected = false;
            bool isPresent = false;
        };

        struct PARTY_STATE final
        {
            LostArk::Shared::PARTY_ID iPartyId =
                LostArk::Shared::INVALID_PARTY_ID;
            std::vector<LostArk::Shared::SOCIAL_PLAYER_ID> Members;
        };

        using PRESENCE_KEY = std::uint64_t;

        static PRESENCE_KEY Make_PresenceKey(
            LostArk::Shared::WORLD_ID worldId,
            LostArk::Shared::NET_ENTITY_ID entityId);
        SOCIAL_PLAYER_STATE& Ensure_Player(SESSION_ID sessionId);
        void Remove_DisconnectedPlayer(
            LostArk::Shared::SOCIAL_PLAYER_ID socialPlayerId,
            std::vector<SERVER_PARTY_OUTBOUND>& outOutbound);
        void Normalize_OrDissolveParty(
            LostArk::Shared::PARTY_ID partyId,
            std::vector<SERVER_PARTY_OUTBOUND>& outOutbound);
        void Publish_Roster(
            LostArk::Shared::PARTY_ID partyId,
            std::vector<SERVER_PARTY_OUTBOUND>& outOutbound);
        void Publish_EmptyRoster(
            LostArk::Shared::SOCIAL_PLAYER_ID memberId,
            std::vector<SERVER_PARTY_OUTBOUND>& outOutbound);
        void Publish_Result(
            SESSION_ID targetSessionId,
            std::uint32_t requestSequence,
            LostArk::Shared::PARTY_COMMAND_RESULT result,
            LostArk::Shared::SOCIAL_PLAYER_ID targetSocialPlayerId,
            std::vector<SERVER_PARTY_OUTBOUND>& outOutbound) const;
        void Cache_AndPublishResult(
            SOCIAL_PLAYER_STATE& player,
            std::uint32_t requestSequence,
            LostArk::Shared::PARTY_COMMAND_RESULT result,
            LostArk::Shared::SOCIAL_PLAYER_ID targetSocialPlayerId,
            std::vector<SERVER_PARTY_OUTBOUND>& outOutbound);

        std::uint64_t m_iNextSocialPlayerId = 1u;
        std::uint64_t m_iNextPartyId = 1u;
        std::uint64_t m_iNextRosterRevision = 1u;
        std::unordered_map<SESSION_ID,
            LostArk::Shared::SOCIAL_PLAYER_ID> m_SocialBySession;
        std::unordered_map<LostArk::Shared::SOCIAL_PLAYER_ID,
            SOCIAL_PLAYER_STATE> m_Players;
        std::unordered_map<PRESENCE_KEY,
            LostArk::Shared::SOCIAL_PLAYER_ID> m_SocialByPresence;
        std::map<LostArk::Shared::PARTY_ID, PARTY_STATE> m_Parties;
        std::unordered_map<LostArk::Shared::SOCIAL_PLAYER_ID,
            LostArk::Shared::PARTY_ID> m_PartyByMember;
    };
}
```

### 3.3 `GameRoom` 정확한 추가 block

`Server/Private/ServerPartyService.cpp` 신규 전체 파일은 다음과 같다.

```cpp
#include "ServerPartyService.h"

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace
{
    using namespace LostArk::Shared;
    using LostArk::Server::ROOM_SOCIAL_PRESENCE;

    bool Is_SocialWorld(const WORLD_ID worldId)
    {
        return WORLD_ID::BERN == worldId ||
            WORLD_ID::VALTAN_ARENA == worldId;
    }

    bool Is_NewerSequence(
        const std::uint32_t candidate,
        const std::uint32_t previous)
    {
        return 0u != candidate &&
            static_cast<std::int32_t>(candidate - previous) > 0;
    }

    bool Presence_Equals(
        const ROOM_SOCIAL_PRESENCE& left,
        const ROOM_SOCIAL_PRESENCE& right)
    {
        return left.iSessionId == right.iSessionId &&
            left.eWorldId == right.eWorldId &&
            left.iNetEntityId == right.iNetEntityId &&
            left.eCharacterClass == right.eCharacterClass &&
            left.strNickname == right.strNickname &&
            left.iCurrentHp == right.iCurrentHp &&
            left.iMaximumHp == right.iMaximumHp;
    }
}

LostArk::Server::CServerPartyService::PRESENCE_KEY
LostArk::Server::CServerPartyService::Make_PresenceKey(
    const LostArk::Shared::WORLD_ID worldId,
    const LostArk::Shared::NET_ENTITY_ID entityId)
{
    return (static_cast<std::uint64_t>(worldId) << 32u) |
        static_cast<std::uint64_t>(entityId);
}

LostArk::Server::CServerPartyService::SOCIAL_PLAYER_STATE&
LostArk::Server::CServerPartyService::Ensure_Player(
    const SESSION_ID sessionId)
{
    const auto existing = m_SocialBySession.find(sessionId);
    if (existing != m_SocialBySession.end())
        return m_Players.at(existing->second);

    LostArk::Shared::SOCIAL_PLAYER_ID socialPlayerId =
        m_iNextSocialPlayerId++;
    if (LostArk::Shared::INVALID_SOCIAL_PLAYER_ID == socialPlayerId)
        socialPlayerId = m_iNextSocialPlayerId++;

    SOCIAL_PLAYER_STATE player{};
    player.iSessionId = sessionId;
    player.iSocialPlayerId = socialPlayerId;
    player.isConnected = true;
    const auto [playerIter, insertedPlayer] =
        m_Players.emplace(socialPlayerId, std::move(player));
    if (!insertedPlayer)
        return playerIter->second;
    m_SocialBySession.emplace(sessionId, socialPlayerId);
    return playerIter->second;
}

void LostArk::Server::CServerPartyService::Publish_Result(
    const SESSION_ID targetSessionId,
    const std::uint32_t requestSequence,
    const LostArk::Shared::PARTY_COMMAND_RESULT result,
    const LostArk::Shared::SOCIAL_PLAYER_ID targetSocialPlayerId,
    std::vector<SERVER_PARTY_OUTBOUND>& outOutbound) const
{
    SERVER_PARTY_OUTBOUND outbound{};
    outbound.iTargetSessionId = targetSessionId;
    outbound.ePacketType =
        LostArk::Shared::PACKET_TYPE::S2C_PARTY_COMMAND_RESULT;
    outbound.CommandResult.iRequestSequence = requestSequence;
    outbound.CommandResult.eResult = result;
    outbound.CommandResult.iTargetSocialPlayerId = targetSocialPlayerId;
    outOutbound.push_back(std::move(outbound));
}

void LostArk::Server::CServerPartyService::Cache_AndPublishResult(
    SOCIAL_PLAYER_STATE& player,
    const std::uint32_t requestSequence,
    const LostArk::Shared::PARTY_COMMAND_RESULT result,
    const LostArk::Shared::SOCIAL_PLAYER_ID targetSocialPlayerId,
    std::vector<SERVER_PARTY_OUTBOUND>& outOutbound)
{
    player.iLastInviteSequence = requestSequence;
    player.eCachedInviteResult = result;
    player.iCachedTargetSocialPlayerId = targetSocialPlayerId;
    Publish_Result(
        player.iSessionId,
        requestSequence,
        result,
        targetSocialPlayerId,
        outOutbound);
}

void LostArk::Server::CServerPartyService::Publish_EmptyRoster(
    const LostArk::Shared::SOCIAL_PLAYER_ID memberId,
    std::vector<SERVER_PARTY_OUTBOUND>& outOutbound)
{
    const auto playerIter = m_Players.find(memberId);
    if (playerIter == m_Players.end() || !playerIter->second.isConnected)
        return;

    SERVER_PARTY_OUTBOUND outbound{};
    outbound.iTargetSessionId = playerIter->second.iSessionId;
    outbound.ePacketType =
        LostArk::Shared::PACKET_TYPE::S2C_PARTY_ROSTER;
    outbound.Roster.iRosterRevision = m_iNextRosterRevision++;
    outbound.Roster.iPartyId = LostArk::Shared::INVALID_PARTY_ID;
    outbound.Roster.iSelfSocialPlayerId = memberId;
    outOutbound.push_back(std::move(outbound));
}

void LostArk::Server::CServerPartyService::Publish_Roster(
    const LostArk::Shared::PARTY_ID partyId,
    std::vector<SERVER_PARTY_OUTBOUND>& outOutbound)
{
    const auto partyIter = m_Parties.find(partyId);
    if (partyIter == m_Parties.end() ||
        partyIter->second.Members.size() < 2u)
    {
        return;
    }

    const std::uint64_t revision = m_iNextRosterRevision++;
    std::vector<LostArk::Shared::PARTY_MEMBER_WIRE> members;
    members.reserve(partyIter->second.Members.size());
    for (std::size_t index = 0u;
        index < partyIter->second.Members.size(); ++index)
    {
        const LostArk::Shared::SOCIAL_PLAYER_ID memberId =
            partyIter->second.Members[index];
        const auto playerIter = m_Players.find(memberId);
        if (playerIter == m_Players.end())
            return;

        const SOCIAL_PLAYER_STATE& player = playerIter->second;
        LostArk::Shared::PARTY_MEMBER_WIRE wire{};
        wire.iSocialPlayerId = memberId;
        wire.eCharacterClass = player.Presence.eCharacterClass;
        wire.strNickname = player.Presence.strNickname;
        wire.iCurrentHp = player.Presence.iCurrentHp;
        wire.iMaximumHp = player.Presence.iMaximumHp;
        wire.iPartySlot = static_cast<std::uint8_t>(index + 1u);
        wire.isLeader = 0u == index;
        wire.isPresent = player.isPresent;
        if (player.isPresent)
        {
            wire.eWorldId = player.Presence.eWorldId;
            wire.iNetEntityId = player.Presence.iNetEntityId;
        }
        members.push_back(std::move(wire));
    }

    for (const LostArk::Shared::SOCIAL_PLAYER_ID memberId :
        partyIter->second.Members)
    {
        const SOCIAL_PLAYER_STATE& player = m_Players.at(memberId);
        if (!player.isConnected)
            continue;
        SERVER_PARTY_OUTBOUND outbound{};
        outbound.iTargetSessionId = player.iSessionId;
        outbound.ePacketType =
            LostArk::Shared::PACKET_TYPE::S2C_PARTY_ROSTER;
        outbound.Roster.iRosterRevision = revision;
        outbound.Roster.iPartyId = partyId;
        outbound.Roster.iSelfSocialPlayerId = memberId;
        outbound.Roster.Members = members;
        outOutbound.push_back(std::move(outbound));
    }
}

void LostArk::Server::CServerPartyService::Normalize_OrDissolveParty(
    const LostArk::Shared::PARTY_ID partyId,
    std::vector<SERVER_PARTY_OUTBOUND>& outOutbound)
{
    const auto partyIter = m_Parties.find(partyId);
    if (partyIter == m_Parties.end())
        return;

    PARTY_STATE& party = partyIter->second;
    std::vector<LostArk::Shared::SOCIAL_PLAYER_ID> removedMembers;
    party.Members.erase(
        std::remove_if(
            party.Members.begin(),
            party.Members.end(),
            [this, &removedMembers](
                const LostArk::Shared::SOCIAL_PLAYER_ID memberId)
            {
                const auto playerIter = m_Players.find(memberId);
                const bool remove = playerIter == m_Players.end() ||
                    !playerIter->second.isConnected;
                if (remove)
                    removedMembers.push_back(memberId);
                return remove;
            }),
        party.Members.end());
    for (const LostArk::Shared::SOCIAL_PLAYER_ID memberId : removedMembers)
        m_PartyByMember.erase(memberId);

    if (party.Members.size() >= 2u)
    {
        Publish_Roster(partyId, outOutbound);
        return;
    }

    for (const LostArk::Shared::SOCIAL_PLAYER_ID memberId : party.Members)
    {
        m_PartyByMember.erase(memberId);
        Publish_EmptyRoster(memberId, outOutbound);
    }
    m_Parties.erase(partyIter);
}

void LostArk::Server::CServerPartyService::Remove_DisconnectedPlayer(
    const LostArk::Shared::SOCIAL_PLAYER_ID socialPlayerId,
    std::vector<SERVER_PARTY_OUTBOUND>& outOutbound)
{
    const auto playerIter = m_Players.find(socialPlayerId);
    if (playerIter == m_Players.end())
        return;

    const SESSION_ID sessionId = playerIter->second.iSessionId;
    const auto membershipIter = m_PartyByMember.find(socialPlayerId);
    LostArk::Shared::PARTY_ID affectedParty =
        LostArk::Shared::INVALID_PARTY_ID;
    if (membershipIter != m_PartyByMember.end())
    {
        affectedParty = membershipIter->second;
        const auto partyIter = m_Parties.find(affectedParty);
        if (partyIter != m_Parties.end())
        {
            auto& members = partyIter->second.Members;
            members.erase(
                std::remove(members.begin(), members.end(), socialPlayerId),
                members.end());
        }
        m_PartyByMember.erase(membershipIter);
    }

    m_SocialBySession.erase(sessionId);
    m_Players.erase(playerIter);
    if (LostArk::Shared::INVALID_PARTY_ID != affectedParty)
        Normalize_OrDissolveParty(affectedParty, outOutbound);
}

void LostArk::Server::CServerPartyService::On_SessionClosed(
    const SESSION_ID sessionId,
    std::vector<SERVER_PARTY_OUTBOUND>& outOutbound)
{
    const auto socialIter = m_SocialBySession.find(sessionId);
    if (socialIter == m_SocialBySession.end())
        return;
    Remove_DisconnectedPlayer(socialIter->second, outOutbound);
}

void LostArk::Server::CServerPartyService::Synchronize_Presence(
    const std::span<const ROOM_SOCIAL_PRESENCE> presence,
    const std::span<const SESSION_ID> connectedSessions,
    std::vector<SERVER_PARTY_OUTBOUND>& outOutbound)
{
    std::unordered_set<SESSION_ID> connected(
        connectedSessions.begin(), connectedSessions.end());
    std::unordered_map<SESSION_ID, const ROOM_SOCIAL_PRESENCE*>
        stagedPresence;
    stagedPresence.reserve(presence.size());
    for (const ROOM_SOCIAL_PRESENCE& current : presence)
    {
        if (connected.contains(current.iSessionId))
            stagedPresence.emplace(current.iSessionId, &current);
    }

    // Social identity is session-scoped, not world-entity-scoped. Create it
    // for every committed connection before processing presence or commands.
    for (const SESSION_ID sessionId : connectedSessions)
    {
        SOCIAL_PLAYER_STATE& player = Ensure_Player(sessionId);
        player.isConnected = true;
    }

    std::unordered_set<LostArk::Shared::PARTY_ID> changedParties;
    for (auto& [socialId, player] : m_Players)
    {
        player.isConnected = connected.contains(player.iSessionId);
        if (!player.isConnected)
        {
            player.isPresent = false;
            continue;
        }
        const auto staged = stagedPresence.find(player.iSessionId);
        const bool nextPresent = staged != stagedPresence.end();
        const bool changed = player.isPresent != nextPresent ||
            (nextPresent &&
                !Presence_Equals(player.Presence, *staged->second));
        if (nextPresent)
            player.Presence = *staged->second;
        player.isPresent = nextPresent;
        if (changed)
        {
            const auto membership = m_PartyByMember.find(socialId);
            if (membership != m_PartyByMember.end())
                changedParties.insert(membership->second);
        }
    }

    for (const auto& [sessionId, current] : stagedPresence)
    {
        if (m_SocialBySession.contains(sessionId))
            continue;
        SOCIAL_PLAYER_STATE& player = Ensure_Player(sessionId);
        player.Presence = *current;
        player.isConnected = true;
        player.isPresent = true;
    }

    m_SocialByPresence.clear();
    for (const auto& [socialId, player] : m_Players)
    {
        if (player.isConnected && player.isPresent &&
            Is_SocialWorld(player.Presence.eWorldId))
        {
            m_SocialByPresence.insert_or_assign(
                Make_PresenceKey(
                    player.Presence.eWorldId,
                    player.Presence.iNetEntityId),
                socialId);
        }
    }

    std::vector<LostArk::Shared::SOCIAL_PLAYER_ID> disconnected;
    for (const auto& [socialId, player] : m_Players)
    {
        if (!player.isConnected)
            disconnected.push_back(socialId);
    }
    for (const LostArk::Shared::SOCIAL_PLAYER_ID socialId : disconnected)
    {
        LostArk::Shared::PARTY_ID affectedParty =
            LostArk::Shared::INVALID_PARTY_ID;
        const auto membership = m_PartyByMember.find(socialId);
        if (membership != m_PartyByMember.end())
            affectedParty = membership->second;
        Remove_DisconnectedPlayer(socialId, outOutbound);
        if (LostArk::Shared::INVALID_PARTY_ID != affectedParty)
            changedParties.erase(affectedParty);
    }

    for (const LostArk::Shared::PARTY_ID partyId : changedParties)
    {
        if (m_Parties.contains(partyId))
            Publish_Roster(partyId, outOutbound);
    }
}

void LostArk::Server::CServerPartyService::Process_Invite(
    const SESSION_ID inviterSessionId,
    const LostArk::Shared::C2S_PARTY_INVITE& invite,
    std::vector<SERVER_PARTY_OUTBOUND>& outOutbound)
{
    using namespace LostArk::Shared;

    const auto inviterIdIter = m_SocialBySession.find(inviterSessionId);
    if (inviterIdIter == m_SocialBySession.end())
    {
        Publish_Result(inviterSessionId, invite.iRequestSequence,
            PARTY_COMMAND_RESULT::REJECTED_WRONG_WORLD,
            INVALID_SOCIAL_PLAYER_ID, outOutbound);
        return;
    }

    SOCIAL_PLAYER_STATE& inviter = m_Players.at(inviterIdIter->second);
    if (invite.iRequestSequence == inviter.iLastInviteSequence &&
        Is_Known_PartyCommandResult(inviter.eCachedInviteResult))
    {
        Publish_Result(
            inviterSessionId,
            invite.iRequestSequence,
            inviter.eCachedInviteResult,
            inviter.iCachedTargetSocialPlayerId,
            outOutbound);
        return;
    }
    if (!Is_NewerSequence(
        invite.iRequestSequence, inviter.iLastInviteSequence))
    {
        Publish_Result(inviterSessionId, invite.iRequestSequence,
            PARTY_COMMAND_RESULT::REJECTED_STALE_REQUEST,
            INVALID_SOCIAL_PLAYER_ID, outOutbound);
        return;
    }

    if (!inviter.isPresent || !Is_SocialWorld(inviter.Presence.eWorldId) ||
        inviter.Presence.eWorldId != invite.eTargetWorldId)
    {
        Cache_AndPublishResult(inviter, invite.iRequestSequence,
            PARTY_COMMAND_RESULT::REJECTED_WRONG_WORLD,
            INVALID_SOCIAL_PLAYER_ID, outOutbound);
        return;
    }

    const auto targetPresence = m_SocialByPresence.find(
        Make_PresenceKey(
            invite.eTargetWorldId, invite.iTargetNetEntityId));
    if (targetPresence == m_SocialByPresence.end())
    {
        Cache_AndPublishResult(inviter, invite.iRequestSequence,
            PARTY_COMMAND_RESULT::REJECTED_TARGET_NOT_FOUND,
            INVALID_SOCIAL_PLAYER_ID, outOutbound);
        return;
    }

    const SOCIAL_PLAYER_ID inviterId = inviter.iSocialPlayerId;
    const SOCIAL_PLAYER_ID targetId = targetPresence->second;
    if (targetId == inviterId)
    {
        Cache_AndPublishResult(inviter, invite.iRequestSequence,
            PARTY_COMMAND_RESULT::REJECTED_SELF,
            targetId, outOutbound);
        return;
    }

    const auto inviterMembership = m_PartyByMember.find(inviterId);
    const auto targetMembership = m_PartyByMember.find(targetId);
    if (inviterMembership == m_PartyByMember.end())
    {
        if (targetMembership != m_PartyByMember.end())
        {
            Cache_AndPublishResult(inviter, invite.iRequestSequence,
                PARTY_COMMAND_RESULT::REJECTED_TARGET_IN_OTHER_PARTY,
                targetId, outOutbound);
            return;
        }

        PARTY_ID partyId = m_iNextPartyId++;
        if (INVALID_PARTY_ID == partyId)
            partyId = m_iNextPartyId++;
        PARTY_STATE party{};
        party.iPartyId = partyId;
        party.Members = { inviterId, targetId };
        m_Parties.emplace(partyId, std::move(party));
        m_PartyByMember.emplace(inviterId, partyId);
        m_PartyByMember.emplace(targetId, partyId);
        Cache_AndPublishResult(inviter, invite.iRequestSequence,
            PARTY_COMMAND_RESULT::ACCEPTED, targetId, outOutbound);
        Publish_Roster(partyId, outOutbound);
        return;
    }

    PARTY_STATE& party = m_Parties.at(inviterMembership->second);
    if (party.Members.empty() || party.Members.front() != inviterId)
    {
        Cache_AndPublishResult(inviter, invite.iRequestSequence,
            PARTY_COMMAND_RESULT::REJECTED_NOT_LEADER,
            targetId, outOutbound);
        return;
    }
    if (targetMembership != m_PartyByMember.end())
    {
        Cache_AndPublishResult(inviter, invite.iRequestSequence,
            targetMembership->second == party.iPartyId ?
                PARTY_COMMAND_RESULT::REJECTED_ALREADY_MEMBER :
                PARTY_COMMAND_RESULT::REJECTED_TARGET_IN_OTHER_PARTY,
            targetId, outOutbound);
        return;
    }
    if (party.Members.size() >= MAX_PARTY_MEMBERS)
    {
        Cache_AndPublishResult(inviter, invite.iRequestSequence,
            PARTY_COMMAND_RESULT::REJECTED_PARTY_FULL,
            targetId, outOutbound);
        return;
    }

    party.Members.push_back(targetId);
    m_PartyByMember.emplace(targetId, party.iPartyId);
    Cache_AndPublishResult(inviter, invite.iRequestSequence,
        PARTY_COMMAND_RESULT::ACCEPTED, targetId, outOutbound);
    Publish_Roster(party.iPartyId, outOutbound);
}
```

`Synchronize_Presence()`는 full staged map과 이전 committed state를 비교한다. world/entity/class/nickname/HP/presence가 바뀐 party만 새 full-state revision을 broadcast하며, 변화 없는 다음 tick은 outbound 0이어야 한다.

`GameRoom.h` include에 추가한다.

```cpp
#include "RoomSocialPresence.h"
```

public의 `Get_PerformanceMetrics()` 다음에 추가한다.

```cpp
        void Collect_SocialPresence(
            std::vector<ROOM_SOCIAL_PRESENCE>& outPresence) const;
```

`GameRoom.cpp`에 다음 전체 함수를 추가한다.

```cpp
void LostArk::Server::CGameRoom::Collect_SocialPresence(
    std::vector<ROOM_SOCIAL_PRESENCE>& outPresence) const
{
    for (const auto& [playerId, player] : m_Players)
    {
        (void)playerId;
        ROOM_SOCIAL_PRESENCE presence{};
        presence.iSessionId = player.iSessionId;
        presence.eWorldId = m_eWorldId;
        presence.iNetEntityId = player.iNetEntityId;
        presence.eCharacterClass = player.eCharacterClass;
        presence.strNickname = player.strNickName;
        presence.iCurrentHp = player.iCurrentHp;
        presence.iMaximumHp = player.iMaximumHp;
        outPresence.push_back(std::move(presence));
    }
}
```

이 함수는 room thread에서만 호출한다. mutex를 추가하거나 외부에서 `m_Players`를 수정하지 않는다.

### 3.4 `ServerApp.h` 추가 계약

include에 추가한다.

```cpp
#include "ServerPartyService.h"
#include "Party/PartyMessages.h"
```

private helper에 추가한다.

```cpp
        bool Enqueue_SocialInvite(
            SESSION_ID sessionId,
            const LostArk::Shared::C2S_PARTY_INVITE& invite);
        void Synchronize_AndProcessParty(
            const std::vector<std::shared_ptr<CGameRoom>>& simulations);
        void Dispatch_PartyOutbound(
            std::span<const SERVER_PARTY_OUTBOUND> outbound);
```

private state에 추가한다.

```cpp
        struct SERVER_SOCIAL_COMMAND final
        {
            SESSION_ID iSessionId = INVALID_SESSION_ID;
            LostArk::Shared::C2S_PARTY_INVITE Invite;
        };

        static constexpr std::size_t MAX_SOCIAL_COMMAND_COUNT = 256u;
        static constexpr std::size_t
            MAX_SOCIAL_COMMANDS_DRAINED_PER_TICK = 64u;
        std::mutex m_SocialCommandMutex;
        std::deque<SERVER_SOCIAL_COMMAND> m_SocialCommands;
        CServerPartyService m_PartyService;
```

### 3.5 `ServerApp.cpp::On_SessionFrame` party branch

`C2S_ENTER_WORLD` 처리 뒤, gameplay command decode 전에 추가한다.

```cpp
    if (frame.ePacketType == PACKET_TYPE::C2S_PARTY_INVITE)
    {
        C2S_PARTY_INVITE invite{};
        if (!Read_Message(reader, invite) ||
            0u != reader.Get_RemainingSize() ||
            !Enqueue_SocialInvite(sessionId, invite))
        {
            Request_SessionClose(sessionId);
        }
        return;
    }
```

queue 함수 전체:

```cpp
bool LostArk::Server::CServerApp::Enqueue_SocialInvite(
    const SESSION_ID sessionId,
    const LostArk::Shared::C2S_PARTY_INVITE& invite)
{
    if (INVALID_SESSION_ID == sessionId)
        return false;
    std::scoped_lock lock{ m_SocialCommandMutex };
    if (m_SocialCommands.size() >= MAX_SOCIAL_COMMAND_COUNT)
        return false;
    SERVER_SOCIAL_COMMAND command{};
    command.iSessionId = sessionId;
    command.Invite = invite;
    m_SocialCommands.push_back(std::move(command));
    return true;
}
```

`Tick_GameplaySimulations()`은 simulation vector를 mutex 아래 복사한 뒤 mutex를 풀고 다음의 두 pass를 사용한다. room별 `Tick -> Transfer`를 섞지 않는다.

```cpp
    for (const std::shared_ptr<CGameRoom>& simulation : simulations)
    {
        if (nullptr != simulation)
            simulation->Tick(fixedDeltaSeconds);
    }
    for (const std::shared_ptr<CGameRoom>& simulation : simulations)
    {
        Handle_WorldTransfers(simulation);
    }
    Synchronize_AndProcessParty(simulations);
    Retire_QuiescentCharacterSelectArenas();
```

party sync 전체 함수:

```cpp
void LostArk::Server::CServerApp::Synchronize_AndProcessParty(
    const std::vector<std::shared_ptr<CGameRoom>>& simulations)
{
    std::vector<ROOM_SOCIAL_PRESENCE> presence;
    for (const std::shared_ptr<CGameRoom>& simulation : simulations)
    {
        if (nullptr != simulation)
            simulation->Collect_SocialPresence(presence);
    }

    std::vector<SESSION_ID> connected;
    {
        std::scoped_lock lock{ m_SessionsMutex };
        connected.reserve(m_Sessions.size());
        for (const auto& [sessionId, session] : m_Sessions)
        {
            if (nullptr != session && session->Is_Open())
                connected.push_back(sessionId);
        }
    }

    std::deque<SERVER_SOCIAL_COMMAND> commands;
    {
        std::scoped_lock lock{ m_SocialCommandMutex };
        const std::size_t drainCount = (std::min)(
            MAX_SOCIAL_COMMANDS_DRAINED_PER_TICK,
            m_SocialCommands.size());
        for (std::size_t index = 0u; index < drainCount; ++index)
        {
            commands.push_back(std::move(m_SocialCommands.front()));
            m_SocialCommands.pop_front();
        }
    }

    std::vector<SERVER_PARTY_OUTBOUND> outbound;
    m_PartyService.Synchronize_Presence(
        presence, connected, outbound);
    for (const SERVER_SOCIAL_COMMAND& command : commands)
    {
        m_PartyService.Process_Invite(
            command.iSessionId, command.Invite, outbound);
    }
    Dispatch_PartyOutbound(outbound);
}
```

send 전체 함수:

```cpp
void LostArk::Server::CServerApp::Dispatch_PartyOutbound(
    const std::span<const SERVER_PARTY_OUTBOUND> outbound)
{
    using namespace LostArk::Shared;
    for (const SERVER_PARTY_OUTBOUND& item : outbound)
    {
        std::shared_ptr<CClientSession> session;
        {
            std::scoped_lock lock{ m_SessionsMutex };
            const auto iter = m_Sessions.find(item.iTargetSessionId);
            if (iter != m_Sessions.end())
                session = iter->second;
        }
        if (nullptr == session)
            continue;

        CPacketWriter writer;
        bool encoded = false;
        if (PACKET_TYPE::S2C_PARTY_COMMAND_RESULT == item.ePacketType)
            encoded = Write_Message(writer, item.CommandResult);
        else if (PACKET_TYPE::S2C_PARTY_ROSTER == item.ePacketType)
            encoded = Write_Message(writer, item.Roster);

        if (!encoded || !session->Send_Frame(
            item.ePacketType, writer.Get_Buffer()))
        {
            Request_SessionClose(item.iTargetSessionId);
        }
    }
}
```

`Room_Loop()`은 각 iteration에서 `Reap_ClosedSessions()`를 `Tick_GameplaySimulations()`보다 먼저 호출한다. shutdown final drain도 같은 순서를 쓴다.

`Reap_ClosedSessions()`는 다음 전체 흐름으로 교체한다. closed ID는 dedupe하고 party removal을 room thread에서 먼저 commit한 뒤 session을 stop한다.

```cpp
void LostArk::Server::CServerApp::Reap_ClosedSessions()
{
    std::deque<SESSION_ID> closedIds;
    {
        std::scoped_lock lock{ m_ClosedSessionMutex };
        closedIds.swap(m_ClosedSessionIds);
    }
    std::sort(closedIds.begin(), closedIds.end());
    closedIds.erase(
        std::unique(closedIds.begin(), closedIds.end()),
        closedIds.end());

    std::vector<SERVER_PARTY_OUTBOUND> outbound;
    std::vector<std::shared_ptr<CClientSession>> stopped;
    for (const SESSION_ID sessionId : closedIds)
    {
        m_PartyService.On_SessionClosed(sessionId, outbound);
        std::shared_ptr<CClientSession> session;
        {
            std::scoped_lock lock{ m_SessionsMutex };
            const auto iter = m_Sessions.find(sessionId);
            if (iter == m_Sessions.end())
                continue;
            session = std::move(iter->second);
            m_Sessions.erase(iter);
        }
        if (nullptr != session)
            stopped.push_back(std::move(session));
    }
    Dispatch_PartyOutbound(outbound);
    for (const std::shared_ptr<CClientSession>& session : stopped)
        session->Stop();
}
```

## 4. Client 신규 public 계약

### 4.1 `Client/Public/PartyCommandSink.h` 신규 전체 파일

```cpp
#pragma once

#include "Client_Defines.h"
#include "Network/NetworkIds.h"
#include "Network/PacketType.h"

NS_BEGIN(Client)

class IPartyCommandSink
{
public:
    virtual ~IPartyCommandSink() = default;
    virtual bool_t Request_Invite(
        LostArk::Shared::WORLD_ID targetWorldId,
        LostArk::Shared::NET_ENTITY_ID targetEntityId) = 0;
};

NS_END
```

### 4.2 `Client/Public/NetworkPartyCommandSink.h` 신규 전체 파일

```cpp
#pragma once

#include "PartyCommandSink.h"

NS_BEGIN(Client)

class CNetworkPartyCommandSink final : public IPartyCommandSink
{
public:
    bool_t Request_Invite(
        LostArk::Shared::WORLD_ID targetWorldId,
        LostArk::Shared::NET_ENTITY_ID targetEntityId) override;
};

NS_END
```

### 4.3 `Client/Private/NetworkPartyCommandSink.cpp` 신규 전체 파일

```cpp
#include "NetworkPartyCommandSink.h"

#include "NetworkManager.h"

bool_t Client::CNetworkPartyCommandSink::Request_Invite(
    const LostArk::Shared::WORLD_ID targetWorldId,
    const LostArk::Shared::NET_ENTITY_ID targetEntityId)
{
    return CNetworkManager::Get().Send_PartyInvite(
        targetWorldId, targetEntityId);
}
```

### 4.4 `Client/Public/PartyViewModel.h` 신규 전체 파일

```cpp
#pragma once

#include "Client_Defines.h"
#include "Party/PartyContracts.h"

#include <string>
#include <vector>

NS_BEGIN(Client)

struct PARTY_VIEW_MEMBER final
{
    LostArk::Shared::SOCIAL_PLAYER_ID iSocialPlayerId =
        LostArk::Shared::INVALID_SOCIAL_PLAYER_ID;
    LostArk::Shared::CHARACTER_CLASS_ID eCharacterClass =
        LostArk::Shared::CHARACTER_CLASS_ID::END;
    string strNickname;
    std::uint32_t iCurrentHp = 0u;
    std::uint32_t iMaximumHp = 0u;
    std::uint8_t iPartySlot = 0u;
    bool_t isLeader = false;
    bool_t isPresent = false;
};

class CPartyViewModel final
{
public:
    void Update_FromNetwork();
    void Reset();
    [[nodiscard]] const std::vector<PARTY_VIEW_MEMBER>& Get_Members() const
    {
        return m_Members;
    }
    [[nodiscard]] const string& Get_Status() const { return m_strStatus; }

private:
    std::uint64_t m_iObservedConnectionGeneration = 0u;
    std::uint64_t m_iRosterRevision = 0u;
    LostArk::Shared::PARTY_ID m_iPartyId =
        LostArk::Shared::INVALID_PARTY_ID;
    std::vector<PARTY_VIEW_MEMBER> m_Members;
    string m_strStatus;
};

NS_END
```

### 4.5 `Client/Private/PartyViewModel.cpp` 신규 전체 파일

```cpp
#include "PartyViewModel.h"

#include "NetworkManager.h"

#include <algorithm>

namespace
{
    const char_t* Describe_Result(
        const LostArk::Shared::PARTY_COMMAND_RESULT result)
    {
        using LostArk::Shared::PARTY_COMMAND_RESULT;
        switch (result)
        {
        case PARTY_COMMAND_RESULT::ACCEPTED: return "Party invite accepted.";
        case PARTY_COMMAND_RESULT::REJECTED_WRONG_WORLD: return "Party invite is available only in Bern or Valtan.";
        case PARTY_COMMAND_RESULT::REJECTED_SELF: return "You cannot invite yourself.";
        case PARTY_COMMAND_RESULT::REJECTED_TARGET_NOT_FOUND: return "The selected player is no longer here.";
        case PARTY_COMMAND_RESULT::REJECTED_NOT_LEADER: return "Only the party leader can invite.";
        case PARTY_COMMAND_RESULT::REJECTED_ALREADY_MEMBER: return "That player is already in your party.";
        case PARTY_COMMAND_RESULT::REJECTED_TARGET_IN_OTHER_PARTY: return "That player is already in another party.";
        case PARTY_COMMAND_RESULT::REJECTED_PARTY_FULL: return "The party already has four members.";
        case PARTY_COMMAND_RESULT::REJECTED_STALE_REQUEST: return "The invite request was stale.";
        default: return "Unknown party result.";
        }
    }
}

void Client::CPartyViewModel::Update_FromNetwork()
{
    CNetworkManager& network = CNetworkManager::Get();
    const std::uint64_t connectionGeneration =
        network.Get_SocialConnectionGeneration();
    if (connectionGeneration != m_iObservedConnectionGeneration)
    {
        Reset();
        m_iObservedConnectionGeneration = connectionGeneration;
    }
    LostArk::Shared::S2C_PARTY_COMMAND_RESULT result{};
    while (network.Try_Consume_PartyCommandResult(result))
        m_strStatus = Describe_Result(result.eResult);

    LostArk::Shared::S2C_PARTY_ROSTER roster{};
    while (network.Try_Consume_PartyRoster(roster))
    {
        if (roster.iRosterRevision <= m_iRosterRevision)
            continue;

        std::vector<PARTY_VIEW_MEMBER> staged;
        staged.reserve(roster.Members.size());
        for (const LostArk::Shared::PARTY_MEMBER_WIRE& source : roster.Members)
        {
            PARTY_VIEW_MEMBER target{};
            target.iSocialPlayerId = source.iSocialPlayerId;
            target.eCharacterClass = source.eCharacterClass;
            target.strNickname = source.strNickname;
            target.iCurrentHp = source.iCurrentHp;
            target.iMaximumHp = source.iMaximumHp;
            target.iPartySlot = source.iPartySlot;
            target.isLeader = source.isLeader;
            target.isPresent = source.isPresent;
            staged.push_back(std::move(target));
        }
        std::sort(staged.begin(), staged.end(),
            [](const PARTY_VIEW_MEMBER& left, const PARTY_VIEW_MEMBER& right)
            {
                return left.iPartySlot < right.iPartySlot;
            });

        m_iRosterRevision = roster.iRosterRevision;
        m_iPartyId = roster.iPartyId;
        m_Members = std::move(staged);
        if (m_Members.empty())
            m_strStatus.clear();
    }
}

void Client::CPartyViewModel::Reset()
{
    m_iRosterRevision = 0u;
    m_iPartyId = LostArk::Shared::INVALID_PARTY_ID;
    m_Members.clear();
    m_strStatus.clear();
}
```

## 5. Client 기존 파일 교체 계약

### 5.1 `NetworkManager`

header include에 추가한다.

```cpp
#include "Party/PartyMessages.h"
```

public send/consume에 추가한다.

```cpp
    bool Send_PartyInvite(
        LostArk::Shared::WORLD_ID targetWorldId,
        LostArk::Shared::NET_ENTITY_ID targetEntityId);
    bool Try_Consume_PartyCommandResult(
        LostArk::Shared::S2C_PARTY_COMMAND_RESULT& message);
    bool Try_Consume_PartyRoster(
        LostArk::Shared::S2C_PARTY_ROSTER& message);
    [[nodiscard]] std::uint64_t Get_SocialConnectionGeneration() const
    {
        return m_iSocialConnectionGeneration;
    }
```

`Reset_WorldInboundState()` 선언 바로 뒤 private helper에 다음 선언을 추가한다. 선언 없이 CPP만 추가하지 않는다.

```cpp
    void Reset_WorldInboundState();
    void Reset_SocialInboundState();
```

private state에 추가한다.

```cpp
    std::deque<LostArk::Shared::S2C_PARTY_COMMAND_RESULT>
        m_PartyCommandResults;
    std::deque<LostArk::Shared::S2C_PARTY_ROSTER> m_PartyRosters;
    std::uint32_t m_iNextPartyRequestSequence = 1u;
    std::uint64_t m_iSocialConnectionGeneration = 0u;
```

`Send_PartyInvite()` 전체:

```cpp
bool CNetworkManager::Send_PartyInvite(
    const LostArk::Shared::WORLD_ID targetWorldId,
    const LostArk::Shared::NET_ENTITY_ID targetEntityId)
{
    using namespace LostArk::Shared;
    C2S_PARTY_INVITE message{};
    message.iRequestSequence = m_iNextPartyRequestSequence;
    message.eTargetWorldId = targetWorldId;
    message.iTargetNetEntityId = targetEntityId;
    CPacketWriter writer;
    if (!Write_Message(writer, message))
        return false;
    std::vector<std::uint8_t> frame;
    const bool sent = Build_Packet_Frame(
        PACKET_TYPE::C2S_PARTY_INVITE,
        writer.Get_Buffer(), frame) && Send_All(frame);
    if (!sent)
        return false;
    ++m_iNextPartyRequestSequence;
    if (0u == m_iNextPartyRequestSequence)
        m_iNextPartyRequestSequence = 1u;
    return true;
}
```

`Handle_Frame()` switch에 추가한다.

```cpp
    case PACKET_TYPE::S2C_PARTY_COMMAND_RESULT:
    {
        S2C_PARTY_COMMAND_RESULT result{};
        if (!Read_Message(reader, result) || 0u != reader.Get_RemainingSize())
        {
            Fail_Protocol(WSAEINVAL);
            return;
        }
        // MainApp consumes only the latest durable command status.
        m_PartyCommandResults.clear();
        m_PartyCommandResults.push_back(std::move(result));
        break;
    }
    case PACKET_TYPE::S2C_PARTY_ROSTER:
    {
        S2C_PARTY_ROSTER roster{};
        if (!Read_Message(reader, roster) || 0u != reader.Get_RemainingSize())
        {
            Fail_Protocol(WSAEINVAL);
            return;
        }
        // Roster is a full-state snapshot with a monotonic revision.
        m_PartyRosters.clear();
        m_PartyRosters.push_back(std::move(roster));
        break;
    }
```

consume 함수:

```cpp
bool CNetworkManager::Try_Consume_PartyCommandResult(
    LostArk::Shared::S2C_PARTY_COMMAND_RESULT& message)
{
    if (m_PartyCommandResults.empty())
        return false;
    message = std::move(m_PartyCommandResults.front());
    m_PartyCommandResults.pop_front();
    return true;
}

bool CNetworkManager::Try_Consume_PartyRoster(
    LostArk::Shared::S2C_PARTY_ROSTER& message)
{
    if (m_PartyRosters.empty())
        return false;
    message = std::move(m_PartyRosters.front());
    m_PartyRosters.pop_front();
    return true;
}
```

party queues는 `Reset_WorldInboundState()`에서 지우지 않는다. 다음 private helper를 추가한다.

```cpp
void CNetworkManager::Reset_SocialInboundState()
{
    m_PartyCommandResults.clear();
    m_PartyRosters.clear();
    m_iNextPartyRequestSequence = 1u;
    ++m_iSocialConnectionGeneration;
    if (0u == m_iSocialConnectionGeneration)
        ++m_iSocialConnectionGeneration;
}
```

호출 위치는 다음 세 군데로 고정한다.

```cpp
// Connect_To_Server(): non-blocking socket을 blocking mode로 되돌린 뒤,
// receive thread를 시작하기 직전의 새 socket commit.
m_StreamParser.Reset();
Reset_WorldInboundState();
Reset_SocialInboundState();

// Fail_Protocol(): inbound frame clear와 parser reset 뒤.
m_StreamParser.Reset();
Reset_WorldInboundState();
Reset_SocialInboundState();

// Close_ServerConnection(): receive thread join과 inbound frame clear 뒤.
m_StreamParser.Reset();
Reset_WorldInboundState();
Reset_SocialInboundState();
```

`Send_EnterWorld()`와 `Handle_Frame(S2C_ENTER_ACCEPTED)`의 기존 `Reset_WorldInboundState()` 옆에는 social reset을 추가하지 않는다. 같은 TCP session의 Bern -> Valtan transfer는 connection generation, result status, party roster를 유지한다. `Reset_SocialInboundState()`가 연속 호출되어 generation이 둘 이상 증가해도 stale state는 되살아나지 않지만, harness는 성공 connect/explicit close/protocol failure 각각에서 값이 strictly increase하는지만 검증하고 정확히 `+1`에 결합하지 않는다.

### 5.2 `PartyWindowView` seed 제거와 binding

`PartyWindowView.h`에서 nested seed DTO와 `m_Members`를 제거하고 include를 추가한다. 파일 자체, texture cache, 왼쪽 geometry는 기존 UI 작업물을 재사용한다.

```cpp
#include "PartyViewModel.h"
```

public Render를 교체한다.

```cpp
    void Render(const CPartyViewModel& viewModel);
```

constructor의 네 `m_Members.push_back` 전체를 삭제한다. CPP `Render()` signature와 loop source를 다음으로 교체한다.

```cpp
void Client::CPartyWindowView::Render(
    const CPartyViewModel& viewModel)
{
    const std::vector<PARTY_VIEW_MEMBER>& members =
        viewModel.Get_Members();
    const string& status = viewModel.Get_Status();
    if (members.empty() && status.empty())
        return;
```

기존 reference constants, viewport scaling과 `vOrigin` 계산은 보존한다. `vOrigin` 계산 직후, Party title art를 그리기 전에 다음 status-only branch를 추가한다. 초대 실패는 보이지만 member가 없는 상태에서 Party title/HP panel을 위조하지 않는다.

```cpp
    if (members.empty())
    {
        pDrawList->AddText(
            vOrigin,
            IM_COL32(255, 225, 160, 255),
            status.c_str());
        return;
    }
```

loop는 `members`를 사용하고 member mapping을 다음으로 교체한다.

```cpp
    for (const PARTY_VIEW_MEMBER& Member : members)
    {
        const std::size_t index = static_cast<std::size_t>(
            Member.iPartySlot - 1u);
        const f32_t rawHpRatio = 0u == Member.iMaximumHp ? 0.f :
            static_cast<f32_t>(Member.iCurrentHp) /
            static_cast<f32_t>(Member.iMaximumHp);
        const f32_t fHpRatio =
            (std::min)(1.f, (std::max)(0.f, rawHpRatio));
```

현재 HP fill block 안의 `Member.fHpRatio` 기반 내부 선언은 삭제하고 위에서 계산한 `fHpRatio`를 사용하도록 block 전체를 교체한다. 이를 남기면 신규 `PARTY_VIEW_MEMBER`에 없는 필드를 참조해 컴파일되지 않는다.

```cpp
        if (nullptr != pHpFillSRV)
        {
            const ImVec2 vFillMin(
                vBarMin.x + HP_FILL_INSET * fScaleX,
                vBarMin.y + HP_FILL_INSET * fScaleY);
            const ImVec2 vFillMax(
                vFillMin.x +
                    (HP_BAR_WIDTH - HP_FILL_INSET * 2.f) *
                    fScaleX * fHpRatio,
                vBarMax.y - HP_FILL_INSET * fScaleY);
            const ImVec2 vFillUvMax(fHpRatio, 1.f);
            pDrawList->AddImage(
                pHpFillSRV,
                vFillMin,
                vFillMax,
                ImVec2(0.f, 0.f),
                vFillUvMax);
        }
```

class symbol은 `CHARACTER_CLASS_ID` switch로 Resources-relative path를 resolve한다. unknown/누락 art는 empty path로 두고 다른 row는 유지한다. vector index를 party 번호로 사용하지 않는다.

```cpp
namespace
{
    const char_t* Resolve_PartyClassSymbol(
        const LostArk::Shared::CHARACTER_CLASS_ID characterClass)
    {
        using LostArk::Shared::CHARACTER_CLASS_ID;
        switch (characterClass)
        {
        case CHARACTER_CLASS_ID::LANCE_MASTER:
            return "UI/ClassSelect/LanceMaster/IdentitySymbol.png";
        case CHARACTER_CLASS_ID::ARTIST:
            return "UI/ClassSelect/Artist/IdentitySymbol.png";
        case CHARACTER_CLASS_ID::WARLORD:
            return "UI/ClassSelect/Warlord/IdentitySymbol.png";
        case CHARACTER_CLASS_ID::GUNSLINGER:
        case CHARACTER_CLASS_ID::SLAYER:
        case CHARACTER_CLASS_ID::DIMENSIONMASTER:
            return ""; // 2026-08-15 물리 Resources에 IdentitySymbol.png 없음
        default:
            return "";
        }
    }
}
```

기존 symbol load는 `Member.strClassSymbolPath` 대신 `Resolve_PartyClassSymbol(Member.eCharacterClass)`의 non-empty 결과를 사용한다. 번호 art도 loop index가 아니라 `Member.iPartySlot - 1u`로 고른다.

기존 member row loop가 끝난 뒤 status를 foreground draw list에 그린다. member가 없는 failure는 앞의 status-only branch가 표시하고, authoritative empty/disband roster는 `CPartyViewModel`이 이전 status를 지운다. 따라서 party 해산 뒤 빈 Party title panel이 남지 않는다.

```cpp
    if (!status.empty())
    {
        const f32_t statusY = fRowY + 2.f * fScaleY;
        pDrawList->AddText(
            ImVec2(vOrigin.x, statusY),
            IM_COL32(255, 225, 160, 255),
            status.c_str());
    }
```

### 5.3 `MainApp`

`MainApp.h` include/forward declaration과 state에 추가한다.

```cpp
#include "PartyViewModel.h"

    CPartyViewModel m_PartyViewModel;
```

`CNetworkManager::Get().Update();` 바로 뒤에 추가한다.

```cpp
    m_PartyViewModel.Update_FromNetwork();
```

Party render 호출은 교체한다.

```cpp
                m_pPartyWindowView->Render(m_PartyViewModel);
```

Effect service update/render block과 Debug tool switch는 건드리지 않는다.

기존 `m_pChatWindowView` 생성과 `MainApp.cpp`의 Enter/Escape 처리, Bern/Valtan render gate는 그대로 둔다. Party 변경을 적용하면서 Chat block을 이동, 삭제하거나 Party protocol에 결합하지 않는다. `Free()`의 기존 global input reset 직후에는 G03이 추가한 per-button latch를 명시적으로 정리한다.

```cpp
    CGameInstance::Get().SetInputBlocked(false, false);
    CGameInstance::Get().SetMouseButtonBlocked(DIM::LB, false);
    CGameInstance::Get().SetMouseButtonBlocked(DIM::RB, false);
```

## 6. Hover/RMB interaction 계약

### 6.1 `Client/Public/WorldPlayerSocialInteractionView.h` 신규 전체 파일

```cpp
#pragma once

#include "ClientReplication.h"
#include "PartyCommandSink.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CWorldPlayerSocialInteractionView final
{
public:
    struct CONSUMED_MOUSE_BUTTONS final
    {
        bool_t left = false;
        bool_t right = false;
    };

    void Set_CommandSink(
        const shared_ptr<IPartyCommandSink>& commandSink);
    CONSUMED_MOUSE_BUTTONS Update(
        LostArk::Shared::WORLD_ID worldId,
        const std::vector<REPLICATED_PLAYER_VIEW>& players);
    void Render();
    void Reset();

private:
    struct TARGET final
    {
        LostArk::Shared::WORLD_ID eWorldId =
            LostArk::Shared::WORLD_ID::END;
        LostArk::Shared::NET_ENTITY_ID iNetEntityId =
            LostArk::Shared::INVALID_NET_ENTITY_ID;
        string strNickname;
    };

    bool_t Try_FindHoveredTarget(
        LostArk::Shared::WORLD_ID worldId,
        const std::vector<REPLICATED_PLAYER_VIEW>& players,
        TARGET& outTarget) const;

    weak_ptr<IPartyCommandSink> m_pCommandSink;
    TARGET m_Target;
    bool_t m_isPopupOpen = false;
    bool_t m_requestPopupOpen = false;
    bool_t m_wasRightMouseDown = false;
    bool_t m_blockRightMouseUntilRelease = false;
    bool_t m_blockLeftMouseUntilRelease = false;
};

NS_END
```

### 6.2 `Client/Private/WorldPlayerSocialInteractionView.cpp` 신규 전체 파일

현재 G02의 public projection API, `REPLICATED_PLAYER_VIEW`, `CCharacter::Get_Transform()`, `CGameInstance` View/Proj/viewport와 현재 입력 차단 API에 맞춘 전체 파일은 다음과 같다. DirectInput의 blocked state를 edge 검출에 다시 쓰지 않고 `GetAsyncKeyState`를 raw release latch에만 사용한다. 새 target 획득은 `IsMouseInputBlocked()`가 false일 때만 허용해 Chat/Skill/Developer UI의 pointer capture를 우회하지 않는다.

```cpp
#include "WorldPlayerSocialInteractionView.h"

#include "Character.h"
#include "GameInstance.h"
#include "Transform.h"
#include "WorldPlayerNameplateView.h"
#include "imgui.h"

#include <Windows.h>

#include <cmath>
#include <limits>
#include <utility>

namespace
{
    constexpr f32_t NAMEPLATE_HEAD_OFFSET = 2.2f;
    constexpr f32_t HIT_HALF_WIDTH = 80.f;
    constexpr f32_t HIT_HALF_HEIGHT = 14.f;

    bool_t Is_SocialWorld(const LostArk::Shared::WORLD_ID worldId)
    {
        return LostArk::Shared::WORLD_ID::BERN == worldId ||
            LostArk::Shared::WORLD_ID::VALTAN_ARENA == worldId;
    }

    bool_t Is_RawMouseDown(const int virtualKey)
    {
        return nullptr != g_hWnd && GetForegroundWindow() == g_hWnd &&
            0 != (::GetAsyncKeyState(virtualKey) & 0x8000);
    }
}

void Client::CWorldPlayerSocialInteractionView::Set_CommandSink(
    const shared_ptr<IPartyCommandSink>& commandSink)
{
    m_pCommandSink = commandSink;
}

bool_t Client::CWorldPlayerSocialInteractionView::Try_FindHoveredTarget(
    const LostArk::Shared::WORLD_ID worldId,
    const std::vector<REPLICATED_PLAYER_VIEW>& players,
    TARGET& outTarget) const
{
    outTarget = {};
    CGameInstance& gameInstance = CGameInstance::Get();
    if (!Is_SocialWorld(worldId) || nullptr == g_hWnd ||
        gameInstance.IsMouseInputBlocked())
        return false;

    POINT cursor{};
    if (!GetCursorPos(&cursor) || !ScreenToClient(g_hWnd, &cursor))
        return false;

    const float4x4_t* const viewMatrix =
        gameInstance.Get_Transform(D3DTS::VIEW);
    const float4x4_t* const projectionMatrix =
        gameInstance.Get_Transform(D3DTS::PROJ);
    if (nullptr == viewMatrix || nullptr == projectionMatrix)
        return false;

    const float2_t viewportSize = gameInstance.Get_ViewportSize();
    f32_t bestViewDepth = (std::numeric_limits<f32_t>::max)();
    LostArk::Shared::NET_ENTITY_ID bestEntityId =
        LostArk::Shared::INVALID_NET_ENTITY_ID;

    for (const REPLICATED_PLAYER_VIEW& player : players)
    {
        if (player.isLocal ||
            LostArk::Shared::INVALID_NET_ENTITY_ID == player.iNetEntityId ||
            player.strNickname.empty())
        {
            continue;
        }

        const shared_ptr<CCharacter> character = player.pCharacter.lock();
        if (nullptr == character)
            continue;
        const shared_ptr<CTransform> transform = character->Get_Transform();
        if (nullptr == transform)
            continue;

        float3_t headPosition{};
        XMStoreFloat3(
            &headPosition,
            transform->Get_State(STATE::POSITION));
        headPosition.y += NAMEPLATE_HEAD_OFFSET;

        float2_t screenPosition{};
        if (!CWorldPlayerNameplateView::Try_ProjectWorldPosition(
            headPosition,
            *viewMatrix,
            *projectionMatrix,
            viewportSize,
            screenPosition))
        {
            continue;
        }

        const f32_t cursorX = static_cast<f32_t>(cursor.x);
        const f32_t cursorY = static_cast<f32_t>(cursor.y);
        if (std::abs(cursorX - screenPosition.x) > HIT_HALF_WIDTH ||
            std::abs(cursorY - screenPosition.y) > HIT_HALF_HEIGHT)
        {
            continue;
        }

        const vector_t viewPosition = XMVector3TransformCoord(
            XMLoadFloat3(&headPosition),
            XMLoadFloat4x4(viewMatrix));
        const f32_t viewDepth = XMVectorGetZ(viewPosition);
        if (!std::isfinite(viewDepth) || viewDepth <= 0.f)
            continue;
        if (viewDepth > bestViewDepth ||
            (viewDepth == bestViewDepth &&
                bestEntityId <= player.iNetEntityId))
        {
            continue;
        }

        bestViewDepth = viewDepth;
        bestEntityId = player.iNetEntityId;
        outTarget.eWorldId = worldId;
        outTarget.iNetEntityId = player.iNetEntityId;
        outTarget.strNickname = player.strNickname;
    }
    return LostArk::Shared::INVALID_NET_ENTITY_ID != bestEntityId;
}

Client::CWorldPlayerSocialInteractionView::CONSUMED_MOUSE_BUTTONS
Client::CWorldPlayerSocialInteractionView::Update(
    const LostArk::Shared::WORLD_ID worldId,
    const std::vector<REPLICATED_PLAYER_VIEW>& players)
{
    const bool_t rightMouseDown = Is_RawMouseDown(VK_RBUTTON);
    const bool_t leftMouseDown = Is_RawMouseDown(VK_LBUTTON);
    if (!rightMouseDown)
        m_blockRightMouseUntilRelease = false;
    if (!leftMouseDown)
        m_blockLeftMouseUntilRelease = false;

    TARGET hovered{};
    const bool_t hasHoveredTarget =
        Try_FindHoveredTarget(worldId, players, hovered);
    const bool_t rightRising =
        rightMouseDown && !m_wasRightMouseDown;
    if (rightRising && hasHoveredTarget)
    {
        m_Target = std::move(hovered);
        m_requestPopupOpen = true;
        m_isPopupOpen = true;
        m_blockRightMouseUntilRelease = true;
    }

    // Popup 위 첫 LMB도 같은 update에서 gameplay basic attack보다 먼저 막는다.
    if (m_isPopupOpen && leftMouseDown)
        m_blockLeftMouseUntilRelease = true;

    m_wasRightMouseDown = rightMouseDown;
    CONSUMED_MOUSE_BUTTONS consumed{};
    consumed.left = m_blockLeftMouseUntilRelease ||
        (m_isPopupOpen && leftMouseDown);
    consumed.right = m_blockRightMouseUntilRelease ||
        m_isPopupOpen || (hasHoveredTarget && rightMouseDown);
    return consumed;
}

void Client::CWorldPlayerSocialInteractionView::Render()
{
    constexpr const char_t* POPUP_ID = "Player Party Command";
    if (m_requestPopupOpen)
    {
        ImGui::OpenPopup(POPUP_ID);
        m_requestPopupOpen = false;
    }

    if (ImGui::BeginPopup(POPUP_ID))
    {
        const bool_t targetValid =
            Is_SocialWorld(m_Target.eWorldId) &&
            LostArk::Shared::INVALID_NET_ENTITY_ID !=
                m_Target.iNetEntityId &&
            !m_Target.strNickname.empty();
        if (!targetValid)
        {
            m_isPopupOpen = false;
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return;
        }

        m_isPopupOpen = true;
        ImGui::TextUnformatted(m_Target.strNickname.c_str());
        if (ImGui::Button("Invite"))
        {
            m_blockLeftMouseUntilRelease = true;
            const shared_ptr<IPartyCommandSink> sink =
                m_pCommandSink.lock();
            if (nullptr != sink && sink->Request_Invite(
                m_Target.eWorldId, m_Target.iNetEntityId))
            {
                m_isPopupOpen = false;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            m_blockLeftMouseUntilRelease = true;
            m_isPopupOpen = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    else if (!ImGui::IsPopupOpen(POPUP_ID))
    {
        m_isPopupOpen = false;
    }
}

void Client::CWorldPlayerSocialInteractionView::Reset()
{
    m_Target = {};
    m_isPopupOpen = false;
    m_requestPopupOpen = false;
    m_wasRightMouseDown = false;
    m_blockRightMouseUntilRelease = false;
    m_blockLeftMouseUntilRelease = false;
    m_pCommandSink.reset();
}
```

겹친 이름표는 view-space depth가 작은 remote player를 고르고 같은 depth에서는 작은 `NetEntityId`를 고른다. 따라서 registry/vector 순서가 바뀌어도 target binding이 흔들리지 않는다. global mouse capture 중에는 새 popup을 열지 않지만 기존 popup의 release latch는 계속 raw state로 정리한다. Level reset 뒤 ImGui global popup ID가 한 frame 남아도 invalid target guard가 즉시 닫는다. Invite send가 실패하면 popup을 유지하고 packet result를 위조하지 않는다. Cancel은 packet을 만들지 않는다.

### 6.3 Bern/Valtan Level integration

MainApp의 MapTool LB block과 Level의 Party interaction LB block을 OR-composition하기 위해 Engine에 read-only getter를 추가한다. `Engine/Public/Input_Device.h`의 `SetMouseButtonBlocked` 바로 뒤:

```cpp
    [[nodiscard]] bool_t IsMouseButtonBlocked(DIM eMouse) const
    {
        const uint32_t index = ETOUI(eMouse);
        return index < ETOUI(DIM::END) &&
            m_MouseButtonBlocked[index];
    }
```

`Engine/Public/GameInstance.h`의 `SetMouseButtonBlocked` 바로 뒤:

```cpp
    bool_t IsMouseButtonBlocked(DIM eMouse) const;
```

`Engine/Private/GameInstance.cpp`의 `SetMouseButtonBlocked` 정의 바로 뒤:

```cpp
bool_t CGameInstance::IsMouseButtonBlocked(const DIM eMouse) const
{
    return nullptr != m_pInput_Device &&
        m_pInput_Device->IsMouseButtonBlocked(eMouse);
}
```

이 getter는 raw input을 노출하지 않고 이미 확정된 per-button block bit만 읽는다. MainApp는 매 frame MapTool source로 LB를 먼저 설정하고, Level은 그 값을 지우지 않은 채 Party source를 OR한다.

두 Level header에 추가한다.

```cpp
#include "NetworkPartyCommandSink.h"
#include "WorldPlayerSocialInteractionView.h"

    shared_ptr<IPartyCommandSink> m_pPartyCommandSink;
    CWorldPlayerSocialInteractionView m_PlayerSocialInteractionView;
```

Initialize 성공 직전에 추가한다.

```cpp
    m_pPartyCommandSink = make_shared<CNetworkPartyCommandSink>();
    m_PlayerSocialInteractionView.Set_CommandSink(m_pPartyCommandSink);
```

Update에서 replication을 적용하고 local character를 얻은 뒤, `m_PlayerController.Update()` 전에 추가한다.

```cpp
    m_Replication.Collect_PlayerViews(m_NameplatePlayers);
    const CWorldPlayerSocialInteractionView::CONSUMED_MOUSE_BUTTONS consumed =
        m_PlayerSocialInteractionView.Update(
            LostArk::Shared::WORLD_ID::BERN,
            m_NameplatePlayers);
    CGameInstance& gameInstance = CGameInstance::Get();
    const bool_t existingLeftBlock =
        gameInstance.IsMouseButtonBlocked(DIM::LB);
    gameInstance.SetMouseButtonBlocked(
        DIM::LB, existingLeftBlock || consumed.left);
    gameInstance.SetMouseButtonBlocked(
        DIM::RB, consumed.right);
```

Valtan은 world argument만 `WORLD_ID::VALTAN_ARENA`로 사용한다. destructor와 world-transfer early return 직전에 다음 cleanup을 호출한다.

```cpp
    m_PlayerSocialInteractionView.Reset();
    CGameInstance::Get().SetMouseButtonBlocked(DIM::RB, false);
```

LB는 destructor/transfer에서 blind `false`로 지우지 않는다. 해당 frame의 controller update는 이미 끝났고, 다음 frame 시작에 MainApp가 MapTool source를 다시 설정한다. MainApp shutdown의 기존 최종 입력 정리에는 LB/RB `false`를 함께 둔다.

Render에서 G02 nameplate 뒤에 추가한다. `CPartyViewModel`은 MainApp 소유이므로 Level popup과 결합하지 않는다. result status는 Party HUD owner만 표시한다.

```cpp
    m_PlayerSocialInteractionView.Render();
```

## 7. Project/filter 등록

아래 anchor는 2026-08-15 실제 물리 프로젝트 파일에서 확인했다. 정렬을 전면 재작성하지 말고 명시한 기존 item 바로 옆에만 추가한다.

### Shared

`Shared/Default/Shared.vcxproj`의 `..\Public\Network\PacketWriter.h` 뒤와 `..\Private\Network\PacketWriter.cpp` 뒤:

```xml
    <ClInclude Include="..\Public\Party\PartyContracts.h" />
    <ClInclude Include="..\Public\Party\PartyMessages.h" />
    <ClCompile Include="..\Private\Party\PartyMessages.cpp" />
```

현재 `Shared.vcxproj.filters`에는 Party filter가 없다. 기존 `Public\Network`/`Private\Network`와 같은 naming으로 다음 두 filter만 추가하고 각각의 item을 연결한다.

```xml
    <Filter Include="Public\Party">
      <UniqueIdentifier>{4DD80E1F-ABF2-46E8-A7AF-7866F6C2BC8B}</UniqueIdentifier>
    </Filter>
    <Filter Include="Private\Party">
      <UniqueIdentifier>{B518FFCA-17C4-4F93-9E6C-0BDAF5568092}</UniqueIdentifier>
    </Filter>
    <ClInclude Include="..\Public\Party\PartyContracts.h">
      <Filter>Public\Party</Filter>
    </ClInclude>
    <ClInclude Include="..\Public\Party\PartyMessages.h">
      <Filter>Public\Party</Filter>
    </ClInclude>
    <ClCompile Include="..\Private\Party\PartyMessages.cpp">
      <Filter>Private\Party</Filter>
    </ClCompile>
```

### Server

`Server/Default/Server.vcxproj`에서 header는 `..\Public\ServerApp.h` 뒤, CPP는 `..\Private\ServerApp.cpp` 뒤에 추가한다.

```xml
    <ClInclude Include="..\Public\RoomSocialPresence.h" />
    <ClInclude Include="..\Public\ServerPartyService.h" />
    <ClCompile Include="..\Private\ServerPartyService.cpp" />
```

`Server.vcxproj.filters`는 새 filter를 만들지 않는다. `ServerApp.h`와 같은 `Public`, `ServerApp.cpp`와 같은 `Private`에 다음 item을 둔다.

```xml
    <ClInclude Include="..\Public\RoomSocialPresence.h"><Filter>Public</Filter></ClInclude>
    <ClInclude Include="..\Public\ServerPartyService.h"><Filter>Public</Filter></ClInclude>
    <ClCompile Include="..\Private\ServerPartyService.cpp"><Filter>Private</Filter></ClCompile>
```

### Client

`Client/Default/Client.vcxproj`의 header item은 `..\Public\PartyWindowView.h`/`..\Public\WorldPlayerNameplateView.h` 인접 위치, CPP item은 대응 기존 CPP 인접 위치에 추가한다.

`ChatWindowView.h/.cpp`와 `PartyWindowView.h/.cpp`는 현재 project와 filters에 이미 각각 정확히 한 번 등록돼 있다. 아래에는 G03 신규 파일만 추가하며 기존 UI item을 다시 쓰지 않는다.

```xml
    <ClInclude Include="..\Public\PartyCommandSink.h" />
    <ClInclude Include="..\Public\NetworkPartyCommandSink.h" />
    <ClInclude Include="..\Public\PartyViewModel.h" />
    <ClInclude Include="..\Public\WorldPlayerSocialInteractionView.h" />
    <ClCompile Include="..\Private\NetworkPartyCommandSink.cpp" />
    <ClCompile Include="..\Private\PartyViewModel.cpp" />
    <ClCompile Include="..\Private\WorldPlayerSocialInteractionView.cpp" />
```

`Client.vcxproj.filters`의 실재 filter를 그대로 쓴다.

```xml
    <ClInclude Include="..\Public\PartyCommandSink.h"><Filter>04. Network</Filter></ClInclude>
    <ClInclude Include="..\Public\NetworkPartyCommandSink.h"><Filter>04. Network</Filter></ClInclude>
    <ClInclude Include="..\Public\PartyViewModel.h"><Filter>02.GameObjects\04. UI</Filter></ClInclude>
    <ClInclude Include="..\Public\WorldPlayerSocialInteractionView.h"><Filter>02.GameObjects\04. UI</Filter></ClInclude>
    <ClCompile Include="..\Private\NetworkPartyCommandSink.cpp"><Filter>04. Network</Filter></ClCompile>
    <ClCompile Include="..\Private\PartyViewModel.cpp"><Filter>02.GameObjects\04. UI</Filter></ClCompile>
    <ClCompile Include="..\Private\WorldPlayerSocialInteractionView.cpp"><Filter>02.GameObjects\04. UI</Filter></ClCompile>
```

### 기존 harness 프로젝트

`Tools/ClientFrontendHarness/Default/ClientFrontendHarness.vcxproj`에는 이미 `Client/Private/NetworkManager.cpp`, `Client/Private/WorldPlayerNameplateView.cpp`, Shared `ProjectReference`가 있다. 같은 `ClCompile` item group에서 nameplate CPP 바로 뒤에 다음 하나만 추가하고 filters의 기존 `Client`에 연결한다.

```xml
    <ClCompile Include="..\..\..\Client\Private\PartyViewModel.cpp" />
    <ClCompile Include="..\..\..\Client\Private\PartyViewModel.cpp">
      <Filter>Client</Filter>
    </ClCompile>
```

`NetworkProtocolHarness`와 `CharacterSelectIsolationHarness`는 이미 Shared project를 참조하며 각각 기존 단일 harness CPP가 있다. 그 CPP만 수정하고 project/filter item은 추가하지 않는다. 어떤 경우에도 G03 전용 `.vcxproj`, `.filters`, `.sln` entry를 만들지 않는다.

Engine header 변경은 새 project item을 만들지 않는다. 구현 검증에서 Engine Debug/Release 뒤 `UpdateLib.bat Debug/Release`를 실행하고 Client까지 다시 link한다.

## 8. Harness 반영 코드

### 8.1 NetworkProtocolHarness

`Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp` include block에 codec 선언을 직접 추가한다.

```cpp
#include "Party/PartyMessages.h"
```

추가 테스트는 다음을 모두 한 함수에서 검증한다.

```cpp
void Test_PartyProtocol(TEST_RUNNER& runner)
{
    using namespace LostArk::Shared;

    C2S_PARTY_INVITE invite{};
    invite.iRequestSequence = 7u;
    invite.eTargetWorldId = WORLD_ID::BERN;
    invite.iTargetNetEntityId = 101u;
    CPacketWriter inviteWriter;
    const bool builtInvite = Write_Message(inviteWriter, invite);
    CPacketReader inviteReader{ inviteWriter.Get_Buffer() };
    C2S_PARTY_INVITE decodedInvite{};
    runner.Require(
        builtInvite &&
        Read_Message(inviteReader, decodedInvite) &&
        decodedInvite.iRequestSequence == invite.iRequestSequence &&
        decodedInvite.eTargetWorldId == invite.eTargetWorldId &&
        decodedInvite.iTargetNetEntityId == invite.iTargetNetEntityId,
        "Party invite round-trips exact target binding");

    S2C_PARTY_ROSTER roster{};
    roster.iRosterRevision = 11u;
    roster.iPartyId = 3u;
    roster.iSelfSocialPlayerId = 1u;
    PARTY_MEMBER_WIRE leader{};
    leader.iSocialPlayerId = 1u;
    leader.eWorldId = WORLD_ID::BERN;
    leader.iNetEntityId = 100u;
    leader.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
    leader.strNickname = "Leader";
    leader.iCurrentHp = 900u;
    leader.iMaximumHp = 1000u;
    leader.iPartySlot = 1u;
    leader.isLeader = true;
    leader.isPresent = true;
    PARTY_MEMBER_WIRE member = leader;
    member.iSocialPlayerId = 2u;
    member.iNetEntityId = 101u;
    member.strNickname = "Member";
    member.iPartySlot = 2u;
    member.isLeader = false;
    roster.Members = { leader, member };

    CPacketWriter rosterWriter;
    const bool builtRoster = Write_Message(rosterWriter, roster);
    CPacketReader rosterReader{ rosterWriter.Get_Buffer() };
    S2C_PARTY_ROSTER decodedRoster{};
    runner.Require(
        builtRoster &&
        Read_Message(rosterReader, decodedRoster) &&
        Is_Valid_PartyRoster(decodedRoster),
        "Party roster round-trips authoritative identity order and HP");

    S2C_PARTY_ROSTER invalid = roster;
    invalid.Members[1].iPartySlot = 1u;
    runner.Require(
        !Is_Valid_PartyRoster(invalid),
        "Party roster rejects duplicate party slot");
    invalid = roster;
    invalid.Members.resize(1u);
    runner.Require(
        !Is_Valid_PartyRoster(invalid),
        "Party roster rejects fake one-member party");
}
```

실제 적용에서는 temporary `CPacketReader`를 non-const lvalue로 만들어 호출한다. malformed decode destination 보존, Character Select target 거부, zero sequence, unknown result, 5-member roster도 같은 함수에 추가한다.

### 8.2 Server contract tests

`Server/Private/ServerGameplayContractTests.cpp`에 `ServerPartyService.h`를 include하고, 기존 anonymous namespace에 다음 형태의 pure service test를 추가한 뒤 `Run_ServerGameplayContractTests()`에서 호출한다. 별도 executable을 만들지 않는다.

```cpp
void Test_ServerPartyService(TESTS& tests)
{
    using namespace LostArk::Server;
    using namespace LostArk::Shared;
    CServerPartyService service;
    std::vector<SERVER_PARTY_OUTBOUND> outbound;
    std::vector<SESSION_ID> connected{ 101u, 202u };
    std::vector<ROOM_SOCIAL_PRESENCE> presence{
        { 101u, WORLD_ID::BERN, 1001u,
          CHARACTER_CLASS_ID::ARTIST, "Party-A", 900u, 1000u },
        { 202u, WORLD_ID::BERN, 2002u,
          CHARACTER_CLASS_ID::WARLORD, "Party-B", 1500u, 2000u }
    };

    service.Synchronize_Presence(presence, connected, outbound);
    tests.Require(outbound.empty(),
        "Party presence alone emits no fake roster");

    C2S_PARTY_INVITE invite{};
    invite.iRequestSequence = 1u;
    invite.eTargetWorldId = WORLD_ID::BERN;
    invite.iTargetNetEntityId = 2002u;
    service.Process_Invite(101u, invite, outbound);

    const auto result = std::find_if(outbound.begin(), outbound.end(),
        [](const SERVER_PARTY_OUTBOUND& item)
        {
            return 101u == item.iTargetSessionId &&
                PACKET_TYPE::S2C_PARTY_COMMAND_RESULT == item.ePacketType;
        });
    std::vector<const S2C_PARTY_ROSTER*> rosters;
    for (const SERVER_PARTY_OUTBOUND& item : outbound)
    {
        if (PACKET_TYPE::S2C_PARTY_ROSTER == item.ePacketType)
            rosters.push_back(&item.Roster);
    }
    tests.Require(result != outbound.end() &&
        PARTY_COMMAND_RESULT::ACCEPTED == result->CommandResult.eResult &&
        2u == rosters.size() &&
        rosters[0]->iPartyId == rosters[1]->iPartyId &&
        rosters[0]->iRosterRevision == rosters[1]->iRosterRevision &&
        2u == rosters[0]->Members.size() &&
        1u == rosters[0]->Members[0].iPartySlot &&
        rosters[0]->Members[0].isLeader &&
        2u == rosters[0]->Members[1].iPartySlot &&
        !rosters[0]->Members[1].isLeader &&
        1500u == rosters[0]->Members[1].iCurrentHp &&
        2000u == rosters[0]->Members[1].iMaximumHp,
        "Party invite commits one identical authoritative roster to both sessions");

    const std::uint64_t acceptedRevision =
        rosters.empty() ? 0u : rosters.front()->iRosterRevision;
    outbound.clear();
    presence[1].iCurrentHp = 1200u;
    service.Synchronize_Presence(presence, connected, outbound);
    const bool exactHpBroadcast = 2u == std::count_if(
        outbound.begin(), outbound.end(),
        [acceptedRevision](const SERVER_PARTY_OUTBOUND& item)
        {
            return PACKET_TYPE::S2C_PARTY_ROSTER == item.ePacketType &&
                item.Roster.iRosterRevision > acceptedRevision &&
                2u == item.Roster.Members.size() &&
                1200u == item.Roster.Members[1].iCurrentHp;
        });
    tests.Require(exactHpBroadcast,
        "Party HP change emits one newer full-state roster per member");

    outbound.clear();
    presence[1].eWorldId = WORLD_ID::VALTAN_ARENA;
    presence[1].iNetEntityId = 2202u;
    service.Synchronize_Presence(presence, connected, outbound);
    tests.Require(2u == std::count_if(outbound.begin(), outbound.end(),
        [](const SERVER_PARTY_OUTBOUND& item)
        {
            return PACKET_TYPE::S2C_PARTY_ROSTER == item.ePacketType &&
                2u == item.Roster.Members.size() &&
                WORLD_ID::VALTAN_ARENA ==
                    item.Roster.Members[1].eWorldId &&
                2u == item.Roster.Members[1].iPartySlot;
        }),
        "Bern to Valtan presence update preserves party identity and slot");

    outbound.clear();
    service.On_SessionClosed(202u, outbound);
    tests.Require(1u == std::count_if(outbound.begin(), outbound.end(),
        [](const SERVER_PARTY_OUTBOUND& item)
        {
            return PACKET_TYPE::S2C_PARTY_ROSTER == item.ePacketType &&
                INVALID_PARTY_ID == item.Roster.iPartyId &&
                item.Roster.Members.empty();
        }),
        "Disconnect dissolves a two-player party with one empty roster");
}
```

`TESTS`는 현재 파일에서 실측한 runner type이다. 위 aggregate presence initializer가 현재 compiler에서 경고를 만들면 field assignment helper로만 기계적으로 바꾸며 assertion 의미는 유지한다. 이어서 같은 service instance/fixture helper로 다음 거부 경로를 각각 실행한다.

1. Bern presence A/B sync
2. A -> B invite accepted
3. 두 outbound roster의 partyId/revision/order/leader/HP exact 일치
4. duplicate/self/full/not-leader/other-party가 revision을 바꾸지 않음
5. B HP 변경 뒤 두 member에 strictly newer roster
6. A/B의 Bern -> Valtan presence 변경 뒤 membership/order 유지
7. disconnect 뒤 roster 정규화 또는 empty dissolve

### 8.3 ClientFrontendHarness

`Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp`에는 `PartyViewModel.h`, `Party/PartyMessages.h`를 include하고 기존 `Make_MessageFrame()` 바로 뒤에 다음 test를 추가한다. main test sequence에서 `Test_PartyViewModel(runner);`를 호출한다.

```cpp
void Test_PartyViewModel(TEST_RUNNER& runner)
{
    using namespace LostArk::Shared;
    CNetworkManager& network = CNetworkManager::Get();
    network.Harness_Reset();
    Client::CPartyViewModel viewModel;

    S2C_PARTY_ROSTER roster{};
    roster.iRosterRevision = 10u;
    roster.iPartyId = 77u;
    roster.iSelfSocialPlayerId = 501u;
    PARTY_MEMBER_WIRE self{};
    self.iSocialPlayerId = 501u;
    self.eWorldId = WORLD_ID::BERN;
    self.iNetEntityId = 1001u;
    self.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
    self.strNickname = "VM-Self";
    self.iCurrentHp = 900u;
    self.iMaximumHp = 1000u;
    self.iPartySlot = 1u;
    self.isLeader = true;
    self.isPresent = true;
    PARTY_MEMBER_WIRE peer = self;
    peer.iSocialPlayerId = 502u;
    peer.iNetEntityId = 1002u;
    peer.eCharacterClass = CHARACTER_CLASS_ID::WARLORD;
    peer.strNickname = "VM-Peer";
    peer.iCurrentHp = 1200u;
    peer.iMaximumHp = 2000u;
    peer.iPartySlot = 2u;
    peer.isLeader = false;
    roster.Members = { self, peer };

    network.Harness_HandleFrame(Make_MessageFrame(
        PACKET_TYPE::S2C_PARTY_ROSTER, roster));
    viewModel.Update_FromNetwork();
    runner.Require(2u == viewModel.Get_Members().size() &&
        "VM-Peer" == viewModel.Get_Members()[1].strNickname &&
        1200u == viewModel.Get_Members()[1].iCurrentHp,
        "Party VM commits a valid full-state roster");

    S2C_PARTY_ROSTER stale = roster;
    stale.iRosterRevision = 9u;
    stale.Members[1].iCurrentHp = 1u;
    network.Harness_HandleFrame(Make_MessageFrame(
        PACKET_TYPE::S2C_PARTY_ROSTER, stale));
    viewModel.Update_FromNetwork();
    runner.Require(1200u == viewModel.Get_Members()[1].iCurrentHp,
        "Party VM preserves committed HP on stale revision");

    S2C_PARTY_COMMAND_RESULT result{};
    result.iRequestSequence = 3u;
    result.eResult = PARTY_COMMAND_RESULT::REJECTED_PARTY_FULL;
    result.iTargetSocialPlayerId = 502u;
    network.Harness_HandleFrame(Make_MessageFrame(
        PACKET_TYPE::S2C_PARTY_COMMAND_RESULT, result));
    viewModel.Update_FromNetwork();
    const string durableStatus = viewModel.Get_Status();
    viewModel.Update_FromNetwork();
    runner.Require(!durableStatus.empty() &&
        durableStatus == viewModel.Get_Status(),
        "Party result status survives after its inbound queue is drained");

    S2C_PARTY_ROSTER empty{};
    empty.iRosterRevision = 11u;
    empty.iSelfSocialPlayerId = 501u;
    network.Harness_HandleFrame(Make_MessageFrame(
        PACKET_TYPE::S2C_PARTY_ROSTER, empty));
    viewModel.Update_FromNetwork();
    runner.Require(viewModel.Get_Members().empty() &&
        viewModel.Get_Status().empty(),
        "Party VM clears roster and stale accepted status on authoritative dissolve");

    network.Harness_Reset();
    viewModel.Update_FromNetwork();
    runner.Require(viewModel.Get_Members().empty() &&
        viewModel.Get_Status().empty(),
        "Party VM clears ghost roster and old status on connection generation");
}
```

### 8.4 CharacterSelectIsolationHarness

`Tools/CharacterSelectIsolationHarness/Private/CharacterSelectIsolationHarness.cpp` include block에 다음을 추가한다.

```cpp
#include "Party/PartyMessages.h"
```

기존 `CTestClient`에 다음 send/getter와 committed state를 추가한다.

```cpp
bool Send_PartyInvite(
    const std::uint32_t requestSequence,
    const WORLD_ID targetWorldId,
    const NET_ENTITY_ID targetEntityId,
    std::string& error)
{
    C2S_PARTY_INVITE invite{};
    invite.iRequestSequence = requestSequence;
    invite.eTargetWorldId = targetWorldId;
    invite.iTargetNetEntityId = targetEntityId;
    return Send_Message(PACKET_TYPE::C2S_PARTY_INVITE, invite, error);
}

const S2C_PARTY_ROSTER& Get_PartyRoster() const
{
    return m_PartyRoster;
}
bool Has_PartyRoster() const { return m_hasPartyRoster; }
```

`Handle_Frame()`의 world snapshot branch 앞에 다음 두 decode branch를 둔다. `S2C_ENTER_ACCEPTED`에서는 이 상태를 지우지 않으므로 같은 socket Bern -> Valtan transfer를 관찰할 수 있고, `Close()`에서만 초기화한다.

```cpp
if (PACKET_TYPE::S2C_PARTY_COMMAND_RESULT == frame.ePacketType)
{
    S2C_PARTY_COMMAND_RESULT result{};
    if (!Read_Message(reader, result) || 0u != reader.Get_RemainingSize())
    {
        error = m_strLabel + ": invalid party result";
        return false;
    }
    m_PartyResults.insert_or_assign(result.iRequestSequence, result);
    return true;
}
if (PACKET_TYPE::S2C_PARTY_ROSTER == frame.ePacketType)
{
    S2C_PARTY_ROSTER roster{};
    if (!Read_Message(reader, roster) || 0u != reader.Get_RemainingSize())
    {
        error = m_strLabel + ": invalid party roster";
        return false;
    }
    if (!m_hasPartyRoster ||
        roster.iRosterRevision > m_PartyRoster.iRosterRevision)
    {
        m_PartyRoster = std::move(roster);
        m_hasPartyRoster = true;
    }
    return true;
}
```

```cpp
std::map<std::uint32_t, S2C_PARTY_COMMAND_RESULT> m_PartyResults;
S2C_PARTY_ROSTER m_PartyRoster{};
bool m_hasPartyRoster = false;
```

기존 Bern shared proof가 두 nickname/entity 관측을 통과한 직후 A가 다음 호출로 B를 초대한다.

```cpp
if (!first->Send_PartyInvite(
    1u, WORLD_ID::BERN, second->Get_NetEntityId(), error))
{
    return false;
}
```

그 뒤 기존 `Pump_Until`을 재사용해 다음 predicate를 실행형으로 확인한다.

1. A/B가 exact nickname과 서로의 entity를 관측
2. A가 B `(BERN, entity)`로 invite
3. A/B 모두 same partyId/revision, order 1/2 수신
4. B를 Valtan으로 transfer하고 A 또는 B의 roster가 membership을 잃지 않음
5. target disconnect 뒤 remaining member가 empty roster 수신

```cpp
const auto sameTwoMemberParty = [&]()
{
    if (!first->Has_PartyRoster() || !second->Has_PartyRoster())
        return false;
    const S2C_PARTY_ROSTER& left = first->Get_PartyRoster();
    const S2C_PARTY_ROSTER& right = second->Get_PartyRoster();
    return INVALID_PARTY_ID != left.iPartyId &&
        left.iPartyId == right.iPartyId &&
        left.iRosterRevision == right.iRosterRevision &&
        2u == left.Members.size() &&
        1u == left.Members[0].iPartySlot &&
        left.Members[0].isLeader &&
        2u == left.Members[1].iPartySlot &&
        !left.Members[1].isLeader;
};
```

transfer 단계는 기존 `Run_BernToValtanTransferProof()`의 authored trigger 이동을 복제하지 않는다. 같은 두 client fixture에서 B에 기존 `Send_Move()`와 기존 Valtan acceptance predicate를 적용하고, acceptance 뒤에도 `partyId`, 두 social ID, slot 1/2가 그대로인지 검사한다. B `Close()` 뒤 A가 strictly newer empty roster를 받을 때까지 pump한다. 테스트 종료 전에 A도 닫는다.

새 `.vcxproj`를 만들지 않는다.

## 9. 구현 전 READY 체크포인트

`Server/Private/ServerPartyService.cpp`와 `Client/Private/WorldPlayerSocialInteractionView.cpp` 전문은 이 문서에 모두 포함됐다. 구현자는 별도 설계를 발명하지 않고 다음 연결만 실제 코드에서 재확인한다.

1. interaction popup은 Level-owned이고 Invite/Cancel만 그린다. Server command result status는 MainApp-owned `CPartyViewModel`과 `CPartyWindowView`만 표시한다.
2. Level update는 `Collect_PlayerViews -> global mouse capture를 존중하는 interaction.Update -> existing MapTool LB와 Party LB OR 합성 -> RB 적용 -> PlayerController.Update` 순서다. Level destructor와 승인된 transfer early return은 interaction reset과 RB unblock만 수행하고 LB를 blind false로 덮어쓰지 않는다.
3. `Reset_SocialInboundState()`는 socket lifecycle에서만 호출하고 Bern/Valtan world acceptance에서는 호출하지 않는다.
4. `ServerPartyService` mutation은 room thread 하나에서만 수행하고 session callback은 bounded social command/closed-session queue만 건드린다.
5. 새 harness project를 만들지 않고 §8의 기존 세 harness와 `Server.exe --contract-test`에 검증을 편입한다.

## 10. 검증 명령

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Network/Sync-TeamLanEndpoint.ps1
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
$requiredUiAssets = @(
    'UI/Chat/English Input.png', 'UI/Chat/Input Bar.png',
    'UI/Chat/Korean Input.png', 'UI/Chat/LogPanelBg.png',
    'UI/Chat/Normal Bar.png', 'UI/Party/Party Hp Bg.png',
    'UI/Party/Party HP.png', 'UI/Party/Party Leader Mark.png',
    'UI/Party/Party Name.png', 'UI/Party/Party No.1.png',
    'UI/Party/Party No.2.png', 'UI/Party/Party No.3.png',
    'UI/Party/Party No.4.png'
)
$missingUiAssets = $requiredUiAssets | Where-Object {
    -not (Test-Path -LiteralPath (Join-Path 'Client/Bin/Resources' $_))
}
if ($missingUiAssets) { throw "Missing UI assets: $($missingUiAssets -join ', ')" }
git diff --check
```

Client/UI는 에이전트가 실행하지 않는다. UI 폴더를 실행 중인 Client에 복사했다면 missing result도 보관하는 `CUITextureCache` 때문에 Client를 재시작해야 한다. 사용자가 Bern/Valtan 두 Client에서 초대 전 fake roster 부재, remote hover, 다른 UI 위 RMB 차단, Invite/Cancel, Party HP/leader/order, transfer persistence와 기존 Chat Enter/Escape 회귀를 직접 확인한 뒤에만 visual PASS를 기록한다.
