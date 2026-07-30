# LostArk 발탄 프로파일러·정적 맵 렌더 최적화·NavGrid/A* 구현 계획서

- 작성일: 2026-07-30
- 대상 저장소: `C:\Users\user\Desktop\LostArk`
- 대상 레벨: `LEVEL::ASSET_TEST` / `LV_LUT_HEARTRB_ED`
- 계획 상태: `feature/valtan-navgrid-runtime`에서 발탄 직접 피킹·A* 런타임 검증 진행 중
- 구현 기준 브랜치: `feature/valtan-navgrid-runtime` (`origin/main`의 `e959ced`에서 분기)
- 개인 계획서 규칙: `.md/계획서작성규칙.local.md`
- 참고 구현:
  - `C:\Users\user\Desktop\Winters`
  - `C:\Users\user\Desktop\SR_MinecraftDungeons`
  - `C:\Users\user\Desktop\LOL`

> **2026-07-31 실행 범위 고정:** 이번 브랜치는 기존 NavGrid/A* 정본을 실제
> `LEVEL::ASSET_TEST`에 연결해 발탄을 좌클릭 목표까지 이동시키고, Navigation profiler
> counter와 JSON 캡처를 추가한 뒤 Engine/UpdateLib/Client Debug·Release를 모두 검증한다.
> `TextureResourceCache` 구현 판단과 LanceMaster 적용은 다음 작업으로 미룬다. 인스턴싱
> 전후 JSON의 육안 비교는 사용자가 실행 검증 중 수행한다. 런타임 검증 전에는 커밋하지 않는다.

### 2026-07-31 구현 순서와 호출 계약

1. `CLoader::Ready_For_Level_AssetTest()`가 `ValtanArena.navgrid`를 읽는
   `Prototype_Component_Navigation_ValtanArena`를 등록한다.
2. `CLevel_AssetTest::Ready_Valtan()`이 위 prototype tag와 정확한 발탄 아레나 시작 좌표를
   `CValtan::VALTAN_DESC`로 전달하고 생성된 `CValtan`을 보관한다.
3. `CLevel_AssetTest::Update_ClickMove()`가 ImGui가 마우스를 점유하지 않은 좌클릭 상승
   에지에서 G-buffer picking world position을 읽고 `CValtan::Request_Move()`를 호출한다.
4. `CValtan::Request_Move()`는 현재 위치와 목표 위치를 `CNavPathFollower`에 전달한다.
5. `CNavPathFollower::Request_Path()`는 `CNavigation::Find_Path()`를 호출하고 성공한 경로만
   기존 waypoint vector와 교체한다. 동시에 query 수·확장 node·소요 microsecond·path cell을
   profiler counter에 누적한다.
6. `CValtan::Update()`는 waypoint를 따라 transform을 이동하고 IDLE/CHASE 애니메이션 상태를
   바꾼다. 목표 Transform이 없는 직접 클릭 모드에서도 기존 경로를 취소하지 않는다.
7. `CProfilerCaptureIO::Save_Json()`은 `CProfiler::Snapshot()` 복사본만 받아 임시 파일에 쓴 뒤
   rename하여 `Client/Bin/ProfilerCaptures`에 완성된 JSON만 남긴다.
8. 프레임 종료는 `Client.cpp`의 `Client.Render` scope가 닫힌 뒤 한 번만 수행한다.

### 2026-07-31 검증 게이트

- `ValtanArena.navgrid`: 62 × 63, cell size 0.5, walkable 2,843, 시작 cell walkable 확인
- Engine x64 Debug/Release → UpdateLib Debug/Release → Client x64 Debug/Release PASS
- Debug `Client.exe` 실행 → F2 → 발탄 아레나 카메라 → 좌클릭 목표 → A* 이동
- F4 → Navigation counter 변화 → `Save profiler JSON` 파일 생성
- 사용자 육안 검증 후에만 기능별 변경을 stage/commit

> **2026-07-31 NavGrid 단순화 결정:** 기존 C4-4와 5-3에 적힌 SHA-256, magic/version,
> `#pragma pack`, bitset, min/max height, agent/slope/step 저장 계약은 첫 구현에서 사용하지
> 않는다. 현재 정본은 `Engine/Public/NavGrid.h`와 `Engine/Private/NavGrid.cpp`다. NavGrid는
> `Width/Height/CellSize/OriginX/OriginZ`와 cell별 `Walkable/Height`만 소유한다. agent radius와
> slope는 baker, max step은 A*가 소유한다. 이 최소 NavGrid의 실제 binary load 검증 후에만
> 파일 version과 압축을 별도 근거로 추가한다.

---

## 1. C1~C8 관점

### C1. Context — 목표와 범위

이번 작업은 발탄 맵의 정적 렌더 비용과 이동 경로 탐색을 같은 측정 체계에서 닫는 작업이다. 먼저 CPU/GPU/DrawCall 병목을 재현 가능한 JSON으로 남기는 프로파일러를 만들고, 그 수치를 기준으로 정적 맵을 하드웨어 인스턴싱·가시성 컬링·텍스처 리소스 공유 경로로 전환한다. 이후 발탄 아레나 바닥의 실제 삼각형으로 NavGrid를 베이크하고 A* 경로 탐색을 연결한다.

이번 계획의 포함 범위는 다음과 같다.

1. Engine 범용 CPU/GPU/파이프라인 통계 프로파일러
2. Client F4 프로파일러 패널과 JSON 캡처
3. `CMapAssetObject` 13,097개 생성 경로를 정적 인스턴스 배치 경로로 전환
4. 프러스텀 컬링 후 보이는 인스턴스만 동적 인스턴스 버퍼에 업로드
5. 동일 파일 및 동일 내용 텍스처의 SRV 공유 캐시
6. WMaterial 의미 기반 DDS 변환 감사 도구와 영수증
7. Floor01/A/B 실제 메시 기반 발탄 아레나 NavGrid 베이크
8. `CNavigation` 범용 façade 아래의 NavGrid/A* 경로
9. AssetTest 내 내비게이션 셀·경로 디버그 표시
10. Debug/Release 빌드와 동일 카메라의 전후 프로파일 비교

이번 계획에서 제외하는 항목은 다음과 같다.

- Trigger/피격/난이도/페이즈가 결정하는 파괴·복원 전이 정책
- `ITR_02326` AnimModel 런타임
- 쇠사슬·CloudPlane·sky·spacehole·hugechaosgate 원본 추출·재질 조리
- 파괴물 상태에 따른 동적 NavGrid obstacle 반영
- 발탄 외 지역의 NavGrid 베이크
- 메시를 하나의 영구 거대 VB/IB로 오프라인 병합하는 방식

제외 항목 중 앞의 세 항목은 별도 동적 환경 계획의 소유 범위다. 이 계획의
`CDeployPropRuntime`은 그 세션이 만든 `.deployassets`/`.deployplacements`와
`CDeployPropObject`를 Release 공통 장면 owner에 연결하고 전달받은 상태를 placement별로
적용하는 소비 계층일 뿐, Trigger 전이 정책이나 원본 자산을 다시 구현하지 않는다.

### C2. Current State — 현재 코드와 데이터 실측

#### C2-1. 단축키와 실행 루프

- `CMainApp::UpdateDebugToolShortcut()`의 F1은 MapTool 표시를 토글한다.
- `CLevel_Logo::Update()`의 F2는 `LEVEL::ASSET_TEST` 진입에 사용 중이다.
- 메인 루프는 `Client/Default/Client.cpp`에서 `Update -> Render` 순서로 진행된다.
- F2는 `LEVEL::ASSET_TEST` 진입 전용으로 보존한다.
- F4는 모든 실행 레벨에서 프로파일러 overlay와 ImGui panel을 함께 켜고 끈다.
- 프로파일러 프레임 범위는 `Update` 직전 `BeginFrame`, `Render` 직후 `EndFrame`으로 고정한다.

#### C2-2. 정적 맵 배치 실측

`Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.mapplacements`와 catalog를 읽어 얻은 값은 다음과 같다.

| 항목 | 실측값 |
|---|---:|
| placement | 13,097 |
| unique asset | 263 |
| singleton asset | 44 |
| 2개 이상 반복 asset | 219 |
| 반복 asset에 속한 placement | 13,053 |
| 10회 이상 반복 asset | 138 |
| 100회 이상 반복 asset | 41 |
| mirror parity 음수 | 4,521 |
| mirror parity 양수 | 8,576 |
| `assetId + mirrorParity` batch key | 395 |
| visible=true | 12,788 |
| visible=false | 309 |

현재 `CMapTool`은 placement마다 `CMapAssetObject` clone을 만들고 ObjectManager에 등록한다. 각 객체는 `Late_Update`에서 렌더 그룹에 들어가며 `Render()`에서 모든 submesh에 `DrawIndexed`를 호출한다. 정적 placement 13,097개가 게임 오브젝트 업데이트, shared_ptr/list 삽입, 재질 바인드, 드로 호출을 각각 발생시키는 구조다.

맵 전체 좌표 범위는 대략 X `-54.8..1026.5`, Y `-203.5..65.9`, Z `-1367..84.5`다. 카메라가 보는 영역보다 배치 범위가 매우 크므로 인스턴싱과 별개로 프러스텀 컬링이 필요하다.

#### C2-3. 렌더 경로 실측

- `CRenderer` 그룹은 `PRIORITY`, `SHADOW`, `NONBLEND`, `LIGHTS`, `COMBINED`, `NONLIGHT`, `BLEND`, `UI`, `DEBUG`다.
- 일반 메시 호출은 `CVIBuffer::Render()`의 `DrawIndexed`다.
- 인스턴스 경로는 `CVIBuffer_Instance::Render()`의 `DrawIndexedInstanced`가 있으나 맵 메시에는 연결되지 않았다.
- `CCookedModel::RenderMesh()`, `Effect_Runtime.cpp`, ImGui backend에는 직접 draw 호출이 존재한다.
- 현재 draw call, bind 횟수, GPU timestamp, pipeline statistics를 수집하는 공통 계층은 없다.
- `CModel` clone은 mesh/material 자원을 공유하므로 placement 수만큼 텍스처가 복제되지는 않지만, asset별로 복사된 동일 내용 텍스처는 별 SRV가 될 수 있다.
- `CMapAssetObject::Late_Update()`에는 프러스텀 컬링이 없다.

#### C2-4. 텍스처 실측

발탄 LV pack의 파일과 내용 해시 결과는 다음과 같다.

| 항목 | 실측값 |
|---|---:|
| 전체 파일 | 733 |
| 전체 바이트 | 372,375,200 |
| DDS | 464 |
| PNG | 7 |
| TGA | 1 |
| WModel | 260 |
| texture 파일 | 476 |
| texture 총 바이트 | 361,543,041 |
| unique payload | 290 |
| unique payload 바이트 | 217,866,699 |
| duplicate texture alias | 186 |
| duplicate 바이트 | 143,676,342 |
| duplicate hash group | 95 |

즉 현재 주된 기회는 PNG 7개를 단순 DDS로 바꾸는 것만이 아니다. 다른 asset 폴더에 복제된 동일 DDS를 동일 SRV로 공유하는 내용 기반 캐시가 필요하다.

동적 환경 세션 반영 뒤 현재 `.mapassets` 269개를 embedded WMA2 슬롯 기준으로 다시
감사한 값은 다음과 같다. 위 표는 pack 물리 파일 기준이고 아래 표는 실제 runtime
material reference 기준이므로 분모가 다르다.

| runtime WMA2 감사 항목 | 실측값 |
|---|---:|
| catalog asset | 269 |
| material texture reference | 579 |
| unique resolved texture path | 493 |
| unique payload | 303 |
| duplicate payload group | 99 |
| content-identical source 중복 바이트 | 147,078,772 |
| loader까지 같은 runtime cache 중복 group | 99 |
| runtime SRV cache로 제거 가능한 source 중복 바이트 | 147,078,772 |
| resolved source 바이트 | 367,153,306 |
| missing reference | 0 |
| DDS reference | 463 |
| PNG/TGA 변환 후보 reference | 116 |

#### C2-5. 참고 엔진 검토 결론

**Winters Engine Profiler**

- QPC CPU scope, 프레임 통계, DX11 timestamp/disjoint query, pipeline statistics, non-blocking readback, JSON timeline을 이미 분리했다.
- query ring과 `D3D11_ASYNC_GETDATA_DONOTFLUSH` 사용 방식은 LostArk에 이식할 가치가 높다.
- RHI 전체를 복사하지 않고 `ProfilerTypes`, CPU scope, DX11 frame query ring, overlay/JSON 스키마만 현재 Engine 구조에 맞춰 재작성한다.

**SR_MinecraftDungeons**

- `CBatchBuffer`는 같은 atlas와 같은 상태의 voxel face를 하나의 baked VB/IB로 합치는 방식이다.
- 균일한 큐브·atlas 기반 맵에는 적합하지만, 263개 모델·복수 submesh·서로 다른 재질·alpha/sky/UV/emissive 상태·음수 determinant가 공존하는 LostArk 맵에는 그대로 적용할 수 없다.
- 참고할 부분은 batch dirty/rebuild 흐름이며, 실제 렌더는 원본 mesh를 공유하는 hardware instancing으로 구현한다.

**Winters/LoL Navigation**

- `CMapSurfaceSampler`, `CMapWalkableBaker`, `CNavGrid`, `CPathfinder`의 역할 분리가 현재 목적에 가장 가깝다.
- cell bitset, 반경 팽창, octile heuristic, diagonal corner-cut 방지, generation array, reachability cache 원칙을 채택한다.

**SR A***

- 인접 방향과 대각선 금지 규칙은 참고 가능하다.
- 노드별 동적 할당과 map 중심 구현은 발탄 실시간 쿼리 경로에는 사용하지 않는다.

#### C2-6. 동적 환경 세션 재대조와 최종 비평

| 우선순위 | 실측 | 최종 코드 결정 |
|---|---|---|
| P1 | `CMapTool` 멤버·생성·갱신이 `_DEBUG` 안에 있어 Release에는 13,097개 맵 장면이 없다. | `CMainApp`이 모든 구성에서 `CMapSceneRuntime`을 소유한다. |
| P1 | `CDeployPropCatalog`이 정의와 배치를 같이 읽고 `CMapTool`이 객체를 소유한다. | `Catalog`/`Document`/`Runtime`을 분리한다. |
| P1 | 전역 `FRACTURED` 상태가 비파괴 placement에도 전달된다. | `CDeployPropRuntime::Resolve_State()`에서 placement별 실제 상태를 정한다. |
| P2 | 한 `CShader` prototype은 모든 pass에 같은 input element 배열을 적용한다. | standalone과 instanced Effect를 분리하고 pixel/material 함수만 HLSLI로 공유한다. |
| P2 | 기존 Section 5는 일부 header만 있고 구현 파일 전체가 없다. | 새 파일 전체와 기존 함수의 최종 교체 블록을 Section 5에 고정한다. |

`CMapSceneRuntime` 전환 전 Release 캡처는 발탄 장면 baseline으로 인정하지 않는다.

### C3. Constraints — 불변 조건과 협업 경계

1. `main` 정본을 기준으로 기능 브랜치와 PR에서 구현한다.
2. 현재 작업 트리의 미커밋 파일은 다른 세션의 소유다. 본 구현 브랜치를 만들기 전 해당 작업을 커밋 또는 별도 worktree로 격리한다.
3. 동적 환경 계획이 소유한 `MapAssetCatalog`, `MapAssetObject`, `MapTool`, `Renderer`, HLSL은 그 계획의 완료 커밋 이후에 리베이스한다.
4. 최종 render profile의 `renderMode`, `uvSpeed`, `opacity`, `emissiveIntensity`, `cullMode` 계약을 바꾸지 않는다.
5. Engine C++ 파일의 기존 인코딩을 유지한다. Markdown과 JSON은 UTF-8로 저장한다.
6. 안정적인 `assetId`, `placementId`를 저장 ID로 유지한다. pointer, vector index, prototype tag는 저장 ID로 사용하지 않는다.
7. 정적 배치 최적화는 placement의 ID, Transform, visible 값을 변경하지 않는다.
8. Deploy/파괴/복원/skeletal/AnimModel 객체는 정적 batch에 들어가지 않는다.
9. `TRANSLUCENT`, `BACKGROUND`, 비표준 pass는 첫 구현에서 standalone fallback으로 남긴다.
10. GPU query 결과를 얻기 위해 `Flush()`하거나 현재 프레임을 blocking wait하지 않는다.
11. 프로파일러가 꺼져 있을 때의 오버헤드도 별도로 측정한다.
12. NavGrid의 기본 보행면은 발탄 아레나의 실제 바닥 삼각형에서 생성한다. 수동 편집은 기본 보행면을 새로 만드는 용도가 아니라 잘못 열린 셀을 닫는 `ForceBlocked` 보정 계층으로만 사용한다.
13. 기존 triangle `CNavigation` 사용 레벨은 동작을 유지한다.
14. `LV_COMMON_MESH_CUL_BOX_8`은 walkable mesh가 아니라 발탄 중앙 아레나의 bake bounds 근거로만 사용한다.
15. 원본 mesh 이름과 삼각형은 offline bake에서만 사용하며 runtime은 `.navgrid`의 bitset과 height 배열만 읽는다.

### C4. Contract — 저장·런타임·프로파일 계약

#### C4-1. F2/F4와 캡처 계약

| 키 | 문맥 | 동작 |
|---|---|---|
| F2 | `LEVEL::LOGO` | 기존과 동일하게 `LEVEL::ASSET_TEST` 진입 |
| F4 | 프로파일러 닫힘 | 좌상단 overlay와 ImGui panel 표시, history reset, 캡처 시작 |
| F4 | 프로파일러 열림 | 캡처 종료, timestamp JSON 저장, overlay와 panel 함께 닫기 |

패널의 `Save` 버튼은 열린 상태에서도 현재 history snapshot을 저장한다. 기본 저장 경로는 `Client/Bin/Profiles/YYYY-MM-DD_HH-mm-ss_LostArkProfiler.json`이며 마지막 성공본을 `Client/Bin/Profiles/profiler.json`으로 원자 교체한다.

Release 성능 비교는 ImGui가 없는 상태에서도 수행할 수 있어야 한다. 명령행 `--profile-warmup 300 --profile-frames 600 --profile-output <path>`를 지원하고 지정 프레임 종료 후 JSON을 저장한다.

JSON schema 이름은 `LostArkProfilerTimeline.v1`로 고정하고 최소 다음 필드를 기록한다.

```json
{
  "schema": "LostArkProfilerTimeline.v1",
  "build": { "configuration": "Release", "gitCommit": "working-tree" },
  "scene": { "level": "ASSET_TEST", "mapId": "LV_LUT_HEARTRB_ED" },
  "capture": { "warmupFrames": 300, "frameCount": 600, "requestedFrameCount": 600 },
  "dropped": { "cpuScopes": 0, "gpuFrames": 0 },
  "frames": [
    {
      "frame": 0,
      "cpuMs": { "frame": 0.0, "update": 0.0, "render": 0.0 },
      "cpuScopes": { "Client.Update": 0.0, "Client.Render": 0.0 },
      "gpuMs": { "frame": 0.0, "valid": true, "latencyFrames": 4 },
      "draw": { "calls": 0, "instancedCalls": 0, "instances": 0, "indices": 0 },
      "pipeline": { "iaVertices": 0, "iaPrimitives": 0, "vsInvocations": 0, "psInvocations": 0 },
      "map": { "placements": 13097, "visibleInstances": 0, "batchCount": 0, "fallbackObjects": 0 },
      "texture": { "requests": 0, "pathHits": 0, "contentHits": 0, "uniqueSrvs": 0, "estimatedGpuBytes": 0 },
      "navigation": { "queries": 0, "expandedNodes": 0, "queryUs": 0, "pathCells": 0 }
    }
  ],
  "summary": { "frameP50Ms": 0.0, "frameP95Ms": 0.0, "frameP99Ms": 0.0, "gpuP95Ms": 0.0 }
}
```

#### C4-2. 정적 batch 계약

정적 batch key는 다음 값으로 구성한다.

```text
MapStaticBatchKey = stable assetId + mirrorParity
mirrorParity = sign(scale.x * scale.y * scale.z) < 0
```

render profile은 catalog의 asset 단위 정의이므로 첫 구현의 key에 중복 저장하지 않는다. catalog가 placement override를 허용하게 될 경우에만 `renderProfileHash`를 key에 추가한다.

batch 적격 조건은 다음과 같다.

```text
sourceLayer == StaticMap
modelType == StaticModel
renderMode == DEFERRED
supportsInstancing == true
```

다음은 standalone fallback이다.

```text
TRANSLUCENT, BACKGROUND, unsupported vertex layout,
Deploy, Destructible, Restore, Skeletal, AnimModel, Effect
```

각 batch는 원본 `CModel` 하나, placement record 배열, visible instance matrix 배열, 동적 instance buffer 하나를 소유한다. 매 프레임 placement bounding sphere를 프러스텀 검사하고 보이는 행렬만 discard/no-overwrite 정책으로 업로드한다. submesh별로 한 번씩 `DrawIndexedInstanced`한다.

#### C4-3. 텍스처 캐시 계약

텍스처 요청은 다음 순서로 처리한다.

```text
normalize absolute path
-> path cache lookup
-> SHA-256 content key lookup
-> miss일 때 기존 DDS/TGA/WIC loader 호출
-> SRV와 metadata를 path/content cache 양쪽에 commit
```

cache key는 `loader + content SHA-256`이다. semantic은 audit/변환 포맷 결정에는
사용하지만 동일 byte와 동일 loader가 만드는 D3D resource는 같으므로 SRV key에는 넣지
않는다. path alias는 같은 content entry를 가리키고 실패한 load는 cache에 commit하지
않는다.

DDS 변환은 WMaterial 슬롯 의미를 기준으로 한다.

| 의미 | 목표 포맷 |
|---|---|
| opaque diffuse/base color | BC7_UNORM_SRGB, 품질 허용 시 BC1_UNORM_SRGB |
| alpha/translucent diffuse | BC7_UNORM_SRGB 또는 BC3_UNORM_SRGB |
| tangent-space normal | BC5_UNORM |
| mask/roughness/metallic/AO | 채널 의미에 따라 BC4/BC7 UNORM |
| emissive color | BC7_UNORM_SRGB |

모든 변환본은 mipmap을 생성한다. 파일명 접미사만으로 normal/mask를 추정하지 않고 `.wmat` 슬롯과 manifest를 읽는다.

#### C4-4. NavGrid 파일 계약

`ValtanArena.navgrid`는 little-endian binary다.

```cpp
struct NAVGRID_FILE_HEADER_V1
{
    char     magic[8];          // "WNAVGRD\0"
    uint32_t version;           // 1
    uint32_t width;
    uint32_t height;
    float    cellSize;
    float    originX;
    float    originZ;
    float    minHeight;
    float    maxHeight;
    float    agentRadius;
    float    maxSlopeDegrees;
    float    maxStepHeight;
    uint32_t walkableByteCount;
    uint32_t heightCount;
    uint8_t  sourceSha256[32];
};
// header -> walkable bitset -> float height[width * height]
```

loader는 magic/version, 파일 전체 크기, `width*height` overflow, 셀 수 상한, 양의 cellSize, 유한 좌표/높이, bitset padding 0, walkable cell의 유한 높이를 검증하고 stage 후 commit한다.

MapTool 보정 문서는 binary grid와 분리해 다음 schema로 저장한다.

```json
{
  "schema": "LostArkNavGridPatch.v1",
  "mapId": "LV_LUT_HEARTRB_ED",
  "baseSourceSha256": "64자리 hex",
  "width": 0,
  "height": 0,
  "cellSize": 0.5,
  "origin": [0.0, 0.0],
  "forceBlocked": [0, 1, 2]
}
```

`forceBlocked`는 정렬·중복 제거한 `z * width + x` cell index다. builder는 mapId,
base source hash, width/height/cellSize/origin이 하나라도 다르면 patch 적용을 거부한다.
첫 버전에는 `forceWalkable`을 만들지 않는다. 잘못 막힌 셀을 사람이 억지로 여는 기능은
바닥이 없는 공간을 이동 가능하게 만들 수 있기 때문이다.

베이크 기준은 다음과 같다.

- 입력: Floor01/A/B source glTF와 exact placement manifest
- bake bounds: `LV_LUT_HEARTRB_ED_SL01:export:2767`의 `MAP_1E5F55FC0524_LV_COMMON_MESH_CUL_BOX_8` world AABB
- cell size: 0.5m
- agent radius: 0.4m
- max slope: 45°
- max step height: 0.6m
- 위를 향한 삼각형만 후보
- 동일 XZ 셀은 상단 표면을 선택
- MapTool이 저장한 `ValtanArena.navpatch.json`의 `ForceBlocked` 셀을 적용
- 중심 seed에서 flood fill하여 연결된 아레나만 보존
- 외곽 및 hole과 agent radius 이내 셀을 비보행 처리
- 최종 grid 크기는 CUL_BOX_8 world AABB에서 계산하며 숫자를 코드에 하드코딩하지 않음

#### C4-5. A* 계약

- 8방향, 직선 비용 10, 대각 비용 14
- octile heuristic
- 대각선 이동은 인접한 두 직선 셀이 모두 보행 가능할 때만 허용
- 높이 차이가 `maxStepHeight`를 넘으면 인접하지 않음
- 요청 지점이 비보행이면 제한 반경 내 가장 가까운 보행 셀을 탐색
- open set은 binary heap
- g-score, parent, open/closed generation은 grid 크기만큼 한 번 할당하고 쿼리마다 재사용
- 성공 경로는 grid line-of-sight로 smoothing
- 첫 단계에서는 동적 장애물 overlay를 적용하지 않되 API 공간은 예약

### C5. Components — 책임 분리

```text
Client main loop
 ├─ Engine::CProfiler
 │   ├─ CPU scope/QPC
 │   ├─ DX11 query ring
 │   ├─ draw/bind/map/texture/nav counters
 │   └─ immutable capture snapshot
 └─ Client::CProfilerPanel
     ├─ F2 context handling
     ├─ ImGui graphs/tables
     └─ async JSON writer

CMapTool
CMainApp
 └─ CMapSceneRuntime (Debug/Release 공통 장면 owner)
     ├─ CMapAssetCatalog + CMapPlacementDocument
     ├─ CMapStaticBatchObject[] (opaque static)
     ├─ CMapAssetObject[] (fallback only)
     ├─ CDeployPropRuntime
     │   ├─ CDeployPropCatalog (생성 가능한 정의)
     │   ├─ CDeployPlacementDocument (ID/Transform/상태 입력)
     │   └─ CDeployPropObject[] (runtime clone)
     └─ CValtanNavigationRuntime

CMapTool (Debug 전용 non-owning UI)
 └─ CMapSceneRuntime에 stable placement ID 명령 전달

CMapAssetRenderUtils
 └─ standalone/batch 공통 material/pass 바인드

CMaterial
 └─ CTextureResourceCache
     ├─ normalized path aliases
     ├─ SHA-256 content entries
     └─ texture statistics

CNavigation (기존 façade)
 ├─ Legacy triangle navigation
 └─ CNavGrid + CPathFinder
     └─ CValtanNavigationRuntime (AssetTest 입력/표시)
```

### C6. Correctness — 정확성 기준

1. 최적화 전후 placement count, assetId, placementId, Transform, visible 값을 JSON diff했을 때 동일해야 한다.
2. 음수 determinant는 양수 batch와 분리하고 해당 rasterizer cull mode를 반전한다.
3. batch와 fallback은 동일 `MapAssetRenderUtils`를 사용해 diffuse/normal/emissive/UV/alpha/cull/pass가 달라지지 않게 한다.
4. 투명·sky·동적 객체는 인스턴스 batch에 포함하지 않는다.
5. 프러스텀 컬링은 local bounding sphere에 world 최대 축척을 적용한다.
6. GPU query가 아직 준비되지 않았으면 해당 프레임을 `valid=false`로 기록하고 CPU를 막지 않는다.
7. texture cache는 load 성공 후에만 commit하고 동일 내용이라도 loader mode가 다르면 별 key를 사용한다.
8. NavGrid checksum은 정확한 source geometry/placement/tool option을 포함한다.
9. A*가 실패하면 기존 경로를 덮어쓰지 않고 명시적 실패 결과를 반환한다.
10. legacy navigation 파일과 레벨은 결과가 바뀌지 않아야 한다.

### C7. Compatibility — 기존 경로와 마이그레이션

- `CNavigation`의 기존 triangle API와 `Navigation.dat` loader는 유지한다.
- `CMapAssetObject`는 `TRANSLUCENT`/`BACKGROUND`/debug/fallback 및 개별 검증 경로로 계속 존재한다.
- `CCookedModel`/`CBinaryAssetObject`는 계측만 추가하며 신규 맵 경로로 승격하지 않는다.
- MapTool 저장 문서 형식은 바꾸지 않는다. load 후 runtime 표현만 object-per-placement에서 batch/fallback으로 변환한다.
- 기존 F1 MapTool, 로고 F2 AssetTest 진입을 보존한다.
- 동적 환경 계획의 shader/material 결과를 baseline으로 삼고 shared HLSLI를 추출할 때 결과가 동일한지 픽셀 캡처로 검증한다.
- Release 자동 캡처는 UI 의존성이 없고 Debug F2 패널은 동일 Engine snapshot만 읽는다.

### C8. Completion Gates — 완료 기준

#### C8-1. 측정 방법

- 동일 Release 실행 파일
- 동일 AssetTest 레벨, 동일 camera Transform, 동일 phase, 동일 해상도/그래픽 설정
- warmup 300 frame
- capture 600 frame
- before/after 각각 JSON과 화면 캡처 보존
- p50/p95/p99 및 최대값 비교

#### C8-2. 성능·정확성 기준

| 항목 | 완료 기준 |
|---|---|
| profiler off CPU overhead | frame p95 기준 0.2ms 이하를 목표, 초과 시 scope/counter 축소 후 재측정 |
| GPU query | 렌더 thread blocking/Flush 없음, 지연 frame과 invalid sample 기록 |
| 정적 map runtime object | 13,097개에서 batch + fallback 기준 450개 이하 목표 |
| 정적 map draw calls | 동일 camera에서 기존 대비 80% 이상 감소 |
| batch key | 실측 395 raw key와 runtime 생성 결과를 비교 |
| 화면 | diffuse/normal/emissive/UV/alpha/sky/cull 회귀 없음 |
| 데이터 | 13,097 placement ID/Transform/visible diff 0 |
| texture cache | full LV load에서 path/content hit와 unique SRV 수 JSON 기록 |
| duplicate opportunity | 186 alias, 143,676,342B 기준으로 실제 hit/미로드 사유 보고 |
| NavGrid | source checksum 일치, outside/hole walkable 0 |
| A* | arena 테스트 쿼리 p95 1ms 이하, corner cut 0 |
| build | Engine Debug/Release, UpdateLib Debug/Release, Client Debug/Release PASS |
| runtime | `Enter -> F2 -> AssetTest`, profiler F4, map reload, nav query PASS |

---

## 2. 문제 해결 ①~⑤

### ① Winters 기반 범용 프로파일러를 현재 Engine에 맞게 이식

#### 문제

현재는 FPS나 체감만으로 병목을 추정해야 하며 CPU update, render group, GPU, draw, texture, navigation 비용을 같은 frame에서 비교할 수 없다.

#### 해결

1. `CGameInstance`가 `CProfiler`를 소유한다.
2. QPC 기반 RAII CPU scope와 frame counter를 구현한다.
3. DX11 query ring은 disjoint/timestamp/pipeline statistics를 프레임별로 보유한다.
4. `GetData(..., D3D11_ASYNC_GETDATA_DONOTFLUSH)`로 준비된 과거 프레임만 읽는다.
5. 모든 draw wrapper와 필요한 직접 draw 지점에서 counter를 증가시킨다.
6. `CRenderer::Draw_RenderGroup()`의 각 그룹을 named CPU scope로 감싼다.
7. Client 패널은 immutable snapshot만 읽고 JSON 쓰기는 worker thread에서 수행한다.
8. F2 AssetTest 진입을 보존하고 F4 profiler toggle과 Release command-line capture를 구현한다.

#### 산출물

- `LostArkProfilerTimeline.v1` JSON
- frame time graph
- CPU scope table
- GPU/pipeline statistics
- draw/instance/index count
- map/texture/navigation custom counter

### ② 거대 메시 병합이 아니라 material-safe hardware instancing 적용

#### 문제

현재 13,097 placement가 개별 객체이므로 update, render queue, 재질 바인드, draw call 비용이 중복된다. 그렇다고 모든 메시를 하나로 합치면 서로 다른 모델·재질·pass·UV 흐름·alpha·sky·cull 상태를 잃고, 카메라 밖 geometry까지 영구 VB에 들어가며 동적/파괴 객체 분리가 불가능해진다.

#### 해결

1. MapTool의 placement 문서를 런타임 정본으로 유지한다.
2. `DEFERRED` static을 `(assetId, mirrorParity)`로 partition한다.
3. 각 batch는 placement sphere를 프러스텀 검사한다.
4. 보이는 world matrix만 `VTXINSTANCE_MODEL` buffer에 업로드한다.
5. 원본 `CMesh` VB/IB를 그대로 사용하고 submesh마다 `DrawIndexedInstanced`한다.
6. batch/standalone의 render profile 바인딩은 `CMapAssetRenderUtils`로 공유한다.
7. placement edit는 stable placementId로 batch slot을 찾아 dirty 처리한다.
8. `TRANSLUCENT`/`BACKGROUND`/dynamic/Deploy/skeletal/effect는 standalone 또는 각 전용 runtime에 둔다.

#### 예상 구조 변화

```text
Before: 13,097 CMapAssetObject + submesh별 DrawIndexed
After : 약 395 CMapStaticBatchObject + 소수 fallback + submesh별 DrawIndexedInstanced
```

이 수치는 상한 보장이 아니라 현재 데이터의 raw batch key 실측값이다. 최종 fallback 수와 실제 batch count는 profiler JSON과 결과 보고서에 기록한다.

### ③ 텍스처 중복 SRV 제거와 의미 기반 DDS 감사

#### 문제

파일의 대부분은 이미 DDS이므로 확장자 변환만으로는 143.7MB의 duplicate payload를 해결하지 못한다. 또한 잘못된 포맷 선택은 normal/alpha/emissive 품질을 깨뜨릴 수 있다.

#### 해결

1. `CMaterial`의 모든 texture load를 `CTextureResourceCache`로 통과시킨다.
2. normalized path hit와 SHA-256 content hit를 구분 계측한다.
3. 동일 payload는 같은 `ID3D11ShaderResourceView`를 공유한다.
4. 감사 도구가 catalog, WModel/WMaterial, texture 슬롯, 크기, 포맷, mip, alpha, content hash를 읽는다.
5. `--apply` 없이는 파일을 변경하지 않는 audit-only를 기본값으로 둔다.
6. 변환 시 임시 파일 생성, load 검증, SHA-256 영수증 생성 후 원자 교체한다.
7. 변환 전후 화면과 VRAM 추정치를 비교한다.

### ④ 발탄 실제 바닥 기반 NavGrid와 A* 구현

#### 문제

현재 `CNavigation`은 triangle containment와 neighbor 이동 검증만 제공하고 A*가 없다. neighbor 파일이 없을 때 O(N²) exact point 비교를 수행한다. AssetTest 발탄 아레나에는 별도 navigation이 없다.

#### 해결

1. Floor01/A/B exact source glTF와 placement manifest를 기본 보행면 입력으로 사용한다.
2. `LV_LUT_HEARTRB_ED_SL01:export:2767`의 CUL_BOX_8 transform과 원본 bounds로 bake 범위를 계산한다. CUL_BOX_8 삼각형 자체는 walkable 후보에 넣지 않는다.
3. 실제 world triangle을 XZ 0.5m grid에 rasterize하고 cell별 Y를 저장한다.
4. slope/step/연결성/agent radius 기준으로 `BaseWalkable` bitset을 만든다.
5. MapTool에서 잘못 열린 셀만 `ForceBlocked`로 칠해 `ValtanArena.navpatch.json`에 저장한다. 첫 버전은 안전성을 위해 `ForceWalkable`을 허용하지 않는다.
6. base와 patch를 합친 binary navgrid와 JSON receipt를 생성한다.
7. `CNavigation`에 `LEGACY_TRIANGLE`과 `NAVGRID_ASTAR` mode를 추가한다.
8. `CPathFinder`는 preallocated node array와 직접 구현한 binary min-heap을 사용한다.
9. AssetTest에서 플레이어 현재 cell을 start로 사용하고 LMB 목적지, N grid 표시로 검증한다.
10. 경로 성공 시 별도 path follower가 waypoint를 순서대로 소비해 LanceMaster Transform을 이동시킨다. A*는 Transform을 직접 변경하지 않는다.
11. 파괴물 동적 obstacle은 이번 단계에서 연결하지 않고, Deploy runtime 완료 후 별도 `DynamicBlocked` 계층으로 추가한다.

### ⑤ 전후 비교와 회귀 검증을 자동화

#### 문제

기능이 보이는 것과 실제로 빨라진 것은 다르다. Debug UI가 켜진 결과와 Release 결과를 섞으면 판단할 수 없다.

#### 해결

1. 최적화 전 baseline commit에서 동일 카메라 자동 capture를 남긴다.
2. 각 단계 후 placement invariant 검사와 Release capture를 수행한다.
3. 프로파일러 자체 off/on overhead도 capture한다.
4. `compare_profiler_captures.py`가 p50/p95/p99와 counter 차이를 Markdown으로 출력한다.
5. 최종 결과 문서에 적용/기각한 최적화와 근거를 기록한다.

#### 구현 순서

```text
Gate 0  다른 세션 동적 환경 결과 완료/커밋/리베이스
Gate 1  Profiler + baseline JSON
Gate 2  Frustum culling만 적용하고 독립 효과 측정
Gate 3  Static instancing 적용하고 독립 효과 측정
Gate 4  Texture cache 적용하고 load/VRAM 효과 측정
Gate 5  DDS audit 결과 검토 후 필요한 파일만 변환
Gate 6  Valtan NavGrid bake/load/A*
Gate 7  전체 Debug/Release/runtime/visual regression
```

각 Gate는 이전 Gate의 capture와 invariant 검사를 통과해야 다음으로 진행한다. 성능이 나빠진 최적화는 별도 근거 없이 누적하지 않는다.

---

## 3. 자료구조·알고리즘 핵심

### 3-1. CPU scope와 frame snapshot

```cpp
enum class EProfilerCounter : uint16_t
{
    DrawCalls,
    InstancedDrawCalls,
    Instances,
    Indices,
    RenderSubmissionsPriority,
    RenderSubmissionsShadow,
    RenderSubmissionsNonBlend,
    RenderSubmissionsBlend,
    MapPlacements,
    MapVisibleInstances,
    MapBatchCount,
    MapFallbackObjects,
    TextureRequests,
    TexturePathHits,
    TextureContentHits,
    TextureUniqueSrvs,
    TextureEstimatedGpuBytes,
    NavigationQueries,
    NavigationExpandedNodes,
    NavigationQueryMicroseconds,
    NavigationPathCells,
    Count
};

struct FProfilerScopeSample
{
    uint32_t NameId;
    uint32_t Depth;
    uint64_t BeginTick;
    uint64_t EndTick;
};

struct FProfilerFrame
{
    uint64_t FrameNumber;
    double CpuFrameMs;
    double GpuFrameMs;
    bool GpuValid;
    uint32_t GpuLatencyFrames;
    std::array<uint64_t, static_cast<size_t>(EProfilerCounter::Count)> Counters;
    std::vector<FProfilerScopeSample> CpuScopes;
    D3D11_QUERY_DATA_PIPELINE_STATISTICS Pipeline;
};
```

CPU scope는 thread-local stack에 begin index를 보관한다. `EndScope`는 같은 frame의 sample을 닫는다. 첫 구현은 main/render thread를 대상으로 하며 loader worker는 texture cache의 atomic counter만 기록한다.

### 3-2. DX11 query ring

```text
BeginFrame N
  Begin(disjoint[N % 8])
  End(timestampBegin[N % 8])
  Begin(pipeline[N % 8])

EndFrame N
  End(pipeline[N % 8])
  End(timestampEnd[N % 8])
  End(disjoint[N % 8])
  TryResolve(N - 4 .. N - 7, DONOTFLUSH)
```

- ring size는 8, 최소 read latency는 4 frame이다.
- slot을 재사용하기 전 아직 resolve되지 않았다면 해당 GPU sample은 drop하고 counter를 증가시킨다.
- `Disjoint == TRUE` 또는 frequency 0이면 invalid다.
- pipeline statistics에는 ImGui와 직접 draw도 포함되므로 wrapper draw counter와 차이를 함께 표시한다.

### 3-3. 인스턴스 batch 자료구조

```cpp
struct FMapStaticBatchKey
{
    std::string AssetId;
    bool Mirrored;

    bool operator==(const FMapStaticBatchKey& rhs) const noexcept;
};

struct FMapStaticInstance
{
    uint64_t PlacementId;
    _float4x4 World;
    float3_t WorldBoundsCenter;
    float WorldBoundsRadius;
    bool Visible;
};

struct FMapStaticBatchBuildResult
{
    std::vector<std::shared_ptr<CMapStaticBatchObject>> Batches;
    std::vector<std::shared_ptr<CMapAssetObject>> FallbackObjects;
    std::unordered_map<uint64_t, FPlacementRuntimeHandle> PlacementLookup;
};
```

#### batch build

```text
parse placement/catalog
-> validate 모든 asset와 transform
-> stage eligible record를 unordered_map<BatchKey, vector<Instance>>에 분류
-> stage fallback objects
-> 모든 model/instance buffer 생성 성공 확인
-> commit으로 기존 runtime set 교체
-> 실패 시 staged 객체 전체 해제, 기존 runtime 보존
```

#### visibility upload

```text
for each instance in batch:
    if !instance.Visible: continue
    if !Frustum.IntersectsSphere(center, radius): continue
    visibleMatrices.push_back(instance.World)

if visibleMatrices.empty(): skip render group
else Map(WRITE_DISCARD), memcpy, Unmap
```

동적 buffer capacity는 다음 2의 거듭제곱으로 증가시키고 줄이지 않는다. placement 편집이나 visible 변경은 CPU record만 변경하며 다음 Late_Update에서 visible buffer를 재작성한다.

### 3-4. mirror parity와 culling

world determinant 부호가 다른 인스턴스를 한 draw에 섞지 않는다. shader에서 winding을 바꾸는 대신 batch별 rasterizer state를 선택한다.

```cpp
const bool mirrored =
    placement.Scale.x * placement.Scale.y * placement.Scale.z < 0.f;
```

기존 render profile이 `CullBack`이면 mirrored batch는 `CullFront`, `CullFront`면 `CullBack`, `CullNone`은 그대로다.

### 3-5. material/pass 공유

`CMapAssetRenderUtils`는 standalone과 batch가 공유하는 순수 바인딩 함수다.

```cpp
struct FMapAssetRenderContext
{
    const MAP_ASSET_RENDER_PROFILE* Profile;
    float2_t UvOffset;
    f32_t EmissiveIntensity;
    f32_t Opacity;
    bool Mirrored;
    bool Instanced;
};

HRESULT BindMapAssetMaterial(
    CShader& shader,
    CModel& model,
    uint32_t meshIndex,
    const FMapAssetRenderContext& context);
```

여기에는 texture 슬롯, render profile 상수, pass index, rasterizer 선택만 둔다. world matrix는 standalone은 기존 Transform, batch는 instance input으로 전달한다.

### 3-6. texture content cache

```cpp
struct FTextureContentKey
{
    ETextureLoader Loader;
    ETextureSemantic Semantic;
    std::array<uint8_t, 32> Sha256;
};

struct FTextureCacheEntry
{
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Srv;
    uint64_t SourceBytes;
    uint64_t EstimatedGpuBytes;
    DXGI_FORMAT Format;
    uint32_t Width;
    uint32_t Height;
    uint32_t MipLevels;
};
```

동시 load가 같은 content를 요청할 수 있으므로 cache mutex 아래에서 double-check한다. 파일 읽기·해시는 lock 밖에서 하고 commit 직전에 다시 검사한다.

### 3-7. NavGrid rasterization

삼각형의 world normal과 XZ projected AABB를 계산한다. 각 cell center의 barycentric coordinate가 삼각형 투영 내부이면 plane equation으로 Y를 얻는다. 여러 표면이 겹치면 발탄 center seed와 연결 가능한 상단 표면을 선택한다.

```text
CUL_BOX_8 world AABB로 grid allocation
-> Floor01/A/B raw candidate cells
-> slope filter
-> max-step neighbor graph
-> blocked distance transform
-> agentRadius/cellSize 만큼 walkable 침식
-> ForceBlocked navpatch 적용
-> center seed flood fill
-> outside/disconnected 제거
-> bitset + height array 직렬화
```

source receipt에는 glTF/placement SHA-256, tool version, options, grid bounds, walkable count, connected component count를 쓴다.

### 3-8. A* 메모리와 실행

```cpp
struct FPathNodeState
{
    float G;
    uint32_t Parent;
    uint32_t OpenGeneration;
    uint32_t ClosedGeneration;
    int32_t HeapPosition;
};
```

query generation이 overflow하면 generation 배열만 0으로 초기화한다. binary heap은 `F = G + H`, tie는 작은 H, 그다음 작은 cell index 순으로 정렬해 결과를 결정적으로 만든다.

한 query의 책임 경계는 다음과 같다.

```text
CValtanNavigationRuntime::Request_Path
  -> CNavigation::Find_Path
    -> CPathFinder::Find_Path
      -> CNavGrid::Find_NearestWalkable(start/goal)
      -> open heap에서 최소 F pop
      -> 8방향 이웃 검사
      -> diagonal corner-cut / max-step 차단
      -> parent 갱신
      -> goal에서 parent 역추적
      -> 필요할 때 line-of-sight smoothing
  -> 성공 결과를 path follower에 commit
  -> follower가 매 frame Transform만 변경
```

A* 실패는 기존 이동 경로를 덮어쓰지 않는다. 클릭 입력, 탐색, 이동을 분리하여 입력 한 번에 query 한 번만 실행하고 매 frame A*를 재실행하지 않는다.

### 3-8a. 보행 데이터의 네 계층

| 계층 | 저장 내용 | 변경 시점 | 소유자 |
|---|---|---|---|
| source geometry | Floor triangle과 exact Transform | 추출/맵 복원 시 | Resource/placement 문서 |
| base grid | cell별 기본 walkable bit와 height | offline bake 시 | `.navgrid` builder |
| authoring patch | 사람이 닫은 `ForceBlocked` cell index | MapTool 편집 시 | `.navpatch.json` |
| runtime path | 현재 query의 waypoint와 cursor | 목적지 클릭/이동 시 | navigation runtime/follower |

Deploy 파괴 상태로 열리고 닫히는 영역은 이 네 계층과 섞지 않고 후속 `DynamicBlocked` overlay와 nav revision으로 관리한다.

### 3-9. 단계별 rollback

- profiler query 생성 실패: GPU 항목만 disabled, CPU capture는 유지
- batch stage 실패: 기존 standalone runtime 보존
- texture load/cache 실패: 기존 loader 결과도 실패로 반환, partial cache commit 없음
- navgrid load 실패: 기존 navigation mode와 grid 보존
- path query 실패: 명시적 status 반환, caller의 기존 path 보존

---

## 4. 추가·수정·삭제 파일 목록

### 4-1. 추가 파일

#### Engine

| 파일 | 역할 |
|---|---|
| `Engine/Public/Profiler.h` | 범용 profiler API, CPU scope, counter, snapshot |
| `Engine/Private/Profiler.cpp` | QPC, DX11 query ring, history 구현 |
| `Engine/Public/TextureResourceCache.h` | path/content 기반 SRV cache 계약 |
| `Engine/Private/TextureResourceCache.cpp` | SHA-256, loader dispatch, 통계 구현 |
| `Engine/Public/NavGrid.h` | binary NavGrid load/query |
| `Engine/Private/NavGrid.cpp` | strict validation, 좌표 변환, bitset 구현 |
| `Engine/Public/PathFinder.h` | A* 요청/결과 및 알고리즘 계약 |
| `Engine/Private/PathFinder.cpp` | heap, generation, smoothing 구현 |

#### Client

| 파일 | 역할 |
|---|---|
| `Client/Public/MapSceneRuntime.h` | Debug/Release 공통 맵 장면 owner와 level 동기화 |
| `Client/Private/MapSceneRuntime.cpp` | 정적 맵·Deploy parse/validate/stage/commit/unload |
| `Client/Public/DeployPlacementDocument.h` | Deploy placement 저장 입력의 독립 문서 계약 |
| `Client/Private/DeployPlacementDocument.cpp` | strict placement parse/validate |
| `Client/Public/DeployPropRuntime.h` | Deploy clone 수명·phase·placement별 상태 해석 |
| `Client/Private/DeployPropRuntime.cpp` | Deploy stage/rollback/commit/unload |
| `Client/Public/ProfilerCaptureIO.h` | Debug/Release 공통 profiler JSON writer 계약 |
| `Client/Private/ProfilerCaptureIO.cpp` | schema v1 원자 저장과 percentile 계산 |
| `Client/Public/ProfilerPanel.h` | F4 panel/capture UI |
| `Client/Private/ProfilerPanel.cpp` | ImGui graph, JSON async save |
| `Client/Public/MapAssetRenderUtils.h` | standalone/batch 공통 render binding |
| `Client/Private/MapAssetRenderUtils.cpp` | render profile/pass/material 바인드 |
| `Client/Public/MapStaticBatchObject.h` | 정적 asset 인스턴스 batch 객체 |
| `Client/Private/MapStaticBatchObject.cpp` | culling, upload, instanced render |
| `Client/Public/ValtanNavigationRuntime.h` | AssetTest nav 입력/디버그 runtime |
| `Client/Private/ValtanNavigationRuntime.cpp` | nav load, 목적지 pick, A* query, waypoint 추종, cell 표시 |
| `Client/Bin/ShaderFiles/Shader_MapAssetMaterial.hlsli` | standalone/instance 공통 pixel·material 계약 |
| `Client/Bin/ShaderFiles/Shader_VtxMeshMap.hlsl` | 맵 standalone 전용 Effect |
| `Client/Bin/ShaderFiles/Shader_VtxMeshMapInstance.hlsl` | `VTXMESH_INSTANCE` 전용 instanced Effect |

#### Tool/Data/Test

| 파일 | 역할 |
|---|---|
| `Tools/LevelPlacementExtractor/audit_optimize_map_textures.py` | WMaterial 의미 기반 texture audit/선택 변환 |
| `Tools/LevelPlacementExtractor/build_valtan_navgrid.py` | glTF/placement 기반 navgrid bake |
| `Client/Bin/DataFiles/Navigation/ValtanArena.navpatch.json` | MapTool `ForceBlocked` 셀 보정 문서 |
| `Tools/LevelPlacementExtractor/compare_profiler_captures.py` | before/after JSON 비교 보고서 |
| `Tools/LevelPlacementExtractor/tests/test_build_valtan_navgrid.py` | raster/bitset/receipt 회귀 테스트 |
| `Tools/LevelPlacementExtractor/tests/test_profiler_capture_schema.py` | capture schema/summary 검증 |
| `Client/Bin/DataFiles/Navigation/ValtanArena.navgrid` | 발탄 arena runtime NavGrid |
| `Client/Bin/DataFiles/Navigation/ValtanArena.navgrid.receipt.json` | source/options/hash/통계 영수증 |

### 4-2. 수정 파일

#### Engine

| 파일 | 변경 블록 |
|---|---|
| `Engine/Public/Engine_Struct.h` | `VTXMESH_INSTANCE` input elements 추가 |
| `Engine/Public/GameInstance.h` | profiler/texture cache 소유 및 접근자 |
| `Engine/Private/GameInstance.cpp` | initialize/free와 façade 구현 |
| `Engine/Private/Renderer.cpp` | render group scopes/submission counters |
| `Engine/Private/VIBuffer.cpp` | indexed draw counter |
| `Engine/Private/VIBuffer_Instance.cpp` | indexed-instanced draw/instance counter |
| `Engine/Private/VIBuffer_Instance_Point.cpp` | instanced draw counter |
| `Engine/Private/CookedModel.cpp` | legacy direct draw counter |
| `Engine/Public/Material.h` | texture semantic/cache loader contract |
| `Engine/Private/Material.cpp` | 모든 texture load를 cache 경유 |
| `Engine/Public/Model.h` | instanced mesh bind/render API |
| `Engine/Private/Model.cpp` | mesh별 instanced render forwarding |
| `Engine/Public/Mesh.h` | instance vertex buffer binding API |
| `Engine/Private/Mesh.cpp` | 원본 VB/IB + instance VB bind/draw |
| `Engine/Public/Navigation.h` | legacy/navgrid mode와 FindPath façade |
| `Engine/Private/Navigation.cpp` | strict mode load/commit/query |
| `Engine/Default/Engine.vcxproj` | 새 Engine 파일 등록 |
| `Engine/Default/Engine.vcxproj.filters` | 물리 폴더 대응 filter 등록 |

#### Client

| 파일 | 변경 블록 |
|---|---|
| `Client/Default/Client.cpp` | frame profiler 범위와 Release CLI capture |
| `Client/Public/MainApp.h` | 공통 MapSceneRuntime과 Debug profiler panel 멤버 |
| `Client/Private/MainApp.cpp` | F1 MapTool, F4 profiler, panel render |
| `Client/Public/MapTool.h` | MapSceneRuntime non-owning 연결과 편집 명령 |
| `Client/Private/MapTool.cpp` | 장면 소유 제거, stable ID 편집 façade |
| `Client/Public/DeployPropCatalog.h` | placement 소유 제거, asset 정의 전용 catalog |
| `Client/Private/DeployPropCatalog.cpp` | `.deployassets`만 strict parse |
| `Client/Private/MapAssetObject.cpp` | 맵 전용 shader prototype과 공통 render utils 사용 |
| `Client/Private/Effect_Runtime.cpp` | 직접 draw counter 계측 |
| `Client/Public/Level_AssetTest.h` | Valtan nav runtime 보유 |
| `Client/Private/Level_AssetTest.cpp` | LanceMaster 생성, navgrid load/입력/디버그 연결 |
| `Client/Private/Loader.cpp` | profiler/texture/nav/batch prototype 단계 등록 |
| `Client/Public/LanceMaster.h` | 생성 prototype level과 테스트 이동 주체 계약 |
| `Client/Private/LanceMaster.cpp` | AssetTest/TestLevel2 공통 part prototype level 전달 |
| `Client/Public/Body_LanceMaster.h` | component prototype level 전달 |
| `Client/Private/Body_LanceMaster.cpp` | 전달받은 level에서 shader/model clone |
| `Client/Public/Weapon_LanceMaster.h` | component prototype level 전달 |
| `Client/Private/Weapon_LanceMaster.cpp` | 전달받은 level에서 shader/model clone |
| `Client/Default/Client.vcxproj` | 새 Client 파일·shader 등록 |
| `Client/Default/Client.vcxproj.filters` | 물리 폴더 대응 filter 등록 |

### 4-3. 삭제 파일

없음. 기존 `CMapAssetObject`, triangle `CNavigation`, legacy draw 경로를 삭제하지 않는다.

### 4-4. 충돌 예상 파일과 적용 순서

다른 세션이 현재 수정 중인 다음 파일은 그 세션 완료 커밋을 기준으로 수정한다.

```text
Client/Public/MapAssetCatalog.h
Client/Private/MapAssetCatalog.cpp
Client/Public/MapAssetObject.h
Client/Private/MapAssetObject.cpp
Client/Public/MapTool.h
Client/Private/MapTool.cpp
Engine/Private/Renderer.cpp
Client/Bin/ShaderFiles/*.hlsl
Engine/Bin/ShaderFiles/*.hlsl
```

이 계획의 새 코드가 현재 미커밋 내용을 덮어쓰지 않게 별도 브랜치/worktree를 만든 뒤 동적 환경 완료 커밋을 merge 또는 rebase한다.

---

## 5. 파일별 전체 구현 코드

이 절은 사용자가 순서대로 복사해 반영할 최종 코드다. 새 파일은 전체 내용을, 기존
파일은 최종 교체 선언·함수·데이터 블록 전체를 싣는다. 실제 소스는 이 문서 작성으로
수정하지 않는다.

### 5-0. Release 공통 장면 생명주기와 Deploy 책임 분리

#### `Client/Public/DeployPlacementDocument.h` 전체 내용

```cpp
#pragma once

#include "Client_Defines.h"
#include "DeployPropCatalog.h"

#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CDeployPlacementDocument final
{
public:
    static constexpr uint32_t MAX_PLACEMENT_COUNT = 4096;

    static bool_t Read_Default(
        const std::string& expectedAreaId,
        const CDeployPropCatalog& catalog,
        std::vector<DEPLOY_PROP_PLACEMENT>& outPlacements,
        std::string& outStatus);

    static bool_t Read(
        const std::filesystem::path& path,
        const std::string& expectedAreaId,
        const CDeployPropCatalog& catalog,
        std::vector<DEPLOY_PROP_PLACEMENT>& outPlacements,
        std::string& outStatus);

    static bool_t Is_Valid(
        const DEPLOY_PROP_PLACEMENT& placement,
        const CDeployPropCatalog& catalog);
};

NS_END
```

#### `Client/Private/DeployPlacementDocument.cpp` 전체 내용

```cpp
#include "DeployPlacementDocument.h"

#include "MapAssetCatalog.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <unordered_set>

namespace
{
    constexpr const char* PLACEMENT_MAGIC =
        "LOSTARK_DEPLOY_PROP_PLACEMENTS";
    constexpr uint32_t PLACEMENT_VERSION = 1;
}

bool_t CDeployPlacementDocument::Read_Default(
    const std::string& expectedAreaId,
    const CDeployPropCatalog& catalog,
    std::vector<DEPLOY_PROP_PLACEMENT>& outPlacements,
    std::string& outStatus)
{
    if (expectedAreaId.empty())
    {
        outStatus = "DeployProp expected area is empty";
        return false;
    }

    const std::filesystem::path path =
        CMapAssetCatalog::Get_MapDataRoot() /
        (std::filesystem::path(expectedAreaId).wstring() +
            L".deployplacements");
    return Read(path, expectedAreaId, catalog, outPlacements, outStatus);
}

bool_t CDeployPlacementDocument::Read(
    const std::filesystem::path& path,
    const std::string& expectedAreaId,
    const CDeployPropCatalog& catalog,
    std::vector<DEPLOY_PROP_PLACEMENT>& outPlacements,
    std::string& outStatus)
{
    std::ifstream input(path, std::ios::binary);
    std::string magic;
    std::string areaId;
    uint32_t version = {};
    uint32_t count = {};
    if (!input || !(input >> magic >> version >> std::quoted(areaId) >> count) ||
        magic != PLACEMENT_MAGIC || version != PLACEMENT_VERSION ||
        areaId != expectedAreaId || areaId != catalog.Get_AreaId() ||
        0 == count || count > MAX_PLACEMENT_COUNT)
    {
        outStatus = "DeployProp placement header is invalid";
        return false;
    }

    std::vector<DEPLOY_PROP_PLACEMENT> staged;
    std::unordered_set<uint64_t> runtimeIds;
    std::unordered_set<uint32_t> actorIds;
    std::unordered_set<std::string> sourceIds;
    staged.reserve(count);

    for (uint32_t index = 0; index < count; ++index)
    {
        DEPLOY_PROP_PLACEMENT row{};
        uint32_t destructible = {};
        if (!(input >> row.runtimePlacementId >> row.deployActorId >>
            row.propDefinitionId >> std::quoted(row.sourcePlacementId) >>
            std::quoted(row.assetId) >> row.position.x >> row.position.y >>
            row.position.z >> row.rotationQuaternion.x >>
            row.rotationQuaternion.y >> row.rotationQuaternion.z >>
            row.rotationQuaternion.w >> row.uniformScale >> destructible >>
            row.stateOffActionId >> row.triggerBinaryOccurrenceCount))
        {
            outStatus = "DeployProp placement row is truncated at " +
                std::to_string(index);
            return false;
        }

        if (destructible > 1)
        {
            outStatus = "DeployProp destructible flag is invalid at " +
                std::to_string(index);
            return false;
        }
        row.destructible = 0 != destructible;

        if (!Is_Valid(row, catalog) ||
            !runtimeIds.insert(row.runtimePlacementId).second ||
            !actorIds.insert(row.deployActorId).second ||
            !sourceIds.insert(row.sourcePlacementId).second)
        {
            outStatus = "DeployProp placement validation failed at " +
                std::to_string(index);
            return false;
        }

        vector_t rotation = XMLoadFloat4(&row.rotationQuaternion);
        rotation = XMQuaternionNormalize(rotation);
        if (XMVectorGetW(rotation) < 0.f)
            rotation = XMVectorNegate(rotation);
        XMStoreFloat4(&row.rotationQuaternion, rotation);
        staged.push_back(std::move(row));
    }

    std::string trailing;
    if (input >> trailing)
    {
        outStatus = "DeployProp placements contain trailing data";
        return false;
    }

    outPlacements = std::move(staged);
    outStatus = "DeployProp placements ready: " +
        std::to_string(outPlacements.size());
    return true;
}

bool_t CDeployPlacementDocument::Is_Valid(
    const DEPLOY_PROP_PLACEMENT& placement,
    const CDeployPropCatalog& catalog)
{
    const f32_t quaternionLength = XMVectorGetX(XMVector4Length(
        XMLoadFloat4(&placement.rotationQuaternion)));
    return 0 != placement.runtimePlacementId &&
        0 != placement.deployActorId &&
        0 != placement.propDefinitionId &&
        !placement.sourcePlacementId.empty() &&
        placement.sourcePlacementId.size() <= 256 &&
        nullptr != catalog.Find(placement.assetId) &&
        std::isfinite(placement.position.x) &&
        std::isfinite(placement.position.y) &&
        std::isfinite(placement.position.z) &&
        std::isfinite(quaternionLength) && quaternionLength > 0.000001f &&
        std::isfinite(placement.uniformScale) &&
        placement.uniformScale > 0.000001f;
}
```

#### `Client/Public/DeployPropCatalog.h` 전체 교체 내용

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>
#include <vector>

NS_BEGIN(Client)

enum class DEPLOY_PROP_MODEL_KIND
{
    STATIC,
    ANIM,
};

enum class DEPLOY_PROP_STATE
{
    INTACT,
    FRACTURED,
    DESPAWNED,
};

struct DEPLOY_PROP_ASSET_ENTRY
{
    std::string id;
    std::string label;
    std::string evidence;
    DEPLOY_PROP_MODEL_KIND kind = DEPLOY_PROP_MODEL_KIND::STATIC;
    std::filesystem::path intactRelativePath;
    std::filesystem::path intactResolvedPath;
    std::wstring intactPrototypeTag;
    std::filesystem::path fracturedRelativePath;
    std::filesystem::path fracturedResolvedPath;
    std::wstring fracturedPrototypeTag;
};

struct DEPLOY_PROP_PLACEMENT
{
    uint64_t runtimePlacementId = {};
    uint32_t deployActorId = {};
    uint32_t propDefinitionId = {};
    std::string sourcePlacementId;
    std::string assetId;
    float3_t position = {};
    float4_t rotationQuaternion = float4_t(0.f, 0.f, 0.f, 1.f);
    f32_t uniformScale = 1.f;
    bool_t destructible = false;
    uint32_t stateOffActionId = {};
    uint32_t triggerBinaryOccurrenceCount = {};
};

class CDeployPropCatalog final
{
public:
    bool_t Load_Default(const std::string& expectedAreaId);
    bool_t Load(
        const std::filesystem::path& catalogPath,
        const std::string& expectedAreaId);

    const DEPLOY_PROP_ASSET_ENTRY* Find(const std::string& assetId) const;
    const std::vector<DEPLOY_PROP_ASSET_ENTRY>& Get_Assets() const
    { return m_Assets; }
    const std::string& Get_AreaId() const { return m_AreaId; }
    const std::string& Get_Status() const { return m_Status; }
    bool_t Is_Ready() const { return m_bReady; }

private:
    std::vector<DEPLOY_PROP_ASSET_ENTRY> m_Assets;
    std::string m_AreaId;
    std::string m_Status = "DeployProp catalog not loaded";
    bool_t m_bReady = false;
};

NS_END
```

#### `Client/Private/DeployPropCatalog.cpp` 전체 교체 내용

```cpp
#include "DeployPropCatalog.h"

#include "MapAssetCatalog.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <unordered_set>

namespace
{
    constexpr const char* CATALOG_MAGIC =
        "LOSTARK_DEPLOY_PROP_CATALOG";
    constexpr uint32_t CATALOG_VERSION = 1;
    constexpr uint32_t MAX_ASSET_COUNT = 64;

    bool_t IsInsideRoot(
        const std::filesystem::path& root,
        const std::filesystem::path& candidate)
    {
        std::error_code error;
        const std::filesystem::path relative =
            std::filesystem::relative(candidate, root, error);
        if (error || relative.empty() || relative.is_absolute())
            return false;
        const auto first = relative.begin();
        return first != relative.end() && *first != L"..";
    }
}

bool_t CDeployPropCatalog::Load_Default(
    const std::string& expectedAreaId)
{
    if (expectedAreaId.empty())
    {
        m_Status = "DeployProp expected area is empty";
        return false;
    }
    const std::filesystem::path root =
        CMapAssetCatalog::Get_MapDataRoot();
    return Load(root /
        (std::filesystem::path(expectedAreaId).wstring() +
            L".deployassets"), expectedAreaId);
}

bool_t CDeployPropCatalog::Load(
    const std::filesystem::path& catalogPath,
    const std::string& expectedAreaId)
{
    std::ifstream input(catalogPath, std::ios::binary);
    std::string magic;
    std::string areaId;
    uint32_t version = {};
    uint32_t count = {};
    if (!input || !(input >> magic >> version >>
        std::quoted(areaId) >> count) ||
        magic != CATALOG_MAGIC || version != CATALOG_VERSION ||
        areaId != expectedAreaId || 0 == count ||
        count > MAX_ASSET_COUNT)
    {
        m_Status = "DeployProp catalog header is invalid";
        return false;
    }

    const std::filesystem::path assetRoot = CRuntimeAssetRoot::Get();
    std::vector<DEPLOY_PROP_ASSET_ENTRY> staged;
    std::unordered_set<std::string> assetIds;
    std::unordered_set<std::wstring> prototypeTags;
    staged.reserve(count);
    for (uint32_t index = 0; index < count; ++index)
    {
        DEPLOY_PROP_ASSET_ENTRY entry{};
        std::string kind;
        std::string intactPath;
        std::string intactPrototype;
        std::string fracturedPath;
        std::string fracturedPrototype;
        if (!(input >> std::quoted(entry.id) >> kind >>
            std::quoted(entry.label) >> std::quoted(intactPath) >>
            std::quoted(intactPrototype) >> std::quoted(fracturedPath) >>
            std::quoted(fracturedPrototype) >> std::quoted(entry.evidence)))
        {
            m_Status = "DeployProp catalog row is truncated at " +
                std::to_string(index);
            return false;
        }
        if (kind == "STATIC")
            entry.kind = DEPLOY_PROP_MODEL_KIND::STATIC;
        else if (kind == "ANIM")
            entry.kind = DEPLOY_PROP_MODEL_KIND::ANIM;
        else
        {
            m_Status = "DeployProp catalog kind is invalid";
            return false;
        }

        entry.intactRelativePath =
            std::filesystem::path(intactPath).lexically_normal();
        entry.intactResolvedPath =
            CRuntimeAssetRoot::Resolve(entry.intactRelativePath);
        entry.intactPrototypeTag.assign(
            intactPrototype.begin(), intactPrototype.end());
        entry.fracturedRelativePath =
            std::filesystem::path(fracturedPath).lexically_normal();
        entry.fracturedPrototypeTag.assign(
            fracturedPrototype.begin(), fracturedPrototype.end());
        if (!fracturedPath.empty())
            entry.fracturedResolvedPath =
                CRuntimeAssetRoot::Resolve(entry.fracturedRelativePath);

        const bool_t staticModel =
            DEPLOY_PROP_MODEL_KIND::STATIC == entry.kind;
        if (entry.id.empty() || entry.label.empty() ||
            entry.evidence.empty() ||
            entry.intactRelativePath.is_absolute() ||
            entry.intactRelativePath.extension() != L".wmodel" ||
            entry.intactPrototypeTag.empty() ||
            !IsInsideRoot(assetRoot, entry.intactResolvedPath) ||
            !std::filesystem::is_regular_file(entry.intactResolvedPath) ||
            (staticModel &&
                (entry.fracturedRelativePath.is_absolute() ||
                 entry.fracturedRelativePath.extension() != L".wmodel" ||
                 entry.fracturedPrototypeTag.empty() ||
                 !IsInsideRoot(assetRoot, entry.fracturedResolvedPath) ||
                 !std::filesystem::is_regular_file(
                    entry.fracturedResolvedPath))) ||
            (!staticModel &&
                (!fracturedPath.empty() || !fracturedPrototype.empty())) ||
            !assetIds.insert(entry.id).second ||
            !prototypeTags.insert(entry.intactPrototypeTag).second ||
            (staticModel && !prototypeTags.insert(
                entry.fracturedPrototypeTag).second))
        {
            m_Status = "DeployProp catalog validation failed for " + entry.id;
            return false;
        }
        staged.push_back(std::move(entry));
    }
    std::string trailing;
    if (input >> trailing)
    {
        m_Status = "DeployProp catalog contains trailing data";
        return false;
    }

    m_Assets = std::move(staged);
    m_AreaId = std::move(areaId);
    m_bReady = true;
    m_Status = "DeployProp catalog ready: " +
        std::to_string(m_Assets.size()) + " assets";
    return true;
}

const DEPLOY_PROP_ASSET_ENTRY* CDeployPropCatalog::Find(
    const std::string& assetId) const
{
    const auto iter = std::find_if(
        m_Assets.begin(), m_Assets.end(),
        [&](const DEPLOY_PROP_ASSET_ENTRY& entry)
        { return entry.id == assetId; });
    return iter == m_Assets.end() ? nullptr : &*iter;
}
```

#### `Client/Public/DeployPropRuntime.h` 전체 내용

```cpp
#pragma once

#include "Client_Defines.h"
#include "DeployPlacementDocument.h"
#include "DeployPropObject.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CDeployPropRuntime final
{
private:
    struct RUNTIME_ENTRY final
    {
        DEPLOY_PROP_PLACEMENT placement;
        shared_ptr<CDeployPropObject> object;
    };

public:
    ~CDeployPropRuntime();

    HRESULT Load(const std::string& areaId);
    void Unload(bool_t removeFromObjectManager = true);

    void Set_GlobalState(DEPLOY_PROP_STATE state);
    DEPLOY_PROP_STATE Get_GlobalState() const { return m_GlobalState; }

    size_t Get_Count() const { return m_Entries.size(); }
    size_t Get_BindPoseOnlyCount() const;
    const std::string& Get_Status() const { return m_Status; }
    bool_t Is_Loaded() const { return m_bLoaded; }

private:
    static DEPLOY_PROP_STATE Resolve_State(
        const DEPLOY_PROP_PLACEMENT& placement,
        DEPLOY_PROP_STATE globalState);
    static void Remove_Staged(std::vector<RUNTIME_ENTRY>& entries);

private:
    CDeployPropCatalog m_Catalog;
    std::vector<DEPLOY_PROP_PLACEMENT> m_Placements;
    std::vector<RUNTIME_ENTRY> m_Entries;
    DEPLOY_PROP_STATE m_GlobalState = DEPLOY_PROP_STATE::INTACT;
    std::string m_Status = "DeployProp runtime not loaded";
    bool_t m_bLoaded = false;
};

NS_END
```

#### `Client/Private/DeployPropRuntime.cpp` 전체 내용

```cpp
#include "DeployPropRuntime.h"

#include "GameInstance.h"

#include <algorithm>

namespace
{
    constexpr const wchar_t* DEPLOY_PROTOTYPE =
        TEXT("Prototype_GameObject_DeployProp");
    constexpr const wchar_t* DEPLOY_LAYER = TEXT("Layer_DeployProps");
}

CDeployPropRuntime::~CDeployPropRuntime()
{
    Unload();
}

HRESULT CDeployPropRuntime::Load(const std::string& areaId)
{
    CDeployPropCatalog stagedCatalog;
    if (!stagedCatalog.Load_Default(areaId))
    {
        m_Status = stagedCatalog.Get_Status();
        return E_FAIL;
    }

    std::vector<DEPLOY_PROP_PLACEMENT> stagedPlacements;
    std::string documentStatus;
    if (!CDeployPlacementDocument::Read_Default(
        areaId, stagedCatalog, stagedPlacements, documentStatus))
    {
        m_Status = documentStatus;
        return E_FAIL;
    }

    std::vector<RUNTIME_ENTRY> stagedEntries;
    stagedEntries.reserve(stagedPlacements.size());
    for (const DEPLOY_PROP_PLACEMENT& placement : stagedPlacements)
    {
        const DEPLOY_PROP_ASSET_ENTRY* asset =
            stagedCatalog.Find(placement.assetId);
        if (nullptr == asset)
        {
            Remove_Staged(stagedEntries);
            m_Status = "DeployProp stage lost asset " + placement.assetId;
            return E_FAIL;
        }

        CDeployPropObject::DEPLOY_PROP_DESC desc{};
        desc.placement = placement;
        desc.modelKind = asset->kind;
        desc.intactPrototypeTag = asset->intactPrototypeTag;
        desc.fracturedPrototypeTag = asset->fracturedPrototypeTag;

        shared_ptr<CGameObject> gameObject;
        if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
            ETOUI(LEVEL::ASSET_TEST), DEPLOY_PROTOTYPE,
            ETOUI(LEVEL::ASSET_TEST), DEPLOY_LAYER,
            &desc, &gameObject)))
        {
            Remove_Staged(stagedEntries);
            m_Status = "DeployProp stage rolled back at " +
                placement.sourcePlacementId;
            return E_FAIL;
        }

        shared_ptr<CDeployPropObject> object =
            dynamic_pointer_cast<CDeployPropObject>(gameObject);
        if (nullptr == object)
        {
            CGameInstance::Get().Remove_GameObject_from_Layer(
                ETOUI(LEVEL::ASSET_TEST), DEPLOY_LAYER, gameObject);
            Remove_Staged(stagedEntries);
            m_Status = "DeployProp clone type mismatch";
            return E_FAIL;
        }
        stagedEntries.push_back({ placement, std::move(object) });
    }

    Unload();
    m_Catalog = std::move(stagedCatalog);
    m_Placements = std::move(stagedPlacements);
    m_Entries = std::move(stagedEntries);
    m_GlobalState = DEPLOY_PROP_STATE::INTACT;
    m_bLoaded = true;
    m_Status = "DeployProp runtime ready: " +
        std::to_string(m_Entries.size()) + " objects";
    return S_OK;
}

void CDeployPropRuntime::Unload(bool_t removeFromObjectManager)
{
    if (removeFromObjectManager)
        Remove_Staged(m_Entries);
    else
        m_Entries.clear();
    m_Placements.clear();
    m_Catalog = CDeployPropCatalog{};
    m_GlobalState = DEPLOY_PROP_STATE::INTACT;
    m_bLoaded = false;
}

void CDeployPropRuntime::Set_GlobalState(DEPLOY_PROP_STATE state)
{
    m_GlobalState = state;
    for (RUNTIME_ENTRY& entry : m_Entries)
        entry.object->Set_State(Resolve_State(entry.placement, state));
}

size_t CDeployPropRuntime::Get_BindPoseOnlyCount() const
{
    return static_cast<size_t>(std::count_if(
        m_Entries.begin(), m_Entries.end(),
        [](const RUNTIME_ENTRY& entry)
        {
            return entry.object->Is_AnimBindPoseOnly();
        }));
}

DEPLOY_PROP_STATE CDeployPropRuntime::Resolve_State(
    const DEPLOY_PROP_PLACEMENT& placement,
    DEPLOY_PROP_STATE globalState)
{
    if (DEPLOY_PROP_STATE::DESPAWNED == globalState)
        return DEPLOY_PROP_STATE::DESPAWNED;
    if (DEPLOY_PROP_STATE::FRACTURED == globalState &&
        !placement.destructible)
        return DEPLOY_PROP_STATE::INTACT;
    return globalState;
}

void CDeployPropRuntime::Remove_Staged(
    std::vector<RUNTIME_ENTRY>& entries)
{
    for (const RUNTIME_ENTRY& entry : entries)
    {
        if (nullptr != entry.object)
            CGameInstance::Get().Remove_GameObject_from_Layer(
                ETOUI(LEVEL::ASSET_TEST), DEPLOY_LAYER,
                static_pointer_cast<CGameObject>(entry.object));
    }
    entries.clear();
}
```

#### `Client/Public/MapSceneRuntime.h` 전체 내용

```cpp
#pragma once

#include "Client_Defines.h"
#include "DeployPropRuntime.h"
#include "MapAssetCatalog.h"
#include "MapPlacementDocument.h"

#include <memory>
#include <string>
#include <vector>

NS_BEGIN(Client)

class CMapAssetObject;

enum class MAP_ENVIRONMENT_PHASE
{
    BASELINE,
    SPACEHOLE,
    CHAOS_GATE,
};

class CMapSceneRuntime final
{
private:
    struct RUNTIME_ENTRY final
    {
        MAP_PLACEMENT_RECORD record;
        std::wstring layerTag;
        shared_ptr<CMapAssetObject> object;
    };

public:
    ~CMapSceneRuntime();

    HRESULT Synchronize_Level(uint32_t currentLevelId);
    HRESULT Reload();
    void Unload(bool_t removeFromObjectManager = true);

    HRESULT Add_Placement(const MAP_PLACEMENT_RECORD& record);
    HRESULT Remove_Placement(uint64_t placementId);
    HRESULT Update_Placement(
        uint64_t placementId,
        const float3_t& position,
        const float4_t& rotationQuaternion,
        const float3_t& signedScale);
    HRESULT Set_PlacementVisible(uint64_t placementId, bool_t visible);
    HRESULT Save_Placements();

    void Set_DeployPhase(DEPLOY_PROP_STATE state);
    void Set_EnvironmentPhase(MAP_ENVIRONMENT_PHASE phase);

    const CMapAssetCatalog& Get_Catalog() const { return m_Catalog; }
    const std::vector<MAP_PLACEMENT_RECORD>& Get_PlacementRecords() const
    { return m_RecordView; }
    const std::string& Get_Status() const { return m_Status; }
    size_t Get_DeployCount() const;
    bool_t Is_Loaded() const { return m_bLoaded; }

private:
    static std::wstring Make_LayerTag(const std::string& sourceLevel);
    static HRESULT Create_Placement(
        const CMapAssetCatalog& catalog,
        const MAP_PLACEMENT_RECORD& record,
        RUNTIME_ENTRY& outEntry);
    static void Remove_Entries(
        std::vector<RUNTIME_ENTRY>& entries,
        bool_t removeFromObjectManager);

    RUNTIME_ENTRY* Find_Entry(uint64_t placementId);
    void Rebuild_RecordView();
    void Apply_EnvironmentPhase();

private:
    CMapAssetCatalog m_Catalog;
    std::vector<RUNTIME_ENTRY> m_Entries;
    std::vector<MAP_PLACEMENT_RECORD> m_RecordView;
    std::unique_ptr<CDeployPropRuntime> m_pDeployRuntime;
    MAP_ENVIRONMENT_PHASE m_EnvironmentPhase =
        MAP_ENVIRONMENT_PHASE::BASELINE;
    uint32_t m_CurrentLevelId = UINT32_MAX;
    std::string m_Status = "Map scene runtime not loaded";
    bool_t m_bLoaded = false;
    bool_t m_bDirty = false;
};

NS_END
```

#### `Client/Private/MapSceneRuntime.cpp` 전체 내용

```cpp
#include "MapSceneRuntime.h"

#include "GameInstance.h"
#include "MapAssetObject.h"

#include <algorithm>

namespace
{
    constexpr const wchar_t* MAP_PROTOTYPE =
        TEXT("Prototype_GameObject_MapAsset");
}

CMapSceneRuntime::~CMapSceneRuntime()
{
    Unload();
}

HRESULT CMapSceneRuntime::Synchronize_Level(uint32_t currentLevelId)
{
    if (m_CurrentLevelId == currentLevelId)
        return S_FALSE;

    const bool_t wasAssetTest =
        ETOUI(LEVEL::ASSET_TEST) == m_CurrentLevelId;
    const bool_t isAssetTest =
        ETOUI(LEVEL::ASSET_TEST) == currentLevelId;
    m_CurrentLevelId = currentLevelId;

    if (!isAssetTest)
    {
        if (wasAssetTest)
            Unload(false);
        m_Status = "Map scene waits for ASSET_TEST";
        return S_OK;
    }

    return Reload();
}

HRESULT CMapSceneRuntime::Reload()
{
    if (ETOUI(LEVEL::ASSET_TEST) != m_CurrentLevelId)
        return E_UNEXPECTED;

    CMapAssetCatalog stagedCatalog;
    if (!stagedCatalog.Load_Default())
    {
        m_Status = stagedCatalog.Get_Status();
        return E_FAIL;
    }

    std::vector<MAP_PLACEMENT_RECORD> document;
    std::string documentStatus;
    if (!CMapPlacementDocument::Read(
        stagedCatalog.Get_PlacementPath(), stagedCatalog,
        document, documentStatus))
    {
        m_Status = documentStatus;
        return E_FAIL;
    }

    std::vector<RUNTIME_ENTRY> stagedEntries;
    stagedEntries.reserve(document.size());
    for (const MAP_PLACEMENT_RECORD& record : document)
    {
        RUNTIME_ENTRY entry{};
        if (FAILED(Create_Placement(stagedCatalog, record, entry)))
        {
            Remove_Entries(stagedEntries, true);
            m_Status = "Map scene stage rolled back at " +
                record.sourcePlacementId;
            return E_FAIL;
        }
        stagedEntries.push_back(std::move(entry));
    }

    auto stagedDeploy = std::make_unique<CDeployPropRuntime>();
    if (FAILED(stagedDeploy->Load(stagedCatalog.Get_AreaId())))
    {
        m_Status = stagedDeploy->Get_Status();
        stagedDeploy->Unload();
        Remove_Entries(stagedEntries, true);
        return E_FAIL;
    }

    if (nullptr != m_pDeployRuntime)
        m_pDeployRuntime->Unload();
    Remove_Entries(m_Entries, true);

    m_Catalog = std::move(stagedCatalog);
    m_Entries = std::move(stagedEntries);
    m_pDeployRuntime = std::move(stagedDeploy);
    m_EnvironmentPhase = MAP_ENVIRONMENT_PHASE::BASELINE;
    m_bLoaded = true;
    m_bDirty = false;
    Rebuild_RecordView();
    Apply_EnvironmentPhase();

    m_Status = "Map scene ready: " +
        std::to_string(m_Entries.size()) + " static + " +
        std::to_string(m_pDeployRuntime->Get_Count()) + " deploy";
    return S_OK;
}

void CMapSceneRuntime::Unload(bool_t removeFromObjectManager)
{
    if (nullptr != m_pDeployRuntime)
        m_pDeployRuntime->Unload(removeFromObjectManager);
    m_pDeployRuntime.reset();
    Remove_Entries(m_Entries, removeFromObjectManager);
    m_RecordView.clear();
    m_Catalog = CMapAssetCatalog{};
    m_EnvironmentPhase = MAP_ENVIRONMENT_PHASE::BASELINE;
    m_bLoaded = false;
    m_bDirty = false;
}

HRESULT CMapSceneRuntime::Add_Placement(
    const MAP_PLACEMENT_RECORD& record)
{
    if (!m_bLoaded || nullptr != Find_Entry(record.placementId) ||
        !CMapPlacementDocument::Is_Valid(record, m_Catalog))
        return E_INVALIDARG;

    RUNTIME_ENTRY entry{};
    if (FAILED(Create_Placement(m_Catalog, record, entry)))
        return E_FAIL;
    m_Entries.push_back(std::move(entry));
    m_bDirty = true;
    Rebuild_RecordView();
    Apply_EnvironmentPhase();
    return S_OK;
}

HRESULT CMapSceneRuntime::Remove_Placement(uint64_t placementId)
{
    const auto iter = std::find_if(
        m_Entries.begin(), m_Entries.end(),
        [placementId](const RUNTIME_ENTRY& entry)
        { return entry.record.placementId == placementId; });
    if (iter == m_Entries.end())
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

    if (FAILED(CGameInstance::Get().Remove_GameObject_from_Layer(
        ETOUI(LEVEL::ASSET_TEST), iter->layerTag,
        static_pointer_cast<CGameObject>(iter->object))))
        return E_FAIL;

    m_Entries.erase(iter);
    m_bDirty = true;
    Rebuild_RecordView();
    return S_OK;
}

HRESULT CMapSceneRuntime::Update_Placement(
    uint64_t placementId,
    const float3_t& position,
    const float4_t& rotationQuaternion,
    const float3_t& signedScale)
{
    RUNTIME_ENTRY* entry = Find_Entry(placementId);
    if (nullptr == entry)
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

    MAP_PLACEMENT_RECORD staged = entry->record;
    staged.position = position;
    staged.rotationQuaternion = rotationQuaternion;
    staged.signedScale = signedScale;
    if (!CMapPlacementDocument::Is_Valid(staged, m_Catalog))
        return E_INVALIDARG;

    entry->record = staged;
    entry->object->Set_PlacementTransform(
        position, rotationQuaternion, signedScale);
    m_bDirty = true;
    Rebuild_RecordView();
    return S_OK;
}

HRESULT CMapSceneRuntime::Set_PlacementVisible(
    uint64_t placementId, bool_t visible)
{
    RUNTIME_ENTRY* entry = Find_Entry(placementId);
    if (nullptr == entry)
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    entry->record.visible = visible;
    entry->object->Set_Visible(visible);
    m_bDirty = true;
    Rebuild_RecordView();
    Apply_EnvironmentPhase();
    return S_OK;
}

HRESULT CMapSceneRuntime::Save_Placements()
{
    if (!m_bLoaded)
        return E_UNEXPECTED;
    Rebuild_RecordView();
    if (!CMapPlacementDocument::Write(
        m_Catalog.Get_PlacementPath(), m_Catalog.Get_AreaId(),
        m_RecordView, m_Catalog, m_Status))
        return E_FAIL;
    m_bDirty = false;
    return S_OK;
}

void CMapSceneRuntime::Set_DeployPhase(DEPLOY_PROP_STATE state)
{
    if (nullptr != m_pDeployRuntime)
        m_pDeployRuntime->Set_GlobalState(state);
}

void CMapSceneRuntime::Set_EnvironmentPhase(
    MAP_ENVIRONMENT_PHASE phase)
{
    m_EnvironmentPhase = phase;
    Apply_EnvironmentPhase();
}

size_t CMapSceneRuntime::Get_DeployCount() const
{
    return nullptr == m_pDeployRuntime ? 0 : m_pDeployRuntime->Get_Count();
}

std::wstring CMapSceneRuntime::Make_LayerTag(
    const std::string& sourceLevel)
{
    std::wstring result = L"Layer_MapAsset_";
    result.append(sourceLevel.begin(), sourceLevel.end());
    return result;
}

HRESULT CMapSceneRuntime::Create_Placement(
    const CMapAssetCatalog& catalog,
    const MAP_PLACEMENT_RECORD& record,
    RUNTIME_ENTRY& outEntry)
{
    const MAP_ASSET_ENTRY* asset = catalog.Find(record.assetId);
    if (nullptr == asset ||
        !CMapPlacementDocument::Is_Valid(record, catalog))
        return E_INVALIDARG;

    CMapAssetObject::MAP_ASSET_DESC desc{};
    desc.placementId = record.placementId;
    desc.assetId = asset->id;
    desc.modelPrototypeTag = asset->prototypeTag;
    desc.position = record.position;
    desc.rotationQuaternion = record.rotationQuaternion;
    desc.signedScale = record.signedScale;
    desc.applyBottomCenter =
        MAP_ASSET_ANCHOR::BOTTOM_CENTER == asset->anchor;
    desc.visible = record.visible;
    desc.renderProfile = asset->renderProfile;

    const std::wstring layerTag = Make_LayerTag(record.sourceLevel);
    shared_ptr<CGameObject> gameObject;
    if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
        ETOUI(LEVEL::ASSET_TEST), MAP_PROTOTYPE,
        ETOUI(LEVEL::ASSET_TEST), layerTag,
        &desc, &gameObject)))
        return E_FAIL;

    shared_ptr<CMapAssetObject> object =
        dynamic_pointer_cast<CMapAssetObject>(gameObject);
    if (nullptr == object)
    {
        CGameInstance::Get().Remove_GameObject_from_Layer(
            ETOUI(LEVEL::ASSET_TEST), layerTag, gameObject);
        return E_FAIL;
    }

    outEntry.record = record;
    outEntry.layerTag = layerTag;
    outEntry.object = std::move(object);
    return S_OK;
}

void CMapSceneRuntime::Remove_Entries(
    std::vector<RUNTIME_ENTRY>& entries,
    bool_t removeFromObjectManager)
{
    if (removeFromObjectManager)
    {
        for (const RUNTIME_ENTRY& entry : entries)
        {
            if (nullptr != entry.object)
                CGameInstance::Get().Remove_GameObject_from_Layer(
                    ETOUI(LEVEL::ASSET_TEST), entry.layerTag,
                    static_pointer_cast<CGameObject>(entry.object));
        }
    }
    entries.clear();
}

CMapSceneRuntime::RUNTIME_ENTRY* CMapSceneRuntime::Find_Entry(
    uint64_t placementId)
{
    const auto iter = std::find_if(
        m_Entries.begin(), m_Entries.end(),
        [placementId](const RUNTIME_ENTRY& entry)
        { return entry.record.placementId == placementId; });
    return iter == m_Entries.end() ? nullptr : &*iter;
}

void CMapSceneRuntime::Rebuild_RecordView()
{
    m_RecordView.clear();
    m_RecordView.reserve(m_Entries.size());
    for (const RUNTIME_ENTRY& entry : m_Entries)
        m_RecordView.push_back(entry.record);
}

void CMapSceneRuntime::Apply_EnvironmentPhase()
{
    for (RUNTIME_ENTRY& entry : m_Entries)
    {
        bool_t visible = entry.record.visible;
        if (entry.record.sourceLevel == "VALTAN_PHASE_SPACEHOLE")
            visible = MAP_ENVIRONMENT_PHASE::BASELINE != m_EnvironmentPhase;
        else if (entry.record.sourceLevel == "VALTAN_PHASE_CHAOSGATE")
            visible = MAP_ENVIRONMENT_PHASE::CHAOS_GATE == m_EnvironmentPhase;
        entry.object->Set_Visible(visible);
    }
}
```

#### `Client/Public/MainApp.h` 최종 교체 선언 블록

```cpp
NS_BEGIN(Client)

class CMapSceneRuntime;
class CMapTool;
class CProfilerPanel;

class CMainApp final
{
    // 기존 생성자/public API는 유지한다.
private:
    ComPtr<ID3D11Device> m_pDevice = { nullptr };
    ComPtr<ID3D11DeviceContext> m_pContext = { nullptr };
    std::unique_ptr<CMapSceneRuntime> m_pMapSceneRuntime = { nullptr };

#ifdef _DEBUG
    std::unique_ptr<Engine::CImGuiLayer> m_pImGuiLayer = { nullptr };
    std::unique_ptr<CMapTool> m_pMapTool = { nullptr };
    std::unique_ptr<CProfilerPanel> m_pProfilerPanel = { nullptr };
    std::unique_ptr<CEffect_Tool> m_pEffectTool = { nullptr };
    std::unique_ptr<CAnimation_Tool> m_pAnimationTool = { nullptr };
    std::unique_ptr<CHUDLayoutTool> m_pHUDLayoutTool = { nullptr };
    bool_t m_bF1Down = false;
    bool_t m_bF4Down = false;
#endif

    // 기존 private 함수 선언을 유지한다.
};

NS_END
```

#### `CMainApp::Initialize()` 최종 교체 함수

```cpp
HRESULT CMainApp::Initialize()
{
    ENGINE_DESC engineDesc{};
    engineDesc.hInstance = g_hInst;
    engineDesc.hWnd = g_hWnd;
    engineDesc.eWinMode = WINMODE::WIN;
    engineDesc.iNumLevels = ETOUI(LEVEL::END);
    engineDesc.iWinSizeX = g_iWinSizeX;
    engineDesc.iWinSizeY = g_iWinSizeY;

    if (FAILED(CGameInstance::Get().Initialize_Engine(
        engineDesc, m_pDevice, m_pContext)))
        return E_FAIL;

    m_pMapSceneRuntime = std::make_unique<CMapSceneRuntime>();

#ifdef _DEBUG
    if (FAILED(ReadyDebugTools()))
        return E_FAIL;
#endif
    if (FAILED(Ready_Gara()) || FAILED(Ready_Fonts()) ||
        FAILED(Ready_Prototype_For_Static()) ||
        FAILED(Start_Level(LEVEL::LOGO)))
        return E_FAIL;
    return S_OK;
}
```

#### `CMainApp::Update()` 최종 교체 함수

```cpp
void CMainApp::Update(f32_t fTimeDelta)
{
#ifdef _DEBUG
    UpdateDebugToolShortcut();
    if (nullptr != m_pImGuiLayer)
        m_pImGuiLayer->BeginFrame();

    const bool_t mapToolOpen =
        nullptr != m_pMapTool && m_pMapTool->IsOpen();
    const HWND foreground = GetForegroundWindow();
    const bool_t externalToolFocused = mapToolOpen &&
        nullptr != foreground && foreground != g_hWnd &&
        IsWindowOwnedByCurrentProcess(foreground);
    CGameInstance::Get().SetInputBlocked(
        mapToolOpen && nullptr != m_pImGuiLayer &&
            (m_pImGuiLayer->WantsCaptureKeyboard() || externalToolFocused),
        mapToolOpen && nullptr != m_pImGuiLayer &&
            (m_pImGuiLayer->WantsCaptureMouse() || externalToolFocused));
#endif

    CGameInstance::Get().Update_Engine(fTimeDelta);

    if (nullptr != m_pMapSceneRuntime)
    {
        const HRESULT result = m_pMapSceneRuntime->Synchronize_Level(
            CGameInstance::Get().Get_CurrentLevelID());
        if (FAILED(result))
            OutputDebugStringA(m_pMapSceneRuntime->Get_Status().c_str());
    }

#ifdef _DEBUG
    if (nullptr != m_pMapTool)
        m_pMapTool->Update(fTimeDelta);
#endif
}
```

#### `CMainApp::Free()` 최종 교체 함수

```cpp
void CMainApp::Free()
{
#ifdef _DEBUG
    CGameInstance::Get().SetInputBlocked(false, false);
    m_pProfilerPanel.reset();
    m_pAnimationTool.reset();
    m_pEffectTool.reset();
    m_pHUDLayoutTool.reset();
    m_pMapTool.reset();
    if (nullptr != m_pImGuiLayer)
        m_pImGuiLayer->Shutdown();
    m_pImGuiLayer.reset();
#endif

    if (nullptr != m_pMapSceneRuntime)
        m_pMapSceneRuntime->Unload(
            ETOUI(LEVEL::ASSET_TEST) ==
            CGameInstance::Get().Get_CurrentLevelID());
    m_pMapSceneRuntime.reset();
    CGameInstance::Get().Release_Engine();
}
```

`MainApp.cpp`의 include에는 다음을 추가한다.

```cpp
#include "MapSceneRuntime.h"
#include "Profiler.h"
#include "TextureResourceCache.h"
```

### 5-1. `Engine/Public/Profiler.h` 전체 내용

```cpp
#pragma once

#include "Engine_Defines.h"
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

NS_BEGIN(Engine)

enum class EProfilerCounter : uint16_t
{
    DrawCalls,
    InstancedDrawCalls,
    Instances,
    Indices,
    RenderSubmissionsPriority,
    RenderSubmissionsShadow,
    RenderSubmissionsNonBlend,
    RenderSubmissionsBlend,
    MapPlacements,
    MapVisibleInstances,
    MapBatchCount,
    MapFallbackObjects,
    TextureRequests,
    TexturePathHits,
    TextureContentHits,
    TextureUniqueSrvs,
    TextureEstimatedGpuBytes,
    NavigationQueries,
    NavigationExpandedNodes,
    NavigationQueryMicroseconds,
    NavigationPathCells,
    Count
};

struct FProfilerScopeSample final
{
    uint32_t NameId = 0;
    uint32_t Depth = 0;
    uint64_t BeginTick = 0;
    uint64_t EndTick = 0;
};

struct FProfilerFrame final
{
    uint64_t FrameNumber = 0;
    double CpuFrameMs = 0.0;
    double GpuFrameMs = 0.0;
    bool GpuValid = false;
    uint32_t GpuLatencyFrames = 0;
    std::array<uint64_t, static_cast<size_t>(EProfilerCounter::Count)> Counters{};
    std::vector<FProfilerScopeSample> CpuScopes;
    D3D11_QUERY_DATA_PIPELINE_STATISTICS Pipeline{};
};

struct FProfilerCaptureSnapshot final
{
    std::vector<std::string> ScopeNames;
    std::vector<FProfilerFrame> Frames;
    uint64_t DroppedCpuScopes = 0;
    uint64_t DroppedGpuFrames = 0;
};

class ENGINE_DLL CProfiler final
{
public:
    static constexpr uint32_t GPU_QUERY_RING_SIZE = 8;
    static constexpr uint32_t GPU_READ_LATENCY = 4;
    static constexpr size_t MAX_HISTORY_FRAMES = 1200;

public:
    CProfiler() = default;
    ~CProfiler() = default;
    HRESULT Initialize(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context);
    void Begin_Frame();
    void End_Frame();

    void Set_Enabled(bool enabled) noexcept;
    bool Is_Enabled() const noexcept;
    void Reset_History();

    uint32_t Begin_Scope(std::string_view name);
    void End_Scope(uint32_t token) noexcept;

    void Add_Counter(EProfilerCounter counter, uint64_t value = 1) noexcept;
    void Set_Counter(EProfilerCounter counter, uint64_t value) noexcept;

    FProfilerCaptureSnapshot Snapshot() const;

private:
    struct FGpuQuerySlot final
    {
        ComPtr<ID3D11Query> Disjoint;
        ComPtr<ID3D11Query> TimestampBegin;
        ComPtr<ID3D11Query> TimestampEnd;
        ComPtr<ID3D11Query> Pipeline;
        uint64_t FrameNumber = 0;
        bool Pending = false;
    };

    struct FOpenScope final
    {
        uint32_t SampleIndex = 0;
        uint32_t Depth = 0;
    };

private:
    uint64_t Query_Tick() const noexcept;
    uint32_t Intern_Name(std::string_view name);
    bool Create_GpuQueries();
    void Begin_GpuFrame(uint64_t frameNumber);
    void End_GpuFrame(uint64_t frameNumber);
    void Resolve_GpuFrames(uint64_t currentFrame);
    void Commit_CurrentFrame();

private:
    ComPtr<ID3D11Device> m_pDevice;
    ComPtr<ID3D11DeviceContext> m_pContext;
    LARGE_INTEGER m_Frequency{};
    std::atomic_bool m_Enabled = false;
    uint64_t m_FrameNumber = 0;
    uint64_t m_FrameBeginTick = 0;
    FProfilerFrame m_CurrentFrame{};
    std::array<std::atomic_uint64_t, static_cast<size_t>(EProfilerCounter::Count)> m_AtomicCounters{};
    std::vector<FOpenScope> m_OpenScopes;
    std::array<FGpuQuerySlot, GPU_QUERY_RING_SIZE> m_GpuSlots{};
    bool m_GpuQueriesAvailable = false;
    bool m_FrameActive = false;
    mutable std::mutex m_Mutex;
    std::deque<FProfilerFrame> m_History;
    std::vector<std::string> m_ScopeNames;
    std::unordered_map<std::string, uint32_t> m_ScopeNameLookup;
    uint64_t m_DroppedCpuScopes = 0;
    uint64_t m_DroppedGpuFrames = 0;

};

class ENGINE_DLL CProfilerScope final
{
public:
    CProfilerScope(CProfiler* profiler, std::string_view name)
        : m_pProfiler(profiler)
        , m_Token(profiler != nullptr ? profiler->Begin_Scope(name) : UINT32_MAX)
    {
    }

    ~CProfilerScope()
    {
        if (m_pProfiler != nullptr && m_Token != UINT32_MAX)
            m_pProfiler->End_Scope(m_Token);
    }

    CProfilerScope(const CProfilerScope&) = delete;
    CProfilerScope& operator=(const CProfilerScope&) = delete;

private:
    CProfiler* m_pProfiler = nullptr;
    uint32_t m_Token = UINT32_MAX;
};

NS_END
```

#### `Engine/Private/Profiler.cpp` 전체 내용

```cpp
#include "Profiler.h"

#include <algorithm>
#include <limits>

using namespace Engine;

namespace
{
    constexpr size_t MAX_SCOPES_PER_FRAME = 4096;
}

HRESULT CProfiler::Initialize(
    ComPtr<ID3D11Device> device,
    ComPtr<ID3D11DeviceContext> context)
{
    if (nullptr == device || nullptr == context ||
        !QueryPerformanceFrequency(&m_Frequency) ||
        0 == m_Frequency.QuadPart)
        return E_INVALIDARG;

    m_pDevice = std::move(device);
    m_pContext = std::move(context);
    m_GpuQueriesAvailable = Create_GpuQueries();
    return S_OK;
}

void CProfiler::Begin_Frame()
{
    if (!m_Enabled.load(std::memory_order_relaxed))
        return;

    m_FrameActive = true;
    ++m_FrameNumber;
    m_CurrentFrame = {};
    m_CurrentFrame.FrameNumber = m_FrameNumber;
    m_CurrentFrame.GpuLatencyFrames = GPU_READ_LATENCY;
    m_CurrentFrame.CpuScopes.clear();
    m_CurrentFrame.CpuScopes.reserve(128);
    m_OpenScopes.clear();
    m_FrameBeginTick = Query_Tick();
    Begin_GpuFrame(m_FrameNumber);
}

void CProfiler::End_Frame()
{
    if (!m_FrameActive)
        return;
    m_FrameActive = false;

    while (!m_OpenScopes.empty())
    {
        const uint32_t sampleIndex = m_OpenScopes.back().SampleIndex;
        m_OpenScopes.pop_back();
        if (sampleIndex < m_CurrentFrame.CpuScopes.size())
            m_CurrentFrame.CpuScopes[sampleIndex].EndTick = Query_Tick();
    }

    const uint64_t endTick = Query_Tick();
    m_CurrentFrame.CpuFrameMs =
        static_cast<double>(endTick - m_FrameBeginTick) * 1000.0 /
        static_cast<double>(m_Frequency.QuadPart);

    for (size_t index = 0; index < m_AtomicCounters.size(); ++index)
    {
        m_CurrentFrame.Counters[index] =
            m_AtomicCounters[index].exchange(0, std::memory_order_relaxed);
    }

    End_GpuFrame(m_FrameNumber);
    Commit_CurrentFrame();
    Resolve_GpuFrames(m_FrameNumber);
}

void CProfiler::Set_Enabled(bool enabled) noexcept
{
    const bool previous = m_Enabled.exchange(
        enabled, std::memory_order_relaxed);
    if (previous == enabled)
        return;

    for (std::atomic_uint64_t& counter : m_AtomicCounters)
        counter.store(0, std::memory_order_relaxed);
    if (!m_FrameActive)
        m_OpenScopes.clear();
}

bool CProfiler::Is_Enabled() const noexcept
{
    return m_Enabled.load(std::memory_order_relaxed);
}

void CProfiler::Reset_History()
{
    std::lock_guard lock(m_Mutex);
    m_History.clear();
    m_DroppedCpuScopes = 0;
    m_DroppedGpuFrames = 0;
}

uint32_t CProfiler::Begin_Scope(std::string_view name)
{
    if (!m_Enabled.load(std::memory_order_relaxed) || !m_FrameActive)
        return UINT32_MAX;
    if (m_CurrentFrame.CpuScopes.size() >= MAX_SCOPES_PER_FRAME)
    {
        ++m_DroppedCpuScopes;
        return UINT32_MAX;
    }

    FProfilerScopeSample sample{};
    sample.NameId = Intern_Name(name);
    sample.Depth = static_cast<uint32_t>(m_OpenScopes.size());
    sample.BeginTick = Query_Tick();
    sample.EndTick = sample.BeginTick;
    const uint32_t token = static_cast<uint32_t>(
        m_CurrentFrame.CpuScopes.size());
    m_CurrentFrame.CpuScopes.push_back(sample);
    m_OpenScopes.push_back({ token, sample.Depth });
    return token;
}

void CProfiler::End_Scope(uint32_t token) noexcept
{
    if (!m_Enabled.load(std::memory_order_relaxed) ||
        !m_FrameActive ||
        token >= m_CurrentFrame.CpuScopes.size())
        return;

    m_CurrentFrame.CpuScopes[token].EndTick = Query_Tick();
    const auto iter = std::find_if(
        m_OpenScopes.rbegin(), m_OpenScopes.rend(),
        [token](const FOpenScope& scope)
        { return scope.SampleIndex == token; });
    if (iter != m_OpenScopes.rend())
        m_OpenScopes.erase(std::next(iter).base());
}

void CProfiler::Add_Counter(
    EProfilerCounter counter, uint64_t value) noexcept
{
    if (!m_Enabled.load(std::memory_order_relaxed) || !m_FrameActive)
        return;
    const size_t index = static_cast<size_t>(counter);
    if (index < m_AtomicCounters.size())
        m_AtomicCounters[index].fetch_add(value, std::memory_order_relaxed);
}

void CProfiler::Set_Counter(
    EProfilerCounter counter, uint64_t value) noexcept
{
    if (!m_Enabled.load(std::memory_order_relaxed) || !m_FrameActive)
        return;
    const size_t index = static_cast<size_t>(counter);
    if (index < m_AtomicCounters.size())
        m_AtomicCounters[index].store(value, std::memory_order_relaxed);
}

FProfilerCaptureSnapshot CProfiler::Snapshot() const
{
    std::lock_guard lock(m_Mutex);
    FProfilerCaptureSnapshot snapshot{};
    snapshot.ScopeNames = m_ScopeNames;
    snapshot.Frames.assign(m_History.begin(), m_History.end());
    snapshot.DroppedCpuScopes = m_DroppedCpuScopes;
    snapshot.DroppedGpuFrames = m_DroppedGpuFrames;
    return snapshot;
}

uint64_t CProfiler::Query_Tick() const noexcept
{
    LARGE_INTEGER value{};
    QueryPerformanceCounter(&value);
    return static_cast<uint64_t>(value.QuadPart);
}

uint32_t CProfiler::Intern_Name(std::string_view name)
{
    const std::string key(name);
    std::lock_guard lock(m_Mutex);
    const auto found = m_ScopeNameLookup.find(key);
    if (found != m_ScopeNameLookup.end())
        return found->second;
    const uint32_t id = static_cast<uint32_t>(m_ScopeNames.size());
    m_ScopeNames.push_back(key);
    m_ScopeNameLookup.emplace(m_ScopeNames.back(), id);
    return id;
}

bool CProfiler::Create_GpuQueries()
{
    D3D11_QUERY_DESC desc{};
    for (FGpuQuerySlot& slot : m_GpuSlots)
    {
        desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        if (FAILED(m_pDevice->CreateQuery(&desc, &slot.Disjoint)))
            return false;
        desc.Query = D3D11_QUERY_TIMESTAMP;
        if (FAILED(m_pDevice->CreateQuery(&desc, &slot.TimestampBegin)) ||
            FAILED(m_pDevice->CreateQuery(&desc, &slot.TimestampEnd)))
            return false;
        desc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
        if (FAILED(m_pDevice->CreateQuery(&desc, &slot.Pipeline)))
            return false;
    }
    return true;
}

void CProfiler::Begin_GpuFrame(uint64_t frameNumber)
{
    if (!m_GpuQueriesAvailable)
        return;
    FGpuQuerySlot& slot = m_GpuSlots[frameNumber % GPU_QUERY_RING_SIZE];
    if (slot.Pending)
    {
        ++m_DroppedGpuFrames;
        return;
    }
    slot.FrameNumber = frameNumber;
    slot.Pending = true;
    m_pContext->Begin(slot.Disjoint.Get());
    m_pContext->Begin(slot.Pipeline.Get());
    m_pContext->End(slot.TimestampBegin.Get());
}

void CProfiler::End_GpuFrame(uint64_t frameNumber)
{
    if (!m_GpuQueriesAvailable)
        return;
    FGpuQuerySlot& slot = m_GpuSlots[frameNumber % GPU_QUERY_RING_SIZE];
    if (!slot.Pending || slot.FrameNumber != frameNumber)
        return;
    m_pContext->End(slot.TimestampEnd.Get());
    m_pContext->End(slot.Pipeline.Get());
    m_pContext->End(slot.Disjoint.Get());
}

void CProfiler::Resolve_GpuFrames(uint64_t currentFrame)
{
    if (!m_GpuQueriesAvailable || currentFrame <= GPU_READ_LATENCY)
        return;

    constexpr uint32_t flags = D3D11_ASYNC_GETDATA_DONOTFLUSH;
    for (FGpuQuerySlot& slot : m_GpuSlots)
    {
        if (!slot.Pending ||
            currentFrame < slot.FrameNumber + GPU_READ_LATENCY)
            continue;
        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint{};
        uint64_t begin = {};
        uint64_t end = {};
        D3D11_QUERY_DATA_PIPELINE_STATISTICS pipeline{};
        if (S_OK != m_pContext->GetData(
            slot.Disjoint.Get(), &disjoint, sizeof(disjoint), flags) ||
            S_OK != m_pContext->GetData(
            slot.TimestampBegin.Get(), &begin, sizeof(begin), flags) ||
            S_OK != m_pContext->GetData(
            slot.TimestampEnd.Get(), &end, sizeof(end), flags) ||
            S_OK != m_pContext->GetData(
            slot.Pipeline.Get(), &pipeline, sizeof(pipeline), flags))
            continue;

        std::lock_guard lock(m_Mutex);
        const auto frame = std::find_if(
            m_History.begin(), m_History.end(),
            [&slot](const FProfilerFrame& value)
            { return value.FrameNumber == slot.FrameNumber; });
        if (frame != m_History.end())
        {
            frame->GpuLatencyFrames = static_cast<uint32_t>(
                currentFrame - slot.FrameNumber);
            frame->Pipeline = pipeline;
            if (!disjoint.Disjoint && 0 != disjoint.Frequency && end >= begin)
            {
                frame->GpuFrameMs =
                    static_cast<double>(end - begin) * 1000.0 /
                    static_cast<double>(disjoint.Frequency);
                frame->GpuValid = true;
            }
        }
        slot.Pending = false;
    }
}

void CProfiler::Commit_CurrentFrame()
{
    std::lock_guard lock(m_Mutex);
    m_History.push_back(m_CurrentFrame);
    while (m_History.size() > MAX_HISTORY_FRAMES)
        m_History.pop_front();
}
```

#### `Engine/Public/GameInstance.h` 추가 선언·멤버 블록

```cpp
public: /* For.Profiler / Texture cache */
    class CProfiler* Get_Profiler() const { return m_pProfiler.get(); }
    class CTextureResourceCache* Get_TextureResourceCache() const
    { return m_pTextureResourceCache.get(); }

private:
    unique_ptr<class CProfiler> m_pProfiler = { nullptr };
    unique_ptr<class CTextureResourceCache> m_pTextureResourceCache = { nullptr };
```

#### `CGameInstance::Initialize_Engine()`에 추가할 최종 블록

`m_pGraphic_Device`가 `pOutDevice/pOutContext`를 만든 직후 다음을 실행한다.

```cpp
m_pProfiler = std::make_unique<CProfiler>();
if (FAILED(m_pProfiler->Initialize(pOutDevice, pOutContext)))
    return E_FAIL;

m_pTextureResourceCache = std::make_unique<CTextureResourceCache>();
if (FAILED(m_pTextureResourceCache->Initialize(pOutDevice)))
    return E_FAIL;
```

#### `CGameInstance::Release_Engine()`에 추가할 최종 블록

`m_pGraphic_Device->Shutdown()`보다 먼저, 다른 GPU resource owner와 함께 다음 순서로
해제한다.

```cpp
m_pTextureResourceCache.reset();
m_pProfiler.reset();
```

`GameInstance.cpp` include에 다음을 추가한다.

```cpp
#include "Profiler.h"
#include "TextureResourceCache.h"
```

### 5-2. `Engine/Public/TextureResourceCache.h` 전체 내용

```cpp
#pragma once

#include "Engine_Defines.h"
#include <array>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <mutex>
#include <unordered_map>

NS_BEGIN(Engine)

enum class ETextureLoader : uint8_t { DDS, TGA, WIC };
enum class ETextureSemantic : uint8_t { Color, Normal, Mask, Emissive, Unknown };

struct FTextureLoadRequest final
{
    std::filesystem::path Path;
    ETextureLoader Loader = ETextureLoader::WIC;
    ETextureSemantic Semantic = ETextureSemantic::Unknown;
};

struct FTextureCacheStats final
{
    uint64_t Requests = 0;
    uint64_t PathHits = 0;
    uint64_t ContentHits = 0;
    uint64_t UniqueSrvs = 0;
    uint64_t SourceBytes = 0;
    uint64_t EstimatedGpuBytes = 0;
};

class ENGINE_DLL CTextureResourceCache final
{
public:
    using LoaderFunction = std::function<HRESULT(
        const FTextureLoadRequest&,
        ID3D11ShaderResourceView**)>;

public:
    HRESULT Initialize(ComPtr<ID3D11Device> device);
    HRESULT Load(
        const FTextureLoadRequest& request,
        const LoaderFunction& loader,
        ID3D11ShaderResourceView** outSrv);
    FTextureCacheStats Get_Stats() const noexcept;
    void Clear();

private:
    struct FContentKey final
    {
        ETextureLoader Loader = ETextureLoader::WIC;
        std::array<uint8_t, 32> Sha256{};
        bool operator==(const FContentKey& rhs) const noexcept;
    };

    struct FContentKeyHasher final
    {
        size_t operator()(const FContentKey& key) const noexcept;
    };

    struct FEntry final
    {
        ComPtr<ID3D11ShaderResourceView> Srv;
        uint64_t SourceBytes = 0;
        uint64_t EstimatedGpuBytes = 0;
    };

private:
    static std::wstring Normalize_Path(const std::filesystem::path& path);
    static HRESULT Read_And_Hash(
        const std::filesystem::path& path,
        std::array<uint8_t, 32>& outSha256,
        uint64_t& outBytes);
    static uint64_t Estimate_GpuBytes(ID3D11ShaderResourceView* srv);

private:
    ComPtr<ID3D11Device> m_pDevice;
    mutable std::mutex m_Mutex;
    std::unordered_map<std::wstring, FContentKey> m_PathAliases;
    std::unordered_map<FContentKey, FEntry, FContentKeyHasher> m_Content;
    FTextureCacheStats m_Stats{};

};

NS_END
```

`TextureResourceCache.cpp`는 Windows CNG `BCryptOpenAlgorithmProvider(BCRYPT_SHA256_ALGORITHM)`과 `BCryptHashData`를 사용한다. `bcrypt.lib`는 pragma로 명시하며 해시/파일 읽기는 mutex 밖, lookup/commit은 mutex 안에서 수행한다. `Load()`는 `outSrv == nullptr`과 빈 경로를 `E_INVALIDARG`로 거부하고 성공 시 caller용 `AddRef()`를 정확히 한 번 수행한다.

#### `Engine/Private/TextureResourceCache.cpp` 전체 내용

```cpp
#include "TextureResourceCache.h"

#include <bcrypt.h>
#include <algorithm>
#include <cwctype>
#include <fstream>
#include <vector>

#pragma comment(lib, "bcrypt.lib")

using namespace Engine;

namespace
{
    uint32_t BitsPerPixel(DXGI_FORMAT format)
    {
        switch (format)
        {
        case DXGI_FORMAT_R8_UNORM: return 8;
        case DXGI_FORMAT_R8G8_UNORM: return 16;
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return 32;
        case DXGI_FORMAT_R16G16B16A16_FLOAT: return 64;
        default: return 0;
        }
    }

    bool IsBlockCompressed(DXGI_FORMAT format, uint32_t& blockBytes)
    {
        switch (format)
        {
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            blockBytes = 8;
            return true;
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            blockBytes = 16;
            return true;
        default:
            blockBytes = 0;
            return false;
        }
    }
}

HRESULT CTextureResourceCache::Initialize(ComPtr<ID3D11Device> device)
{
    if (nullptr == device)
        return E_INVALIDARG;
    m_pDevice = std::move(device);
    return S_OK;
}

HRESULT CTextureResourceCache::Load(
    const FTextureLoadRequest& request,
    const LoaderFunction& loader,
    ID3D11ShaderResourceView** outSrv)
{
    if (nullptr == outSrv || request.Path.empty() || !loader)
        return E_INVALIDARG;
    *outSrv = nullptr;

    const std::wstring normalizedPath = Normalize_Path(request.Path);
    if (normalizedPath.empty())
        return E_INVALIDARG;
    {
        std::lock_guard lock(m_Mutex);
        ++m_Stats.Requests;
        const auto alias = m_PathAliases.find(normalizedPath);
        if (alias != m_PathAliases.end())
        {
            const auto content = m_Content.find(alias->second);
            if (content != m_Content.end() && nullptr != content->second.Srv)
            {
                ++m_Stats.PathHits;
                *outSrv = content->second.Srv.Get();
                (*outSrv)->AddRef();
                return S_OK;
            }
        }
    }

    FContentKey key{};
    key.Loader = request.Loader;
    uint64_t sourceBytes = {};
    if (FAILED(Read_And_Hash(request.Path, key.Sha256, sourceBytes)))
        return E_FAIL;

    {
        std::lock_guard lock(m_Mutex);
        const auto content = m_Content.find(key);
        if (content != m_Content.end())
        {
            m_PathAliases[normalizedPath] = key;
            ++m_Stats.ContentHits;
            *outSrv = content->second.Srv.Get();
            (*outSrv)->AddRef();
            return S_OK;
        }
    }

    ComPtr<ID3D11ShaderResourceView> loaded;
    ID3D11ShaderResourceView* raw = nullptr;
    const HRESULT loadResult = loader(request, &raw);
    if (FAILED(loadResult) || nullptr == raw)
    {
        if (nullptr != raw)
            raw->Release();
        return FAILED(loadResult) ? loadResult : E_FAIL;
    }
    loaded.Attach(raw);
    const uint64_t gpuBytes = Estimate_GpuBytes(loaded.Get());

    std::lock_guard lock(m_Mutex);
    const auto existing = m_Content.find(key);
    if (existing != m_Content.end())
    {
        m_PathAliases[normalizedPath] = key;
        ++m_Stats.ContentHits;
        *outSrv = existing->second.Srv.Get();
        (*outSrv)->AddRef();
        return S_OK;
    }

    FEntry entry{};
    entry.Srv = loaded;
    entry.SourceBytes = sourceBytes;
    entry.EstimatedGpuBytes = gpuBytes;
    m_Content.emplace(key, entry);
    m_PathAliases[normalizedPath] = key;
    m_Stats.UniqueSrvs = m_Content.size();
    m_Stats.SourceBytes += sourceBytes;
    m_Stats.EstimatedGpuBytes += gpuBytes;
    *outSrv = loaded.Get();
    (*outSrv)->AddRef();
    return S_OK;
}

FTextureCacheStats CTextureResourceCache::Get_Stats() const noexcept
{
    std::lock_guard lock(m_Mutex);
    return m_Stats;
}

void CTextureResourceCache::Clear()
{
    std::lock_guard lock(m_Mutex);
    m_PathAliases.clear();
    m_Content.clear();
    m_Stats = {};
}

bool CTextureResourceCache::FContentKey::operator==(
    const FContentKey& rhs) const noexcept
{
    return Loader == rhs.Loader && Sha256 == rhs.Sha256;
}

size_t CTextureResourceCache::FContentKeyHasher::operator()(
    const FContentKey& key) const noexcept
{
    size_t value = static_cast<size_t>(key.Loader) * 131u;
    for (uint8_t byte : key.Sha256)
        value = (value ^ byte) * static_cast<size_t>(1099511628211ull);
    return value;
}

std::wstring CTextureResourceCache::Normalize_Path(
    const std::filesystem::path& path)
{
    std::error_code error;
    std::filesystem::path result = std::filesystem::weakly_canonical(path, error);
    if (error)
    {
        error.clear();
        result = std::filesystem::absolute(path, error);
    }
    if (error)
        return {};
    std::wstring value = result.lexically_normal().wstring();
    std::transform(value.begin(), value.end(), value.begin(), towlower);
    return value;
}

HRESULT CTextureResourceCache::Read_And_Hash(
    const std::filesystem::path& path,
    std::array<uint8_t, 32>& outSha256,
    uint64_t& outBytes)
{
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input)
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    const std::streamoff length = input.tellg();
    if (length < 0)
        return E_FAIL;
    input.seekg(0, std::ios::beg);

    BCRYPT_ALG_HANDLE algorithm = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    DWORD objectSize = {};
    DWORD resultSize = {};
    NTSTATUS status = BCryptOpenAlgorithmProvider(
        &algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
    if (status < 0)
        return E_FAIL;
    status = BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH,
        reinterpret_cast<PUCHAR>(&objectSize), sizeof(objectSize),
        &resultSize, 0);
    std::vector<uint8_t> object(objectSize);
    if (status >= 0)
        status = BCryptCreateHash(algorithm, &hash,
            object.data(), objectSize, nullptr, 0, 0);

    std::array<uint8_t, 1024 * 1024> buffer{};
    outBytes = 0;
    while (status >= 0 && input)
    {
        input.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        const std::streamsize read = input.gcount();
        if (read <= 0)
            break;
        status = BCryptHashData(hash, buffer.data(),
            static_cast<ULONG>(read), 0);
        outBytes += static_cast<uint64_t>(read);
    }
    if (status >= 0 && !input.eof())
        status = static_cast<NTSTATUS>(0xC0000001L);
    if (status >= 0)
        status = BCryptFinishHash(hash, outSha256.data(),
            static_cast<ULONG>(outSha256.size()), 0);

    if (nullptr != hash)
        BCryptDestroyHash(hash);
    if (nullptr != algorithm)
        BCryptCloseAlgorithmProvider(algorithm, 0);
    return status >= 0 ? S_OK : E_FAIL;
}

uint64_t CTextureResourceCache::Estimate_GpuBytes(
    ID3D11ShaderResourceView* srv)
{
    if (nullptr == srv)
        return 0;
    ComPtr<ID3D11Resource> resource;
    srv->GetResource(&resource);
    ComPtr<ID3D11Texture2D> texture;
    if (FAILED(resource.As(&texture)))
        return 0;
    D3D11_TEXTURE2D_DESC desc{};
    texture->GetDesc(&desc);
    uint64_t bytes = 0;
    uint32_t width = desc.Width;
    uint32_t height = desc.Height;
    const uint32_t mipCount = 0 == desc.MipLevels ? 1 : desc.MipLevels;
    uint32_t blockBytes = {};
    const bool blockCompressed = IsBlockCompressed(desc.Format, blockBytes);
    const uint32_t bits = BitsPerPixel(desc.Format);
    for (uint32_t mip = 0; mip < mipCount; ++mip)
    {
        if (blockCompressed)
            bytes += static_cast<uint64_t>((width + 3) / 4) *
                ((height + 3) / 4) * blockBytes;
        else if (0 != bits)
            bytes += (static_cast<uint64_t>(width) * height * bits + 7) / 8;
        width = (std::max)(1u, width >> 1u);
        height = (std::max)(1u, height >> 1u);
    }
    return bytes * desc.ArraySize;
}
```

#### `Engine/Private/Material.cpp` texture helper 교체 블록

`Material.cpp` include에 다음을 추가한다.

```cpp
#include "GameInstance.h"
#include "TextureResourceCache.h"
```

anonymous namespace의 기존 `AddTexture()`를 다음 전체 함수로 교체한다.

```cpp
ETextureSemantic ToSemantic(aiTextureType type)
{
    switch (type)
    {
    case aiTextureType_DIFFUSE: return ETextureSemantic::Color;
    case aiTextureType_NORMALS: return ETextureSemantic::Normal;
    case aiTextureType_EMISSIVE: return ETextureSemantic::Emissive;
    case aiTextureType_OPACITY:
    case aiTextureType_METALNESS:
    case aiTextureType_DIFFUSE_ROUGHNESS:
    case aiTextureType_AMBIENT_OCCLUSION:
    case aiTextureType_UNKNOWN: return ETextureSemantic::Mask;
    default: return ETextureSemantic::Unknown;
    }
}

ETextureLoader ToLoader(const filesystem::path& path)
{
    wstring extension = path.extension().wstring();
    transform(extension.begin(), extension.end(), extension.begin(), towlower);
    if (extension == L".dds")
        return ETextureLoader::DDS;
    if (extension == L".tga")
        return ETextureLoader::TGA;
    return ETextureLoader::WIC;
}

HRESULT AddTexture(
    ComPtr<ID3D11Device> device,
    const filesystem::path& path,
    aiTextureType type,
    vector<ComPtr<ID3D11ShaderResourceView>> (&textures)[AI_TEXTURE_TYPE_MAX])
{
    if (path.empty())
        return S_OK;
    CTextureResourceCache* cache =
        CGameInstance::Get().Get_TextureResourceCache();
    if (nullptr == cache)
        return E_UNEXPECTED;

    FTextureLoadRequest request{};
    request.Path = path;
    request.Loader = ToLoader(path);
    request.Semantic = ToSemantic(type);
    ID3D11ShaderResourceView* raw = nullptr;
    const HRESULT result = cache->Load(
        request,
        [device](const FTextureLoadRequest& value,
            ID3D11ShaderResourceView** output)
        {
            ComPtr<ID3D11ShaderResourceView> loaded;
            const HRESULT loadResult = LoadTexture(
                device, value.Path, loaded);
            if (FAILED(loadResult))
                return loadResult;
            *output = loaded.Detach();
            return S_OK;
        },
        &raw);
    if (FAILED(result))
        return result;
    ComPtr<ID3D11ShaderResourceView> resource;
    resource.Attach(raw);
    textures[type].push_back(std::move(resource));
    return S_OK;
}
```

`CMaterial::Initialize(const aiMaterial*, const char_t*)`의 texture별 직접 loader 분기를
다음 호출로 교체한다.

```cpp
if (FAILED(AddTexture(
    m_pDevice,
    std::filesystem::path(szTextureFilePath),
    static_cast<aiTextureType>(i),
    m_Textures)))
    return E_FAIL;
```

### 5-3. `Engine/Public/NavGrid.h` 전체 내용

```cpp
#pragma once

#include "Engine_Defines.h"
#include <cstdint>
#include <filesystem>
#include <shared_mutex>
#include <vector>

NS_BEGIN(Engine)

struct FNavGridDesc final
{
    uint32_t Width = 0;
    uint32_t Height = 0;
    float CellSize = 0.f;
    float OriginX = 0.f;
    float OriginZ = 0.f;
    float MinHeight = 0.f;
    float MaxHeight = 0.f;
    float AgentRadius = 0.f;
    float MaxSlopeDegrees = 0.f;
    float MaxStepHeight = 0.f;
};

class ENGINE_DLL CNavGrid final
{
public:
    HRESULT Load_FromFile(const std::filesystem::path& path);
    HRESULT Load_FromMemory(const uint8_t* bytes, size_t size);

    const FNavGridDesc& Get_Desc() const noexcept;
    uint32_t Get_CellCount() const noexcept;
    bool Is_ValidCell(int32_t x, int32_t z) const noexcept;
    bool Is_Walkable(int32_t x, int32_t z) const noexcept;
    bool Is_Walkable(uint32_t index) const noexcept;
    float Get_Height(uint32_t index) const noexcept;
    uint32_t To_Index(int32_t x, int32_t z) const noexcept;
    bool World_ToCell(const float3_t& world, int32_t& outX, int32_t& outZ) const noexcept;
    float3_t Cell_ToWorld(uint32_t index) const noexcept;
    bool Find_NearestWalkable(
        const float3_t& world,
        uint32_t maxRadiusCells,
        uint32_t& outIndex) const noexcept;

private:
    FNavGridDesc m_Desc{};
    std::vector<uint8_t> m_WalkableBits;
    std::vector<float> m_Heights;

};

NS_END
```

#### `Engine/Private/NavGrid.cpp` 전체 내용

```cpp
#include "NavGrid.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <limits>

using namespace Engine;

namespace
{
#pragma pack(push, 1)
    struct NAVGRID_FILE_HEADER_V1 final
    {
        char Magic[8];
        uint32_t Version;
        uint32_t Width;
        uint32_t Height;
        float CellSize;
        float OriginX;
        float OriginZ;
        float MinHeight;
        float MaxHeight;
        float AgentRadius;
        float MaxSlopeDegrees;
        float MaxStepHeight;
        uint32_t WalkableByteCount;
        uint32_t HeightCount;
        uint8_t SourceSha256[32];
    };
#pragma pack(pop)

    constexpr char MAGIC[8] = { 'W', 'N', 'A', 'V', 'G', 'R', 'D', '\0' };
    constexpr uint32_t VERSION = 1;
    constexpr uint64_t MAX_CELL_COUNT = 16ull * 1024ull * 1024ull;

    bool IsFinite(float value)
    {
        return std::isfinite(value);
    }
}

HRESULT CNavGrid::Load_FromFile(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input)
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    const std::streamoff length = input.tellg();
    if (length <= 0 ||
        static_cast<uint64_t>(length) >
            static_cast<uint64_t>((std::numeric_limits<size_t>::max)()))
        return E_FAIL;
    input.seekg(0, std::ios::beg);
    std::vector<uint8_t> bytes(static_cast<size_t>(length));
    if (!input.read(reinterpret_cast<char*>(bytes.data()), length))
        return E_FAIL;
    return Load_FromMemory(bytes.data(), bytes.size());
}

HRESULT CNavGrid::Load_FromMemory(const uint8_t* bytes, size_t size)
{
    if (nullptr == bytes || size < sizeof(NAVGRID_FILE_HEADER_V1))
        return E_INVALIDARG;

    NAVGRID_FILE_HEADER_V1 header{};
    std::memcpy(&header, bytes, sizeof(header));
    if (0 != std::memcmp(header.Magic, MAGIC, sizeof(MAGIC)) ||
        header.Version != VERSION || 0 == header.Width || 0 == header.Height)
        return E_FAIL;

    const uint64_t cellCount64 =
        static_cast<uint64_t>(header.Width) * header.Height;
    if (0 == cellCount64 || cellCount64 > MAX_CELL_COUNT ||
        header.HeightCount != cellCount64)
        return E_FAIL;
    const uint64_t walkableBytes64 = (cellCount64 + 7ull) / 8ull;
    if (header.WalkableByteCount != walkableBytes64)
        return E_FAIL;
    const uint64_t expectedSize = sizeof(header) + walkableBytes64 +
        cellCount64 * sizeof(float);
    if (expectedSize != size)
        return E_FAIL;

    FNavGridDesc stagedDesc{};
    stagedDesc.Width = header.Width;
    stagedDesc.Height = header.Height;
    stagedDesc.CellSize = header.CellSize;
    stagedDesc.OriginX = header.OriginX;
    stagedDesc.OriginZ = header.OriginZ;
    stagedDesc.MinHeight = header.MinHeight;
    stagedDesc.MaxHeight = header.MaxHeight;
    stagedDesc.AgentRadius = header.AgentRadius;
    stagedDesc.MaxSlopeDegrees = header.MaxSlopeDegrees;
    stagedDesc.MaxStepHeight = header.MaxStepHeight;
    if (!IsFinite(stagedDesc.CellSize) || stagedDesc.CellSize <= 0.f ||
        !IsFinite(stagedDesc.OriginX) || !IsFinite(stagedDesc.OriginZ) ||
        !IsFinite(stagedDesc.MinHeight) || !IsFinite(stagedDesc.MaxHeight) ||
        stagedDesc.MinHeight > stagedDesc.MaxHeight ||
        !IsFinite(stagedDesc.AgentRadius) || stagedDesc.AgentRadius < 0.f ||
        !IsFinite(stagedDesc.MaxSlopeDegrees) ||
        stagedDesc.MaxSlopeDegrees <= 0.f ||
        stagedDesc.MaxSlopeDegrees > 90.f ||
        !IsFinite(stagedDesc.MaxStepHeight) ||
        stagedDesc.MaxStepHeight < 0.f)
        return E_FAIL;

    const size_t cellCount = static_cast<size_t>(cellCount64);
    const size_t walkableBytes = static_cast<size_t>(walkableBytes64);
    std::vector<uint8_t> stagedBits(walkableBytes);
    std::memcpy(stagedBits.data(), bytes + sizeof(header), walkableBytes);
    if (0 != (cellCount & 7u))
    {
        const uint8_t validMask = static_cast<uint8_t>(
            (1u << (cellCount & 7u)) - 1u);
        if (0 != (stagedBits.back() & static_cast<uint8_t>(~validMask)))
            return E_FAIL;
    }

    std::vector<float> stagedHeights(cellCount);
    std::memcpy(stagedHeights.data(),
        bytes + sizeof(header) + walkableBytes,
        cellCount * sizeof(float));
    for (size_t index = 0; index < cellCount; ++index)
    {
        const bool walkable =
            0 != (stagedBits[index >> 3u] & (1u << (index & 7u)));
        if (walkable && (!IsFinite(stagedHeights[index]) ||
            stagedHeights[index] < stagedDesc.MinHeight ||
            stagedHeights[index] > stagedDesc.MaxHeight))
            return E_FAIL;
    }

    m_Desc = stagedDesc;
    m_WalkableBits = std::move(stagedBits);
    m_Heights = std::move(stagedHeights);
    return S_OK;
}

const FNavGridDesc& CNavGrid::Get_Desc() const noexcept
{
    return m_Desc;
}

uint32_t CNavGrid::Get_CellCount() const noexcept
{
    return static_cast<uint32_t>(m_Heights.size());
}

bool CNavGrid::Is_ValidCell(int32_t x, int32_t z) const noexcept
{
    return x >= 0 && z >= 0 &&
        x < static_cast<int32_t>(m_Desc.Width) &&
        z < static_cast<int32_t>(m_Desc.Height);
}

bool CNavGrid::Is_Walkable(int32_t x, int32_t z) const noexcept
{
    return Is_ValidCell(x, z) && Is_Walkable(To_Index(x, z));
}

bool CNavGrid::Is_Walkable(uint32_t index) const noexcept
{
    return index < m_Heights.size() &&
        0 != (m_WalkableBits[index >> 3u] & (1u << (index & 7u)));
}

float CNavGrid::Get_Height(uint32_t index) const noexcept
{
    return index < m_Heights.size() ? m_Heights[index] : 0.f;
}

uint32_t CNavGrid::To_Index(int32_t x, int32_t z) const noexcept
{
    return static_cast<uint32_t>(z) * m_Desc.Width +
        static_cast<uint32_t>(x);
}

bool CNavGrid::World_ToCell(
    const float3_t& world, int32_t& outX, int32_t& outZ) const noexcept
{
    if (m_Desc.CellSize <= 0.f || !std::isfinite(world.x) ||
        !std::isfinite(world.z))
        return false;
    outX = static_cast<int32_t>(std::floor(
        (world.x - m_Desc.OriginX) / m_Desc.CellSize));
    outZ = static_cast<int32_t>(std::floor(
        (world.z - m_Desc.OriginZ) / m_Desc.CellSize));
    return Is_ValidCell(outX, outZ);
}

float3_t CNavGrid::Cell_ToWorld(uint32_t index) const noexcept
{
    if (index >= m_Heights.size())
        return {};
    const uint32_t x = index % m_Desc.Width;
    const uint32_t z = index / m_Desc.Width;
    return float3_t(
        m_Desc.OriginX + (static_cast<float>(x) + 0.5f) * m_Desc.CellSize,
        m_Heights[index],
        m_Desc.OriginZ + (static_cast<float>(z) + 0.5f) * m_Desc.CellSize);
}

bool CNavGrid::Find_NearestWalkable(
    const float3_t& world,
    uint32_t maxRadiusCells,
    uint32_t& outIndex) const noexcept
{
    int32_t centerX = {};
    int32_t centerZ = {};
    if (!World_ToCell(world, centerX, centerZ))
        return false;
    if (Is_Walkable(centerX, centerZ))
    {
        outIndex = To_Index(centerX, centerZ);
        return true;
    }

    uint64_t bestDistance = UINT64_MAX;
    uint32_t bestIndex = UINT32_MAX;
    for (int32_t dz = -static_cast<int32_t>(maxRadiusCells);
        dz <= static_cast<int32_t>(maxRadiusCells); ++dz)
    {
        for (int32_t dx = -static_cast<int32_t>(maxRadiusCells);
            dx <= static_cast<int32_t>(maxRadiusCells); ++dx)
        {
            const uint64_t distance = static_cast<uint64_t>(dx * dx + dz * dz);
            if (distance > static_cast<uint64_t>(maxRadiusCells) *
                maxRadiusCells)
                continue;
            const int32_t x = centerX + dx;
            const int32_t z = centerZ + dz;
            if (!Is_Walkable(x, z))
                continue;
            const uint32_t index = To_Index(x, z);
            if (distance < bestDistance ||
                (distance == bestDistance && index < bestIndex))
            {
                bestDistance = distance;
                bestIndex = index;
            }
        }
    }
    if (UINT32_MAX == bestIndex)
        return false;
    outIndex = bestIndex;
    return true;
}
```

### 5-4. `Engine/Public/PathFinder.h` 전체 내용

```cpp
#pragma once

#include "Engine_Defines.h"
#include "NavGrid.h"
#include <cstdint>
#include <vector>

NS_BEGIN(Engine)

enum class EPathResult : uint8_t
{
    Success,
    InvalidGrid,
    StartNotFound,
    GoalNotFound,
    Unreachable,
    ExpansionLimit
};

struct FPathQuery final
{
    float3_t Start{};
    float3_t Goal{};
    uint32_t NearestRadiusCells = 4;
    uint32_t MaxExpandedNodes = 16384;
    bool Smooth = true;
};

struct FPathQueryResult final
{
    EPathResult Result = EPathResult::InvalidGrid;
    std::vector<float3_t> Points;
    uint32_t ExpandedNodes = 0;
    uint64_t QueryMicroseconds = 0;
};

class ENGINE_DLL CPathFinder final
{
public:
    HRESULT Initialize(const std::shared_ptr<CNavGrid>& grid);
    FPathQueryResult Find_Path(const FPathQuery& query);

private:
    struct FNodeState final
    {
        float G = 0.f;
        uint32_t Parent = UINT32_MAX;
        uint32_t OpenGeneration = 0;
        uint32_t ClosedGeneration = 0;
        int32_t HeapPosition = -1;
    };

    float Heuristic(uint32_t from, uint32_t to) const noexcept;
    bool Can_Step(uint32_t from, int32_t dx, int32_t dz, uint32_t& outTo) const noexcept;
    void Heap_Push(uint32_t index, uint32_t goal);
    uint32_t Heap_Pop(uint32_t goal);
    void Heap_Update(uint32_t index, uint32_t goal);
    bool Has_LineOfSight(uint32_t from, uint32_t to) const noexcept;
    void Build_Result(uint32_t goal, bool smooth, FPathQueryResult& result) const;

private:
    std::shared_ptr<CNavGrid> m_Grid;
    std::vector<FNodeState> m_Nodes;
    std::vector<uint32_t> m_Heap;
    uint32_t m_QueryGeneration = 0;

};

NS_END
```

#### `Engine/Private/PathFinder.cpp` 전체 내용

```cpp
#include "PathFinder.h"

#include <algorithm>
#include <chrono>
#include <cmath>

using namespace Engine;

HRESULT CPathFinder::Initialize(const std::shared_ptr<CNavGrid>& grid)
{
    if (nullptr == grid || 0 == grid->Get_CellCount())
        return E_INVALIDARG;
    m_Grid = grid;
    m_Nodes.assign(grid->Get_CellCount(), FNodeState{});
    m_Heap.clear();
    m_Heap.reserve(grid->Get_CellCount());
    m_QueryGeneration = 0;
    return S_OK;
}

FPathQueryResult CPathFinder::Find_Path(const FPathQuery& query)
{
    const auto begin = std::chrono::steady_clock::now();
    FPathQueryResult result{};
    if (nullptr == m_Grid || 0 == m_Grid->Get_CellCount())
        return result;

    uint32_t start = {};
    uint32_t goal = {};
    if (!m_Grid->Find_NearestWalkable(
        query.Start, query.NearestRadiusCells, start))
    {
        result.Result = EPathResult::StartNotFound;
        return result;
    }
    if (!m_Grid->Find_NearestWalkable(
        query.Goal, query.NearestRadiusCells, goal))
    {
        result.Result = EPathResult::GoalNotFound;
        return result;
    }

    ++m_QueryGeneration;
    if (0 == m_QueryGeneration)
    {
        for (FNodeState& node : m_Nodes)
            node = {};
        m_QueryGeneration = 1;
    }
    m_Heap.clear();

    FNodeState& startNode = m_Nodes[start];
    startNode.G = 0.f;
    startNode.Parent = UINT32_MAX;
    startNode.OpenGeneration = m_QueryGeneration;
    startNode.ClosedGeneration = 0;
    Heap_Push(start, goal);

    constexpr int32_t directions[8][2] = {
        { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 },
        { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 }
    };

    while (!m_Heap.empty())
    {
        if (result.ExpandedNodes >= query.MaxExpandedNodes)
        {
            result.Result = EPathResult::ExpansionLimit;
            break;
        }

        const uint32_t current = Heap_Pop(goal);
        FNodeState& currentNode = m_Nodes[current];
        currentNode.ClosedGeneration = m_QueryGeneration;
        ++result.ExpandedNodes;
        if (current == goal)
        {
            result.Result = EPathResult::Success;
            Build_Result(goal, query.Smooth, result);
            break;
        }

        for (const auto& direction : directions)
        {
            uint32_t neighbor = {};
            if (!Can_Step(current, direction[0], direction[1], neighbor))
                continue;
            FNodeState& next = m_Nodes[neighbor];
            if (next.ClosedGeneration == m_QueryGeneration)
                continue;

            const float stepCost =
                0 != direction[0] && 0 != direction[1] ? 14.f : 10.f;
            const float candidate = currentNode.G + stepCost;
            if (next.OpenGeneration != m_QueryGeneration)
            {
                next.G = candidate;
                next.Parent = current;
                next.OpenGeneration = m_QueryGeneration;
                next.ClosedGeneration = 0;
                Heap_Push(neighbor, goal);
            }
            else if (candidate < next.G)
            {
                next.G = candidate;
                next.Parent = current;
                Heap_Update(neighbor, goal);
            }
        }
    }

    if (EPathResult::InvalidGrid == result.Result)
        result.Result = EPathResult::Unreachable;
    result.QueryMicroseconds = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - begin).count());
    return result;
}

float CPathFinder::Heuristic(uint32_t from, uint32_t to) const noexcept
{
    const uint32_t width = m_Grid->Get_Desc().Width;
    const int32_t ax = static_cast<int32_t>(from % width);
    const int32_t az = static_cast<int32_t>(from / width);
    const int32_t bx = static_cast<int32_t>(to % width);
    const int32_t bz = static_cast<int32_t>(to / width);
    const int32_t dx = std::abs(ax - bx);
    const int32_t dz = std::abs(az - bz);
    return static_cast<float>(10 * (dx + dz) - 6 * (std::min)(dx, dz));
}

bool CPathFinder::Can_Step(
    uint32_t from, int32_t dx, int32_t dz, uint32_t& outTo) const noexcept
{
    const FNavGridDesc& desc = m_Grid->Get_Desc();
    const int32_t x = static_cast<int32_t>(from % desc.Width);
    const int32_t z = static_cast<int32_t>(from / desc.Width);
    const int32_t targetX = x + dx;
    const int32_t targetZ = z + dz;
    if (!m_Grid->Is_Walkable(targetX, targetZ))
        return false;
    if (0 != dx && 0 != dz &&
        (!m_Grid->Is_Walkable(x + dx, z) ||
            !m_Grid->Is_Walkable(x, z + dz)))
        return false;

    const uint32_t target = m_Grid->To_Index(targetX, targetZ);
    if (std::abs(m_Grid->Get_Height(target) -
        m_Grid->Get_Height(from)) > desc.MaxStepHeight)
        return false;
    outTo = target;
    return true;
}

void CPathFinder::Heap_Push(uint32_t index, uint32_t goal)
{
    auto less = [&](uint32_t lhs, uint32_t rhs)
    {
        const float lhsF = m_Nodes[lhs].G + Heuristic(lhs, goal);
        const float rhsF = m_Nodes[rhs].G + Heuristic(rhs, goal);
        return lhsF < rhsF || (lhsF == rhsF && lhs < rhs);
    };
    m_Nodes[index].HeapPosition = static_cast<int32_t>(m_Heap.size());
    m_Heap.push_back(index);
    size_t position = m_Heap.size() - 1;
    while (position > 0)
    {
        const size_t parent = (position - 1) / 2;
        if (!less(m_Heap[position], m_Heap[parent]))
            break;
        std::swap(m_Heap[position], m_Heap[parent]);
        m_Nodes[m_Heap[position]].HeapPosition = static_cast<int32_t>(position);
        m_Nodes[m_Heap[parent]].HeapPosition = static_cast<int32_t>(parent);
        position = parent;
    }
}

uint32_t CPathFinder::Heap_Pop(uint32_t goal)
{
    auto less = [&](uint32_t lhs, uint32_t rhs)
    {
        const float lhsF = m_Nodes[lhs].G + Heuristic(lhs, goal);
        const float rhsF = m_Nodes[rhs].G + Heuristic(rhs, goal);
        return lhsF < rhsF || (lhsF == rhsF && lhs < rhs);
    };
    const uint32_t result = m_Heap.front();
    const uint32_t tail = m_Heap.back();
    m_Heap.pop_back();
    m_Nodes[result].HeapPosition = -1;
    if (m_Heap.empty())
        return result;
    m_Heap[0] = tail;
    m_Nodes[tail].HeapPosition = 0;
    size_t position = 0;
    for (;;)
    {
        const size_t left = position * 2 + 1;
        const size_t right = left + 1;
        if (left >= m_Heap.size())
            break;
        size_t best = left;
        if (right < m_Heap.size() && less(m_Heap[right], m_Heap[left]))
            best = right;
        if (!less(m_Heap[best], m_Heap[position]))
            break;
        std::swap(m_Heap[position], m_Heap[best]);
        m_Nodes[m_Heap[position]].HeapPosition = static_cast<int32_t>(position);
        m_Nodes[m_Heap[best]].HeapPosition = static_cast<int32_t>(best);
        position = best;
    }
    return result;
}

void CPathFinder::Heap_Update(uint32_t index, uint32_t goal)
{
    int32_t storedPosition = m_Nodes[index].HeapPosition;
    if (storedPosition < 0)
        return;
    auto less = [&](uint32_t lhs, uint32_t rhs)
    {
        const float lhsF = m_Nodes[lhs].G + Heuristic(lhs, goal);
        const float rhsF = m_Nodes[rhs].G + Heuristic(rhs, goal);
        return lhsF < rhsF || (lhsF == rhsF && lhs < rhs);
    };
    size_t position = static_cast<size_t>(storedPosition);
    while (position > 0)
    {
        const size_t parent = (position - 1) / 2;
        if (!less(m_Heap[position], m_Heap[parent]))
            break;
        std::swap(m_Heap[position], m_Heap[parent]);
        m_Nodes[m_Heap[position]].HeapPosition = static_cast<int32_t>(position);
        m_Nodes[m_Heap[parent]].HeapPosition = static_cast<int32_t>(parent);
        position = parent;
    }
}

bool CPathFinder::Has_LineOfSight(uint32_t from, uint32_t to) const noexcept
{
    const uint32_t width = m_Grid->Get_Desc().Width;
    int32_t x0 = static_cast<int32_t>(from % width);
    int32_t z0 = static_cast<int32_t>(from / width);
    const int32_t x1 = static_cast<int32_t>(to % width);
    const int32_t z1 = static_cast<int32_t>(to / width);
    const int32_t dx = std::abs(x1 - x0);
    const int32_t dz = std::abs(z1 - z0);
    const int32_t sx = x0 < x1 ? 1 : -1;
    const int32_t sz = z0 < z1 ? 1 : -1;
    int32_t error = dx - dz;
    uint32_t current = from;
    while (x0 != x1 || z0 != z1)
    {
        const int32_t twiceError = error * 2;
        int32_t stepX = 0;
        int32_t stepZ = 0;
        if (twiceError > -dz)
        {
            error -= dz;
            stepX = sx;
        }
        if (twiceError < dx)
        {
            error += dx;
            stepZ = sz;
        }
        uint32_t next = {};
        if (!Can_Step(current, stepX, stepZ, next))
            return false;
        current = next;
        x0 += stepX;
        z0 += stepZ;
    }
    return true;
}

void CPathFinder::Build_Result(
    uint32_t goal, bool smooth, FPathQueryResult& result) const
{
    std::vector<uint32_t> cells;
    for (uint32_t current = goal; UINT32_MAX != current;
        current = m_Nodes[current].Parent)
        cells.push_back(current);
    std::reverse(cells.begin(), cells.end());

    std::vector<uint32_t> selected;
    if (!smooth || cells.size() <= 2)
        selected = cells;
    else
    {
        selected.push_back(cells.front());
        size_t anchor = 0;
        while (anchor + 1 < cells.size())
        {
            size_t farthest = anchor + 1;
            for (size_t candidate = cells.size() - 1;
                candidate > anchor + 1; --candidate)
            {
                if (Has_LineOfSight(cells[anchor], cells[candidate]))
                {
                    farthest = candidate;
                    break;
                }
            }
            selected.push_back(cells[farthest]);
            anchor = farthest;
        }
    }

    result.Points.clear();
    result.Points.reserve(selected.size());
    for (uint32_t cell : selected)
        result.Points.push_back(m_Grid->Cell_ToWorld(cell));
}
```

### 5-4a. 공통 재질 바인딩과 정적 mesh instancing

#### `Client/Public/MapAssetRenderUtils.h` 전체 내용

```cpp
#pragma once

#include "Client_Defines.h"
#include "MapAssetCatalog.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)

class CMapAssetRenderUtils final
{
public:
    static uint32_t Select_Pass(
        const MAP_ASSET_RENDER_PROFILE& profile,
        bool_t mirrored);
    static HRESULT Bind_Material(
        const shared_ptr<Engine::CModel>& model,
        const shared_ptr<Engine::CShader>& shader,
        uint32_t meshIndex,
        const MAP_ASSET_RENDER_PROFILE& profile,
        f32_t elapsedTime);
};

NS_END
```

#### `Client/Private/MapAssetRenderUtils.cpp` 전체 내용

```cpp
#include "MapAssetRenderUtils.h"

#include "Model.h"
#include "Shader.h"

using namespace Client;

uint32_t CMapAssetRenderUtils::Select_Pass(
    const MAP_ASSET_RENDER_PROFILE& profile,
    bool_t mirrored)
{
    MAP_ASSET_CULL_MODE cull = profile.cullMode;
    if (mirrored && MAP_ASSET_CULL_MODE::TWO_SIDED != cull)
    {
        cull = MAP_ASSET_CULL_MODE::CULL_BACK == cull ?
            MAP_ASSET_CULL_MODE::CULL_FRONT :
            MAP_ASSET_CULL_MODE::CULL_BACK;
    }
    const uint32_t cullOffset =
        MAP_ASSET_CULL_MODE::CULL_BACK == cull ? 0u :
        MAP_ASSET_CULL_MODE::CULL_FRONT == cull ? 1u : 2u;
    const uint32_t modeOffset =
        MAP_ASSET_RENDER_MODE::DEFERRED == profile.renderMode ? 0u :
        MAP_ASSET_RENDER_MODE::TRANSLUCENT == profile.renderMode ? 3u :
        MAP_ASSET_RENDER_MODE::BACKGROUND == profile.renderMode ? 6u : 9u;
    return modeOffset + cullOffset;
}

HRESULT CMapAssetRenderUtils::Bind_Material(
    const shared_ptr<Engine::CModel>& model,
    const shared_ptr<Engine::CShader>& shader,
    uint32_t meshIndex,
    const MAP_ASSET_RENDER_PROFILE& profile,
    f32_t elapsedTime)
{
    if (nullptr == model || nullptr == shader ||
        meshIndex >= model->Get_NumMeshes())
        return E_INVALIDARG;

    const uint32_t hasNormal = model->Has_MaterialTexture(
        meshIndex, aiTextureType_NORMALS) ? 1u : 0u;
    const uint32_t hasEmissive = model->Has_MaterialTexture(
        meshIndex, aiTextureType_EMISSIVE) ? 1u : 0u;
    const uint32_t hasSpecular = model->Has_MaterialTexture(
        meshIndex, aiTextureType_SPECULAR) ? 1u : 0u;
    const uint32_t hasOpacity = model->Has_MaterialTexture(
        meshIndex, aiTextureType_OPACITY) ? 1u : 0u;
    const float2_t uvOffset(
        profile.uvSpeed.x * elapsedTime,
        profile.uvSpeed.y * elapsedTime);

    if (FAILED(model->Bind_Material(
        shader, "g_DiffuseTexture", meshIndex, aiTextureType_DIFFUSE)) ||
        FAILED(shader->Bind_RawValue(
            "g_UVScale", &profile.uvScale, sizeof(profile.uvScale))) ||
        FAILED(shader->Bind_RawValue(
            "g_UVOffset", &uvOffset, sizeof(uvOffset))) ||
        FAILED(shader->Bind_RawValue(
            "g_Opacity", &profile.opacity, sizeof(profile.opacity))) ||
        FAILED(shader->Bind_RawValue(
            "g_ColorTint", &profile.colorTint, sizeof(profile.colorTint))) ||
        FAILED(shader->Bind_RawValue(
            "g_HasNormalTexture", &hasNormal, sizeof(hasNormal))) ||
        (0 != hasNormal && FAILED(model->Bind_Material(
            shader, "g_NormalTexture", meshIndex,
            aiTextureType_NORMALS))) ||
        FAILED(shader->Bind_RawValue(
            "g_HasEmissiveTexture", &hasEmissive, sizeof(hasEmissive))) ||
        FAILED(shader->Bind_RawValue(
            "g_EmissiveIntensity", &profile.emissiveIntensity,
            sizeof(profile.emissiveIntensity))) ||
        (0 != hasEmissive && FAILED(model->Bind_Material(
            shader, "g_EmissiveTexture", meshIndex,
            aiTextureType_EMISSIVE))) ||
        FAILED(shader->Bind_RawValue(
            "g_HasSpecularTexture", &hasSpecular, sizeof(hasSpecular))) ||
        FAILED(shader->Bind_RawValue(
            "g_SpecularIntensity", &profile.specularIntensity,
            sizeof(profile.specularIntensity))) ||
        FAILED(shader->Bind_RawValue(
            "g_SpecularPower", &profile.specularPower,
            sizeof(profile.specularPower))) ||
        (0 != hasSpecular && FAILED(model->Bind_Material(
            shader, "g_SpecularTexture", meshIndex,
            aiTextureType_SPECULAR))) ||
        FAILED(shader->Bind_RawValue(
            "g_HasOpacityTexture", &hasOpacity, sizeof(hasOpacity))) ||
        (0 != hasOpacity && FAILED(model->Bind_Material(
            shader, "g_OpacityTexture", meshIndex,
            aiTextureType_OPACITY))))
        return E_FAIL;
    return S_OK;
}
```

#### `Engine/Public/Engine_Struct.h`에 추가할 전체 구조체

```cpp
typedef struct tagVertexMeshInstance
{
    XMFLOAT4 vRight;
    XMFLOAT4 vUp;
    XMFLOAT4 vLook;
    XMFLOAT4 vTranslation;

    static constexpr uint32_t iNumElements = 9;
    static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
            0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,
            0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,
            0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,
            0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,
            0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
            1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT,
            1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT,
            1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT,
            1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    };
} VTXMESH_INSTANCE;
```

#### `Engine/Public/Mesh.h` 추가 선언

```cpp
public:
    HRESULT Render_Instanced(
        ID3D11Buffer* instanceBuffer,
        uint32_t instanceStride,
        uint32_t instanceCount);
```

#### `Engine/Private/Mesh.cpp` 추가 함수 전체

```cpp
HRESULT CMesh::Render_Instanced(
    ID3D11Buffer* instanceBuffer,
    uint32_t instanceStride,
    uint32_t instanceCount)
{
    if (nullptr == instanceBuffer || 0 == instanceStride ||
        0 == instanceCount)
        return E_INVALIDARG;

    ID3D11Buffer* vertexBuffers[2] = {
        m_pVB.Get(), instanceBuffer
    };
    const uint32_t strides[2] = {
        m_iVertexStride, instanceStride
    };
    const uint32_t offsets[2] = { 0u, 0u };
    m_pContext->IASetVertexBuffers(
        0, 2, vertexBuffers, strides, offsets);
    m_pContext->IASetIndexBuffer(
        m_pIB.Get(), m_eIndexFormat, 0);
    m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology);
    m_pContext->DrawIndexedInstanced(
        m_iNumIndices, instanceCount, 0, 0, 0);

    if (CProfiler* profiler = CGameInstance::Get().Get_Profiler())
    {
        profiler->Add_Counter(EProfilerCounter::DrawCalls);
        profiler->Add_Counter(EProfilerCounter::InstancedDrawCalls);
        profiler->Add_Counter(EProfilerCounter::Instances, instanceCount);
        profiler->Add_Counter(EProfilerCounter::Indices,
            static_cast<uint64_t>(m_iNumIndices) * instanceCount);
    }
    return S_OK;
}
```

`Mesh.cpp` include에 다음을 추가한다.

```cpp
#include "GameInstance.h"
#include "Profiler.h"
```

#### `Engine/Public/Model.h` 추가 선언

```cpp
public:
    HRESULT Render_Instanced(
        uint32_t meshIndex,
        ID3D11Buffer* instanceBuffer,
        uint32_t instanceStride,
        uint32_t instanceCount);
```

#### `Engine/Private/Model.cpp` 추가 함수 전체

```cpp
HRESULT CModel::Render_Instanced(
    uint32_t meshIndex,
    ID3D11Buffer* instanceBuffer,
    uint32_t instanceStride,
    uint32_t instanceCount)
{
    if (meshIndex >= m_Meshes.size())
        return E_INVALIDARG;
    return m_Meshes[meshIndex]->Render_Instanced(
        instanceBuffer, instanceStride, instanceCount);
}
```

### 5-5. `Client/Public/MapStaticBatchObject.h` 전체 내용

```cpp
#pragma once

#include "GameObject.h"
#include "MapAssetCatalog.h"
#include <cstdint>
#include <unordered_map>
#include <vector>

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END

NS_BEGIN(Client)

struct FMapStaticInstance final
{
    uint64_t PlacementId = 0;
    _float4x4 World{};
    float3_t WorldBoundsCenter{};
    float WorldBoundsRadius = 0.f;
    bool Visible = true;
};

class CMapStaticBatchObject final : public Engine::CGameObject
{
public:
    struct DESC final : public Engine::CGameObject::GAMEOBJECT_DESC
    {
        std::string AssetId;
        std::wstring ModelPrototypeTag;
        MAP_ASSET_RENDER_PROFILE RenderProfile{};
        bool Mirrored = false;
        std::vector<FMapStaticInstance> Instances;
    };

public:
    CMapStaticBatchObject(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context);
    CMapStaticBatchObject(const CMapStaticBatchObject& rhs);
    virtual ~CMapStaticBatchObject() = default;

    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* arg) override;
    virtual void Priority_Update(f32_t timeDelta) override;
    virtual void Update(f32_t timeDelta) override;
    virtual void Late_Update(f32_t timeDelta) override;
    virtual HRESULT Render() override;

    HRESULT Update_Instance(uint64_t placementId, const FMapStaticInstance& instance);
    HRESULT Set_InstanceVisible(uint64_t placementId, bool visible);
    uint32_t Get_VisibleInstanceCount() const noexcept;
    const std::string& Get_AssetId() const noexcept;
    bool Is_Mirrored() const noexcept;

private:
    HRESULT Ready_Components();
    HRESULT Ensure_InstanceCapacity(uint32_t requiredCount);
    HRESULT Upload_VisibleInstances();
    void Rebuild_PlacementLookup();

private:
    std::string m_AssetId;
    MAP_ASSET_RENDER_PROFILE m_RenderProfile{};
    bool m_Mirrored = false;
    f32_t m_fElapsedTime = 0.f;
    std::vector<FMapStaticInstance> m_Instances;
    std::vector<VTXMESH_INSTANCE> m_VisibleInstances;
    std::unordered_map<uint64_t, uint32_t> m_PlacementLookup;
    uint32_t m_InstanceCapacity = 0;
    ComPtr<ID3D11Buffer> m_pInstanceBuffer;
    shared_ptr<Engine::CModel> m_pModelCom;
    shared_ptr<Engine::CShader> m_pShaderCom;

public:
    static unique_ptr<CMapStaticBatchObject> Create(
        ComPtr<ID3D11Device> device,
        ComPtr<ID3D11DeviceContext> context);
    virtual shared_ptr<CPrototype> Clone(void* arg) override;
};

NS_END
```

#### `Client/Private/MapStaticBatchObject.cpp` 전체 내용

```cpp
#include "MapStaticBatchObject.h"

#include "GameInstance.h"
#include "MapAssetRenderUtils.h"
#include "Model.h"
#include "Profiler.h"
#include "Shader.h"

#include <algorithm>
#include <cstring>

CMapStaticBatchObject::CMapStaticBatchObject(
    ComPtr<ID3D11Device> device,
    ComPtr<ID3D11DeviceContext> context)
    : CGameObject(device, context)
{
}

CMapStaticBatchObject::CMapStaticBatchObject(
    const CMapStaticBatchObject& rhs)
    : CGameObject(rhs)
{
}

HRESULT CMapStaticBatchObject::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMapStaticBatchObject::Initialize(void* arg)
{
    if (nullptr == arg || FAILED(__super::Initialize(arg)))
        return E_INVALIDARG;
    const DESC& desc = *static_cast<DESC*>(arg);
    if (desc.AssetId.empty() || desc.ModelPrototypeTag.empty() ||
        desc.Instances.empty() ||
        MAP_ASSET_RENDER_MODE::DEFERRED != desc.RenderProfile.renderMode)
        return E_INVALIDARG;

    m_AssetId = desc.AssetId;
    m_RenderProfile = desc.RenderProfile;
    m_Mirrored = desc.Mirrored;
    m_Instances = desc.Instances;
    if (FAILED(__super::Add_Component(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_Component_Shader_VtxMeshMapInstance"),
        TEXT("Com_Shader"), m_pShaderCom)) ||
        FAILED(__super::Add_Component(
            ETOUI(LEVEL::ASSET_TEST), desc.ModelPrototypeTag,
            TEXT("Com_Model"), m_pModelCom)) ||
        FAILED(Ensure_InstanceCapacity(
            static_cast<uint32_t>(m_Instances.size()))))
        return E_FAIL;
    Rebuild_PlacementLookup();
    return S_OK;
}

void CMapStaticBatchObject::Priority_Update(f32_t timeDelta)
{
    UNREFERENCED_PARAMETER(timeDelta);
}

void CMapStaticBatchObject::Update(f32_t timeDelta)
{
    m_fElapsedTime += timeDelta;
}

void CMapStaticBatchObject::Late_Update(f32_t timeDelta)
{
    UNREFERENCED_PARAMETER(timeDelta);
    if (SUCCEEDED(Upload_VisibleInstances()) &&
        !m_VisibleInstances.empty())
    {
        CGameInstance::Get().Add_RenderObject(
            RENDERGROUP::NONBLEND,
            static_pointer_cast<CGameObject>(shared_from_this()));
    }
}

HRESULT CMapStaticBatchObject::Render()
{
    if (m_VisibleInstances.empty())
        return S_OK;
    if (FAILED(CGameInstance::Get().Bind_Transform(
        m_pShaderCom, "g_ViewMatrix", D3DTS::VIEW)) ||
        FAILED(CGameInstance::Get().Bind_Transform(
            m_pShaderCom, "g_ProjMatrix", D3DTS::PROJ)))
        return E_FAIL;

    const uint32_t pass = CMapAssetRenderUtils::Select_Pass(
        m_RenderProfile, m_Mirrored);
    for (uint32_t mesh = 0; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
    {
        if (FAILED(CMapAssetRenderUtils::Bind_Material(
            m_pModelCom, m_pShaderCom, mesh,
            m_RenderProfile, m_fElapsedTime)) ||
            FAILED(m_pShaderCom->Begin(pass)) ||
            FAILED(m_pModelCom->Render_Instanced(
                mesh, m_pInstanceBuffer.Get(),
                sizeof(VTXMESH_INSTANCE),
                static_cast<uint32_t>(m_VisibleInstances.size()))))
            return E_FAIL;
    }
    return S_OK;
}

HRESULT CMapStaticBatchObject::Update_Instance(
    uint64_t placementId,
    const FMapStaticInstance& instance)
{
    const auto found = m_PlacementLookup.find(placementId);
    if (found == m_PlacementLookup.end() ||
        instance.PlacementId != placementId)
        return E_INVALIDARG;
    m_Instances[found->second] = instance;
    return S_OK;
}

HRESULT CMapStaticBatchObject::Set_InstanceVisible(
    uint64_t placementId, bool visible)
{
    const auto found = m_PlacementLookup.find(placementId);
    if (found == m_PlacementLookup.end())
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    m_Instances[found->second].Visible = visible;
    return S_OK;
}

uint32_t CMapStaticBatchObject::Get_VisibleInstanceCount() const noexcept
{
    return static_cast<uint32_t>(m_VisibleInstances.size());
}

const std::string& CMapStaticBatchObject::Get_AssetId() const noexcept
{
    return m_AssetId;
}

bool CMapStaticBatchObject::Is_Mirrored() const noexcept
{
    return m_Mirrored;
}

HRESULT CMapStaticBatchObject::Ready_Components()
{
    return S_OK;
}

HRESULT CMapStaticBatchObject::Ensure_InstanceCapacity(uint32_t requiredCount)
{
    if (0 == requiredCount)
        return E_INVALIDARG;
    if (requiredCount <= m_InstanceCapacity && nullptr != m_pInstanceBuffer)
        return S_OK;
    uint32_t capacity = 1;
    while (capacity < requiredCount)
        capacity <<= 1u;

    D3D11_BUFFER_DESC desc{};
    desc.ByteWidth = capacity * sizeof(VTXMESH_INSTANCE);
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ComPtr<ID3D11Buffer> staged;
    if (FAILED(m_pDevice->CreateBuffer(&desc, nullptr, &staged)))
        return E_FAIL;
    m_pInstanceBuffer = std::move(staged);
    m_InstanceCapacity = capacity;
    m_VisibleInstances.reserve(capacity);
    return S_OK;
}

HRESULT CMapStaticBatchObject::Upload_VisibleInstances()
{
    m_VisibleInstances.clear();
    for (const FMapStaticInstance& instance : m_Instances)
    {
        if (!instance.Visible ||
            !CGameInstance::Get().isIn_Frustum_InWorldSpace(
                XMLoadFloat3(&instance.WorldBoundsCenter),
                instance.WorldBoundsRadius))
            continue;
        VTXMESH_INSTANCE gpu{};
        gpu.vRight = float4_t(
            instance.World._11, instance.World._12,
            instance.World._13, instance.World._14);
        gpu.vUp = float4_t(
            instance.World._21, instance.World._22,
            instance.World._23, instance.World._24);
        gpu.vLook = float4_t(
            instance.World._31, instance.World._32,
            instance.World._33, instance.World._34);
        gpu.vTranslation = float4_t(
            instance.World._41, instance.World._42,
            instance.World._43, instance.World._44);
        m_VisibleInstances.push_back(gpu);
    }
    if (m_VisibleInstances.empty())
        return S_OK;
    if (FAILED(Ensure_InstanceCapacity(
        static_cast<uint32_t>(m_VisibleInstances.size()))))
        return E_FAIL;

    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (FAILED(m_pContext->Map(
        m_pInstanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD,
        0, &mapped)))
        return E_FAIL;
    std::memcpy(mapped.pData, m_VisibleInstances.data(),
        m_VisibleInstances.size() * sizeof(VTXMESH_INSTANCE));
    m_pContext->Unmap(m_pInstanceBuffer.Get(), 0);

    if (CProfiler* profiler = CGameInstance::Get().Get_Profiler())
        profiler->Add_Counter(
            EProfilerCounter::MapVisibleInstances,
            m_VisibleInstances.size());
    return S_OK;
}

void CMapStaticBatchObject::Rebuild_PlacementLookup()
{
    m_PlacementLookup.clear();
    m_PlacementLookup.reserve(m_Instances.size());
    for (uint32_t index = 0; index < m_Instances.size(); ++index)
        m_PlacementLookup.emplace(m_Instances[index].PlacementId, index);
}

unique_ptr<CMapStaticBatchObject> CMapStaticBatchObject::Create(
    ComPtr<ID3D11Device> device,
    ComPtr<ID3D11DeviceContext> context)
{
    auto instance = unique_ptr<CMapStaticBatchObject>(
        new CMapStaticBatchObject(device, context));
    if (FAILED(instance->Initialize_Prototype()))
        return nullptr;
    return instance;
}

shared_ptr<CPrototype> CMapStaticBatchObject::Clone(void* arg)
{
    auto instance = shared_ptr<CMapStaticBatchObject>(
        new CMapStaticBatchObject(*this));
    if (FAILED(instance->Initialize(arg)))
        return nullptr;
    return instance;
}
```

### 5-6. capture JSON과 Debug profiler panel

#### `Client/Public/ProfilerCaptureIO.h` 전체 내용

```cpp
#pragma once

#include "Client_Defines.h"
#include "Profiler.h"

#include <filesystem>
#include <string>

NS_BEGIN(Client)

struct PROFILER_CAPTURE_METADATA final
{
    std::string configuration;
    std::string gitCommit;
    std::string level;
    std::string mapId;
    uint32_t warmupFrames = 0;
    uint32_t captureFrames = 0;
};

class CProfilerCaptureIO final
{
public:
    static HRESULT Write_Json(
        const Engine::FProfilerCaptureSnapshot& snapshot,
        const PROFILER_CAPTURE_METADATA& metadata,
        const std::filesystem::path& outputPath);
};

NS_END
```

#### `Client/Private/ProfilerCaptureIO.cpp` 전체 내용

```cpp
#include "ProfilerCaptureIO.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <vector>

using namespace Client;

namespace
{
    std::string Escape(std::string_view value)
    {
        std::ostringstream output;
        for (const unsigned char character : value)
        {
            switch (character)
            {
            case '"': output << "\\\""; break;
            case '\\': output << "\\\\"; break;
            case '\b': output << "\\b"; break;
            case '\f': output << "\\f"; break;
            case '\n': output << "\\n"; break;
            case '\r': output << "\\r"; break;
            case '\t': output << "\\t"; break;
            default:
                if (character < 0x20)
                    output << "\\u" << std::hex << std::setw(4) <<
                        std::setfill('0') << static_cast<uint32_t>(character) <<
                        std::dec;
                else
                    output << static_cast<char>(character);
                break;
            }
        }
        return output.str();
    }

    double Percentile(std::vector<double> values, double percentile)
    {
        if (values.empty())
            return 0.0;
        std::sort(values.begin(), values.end());
        const double position = percentile *
            static_cast<double>(values.size() - 1);
        const size_t lower = static_cast<size_t>(position);
        const size_t upper = (std::min)(lower + 1, values.size() - 1);
        const double fraction = position - static_cast<double>(lower);
        return values[lower] * (1.0 - fraction) + values[upper] * fraction;
    }

    double ScopeMilliseconds(
        const Engine::FProfilerFrame& frame,
        const Engine::FProfilerCaptureSnapshot& snapshot,
        std::string_view name,
        double ticksPerSecond)
    {
        double total = 0.0;
        for (const Engine::FProfilerScopeSample& scope : frame.CpuScopes)
        {
            if (scope.NameId >= snapshot.ScopeNames.size() ||
                snapshot.ScopeNames[scope.NameId] != name ||
                scope.EndTick < scope.BeginTick)
                continue;
            total += static_cast<double>(scope.EndTick - scope.BeginTick) *
                1000.0 / ticksPerSecond;
        }
        return total;
    }

    uint64_t Counter(
        const Engine::FProfilerFrame& frame,
        Engine::EProfilerCounter counter)
    {
        return frame.Counters[static_cast<size_t>(counter)];
    }

    std::map<std::string, double> ScopeBreakdownMilliseconds(
        const Engine::FProfilerFrame& frame,
        const Engine::FProfilerCaptureSnapshot& snapshot,
        double ticksPerSecond)
    {
        std::map<std::string, double> totals;
        for (const Engine::FProfilerScopeSample& scope : frame.CpuScopes)
        {
            if (scope.NameId >= snapshot.ScopeNames.size() ||
                scope.EndTick < scope.BeginTick)
                continue;
            totals[snapshot.ScopeNames[scope.NameId]] +=
                static_cast<double>(scope.EndTick - scope.BeginTick) *
                1000.0 / ticksPerSecond;
        }
        return totals;
    }
}

HRESULT CProfilerCaptureIO::Write_Json(
    const Engine::FProfilerCaptureSnapshot& snapshot,
    const PROFILER_CAPTURE_METADATA& metadata,
    const std::filesystem::path& outputPath)
{
    if (outputPath.empty())
        return E_INVALIDARG;
    std::error_code error;
    std::filesystem::create_directories(outputPath.parent_path(), error);
    if (error)
        return E_FAIL;

    LARGE_INTEGER frequency{};
    if (!QueryPerformanceFrequency(&frequency) || 0 == frequency.QuadPart)
        return E_FAIL;
    const double ticksPerSecond = static_cast<double>(frequency.QuadPart);

    const std::filesystem::path temporary =
        outputPath.wstring() + L".tmp";
    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
    if (!output)
        return E_FAIL;
    output << std::fixed << std::setprecision(6);
    output << "{\n";
    output << "  \"schema\": \"LostArkProfilerTimeline.v1\",\n";
    output << "  \"build\": {\"configuration\": \"" <<
        Escape(metadata.configuration) << "\", \"gitCommit\": \"" <<
        Escape(metadata.gitCommit) << "\"},\n";
    output << "  \"scene\": {\"level\": \"" <<
        Escape(metadata.level) << "\", \"mapId\": \"" <<
        Escape(metadata.mapId) << "\"},\n";
    output << "  \"capture\": {\"warmupFrames\": " <<
        metadata.warmupFrames << ", \"frameCount\": " <<
        snapshot.Frames.size() << ", \"requestedFrameCount\": " <<
        metadata.captureFrames << "},\n";
    output << "  \"dropped\": {\"cpuScopes\": " <<
        snapshot.DroppedCpuScopes << ", \"gpuFrames\": " <<
        snapshot.DroppedGpuFrames << "},\n";
    output << "  \"frames\": [\n";

    std::vector<double> cpuFrames;
    std::vector<double> gpuFrames;
    cpuFrames.reserve(snapshot.Frames.size());
    gpuFrames.reserve(snapshot.Frames.size());
    for (size_t index = 0; index < snapshot.Frames.size(); ++index)
    {
        const Engine::FProfilerFrame& frame = snapshot.Frames[index];
        cpuFrames.push_back(frame.CpuFrameMs);
        if (frame.GpuValid)
            gpuFrames.push_back(frame.GpuFrameMs);
        const double updateMs = ScopeMilliseconds(
            frame, snapshot, "Client.Update", ticksPerSecond);
        const double renderMs = ScopeMilliseconds(
            frame, snapshot, "Client.Render", ticksPerSecond);
        const std::map<std::string, double> scopeBreakdown =
            ScopeBreakdownMilliseconds(frame, snapshot, ticksPerSecond);

        output << "    {\"frame\": " << frame.FrameNumber <<
            ", \"cpuMs\": {\"frame\": " << frame.CpuFrameMs <<
            ", \"update\": " << updateMs <<
            ", \"render\": " << renderMs << "}, ";
        output << "\"cpuScopes\": {";
        bool firstScope = true;
        for (const auto& [name, milliseconds] : scopeBreakdown)
        {
            if (!firstScope)
                output << ", ";
            firstScope = false;
            output << "\"" << Escape(name) << "\": " << milliseconds;
        }
        output << "}, ";
        output << "\"gpuMs\": {\"frame\": " << frame.GpuFrameMs <<
            ", \"valid\": " << (frame.GpuValid ? "true" : "false") <<
            ", \"latencyFrames\": " << frame.GpuLatencyFrames << "}, ";
        output << "\"draw\": {\"calls\": " <<
            Counter(frame, Engine::EProfilerCounter::DrawCalls) <<
            ", \"instancedCalls\": " <<
            Counter(frame, Engine::EProfilerCounter::InstancedDrawCalls) <<
            ", \"instances\": " <<
            Counter(frame, Engine::EProfilerCounter::Instances) <<
            ", \"indices\": " <<
            Counter(frame, Engine::EProfilerCounter::Indices) << "}, ";
        output << "\"pipeline\": {\"iaVertices\": " <<
            frame.Pipeline.IAVertices << ", \"iaPrimitives\": " <<
            frame.Pipeline.IAPrimitives << ", \"vsInvocations\": " <<
            frame.Pipeline.VSInvocations << ", \"psInvocations\": " <<
            frame.Pipeline.PSInvocations << "}, ";
        output << "\"map\": {\"placements\": " <<
            Counter(frame, Engine::EProfilerCounter::MapPlacements) <<
            ", \"visibleInstances\": " <<
            Counter(frame, Engine::EProfilerCounter::MapVisibleInstances) <<
            ", \"batchCount\": " <<
            Counter(frame, Engine::EProfilerCounter::MapBatchCount) <<
            ", \"fallbackObjects\": " <<
            Counter(frame, Engine::EProfilerCounter::MapFallbackObjects) <<
            "}, ";
        output << "\"texture\": {\"requests\": " <<
            Counter(frame, Engine::EProfilerCounter::TextureRequests) <<
            ", \"pathHits\": " <<
            Counter(frame, Engine::EProfilerCounter::TexturePathHits) <<
            ", \"contentHits\": " <<
            Counter(frame, Engine::EProfilerCounter::TextureContentHits) <<
            ", \"uniqueSrvs\": " <<
            Counter(frame, Engine::EProfilerCounter::TextureUniqueSrvs) <<
            ", \"estimatedGpuBytes\": " <<
            Counter(frame,
                Engine::EProfilerCounter::TextureEstimatedGpuBytes) << "}, ";
        output << "\"navigation\": {\"queries\": " <<
            Counter(frame, Engine::EProfilerCounter::NavigationQueries) <<
            ", \"expandedNodes\": " <<
            Counter(frame,
                Engine::EProfilerCounter::NavigationExpandedNodes) <<
            ", \"queryUs\": " <<
            Counter(frame,
                Engine::EProfilerCounter::NavigationQueryMicroseconds) <<
            ", \"pathCells\": " <<
            Counter(frame, Engine::EProfilerCounter::NavigationPathCells) <<
            "}}" << (index + 1 == snapshot.Frames.size() ? "\n" : ",\n");
    }
    output << "  ],\n";
    output << "  \"summary\": {\"frameP50Ms\": " <<
        Percentile(cpuFrames, 0.50) << ", \"frameP95Ms\": " <<
        Percentile(cpuFrames, 0.95) << ", \"frameP99Ms\": " <<
        Percentile(cpuFrames, 0.99) << ", \"gpuP95Ms\": " <<
        Percentile(gpuFrames, 0.95) << "}\n";
    output << "}\n";
    output.flush();
    const bool good = output.good();
    output.close();
    if (!good || !MoveFileExW(
        temporary.c_str(), outputPath.c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        std::filesystem::remove(temporary, error);
        return E_FAIL;
    }
    return S_OK;
}
```

#### `Client/Public/ProfilerPanel.h` 전체 내용

```cpp
#pragma once

#include "Client_Defines.h"
#include "Profiler.h"
#include <atomic>
#include <filesystem>
#include <future>

NS_BEGIN(Client)

class CProfilerPanel final
{
public:
    HRESULT Initialize();
    void Toggle();
    void Set_Open(bool open);
    bool Is_Open() const noexcept;
    void Render();
    void Poll_SaveTask();
    HRESULT Save_CurrentCapture(const std::filesystem::path& explicitPath = {});

private:
    static std::filesystem::path Make_TimestampPath();

private:
    bool m_Open = false;
    std::future<HRESULT> m_SaveTask;
    std::atomic_bool m_SaveInProgress = false;
    std::filesystem::path m_LastSavedPath;
};

NS_END
```

#### `Client/Private/ProfilerPanel.cpp` 전체 내용

```cpp
#include "ProfilerPanel.h"

#include "GameInstance.h"
#include "ProfilerCaptureIO.h"
#include "imgui.h"

#include <algorithm>
#include <chrono>
#include <format>
#include <limits>
#include <string>
#include <vector>

using namespace Client;

namespace
{
    struct SCOPE_ROW final
    {
        std::string Name;
        double Milliseconds = 0.0;
        uint32_t Depth = 0;
    };

    uint64_t Counter(
        const Engine::FProfilerFrame& frame,
        Engine::EProfilerCounter counter)
    {
        return frame.Counters[static_cast<size_t>(counter)];
    }

    std::vector<SCOPE_ROW> Build_ScopeRows(
        const Engine::FProfilerFrame& frame,
        const Engine::FProfilerCaptureSnapshot& snapshot)
    {
        LARGE_INTEGER frequency{};
        if (!QueryPerformanceFrequency(&frequency) || frequency.QuadPart <= 0)
            return {};
        std::vector<SCOPE_ROW> rows;
        rows.reserve(frame.CpuScopes.size());
        for (const Engine::FProfilerScopeSample& scope : frame.CpuScopes)
        {
            if (scope.NameId >= snapshot.ScopeNames.size() ||
                scope.EndTick < scope.BeginTick)
                continue;
            const std::string& name = snapshot.ScopeNames[scope.NameId];
            const double milliseconds =
                static_cast<double>(scope.EndTick - scope.BeginTick) *
                1000.0 / static_cast<double>(frequency.QuadPart);
            const auto existing = std::find_if(
                rows.begin(), rows.end(),
                [&](const SCOPE_ROW& row)
                {
                    return row.Name == name && row.Depth == scope.Depth;
                });
            if (existing != rows.end())
                existing->Milliseconds += milliseconds;
            else
                rows.push_back(SCOPE_ROW{ name, milliseconds, scope.Depth });
        }
        std::sort(rows.begin(), rows.end(),
            [](const SCOPE_ROW& left, const SCOPE_ROW& right)
            {
                if (left.Milliseconds != right.Milliseconds)
                    return left.Milliseconds > right.Milliseconds;
                if (left.Depth != right.Depth)
                    return left.Depth < right.Depth;
                return left.Name < right.Name;
            });
        return rows;
    }
}

HRESULT CProfilerPanel::Initialize()
{
    std::error_code error;
    std::filesystem::create_directories(
        L"../Bin/Profiles", error);
    return error ? E_FAIL : S_OK;
}

void CProfilerPanel::Toggle()
{
    Set_Open(!m_Open);
}

void CProfilerPanel::Set_Open(bool open)
{
    if (m_Open == open)
        return;
    Engine::CProfiler* profiler =
        CGameInstance::Get().Get_Profiler();
    if (nullptr == profiler)
        return;
    if (open)
    {
        profiler->Set_Enabled(false);
        profiler->Reset_History();
        profiler->Set_Enabled(true);
        m_Open = true;
    }
    else
    {
        Save_CurrentCapture();
        profiler->Set_Enabled(false);
        m_Open = false;
    }
}

bool CProfilerPanel::Is_Open() const noexcept
{
    return m_Open;
}

void CProfilerPanel::Render()
{
    if (!m_Open)
        return;
    Engine::CProfiler* profiler =
        CGameInstance::Get().Get_Profiler();
    if (nullptr == profiler)
        return;
    const Engine::FProfilerCaptureSnapshot snapshot = profiler->Snapshot();
    bool_t requestedOpen = m_Open;
    if (!ImGui::Begin("LostArk Profiler", &requestedOpen))
    {
        ImGui::End();
        if (!requestedOpen)
            Set_Open(false);
        return;
    }

    ImGui::Text("Frames: %zu | dropped CPU scopes: %llu | dropped GPU: %llu",
        snapshot.Frames.size(),
        static_cast<unsigned long long>(snapshot.DroppedCpuScopes),
        static_cast<unsigned long long>(snapshot.DroppedGpuFrames));
    if (!snapshot.Frames.empty())
    {
        const Engine::FProfilerFrame& frame = snapshot.Frames.back();
        ImGui::Text("CPU %.3f ms | GPU %s %.3f ms | draw %llu | instances %llu",
            frame.CpuFrameMs, frame.GpuValid ? "" : "pending",
            frame.GpuFrameMs,
            static_cast<unsigned long long>(Counter(
                frame, Engine::EProfilerCounter::DrawCalls)),
            static_cast<unsigned long long>(Counter(
                frame, Engine::EProfilerCounter::Instances)));

        const size_t first = snapshot.Frames.size() > 240 ?
            snapshot.Frames.size() - 240 : 0;
        std::vector<float> cpu;
        std::vector<float> gpu;
        std::vector<float> draw;
        cpu.reserve(snapshot.Frames.size() - first);
        gpu.reserve(snapshot.Frames.size() - first);
        draw.reserve(snapshot.Frames.size() - first);
        float maximumMilliseconds = 1.f;
        float maximumDrawCalls = 1.f;
        for (size_t index = first; index < snapshot.Frames.size(); ++index)
        {
            const Engine::FProfilerFrame& sample = snapshot.Frames[index];
            const float cpuValue = static_cast<float>(sample.CpuFrameMs);
            const float gpuValue = sample.GpuValid ?
                static_cast<float>(sample.GpuFrameMs) : 0.f;
            const float drawValue = static_cast<float>(Counter(
                sample, Engine::EProfilerCounter::DrawCalls));
            cpu.push_back(cpuValue);
            gpu.push_back(gpuValue);
            draw.push_back(drawValue);
            maximumMilliseconds = (std::max)(
                maximumMilliseconds, (std::max)(cpuValue, gpuValue));
            maximumDrawCalls = (std::max)(maximumDrawCalls, drawValue);
        }
        ImGui::PlotLines(
            "CPU frame ms", cpu.data(), static_cast<int>(cpu.size()),
            0, nullptr, 0.f, maximumMilliseconds * 1.1f,
            ImVec2(0.f, 72.f));
        ImGui::PlotLines(
            "GPU frame ms", gpu.data(), static_cast<int>(gpu.size()),
            0, "0 = unresolved", 0.f, maximumMilliseconds * 1.1f,
            ImVec2(0.f, 72.f));
        ImGui::PlotLines(
            "Draw calls", draw.data(), static_cast<int>(draw.size()),
            0, nullptr, 0.f, maximumDrawCalls * 1.1f,
            ImVec2(0.f, 72.f));

        if (ImGui::CollapsingHeader(
            "CPU scopes", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const std::vector<SCOPE_ROW> rows =
                Build_ScopeRows(frame, snapshot);
            if (ImGui::BeginTable(
                "ProfilerScopeTable", 3,
                ImGuiTableFlags_Borders |
                ImGuiTableFlags_RowBg |
                ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("Scope");
                ImGui::TableSetupColumn("Depth");
                ImGui::TableSetupColumn("Milliseconds");
                ImGui::TableHeadersRow();
                const size_t visibleCount = (std::min)(rows.size(), size_t(32));
                for (size_t index = 0; index < visibleCount; ++index)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(rows[index].Name.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", rows[index].Depth);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%.3f", rows[index].Milliseconds);
                }
                ImGui::EndTable();
            }
        }

        if (ImGui::CollapsingHeader(
            "Counters", ImGuiTreeNodeFlags_DefaultOpen) &&
            ImGui::BeginTable(
                "ProfilerCounterTable", 2,
                ImGuiTableFlags_Borders |
                ImGuiTableFlags_RowBg |
                ImGuiTableFlags_SizingStretchProp))
        {
            struct COUNTER_ROW final
            {
                const char* Name;
                Engine::EProfilerCounter Counter;
            };
            constexpr COUNTER_ROW rows[] =
            {
                { "Draw calls", Engine::EProfilerCounter::DrawCalls },
                { "Instanced draw calls", Engine::EProfilerCounter::InstancedDrawCalls },
                { "Instances", Engine::EProfilerCounter::Instances },
                { "Indices", Engine::EProfilerCounter::Indices },
                { "Map placements", Engine::EProfilerCounter::MapPlacements },
                { "Visible map instances", Engine::EProfilerCounter::MapVisibleInstances },
                { "Map batches", Engine::EProfilerCounter::MapBatchCount },
                { "Map fallback objects", Engine::EProfilerCounter::MapFallbackObjects },
                { "Texture requests", Engine::EProfilerCounter::TextureRequests },
                { "Texture path hits", Engine::EProfilerCounter::TexturePathHits },
                { "Texture content hits", Engine::EProfilerCounter::TextureContentHits },
                { "Unique SRVs", Engine::EProfilerCounter::TextureUniqueSrvs },
                { "Texture GPU bytes", Engine::EProfilerCounter::TextureEstimatedGpuBytes },
                { "Navigation queries", Engine::EProfilerCounter::NavigationQueries },
                { "Navigation expanded nodes", Engine::EProfilerCounter::NavigationExpandedNodes },
                { "Navigation query us", Engine::EProfilerCounter::NavigationQueryMicroseconds },
                { "Navigation path cells", Engine::EProfilerCounter::NavigationPathCells },
            };
            ImGui::TableSetupColumn("Counter");
            ImGui::TableSetupColumn("Value");
            ImGui::TableHeadersRow();
            for (const COUNTER_ROW& row : rows)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(row.Name);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%llu", static_cast<unsigned long long>(
                    Counter(frame, row.Counter)));
            }
            ImGui::EndTable();
        }

        if (frame.GpuValid)
            ImGui::Text("GPU query latency: %u frames",
                frame.GpuLatencyFrames);
        else
            ImGui::TextDisabled(
                "GPU query unresolved; no context Flush is issued.");
    }
    if (ImGui::Button("Save JSON"))
        Save_CurrentCapture();
    ImGui::SameLine();
    ImGui::BeginDisabled(m_SaveInProgress.load());
    if (ImGui::Button("Reset"))
        profiler->Reset_History();
    ImGui::EndDisabled();
    if (!m_LastSavedPath.empty())
        ImGui::TextWrapped("Last: %ls", m_LastSavedPath.c_str());
    ImGui::End();
    if (!requestedOpen)
        Set_Open(false);
}

void CProfilerPanel::Poll_SaveTask()
{
    if (!m_SaveTask.valid() ||
        m_SaveTask.wait_for(std::chrono::seconds(0)) !=
            std::future_status::ready)
        return;
    m_SaveTask.get();
    m_SaveInProgress.store(false);
}

HRESULT CProfilerPanel::Save_CurrentCapture(
    const std::filesystem::path& explicitPath)
{
    if (m_SaveInProgress.exchange(true))
        return HRESULT_FROM_WIN32(ERROR_BUSY);
    Engine::CProfiler* profiler =
        CGameInstance::Get().Get_Profiler();
    if (nullptr == profiler)
    {
        m_SaveInProgress.store(false);
        return E_UNEXPECTED;
    }
    const Engine::FProfilerCaptureSnapshot snapshot = profiler->Snapshot();
    const std::filesystem::path path = explicitPath.empty() ?
        Make_TimestampPath() : explicitPath;
    m_LastSavedPath = path;
    PROFILER_CAPTURE_METADATA metadata{};
#ifdef _DEBUG
    metadata.configuration = "Debug";
#else
    metadata.configuration = "Release";
#endif
    metadata.gitCommit = "working-tree";
    metadata.level = "ASSET_TEST";
    metadata.mapId = "LV_LUT_HEARTRB_ED";
    metadata.captureFrames = static_cast<uint32_t>(snapshot.Frames.size());
    m_SaveTask = std::async(std::launch::async,
        [snapshot, metadata, path]()
        {
            const HRESULT result = CProfilerCaptureIO::Write_Json(
                snapshot, metadata, path);
            if (SUCCEEDED(result))
            {
                std::error_code error;
                const std::filesystem::path latest =
                    path.parent_path() / L"profiler.json";
                const std::filesystem::path temporary =
                    path.parent_path() / L"profiler.json.tmp";
                std::filesystem::copy_file(path, temporary,
                    std::filesystem::copy_options::overwrite_existing,
                    error);
                if (error || !MoveFileExW(
                    temporary.c_str(), latest.c_str(),
                    MOVEFILE_REPLACE_EXISTING |
                    MOVEFILE_WRITE_THROUGH))
                {
                    std::filesystem::remove(temporary, error);
                    return E_FAIL;
                }
            }
            return result;
        });
    return S_OK;
}

std::filesystem::path CProfilerPanel::Make_TimestampPath()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm local{};
    localtime_s(&local, &time);
    return std::filesystem::path(L"../Bin/Profiles") /
        std::format(L"{:04}-{:02}-{:02}_{:02}-{:02}-{:02}_LostArkProfiler.json",
            local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
            local.tm_hour, local.tm_min, local.tm_sec);
}
```

### 5-7. `Client/Public/ValtanNavigationRuntime.h` 전체 내용

```cpp
#pragma once

#include "Client_Defines.h"
#include "Navigation.h"
#include "Transform.h"
#include <filesystem>

NS_BEGIN(Client)

class CValtanNavigationRuntime final
{
public:
    HRESULT Initialize(
        ComPtr<ID3D11Device> device,
        ComPtr<ID3D11DeviceContext> context,
        const std::filesystem::path& navGridPath,
        const shared_ptr<Engine::CTransform>& agentTransform);
    void Update(f32_t timeDelta);
    void Render_ImGui();
    void Render_Debug();

private:
    bool Pick_Arena(float3_t& outWorld) const;
    void Request_Path();
    void Follow_Path(f32_t timeDelta);
    void Build_DebugGrid();

private:
    ComPtr<ID3D11DeviceContext> m_pContext;
    unique_ptr<Engine::CNavigation> m_pNavigation;
    shared_ptr<Engine::CTransform> m_pAgentTransform;
    unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_pBatch;
    unique_ptr<DirectX::BasicEffect> m_pEffect;
    ComPtr<ID3D11InputLayout> m_pInputLayout;
    vector<DirectX::VertexPositionColor> m_DebugGridLines;
    bool m_ShowGrid = false;
    bool m_PreviousNDown = false;
    bool m_PreviousLeftDown = false;
    float3_t m_Start{};
    float3_t m_Goal{};
    Engine::FPathQueryResult m_LastResult{};
    size_t m_NextWaypoint = 0;
    f32_t m_MoveSpeed = 6.f;
    f32_t m_ArrivalDistance = 0.12f;
};

NS_END
```

#### `Client/Private/ValtanNavigationRuntime.cpp` 전체 내용

```cpp
#include "ValtanNavigationRuntime.h"

#include "GameInstance.h"
#include "Profiler.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>

using namespace Client;

HRESULT CValtanNavigationRuntime::Initialize(
    ComPtr<ID3D11Device> device,
    ComPtr<ID3D11DeviceContext> context,
    const std::filesystem::path& navGridPath,
    const shared_ptr<Engine::CTransform>& agentTransform)
{
    if (nullptr == device || nullptr == context || nullptr == agentTransform)
        return E_INVALIDARG;
    m_pNavigation = Engine::CNavigation::Create_NavGrid(
        device, context, navGridPath);
    if (nullptr == m_pNavigation)
        return E_FAIL;

    m_pContext = context;
    m_pAgentTransform = agentTransform;
    m_pBatch = std::make_unique<DirectX::PrimitiveBatch<
        DirectX::VertexPositionColor>>(context.Get());
    m_pEffect = std::make_unique<DirectX::BasicEffect>(device.Get());
    m_pEffect->SetVertexColorEnabled(true);
    const void* shaderByteCode = nullptr;
    size_t byteCodeLength = 0;
    m_pEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
    if (FAILED(device->CreateInputLayout(
        DirectX::VertexPositionColor::InputElements,
        DirectX::VertexPositionColor::InputElementCount,
        shaderByteCode, byteCodeLength, &m_pInputLayout)))
        return E_FAIL;
    Build_DebugGrid();
    return S_OK;
}

void CValtanNavigationRuntime::Update(f32_t timeDelta)
{
    if (nullptr == m_pNavigation || nullptr == m_pAgentTransform)
        return;

    Follow_Path(timeDelta);

    if (!CGameInstance::Get().IsKeyboardInputBlocked())
    {
        const bool nDown = 0 != (GetAsyncKeyState('N') & 0x8000);
        if (nDown && !m_PreviousNDown)
            m_ShowGrid = !m_ShowGrid;
        m_PreviousNDown = nDown;
    }

    const bool leftDown = 0 != (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
    const bool pressed = leftDown && !m_PreviousLeftDown;
    m_PreviousLeftDown = leftDown;
    if (!pressed || CGameInstance::Get().IsMouseInputBlocked() ||
        CGameInstance::Get().IsKeyboardInputBlocked())
        return;

    float3_t picked{};
    if (!Pick_Arena(picked))
        return;
    XMStoreFloat3(&m_Start,
        m_pAgentTransform->Get_State(STATE::POSITION));
    m_Goal = picked;
    Request_Path();
}

void CValtanNavigationRuntime::Render_ImGui()
{
    if (!ImGui::Begin("Valtan NavGrid"))
    {
        ImGui::End();
        return;
    }
    ImGui::Checkbox("Show grid (N)", &m_ShowGrid);
    ImGui::TextUnformatted("LMB: move agent | N: show walkable grid");
    ImGui::SliderFloat("Move speed", &m_MoveSpeed, 0.5f, 20.f, "%.1f m/s");
    ImGui::Text("result=%u points=%zu expanded=%u query=%llu us",
        static_cast<uint32_t>(m_LastResult.Result),
        m_LastResult.Points.size(), m_LastResult.ExpandedNodes,
        static_cast<unsigned long long>(m_LastResult.QueryMicroseconds));
    ImGui::End();
}

void CValtanNavigationRuntime::Render_Debug()
{
    if (!m_ShowGrid || nullptr == m_pNavigation || nullptr == m_pEffect ||
        nullptr == m_pBatch || nullptr == m_pInputLayout)
        return;
    m_pEffect->SetWorld(XMMatrixIdentity());
    m_pEffect->SetView(XMLoadFloat4x4(
        CGameInstance::Get().Get_Transform(D3DTS::VIEW)));
    m_pEffect->SetProjection(XMLoadFloat4x4(
        CGameInstance::Get().Get_Transform(D3DTS::PROJ)));
    m_pEffect->Apply(m_pContext.Get());
    m_pContext->IASetInputLayout(m_pInputLayout.Get());

    m_pBatch->Begin();
    for (size_t index = 0; index + 1 < m_DebugGridLines.size(); index += 2)
        m_pBatch->DrawLine(
            m_DebugGridLines[index], m_DebugGridLines[index + 1]);
    const float4_t pathColor(0.1f, 1.f, 0.2f, 1.f);
    for (size_t index = 1; index < m_LastResult.Points.size(); ++index)
    {
        float3_t from = m_LastResult.Points[index - 1];
        float3_t to = m_LastResult.Points[index];
        from.y += 0.08f;
        to.y += 0.08f;
        m_pBatch->DrawLine(
            DirectX::VertexPositionColor(from, pathColor),
            DirectX::VertexPositionColor(to, pathColor));
    }
    m_pBatch->End();
}

bool CValtanNavigationRuntime::Pick_Arena(float3_t& outWorld) const
{
    float4_t picked{};
    if (!CGameInstance::Get().Picking(picked))
        return false;
    outWorld = float3_t(picked.x, picked.y, picked.z);
    return std::isfinite(outWorld.x) && std::isfinite(outWorld.y) &&
        std::isfinite(outWorld.z);
}

void CValtanNavigationRuntime::Request_Path()
{
    Engine::FPathQuery query{};
    query.Start = m_Start;
    query.Goal = m_Goal;
    query.NearestRadiusCells = 4;
    query.MaxExpandedNodes = 16384;
    query.Smooth = true;
    Engine::FPathQueryResult staged = m_pNavigation->Find_Path(query);
    const uint64_t stagedPathPointCount = staged.Points.size();
    if (Engine::EPathResult::Success == staged.Result)
    {
        m_LastResult = std::move(staged);
        m_NextWaypoint = m_LastResult.Points.size() > 1 ? 1u : 0u;
    }
    else
    {
        // 실패한 클릭이 현재 이동을 끊지 않도록 기존 Points/cursor는 보존한다.
        m_LastResult.Result = staged.Result;
        m_LastResult.ExpandedNodes = staged.ExpandedNodes;
        m_LastResult.QueryMicroseconds = staged.QueryMicroseconds;
    }

    if (Engine::CProfiler* profiler =
        CGameInstance::Get().Get_Profiler())
    {
        profiler->Add_Counter(
            Engine::EProfilerCounter::NavigationQueries);
        profiler->Add_Counter(
            Engine::EProfilerCounter::NavigationExpandedNodes,
            staged.ExpandedNodes);
        profiler->Add_Counter(
            Engine::EProfilerCounter::NavigationQueryMicroseconds,
            staged.QueryMicroseconds);
        profiler->Add_Counter(
            Engine::EProfilerCounter::NavigationPathCells,
            stagedPathPointCount);
    }
}

void CValtanNavigationRuntime::Follow_Path(f32_t timeDelta)
{
    if (nullptr == m_pAgentTransform || timeDelta <= 0.f ||
        m_NextWaypoint >= m_LastResult.Points.size())
        return;

    float3_t position{};
    XMStoreFloat3(&position,
        m_pAgentTransform->Get_State(STATE::POSITION));

    while (m_NextWaypoint < m_LastResult.Points.size())
    {
        const float3_t target = m_LastResult.Points[m_NextWaypoint];
        const float dx = target.x - position.x;
        const float dy = target.y - position.y;
        const float dz = target.z - position.z;
        const float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

        if (distance <= m_ArrivalDistance)
        {
            position = target;
            ++m_NextWaypoint;
            continue;
        }

        const float moveDistance =
            (std::min)(m_MoveSpeed * timeDelta, distance);
        const float inverseDistance = 1.f / distance;
        position.x += dx * inverseDistance * moveDistance;
        position.y += dy * inverseDistance * moveDistance;
        position.z += dz * inverseDistance * moveDistance;

        const vector_t lookTarget = XMVectorSet(
            target.x, position.y, target.z, 1.f);
        if (std::abs(dx) + std::abs(dz) > 0.0001f)
            m_pAgentTransform->LookAt(lookTarget);
        break;
    }

    m_pAgentTransform->Set_State(
        STATE::POSITION,
        XMVectorSet(position.x, position.y, position.z, 1.f));
}

void CValtanNavigationRuntime::Build_DebugGrid()
{
    m_DebugGridLines.clear();
    const std::shared_ptr<Engine::CNavGrid>& grid =
        m_pNavigation->Get_NavGrid();
    if (nullptr == grid)
        return;
    const Engine::FNavGridDesc& desc = grid->Get_Desc();
    const float4_t color(0.1f, 0.65f, 1.f, 0.35f);
    for (uint32_t index = 0; index < grid->Get_CellCount(); ++index)
    {
        if (!grid->Is_Walkable(index))
            continue;
        const float3_t center = grid->Cell_ToWorld(index);
        const float half = desc.CellSize * 0.5f;
        const float y = center.y + 0.025f;
        const float3_t a(center.x - half, y, center.z - half);
        const float3_t b(center.x + half, y, center.z - half);
        const float3_t c(center.x + half, y, center.z + half);
        const float3_t d(center.x - half, y, center.z + half);
        const float3_t points[8] = { a, b, b, c, c, d, d, a };
        for (const float3_t& point : points)
            m_DebugGridLines.emplace_back(point, color);
    }
}
```

#### `Level_AssetTest` NavGrid 연결 교체 블록

`Level_AssetTest.h`의 class 전방 선언과 private 멤버에 다음을 추가한다.

```cpp
class CValtanNavigationRuntime;

#ifdef _DEBUG
private:
    unique_ptr<CValtanNavigationRuntime> m_pNavigationRuntime;
#endif
```

`Level_AssetTest.cpp` include에 다음을 추가한다.

```cpp
#include "LanceMaster.h"
#include "Transform.h"
#include "ValtanNavigationRuntime.h"
```

`Initialize()`의 `Ready_Valtan()` 성공 뒤, return 전에 다음을 추가한다.

```cpp
#ifdef _DEBUG
shared_ptr<CGameObject> navigationAgent;
CLanceMaster::LANCEMASTER_DESC agentDesc{};
agentDesc.iPrototypeLevelIndex = ETOUI(LEVEL::ASSET_TEST);
agentDesc.vPosition = float3_t(156.279f, 23.242f, -121.977f);
if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
    ETOUI(LEVEL::ASSET_TEST),
    TEXT("Prototype_GameObject_LanceMaster"),
    ETOUI(LEVEL::ASSET_TEST),
    TEXT("Layer_NavigationAgent"),
    &agentDesc,
    &navigationAgent)))
    return E_FAIL;

const shared_ptr<CTransform> agentTransform =
    dynamic_pointer_cast<CTransform>(
        navigationAgent->Get_Component(g_strTransformComTag));
if (nullptr == agentTransform)
    return E_FAIL;

m_pNavigationRuntime = std::make_unique<CValtanNavigationRuntime>();
if (FAILED(m_pNavigationRuntime->Initialize(
    m_pDevice, m_pContext,
    TEXT("../Bin/DataFiles/Navigation/ValtanArena.navgrid"),
    agentTransform)))
    return E_FAIL;
#endif
```

`Update()`와 `Render()`를 다음 전체 함수로 교체한다.

```cpp
void CLevel_AssetTest::Update(f32_t fTimeDelta)
{
    __super::Update(fTimeDelta);
#ifdef _DEBUG
    if (nullptr != m_pNavigationRuntime)
        m_pNavigationRuntime->Update(fTimeDelta);
#endif
}

HRESULT CLevel_AssetTest::Render()
{
    if (FAILED(__super::Render()))
        return E_FAIL;
#ifdef _DEBUG
    SetWindowText(g_hWnd, TEXT("Valtan WModel Asset Test"));
    if (nullptr != m_pNavigationRuntime)
    {
        m_pNavigationRuntime->Render_Debug();
        m_pNavigationRuntime->Render_ImGui();
    }
#endif
    return S_OK;
}
```

### 5-7a. LanceMaster를 AssetTest 이동 주체로 재사용하는 교체 블록

`LanceMaster.h`의 descriptor와 private 상태를 다음으로 교체한다.

```cpp
typedef struct tagLanceMasterDesc : public CContainerObject::CONTAINEROBJECT_DESC
{
    uint32_t iPrototypeLevelIndex = ETOUI(LEVEL::TEST_LEVEL2);
    float3_t vPosition = {};
} LANCEMASTER_DESC;

private:
    uint32_t m_iState = { LANCEMASTER_STATE::IDLE };
    uint32_t m_iPrototypeLevelIndex = ETOUI(LEVEL::TEST_LEVEL2);
```

`CLanceMaster::Initialize()`와 `Ready_PartObjects()`를 다음 전체 함수로 교체한다.

```cpp
HRESULT CLanceMaster::Initialize(void* pArg)
{
    LANCEMASTER_DESC desc{};
    desc.fSpeedPerSec = 0.f;
    desc.fRotationPerSec = 0.f;
    if (nullptr != pArg)
        desc = *static_cast<LANCEMASTER_DESC*>(pArg);
    if (desc.iPrototypeLevelIndex >= ETOUI(LEVEL::END))
        return E_INVALIDARG;

    m_iPrototypeLevelIndex = desc.iPrototypeLevelIndex;
    if (FAILED(__super::Initialize(&desc)))
        return E_FAIL;
    m_pTransformCom->Set_State(STATE::POSITION,
        XMVectorSet(desc.vPosition.x, desc.vPosition.y, desc.vPosition.z, 1.f));
    return Ready_PartObjects();
}

HRESULT CLanceMaster::Ready_PartObjects()
{
    CBody_LanceMaster::BODY_LANCEMASTER_DESC bodyDesc{};
    bodyDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
    bodyDesc.pParentState = &m_iState;
    bodyDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;

    if (FAILED(__super::Add_PartObject(
        m_iPrototypeLevelIndex,
        TEXT("Prototype_GameObject_Body_LanceMaster"),
        TEXT("Part_Body"),
        &bodyDesc)))
        return E_FAIL;

    CWeapon_LanceMaster::WEAPON_LANCEMASTER_DESC weaponDesc{};
    weaponDesc.pParentMatrix = m_pTransformCom->Get_WorldMatrixPtr();
    weaponDesc.pSocketModel = dynamic_pointer_cast<CModel>(
        __super::Get_Component(TEXT("Part_Body"), TEXT("Com_Model")));
    weaponDesc.pSocketBoneName = "b_weapon_rhand";
    weaponDesc.iPrototypeLevelIndex = m_iPrototypeLevelIndex;
    if (nullptr == weaponDesc.pSocketModel)
        return E_FAIL;

    return __super::Add_PartObject(
        m_iPrototypeLevelIndex,
        TEXT("Prototype_GameObject_Weapon_LanceMaster"),
        TEXT("Part_Weapon_R"),
        &weaponDesc);
}
```

`Body_LanceMaster.h` descriptor와 private 상태에 다음을 추가한다.

```cpp
uint32_t iPrototypeLevelIndex = ETOUI(LEVEL::TEST_LEVEL2);
```

```cpp
uint32_t m_iPrototypeLevelIndex = ETOUI(LEVEL::TEST_LEVEL2);
```

`CBody_LanceMaster::Initialize()`에서 `m_pParentState` 대입 직후 다음을 추가한다.

```cpp
m_iPrototypeLevelIndex = pDesc->iPrototypeLevelIndex;
if (m_iPrototypeLevelIndex >= ETOUI(LEVEL::END))
    return E_INVALIDARG;
```

`CBody_LanceMaster::Ready_Components()`의 두 `ETOUI(LEVEL::TEST_LEVEL2)` 인자를 모두
`m_iPrototypeLevelIndex`로 교체한다.

`Weapon_LanceMaster.h` descriptor와 private 상태에도 같은 level 값을 추가한다.

```cpp
uint32_t iPrototypeLevelIndex = ETOUI(LEVEL::TEST_LEVEL2);
```

```cpp
uint32_t m_iPrototypeLevelIndex = ETOUI(LEVEL::TEST_LEVEL2);
```

`CWeapon_LanceMaster::Initialize()`에서 socket 대입 직후 다음을 추가한다.

```cpp
m_iPrototypeLevelIndex = pDesc->iPrototypeLevelIndex;
if (m_iPrototypeLevelIndex >= ETOUI(LEVEL::END))
    return E_INVALIDARG;
```

`CWeapon_LanceMaster::Ready_Components()`의 두 `ETOUI(LEVEL::TEST_LEVEL2)` 인자를 모두
`m_iPrototypeLevelIndex`로 교체한다. 이 변경은 navigation 종속 코드가 아니라 같은
LanceMaster prototype을 어느 level에서 생성해도 자기 level의 part/model prototype을
찾게 만드는 기존 결합 제거다.

`CLoader::Ready_For_Level_AssetTest()`에는 TestLevel2와 동일한 LanceMaster shader/model/body/
weapon/object prototype 등록을 `LEVEL::ASSET_TEST` 키로 추가한다. model 경로와 pre-transform은
TestLevel2 등록과 정확히 동일하게 유지한다.

### 5-8. `CMainApp::UpdateDebugToolShortcut()` 전체 교체 코드

동적 환경 작업 완료 후 현재 함수의 다른 단축키를 보존한 상태에서 아래 전체 블록으로 교체한다.

```cpp
void CMainApp::UpdateDebugToolShortcut()
{
    const bool_t windowFocused =
        IsWindowOwnedByCurrentProcess(GetForegroundWindow());
    const bool_t f1Down = windowFocused &&
        0 != (GetAsyncKeyState(VK_F1) & 0x8000);
    const bool_t f4Down = windowFocused &&
        0 != (GetAsyncKeyState(VK_F4) & 0x8000);

    if (f1Down && !m_bF1Down && nullptr != m_pMapTool)
        m_pMapTool->Toggle();

    if (f4Down && !m_bF4Down && nullptr != m_pProfilerPanel)
        m_pProfilerPanel->Toggle();

    m_bF1Down = f1Down;
    m_bF4Down = f4Down;
    if (nullptr != m_pProfilerPanel)
        m_pProfilerPanel->Poll_SaveTask();
}
```

`CLevel_Logo`의 F2 AssetTest 진입 블록은 수정하지 않는다.

`MainApp.cpp`의 `_DEBUG` include 블록에 다음을 추가한다.

```cpp
#include "ProfilerPanel.h"
```

`Client/Public/MainApp.h`의 `_DEBUG` 도구 멤버 블록에는 다음 선언을 추가한다.

```cpp
private:
    std::unique_ptr<CProfilerPanel> m_pProfilerPanel;
    bool_t m_bF4Down = false;
```

`ReadyDebugTools()`에서 `m_pProfilerPanel = std::make_unique<CProfilerPanel>()` 후 `Initialize()`하고, `Free()`에서 MapTool보다 먼저 reset한다.

`ReadyDebugTools()`에서 ImGui 초기화 성공 직후 다음 블록을 실행한다.

```cpp
m_pProfilerPanel = std::make_unique<CProfilerPanel>();
if (FAILED(m_pProfilerPanel->Initialize()))
    return E_FAIL;
```

`CMainApp::Render()`의 `m_pImGuiLayer` 블록에서 MapTool 렌더 뒤 다음을 추가한다.

```cpp
if (nullptr != m_pProfilerPanel)
    m_pProfilerPanel->Render();
```

### 5-9. `Client/Default/Client.cpp` Release CLI와 frame loop 교체 코드

include에 다음을 추가한다.

```cpp
#include "Profiler.h"
#include "ProfilerCaptureIO.h"
#include <shellapi.h>
#include <filesystem>
#include <string>
#include <vector>
#pragma comment(lib, "shell32.lib")
```

전역 변수 아래에 다음 전체 helper를 추가한다.

```cpp
namespace
{
    struct PROFILE_CAPTURE_STATE final
    {
        bool active = false;
        bool optionsValid = true;
        bool captureStarted = false;
        bool drainingGpuQueries = false;
        uint32_t warmupFrames = 300;
        uint32_t captureFrames = 600;
        uint32_t warmupCompleted = 0;
        uint32_t captureCompleted = 0;
        uint32_t drainFramesCompleted = 0;
        uint32_t sceneWaitFrames = 0;
        std::filesystem::path outputPath =
            L"../Bin/Profiles/release_valtan.json";
    };

    constexpr uint32_t MAX_PROFILE_SCENE_WAIT_FRAMES = 3600;

    bool ParseUInt(
        const wchar_t* value, bool allowZero, uint32_t& output)
    {
        if (nullptr == value || L'\0' == *value)
            return false;
        wchar_t* end = nullptr;
        const unsigned long parsed = wcstoul(value, &end, 10);
        if (nullptr == end || L'\0' != *end ||
            (!allowZero && 0 == parsed) || parsed > UINT32_MAX)
            return false;
        output = static_cast<uint32_t>(parsed);
        return true;
    }

    PROFILE_CAPTURE_STATE ParseProfileCaptureOptions()
    {
        PROFILE_CAPTURE_STATE result{};
        int count = 0;
        LPWSTR* arguments = CommandLineToArgvW(GetCommandLineW(), &count);
        if (nullptr == arguments)
            return result;
        bool sawProfileOption = false;
        for (int index = 1; index < count; ++index)
        {
            const std::wstring_view argument(arguments[index]);
            if (argument == L"--profile-warmup" && index + 1 < count)
            {
                sawProfileOption = true;
                result.optionsValid &= ParseUInt(
                    arguments[++index], true, result.warmupFrames);
            }
            else if (argument == L"--profile-frames" && index + 1 < count)
            {
                sawProfileOption = true;
                result.optionsValid &= ParseUInt(
                    arguments[++index], false, result.captureFrames);
            }
            else if (argument == L"--profile-output" && index + 1 < count)
            {
                sawProfileOption = true;
                result.outputPath = arguments[++index];
                result.optionsValid &= !result.outputPath.empty();
            }
            else if (argument.starts_with(L"--profile-"))
            {
                sawProfileOption = true;
                result.optionsValid = false;
            }
        }
        LocalFree(arguments);
        result.optionsValid &= result.captureFrames <=
            Engine::CProfiler::MAX_HISTORY_FRAMES -
            Engine::CProfiler::GPU_READ_LATENCY;
        result.active = sawProfileOption && result.optionsValid;
        return result;
    }
}
```

`wWinMain()`에서 `CMainApp::Create()` 직전에 다음을 둔다.

```cpp
PROFILE_CAPTURE_STATE profileCapture = ParseProfileCaptureOptions();
if (!profileCapture.optionsValid)
{
    MessageBoxW(
        g_hWnd,
        L"Invalid profiler CLI options or capture frame count exceeds history.",
        L"LostArk Profiler",
        MB_OK | MB_ICONERROR);
    return EXIT_FAILURE;
}
```

기존 메시지 loop 전체를 다음으로 교체한다.

```cpp
MSG msg{};
f32_t timeAccumulator = {};
int exitCode = EXIT_SUCCESS;
Engine::CProfiler* profiler = CGameInstance::Get().Get_Profiler();

while (true)
{
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (WM_QUIT == msg.message)
            break;
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    CGameInstance::Get().Update_TimeDelta(TEXT("Timer_Default"));
    timeAccumulator += CGameInstance::Get().Get_TimeDelta(
        TEXT("Timer_Default"));
    if (timeAccumulator < 1.f / 60.f)
        continue;

    CGameInstance::Get().Update_TimeDelta(TEXT("Timer_60"));
    const f32_t timeDelta = CGameInstance::Get().Get_TimeDelta(
        TEXT("Timer_60"));

    const bool sceneReady = pMainApp->Is_ProfileSceneReady();
    if (profileCapture.active && sceneReady &&
        !profileCapture.captureStarted &&
        profileCapture.warmupCompleted >= profileCapture.warmupFrames)
    {
        profiler->Set_Enabled(false);
        profiler->Reset_History();
        profiler->Set_Enabled(true);
        profileCapture.captureStarted = true;
    }

    profiler->Begin_Frame();
    {
        Engine::CProfilerScope updateScope(profiler, "Client.Update");
        pMainApp->Update(timeDelta);
    }
    HRESULT renderResult = S_OK;
    {
        Engine::CProfilerScope renderScope(profiler, "Client.Render");
        renderResult = pMainApp->Render();
    }
    profiler->End_Frame();
    timeAccumulator = 0.f;
    if (FAILED(renderResult))
    {
        exitCode = EXIT_FAILURE;
        break;
    }

    if (!profileCapture.active)
        continue;
    if (!pMainApp->Is_ProfileSceneReady())
    {
        profiler->Set_Enabled(false);
        if (profileCapture.captureStarted ||
            ++profileCapture.sceneWaitFrames >=
                MAX_PROFILE_SCENE_WAIT_FRAMES)
        {
            exitCode = EXIT_FAILURE;
            break;
        }
        continue;
    }
    profileCapture.sceneWaitFrames = 0;
    if (!profileCapture.captureStarted)
    {
        profiler->Set_Enabled(false);
        ++profileCapture.warmupCompleted;
        continue;
    }

    if (!profileCapture.drainingGpuQueries)
    {
        ++profileCapture.captureCompleted;
        if (profileCapture.captureCompleted <
            profileCapture.captureFrames)
            continue;
        profileCapture.drainingGpuQueries = true;
        continue;
    }
    ++profileCapture.drainFramesCompleted;
    if (profileCapture.drainFramesCompleted <
        Engine::CProfiler::GPU_READ_LATENCY)
        continue;

    profiler->Set_Enabled(false);
    Engine::FProfilerCaptureSnapshot snapshot = profiler->Snapshot();
    if (snapshot.Frames.size() <
        profileCapture.captureFrames +
        profileCapture.drainFramesCompleted)
    {
        exitCode = EXIT_FAILURE;
        break;
    }
    snapshot.Frames.resize(
        snapshot.Frames.size() -
        profileCapture.drainFramesCompleted);
    if (snapshot.Frames.size() > profileCapture.captureFrames)
    {
        snapshot.Frames.erase(
            snapshot.Frames.begin(),
            snapshot.Frames.end() -
                profileCapture.captureFrames);
    }
    PROFILER_CAPTURE_METADATA metadata{};
#ifdef _DEBUG
    metadata.configuration = "Debug";
#else
    metadata.configuration = "Release";
#endif
    metadata.gitCommit = "working-tree";
    metadata.level = "ASSET_TEST";
    metadata.mapId = "LV_LUT_HEARTRB_ED";
    metadata.warmupFrames = profileCapture.warmupFrames;
    metadata.captureFrames = profileCapture.captureFrames;
    if (FAILED(CProfilerCaptureIO::Write_Json(
        snapshot, metadata, profileCapture.outputPath)))
        exitCode = EXIT_FAILURE;
    break;
}

return exitCode;
```

`CMainApp` public 선언에 다음을 추가한다.

```cpp
bool_t Is_ProfileSceneReady() const;
```

`MainApp.cpp`에 다음 전체 함수를 추가한다.

```cpp
bool_t CMainApp::Is_ProfileSceneReady() const
{
    return nullptr != m_pMapSceneRuntime &&
        m_pMapSceneRuntime->Is_Loaded();
}
```

`CMainApp::Initialize()`의 시작 레벨 결정은 다음 블록으로 교체한다.

```cpp
const bool_t profileRun = nullptr != wcsstr(
    GetCommandLineW(), L"--profile-");
if (FAILED(Start_Level(
    profileRun ? LEVEL::ASSET_TEST : LEVEL::LOGO)))
    return E_FAIL;
```

### 5-10. draw wrapper 전체 교체 코드

`Renderer.cpp` include에 다음을 추가한다.

```cpp
#include "Profiler.h"
```

`CRenderer::Add_RenderObject()`를 다음 전체 함수로 교체한다.

```cpp
HRESULT CRenderer::Add_RenderObject(
    RENDERGROUP group,
    shared_ptr<CGameObject> object)
{
    if (nullptr == object || group >= RENDERGROUP::END)
        return E_INVALIDARG;
    m_RenderObjects[ETOUI(group)].push_back(std::move(object));
    if (CProfiler* profiler = CGameInstance::Get().Get_Profiler())
    {
        switch (group)
        {
        case RENDERGROUP::PRIORITY:
            profiler->Add_Counter(
                EProfilerCounter::RenderSubmissionsPriority);
            break;
        case RENDERGROUP::SHADOW:
            profiler->Add_Counter(
                EProfilerCounter::RenderSubmissionsShadow);
            break;
        case RENDERGROUP::NONBLEND:
            profiler->Add_Counter(
                EProfilerCounter::RenderSubmissionsNonBlend);
            break;
        case RENDERGROUP::BLEND:
            profiler->Add_Counter(
                EProfilerCounter::RenderSubmissionsBlend);
            break;
        default:
            break;
        }
    }
    return S_OK;
}
```

`CRenderer::Draw()`를 다음 전체 함수로 교체한다.

```cpp
HRESULT CRenderer::Draw()
{
    CProfiler* profiler = CGameInstance::Get().Get_Profiler();
    {
        CProfilerScope scope(profiler, "Renderer.Priority");
        if (FAILED(Render_Priority())) return E_FAIL;
    }
    {
        CProfilerScope scope(profiler, "Renderer.Shadow");
        if (FAILED(Render_Shadow())) return E_FAIL;
    }
    {
        CProfilerScope scope(profiler, "Renderer.NonBlend");
        if (FAILED(Render_NonBlend())) return E_FAIL;
    }
    {
        CProfilerScope scope(profiler, "Renderer.Lights");
        if (FAILED(Render_Lights())) return E_FAIL;
    }
    {
        CProfilerScope scope(profiler, "Renderer.Combined");
        if (FAILED(Render_Combined())) return E_FAIL;
    }
    {
        CProfilerScope scope(profiler, "Renderer.NonLight");
        if (FAILED(Render_NonLight())) return E_FAIL;
    }
    {
        CProfilerScope scope(profiler, "Renderer.Blend");
        if (FAILED(Render_Blend())) return E_FAIL;
    }
    {
        CProfilerScope scope(profiler, "Renderer.UI");
        if (FAILED(Render_UI())) return E_FAIL;
    }
#ifdef _DEBUG
    {
        CProfilerScope scope(profiler, "Renderer.Debug");
        if (FAILED(Render_Debug())) return E_FAIL;
    }
#endif
    return S_OK;
}
```

`CVIBuffer::Render()`는 기존 IA binding을 유지하고 마지막 draw 블록을 다음으로 교체한다.

`VIBuffer.cpp`와 `VIBuffer_Instance.cpp` include에 다음을 추가한다.

```cpp
#include "GameInstance.h"
#include "Profiler.h"
```

```cpp
m_pContext->DrawIndexed(m_iNumIndices, 0, 0);

if (CProfiler* profiler = CGameInstance::Get().Get_Profiler())
{
    profiler->Add_Counter(EProfilerCounter::DrawCalls);
    profiler->Add_Counter(EProfilerCounter::Indices, m_iNumIndices);
}

return S_OK;
```

`CVIBuffer_Instance::Render()`의 마지막 draw 블록은 다음으로 교체한다.

```cpp
m_pContext->DrawIndexedInstanced(
    m_iNumIndexPerInstance,
    m_iNumInstances,
    0,
    0,
    0);

if (CProfiler* profiler = CGameInstance::Get().Get_Profiler())
{
    profiler->Add_Counter(EProfilerCounter::DrawCalls);
    profiler->Add_Counter(EProfilerCounter::InstancedDrawCalls);
    profiler->Add_Counter(EProfilerCounter::Instances, m_iNumInstances);
    profiler->Add_Counter(
        EProfilerCounter::Indices,
        static_cast<uint64_t>(m_iNumIndexPerInstance) * m_iNumInstances);
}

return S_OK;
```

직접 draw 지점도 같은 규칙으로 계측하되 draw를 중복 집계하지 않는다.

### 5-11. `CNavigation` public 선언 전체 교체 블록

기존 public API를 삭제하지 않고 아래 mode/load/path API를 추가한다.

```cpp
enum class MODE : uint8_t
{
    LEGACY_TRIANGLE,
    NAVGRID_ASTAR
};

HRESULT Load_NavGrid(const std::filesystem::path& path);
FPathQueryResult Find_Path(const FPathQuery& query);
MODE Get_Mode() const noexcept { return m_Mode; }
const std::shared_ptr<CNavGrid>& Get_NavGrid() const noexcept { return m_NavGrid; }

static unique_ptr<CNavigation> Create_NavGrid(
    ComPtr<ID3D11Device> device,
    ComPtr<ID3D11DeviceContext> context,
    const std::filesystem::path& path);

private:
    MODE m_Mode = MODE::LEGACY_TRIANGLE;
    std::shared_ptr<CNavGrid> m_NavGrid;
    std::shared_ptr<CPathFinder> m_PathFinder;
```

`Load_NavGrid()`는 local staged grid/pathfinder 둘 다 성공한 뒤 세 멤버를 한 번에 commit한다. 실패 시 기존 triangle 또는 grid 상태를 그대로 둔다.

`Navigation.h` include에 다음을 추가한다.

```cpp
#include "NavGrid.h"
#include "PathFinder.h"
#include <filesystem>
```

`Navigation.cpp`에 다음 전체 함수를 추가한다.

```cpp
HRESULT CNavigation::Load_NavGrid(const std::filesystem::path& path)
{
    auto stagedGrid = std::make_shared<CNavGrid>();
    if (FAILED(stagedGrid->Load_FromFile(path)))
        return E_FAIL;
    auto stagedPathFinder = std::make_shared<CPathFinder>();
    if (FAILED(stagedPathFinder->Initialize(stagedGrid)))
        return E_FAIL;

    m_NavGrid = std::move(stagedGrid);
    m_PathFinder = std::move(stagedPathFinder);
    m_Mode = MODE::NAVGRID_ASTAR;
    return S_OK;
}

FPathQueryResult CNavigation::Find_Path(const FPathQuery& query)
{
    if (MODE::NAVGRID_ASTAR != m_Mode || nullptr == m_PathFinder)
        return {};
    return m_PathFinder->Find_Path(query);
}

unique_ptr<CNavigation> CNavigation::Create_NavGrid(
    ComPtr<ID3D11Device> device,
    ComPtr<ID3D11DeviceContext> context,
    const std::filesystem::path& path)
{
    auto instance = unique_ptr<CNavigation>(
        new CNavigation(device, context));
    if (FAILED(instance->CComponent::Initialize_Prototype()) ||
        FAILED(instance->Load_NavGrid(path)))
        return nullptr;
    return instance;
}
```

### 5-12. `CMapSceneRuntime` batch/fallback 최종 교체 코드

이 절의 코드는 5-0의 object-per-placement `CMapSceneRuntime`을 최종 runtime 표현으로
전환한다. `MapSceneRuntime.h` include에 다음을 추가한다.

```cpp
#include "MapStaticBatchObject.h"
#include <unordered_map>
```

`CMapSceneRuntime` private 선언과 public count 접근자를 다음 블록으로 교체한다.

```cpp
private:
    struct RUNTIME_ENTRY final
    {
        MAP_PLACEMENT_RECORD record;
        std::wstring layerTag;
        shared_ptr<CMapAssetObject> object;
    };

    struct STATIC_BATCH_ENTRY final
    {
        std::string assetId;
        bool_t mirrored = false;
        shared_ptr<CMapStaticBatchObject> object;
    };

public:
    size_t Get_PlacementCount() const { return m_RecordView.size(); }
    size_t Get_BatchCount() const { return m_StaticBatches.size(); }
    size_t Get_FallbackCount() const { return m_Entries.size(); }

private:
    static bool_t Is_BatchEligible(const MAP_ASSET_ENTRY& asset);
    static HRESULT Build_Instance(
        const MAP_ASSET_ENTRY& asset,
        const shared_ptr<Engine::CModel>& model,
        const MAP_PLACEMENT_RECORD& record,
        FMapStaticInstance& outInstance);
    static HRESULT Stage_StaticBatches(
        const CMapAssetCatalog& catalog,
        const std::vector<MAP_PLACEMENT_RECORD>& records,
        std::vector<STATIC_BATCH_ENTRY>& outBatches,
        std::unordered_map<uint64_t,
            shared_ptr<CMapStaticBatchObject>>& outLookup);
    static void Remove_Batches(
        std::vector<STATIC_BATCH_ENTRY>& batches,
        bool_t removeFromObjectManager);

    MAP_PLACEMENT_RECORD* Find_Record(uint64_t placementId);

private:
    CMapAssetCatalog m_Catalog;
    std::vector<RUNTIME_ENTRY> m_Entries;
    std::vector<STATIC_BATCH_ENTRY> m_StaticBatches;
    std::unordered_map<uint64_t,
        shared_ptr<CMapStaticBatchObject>> m_BatchLookup;
    std::vector<MAP_PLACEMENT_RECORD> m_RecordView;
    std::unique_ptr<CDeployPropRuntime> m_pDeployRuntime;
    MAP_ENVIRONMENT_PHASE m_EnvironmentPhase =
        MAP_ENVIRONMENT_PHASE::BASELINE;
    uint32_t m_CurrentLevelId = UINT32_MAX;
    std::string m_Status = "Map scene runtime not loaded";
    bool_t m_bLoaded = false;
    bool_t m_bDirty = false;
```

`MapSceneRuntime.cpp` include에 다음을 추가한다.

```cpp
#include "Model.h"
#include "Profiler.h"
#include <cmath>
#include <map>
#include <unordered_set>
```

anonymous namespace에 batch 상수를 추가한다.

```cpp
constexpr const wchar_t* MAP_BATCH_PROTOTYPE =
    TEXT("Prototype_GameObject_MapStaticBatch");
constexpr const wchar_t* MAP_BATCH_LAYER =
    TEXT("Layer_MapStaticBatch");
```

`CMapSceneRuntime::Reload()`을 다음 전체 함수로 교체한다.

```cpp
HRESULT CMapSceneRuntime::Reload()
{
    if (ETOUI(LEVEL::ASSET_TEST) != m_CurrentLevelId)
        return E_UNEXPECTED;

    CMapAssetCatalog stagedCatalog;
    if (!stagedCatalog.Load_Default())
    {
        m_Status = stagedCatalog.Get_Status();
        return E_FAIL;
    }
    std::vector<MAP_PLACEMENT_RECORD> stagedRecords;
    std::string documentStatus;
    if (!CMapPlacementDocument::Read(
        stagedCatalog.Get_PlacementPath(), stagedCatalog,
        stagedRecords, documentStatus))
    {
        m_Status = documentStatus;
        return E_FAIL;
    }

    std::vector<RUNTIME_ENTRY> stagedStandalone;
    stagedStandalone.reserve(stagedRecords.size());
    for (const MAP_PLACEMENT_RECORD& record : stagedRecords)
    {
        const MAP_ASSET_ENTRY* asset = stagedCatalog.Find(record.assetId);
        if (nullptr == asset)
        {
            Remove_Entries(stagedStandalone, true);
            m_Status = "Map scene stage lost asset " + record.assetId;
            return E_FAIL;
        }
        if (Is_BatchEligible(*asset))
            continue;
        RUNTIME_ENTRY entry{};
        if (FAILED(Create_Placement(stagedCatalog, record, entry)))
        {
            Remove_Entries(stagedStandalone, true);
            m_Status = "Map scene stage rolled back at " +
                record.sourcePlacementId;
            return E_FAIL;
        }
        stagedStandalone.push_back(std::move(entry));
    }

    std::vector<STATIC_BATCH_ENTRY> stagedBatches;
    std::unordered_map<uint64_t,
        shared_ptr<CMapStaticBatchObject>> stagedBatchLookup;
    if (FAILED(Stage_StaticBatches(
        stagedCatalog, stagedRecords,
        stagedBatches, stagedBatchLookup)))
    {
        Remove_Batches(stagedBatches, true);
        Remove_Entries(stagedStandalone, true);
        m_Status = "Static batch stage rolled back";
        return E_FAIL;
    }

    if (stagedBatchLookup.size() + stagedStandalone.size() !=
        stagedRecords.size())
    {
        Remove_Batches(stagedBatches, true);
        Remove_Entries(stagedStandalone, true);
        m_Status = "Static runtime representation invariant failed";
        return E_FAIL;
    }

    auto stagedDeploy = std::make_unique<CDeployPropRuntime>();
    if (FAILED(stagedDeploy->Load(stagedCatalog.Get_AreaId())))
    {
        m_Status = stagedDeploy->Get_Status();
        stagedDeploy->Unload();
        Remove_Batches(stagedBatches, true);
        Remove_Entries(stagedStandalone, true);
        return E_FAIL;
    }

    if (nullptr != m_pDeployRuntime)
        m_pDeployRuntime->Unload();
    Remove_Batches(m_StaticBatches, true);
    Remove_Entries(m_Entries, true);

    m_Catalog = std::move(stagedCatalog);
    m_RecordView = std::move(stagedRecords);
    m_Entries = std::move(stagedStandalone);
    m_StaticBatches = std::move(stagedBatches);
    m_BatchLookup = std::move(stagedBatchLookup);
    m_pDeployRuntime = std::move(stagedDeploy);
    m_EnvironmentPhase = MAP_ENVIRONMENT_PHASE::BASELINE;
    m_bLoaded = true;
    m_bDirty = false;
    Apply_EnvironmentPhase();
    m_Status = "Map scene ready: " +
        std::to_string(m_RecordView.size()) + " placements, " +
        std::to_string(m_StaticBatches.size()) + " batches, " +
        std::to_string(m_Entries.size()) + " fallbacks, " +
        std::to_string(m_pDeployRuntime->Get_Count()) + " deploy";
    return S_OK;
}
```

`CMapSceneRuntime::Unload()`을 다음 전체 함수로 교체한다.

```cpp
void CMapSceneRuntime::Unload(bool_t removeFromObjectManager)
{
    if (nullptr != m_pDeployRuntime)
        m_pDeployRuntime->Unload(removeFromObjectManager);
    m_pDeployRuntime.reset();
    Remove_Batches(m_StaticBatches, removeFromObjectManager);
    Remove_Entries(m_Entries, removeFromObjectManager);
    m_BatchLookup.clear();
    m_RecordView.clear();
    m_Catalog = CMapAssetCatalog{};
    m_EnvironmentPhase = MAP_ENVIRONMENT_PHASE::BASELINE;
    m_bLoaded = false;
    m_bDirty = false;
}
```

`Add_Placement`, `Remove_Placement`, `Update_Placement`,
`Set_PlacementVisible`, `Save_Placements`를 다음 전체 함수로 교체한다.

```cpp
HRESULT CMapSceneRuntime::Add_Placement(
    const MAP_PLACEMENT_RECORD& record)
{
    if (!m_bLoaded || nullptr != Find_Record(record.placementId) ||
        !CMapPlacementDocument::Is_Valid(record, m_Catalog))
        return E_INVALIDARG;
    RUNTIME_ENTRY entry{};
    if (FAILED(Create_Placement(m_Catalog, record, entry)))
        return E_FAIL;
    m_Entries.push_back(std::move(entry));
    m_RecordView.push_back(record);
    m_bDirty = true;
    Apply_EnvironmentPhase();
    return S_OK;
}

HRESULT CMapSceneRuntime::Remove_Placement(uint64_t placementId)
{
    MAP_PLACEMENT_RECORD* record = Find_Record(placementId);
    if (nullptr == record)
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

    RUNTIME_ENTRY* fallback = Find_Entry(placementId);
    if (nullptr != fallback)
    {
        if (FAILED(CGameInstance::Get().Remove_GameObject_from_Layer(
            ETOUI(LEVEL::ASSET_TEST), fallback->layerTag,
            static_pointer_cast<CGameObject>(fallback->object))))
            return E_FAIL;
        m_Entries.erase(std::remove_if(
            m_Entries.begin(), m_Entries.end(),
            [placementId](const RUNTIME_ENTRY& entry)
            { return entry.record.placementId == placementId; }),
            m_Entries.end());
    }
    else
    {
        const auto batch = m_BatchLookup.find(placementId);
        if (batch == m_BatchLookup.end() ||
            FAILED(batch->second->Set_InstanceVisible(placementId, false)))
            return E_FAIL;
        m_BatchLookup.erase(batch);
    }

    m_RecordView.erase(std::remove_if(
        m_RecordView.begin(), m_RecordView.end(),
        [placementId](const MAP_PLACEMENT_RECORD& value)
        { return value.placementId == placementId; }),
        m_RecordView.end());
    m_bDirty = true;
    return S_OK;
}

HRESULT CMapSceneRuntime::Update_Placement(
    uint64_t placementId,
    const float3_t& position,
    const float4_t& rotationQuaternion,
    const float3_t& signedScale)
{
    MAP_PLACEMENT_RECORD* record = Find_Record(placementId);
    if (nullptr == record)
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    MAP_PLACEMENT_RECORD staged = *record;
    staged.position = position;
    staged.rotationQuaternion = rotationQuaternion;
    staged.signedScale = signedScale;
    if (!CMapPlacementDocument::Is_Valid(staged, m_Catalog))
        return E_INVALIDARG;

    if (RUNTIME_ENTRY* fallback = Find_Entry(placementId))
    {
        fallback->record = staged;
        fallback->object->Set_PlacementTransform(
            position, rotationQuaternion, signedScale);
    }
    else
    {
        const auto batch = m_BatchLookup.find(placementId);
        const MAP_ASSET_ENTRY* asset = m_Catalog.Find(staged.assetId);
        if (batch == m_BatchLookup.end() || nullptr == asset)
            return E_FAIL;
        const bool_t oldMirrored = record->signedScale.x *
            record->signedScale.y * record->signedScale.z < 0.f;
        const bool_t newMirrored = signedScale.x * signedScale.y *
            signedScale.z < 0.f;
        if (oldMirrored != newMirrored)
        {
            RUNTIME_ENTRY migrated{};
            if (FAILED(batch->second->Set_InstanceVisible(
                placementId, false)))
                return E_FAIL;
            if (FAILED(Create_Placement(m_Catalog, staged, migrated)))
            {
                batch->second->Set_InstanceVisible(
                    placementId, record->visible);
                return E_FAIL;
            }
            m_Entries.push_back(std::move(migrated));
            m_BatchLookup.erase(batch);
            *record = staged;
            m_bDirty = true;
            return S_OK;
        }
        shared_ptr<Engine::CModel> model = dynamic_pointer_cast<Engine::CModel>(
            CGameInstance::Get().Clone_Prototype(
                ETOUI(LEVEL::ASSET_TEST), asset->prototypeTag));
        FMapStaticInstance instance{};
        if (nullptr == model || FAILED(Build_Instance(
            *asset, model, staged, instance)) ||
            FAILED(batch->second->Update_Instance(placementId, instance)))
            return E_FAIL;
    }
    *record = staged;
    m_bDirty = true;
    return S_OK;
}

HRESULT CMapSceneRuntime::Set_PlacementVisible(
    uint64_t placementId, bool_t visible)
{
    MAP_PLACEMENT_RECORD* record = Find_Record(placementId);
    if (nullptr == record)
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    record->visible = visible;
    if (RUNTIME_ENTRY* fallback = Find_Entry(placementId))
    {
        fallback->record.visible = visible;
        fallback->object->Set_Visible(visible);
    }
    else
    {
        const auto batch = m_BatchLookup.find(placementId);
        if (batch == m_BatchLookup.end() ||
            FAILED(batch->second->Set_InstanceVisible(placementId, visible)))
            return E_FAIL;
    }
    m_bDirty = true;
    Apply_EnvironmentPhase();
    return S_OK;
}

HRESULT CMapSceneRuntime::Save_Placements()
{
    if (!m_bLoaded)
        return E_UNEXPECTED;
    if (!CMapPlacementDocument::Write(
        m_Catalog.Get_PlacementPath(), m_Catalog.Get_AreaId(),
        m_RecordView, m_Catalog, m_Status))
        return E_FAIL;
    m_bDirty = false;
    return S_OK;
}
```

`Apply_EnvironmentPhase()`를 다음 전체 함수로 교체한다.

```cpp
void CMapSceneRuntime::Apply_EnvironmentPhase()
{
    for (MAP_PLACEMENT_RECORD& record : m_RecordView)
    {
        bool_t visible = record.visible;
        if (record.sourceLevel == "VALTAN_PHASE_SPACEHOLE")
            visible = MAP_ENVIRONMENT_PHASE::BASELINE != m_EnvironmentPhase;
        else if (record.sourceLevel == "VALTAN_PHASE_CHAOSGATE")
            visible = MAP_ENVIRONMENT_PHASE::CHAOS_GATE == m_EnvironmentPhase;

        if (RUNTIME_ENTRY* fallback = Find_Entry(record.placementId))
            fallback->object->Set_Visible(visible);
        else
        {
            const auto batch = m_BatchLookup.find(record.placementId);
            if (batch != m_BatchLookup.end())
                batch->second->Set_InstanceVisible(
                    record.placementId, visible);
        }
    }
}
```

다음 helper 함수 전체를 `MapSceneRuntime.cpp`에 추가한다.

```cpp
bool_t CMapSceneRuntime::Is_BatchEligible(const MAP_ASSET_ENTRY& asset)
{
    return MAP_ASSET_RENDER_MODE::DEFERRED ==
        asset.renderProfile.renderMode;
}

HRESULT CMapSceneRuntime::Build_Instance(
    const MAP_ASSET_ENTRY& asset,
    const shared_ptr<Engine::CModel>& model,
    const MAP_PLACEMENT_RECORD& record,
    FMapStaticInstance& outInstance)
{
    if (nullptr == model || !model->Has_LocalBounds())
        return E_FAIL;
    const float3_t& minimum = model->Get_LocalBoundsMin();
    const float3_t& maximum = model->Get_LocalBoundsMax();
    const float3_t localCenter(
        (minimum.x + maximum.x) * 0.5f,
        (minimum.y + maximum.y) * 0.5f,
        (minimum.z + maximum.z) * 0.5f);
    const vector_t halfExtents = XMVectorSet(
        (maximum.x - minimum.x) * 0.5f,
        (maximum.y - minimum.y) * 0.5f,
        (maximum.z - minimum.z) * 0.5f, 0.f);
    const float localRadius = XMVectorGetX(XMVector3Length(halfExtents));

    const vector_t rotation = XMQuaternionNormalize(
        XMLoadFloat4(&record.rotationQuaternion));
    matrix_t world = XMMatrixScaling(
        record.signedScale.x,
        record.signedScale.y,
        record.signedScale.z) * XMMatrixRotationQuaternion(rotation);
    float3_t origin = record.position;
    if (MAP_ASSET_ANCHOR::BOTTOM_CENTER == asset.anchor)
    {
        const vector_t localAnchor = XMVectorSet(
            localCenter.x, minimum.y, localCenter.z, 1.f);
        float3_t offset{};
        XMStoreFloat3(&offset,
            XMVector3TransformCoord(localAnchor, world));
        origin.x -= offset.x;
        origin.y -= offset.y;
        origin.z -= offset.z;
    }
    world.r[3] = XMVectorSet(origin.x, origin.y, origin.z, 1.f);

    outInstance = {};
    outInstance.PlacementId = record.placementId;
    outInstance.Visible = record.visible;
    XMStoreFloat4x4(&outInstance.World, world);
    XMStoreFloat3(&outInstance.WorldBoundsCenter,
        XMVector3TransformCoord(XMLoadFloat3(&localCenter), world));
    const float maximumScale = (std::max)({
        std::abs(record.signedScale.x),
        std::abs(record.signedScale.y),
        std::abs(record.signedScale.z) });
    outInstance.WorldBoundsRadius = localRadius * maximumScale;
    return S_OK;
}

HRESULT CMapSceneRuntime::Stage_StaticBatches(
    const CMapAssetCatalog& catalog,
    const std::vector<MAP_PLACEMENT_RECORD>& records,
    std::vector<STATIC_BATCH_ENTRY>& outBatches,
    std::unordered_map<uint64_t,
        shared_ptr<CMapStaticBatchObject>>& outLookup)
{
    using KEY = std::pair<std::string, bool_t>;
    std::map<KEY, std::vector<const MAP_PLACEMENT_RECORD*>> groups;
    for (const MAP_PLACEMENT_RECORD& record : records)
    {
        const MAP_ASSET_ENTRY* asset = catalog.Find(record.assetId);
        if (nullptr == asset)
            return E_FAIL;
        if (!Is_BatchEligible(*asset))
            continue;
        const bool_t mirrored = record.signedScale.x *
            record.signedScale.y * record.signedScale.z < 0.f;
        groups[{ record.assetId, mirrored }].push_back(&record);
    }

    for (const auto& [key, placements] : groups)
    {
        const MAP_ASSET_ENTRY* asset = catalog.Find(key.first);
        if (nullptr == asset)
            return E_FAIL;
        shared_ptr<Engine::CModel> model = dynamic_pointer_cast<Engine::CModel>(
            CGameInstance::Get().Clone_Prototype(
                ETOUI(LEVEL::ASSET_TEST), asset->prototypeTag));
        if (nullptr == model)
            return E_FAIL;

        CMapStaticBatchObject::DESC desc{};
        desc.AssetId = asset->id;
        desc.ModelPrototypeTag = asset->prototypeTag;
        desc.RenderProfile = asset->renderProfile;
        desc.Mirrored = key.second;
        desc.Instances.reserve(placements.size());
        for (const MAP_PLACEMENT_RECORD* record : placements)
        {
            FMapStaticInstance instance{};
            if (FAILED(Build_Instance(*asset, model, *record, instance)))
                return E_FAIL;
            desc.Instances.push_back(instance);
        }

        shared_ptr<CGameObject> gameObject;
        if (FAILED(CGameInstance::Get().Add_GameObject_to_Layer(
            ETOUI(LEVEL::ASSET_TEST), MAP_BATCH_PROTOTYPE,
            ETOUI(LEVEL::ASSET_TEST), MAP_BATCH_LAYER,
            &desc, &gameObject)))
            return E_FAIL;
        shared_ptr<CMapStaticBatchObject> batch =
            dynamic_pointer_cast<CMapStaticBatchObject>(gameObject);
        if (nullptr == batch)
        {
            CGameInstance::Get().Remove_GameObject_from_Layer(
                ETOUI(LEVEL::ASSET_TEST), MAP_BATCH_LAYER, gameObject);
            return E_FAIL;
        }
        outBatches.push_back({ key.first, key.second, batch });
        for (const MAP_PLACEMENT_RECORD* record : placements)
        {
            if (!outLookup.emplace(record->placementId, batch).second)
                return E_FAIL;
        }
    }
    return S_OK;
}

void CMapSceneRuntime::Remove_Batches(
    std::vector<STATIC_BATCH_ENTRY>& batches,
    bool_t removeFromObjectManager)
{
    if (removeFromObjectManager)
    {
        for (const STATIC_BATCH_ENTRY& entry : batches)
        {
            if (nullptr != entry.object)
                CGameInstance::Get().Remove_GameObject_from_Layer(
                    ETOUI(LEVEL::ASSET_TEST), MAP_BATCH_LAYER,
                    static_pointer_cast<CGameObject>(entry.object));
        }
    }
    batches.clear();
}

MAP_PLACEMENT_RECORD* CMapSceneRuntime::Find_Record(
    uint64_t placementId)
{
    const auto iter = std::find_if(
        m_RecordView.begin(), m_RecordView.end(),
        [placementId](const MAP_PLACEMENT_RECORD& record)
        { return record.placementId == placementId; });
    return iter == m_RecordView.end() ? nullptr : &*iter;
}
```

기존 `Rebuild_RecordView()` 선언과 정의, 그리고 이를 호출하는 코드는 삭제한다. 정본
record vector는 이제 `m_RecordView`이며 batch/fallback은 그 runtime 표현이다.

`CMainApp::Update()`에서 `Synchronize_Level()` 직후 다음 counter 갱신을 추가한다.

```cpp
if (Engine::CProfiler* profiler = CGameInstance::Get().Get_Profiler())
{
    profiler->Set_Counter(
        Engine::EProfilerCounter::MapPlacements,
        m_pMapSceneRuntime->Get_PlacementCount());
    profiler->Set_Counter(
        Engine::EProfilerCounter::MapBatchCount,
        m_pMapSceneRuntime->Get_BatchCount());
    profiler->Set_Counter(
        Engine::EProfilerCounter::MapFallbackObjects,
        m_pMapSceneRuntime->Get_FallbackCount());
    if (Engine::CTextureResourceCache* cache =
        CGameInstance::Get().Get_TextureResourceCache())
    {
        const Engine::FTextureCacheStats stats = cache->Get_Stats();
        profiler->Set_Counter(
            Engine::EProfilerCounter::TextureRequests, stats.Requests);
        profiler->Set_Counter(
            Engine::EProfilerCounter::TexturePathHits, stats.PathHits);
        profiler->Set_Counter(
            Engine::EProfilerCounter::TextureContentHits, stats.ContentHits);
        profiler->Set_Counter(
            Engine::EProfilerCounter::TextureUniqueSrvs, stats.UniqueSrvs);
        profiler->Set_Counter(
            Engine::EProfilerCounter::TextureEstimatedGpuBytes,
            stats.EstimatedGpuBytes);
    }
}
```

#### `CMapTool` non-owning façade 최종 교체 블록

`MapTool.h` include에 다음을 추가한다.

```cpp
#include "MapSceneRuntime.h"
```

`PLACED_ENTRY`, `Initialize()` 선언, private helper와 멤버를 다음 블록으로 교체한다.

```cpp
private:
    struct PLACED_ENTRY
    {
        MAP_PLACEMENT_RECORD record;
    };

public:
    HRESULT Initialize(
        ComPtr<ID3D11Device> device,
        ComPtr<ID3D11DeviceContext> context,
        CMapSceneRuntime* sceneRuntime);

private:
    void Refresh_FromRuntime();

private:
    CMapSceneRuntime* m_pSceneRuntime = nullptr; // non-owning, CMainApp owner
    CMapAssetCatalog m_Catalog;                 // UI snapshot
    std::unique_ptr<CMapAssetPreview> m_pAssetPreview;
    vector<PLACED_ENTRY> m_Placements;          // UI record snapshot
    DEPLOY_PROP_STATE m_DeployPhase = DEPLOY_PROP_STATE::INTACT;
    MAP_ENVIRONMENT_PHASE m_EnvironmentPhase =
        MAP_ENVIRONMENT_PHASE::BASELINE;
```

기존 `CDeployPropCatalog m_DeployCatalog`, `vector<DEPLOY_ENTRY> m_DeployProps`,
`ENVIRONMENT_PHASE` enum, `DEPLOY_ENTRY` 구조체를 삭제한다.

`CMapTool::Initialize()`을 다음 전체 함수로 교체한다.

```cpp
HRESULT CMapTool::Initialize(
    ComPtr<ID3D11Device> device,
    ComPtr<ID3D11DeviceContext> context,
    CMapSceneRuntime* sceneRuntime)
{
    if (nullptr == sceneRuntime)
        return E_INVALIDARG;
    auto preview = std::make_unique<CMapAssetPreview>();
    if (FAILED(preview->Initialize(device, context)))
        return E_FAIL;
    m_pSceneRuntime = sceneRuntime;
    m_pAssetPreview = std::move(preview);
    return S_OK;
}
```

`Handle_LevelTransition()`과 새 refresh 함수를 다음으로 교체·추가한다.

```cpp
void CMapTool::Handle_LevelTransition(bool_t isAssetTest)
{
    if (isAssetTest == m_bWasInAssetTest)
        return;
    m_bWasInAssetTest = isAssetTest;
    m_ePlacementState = PLACEMENT_STATE::IDLE;
    m_iSelectedPlacementId = 0;
    m_SelectedAssetId.clear();
    if (nullptr != m_pAssetPreview)
        m_pAssetPreview->Reset_LevelResources();
    if (!isAssetTest)
    {
        m_Placements.clear();
        m_Catalog = CMapAssetCatalog{};
        m_iNextPlacementId = 1;
        m_bDirty = false;
        m_Status = "Enter AssetTest with F2";
        return;
    }
    Refresh_FromRuntime();
}

void CMapTool::Refresh_FromRuntime()
{
    m_Placements.clear();
    if (nullptr == m_pSceneRuntime || !m_pSceneRuntime->Is_Loaded())
    {
        m_Status = nullptr == m_pSceneRuntime ?
            "MapSceneRuntime is missing" : m_pSceneRuntime->Get_Status();
        return;
    }
    m_Catalog = m_pSceneRuntime->Get_Catalog();
    const vector<MAP_PLACEMENT_RECORD>& records =
        m_pSceneRuntime->Get_PlacementRecords();
    m_Placements.reserve(records.size());
    m_iNextPlacementId = 1;
    for (const MAP_PLACEMENT_RECORD& record : records)
    {
        m_Placements.push_back({ record });
        if ((record.transformSource == "editor" ||
            record.transformSource == "legacy") &&
            record.placementId <=
                CMapPlacementDocument::MAX_EDITOR_PLACEMENT_ID)
        {
            m_iNextPlacementId = (std::max)(
                m_iNextPlacementId, record.placementId + 1);
        }
    }
    m_bDirty = false;
    m_Status = m_pSceneRuntime->Get_Status();
}
```

`Try_PlaceSelected()` 내부의 `m_Catalog` area 사용은 유지하고, `Create_Placement`,
`Remove_Placement`, `Remove_AllPlacements`, `Save_Placements`, `Load_Placements`를
다음 전체 함수로 교체한다.

```cpp
bool_t CMapTool::Create_Placement(
    const MAP_PLACEMENT_RECORD& record,
    PLACED_ENTRY& outEntry)
{
    if (nullptr == m_pSceneRuntime ||
        FAILED(m_pSceneRuntime->Add_Placement(record)))
        return false;
    outEntry.record = record;
    return true;
}

bool_t CMapTool::Remove_Placement(uint64_t placementId)
{
    if (nullptr == m_pSceneRuntime ||
        FAILED(m_pSceneRuntime->Remove_Placement(placementId)))
        return false;
    m_Placements.erase(std::remove_if(
        m_Placements.begin(), m_Placements.end(),
        [placementId](const PLACED_ENTRY& entry)
        { return entry.record.placementId == placementId; }),
        m_Placements.end());
    if (m_iSelectedPlacementId == placementId)
        m_iSelectedPlacementId = 0;
    m_bDirty = true;
    return true;
}

void CMapTool::Remove_AllPlacements()
{
    if (nullptr == m_pSceneRuntime)
        return;
    vector<uint64_t> ids;
    ids.reserve(m_Placements.size());
    for (const PLACED_ENTRY& entry : m_Placements)
        ids.push_back(entry.record.placementId);
    for (uint64_t id : ids)
    {
        if (FAILED(m_pSceneRuntime->Remove_Placement(id)))
        {
            m_Status = "Clear stopped at placement #" +
                std::to_string(id);
            Refresh_FromRuntime();
            return;
        }
    }
    m_Placements.clear();
    m_iSelectedPlacementId = 0;
    m_iNextPlacementId = 1;
    m_bDirty = true;
}

bool_t CMapTool::Save_Placements()
{
    if (nullptr == m_pSceneRuntime ||
        FAILED(m_pSceneRuntime->Save_Placements()))
    {
        m_Status = nullptr == m_pSceneRuntime ?
            "MapSceneRuntime is missing" : m_pSceneRuntime->Get_Status();
        return false;
    }
    m_bDirty = false;
    m_Status = m_pSceneRuntime->Get_Status();
    return true;
}

bool_t CMapTool::Load_Placements()
{
    if (nullptr == m_pSceneRuntime ||
        FAILED(m_pSceneRuntime->Reload()))
    {
        m_Status = nullptr == m_pSceneRuntime ?
            "MapSceneRuntime is missing" : m_pSceneRuntime->Get_Status();
        return false;
    }
    Refresh_FromRuntime();
    return true;
}
```

Deploy/environment 함수는 다음 전체 함수로 교체한다. 기존 `Load_DeployProps()`와
`Remove_DeployProps()` 정의·선언은 삭제한다.

```cpp
void CMapTool::Set_DeployPhase(DEPLOY_PROP_STATE state)
{
    m_DeployPhase = state;
    if (nullptr != m_pSceneRuntime)
        m_pSceneRuntime->Set_DeployPhase(state);
}

void CMapTool::Set_EnvironmentPhase(MAP_ENVIRONMENT_PHASE phase)
{
    m_EnvironmentPhase = phase;
    if (nullptr != m_pSceneRuntime)
        m_pSceneRuntime->Set_EnvironmentPhase(phase);
}
```

`Render_Inspector()`를 다음 전체 함수로 교체한다.

```cpp
void CMapTool::Render_Inspector()
{
    ImGui::TextUnformatted("Inspector");
    PLACED_ENTRY* entry = Find_Placement(m_iSelectedPlacementId);
    if (nullptr == entry)
    {
        ImGui::TextDisabled("Select a placed object.");
        return;
    }

    ImGui::Text("Placement #%llu",
        static_cast<unsigned long long>(entry->record.placementId));
    ImGui::TextWrapped("Source: %s",
        entry->record.sourcePlacementId.c_str());
    ImGui::Text("Level: %s | Transform: %s",
        entry->record.sourceLevel.c_str(),
        entry->record.transformSource.c_str());
    ImGui::TextWrapped("Asset: %s", entry->record.assetId.c_str());

    float3_t position = entry->record.position;
    float4_t quaternion = entry->record.rotationQuaternion;
    float3_t scale = entry->record.signedScale;
    bool_t visible = entry->record.visible;
    const bool_t transformChanged =
        ImGui::DragFloat3("Position", &position.x, 0.1f) |
        ImGui::DragFloat4(
            "Rotation quaternion", &quaternion.x, 0.0025f) |
        ImGui::DragFloat3(
            "Signed scale", &scale.x, 0.01f, -1000.f, 1000.f);
    if (transformChanged)
    {
        vector_t rotation = XMLoadFloat4(&quaternion);
        const float length = XMVectorGetX(XMVector4Length(rotation));
        if (std::isfinite(length) && length >= 0.000001f &&
            std::abs(scale.x) >= 0.000001f &&
            std::abs(scale.y) >= 0.000001f &&
            std::abs(scale.z) >= 0.000001f)
        {
            rotation = XMQuaternionNormalize(rotation);
            if (XMVectorGetW(rotation) < 0.f)
                rotation = XMVectorNegate(rotation);
            XMStoreFloat4(&quaternion, rotation);
            if (SUCCEEDED(m_pSceneRuntime->Update_Placement(
                entry->record.placementId,
                position, quaternion, scale)))
            {
                entry->record.position = position;
                entry->record.rotationQuaternion = quaternion;
                entry->record.signedScale = scale;
                m_bDirty = true;
            }
            else
                m_Status = "Transform edit was rejected by runtime";
        }
        else
            m_Status = "Transform edit rejected: zero quaternion/scale axis";
    }

    if (ImGui::Checkbox("Visible", &visible))
    {
        if (SUCCEEDED(m_pSceneRuntime->Set_PlacementVisible(
            entry->record.placementId, visible)))
        {
            entry->record.visible = visible;
            m_bDirty = true;
        }
    }
    ImGui::Text("Mirrored pass: %s",
        entry->record.signedScale.x * entry->record.signedScale.y *
            entry->record.signedScale.z < 0.f ? "YES" : "NO");

    if (ImGui::Button("Delete selected"))
    {
        const uint64_t id = entry->record.placementId;
        if (Remove_Placement(id))
            m_Status = "Deleted placement #" + std::to_string(id);
    }
}
```

`Render_Toolbar()`의 Deploy count 표현은 다음으로 교체한다.

```cpp
const size_t deployCount = nullptr == m_pSceneRuntime ?
    0 : m_pSceneRuntime->Get_DeployCount();
ImGui::Text("Gameplay DeployProps: %zu | phase:", deployCount);
```

같은 함수의 Reload 버튼 블록은 다음으로 교체한다.

```cpp
if (ImGui::Button("Reload"))
    Load_Placements();
```

환경 phase 비교의 enum 이름은 모두 `MAP_ENVIRONMENT_PHASE`로 교체한다.

`CMainApp::ReadyDebugTools()`의 MapTool 초기화 호출은 다음으로 교체한다.

```cpp
auto mapTool = std::make_unique<CMapTool>();
if (FAILED(mapTool->Initialize(
    m_pDevice, m_pContext, m_pMapSceneRuntime.get())))
    return E_FAIL;
m_pMapTool = std::move(mapTool);
```

#### `Loader.cpp` batch prototype 등록 블록

include에 다음을 추가한다.

```cpp
#include "MapStaticBatchObject.h"
```

#### `MapAssetObject.cpp` 공통 material 경로 교체 함수

include에 다음을 추가한다.

```cpp
#include "MapAssetRenderUtils.h"
```

`Render()`와 `Select_ShaderPass()`를 다음 전체 함수로 교체한다.

```cpp
HRESULT CMapAssetObject::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;
    const uint32_t pass = Select_ShaderPass();
    for (uint32_t mesh = 0; mesh < m_pModelCom->Get_NumMeshes(); ++mesh)
    {
        if (FAILED(CMapAssetRenderUtils::Bind_Material(
            m_pModelCom, m_pShaderCom, mesh,
            m_RenderProfile, m_fElapsedTime)) ||
            FAILED(m_pShaderCom->Begin(pass)) ||
            FAILED(m_pModelCom->Render(mesh)))
            return E_FAIL;
    }
    return S_OK;
}

uint32_t CMapAssetObject::Select_ShaderPass() const
{
    return CMapAssetRenderUtils::Select_Pass(
        m_RenderProfile, m_bMirrored);
}

HRESULT CMapAssetObject::Ready_Components(
    const std::wstring& modelPrototypeTag)
{
    if (FAILED(__super::Add_Component(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_Component_Shader_VtxMeshMap"),
        TEXT("Com_Shader"), m_pShaderCom)) ||
        FAILED(__super::Add_Component(
            ETOUI(LEVEL::ASSET_TEST), modelPrototypeTag,
            TEXT("Com_Model"), m_pModelCom)))
        return E_FAIL;
    return S_OK;
}
```

`Ready_For_Level_AssetTest()`에서 기존
`Prototype_Component_Shader_VtxMeshBinary` 등록 직후 다음을 추가한다. 전역 binary
Effect는 Weapon/Deploy가 공유하므로 수정하지 않는다.

```cpp
if (FAILED(CGameInstance::Get().Add_Prototype(
    ETOUI(LEVEL::ASSET_TEST),
    TEXT("Prototype_Component_Shader_VtxMeshMap"),
    CShader::Create(
        m_pDevice, m_pContext,
        TEXT("../Bin/ShaderFiles/Shader_VtxMeshMap.hlsl"),
        VTXMESH::Elements,
        VTXMESH::iNumElements))) ||
    FAILED(CGameInstance::Get().Add_Prototype(
        ETOUI(LEVEL::ASSET_TEST),
        TEXT("Prototype_Component_Shader_VtxMeshMapInstance"),
        CShader::Create(
            m_pDevice, m_pContext,
            TEXT("../Bin/ShaderFiles/Shader_VtxMeshMapInstance.hlsl"),
            VTXMESH_INSTANCE::Elements,
            VTXMESH_INSTANCE::iNumElements))))
    return E_FAIL;
```

기존 `Prototype_GameObject_MapAsset` 등록 직후 다음을 추가한다.

```cpp
if (FAILED(CGameInstance::Get().Add_Prototype(
    ETOUI(LEVEL::ASSET_TEST),
    TEXT("Prototype_GameObject_MapStaticBatch"),
    CMapStaticBatchObject::Create(m_pDevice, m_pContext))))
    return E_FAIL;
```

### 5-13. map material 공통 HLSLI와 전용 standalone/instance Effect

#### `Client/Bin/ShaderFiles/Shader_MapAssetMaterial.hlsli` 전체 내용

```hlsl
Texture2D g_DiffuseTexture;
Texture2D g_NormalTexture;
Texture2D g_EmissiveTexture;
Texture2D g_SpecularTexture;
Texture2D g_OpacityTexture;
uint g_HasNormalTexture = 0;
uint g_HasEmissiveTexture = 0;
uint g_HasSpecularTexture = 0;
uint g_HasOpacityTexture = 0;
float g_EmissiveIntensity = 1.f;
float g_SpecularIntensity = 1.f;
float g_SpecularPower = 50.f;
float2 g_UVScale = float2(1.f, 1.f);
float2 g_UVOffset = float2(0.f, 0.f);
float g_Opacity = 1.f;
float4 g_ColorTint = 1.f;

struct MAP_VS_OUT
{
    float4 vPosition : SV_POSITION;
    float4 vNormal : NORMAL;
    float4 vTangent : TANGENT;
    float4 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorldPos : TEXCOORD1;
    float4 vProjPos : TEXCOORD2;
};

struct MAP_PS_OUT
{
    float4 vDiffuse : SV_TARGET0;
    float4 vNormal : SV_TARGET1;
    float4 vDepth : SV_TARGET2;
    float4 vPickPos : SV_TARGET3;
    float4 vEmissive : SV_TARGET4;
};

struct MAP_PS_OUT_FORWARD
{
    float4 vColor : SV_TARGET0;
};

MAP_PS_OUT PS_MAP_DEFERRED(MAP_VS_OUT input)
{
    MAP_PS_OUT output;
    float4 diffuse = g_DiffuseTexture.Sample(
        LinearSampler, input.vTexcoord) * g_ColorTint;
    if (diffuse.a < 0.3f)
        discard;

    float3 normal = normalize(input.vNormal.xyz);
    if (0 != g_HasNormalTexture)
    {
        float4 encoded = g_NormalTexture.Sample(
            LinearSampler, input.vTexcoord);
        float3 tangentNormal;
        if (encoded.b <= 0.0001f)
        {
            float2 xy = encoded.rg * 2.f - 1.f;
            tangentNormal = float3(
                xy, sqrt(saturate(1.f - dot(xy, xy))));
        }
        else
            tangentNormal = normalize(encoded.xyz * 2.f - 1.f);

        float3x3 tangentToWorld = float3x3(
            normalize(input.vTangent.xyz),
            normalize(input.vBinormal.xyz) * -1.f,
            normal);
        normal = normalize(mul(tangentNormal, tangentToWorld));
    }

    float specularMask = g_SpecularIntensity;
    if (0 != g_HasSpecularTexture)
    {
        float3 specular = g_SpecularTexture.Sample(
            LinearSampler, input.vTexcoord).rgb;
        specularMask *= dot(specular,
            float3(0.299f, 0.587f, 0.114f));
    }

    output.vDiffuse = diffuse;
    output.vNormal = float4(normal * 0.5f + 0.5f, specularMask);
    output.vDepth = float4(
        input.vProjPos.z / input.vProjPos.w,
        input.vProjPos.w / 1000.f, g_SpecularPower, 0.f);
    output.vPickPos = input.vWorldPos;
    output.vEmissive = 0.f;
    if (0 != g_HasEmissiveTexture)
    {
        const float3 emissive = g_EmissiveTexture.Sample(
            LinearSampler, input.vTexcoord).rgb;
        output.vEmissive = float4(
            emissive * g_EmissiveIntensity, 0.f);
    }
    return output;
}

MAP_PS_OUT_FORWARD PS_MAP_ALPHA(MAP_VS_OUT input)
{
    MAP_PS_OUT_FORWARD output;
    float4 color = g_DiffuseTexture.Sample(
        LinearSampler, input.vTexcoord);
    color.rgb *= g_ColorTint.rgb;
    float opacityMask = 1.f;
    if (0 != g_HasOpacityTexture)
        opacityMask = g_OpacityTexture.Sample(
            LinearSampler, input.vTexcoord).r;
    color.a = saturate(
        color.a * opacityMask * g_Opacity * g_ColorTint.a);
    if (color.a < 0.001f)
        discard;
    output.vColor = color;
    return output;
}

MAP_PS_OUT_FORWARD PS_MAP_SKY(MAP_VS_OUT input)
{
    MAP_PS_OUT_FORWARD output;
    const float4 color = g_DiffuseTexture.Sample(
        LinearSampler, input.vTexcoord);
    output.vColor = float4(color.rgb * g_ColorTint.rgb, 1.f);
    return output;
}
```

#### `Client/Bin/ShaderFiles/Shader_VtxMeshMap.hlsl` 전체 내용

```hlsl
#include "Engine_Shader_Defines.hlsli"
#include "Shader_MapAssetMaterial.hlsli"

float4x4 g_WorldMatrix;
float4x4 g_WorldInvTransposeMatrix;
float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
};

MAP_VS_OUT VS_MAIN(VS_IN input)
{
    MAP_VS_OUT output;
    const float4 worldPosition = mul(
        float4(input.vPosition, 1.f), g_WorldMatrix);
    output.vPosition = mul(mul(
        worldPosition, g_ViewMatrix), g_ProjMatrix);
    const float3 normal = normalize(mul(
        float4(input.vNormal, 0.f),
        g_WorldInvTransposeMatrix).xyz);
    const float3 tangentLinear = mul(
        float4(input.vTangent, 0.f), g_WorldMatrix).xyz;
    const float3 tangent = normalize(
        tangentLinear - normal * dot(tangentLinear, normal));
    const float3 sourceBinormal = mul(
        float4(input.vBinormal, 0.f), g_WorldMatrix).xyz;
    const float handedness =
        dot(cross(normal, tangent), sourceBinormal) < 0.f ? -1.f : 1.f;
    output.vNormal = float4(normal, 0.f);
    output.vTangent = float4(tangent, 0.f);
    output.vBinormal = float4(
        normalize(cross(normal, tangent)) * handedness, 0.f);
    output.vTexcoord = input.vTexcoord * g_UVScale + g_UVOffset;
    output.vWorldPos = worldPosition;
    output.vProjPos = output.vPosition;
    return output;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAP_DEFERRED();
    }
    pass MirroredPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAP_DEFERRED();
    }
    pass TwoSidedOpaquePass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAP_DEFERRED();
    }
    pass AlphaBackPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAP_ALPHA();
    }
    pass AlphaFrontPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAP_ALPHA();
    }
    pass AlphaTwoSidedPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_AlphaBlend, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAP_ALPHA();
    }
    pass SkyBackPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAP_SKY();
    }
    pass SkyFrontPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAP_SKY();
    }
    pass SkyTwoSidedPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAP_SKY();
    }
    pass AdditiveBackPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAP_ALPHA();
    }
    pass AdditiveFrontPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAP_ALPHA();
    }
    pass AdditiveTwoSidedPass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_ReadOnly, 0);
        SetBlendState(BS_Additive, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAP_ALPHA();
    }
}
```

#### `Client/Bin/ShaderFiles/Shader_VtxMeshMapInstance.hlsl` 전체 내용

```hlsl
#include "Engine_Shader_Defines.hlsli"
#include "Shader_MapAssetMaterial.hlsli"

float4x4 g_ViewMatrix;
float4x4 g_ProjMatrix;

struct VS_IN
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float3 vBinormal : BINORMAL;
    float2 vTexcoord : TEXCOORD0;
    float4 vWorld0 : WORLD0;
    float4 vWorld1 : WORLD1;
    float4 vWorld2 : WORLD2;
    float4 vWorld3 : WORLD3;
};

MAP_VS_OUT VS_MAIN(VS_IN input)
{
    MAP_VS_OUT output;
    const float4x4 world = float4x4(
        input.vWorld0, input.vWorld1,
        input.vWorld2, input.vWorld3);
    const float4 worldPosition = mul(
        float4(input.vPosition, 1.f), world);
    output.vPosition = mul(mul(
        worldPosition, g_ViewMatrix), g_ProjMatrix);

    const float3 right = world[0].xyz;
    const float3 up = world[1].xyz;
    const float3 look = world[2].xyz;
    const float3x3 inverseTranspose = float3x3(
        right / max(dot(right, right), 1e-12f),
        up / max(dot(up, up), 1e-12f),
        look / max(dot(look, look), 1e-12f));
    const float3 normal = normalize(mul(
        input.vNormal, inverseTranspose));
    const float3x3 linearWorld = (float3x3)world;
    const float3 tangentLinear = mul(input.vTangent, linearWorld);
    const float3 tangent = normalize(
        tangentLinear - normal * dot(tangentLinear, normal));
    const float3 sourceBinormal = mul(input.vBinormal, linearWorld);
    const float handedness =
        dot(cross(normal, tangent), sourceBinormal) < 0.f ? -1.f : 1.f;

    output.vNormal = float4(normal, 0.f);
    output.vTangent = float4(tangent, 0.f);
    output.vBinormal = float4(
        normalize(cross(normal, tangent)) * handedness, 0.f);
    output.vTexcoord = input.vTexcoord * g_UVScale + g_UVOffset;
    output.vWorldPos = worldPosition;
    output.vProjPos = output.vPosition;
    return output;
}

technique11 DefaultTechnique
{
    pass DefaultPass
    {
        SetRasterizerState(RS_Default);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAP_DEFERRED();
    }
    pass MirroredPass
    {
        SetRasterizerState(RS_Cull_CW);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAP_DEFERRED();
    }
    pass TwoSidedOpaquePass
    {
        SetRasterizerState(RS_Cull_None);
        SetDepthStencilState(DSS_Default, 0);
        SetBlendState(BS_Default, float4(0.f, 0.f, 0.f, 0.f), 0xffffffff);
        VertexShader = compile vs_5_0 VS_MAIN();
        GeometryShader = NULL;
        PixelShader = compile ps_5_0 PS_MAP_DEFERRED();
    }
}
```

### 5-14. 도구 CLI 전체 계약

#### texture audit

```text
python audit_optimize_map_textures.py \
  --catalog Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.mapassets \
  --asset-root Client/Bin/Resources/LostArk \
  --report C:/Users/user/Desktop/Resource_LostArk/05_Reports/MapExtraction/LV_LUT_HEARTRB_ED/optimization/valtan_texture_audit.json

python audit_optimize_map_textures.py \
  --catalog Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.mapassets \
  --asset-root Client/Bin/Resources/LostArk \
  --report C:/Users/user/Desktop/Resource_LostArk/05_Reports/MapExtraction/LV_LUT_HEARTRB_ED/optimization/valtan_texture_audit.json \
  --apply \
  --texconv C:/Users/user/Desktop/LOL_Tools/texconv.exe \
  --converted-root .codex_tmp/valtan_texture_candidates \
  --receipt C:/Users/user/Desktop/Resource_LostArk/05_Reports/MapExtraction/LV_LUT_HEARTRB_ED/optimization/valtan_texture_optimize_receipt.json
```

필수 동작은 `path`, `materialSlot`, `semantic`, `format`, `dimensions`, `mips`, `bytes`, `sha256`, `duplicateGroup`, `recommendedFormat`, `action` 기록이다. `--apply`는 원본을 덮지 않고 `--converted-root` 아래에 검토 후보를 원자 저장하며 source/target SHA-256과 texconv 명령을 영수증에 남긴다. `.wmodel`의 embedded WMA2 슬롯을 정본으로 읽으므로 파일명 접미사 추측은 사용하지 않는다.

#### `Tools/LevelPlacementExtractor/audit_optimize_map_textures.py` 전체 내용

```python
from __future__ import annotations

import argparse
import hashlib
import json
import shlex
import struct
import subprocess
import tempfile
from collections import defaultdict
from pathlib import Path
from typing import Any, Iterable


CATALOG_MAGIC = "LOSTARK_MAP_ASSET_CATALOG"
WINT_HEADER = struct.Struct("<4sHHII")
WMODEL_HEADER = struct.Struct("<4sIII4I")
WMODEL_SECTION = struct.Struct("<IIQQ40s")
WMATERIAL_HEADER = struct.Struct("<4sI")
WMATERIAL_PREFIX = struct.Struct("<IQ64s")
MATERIAL_SECTION_TYPE = 2
WIDE_PATH_BYTES = 260 * 2
SLOT_NAMES = (
    "baseColor",
    "normal",
    "specular",
    "emissive",
    "opacity",
    "orm",
    "metallic",
    "roughness",
    "ambientOcclusion",
)
DX10_FORMATS = {
    28: "R8G8B8A8_UNORM",
    29: "R8G8B8A8_UNORM_SRGB",
    71: "BC1_UNORM",
    72: "BC1_UNORM_SRGB",
    74: "BC2_UNORM",
    75: "BC2_UNORM_SRGB",
    77: "BC3_UNORM",
    78: "BC3_UNORM_SRGB",
    80: "BC4_UNORM",
    81: "BC4_SNORM",
    83: "BC5_UNORM",
    84: "BC5_SNORM",
    95: "BC6H_UF16",
    96: "BC6H_SF16",
    98: "BC7_UNORM",
    99: "BC7_UNORM_SRGB",
}
FOURCC_FORMATS = {
    b"DXT1": "BC1_UNORM",
    b"DXT3": "BC2_UNORM",
    b"DXT5": "BC3_UNORM",
    b"ATI1": "BC4_UNORM",
    b"BC4U": "BC4_UNORM",
    b"BC4S": "BC4_SNORM",
    b"ATI2": "BC5_UNORM",
    b"BC5U": "BC5_UNORM",
    b"BC5S": "BC5_SNORM",
}


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def atomic_write_json(path: Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + ".tmp")
    temporary.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    temporary.replace(path)


def read_catalog(path: Path) -> tuple[str, list[dict[str, str]]]:
    lines = [
        line.strip()
        for line in path.read_text(encoding="utf-8-sig").splitlines()
        if line.strip()
    ]
    if not lines:
        raise ValueError("empty map asset catalog")
    header = shlex.split(lines[0], posix=True)
    if (
        len(header) != 4
        or header[0] != CATALOG_MAGIC
        or int(header[1]) not in (2, 3)
    ):
        raise ValueError("unsupported map asset catalog header")
    area_id = header[2]
    declared_count = int(header[3])
    rows: list[dict[str, str]] = []
    seen_ids: set[str] = set()
    for line_number, line in enumerate(lines[1:], 2):
        fields = shlex.split(line, posix=True)
        if len(fields) < 4:
            raise ValueError(f"catalog row {line_number} is truncated")
        asset_id, model_path = fields[0], fields[2]
        if not asset_id or asset_id in seen_ids:
            raise ValueError(f"duplicate/empty assetId at row {line_number}")
        if Path(model_path).is_absolute() or Path(model_path).suffix.lower() != ".wmodel":
            raise ValueError(f"invalid model path at row {line_number}: {model_path}")
        seen_ids.add(asset_id)
        rows.append({"assetId": asset_id, "modelPath": model_path})
    if len(rows) != declared_count:
        raise ValueError(
            f"catalog count mismatch: declared={declared_count}, actual={len(rows)}"
        )
    return area_id, rows


def unpack_wint(data: bytes, label: str) -> memoryview:
    if len(data) < WINT_HEADER.size:
        raise ValueError(f"{label}: truncated WINT header")
    magic, major, _minor, flags, content_size = WINT_HEADER.unpack_from(data)
    if magic != b"WINT" or major != 1 or flags != 0:
        raise ValueError(f"{label}: invalid WINT header")
    if content_size != len(data) - WINT_HEADER.size:
        raise ValueError(f"{label}: WINT content size mismatch")
    return memoryview(data)[WINT_HEADER.size:]


def decode_wide_path(data: memoryview) -> str:
    if len(data) != WIDE_PATH_BYTES:
        raise ValueError("invalid fixed UTF-16 path width")
    value = bytes(data).decode("utf-16-le", errors="strict")
    return value.split("\0", 1)[0]


def read_embedded_materials(model_path: Path) -> list[dict[str, Any]]:
    content = unpack_wint(model_path.read_bytes(), str(model_path))
    if len(content) < WMODEL_HEADER.size:
        raise ValueError(f"{model_path}: truncated WMOD header")
    magic, section_count, _animation_count, _flags, *_reserved = (
        WMODEL_HEADER.unpack_from(content)
    )
    if magic != b"WMOD" or section_count < 2 or section_count > 4096:
        raise ValueError(f"{model_path}: invalid WMOD metadata")
    table_end = WMODEL_HEADER.size + section_count * WMODEL_SECTION.size
    if table_end > len(content):
        raise ValueError(f"{model_path}: truncated WMOD section table")
    material_sections: list[memoryview] = []
    for index in range(section_count):
        section_type, _section_index, offset, size, _name = (
            WMODEL_SECTION.unpack_from(
                content, WMODEL_HEADER.size + index * WMODEL_SECTION.size
            )
        )
        if offset > len(content) or size > len(content) - offset:
            raise ValueError(f"{model_path}: section outside WMOD content")
        if section_type == MATERIAL_SECTION_TYPE:
            material_sections.append(content[offset : offset + size])
    if len(material_sections) != 1:
        raise ValueError(
            f"{model_path}: expected one embedded material section, "
            f"found {len(material_sections)}"
        )

    material_content = unpack_wint(
        bytes(material_sections[0]), f"{model_path}:material"
    )
    if len(material_content) < WMATERIAL_HEADER.size:
        raise ValueError(f"{model_path}: truncated material metadata")
    material_magic, material_count = WMATERIAL_HEADER.unpack_from(material_content)
    if material_count > 4096 or material_magic not in (b"WMAT", b"WMA2"):
        raise ValueError(f"{model_path}: invalid material metadata")

    cursor = WMATERIAL_HEADER.size
    entries: list[dict[str, Any]] = []
    for _ in range(material_count):
        path_count = 9 if material_magic == b"WMA2" else 1
        entry_size = WMATERIAL_PREFIX.size + path_count * WIDE_PATH_BYTES
        if cursor + entry_size > len(material_content):
            raise ValueError(f"{model_path}: truncated material entry")
        material_index, _material_hash, raw_name = WMATERIAL_PREFIX.unpack_from(
            material_content, cursor
        )
        cursor += WMATERIAL_PREFIX.size
        name = raw_name.split(b"\0", 1)[0].decode("utf-8", errors="replace")
        paths: dict[str, str] = {}
        semantics = SLOT_NAMES if material_magic == b"WMA2" else ("baseColor",)
        for semantic in semantics:
            paths[semantic] = decode_wide_path(
                material_content[cursor : cursor + WIDE_PATH_BYTES]
            )
            cursor += WIDE_PATH_BYTES
        entries.append(
            {
                "materialSlot": material_index,
                "materialName": name,
                "paths": paths,
            }
        )
    return entries


def resolve_texture_path(
    stored_value: str, model_path: Path, asset_root: Path
) -> Path:
    normalized_value = stored_value.replace("\\", "/")
    stored = Path(normalized_value)
    candidates: list[Path] = []
    if stored.is_absolute():
        candidates.append(stored)
    lower = normalized_value.casefold()
    for marker in ("resources/lostark/", "resource/lostark/"):
        position = lower.rfind(marker)
        if position >= 0:
            candidates.append(asset_root / normalized_value[position + len(marker) :])
    if not stored.is_absolute():
        candidates.extend((model_path.parent / stored, asset_root / stored))
    for candidate in candidates:
        resolved = candidate.resolve(strict=False)
        if resolved.is_file():
            return resolved
    return candidates[0].resolve(strict=False) if candidates else stored.resolve(strict=False)


def inspect_dds(path: Path) -> dict[str, Any]:
    header = path.read_bytes()[:148]
    if len(header) < 128 or header[:4] != b"DDS ":
        raise ValueError(f"{path}: invalid DDS header")
    size = struct.unpack_from("<I", header, 4)[0]
    height, width = struct.unpack_from("<II", header, 12)
    mip_count = struct.unpack_from("<I", header, 28)[0]
    if size != 124 or width == 0 or height == 0:
        raise ValueError(f"{path}: invalid DDS dimensions")
    four_cc = header[84:88]
    if four_cc == b"DX10":
        if len(header) < 148:
            raise ValueError(f"{path}: truncated DDS DX10 header")
        dxgi_format = struct.unpack_from("<I", header, 128)[0]
        format_name = DX10_FORMATS.get(dxgi_format, f"DXGI_{dxgi_format}")
    elif four_cc.strip(b"\0 "):
        format_name = FOURCC_FORMATS.get(
            four_cc, "FOURCC_" + four_cc.decode("ascii", errors="replace")
        )
    else:
        rgb_bits = struct.unpack_from("<I", header, 88)[0]
        format_name = f"RGB{rgb_bits}_UNCOMPRESSED"
    return {
        "format": format_name,
        "dimensions": [width, height],
        "mips": max(1, mip_count),
    }


def inspect_png(path: Path) -> dict[str, Any]:
    header = path.read_bytes()[:26]
    if len(header) < 26 or header[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"{path}: invalid PNG header")
    width, height = struct.unpack_from(">II", header, 16)
    if width == 0 or height == 0:
        raise ValueError(f"{path}: invalid PNG dimensions")
    return {"format": "PNG", "dimensions": [width, height], "mips": 1}


def inspect_tga(path: Path) -> dict[str, Any]:
    header = path.read_bytes()[:18]
    if len(header) != 18:
        raise ValueError(f"{path}: truncated TGA header")
    width, height = struct.unpack_from("<HH", header, 12)
    if width == 0 or height == 0:
        raise ValueError(f"{path}: invalid TGA dimensions")
    return {
        "format": f"TGA_{header[16]}BPP",
        "dimensions": [width, height],
        "mips": 1,
    }


def inspect_texture(path: Path) -> dict[str, Any]:
    suffix = path.suffix.casefold()
    if suffix == ".dds":
        return inspect_dds(path)
    if suffix == ".png":
        return inspect_png(path)
    if suffix == ".tga":
        return inspect_tga(path)
    return {"format": suffix.lstrip(".").upper() or "UNKNOWN",
            "dimensions": [0, 0], "mips": 0}


def loader_kind(path: Path) -> str:
    suffix = path.suffix.casefold()
    if suffix == ".dds":
        return "DDS"
    if suffix == ".tga":
        return "TGA"
    return "WIC"


def recommended_format(semantic: str) -> str:
    if semantic == "normal":
        return "BC5_UNORM"
    if semantic in ("opacity", "metallic", "roughness", "ambientOcclusion"):
        return "BC4_UNORM"
    if semantic == "orm":
        return "BC7_UNORM"
    if semantic in ("baseColor", "emissive"):
        return "BC7_UNORM_SRGB"
    return "BC7_UNORM"


def accepted_formats(semantic: str) -> set[str]:
    if semantic == "normal":
        return {"BC5_UNORM", "BC5_SNORM"}
    if semantic in ("opacity", "metallic", "roughness", "ambientOcclusion"):
        return {"BC4_UNORM", "BC4_SNORM"}
    if semantic == "orm":
        return {"BC1_UNORM", "BC3_UNORM", "BC7_UNORM"}
    if semantic in ("baseColor", "emissive"):
        return {"BC1_UNORM_SRGB", "BC3_UNORM_SRGB", "BC7_UNORM_SRGB"}
    return {"BC1_UNORM", "BC3_UNORM", "BC7_UNORM"}


def audit(
    catalog_path: Path, asset_root: Path
) -> tuple[str, list[dict[str, Any]], dict[str, Any]]:
    area_id, catalog = read_catalog(catalog_path)
    records: list[dict[str, Any]] = []
    for asset in catalog:
        model_path = (asset_root / asset["modelPath"]).resolve(strict=False)
        if not model_path.is_file():
            raise FileNotFoundError(f"catalog model is missing: {model_path}")
        for material in read_embedded_materials(model_path):
            for semantic, stored_value in material["paths"].items():
                if not stored_value:
                    continue
                texture_path = resolve_texture_path(
                    stored_value, model_path, asset_root
                )
                record: dict[str, Any] = {
                    "assetId": asset["assetId"],
                    "modelPath": str(model_path),
                    "materialSlot": material["materialSlot"],
                    "materialName": material["materialName"],
                    "semantic": semantic,
                    "storedPath": stored_value,
                    "path": str(texture_path),
                    "format": "MISSING",
                    "dimensions": [0, 0],
                    "mips": 0,
                    "bytes": 0,
                    "sha256": "",
                    "duplicateGroup": "",
                    "recommendedFormat": recommended_format(semantic),
                    "action": "MISSING",
                }
                if texture_path.is_file():
                    record.update(inspect_texture(texture_path))
                    record["bytes"] = texture_path.stat().st_size
                    record["sha256"] = sha256_file(texture_path)
                    if texture_path.suffix.casefold() in (".png", ".tga"):
                        record["action"] = "CONVERT_CANDIDATE"
                    elif (
                        record["format"] in accepted_formats(semantic)
                        and record["mips"] > 1
                    ):
                        record["action"] = "KEEP"
                    else:
                        record["action"] = "REVIEW_REENCODE"
                records.append(record)

    paths_by_hash: dict[str, set[str]] = defaultdict(set)
    for record in records:
        if record["sha256"]:
            paths_by_hash[record["sha256"]].add(
                str(Path(record["path"]).resolve(strict=False)).casefold()
            )
    duplicate_hashes = sorted(
        digest for digest, paths in paths_by_hash.items() if len(paths) > 1
    )
    duplicate_ids = {
        digest: f"DUP-{index:04d}"
        for index, digest in enumerate(duplicate_hashes, 1)
    }
    for record in records:
        record["duplicateGroup"] = duplicate_ids.get(record["sha256"], "")
        if record["duplicateGroup"] and record["action"] == "KEEP":
            record["action"] = "KEEP_RUNTIME_DEDUP"

    unique_paths = {
        Path(record["path"])
        for record in records
        if record["sha256"]
    }
    payload_bytes = {
        digest: max(
            record["bytes"]
            for record in records
            if record["sha256"] == digest
        )
        for digest in paths_by_hash
    }
    cache_paths: dict[tuple[str, str], set[str]] = defaultdict(set)
    cache_payload_bytes: dict[tuple[str, str], int] = {}
    for record in records:
        if not record["sha256"]:
            continue
        cache_key = (
            record["sha256"],
            loader_kind(Path(record["path"])),
        )
        cache_paths[cache_key].add(
            str(Path(record["path"]).resolve(strict=False)).casefold()
        )
        cache_payload_bytes[cache_key] = record["bytes"]
    duplicate_cache_keys = [
        key for key, paths in cache_paths.items() if len(paths) > 1
    ]
    summary = {
        "catalogAssets": len(catalog),
        "materialReferences": len(records),
        "uniqueTexturePaths": len(
            {str(path.resolve(strict=False)).casefold() for path in unique_paths}
        ),
        "uniquePayloads": len(paths_by_hash),
        "duplicatePayloadGroups": len(duplicate_hashes),
        "deduplicableSourceBytes": sum(
            (len(paths_by_hash[digest]) - 1) * payload_bytes[digest]
            for digest in duplicate_hashes
        ),
        "runtimeCacheDuplicateGroups": len(duplicate_cache_keys),
        "runtimeCacheDeduplicableSourceBytes": sum(
            (len(cache_paths[key]) - 1) * cache_payload_bytes[key]
            for key in duplicate_cache_keys
        ),
        "sourceBytes": sum(path.stat().st_size for path in unique_paths),
        "missingReferences": sum(record["action"] == "MISSING" for record in records),
        "ddsReferences": sum(
            Path(record["path"]).suffix.casefold() == ".dds"
            for record in records if record["sha256"]
        ),
        "convertCandidates": sum(
            record["action"] == "CONVERT_CANDIDATE" for record in records
        ),
    }
    return area_id, records, summary


def apply_candidates(
    records: Iterable[dict[str, Any]],
    asset_root: Path,
    converted_root: Path,
    texconv: Path,
) -> list[dict[str, Any]]:
    recommendations: dict[Path, set[str]] = defaultdict(set)
    for record in records:
        if record["action"] == "CONVERT_CANDIDATE":
            recommendations[Path(record["path"])].add(record["recommendedFormat"])

    receipt: list[dict[str, Any]] = []
    for source in sorted(recommendations, key=lambda value: str(value).casefold()):
        formats = recommendations[source]
        row: dict[str, Any] = {
            "source": str(source),
            "sourceSha256": sha256_file(source),
            "recommendedFormats": sorted(formats),
            "status": "SKIPPED_CONFLICTING_SEMANTICS",
        }
        if len(formats) != 1:
            receipt.append(row)
            continue
        try:
            relative = source.resolve().relative_to(asset_root.resolve())
        except ValueError:
            relative = Path("_external") / source.name
        target = (converted_root / relative).with_suffix(".dds")
        target.parent.mkdir(parents=True, exist_ok=True)
        texture_format = next(iter(formats))
        with tempfile.TemporaryDirectory(
            prefix="texconv-", dir=target.parent
        ) as temporary_name:
            temporary = Path(temporary_name)
            command = [
                str(texconv),
                "-nologo",
                "-y",
                "-m",
                "0",
                "-f",
                texture_format,
                "-o",
                str(temporary),
                str(source),
            ]
            process = subprocess.run(
                command, capture_output=True, text=True, check=False
            )
            row["command"] = command
            row["exitCode"] = process.returncode
            row["stdout"] = process.stdout[-4096:]
            row["stderr"] = process.stderr[-4096:]
            if process.returncode != 0:
                row["status"] = "FAILED_TEXCONV"
                receipt.append(row)
                continue
            candidates = [
                path for path in temporary.iterdir()
                if path.is_file() and path.suffix.casefold() == ".dds"
            ]
            if len(candidates) != 1:
                row["status"] = "FAILED_OUTPUT_COUNT"
                receipt.append(row)
                continue
            metadata = inspect_dds(candidates[0])
            source_metadata = inspect_texture(source)
            if metadata["format"] != texture_format:
                row["status"] = "FAILED_FORMAT_MISMATCH"
                receipt.append(row)
                continue
            if metadata["dimensions"] != source_metadata["dimensions"]:
                row["status"] = "FAILED_DIMENSION_MISMATCH"
                receipt.append(row)
                continue
            if max(metadata["dimensions"]) > 1 and metadata["mips"] <= 1:
                row["status"] = "FAILED_MIP_VALIDATION"
                receipt.append(row)
                continue
            candidates[0].replace(target)
        row.update(
            {
                "target": str(target),
                "targetSha256": sha256_file(target),
                "targetBytes": target.stat().st_size,
                "metadata": inspect_dds(target),
                "status": "WRITTEN",
            }
        )
        receipt.append(row)
    return receipt


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--catalog", required=True, type=Path)
    parser.add_argument("--asset-root", required=True, type=Path)
    parser.add_argument("--report", required=True, type=Path)
    parser.add_argument("--apply", action="store_true")
    parser.add_argument("--texconv", type=Path)
    parser.add_argument("--converted-root", type=Path)
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    if not args.catalog.is_file():
        parser.error(f"catalog is missing: {args.catalog}")
    if not args.asset_root.is_dir():
        parser.error(f"asset root is missing: {args.asset_root}")
    if args.apply and (
        args.texconv is None
        or not args.texconv.is_file()
        or args.converted_root is None
        or args.receipt is None
    ):
        parser.error(
            "--apply requires existing --texconv, --converted-root, and --receipt"
        )

    area_id, records, summary = audit(args.catalog, args.asset_root)
    report = {
        "schema": "LostArkTextureAudit.v1",
        "areaId": area_id,
        "catalog": str(args.catalog.resolve()),
        "catalogSha256": sha256_file(args.catalog),
        "assetRoot": str(args.asset_root.resolve()),
        "summary": summary,
        "textures": records,
    }
    atomic_write_json(args.report, report)
    if args.apply:
        converted = apply_candidates(
            records, args.asset_root, args.converted_root, args.texconv
        )
        atomic_write_json(
            args.receipt,
            {
                "schema": "LostArkTextureOptimizeReceipt.v1",
                "auditReport": str(args.report.resolve()),
                "auditReportSha256": sha256_file(args.report),
                "sourceMutation": False,
                "convertedRoot": str(args.converted_root.resolve()),
                "results": converted,
            },
        )
    print(json.dumps(summary, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

#### navgrid bake

```text
python build_valtan_navgrid.py \
  --floor01 C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Map/StaticMesh_Raw_20260729/BG/BG_RAD_VALTAN_A__542N9YJ2R1Y3NYH2RYFTMPE9/BG_RAD_VALTAN_A/StaticMesh3/bg_rad_valtan_floor01_sm.gltf \
  --floor01a C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Map/StaticMesh_Raw_20260729/BG/BG_RAD_VALTAN_A__542N9YJ2R1Y3NYH2RYFTMPE9/BG_RAD_VALTAN_A/StaticMesh3/bg_rad_valtan_floor01a_sm.gltf \
  --floor01b C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Map/StaticMesh_Raw_20260729/BG/BG_RAD_VALTAN_A__542N9YJ2R1Y3NYH2RYFTMPE9/BG_RAD_VALTAN_A/StaticMesh3/bg_rad_valtan_floor01b_sm.gltf \
  --placements Tools/LevelPlacementExtractor/heartrb_valtan_core_overlay.json \
  --mapplacements Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.mapplacements \
  --bounds-gltf C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Map/StaticMesh_Raw_20260729/LV/LV_NAVIMESH__1Z0LFWZG8OE9D61V6R2IMO/LV_NAVIMESH/StaticMesh3/lv_common_mesh_cul_box_8.gltf \
  --cell-size 0.5 \
  --agent-radius 0.4 \
  --max-slope 45 \
  --max-step 0.6 \
  --output Client/Bin/DataFiles/Navigation/ValtanArena.navgrid \
  --receipt Client/Bin/DataFiles/Navigation/ValtanArena.navgrid.receipt.json
```

script는 입력 경로가 없거나 exact assetId가 일치하지 않으면 실패한다. 자동으로 임의 원형 fallback을 만들지 않는다.
첫 bake는 `--navpatch` 없이 실행한다. cell을 확인하고 MapTool이 patch를 저장한 뒤 같은
명령에 `--navpatch Client/Bin/DataFiles/Navigation/ValtanArena.navpatch.json`만 추가해
최종 binary를 다시 생성한다.

#### `Tools/LevelPlacementExtractor/build_valtan_navgrid.py` 전체 내용

```python
from __future__ import annotations

import argparse
import base64
import hashlib
import json
import math
import shlex
import struct
from collections import deque
from pathlib import Path
from typing import Any

import numpy as np


MAGIC = b"WNAVGRD\0"
VERSION = 1
TOOL_VERSION = "1.0.0"
ASSET_PATH_ARGUMENTS = {
    "BG_RAD_VALTAN_FLOOR01_SM": "floor01",
    "BG_RAD_VALTAN_FLOOR01A_SM": "floor01a",
    "BG_RAD_VALTAN_FLOOR01B_SM": "floor01b",
}
BOUNDS_ASSET_ID = "MAP_1E5F55FC0524_LV_COMMON_MESH_CUL_BOX_8"
BOUNDS_SOURCE_PLACEMENT_ID = "LV_LUT_HEARTRB_ED_SL01:export:2767"
COMPONENT_DTYPES = {
    5121: np.dtype("<u1"),
    5123: np.dtype("<u2"),
    5125: np.dtype("<u4"),
    5126: np.dtype("<f4"),
}
TYPE_COUNTS = {
    "SCALAR": 1,
    "VEC2": 2,
    "VEC3": 3,
    "VEC4": 4,
    "MAT4": 16,
}


def read_gltf_document(path: Path) -> tuple[dict[str, Any], list[bytes]]:
    if path.suffix.lower() == ".glb":
        raw = path.read_bytes()
        if len(raw) < 12 or raw[:4] != b"glTF":
            raise ValueError(f"invalid GLB: {path}")
        _, version, declared_size = struct.unpack_from("<4sII", raw, 0)
        if version != 2 or declared_size != len(raw):
            raise ValueError(f"unsupported GLB header: {path}")
        cursor = 12
        json_chunk: bytes | None = None
        binary_chunks: list[bytes] = []
        while cursor < len(raw):
            if cursor + 8 > len(raw):
                raise ValueError("truncated GLB chunk")
            length, kind = struct.unpack_from("<II", raw, cursor)
            cursor += 8
            chunk = raw[cursor : cursor + length]
            cursor += length
            if len(chunk) != length:
                raise ValueError("truncated GLB payload")
            if kind == 0x4E4F534A:
                json_chunk = chunk
            elif kind == 0x004E4942:
                binary_chunks.append(chunk)
        if json_chunk is None:
            raise ValueError("GLB has no JSON chunk")
        document = json.loads(json_chunk.rstrip(b" \t\r\n\0"))
        glb_binary = binary_chunks[0] if binary_chunks else b""
    else:
        document = json.loads(path.read_text(encoding="utf-8"))
        glb_binary = b""

    buffers: list[bytes] = []
    for index, buffer in enumerate(document.get("buffers", [])):
        uri = buffer.get("uri")
        if uri is None:
            if index != 0 or not glb_binary:
                raise ValueError("buffer without URI has no GLB BIN chunk")
            payload = glb_binary
        elif uri.startswith("data:"):
            marker = uri.find(",")
            if marker < 0 or ";base64" not in uri[:marker]:
                raise ValueError("only base64 data URI buffers are supported")
            payload = base64.b64decode(uri[marker + 1 :], validate=True)
        else:
            payload = (path.parent / uri).resolve().read_bytes()
        if len(payload) < int(buffer.get("byteLength", 0)):
            raise ValueError(f"short glTF buffer {index}")
        buffers.append(payload)
    return document, buffers


def accessor_array(
    document: dict[str, Any], buffers: list[bytes], accessor_index: int
) -> np.ndarray:
    accessor = document["accessors"][accessor_index]
    if "sparse" in accessor:
        raise ValueError("sparse glTF accessors are not supported")
    view = document["bufferViews"][accessor["bufferView"]]
    dtype = COMPONENT_DTYPES.get(accessor["componentType"])
    width = TYPE_COUNTS.get(accessor["type"])
    if dtype is None or width is None:
        raise ValueError("unsupported glTF accessor type")
    count = int(accessor["count"])
    offset = int(view.get("byteOffset", 0)) + int(accessor.get("byteOffset", 0))
    packed_stride = dtype.itemsize * width
    stride = int(view.get("byteStride", packed_stride))
    if stride < packed_stride:
        raise ValueError("invalid glTF byteStride")
    payload = buffers[view["buffer"]]
    required = offset + (count - 1) * stride + packed_stride if count else offset
    if required > len(payload):
        raise ValueError("glTF accessor exceeds its buffer")
    result = np.ndarray(
        shape=(count, width), dtype=dtype, buffer=payload,
        offset=offset, strides=(stride, dtype.itemsize)
    ).copy()
    return result


def quaternion_matrix(value: list[float]) -> np.ndarray:
    x, y, z, w = (float(component) for component in value)
    length = math.sqrt(x * x + y * y + z * z + w * w)
    if not math.isfinite(length) or length < 1e-12:
        raise ValueError("zero or non-finite quaternion")
    x, y, z, w = x / length, y / length, z / length, w / length
    return np.array([
        [1 - 2 * (y * y + z * z), 2 * (x * y - z * w),
         2 * (x * z + y * w), 0],
        [2 * (x * y + z * w), 1 - 2 * (x * x + z * z),
         2 * (y * z - x * w), 0],
        [2 * (x * z - y * w), 2 * (y * z + x * w),
         1 - 2 * (x * x + y * y), 0],
        [0, 0, 0, 1],
    ], dtype=np.float64)


def node_matrix(node: dict[str, Any]) -> np.ndarray:
    if "matrix" in node:
        values = np.asarray(node["matrix"], dtype=np.float64)
        if values.shape != (16,):
            raise ValueError("invalid glTF node matrix")
        return values.reshape((4, 4), order="F")
    translation = np.eye(4, dtype=np.float64)
    translation[:3, 3] = np.asarray(
        node.get("translation", [0, 0, 0]), dtype=np.float64)
    rotation = quaternion_matrix(node.get("rotation", [0, 0, 0, 1]))
    scale = np.eye(4, dtype=np.float64)
    scale_values = np.asarray(node.get("scale", [1, 1, 1]), dtype=np.float64)
    scale[0, 0], scale[1, 1], scale[2, 2] = scale_values
    return translation @ rotation @ scale


def load_triangles(path: Path, source_scale: float) -> tuple[np.ndarray, list[bytes]]:
    document, buffers = read_gltf_document(path)
    triangles: list[np.ndarray] = []
    nodes = document.get("nodes", [])

    def visit(node_index: int, parent: np.ndarray) -> None:
        node = nodes[node_index]
        transform = parent @ node_matrix(node)
        if "mesh" in node:
            mesh = document["meshes"][node["mesh"]]
            for primitive in mesh.get("primitives", []):
                if int(primitive.get("mode", 4)) != 4:
                    continue
                position_index = primitive.get("attributes", {}).get("POSITION")
                if position_index is None:
                    continue
                positions = accessor_array(document, buffers, position_index)
                if positions.shape[1] != 3:
                    raise ValueError("POSITION accessor is not VEC3")
                homogeneous = np.concatenate(
                    [positions.astype(np.float64),
                     np.ones((len(positions), 1), dtype=np.float64)], axis=1)
                transformed = (transform @ homogeneous.T).T[:, :3] * source_scale
                if "indices" in primitive:
                    indices = accessor_array(
                        document, buffers, primitive["indices"]).reshape(-1)
                else:
                    indices = np.arange(len(transformed), dtype=np.uint32)
                if len(indices) % 3:
                    raise ValueError("triangle index count is not divisible by three")
                triangles.extend(transformed[indices].reshape((-1, 3, 3)))
        for child in node.get("children", []):
            visit(int(child), transform)

    scene_index = int(document.get("scene", 0))
    scenes = document.get("scenes", [])
    roots = scenes[scene_index].get("nodes", []) if scenes else range(len(nodes))
    for root in roots:
        visit(int(root), np.eye(4, dtype=np.float64))
    if not triangles:
        raise ValueError(f"no triangle primitives in {path}")
    return np.asarray(triangles, dtype=np.float64), buffers


def placement_matrix(row: dict[str, Any]) -> np.ndarray:
    position = np.asarray(row["position"], dtype=np.float64)
    scale_value = np.asarray(row["scale"], dtype=np.float64)
    if position.shape != (3,) or scale_value.shape != (3,):
        raise ValueError("placement position/scale must have three values")
    translation = np.eye(4, dtype=np.float64)
    translation[:3, 3] = position
    scale = np.eye(4, dtype=np.float64)
    scale[0, 0], scale[1, 1], scale[2, 2] = scale_value
    return translation @ quaternion_matrix(row["quaternion"]) @ scale


def load_runtime_placements(path: Path) -> list[dict[str, Any]]:
    lines = path.read_text(encoding="utf-8").splitlines()
    if not lines:
        raise ValueError("empty mapplacements document")
    header = shlex.split(lines[0])
    if len(header) != 4 or header[:2] != ["LOSTARK_MAP_PLACEMENTS", "2"]:
        raise ValueError("unsupported mapplacements header")
    declared_count = int(header[3])
    rows: list[dict[str, Any]] = []
    for line_number, line in enumerate(lines[1:], 2):
        if not line.strip():
            continue
        values = shlex.split(line)
        if len(values) != 16:
            raise ValueError(
                f"mapplacements line {line_number}: expected 16 fields")
        rows.append({
            "placementId": int(values[0]),
            "sourcePlacementId": values[1],
            "sourceLevel": values[2],
            "transformSource": values[3],
            "assetId": values[4],
            "position": [float(value) for value in values[5:8]],
            "quaternion": [float(value) for value in values[8:12]],
            "scale": [float(value) for value in values[12:15]],
            "visible": values[15] == "1",
        })
    if len(rows) != declared_count:
        raise ValueError(
            f"mapplacements count mismatch: {len(rows)} != {declared_count}")
    return rows


def apply_placement(triangles: np.ndarray, matrix: np.ndarray) -> np.ndarray:
    flat = triangles.reshape((-1, 3))
    homogeneous = np.concatenate(
        [flat, np.ones((len(flat), 1), dtype=np.float64)], axis=1)
    return (matrix @ homogeneous.T).T[:, :3].reshape((-1, 3, 3))


def rasterize(
    triangles: np.ndarray,
    bounds_min: np.ndarray,
    bounds_max: np.ndarray,
    cell_size: float,
    max_slope: float,
) -> tuple[float, float, int, int, np.ndarray, np.ndarray]:
    if bounds_min.shape != (3,) or bounds_max.shape != (3,) or np.any(
        bounds_max <= bounds_min
    ):
        raise ValueError("invalid bake bounds")
    origin_x = math.floor(float(bounds_min[0]) / cell_size) * cell_size
    origin_z = math.floor(float(bounds_min[2]) / cell_size) * cell_size
    width = math.ceil((float(bounds_max[0]) - origin_x) / cell_size)
    height = math.ceil((float(bounds_max[2]) - origin_z) / cell_size)
    if width <= 0 or height <= 0 or width * height > 16_777_216:
        raise ValueError("computed NavGrid dimensions are invalid")
    heights = np.full((height, width), np.nan, dtype=np.float32)
    candidates = np.zeros((height, width), dtype=np.bool_)
    minimum_normal_y = math.cos(math.radians(max_slope))

    for triangle in triangles:
        edge_a = triangle[1] - triangle[0]
        edge_b = triangle[2] - triangle[0]
        normal = np.cross(edge_a, edge_b)
        length = np.linalg.norm(normal)
        if length < 1e-12 or abs(float(normal[1] / length)) < minimum_normal_y:
            continue
        xz = triangle[:, [0, 2]]
        denominator = ((xz[1, 1] - xz[2, 1]) * (xz[0, 0] - xz[2, 0]) +
                       (xz[2, 0] - xz[1, 0]) * (xz[0, 1] - xz[2, 1]))
        if abs(float(denominator)) < 1e-12:
            continue
        min_x = max(0, math.floor((float(xz[:, 0].min()) - origin_x) / cell_size))
        max_x = min(width - 1, math.floor((float(xz[:, 0].max()) - origin_x) / cell_size))
        min_z = max(0, math.floor((float(xz[:, 1].min()) - origin_z) / cell_size))
        max_z = min(height - 1, math.floor((float(xz[:, 1].max()) - origin_z) / cell_size))
        for z in range(min_z, max_z + 1):
            sample_z = origin_z + (z + 0.5) * cell_size
            for x in range(min_x, max_x + 1):
                sample_x = origin_x + (x + 0.5) * cell_size
                a = ((xz[1, 1] - xz[2, 1]) * (sample_x - xz[2, 0]) +
                     (xz[2, 0] - xz[1, 0]) * (sample_z - xz[2, 1])) / denominator
                b = ((xz[2, 1] - xz[0, 1]) * (sample_x - xz[2, 0]) +
                     (xz[0, 0] - xz[2, 0]) * (sample_z - xz[2, 1])) / denominator
                c = 1.0 - a - b
                if min(a, b, c) < -1e-7:
                    continue
                y = float(a * triangle[0, 1] + b * triangle[1, 1] +
                          c * triangle[2, 1])
                if not candidates[z, x] or y > heights[z, x]:
                    candidates[z, x] = True
                    heights[z, x] = y
    return origin_x, origin_z, width, height, candidates, heights


def connected_component(
    mask: np.ndarray, heights: np.ndarray,
    seed_x: int, seed_z: int, max_step: float
) -> np.ndarray:
    height, width = mask.shape
    walkable_cells = np.argwhere(mask)
    if len(walkable_cells) == 0:
        raise ValueError("rasterization produced no walkable cells")
    if not (0 <= seed_x < width and 0 <= seed_z < height and mask[seed_z, seed_x]):
        distances = ((walkable_cells[:, 1] - seed_x) ** 2 +
                     (walkable_cells[:, 0] - seed_z) ** 2)
        seed_z, seed_x = walkable_cells[int(np.argmin(distances))]
    result = np.zeros_like(mask)
    result[seed_z, seed_x] = True
    queue = deque([(int(seed_x), int(seed_z))])
    while queue:
        x, z = queue.popleft()
        for dx, dz in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nx, nz = x + dx, z + dz
            if not (0 <= nx < width and 0 <= nz < height):
                continue
            if result[nz, nx] or not mask[nz, nx]:
                continue
            if abs(float(heights[nz, nx] - heights[z, x])) > max_step:
                continue
            result[nz, nx] = True
            queue.append((nx, nz))
    return result


def component_count(
    mask: np.ndarray, heights: np.ndarray, max_step: float
) -> int:
    height, width = mask.shape
    visited = np.zeros_like(mask)
    count = 0
    for seed_z, seed_x in np.argwhere(mask):
        seed_x, seed_z = int(seed_x), int(seed_z)
        if visited[seed_z, seed_x]:
            continue
        count += 1
        visited[seed_z, seed_x] = True
        queue = deque([(seed_x, seed_z)])
        while queue:
            x, z = queue.popleft()
            for dx, dz in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                nx, nz = x + dx, z + dz
                if not (0 <= nx < width and 0 <= nz < height):
                    continue
                if visited[nz, nx] or not mask[nz, nx]:
                    continue
                if abs(float(heights[nz, nx] - heights[z, x])) > max_step:
                    continue
                visited[nz, nx] = True
                queue.append((nx, nz))
    return count


def erode(mask: np.ndarray, radius_cells: int) -> np.ndarray:
    if radius_cells <= 0:
        return mask.copy()
    height, width = mask.shape
    result = mask.copy()
    offsets = [(dx, dz)
               for dz in range(-radius_cells, radius_cells + 1)
               for dx in range(-radius_cells, radius_cells + 1)
               if dx * dx + dz * dz <= radius_cells * radius_cells]
    for z, x in np.argwhere(mask):
        for dx, dz in offsets:
            nx, nz = int(x) + dx, int(z) + dz
            if not (0 <= nx < width and 0 <= nz < height) or not mask[nz, nx]:
                result[z, x] = False
                break
    return result


def apply_force_blocked_patch(
    mask: np.ndarray,
    path: Path | None,
    base_source_sha256: str,
    origin_x: float,
    origin_z: float,
    cell_size: float,
) -> tuple[np.ndarray, str | None, int]:
    if path is None:
        return mask.copy(), None, 0
    raw = path.read_bytes()
    value = json.loads(raw.decode("utf-8"))
    height, width = mask.shape
    if value.get("schema") != "LostArkNavGridPatch.v1":
        raise ValueError("unsupported navpatch schema")
    if value.get("mapId") != "LV_LUT_HEARTRB_ED":
        raise ValueError("navpatch mapId mismatch")
    if value.get("baseSourceSha256") != base_source_sha256:
        raise ValueError("navpatch base source hash mismatch")
    if value.get("width") != width or value.get("height") != height:
        raise ValueError("navpatch dimensions mismatch")
    if not math.isclose(float(value.get("cellSize", -1.0)), cell_size,
                        rel_tol=0.0, abs_tol=1e-6):
        raise ValueError("navpatch cellSize mismatch")
    origin = value.get("origin")
    if (not isinstance(origin, list) or len(origin) != 2 or
        not math.isclose(float(origin[0]), origin_x, rel_tol=0.0, abs_tol=1e-6) or
        not math.isclose(float(origin[1]), origin_z, rel_tol=0.0, abs_tol=1e-6)):
        raise ValueError("navpatch origin mismatch")

    indices = value.get("forceBlocked")
    if not isinstance(indices, list) or any(
        isinstance(index, bool) or not isinstance(index, int)
        for index in indices
    ):
        raise ValueError("navpatch forceBlocked must be an integer array")
    if indices != sorted(set(indices)):
        raise ValueError("navpatch forceBlocked must be sorted and unique")

    result = mask.copy()
    for index in indices:
        if index < 0 or index >= width * height:
            raise ValueError("navpatch cell index is out of range")
        result[index // width, index % width] = False
    return result, hashlib.sha256(raw).hexdigest(), len(indices)


def atomic_write(path: Path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + ".tmp")
    temporary.write_bytes(data)
    temporary.replace(path)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--floor01", required=True, type=Path)
    parser.add_argument("--floor01a", required=True, type=Path)
    parser.add_argument("--floor01b", required=True, type=Path)
    parser.add_argument("--placements", required=True, type=Path)
    parser.add_argument("--mapplacements", required=True, type=Path)
    parser.add_argument("--bounds-gltf", required=True, type=Path)
    parser.add_argument("--navpatch", type=Path)
    parser.add_argument("--source-scale", type=float, default=0.01)
    parser.add_argument("--cell-size", type=float, default=0.5)
    parser.add_argument("--agent-radius", type=float, default=0.4)
    parser.add_argument("--max-slope", type=float, default=45.0)
    parser.add_argument("--max-step", type=float, default=0.6)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--receipt", required=True, type=Path)
    args = parser.parse_args()
    if args.source_scale <= 0 or args.cell_size <= 0 or args.agent_radius < 0:
        parser.error("scale/cell-size must be positive and radius non-negative")
    required_paths = (
        args.floor01, args.floor01a, args.floor01b, args.placements,
        args.mapplacements, args.bounds_gltf,
    )
    for path in required_paths:
        if not path.is_file():
            parser.error(f"missing exact input: {path}")
    if args.navpatch is not None and not args.navpatch.is_file():
        parser.error(f"missing navpatch: {args.navpatch}")

    overlay_bytes = args.placements.read_bytes()
    overlay = json.loads(overlay_bytes.decode("utf-8"))
    placements = overlay.get("placements", [])
    source_paths = {
        asset: getattr(args, argument)
        for asset, argument in ASSET_PATH_ARGUMENTS.items()
    }
    source_triangles: dict[str, np.ndarray] = {}
    source_hash = hashlib.sha256()
    source_hash.update(overlay_bytes)

    runtime_placement_bytes = args.mapplacements.read_bytes()
    runtime_placements = load_runtime_placements(args.mapplacements)
    bounds_rows = [row for row in runtime_placements
                   if row["assetId"] == BOUNDS_ASSET_ID and
                   row["sourcePlacementId"] == BOUNDS_SOURCE_PLACEMENT_ID]
    if len(bounds_rows) != 1:
        raise ValueError(
            f"expected one exact CUL_BOX_8 placement, found {len(bounds_rows)}")
    bounds_source, bounds_buffers = load_triangles(
        args.bounds_gltf, args.source_scale)
    bounds_world = apply_placement(
        bounds_source, placement_matrix(bounds_rows[0]))
    bounds_min = bounds_world.min(axis=(0, 1))
    bounds_max = bounds_world.max(axis=(0, 1))
    source_hash.update(runtime_placement_bytes)
    source_hash.update(args.bounds_gltf.read_bytes())
    for payload in bounds_buffers:
        source_hash.update(payload)
    for asset, path in source_paths.items():
        triangles, buffers = load_triangles(path, args.source_scale)
        source_triangles[asset] = triangles
        source_hash.update(path.read_bytes())
        for payload in buffers:
            source_hash.update(payload)

    world_triangles: list[np.ndarray] = []
    used_ids: list[int] = []
    seen_assets: set[str] = set()
    for placement in placements:
        asset = placement.get("assetId")
        if asset not in source_triangles:
            continue
        world_triangles.append(apply_placement(
            source_triangles[asset], placement_matrix(placement)))
        used_ids.append(int(placement["placementId"]))
        seen_assets.add(asset)
    if seen_assets != set(source_paths):
        missing = sorted(set(source_paths) - seen_assets)
        raise ValueError(f"exact floor placement is missing: {missing}")
    triangles = np.concatenate(world_triangles, axis=0)

    option_bytes = json.dumps({
        "toolVersion": TOOL_VERSION,
        "sourceScale": args.source_scale,
        "cellSize": args.cell_size,
        "agentRadius": args.agent_radius,
        "maxSlope": args.max_slope,
        "maxStep": args.max_step,
        "placementIds": used_ids,
        "boundsAssetId": BOUNDS_ASSET_ID,
        "boundsSourcePlacementId": BOUNDS_SOURCE_PLACEMENT_ID,
    }, sort_keys=True, separators=(",", ":")).encode("utf-8")
    source_hash.update(option_bytes)
    base_source_sha256 = source_hash.hexdigest()

    origin_x, origin_z, width, height, candidates, heights = rasterize(
        triangles, bounds_min, bounds_max, args.cell_size, args.max_slope)
    candidate_component_count = component_count(
        candidates, heights, args.max_step)
    center = (bounds_min + bounds_max) * 0.5
    seed_x = math.floor((float(center[0]) - origin_x) / args.cell_size)
    seed_z = math.floor((float(center[2]) - origin_z) / args.cell_size)
    connected = connected_component(
        candidates, heights, seed_x, seed_z, args.max_step)
    radius_cells = math.ceil(args.agent_radius / args.cell_size)
    walkable = erode(connected, radius_cells)
    walkable, patch_sha256, force_blocked_count = apply_force_blocked_patch(
        walkable, args.navpatch, base_source_sha256,
        origin_x, origin_z, args.cell_size)
    walkable = connected_component(
        walkable, heights, seed_x, seed_z, args.max_step)
    finite_heights = heights[walkable]
    if finite_heights.size == 0:
        raise ValueError("agent-radius erosion removed every walkable cell")

    flat = walkable.reshape(-1)
    bitset = bytearray((flat.size + 7) // 8)
    for index, value in enumerate(flat):
        if value:
            bitset[index >> 3] |= 1 << (index & 7)
    final_source_hash = source_hash.copy()
    if args.navpatch is not None:
        final_source_hash.update(args.navpatch.read_bytes())
    digest = final_source_hash.digest()
    header = struct.pack(
        "<8sIII8fII32s", MAGIC, VERSION, width, height,
        args.cell_size, origin_x, origin_z,
        float(finite_heights.min()), float(finite_heights.max()),
        args.agent_radius, args.max_slope, args.max_step,
        len(bitset), flat.size, digest,
    )
    height_payload = heights.astype("<f4", copy=False).tobytes(order="C")
    atomic_write(args.output, header + bytes(bitset) + height_payload)
    output_hash = hashlib.sha256(args.output.read_bytes()).hexdigest()
    receipt = {
        "schema": "LostArkValtanNavGridReceipt.v1",
        "toolVersion": TOOL_VERSION,
        "output": str(args.output.resolve()),
        "outputSha256": output_hash,
        "sourceSha256": digest.hex(),
        "baseSourceSha256": base_source_sha256,
        "sourcePaths": {key: str(value.resolve()) for key, value in source_paths.items()},
        "placementFile": str(args.placements.resolve()),
        "runtimePlacementFile": str(args.mapplacements.resolve()),
        "boundsGltf": str(args.bounds_gltf.resolve()),
        "navPatch": None if args.navpatch is None else str(args.navpatch.resolve()),
        "navPatchSha256": patch_sha256,
        "forceBlockedCount": force_blocked_count,
        "placementIds": used_ids,
        "options": json.loads(option_bytes),
        "grid": {
            "width": width,
            "height": height,
            "walkableCount": int(np.count_nonzero(walkable)),
            "candidateCount": int(np.count_nonzero(candidates)),
            "candidateComponentCount": candidate_component_count,
            "connectedComponentCount": component_count(
                walkable, heights, args.max_step),
            "origin": [origin_x, origin_z],
            "boundsMin": bounds_min.tolist(),
            "boundsMax": bounds_max.tolist(),
            "minHeight": float(finite_heights.min()),
            "maxHeight": float(finite_heights.max()),
        },
    }
    atomic_write(
        args.receipt,
        (json.dumps(receipt, ensure_ascii=False, indent=2) + "\n").encode("utf-8"),
    )
    print(json.dumps(receipt["grid"], ensure_ascii=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

#### capture compare

```text
python compare_profiler_captures.py \
  --before .codex_tmp/profiles/valtan_before.json \
  --after .codex_tmp/profiles/valtan_after.json \
  --output .md/GB/07-30/2026-07-30_LOSTARK_VALTAN_OPTIMIZATION_RESULT.md
```

#### `Tools/LevelPlacementExtractor/compare_profiler_captures.py` 전체 내용

```python
from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
from typing import Any, Iterable


SCHEMA = "LostArkProfilerTimeline.v1"
METRICS = (
    ("CPU frame (ms)", ("cpuMs", "frame"), True),
    ("CPU update (ms)", ("cpuMs", "update"), True),
    ("CPU render (ms)", ("cpuMs", "render"), True),
    ("GPU frame (ms)", ("gpuMs", "frame"), True),
    ("Draw calls", ("draw", "calls"), True),
    ("Instanced draw calls", ("draw", "instancedCalls"), False),
    ("Rendered instances", ("draw", "instances"), False),
    ("Indices", ("draw", "indices"), True),
    ("IA vertices", ("pipeline", "iaVertices"), True),
    ("IA primitives", ("pipeline", "iaPrimitives"), True),
    ("VS invocations", ("pipeline", "vsInvocations"), True),
    ("PS invocations", ("pipeline", "psInvocations"), True),
    ("Map placements", ("map", "placements"), False),
    ("Visible map instances", ("map", "visibleInstances"), False),
    ("Map batches", ("map", "batchCount"), True),
    ("Map fallback objects", ("map", "fallbackObjects"), True),
    ("Texture requests", ("texture", "requests"), True),
    ("Texture path hits", ("texture", "pathHits"), False),
    ("Texture content hits", ("texture", "contentHits"), False),
    ("Unique SRVs", ("texture", "uniqueSrvs"), True),
    ("Estimated texture GPU bytes", ("texture", "estimatedGpuBytes"), True),
    ("Navigation queries", ("navigation", "queries"), False),
    ("Navigation expanded nodes", ("navigation", "expandedNodes"), True),
    ("Navigation query (us)", ("navigation", "queryUs"), True),
    ("Navigation path cells", ("navigation", "pathCells"), False),
)


def atomic_write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + ".tmp")
    temporary.write_text(text, encoding="utf-8")
    temporary.replace(path)


def finite_number(value: Any, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ValueError(f"{label} is not numeric")
    result = float(value)
    if not math.isfinite(result):
        raise ValueError(f"{label} is not finite")
    return result


def nested_value(frame: dict[str, Any], path: tuple[str, ...]) -> float:
    value: Any = frame
    for component in path:
        if not isinstance(value, dict) or component not in value:
            raise ValueError(f"frame is missing {'.'.join(path)}")
        value = value[component]
    return finite_number(value, ".".join(path))


def percentile(values: Iterable[float], fraction: float) -> float:
    ordered = sorted(values)
    if not ordered:
        return 0.0
    position = fraction * (len(ordered) - 1)
    lower = int(math.floor(position))
    upper = min(lower + 1, len(ordered) - 1)
    weight = position - lower
    return ordered[lower] * (1.0 - weight) + ordered[upper] * weight


def load_capture(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict) or value.get("schema") != SCHEMA:
        raise ValueError(f"{path}: unsupported profiler schema")
    for key in ("build", "scene", "capture", "dropped", "frames", "summary"):
        if key not in value:
            raise ValueError(f"{path}: missing root field {key}")
    if not isinstance(value["frames"], list) or not value["frames"]:
        raise ValueError(f"{path}: capture has no frames")
    declared = value["capture"].get("frameCount")
    if declared != len(value["frames"]):
        raise ValueError(
            f"{path}: frame count mismatch "
            f"({declared} != {len(value['frames'])})"
        )
    previous_frame: int | None = None
    for index, frame in enumerate(value["frames"]):
        if not isinstance(frame, dict):
            raise ValueError(f"{path}: frame {index} is not an object")
        frame_number = frame.get("frame")
        if not isinstance(frame_number, int) or frame_number < 0:
            raise ValueError(f"{path}: frame {index} has invalid frame number")
        if previous_frame is not None and frame_number <= previous_frame:
            raise ValueError(f"{path}: frame numbers are not strictly increasing")
        previous_frame = frame_number
        for _label, metric_path, _lower_is_better in METRICS:
            nested_value(frame, metric_path)
        cpu_scopes = frame.get("cpuScopes", {})
        if not isinstance(cpu_scopes, dict):
            raise ValueError(f"{path}: frame {index} has invalid cpuScopes")
        for name, milliseconds in cpu_scopes.items():
            if not isinstance(name, str) or not name:
                raise ValueError(f"{path}: frame {index} has invalid scope name")
            finite_number(milliseconds, f"cpuScopes.{name}")
        gpu = frame.get("gpuMs")
        if not isinstance(gpu, dict) or not isinstance(gpu.get("valid"), bool):
            raise ValueError(f"{path}: frame {index} has invalid gpu validity")
    return value


def values_for(
    capture: dict[str, Any], metric_path: tuple[str, ...]
) -> list[float]:
    frames = capture["frames"]
    if metric_path == ("gpuMs", "frame"):
        frames = [frame for frame in frames if frame["gpuMs"]["valid"]]
    return [nested_value(frame, metric_path) for frame in frames]


def stats(values: list[float]) -> dict[str, float]:
    if not values:
        return {"p50": 0.0, "p95": 0.0, "p99": 0.0, "mean": 0.0}
    return {
        "p50": percentile(values, 0.50),
        "p95": percentile(values, 0.95),
        "p99": percentile(values, 0.99),
        "mean": sum(values) / len(values),
    }


def scope_p95(capture: dict[str, Any]) -> dict[str, float]:
    names = {
        name
        for frame in capture["frames"]
        for name in frame.get("cpuScopes", {})
    }
    return {
        name: percentile(
            [
                finite_number(
                    frame.get("cpuScopes", {}).get(name, 0.0),
                    f"cpuScopes.{name}",
                )
                for frame in capture["frames"]
            ],
            0.95,
        )
        for name in names
    }


def percent_change(before: float, after: float) -> str:
    if before == 0.0:
        return "n/a" if after != 0.0 else "0.00%"
    return f"{(after - before) * 100.0 / before:+.2f}%"


def escape_markdown(value: Any) -> str:
    return str(value).replace("|", "\\|").replace("\n", " ")


def build_report(
    before_path: Path,
    before: dict[str, Any],
    after_path: Path,
    after: dict[str, Any],
) -> str:
    before_scene = before["scene"]
    after_scene = after["scene"]
    if (
        before_scene.get("level") != after_scene.get("level")
        or before_scene.get("mapId") != after_scene.get("mapId")
    ):
        raise ValueError("before/after captures are from different scenes")

    lines = [
        "# Valtan profiler before/after comparison",
        "",
        "## Capture identity",
        "",
        "| Field | Before | After |",
        "|---|---:|---:|",
        f"| File | {escape_markdown(before_path)} | {escape_markdown(after_path)} |",
        f"| Configuration | {escape_markdown(before['build'].get('configuration', ''))} | "
        f"{escape_markdown(after['build'].get('configuration', ''))} |",
        f"| Git commit | {escape_markdown(before['build'].get('gitCommit', ''))} | "
        f"{escape_markdown(after['build'].get('gitCommit', ''))} |",
        f"| Level | {escape_markdown(before_scene.get('level', ''))} | "
        f"{escape_markdown(after_scene.get('level', ''))} |",
        f"| Map | {escape_markdown(before_scene.get('mapId', ''))} | "
        f"{escape_markdown(after_scene.get('mapId', ''))} |",
        f"| Frames | {len(before['frames'])} | {len(after['frames'])} |",
        f"| Warmup frames | {before['capture'].get('warmupFrames', 0)} | "
        f"{after['capture'].get('warmupFrames', 0)} |",
        "",
        "## Metric comparison",
        "",
        "| Metric | Stat | Before | After | Delta | Direction |",
        "|---|---:|---:|---:|---:|---|",
    ]
    for label, metric_path, lower_is_better in METRICS:
        before_values = values_for(before, metric_path)
        after_values = values_for(after, metric_path)
        before_stats = stats(before_values)
        after_stats = stats(after_values)
        for statistic in ("p50", "p95", "p99"):
            old = before_stats[statistic]
            new = after_stats[statistic]
            delta = new - old
            if abs(delta) < 1e-12:
                direction = "same"
            elif lower_is_better:
                direction = "improved" if delta < 0 else "regressed"
            else:
                direction = "informational"
            lines.append(
                f"| {label} | {statistic} | {old:.3f} | {new:.3f} | "
                f"{new - old:+.3f} ({percent_change(old, new)}) | {direction} |"
            )

    before_scopes = scope_p95(before)
    after_scopes = scope_p95(after)
    scope_names = sorted(
        set(before_scopes) | set(after_scopes),
        key=lambda name: (
            -after_scopes.get(name, 0.0),
            -before_scopes.get(name, 0.0),
            name,
        ),
    )
    lines.extend(
        [
            "",
            "## CPU scope p95",
            "",
            "| Scope | Before ms | After ms | Delta |",
            "|---|---:|---:|---:|",
        ]
    )
    for name in scope_names[:32]:
        old = before_scopes.get(name, 0.0)
        new = after_scopes.get(name, 0.0)
        lines.append(
            f"| {escape_markdown(name)} | {old:.3f} | {new:.3f} | "
            f"{new - old:+.3f} ({percent_change(old, new)}) |"
        )

    before_gpu_valid = sum(frame["gpuMs"]["valid"] for frame in before["frames"])
    after_gpu_valid = sum(frame["gpuMs"]["valid"] for frame in after["frames"])
    lines.extend(
        [
            "",
            "## Capture health",
            "",
            "| Signal | Before | After |",
            "|---|---:|---:|",
            f"| Valid GPU frames | {before_gpu_valid}/{len(before['frames'])} | "
            f"{after_gpu_valid}/{len(after['frames'])} |",
            f"| Dropped CPU scopes | {before['dropped'].get('cpuScopes', 0)} | "
            f"{after['dropped'].get('cpuScopes', 0)} |",
            f"| Dropped GPU frames | {before['dropped'].get('gpuFrames', 0)} | "
            f"{after['dropped'].get('gpuFrames', 0)} |",
            "",
        ]
    )
    cpu_before = stats(values_for(before, ("cpuMs", "frame")))["p95"]
    cpu_after = stats(values_for(after, ("cpuMs", "frame")))["p95"]
    draw_before = stats(values_for(before, ("draw", "calls")))["p95"]
    draw_after = stats(values_for(after, ("draw", "calls")))["p95"]
    lines.extend(
        [
            "## Decision summary",
            "",
            f"- CPU frame p95: {cpu_before:.3f} ms → {cpu_after:.3f} ms "
            f"({percent_change(cpu_before, cpu_after)})",
            f"- Draw-call p95: {draw_before:.3f} → {draw_after:.3f} "
            f"({percent_change(draw_before, draw_after)})",
            "- GPU 판단은 valid GPU frame 비율이 양쪽 모두 충분할 때만 확정한다.",
            "- placement 수와 시각 회귀가 보존되지 않으면 성능 수치와 무관하게 적용을 기각한다.",
            "",
        ]
    )
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--before", required=True, type=Path)
    parser.add_argument("--after", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    for path in (args.before, args.after):
        if not path.is_file():
            parser.error(f"capture is missing: {path}")
    before = load_capture(args.before)
    after = load_capture(args.after)
    atomic_write(
        args.output,
        build_report(args.before, before, args.after, after),
    )
    print(args.output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

#### `Tools/LevelPlacementExtractor/tests/test_build_valtan_navgrid.py` 전체 내용

```python
from __future__ import annotations

import base64
import json
import struct
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

import numpy as np


TOOLS_ROOT = Path(__file__).resolve().parents[1]
if str(TOOLS_ROOT) not in sys.path:
    sys.path.insert(0, str(TOOLS_ROOT))

import build_valtan_navgrid as navgrid


class BuildValtanNavGridTests(unittest.TestCase):
    def test_connected_component_rejects_step_and_erode_removes_border(self) -> None:
        mask = np.ones((5, 5), dtype=np.bool_)
        heights = np.zeros((5, 5), dtype=np.float32)
        heights[:, 4] = 2.0
        connected = navgrid.connected_component(
            mask, heights, seed_x=2, seed_z=2, max_step=0.6
        )
        self.assertTrue(np.all(connected[:, :4]))
        self.assertFalse(np.any(connected[:, 4]))
        eroded = navgrid.erode(np.ones((5, 5), dtype=np.bool_), 1)
        self.assertEqual(9, int(np.count_nonzero(eroded)))
        self.assertTrue(np.all(eroded[1:4, 1:4]))

    def test_rasterize_keeps_top_walkable_surface(self) -> None:
        triangles = np.asarray(
            [
                [[-1.0, 0.0, -1.0], [1.0, 0.0, -1.0], [1.0, 0.0, 1.0]],
                [[-1.0, 0.0, -1.0], [1.0, 0.0, 1.0], [-1.0, 0.0, 1.0]],
                [[-1.0, 1.0, -1.0], [1.0, 1.0, -1.0], [1.0, 1.0, 1.0]],
                [[-1.0, 1.0, -1.0], [1.0, 1.0, 1.0], [-1.0, 1.0, 1.0]],
            ],
            dtype=np.float64,
        )
        _origin_x, _origin_z, _width, _height, candidates, heights = (
            navgrid.rasterize(triangles, cell_size=0.5, max_slope=45.0)
        )
        self.assertGreater(int(np.count_nonzero(candidates)), 0)
        self.assertTrue(np.allclose(heights[candidates], 1.0))

    @staticmethod
    def write_plane_gltf(path: Path) -> None:
        positions = struct.pack(
            "<12f",
            -1.0, 0.0, -1.0,
            1.0, 0.0, -1.0,
            1.0, 0.0, 1.0,
            -1.0, 0.0, 1.0,
        )
        indices = struct.pack("<6H", 0, 1, 2, 0, 2, 3)
        payload = positions + indices
        document = {
            "asset": {"version": "2.0"},
            "scene": 0,
            "scenes": [{"nodes": [0]}],
            "nodes": [{"mesh": 0}],
            "meshes": [
                {
                    "primitives": [
                        {
                            "attributes": {"POSITION": 0},
                            "indices": 1,
                            "mode": 4,
                        }
                    ]
                }
            ],
            "buffers": [
                {
                    "byteLength": len(payload),
                    "uri": "data:application/octet-stream;base64,"
                    + base64.b64encode(payload).decode("ascii"),
                }
            ],
            "bufferViews": [
                {"buffer": 0, "byteOffset": 0, "byteLength": len(positions)},
                {
                    "buffer": 0,
                    "byteOffset": len(positions),
                    "byteLength": len(indices),
                },
            ],
            "accessors": [
                {
                    "bufferView": 0,
                    "componentType": 5126,
                    "count": 4,
                    "type": "VEC3",
                },
                {
                    "bufferView": 1,
                    "componentType": 5123,
                    "count": 6,
                    "type": "SCALAR",
                },
            ],
        }
        path.write_text(json.dumps(document), encoding="utf-8")

    def test_main_writes_deterministic_binary_and_receipt(self) -> None:
        with tempfile.TemporaryDirectory() as directory_name:
            directory = Path(directory_name)
            gltf = directory / "floor.gltf"
            self.write_plane_gltf(gltf)
            placements = directory / "placements.json"
            placements.write_text(
                json.dumps(
                    {
                        "placements": [
                            {
                                "placementId": index,
                                "assetId": asset_id,
                                "position": [0.0, 0.0, 0.0],
                                "quaternion": [0.0, 0.0, 0.0, 1.0],
                                "scale": [1.0, 1.0, 1.0],
                            }
                            for index, asset_id in enumerate(
                                navgrid.ASSET_PATH_ARGUMENTS, 1
                            )
                        ]
                    }
                ),
                encoding="utf-8",
            )

            outputs: list[bytes] = []
            for run in range(2):
                output = directory / f"arena-{run}.navgrid"
                receipt = directory / f"arena-{run}.receipt.json"
                arguments = [
                    "build_valtan_navgrid.py",
                    "--floor01", str(gltf),
                    "--floor01a", str(gltf),
                    "--floor01b", str(gltf),
                    "--placements", str(placements),
                    "--source-scale", "1",
                    "--cell-size", "0.5",
                    "--agent-radius", "0",
                    "--max-slope", "45",
                    "--max-step", "0.6",
                    "--output", str(output),
                    "--receipt", str(receipt),
                ]
                with mock.patch.object(sys, "argv", arguments):
                    self.assertEqual(0, navgrid.main())
                outputs.append(output.read_bytes())
                receipt_value = json.loads(receipt.read_text(encoding="utf-8"))
                self.assertEqual(
                    "LostArkValtanNavGridReceipt.v1",
                    receipt_value["schema"],
                )
                self.assertGreater(receipt_value["grid"]["walkableCount"], 0)
                self.assertEqual(
                    1, receipt_value["grid"]["connectedComponentCount"]
                )
                self.assertGreaterEqual(
                    receipt_value["grid"]["candidateComponentCount"], 1
                )

            self.assertEqual(outputs[0], outputs[1])
            header_format = struct.Struct("<8sIII8fII32s")
            unpacked = header_format.unpack_from(outputs[0])
            self.assertEqual(navgrid.MAGIC, unpacked[0])
            self.assertEqual(navgrid.VERSION, unpacked[1])
            width, height = unpacked[2], unpacked[3]
            walkable_bytes, height_count = unpacked[12], unpacked[13]
            self.assertEqual(width * height, height_count)
            self.assertEqual((height_count + 7) // 8, walkable_bytes)
            self.assertEqual(
                header_format.size + walkable_bytes + height_count * 4,
                len(outputs[0]),
            )


if __name__ == "__main__":
    unittest.main()
```

#### `Tools/LevelPlacementExtractor/tests/test_profiler_capture_schema.py` 전체 내용

```python
from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path


TOOLS_ROOT = Path(__file__).resolve().parents[1]
if str(TOOLS_ROOT) not in sys.path:
    sys.path.insert(0, str(TOOLS_ROOT))

import compare_profiler_captures as compare


def make_frame(number: int, scale: float = 1.0) -> dict:
    return {
        "frame": number,
        "cpuMs": {
            "frame": 10.0 * scale,
            "update": 3.0 * scale,
            "render": 5.0 * scale,
        },
        "cpuScopes": {
            "Client.Update": 3.0 * scale,
            "Client.Render": 5.0 * scale,
            "Renderer.NonBlend": 2.0 * scale,
        },
        "gpuMs": {
            "frame": 8.0 * scale,
            "valid": True,
            "latencyFrames": 4,
        },
        "draw": {
            "calls": int(100 * scale),
            "instancedCalls": 10,
            "instances": 1000,
            "indices": int(10000 * scale),
        },
        "pipeline": {
            "iaVertices": int(10000 * scale),
            "iaPrimitives": int(5000 * scale),
            "vsInvocations": int(10000 * scale),
            "psInvocations": int(20000 * scale),
        },
        "map": {
            "placements": 13097,
            "visibleInstances": 9000,
            "batchCount": 395,
            "fallbackObjects": 20,
        },
        "texture": {
            "requests": 100,
            "pathHits": 80,
            "contentHits": 10,
            "uniqueSrvs": 500,
            "estimatedGpuBytes": 1048576,
        },
        "navigation": {
            "queries": 1,
            "expandedNodes": int(50 * scale),
            "queryUs": int(100 * scale),
            "pathCells": 20,
        },
    }


def make_capture(scale: float = 1.0) -> dict:
    frames = [make_frame(index, scale) for index in range(1, 6)]
    return {
        "schema": compare.SCHEMA,
        "build": {"configuration": "Release", "gitCommit": "test"},
        "scene": {"level": "ASSET_TEST", "mapId": "LV_LUT_HEARTRB_ED"},
        "capture": {
            "warmupFrames": 60,
            "frameCount": len(frames),
            "requestedFrameCount": len(frames),
        },
        "dropped": {"cpuScopes": 0, "gpuFrames": 0},
        "frames": frames,
        "summary": {
            "frameP50Ms": 10.0 * scale,
            "frameP95Ms": 10.0 * scale,
            "frameP99Ms": 10.0 * scale,
            "gpuP95Ms": 8.0 * scale,
        },
    }


class ProfilerCaptureSchemaTests(unittest.TestCase):
    def test_load_and_compare_valid_capture(self) -> None:
        with tempfile.TemporaryDirectory() as directory_name:
            directory = Path(directory_name)
            before_path = directory / "before.json"
            after_path = directory / "after.json"
            before_path.write_text(
                json.dumps(make_capture(1.0)), encoding="utf-8"
            )
            after_path.write_text(
                json.dumps(make_capture(0.5)), encoding="utf-8"
            )
            before = compare.load_capture(before_path)
            after = compare.load_capture(after_path)
            report = compare.build_report(
                before_path, before, after_path, after
            )
            self.assertIn("CPU frame (ms)", report)
            self.assertIn("improved", report)
            self.assertIn("Draw-call p95", report)

    def test_rejects_frame_count_mismatch(self) -> None:
        with tempfile.TemporaryDirectory() as directory_name:
            path = Path(directory_name) / "bad.json"
            capture = make_capture()
            capture["capture"]["frameCount"] += 1
            path.write_text(json.dumps(capture), encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "frame count mismatch"):
                compare.load_capture(path)

    def test_rejects_non_monotonic_frame_numbers(self) -> None:
        with tempfile.TemporaryDirectory() as directory_name:
            path = Path(directory_name) / "bad.json"
            capture = make_capture()
            capture["frames"][1]["frame"] = capture["frames"][0]["frame"]
            path.write_text(json.dumps(capture), encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "not strictly increasing"):
                compare.load_capture(path)

    def test_gpu_stats_ignore_invalid_frames(self) -> None:
        capture = make_capture()
        capture["frames"][0]["gpuMs"]["valid"] = False
        capture["frames"][0]["gpuMs"]["frame"] = 9999.0
        values = compare.values_for(capture, ("gpuMs", "frame"))
        self.assertEqual(4, len(values))
        self.assertNotIn(9999.0, values)


if __name__ == "__main__":
    unittest.main()
```

### 5-15. 오류·로그 문구 계약

다음 메시지는 결과 보고서와 자동 검증에서 검색할 수 있게 고정한다.

```text
[Profiler] GPU queries unavailable; CPU capture remains enabled.
[Profiler] Capture saved: <path>
[MapBatch] committed placements=<n> batches=<n> fallback=<n>
[MapBatch] stage failed; previous runtime preserved: <reason>
[TextureCache] requests=<n> pathHits=<n> contentHits=<n> uniqueSrvs=<n>
[NavGrid] loaded path=<path> size=<w>x<h> walkable=<n> sha256=<hash>
[NavGrid] load failed; previous navigation preserved: <reason>
[PathFinder] result=<status> expanded=<n> queryUs=<n> points=<n>
```

---

## 6. 프로젝트 등록과 검증

### 6-1. Engine 프로젝트 등록 XML

`Engine/Default/Engine.vcxproj`의 기존 `ClInclude` ItemGroup에 추가한다.

```xml
<ClInclude Include="..\Public\Profiler.h" />
<ClInclude Include="..\Public\TextureResourceCache.h" />
<ClInclude Include="..\Public\NavGrid.h" />
<ClInclude Include="..\Public\PathFinder.h" />
```

기존 `ClCompile` ItemGroup에 추가한다.

```xml
<ClCompile Include="..\Private\Profiler.cpp" />
<ClCompile Include="..\Private\TextureResourceCache.cpp" />
<ClCompile Include="..\Private\NavGrid.cpp" />
<ClCompile Include="..\Private\PathFinder.cpp" />
```

`Engine/Default/Engine.vcxproj.filters`에는 기존 filter를 재배치하지 않고 다음 항목만 추가한다.

```xml
<ClInclude Include="..\Public\Profiler.h">
  <Filter>02.Utility\03.Renderer</Filter>
</ClInclude>
<ClInclude Include="..\Public\TextureResourceCache.h">
  <Filter>02.Utility\04.Component\Model\Material</Filter>
</ClInclude>
<ClInclude Include="..\Public\NavGrid.h">
  <Filter>02.Utility\04.Component\Navigation</Filter>
</ClInclude>
<ClInclude Include="..\Public\PathFinder.h">
  <Filter>02.Utility\04.Component\Navigation</Filter>
</ClInclude>
<ClCompile Include="..\Private\Profiler.cpp">
  <Filter>02.Utility\03.Renderer</Filter>
</ClCompile>
<ClCompile Include="..\Private\TextureResourceCache.cpp">
  <Filter>02.Utility\04.Component\Model\Material</Filter>
</ClCompile>
<ClCompile Include="..\Private\NavGrid.cpp">
  <Filter>02.Utility\04.Component\Navigation</Filter>
</ClCompile>
<ClCompile Include="..\Private\PathFinder.cpp">
  <Filter>02.Utility\04.Component\Navigation</Filter>
</ClCompile>
```

위 값은 현재 `Engine.vcxproj.filters`에 이미 존재하는 filter의 정확한 이름이다. 새 filter 선언이나 GUID는 추가하지 않는다.

### 6-2. Client 프로젝트 등록 XML

`Client/Default/Client.vcxproj`에 추가한다.

```xml
<ClInclude Include="..\Public\MapSceneRuntime.h" />
<ClInclude Include="..\Public\DeployPlacementDocument.h" />
<ClInclude Include="..\Public\DeployPropRuntime.h" />
<ClInclude Include="..\Public\ProfilerCaptureIO.h" />
<ClInclude Include="..\Public\ProfilerPanel.h" />
<ClInclude Include="..\Public\MapAssetRenderUtils.h" />
<ClInclude Include="..\Public\MapStaticBatchObject.h" />
<ClInclude Include="..\Public\ValtanNavigationRuntime.h" />
<ClCompile Include="..\Private\MapSceneRuntime.cpp" />
<ClCompile Include="..\Private\DeployPlacementDocument.cpp" />
<ClCompile Include="..\Private\DeployPropRuntime.cpp" />
<ClCompile Include="..\Private\ProfilerCaptureIO.cpp" />
<ClCompile Include="..\Private\ProfilerPanel.cpp" />
<ClCompile Include="..\Private\MapAssetRenderUtils.cpp" />
<ClCompile Include="..\Private\MapStaticBatchObject.cpp" />
<ClCompile Include="..\Private\ValtanNavigationRuntime.cpp" />
<None Include="..\Bin\ShaderFiles\Shader_MapAssetMaterial.hlsli" />
<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshMap.hlsl" />
<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshMapInstance.hlsl" />
```

`Client/Default/Client.vcxproj.filters`에 추가한다.

```xml
<ClInclude Include="..\Public\MapSceneRuntime.h">
  <Filter>02.GameObjects\02. World\Map</Filter>
</ClInclude>
<ClInclude Include="..\Public\DeployPlacementDocument.h">
  <Filter>02.GameObjects\02. World\Map</Filter>
</ClInclude>
<ClInclude Include="..\Public\DeployPropRuntime.h">
  <Filter>02.GameObjects\02. World\Map</Filter>
</ClInclude>
<ClInclude Include="..\Public\ProfilerCaptureIO.h">
  <Filter>03. Tools</Filter>
</ClInclude>
<ClInclude Include="..\Public\ProfilerPanel.h">
  <Filter>03. Tools</Filter>
</ClInclude>
<ClInclude Include="..\Public\MapAssetRenderUtils.h">
  <Filter>02.GameObjects\02. World\Map</Filter>
</ClInclude>
<ClInclude Include="..\Public\MapStaticBatchObject.h">
  <Filter>02.GameObjects\02. World\Map</Filter>
</ClInclude>
<ClInclude Include="..\Public\ValtanNavigationRuntime.h">
  <Filter>01.Levels\00. TestLevel1</Filter>
</ClInclude>
<ClCompile Include="..\Private\MapSceneRuntime.cpp">
  <Filter>02.GameObjects\02. World\Map</Filter>
</ClCompile>
<ClCompile Include="..\Private\DeployPlacementDocument.cpp">
  <Filter>02.GameObjects\02. World\Map</Filter>
</ClCompile>
<ClCompile Include="..\Private\DeployPropRuntime.cpp">
  <Filter>02.GameObjects\02. World\Map</Filter>
</ClCompile>
<ClCompile Include="..\Private\ProfilerCaptureIO.cpp">
  <Filter>03. Tools</Filter>
</ClCompile>
<ClCompile Include="..\Private\ProfilerPanel.cpp">
  <Filter>03. Tools</Filter>
</ClCompile>
<ClCompile Include="..\Private\MapAssetRenderUtils.cpp">
  <Filter>02.GameObjects\02. World\Map</Filter>
</ClCompile>
<ClCompile Include="..\Private\MapStaticBatchObject.cpp">
  <Filter>02.GameObjects\02. World\Map</Filter>
</ClCompile>
<ClCompile Include="..\Private\ValtanNavigationRuntime.cpp">
  <Filter>01.Levels\00. TestLevel1</Filter>
</ClCompile>
<None Include="..\Bin\ShaderFiles\Shader_MapAssetMaterial.hlsli">
  <Filter>97.ShaderFiles</Filter>
</None>
<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshMap.hlsl">
  <Filter>97.ShaderFiles</Filter>
</FxCompile>
<FxCompile Include="..\Bin\ShaderFiles\Shader_VtxMeshMapInstance.hlsl">
  <Filter>97.ShaderFiles</Filter>
</FxCompile>
```

위 값은 현재 `Client.vcxproj.filters`에 이미 존재하는 filter의 정확한 이름이다. 기존 항목은 이동하지 않는다.

### 6-3. Python 정적 검증

```powershell
python -m py_compile `
  Tools/LevelPlacementExtractor/audit_optimize_map_textures.py `
  Tools/LevelPlacementExtractor/build_valtan_navgrid.py `
  Tools/LevelPlacementExtractor/compare_profiler_captures.py

python -m unittest `
  Tools.LevelPlacementExtractor.tests.test_build_valtan_navgrid `
  Tools.LevelPlacementExtractor.tests.test_profiler_capture_schema
```

추가 검증:

```powershell
python Tools/LevelPlacementExtractor/build_valtan_navgrid.py `
  --floor01 C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Map/StaticMesh_Raw_20260729/BG/BG_RAD_VALTAN_A__542N9YJ2R1Y3NYH2RYFTMPE9/BG_RAD_VALTAN_A/StaticMesh3/bg_rad_valtan_floor01_sm.gltf `
  --floor01a C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Map/StaticMesh_Raw_20260729/BG/BG_RAD_VALTAN_A__542N9YJ2R1Y3NYH2RYFTMPE9/BG_RAD_VALTAN_A/StaticMesh3/bg_rad_valtan_floor01a_sm.gltf `
  --floor01b C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Map/StaticMesh_Raw_20260729/BG/BG_RAD_VALTAN_A__542N9YJ2R1Y3NYH2RYFTMPE9/BG_RAD_VALTAN_A/StaticMesh3/bg_rad_valtan_floor01b_sm.gltf `
  --placements Tools/LevelPlacementExtractor/heartrb_valtan_core_overlay.json `
  --cell-size 0.5 `
  --agent-radius 0.4 `
  --max-slope 45 `
  --max-step 0.6 `
  --output Client/Bin/DataFiles/Navigation/ValtanArena.navgrid `
  --receipt Client/Bin/DataFiles/Navigation/ValtanArena.navgrid.receipt.json
Get-FileHash Client/Bin/DataFiles/Navigation/ValtanArena.navgrid -Algorithm SHA256
python Tools/LevelPlacementExtractor/build_valtan_navgrid.py `
  --floor01 C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Map/StaticMesh_Raw_20260729/BG/BG_RAD_VALTAN_A__542N9YJ2R1Y3NYH2RYFTMPE9/BG_RAD_VALTAN_A/StaticMesh3/bg_rad_valtan_floor01_sm.gltf `
  --floor01a C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Map/StaticMesh_Raw_20260729/BG/BG_RAD_VALTAN_A__542N9YJ2R1Y3NYH2RYFTMPE9/BG_RAD_VALTAN_A/StaticMesh3/bg_rad_valtan_floor01a_sm.gltf `
  --floor01b C:/Users/user/Desktop/Resource_LostArk/01_Extracted/Map/StaticMesh_Raw_20260729/BG/BG_RAD_VALTAN_A__542N9YJ2R1Y3NYH2RYFTMPE9/BG_RAD_VALTAN_A/StaticMesh3/bg_rad_valtan_floor01b_sm.gltf `
  --placements Tools/LevelPlacementExtractor/heartrb_valtan_core_overlay.json `
  --cell-size 0.5 `
  --agent-radius 0.4 `
  --max-slope 45 `
  --max-step 0.6 `
  --output Client/Bin/DataFiles/Navigation/ValtanArena.navgrid `
  --receipt Client/Bin/DataFiles/Navigation/ValtanArena.navgrid.receipt.json
Get-FileHash Client/Bin/DataFiles/Navigation/ValtanArena.navgrid -Algorithm SHA256
```

두 해시는 같아야 한다.

### 6-4. 빌드 순서

```text
1. Engine x64 Debug
2. Engine x64 Release
3. UpdateLib.bat Debug
4. UpdateLib.bat Release
5. Client x64 Debug
6. Client x64 Release
```

Engine public header 변경 후 UpdateLib과 Client를 생략하지 않는다. 실행 중 `Client.exe`가 링크 출력을 점유하면 해당 프로세스만 정상 종료하고 다시 링크한다.

### 6-5. 프로파일러 검증

1. 로고에서 F2를 눌러 AssetTest 진입이 유지되는지 확인한다.
2. AssetTest에서 F4를 눌러 overlay와 panel이 함께 열리고 history가 reset되는지 확인한다.
3. CPU frame/update/render scope가 표시되는지 확인한다.
4. GPU sample은 4 frame 이상 지연되어 valid가 되는지 확인한다.
5. F4로 닫았을 때 timestamp JSON과 `profiler.json`이 생성되는지 확인한다.
6. JSON schema와 600 frame capture를 테스트 script로 검증한다.
7. GPU query 미지원 강제 경로에서 CPU capture가 계속되는지 확인한다.
8. Release CLI capture가 ImGui 없이 정상 저장되는지 확인한다.
9. profiler off/on p95 차이를 기록한다.

### 6-6. 정적 맵 최적화 검증

1. baseline에서 placement 13,097, batch 0, fallback 13,097을 기록한다.
2. culling-only 단계에서 화면과 visible count를 기록한다.
3. instancing 단계에서 raw key 395와 runtime batch count를 비교한다.
4. placement invariant script로 ID/Transform/visible diff 0을 확인한다.
5. 음수 scale이 많은 Floor01/A/B와 쇠사슬 주변에서 winding/cull 회귀를 확인한다.
6. alpha, CloudPlane, sky, crack emissive, UV flow가 fallback 또는 공통 shader에서 동일한지 캡처한다.
7. MapTool에서 placement 선택, Transform 수정, visible 토글, 저장, 재로드를 수행한다.
8. 잘못된 catalog/placement를 주입했을 때 기존 runtime이 보존되는지 확인한다.
9. 동일 camera의 draw call, CPU render, GPU frame p95를 before/after 비교한다.

### 6-7. 텍스처 검증

1. audit-only 실행이 source 파일을 변경하지 않는지 git/hash로 확인한다.
2. 95 duplicate group과 186 alias 기회를 결과에 기록한다.
3. full level load에서 path/content hit, unique SRV, estimated GPU bytes를 캡처한다.
4. DDS 변환 후보별 WMaterial semantic과 alpha 사용을 확인한다.
5. 적용 후보는 임시 폴더에서 먼저 조리하고 DDS load/mip/format을 검증한다.
6. diffuse/normal/emissive/alpha 화면을 전후 캡처한다.
7. 실패한 변환은 원본을 유지하고 receipt에 실패 원인을 남긴다.

### 6-8. NavGrid/A* 검증

#### geometry/grid

- exact source glTF와 placement hash가 receipt와 일치
- center `(156.279, 23.242, -121.977)` 부근이 walkable
- arena 바깥과 floor hole이 non-walkable
- walkable cell이 하나의 주 연결 성분
- agent radius 침식 후 외곽 이동이 mesh 밖으로 나가지 않음

#### A*

- center -> 동/서/남/북 경계 성공
- 경계를 가로지르는 대각선 corner cut 없음
- start/goal 비보행 시 제한 반경 nearest resolution
- 영역 밖 입력은 명시적 실패
- 같은 입력의 cell path와 smoothed path가 결정적
- 1,000개 고정 query의 p95 1ms 이하
- expansion limit 경로에서 hang 없음

#### runtime

- N으로 grid 표시 토글
- Ctrl+LMB start, Shift+LMB goal
- 결과 path, expanded nodes, query μs 표시
- navgrid 파일 손상 시 level crash 없이 기존 상태 보존

### 6-9. 최종 결과 보고서

구현 완료 후 같은 날짜 폴더에 다음 파일을 작성한다.

```text
.md/GB/07-30/2026-07-30_LOSTARK_VALTAN_PROFILER_RENDER_OPTIMIZATION_NAVGRID_RESULT.md
```

필수 내용:

1. 적용 commit과 dirty worktree 분리 내역
2. Winters/SR/LoL에서 채택·기각한 요소와 이유
3. before/after JSON 경로와 동일 실험 조건
4. CPU/GPU/draw/instance/texture/nav p50/p95/p99 표
5. 13,097 placement invariant 결과
6. 최종 batch/fallback 분포와 fallback reason
7. texture duplicate hit와 실제 GPU memory 추정 변화
8. NavGrid 해시/크기/walkable 수/A* 성능
9. 전체 Debug/Release 빌드와 runtime 검증
10. 남은 후속 작업: Deploy 상태 기반 dynamic obstacle overlay

---

### 6-10. 최종 의사결정

- 모든 개별 mesh를 한 개의 거대 mesh로 합치지 않는다.
- 발탄 `DEFERRED` static은 원본 mesh 공유 hardware instancing으로 묶는다.
- 현재 데이터 기준 핵심 batch key는 `assetId + mirrorParity`이며 raw key는 395개다.
- `TRANSLUCENT`/`BACKGROUND`/Deploy/destructible/skeletal/effect는 각 전용 또는 fallback draw 경로를 유지한다.
- PNG→DDS만을 주 최적화로 보지 않고 texture content cache와 의미 기반 변환을 함께 측정한다.
- 프로파일러를 먼저 만들고 각 최적화 단계를 독립 캡처해 실제 효과가 있는 변경만 유지한다.
- 내비게이션은 기존 `CNavigation` façade를 확장하고 발탄 Floor01/A/B actual geometry에서 NavGrid를 베이크한다.
- 첫 NavGrid 단계는 정적 보행면과 A*까지만 닫고, 파괴 상태 기반 동적 장애물은 Deploy runtime 완료 후 별도 단계로 연결한다.
