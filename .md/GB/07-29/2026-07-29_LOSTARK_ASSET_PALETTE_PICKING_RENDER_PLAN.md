# Session - LostArk ImGui 맵 에셋 팔레트 · 피킹 · 배치 · 렌더 계획

> 작성일: 2026-07-29
> 상태: **IMPLEMENTATION APPROVED / IN PROGRESS**
> 기준 저장소: `C:\Users\user\Desktop\LostArk`
> 비교 기준: `C:\Users\user\Desktop\Winters\Client\Private\Scene\Editor\Scene_Editor.cpp`
> 이 문서를 구현 계약으로 사용해 Client/Engine/리소스 반영, 빌드, 런타임 검증, RESULT 작성까지 닫는다.

## 0. 이번 계획의 확정 결론

질문의 답은 **거의 맞다.** 다만 Prototype을 두 종류로 나눠서 생각해야 한다.

```text
맵 파츠 17개
  -> CModel 컴포넌트 Prototype 17개

실제 월드에 놓이는 맵 오브젝트
  -> 공용 CMapAssetObject 게임오브젝트 Prototype 1개
```

17개마다 `CRock`, `CFloor`, `CStatue` 같은 게임오브젝트 클래스를 만들지 않는다.

에디터에서 한 행을 선택하면 그 행이 가리키는 **모델 Prototype 태그**를 기억한다. 다음 월드 좌클릭 위치에서 공용 `CMapAssetObject`를 Clone하고, Clone 인자로 선택된 모델 태그와 Transform을 넘긴다.

```text
F2 -> LEVEL::ASSET_TEST
F1 -> MapTool 열기
17개 에셋 중 한 행 클릭
-> Placement Armed
-> 월드 좌클릭
-> 보이는 표면 PickPos, 없으면 Y=0 평면 교차점 계산
-> Prototype_GameObject_MapAsset Clone
-> 선택한 Prototype_Component_Model_Map_* Clone
-> CModel 렌더
```

이것이 현재 LostArk 수업 프레임워크와 Winters 에디터 경험을 가장 적게 비틀면서 합치는 구조다.

---

## 1. 현재 코드에서 확인된 사실

### 1.1 이미 준비된 것

- `CMainApp`은 Debug에서 F1으로 `CMapTool`을 열고 닫는다.
- ImGui가 키보드/마우스를 잡았을 때 `CGameInstance::SetInputBlocked()`로 게임 입력을 차단한다.
- Logo에서 F2를 누르면 기존 Loading을 거쳐 `LEVEL::ASSET_TEST`로 이동한다.
- `CLevel_AssetTest`에는 자유 카메라, 조명, 발탄이 존재한다.
- `CModel::Initialize_Prototype()`은 `.wmodel`이면 `CWModelDecoder`를 거쳐 기존 `CMesh`, `CMaterial`, `CBone`, `CAnimation`으로 올린다.
- `CModel` Clone은 정적 mesh/material GPU 자원을 공유하고, bone/animation 상태만 필요한 경우 Clone한다.
- `CPicking`은 `Target_PickPos`에 기록된 전 프레임 월드 좌표를 CPU로 읽을 수 있다.
- `Tools/ModelAssetConverter`는 기존 `.wmesh + .wmat`을 하나의 `.wmodel`로 묶는 `pack` 명령을 지원한다.
- `CRuntimeAssetRoot`는 `LOSTARK_SHARED_ASSET_ROOT` 또는 실행 파일 옆 `Resources/LostArk`를 공용 리소스 루트로 해석한다.

### 1.2 아직 없는 것

- `CMapTool`은 현재 상태 표시 창뿐이며 에셋 목록·선택·배치 Update가 없다.
- 17개 맵 모델 Prototype이 `CLoader::Ready_For_Level_AssetTest()`에 등록되어 있지 않다.
- 공용 `CMapAssetObject`가 없다.
- 정적 `.wmodel`용 선택적 Normal 렌더 셰이더가 없다.
- 현재 맵 폴더의 17개는 `.wmesh/.wmat`이고 `.wmodel`은 0개다.
- `CPicking::Picking()`은 마우스가 창 밖일 때 인덱스 범위를 검사하지 않는다.
- `Target_PickPos`는 배경 픽셀에서 `w == 0`이므로 아무 모델도 없는 빈 화면은 찍지 못한다.
- 배치 결과 저장/불러오기와 배치된 오브젝트 선택은 아직 없다.

