# LostArk MapTool viewport, picking, reference terrain stabilization plan

> 작성일: 2026-07-29  
> 대상 저장소: `C:\Users\user\Desktop\LostArk`  
> 상태: **PLAN ONLY — 구현 전 원인 확정 및 작업 순서 문서**  
> 범위: 외부 ImGui MapTool 안정화 + ASSET_TEST 기준 지면 + 피킹 좌표 검증  
> 비범위: 발탄/맵 `.wmodel` 포맷 변경, 저장 파일 포맷 교체, Gameplay Terrain 교체

## 0. 결론

현재 현상은 한 가지 문제가 아니라 다음 두 문제가 겹친 것이다.

1. 게임은 내부적으로 계속 `1280 x 720`을 렌더링하고 피킹하는데, 실제 메인 창은 `WS_OVERLAPPEDWINDOW`라 사용자가 크기를 바꿀 수 있다. 창 크기가 달라지면 마우스의 클라이언트 좌표와 `Target_PickPos` 텍스처 좌표가 일치하지 않는다.
2. 화면에 피킹 가능한 지면이 없으면 `CMapTool`이 보이지 않는 `Y=0` 평면과 광선을 교차시켜 배치한다. 따라서 사용자는 아무 기준도 없는 파란 화면을 클릭하고, 실제로는 카메라 밖의 `(-49, 0, -25)` 같은 위치에 오브젝트를 만들 수 있다.

스크린샷의 Hierarchy에 `Valtan Crystal 01 #1`이 있고 Position도 기록되어 있으므로 **Clone 실패가 아니라 배치 좌표와 시야 기준 문제일 가능성이 가장 높다.**

외부 ImGui 창은 유지한다. 외부 창 자체가 모델 Scale을 망가뜨리는 것은 아니다. 다만 별도 HWND가 생기기 때문에 포커스와 마우스 소유권을 명확히 나눠야 하며, 현재처럼 전역 `GetAsyncKeyState()`에 의존하면 사용 경험이 불안정해진다.

이번 수정의 최종 형태는 다음과 같다.

```text
F2 -> ASSET_TEST
  -> 카메라 + 조명 + CAssetTestGround 생성
  -> 게임 창 안에서만 배치 클릭 허용
  -> 실제 클라이언트 좌표를 1280x720 PickPos 좌표로 변환
  -> PickPos 또는 명시적으로 선택한 Y=0 Ground Plane에서 위치 획득
  -> 미리보기 위치/피킹 출처 표시
  -> 클릭 시 CMapAssetObject Clone

F1 -> 외부 MapTool 창
  -> Palette / Hierarchy / Inspector
  -> 창 크기가 작으면 3열을 억지로 압축하지 않고 최소 크기 또는 세로 레이아웃 사용
  -> Scale은 기본 Uniform Scale
```

---

## 1. 현재 코드에서 확인된 원인

### 1.1 외부 ImGui 창은 실제로 활성화되어 있다

`Engine/Private/ImGuiLayer.cpp`에서 다음 설정을 사용한다.

```cpp
io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
io.IniFilename = nullptr;
```

- `ViewportsEnable` 때문에 MapTool을 메인 게임 창 밖의 별도 OS 창으로 뺄 수 있다.
- 이것은 요청한 기능이며 제거할 필요가 없다.
- `IniFilename == nullptr`라 창 위치와 크기가 다음 실행에 저장되지 않는다.
- 별도 창을 클릭한 동안에는 `GetForegroundWindow() != g_hWnd`이므로 배치 클릭은 막혀 있다. 따라서 **MapTool 창을 밖으로 뺀 것 자체가 잘못된 월드 좌표를 만드는 직접 원인은 아니다.**

### 1.2 렌더 해상도와 실제 게임 창 크기가 동기화되지 않는다

고정 렌더 크기:

```text
Client/Public/Client_Defines.h
  g_iWinSizeX = 1280
  g_iWinSizeY = 720

Client/Private/MainApp.cpp
  EngineDesc.iWinSizeX/Y에 위 값을 전달

Engine/Private/Picking.cpp
  초기화 때 Get_ViewportSize()로 1280x720 staging texture를 한 번 생성
```

