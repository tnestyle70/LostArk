# LostArk MapTool Navigation UI 단순화 구현 계획

> 상태: 2026-07-31 실행 비평으로 이 문서의 기본 화면·no-surface 계약은
> `2026-07-31_LOSTARK_NAVBOUNDS_PICK_BAKE_COMPLETION_PLAN.md` Section 8에 의해 대체됐다.
> 현재 구현 판단에는 Section 8을 사용한다.

## 문서 옵션

```text
C1~C8: OFF
문제 해결 ①~⑤: OFF
자료구조·알고리즘: OFF
```

이 문서는 현재 코드의 수정 범위, 최종 반영 코드, 프로젝트 등록, 적용·검증만 작성한다.

## 1. 변경 범위

- 높이가 없는 cell은 자동 nonwalkable로 export한다.
- 높이가 없는 cell은 MapTool 기본 overlay에서 그리지 않고 paint도 거부한다.
- Walkability와 Destruction Area control을 동시에 보여 주지 않는다.
- 기본 화면에서 count, 절대 경로, reload, condition preview를 제거한다.
- `Save Paint`, `Save Runtime Regions`, `Export Runtime`을 `Save Navigation` 하나로 합친다.
- F5와 MapTool에서 초록은 walkable, 노랑은 nonwalkable이라는 의미를 유지한다.
- 새 runtime class, 새 저장 포맷, 새 C++ 파일은 만들지 않는다.

## 2. 수정 파일

| 구분 | 절대 경로 | 수정 내용 |
|---|---|---|
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/NavGridPaintDocument.h` | `NO_SURFACE` 상태와 높이 조회 계약으로 축소 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/NavGridPaintDocument.cpp` | no-surface 자동 차단, paint 거부, export 단순화 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/MapTool.h` | Navigation mode, edit action, 단일 Save와 접이 UI 함수 선언 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MapTool.cpp` | 최소 panel, mode별 control, no-surface 숨김, 단일 Save |

### 2.1 Engine/Client 배치와 의존성

| 파일/클래스 | 위치 | 이유 | 직접 의존 | 의존하면 안 되는 것 |
|---|---|---|---|---|
| `CNavGrid` | `Engine` | walkability와 runtime blocker를 계산하는 범용 runtime 자료 | Engine 기본 타입 | ImGui, MapTool, LostArk asset ID |
| `CNavigation` | `Engine` | Client actor가 사용하는 Navigation façade | `CNavGrid`, `CPathFinder` | `.navpaint`, editor panel |
| `CNavGridPaintDocument` | `Client` | LostArk MapTool의 `.navsource/.navpaint` authoring 문서 | Client/Engine 기본 타입, filesystem | ImGui, GameInstance, runtime actor |
| `CNavRuntimeBlockerDocument` | `Client` | LostArk 파괴 region authoring 문서 | Client 기본 타입, filesystem | A*, Transform, render |
| `CMapTool` | `Client` | ImGui 선택·피킹·저장 command 전달 | 두 authoring document, `CNavigation` preview | bake 알고리즘, A* 내부 상태 |

의존 방향은 다음 하나로 유지한다.

```text
CMapTool
  -> CNavGridPaintDocument
  -> CNavRuntimeBlockerDocument
  -> CNavigation public façade

CNavigation
  -> CNavGrid
  -> CPathFinder
```

`CNavGridPaintDocument`는 이미 존재하는 Client authoring 파일이다. 이번 수정에서 새로
추가하는 파일이 아니며 Engine으로 이동하지 않는다. `.navsource/.navpaint`는 LostArk
MapTool의 저장 계약이고 Engine runtime은 최종 `.navgrid`만 소비하기 때문이다.

## 3. 최종 ImGui 구성

### 3.1 전체 화면

```text
┌────────────────────────────── Valtan WModel Asset Test ───────────────────────────────┐
│                                                                                       │
│                               World Viewport                                           │
│                                                                                       │
│          ┌─┐ ┌─┐ ┌─┐  Green: Walkable                                                │
│          └─┘ └─┘ └─┘  Yellow: Blocked                                                 │
│                                                                                       │
│          높이가 없는 맵 외곽 cell은 그리지 않는다.                                    │
└───────────────────────────────────────────────────────────────────────────────────────┘

┌────────────────────────────── LostArk Map Tool ───────────────────────────────────────┐
│  Map Assets   ● Navigation                                                            │
│───────────────────────────────────────────────────────────────────────────────────────│
│  Mode     ● Walkability    ○ Destruction Area                                         │
│                                                                                       │
│  Tool     ● Block          ○ Erase                                                     │
│  Brush    [────────●────────]  1                                                       │
│                                                                                       │
│  [ Save Navigation ]   Unsaved                                                        │
│                                                                                       │
│  Green: Walkable    Yellow: Blocked                                                   │
│                                                                                       │
│  ▶ Diagnostics                                                                       │
└───────────────────────────────────────────────────────────────────────────────────────┘
```

### 3.2 Walkability mode

```text
Navigation

Mode     [Walkability] [Destruction Area]

Tool     [Block] [Erase]
Brush    [ 0 ]

[Save Navigation]                       Saved / Unsaved

Cell 0.50 x 0.50 | Green: Walkable | Yellow: Blocked
▶ Diagnostics
```

이 화면에서는 정적 cell 편집에 필요하지 않은 region, blocker ID, condition, 파일 경로를
표시하지 않는다.

### 3.3 Destruction Area mode

