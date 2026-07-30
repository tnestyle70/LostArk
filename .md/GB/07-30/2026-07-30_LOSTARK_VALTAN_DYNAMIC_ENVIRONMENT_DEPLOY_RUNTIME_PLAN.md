# 발탄 동적 환경·Deploy 런타임 구현 계획서

- 작성일: 2026-07-30
- 대상: `LV_LUT_HEARTRB_ED`, zone `37051`
- 상태: 구현·자동 검증 완료, 대상 카메라 시각 QA 대기
- 정적 맵 결과: Catalog v3 269개, Placement 13,103개
- 동적 복원 범위: crack emissive, 쇠사슬 D/N/S, CloudPlane, sky mirror, Deploy Prop, `ITR_02326`, spacehole/hugechaosgate

## 1. C1~C8 관점

### C1. Context

정적 PS/SL 복구는 원본 Actor/Component Transform을 보존한다. 쇠사슬 13개, CloudPlane 2개, sky mirror 1개는 이미 이 문서에 존재한다. 파괴 가능한 Prop은 정적 배치가 아니라 `DeployData.loa`, `EFTable_Prop.db`, LookInfo, GameAction/SkillEffect 및 TriggerMapData가 소유한다.

### C2. 착수 시 상태와 해소 결과

- Map Catalog를 v3로 올려 render mode/cull/UV/opacity/emissive/specular/tint를 저장한다.
- crack emissive를 asset별 profile로 이동하고 모든 G-buffer writer가 Target4를 덮는다.
- arena summary를 112개 subset으로 다시 계산하고 전역 169개는 `globalSummary`로 분리한다.
- Trigger 근거명은 구조 참조가 아니라 deploy ID의 raw little-endian byte occurrence로 교정했다.
- `ITR_02326` exact SkeletalMesh/skin은 유지하되 glTF animation clip 0을 bind-pose-only로 노출한다.

### C3. Constraints

- 신규 모델은 `CModel -> CMaterial` 통합 경로를 사용한다.
- Catalog는 생성 정의, Placement/Deploy 문서는 인스턴스와 상태를 소유한다.
- 모든 로드는 `parse -> validate -> stage -> commit`, 실패 시 해당 레이어만 rollback한다.
- exact와 reconstruction을 같은 provenance로 표기하지 않는다.
- 정적 PS/SL placement와 gameplay Deploy placement를 합치지 않는다.

### C4. Contract

Map Catalog v3는 v1/v2를 읽는 하위 호환 reader를 유지하고 각 asset에 다음 render profile을 저장한다.

```text
renderMode       Opaque | Alpha | Sky | Additive
cullMode         Back | Front | None
uvScale          float2
uvSpeed          float2 / second
opacity          0..1
emissiveIntensity >= 0
specularIntensity >= 0
specularPower     >= 1
```

Deploy runtime은 한 `CDeployPropCatalog`가 독립 `.deployassets`와 `.deployplacements`를 함께 parse/validate하고 안정적인 `assetId`, `runtimePlacementId`, 원본 `deployActorId`를 사용한다. 상태는 `INTACT`, `FRACTURED`, `DESPAWNED`를 지원한다. TriggerMap 구조 파싱 전 phase 선택은 명시적인 debug selector이며 원작 자동 타이밍으로 표기하지 않는다.

### C5. Components

1. `CMapAssetCatalog` v3 render profile
2. `CMapAssetObject` opaque/alpha/cull/UV/emissive/specular 적용
3. `CDeployPropCatalog`의 독립 catalog/placement 문서 parser
4. `CDeployPropObject` intact/fractured/skeletal 상태기
5. `CMapTool`의 atomic Deploy layer loader와 상태 selector
6. 환경 데이터 생성기와 exact texture recook
7. `CMapAssetObject` render profile을 이용한 spacehole/hugechaosgate reconstruction

### C6. Correctness

- 모든 G-buffer writer는 `SV_TARGET4=0`을 출력한다.
- normal target alpha는 material specular mask로 사용하고 기존 writer는 1을 기록해 회귀를 막는다.
- arena JSON의 `summary.recordCount == records.length`를 강제하고 전역 통계는 `globalSummary`로 분리한다.
- raw Trigger 증거는 `triggerBinaryOccurrenceCount`로만 표기한다.
- fractured LookInfo 직접 참조와 same-package intact sibling은 provenance를 분리한다.
- CloudPlane만 Alpha/UV panning, crack texture가 있는 재질만 emissive 값을 쓴다.

### C7. Compatibility

