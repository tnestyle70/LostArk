# LostArk 발탄 NavGrid 기본 Walkable · 피킹 Non-Walkable 편집 계획서

- 작성일: 2026-07-31
- 현재 브랜치: `codex/valtan-nav-movement-architecture-plan`
- 대상 레벨: `LEVEL::ASSET_TEST`
- 문서 유형: 혼합형 구현 계획서
- 사용자 목표: CUL_BOX 격자는 우선 전부 walkable로 시작하고, 실제 이동 불가 지역은 사용자가 화면에서 피킹해 non-walkable로 칠한다.
- 완료 범위: 현재 F5/bake/A* 흐름 설명, F1 Navigation 편집 모드, block/unblock, Save/Reload, 기존 `.navgrid` export, F5와 A* 재검증

> 핵심 결정: Python은 셀의 위치와 높이 근거까지만 만든다. 이동 가능 여부의 정본은
> MapTool에서 사용자가 저장한 blocked 셀이다.

## 1. C1~C8 관점

| 관점 | 이번 작업에 적용한 사실 | 중요도 |
|---|---|---:|
| C1 기준계 | 격자 `(x,z)`와 월드 X/Z는 `origin + cell * cellSize`로 변환한다. 현재 값은 `62×63`, `0.5m`, origin `(140.5,-137.5)`다. | ★★★ |
| C2 이동>계산 | glTF/placement 결합과 셀별 높이 표본은 Python에서 한 번 계산한다. 런타임은 최종 byte/height를 읽고 A*만 수행한다. | ★★★ |
| C3 공유는 비싸다 | `.navsource`와 `.navpaint`는 편집 정본이고 `.navgrid`는 파생물이다. A* open/closed와 actor waypoint는 인스턴스별 임시 상태다. | ★★☆ |
| C4 수명은 선언된다 | MapTool 문서는 AssetTest 편집 세션 동안 살아 있고, 저장 파일은 세션 밖에서도 유지된다. `CNavigation` Prototype/clone은 현재 레벨 수명을 따른다. | ★★★ |
| C5 이산화와 오차 | 월드 클릭은 `floor((world-origin)/cellSize)`로 셀 하나에 귀속한다. 바닥이 없는 구역은 GPU 피킹이 실패하므로 격자 표시 높이와 카메라 ray를 추가로 교차한다. | ★★★ |
| C6 가지치기 | 범위 밖 클릭, ImGui가 잡은 마우스, 잘못된 source/paint, 높이 없는 최종 walkable 셀은 저장 또는 export 전에 제외한다. | ★★★ |
| C7 권위와 정합성 | 바닥 메시/placement는 높이 입력 정본, `.navsource`는 높이 bake 정본, `.navpaint`는 수동 blocked 정본, `.navgrid`는 런타임 파생물이다. | ★★★ |
| C8 검증이 병목 | 전부 기본 walkable, block/unblock, 빈 공간 피킹, 저장·재로드, 실패 시 기존 파일 보존, F5 색, A* 우회를 실제 실행으로 확인한다. | ★★★ |

핵심 축은 `높이와 walkability 분리`, `기본 허용 + 명시적 차단`, `편집 정본과 런타임 파생물 분리`, `빈 공간도 선택 가능한 grid picking`이다.

## 2. 문제 해결 ①~⑤

① 문제·제약: 현재 노란 1,063셀은 사용자가 막은 셀이 아니다. Python이 바닥 삼각형
중심 표본을 얻지 못했거나 45도 경사 기준에서 제외한 셀을 곧바로 `walkable=0`으로
기록한 결과다. F5는 그 binary를 그대로 초록/노랑으로 표시한다.

② 단순 해법의 문제: 현재 `.navgrid`의 0을 곧바로 수동 blocked 정본으로 사용하면
“표본 없음”과 “사용자가 이동 불가로 판정함”을 구별할 수 없다. 반대로 3,906셀을
무조건 1로 저장하면서 미검출 height를 0으로 두면 A*가 성공한 actor가 Y=0으로 이동한다.

③ 해결 방식: Python은 CUL_BOX 전체 3,906셀의 `heightResolved + height`를
`.navsource`로 쓴다. MapTool은 전 셀을 기본 walkable로 시작하고 사용자가 칠한 셀만
`.navpaint`에 blocked 좌표로 저장한다. 최종 walkable인데 높이 미해결인 셀이 하나라도
있으면 `.navgrid` export를 거부한다.

④ 비교: 기존 코드는 메시 표본 유무가 곧 walkability였다. 변경 후에는 메시 표본은
Y 좌표의 근거이고 walkability는 사용자 authoring 값이다. Unreal/Recast처럼 collision을
자동 분류하는 NavMesh bake가 아니라, 현재 수업식 grid와 MapTool 소유권을 유지한다.

⑤ 대가: `.navsource`, `.navpaint`, `.navgrid` 세 파일과 F1 편집 UI가 필요하다.
대신 재베이크가 수동 paint를 지우지 않고, 노란색은 오직 사용자가 막은 셀만 뜻한다.
현재 3,906셀에서 paint는 최대 289셀, ray-grid pick과 export는 O(3,906)이므로 편집
프레임에서 충분히 작다.

## 3. 자료구조·알고리즘 핵심

### 3.1 저장 자료와 수명

| 이름과 타입 | 표현하는 상태 | owner / writer | reader | 불변식·규모·빈도 |
|---|---|---|---|---|
| `ValtanArena.navsource` | grid desc, 셀별 높이 검출 여부와 Y | Python baker | `CNavGridPaintDocument` | 3,906 row, bake 때만 씀 |
| `ValtanArena.navpaint` | 사용자가 명시적으로 막은 `(x,z)` | MapTool Save | MapTool Reload | 범위 안, 중복 없음, 보통 1,063개 이하 |
| `ValtanArena.navgrid` | 최종 walkable byte + height | MapTool Export | `CNavGrid::Load` | 현재 binary 계약 19,550 bytes |
| `vector<NAV_SOURCE_CELL>` | 로드된 높이 source | `CNavGridPaintDocument` | overlay/export | size=`width*height`, 세션 동안 불변 |
| `vector<uint8_t> m_BlockedCells` | 셀별 수동 blocked bit | `CNavGridPaintDocument` | overlay/save/export | 값은 0/1, Paint만 writer |
| `vector<float> m_DisplayHeights` | 미해결 셀을 화면에 놓기 위한 최근접 높이 | `CNavGridPaintDocument::Build_DisplayHeights` | F1 overlay/grid ray pick | 편집 파생값이며 runtime height 정본이 아님 |

고정 grid이므로 `unordered_set`보다 `vector<uint8_t>`가 단순하다. index는 메모리 접근용
파생값이고 파일의 안정 ID는 `(x,z)`다.

### 3.2 상태 계산

```text
effectiveWalkable(x,z) = blocked[x,z] == 0

F1 authoring color:
  blocked == 1                         -> yellow
  blocked == 0 && heightResolved == 0 -> red
  blocked == 0 && heightResolved == 1 -> green

runtime export 가능:
  모든 셀에서 blocked == 1 또는 heightResolved == 1
```

빨강은 “자동 non-walkable”이 아니다. 기본 walkable이지만 아직 안전한 runtime height가
없으므로 사용자가 blocked로 확정하거나 source를 고쳐야 하는 셀이다.

`walkable`은 셀 진입 허용 여부이고 A*의 edge 조건 전부를 대체하지 않는다.
`CPathFinder::Can_Step()`은 두 초록 셀 사이에서도 `abs(toHeight-fromHeight) >
fMaxStepHeight`이면 그 연결을 제외한다. 이번 단계는 그 기존 높이 차 계약을 유지한다.

### 3.3 높이 bake 알고리즘

```text
입력:
  Floor01/A/B glTF, main floor glTF, center floor glTF
  overlay와 exact mapplacements
  CUL_BOX glTF placement, cellSize 0.5

출력:
  .navsource의 3,906개 heightResolved/height

처리:
  glTF local triangle
  -> placement matrix로 world triangle
  -> CUL_BOX world bounds를 0.5m grid로 이산화
  -> cell center XZ가 triangle 안인지 barycentric 검사
  -> 45도 이하 surface height를 우선 기록
  -> 우선 surface가 없는 셀만 더 가파른 surface 높이로 보충
  -> 어느 triangle도 덮지 않은 셀은 heightResolved=0

종료:
  모든 world triangle과 그 XZ bounding cell 순회 완료

실패:
  입력 누락, placement/asset ID 불일치, 잘못된 grid 크기, 유한하지 않은 값

시간:
  O(삼각형별 XZ 후보 셀 수), offline 1회

공간:
  O(width*height), 현재 각 배열 3,906개
```

경사 기준은 더 이상 walkability를 쓰지 않는다. 같은 셀에 평평한 바닥과 가파른 장식
면이 겹쳤을 때 실제 바닥 height를 우선 선택하는 품질 기준으로만 남는다.

### 3.4 Paint 알고리즘

```text
입력:
  picked cell, brush radius 0..8, BLOCKED 또는 WALKABLE

출력:
  m_BlockedCells 변경, dirty=true

처리:
  GPU world pick -> World_ToCell
  실패하면 camera ray -> 3,906개 display-height cell quad 중 최근접 교차
  -> 원형 brush 안의 유효 셀
  -> BLOCKED는 1, WALKABLE은 0

실패:
  main viewport 밖, grid 밖, ImGui mouse capture, document 미로드

시간:
  pick O(3,906), paint O((2r+1)^2), r 최대 8

공간:
  frame당 heap 할당 없음
```

GPU 피킹만 사용하면 낙사 구멍처럼 렌더된 표면이 없는 곳을 선택할 수 없다. 따라서
authoring grid 자체를 CPU ray와 교차하는 fallback이 이번 목표에 필수다.

### 3.5 Save와 Export 알고리즘

```text
Save Paint:
  validate -> .navpaint.tmp write/flush/close -> ReplaceFileW/MoveFileExW
  실패 시 기존 .navpaint 유지

Export Runtime:
  unresolved walkable count가 0인지 검사
  -> 현재 CNavGrid binary payload stage
  -> .navgrid.tmp write/flush/close
  -> ReplaceFileW/MoveFileExW
  실패 시 기존 .navgrid 유지
```

두 파일을 한 transaction처럼 위장하지 않는다. `Save Paint`와 `Export Runtime`은 별도
버튼·별도 원자적 commit이다. Export는 dirty가 false일 때만 UI에서 허용해 저장된
`.navpaint`와 runtime 결과를 일치시킨다.

### 3.6 실제 값 하나의 전체 흐름

```text
F1 Navigation에서 월드 (156.25, 22.99751, -121.75) 클릭
 -> World_ToCell:
    x=floor((156.25-140.5)/0.5)=31
    z=floor((-121.75-(-137.5))/0.5)=31
 -> index = 31*62+31 = 1953
 -> Paint Non-Walkable
 -> m_BlockedCells[1953] = 1
 -> F1 overlay yellow
 -> Save Paint row "31 31"
 -> Export Runtime walkable[1953] = 0
 -> AssetTest 재진입
 -> Loader가 ValtanArena.navgrid로 Navigation Prototype 생성
 -> Character/Valtan이 Prototype clone
 -> F5에서 index 1953 yellow
 -> CPathFinder::Can_Step가 Is_Walkable(1953)==false로 후보 제외
 -> A* 경로가 이 셀을 우회
```

## 4. 현재 코드: 어디서 베이크하고 F5가 무엇을 하는가

### 4.1 현재 수직 흐름