```text
Navigation

Mode     [Walkability] [Destruction Area]

Region   [VALTAN_OUTER_RING_COLLAPSE ▼] [New]
Tool     [Add Cells] [Remove Cells]
Brush    [ 0 ]

[Save Navigation]                       Saved / Unsaved

Magenta: Selected destruction area
▶ Test
▶ Diagnostics
```

`New`를 눌렀을 때만 다음 popup을 표시한다.

```text
┌──────────── New Destruction Area ────────────┐
│ Name       [VALTAN_OUTER_RING_COLLAPSE     ] │
│                                              │
│ Behavior   ● Block after destruction         │
│            ○ Open after destruction          │
│                                              │
│ ▶ Advanced                                  │
│                                              │
│ [Create] [Cancel]                            │
└──────────────────────────────────────────────┘
```

`Advanced`를 펼쳤을 때만 condition ID를 표시한다.

```text
▼ Advanced
Condition  [VALTAN_ARENA_DESTROYED]
```

### 3.4 Diagnostics

기본값은 닫힘이다.

```text
▼ Diagnostics
Surface cells: 2897
Excluded cells: 1009
Blocked cells: 12

Source:   .../ValtanArena.navsource
Paint:    .../ValtanArena.navpaint
Runtime:  .../ValtanArena.navgrid
Blockers: .../ValtanArena.navblockers

[Reload from Disk]
```

## 4. 최종 반영 코드

### 4.1 `Client/Public/NavGridPaintDocument.h`

파일 전체를 다음으로 교체한다.

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

enum class NAVGRID_AUTHORING_CELL_STATE : uint8_t
{
	WALKABLE,
	BLOCKED,
	NO_SURFACE,
};

struct NAVGRID_AUTHORING_DESC final
{
	std::string areaId;
	uint32_t width = {};
	uint32_t height = {};
	f32_t cellSize = {};
	f32_t originX = {};
	f32_t originZ = {};
};

struct NAV_SOURCE_CELL final
{
	bool_t heightResolved = false;
	f32_t height = {};
};

class CNavGridPaintDocument final
{
public:
	static constexpr uint32_t MAX_CELL_COUNT = 1000000;
	static constexpr uint32_t MAX_BRUSH_RADIUS = 8;

public:
	bool_t Load(
		const std::filesystem::path& sourcePath,
		const std::filesystem::path& paintPath,
		std::string& outStatus);
	bool_t Paint(
		int32_t cellX,
		int32_t cellZ,
		uint32_t brushRadius,
		bool_t walkable);
	bool_t Save_Paint(
		const std::filesystem::path& paintPath,
		std::string& outStatus);
	bool_t Export_Runtime(
		const std::filesystem::path& runtimePath,
		std::string& outStatus) const;

public:
	bool_t Is_Ready() const { return m_isReady; }
	bool_t Is_Dirty() const { return m_isDirty; }
	bool_t Is_ValidCell(int32_t cellX, int32_t cellZ) const;
	bool_t World_ToCell(
		fvector_t worldPosition,
		int32_t& outCellX,
		int32_t& outCellZ) const;
	uint32_t To_Index(int32_t cellX, int32_t cellZ) const;
	NAVGRID_AUTHORING_CELL_STATE Get_CellState(uint32_t index) const;
	bool_t Has_ResolvedHeight(uint32_t index) const;
	f32_t Get_CellHeight(uint32_t index) const;
	uint32_t Get_CellCount() const;
	uint32_t Get_BlockedCount() const;
	uint32_t Get_ResolvedHeightCount() const;
	const NAVGRID_AUTHORING_DESC& Get_Desc() const { return m_Desc; }

private:
	NAVGRID_AUTHORING_DESC m_Desc;
	std::vector<NAV_SOURCE_CELL> m_SourceCells;
	std::vector<uint8_t> m_BlockedCells;
	bool_t m_isReady = false;
	bool_t m_isDirty = false;
};

NS_END
```

### 4.2 `Client/Private/NavGridPaintDocument.cpp`

`#include <limits>`를 삭제한다.

`Load()`의 `Build_DisplayHeights()` 호출부터 commit까지를 다음으로 교체한다.

```cpp
	for (uint32_t index = 0; index < cellCount; ++index)
	{
		if (!stagedSourceCells[index].heightResolved)
			stagedBlockedCells[index] = 0;
	}

	const bool_t hasResolvedHeight = std::any_of(
		stagedSourceCells.begin(),
		stagedSourceCells.end(),
		[](const NAV_SOURCE_CELL& cell)
		{
			return cell.heightResolved;
		});
	if (!hasResolvedHeight)
	{
		outStatus = "NavGrid source has no usable height";
		return false;
	}

	m_Desc = std::move(stagedDesc);
	m_SourceCells = std::move(stagedSourceCells);
	m_BlockedCells = std::move(stagedBlockedCells);
	m_isReady = true;
	m_isDirty = false;
	outStatus = paintExists
		? "Loaded navigation"
		: "Loaded navigation; surface cells start walkable";
	return true;
```

`Paint()` 전체를 다음으로 교체한다.