반면 `Client/Default/Client.cpp`는 `WS_OVERLAPPEDWINDOW`로 메인 창을 생성한다. 즉 사용자가 게임 창을 자유롭게 줄이거나 늘릴 수 있다. 현재 프로젝트에는 메인 swap chain과 G-buffer, `Target_PickPos`, depth buffer, picking staging texture를 함께 재생성하는 `WM_SIZE -> ResizeBuffers` 경로가 없다.

현재 `CPicking::Picking()`은 다음처럼 실제 마우스 클라이언트 픽셀을 고정 1280x720 텍스처 인덱스로 바로 사용한다.

```text
실제 게임 창 좌표 (예: 700, 300)
             그대로 사용
1280x720 Target_PickPos 좌표 (700, 300)
```

게임 창의 실제 클라이언트가 1280x720이 아니라면 같은 화면 위치가 아니다.

### 1.3 피킹 실패가 보이지 않는 Y=0 배치로 숨겨진다

`Client/Private/MapTool.cpp::Try_PickPlacementPosition()`의 현재 순서:

```text
Target_PickPos 성공 -> 해당 월드 위치
Target_PickPos 실패 -> 카메라 광선과 Y=0 평면 교차
```

지면이 없어 파란 배경만 보이더라도 두 번째 계산은 성공할 수 있다. 사용자는 화면상 기준을 보지 못하지만 오브젝트는 멀리 떨어진 좌표에 생성된다. 스크린샷의 `(-49.110, 0, -24.958)`은 이 경로와 일치한다.

문제는 Y=0 계산 자체가 아니라 다음 두 가지이다.

- 어떤 경로로 피킹했는지 UI에서 알 수 없다.
- 보이지 않는 평면을 기본 fallback으로 조용히 사용한다.

### 1.4 “크기를 줄이면 쭉 줄어드는 느낌”은 두 현상을 구분해야 한다

#### MapTool 창이 줄어드는 경우

현재 MapTool은 3개 열을 전부 `WidthStretch`로 만든다.

```text
Asset Palette 38%
Hierarchy     27%
Inspector     35%
```

최소 창 크기나 좁은 창용 대체 레이아웃이 없어서 외부 창 너비를 줄이면 세 열이 계속 압축된다. 이것은 ImGui 레이아웃 문제다.

#### 배치된 모델 Scale을 줄이는 경우

Inspector는 `DragFloat3("Scale")`로 X/Y/Z를 독립 편집한다. 또한 `CMapAssetObject::Set_PlacementTransform()`은 로컬 AABB의 bottom-center가 배치 위치에 남도록 Scale이 바뀔 때마다 실제 월드 원점을 다시 계산한다.

```text
논리 배치 위치 = 바닥 중앙 고정
Scale 변경
-> local bottom-center offset도 변경
-> 실제 Transform Position 재계산
```

의도 자체는 “지면에서 발이 떨어지지 않게 하기”이지만, 원본 메시의 원점이 멀거나 크기가 아주 큰 경우 모델이 한쪽으로 끌리며 줄어드는 것처럼 보일 수 있다. 현재 UI에는 논리 배치 위치, 실제 월드 원점, bounds 크기가 구분되어 표시되지 않는다.

### 1.5 수업용 CTerrain을 ASSET_TEST에 바로 Clone하면 안 된다

`Client/Private/Terrain.cpp::Ready_Components()`는 모든 구성요소를 `LEVEL::GAMEPLAY`에서 찾는다.

- Terrain diffuse/mask texture
- `Shader_VtxNorTex`
- `VIBuffer_Terrain`
- `TerrainNavigation`

그리고 `Engine/Private/VIBuffer_Terrain.cpp`는 초기화 과정에서 `TerrainNavigation.dat`를 `CREATE_ALWAYS`로 연다. 피킹 확인용 지면을 하나 만들기 위해 Gameplay 의존성과 Navigation 파일 덮어쓰기 가능성까지 가져오게 된다.

따라서 1차 검증용으로는 수업 Terrain이 아니라 **ASSET_TEST 전용 표시 지면**을 추가한다.

---

## 2. 확정할 사용자 경험