```text
[개발자가 수동 실행]
Tools/LevelPlacementExtractor/build_valtan_navgrid.py
  ├─ glTF triangle 읽기
  ├─ overlay/mapplacements transform 적용
  ├─ CUL_BOX로 62×63 범위 계산
  ├─ rasterize(): slope + cell-center 표본
  └─ write_navgrid()
       ↓
Client/Bin/DataFiles/Navigation/ValtanArena.navgrid
       ↓ AssetTest loading thread
CLoader::Ready_For_Level_AssetTest()
  -> CNavigation::Create_NavGrid()
  -> CNavGrid::Load()
  -> Navigation Prototype 등록
       ↓ Level initialize
CCharacter / CValtan
  -> Prototype_Component_Navigation_ValtanArena clone
       ↓ F5 edge
CLevel_AssetTest::Update_NavigationDebug()
  -> 두 actor의 Set_NavigationDebugVisible()
       ↓ actor Late_Update
CGameInstance::Add_DebugComponent(CNavigation)
       ↓
CNavigation::Render()
  -> walkable green
  -> non-walkable yellow
  -> 마지막 성공 path cyan
       ↓ click move / boss query
CNavigation::Find_Path()
  -> CPathFinder 8방향 A*
```

F5는 bake 명령이 아니다. 파일을 쓰지도 않고 walkability를 계산하지도 않는다.
이미 Loader가 읽은 runtime grid를 보이거나 숨기는 Debug 토글이다.

### 4.2 현재 baker 실행 위치와 명령

현재 bake는 Visual Studio build나 Client 실행에 자동 연결되어 있지 않다. 저장소
루트에서 개발자가 다음 Python 명령을 수동 실행한다.

```powershell
$extractRoot = 'C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Map/StaticMesh_Raw_20260729'
$valtanBg = Join-Path $extractRoot 'BG/BG_RAD_VALTAN_A__542N9YJ2R1Y3NYH2RYFTMPE9/BG_RAD_VALTAN_A/StaticMesh3'
$heartLv = Join-Path $extractRoot 'LV/LV_LUT_HEARTRB__201M2TM1QAPX8M84E7DM961/LV_LUT_HEARTRB/StaticMesh3'
$centerBg = Join-Path $extractRoot 'BG/BG_LUT_WAGLOY_A__542N3UN2R8Y43OM2RYFTUHE9/BG_LUT_WAGLOY_A/StaticMesh3'
$navLv = Join-Path $extractRoot 'LV/LV_NAVIMESH__1Z0LFWZG8OE9D61V6R2IMO/LV_NAVIMESH/StaticMesh3'

python Tools/LevelPlacementExtractor/build_valtan_navgrid.py `
  --floor01 (Join-Path $valtanBg 'bg_rad_valtan_floor01_sm.gltf') `
  --floor01a (Join-Path $valtanBg 'bg_rad_valtan_floor01a_sm.gltf') `
  --floor01b (Join-Path $valtanBg 'bg_rad_valtan_floor01b_sm.gltf') `
  --main-floor (Join-Path $heartLv 'lv_lut_heartrb_floor01_sm.gltf') `
  --center-floor (Join-Path $centerBg 'bg_lut_wagloy_circlefloor01_sm_jjy.gltf') `
  --overlay Tools/LevelPlacementExtractor/heartrb_valtan_core_overlay.json `
  --mapplacements Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.mapplacements `
  --bounds-gltf (Join-Path $navLv 'lv_common_mesh_cul_box_8.gltf') `
  --cell-size 0.5 `
  --preferred-floor-slope 45 `
  --area-id LV_LUT_HEARTRB_ED `
  --output Client/Bin/DataFiles/Navigation/ValtanArena.navsource
```

### 4.3 현재 binary 실측

2026-07-31 현재 파일을 독립적으로 읽은 결과다.

```text
path:
  Client/Bin/DataFiles/Navigation/ValtanArena.navgrid

SHA-256:
  7FDCEAA76A24AFF6D58AB1904C28D0CA99D779DAE486938BEA281EB8E0275638

binary:
  uint32 width      = 62
  uint32 height     = 63
  float cellSize    = 0.5
  float originX     = 140.5
  float originZ     = -137.5
  uint8 walkable[3906]
  float height[3906]

size:
  20 + 3906 + 3906*4 = 19,550 bytes

walkable:
  2,843

non-walkable:
  1,063

non-walkable height == 0:
  1,063

walkable height:
  20.95236397 ~ 23.29906082
```

현재 Python slope 제한을 45도에서 사실상 90도로 완화한 별도 실측에서는 높이가 잡힌
셀이 2,897개로 54개 늘었고, triangle 표본 자체가 없는 셀은 1,009개였다. 변경
baker는 2,843개 기존 평탄 표본을 우선 유지하고 54개만 보충한다. 1,009개는 기본
walkable authoring 상태로 시작하지만 빨강으로 표시되며, blocked 확정 전에는 export할
수 없다.

### 4.4 현재 노란 셀 표시 높이의 정체

`CNavigation::Initialize_NavGrid_Prototype()`은 non-walkable height가 0이므로 가장
가까운 walkable ring의 평균 높이를 `m_DebugCellHeights`에 만든다. 이는 F5 선을
Y=0이 아니라 바닥 근처에 보이게 하는 Debug 전용 값이다.

```text
파일의 실제 runtime 값:
  walkable=0, height=0

F5 표시:
  주변 walkable height 평균을 임시 사용

A*:
  원래 height를 보기 전에 Is_Walkable=false로 후보 제외
```

이번 작업은 이 Engine 경로를 수정하지 않는다. F1 authoring overlay는 자기
`displayHeight`를 사용하고, F5는 export된 최종 runtime grid만 계속 관찰한다.

## 5. 변경 후 수직 흐름

```text
현재
  Python: height 표본 + walkability 자동 결정 -> .navgrid

변경
  Python: height 표본만 결정 -> .navsource
  MapTool: 기본 all walkable + picked blocked -> .navpaint
  MapTool: source + paint 검증 -> .navgrid

결과
  F5/CNavGrid/CNavigation/CPathFinder는 기존 runtime 계약 그대로 사용
```

F1은 authoring UI와 live overlay, F5는 저장 후 다시 로드된 runtime 결과의 관찰이다.
Save/Export 후 현재 레벨의 Navigation Prototype을 몰래 교체하지 않는다. AssetTest를
나갔다 다시 들어와 기존 Prototype/Clone 수명으로 재로드한다.

## 6. 추가·수정·삭제 파일 목록

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/LevelPlacementExtractor/build_valtan_navgrid.py` | 자동 walkability 출력 대신 height source 생성 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/NavGridPaintDocument.h` | source/paint 문서와 block/export 계약 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/NavGridPaintDocument.cpp` | parse→validate→stage→commit, paint, atomic save/export |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/MapTool.h` | Navigation mode, paint state, overlay resource 계약 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MapTool.cpp` | F1 UI, grid picking, paint, overlay, Save/Reload/Export |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MainApp.cpp` | Navigation paint 중 게임 LMB 입력 차단 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Navigation/ValtanArena.navsource` | Python이 생성한 높이 정본 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Navigation/ValtanArena.navpaint` | 사용자가 저장한 blocked 정본 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Bin/DataFiles/Navigation/ValtanArena.navgrid` | MapTool이 export하는 기존 runtime 형식 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj` | 새 h/cpp 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters` | 기존 MapTool filter에 새 h/cpp 등록 |

Engine 파일, `CNavGrid`, `CNavigation`, `CPathFinder`, `Loader`, `Level_AssetTest`,
Character, Valtan은 수정하지 않는다. 현재 작업 트리에 있는 다른 팀원의 해당 파일
변경과 겹치지 않는 것이 이 범위의 중요한 장점이다.

## 7. 파일별 본질·의존성·최종 구현 코드

## 7.1 `build_valtan_navgrid.py`

### 본질·책임

1. 한 문장 본질: 월드에 배치된 바닥 메시에서 grid cell의 높이 근거만 굽는다.
2. 역할: glTF/placement 변환, CUL_BOX 범위, preferred/fallback height rasterize,
   `.navsource` 원자적 작성.
3. 비역할: blocked 판정, `.navpaint` 수정, runtime A*, ImGui.
4. 의존성: 기존 `read_gltf`/`load_triangles`/`placement_matrix`를 그대로 사용한다.
5. 자료구조: `preferred_resolved/heights`, `fallback_resolved/heights` 네 NumPy 배열은
   `main()` 한 번 동안만 존재한다.
6. 실패: 입력·ID·수치·grid가 잘못되면 기존 output을 교체하지 않고 예외로 종료한다.
7. 복잡도: 기존 triangle rasterize와 같고 저장은 O(3,906)이다.
8. 경계: Python/offline만 바뀌며 Client runtime에는 Python 의존성이 없다.

### 최종 교체 코드: `rasterize` 전체

기존 `rasterize()`를 아래 `rasterize_height_source()`로 통째로 교체한다.

```python
def rasterize_height_source(
    triangles: np.ndarray,
    bounds_min: np.ndarray,
    bounds_max: np.ndarray,
    cell_size: float,
    preferred_floor_slope_degrees: float,
) -> tuple[
    float,
    float,
    int,
    int,
    np.ndarray,
    np.ndarray,
    int,
    int,
]:
    origin_x = (
        math.floor(float(bounds_min[0]) / cell_size)
        * cell_size
    )
    origin_z = (
        math.floor(float(bounds_min[2]) / cell_size)
        * cell_size
    )
    width = math.ceil(
        (float(bounds_max[0]) - origin_x)
        / cell_size
    )
    height = math.ceil(
        (float(bounds_max[2]) - origin_z)
        / cell_size
    )

    if width <= 0 or height <= 0:
        raise ValueError("invalid NavGrid dimensions")
    if width * height > 1_000_000:
        raise ValueError(
            "NavGrid exceeds 1,000,000 cells"
        )

    preferred_resolved = np.zeros(
        (height, width),
        dtype=np.uint8,
    )
    preferred_heights = np.zeros(
        (height, width),
        dtype=np.float32,
    )
    fallback_resolved = np.zeros(
        (height, width),
        dtype=np.uint8,
    )
    fallback_heights = np.zeros(
        (height, width),
        dtype=np.float32,
    )
    minimum_preferred_normal_y = math.cos(
        math.radians(
            preferred_floor_slope_degrees
        )
    )

    for triangle in triangles:
        edge_a = triangle[1] - triangle[0]
        edge_b = triangle[2] - triangle[0]
        normal = np.cross(edge_a, edge_b)
        normal_length = float(
            np.linalg.norm(normal)
        )
        if normal_length <= 1e-12:
            continue

        normal_y = (
            abs(float(normal[1]))
            / normal_length
        )
        is_preferred = (
            normal_y
            >= minimum_preferred_normal_y
        )

        triangle_xz = triangle[:, [0, 2]]
        denominator = (
            (
                triangle_xz[1, 1]
                - triangle_xz[2, 1]
            )
            * (
                triangle_xz[0, 0]
                - triangle_xz[2, 0]
            )
            + (
                triangle_xz[2, 0]
                - triangle_xz[1, 0]
            )
            * (
                triangle_xz[0, 1]
                - triangle_xz[2, 1]
            )
        )
        if abs(float(denominator)) <= 1e-12:
            continue

        min_x = max(
            0,
            math.floor(
                (
                    float(
                        triangle_xz[:, 0].min()
                    )
                    - origin_x
                )
                / cell_size
            ),
        )
        max_x = min(
            width - 1,
            math.floor(
                (
                    float(
                        triangle_xz[:, 0].max()
                    )
                    - origin_x
                )
                / cell_size
            ),
        )
        min_z = max(
            0,
            math.floor(
                (
                    float(
                        triangle_xz[:, 1].min()
                    )
                    - origin_z
                )
                / cell_size
            ),
        )
        max_z = min(
            height - 1,
            math.floor(
                (
                    float(
                        triangle_xz[:, 1].max()
                    )
                    - origin_z
                )
                / cell_size
            ),
        )

        target_resolved = (
            preferred_resolved
            if is_preferred
            else fallback_resolved
        )
        target_heights = (
            preferred_heights
            if is_preferred
            else fallback_heights
        )

        for cell_z in range(
            min_z,
            max_z + 1,
        ):
            sample_z = (
                origin_z
                + (cell_z + 0.5) * cell_size
            )
            if (
                sample_z < bounds_min[2]
                or sample_z > bounds_max[2]
            ):
                continue

            for cell_x in range(
                min_x,
                max_x + 1,
            ):
                sample_x = (
                    origin_x
                    + (cell_x + 0.5)
                    * cell_size
                )
                if (
                    sample_x < bounds_min[0]
                    or sample_x > bounds_max[0]
                ):
                    continue

                barycentric_a = (
                    (
                        triangle_xz[1, 1]
                        - triangle_xz[2, 1]
                    )
                    * (
                        sample_x
                        - triangle_xz[2, 0]
                    )
                    + (
                        triangle_xz[2, 0]
                        - triangle_xz[1, 0]
                    )
                    * (
                        sample_z
                        - triangle_xz[2, 1]
                    )
                ) / denominator
                barycentric_b = (
                    (
                        triangle_xz[2, 1]
                        - triangle_xz[0, 1]
                    )
                    * (
                        sample_x
                        - triangle_xz[2, 0]
                    )
                    + (
                        triangle_xz[0, 0]
                        - triangle_xz[2, 0]
                    )
                    * (
                        sample_z
                        - triangle_xz[2, 1]
                    )
                ) / denominator
                barycentric_c = (
                    1.0
                    - barycentric_a
                    - barycentric_b
                )
                if min(
                    barycentric_a,
                    barycentric_b,
                    barycentric_c,
                ) < -1e-7:
                    continue

                sample_y = float(
                    barycentric_a
                    * triangle[0, 1]
                    + barycentric_b
                    * triangle[1, 1]
                    + barycentric_c
                    * triangle[2, 1]
                )
                if (
                    target_resolved[
                        cell_z,
                        cell_x,
                    ] == 0
                    or sample_y
                    > target_heights[
                        cell_z,
                        cell_x,
                    ]
                ):
                    target_resolved[
                        cell_z,
                        cell_x,
                    ] = 1
                    target_heights[
                        cell_z,
                        cell_x,
                    ] = sample_y

    resolved = np.maximum(
        preferred_resolved,
        fallback_resolved,
    )
    heights = np.where(
        preferred_resolved != 0,
        preferred_heights,
        fallback_heights,
    ).astype(np.float32)
    fallback_only = np.logical_and(
        preferred_resolved == 0,
        fallback_resolved != 0,
    )

    return (
        origin_x,
        origin_z,
        width,
        height,
        resolved,
        heights,
        int(preferred_resolved.sum()),
        int(fallback_only.sum()),
    )