```cpp
bool_t Client::CNavGridPaintDocument::Paint(
	int32_t cellX,
	int32_t cellZ,
	uint32_t brushRadius,
	bool_t walkable)
{
	if (!m_isReady ||
		brushRadius > MAX_BRUSH_RADIUS ||
		!Is_ValidCell(cellX, cellZ))
	{
		return false;
	}

	const int32_t radius = static_cast<int32_t>(brushRadius);
	const int32_t radiusSquared = radius * radius;
	bool_t changed = false;
	for (int32_t offsetZ = -radius; offsetZ <= radius; ++offsetZ)
	{
		for (int32_t offsetX = -radius; offsetX <= radius; ++offsetX)
		{
			if (offsetX * offsetX + offsetZ * offsetZ > radiusSquared)
				continue;

			const int32_t targetX = cellX + offsetX;
			const int32_t targetZ = cellZ + offsetZ;
			if (!Is_ValidCell(targetX, targetZ))
				continue;

			const uint32_t index = To_Index(targetX, targetZ);
			if (!Has_ResolvedHeight(index))
				continue;

			const uint8_t newValue = walkable ? 0 : 1;
			if (m_BlockedCells[index] == newValue)
				continue;

			m_BlockedCells[index] = newValue;
			changed = true;
		}
	}

	if (changed)
		m_isDirty = true;
	return changed;
}
```

`Export_Runtime()` 전체를 다음으로 교체한다.

```cpp
bool_t Client::CNavGridPaintDocument::Export_Runtime(
	const std::filesystem::path& runtimePath,
	std::string& outStatus) const
{
	if (!m_isReady)
	{
		outStatus = "Navigation is not loaded";
		return false;
	}

	std::error_code directoryError;
	std::filesystem::create_directories(
		runtimePath.parent_path(),
		directoryError);
	if (directoryError)
	{
		outStatus = "Could not create navigation runtime directory";
		return false;
	}

	const uint32_t cellCount = Get_CellCount();
	uint32_t walkableCount = {};
	std::vector<uint8_t> walkable(cellCount, 0);
	std::vector<f32_t> heights(cellCount, 0.f);
	for (uint32_t index = 0; index < cellCount; ++index)
	{
		const bool_t hasSurface = m_SourceCells[index].heightResolved;
		const bool_t isWalkable =
			hasSurface && 0 == m_BlockedCells[index];
		walkable[index] = isWalkable ? 1 : 0;
		heights[index] =
			hasSurface ? m_SourceCells[index].height : 0.f;
		if (isWalkable)
			++walkableCount;
	}

	std::filesystem::path temporary = runtimePath;
	temporary += L".tmp";
	std::ofstream output(
		temporary,
		std::ios::binary | std::ios::trunc);
	if (!output)
	{
		outStatus = "Could not create temporary runtime navigation";
		return false;
	}

	output.write(
		reinterpret_cast<const char*>(&m_Desc.width),
		sizeof(uint32_t));
	output.write(
		reinterpret_cast<const char*>(&m_Desc.height),
		sizeof(uint32_t));
	output.write(
		reinterpret_cast<const char*>(&m_Desc.cellSize),
		sizeof(f32_t));
	output.write(
		reinterpret_cast<const char*>(&m_Desc.originX),
		sizeof(f32_t));
	output.write(
		reinterpret_cast<const char*>(&m_Desc.originZ),
		sizeof(f32_t));
	output.write(
		reinterpret_cast<const char*>(walkable.data()),
		static_cast<std::streamsize>(walkable.size()));
	output.write(
		reinterpret_cast<const char*>(heights.data()),
		static_cast<std::streamsize>(heights.size() * sizeof(f32_t)));
	output.flush();

	const bool_t wroteSuccessfully = output.good();
	output.close();
	if (!wroteSuccessfully ||
		!CommitTemporaryFile(runtimePath, temporary))
	{
		RemoveTemporaryFile(temporary);
		outStatus = "Failed to commit runtime navigation";
		return false;
	}

	outStatus =
		"Exported navigation: " +
		std::to_string(walkableCount) +
		" walkable cells";
	return true;
}
```

`Get_CellState()`부터 파일 끝까지를 다음으로 교체한다.

```cpp
NAVGRID_AUTHORING_CELL_STATE
Client::CNavGridPaintDocument::Get_CellState(uint32_t index) const
{
	if (!Has_ResolvedHeight(index))
		return NAVGRID_AUTHORING_CELL_STATE::NO_SURFACE;
	if (0 != m_BlockedCells[index])
		return NAVGRID_AUTHORING_CELL_STATE::BLOCKED;
	return NAVGRID_AUTHORING_CELL_STATE::WALKABLE;
}

bool_t Client::CNavGridPaintDocument::Has_ResolvedHeight(
	uint32_t index) const
{
	return m_isReady &&
		index < m_SourceCells.size() &&
		m_SourceCells[index].heightResolved;
}

f32_t Client::CNavGridPaintDocument::Get_CellHeight(
	uint32_t index) const
{
	return Has_ResolvedHeight(index) ?
		m_SourceCells[index].height :
		0.f;
}

uint32_t Client::CNavGridPaintDocument::Get_CellCount() const
{
	return static_cast<uint32_t>(m_SourceCells.size());
}

uint32_t Client::CNavGridPaintDocument::Get_BlockedCount() const
{
	uint32_t count = {};
	for (uint32_t index = 0; index < m_SourceCells.size(); ++index)
	{
		if (m_SourceCells[index].heightResolved &&
			0 != m_BlockedCells[index])
		{
			++count;
		}
	}
	return count;
}

uint32_t Client::CNavGridPaintDocument::Get_ResolvedHeightCount() const
{
	return static_cast<uint32_t>(std::count_if(
		m_SourceCells.begin(),
		m_SourceCells.end(),
		[](const NAV_SOURCE_CELL& cell)
		{
			return cell.heightResolved;
		}));
}
```

기존 `Get_DisplayHeight()`, `Get_UnresolvedWalkableCount()`,
`Build_DisplayHeights()` 정의는 삭제한다.

