# LostArk WSKL/WANM Cooked Animation 상세 구현 계획

> 작성일: 2026-07-29  
> 문서 성격: 구현 전 상세 계획. 이 문서 작성으로 Engine/Client 코드는 수정하지 않는다.  
> 상위 문서: `2026-07-29_LOSTARK_MULTI_FORMAT_BINARY_ASSET_EDITOR_PLAN.md`의 A02/A03 애니메이션 확장  
> 첫 검증 대상: Winters `MN_RPBF_01`, `SK_GSC_BST_00`의 `.wmesh/.wmat/.wskel/.wanim`

## 1. 이번 단계의 목표

현재 `CCookedModel`은 정적 정점/인덱스 버퍼와 재질만 만든다. 이번 단계에서는 다음 연결을 완성한다.

```text
.wmesh  ── 정점, 인덱스, blend index/weight, inverse-bind 행렬 ─┐
.wmat   ── 재질과 텍스처 경로                                  │
.wskel  ── 본 계층, Rest Pose, GlobalInverseRoot, Socket       ├─ MODEL_ASSET_DATA
.wanim  ── 채널, Position/Rotation/Scale 키, Event, skel_hash ┘
                                                                    ↓
                                                             CCookedModel
                                                                    ↓
                                             정적/스키닝 GPU 버퍼 + 애니메이션 재생
```

완료 상태는 다음과 같다.

- 정적 `.wmesh`는 현재처럼 렌더링된다.
- 스키닝 `.wmesh`는 `VTXANIMMESH` GPU 버퍼로 생성된다.
- `.wskel`은 본 계층과 Rest Pose를 제공한다.
- `.wmesh`의 BoneEntry는 각 본의 inverse-bind(offset) 행렬을 제공한다.
- `.wanim`은 Skeleton hash가 맞을 때만 Clip으로 등록된다.
- `CCookedModel`이 애니메이션 목록, 선택, 재생, 정지, 반복, 속도, 현재 시간을 제공한다.
- 매 프레임 계산한 최종 본 행렬을 `g_BoneMatrices`에 바인딩하고 기존 `Shader_VtxAnimMesh.hlsl`로 렌더링한다.
- 기존 수업 FBX/Assimp `CModel` 경로는 변경하지 않는다.

## 2. 먼저 고정할 개념

### 2.1 Reader, Decoder, CookedModel의 책임

```text
CWSkeletonReader
  = wskel 파일 하나를 읽어 MODEL_SKELETON_DATA로 번역

CWAnimationReader
  = wanim 파일 하나를 읽어 MODEL_ANIMATION_DATA로 번역

CWModelDecoder
  = wmesh/wmat/wskel/wanim Reader들을 호출해 한 모델 패키지로 조립

CCookedModel
  = 조립된 공통 데이터를 GPU 자원과 런타임 애니메이션 상태로 변환
```

Registry에 Skeleton/Animation Reader를 각각 등록하지 않는다. Registry는 여전히 모델 패키지 포맷을 고르는 `IModelDecoder`만 관리한다. W 포맷 내부에서 `CWModelDecoder`가 파일별 Reader를 조정한다.

### 2.2 공유 데이터와 인스턴스 상태

한 파일에서 읽은 다음 데이터는 여러 인스턴스가 공유할 수 있는 불변 데이터다.

- GPU Vertex/Index Buffer
- Material/Texture
- Skeleton 정의
- Animation Clip과 Key

다음 데이터는 오브젝트마다 달라야 한다.

- 현재 Clip
- 현재 재생 시간
- Loop/Speed/Playing 상태
- Local/Global/Final Bone Matrix 배열
- 직전 Event cursor

첫 구현에서는 `CBinaryAssetObject`마다 `CCookedModel::Create()`를 호출하므로 `CCookedModel` 안에 불변 데이터와 인스턴스 상태를 함께 둘 수 있다. 단, 멤버를 명확히 분리한다. 이후 Model Cache를 연결할 때 불변 부분만 `COOKED_MODEL_RESOURCE`로 이동하고 애니메이터 상태는 인스턴스에 남긴다.