### 2.1 외부 MapTool은 유지한다

- F1으로 표시/숨김.
- 게임 화면 밖으로 이동 가능.
- MapTool 창 클릭은 절대 월드 배치 클릭으로 처리하지 않는다.
- 창 위치와 크기는 Debug 전용 설정 파일에 저장한다.
- 최소 창 크기를 강제하고, 최소 폭보다 좁아질 경우 3열 대신 탭 또는 세로 배치를 사용한다.

### 2.2 피킹 모드를 명시적으로 보인다

Toolbar에 다음 모드를 둔다.

```cpp
enum class PLACEMENT_SURFACE
{
    VISIBLE_SURFACE,  // Target_PickPos
    GROUND_Y0         // 명시적인 Y=0 평면
};
```

초기 권장값은 `VISIBLE_SURFACE`이다. ASSET_TEST 전용 Ground가 실제로 렌더되기 때문에 정상이라면 Ground의 `Target_PickPos`를 얻는다.

Toolbar 또는 Status에 다음을 항상 표시한다.

```text
Mouse client : (x, y)
Pick texture : (x, y)
Pick source  : Target_PickPos / Ground Y=0 / Invalid
World        : (x, y, z)
```

`VISIBLE_SURFACE`에서 실패하면 자동으로 Y=0을 사용하지 않는다. 사용자가 `GROUND_Y0`을 선택했을 때만 평면 계산을 사용한다.

### 2.3 배치 전에 위치를 미리 본다

- Palette 선택 시 ARMED 상태 진입.
- 게임 화면 위 마우스 위치에 작은 십자 또는 원형 마커를 렌더한다.
- 유효 피킹이면 초록색, 무효면 빨간색.
- 클릭 전부터 좌표가 말이 되는지 확인할 수 있어야 한다.
- 클릭 후에는 한 번만 Clone하고 ID를 발급한다.

### 2.4 Transform 편집은 안전한 기본값을 사용한다

- Map asset 기본값은 `Uniform Scale = ON`.
- 단일 float scale을 바꾸면 X/Y/Z에 같은 값을 적용한다.
- 필요할 때만 `Non-uniform`을 켜 `float3`를 노출한다.
- `Reset Transform`을 제공한다.
- Inspector에 아래 값을 따로 표시한다.

```text
Placement anchor : 사용자가 저장하는 위치
World origin     : CTransform에 실제로 들어가는 위치
Local bounds     : min/max 및 size
World bounds     : 현재 Scale 적용 예상 크기
```

- 지나치게 큰/작은 모델은 bounds 기준 경고를 표시한다.
- Catalog의 `defaultScale`은 실제 bounds 검증 후 에셋별로 조정한다.

---

## 3. Part A — ImGui 및 피킹 안정화 구현 계획

### A-1. 먼저 진단 정보를 추가한다

행동을 바꾸기 전에 한 프레임의 피킹 상태를 구조체로 남긴다.

```cpp
enum class PICK_SOURCE
{
    INVALID,
    TARGET_PICK_POS,
    GROUND_PLANE
};

struct EDITOR_PICK_RESULT
{
    PICK_SOURCE source = PICK_SOURCE::INVALID;
    POINT clientPixel{};
    POINT renderPixel{};
    float3_t worldPosition{};
};
```

이 정보로 다음을 즉시 구분한다.

- 창 크기 배율 문제
- Target_PickPos가 비어 있는 문제
- Y=0 fallback으로 빠진 문제
- 마우스가 외부 ImGui 창 위에 있는 문제

### A-2. 게임 창 좌표를 렌더 좌표로 변환한다

단기 기준은 수업 프레임워크의 고정 1280x720 렌더를 유지하는 것이다. 실제 클라이언트 크기를 매 클릭마다 `GetClientRect(g_hWnd)`로 읽고 아래처럼 변환한다.

```cpp
renderX = clientX * renderWidth  / clientWidth;
renderY = clientY * renderHeight / clientHeight;
```

변환 함수는 `CMapTool`과 `CPicking`에 중복 구현하지 않는다. `CPicking`이 명시적인 client pixel을 받고 내부에서 render pixel로 변환하거나, 공용 helper가 변환한 render pixel을 `CPicking`에 전달한다.