### 4.3 `Client/Public/MapTool.h`

기존 `PAINT_MODE`, `NAVIGATION_PAINT_TARGET` 선언을 다음으로 교체한다.

```cpp
	enum class NAVIGATION_MODE
	{
		WALKABILITY,
		DESTRUCTION_AREA,
	};

	enum class NAVIGATION_EDIT_ACTION
	{
		APPLY,
		ERASE,
	};
```

Navigation private 함수 선언을 다음으로 교체한다.

```cpp
	bool_t Load_NavigationDocument();
	bool_t Load_RuntimeBlockers();
	bool_t Register_RuntimeBlockers();
	bool_t Set_NavigationCondition(
		const std::string& conditionId,
		bool_t value);
	bool_t Save_Navigation();
	bool_t Try_PickNavigationCell(
		int32_t& outCellX,
		int32_t& outCellZ) const;
	bool_t Try_PaintNavigation();
	void Render_ModeBar();
	void Render_NavigationPanel();
	void Render_DestructionAreaControls();
	void Render_NavigationDiagnostics();
	void Render_NavigationOverlay();
```

기존 mouse 입력 member에는 Navigation stroke 상태를 바로 뒤에 추가한다.

```cpp
	bool_t m_bPreviousMouseDown = false;
	bool_t m_bNavigationStrokeActive = false;
```

Navigation member block을 다음으로 교체한다.

```cpp
	TOOL_MODE m_eToolMode = TOOL_MODE::MAP_ASSETS;
	NAVIGATION_MODE m_eNavigationMode =
		NAVIGATION_MODE::WALKABILITY;
	NAVIGATION_EDIT_ACTION m_eNavigationEditAction =
		NAVIGATION_EDIT_ACTION::APPLY;
	uint32_t m_iBrushRadius = {};
	CNavGridPaintDocument m_NavigationDocument;
	CNavRuntimeBlockerDocument m_RuntimeBlockerDocument;
	std::filesystem::path m_NavigationSourcePath;
	std::filesystem::path m_NavigationPaintPath;
	std::filesystem::path m_NavigationRuntimePath;
	std::filesystem::path m_RuntimeBlockerPath;
	std::string m_NavigationStatus = "Open ASSET_TEST with F2";
	size_t m_iSelectedRuntimeRegion = {};
	char m_RuntimeBlockerId[128] =
		"VALTAN_OUTER_RING_COLLAPSE";
	char m_RuntimeConditionId[128] =
		"VALTAN_ARENA_DESTROYED";
	bool_t m_RuntimeActivateWhenConditionTrue = true;
	std::unordered_map<std::string, bool_t> m_NavigationConditions;
	std::unique_ptr<NAVIGATION_RENDER_RESOURCES>
		m_pNavigationRenderResources;
```

### 4.4 `Client/Private/MapTool.cpp`

`Update()`의 Navigation 입력은 LMB가 월드에서 시작된 동안에만 paint하도록 교체한다.
ImGui 버튼에서 시작한 drag, focus 이탈, tool/level 종료는 stroke를 취소한다.

```cpp
void Client::CMapTool::Update(f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	const bool_t isAssetTest =
		ETOUI(LEVEL::ASSET_TEST) == CGameInstance::Get().Get_CurrentLevelID();
	Handle_LevelTransition(isAssetTest);
	if (isAssetTest && GetForegroundWindow() == g_hWnd)
	{
		if (0 != (GetAsyncKeyState(VK_F7) & 1))
		{
			Set_EnvironmentPhase(ENVIRONMENT_PHASE::BASELINE);
			m_Status = "Sky phase: Baseline (F7)";
		}
		else if (0 != (GetAsyncKeyState(VK_F8) & 1))
		{
			Set_EnvironmentPhase(ENVIRONMENT_PHASE::SPACEHOLE);
			m_Status = "Sky phase: SpaceHole (F8)";
		}
		else if (0 != (GetAsyncKeyState(VK_F9) & 1))
		{
			Set_EnvironmentPhase(ENVIRONMENT_PHASE::CHAOS_GATE);
			m_Status = "Sky phase: ChaosGate (F9)";
		}
	}

	const bool_t mouseDown =
		0 != (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
	const bool_t mousePressed = mouseDown && !m_bPreviousMouseDown;
	m_bPreviousMouseDown = mouseDown;

	if (!m_bOpen || !isAssetTest)
	{
		m_bNavigationStrokeActive = false;
		return;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
	{
		m_ePlacementState = PLACEMENT_STATE::IDLE;
		m_Status = "Placement cancelled";
	}

	const bool_t canUseWorldMouse =
		GetForegroundWindow() == g_hWnd &&
		!ImGui::GetIO().WantCaptureMouse;

	if (TOOL_MODE::NAVIGATION == m_eToolMode)
	{
		if (!mouseDown || !canUseWorldMouse)
		{
			m_bNavigationStrokeActive = false;
			return;
		}

		if (mousePressed)
			m_bNavigationStrokeActive = true;

		if (m_bNavigationStrokeActive)
			Try_PaintNavigation();
		return;
	}

	if (!canUseWorldMouse)
		return;

	if (PLACEMENT_STATE::ARMED == m_ePlacementState && mousePressed)
		Try_PlaceSelected();
}
```

`Load_NavigationDocument()` 전체를 다음으로 교체한다.