### 1.3 구현 전에 먼저 정리할 기존 이상점

현재 커밋 기준의 `Client/Private/Level_Test2.cpp`와 `Client/Public/Level_Test2.h`는 파일명과 달리 `CLevel_AssetTest`를 다시 정의한다. 다만 현재 작업 트리에서는 다른 세션이 이를 `CLevel_Test2`로 분리하는 수정 작업을 진행 중이다. 신규 맵 에디터 구현 전에 이 수정의 빌드 결과와 실제 레벨 연결 여부를 확인해야 한다.

결정:

- 이번 기능의 실제 레벨은 기존 `CLevel_AssetTest`를 사용한다.
- 현재 작업 트리의 `CLevel_Test2` 이름 분리가 컴파일·링크되는지 먼저 검증한다.
- `CLevel_Test2`가 별도 용도로 필요하면 유지하되 AssetTest Loader/MapTool 경로에는 섞지 않는다.
- 필요 없는 복사본으로 확정될 때만 별도 커밋에서 프로젝트와 필터에서 제거한다.

---

## 2. Winters에서 가져올 것과 가져오지 않을 것

### 2.1 가져올 편집 경험

Winters `CScene_Editor`의 현재 LostArk Asset 흐름은 다음과 같다.

```text
RenderLostArkAssetPalette
  -> ImGui::Selectable 행 클릭
  -> PlaceAsset 상태 진입

HandleLostArkAssetPlacement
  -> ImGui::GetIO().WantCaptureMouse 검사
  -> 좌클릭 edge 검사
  -> TryPickGroundPlane(Y=0)
  -> renderer와 transform을 가진 배치 인스턴스 생성
```

LostArk도 별도의 `Place Selected` 버튼 없이 동일하게 간다.

```text
목록 클릭 = 즉시 배치 준비
월드 한 번 클릭 = 한 개 생성 후 Idle
Esc = 배치 취소
다른 목록 클릭 = 배치 대상 교체
```

### 2.2 가져오지 않을 런타임 구조

Winters는 `ModelRenderer`를 에디터가 직접 소유한다. LostArk에서 이 방식을 복사하지 않는다.

LostArk는 수업 구조를 유지한다.

```text
CLoader
  -> Component Prototype 등록
  -> GameObject Prototype 등록

CMapAssetObject Clone
  -> CTransform
  -> CShader Component
  -> 선택된 CModel Component
  -> Renderer NONBLEND 등록
```

또한 예전에 만든 `CBinaryAssetObject -> CCookedModel` 경로를 이 기능의 주 경로로 되살리지 않는다. 현재 확정 기준은 `CModel(.fbx/.wmodel 공통 입구)`이다.

---

## 3. 17개 맵 파츠의 현재 입력 상태

현재 위치:

```text
Client/Bin/Resources/LostArk/Map/BG_RAD_VALTAN_A/
```

현재 17개 폴더:

```text
BG_RAD_VALTAN_CRYSTAL01_SM
BG_RAD_VALTAN_CRYSTAL01_SM_KHB
BG_RAD_VALTAN_CRYSTAL01A_SM
BG_RAD_VALTAN_CRYSTAL01A_SM_KHB
BG_RAD_VALTAN_CRYSTAL01B_SM_KHB
BG_RAD_VALTAN_CRYSTAL01C_SM_KHB
BG_RAD_VALTAN_CRYSTAL01D_SM_KHB
BG_RAD_VALTAN_CRYSTAL01E_SM_KHB
BG_RAD_VALTAN_ENTERPROP01_SM
BG_RAD_VALTAN_ENTERPROP01A_SM
BG_RAD_VALTAN_ENTERPROP01B_SM
BG_RAD_VALTAN_FLOOR01_SM
BG_RAD_VALTAN_FLOOR01A_SM
BG_RAD_VALTAN_FLOOR01B_SM
BG_RAD_VALTAN_STATUE01_SM
BG_RAD_VALTAN_TOOLANVIL01_SM
BG_RAD_VALTAN_TOOLANVIL01A_SM
```

