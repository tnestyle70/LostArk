# LostArk Asset Palette → Y=0 Picking → Binary Render 계획

> 작성일: 2026-07-29  
> 상태: **PLAN ONLY — 실제 코드/프로젝트/리소스는 수정하지 않음**  
> 목표: 기존 Logo/Loading/Gameplay를 유지한 채 AssetTest에서 F1 MapTool을 열고, 발탄 맵·스카우터·발탄 카탈로그 항목을 선택한 뒤 지면을 한 번 클릭하여 바이너리 메시를 생성하고 렌더링하는 첫 번째 닫힌 흐름을 만든다.

## 0. 이번 계획의 한 줄 결론

```text
공용 AssetCatalog 로드
→ ImGui 목록 표시
→ 사용자가 항목 한 개 선택(Armed)
→ 게임 화면을 한 번 클릭
→ 카메라 Ray와 Y=0 평면의 교점 계산
→ CBinaryAssetObject Clone
→ CCookedModel / WModelDecoder
→ NONBLEND / Target_PickPos 포함 기존 렌더 경로
→ 화면 표시
```

첫 번째 완료선은 **목록 선택 → 단발 배치 → 렌더 성공**이다. 저장, Undo/Redo, Transform Inspector, 실제 애니메이션 재생, Gameplay 배치는 이번 범위에 넣지 않는다.

---

## 1. 현재 추출물 검토 결과

### 1.1 실제 리소스 위치

현재 LostArk 로컬 런타임에는 Annie 테스트 팩만 있다.

```text
C:\Users\user\Desktop\LostArk\Client\Bin\Resources\LostArk\Packs\dev-lol-annie
```

발탄·스카우터·맵 추출물은 현재 Winters 출력에 있다.

```text
C:\Users\user\Desktop\Winters\Client\Bin\Resource\LostArk
├─ AssetCatalog.json
├─ Map\BG_RAD_VALTAN_A\...
└─ Character
   ├─ SK_GSC_BST_00
   └─ MN_RPBF_01
```

LostArk 런타임이 Winters 저장소의 절대 경로를 직접 하드코딩해서 읽는 구조는 금지한다. 구현 시에는 공용 Drive 또는 LostArk 로컬 동기화 루트를 `LOSTARK_SHARED_ASSET_ROOT`로 지정한다.

### 1.2 카탈로그 및 바이너리 상태

`AssetCatalog.json`은 schemaVersion 1이며 총 19행이다.

| 분류 | 수량 | 상태 |
|---|---:|---|
| 발탄 맵 메시 | 17 | WINT/WMSH v1, static stride 48, bone 0 |
| 스카우터 `SK_GSC_BST_00` | 1 | WINT/WMSH v1, skinned stride 76, bone 17 |
| 발탄 본체 `MN_RPBF_01` | 1 | WINT/WMSH v1, skinned stride 76, bone 87 |

전체 파일 수 및 용량:

| 확장자 | 수량 | 총 바이트 |
|---|---:|---:|
| `.wmesh` | 19 | 9,300,030 |
| `.wmat` | 19 | 17,144 |
| `.wskel` | 2 | 26,976 |
| `.wanim` | 28 | 10,307,028 |
| `.png` | 28 | 20,562,604 |
| `.json` | 1 | 9,495 |

헤더 검사 결과 19개 `.wmesh` 모두 현재 `CWModelDecoder::CanDecode()`가 기대하는 `WINT + WMSH` 구조다.

### 1.3 현재 LostArk에서 바로 막히는 부분

#### A. Material 텍스처 루트 불일치

새 `.wmat` 19개에는 총 28개의 텍스처 경로가 들어 있다.

예:

```text
Resource/LostArk/Character/MN_RPBF_01/textures/mn_rpbf_01_d.png
```

현재 `CWModelDecoder::ResolveTexturePath()`는 상대 경로를 `.wmat` 파일의 부모 폴더에 붙인다.

```text
현재 계산:
<material folder>/Resource/LostArk/...
→ 28개 중 0개 존재

콘텐츠 루트 계산:
<shared content root>/Resource/LostArk/...
→ 28개 중 28개 존재
```