```cpp
bool_t Client::CMapTool::Load_NavigationDocument()
{
	const std::filesystem::path root = GetNavigationDataRoot();
	if (root.empty())
	{
		m_NavigationStatus = "Navigation data is unavailable";
		return false;
	}

	m_NavigationSourcePath = root / L"ValtanArena.navsource";
	m_NavigationPaintPath = root / L"ValtanArena.navpaint";
	m_NavigationRuntimePath = root / L"ValtanArena.navgrid";
	m_RuntimeBlockerPath = root / L"ValtanArena.navblockers";

	if (!m_NavigationDocument.Load(
		m_NavigationSourcePath,
		m_NavigationPaintPath,
		m_NavigationStatus))
	{
		return false;
	}

	if (!Load_RuntimeBlockers())
		return false;

	m_NavigationStatus = "Saved";
	return true;
}
```

`Save_Navigation()`을 `Set_NavigationCondition()` 다음에 추가한다.

```cpp
bool_t Client::CMapTool::Save_Navigation()
{
	if (!m_NavigationDocument.Is_Ready() ||
		!m_RuntimeBlockerDocument.Is_Ready())
	{
		m_NavigationStatus = "Navigation is not loaded";
		return false;
	}

	std::string status;
	if (!m_NavigationDocument.Save_Paint(
		m_NavigationPaintPath,
		status))
	{
		m_NavigationStatus = status;
		return false;
	}

	if (!m_RuntimeBlockerDocument.Save(
		m_RuntimeBlockerPath,
		status))
	{
		m_NavigationStatus = status;
		return false;
	}

	if (!m_NavigationDocument.Export_Runtime(
		m_NavigationRuntimePath,
		status))
	{
		m_NavigationStatus = status;
		return false;
	}

	m_NavigationStatus =
		"Saved. Re-enter ASSET_TEST to reload runtime navigation.";
	return true;
}
```

`Try_PickNavigationCell()` 전체를 다음으로 교체한다.

```cpp
bool_t Client::CMapTool::Try_PickNavigationCell(
	int32_t& outCellX,
	int32_t& outCellZ) const
{
	if (!m_NavigationDocument.Is_Ready())
		return false;

	float4_t picked{};
	if (!CGameInstance::Get().Picking(picked) ||
		!m_NavigationDocument.World_ToCell(
			XMLoadFloat4(&picked),
			outCellX,
			outCellZ))
	{
		return false;
	}

	return m_NavigationDocument.Has_ResolvedHeight(
		m_NavigationDocument.To_Index(
			outCellX,
			outCellZ));
}
```

`Try_PaintNavigation()` 전체를 다음으로 교체한다.

```cpp
bool_t Client::CMapTool::Try_PaintNavigation()
{
	int32_t cellX = {};
	int32_t cellZ = {};
	if (!Try_PickNavigationCell(cellX, cellZ))
		return false;

	const bool_t erase =
		NAVIGATION_EDIT_ACTION::ERASE ==
		m_eNavigationEditAction;

	bool_t changed = false;
	if (NAVIGATION_MODE::DESTRUCTION_AREA == m_eNavigationMode)
	{
		changed = m_RuntimeBlockerDocument.Paint(
			m_iSelectedRuntimeRegion,
			cellX,
			cellZ,
			m_iBrushRadius,
			!erase);
	}
	else
	{
		changed = m_NavigationDocument.Paint(
			cellX,
			cellZ,
			m_iBrushRadius,
			erase);
	}

	if (changed)
		m_NavigationStatus = "Unsaved";
	return changed;
}
```

`Render()` 전체를 다음으로 교체한다.

```cpp
void Client::CMapTool::Render()
{
	if (!m_bOpen)
		return;

	const bool_t isAssetTest =
		ETOUI(LEVEL::ASSET_TEST) ==
		CGameInstance::Get().Get_CurrentLevelID();
	if (isAssetTest && TOOL_MODE::NAVIGATION == m_eToolMode)
		Render_NavigationOverlay();

	ImGui::SetNextWindowSize(
		ImVec2(1180.f, 900.f),
		ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("LostArk Map Tool", &m_bOpen))
	{
		ImGui::End();
		return;
	}

	Render_ModeBar();
	ImGui::Separator();

	if (TOOL_MODE::NAVIGATION == m_eToolMode)
	{
		ImGui::BeginDisabled(!isAssetTest);
		Render_NavigationPanel();
		ImGui::EndDisabled();
		ImGui::End();
		return;
	}

	ImGui::Text("Level: %s",
		isAssetTest ? "ASSET_TEST" : "Open ASSET_TEST with F2");
	ImGui::SameLine();
	ImGui::Text("| Catalog: %s",
		m_Catalog.Is_Ready() ? "READY" : "NOT READY");
	ImGui::TextWrapped("%s", m_Status.c_str());
	ImGui::Separator();

	ImGui::BeginDisabled(!isAssetTest || !m_Catalog.Is_Ready());
	Render_Toolbar();

	const f32_t availableHeight = ImGui::GetContentRegionAvail().y;
	const f32_t topPanelHeight = (std::max)(
		280.f,
		(std::min)(480.f, availableHeight * 0.48f));

	if (ImGui::BeginTable(
		"MapEditorColumns",
		3,
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_BordersInnerV))
	{
		ImGui::TableSetupColumn(
			"Palette",
			ImGuiTableColumnFlags_WidthStretch,
			0.38f);
		ImGui::TableSetupColumn(
			"Hierarchy",
			ImGuiTableColumnFlags_WidthStretch,
			0.27f);
		ImGui::TableSetupColumn(
			"Inspector",
			ImGuiTableColumnFlags_WidthStretch,
			0.35f);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		Render_Palette(topPanelHeight);
		ImGui::TableSetColumnIndex(1);
		Render_Hierarchy(topPanelHeight);
		ImGui::TableSetColumnIndex(2);
		Render_Inspector();
		ImGui::EndTable();
	}
	Render_AssetPreview();
	ImGui::EndDisabled();
	ImGui::End();
}
```