각 폴더에는 현재 `.wmesh`, `.wmat`, `textures/`가 있고 `.wmodel`은 없다.

따라서 구현의 P0는 17개를 먼저 단일 파일로 묶는 것이다.

```powershell
.\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe pack `
  ".\Client\Bin\Resources\LostArk\Map\BG_RAD_VALTAN_A\<ASSET>\<ASSET>.wmesh" `
  -o ".\Client\Bin\Resources\LostArk\Map\BG_RAD_VALTAN_A\<ASSET>\<ASSET>.wmodel" `
  --material ".\Client\Bin\Resources\LostArk\Map\BG_RAD_VALTAN_A\<ASSET>\<ASSET>.wmat"

.\Tools\ModelAssetConverter\Bin\ModelAssetConverter.exe info `
  ".\Client\Bin\Resources\LostArk\Map\BG_RAD_VALTAN_A\<ASSET>\<ASSET>.wmodel"
```

정적 맵 파츠의 기대값:

```text
MODEL::NONANIM
skeleton 없음
animation 없음
mesh + material section 존재
defaultScale = 1.0
placementAnchor = BottomCenter
```

`pack`은 기존 정점의 scale/pretransform을 다시 바꾸지 않는다. 따라서 기존 `.wmesh`가 가진 좌표를 그대로 사용하고, Loader에서 추가 `0.01f` scaling을 임의로 넣지 않는다. 화면 검증 후 카탈로그 `defaultScale`만 조절한다.

---

## 4. 카탈로그가 Prototype 등록과 ImGui의 단일 기준이다

17개 경로를 Loader와 MapTool에 각각 하드코딩하면 두 목록이 언젠가 달라진다. 작은 카탈로그 하나가 두 곳의 입력이어야 한다.

확정 위치:

```text
Client/Bin/DataFiles/Map/BG_RAD_VALTAN_A.mapassets
```

`Client/Bin/DataFiles/**`는 현재 Git 추적 대상이므로 대용량 Resource zip과 분리해서 코드와 함께 공유할 수 있다.

외부 JSON 라이브러리를 추가하지 않고 `std::quoted` 기반의 좁은 버전형 텍스트 포맷을 사용한다. 공백이 포함된 label/path도 안전하게 읽고 Git diff도 가능하다.

```text
LOSTARK_MAP_ASSET_CATALOG 1 "BG_RAD_VALTAN_A" 17
"BG_RAD_VALTAN_FLOOR01_SM" "Valtan Floor 01" "Map/BG_RAD_VALTAN_A/BG_RAD_VALTAN_FLOOR01_SM/BG_RAD_VALTAN_FLOOR01_SM.wmodel" "Prototype_Component_Model_Map_BG_RAD_VALTAN_FLOOR01_SM" 1 1 1 BottomCenter
```

카탈로그 검증 규칙:

1. magic `LOSTARK_MAP_ASSET_CATALOG`, version `1`
2. `id`와 `prototypeTag` 중복 금지
3. 모델 경로 확장자는 `.wmodel`
4. `CRuntimeAssetRoot::Resolve(model)`로 경로를 만들고, 정규화 후 공용 LostArk 리소스 루트 밖으로 탈출 금지
5. 파일 존재 확인
6. scale은 유한수이며 각 축이 0보다 큼
7. 정적 맵 카탈로그에는 animation/skeleton 요구 금지
8. 17개 중 하나라도 누락되면 어떤 행인지 Loading/Debug 출력에 표시

카탈로그에는 사용자 PC의 절대 경로와 `../Bin`을 저장하지 않는다. 모델 경로는 항상 `CRuntimeAssetRoot` 기준 상대 경로다. 따라서 기본 로컬 폴더와 팀 공용 Drive 환경 변수가 같은 카탈로그를 사용한다.

---

## 5. Prototype 등록 설계

### 5.1 왜 모델 Prototype은 17개인가

