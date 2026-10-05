#include "EditorUndoHistory.h"
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace
{
int checks = 0, failures = 0;
void Check(bool condition, const char* label)
{
    ++checks;
    if (!condition) { ++failures; std::cerr << "FAIL: " << label << '\n'; }
}
struct State
{
    std::vector<std::string> elements;
    std::string selection;
    bool operator==(const State&) const = default;
};
bool SameDocument(const State& a, const State& b) { return a.elements == b.elements; }

void OwnerTransactions()
{
    Client::CEditorUndoHistory<State> history(3);
    State draft{{"plant", "water", "rock"}, "water"};
    State saved = draft;
    const auto apply = [&](const State& state) { draft = state; return true; };
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
    Check(saved.elements.size() == 2 && saved.selection == "rock", "undo does not change saved baseline");
    Check(history.Redo(apply) && draft == saved, "redo to saved state is clean by owner comparison");

    const auto beforeRejected = draft;
    Check(!history.Undo([](const State&) { return false; }), "owner freshness rejection keeps history");
    Check(draft == beforeRejected && history.Count_Undo() == 1 && history.Count_Redo() == 0,
        "rejected undo preserves both stack counts and document");
    Check(history.Undo(apply), "rejected undo remains retryable");
    Check(!history.Redo([](const State&) { return false; }) && history.Count_Redo() == 1,
        "rejected redo remains retryable");

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
    Check(history.Redo(apply) && draft.elements.size() == 5, "one gesture restores final after snapshot");

    history.Begin(draft);
    history.Cancel();
    Check(!history.Is_Pending() && history.Count_Undo() == 1, "cancel does not add history");
    history.Begin(draft);
    history.Clear();
    Check(!history.Is_Pending() && !history.Can_Undo() && !history.Can_Redo(),
        "new document clears pending and both stacks");
}

void BoundedHistory()
{
    Client::CEditorUndoHistory<int> history(2);
    int value = 0;
    const auto equal = [](int a, int b) { return a == b; };
    const auto apply = [&](int next) { value = next; return true; };
    for (int next = 1; next <= 5; ++next)
    {
        history.Begin(value); value = next; Check(history.Commit(value, equal), "bounded edit committed");
    }
    Check(history.Count_Undo() == 2, "capacity bounds retained transactions");
    Check(history.Undo(apply) && value == 4 && history.Undo(apply) && value == 3,
        "capacity retains latest contiguous undo range");
    Check(!history.Undo(apply) && value == 3, "discarded oldest state is unreachable");
    Check(history.Redo(apply) && value == 4 && history.Redo(apply) && value == 5,
        "bounded range redo is ordered");
    Client::CEditorUndoHistory<int> minimum(0);
    minimum.Begin(1); Check(minimum.Commit(2, equal) && minimum.Count_Undo() == 1,
        "zero capacity clamps to one useful transaction");
}

void ExpectedStateArguments()
{
    Client::CEditorUndoHistory<int> history;
    history.Begin(10);
    Check(history.Commit(20, [](int a, int b) { return a == b; }), "two-owner edit recorded");
    int calls = 0;
    Check(history.Undo([&](const int& target, const int& expected)
    {
        ++calls;
        Check(target == 10 && expected == 20, "undo passes before target and after expectation");
        return true;
    }), "two-argument undo callback accepted");
    Check(history.Redo([&](const int& target, const int& expected)
    {
        ++calls;
        Check(target == 20 && expected == 10, "redo passes after target and before expectation");
        return true;
    }), "two-argument redo callback accepted");
    Check(calls == 2, "two-argument callback invoked once per restore");
}

void CrossOwnerConflict()
{
    struct Snapshot
    {
        std::string document, world, selection;
        bool operator==(const Snapshot&) const = default;
    };
    Client::CEditorUndoHistory<Snapshot> history;
    std::string document = "pattern.0", world = "A", selection = "world.box";
    const auto capture = [&] { return Snapshot{document, world, selection}; };
    const auto equal = [](const Snapshot& a, const Snapshot& b) { return a == b; };
    const auto apply = [&](const Snapshot& target, const Snapshot& expected)
    {
        // Validate every owner changed by this operation before committing any.
        if (document != expected.document) return false;
        const bool changesWorld = target.world != expected.world;
        if (changesWorld && world != expected.world) return false;
        document = target.document;
        if (changesWorld) world = target.world;
        selection = target.selection;
        return true;
    };

    history.Begin(capture());
    world = "B";
    Check(history.Commit(capture(), equal), "workbench A to B world edit recorded");
    world = "C"; // An independent World owner commits after that edit.
    history.Begin(capture());
    document = "pattern.1"; selection = "effect.row";
    Check(history.Commit(capture(), equal), "local edit after external world edit recorded");
    Check(history.Undo(apply) && document == "pattern.0" && world == "C" && selection == "world.box",
        "local-only undo preserves independently edited World C");

    const auto beforeConflict = capture();
    Check(!history.Undo(apply), "old world undo rejects expected B versus current C");
    Check(capture() == beforeConflict, "cross-owner conflict preserves all documents and selection");
    Check(history.Count_Undo() == 1 && history.Count_Redo() == 1,
        "cross-owner conflict preserves both stack positions");
    Check(history.Redo(apply) && document == "pattern.1" && world == "C",
        "local-only redo remains usable after rejected world undo");
    Check(history.Undo(apply) && document == "pattern.0" && world == "C",
        "local-only undo remains retryable after cross-owner conflict");

    world = "B"; // The independent owner explicitly restores the expected state.
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
    // Independent cursor model compares long mixed edit/undo/redo/failure paths.
    constexpr std::size_t limit = 8;
    Client::CEditorUndoHistory<int> history(limit);
    std::vector<int> states{0};
    std::size_t cursor = 0;
    int current = 0;
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
                states.resize(cursor + 1);
                states.push_back(next);
                ++cursor;
                if (states.size() > limit + 1) { states.erase(states.begin()); --cursor; }
            }
        }
        else if (operation < 5)
        {
            const bool expected = cursor > 0 && !reject;
            const bool actual = history.Undo([&](int next)
            { if (reject) return false; current = next; return true; });
            if (expected) --cursor;
            Check(actual == expected, "model undo result");
        }
        else
        {
            const bool expected = cursor + 1 < states.size() && !reject;
            const bool actual = history.Redo([&](int next)
            { if (reject) return false; current = next; return true; });
            if (expected) ++cursor;
            Check(actual == expected, "model redo result");
        }
        Check(current == states[cursor], "model restored value");
        Check(history.Count_Undo() == cursor && history.Count_Redo() == states.size() - cursor - 1,
            "model history position");
    }
}
}

int main()
{
    OwnerTransactions(); BoundedHistory(); ExpectedStateArguments(); CrossOwnerConflict(); ModelSequence();
    std::cout << "EditorUndoHistory: " << checks << " checks, " << failures << " failures\n";
    return failures ? 1 : 0;
}