따라서 Palette를 만들기 전에 `MODEL_ASSET_LOAD_DESC`에 `contentRoot`를 전달하고 Material 경로를 콘텐츠 루트 기준으로도 해석해야 한다. 이 선행 작업 없이 모델은 생성되더라도 흰색 fallback 텍스처로 보일 수 있다.

#### B. Character는 현재 애니메이션 재생 불가

현재 WModelDecoder는 skinned stride 76을 인식하지만 position/normal/uv/tangent만 `VTXMESH`로 옮기고 bone weight/index는 렌더링에 연결하지 않는다.

```text
발탄/스카우터 첫 단계 결과
= bind-pose 미리보기
≠ .wskel/.wanim 애니메이션 재생
```

이번 Palette 단계에서는 캐릭터 행을 `BindPose Preview`로 표시하고 생성은 허용한다. 실제 애니메이션은 별도 계획으로 분리한다.

---

## 2. Y=0과 기존 수업 Picking의 정확한 차이

### 2.1 LostArk 수업 Picking

현재 `Engine/Private/Picking.cpp`는 `Target_PickPos` 렌더 타깃 전체를 staging texture로 복사한 후 마우스 픽셀에 기록된 월드 위치를 반환한다.

```cpp
if (0 != m_pWorldPositions[iIndex].w)
{
    vOut = m_pWorldPositions[iIndex];
    return true;
}
```

따라서 수업 Picking의 Y는 0으로 고정되지 않는다.

```text
Terrain을 클릭 → Terrain 표면의 실제 Y
Model을 클릭   → Model 표면의 실제 Y
배경을 클릭    → w == 0, Picking 실패
```

### 2.2 Winters Editor Picking

Winters `TryPickGroundPlane()`은 카메라 Ray와 `Y=0` 평면을 직접 교차시킨다.

```cpp
const float t = -ray.Origin.y / ray.Dir.y;
outWorld = {
    ray.Origin.x + ray.Dir.x * t,
    0.f,
    ray.Origin.z + ray.Dir.z * t
};
```

### 2.3 이번 단계의 결정

두 정책을 섞어서 의미를 흐리지 않는다.

```cpp
enum class ASSET_PLACEMENT_SURFACE
{
    FLAT_GROUND_Y0,  // 이번 AssetTest 기본값
    VISIBLE_SURFACE  // 추후 맵 표면 배치용
};
```

- 첫 번째 AssetTest 닫힌 흐름: 카메라 Ray × Y=0 평면
- 기존 Gameplay/Player: 기존 `CGameInstance::Picking()` 그대로 유지
- 추후 실제 맵 지형 위 배치: `VISIBLE_SURFACE` 모드에서 기존 Picking 결과의 Y를 그대로 사용

즉, **Y=0은 엔진 Picking의 규칙이 아니라 AssetTest 첫 배치 모드의 정책**이다.

---

## 3. 범위 고정

### 이번에 구현할 것

1. 공용 콘텐츠 루트 결정
2. AssetCatalog schemaVersion 1 로드/검증
3. 19개 에셋 목록을 ImGui에 표시
4. Map/Character 필터와 문자열 검색
5. 사용 가능한 행 선택 시 Placement Armed 상태 진입
6. ImGui가 마우스를 캡처하지 않을 때만 게임 화면 클릭 처리
7. 카메라 Ray와 Y=0 평면 교차
8. 클릭 한 번당 `CBinaryAssetObject` 한 개 생성
9. Map은 static textured render
10. Scouter/Valtan은 bind-pose textured preview
11. 디코더/생성 실패 이유를 MapTool에 표시
12. Esc 또는 다른 행 선택으로 배치 취소/변경

### 이번에 구현하지 않을 것

- `.wskel/.wanim` 실제 애니메이션
- 발탄 AI/스카우터 게임 로직
- 배치 문서 Save/Load
- Undo/Redo
- Transform Inspector
- 기즈모
- 배치 오브젝트 삭제
- Gameplay/Dungeon 반영
- Assimp fallback
- 배치된 오브젝트 클릭 재선택/AABB outline