각 `.wmodel`은 서로 다른 정점·인덱스·재질을 가진다. 따라서 `CModel` Prototype은 파일별로 하나씩 필요하다.

```text
Prototype_Component_Model_Map_BG_RAD_VALTAN_FLOOR01_SM
Prototype_Component_Model_Map_BG_RAD_VALTAN_STATUE01_SM
...
총 17개
```

`CModel` Prototype 생성 시 파일을 읽고 GPU buffer/material을 준비한다. 이후 같은 모델을 여러 번 배치하면 Clone이 그 자원을 공유하므로, 같은 바닥 파츠를 열 번 놓아도 모델 파일을 열 번 다시 읽는 구조가 아니다.

### 5.2 왜 게임오브젝트 Prototype은 하나인가

17개 파츠의 동작은 전부 같다.

```text
Transform 보유
선택된 정적 CModel 보유
동일 셰이더 사용
NONBLEND 렌더 그룹 등록
mesh별 material bind
```

따라서 클래스도 Prototype도 하나면 된다.

```text
Prototype_GameObject_MapAsset
```

Clone 인자:

```cpp
struct MAP_ASSET_DESC
{
    uint64_t placementId;
    std::wstring assetId;
    std::wstring modelPrototypeTag;
    float3_t position;
    float3_t rotationDegrees;
    float3_t scale;
};
```

`CMapAssetObject::Initialize()`는 `modelPrototypeTag`를 이용해 선택된 CModel Component를 붙인다.

### 5.3 등록 레벨

첫 구현은 모두 `LEVEL::ASSET_TEST`에 등록한다.

- Loader가 AssetTest 진입 때 17개 모델과 공용 오브젝트를 등록한다.
- AssetTest를 나가면 현재 `CLevel_Manager -> Clear_Resources(currentLevel)` 흐름에 따라 제거한다.
- 아직 `LEVEL::STATIC`에 올리지 않는다. Static에 두면 Logo/GamePlay까지 계속 살아 있어 메모리와 수명 경계가 흐려진다.
- 나중에 여러 레벨이 같은 맵 카탈로그를 공유할 때만 영속 Asset Registry/streaming 정책을 별도로 설계한다.

17개 규모에서는 전부 선등록이 맞다. 수백·수천 개로 늘어나면 목록 선택 시 지연 등록하는 lazy prototype cache로 바꾼다.

---

## 6. 정적 `.wmodel` 렌더 객체

신규 `CMapAssetObject`는 수업의 `CForkLift`와 현재 `CBody_Valtan`의 결합 방식을 따른다.

보유 컴포넌트:

```text
Com_Transform   CTransform
Com_Shader      Prototype_Component_Shader_VtxMeshBinary
Com_Model       카탈로그에서 선택된 17개 중 하나
```

렌더 흐름:

```text
Late_Update
  -> RENDERGROUP::NONBLEND

Render
  -> World/View/Proj bind
  -> mesh 순회
  -> diffuse bind
  -> normal 존재 여부 bind
  -> normal이 있을 때만 normal bind
  -> shader pass 0
  -> CModel::Render(meshIndex)
  -> Target_PickPos(SV_TARGET3)에 world position 기록
```

기존 수업 `Shader_VtxMesh.hlsl`을 맵 에셋 때문에 직접 바꾸지 않는다. LostArk 바이너리 맵용 셰이더를 분리한다.

```text
Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl
Prototype_Component_Shader_VtxMeshBinary
```

이유:

- 17개 중 다수가 diffuse-only다.
- 기존 수업 셰이더는 normal texture가 항상 있다고 가정한다.
- 발탄용 `Shader_VtxAnimMeshBinary.hlsl`은 애니메이션 정점 형식이라 정적 맵에 그대로 쓸 수 없다.
- WMA2의 specular/emissive/ORM 경로는 보존되지만, 현재 화면 조명은 우선 diffuse + optional normal까지만 닫는다.

---

## 7. ImGui MapTool 상태와 UX

상태는 처음에는 두 개면 충분하다.

```cpp
enum class MAP_PLACEMENT_STATE
{
    IDLE,
    ARMED
};
```

MapTool이 보관할 최소 상태:

```text
열림 여부
카탈로그 17행
검색 문자열
선택 assetId
배치 상태 IDLE/ARMED
이전 프레임 좌클릭 상태
마지막 성공/실패 메시지
```

화면:

```text
LostArk Map Tool
------------------------------------------------
Area: BG_RAD_VALTAN_A
Catalog: Ready 17/17
[Filter________________]

Floor
  BG_RAD_VALTAN_FLOOR01_SM       Ready
  BG_RAD_VALTAN_FLOOR01A_SM      Ready
Crystal
  BG_RAD_VALTAN_CRYSTAL01_SM     Ready
...

Armed: BG_RAD_VALTAN_STATUE01_SM
Click world once / Esc cancels
```

행 클릭 동작:

```cpp
selectedAssetId = entry.id;
placementState = MAP_PLACEMENT_STATE::ARMED;
```

별도 `Place Selected` 버튼을 두지 않는다. 이것이 현재 Winters LostArk Asset 행 클릭 동작과 맞는다.

---

## 8. 입력 순서와 충돌 방지

현재 `CMainApp::Update()`에는 MapTool의 배치 Update가 없다. 다음 순서로 확장한다.

```text
F1 shortcut edge 처리
ImGui BeginFrame
WantCaptureKeyboard/Mouse -> SetInputBlocked
CGameInstance::Update_Engine
  -> 전 프레임 PickPos 복사
  -> DirectInput 갱신
  -> Level/Object Update
CMapTool::Update
  -> 현재 level/열림/ARMED 검사
  -> ImGui mouse capture 검사
  -> 좌클릭 edge에서 배치
```

규칙:

- ImGui 창 위 클릭은 절대 배치하지 않는다.
- 좌클릭을 누르고 있는 동안 여러 개 생성하지 않고 down edge 한 번만 사용한다.
- F1으로 도구를 닫거나 Esc를 누르면 ARMED를 취소한다.
- 자유 카메라 회전용 마우스 입력과 배치 좌클릭을 분리한다.
- `LEVEL::ASSET_TEST`가 아닐 때 목록은 비활성화하거나 “F2로 AssetTest 진입”을 표시한다.

현재 Engine에는 current level getter가 없으므로 다음 중 한 가지를 구현 때 확정한다.

1. `CLevel_Manager::Get_CurrentLevelID()`를 `CGameInstance`로 노출한다. **권장**
2. AssetTest가 MapTool에 활성 상태를 명시적으로 전달한다.

전역 Prototype 존재 여부로 레벨을 추측하지 않는다.

---

## 9. 피킹 규칙

초기 배치는 두 단계 fallback으로 확정한다.

```text
1순위: CGameInstance::Picking(Target_PickPos)
  -> 이미 보이는 바닥/맵 표면의 실제 월드 위치

2순위: 화면 ray와 Y=0 평면 교차
  -> 아무 모델도 없는 빈 AssetTest에서도 첫 파츠 배치 가능
```

### 9.1 기존 Picking 안전 보정

`CPicking::Picking()`에서 반드시 확인한다.

```text
ScreenToClient 성공
mouse.x >= 0
mouse.y >= 0
mouse.x < viewport width
mouse.y < viewport height
계산한 index < width * height
```

이 검사가 없으면 창 밖 좌표가 unsigned로 변환되어 메모리 범위를 벗어날 수 있다.

### 9.2 Y=0 fallback

Winters의 `TryPickGroundPlane()`과 동일한 원리다.

```text
mouse -> view ray(origin, direction)
t = -origin.y / direction.y
hit = origin + direction * t
hit.y = 0
```

거절 조건:

- 마우스가 viewport 밖
- ray가 평면과 거의 평행
- `t < 0`, 즉 카메라 뒤쪽 교차
- 계산 결과가 finite가 아님

### 9.3 바닥에 묻히지 않게 하기

파츠 pivot이 모델 바닥에 있다는 보장이 없다. Winters처럼 `BottomCenter` 기준을 사용한다.

```text
local AABB min/max
-> bottom center 계산
-> world transform 적용
-> 클릭한 ground anchor와의 차이만큼 position 보정
```