`Render_NavigationPanel()` 전체를 다음으로 교체한다.

```cpp
void Client::CMapTool::Render_NavigationPanel()
{
	if (!m_NavigationDocument.Is_Ready() ||
		!m_RuntimeBlockerDocument.Is_Ready())
	{
		ImGui::TextUnformatted("Navigation is unavailable.");
		if (ImGui::Button("Retry"))
			Load_NavigationDocument();
		ImGui::SameLine();
		ImGui::TextWrapped("%s", m_NavigationStatus.c_str());
		return;
	}

	if (ImGui::RadioButton(
		"Walkability",
		NAVIGATION_MODE::WALKABILITY == m_eNavigationMode))
	{
		m_eNavigationMode = NAVIGATION_MODE::WALKABILITY;
		m_eNavigationEditAction =
			NAVIGATION_EDIT_ACTION::APPLY;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"Destruction Area",
		NAVIGATION_MODE::DESTRUCTION_AREA ==
		m_eNavigationMode))
	{
		m_eNavigationMode =
			NAVIGATION_MODE::DESTRUCTION_AREA;
		m_eNavigationEditAction =
			NAVIGATION_EDIT_ACTION::APPLY;
	}

	ImGui::Separator();

	if (NAVIGATION_MODE::DESTRUCTION_AREA == m_eNavigationMode)
		Render_DestructionAreaControls();

	const char* applyLabel =
		NAVIGATION_MODE::DESTRUCTION_AREA == m_eNavigationMode ?
		"Add Cells" :
		"Block";
	const char* eraseLabel =
		NAVIGATION_MODE::DESTRUCTION_AREA == m_eNavigationMode ?
		"Remove Cells" :
		"Erase";

	if (ImGui::RadioButton(
		applyLabel,
		NAVIGATION_EDIT_ACTION::APPLY ==
		m_eNavigationEditAction))
	{
		m_eNavigationEditAction =
			NAVIGATION_EDIT_ACTION::APPLY;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		eraseLabel,
		NAVIGATION_EDIT_ACTION::ERASE ==
		m_eNavigationEditAction))
	{
		m_eNavigationEditAction =
			NAVIGATION_EDIT_ACTION::ERASE;
	}

	int32_t brushRadius =
		static_cast<int32_t>(m_iBrushRadius);
	if (ImGui::SliderInt(
		"Brush",
		&brushRadius,
		0,
		static_cast<int32_t>(
			CNavGridPaintDocument::MAX_BRUSH_RADIUS)))
	{
		m_iBrushRadius =
			static_cast<uint32_t>(brushRadius);
	}

	ImGui::Separator();
	if (ImGui::Button("Save Navigation"))
		Save_Navigation();
	ImGui::SameLine();

	const bool_t dirty =
		m_NavigationDocument.Is_Dirty() ||
		m_RuntimeBlockerDocument.Is_Dirty();
	ImGui::TextUnformatted(
		dirty ? "Unsaved" : m_NavigationStatus.c_str());

	if (NAVIGATION_MODE::DESTRUCTION_AREA == m_eNavigationMode)
	{
		ImGui::TextDisabled(
			"Magenta: selected destruction area");
	}
	else
	{
		ImGui::TextDisabled(
			"Cell %.2f x %.2f | Green: walkable | Yellow: blocked",
			m_NavigationDocument.Get_Desc().cellSize,
			m_NavigationDocument.Get_Desc().cellSize);
	}

	Render_NavigationDiagnostics();
}
```

`Render_DestructionAreaControls()`을 추가한다.

