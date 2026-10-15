#include "EditorUndoHistory.h"
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace
{
// 실제 EditorUndoHistory.h를 포함한 작은 실행 파일이다. UI나 저장 파일 없이 값 이력의 계약만 확인한다.
int checks = 0, failures = 0;
// 조건을 누적 채점하며 실패 label을 stderr로 남긴다. 첫 실패 뒤에도 나머지를 실행하고 main의 종료 코드로 전체 실패를 알린다.
void Check(bool condition, const char *label)
{
    ++checks;
    if (!condition)
    {
        ++failures;
        std::cerr << "FAIL: " << label << '\n';
    }
}
// 편집 owner의 문서와 선택을 함께 snapshot으로 보관한다. saved는 별도 복사본이므로 Undo가 파일 저장 기준을 바꾸지 않아야 한다.
struct State
{
    std::vector<std::string> elements;
    std::string selection;
    bool operator==(const State &) const = default;
};
// 선택 이동만으로 새 Undo 항목을 만들지 않도록 문서 요소만 비교한다. 실제 복원 snapshot에는 선택도 포함된다.
bool SameDocument(const State &a, const State &b)
{
    return a.elements == b.elements;
}

// 삭제→Undo→Redo, 저장 기준의 독립성, owner의 복원 거절과 재시도, drag 묶음과 취소를 실제 값으로 확인한다.
void OwnerTransactions()
{
    Client::CEditorUndoHistory<State> history(3);
    State draft{{"plant", "water", "rock"}, "water"};
    State saved = draft;
    const auto apply = [&](const State &state) {
        draft = state;
        return true;
    };
    Check(!history.Can_Undo() && !history.Can_Redo(), "fresh history is empty");
    Check(!history.Undo(apply) && !history.Redo(apply), "empty navigation does not apply");

    history.Begin(draft);
    draft.elements.erase(draft.elements.begin() + 1);
    draft.selection = "rock";
    Check(history.Commit(draft, SameDocument), "delete records a transaction");
    Check(history.Undo(apply) && draft == saved, "undo restores deleted value and original selection");
    Check(history.Redo(apply) && draft.elements.size() == 2 && draft.selection == "rock",
          "redo restores deletion and resulting selection");
    saved = draft;
    Check(history.Undo(apply) && draft != saved, "save baseline does not suppress undo");
    Check(saved.elements.size() == 2 && saved.selection == "rock",
          "undo does not change saved baseline");
    Check(history.Redo(apply) && draft == saved, "redo to saved state is clean by owner comparison");

    // 이 거절 callback은 문서를 바꾸지 않는다. 외부 상태 보존은 owner 책임이며 history가 임의 부작용을 rollback하는 테스트가 아니다.
    const auto beforeRejected = draft;
    Check(!history.Undo([](const State &) { return false; }),
          "owner freshness rejection keeps history");
    Check(draft == beforeRejected && history.Count_Undo() == 1 && history.Count_Redo() == 0,
          "rejected undo preserves both stack counts and document");
    Check(history.Undo(apply), "rejected undo remains retryable");
    Check(!history.Redo([](const State &) { return false; }) && history.Count_Redo() == 1,
          "rejected redo remains retryable");

    // 선택만 바꾼 no-op commit은 기존 redo 분기를 유지해야 한다. 뒤의 실제 요소 추가는 그 분기를 폐기해야 한다.
    history.Begin(draft);
    draft.selection = "plant";
    Check(!history.Commit(draft, SameDocument) && history.Can_Redo(),
          "selection-only noop does not discard redo");
    history.Begin(draft);
    draft.elements.push_back("flower");
    history.Begin(draft);
    draft.elements.push_back("tree");
    Check(history.Is_Pending() && !history.Can_Undo() && !history.Can_Redo(),
          "unfinished gesture cannot navigate");
    Check(history.Commit(draft, SameDocument) && history.Count_Redo() == 0,
          "new edit invalidates redo branch");
    Check(history.Undo(apply) && draft.elements.size() == 3 && draft.selection == "plant",
          "one gesture restores first before snapshot");
    Check(history.Redo(apply) && draft.elements.size() == 5,
          "one gesture restores final after snapshot");

    history.Begin(draft);
    history.Cancel();
    Check(!history.Is_Pending() && history.Count_Undo() == 1, "cancel does not add history");
    history.Begin(draft);
    history.Clear();
    Check(!history.Is_Pending() && !history.Can_Undo() && !history.Can_Redo(),
          "new document clears pending and both stacks");
}

// 한도 2에 편집 다섯 번을 넣어 가장 최근 두 전이만 남는지 검사한다. 한도 0은 최소 1로 처리되는 계약이다.
void BoundedHistory()
{
    Client::CEditorUndoHistory<int> history(2);
    int value = 0;
    const auto equal = [](int a, int b) { return a == b; };
    const auto apply = [&](int next) {
        value = next;
        return true;
    };
    for (int next = 1; next <= 5; ++next)
    {
        history.Begin(value);
        value = next;
        Check(history.Commit(value, equal), "bounded edit committed");
    }
    Check(history.Count_Undo() == 2, "capacity bounds retained transactions");
    Check(history.Undo(apply) && value == 4 && history.Undo(apply) && value == 3,
          "capacity retains latest contiguous undo range");
    Check(!history.Undo(apply) && value == 3, "discarded oldest state is unreachable");
    Check(history.Redo(apply) && value == 4 && history.Redo(apply) && value == 5,
          "bounded range redo is ordered");
    Client::CEditorUndoHistory<int> minimum(0);
    minimum.Begin(1);
    Check(minimum.Commit(2, equal) && minimum.Count_Undo() == 1,
          "zero capacity clamps to one useful transaction");
}

// 두 인자 callback의 순서를 확인한다. Undo는 before가 목적지, after가 기대 현재 값이고 Redo는 반대다.
void ExpectedStateArguments()
{
    Client::CEditorUndoHistory<int> history;
    history.Begin(10);
    Check(history.Commit(20, [](int a, int b) { return a == b; }), "two-owner edit recorded");
    int calls = 0;
    Check(history.Undo([&](const int &target, const int &expected) {
        ++calls;
        Check(target == 10 && expected == 20, "undo passes before target and after expectation");
        return true;
    }),
          "two-argument undo callback accepted");
    Check(history.Redo([&](const int &target, const int &expected) {
        ++calls;
        Check(target == 20 && expected == 10, "redo passes after target and before expectation");
        return true;
    }),
          "two-argument redo callback accepted");
    Check(calls == 2, "two-argument callback invoked once per restore");
}

// 지역 문서와 World를 함께 참조할 때, 다른 owner의 후속 편집을 옛 snapshot으로 덮지 않는지 검사한다.
void CrossOwnerConflict()
{
    struct Snapshot
    {
        std::string document, world, selection;
        bool operator==(const Snapshot &) const = default;
    };
    Client::CEditorUndoHistory<Snapshot> history;
    std::string document = "pattern.0", world = "A", selection = "world.box";
    const auto capture = [&] { return Snapshot{document, world, selection}; };
    const auto equal = [](const Snapshot &a, const Snapshot &b) { return a == b; };
    const auto apply = [&](const Snapshot &target, const Snapshot &expected) {
        // document는 항상, world는 이번 전이가 바꿀 때만 기대 현재 값과 비교한다. selection은 비교 없이 복원한다.
        // 두 검사를 모두 통과하기 전에 문서에 대입하지 않으므로 충돌 거절은 부분 복원을 남기지 않는다.
        if (document != expected.document)
            return false;
        const bool changesWorld = target.world != expected.world;
        if (changesWorld && world != expected.world)
            return false;
        document = target.document;
        if (changesWorld)
            world = target.world;
        selection = target.selection;
        return true;
    };

    history.Begin(capture());
    world = "B";
    Check(history.Commit(capture(), equal), "workbench A to B world edit recorded");
    world = "C"; // history 밖의 독립 World 편집이 일어난 상황이다.
    history.Begin(capture());
    document = "pattern.1";
    selection = "effect.row";
    Check(history.Commit(capture(), equal), "local edit after external world edit recorded");
    Check(history.Undo(apply) && document == "pattern.0" && world == "C" && selection == "world.box",
          "local-only undo preserves independently edited World C");

    // 옛 World 편집은 B를 기대하지만 현재 C이므로 실패해야 한다. 실패 뒤 전체 문서와 두 stack 위치를 모두 비교한다.
    const auto beforeConflict = capture();
    Check(!history.Undo(apply), "old world undo rejects expected B versus current C");
    Check(capture() == beforeConflict, "cross-owner conflict preserves all documents and selection");
    Check(history.Count_Undo() == 1 && history.Count_Redo() == 1,
          "cross-owner conflict preserves both stack positions");
    Check(history.Redo(apply) && document == "pattern.1" && world == "C",
          "local-only redo remains usable after rejected world undo");
    Check(history.Undo(apply) && document == "pattern.0" && world == "C",
          "local-only undo remains retryable after cross-owner conflict");

    world = "B"; // 독립 owner가 기대 상태로 되돌린 뒤 같은 Undo를 재시도한다.
    Check(history.Undo(apply) && document == "pattern.0" && world == "A",
          "world undo retries once its actual expected state is restored");
    world = "C";
    const auto beforeRedoConflict = capture();
    Check(!history.Redo(apply) && capture() == beforeRedoConflict,
          "world redo rejects expected A versus current C without partial restore");
    Check(history.Count_Undo() == 0 && history.Count_Redo() == 2,
          "rejected world redo preserves the whole redo chain");
    world = "A";
    Check(history.Redo(apply) && world == "B", "world redo retries after expected state is restored");
    Check(history.Redo(apply) && document == "pattern.1" && world == "B" && selection == "effect.row",
          "local-only redo never reapplies its historical unchanged World C snapshot");
}

void ModelSequence()
{
    // 제품의 두 stack 대신 상태 배열과 cursor를 정답 모델로 쓴다. 혼합 조작마다 반환값·현재 값·이력 위치를 비교한다.
    constexpr std::size_t limit = 8;
    Client::CEditorUndoHistory<int> history(limit);
    std::vector<int> states{0};
    std::size_t cursor = 0;
    int current = 0;
    // 고정 seed라 실패 순서를 재현할 수 있다. 이 5,000회 표본을 모든 State 타입·예외·동시성에 대한 증명으로 해석하지 않는다.
    std::mt19937 random(20261005);
    for (int step = 0; step < 5000; ++step)
    {
        const auto operation = random() % 7;
        const bool reject = random() % 5 == 0;
        if (operation < 3)
        {
            const int next = operation == 0 ? current : static_cast<int>(random() % 10000);
            const bool changed = next != current;
            history.Begin(current);
            current = next;
            Check(history.Commit(current, [](int a, int b) { return a == b; }) == changed,
                  "model commit result");
            if (changed)
            {
                // 새 편집은 정답 모델에서도 cursor 뒤의 미래 상태를 버리고 다음 값을 붙인다. 유지할 상태 수는 전이 한도+1이다.
                states.resize(cursor + 1);
                states.push_back(next);
                ++cursor;
                if (states.size() > limit + 1)
                {
                    states.erase(states.begin());
                    --cursor;
                }
            }
        }
        else if (operation < 5)
        {
            const bool expected = cursor > 0 && !reject;
            const bool actual = history.Undo([&](int next) {
                if (reject)
                    return false;
                current = next;
                return true;
            });
            if (expected)
                --cursor;
            Check(actual == expected, "model undo result");
        }
        else
        {
            const bool expected = cursor + 1 < states.size() && !reject;
            const bool actual = history.Redo([&](int next) {
                if (reject)
                    return false;
                current = next;
                return true;
            });
            if (expected)
                ++cursor;
            Check(actual == expected, "model redo result");
        }
        // 결과 bool만 맞는 구현도 문서 값이나 stack 위치가 틀릴 수 있으므로 세 결과를 따로 대조한다.
        Check(current == states[cursor], "model restored value");
        Check(history.Count_Undo() == cursor && history.Count_Redo() == states.size() - cursor - 1,
              "model history position");
    }
}
} // namespace

// 기존 검사 다섯 묶음을 실행한다. 출력은 개별 Check 호출 수이며 실패가 하나라도 있으면 runner가 감지할 종료 코드 1을 반환한다.
int main()
{
    OwnerTransactions();
    BoundedHistory();
    ExpectedStateArguments();
    CrossOwnerConflict();
    ModelSequence();
    std::cout << "EditorUndoHistory: " << checks << " checks, " << failures << " failures\n";
    return failures ? 1 : 0;
}