- Map Catalog v1/v2는 기본 Opaque, Back cull, UV 1/0, opacity 1, emissive/specular 1로 승격한다.
- 기존 exact 13,091 placement row는 변경하지 않고 overlay 12개만 별도 provenance로 더한다.
- Deploy 문서가 없거나 검증에 실패하면 정적 맵은 그대로 유지되고 Deploy layer만 생성하지 않는다.
- `ITR_02326` animation clip이 없으면 exact skinned bind pose를 사용하고 fabricated animation으로 표기하지 않는다.

### C8. Completion Gates

- G1: Python/HLSL 정적 검증 및 데이터 invariant PASS
- G2: 쇠사슬 D/N/S, CloudPlane, sky mirror WModel texture receipt PASS
- G3: Catalog v3 269개와 Placement 13,103개 parse PASS
- G4: Deploy arena 112개, 모델 85개, skeletal 8개 stage/commit PASS
- G5: intact/fractured/despawned 전환 및 모든 staging 실패 branch rollback 구현·빌드 PASS;
  UI 클릭과 강제 실패 주입 QA는 수동 검증 항목
- G6: Engine/UpdateLib/Client Debug·Release PASS
- G7: AssetTest 시작, F2 진입과 crash-free smoke PASS. 대상 카메라 시각 QA는 별도 판정
- G8: exact/reconstructed/unresolved provenance가 결과 문서와 일치

## 2. 문제 해결 ①~⑤

### ① 비평 P1/P2/P3 수정

G-buffer writer Target4 clear, arena summary 재계산, Trigger 근거명 수정, emissive 전역 상수 제거, intact sibling provenance 분리, overlay 상태/절대 경로/빌드 로그를 수정한다.

### ② 누락 정적 환경 재질

쇠사슬 A/B를 exact diffuse/normal/specular로, CloudPlane을 exact diffuse/normal/opacity와 Alpha UV panning으로, sky mirror를 exact diffuse와 inside-visible cull로 recook한다. 기존 exact Transform은 재사용한다.

### ③ Deploy 생성 경로

추출 JSON에서 생성 가능한 model family를 catalog로, 아레나 records를 placement/state 문서로 변환한다. 모델 없는 record는 gameplay-only로 보존하고 시각 proxy를 만들지 않는다.

### ④ 파괴와 AnimModel

Static family는 intact와 fractured sibling을 별도 prototype으로 등록하고 상태 전환한다. `ITR_02326`은 ANIM `CModel`로 등록하되 clip이 없으면 bind pose로 렌더한다. 추후 exact AnimSet export가 생기면 같은 asset ID의 WModel을 교체한다.

### ⑤ 하늘과 연출

기본 sky mirror는 정적 exact layer다. `par_d_spacehole_03`과 `par_d_hugechaosgate_01`은 extracted texture/parameter 기반 Effect asset으로 분리하고 debug phase selector가 활성화한다. Trigger/Matinee exact timing이 확인되기 전에는 `RECONSTRUCTED_TIMING`으로 기록한다.

## 3. 자료구조·알고리즘 핵심

```cpp
enum class MAP_ASSET_RENDER_MODE { DEFERRED, TRANSLUCENT, BACKGROUND, ADDITIVE };
enum class MAP_ASSET_CULL_MODE { CULL_BACK, CULL_FRONT, TWO_SIDED };

struct MAP_ASSET_RENDER_PROFILE
{
    MAP_ASSET_RENDER_MODE renderMode = MAP_ASSET_RENDER_MODE::DEFERRED;
    MAP_ASSET_CULL_MODE cullMode = MAP_ASSET_CULL_MODE::CULL_BACK;
    float2_t uvScale = float2_t(1.f, 1.f);
    float2_t uvSpeed = float2_t(0.f, 0.f);
    float opacity = 1.f;
    float emissiveIntensity = 1.f;
    float specularIntensity = 1.f;
    float specularPower = 50.f;
    float4_t colorTint = float4_t(1.f, 1.f, 1.f, 1.f);
};

enum class DEPLOY_PROP_STATE { INTACT, FRACTURED, DESPAWNED };
enum class ENVIRONMENT_PHASE { BASELINE, SPACEHOLE, CHAOS_GATE };
```

Deploy 로드는 문서 두 개를 모두 parse/validate하고 필요한 prototype 존재 여부를 확인한 뒤 임시 vector에 객체를 clone한다. 하나라도 실패하면 임시 객체를 제거한다. 전부 성공한 뒤 기존 Deploy layer를 제거하고 staged vector를 commit한다. 상태 변경은 객체를 재생성하지 않고 intact/fractured model visibility 또는 skeletal animation lane을 전환한다. phase proxy 6개는 기본 hidden으로 저장하고 MapTool radio가 visibility만 바꾼다.

## 4. 추가·수정·삭제 파일 목록

### 추가