```

### 최종 교체 코드: writer 전체

기존 `write_navgrid()`는 삭제하고 아래 함수로 교체한다.

```python
def write_navsource(
    output: Path,
    area_id: str,
    width: int,
    height: int,
    cell_size: float,
    origin_x: float,
    origin_z: float,
    resolved: np.ndarray,
    heights: np.ndarray,
) -> None:
    cell_count = width * height
    lines = [
        (
            "LOSTARK_NAVGRID_SOURCE 1 "
            + json.dumps(
                area_id,
                ensure_ascii=False,
            )
            + f" {width} {height}"
            + f" {cell_size:.9g}"
            + f" {origin_x:.9g}"
            + f" {origin_z:.9g}"
            + f" {cell_count}"
        )
    ]

    for cell_z in range(height):
        for cell_x in range(width):
            is_resolved = int(
                resolved[cell_z, cell_x]
            )
            cell_height = (
                float(heights[cell_z, cell_x])
                if is_resolved
                else 0.0
            )
            lines.append(
                f"{cell_x} {cell_z} "
                f"{is_resolved} "
                f"{cell_height:.9g}"
            )

    output.parent.mkdir(
        parents=True,
        exist_ok=True,
    )
    temporary = output.with_suffix(
        output.suffix + ".tmp"
    )
    try:
        temporary.write_text(
            "\n".join(lines) + "\n",
            encoding="utf-8",
            newline="\n",
        )
        temporary.replace(output)
    finally:
        if temporary.exists():
            temporary.unlink()
```

### 최종 교체 코드: `main()` 전체

```python
def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--floor01",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--floor01a",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--floor01b",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--main-floor",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--center-floor",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--overlay",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--mapplacements",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--bounds-gltf",
        required=True,
        type=Path,
    )
    parser.add_argument(
        "--cell-size",
        type=float,
        default=0.5,
    )
    parser.add_argument(
        "--preferred-floor-slope",
        type=float,
        default=45.0,
    )
    parser.add_argument(
        "--area-id",
        default="LV_LUT_HEARTRB_ED",
    )
    parser.add_argument(
        "--output",
        required=True,
        type=Path,
    )
    args = parser.parse_args()

    input_paths = [
        args.floor01,
        args.floor01a,
        args.floor01b,
        args.main_floor,
        args.center_floor,
        args.overlay,
        args.mapplacements,
        args.bounds_gltf,
    ]
    missing_inputs = [
        path
        for path in input_paths
        if not path.is_file()
    ]
    if missing_inputs:
        parser.error(
            "input files do not exist: "
            + ", ".join(
                str(path)
                for path in missing_inputs
            )
        )
    if (
        not math.isfinite(args.cell_size)
        or args.cell_size <= 0
    ):
        parser.error(
            "cell-size must be positive"
        )
    if (
        not math.isfinite(
            args.preferred_floor_slope
        )
        or not 0
        <= args.preferred_floor_slope
        < 90
    ):
        parser.error(
            "preferred-floor-slope "
            "must be in [0, 90)"
        )
    if not args.area_id:
        parser.error(
            "area-id must not be empty"
        )

    source_paths = {
        "BG_RAD_VALTAN_FLOOR01_SM":
            args.floor01,
        "BG_RAD_VALTAN_FLOOR01A_SM":
            args.floor01a,
        "BG_RAD_VALTAN_FLOOR01B_SM":
            args.floor01b,
        MAIN_FLOOR_ASSET_ID:
            args.main_floor,
        CENTER_FLOOR_ASSET_ID:
            args.center_floor,
    }
    source_triangles = {
        asset_id: load_triangles(path)
        for asset_id, path
        in source_paths.items()
    }
    world_triangle_blocks: list[
        np.ndarray
    ] = []

    for placement in load_overlay_placements(
        args.overlay
    ):
        asset_id = placement["assetId"]
        world_triangle_blocks.append(
            apply_placement(
                source_triangles[asset_id],
                placement_matrix(placement),
            )
        )

    exact_placements = load_exact_placements(
        args.mapplacements
    )
    for source_id in (
        MAIN_FLOOR_SOURCE_PLACEMENT_IDS
    ):
        world_triangle_blocks.append(
            apply_placement(
                source_triangles[
                    MAIN_FLOOR_ASSET_ID
                ],
                placement_matrix(
                    exact_placements[source_id]
                ),
            )
        )

    world_triangle_blocks.append(
        apply_placement(
            source_triangles[
                CENTER_FLOOR_ASSET_ID
            ],
            placement_matrix(
                exact_placements[
                    CENTER_FLOOR_SOURCE_PLACEMENT_ID
                ]
            ),
        )
    )
    floor_triangles = np.concatenate(
        world_triangle_blocks,
        axis=0,
    )

    bounds_source = load_triangles(
        args.bounds_gltf
    )
    bounds_world = apply_placement(
        bounds_source,
        placement_matrix(
            exact_placements[
                BOUNDS_SOURCE_PLACEMENT_ID
            ]
        ),
    )
    bounds_min = bounds_world.min(
        axis=(0, 1)
    )
    bounds_max = bounds_world.max(
        axis=(0, 1)
    )

    (
        origin_x,
        origin_z,
        width,
        height,
        height_resolved,
        heights,
        preferred_height_cells,
        fallback_height_cells,
    ) = rasterize_height_source(
        floor_triangles,
        bounds_min,
        bounds_max,
        args.cell_size,
        args.preferred_floor_slope,
    )

    resolved_height_cells = int(
        height_resolved.sum()
    )
    if resolved_height_cells == 0:
        raise ValueError(
            "bake produced no height samples"
        )

    write_navsource(
        args.output,
        args.area_id,
        width,
        height,
        args.cell_size,
        origin_x,
        origin_z,
        height_resolved,
        heights,
    )

    resolved_heights = heights[
        height_resolved != 0
    ]
    result = {
        "areaId": args.area_id,
        "width": width,
        "height": height,
        "cellSize": args.cell_size,
        "origin": [
            origin_x,
            origin_z,
        ],
        "boundsMin": bounds_min.tolist(),
        "boundsMax": bounds_max.tolist(),
        "triangleCount": int(
            len(floor_triangles)
        ),
        "preferredHeightCells":
            preferred_height_cells,
        "fallbackHeightCells":
            fallback_height_cells,
        "resolvedHeightCells":
            resolved_height_cells,
        "unresolvedHeightCells":
            width * height
            - resolved_height_cells,
        "minResolvedHeight": float(
            resolved_heights.min()
        ),
        "maxResolvedHeight": float(
            resolved_heights.max()
        ),
        "output": str(
            args.output.resolve()
        ),
    }
    print(
        json.dumps(
            result,
            ensure_ascii=False,
            indent=2,
        )
    )
    return 0
```

기존 helper와 `if __name__ == "__main__":` 진입 코드는 변경하지 않는다.

## 7.2 `NavGridPaintDocument.h`

### 본질·책임

1. 한 문장 본질: 높이 source와 수동 blocked 정본을 검증된 메모리 상태로 유지한다.
2. 역할: Load, block/unblock, 상태 query, paint Save, runtime Export.
3. 비역할: ImGui, 화면 cursor, DirectX draw, A*.
4. 의존성: `Client_Defines`와 표준 filesystem/vector만 사용한다.
5. owner: `CMapTool` 값 멤버 하나.
6. public 함수: UI가 필요한 명령과 읽기 query만 공개한다.
7. 실패: out status를 채우고 기존 메모리 또는 기존 파일을 보존한다.
8. 헤더/CPP 경계: 구조와 계약은 헤더, parser/atomic I/O는 CPP다.

### 새 파일 전체 코드

```cpp
#pragma once

#include "Client_Defines.h"

#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

enum class NAVGRID_AUTHORING_CELL_STATE : uint8_t
{
	WALKABLE,
	BLOCKED,
	UNRESOLVED,
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
	bool_t Is_ValidCell(
		int32_t cellX,
		int32_t cellZ) const;
	bool_t World_ToCell(
		fvector_t worldPosition,
		int32_t& outCellX,
		int32_t& outCellZ) const;
	uint32_t To_Index(
		int32_t cellX,
		int32_t cellZ) const;
	NAVGRID_AUTHORING_CELL_STATE Get_CellState(
		uint32_t index) const;
	f32_t Get_DisplayHeight(uint32_t index) const;
	uint32_t Get_CellCount() const;
	uint32_t Get_BlockedCount() const;
	uint32_t Get_ResolvedHeightCount() const;
	uint32_t Get_UnresolvedWalkableCount() const;
	const NAVGRID_AUTHORING_DESC& Get_Desc() const
	{
		return m_Desc;
	}

private:
	static bool_t Build_DisplayHeights(
		const NAVGRID_AUTHORING_DESC& desc,
		const std::vector<NAV_SOURCE_CELL>& sourceCells,
		std::vector<f32_t>& outDisplayHeights);

private:
	NAVGRID_AUTHORING_DESC m_Desc;
	std::vector<NAV_SOURCE_CELL> m_SourceCells;
	std::vector<uint8_t> m_BlockedCells;
	std::vector<f32_t> m_DisplayHeights;
	bool_t m_isReady = false;
	bool_t m_isDirty = false;
};