`배치된 오브젝트 클릭 재선택`은 Palette → 배치 → 렌더가 안정화된 다음 두 번째 닫힌 흐름에서 Winters의 Ray/AABB 방식을 옮긴다.

---

## 4. 제안 파일 및 필터

실제 물리 파일은 현재 `Public/Private` 평면 구조를 유지하고 Visual Studio 필터만 분류한다.

```text
03.AssetSystem
├─ Catalog
│  ├─ AssetCatalog.h
│  └─ AssetCatalog.cpp
└─ Runtime
   ├─ RuntimeAssetRoot.h
   └─ RuntimeAssetRoot.cpp

04.Tools
└─ Map
   ├─ MapTool.h
   └─ MapTool.cpp

02.GameObjects
└─ 00.Shared
   └─ Asset
      ├─ BinaryAssetObject.h
      └─ BinaryAssetObject.cpp
```

예정 변경 파일:

```text
Client/Public/AssetCatalog.h                       (신규)
Client/Private/AssetCatalog.cpp                    (신규)
Client/Public/MapTool.h                            (수정)
Client/Private/MapTool.cpp                         (수정)
Client/Private/MainApp.cpp                         (수정)
Client/Public/RuntimeAssetRoot.h                    (수정)
Client/Private/RuntimeAssetRoot.cpp                 (수정)
Client/Public/BinaryAssetObject.h                   (필요 시 contentRoot 전달)
Engine/Public/BinaryAsset/ModelAssetData.h          (contentRoot 추가)
Engine/Private/BinaryAsset/WModelDecoder.cpp        (texture root 해석 보정)
Client/Default/Client.vcxproj                       (신규 파일 등록)
Client/Default/Client.vcxproj.filters               (필터 등록)
Client/ThirdParty/nlohmann/json.hpp                 (vendored single header 검토)
```

JSON parser는 팀원 PC의 vcpkg 설치에 의존하지 않도록 Winters에서 사용 중인 nlohmann single header의 버전을 고정하여 저장소에 포함하는 방향을 우선한다.

---

## 5. 공용 콘텐츠 루트 계약

`LOSTARK_SHARED_ASSET_ROOT`는 개별 캐릭터 폴더가 아니라 `Resource` 폴더를 포함하는 콘텐츠 루트를 가리킨다.

예:

```text
LOSTARK_SHARED_ASSET_ROOT=D:\TeamLostArkContent

D:\TeamLostArkContent
└─ Resource
   └─ LostArk
      ├─ AssetCatalog.json
      ├─ Map\...
      └─ Character\...
```

환경 변수가 없을 때 개발 fallback은 `Client.exe`가 있는 `Client/Bin`을 콘텐츠 루트로 사용한다.

```cpp
filesystem::path CRuntimeAssetRoot::GetContentRoot();
filesystem::path CRuntimeAssetRoot::ResolveContent(const filesystem::path& relative);
filesystem::path CRuntimeAssetRoot::GetCatalogPath();
```

계약:

```cpp
GetCatalogPath()
= GetContentRoot() / L"Resource/LostArk/AssetCatalog.json";
```

어떤 코드에도 다음 경로를 저장하지 않는다.

```text
C:\Users\user\Desktop\Winters\...
C:\Users\<팀원 이름>\...
```

---

## 6. Catalog 데이터 구조 계획

```cpp
enum class ASSET_PREVIEW_CAPABILITY
{
    STATIC_READY,
    BIND_POSE_ONLY,
    MISSING,
    INVALID
};

struct ASSET_CATALOG_ENTRY
{
    string id;
    string label;
    string kind;

    filesystem::path meshPath;
    filesystem::path materialPath;
    filesystem::path diffusePath;
    filesystem::path contentRoot;

    string defaultAnimation;
    f32_t defaultScale = 1.f;
    bool_t animated = false;
    bool_t textured = false;
    ASSET_PREVIEW_CAPABILITY capability = ASSET_PREVIEW_CAPABILITY::INVALID;
};
```

```cpp
class CAssetCatalog final
{
public:
    bool_t Reload();
    const vector<ASSET_CATALOG_ENTRY>& GetEntries() const;
    const ASSET_CATALOG_ENTRY* Find(const string& id) const;
    const string& GetStatus() const;
};
```

