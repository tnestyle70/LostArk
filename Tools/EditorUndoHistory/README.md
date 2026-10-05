# Editor Undo/Redo value-history contracts

`EditorUndoHistoryContracts.cpp` compiles the actual
`Client/Public/EditorUndoHistory.h` with the installed MSVC toolchain. It checks
deleted values and stable selections, saved-baseline isolation, rejected restore
retry, gesture coalescing, redo invalidation, capacity, and 5,000 deterministic
mixed operations against an independent cursor model.
It also checks both callback signatures, exact target/expected-state argument
ordering, and a shared World owner changing independently between local edits.
An old World undo must reject that conflict without changing either history
stack, while a local-only undo must preserve the external World edit.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/EditorUndoHistory/Run-EditorUndoHistoryContracts.ps1 -Configuration Debug
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/EditorUndoHistory/Run-EditorUndoHistoryContracts.ps1 -Configuration Release
```

Logs, executable and source-hash receipt are written only under
`out/EditorUndoHistory`. The probe does not start Client, publish data, or build
Product binaries. Tool-specific restore validation and previews remain the
responsibility of each document owner; these tests do not claim UI or rendering
verification.