NS_END
```

## 7.3 `NavGridPaintDocument.cpp`

### 본질·책임

1. 한 문장 본질: 파일 전체를 먼저 stage·검증하고 성공한 상태만 commit한다.
2. 역할: source/paint parser, identity 검사, display height 파생, atomic writer.
3. 비역할: 파일을 프레임마다 읽기, runtime grid를 직접 mutate하기.
4. 의존성 방향: `CMapTool -> CNavGridPaintDocument -> filesystem`.
5. 자료구조: 모든 staging vector는 Load 함수 지역이고 성공 끝에서 move한다.
6. 함수 실패: false와 status, 기존 document/file 불변.
7. 알고리즘: Load O(N²)는 display height 최근접 계산 때문에 세션 진입 1회만 수행한다.
8. 경계: binary export는 현재 Windows little-endian `CNavGrid::Load` 계약과 정확히 같다.

### 새 파일 전체 코드

```cpp
#include "NavGridPaintDocument.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>

namespace
{
	constexpr const char* SOURCE_MAGIC =
		"LOSTARK_NAVGRID_SOURCE";
	constexpr const char* PAINT_MAGIC =
		"LOSTARK_NAVGRID_PAINT";
	constexpr uint32_t SOURCE_VERSION = 1;
	constexpr uint32_t PAINT_VERSION = 1;

	bool_t IsSameFloat(f32_t left, f32_t right)
	{
		return std::fabs(left - right) <= 0.000001f;
	}

	bool_t CommitTemporaryFile(
		const std::filesystem::path& destination,
		const std::filesystem::path& temporary)
	{
		std::error_code existsError;
		if (std::filesystem::exists(
			destination,
			existsError) &&
			!existsError &&
			ReplaceFileW(
				destination.c_str(),
				temporary.c_str(),
				nullptr,
				REPLACEFILE_WRITE_THROUGH,
				nullptr,
				nullptr))
		{
			return true;
		}

		return MoveFileExW(
			temporary.c_str(),
			destination.c_str(),
			MOVEFILE_REPLACE_EXISTING |
			MOVEFILE_WRITE_THROUGH);
	}

	void RemoveTemporaryFile(
		const std::filesystem::path& temporary)
	{
		std::error_code removeError;
		std::filesystem::remove(
			temporary,
			removeError);
	}
}

bool_t Client::CNavGridPaintDocument::Load(
	const std::filesystem::path& sourcePath,
	const std::filesystem::path& paintPath,
	std::string& outStatus)
{
	std::ifstream source(
		sourcePath,
		std::ios::binary);
	if (!source)
	{
		outStatus =
			"Could not open NavGrid source";
		return false;
	}

	std::string sourceMagic;
	uint32_t sourceVersion = {};
	NAVGRID_AUTHORING_DESC stagedDesc;
	uint64_t declaredCellCount = {};
	if (!(source >>
		sourceMagic >>
		sourceVersion >>
		std::quoted(stagedDesc.areaId) >>
		stagedDesc.width >>
		stagedDesc.height >>
		stagedDesc.cellSize >>
		stagedDesc.originX >>
		stagedDesc.originZ >>
		declaredCellCount) ||
		sourceMagic != SOURCE_MAGIC ||
		sourceVersion != SOURCE_VERSION ||
		stagedDesc.areaId.empty() ||
		0 == stagedDesc.width ||
		0 == stagedDesc.height ||
		!std::isfinite(stagedDesc.cellSize) ||
		stagedDesc.cellSize <= 0.f ||
		!std::isfinite(stagedDesc.originX) ||
		!std::isfinite(stagedDesc.originZ))
	{
		outStatus =
			"NavGrid source header is invalid";
		return false;
	}

	const uint64_t cellCount64 =
		static_cast<uint64_t>(
			stagedDesc.width) *
		stagedDesc.height;
	if (0 == cellCount64 ||
		cellCount64 > MAX_CELL_COUNT ||
		cellCount64 != declaredCellCount)
	{
		outStatus =
			"NavGrid source cell count is invalid";
		return false;
	}

	const uint32_t cellCount =
		static_cast<uint32_t>(
			cellCount64);
	std::vector<NAV_SOURCE_CELL>
		stagedSourceCells(cellCount);
	std::vector<uint8_t>
		stagedSeen(cellCount, 0);

	for (uint32_t row = 0;
		row < cellCount;
		++row)
	{
		int32_t cellX = {};
		int32_t cellZ = {};
		uint32_t heightResolved = {};
		f32_t height = {};
		if (!(source >>
			cellX >>
			cellZ >>
			heightResolved >>
			height) ||
			cellX < 0 ||
			cellZ < 0 ||
			cellX >= static_cast<int32_t>(
				stagedDesc.width) ||
			cellZ >= static_cast<int32_t>(
				stagedDesc.height) ||
			heightResolved > 1 ||
			!std::isfinite(height))
		{
			outStatus =
				"NavGrid source row is invalid";
			return false;
		}

		const uint32_t index =
			static_cast<uint32_t>(cellZ) *
			stagedDesc.width +
			static_cast<uint32_t>(cellX);
		if (0 != stagedSeen[index])
		{
			outStatus =
				"NavGrid source has duplicate cells";
			return false;
		}

		stagedSeen[index] = 1;
		stagedSourceCells[index].heightResolved =
			0 != heightResolved;
		stagedSourceCells[index].height =
			0 != heightResolved ? height : 0.f;
	}

	source >> std::ws;
	if (source.peek() !=
		std::char_traits<char>::eof())
	{
		outStatus =
			"NavGrid source has trailing data";
		return false;
	}

	std::vector<uint8_t>
		stagedBlockedCells(cellCount, 0);
	std::error_code existsError;
	const bool_t paintExists =
		std::filesystem::exists(
			paintPath,
			existsError);
	if (existsError)
	{
		outStatus =
			"Could not inspect NavGrid paint file";
		return false;
	}

	if (paintExists)
	{
		std::ifstream paint(
			paintPath,
			std::ios::binary);
		std::string paintMagic;
		uint32_t paintVersion = {};
		std::string paintAreaId;
		uint32_t paintWidth = {};
		uint32_t paintHeight = {};
		f32_t paintCellSize = {};
		f32_t paintOriginX = {};
		f32_t paintOriginZ = {};
		uint64_t blockedCount = {};

		if (!paint ||
			!(paint >>
				paintMagic >>
				paintVersion >>
				std::quoted(paintAreaId) >>
				paintWidth >>
				paintHeight >>
				paintCellSize >>
				paintOriginX >>
				paintOriginZ >>
				blockedCount) ||
			paintMagic != PAINT_MAGIC ||
			paintVersion != PAINT_VERSION ||
			paintAreaId != stagedDesc.areaId ||
			paintWidth != stagedDesc.width ||
			paintHeight != stagedDesc.height ||
			!IsSameFloat(
				paintCellSize,
				stagedDesc.cellSize) ||
			!IsSameFloat(
				paintOriginX,
				stagedDesc.originX) ||
			!IsSameFloat(
				paintOriginZ,
				stagedDesc.originZ) ||
			blockedCount > cellCount)
		{
			outStatus =
				"NavGrid paint header does not match source";
			return false;
		}

		for (uint64_t row = 0;
			row < blockedCount;
			++row)
		{
			int32_t cellX = {};
			int32_t cellZ = {};
			if (!(paint >> cellX >> cellZ) ||
				cellX < 0 ||
				cellZ < 0 ||
				cellX >= static_cast<int32_t>(
					stagedDesc.width) ||
				cellZ >= static_cast<int32_t>(
					stagedDesc.height))
			{
				outStatus =
					"NavGrid paint row is invalid";
				return false;
			}

			const uint32_t index =
				static_cast<uint32_t>(cellZ) *
				stagedDesc.width +
				static_cast<uint32_t>(cellX);
			if (0 != stagedBlockedCells[index])
			{
				outStatus =
					"NavGrid paint has duplicate cells";
				return false;
			}
			stagedBlockedCells[index] = 1;
		}

		paint >> std::ws;
		if (paint.peek() !=
			std::char_traits<char>::eof())
		{
			outStatus =
				"NavGrid paint has trailing data";
			return false;
		}
	}

	std::vector<f32_t> stagedDisplayHeights;
	if (!Build_DisplayHeights(
		stagedDesc,
		stagedSourceCells,
		stagedDisplayHeights))
	{
		outStatus =
			"NavGrid source has no usable height";
		return false;
	}

	m_Desc = std::move(stagedDesc);
	m_SourceCells =
		std::move(stagedSourceCells);
	m_BlockedCells =
		std::move(stagedBlockedCells);
	m_DisplayHeights =
		std::move(stagedDisplayHeights);
	m_isReady = true;
	m_isDirty = false;

	outStatus =
		paintExists
		? "Loaded NavGrid source and paint"
		: "Loaded NavGrid source; all cells start walkable";
	return true;
}

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

	const int32_t radius =
		static_cast<int32_t>(brushRadius);
	const int32_t radiusSquared =
		radius * radius;
	bool_t changed = false;

	for (int32_t offsetZ = -radius;
		offsetZ <= radius;
		++offsetZ)
	{
		for (int32_t offsetX = -radius;
			offsetX <= radius;
			++offsetX)
		{
			if (offsetX * offsetX +
				offsetZ * offsetZ >
				radiusSquared)
			{
				continue;
			}

			const int32_t targetX =
				cellX + offsetX;
			const int32_t targetZ =
				cellZ + offsetZ;
			if (!Is_ValidCell(
				targetX,
				targetZ))
			{
				continue;
			}

			const uint32_t index =
				To_Index(targetX, targetZ);
			const uint8_t newValue =
				walkable ? 0 : 1;
			if (m_BlockedCells[index] !=
				newValue)
			{
				m_BlockedCells[index] =
					newValue;
				changed = true;
			}
		}
	}

	if (changed)
		m_isDirty = true;
	return changed;
}

bool_t Client::CNavGridPaintDocument::Save_Paint(
	const std::filesystem::path& paintPath,
	std::string& outStatus)
{
	if (!m_isReady)
	{
		outStatus =
			"NavGrid document is not loaded";
		return false;
	}

	std::error_code directoryError;
	std::filesystem::create_directories(
		paintPath.parent_path(),
		directoryError);
	if (directoryError)
	{
		outStatus =
			"Could not create NavGrid paint directory";
		return false;
	}

	const std::filesystem::path temporary =
		paintPath.wstring() + L".tmp";
	std::ofstream output(
		temporary,
		std::ios::binary |
		std::ios::trunc);
	if (!output)
	{
		outStatus =
			"Could not create temporary NavGrid paint";
		return false;
	}

	const uint32_t blockedCount =
		Get_BlockedCount();
	output << PAINT_MAGIC << ' ' <<
		PAINT_VERSION << ' ' <<
		std::quoted(m_Desc.areaId) << ' ' <<
		m_Desc.width << ' ' <<
		m_Desc.height << ' ';
	output << std::setprecision(9) <<
		m_Desc.cellSize << ' ' <<
		m_Desc.originX << ' ' <<
		m_Desc.originZ << ' ' <<
		blockedCount << '\n';

	for (uint32_t cellZ = 0;
		cellZ < m_Desc.height;
		++cellZ)
	{
		for (uint32_t cellX = 0;
			cellX < m_Desc.width;
			++cellX)
		{
			const uint32_t index =
				cellZ * m_Desc.width +
				cellX;
			if (0 != m_BlockedCells[index])
			{
				output <<
					cellX << ' ' <<
					cellZ << '\n';
			}
		}
	}

	output.flush();
	const bool_t wroteSuccessfully =
		output.good();
	output.close();
	if (!wroteSuccessfully ||
		!CommitTemporaryFile(
			paintPath,
			temporary))
	{
		RemoveTemporaryFile(temporary);
		outStatus =
			"Failed to commit NavGrid paint atomically";
		return false;
	}

	m_isDirty = false;
	outStatus =
		"Saved NavGrid paint: " +
		std::to_string(blockedCount) +
		" blocked cells";
	return true;
}