애니메이션 재생 시간과 최종 본 행렬을 공유 캐시에 넣는 것은 금지한다. 그렇게 하면 같은 발탄 두 개가 항상 같은 애니메이션 시간으로 움직인다.

## 3. 확인된 현재 코드 차이

### 3.1 현재 LostArk BinaryAsset의 제한

- `MODEL_MESH_DATA`는 `vector<VTXMESH>`만 보유한다.
- `CWModelDecoder`는 skinned vertex의 blend index/weight를 버리고 bind-pose `VTXMESH`로 바꾼다.
- `.wmesh` BoneEntry 128바이트 블록을 읽지 않고 `Skip()`한다.
- `MODEL_ASSET_LOAD_DESC`에는 skeleton/animation 경로가 없다.
- `MODEL_ASSET_DATA`에는 skeleton/animation 저장 공간이 없다.
- `CCookedModel::RUNTIME_MESH`의 stride는 항상 `sizeof(VTXMESH)`다.
- `CBinaryAssetObject`는 항상 `Prototype_Component_Shader_VtxMesh`를 사용한다.
- `CBinaryAssetObject::Late_Update()`는 모델 애니메이션을 갱신하지 않는다.

### 3.2 W 포맷과 LostArk Vertex Layout의 차이

Winters skinned vertex는 76바이트다.

```text
Position 12 + Normal 12 + UV 8 + Tangent 12
+ BlendIndices 16 + BlendWeights 16 = 76
```

LostArk `VTXANIMMESH`는 88바이트다.

```text
Position 12 + Normal 12 + Tangent 12 + Binormal 12 + UV 8
+ BlendIndices 16 + BlendWeights 16 = 88
```

W v1에는 Binormal과 Tangent handedness가 없다. 첫 구현은 다음으로 고정한다.

- Position/Normal/UV/Tangent/BlendIndices/BlendWeights를 W vertex에서 복사한다.
- Binormal은 `normalize(cross(Normal, Tangent))`로 만든다.
- 현재 애니메이션 셰이더는 Tangent/Binormal을 실제 조명 계산에 사용하지 않으므로 첫 diffuse smoke에는 영향이 없다.
- 향후 normal-map 정확도가 필요하면 WMesh v2에 tangent sign을 추가하거나 76바이트 전용 input layout/shader를 만든다.

### 3.3 Bone 수 제한

- W 포맷 최대: 1024
- 현재 LostArk `g_BoneMatrices`: 512
- 첫 Runtime 최대: `MAX_COOKED_BONES = 512`

512개를 넘는 에셋을 조용히 잘라내지 않는다. Decoder validation에서 명확히 실패시키고 보고서에 실제 bone count를 기록한다. Bone palette/texture 방식은 별도 확장으로 남긴다.

## 4. 공통 CPU 데이터 계약 확장

대상: `Engine/Public/BinaryAsset/ModelAssetData.h`

정확한 이름은 구현 직전 현재 스타일에 맞춰 확정하되, 정보 계약은 아래를 만족해야 한다.

```cpp
enum class MODEL_VERTEX_KIND
{
    STATIC,
    SKINNED,
};

struct MODEL_SKINNED_VERTEX_DATA
{
    float3_t position;
    float3_t normal;
    float3_t tangent;
    float3_t binormal;
    float2_t texcoord;
    uint4_t blendIndices;
    float4_t blendWeights;
};

struct MODEL_MESH_DATA
{
    string name;
    uint32_t materialIndex;
    MODEL_VERTEX_KIND vertexKind;
    vector<VTXMESH> staticVertices;
    vector<MODEL_SKINNED_VERTEX_DATA> skinnedVertices;
    vector<uint32_t> indices;
};
```