- `Client/Public/DeployPropCatalog.h`
- `Client/Private/DeployPropCatalog.cpp`
- `Client/Public/DeployPropObject.h`
- `Client/Private/DeployPropObject.cpp`
- `Tools/LevelPlacementExtractor/build_valtan_environment_runtime.py`
- `Tools/LevelPlacementExtractor/build_valtan_phase_layers.py`
- `Tools/LevelPlacementExtractor/build_deployprop_runtime.py`
- `Tools/LevelPlacementExtractor/heartrb_environment_runtime.json`
- `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.deployassets`
- `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.deployplacements`

### 수정

- `Client/Public/MapAssetCatalog.h`
- `Client/Private/MapAssetCatalog.cpp`
- `Client/Public/MapAssetObject.h`
- `Client/Private/MapAssetObject.cpp`
- `Client/Private/MapTool.cpp`
- `Client/Private/Loader.cpp`
- `Client/Public/Level_AssetTest.h`
- `Client/Private/Level_AssetTest.cpp`
- `Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl`
- 모든 NONBLEND G-buffer writer HLSL
- `Client/Bin/ShaderFiles/Shader_Deferred.hlsl`
- `Engine/Bin/ShaderFiles/Shader_Deferred.hlsl`
- `Tools/LevelPlacementExtractor/build_maptool_scene.py`
- `Tools/LevelPlacementExtractor/extract_deploydata_props.py`
- `Tools/LevelPlacementExtractor/heartrb_valtan_core_overlay.json`
- 추출 README/gotchas/결과/공통 파이프라인 문서
- `Client/Default/Client.vcxproj`, `Client/Default/Client.vcxproj.filters`

### 삭제

- 없음

## 5. 파일별 전체 구현 코드

### 신규 C++ 파일의 최종 공개 계약

`Client/Public/DeployPropCatalog.h`의 전체 저장 계약은 다음과 같다.

```cpp
enum class DEPLOY_PROP_MODEL_KIND { STATIC, ANIM };
enum class DEPLOY_PROP_STATE { INTACT, FRACTURED, DESPAWNED };

struct DEPLOY_PROP_ASSET_ENTRY
{
    std::string id, label, evidence;
    DEPLOY_PROP_MODEL_KIND kind = DEPLOY_PROP_MODEL_KIND::STATIC;
    std::filesystem::path intactRelativePath, intactResolvedPath;
    std::wstring intactPrototypeTag;
    std::filesystem::path fracturedRelativePath, fracturedResolvedPath;
    std::wstring fracturedPrototypeTag;
};

struct DEPLOY_PROP_PLACEMENT
{
    uint64_t runtimePlacementId = {};
    uint32_t deployActorId = {}, propDefinitionId = {};
    std::string sourcePlacementId, assetId;
    float3_t position = {};
    float4_t rotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
    f32_t uniformScale = 1.f;
    bool_t destructible = false;
    uint32_t stateOffActionId = {}, triggerBinaryOccurrenceCount = {};
};

class CDeployPropCatalog final
{
public:
    bool_t Load_Default(const std::string& expectedAreaId);
    bool_t Load(const std::filesystem::path& catalogPath,
        const std::filesystem::path& placementPath,
        const std::string& expectedAreaId);
    const DEPLOY_PROP_ASSET_ENTRY* Find(const std::string& assetId) const;
    const std::vector<DEPLOY_PROP_ASSET_ENTRY>& Get_Assets() const;
    const std::vector<DEPLOY_PROP_PLACEMENT>& Get_Placements() const;
    const std::string& Get_AreaId() const;
    const std::string& Get_Status() const;
    bool_t Is_Ready() const;
};
```

`Client/Public/DeployPropObject.h`의 최종 상태 계약은 다음과 같다.

```cpp
class CDeployPropObject final : public CGameObject
{
public:
    struct DEPLOY_PROP_DESC : public CGameObject::GAMEOBJECT_DESC
    {
        DEPLOY_PROP_PLACEMENT placement;
        DEPLOY_PROP_MODEL_KIND modelKind = DEPLOY_PROP_MODEL_KIND::STATIC;
        std::wstring intactPrototypeTag, fracturedPrototypeTag;
    };

    HRESULT Initialize_Prototype() override;
    HRESULT Initialize(void* pArg) override;
    void Update(f32_t fTimeDelta) override;
    void Late_Update(f32_t fTimeDelta) override;
    HRESULT Render() override;
    void Set_State(DEPLOY_PROP_STATE state);
    DEPLOY_PROP_STATE Get_State() const;
    uint64_t Get_RuntimePlacementId() const;
    uint32_t Get_DeployActorId() const;
    bool_t Is_Destructible() const;
    bool_t Is_AnimBindPoseOnly() const;
};
```