장기적으로 게임 창 자체를 진짜 리사이즈하려면 다음 자원을 한 번에 재생성해야 한다.

- swap chain back buffer
- depth stencil
- 모든 G-buffer/MRT
- `Target_PickPos`
- Picking staging texture와 CPU buffer
- 카메라 aspect ratio

이번 작업에서는 이 대규모 resize 경로까지 확장하지 않는다. 먼저 좌표 변환으로 편집 피킹을 안정화한다. 렌더 스트레칭까지 없애고 싶다면 별도 단계로 진행한다.

### A-3. Picking staging 복사를 RowPitch 안전하게 만든다

`CPicking::Update()`의 한 번짜리 `memcpy(width * height)`를 행 단위 복사로 바꾼다.

```text
for each y
  source = pData + y * RowPitch
  target = worldPositions + y * width
  copy width * sizeof(float4)
```

현재 폭에서는 우연히 RowPitch가 같을 수 있지만, resize 또는 다른 GPU 환경에서도 안전하도록 고친다.

### A-4. 외부 ImGui 창과 게임 클릭을 분리한다

현재 `GetAsyncKeyState(VK_LBUTTON)` 전역 polling만으로 판단하는 부분을 다음 조건으로 감싼다.

```text
배치 가능 =
  ASSET_TEST이고
  Placement가 ARMED이고
  foreground HWND가 메인 게임 HWND이고
  cursor가 메인 client rect 안이고
  ImGui WantCaptureMouse가 false이고
  포커스를 얻은 바로 그 프레임이 아니고
  LButton down edge가 한 번 발생함
```

가능하면 최종적으로 Win32 `WM_LBUTTONDOWN` 이벤트를 엔진 입력 큐에 기록해 프레임당 한 번 소비한다. 1차 수정에서는 기존 polling을 유지하더라도 포커스 전환 시 이전 버튼 상태를 초기화한다.

### A-5. 피킹 fallback을 모드로 분리한다

`Try_PickPlacementPosition()`을 다음 세 함수로 나눈다.

```cpp
bool Try_GetMainViewportPixel(...);
bool Try_PickVisibleSurface(...);
bool Try_PickGroundPlane(float groundY, ...);
```

그리고 선택된 `PLACEMENT_SURFACE` 하나만 실행한다. 실패 원인을 status에 남긴다. 자동 fallback은 하지 않는다.

### A-6. MapTool 레이아웃과 Scale UI를 안정화한다

- `SetNextWindowSizeConstraints()`로 최소 크기 지정.
- 3열 레이아웃을 사용할 최소 폭 지정.
- 좁으면 Palette / Hierarchy / Inspector 탭으로 전환.
- 열 최소 폭을 지정하고 지나친 stretch를 제한.
- Debug 전용 `imgui.ini` 저장 경로 지정.
- Uniform Scale 기본 ON.
- Reset, 0.1, 1.0, 10.0 정도의 명시적인 Scale preset 제공.
- `DragFloat` 활성 중에는 수치가 너무 빠르게 바뀌지 않도록 속도와 format을 조정.
- anchor/world origin/bounds를 함께 표시.

---

## 4. Part B — ASSET_TEST 기준 Terrain 구현 계획

### B-1. 클래스 이름과 책임

새 클래스는 `CTerrain`이 아니라 `CAssetTestGround`로 만든다.

```text
CAssetTestGround
  목적: 에디터의 원점, 거리, 스케일, 피킹 기준을 시각화
  포함: Transform + Renderer + Shader + Rect VIBuffer
  제외: Navigation, height map, terrain data 파일 생성, 저장 대상
```

Visual Studio 필터 권장 위치:

```text
Client
└─ 12.BinaryAsset
   └─ 30.Editor
      ├─ AssetTestGround.h
      └─ AssetTestGround.cpp
```

물리 경로:

```text
Client/Public/AssetTestGround.h
Client/Private/AssetTestGround.cpp
Client/Bin/ShaderFiles/Shader_AssetTestGround.hlsl
```