정적/스키닝 vector 중 `vertexKind`에 해당하는 하나만 채운다. `void*`나 raw byte vector로 공통 구조를 만들지 않는다.

Skeleton 데이터:

```cpp
struct MODEL_BONE_DATA
{
    uint64_t nameHash;
    string name;
    int32_t parentIndex;
    float4x4_t restLocal;
    float4x4_t inverseBind;
};

struct MODEL_SOCKET_DATA
{
    uint64_t nameHash;
    string name;
    int32_t parentBoneIndex;
    float4x4_t localOffset;
};

struct MODEL_SKELETON_DATA
{
    uint64_t skeletonHash;
    float4x4_t globalInverseRoot;
    vector<MODEL_BONE_DATA> bones;
    vector<MODEL_SOCKET_DATA> sockets;
};
```

Animation 데이터:

```cpp
struct MODEL_VECTOR_KEY_DATA
{
    float timeTicks;
    float3_t value;
};

struct MODEL_QUAT_KEY_DATA
{
    float timeTicks;
    float4_t value;
};

struct MODEL_ANIMATION_CHANNEL_DATA
{
    uint64_t boneNameHash;
    int32_t resolvedBoneIndex;
    vector<MODEL_VECTOR_KEY_DATA> positionKeys;
    vector<MODEL_QUAT_KEY_DATA> rotationKeys;
    vector<MODEL_VECTOR_KEY_DATA> scaleKeys;
};

struct MODEL_ANIMATION_EVENT_DATA
{
    float timeTicks;
    uint16_t type;
    uint32_t skillId;
    uint32_t paramU32;
    float paramF32;
    uint64_t stringHash;
};

struct MODEL_ANIMATION_DATA
{
    string name;
    uint64_t skeletonHash;
    float durationTicks;
    float ticksPerSecond;
    bool_t defaultLoop;
    vector<MODEL_ANIMATION_CHANNEL_DATA> channels;
    vector<MODEL_ANIMATION_EVENT_DATA> events;
};
```

최종 모델 데이터와 요청:

```cpp
struct MODEL_ASSET_DATA
{
    vector<MODEL_MESH_DATA> meshes;
    vector<MODEL_MATERIAL_DATA> materials;
    optional<MODEL_SKELETON_DATA> skeleton;
    vector<MODEL_ANIMATION_DATA> animations;
};

struct MODEL_ASSET_LOAD_DESC
{
    filesystem::path meshPath;
    filesystem::path materialPath;
    filesystem::path skeletonPath;
    vector<filesystem::path> animationPaths;
    filesystem::path fallbackDiffusePath;
    string defaultAnimationName;
};
```

프로젝트 표준이 `std::optional`을 허용하지 않으면 `bool_t hasSkeleton + MODEL_SKELETON_DATA`로 대체한다. 정보 의미는 바꾸지 않는다.

## 5. CWSkeletonReader 구현 계획

### 5.1 추가 파일

```text
Engine/Private/BinaryAsset/Winters/WFormatTypes.h
Engine/Private/BinaryAsset/Winters/WSkeletonReader.h
Engine/Private/BinaryAsset/Winters/WSkeletonReader.cpp
```

Reader는 packed W 구조를 외부 public API로 노출하지 않고 `MODEL_SKELETON_DATA`만 반환한다.

예정 인터페이스:

```cpp
class CWSkeletonReader final
{
public:
    bool_t Read(
        const filesystem::path& skeletonPath,
        MODEL_SKELETON_DATA& outSkeleton,
        MODEL_DECODE_REPORT& outReport) const;
};
```

### 5.2 WSKL 바이트 순서

```text
WINTERS_FILE_HEADER       16 bytes : WINT, version, flags, contentSize
SKEL_META_HEADER          32 bytes : WSKL, boneCount, socketCount
BONE_NODE                 256 bytes × boneCount
GLOBAL_ROOT_MATRIX        128 bytes
SOCKET_ENTRY              128 bytes × socketCount
```

