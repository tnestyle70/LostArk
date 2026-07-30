# LostArk 발탄 NavGrid 수동 Walkability 편집 계획서

- 작성일: 2026-07-31
- 대상 브랜치: `feature/valtan-navgrid-runtime`
- 대상 레벨: `LEVEL::ASSET_TEST`
- 문서 유형: 혼합형 구현 계획서
- 이번 결정: `CUL_BOX` 안의 저작성 셀은 기본 walkable이며, non-walkable은 F1 MapTool에서 사용자가 직접 칠한다.
- 완료 범위: F1 편집 모드, 양방향 칠하기, 원자적 Save/Reload, 런타임 `.navgrid` 산출, A* 재진입 검증

> 동작함 != 이해함. 높이 검출 실패와 non-walkable을 같은 값으로 저장한 코드는 제거한다.

## 1. C1~C8 관점

| 관점 | 이번 작업에 적용한 사실 | 중요도 |
|---|---|---:|
| C1 기준계 | 셀 좌표 `(x,z)`와 월드 X/Z는 `origin + cell * cellSize`로만 변환한다. 화면에서 마름모로 보이는 것은 카메라 투영이며 셀을 회전하지 않는다. | ★★★ |
| C2 이동>계산 | CUL_BOX 범위, floor 높이 표본, 셀 크기는 Python에서 한 번 굽는다. 런타임은 최종 bool/height를 읽고 A*만 수행한다. | ★★★ |
| C3 공유는 비싸다 | 원본 높이 자료와 수동 페인트 문서는 공유 정본이다. A* open/closed와 follower waypoint는 객체별 임시 상태다. | ★★☆ |
| C4 수명은 선언된다 | MapTool 편집 상태는 AssetTest 진입부터 이탈까지, 저장 문서는 파일 수명, `CNavigation` clone은 게임 오브젝트 수명이다. | ★★★ |
| C5 이산화와 오차 | 월드 클릭은 `floor((world-origin)/cellSize)`로 셀 하나에 귀속한다. brush는 셀 좌표 반경으로만 확장한다. | ★★★ |
| C6 가지치기 | 높이가 해결되지 않은 셀은 자동 non-walkable로 둔갑시키지 않는다. 사용자가 막지 않은 미해결 셀이 하나라도 있으면 런타임 export를 거부한다. | ★★★ |
| C7 권위와 정합성 | floor bake 결과는 높이 정본, `.navpaint`는 수동 판정 정본, `.navgrid`는 둘을 결합한 런타임 파생물이다. | ★★★ |
| C8 검증이 병목 | 노란색 의미, 양방향 칠하기, 저장 후 재로드, 실패 시 기존 파일 보존, A* 우회까지 눈으로 확인한다. | ★★★ |

핵심 축은 `높이와 walkability 분리`, `사용자 override의 권위`, `원자적 저장`, `A* 소비 데이터 단순화`다.

## 2. 문제 해결 ①~⑤

① 문제·제약: 현재 62×63 격자의 1,063개 0은 실제 장애물뿐 아니라 floor 표본 없음, 메시 틈, 경사 제외를 모두 뜻한다. Debug 코드가 이 셀의 높이를 주변 평균으로 꾸며 노란 셀이 실제 차단처럼 보인다.

② 단순 해법의 문제: 현재 bool을 그대로 non-walkable로 부르면 잘못된 차단이 정본이 된다. 반대로 3,906개를 무조건 walkable로 저장하면서 미검출 높이를 0으로 두면 캐릭터가 Y=0으로 떨어진다.

③ 해결 방식: bake source에는 `heightResolved + height`만 저장하고 기본 walkability는 true로 둔다. F1 MapTool은 사용자가 칠한 blocked 셀만 `.navpaint`에 저장한다. `Save`는 모든 최종 walkable 셀의 높이가 해결됐는지 검증한 뒤 기존 형식의 `.navgrid`를 원자적으로 교체한다.

④ 비교: Winters/LoL 편집기는 평면 grid를 `SetAllWalkable(true)`로 만든 뒤 수동으로 false/true를 칠한다. 발탄은 높낮이가 있으므로 같은 방식에 `heightResolved` 검증 하나가 추가된다. Unreal/Recast의 자동 영역 생성은 이번 학습 목표와 저장 구조가 다르므로 사용하지 않는다.

