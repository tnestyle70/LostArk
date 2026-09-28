#pragma once

#include "GameplayDataRevision.h"
#include <cstdint>
#include <string>
#include <vector>

namespace LostArk::Shared
{
    class CPacketReader;
    class CPacketWriter;
    inline constexpr std::size_t MAX_BALANCE_CHANGES = 128u;
    inline constexpr std::size_t MAX_BALANCE_PAGE_ENTRIES = 64u;
    inline constexpr std::uint32_t MAX_BALANCE_PAGES = 512u;
    inline constexpr std::size_t MAX_BALANCE_ID_BYTES = 128u;
    inline constexpr std::size_t MAX_BALANCE_FIELD_BYTES = 64u;
    inline constexpr std::size_t MAX_BALANCE_REASON_BYTES = 512u;

    enum class BALANCE_DOMAIN : std::uint8_t { PLAYER, SKILL, DAMAGE, BOSS, MADNESS, STAGGER, PATTERN_DAMAGE, END };
    enum class BALANCE_APPLY_RESULT : std::uint8_t
    { APPLIED, STALE_REVISION, INVALID_CHANGE, BUSY, SAVE_FAILED, INVALID_SESSION, END };

    struct BALANCE_NUMERIC_ENTRY
    {
        BALANCE_DOMAIN eDomain = BALANCE_DOMAIN::PLAYER;
        std::string strId, strField;
        double fValue = 0.0;
        bool isIntegral = true;
    };
    struct BALANCE_NUMERIC_CHANGE
    {
        BALANCE_DOMAIN eDomain = BALANCE_DOMAIN::PLAYER;
        std::string strId, strField;
        double fBefore = 0.0, fValue = 0.0;
    };
    struct C2S_BALANCE_QUERY
    {
        std::uint32_t iRequestSequence = 0u, iPageIndex = 0u;
    };
    struct S2C_BALANCE_SNAPSHOT
    {
        std::uint32_t iRequestSequence = 0u;
        GameplayDataRevision NumericRevision{};
        std::uint32_t iPageIndex = 0u, iPageCount = 0u;
        std::vector<BALANCE_NUMERIC_ENTRY> Entries;
    };
    struct C2S_BALANCE_PATCH
    {
        std::uint32_t iRequestSequence = 0u;
        GameplayDataRevision BaseNumericRevision{};
        std::vector<BALANCE_NUMERIC_CHANGE> Changes;
    };
    struct S2C_BALANCE_RESULT
    {
        // Zero is the process-wide applied-generation announcement.
        std::uint32_t iRequestSequence = 0u;
        BALANCE_APPLY_RESULT eResult = BALANCE_APPLY_RESULT::INVALID_CHANGE;
        GameplayDataRevision ActiveNumericRevision{};
        std::string strReason;
    };

    bool Write_Message(CPacketWriter&, const C2S_BALANCE_QUERY&);
    bool Read_Message(CPacketReader&, C2S_BALANCE_QUERY&);
    bool Write_Message(CPacketWriter&, const S2C_BALANCE_SNAPSHOT&);
    bool Read_Message(CPacketReader&, S2C_BALANCE_SNAPSHOT&);
    bool Write_Message(CPacketWriter&, const C2S_BALANCE_PATCH&);
    bool Read_Message(CPacketReader&, C2S_BALANCE_PATCH&);
    bool Write_Message(CPacketWriter&, const S2C_BALANCE_RESULT&);
    bool Read_Message(CPacketReader&, S2C_BALANCE_RESULT&);
}
