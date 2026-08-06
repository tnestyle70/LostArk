# Bern Navigation 바닥 고정 Bounds / Authoring Bootstrap 계획

## G0. 목표와 완료 경계

- MapTool의 Nav Bounds 세로 편집을 `Bottom Y + Height` 계약으로 바꿔 높이를 늘려도 바닥이 움직이지 않게 한다.
- `Place Nav Bounds`로 선택한 렌더 표면을 Bottom Y로 유지한다.
- Bern Area에 `Data/Navigation/LV_BER_BERNCASTLE.navsource/.navpaint` authoring bootstrap을 허용한다.
- 이번 변경에서는 Bern Server runtime `.navgrid`를 publish하거나 Server room에 강제로 연결하지 않는다. MapTool에서 실제 범위를 확인하고 bake/paint한 뒤 별도 검증 단위에서 제품 runtime을 활성화한다.
- 기존 Valtan/Character Select source·paint·blocker 계약과 파일은 변경하지 않는다.

## G1. 조사된 현재 상태

- `CMapTool::Try_PlaceNavigationBounds`는 최초 배치 시 `position.y = picked.y + size.y * 0.5`로 Bottom Y를 선택 표면에 맞춘다.
- 현재 UI의 `DragFloat3("Size")`는 중심을 유지한 채 Size Y를 변경하므로 최초 Bottom Y 고정이 풀린다.
- `CNavGridBaker::Build`는 `position.y ± size.y * 0.5` 범위의 triangle surface만 채택한다.
- grid cell 수는 X/Z와 Cell Size로 결정된다. Y 제한은 잘못된 상·하부 표면을 제외하지만 dense grid 행 수는 줄이지 않는다.
- Bern은 MapCatalog와 ProjectAudit에서 navigation disabled로 고정되어 있고 source 파일이 없다.
- source/paint-only Area는 blocker path가 비어 있다. 기존 Bake rollback은 빈 blocker path도 파일처럼 처리해 bootstrap layout reset이 실패할 수 있다.

## G2. 구현

### G2-1. MapTool Area policy

- Bern을 `EDITOR_NAVIGATION_POLICY::SOURCE_PAINT` 대상으로 추가한다.
- Bern에 한해서 source 부재를 정상 bootstrap 상태로 허용한다.
- MapCatalog Bern row에 다음 authoring path만 추가한다.

```json
"navigationSource": "Data/Navigation/LV_BER_BERNCASTLE.navsource",
"navigationPaint": "Data/Navigation/LV_BER_BERNCASTLE.navpaint"
```

### G2-2. 바닥 고정 Nav Bounds UI

기존 Position/Size 3축 직접 편집 대신 다음 의미를 표시한다.

```text
Position XZ
Bottom Y
Size XZ
Height from Bottom
Top Y (계산 표시)
Yaw
```

편집 결과는 기존 `NAVGRID_BAKE_DESC` 형식을 그대로 사용한다.

```cpp
position.y = bottomY + height * 0.5f;
size.y = height;
```

따라서 `.navsource` 형식과 기존 Valtan 데이터 호환성은 바뀌지 않는다.

### G2-3. 빈 blocker rollback

- backup, reset, restore는 path가 비어 있지 않은 파일만 처리한다.
- Bern과 Character Select처럼 source/paint-only인 Area는 blocker 파일을 생성하거나 삭제하지 않는다.
- Valtan의 non-empty blocker path 처리는 기존대로 유지한다.

### G2-4. Area 카메라 재포커스

- Area 전환 commit 뒤 제품 Area의 stable `playerSpawn`을 기준으로 authoring camera를 이동한다.
- 현재 선택된 Bern을 다시 눌러도 무시하지 않고 카메라를 다시 베른 진입점으로 이동한다.
- Area bar에 `Focus Area`를 제공해 전환을 다시 실행하지 않고 현재 Area만 재포커스한다.
- weak camera reference가 만료됐으면 기존 `Layer_Camera`의 `CCamera_Free`를 다시 찾고, 실패 이유를 UI 상태에 남긴다.

## G3. 변경 파일

- `Client/Private/MapTool.cpp`
- `Client/Public/MapTool.h`
- `Data/Maps/MapCatalog.json`
- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`
- `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`
- 대응 RESULT

새 C++ 파일과 public header를 추가하지 않으므로 `.vcxproj/.filters` 변경은 없다.

## G4. 실패와 rollback

- Bern source가 아직 없을 때 Area 전환은 성공하고 Bake mode를 제공한다.
- Bake 실패 시 source/paint 기존 파일은 유지한다.
- 잘못된 Bottom/Height, 1,000,000 cell 초과, 겹치는 static mesh 없음은 기존 fail-closed 검증을 유지한다.
- 실제 Bern source가 생성되기 전에는 publisher/Server runtime 계약을 활성화하지 않는다.

## G5. 검증

1. MapCatalog JSON parse와 ProjectAudit의 Bern authoring policy 검사.
2. Client x64 Debug build.
3. MapTool에서 Bern 선택 후 source 부재 상태가 bootstrap으로 표시되는지 확인.
4. 바닥 pick 후 Height 변경 전후 Bottom Y가 동일한지 확인.
5. Bake 후 `Data/Navigation/LV_BER_BERNCASTLE.navsource`가 생성되고 reload되는지 확인.
6. Valtan navigation source/paint/blocker hash가 바뀌지 않았는지 확인.
7. `git diff --check`와 ProjectAudit.
8. 현재 Bern을 다시 선택하거나 `Focus Area`를 눌렀을 때 `player.spawn.bern.entry`로 카메라가 이동하는지 확인.

실제 Bern 범위, resolved/walkable cell 통계와 수동 화면 검증은 bake 후 RESULT에 기록한다.