두 신규 `.cpp`의 전체 구현은 각각
`C:/Users/user/Desktop/LostArk/Client/Private/DeployPropCatalog.cpp`와
`C:/Users/user/Desktop/LostArk/Client/Private/DeployPropObject.cpp`가 정본이다. parser는
헤더 count와 실제 row 수, ID 중복, area ID, root 밖 경로, quaternion/scale 유효성을
검증한다. Object는 `CModel -> CMaterial` 경로만 사용하고 static은 두 prototype을,
ANIM은 bone matrices를 shader에 전달한다. `DESPAWNED`는 render queue에 들어가지 않는다.

### 수정 함수의 최종 교체 계약

`CMapTool::Load_DeployProps()`는 catalog/placement parse 성공 뒤 85개 clone을 임시
vector에 stage한다. asset 누락, clone 실패, type mismatch 어느 경우든 이미 stage한
객체를 `Layer_DeployProps`에서 전부 제거한다. 85개가 모두 성공한 경우에만 기존 layer를
제거하고 staged vector를 commit한다. `Set_DeployPhase()`는 commit된 객체에만 상태를
전달한다. `Set_EnvironmentPhase()`는 `VALTAN_PHASE_SPACEHOLE`과
`VALTAN_PHASE_CHAOSGATE` placement의 visibility를 바꾼다.

`CMapAssetObject`는 render mode에 따라 NONBLEND/BLEND 우선순위와 pass 0~11을 선택하고,
render profile의 UV/tint/opacity/emissive/specular를 shader에 전달한다. global `0.35`
상수는 없다. `Shader_VtxMeshBinary.hlsl`은 Deferred/Translucent/Background/Additive와
Back/Front/None 조합 12 pass를 제공한다. 다른 모든 G-buffer writer는
`SV_TARGET4=float4(0,0,0,0)`을 출력한다.

### 생성기 전체 입출력 계약

- `build_valtan_environment_runtime.py`: exact chain/cloud/sky source와 texture를 검증하고
  4 WModel pack 및 receipt를 만든다.
- `build_valtan_phase_layers.py`: exact ParticleSystem texture 6개를 plane WModel로 만들고
  `VIDEO_MATCH_RECONSTRUCTION` provenance와 receipt를 쓴다.
- `build_deployprop_runtime.py`: arena JSON 112개에서 시각 85개를 고르고 static family
  8개와 skeletal family 1개를 조리해 `.deployassets/.deployplacements`를 원자 교체한다.
- `build_maptool_scene.py`: exact 260/13,091과 overlay 9/12를 합쳐 v3
  `.mapassets/.mapplacements`를 원자 교체한다.

세 생성기의 전체 최종 코드는 위 파일 목록의 실제 UTF-8 소스이며, receipt hash와
생성 파일 수를 코드 내부 expectation으로 검증한다. 이 계획서의 자료구조 또는 수치와
실제 소스가 불일치하면 G8 실패로 처리한다.

### 프로젝트 XML

```xml
<ClInclude Include="..\Public\DeployPropCatalog.h" />
<ClInclude Include="..\Public\DeployPropObject.h" />
<ClCompile Include="..\Private\DeployPropCatalog.cpp" />
<ClCompile Include="..\Private\DeployPropObject.cpp" />
```

filters는 Catalog를 `03. Tools\00. Map`, Object를
`02.GameObjects\02. World\Map`에만 추가한다.

## 6. 프로젝트 등록과 검증

신규 `.cpp`는 `Client/Default/Client.vcxproj`의 `ClCompile`, 신규 `.h`는 `ClInclude`에 추가하고 실제 물리 폴더와 같은 기존 필터에만 추가한다.

```text
1. Engine x64 Debug Rebuild                         PASS
2. UpdateLib.bat Debug                             PASS
3. Client x64 Debug Rebuild                        PASS
4. Engine x64 Release                              PASS
5. UpdateLib.bat Release                           PASS
6. Client x64 Release                              PASS
7. Python py_compile + data invariant              PASS
8. 변경 HLSL 7개 최종 FXC                         PASS
9. Debug Client Enter -> Logo -> F2 -> Enter       PASS
10. AssetTest title + 4초 생존                     PASS
11. 대상 카메라 시각 QA                            PENDING
```

Debug/Release가 동일 Bin을 공유하므로 최종 산출물은 Debug Rebuild로 되돌렸다. 모든 명령,
로그 절대 경로, 종료 코드는 최종 결과 문서에 기록한다. 시각 QA는 쇠사슬, arena crack,
CloudPlane 흐름, sky 내부, intact/fractured Prop, `ITR_02326` bind pose를 각각 캡처한다.