⑤ 대가: source·paint·runtime 세 파일이 생긴다. 대신 재베이크가 수동 페인트를 지우지 않고, 노란색 의미가 하나로 고정된다. 3,906셀에서 brush와 export는 선형 비용이라 편집 시점에 충분히 작다.

## 3. 자료구조·알고리즘 핵심

### 3.1 세 파일의 단일 책임

| 파일 | 담는 값 | writer | reader | 정본 여부 |
|---|---|---|---|---|
| `ValtanArena.navsource` | width/height/cellSize/origin, 셀별 `heightResolved`, height | Python baker | MapTool | 높이 정본 |
| `ValtanArena.navpaint` | area/grid 식별자와 사용자가 막은 `(x,z)` | MapTool Save | MapTool Reload | 수동 판정 정본 |
| `ValtanArena.navgrid` | 최종 walkable byte + height float | MapTool export | `CNavGrid::Load` | 런타임 파생물 |

`Prototype tag`, 포인터, `vector` index는 저장 ID로 사용하지 않는다. 저장 좌표는 `(x,z)`이고, 헤더의 width/height/cellSize/origin이 달라지면 paint load를 거부한다.

### 3.2 `NAV_SOURCE_CELL`

```text
이름과 타입: NAV_SOURCE_CELL { bool heightResolved; float height; }
표현하는 상태: 이 셀에 실제 floor 근거가 있는가와 그 월드 Y.
owner: MapTool의 authoring document.
writer: Python baker.
reader: F1 overlay와 runtime export 검증.
불변식: heightResolved=true이면 height는 finite.
원소 수: 현재 3,906개.
프레임당 빈도: 편집 클릭과 debug draw에서 읽고, 런타임에서는 읽지 않는다.
```

### 3.3 `unordered_set<NAV_CELL_COORD>` blocked cells

```text
표현하는 상태: 사용자가 명시적으로 막은 셀만 담는다.
owner: CNavGridPaintDocument.
writer: Paint Non-Walkable은 insert, Paint Walkable은 erase.
reader: overlay, Save, runtime export.
불변식: 좌표는 grid 범위 안이고 중복이 없다.
규모: 최악 3,906개지만 실제 blocked 셀만 존재한다.
```

기본 상태는 `blocked set에 없으면 walkable`이다. 따라서 이미 막은 셀을 다시 walkable로 만드는 기능은 별도 미래 시스템 없이 `erase` 한 줄로 현재 단계에서 완성된다.

### 3.4 최종 상태 계산

```text
effectiveWalkable(x,z) = !blocked.contains(x,z)

export 가능 조건 =
    모든 셀에 대해
    effectiveWalkable == false
    또는 source.heightResolved == true
```

미해결 셀은 non-walkable이 아니다. 사용자가 아직 판단하지 않은 authoring 오류다.

### 3.5 실제 값 흐름

```text
월드 클릭 (156.25, 22.99, -121.75)
 -> World_ToCell = (31,31)
 -> Paint Non-Walkable 선택 상태
 -> blocked.insert({31,31})
 -> overlay는 해당 셀만 노란색
 -> Save
 -> navpaint row: 31 31
 -> source와 paint 결합
 -> runtime navgrid index 1953의 walkable byte = 0
 -> AssetTest 재진입
 -> CNavGrid::Load
 -> CPathFinder가 index 1953을 이웃 후보에서 제외
```

### 3.6 Paint 알고리즘

```text
입력: picked world position, brush radius, PAINT_BLOCKED/PAINT_WALKABLE.
출력: blocked set 변경과 dirty=true.
처리: world-to-cell -> 원형 brush 안의 유효 좌표 순회 -> insert 또는 erase.
종료: brush bounding square 순회 완료.
실패: 피킹 실패, grid 밖, ImGui가 mouse를 capture함.
시간 복잡도: O((2r+1)^2), r 기본 0, 최대 8.
공간 복잡도: 새 임시 할당 없음.
호출 빈도: F1 Navigation 편집 모드에서 mouse drag frame마다 최대 1회.
```

### 3.7 Save 알고리즘

```text
입력: staged source, staged blocked set.
출력: navpaint와 runtime navgrid 두 파일.
처리: validate -> 두 임시 파일 작성 -> flush/close -> 목적 파일 교체.
실패: header 불일치, 범위 밖 좌표, 미해결 walkable 셀, I/O 실패.
실패 전파: false + MapTool status. 기존 파일은 그대로 유지.
시간/공간: O(3,906), walkable byte 3,906B + height 15,624B.
호출: 사용자가 Save 버튼을 눌렀을 때만 main thread.
```

