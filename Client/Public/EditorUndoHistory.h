#pragma once

#include <algorithm>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace Client
{
// 값 snapshot만 저장한다. 저장 baseline, 파일 freshness, runtime 자원은 각 편집기 owner가 별도로 관리한다.
// State는 값 자체 또는 수정 불가능한 문서를 가리키는 shared_ptr처럼 안전하게 보관할 수 있는 타입이어야 한다.
template<class State>
class CEditorUndoHistory final
{
public:
    // 각 Workbench/WorldObjectTool 등이 소유하는 bounded history다. 보관 한도 0은 1로 올린다. 내부 lock은 없다.
    explicit CEditorUndoHistory(std::size_t limit = 64u)
        : m_Limit((std::max)(std::size_t{1}, limit))
    {}

    // 한 drag/편집 동작의 최초 before만 보관한다. 이미 pending이면 중간 프레임의 Begin으로 시작점을 덮지 않는다.
    void Begin(State before)
    {
        if (!m_Pending)
            m_Pending = std::move(before);
    }
    // owner의 same 비교로 실질 문서 변경을 판단한다. 같으면 pending만 닫고 기존 redo는 유지한다.
    // 변경이 있으면 undo에 쌓고 오래된 기록부터 제거하며 새 분기와 양립하지 않는 redo는 비운다.
    template<class EqualValues>
    bool Commit(State after, EqualValues same)
    {
        if (!m_Pending)
            return false;
        if (same(*m_Pending, after))
        {
            m_Pending.reset();
            return false;
        }
        ENTRY entry{*m_Pending, std::move(after)};
        m_Undo.push_back(std::move(entry));
        if (m_Undo.size() > m_Limit)
            m_Undo.erase(m_Undo.begin());
        m_Redo.clear();
        m_Pending.reset();
        return true;
    }
    // 편집 중 기록만 취소한다. 이미 바뀐 owner 문서까지 되돌리지는 않는다.
    void Cancel()
    {
        m_Pending.reset();
    }
    // 새 문서 등에서 pending/undo/redo를 모두 버린다. owner의 문서나 저장 파일은 직접 건드리지 않는다.
    void Clear()
    {
        m_Pending.reset();
        m_Undo.clear();
        m_Redo.clear();
    }
    [[nodiscard]] bool Is_Pending() const
    {
        return m_Pending.has_value();
    }
    // 미완료 gesture가 있으면 과거 기록으로 이동하지 않는다.
    [[nodiscard]] bool Can_Undo() const
    {
        return !m_Pending && !m_Undo.empty();
    }
    [[nodiscard]] bool Can_Redo() const
    {
        return !m_Pending && !m_Redo.empty();
    }
    [[nodiscard]] std::size_t Count_Undo() const
    {
        return m_Undo.size();
    }
    [[nodiscard]] std::size_t Count_Redo() const
    {
        return m_Redo.size();
    }

    // owner가 before 복원을 승인한 뒤에만 기록을 redo로 옮긴다. callback false이면 stack 위치는 유지한다.
    // callback이 바꾼 외부 상태의 원상복구와 freshness 검증은 callback 자체 책임이다.
    template<class Apply>
    bool Undo(Apply apply)
    {
        if (!Can_Undo())
            return false;
        // owner의 실제 문서 복원 전에 목적지 vector 용량을 확보한다.
        // 용량 할당 실패가 복원 뒤 발생하지 않게 한다. 임의 State의 이동/복사 예외까지 보장하는 transaction은 아니다.
        m_Redo.reserve(m_Redo.size() + 1u);
        if (!Apply_Target(apply, m_Undo.back().before, m_Undo.back().after))
            return false;
        m_Redo.push_back(std::move(m_Undo.back()));
        m_Undo.pop_back();
        return true;
    }
    // owner가 after 복원을 승인한 뒤 기록을 undo로 옮긴다. 실패한 항목은 다음 재시도에 남긴다.
    template<class Apply>
    bool Redo(Apply apply)
    {
        if (!Can_Redo())
            return false;
        m_Undo.reserve(m_Undo.size() + 1u);
        if (!Apply_Target(apply, m_Redo.back().after, m_Redo.back().before))
            return false;
        m_Undo.push_back(std::move(m_Redo.back()));
        m_Redo.pop_back();
        return true;
    }

private:
    template<class Apply>
    static bool Apply_Target(Apply& apply, const State& target, const State& expectedCurrent)
    {
        // 여러 owner를 복원하는 callback에는 기록된 반대편 endpoint를 기대 상태로 함께 전달할 수 있다.
        // 이 값은 현재 문서를 새로 읽은 값이 아니며, 각 owner가 실제 상태와 대조할 CAS 기준이다.
        if constexpr (std::is_invocable_r_v<bool, Apply&, const State&, const State&>)
            return apply(target, expectedCurrent);
        else
            return apply(target);
    }

    // 각 기록이 편집 전후 값을 소유하고 두 vector의 뒤쪽을 최신 기록으로 사용한다.
    struct ENTRY
    {
        State before, after;
    };
    std::size_t m_Limit;
    std::optional<State> m_Pending;
    std::vector<ENTRY> m_Undo, m_Redo;
};
}