현재 `CModel/CMesh`에는 local bounds 조회 API가 없다. 첫 구현에서 CModel에 읽기 전용 aggregate AABB를 계산·조회하는 최소 API를 추가한다. 이 값은 BottomCenter 보정에만 쓰고, 충돌 시스템까지 함께 만들지 않는다.

---

## 10. 배치 실행 책임

MapTool이 `CGameInstance::Add_GameObject_to_Layer()`를 호출하고 생성된 객체 포인터를 돌려받는다.

```cpp
CGameInstance::Get().Add_GameObject_to_Layer(
    ETOUI(LEVEL::ASSET_TEST),
    TEXT("Prototype_GameObject_MapAsset"),
    ETOUI(LEVEL::ASSET_TEST),
    TEXT("Layer_MapAsset"),
    &desc,
    &placedObject);
```

현재 Add API는 `HRESULT`만 반환하므로 다음 최소 확장을 적용한다.

```cpp
HRESULT Add_GameObject_to_Layer(...,
    void* pArg = nullptr,
    std::shared_ptr<CGameObject>* pOutGameObject = nullptr);

HRESULT Remove_GameObject_from_Layer(
    uint32_t levelIndex,
    const std::wstring& layerTag,
    const std::shared_ptr<CGameObject>& gameObject);
```

동일 확장을 `CGameInstance -> CObject_Manager -> CLayer`에 관통시킨다. MapTool은 `std::shared_ptr<CMapAssetObject>`를 보관하여 Inspector 수정, Delete, 저장, load rollback을 수행한다. 제거는 Engine object update가 끝난 뒤 `CMapTool::Update()`에서 실행하므로 현재 layer 순회 중 erase하지 않는다.

생성 성공 후:

```text
ARMED -> IDLE
상태: Placed <assetId>
```

생성 실패 후:

```text
ARMED 유지
상태: Failed <assetId>: <reason>
```

실패했다고 선택을 지워버리면 사용자가 원인을 고치고 같은 위치에서 재시도하기 어렵다.

### 10.1 배치 레코드

```cpp
struct MAP_PLACED_ENTRY
{
    uint64_t placementId;
    std::string assetId;
    std::shared_ptr<CMapAssetObject> object;
};
```

저장되는 것은 GPU buffer나 Prototype 포인터가 아니다. `placementId + assetId + position + rotationDegrees + scale + visible`만 저장한다. Load는 assetId를 카탈로그에서 찾아 동일한 공용 GameObject Prototype을 다시 Clone한다.

### 10.2 저장 파일 계약

확정 위치:

```text
Client/Bin/DataFiles/Map/BG_RAD_VALTAN_A.mapplacements
```

형식:

```text
LOSTARK_MAP_PLACEMENTS 1 "BG_RAD_VALTAN_A" 2
1 "BG_RAD_VALTAN_FLOOR01_SM" 0 0 0 0 0 0 1 1 1 1
2 "BG_RAD_VALTAN_STATUE01_SM" 5 0 8 0 45 0 1 1 1 1
```

각 행 마지막 값은 visible의 `0/1`이다. Save는 `.tmp`에 전부 기록하고 flush/close 검증 후 `ReplaceFileW` 또는 `MoveFileExW`로 교체한다. Load는 전체 문서를 임시 벡터로 검증·생성한 다음에만 기존 배치를 교체한다. 중간 행 실패 시 새 객체를 제거하고 기존 배치를 유지한다.

---

## 11. 이번 1차 구현 범위와 후속 범위

### 11.1 이번 구현에서 반드시 닫을 범위

1. 현재 작업 트리의 `CLevel_Test2` 이름 분리 빌드 검증
2. 17개 `.wmodel` 생성과 `info` 검증
3. 맵 에셋 카탈로그 17행 작성
4. 카탈로그 로더/검증 구현
5. 정적 바이너리 셰이더 구현
6. `CMapAssetObject` 공용 클래스 구현
7. AssetTest Loader에서 shader 1개, model 17개, map object 1개 Prototype 등록
8. F1 MapTool에 17개 목록/검색/상태 표시
9. 목록 클릭 즉시 ARMED
10. surface PickPos 우선 + Y=0 fallback
11. 월드 좌클릭 한 번에 한 개 Clone
12. 실제 diffuse/optional normal 렌더 확인
13. Hierarchy에서 배치 인스턴스 선택
14. Position/Rotation/Scale/Visible Inspector
15. Delete
16. Save/Load 및 잘못된 파일 rollback