행 검증 순서:

1. schemaVersion == 1
2. assets가 배열
3. id 중복 금지
4. kind는 Map 또는 Character
5. defaultScale은 finite, 0보다 큼
6. mesh 상대 경로는 contentRoot 밖으로 탈출 금지
7. `.wmesh` 파일 존재
8. 같은 이름의 `.wmat` 존재
9. diffuse 파일 존재
10. 첫 20바이트가 `WINT + WMSH`
11. static이면 `STATIC_READY`
12. skinned이면 이번 단계에서 `BIND_POSE_ONLY`

Catalog에 `.wmat` 필드가 없으므로 첫 단계에서는 mesh 확장자를 `.wmat`으로 교체하여 companion path를 만든다.

---

## 7. Material 경로 보정 계획

`MODEL_ASSET_LOAD_DESC`에 콘텐츠 루트를 추가한다.

```cpp
struct MODEL_ASSET_LOAD_DESC
{
    filesystem::path contentRoot;
    filesystem::path meshPath;
    filesystem::path materialPath;
    filesystem::path fallbackDiffusePath;
};
```

WMat 텍스처 후보 순서:

```cpp
filesystem::path ResolveTexturePath(
    const filesystem::path& contentRoot,
    const filesystem::path& materialPath,
    const wchar_t* storedPath)
{
    filesystem::path value(storedPath);

    if (value.is_absolute() && filesystem::exists(value))
        return value.lexically_normal();

    const filesystem::path besideMaterial =
        (materialPath.parent_path() / value).lexically_normal();
    if (filesystem::exists(besideMaterial))
        return besideMaterial;

    const filesystem::path belowContentRoot =
        (contentRoot / value).lexically_normal();
    if (filesystem::exists(belowContentRoot))
        return belowContentRoot;

    return {};
}
```

경로는 `lexically_normal()` 후 반드시 contentRoot 내부인지 검사한다. `..`, rooted device path, UNC 등으로 루트 밖을 참조하면 해당 Catalog 행을 Invalid 처리한다.

---

## 8. MapTool 상태 계획

```cpp
enum class ASSET_PLACEMENT_STATE
{
    IDLE,
    ARMED
};

class CMapTool final
{
private:
    bool_t m_bOpen = false;
    bool_t m_bPreviousLeftDown = false;

    CAssetCatalog m_AssetCatalog;
    ASSET_PLACEMENT_STATE m_ePlacementState = ASSET_PLACEMENT_STATE::IDLE;
    string m_SelectedAssetId;
    string m_AssetFilter;
    string m_PlacementStatus;

public:
    void Update(f32_t fTimeDelta);
    void Render();

private:
    void RenderAssetPalette();
    bool_t TryPickGroundY0(float3_t& outWorld) const;
    bool_t TryPlaceSelected(const float3_t& worldPosition);
    void CancelPlacement();
};
```

상태 전이:

```text
IDLE
  └─ 사용 가능한 Catalog 행 클릭
       → ARMED(selectedAssetId)

ARMED
  ├─ 게임 화면 좌클릭 성공
  │    → BinaryAssetObject 생성
  │    → IDLE
  ├─ Esc
  │    → IDLE
  ├─ 다른 행 클릭
  │    → ARMED(newAssetId)
  └─ F1로 Tool 닫기
       → IDLE
```

한 번 클릭한 동안 여러 개 생성되지 않도록 반드시 좌클릭 edge를 사용한다.

```cpp
const bool_t leftDown =
    0 != (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
const bool_t leftPressed = leftDown && !m_bPreviousLeftDown;
m_bPreviousLeftDown = leftDown;
```

---

## 9. ImGui Palette 계획

첫 UI는 기능만 확인하도록 단순하게 유지한다.

```text
LostArk Map Tool
┌──────────────────────────────────────────┐
│ Asset Catalog: Ready (19)                │
│ [Reload] [Filter:________________]       │
│ [All] [Map 17] [Character 2]             │
├──────────────────────────────────────────┤
│ Valtan package prop / ...   Ready/Tex    │
│ Valtan package prop / ...   Ready/Tex    │
│ Scouter / ...               BindPose     │
│ Valtan boss / ...           BindPose     │
├──────────────────────────────────────────┤
│ Armed: MN_RPBF_01                        │
│ Click the game ground once / Esc cancel │
└──────────────────────────────────────────┘
```