`BONE_NODE`는 다음 정보를 준다.

- `name_hash`
- `name[64]`
- `parent_index`
- `rest_transform[16]`
- child metadata

주의: inverse-bind 행렬은 `.wskel`에 없다. 같은 인덱스의 `.wmesh` BoneEntry에서 가져와 최종 `MODEL_BONE_DATA.inverseBind`에 결합한다.

### 5.3 WSKL 검증 순서

1. 파일 open과 최소 크기를 검사한다.
2. 외부 magic이 `WINT`인지 확인한다.
3. major version이 1이고 flags가 0인지 확인한다.
4. content size가 파일 범위 안인지 overflow-safe 계산으로 검사한다.
5. payload magic이 `WSKL`인지 확인한다.
6. bone count가 `1..512`인지 확인한다.
7. socket count가 `0..256`인지 확인한다.
8. `32 + boneCount*256 + 128 + socketCount*128`이 payload 안에 완전히 들어가는지 확인한다.
9. bone name이 NUL 범위 안에서 읽히며 비어 있지 않은지 확인한다.
10. parent는 `-1` 또는 `0 <= parent < currentBoneIndex`인지 확인한다.
11. 최소 한 개의 root가 있는지 확인한다. v1 canonical fixture는 root 하나를 기대하며 여러 root는 report warning/정책으로 확정한다.
12. Rest/Root/Socket matrix의 모든 원소가 finite인지 확인한다.
13. socket parent index가 bone 범위인지 확인한다.
14. 동일한 bone hash와 socket hash가 중복되지 않는지 확인한다.
15. Skeleton hash를 Winters와 같은 FNV 순서로 계산한다.

Skeleton hash 계산:

```text
hash = FNV offset basis
for bone in index order:
    hash ^= bone.name_hash
    hash *= FNV prime
```

이 hash가 이후 `.wanim` trailer 검증 기준이 된다.

## 6. CWAnimationReader 구현 계획

### 6.1 추가 파일

```text
Engine/Private/BinaryAsset/Winters/WAnimationReader.h
Engine/Private/BinaryAsset/Winters/WAnimationReader.cpp
```

예정 인터페이스:

```cpp
class CWAnimationReader final
{
public:
    bool_t Read(
        const filesystem::path& animationPath,
        const MODEL_SKELETON_DATA& skeleton,
        MODEL_ANIMATION_DATA& outAnimation,
        MODEL_DECODE_REPORT& outReport) const;
};
```

### 6.2 WANM 바이트 순서

```text
WINTERS_FILE_HEADER       16 bytes : WINT
ANIM_META_HEADER          32 bytes : WANM
ANIM_CHANNEL              40 bytes × channelCount
KEY BLOCK                 variable size
ANIM_EVENT                32 bytes × eventCount
WANIM_TRAILER              8 bytes : skeletonHash
```

각 Channel은 bone name hash와 Position/Rotation/Scale 키의 count/offset을 갖는다. Offset은 Key Block 시작 기준이다.

### 6.3 WANM 검증 순서