### 11.2 후속 범위

- 선택 outline
- Undo/Redo
- 배치 ghost preview
- grid/angle snap
- 맵 오브젝트 ID picking target

중요: 현재 `Target_PickPos`에는 월드 좌표만 있고 오브젝트 ID는 없다. 따라서 “배치된 물체를 화면에서 클릭해 선택”하려면 `Target_PickID(R32_UINT)` 또는 CPU ray/AABB 선택이 추가로 필요하다. 1차의 “피킹”은 **배치 위치 피킹**이며, 오브젝트 선택 피킹과 다르다.

---

## 12. 예상 수정 파일과 Visual Studio 필터

### 신규

```text
Client/Public/MapAssetCatalog.h
Client/Private/MapAssetCatalog.cpp
Client/Public/MapAssetObject.h
Client/Private/MapAssetObject.cpp
Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl
Client/Bin/DataFiles/Map/BG_RAD_VALTAN_A.mapassets
Client/Bin/DataFiles/Map/BG_RAD_VALTAN_A.mapplacements # 첫 Save 때 생성
```

### 수정

```text
Client/Public/MapTool.h
Client/Private/MapTool.cpp
Client/Private/MainApp.cpp
Client/Private/Loader.cpp
Client/Private/Level_AssetTest.cpp       # 필요 시 에디터 초기 배치/상태 연결만
Engine/Public/Picking.h
Engine/Private/Picking.cpp
Engine/Public/Layer.h
Engine/Private/Layer.cpp
Engine/Public/Object_Manager.h
Engine/Private/Object_Manager.cpp
Engine/Public/Level_Manager.h            # current level getter 선택 시
Engine/Public/GameInstance.h             # current level getter 선택 시
Engine/Private/GameInstance.cpp           # current level getter 선택 시
Engine/Public/Model.h                     # local AABB getter
Engine/Private/Model.cpp                  # binary/static AABB 계산
Client/Default/Client.vcxproj
Client/Default/Client.vcxproj.filters
```

### 선행 검증 대상

```text
Client/Public/Level_Test2.h
Client/Private/Level_Test2.cpp
```

권장 필터:

```text
01.Levels
└─ 00.TestLevel1
   └─ Level_AssetTest

02.GameObjects
└─ 02.World
   └─ Map
      └─ MapAssetObject

03. Tools
└─ 00. Map
   ├─ MapTool
   └─ MapAssetCatalog

97.ShaderFiles
└─ Shader_VtxMeshBinary.hlsl
```

Engine의 `12.BinaryAsset`는 `.wmodel` 해석 책임만 유지한다. MapTool, 카탈로그, 배치 상태를 Engine BinaryAsset 필터에 넣지 않는다.

---

## 13. 구현 순서와 단계별 검증

### P0. 입력과 기준선 정리

- 현재 작업 트리의 `CLevel_Test2` 이름 분리 컴파일·링크 확인
- 17개 `.wmodel` 생성
- `ModelAssetConverter info` 17개 통과
- 카탈로그 경로/태그/scale 검증

통과 조건:

```text
Map assets: 17
WModel ready: 17
Missing model: 0
Duplicate id/tag: 0
```

### P1. 정적 모델 한 개 수동 Prototype smoke

- Floor 하나만 Loader에 임시 등록
- 공용 MapAssetObject 한 개 수동 생성
- diffuse/normal 분기 렌더

통과 조건:

```text
AssetTest에서 Floor 1개가 원점에 정상 출력
셰이더 컴파일 오류 0
normal 없는 재질도 렌더 실패 0
```

이 smoke가 통과한 뒤 17개 반복 등록으로 넓힌다.

### P2. 카탈로그 기반 17 Prototype 등록

- Loader와 MapTool이 같은 카탈로그를 읽음
- 17개 CModel Prototype 등록
- 공용 GameObject Prototype 등록

