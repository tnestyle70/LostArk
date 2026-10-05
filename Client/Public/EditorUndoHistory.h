#pragma once

#include <algorithm>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace Client
{
// Value snapshots only: owners keep save baselines, file freshness and runtime
// resources outside this history. A shared immutable document is a valid value.
template<class State>
class CEditorUndoHistory final
{
public:
    explicit CEditorUndoHistory(std::size_t limit = 64u) : m_Limit((std::max)(std::size_t{1}, limit)) {}

    void Begin(State before)
    {
        if (!m_Pending) m_Pending = std::move(before);
    }
    template<class EqualValues>
    bool Commit(State after, EqualValues same)
    {
        if (!m_Pending) return false;
        if (same(*m_Pending, after)) { m_Pending.reset(); return false; }
        ENTRY entry{*m_Pending, std::move(after)};
        m_Undo.push_back(std::move(entry));
        if (m_Undo.size() > m_Limit) m_Undo.erase(m_Undo.begin());
        m_Redo.clear(); m_Pending.reset();
        return true;
    }
    void Cancel() { m_Pending.reset(); }
    void Clear() { m_Pending.reset(); m_Undo.clear(); m_Redo.clear(); }
    [[nodiscard]] bool Is_Pending() const { return m_Pending.has_value(); }
    [[nodiscard]] bool Can_Undo() const { return !m_Pending && !m_Undo.empty(); }
    [[nodiscard]] bool Can_Redo() const { return !m_Pending && !m_Redo.empty(); }
    [[nodiscard]] std::size_t Count_Undo() const { return m_Undo.size(); }
    [[nodiscard]] std::size_t Count_Redo() const { return m_Redo.size(); }

    template<class Apply>
    bool Undo(Apply apply)
    {
        if (!Can_Undo()) return false;
        // Reserve before the owner's transaction; allocation failure must not
        // leave a restored document without its corresponding history entry.
        m_Redo.reserve(m_Redo.size() + 1u);
        if (!Apply_Target(apply, m_Undo.back().before, m_Undo.back().after)) return false;
        m_Redo.push_back(std::move(m_Undo.back())); m_Undo.pop_back();
        return true;
    }
    template<class Apply>
    bool Redo(Apply apply)
    {
        if (!Can_Redo()) return false;
        m_Undo.reserve(m_Undo.size() + 1u);
        if (!Apply_Target(apply, m_Redo.back().after, m_Redo.back().before)) return false;
        m_Undo.push_back(std::move(m_Redo.back())); m_Redo.pop_back();
        return true;
    }

private:
    template<class Apply>
    static bool Apply_Target(Apply& apply, const State& target, const State& expectedCurrent)
    {
        // Composite owners may require the entry's exact opposite endpoint as
        // a CAS precondition. A newly captured live value is not that receipt.
        if constexpr (std::is_invocable_r_v<bool, Apply&, const State&, const State&>)
            return apply(target, expectedCurrent);
        else
            return apply(target);
    }

    struct ENTRY { State before, after; };
    std::size_t m_Limit;
    std::optional<State> m_Pending;
    std::vector<ENTRY> m_Undo, m_Redo;
};
}