1. 외부 `WINT`, major 1, flags 0, content size를 검사한다.
2. payload `WANM` magic을 검사한다.
3. channel count `<= 1024`, total key count `<= 1,000,000`을 검사한다.
4. `durationTicks`가 finite이고 0보다 큰지 검사한다.
5. `ticksPerSecond`가 finite이고 0보다 큰지 검사한다.
6. channel table, event block, trailer 크기를 overflow-safe로 계산한다.
7. 각 Position/Rotation/Scale offset+count 범위가 Key Block 내부인지 검사한다.
8. 모든 key time/value가 finite인지 검사한다.
9. 키 시간이 오름차순인지 검사한다. 뒤섞인 키를 runtime에서 자동 정렬하지 않고 invalid asset으로 보고한다.
10. Rotation quaternion 길이가 0에 가깝지 않은지 검사하고 정상 범위는 normalize한다.
11. 채널 bone hash를 Skeleton hash map에서 찾는다.
12. 파일의 `bone_index_cached`는 신뢰하지 않는다. 범위와 hash가 일치할 때만 참고하고 최종 index는 Skeleton 기준으로 확정한다.
13. 해석하지 못한 channel을 조용히 버리지 않는다. strict cooked load에서는 실패한다.
14. trailer `skel_hash`가 현재 Skeleton hash와 같은지 검사한다.
15. Event time이 `0..durationTicks` 범위인지 검사한다.
16. 실제 channel key count 합이 header `total_key_count`와 맞는지 검사한다.

Animation 이름은 파일 stem을 사용한다. 경로가 달라도 같은 모델 안에서 clip 이름이 중복되면 실패시키거나 catalog의 stable clip ID를 쓰도록 후속 schema에서 확정한다.

### 6.4 Event 범위

이번 단계는 Event 데이터를 잃지 않고 저장하는 데까지 구현한다.

- `HitStart`, `HitEnd`, `Footstep`, `SFX`, `VfxSpawn` 등의 숫자와 parameter를 보존한다.
- 첫 렌더 smoke에서는 Client gameplay callback을 호출하지 않는다.
- 추후 `CCookedModel::Collect_PassedEvents(previousTick, currentTick)` 같은 읽기 API로 노출한다.
- Loop 경계를 넘을 때 event가 두 번 또는 누락되지 않는 테스트를 먼저 만든 뒤 실제 SFX/Hit 로직에 연결한다.

## 7. CWModelDecoder 조립 계획

현재 한 파일에 섞인 mesh/material parsing도 Reader 단위로 나누는 것을 목표로 한다.

```text
CWModelDecoder::Decode
  1. CWMeshReader.Read
  2. CWMaterialReader.Read
  3. static mesh면 종료
  4. skinned mesh면 CWSkeletonReader.Read 필수
  5. WMesh bone table과 WSkeleton bone table 결합/검증
  6. desc.animationPaths 순서대로 CWAnimationReader.Read
  7. MODEL_ASSET_DATA 전체 invariant 검증
```

### 7.1 WMesh/WSkeleton 결합 규칙

- bone count가 정확히 같아야 한다.
- 같은 index의 `name_hash`가 같아야 한다.
- `.wmesh` BoneEntry의 `offset_matrix`를 inverse-bind로 쓴다.
- `.wskel` BoneNode의 `rest_transform`과 `parent_index`를 계층/Rest Pose로 쓴다.
- `.wskel` GlobalRootMatrix를 최종 skin matrix 계산에 쓴다.
- 모든 vertex blend index가 skeleton bone count보다 작아야 한다.
- weight는 finite/음수 금지이며 합이 허용 오차 안에서 1이 아니면 normalize한다.
- 모든 weight가 0이면 bone 0 weight 1 fallback은 producer와 동일하게 허용하되 report count를 남긴다.

### 7.2 Missing dependency 정책

| 상황 | 결과 |
|---|---|
| static `.wmesh`, skeleton 없음 | 정상 |
| skinned `.wmesh`, skeleton 없음 | 실패 |
| skinned `.wmesh`, animation 목록 없음 | Skeleton/Bind Pose 모델로 정상 |
| catalog에 선언한 `.wanim` 누락 | 실패 |
| `.wanim` skeleton hash 불일치 | 실패 |
| animation 없는 모델에서 Play 요청 | false 반환, crash 금지 |

엔진이 `anims/` 폴더를 임의로 순회하지 않는다. `AssetCatalog`가 명시한 `animationPaths`만 읽는다. Winters의 폴더 scan 방식은 catalog를 생성하는 오프라인 importer에서만 사용할 수 있다.

## 8. CCookedModel Runtime 확장 계획

