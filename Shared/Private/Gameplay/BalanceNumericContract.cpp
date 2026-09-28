#include "Gameplay/BalanceNumericContract.h"
#include "Network/PacketReader.h"
#include "Network/PacketWriter.h"
#include <bit>
#include <cmath>
#include <set>
#include <tuple>
#include <utility>

using namespace LostArk::Shared;
namespace
{
    bool Key(BALANCE_DOMAIN domain, const std::string& id, const std::string& field)
    {
        const auto stable = [](const std::string& s, std::size_t limit) {
            if (s.empty() || s.size() > limit) return false;
            for (const unsigned char c : s)
                if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                    (c >= '0' && c <= '9') || c == '_' || c == '.' || c == '-' || c == ':' || c == '/' || c == '|')) return false;
            return true;
        };
        return domain < BALANCE_DOMAIN::END && stable(id, MAX_BALANCE_ID_BYTES) && stable(field, MAX_BALANCE_FIELD_BYTES);
    }
    void Double(CPacketWriter& writer, double value)
    {
        const auto bits = std::bit_cast<std::uint64_t>(value);
        writer.Write_U32(static_cast<std::uint32_t>(bits));
        writer.Write_U32(static_cast<std::uint32_t>(bits >> 32u));
    }
    bool Double(CPacketReader& reader, double& value)
    {
        std::uint32_t low{}, high{};
        if (!reader.Read_U32(low) || !reader.Read_U32(high)) return false;
        value = std::bit_cast<double>((static_cast<std::uint64_t>(high) << 32u) | low);
        return std::isfinite(value);
    }
    template<class T> bool UniqueKeys(const std::vector<T>& entries)
    {
        std::set<std::tuple<BALANCE_DOMAIN, std::string, std::string>> keys;
        for (const auto& entry : entries)
            if (!Key(entry.eDomain, entry.strId, entry.strField) ||
                !std::isfinite(entry.fValue) || !keys.emplace(entry.eDomain, entry.strId, entry.strField).second) return false;
        return true;
    }
    template<class T> bool WriteKey(CPacketWriter& writer, const T& entry)
    {
        writer.Write_U8(static_cast<std::uint8_t>(entry.eDomain));
        return writer.Write_String(entry.strId, MAX_BALANCE_ID_BYTES) && writer.Write_String(entry.strField, MAX_BALANCE_FIELD_BYTES);
    }
    template<class T> bool ReadKey(CPacketReader& reader, T& entry)
    {
        std::uint8_t domain{};
        if (!reader.Read_U8(domain) || !reader.Read_String(entry.strId, MAX_BALANCE_ID_BYTES) ||
            !reader.Read_String(entry.strField, MAX_BALANCE_FIELD_BYTES)) return false;
        entry.eDomain = static_cast<BALANCE_DOMAIN>(domain);
        return Key(entry.eDomain, entry.strId, entry.strField);
    }
}
bool LostArk::Shared::Write_Message(CPacketWriter& w, const C2S_BALANCE_QUERY& m)
{
    if (!m.iRequestSequence || m.iPageIndex >= MAX_BALANCE_PAGES) return false;
    w.Write_U32(m.iRequestSequence); w.Write_U32(m.iPageIndex); return true;
}
bool LostArk::Shared::Read_Message(CPacketReader& r, C2S_BALANCE_QUERY& m)
{
    C2S_BALANCE_QUERY s;
    if (!r.Read_U32(s.iRequestSequence) || !r.Read_U32(s.iPageIndex) || !s.iRequestSequence || s.iPageIndex >= MAX_BALANCE_PAGES) return false;
    m = s; return true;
}
bool LostArk::Shared::Write_Message(CPacketWriter& w, const S2C_BALANCE_SNAPSHOT& m)
{
    if (!m.iRequestSequence || !m.NumericRevision.Is_Valid() || !m.iPageCount || m.iPageCount > MAX_BALANCE_PAGES ||
        m.iPageIndex >= m.iPageCount || m.Entries.size() > MAX_BALANCE_PAGE_ENTRIES || !UniqueKeys(m.Entries)) return false;
    for (const auto& e : m.Entries) if (e.isIntegral && std::floor(e.fValue) != e.fValue) return false;
    w.Write_U32(m.iRequestSequence); if (!Write_GameplayDataRevision(w, m.NumericRevision)) return false;
    w.Write_U32(m.iPageIndex); w.Write_U32(m.iPageCount); w.Write_U16(static_cast<std::uint16_t>(m.Entries.size()));
    for (const auto& e : m.Entries) { if (!WriteKey(w, e)) return false; Double(w, e.fValue); w.Write_U8(e.isIntegral ? 1u : 0u); }
    return true;
}
bool LostArk::Shared::Read_Message(CPacketReader& r, S2C_BALANCE_SNAPSHOT& m)
{
    S2C_BALANCE_SNAPSHOT s; std::uint16_t count{};
    if (!r.Read_U32(s.iRequestSequence) || !s.iRequestSequence || !Read_GameplayDataRevision(r, s.NumericRevision) ||
        !r.Read_U32(s.iPageIndex) || !r.Read_U32(s.iPageCount) || !s.iPageCount || s.iPageCount > MAX_BALANCE_PAGES ||
        s.iPageIndex >= s.iPageCount || !r.Read_U16(count) || count > MAX_BALANCE_PAGE_ENTRIES) return false;
    s.Entries.resize(count);
    for (auto& e : s.Entries)
    {
        std::uint8_t integral{};
        if (!ReadKey(r, e) || !Double(r, e.fValue) || !r.Read_U8(integral) || integral > 1u) return false;
        e.isIntegral = integral != 0u;
        if (e.isIntegral && std::floor(e.fValue) != e.fValue) return false;
    }
    if (!UniqueKeys(s.Entries)) return false;
    m = std::move(s); return true;
}
bool LostArk::Shared::Write_Message(CPacketWriter& w, const C2S_BALANCE_PATCH& m)
{
    if (!m.iRequestSequence || !m.BaseNumericRevision.Is_Valid() || m.Changes.empty() || m.Changes.size() > MAX_BALANCE_CHANGES || !UniqueKeys(m.Changes)) return false;
    for (const auto& e : m.Changes) if (!std::isfinite(e.fBefore)) return false;
    w.Write_U32(m.iRequestSequence); if (!Write_GameplayDataRevision(w, m.BaseNumericRevision)) return false;
    w.Write_U16(static_cast<std::uint16_t>(m.Changes.size()));
    for (const auto& e : m.Changes) { if (!WriteKey(w, e)) return false; Double(w, e.fBefore); Double(w, e.fValue); }
    return true;
}
bool LostArk::Shared::Read_Message(CPacketReader& r, C2S_BALANCE_PATCH& m)
{
    C2S_BALANCE_PATCH s; std::uint16_t count{};
    if (!r.Read_U32(s.iRequestSequence) || !s.iRequestSequence || !Read_GameplayDataRevision(r, s.BaseNumericRevision) ||
        !r.Read_U16(count) || !count || count > MAX_BALANCE_CHANGES) return false;
    s.Changes.resize(count);
    for (auto& e : s.Changes) if (!ReadKey(r, e) || !Double(r, e.fBefore) || !Double(r, e.fValue)) return false;
    if (!UniqueKeys(s.Changes)) return false;
    m = std::move(s); return true;
}
bool LostArk::Shared::Write_Message(CPacketWriter& w, const S2C_BALANCE_RESULT& m)
{
    if (m.eResult >= BALANCE_APPLY_RESULT::END || !m.ActiveNumericRevision.Is_Valid() || m.strReason.size() > MAX_BALANCE_REASON_BYTES ||
        (!m.iRequestSequence && m.eResult != BALANCE_APPLY_RESULT::APPLIED)) return false;
    w.Write_U32(m.iRequestSequence); w.Write_U8(static_cast<std::uint8_t>(m.eResult));
    return Write_GameplayDataRevision(w, m.ActiveNumericRevision) && w.Write_String(m.strReason, MAX_BALANCE_REASON_BYTES);
}
bool LostArk::Shared::Read_Message(CPacketReader& r, S2C_BALANCE_RESULT& m)
{
    S2C_BALANCE_RESULT s; std::uint8_t result{};
    if (!r.Read_U32(s.iRequestSequence) || !r.Read_U8(result) || result >= static_cast<std::uint8_t>(BALANCE_APPLY_RESULT::END) ||
        !Read_GameplayDataRevision(r, s.ActiveNumericRevision) || !r.Read_String(s.strReason, MAX_BALANCE_REASON_BYTES)) return false;
    s.eResult = static_cast<BALANCE_APPLY_RESULT>(result);
    if (!s.iRequestSequence && s.eResult != BALANCE_APPLY_RESULT::APPLIED) return false;
    m = std::move(s); return true;
}