통과 조건:

```text
AssetTest Loading 완료
registered model prototypes: 17
등록 실패 태그가 로그에 정확히 표시
```

### P3. ImGui 팔레트

- 검색
- 17개 행
- Ready/Missing 상태
- 클릭 즉시 ARMED
- Esc/F1 취소

통과 조건:

```text
F1 -> Catalog Ready 17/17
행 클릭 -> Armed: <id>
ImGui 안 클릭 -> 월드 생성 0개
```

### P4. 위치 피킹과 배치

- Picking bounds guard
- visible surface 우선
- Y=0 fallback
- 좌클릭 edge
- `Layer_MapAsset` Clone

통과 조건:

```text
빈 배경 좌클릭 -> Y=0에 첫 파츠 생성
기존 Floor 위 좌클릭 -> PickPos 높이에 생성
마우스 hold -> 1개만 생성
창 밖 클릭 -> 충돌/생성 없음
```

### P5. 17개 전수 렌더

- 각 행을 한 번씩 배치
- pivot/scale/material 확인
- emissive-only 자산의 diffuse fallback 확인

통과 조건:

```text
17/17 생성 성공
검은색/완전 투명/폭발 scale 자산 목록 0
누락 texture 경로 0
```

### P6. Hierarchy·Inspector·Save/Load

- 생성 객체 포인터 보관
- 선택/Transform/Visible/Delete
- atomic Save
- validation-first Load와 rollback

통과 조건:

```text
3개 배치 -> Save -> 좌표 수정/삭제 -> Load -> 저장 상태 복원
Client 재실행 -> F2 -> Load -> 동일 3개 복원
손상 파일 Load -> 기존 월드 상태 유지 + 오류 표시
```

### P7. 빌드·런타임 회귀

```text
Engine x64 Debug Build
UpdateLib.bat
Client x64 Debug Build
Logo 유지
Space -> 기존 Gameplay 유지
F2 -> Loading -> AssetTest 유지
F1 -> MapTool
발탄 idle 유지
맵 파츠 배치 유지
```

---

## 14. 협업 방식

Git에 올릴 것:

```text
카탈로그 JSON
Client/Engine 코드
HLSL
vcxproj / vcxproj.filters
ModelAssetConverter 코드와 실행 번들
계획/결과 문서
```

공용 Drive/zip으로 공유할 것:

```text
Client/Bin/Resources/LostArk/Map/BG_RAD_VALTAN_A/**
  -> .wmodel
  -> textures/**
  -> 필요 시 원본 보존용 .wmesh/.wmat
```

팀원 추가 절차:

```text
1. FBX를 ModelAssetConverter로 .wmodel 변환
2. Resources/LostArk/Map/<Area>/<Asset>/에 배치
3. info 검증
4. 해당 area catalog에 한 행 추가
5. Loader가 catalog를 읽어 Component Prototype 등록
6. F1 팔레트에서 선택 후 월드 클릭
```

파일 이름이 달라도 내부 `.wmodel` 포맷이 같고 카탈로그 행이 올바르면 같은 `CModel` 파이프라인으로 동작한다.

---

## 15. 최종 완료 기준

이번 기능은 다음 장면이 실제로 재현되면 완료다.

```text
1. 기존 Logo 실행
2. F2
3. 기존 Loading 통과
4. AssetTest에서 발탄 idle 확인
5. F1
6. BG_RAD_VALTAN_A 17개 목록 확인
7. Floor 행 클릭
8. 빈 월드 클릭
9. Floor가 Y=0에 렌더
10. Statue 행 클릭
11. Floor 표면 클릭
12. Statue가 클릭 위치에 렌더
13. Hierarchy 선택 후 Transform 변경
14. Save
15. 배치 삭제/이동 후 Load하여 저장 상태 복원
16. Client 재실행 후 Load하여 동일 배치 복원
17. ImGui 위 클릭, 창 밖 클릭, 마우스 hold에서 오배치 없음
18. Space 경로의 기존 Gameplay 회귀 없음
```

이 완료 뒤에 선택 outline, 화면 오브젝트 ID 피킹, undo/redo, ghost preview를 붙인다.