## 4. 현재 상태와 제거할 회귀

현재 실측값은 다음과 같다.

```text
width 62, height 63, cellSize 0.5
origin (140.5, -137.5)
총 3,906
기존 walkable 2,843
기존 0 1,063
기존 0 셀의 저장 height는 전부 0
기존 walkable height 범위 20.95236 ~ 23.29906
```

`CNavigation::Initialize_NavGrid_Prototype`의 `StagedDebugCellHeights` 주변 평균 생성은 삭제한다. Debug가 없는 근거를 만들어 내면 안 된다. 최종 표시 의미는 다음으로 고정한다.

- 초록: 높이가 해결됐고 현재 walkable.
- 노랑: 사용자가 `.navpaint`에서 막은 셀.
- 빨강: 높이 미해결인데 아직 막지 않은 셀. Save 불가.
- 하늘색: 마지막 성공 A* 경로.

## 5. 추가·수정·삭제 파일 목록

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/build_valtan_navgrid.py` | walkability 판정을 제거하고 height source를 생성 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/NavGridPaintDocument.h` | source/paint 문서와 편집 계약 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/NavGridPaintDocument.cpp` | parse→validate→stage→commit, atomic save/export |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/MapTool.h` | Navigation 편집 모드와 최소 상태 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MapTool.cpp` | F1 tab, paint, Save/Reload, status |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Public/Navigation.h` | Debug 전용 authoring mask 전달 계약 |
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/Navigation.cpp` | 꾸며낸 높이 제거와 3색 overlay |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Navigation/ValtanArena.navsource` | bake height source |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Navigation/ValtanArena.navpaint` | 수동 blocked 좌표 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Navigation/ValtanArena.navgrid` | 검증 완료 후 생성되는 런타임 파일 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj` | 새 문서 h/cpp 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters` | 물리 폴더에 대응하는 항목만 추가 |

## 6. 파일별 본질·의존성·최종 계약

### 6.1 `build_valtan_navgrid.py`

한 문장 본질: 메시와 placement로부터 각 셀의 실제 높이 근거만 계산한다.

- 역할: CUL_BOX 범위, 0.5m 셀, floor triangle 높이, `heightResolved` 생성.
- 비역할: 장애물·이동 가능 여부 결정, 수동 paint 파일 수정.
- 입력: 기존 floor01/A/B, main floor, center floor, placement, CUL_BOX.
- 출력: `.navsource` 하나.
- 유지할 알고리즘: 배치 행렬 적용, XZ rasterization, 겹친 표면 중 가장 높은 Y.
- 제거할 의미: `maxSlope` 실패나 표본 없음이 곧 non-walkable이라는 결론.

source row 형식은 다음으로 고정한다.

```text
LOSTARK_NAVGRID_SOURCE 1 "LV_LUT_HEARTRB_ED" 62 63 0.5 140.5 -137.5 3906
0 0 0 0
1 0 1 22.991234
...
```

row는 `x z heightResolved height`다. 좌표는 외부 안정 ID이고 row 순서는 검증 편의를 위해 z-major로 고정한다.

### 6.2 `CNavGridPaintDocument`

한 문장 본질: 편집 정본을 메모리에 staged하고 검증된 결과만 파일로 commit한다.

- 역할: source load, paint load, block/unblock, dirty, atomic save, runtime export.
- 비역할: ImGui, 피킹, A*, 렌더링.
- 호출자: `CMapTool`.
- 의존성: 표준 라이브러리와 `Engine_Defines`의 수치 타입만 사용한다.
- 실패: out parameter status로 사람이 이해할 수 있는 한 문장을 반환하고 기존 상태를 보존한다.

필수 public 계약은 아래 여섯 개만 둔다.

```cpp
bool_t Load(const filesystem::path& sourcePath,
    const filesystem::path& paintPath, string& outStatus);
bool_t Paint(fvector_t worldPosition, uint32_t brushRadius,
    bool_t walkable);
bool_t Save(const filesystem::path& paintPath,
    const filesystem::path& runtimePath, string& outStatus) const;
bool_t Is_Walkable(uint32_t index) const;
bool_t Is_HeightResolved(uint32_t index) const;
const NAVGRID_AUTHORING_DESC& Get_Desc() const;
```