### 8.1 Runtime mesh

`RUNTIME_MESH`에 다음을 추가한다.

```cpp
MODEL_VERTEX_KIND vertexKind;
uint32_t vertexStride;
```

- static: `VTXMESH`, static shader
- skinned: `VTXANIMMESH`, animation shader
- 한 모델 안에서 static/skinned submesh 혼합은 v1에서 금지하고 decoder에서 실패시킨다.

### 8.2 Runtime Skeleton/Clip/Animator

`CCookedModel` 내부 책임을 아래처럼 구분한다.

```text
불변
  m_Skeleton
  m_AnimationClips
  m_Meshes
  m_Materials

인스턴스 상태
  m_CurrentAnimationIndex
  m_CurrentTimeTicks
  m_PlaySpeed
  m_IsPlaying
  m_IsLoop
  m_LocalBoneMatrices
  m_GlobalBoneMatrices
  m_FinalBoneMatrices
  m_PreviousEventTimeTicks
```

필요 public API:

```cpp
bool_t Is_Skinned() const;
bool_t Has_Skeleton() const;
bool_t Has_Animations() const;
uint32_t Get_NumAnimations() const;
const string& Get_AnimationName(uint32_t index) const;

bool_t Play_Animation(uint32_t index, bool_t loop);
bool_t Play_Animation(const string& name, bool_t loop);
void Stop_Animation();
void Set_AnimationSpeed(f32_t speed);
void Update_Animation(f32_t deltaTime);

HRESULT Bind_BoneMatrices(
    shared_ptr<CShader> shader,
    const char_t* constantName) const;
```

### 8.3 매 프레임 계산

```text
delta seconds
  ↓ × ticksPerSecond × playSpeed
current time ticks
  ↓ loop/clamp
각 Animation Channel 보간
  Position = Lerp
  Rotation = Quaternion Slerp
  Scale    = Lerp
  ↓
채널이 없는 본은 Rest Local 사용
  ↓
Global[i] = Local[i] * Global[parent]
  ↓
Final[i] = InverseBind[i] * Global[i] * GlobalInverseRoot
  ↓
g_BoneMatrices[i]
```

행렬 곱 순서는 W 데이터를 실제 사용 중인 Winters의 row-vector 규칙을 기준으로 고정한다. 구현 시 bind pose에서 `Final`이 identity에 가까운지 fixture로 검사해 transpose/곱 순서 오류를 잡는다.

### 8.4 애니메이션 없는 안전 경로

- `Update_Animation()`은 clip이 없으면 즉시 반환한다.
- `Play_Animation()`은 잘못된 index/name이면 false를 반환하고 기존 상태를 손상시키지 않는다.
- skeleton만 있고 clip이 없으면 Rest Pose final matrix를 한 번 계산한다.
- static model에서 `Bind_BoneMatrices()`를 호출하면 `S_FALSE` 또는 명시 실패를 반환하도록 호출 계약을 확정한다.

## 9. Client 연결 계획

대상:

```text
Client/Public/BinaryAssetObject.h
Client/Private/BinaryAssetObject.cpp
Client/Private/Loader.cpp
Client/Private/Level_AssetTest.cpp
```

변경 순서:

1. Asset Test level에 static shader와 animation shader prototype이 모두 등록돼 있는지 확인한다.
2. `CBinaryAssetObject::Ready_Components()`는 load descriptor를 먼저 decode/create한다.
3. `m_pModel->Is_Skinned()` 결과에 따라 `Shader_VtxMesh` 또는 `Shader_VtxAnimMesh`를 선택한다.
4. `CBinaryAssetObject::Update()` 또는 현재 엔진의 실제 update hook에서 `m_pModel->Update_Animation(deltaTime)`을 호출한다.
5. Render에서 skinned model일 때 `g_BoneMatrices`를 bind한다.
6. 첫 animation이 있으면 descriptor의 `defaultAnimationName`을 재생하고, 없으면 첫 clip을 fallback으로 선택한다.
7. ImGui 패널에서는 clip 목록/Play/Pause/Loop/Speed만 연결한다. Map picking/input 파일은 이 단계에서 수정하지 않는다.