`CAssetTestGround`는 런타임 게임 오브젝트지만 Map 저장에는 포함하지 않는다. Level이 열릴 때 항상 생성되는 편집 보조물이다.

### B-2. 기존 사각형 버퍼를 재사용한다

`LEVEL::STATIC`에 이미 등록된 `Prototype_Component_VIBuffer_Rect`를 사용한다. XY 사각형을 XZ 지면으로 회전하고 충분한 크기로 Scale한다.

초기값 예시:

```text
Center     = (0, 0, 0)
HalfExtent = 50 또는 100
Rotation X = 90도
Grid major = 10
Grid minor = 1
```

실제 프로젝트의 좌표계에 맞춰 회전 부호는 화면 검증 후 확정한다.

### B-3. 전용 shader가 색상과 PickPos를 함께 출력한다

`Shader_AssetTestGround.hlsl`은 텍스처 없이 checker/grid를 그린다.

```text
Target0 = 어두운 회색 바닥 + 1m/10m grid + X/Z axis 색상
Target1 = normal
Target2 = depth용 값
Target3 = world position, w = 1   // Target_PickPos
```

파란 clear color와 확실히 구분되어야 하고, 원점 X/Z 축을 색으로 보여 카메라 방향까지 알 수 있어야 한다.

### B-4. ASSET_TEST 전용 Prototype과 Layer를 등록한다

`CLoader::Ready_For_Level_AssetTest()`:

```text
Prototype_Component_Shader_AssetTestGround
Prototype_GameObject_AssetTestGround
```

`CLevel_AssetTest::Initialize()`:

```text
Ready_Layer_Camera
Ready_Lights
Ready_Layer_EditorGround
Ready_Layer_Valtan 또는 기존 테스트 모델
```

Layer 이름:

```text
Layer_EditorGround
```

Ground 생성 실패는 ASSET_TEST 초기화 실패로 처리해 파란 화면만 뜬 채 계속 진행하지 않게 한다.

### B-5. 카메라 시작 위치에서 반드시 지면이 보이게 한다

현재 카메라는 대략 `(-18, 10, -18) -> (0, 3, 0)`을 본다. Ground 중심을 원점에 두면 시작 직후 원점 grid가 보여야 한다.

추가 권장 기능:

- `Home`: 카메라를 원점 기준 기본 위치로 복귀.
- `F`: 선택한 오브젝트 bounds 중심과 반경으로 카메라 focus.
- 배치 마커는 Ground보다 약간 위에 렌더해 z-fighting 방지.

`F` focus는 Camera_Free에 위치/시선 설정 API가 필요하므로 피킹 안정화 이후 단계로 둔다.

### B-6. 실제 수업 Terrain은 2차 선택 사항이다

높이맵 위 배치가 실제로 필요해지면 `CTerrain`을 다음처럼 분리한다.

```cpp
struct TERRAIN_DESC
{
    uint32_t componentPrototypeLevel;
    bool useNavigation;
    bool writeNavigationData;
    // heightmap/resource tags
};
```

- `LEVEL::GAMEPLAY` 하드코딩 제거.
- Navigation을 optional로 변경.
- `CVIBuffer_Terrain` 생성과 navigation bake 파일 출력을 분리.
- AssetTest는 읽기 전용 heightmap + no navigation으로 사용.

그러나 이것은 단순 평면 피킹이 정상임을 검증한 뒤 진행한다. 두 문제를 동시에 섞으면 피킹 오류와 Terrain 초기화 오류를 구분하기 어렵다.

---

## 5. 수정 예정 파일

### 반드시 수정