행 클릭 코드 형태:

```cpp
if (ImGui::Selectable(entry.label.c_str(), selected))
{
    m_SelectedAssetId = entry.id;
    m_ePlacementState = ASSET_PLACEMENT_STATE::ARMED;
    m_PlacementStatus = "Armed: " + entry.id;
}
```

상태 색:

```text
STATIC_READY   → 초록색 Ready/Tex
BIND_POSE_ONLY → 노란색 BindPose
MISSING        → 빨간색 Missing, 선택 불가
INVALID        → 빨간색 Invalid, 선택 불가
```

---

## 10. Y=0 Ray Picking 코드 계획

LostArk의 현재 View/Projection과 viewport를 사용하여 near/far 점을 unproject한다.

```cpp
bool_t CMapTool::TryPickGroundY0(float3_t& outWorld) const
{
    if (CGameInstance::Get().IsMouseInputBlocked())
        return false;

    POINT mouse{};
    GetCursorPos(&mouse);
    ScreenToClient(g_hWnd, &mouse);

    const float2_t viewport = CGameInstance::Get().Get_ViewportSize();
    if (mouse.x < 0 || mouse.y < 0 ||
        mouse.x >= static_cast<LONG>(viewport.x) ||
        mouse.y >= static_cast<LONG>(viewport.y))
        return false;

    const matrix_t view = XMLoadFloat4x4(
        CGameInstance::Get().Get_Transform(D3DTS::VIEW));
    const matrix_t proj = XMLoadFloat4x4(
        CGameInstance::Get().Get_Transform(D3DTS::PROJ));

    const vector_t nearPoint = XMVector3Unproject(
        XMVectorSet(static_cast<f32_t>(mouse.x), static_cast<f32_t>(mouse.y), 0.f, 1.f),
        0.f, 0.f, viewport.x, viewport.y, 0.f, 1.f,
        proj, view, XMMatrixIdentity());

    const vector_t farPoint = XMVector3Unproject(
        XMVectorSet(static_cast<f32_t>(mouse.x), static_cast<f32_t>(mouse.y), 1.f, 1.f),
        0.f, 0.f, viewport.x, viewport.y, 0.f, 1.f,
        proj, view, XMMatrixIdentity());

    const vector_t direction = XMVector3Normalize(farPoint - nearPoint);
    const f32_t dirY = XMVectorGetY(direction);
    if (fabsf(dirY) < 1.e-4f)
        return false;

    const f32_t t = -XMVectorGetY(nearPoint) / dirY;
    if (t < 0.f)
        return false;

    float3_t hit{};
    XMStoreFloat3(&hit, nearPoint + direction * t);
    hit.y = 0.f;
    outWorld = hit;
    return true;
}
```

이 함수는 기존 `CPicking`을 교체하지 않는다. MapTool의 `FLAT_GROUND_Y0` 정책 전용이다.

---

## 11. MainApp 업데이트 순서

ImGui 클릭이 월드 배치로 새지 않도록 입력 차단을 먼저 계산한다.

```cpp
void CMainApp::Update(f32_t fTimeDelta)
{
#ifdef _DEBUG
    UpdateDebugToolShortcut();

    if (m_pImGuiLayer)
        m_pImGuiLayer->BeginFrame();

    const bool_t toolOpen = m_pMapTool && m_pMapTool->IsOpen();
    const bool_t keyboardCaptured = toolOpen &&
        m_pImGuiLayer && m_pImGuiLayer->WantsCaptureKeyboard();
    const bool_t mouseCaptured = toolOpen &&
        m_pImGuiLayer && m_pImGuiLayer->WantsCaptureMouse();

    CGameInstance::Get().SetInputBlocked(
        keyboardCaptured,
        mouseCaptured);
#endif

    CGameInstance::Get().Update_Engine(fTimeDelta);

#ifdef _DEBUG
    if (m_pMapTool)
        m_pMapTool->Update(fTimeDelta);
#endif
}
```