```cpp
void Client::CMapTool::Render_DestructionAreaControls()
{
	const NAV_RUNTIME_BLOCKER_REGION* selectedRegion =
		m_RuntimeBlockerDocument.Get_Region(
			m_iSelectedRuntimeRegion);
	const char* preview =
		nullptr != selectedRegion ?
		selectedRegion->id.c_str() :
		"<none>";

	ImGui::SetNextItemWidth(320.f);
	if (ImGui::BeginCombo("Region", preview))
	{
		for (size_t index = 0;
			index < m_RuntimeBlockerDocument.Get_RegionCount();
			++index)
		{
			const NAV_RUNTIME_BLOCKER_REGION* region =
				m_RuntimeBlockerDocument.Get_Region(index);
			if (nullptr == region)
				continue;

			const bool_t selected =
				index == m_iSelectedRuntimeRegion;
			if (ImGui::Selectable(
				region->id.c_str(),
				selected))
			{
				m_iSelectedRuntimeRegion = index;
			}
			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::SameLine();
	if (ImGui::Button("New"))
		ImGui::OpenPopup("New Destruction Area");

	if (ImGui::BeginPopupModal(
		"New Destruction Area",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::InputText(
			"Name",
			m_RuntimeBlockerId,
			sizeof(m_RuntimeBlockerId));

		if (ImGui::RadioButton(
			"Block after destruction",
			m_RuntimeActivateWhenConditionTrue))
		{
			m_RuntimeActivateWhenConditionTrue = true;
		}
		if (ImGui::RadioButton(
			"Open after destruction",
			!m_RuntimeActivateWhenConditionTrue))
		{
			m_RuntimeActivateWhenConditionTrue = false;
		}

		if (ImGui::CollapsingHeader("Advanced"))
		{
			ImGui::InputText(
				"Condition",
				m_RuntimeConditionId,
				sizeof(m_RuntimeConditionId));
		}

		const bool_t canCreate =
			'\0' != m_RuntimeBlockerId[0] &&
			'\0' != m_RuntimeConditionId[0];
		ImGui::BeginDisabled(!canCreate);
		if (ImGui::Button("Create") &&
			m_RuntimeBlockerDocument.Add_Region(
				m_RuntimeBlockerId,
				m_RuntimeConditionId,
				m_RuntimeActivateWhenConditionTrue,
				m_NavigationStatus))
		{
			m_iSelectedRuntimeRegion =
				m_RuntimeBlockerDocument.Get_RegionCount() - 1;
			m_NavigationStatus = "Unsaved";
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndDisabled();

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

	selectedRegion = m_RuntimeBlockerDocument.Get_Region(
		m_iSelectedRuntimeRegion);
	if (nullptr != selectedRegion &&
		ImGui::CollapsingHeader("Test"))
	{
		bool_t conditionValue =
			m_NavigationConditions[selectedRegion->conditionId];
		if (ImGui::Checkbox(
			"Destroyed",
			&conditionValue))
		{
			if (!Set_NavigationCondition(
				selectedRegion->conditionId,
				conditionValue))
			{
				m_NavigationStatus =
					"Re-enter ASSET_TEST before testing this region.";
			}
		}
	}
}
```

`Render_NavigationDiagnostics()`를 추가한다.

```cpp
void Client::CMapTool::Render_NavigationDiagnostics()
{
	if (!ImGui::CollapsingHeader("Diagnostics"))
		return;

	const uint32_t surfaceCells =
		m_NavigationDocument.Get_ResolvedHeightCount();
	ImGui::Text(
		"Surface: %u | Excluded: %u | Blocked: %u",
		surfaceCells,
		m_NavigationDocument.Get_CellCount() - surfaceCells,
		m_NavigationDocument.Get_BlockedCount());
	ImGui::TextWrapped(
		"Source: %s",
		m_NavigationSourcePath.string().c_str());
	ImGui::TextWrapped(
		"Paint: %s",
		m_NavigationPaintPath.string().c_str());
	ImGui::TextWrapped(
		"Runtime: %s",
		m_NavigationRuntimePath.string().c_str());
	ImGui::TextWrapped(
		"Blockers: %s",
		m_RuntimeBlockerPath.string().c_str());

	if (ImGui::Button("Reload from Disk"))
	{
		const bool_t dirty =
			m_NavigationDocument.Is_Dirty() ||
			m_RuntimeBlockerDocument.Is_Dirty();
		if (dirty)
			ImGui::OpenPopup("Discard Navigation Changes?");
		else
			Load_NavigationDocument();
	}

	if (ImGui::BeginPopupModal(
		"Discard Navigation Changes?",
		nullptr,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextUnformatted(
			"Reload and discard unsaved navigation changes?");
		if (ImGui::Button("Discard and Reload"))
		{
			Load_NavigationDocument();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
}
```

`Render_NavigationOverlay()` 전체를 다음으로 교체한다.

```cpp
void Client::CMapTool::Render_NavigationOverlay()
{
	if (!m_NavigationDocument.Is_Ready() ||
		nullptr == m_pContext ||
		nullptr == m_pNavigationRenderResources ||
		nullptr == m_pNavigationRenderResources->pBatch ||
		nullptr == m_pNavigationRenderResources->pEffect ||
		nullptr == m_pNavigationRenderResources->pInputLayout)
	{
		return;
	}

	auto& resources = *m_pNavigationRenderResources;
	resources.pEffect->SetWorld(XMMatrixIdentity());
	resources.pEffect->SetView(XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::VIEW)));
	resources.pEffect->SetProjection(XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::PROJ)));
	m_pContext->IASetInputLayout(
		resources.pInputLayout.Get());
	resources.pEffect->Apply(m_pContext.Get());

	const float4_t green(0.1f, 1.f, 0.2f, 1.f);
	const float4_t yellow(1.f, 0.85f, 0.05f, 1.f);
	const float4_t magenta(1.f, 0.15f, 0.85f, 1.f);
	const NAVGRID_AUTHORING_DESC& desc =
		m_NavigationDocument.Get_Desc();
	const f32_t halfCell = desc.cellSize * 0.5f;

	resources.pBatch->Begin();
	for (uint32_t index = 0;
		index < m_NavigationDocument.Get_CellCount();
		++index)
	{
		const NAVGRID_AUTHORING_CELL_STATE state =
			m_NavigationDocument.Get_CellState(index);
		if (NAVGRID_AUTHORING_CELL_STATE::NO_SURFACE == state)
			continue;

		const uint32_t cellX = index % desc.width;
		const uint32_t cellZ = index / desc.width;
		const float3_t center(
			desc.originX +
				(static_cast<f32_t>(cellX) + 0.5f) *
				desc.cellSize,
			m_NavigationDocument.Get_CellHeight(index) + 0.08f,
			desc.originZ +
				(static_cast<f32_t>(cellZ) + 0.5f) *
				desc.cellSize);

		const bool_t selectedDestructionCell =
			NAVIGATION_MODE::DESTRUCTION_AREA ==
				m_eNavigationMode &&
			m_RuntimeBlockerDocument.Is_CellInRegion(
				m_iSelectedRuntimeRegion,
				index);
		const float4_t& color =
			selectedDestructionCell ?
			magenta :
			NAVGRID_AUTHORING_CELL_STATE::BLOCKED == state ?
			yellow :
			green;

		const VertexPositionColor leftTop(
			float3_t(
				center.x - halfCell,
				center.y,
				center.z + halfCell),
			color);
		const VertexPositionColor rightTop(
			float3_t(
				center.x + halfCell,
				center.y,
				center.z + halfCell),
			color);
		const VertexPositionColor rightBottom(
			float3_t(
				center.x + halfCell,
				center.y,
				center.z - halfCell),
			color);
		const VertexPositionColor leftBottom(
			float3_t(
				center.x - halfCell,
				center.y,
				center.z - halfCell),
			color);

		resources.pBatch->DrawLine(leftTop, rightTop);
		resources.pBatch->DrawLine(rightTop, rightBottom);
		resources.pBatch->DrawLine(rightBottom, leftBottom);
		resources.pBatch->DrawLine(leftBottom, leftTop);
	}
	resources.pBatch->End();
}
```