| 파일 | 계획된 변경 |
|---|---|
| `Engine/Public/Picking.h` | 명시적 픽셀 또는 좌표 변환을 받는 picking API와 진단 정보 |
| `Engine/Private/Picking.cpp` | client→render 좌표 변환, bounds 검사, RowPitch 행 복사 |
| `Engine/Public/GameInstance.h` | 필요한 최소 picking overload 전달 |
| `Engine/Private/GameInstance.cpp` | Picking facade 구현 |
| `Client/Public/MapTool.h` | surface mode, pick result, uniform scale 상태 |
| `Client/Private/MapTool.cpp` | 외부 창 입력 분리, 자동 fallback 제거, 상태 표시, 안정적 레이아웃/Scale UI |
| `Client/Public/AssetTestGround.h` | 전용 기준 지면 클래스 |
| `Client/Private/AssetTestGround.cpp` | 기준 지면 Clone/Render 구현 |
| `Client/Bin/ShaderFiles/Shader_AssetTestGround.hlsl` | grid 렌더와 `Target_PickPos` 출력 |
| `Client/Private/Loader.cpp` | ASSET_TEST 전용 Ground shader/object Prototype 등록 |
| `Client/Public/Level_AssetTest.h` | Ground layer 준비 함수 선언 |
| `Client/Private/Level_AssetTest.cpp` | `Layer_EditorGround` 생성 |
| `Client/Default/Client.vcxproj` | 새 cpp/h/shader 프로젝트 항목 |
| `Client/Default/Client.vcxproj.filters` | `12.BinaryAsset/30.Editor` 필터 배치 |

### 선택 수정

| 파일 | 조건 |
|---|---|
| `Engine/Private/ImGuiLayer.cpp` | imgui.ini 저장 경로를 엔진에서 통합할 경우 |
| `Client/Default/Client.cpp` | 단기적으로 메인 창 resize 자체를 막기로 결정할 경우 |
| `Client/Private/Camera_Free.cpp` | Home/Focus Selected 기능까지 포함할 경우 |

### 이번 작업에서 수정하지 않을 파일

- `.wmodel` reader/decoder/converter
- `CModel`의 `.fbx/.wmodel` 통합 경로
- Valtan/Scouter animation
- Gameplay Navigation 데이터
- 기존 `TerrainNavigation.dat`
- Map placement 저장 JSON 구조

---

## 6. 구현 순서

### Phase 0 — 현상 계측

1. 실제 game client width/height 표시.
2. client pixel, render pixel, pick source, world position 표시.
3. 창을 1280x720, 축소, 확대했을 때 값 캡처.
4. 현재 스크린샷의 `(-49, 0, -25)`가 어느 source에서 나온 것인지 확정.

완료 조건: 좌표 오류인지 PickPos 부재인지 UI에서 한눈에 구분된다.

### Phase 1 — 좌표와 입력 안정화

1. client→render 변환 함수 추가.
2. `CPicking` RowPitch 수정.
3. 외부 ImGui HWND 클릭과 game HWND 클릭을 분리.
4. 자동 Y=0 fallback 제거.
5. surface mode와 pick source 표시.

완료 조건: 게임 창을 줄이거나 늘려도 같은 화면 지점을 클릭하면 같은 대상 위치가 선택된다.

### Phase 2 — 기준 Ground 추가

1. `CAssetTestGround`와 전용 shader 추가.
2. ASSET_TEST Prototype 등록.
3. `Layer_EditorGround` 생성.
4. grid/axis와 `Target_PickPos` 동시 출력 확인.

완료 조건: F2 직후 파란 빈 화면이 아니라 원점 grid가 보이고, ground 위 마우스 좌표가 안정적으로 표시된다.

### Phase 3 — 배치 UX 안정화

1. placement preview marker.
2. MapTool 최소 폭과 좁은 창용 레이아웃.
3. Uniform Scale, Reset, bounds/anchor 표시.
4. 비정상 bounds/defaultScale 경고.

완료 조건: 배치 전에 결과 위치를 볼 수 있고, Scale을 바꿔도 왜 오브젝트 원점이 이동하는지 UI에서 확인 가능하다.

### Phase 4 — 저장/불러오기 회귀 검증

1. ground 위에 서로 다른 에셋 3개 배치.
2. position/rotation/uniform scale 변경.
3. 저장 후 ASSET_TEST 재진입.
4. ID, transform, visibility가 동일한지 확인.
5. `CAssetTestGround`는 저장 목록에 포함되지 않는지 확인.

---

## 7. 검증표