bool_t Client::CNavGridPaintDocument::Export_Runtime(
	const std::filesystem::path& runtimePath,
	std::string& outStatus) const
{
	if (!m_isReady)
	{
		outStatus =
			"NavGrid document is not loaded";
		return false;
	}

	const uint32_t unresolvedWalkable =
		Get_UnresolvedWalkableCount();
	if (0 != unresolvedWalkable)
	{
		outStatus =
			"Runtime export rejected: " +
			std::to_string(
				unresolvedWalkable) +
			" walkable cells have no height";
		return false;
	}

	std::error_code directoryError;
	std::filesystem::create_directories(
		runtimePath.parent_path(),
		directoryError);
	if (directoryError)
	{
		outStatus =
			"Could not create NavGrid runtime directory";
		return false;
	}

	const uint32_t cellCount =
		Get_CellCount();
	std::vector<uint8_t>
		walkable(cellCount, 0);
	std::vector<f32_t>
		heights(cellCount, 0.f);
	for (uint32_t index = 0;
		index < cellCount;
		++index)
	{
		const bool_t isWalkable =
			0 == m_BlockedCells[index];
		walkable[index] =
			isWalkable ? 1 : 0;
		if (m_SourceCells[index].
			heightResolved)
		{
			heights[index] =
				m_SourceCells[index].height;
		}
	}

	const std::filesystem::path temporary =
		runtimePath.wstring() + L".tmp";
	std::ofstream output(
		temporary,
		std::ios::binary |
		std::ios::trunc);
	if (!output)
	{
		outStatus =
			"Could not create temporary runtime NavGrid";
		return false;
	}

	output.write(
		reinterpret_cast<const char*>(
			&m_Desc.width),
		sizeof(uint32_t));
	output.write(
		reinterpret_cast<const char*>(
			&m_Desc.height),
		sizeof(uint32_t));
	output.write(
		reinterpret_cast<const char*>(
			&m_Desc.cellSize),
		sizeof(f32_t));
	output.write(
		reinterpret_cast<const char*>(
			&m_Desc.originX),
		sizeof(f32_t));
	output.write(
		reinterpret_cast<const char*>(
			&m_Desc.originZ),
		sizeof(f32_t));
	output.write(
		reinterpret_cast<const char*>(
			walkable.data()),
		static_cast<std::streamsize>(
			walkable.size()));
	output.write(
		reinterpret_cast<const char*>(
			heights.data()),
		static_cast<std::streamsize>(
			heights.size() *
			sizeof(f32_t)));
	output.flush();
	const bool_t wroteSuccessfully =
		output.good();
	output.close();

	if (!wroteSuccessfully ||
		!CommitTemporaryFile(
			runtimePath,
			temporary))
	{
		RemoveTemporaryFile(temporary);
		outStatus =
			"Failed to commit runtime NavGrid atomically";
		return false;
	}

	outStatus =
		"Exported runtime NavGrid: " +
		std::to_string(
			cellCount -
			Get_BlockedCount()) +
		" walkable cells";
	return true;
}

bool_t Client::CNavGridPaintDocument::Is_ValidCell(
	int32_t cellX,
	int32_t cellZ) const
{
	return m_isReady &&
		cellX >= 0 &&
		cellZ >= 0 &&
		cellX < static_cast<int32_t>(
			m_Desc.width) &&
		cellZ < static_cast<int32_t>(
			m_Desc.height);
}

bool_t Client::CNavGridPaintDocument::World_ToCell(
	fvector_t worldPosition,
	int32_t& outCellX,
	int32_t& outCellZ) const
{
	if (!m_isReady ||
		m_Desc.cellSize <= 0.f)
	{
		return false;
	}

	const f32_t worldX =
		XMVectorGetX(worldPosition);
	const f32_t worldZ =
		XMVectorGetZ(worldPosition);
	if (!std::isfinite(worldX) ||
		!std::isfinite(worldZ))
	{
		return false;
	}

	const f32_t cellX =
		(worldX - m_Desc.originX) /
		m_Desc.cellSize;
	const f32_t cellZ =
		(worldZ - m_Desc.originZ) /
		m_Desc.cellSize;
	if (!std::isfinite(cellX) ||
		!std::isfinite(cellZ) ||
		cellX < 0.f ||
		cellZ < 0.f ||
		cellX >= static_cast<f32_t>(
			m_Desc.width) ||
		cellZ >= static_cast<f32_t>(
			m_Desc.height))
	{
		return false;
	}

	outCellX =
		static_cast<int32_t>(
			std::floor(cellX));
	outCellZ =
		static_cast<int32_t>(
			std::floor(cellZ));
	return Is_ValidCell(
		outCellX,
		outCellZ);
}

uint32_t Client::CNavGridPaintDocument::To_Index(
	int32_t cellX,
	int32_t cellZ) const
{
	return static_cast<uint32_t>(
		cellZ) *
		m_Desc.width +
		static_cast<uint32_t>(
			cellX);
}

NAVGRID_AUTHORING_CELL_STATE
Client::CNavGridPaintDocument::Get_CellState(
	uint32_t index) const
{
	if (!m_isReady ||
		index >= m_SourceCells.size())
	{
		return
			NAVGRID_AUTHORING_CELL_STATE::
			UNRESOLVED;
	}
	if (0 != m_BlockedCells[index])
	{
		return
			NAVGRID_AUTHORING_CELL_STATE::
			BLOCKED;
	}
	if (!m_SourceCells[index].
		heightResolved)
	{
		return
			NAVGRID_AUTHORING_CELL_STATE::
			UNRESOLVED;
	}
	return
		NAVGRID_AUTHORING_CELL_STATE::
		WALKABLE;
}

f32_t Client::CNavGridPaintDocument::Get_DisplayHeight(
	uint32_t index) const
{
	if (index >= m_DisplayHeights.size())
		return 0.f;
	return m_DisplayHeights[index];
}

uint32_t Client::CNavGridPaintDocument::Get_CellCount() const
{
	return static_cast<uint32_t>(
		m_SourceCells.size());
}

uint32_t Client::CNavGridPaintDocument::Get_BlockedCount() const
{
	return static_cast<uint32_t>(
		std::count(
			m_BlockedCells.begin(),
			m_BlockedCells.end(),
			static_cast<uint8_t>(1)));
}

uint32_t Client::CNavGridPaintDocument::Get_ResolvedHeightCount() const
{
	return static_cast<uint32_t>(
		std::count_if(
			m_SourceCells.begin(),
			m_SourceCells.end(),
			[](const NAV_SOURCE_CELL& cell)
			{
				return cell.heightResolved;
			}));
}

uint32_t Client::CNavGridPaintDocument::Get_UnresolvedWalkableCount() const
{
	uint32_t count = {};
	for (size_t index = 0;
		index < m_SourceCells.size();
		++index)
	{
		if (0 == m_BlockedCells[index] &&
			!m_SourceCells[index].
			heightResolved)
		{
			++count;
		}
	}
	return count;
}

bool_t Client::CNavGridPaintDocument::Build_DisplayHeights(
	const NAVGRID_AUTHORING_DESC& desc,
	const std::vector<NAV_SOURCE_CELL>& sourceCells,
	std::vector<f32_t>& outDisplayHeights)
{
	if (sourceCells.empty())
		return false;

	outDisplayHeights.assign(
		sourceCells.size(),
		0.f);
	uint32_t resolvedCount = {};
	for (size_t index = 0;
		index < sourceCells.size();
		++index)
	{
		if (sourceCells[index].
			heightResolved)
		{
			outDisplayHeights[index] =
				sourceCells[index].height;
			++resolvedCount;
		}
	}
	if (0 == resolvedCount)
		return false;

	for (uint32_t index = 0;
		index < sourceCells.size();
		++index)
	{
		if (sourceCells[index].
			heightResolved)
		{
			continue;
		}

		const int32_t sourceX =
			static_cast<int32_t>(
				index % desc.width);
		const int32_t sourceZ =
			static_cast<int32_t>(
				index / desc.width);
		uint64_t bestDistance =
			(std::numeric_limits<
				uint64_t>::max)();
		f64_t heightSum = {};
		uint32_t heightCount = {};

		for (uint32_t candidate = 0;
			candidate < sourceCells.size();
			++candidate)
		{
			if (!sourceCells[candidate].
				heightResolved)
			{
				continue;
			}

			const int64_t deltaX =
				static_cast<int64_t>(
					candidate %
					desc.width) -
				sourceX;
			const int64_t deltaZ =
				static_cast<int64_t>(
					candidate /
					desc.width) -
				sourceZ;
			const uint64_t distance =
				static_cast<uint64_t>(
					deltaX * deltaX +
					deltaZ * deltaZ);
			if (distance < bestDistance)
			{
				bestDistance = distance;
				heightSum =
					sourceCells[candidate].
					height;
				heightCount = 1;
			}
			else if (distance ==
				bestDistance)
			{
				heightSum +=
					sourceCells[candidate].
					height;
				++heightCount;
			}
		}

		if (0 == heightCount)
			return false;
		outDisplayHeights[index] =
			static_cast<f32_t>(
				heightSum /
				heightCount);
	}
	return true;
}
```

## 7.4 `MapTool.h`

### 본질·책임

1. 한 문장 본질: 사용자의 F1 선택과 viewport 입력을 문서 명령으로 변환한다.
2. 역할: Map/Navigation mode 배타성, LMB state, grid pick, UI/overlay 호출.
3. 비역할: source parser, A*, actor Transform.
4. 기존 접점: `CMainApp`이 생성·소유하고 Engine update 뒤 `Update`, render 뒤 `Render`한다.
5. 수명: 앱 Debug 수명. Level 전환 때 runtime placement와 navigation 문서를 재로드한다.
6. 입력: Win32 LMB는 tool이 소비하고 `CMainApp`이 DirectInput 게임 LMB를 차단한다.
7. 의존성: `CMapTool -> CNavGridPaintDocument`, 역방향 의존 없음.
8. 헤더에는 상태/계약만 두고 DirectX 리소스 세부는 CPP의 private struct로 숨긴다.

### include 추가

```cpp
#include "NavGridPaintDocument.h"
```

### class 선언에 추가할 최종 블록

`PLACEMENT_STATE` 앞에 mode를 추가한다.

```cpp
	enum class TOOL_MODE
	{
		MAP_ASSETS,
		NAVIGATION,
	};

	enum class PAINT_MODE
	{
		BLOCKED,
		WALKABLE,
	};

	struct NAVIGATION_RENDER_RESOURCES;
```

public 계약에는 아래 함수를 추가한다.

```cpp
	bool_t ConsumesWorldMouse() const;
```

private 함수 선언에는 아래 블록을 추가한다.

```cpp
	bool_t Load_NavigationDocument();
	bool_t Try_PickNavigationCell(
		int32_t& outCellX,
		int32_t& outCellZ) const;
	bool_t Try_PaintNavigation();
	void Render_ModeBar();
	void Render_NavigationPanel();
	void Render_NavigationOverlay();
```

private 데이터에는 아래 블록을 추가한다.

```cpp
	TOOL_MODE m_eToolMode =
		TOOL_MODE::MAP_ASSETS;
	PAINT_MODE m_ePaintMode =
		PAINT_MODE::BLOCKED;
	uint32_t m_iBrushRadius = {};
	CNavGridPaintDocument m_NavigationDocument;
	std::filesystem::path m_NavigationSourcePath;
	std::filesystem::path m_NavigationPaintPath;
	std::filesystem::path m_NavigationRuntimePath;
	std::string m_NavigationStatus =
		"Enter AssetTest with F2";
	std::unique_ptr<NAVIGATION_RENDER_RESOURCES>
		m_pNavigationRenderResources;