MapTool Update를 Engine Update 뒤에 두는 이유는 현재 프레임 DirectInput 상태와 카메라 View/Projection이 갱신된 다음 배치 Ray를 계산하기 위해서다. 생성된 오브젝트는 다음 프레임부터 Render Queue에 들어가도 문제없다.

---

## 12. 실제 생성 코드 계획

```cpp
bool_t CMapTool::TryPlaceSelected(const float3_t& worldPosition)
{
    const ASSET_CATALOG_ENTRY* entry =
        m_AssetCatalog.Find(m_SelectedAssetId);
    if (!entry ||
        (entry->capability != ASSET_PREVIEW_CAPABILITY::STATIC_READY &&
         entry->capability != ASSET_PREVIEW_CAPABILITY::BIND_POSE_ONLY))
        return false;

    CBinaryAssetObject::BINARY_ASSET_DESC desc{};
    desc.asset.contentRoot = entry->contentRoot;
    desc.asset.meshPath = entry->meshPath;
    desc.asset.materialPath = entry->materialPath;
    desc.asset.fallbackDiffusePath = entry->diffusePath;
    desc.position = worldPosition;
    desc.scale = entry->defaultScale;

    const HRESULT hr = CGameInstance::Get().Add_GameObject_to_Layer(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_GameObject_BinaryAsset"),
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Layer_BinaryAsset"),
        &desc);

    if (FAILED(hr))
        return false;

    m_PlacementStatus = "Placed: " + entry->id;
    m_ePlacementState = ASSET_PLACEMENT_STATE::IDLE;
    return true;
}
```

`CBinaryAssetObject`의 기존 흐름은 그대로 재사용한다.

```text
Clone
→ Initialize(desc)
→ CCookedModel::Create
→ DecoderRegistry
→ CWModelDecoder
→ Vertex/Index Buffer
→ Late_Update NONBLEND 등록
→ Shader_VtxMesh pass 0
→ Target_Diffuse/Normal/Depth/PickPos
→ 화면 출력
```

---

## 13. 구현 순서

### P0. 입력 에셋 게이트

1. 19개 Catalog 행 검사 도구/로그
2. WINT/WMSH v1 확인
3. `.wmat` companion 확인
4. diffuse 28개 콘텐츠 루트 해석 확인
5. Map/Character capability 분류

완료 조건:

```text
Catalog Ready: 19
Map Ready: 17
Character BindPoseOnly: 2
Missing texture: 0
```

### P1. 콘텐츠 루트와 Material 경로

1. `LOSTARK_SHARED_ASSET_ROOT` 계약 변경
2. Catalog 경로 결정
3. `MODEL_ASSET_LOAD_DESC.contentRoot` 추가
4. WMat texture root 후보 추가
5. Annie 테스트 경로 회귀 확인

완료 조건: Annie와 발탄 Map 한 개가 모두 올바른 텍스처로 렌더링된다.

### P2. Catalog와 ImGui Palette

1. JSON parser 고정
2. `CAssetCatalog` 작성
3. Reload 버튼
4. 검색/종류 필터
5. Ready/BindPose/Missing 상태
6. 행 클릭 → Armed

완료 조건: F1 패널에서 19개가 보이고 발탄/스카우터 검색이 된다.

### P3. Y=0 Picking과 단발 생성

1. MapTool Update 추가
2. 마우스 edge 처리
3. ImGui capture 차단
4. Ray × Y=0
5. BinaryAssetObject 생성
6. 성공 후 Idle 복귀
7. Esc 취소

완료 조건: 한 번 클릭에 정확히 한 개 생성되고, 클릭 위치의 X/Z와 생성 Transform이 일치하며 Y는 정확히 0이다.

### P4. 렌더 회귀와 실패 표시

1. Map 17종 중 최소 3종 렌더
2. multi-material Floor 렌더
3. Scouter bind pose 렌더
4. Valtan bind pose 렌더
5. Decoder 실패 이유 UI 출력
6. Logo/Gameplay 기존 흐름 확인

완료 조건: placeholder cube나 Assimp fallback 없이 실제 추출 바이너리 형상이 보인다.