현재 단계에는 undo stack, command pattern, 압축, SHA, 비동기 저장을 넣지 않는다.

### 6.3 `CMapTool`

한 문장 본질: F1에서 사용자의 편집 의도를 문서 명령으로 바꾼다.

- 기존 `PLACEMENT_STATE::ARMED`와 Navigation paint는 동시에 켜질 수 없다.
- 상단 tab은 `Map Assets`와 `Navigation` 둘만 추가한다.
- Navigation tab은 `Paint Non-Walkable`, `Paint Walkable`, brush radius, `Save`, `Reload`, 통계만 가진다.
- mouse가 ImGui에 잡히지 않았을 때 viewport LMB drag로 현재 모드를 칠한다.
- Save 성공 후 status에 `AssetTest 재진입 시 runtime 적용`을 명시한다.
- 매 프레임 파일을 읽지 않는다.

### 6.4 `CNavigation` Debug 경로

한 문장 본질: runtime 판정과 authoring 관찰을 같은 것으로 착각하지 않도록 화면에만 표시한다.

- `StagedDebugCellHeights`, `m_DebugCellHeights`를 삭제한다.
- Debug 전용으로 MapTool이 제공한 셀 표시 상태를 복사해 그린다.
- 이 mask는 `CPathFinder`가 읽지 않는다.
- F5는 기존 runtime grid/path 관찰, F1 Navigation tab은 authoring overlay 관찰이다.

## 7. 프로젝트 등록

새 C++ 파일은 Client 프로젝트에만 등록한다.

```xml
<ClInclude Include="..\Public\NavGridPaintDocument.h" />
<ClCompile Include="..\Private\NavGridPaintDocument.cpp" />
```

기존 `Public`, `Private` filter를 그대로 사용하고 새 filter GUID는 만들지 않는다.

## 8. 적용 순서와 검증

1. 현재 `.navgrid`를 검산 자료로 보존하고 새 `.navsource` 생성기를 먼저 닫는다.
2. 독립 parser로 3,906 row, 좌표 중복 0, finite height 조건을 확인한다.
3. `CNavGridPaintDocument`의 load/paint/save를 GPU 없이 작은 테스트로 확인한다.
4. MapTool F1 Navigation tab과 편집 mode exclusivity를 연결한다.
5. Navigation의 꾸며낸 높이 코드를 삭제하고 표시 색 의미를 고정한다.
6. Client 프로젝트 XML을 등록한다.
7. Engine Debug → UpdateLib Debug → Client Debug 순서로 빌드한다.
8. `Client/Bin/Debug/Client.exe`, working directory `Client/Bin`으로 실행한다.
9. `F2 → AssetTest → F1 → Navigation`으로 진입한다.
10. 초록 셀 하나를 노랑으로 칠하고 다시 walkable로 칠해 원상 복구되는지 확인한다.
11. brush로 외곽·기둥·낙사 영역을 막는다. 노랑은 수동 paint만 나타나야 한다.
12. 미해결 빨강 셀이 남았을 때 Save가 거부되고 기존 두 출력 파일 hash가 유지되는지 확인한다.
13. 모든 미해결 셀을 막은 뒤 Save 성공, Reload 동일, dirty false를 확인한다.
14. AssetTest 재진입 후 F5에서 최종 grid와 A* 우회를 확인한다.
15. Release에서는 ImGui/editor가 제외되고 `.navgrid` runtime load와 A*만 남는지 확인한다.

## 9. 이번 단계에서 하지 않는 것

- Deploy 파괴 상태에 따른 동적 blocker.
- agent radius inflation.
- 서로 다른 높이 층이 같은 XZ에 겹치는 다층 grid.
- undo/redo와 다중 선택.
- 자동 장애물 추론.

후속 동적 상태는 `최종 authoring walkability + runtime blocker`로 겹쳐야 하며 `.navpaint`를 런타임에서 수정하지 않는다.

## 10. 완료 판정

다음 문장을 코드와 화면으로 설명할 수 있어야 완료다.

> Python은 높이를 굽고, 사용자는 F1에서 이동 가능 여부를 칠하며, Save가 둘을 검증해 A*가 읽는 단순한 최종 grid를 만든다.