애니메이션 셰이더의 현재 `vWorldPos`는 skinning 전 position을 사용한다. 첫 diffuse 렌더에는 치명적이지 않지만 이후 라이팅/피킹 정확도를 위해 skinning된 `vPosition`을 사용하도록 별도 검토한다.

## 10. AssetCatalog 확장 계약

권장 예시:

```json
{
  "id": "MN_RPBF_01",
  "mesh": "Character/MN_RPBF_01/MN_RPBF_01.wmesh",
  "material": "Character/MN_RPBF_01/MN_RPBF_01.wmat",
  "skeleton": "Character/MN_RPBF_01/MN_RPBF_01.wskel",
  "animations": [
    "Character/MN_RPBF_01/anims/idle_normal_1.wanim",
    "Character/MN_RPBF_01/anims/run_battle_1.wanim"
  ],
  "defaultAnimation": "idle_normal_1"
}
```

규칙:

- static asset은 skeleton/animations 필드를 생략할 수 있다.
- skinned asset은 skeleton이 필수다.
- animation 배열 순서는 catalog에 고정하며 directory iteration 순서에 의존하지 않는다.
- 모든 경로는 pack root 상대 경로다.
- decoder가 같은 stem을 추측하는 것은 migration convenience로만 두고 정식 catalog에서는 명시 경로를 사용한다.

## 11. `12.BinaryAsset` Visual Studio 필터 계획

```text
12.BinaryAsset
├─ 00.Core
│  ├─ BinaryReader
│  ├─ BinaryAssetLimits
│  └─ BinaryAssetValidation
│
├─ 01.Data
│  └─ ModelAssetData
│
├─ 02.Registry
│  ├─ ModelDecoder
│  └─ ModelDecoderRegistry
│
├─ 10.Formats
│  └─ Winters
│     ├─ WFormatTypes
│     ├─ WMeshReader
│     ├─ WMaterialReader
│     ├─ WSkeletonReader
│     ├─ WAnimationReader
│     └─ WModelDecoder
│
└─ 20.Runtime
   ├─ CookedModel
   └─ CookedAnimator     # 분리 시에만 추가
```

필터는 사람 이름이나 확장자만으로 나누지 않는다. `Formats/Winters`는 W 바이트 계약을 아는 코드, `Data`는 포맷 독립 공통 데이터, `Runtime`은 DirectX/재생 상태를 담당한다.

## 12. 구현 Slice와 Commit 경계

### S1 — 공통 데이터와 WMesh 보존

- `ModelAssetData` 확장
- skinned vertex 변환 추가
- WMesh BoneEntry 실제 읽기
- 기존 static asset regression

완료 조건: static 모델은 기존과 동일하게 나오고 skinned 모델의 vertex/bone 정보가 CPU report에 보존된다.

### S2 — CWSkeletonReader

- WSKL parsing/validation
- mesh/skeleton bone table match
- skeleton hash/Rest Pose/socket 저장

완료 조건: 두 character fixture의 bone count/hash/parent tree가 Winters 결과와 같다.

### S3 — CWAnimationReader

- WANM parsing/validation
- channel hash resolve
- key/event/trailer 저장

완료 조건: 28개 clip의 이름, duration, TPS, channel/key/event 수가 reference와 같다. 다른 skeleton의 clip을 넣으면 실패한다.

### S4 — CCookedModel Animator

- runtime skeleton/clip/state
- interpolation/hierarchy/final matrices
- static/skinned GPU vertex buffer 분기
- bone matrix binding

완료 조건: Rest Pose와 첫 idle clip의 시간 진행이 CPU test에서 통과한다.