## 5. 삭제 코드

다음 UI와 관련 분기를 삭제한다.

```text
Cells / Resolved height 상시 표시
Blocked / Unresolved walkable 상시 표시
Runtime regions 상시 표시
Static Walkability / Runtime Blocker Region 동시 control
Save Paint
Save Runtime Regions
Export Runtime
상시 Reload
상시 condition preview
상시 absolute path 출력
빨간 unresolved overlay
가까운 resolved 높이를 찾는 Build_DisplayHeights()
CPU ray로 모든 grid cell을 순회하는 picking fallback
```

## 6. 프로젝트 등록

이번 수정에서는 새 파일이 없으므로 `.vcxproj`와 `.vcxproj.filters`를 수정하지 않는다.
기존 등록은 다음과 같으며 적용 후 각 항목이 정확히 한 번 존재하는지 XML 파싱으로
검증한다.

```xml
<!-- Client/Default/Client.vcxproj -->
<ClInclude Include="..\Public\NavGridPaintDocument.h" />
<ClInclude Include="..\Public\NavRuntimeBlockerDocument.h" />
<ClCompile Include="..\Private\NavGridPaintDocument.cpp" />
<ClCompile Include="..\Private\NavRuntimeBlockerDocument.cpp" />
```

```xml
<!-- Client/Default/Client.vcxproj.filters -->
<ClCompile Include="..\Private\NavGridPaintDocument.cpp">
  <Filter>03. Tools\00. Map</Filter>
</ClCompile>
<ClCompile Include="..\Private\NavRuntimeBlockerDocument.cpp">
  <Filter>03. Tools\00. Map</Filter>
</ClCompile>
<ClInclude Include="..\Public\NavGridPaintDocument.h">
  <Filter>03. Tools\00. Map</Filter>
</ClInclude>
<ClInclude Include="..\Public\NavRuntimeBlockerDocument.h">
  <Filter>03. Tools\00. Map</Filter>
</ClInclude>
```

앞으로 파일을 추가할 때는 물리 역할을 먼저 정하고 같은 변경에서 프로젝트 등록까지
완료한다.

```text
Engine 범용 runtime/algorithm
  -> Engine/Public 또는 Engine/Private
  -> Engine.vcxproj
  -> Engine.vcxproj.filters
  -> public header면 UpdateLib 후 Client 빌드

LostArk Level/GameObject/MapTool/authoring
  -> Client/Public 또는 Client/Private
  -> Client.vcxproj
  -> Client.vcxproj.filters
```

새 필터는 기존 역할 필터가 없을 때만 추가하고, 기존 항목을 재배치하지 않는다.

## 7. 적용 순서

1. `NavGridPaintDocument.h/.cpp`를 수정한다.
2. `MapTool.h`의 Navigation enum, 함수, member를 수정한다.
3. `MapTool.cpp`의 load, save, picking, painting, panel, overlay를 교체한다.
4. 기존 unresolved 관련 symbol이 남아 있지 않은지 검색한다.
5. 프로젝트 XML을 파싱해 기존 등록이 유지되는지 확인한다.
6. 빌드와 실행 검증을 진행한다.

## 8. 빌드·실행 검증

```text
1. Engine x64 Debug
2. UpdateLib.bat Debug
3. Client x64 Debug
4. Engine x64 Release
5. UpdateLib.bat Release
6. Client x64 Release
```

실행 검증:

1. `Client/Default`를 working directory로 Debug Client를 실행한다.
2. F2로 AssetTest에 진입한다.
3. F1 → Navigation을 연다.
4. 기본 화면에 Walkability, Block/Erase, Brush, Save만 보이는지 확인한다.
5. 빨간 외곽 cell이 보이지 않는지 확인한다.
6. 높이가 없는 맵 외곽 클릭이 무시되는지 확인한다.
7. 초록 cell을 Block하면 노랑, Erase하면 초록으로 돌아오는지 확인한다.
8. Save Navigation 후 `.navpaint`, `.navblockers`, `.navgrid`가 갱신되는지 확인한다.
9. 재진입 후 편집 결과가 유지되는지 확인한다.
10. Destruction Area에서만 region, New, Test가 보이는지 확인한다.
11. unsaved 상태에서 Reload를 누르면 확인 popup이 뜨는지 확인한다.
12. F5에서 초록은 walkable, 노랑은 nonwalkable로 표시되는지 확인한다.