### P5. 다음 별도 닫힌 흐름

첫 완료선 이후에만 진행한다.

1. `CCookedModel` local AABB 노출
2. 배치 record와 runtime object handle 연결
3. Ray/AABB nearest hit
4. 배치된 오브젝트 클릭 재선택
5. 선택 outline
6. Transform Inspector
7. Delete/Save/Load/Undo/Redo

---

## 14. 검증 시나리오

### 정상 시나리오

1. Client 실행
2. Loading → Enter → Logo
3. F2 → AssetTest Loading → Enter
4. F1 → MapTool
5. Catalog Ready 19 확인
6. `FLOOR01` 검색
7. Map 행 클릭
8. 게임 화면 클릭
9. Y=0에 실제 Floor 메시 렌더 확인
10. `Scouter` 검색 후 클릭/배치
11. bind-pose 경고와 실제 스카우터 형상 확인
12. `Valtan boss` 검색 후 클릭/배치
13. bind-pose 경고와 실제 발탄 형상 확인

### 입력 안전성

- ImGui 목록 클릭은 월드에 생성하지 않는다.
- 좌클릭을 누르고 있어도 한 개만 생성한다.
- F1로 패널을 닫으면 Armed 상태를 취소한다.
- Esc는 Armed만 취소하고 Level을 변경하지 않는다.
- 카메라 이동은 ImGui가 입력을 잡지 않을 때 유지된다.

### 실패 시나리오

- Catalog 없음 → `Catalog missing`, 기존 목록 유지 또는 빈 상태
- schema 불일치 → `Catalog invalid`, 행 선택 금지
- mesh 없음 → `Missing`, 행 비활성화
- texture 없음 → 생성 전 Missing 처리, 흰색 fallback으로 성공처럼 보이지 않음
- decoder header 불일치 → `Invalid format`, Assimp fallback 금지
- 캐릭터 animation 요청 → `BindPoseOnly`, 애니메이션 성공으로 표시 금지

---

## 15. 완료 정의

다음을 모두 만족해야 이번 계획의 구현이 완료된 것으로 본다.

- [ ] 코드나 카탈로그에 개인 PC 절대 경로가 없다.
- [ ] Catalog 19행이 모두 검증된다.
- [ ] Map 17행은 Ready, Character 2행은 BindPoseOnly로 구분된다.
- [ ] `.wmat` 텍스처 경로 28개가 모두 콘텐츠 루트에서 해석된다.
- [ ] ImGui 행 클릭으로 단발 Placement Armed 상태가 된다.
- [ ] ImGui 위 클릭은 월드 배치를 일으키지 않는다.
- [ ] 월드 클릭 한 번에 오브젝트 한 개만 생성된다.
- [ ] 초기 배치 Y는 정확히 0이다.
- [ ] Map static textured render가 성공한다.
- [ ] Scouter/Valtan bind-pose preview가 성공한다.
- [ ] 기존 Logo/Loading/Gameplay와 Annie smoke가 깨지지 않는다.
- [ ] Assimp fallback을 사용하지 않는다.
- [ ] Debug x64 빌드가 성공한다.
- [ ] `git diff --check`가 통과한다.

---

## 16. 구현 전 최종 확인 사항

1. `LOSTARK_SHARED_ASSET_ROOT`를 공용 Drive root로 둘지 `Client/Bin` 동기화 root로 둘지 경로만 확정한다. 구조는 둘 다 동일하게 `Resource/LostArk/...`를 유지한다.
2. 첫 완료선에서는 발탄/스카우터가 **움직이지 않는 bind-pose**라는 점을 UI에 명확히 표시한다.
3. 실제 발탄 맵 전체 조립 좌표는 Catalog에 없으므로 이번 단계는 개별 mesh 수동 배치까지만 한다.
4. 배치된 오브젝트 재선택/편집은 P5로 분리하여 첫 Palette/Picking/Render 흐름을 먼저 닫는다.

이 문서는 구현 순서와 코드 계약만 정의한다. 작성 시점에 C++ 파일, vcxproj, filters, 리소스 복사 및 런타임 설정은 변경하지 않았다.