```

기존 placement 자료구조와 함수는 그대로 유지한다.

## 7.5 `MapTool.cpp`

### 본질·의존성

MapTool CPP는 UI·피킹·overlay 절차만 가진다. `m_NavigationDocument`의 vector를 직접
수정하거나 파일 형식을 해석하지 않는다. 렌더는 authoring 관찰용이며 F5 runtime
`CNavigation`과 다른 데이터를 소유하지 않는다.

### anonymous namespace에 추가

```cpp
	std::filesystem::path GetNavigationDataRoot()
	{
		wchar_t modulePath[32768]{};
		const DWORD length = GetModuleFileNameW(
			nullptr,
			modulePath,
			static_cast<DWORD>(
				std::size(modulePath)));
		if (0 == length ||
			length >= std::size(modulePath))
		{
			return {};
		}

		const std::filesystem::path moduleDirectory =
			std::filesystem::path(
				modulePath).parent_path();
		const std::filesystem::path adjacent =
			moduleDirectory /
			L"DataFiles" /
			L"Navigation";
		if (std::filesystem::exists(adjacent))
			return adjacent.lexically_normal();

		return (
			moduleDirectory.parent_path() /
			L"DataFiles" /
			L"Navigation").lexically_normal();
	}
```

### private render resource 정의 추가

anonymous namespace 뒤에 둔다.

```cpp
struct Client::CMapTool::NAVIGATION_RENDER_RESOURCES final
{
	shared_ptr<
		PrimitiveBatch<VertexPositionColor>>
		pBatch;
	shared_ptr<BasicEffect> pEffect;
	ComPtr<ID3D11InputLayout> pInputLayout;
};
```

### `Initialize()` 전체 교체

```cpp
HRESULT Client::CMapTool::Initialize(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto preview =
		std::make_unique<CMapAssetPreview>();
	if (FAILED(preview->Initialize(
		pDevice,
		pContext)))
	{
		return E_FAIL;
	}

	auto navigationResources =
		std::make_unique<
			NAVIGATION_RENDER_RESOURCES>();
	navigationResources->pBatch =
		make_shared<
			PrimitiveBatch<
				VertexPositionColor>>(
			pContext.Get());
	navigationResources->pEffect =
		make_shared<BasicEffect>(
			pDevice.Get());
	navigationResources->pEffect->
		SetVertexColorEnabled(true);

	const void* vertexShaderByteCode =
		nullptr;
	size_t byteCodeLength = {};
	navigationResources->pEffect->
		GetVertexShaderBytecode(
			&vertexShaderByteCode,
			&byteCodeLength);
	if (FAILED(pDevice->CreateInputLayout(
		VertexPositionColor::InputElements,
		VertexPositionColor::
			InputElementCount,
		vertexShaderByteCode,
		byteCodeLength,
		navigationResources->
			pInputLayout.GetAddressOf())))
	{
		return E_FAIL;
	}

	m_pAssetPreview = std::move(preview);
	m_pNavigationRenderResources =
		std::move(navigationResources);
	return S_OK;
}
```

### `ConsumesWorldMouse()` 추가

```cpp
bool_t Client::CMapTool::ConsumesWorldMouse() const
{
	if (!m_bOpen ||
		ETOUI(LEVEL::ASSET_TEST) !=
		CGameInstance::Get().
		Get_CurrentLevelID())
	{
		return false;
	}

	return TOOL_MODE::NAVIGATION ==
		m_eToolMode ||
		PLACEMENT_STATE::ARMED ==
		m_ePlacementState;
}
```

### `Update()` 전체 교체

```cpp
void Client::CMapTool::Update(f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);

	const bool_t isAssetTest =
		ETOUI(LEVEL::ASSET_TEST) ==
		CGameInstance::Get().
		Get_CurrentLevelID();
	Handle_LevelTransition(isAssetTest);

	if (isAssetTest &&
		GetForegroundWindow() == g_hWnd)
	{
		if (0 != (
			GetAsyncKeyState(VK_F7) & 1))
		{
			Set_EnvironmentPhase(
				ENVIRONMENT_PHASE::BASELINE);
			m_Status =
				"Sky phase: Baseline (F7)";
		}
		else if (0 != (
			GetAsyncKeyState(VK_F8) & 1))
		{
			Set_EnvironmentPhase(
				ENVIRONMENT_PHASE::SPACEHOLE);
			m_Status =
				"Sky phase: SpaceHole (F8)";
		}
		else if (0 != (
			GetAsyncKeyState(VK_F9) & 1))
		{
			Set_EnvironmentPhase(
				ENVIRONMENT_PHASE::CHAOS_GATE);
			m_Status =
				"Sky phase: ChaosGate (F9)";
		}
	}

	const bool_t mouseDown =
		0 != (
			GetAsyncKeyState(VK_LBUTTON) &
			0x8000);
	const bool_t mousePressed =
		mouseDown &&
		!m_bPreviousMouseDown;
	m_bPreviousMouseDown = mouseDown;

	if (!m_bOpen || !isAssetTest)
		return;

	if (ImGui::IsKeyPressed(
		ImGuiKey_Escape,
		false))
	{
		m_ePlacementState =
			PLACEMENT_STATE::IDLE;
		m_Status = "Placement cancelled";
	}

	const bool_t canUseWorldMouse =
		GetForegroundWindow() == g_hWnd &&
		!ImGui::GetIO().WantCaptureMouse;
	if (!canUseWorldMouse)
		return;

	if (TOOL_MODE::NAVIGATION ==
		m_eToolMode)
	{
		if (mouseDown)
			Try_PaintNavigation();
		return;
	}

	if (PLACEMENT_STATE::ARMED ==
		m_ePlacementState &&
		mousePressed)
	{
		Try_PlaceSelected();
	}
}
```

### `Render()` 전체 교체

```cpp
void Client::CMapTool::Render()
{
	if (!m_bOpen)
		return;

	const bool_t isAssetTest =
		ETOUI(LEVEL::ASSET_TEST) ==
		CGameInstance::Get().
		Get_CurrentLevelID();
	if (isAssetTest &&
		TOOL_MODE::NAVIGATION ==
		m_eToolMode)
	{
		Render_NavigationOverlay();
	}

	ImGui::SetNextWindowSize(
		ImVec2(1180.f, 900.f),
		ImGuiCond_FirstUseEver);
	if (!ImGui::Begin(
		"LostArk Map Tool",
		&m_bOpen))
	{
		ImGui::End();
		return;
	}

	ImGui::Text(
		"Level: %s",
		isAssetTest
		? "ASSET_TEST"
		: "Open ASSET_TEST with F2");
	ImGui::SameLine();
	ImGui::Text(
		"| Catalog: %s",
		m_Catalog.Is_Ready()
		? "READY"
		: "NOT READY");
	Render_ModeBar();
	ImGui::TextWrapped(
		"%s",
		TOOL_MODE::NAVIGATION ==
			m_eToolMode
		? m_NavigationStatus.c_str()
		: m_Status.c_str());
	ImGui::Separator();

	if (TOOL_MODE::NAVIGATION ==
		m_eToolMode)
	{
		ImGui::BeginDisabled(
			!isAssetTest);
		Render_NavigationPanel();
		ImGui::EndDisabled();
		ImGui::End();
		return;
	}

	ImGui::BeginDisabled(
		!isAssetTest ||
		!m_Catalog.Is_Ready());
	Render_Toolbar();

	const f32_t availableHeight =
		ImGui::GetContentRegionAvail().y;
	const f32_t topPanelHeight =
		(std::max)(
			280.f,
			(std::min)(
				480.f,
				availableHeight *
				0.48f));

	if (ImGui::BeginTable(
		"MapEditorColumns",
		3,
		ImGuiTableFlags_Resizable |
		ImGuiTableFlags_BordersInnerV))
	{
		ImGui::TableSetupColumn(
			"Asset Palette",
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

### `Handle_LevelTransition()` 전체 교체

```cpp
void Client::CMapTool::Handle_LevelTransition(
	bool_t isAssetTest)
{
	if (isAssetTest ==
		m_bWasInAssetTest)
	{
		return;
	}

	m_bWasInAssetTest = isAssetTest;
	m_ePlacementState =
		PLACEMENT_STATE::IDLE;
	m_iSelectedPlacementId = 0;
	m_SelectedAssetId.clear();
	if (nullptr != m_pAssetPreview)
		m_pAssetPreview->
			Reset_LevelResources();

	if (!isAssetTest)
	{
		m_Placements.clear();
		m_StaticBatches.clear();
		m_DeployProps.clear();
		m_iNextPlacementId = 1;
		m_bDirty = false;
		m_Status =
			"Enter AssetTest with F2";
		m_NavigationStatus =
			"Enter AssetTest with F2";
		return;
	}

	if (!m_Catalog.Load_Default())
	{
		m_Status =
			m_Catalog.Get_Status();
	}
	else
	{
		m_Status =
			m_Catalog.Get_Status();
		if (Load_Placements())
			Load_DeployProps();
	}

	Load_NavigationDocument();
}
```

### Navigation 문서 load 추가

```cpp
bool_t Client::CMapTool::Load_NavigationDocument()
{
	const std::filesystem::path root =
		GetNavigationDataRoot();
	if (root.empty())
	{
		m_NavigationStatus =
			"Navigation data root is unavailable";
		return false;
	}

	m_NavigationSourcePath =
		root / L"ValtanArena.navsource";
	m_NavigationPaintPath =
		root / L"ValtanArena.navpaint";
	m_NavigationRuntimePath =
		root / L"ValtanArena.navgrid";
	return m_NavigationDocument.Load(
		m_NavigationSourcePath,
		m_NavigationPaintPath,
		m_NavigationStatus);
}
```

### Navigation cell picking 추가

```cpp
bool_t Client::CMapTool::Try_PickNavigationCell(
	int32_t& outCellX,
	int32_t& outCellZ) const
{
	if (!m_NavigationDocument.Is_Ready())
		return false;

	float4_t picked{};
	if (CGameInstance::Get().Picking(picked) &&
		m_NavigationDocument.World_ToCell(
			XMLoadFloat4(&picked),
			outCellX,
			outCellZ))
	{
		return true;
	}

	::POINT cursor{};
	if (!GetCursorPos(&cursor) ||
		!ScreenToClient(g_hWnd, &cursor))
	{
		return false;
	}

	const float2_t viewport =
		CGameInstance::Get().
		Get_ViewportSize();
	if (cursor.x < 0 ||
		cursor.y < 0 ||
		cursor.x >= static_cast<LONG>(
			viewport.x) ||
		cursor.y >= static_cast<LONG>(
			viewport.y))
	{
		return false;
	}

	const matrix_t view =
		XMLoadFloat4x4(
			CGameInstance::Get().
			Get_Transform(D3DTS::VIEW));
	const matrix_t projection =
		XMLoadFloat4x4(
			CGameInstance::Get().
			Get_Transform(D3DTS::PROJ));
	const vector_t nearPoint =
		XMVector3Unproject(
			XMVectorSet(
				static_cast<f32_t>(cursor.x),
				static_cast<f32_t>(cursor.y),
				0.f,
				1.f),
			0.f,
			0.f,
			viewport.x,
			viewport.y,
			0.f,
			1.f,
			projection,
			view,
			XMMatrixIdentity());
	const vector_t farPoint =
		XMVector3Unproject(
			XMVectorSet(
				static_cast<f32_t>(cursor.x),
				static_cast<f32_t>(cursor.y),
				1.f,
				1.f),
			0.f,
			0.f,
			viewport.x,
			viewport.y,
			0.f,
			1.f,
			projection,
			view,
			XMMatrixIdentity());

	float3_t rayOrigin{};
	float3_t rayDirection{};
	XMStoreFloat3(
		&rayOrigin,
		nearPoint);
	XMStoreFloat3(
		&rayDirection,
		farPoint - nearPoint);
	if (!std::isfinite(rayDirection.y) ||
		std::fabs(rayDirection.y) <
		0.000001f)
	{
		return false;
	}

	const NAVGRID_AUTHORING_DESC& desc =
		m_NavigationDocument.Get_Desc();
	f32_t bestRatio =
		(std::numeric_limits<f32_t>::max)();
	bool_t found = false;

	for (uint32_t index = 0;
		index <
			m_NavigationDocument.
			Get_CellCount();
		++index)
	{
		const f32_t displayHeight =
			m_NavigationDocument.
			Get_DisplayHeight(index);
		const f32_t ratio =
			(displayHeight -
				rayOrigin.y) /
			rayDirection.y;
		if (ratio < 0.f ||
			ratio > 1.f ||
			ratio >= bestRatio)
		{
			continue;
		}

		const f32_t worldX =
			rayOrigin.x +
			rayDirection.x * ratio;
		const f32_t worldZ =
			rayOrigin.z +
			rayDirection.z * ratio;
		const uint32_t cellX =
			index % desc.width;
		const uint32_t cellZ =
			index / desc.width;
		const f32_t minimumX =
			desc.originX +
			cellX * desc.cellSize;
		const f32_t minimumZ =
			desc.originZ +
			cellZ * desc.cellSize;
		if (worldX < minimumX ||
			worldX >=
				minimumX +
				desc.cellSize ||
			worldZ < minimumZ ||
			worldZ >=
				minimumZ +
				desc.cellSize)
		{
			continue;
		}

		bestRatio = ratio;
		outCellX =
			static_cast<int32_t>(cellX);
		outCellZ =
			static_cast<int32_t>(cellZ);
		found = true;
	}
	return found;
}
```

### paint 명령 추가

```cpp
bool_t Client::CMapTool::Try_PaintNavigation()
{
	int32_t cellX = {};
	int32_t cellZ = {};
	if (!Try_PickNavigationCell(
		cellX,
		cellZ))
	{
		m_NavigationStatus =
			"No NavGrid cell under cursor";
		return false;
	}

	const bool_t walkable =
		PAINT_MODE::WALKABLE ==
		m_ePaintMode;
	if (!m_NavigationDocument.Paint(
		cellX,
		cellZ,
		m_iBrushRadius,
		walkable))
	{
		return false;
	}

	m_NavigationStatus =
		std::string(
			walkable
			? "Painted walkable at "
			: "Painted non-walkable at ") +
		std::to_string(cellX) +
		"," +
		std::to_string(cellZ);
	return true;
}
```

### mode bar와 panel 추가

```cpp
void Client::CMapTool::Render_ModeBar()
{
	if (ImGui::RadioButton(
		"Map Assets",
		TOOL_MODE::MAP_ASSETS ==
		m_eToolMode))
	{
		m_eToolMode =
			TOOL_MODE::MAP_ASSETS;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"Navigation",
		TOOL_MODE::NAVIGATION ==
		m_eToolMode))
	{
		m_eToolMode =
			TOOL_MODE::NAVIGATION;
		m_ePlacementState =
			PLACEMENT_STATE::IDLE;
	}
}

void Client::CMapTool::Render_NavigationPanel()
{
	if (!m_NavigationDocument.Is_Ready())
	{
		ImGui::TextUnformatted(
			"Navigation document is unavailable.");
		if (ImGui::Button("Reload Navigation"))
			Load_NavigationDocument();
		return;
	}

	ImGui::Text(
		"Cells: %u | Resolved height: %u",
		m_NavigationDocument.
			Get_CellCount(),
		m_NavigationDocument.
			Get_ResolvedHeightCount());
	ImGui::Text(
		"Blocked: %u | Unresolved walkable: %u%s",
		m_NavigationDocument.
			Get_BlockedCount(),
		m_NavigationDocument.
			Get_UnresolvedWalkableCount(),
		m_NavigationDocument.Is_Dirty()
			? " | *unsaved"
			: "");

	if (ImGui::RadioButton(
		"Paint Non-Walkable",
		PAINT_MODE::BLOCKED ==
		m_ePaintMode))
	{
		m_ePaintMode =
			PAINT_MODE::BLOCKED;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton(
		"Paint Walkable",
		PAINT_MODE::WALKABLE ==
		m_ePaintMode))
	{
		m_ePaintMode =
			PAINT_MODE::WALKABLE;
	}

	int32_t brushRadius =
		static_cast<int32_t>(
			m_iBrushRadius);
	if (ImGui::SliderInt(
		"Brush radius",
		&brushRadius,
		0,
		static_cast<int32_t>(
			CNavGridPaintDocument::
			MAX_BRUSH_RADIUS)))
	{
		m_iBrushRadius =
			static_cast<uint32_t>(
				brushRadius);
	}

	if (ImGui::Button("Save Paint"))
	{
		m_NavigationDocument.Save_Paint(
			m_NavigationPaintPath,
			m_NavigationStatus);
	}
	ImGui::SameLine();
	if (ImGui::Button("Reload"))
	{
		Load_NavigationDocument();
	}
	ImGui::SameLine();

	const bool_t exportDisabled =
		m_NavigationDocument.Is_Dirty() ||
		0 != m_NavigationDocument.
			Get_UnresolvedWalkableCount();
	ImGui::BeginDisabled(exportDisabled);
	if (ImGui::Button("Export Runtime"))
	{
		if (m_NavigationDocument.
			Export_Runtime(
				m_NavigationRuntimePath,
				m_NavigationStatus))
		{
			m_NavigationStatus +=
				"; re-enter AssetTest to reload";
		}
	}
	ImGui::EndDisabled();

	ImGui::Separator();
	ImGui::TextUnformatted(
		"LMB drag: paint selected mode");
	ImGui::TextUnformatted(
		"Green: walkable with height");
	ImGui::TextUnformatted(
		"Yellow: user-blocked");
	ImGui::TextUnformatted(
		"Red: default walkable but height unresolved");
	ImGui::TextWrapped(
		"Runtime export requires a saved paint "
		"document and zero red cells.");
	ImGui::TextWrapped(
		"Source: %s",
		m_NavigationSourcePath.
			string().c_str());
	ImGui::TextWrapped(
		"Paint: %s",
		m_NavigationPaintPath.
			string().c_str());
	ImGui::TextWrapped(
		"Runtime: %s",
		m_NavigationRuntimePath.
			string().c_str());
}
```

### authoring overlay 추가

```cpp
void Client::CMapTool::Render_NavigationOverlay()
{
	if (!m_NavigationDocument.Is_Ready() ||
		nullptr ==
		m_pNavigationRenderResources ||
		nullptr ==
		m_pNavigationRenderResources->
			pBatch ||
		nullptr ==
		m_pNavigationRenderResources->
			pEffect ||
		nullptr ==
		m_pNavigationRenderResources->
			pInputLayout)
	{
		return;
	}

	auto& resources =
		*m_pNavigationRenderResources;
	resources.pEffect->SetWorld(
		XMMatrixIdentity());
	resources.pEffect->SetView(
		XMLoadFloat4x4(
			CGameInstance::Get().
			Get_Transform(D3DTS::VIEW)));
	resources.pEffect->SetProjection(
		XMLoadFloat4x4(
			CGameInstance::Get().
			Get_Transform(D3DTS::PROJ)));
	m_pContext->IASetInputLayout(
		resources.pInputLayout.Get());
	resources.pEffect->Apply(
		m_pContext.Get());

	const float4_t green(
		0.1f,
		1.f,
		0.2f,
		1.f);
	const float4_t yellow(
		1.f,
		0.85f,
		0.05f,
		1.f);
	const float4_t red(
		1.f,
		0.15f,
		0.1f,
		1.f);
	const NAVGRID_AUTHORING_DESC& desc =
		m_NavigationDocument.Get_Desc();
	const f32_t halfCell =
		desc.cellSize * 0.5f;

	resources.pBatch->Begin();
	for (uint32_t index = 0;
		index <
			m_NavigationDocument.
			Get_CellCount();
		++index)
	{
		const uint32_t cellX =
			index % desc.width;
		const uint32_t cellZ =
			index / desc.width;
		const float3_t center(
			desc.originX +
			(static_cast<f32_t>(cellX) +
				0.5f) *
			desc.cellSize,
			m_NavigationDocument.
				Get_DisplayHeight(index) +
			0.08f,
			desc.originZ +
			(static_cast<f32_t>(cellZ) +
				0.5f) *
			desc.cellSize);

		const auto state =
			m_NavigationDocument.
			Get_CellState(index);
		const float4_t& color =
			NAVGRID_AUTHORING_CELL_STATE::
				BLOCKED == state
			? yellow
			: NAVGRID_AUTHORING_CELL_STATE::
				UNRESOLVED == state
			? red
			: green;
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

		resources.pBatch->DrawLine(
			leftTop,
			rightTop);
		resources.pBatch->DrawLine(
			rightTop,
			rightBottom);
		resources.pBatch->DrawLine(
			rightBottom,
			leftBottom);
		resources.pBatch->DrawLine(
			leftBottom,
			leftTop);
	}
	resources.pBatch->End();
}
```

주의: 위 함수는 기존 `CMapTool`이 `m_pContext`를 보관하지 않으므로 `MapTool.h`에 다음
멤버도 추가하고 `Initialize()` 시작에서 대입한다.

```cpp
	ComPtr<ID3D11DeviceContext> m_pContext =
		{ nullptr };
```

```cpp
	m_pContext = pContext;
```

## 7.6 `MainApp.cpp`

### 본질·책임

1. 한 문장 본질: tool이 world mouse를 소유하는 프레임에는 게임 DirectInput 소비를 막는다.
2. 역할: 기존 입력 차단 gate에 MapTool 명시적 world ownership을 합친다.
3. 비역할: paint mode 해석, cell pick.
4. 호출: 매 Debug frame, Engine update 직전.
5. 결과: LMB paint가 Character click-move로 동시에 전달되지 않는다.

### `CMainApp::Update()` 전체 교체

```cpp
void CMainApp::Update(f32_t fTimeDelta)
{
#ifdef _DEBUG
	UpdateDebugToolShortcut();

	if (nullptr != m_pImGuiLayer)
		m_pImGuiLayer->BeginFrame();

	const bool_t bMapToolOpen =
		nullptr != m_pMapTool &&
		m_pMapTool->IsOpen();
	const HWND hForegroundWindow =
		GetForegroundWindow();
	const bool_t bExternalToolFocused =
		bMapToolOpen &&
		nullptr != hForegroundWindow &&
		hForegroundWindow != g_hWnd &&
		IsWindowOwnedByCurrentProcess(
			hForegroundWindow);
	const bool_t bImGuiPanelOpen =
		bMapToolOpen ||
		m_bProfilerVisible;
	const bool_t bWorldMouseOwnedByTool =
		bMapToolOpen &&
		m_pMapTool->ConsumesWorldMouse();

	const bool_t bKeyboardCaptured =
		bImGuiPanelOpen &&
		nullptr != m_pImGuiLayer &&
		(m_pImGuiLayer->
			WantsCaptureKeyboard() ||
			bExternalToolFocused);
	const bool_t bMouseCaptured =
		bImGuiPanelOpen &&
		nullptr != m_pImGuiLayer &&
		(m_pImGuiLayer->
			WantsCaptureMouse() ||
			bExternalToolFocused ||
			bWorldMouseOwnedByTool);
	CGameInstance::Get().SetInputBlocked(
		bKeyboardCaptured,
		bMouseCaptured);
#endif

	CGameInstance::Get().Update_Engine(
		fTimeDelta);

#ifdef _DEBUG
	if (nullptr != m_pMapTool)
		m_pMapTool->Update(fTimeDelta);
#endif
}
```

## 7.7 데이터 파일 계약

### `.navsource`

UTF-8 text, z-major row다. Python만 writer다.

```text
LOSTARK_NAVGRID_SOURCE 1 "LV_LUT_HEARTRB_ED" 62 63 0.5 140.5 -137.5 3906
0 0 0 0
1 0 0 0
...
21 31 1 22.96835
31 31 1 22.99751
...
61 62 0 0
```

첫 bake 완료 시 기대 통계:

```text
preferredHeightCells = 2843
fallbackHeightCells = 54
resolvedHeightCells = 2897
unresolvedHeightCells = 1009
```

실제 생성 파일의 모든 row와 통계는 bake 후 독립 parser로 다시 검산한다. 위 통계가
달라지면 자동으로 계획값에 맞추지 않고 입력 glTF/placement와 변경 원인을 조사한다.

### `.navpaint`

UTF-8 text, blocked 좌표만 저장한다. 초기 파일 전체 내용은 다음 한 줄이다.

```text
LOSTARK_NAVGRID_PAINT 1 "LV_LUT_HEARTRB_ED" 62 63 0.5 140.5 -137.5 0
```

예를 들어 `(31,31)`을 막으면 전체 파일은 다음과 같다.

```text
LOSTARK_NAVGRID_PAINT 1 "LV_LUT_HEARTRB_ED" 62 63 0.5 140.5 -137.5 1
31 31
```

### `.navgrid`

형식은 변경하지 않는다.

```text
uint32 width
uint32 height
float cellSize
float originX
float originZ
uint8 walkable[width*height]
float height[width*height]
```

현재 포맷에는 magic/version이 없지만 이번 목표 때문에 runtime loader까지 확장하지
않는다. source/paint parser가 identity와 수를 검증한 뒤에만 이 binary를 만든다.

## 8. 프로젝트 등록

새 C++ 파일만 Client 프로젝트에 등록한다.

### `Client.vcxproj`

기존 `MapPlacementDocument` 옆에 추가한다.

```xml
<ClInclude Include="..\Public\NavGridPaintDocument.h" />
<ClCompile Include="..\Private\NavGridPaintDocument.cpp" />
```

### `Client.vcxproj.filters`

기존 MapTool과 같은 filter를 사용하고 새 filter/GUID를 만들지 않는다.

```xml
<ClInclude Include="..\Public\NavGridPaintDocument.h">
  <Filter>03. Tools\00. Map</Filter>
</ClInclude>
<ClCompile Include="..\Private\NavGridPaintDocument.cpp">
  <Filter>03. Tools\00. Map</Filter>
</ClCompile>
```

`Client/Bin/DataFiles`는 현재 프로젝트 item이 아니라 runtime data root로 Git 추적되는
관례다. `.navsource`, `.navpaint`, `.navgrid`에 `<None>` 항목이나 build copy step을
새로 만들지 않는다.

## 9. 적용 순서

1. 현재 `.navgrid`의 hash와 62×63/2,843/1,063 실측값을 결과 문서에 보존한다.
2. Python baker를 height source writer로 바꾼다.
3. 실제 추출물로 `.navsource`를 두 번 생성해 byte/hash가 같은지 확인한다.
4. 독립 parser로 header, 3,906 row, 중복 0, resolved 2,897, unresolved 1,009를 확인한다.
5. `CNavGridPaintDocument` h/cpp를 추가하고 프로젝트에 등록한다.
6. MapTool에 Navigation mode, overlay, GPU+grid fallback pick, paint UI를 연결한다.
7. MainApp input gate에 `ConsumesWorldMouse()`를 연결한다.
8. Debug 전체 빌드 뒤 F1 편집·Save·Reload·Export를 검증한다.
9. AssetTest를 재진입해 기존 Loader/Prototype/Clone 경로로 새 `.navgrid`를 읽는다.
10. F5 색과 Character/Valtan A* 우회를 확인한다.
11. Release 전체 빌드에서 ImGui/authoring 경로가 제외되고 runtime load/A*만 남는지 확인한다.

## 10. 빌드·정적 검증

C++ 파일은 기존 CP949 인코딩을 유지하고 Markdown/data text는 UTF-8로 저장한다.

```powershell
python -m py_compile `
  Tools/LevelPlacementExtractor/build_valtan_navgrid.py
```

Debug:

```powershell
msbuild .\Engine\Default\Engine.vcxproj `
  /t:Build /p:Configuration=Debug /p:Platform=x64
.\UpdateLib.bat Debug
msbuild .\Client\Default\Client.vcxproj `
  /t:Build /p:Configuration=Debug /p:Platform=x64
```

Release:

```powershell
msbuild .\Engine\Default\Engine.vcxproj `
  /t:Build /p:Configuration=Release /p:Platform=x64
.\UpdateLib.bat Release
msbuild .\Client\Default\Client.vcxproj `
  /t:Build /p:Configuration=Release /p:Platform=x64
```

Engine source를 이 계획에서 수정하지 않지만 팀 공통 완료 순서에 따라 Engine →
UpdateLib → Client 두 구성을 모두 검증한다. 실행 중인 `Client.exe`가 출력물을 잡고
있으면 종료한 뒤 Client link를 다시 수행한다.

## 11. 실행 검증

### 11.1 기본 상태와 색

1. `Client/Bin/Debug/Client.exe`를 working directory `Client/Bin`으로 실행한다.
2. F2로 AssetTest, F1로 MapTool, `Navigation` mode로 들어간다.
3. paint 파일이 없을 때 `all cells start walkable` status를 확인한다.
4. 높이가 있는 셀은 초록, 높이 미해결 기본 walkable은 빨강, 노랑은 0개인지 확인한다.
5. F5는 기존 runtime `.navgrid` 관찰이므로 F1 source overlay와 구분해 설명한다.

### 11.2 피킹과 양방향 paint

1. 실제 바닥 초록 셀을 `Paint Non-Walkable`로 클릭해 노랑이 되는지 확인한다.
2. 같은 셀을 `Paint Walkable`로 되돌려 초록이 되는지 확인한다.
3. 렌더 표면이 없는 외곽/구멍의 빨강 셀을 클릭해 노랑이 되는지 확인한다.
4. brush 0과 brush 8에서 원형 범위와 grid 경계 clipping을 확인한다.
5. UI 위 LMB는 칠하지 않고, world paint 중 Character가 click-move하지 않는지 확인한다.

### 11.3 저장·재로드·실패 보존

1. 여러 셀을 막고 `Save Paint` 후 `.navpaint` row 수와 blocked 통계가 같은지 확인한다.
2. 일부를 바꾼 뒤 `Reload`해 마지막 저장 상태로 돌아가는지 확인한다.
3. source header area/width를 바꾼 복사본 Load가 실패하고 현재 화면 상태가 유지되는지 확인한다.
4. paint에 중복/범위 밖 row를 넣은 복사본 Load가 실패하고 현재 상태가 유지되는지 확인한다.
5. `.navpaint.tmp` write/commit 실패를 유도했을 때 기존 paint hash가 유지되는지 확인한다.

### 11.4 runtime export와 A*

1. 빨강 셀이 하나라도 남으면 Export 버튼이 비활성이고 직접 호출도 실패하는지 확인한다.
2. 모든 unresolved 셀을 blocked로 확정하고 Save Paint를 누른다.
3. `Export Runtime` 성공 뒤 binary size가 19,550 bytes인지 확인한다.
4. binary의 walkable count가 `3906 - blockedCount`와 같은지 확인한다.
5. AssetTest를 나갔다 재진입하고 F5를 누른다.
6. F1에서 막은 셀만 F5에서 노랑인지 확인한다.
7. 초록 goal click은 이동하고 노랑 goal click은 `GOAL_NOT_WALKABLE`로 거부되는지 확인한다.
8. 통로 일부를 막고 Character와 Valtan 경로가 노란 셀을 우회하는지 확인한다.
9. 막아서 연결이 끊긴 goal은 `UNREACHABLE`이고 기존 actor 상태가 망가지지 않는지 확인한다.
10. F5 cyan path가 노랑 cell을 통과하지 않는지 확인한다.

### 11.5 Release

1. Release 실행에서 F1 MapTool과 authoring overlay가 없음을 확인한다.
2. Loader가 최종 `.navgrid`를 읽고 Character/Valtan A*가 Debug와 같은 walkability를 사용하는지 확인한다.

## 12. 이번 단계에서 하지 않는 것

- 실행 중인 Navigation Prototype/clone hot reload.
- agent radius inflation.
- 다층 NavMesh와 같은 XZ의 복수 height.
- undo/redo와 paint history.
- 자동 장애물/Collider 추론.
- runtime `.navgrid` magic/version 변경.
- MapTool paint를 Engine `CNavigation` 내부 상태로 복제.

이번 반영에는 `.navblockers` 기반 동적 blocker를 포함한다.
MapTool에서 stable blocker ID, condition ID, 활성 극성을 정하고 셀을 피킹해 저장한다.
Engine은 base walkability와 runtime blocker reference count를 합성하고, blocker 상태가
바뀌면 revision을 올려 진행 중인 `CNavPathFollower` 경로를 취소한다. AssetTest에서는
`VALTAN_ARENA_DESTROYED` condition을 `Set_DeployPhase()`에 연결해 INTACT와
FRACTURED/DESPAWNED 전환을 검증할 수 있다.

실제 레이드 전투 시퀀스에서 어느 패턴 프레임에 condition을 전환할지는 보스 패턴
controller의 책임으로 남긴다. 정적 원본인 `.navpaint`는 플레이 중 수정하지 않는다.

## 13. 완료 판정과 30초 설명

다음 문장을 코드, 파일, 화면으로 모두 증명하면 완료다.

> 현재 F5는 베이크가 아니라 `ValtanArena.navgrid`의 bool을 그리는 runtime Debug다.
> 실제 bake는 Python이 glTF와 placement를 월드 격자로 바꾸며 수행한다. 변경 후 Python은
> 높이만 `.navsource`에 굽고, MapTool은 전 셀을 기본 walkable로 시작한 뒤 사용자가
> 피킹한 blocked 셀을 `.navpaint`에 저장한다. 검증된 source와 paint만 기존 `.navgrid`
> 형식으로 export한다. 런타임 파괴 지형은 `.navblockers`의 stable ID와 condition으로
> 별도 관리하며 `CNavigation`의 공유 grid에 합성된다. blocker revision이 바뀌면 오래된
> 경로를 취소하므로 Character와 Valtan이 다음 요청에서 새 이동 가능 상태로 다시 탐색한다.

## 14. 구현 결과

### 14.1 Authoring

- Python bake 결과를 높이 전용 `ValtanArena.navsource`로 분리했다.
- MapTool Navigation 모드에서 정적 blocked/walkable과 동적 blocker 셀을 피킹한다.
- 정적 편집은 `.navpaint`, 동적 region은 `.navblockers`에 원자적으로 저장한다.
- 미해결 높이 셀은 빨강, 정적 blocked는 노랑, 유효 셀은 초록, 선택한 동적 region은
  자홍색으로 표시한다.
- 미해결 셀이 walkable로 남아 있으면 runtime `.navgrid` export를 거부한다.

### 14.2 Runtime

- base walkability와 runtime blocker count를 `base && count == 0`으로 합성한다.
- Navigation clone들이 같은 runtime grid를 공유한다.
- condition 전환은 blocker ID 기준으로 멱등 처리한다.
- revision이 달라진 follower 경로는 즉시 취소하며 기존 `.navgrid` binary 계약은 유지한다.

### 14.3 검증

- 높이 source: `62 x 63`, 총 `3,906`셀, resolved `2,897`, unresolved `1,009`.
- 동일 입력 재생성 SHA-256 일치로 bake 결정성을 확인했다.
- Engine Debug/Release, UpdateLib Debug/Release, Client Debug/Release 빌드를 통과했다.
- Debug Client에서 Logo를 거쳐 `Valtan WModel Asset Test` 진입과 프로세스 응답을 확인했다.
- 실제 이동 불가 셀과 파괴 region은 지형 의미를 임의 추측하지 않고 MapTool 피킹으로
  작성하도록 빈 paint/region 상태를 유지한다.