| 검증 | 절차 | 기대 결과 |
|---|---|---|
| 시작 화면 | Logo → F2 | 원점 grid와 테스트 모델이 보임 |
| 외부 창 | F1 후 MapTool을 게임 창 밖으로 이동 | 게임 렌더와 Tool 모두 유지 |
| ImGui 클릭 | Palette/Inspector 클릭 | 월드에 오브젝트가 생성되지 않음 |
| 1280x720 피킹 | Ground 중앙 클릭 | 거의 원점 부근 배치 |
| 축소 창 피킹 | 게임 창 축소 후 같은 시각 지점 클릭 | 좌표가 비례 보정되어 같은 지점 배치 |
| 확대 창 피킹 | 게임 창 확대 후 같은 시각 지점 클릭 | 좌표가 비례 보정되어 같은 지점 배치 |
| surface only | 파란 배경/무효 영역 클릭 | 생성하지 않고 Invalid 표시 |
| ground mode | 명시적 Y=0 모드로 클릭 | Pick source가 Ground Y=0으로 표시 |
| preview | ARMED 상태로 마우스 이동 | 유효/무효 마커 표시 |
| uniform scale | Scale 1→0.5 | XYZ 모두 0.5, anchor 유지 |
| non-uniform | 옵션 켜고 한 축 변경 | 선택한 축만 변경 |
| bounds | 큰 메시 선택 | local/world bounds와 경고 표시 |
| save/load | 3개 배치 후 저장/재진입 | 동일 ID/transform 복구 |
| regression | Logo와 Gameplay 진입 | 기존 수업 흐름 영향 없음 |
| navigation | 실행 전후 파일 비교 | TerrainNavigation 데이터가 변경되지 않음 |

---

## 8. 빌드 및 런타임 검증 계획

구현이 승인된 다음 아래 순서로 닫는다.

```powershell
msbuild LostArk.sln /t:Build /p:Configuration=Debug /p:Platform=x64 /m
```

그 다음 실행 검증:

```text
1. Logo 정상 표시
2. F2 -> ASSET_TEST
3. Ground/grid 표시
4. F1 -> 외부 MapTool
5. 게임 창 1280x720 / 축소 / 확대 각각 피킹
6. 에셋 3개 배치
7. uniform/non-uniform scale 확인
8. 저장 -> 재진입 -> 불러오기
9. 종료 시 예외 없음
```

캡처해야 할 화면:

- Ground 중앙을 가리키는 preview marker
- Tool의 client/render pixel과 world position
- 원점 주변에 생성된 오브젝트
- Scale 전후 anchor/world origin/bounds
- 창 크기를 바꾼 뒤에도 같은 위치에 배치되는 결과

---

## 9. 완료 판정

다음이 모두 만족되어야 수정 완료로 본다.

- 외부 ImGui viewport를 유지해도 잘못된 게임 클릭이 발생하지 않는다.
- 게임 창 크기와 무관하게 피킹 좌표가 렌더 좌표에 맞게 변환된다.
- 자동으로 보이지 않는 Y=0 fallback을 사용하지 않는다.
- F2 직후 ASSET_TEST에 눈에 보이는 기준 Ground가 있다.
- Ground shader가 `Target_PickPos`를 정상 출력한다.
- 배치 전에 좌표와 source를 확인할 수 있다.
- 기본 Scale은 uniform이며 anchor와 실제 world origin을 구분해 볼 수 있다.
- 기준 Ground는 Map 저장 대상과 Navigation에 영향을 주지 않는다.
- 기존 Logo, Loading, Gameplay, Valtan `.wmodel` 경로가 유지된다.
- Debug x64 전체 솔루션 빌드와 종료까지 통과한다.

## 10. 이번 계획의 핵심 선택

```text
외부 ImGui 제거                 -> 하지 않음
수업 CTerrain 즉시 재사용       -> 하지 않음
보이지 않는 Y=0 자동 fallback   -> 제거
ASSET_TEST 전용 visible ground  -> 추가
고정 렌더와 client 좌표 변환     -> 우선 적용
전체 DX11 resize 파이프라인       -> 후속 선택 작업
```

이 순서면 먼저 “클릭한 곳과 생성 위치가 같은가”를 단순한 평면으로 확정한 뒤, 실제 높이맵 Terrain과 복잡한 맵 메시 피킹으로 확장할 수 있다.