### S5 — AssetTest 렌더 연결

- shader 선택
- per-frame update
- default clip 재생
- 최소 ImGui clip control

완료 조건: F2 AssetTest에서 `MN_RPBF_01` 또는 `SK_GSC_BST_00`이 bind pose가 아니라 실제 clip으로 움직인다.

### S6 — 회귀/협업 마감

- project/filter wiring
- Debug/Release 빌드
- static map asset regression
- missing/mismatch diagnostics
- catalog/pack validation

완료 조건: 다른 팀원이 동일 pack을 pull/sync한 뒤 Assimp 없이 같은 animation smoke를 재현한다.

## 13. 자동 검증 목록

### Skeleton Reader

- valid WSKL
- bad WINT/WSKL magic
- unsupported major/flags
- truncated bone/global/socket block
- bone count 0/513 이상
- invalid parent, cycle 가능 순서, missing root
- invalid socket parent
- NaN/Inf matrix
- duplicate bone hash

### Animation Reader

- valid WANM
- bad WINT/WANM magic
- zero/NaN duration or TPS
- channel/key/event count overflow
- key offset out of block
- unsorted/NaN key time
- zero quaternion
- unresolved bone hash
- mismatched skeleton trailer hash
- event outside duration

### Runtime

- no animation guard
- rest pose final matrix
- key 시작/중간/끝 보간
- loop 경계
- non-loop 종료
- speed 0/양수/음수 정책
- 두 인스턴스가 서로 다른 current time을 유지
- 512 bone shader limit
- static asset 결과 불변

## 14. 수동 시각 검증 순서

1. 기존 static map mesh를 F2 AssetTest에서 렌더한다.
2. `SK_GSC_BST_00`을 Rest Pose로 한 프레임 렌더한다.
3. 첫 clip을 loop 재생한다.
4. 0.25×, 1×, 2× 속도를 확인한다.
5. Pause 후 time slider를 움직여 pose가 고정되는지 확인한다.
6. 동일 모델 두 개를 생성해 서로 다른 시점으로 재생한다.
7. 잘못된 `.wanim`을 연결해 `SkeletonHashMismatch`가 표시되는지 확인한다.
8. Client 실행 로그에 decoder, mesh/material/bone/clip/key 수를 출력한다.

## 15. 명시적 비범위

이번 단계에서 하지 않는다.

- FBX에서 W 파일을 만드는 Converter 수정
- 애니메이션 blending/state machine/root motion
- IK, ragdoll
- gameplay hit/SFX/VFX event dispatch
- GPU texture bone palette 또는 512개 초과 bone 지원
- normal map tangent handedness 포맷 변경
- 기존 수업 FBX `CModel` 제거
- MapTool picking/input 구조 수정
- 발탄 AI/전투 로직

## 16. 완료 정의

아래가 모두 만족되면 WSKL/WANM Cooked Animation 1차 완료다.

- `CWModelDecoder`가 WMesh/WMaterial/WSkeleton/WAnimation Reader를 조정한다.
- 공통 `MODEL_ASSET_DATA`가 static/skinned mesh, skeleton, sockets, animation clips/events를 손실 없이 담는다.
- skinned vertex의 index/weight가 GPU까지 유지된다.
- WMesh/WSkeleton bone table과 WANM skeleton hash 불일치가 명확히 실패한다.
- `CCookedModel`이 clip을 보유하고 인스턴스별 재생 상태를 갱신한다.
- 최종 bone matrix가 `Shader_VtxAnimMesh.hlsl`에 바인딩된다.
- AssetTest에서 실제 animation이 재생된다.
- 기존 Logo/Loading/GamePlay 수업 흐름과 static cooked asset 렌더가 회귀하지 않는다.
- 다른 팀원은 같은 W 포맷 pack을 받으면 별도 Decoder나 Assimp 설치 없이 같은 결과를 재현한다.

