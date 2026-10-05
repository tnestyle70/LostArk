# Placement Undo/Redo 계약 검사

`Run-MapPlacementHistoryContracts.ps1`은 현재 `MapPlacementEditSession.cpp`에서
실제 history 메서드를 추출해 실패를 주입할 수 있는 최소 host/runtime 대역과 함께 컴파일한다.
Client, D3D, ImGui 창을 실행하지 않는다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/WorldLevelHierarchy/Run-MapPlacementHistoryContracts.ps1 -Configuration Debug
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/WorldLevelHierarchy/Run-MapPlacementHistoryContracts.ps1 -Configuration Release
```

검사 범위는 TRS delta의 Undo/Redo, 선택 stable ID 복원, drag 합치기, 64회 제한,
Save revision 복귀와 새 분기에서 Redo 제거, transform 실패, 현재 asset/pose 불일치,
owner 해제·게시 중 거절, duplicate 삭제/복구와 생성 실패 후 재시도다.
결과와 소스 SHA-256은 `out/MapPlacementHistoryContracts/<Configuration>`에 기록한다.

실제 runtime 함수의 GPU/Layer 부작용과 ImGui 입력은 이 대역 검사의 범위가 아니다.
실제 Client 컴파일과 사용자의 화면 조작 확인을 별도로 기록한다.

`Run-WorldObjectProvenanceContracts.ps1`은 실제 Object Save의
`SynchronizeEmissionReferences`와 `CWorldSequenceDocument` 검색 메서드를 추출하고
제품 struct를 그대로 사용한다. emission 복원 시 연결된 Collider가 없는 경우의 허용,
사라진 저장 provenance가 있는 Collider의 안전 거절, 기존 행 보존, 정상 복제,
모호한 World occurrence 거절을 검사한다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File Tools/WorldLevelHierarchy/Run-WorldObjectProvenanceContracts.ps1
```
