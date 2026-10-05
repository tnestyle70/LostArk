# WintersEngine / Unreal 비교 코드 가이드

[전체 코드 지도](2026-10-03_VISUAL_STUDIO_CODE_ATLAS.md) · [최종 VS 필터 트리](2026-10-03_VISUAL_STUDIO_FILTER_TREE.md)

조사일: 2026-10-03. Winters 소스는 `C:/Users/tnest/Desktop/WintersEngine`의 현재 디스크 저장본을 읽었다. Winters 저장소의 파일 수정·빌드·실행·Git 명령은 수행하지 않았다. 초기 조사 뒤 Unreal 소스 clone이 완료되어 현재 `C:/Users/tnest/Desktop/UnrealEngine`의 실제 코드도 대조했다. [Build.version](C:/Users/tnest/Desktop/UnrealEngine/Engine/Build/Build.version:1)은 **5.8.3**, 조사한 checkout commit은 `396c9f059903aed5fec78ecd3d437a40c6415368`이다. 아래 GC·객체 구성·Camera·World·AI 비교는 이 로컬 소스 근거이며, 기존 공식 문서 비교표는 해당 문서 버전의 보조 자료로 보존한다. 과거 Winters 문서의 Unreal 5.7.4 및 `C:/Users/user/Desktop/UnrealEngine/UnrealEngine`는 현재 실측 버전/경로가 아니다.

이 문서는 전수 함수 명세나 구현 완료 선언이 아니라 기술서 첫 조사 지도다. 클래스가 존재한다는 사실, 제품에서 호출된다는 사실, 빌드 성공, 실제 표시 성공을 구분한다.

## 1. Visual Studio에서 보이는 구조를 출발점으로 읽기

세 층을 분리한다. `.sln`의 solution folder는 프로젝트/문서를 묶는 화면 구조, `.vcxproj.filters`는 한 C++ 프로젝트 내부 표시 구조, `.vcxproj`는 해당 MSBuild 프로젝트의 실제 컴파일 항목이다. 실제 물리 파일 경로와 호출 관계를 같이 적어야 필터 이름을 런타임 모듈로 오해하지 않는다.

### Winters Engine 프로젝트의 실제 필터

[Engine.vcxproj.filters](C:/Users/tnest/Desktop/WintersEngine/Engine/Include/Engine.vcxproj.filters:8)에서 확인한 최상위 순서:

| VS 필터 | 실파일을 읽을 출발점 / 역할 |
|---|---|
| `00. Core` | Timer, Transform, Platform, Paths, Input, PCH, Profiler |
| `01. Runtime` | EngineApp, WintersEngine, GameInstance, Scene |
| `02. RHI` | Interface, DX11, DX12, Texture, Geometry |
| `03. Renderer` | Camera, Cube, Model, Plane, FX, Material, FogOfWar |
| `04. Resource` | Mesh, Texture, Model, Bone, Skeleton, Animation, Animator, ResourceCache, AssetFormat |
| `05. ECS` | Core, System, Components, Systems |
| `06. Navigation` | NavGrid, Pathfinder, MapSurface |
| `07. UI` | AtlasManifest, HUD, Font, Lua |
| `08. Sound` | 음향 계층 |
| `09. AI` | BT, MCTS, RL 분류 |
| `10. Editor` | ImGui, ProfilerOverlay |
| `11. Scripting` | Lua VM |
| `12. Cinematic` | CSequenceAsset, CSequencePlayer, ISeqBindingResolver, ISeqEventSink |
| `13. Physics3D` | Physics3D 분류 |
| `14. World` | World 분류 |

이는 존재하는 **표시 분류**다. AI/MCTS/RL 등의 필터가 있다는 이유만으로 제품 완성·성능·운영 검증까지 주장하지 않는다.

| 실제 VS 표시 | 물리 파일 / 심볼 |
|---|---|
| `01. Runtime/00. EngineApp` | [CEngineApp.h](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/Framework/CEngineApp.h:15), [필터 등록](C:/Users/tnest/Desktop/WintersEngine/Engine/Include/Engine.vcxproj.filters:424) |
| `00. Core/06. Profiler/00. CPU` | [CPUProfiler.h](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/Core/Profiler/CPUProfiler.h:9), [필터 등록](C:/Users/tnest/Desktop/WintersEngine/Engine/Include/Engine.vcxproj.filters:637) |
| `02. RHI/00. Interface` | [IRHIDevice.h](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/RHI/IRHIDevice.h:12), [필터 등록](C:/Users/tnest/Desktop/WintersEngine/Engine/Include/Engine.vcxproj.filters:808) |
| `03. Renderer/04. FX/08. RHI` | [RenderWorldSnapshot.h](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/Renderer/RenderWorldSnapshot.h:59), [RHISceneRenderer.h](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/Renderer/RHISceneRenderer.h:10), [필터 등록](C:/Users/tnest/Desktop/WintersEngine/Engine/Include/Engine.vcxproj.filters:688) |
| `12. Cinematic` | [CSequenceAsset.h](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/Cinematic/CSequenceAsset.h:98), [CSequencePlayer.h](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/Cinematic/CSequencePlayer.h:7), [필터 등록](C:/Users/tnest/Desktop/WintersEngine/Engine/Include/Engine.vcxproj.filters:871) |

`RenderWorldSnapshot`과 `RHISceneRenderer`는 이름이나 물리 폴더를 보고 새로 만든 가상 분류가 아니라 현재 필터상 FX/RHI 아래에 있다. 기술서에는 이 차이를 그대로 표시하고 필터를 임의 재배치하지 않는다.

Client의 주요 필터는 `00. MainApp`, `01. Scene`, `02. GameObject`, `03. GamePlay`, `04. Manager`, `05. UI`, `06. Network`, `07. Data`, `08. Dev`, `09. GameModule`, `10. Shell`, `11. GameMode`, `12. Replay`, `99. Defines`다. UI 내부에 ChampionTuner, WfxEffectTool, ModelAnim 등이 있고 Network/Client 아래에 EventApplier, SnapshotApplier가 구분돼 있다. 근거: [Client.vcxproj.filters](C:/Users/tnest/Desktop/WintersEngine/Client/Include/Client.vcxproj.filters:1).

### Solution과 CMake는 무엇을 빌드하는가

[Winters.sln](C:/Users/tnest/Desktop/WintersEngine/Winters.sln:6)의 실제 C++ 프로젝트는 Engine, Server, Client, GameSim, WintersAssetConverter, SimLab이고 EldenRingClient도 [820행](C:/Users/tnest/Desktop/WintersEngine/Winters.sln:820)에 등록된다. `00. Docs`~`08. Tools` 등은 solution folder이며 Services Go 소스도 솔루션 탐색기에 보인다. 솔루션에 보이는 Go 파일이 MSVC에서 C++로 컴파일되는 것은 아니다.

[root CMakeLists.txt](C:/Users/tnest/Desktop/WintersEngine/CMakeLists.txt:24)는 Engine CMake 모듈, EldenRingClient, EldenRingEditor, LoLEditor를 추가한다. [WintersEngine.cmake](C:/Users/tnest/Desktop/WintersEngine/cmake/WintersEngine.cmake:61)는 실제 `WintersEngine` shared library를 만들고, [LoLEditor CMake](C:/Users/tnest/Desktop/WintersEngine/LoLEditor/CMakeLists.txt:17)는 실제 별도 editor executable을 만든다.

반면 [WintersWorkspaceMap.cmake](C:/Users/tnest/Desktop/WintersEngine/cmake/WintersWorkspaceMap.cmake:127)의 `WintersWorkspaceMap`은 source browsing용 custom target이다. `source_group(TREE ...)`가 Client/Server/Engine/Shared/Shaders/Data/Services/Tools를 보여준다. 이 화면 지도가 legacy Client·Server·GameSim의 `.vcxproj` 컴파일 목록을 자동 대체하지 않는다.

## 2. 폴더별 실제 소유권

| 물리 영역 | 현재 근거와 설명 |
|---|---|
| Engine | 공용 frame loop, RHI, resource, renderer, ECS 기반, cinematic, profiler. [CEngineApp::Initialize](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Framework/CEngineApp.cpp:519) |
| Client | LoL scene/input/network presentation/tool UI. EventApplier와 SnapshotApplier가 각자 실제 Client 프로젝트에 등록됨. 위 VS 필터 지도 참조 |
| EldenRingClient | 같은 엔진을 사용하는 별도 제품 target. [CMake](C:/Users/tnest/Desktop/WintersEngine/EldenRingClient/CMakeLists.txt:20) |
| LoLEditor / EldenRingEditor | 별도 editor executable. [LoLEditor](C:/Users/tnest/Desktop/WintersEngine/LoLEditor/CMakeLists.txt:17), [EldenRingEditor](C:/Users/tnest/Desktop/WintersEngine/EldenRingEditor/CMakeLists.txt:19) |
| Shared/GameSim | component, command, definition, system, deterministic simulation 계약. [GameplayDefinitionPack](C:/Users/tnest/Desktop/WintersEngine/Shared/GameSim/Definitions/GameplayDefinitionPack.h:13) |
| Server | transport ingress를 room tick으로 넘겨 command 실행과 simulation, event/snapshot 송신. [CGameRoom::Tick](C:/Users/tnest/Desktop/WintersEngine/Server/Private/Game/GameRoomTick.cpp:102) |
| Services | Go backend. cmd entrypoint, internal handler/service/repository, migrations, pkg. [Replay service](C:/Users/tnest/Desktop/WintersEngine/Services/internal/replay/service.go:34) |
| Tools | asset conversion, JSON/code generation, SimLab, validation. [Build-LoLDefinitionPack.py](C:/Users/tnest/Desktop/WintersEngine/Tools/LoLData/Build-LoLDefinitionPack.py:3547) |
| Data | Account, GameModes, Gameplay, LoL. 저작 JSON과 server/client 공개 범위별 입력 데이터 |
| Shaders | GPU 프로그램 원본. CMake editor 배포 단계에서 shader 폴더 복사 |
| EngineSDK | 소비자에 배포하는 엔진 헤더/라이브러리 영역. 실코드 정본과 구분 필요 |

`Shared/GameSim은 Engine 구현에서 완전히 분리됐다`고 단정하면 안 된다. [Shared/GameSim/Core/World/World.h](C:/Users/tnest/Desktop/WintersEngine/Shared/GameSim/Core/World/World.h:4)는 현재 `ECS/World.h`를 include하고 `SharedSim::World = ::CWorld`인 **임시 adapter**다. 설계 방향과 현 구현의 중간 상태를 설명할 수 있는 좋은 사례다.

## 3. 프로그램 시작에서 한 프레임까지

공통 실행 소유자는 `CEngineApp`이다. [헤더](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/Framework/CEngineApp.h:56)에서 game app 포인터와 `unique_ptr<IRHIDevice>` 등의 소유 구조를 확인할 수 있다.

```text
CEngineApp::Initialize(gameApp, EngineConfig)
  → CEngineApp::Run
    → CGameInstance::Tick_Engine
    → CEngineApp::Update(deltaTime)
      → IWintersApp::OnUpdate 또는 SceneManager Update/LateUpdate
    → CEngineApp::Render
      → IRHIDevice::BeginFrame
      → 게임/scene render 및 ImGui 경로
      → IRHIDevice::EndFrame
```

실제 함수 출발점: [Run](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Framework/CEngineApp.cpp:739), [Update](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Framework/CEngineApp.cpp:873), [Render](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Framework/CEngineApp.cpp:890). 조건에 따른 앱/scene 경로가 있으므로 위 도식은 책임 흐름이며 모든 분기가 매 프레임 동시에 실행된다는 뜻은 아니다.

서버의 시간은 별도다. [CGameRoom::Tick](C:/Users/tnest/Desktop/WintersEngine/Server/Private/Game/GameRoomTick.cpp:102)은 ingress를 drain하고 `TickContext`에 tick index, fixed delta, RNG, entity map, gameplay definition pack을 넣는다. [147행](C:/Users/tnest/Desktop/WintersEngine/Server/Private/Game/GameRoomTick.cpp:147)부터 command drain → bot command 생산 → command 실행 → simulation systems → game end → event broadcast → snapshot broadcast → keyframe capture 순서다. Client 연출 프레임과 Server 판정 tick을 같은 시간으로 설명하지 않는다.

## 4. Sequencer: 데이터와 재생기와 UI를 분리해서 이해하기

### 구조체

[CSequenceAsset.h](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/Cinematic/CSequenceAsset.h:10)의 `eSeqTrackType`은 Camera, Anim, Fx, Audio, Event, Visibility, TimeDilation이다. `SeqTrack`은 type, name, binding 문자열과 각 타입의 key vector를 가진다. [SeqTrack](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/Cinematic/CSequenceAsset.h:83), [asset 공통 필드](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/Cinematic/CSequenceAsset.h:109).

Camera key에는 초 단위 시각, 위치, Euler 회전, FOV, 보간 방식, cut이 있다. FX key에는 시각, WFX 경로, anchor, one-shot이 있고 Event key에는 event 이름과 payload 문자열이 있다. 포인터가 아니라 문자열 binding을 저장하고 재생 순간 [ISeqBindingResolver](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/Cinematic/ISeqBindingResolver.h:1)가 실제 camera/model/FX 소비자를 연결한다.

### 호출 흐름과 알고리즘

`Play`는 asset/resolver/event sink의 포인터를 보관하고 시간/발화 이력을 초기화한다. 재생기가 이 raw pointer 대상의 수명을 소유하지 않으므로 호출자가 더 오래 유지해야 한다. [Play](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Cinematic/CSequencePlayer.cpp:51), [포인터 멤버](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/Cinematic/CSequencePlayer.h:59).

`Tick`은 이전 시간과 현재 시간의 구간을 평가한다. loop 경계를 넘으면 끝 구간을 평가한 뒤 새 cycle을 시작하여 0초 키도 처리한다. Camera/Visibility/TimeDilation은 현재 시각의 연속 상태로 평가하고 Anim/FX/Audio/Event는 시간을 통과할 때만 발화한다. [Tick](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Cinematic/CSequencePlayer.cpp:75), [Evaluate switch](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Cinematic/CSequencePlayer.cpp:162).

발화 조건은 `previous < key <= current`이며 `(type, track index, key index, cycle)` 기록으로 같은 cycle의 중복 발화를 막는다. [ShouldFireDiscreteKey](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Cinematic/CSequencePlayer.cpp:388), [DidCrossKey](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Cinematic/CSequencePlayer.cpp:461). 이 index는 재생 중 중복 제어 수단이며 영구 에셋 identity라고 부르면 안 된다.

`Seek`는 시간을 clamp하고 `Evaluate(false)`를 호출한다. 따라서 scrub으로 화면 연속 상태는 바꾸지만 FX/소리/event를 지나간 만큼 전부 재생하지 않는다. [Seek](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Cinematic/CSequencePlayer.cpp:140).

`Cubic`은 여기서 사용자 tangent를 가진 일반 spline이 아니다. alpha에 `alpha²(3−2alpha)`를 적용하는 smoothstep이다. [SampleScalar](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Cinematic/CSequencePlayer.cpp:426). 영상에서 커브 편집기 수준의 cubic tangent authoring을 구현했다고 소개하면 과장이다.

### 저장과 검증의 실제 범위

[LoadFromJson](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Cinematic/CSequenceAsset.cpp:573)은 임시 parsed asset을 만들고 key 정렬 후 outAsset에 이동한다. `Validate`는 별도 함수다. 즉 JSON parsing 성공과 sequence 의미 검증 성공을 합쳐 말하지 않는다. [commit 위치](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Cinematic/CSequenceAsset.cpp:706), [Validate](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Cinematic/CSequenceAsset.cpp:851).

[SaveToJson](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Cinematic/CSequenceAsset.cpp:711)은 현재 `ofstream`의 trunc 모드로 직접 파일을 쓰며 format=wseq, version=1을 출력한다. 이 경로를 temp/backup/rename을 갖춘 원자 저장이라고 설명해서는 안 된다. 장래 저장 내구성 개선 후보이며 이번 조사에서는 수정하지 않았다.

[EldenRingEditor의 DrawSequencerPanel](C:/Users/tnest/Desktop/WintersEngine/EldenRingEditor/Private/EldenRingEditorScene.cpp:815)은 실제로 WSEQ JSON 경로, `Load / Validate` 버튼, track/key/duration, 오류 목록을 보여준다. 이 UI를 다중 track drag 편집기나 완전한 cinematic editor로 소개하면 안 된다. Engine 재생 API의 존재와 editor UI의 연결 정도를 별도 표로 유지한다.

## 5. FX graph, 리소스, 데이터 저장

### FX

[FxEmitterDesc](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/FX/FxAsset.h:82)에는 renderer type, maxParticles/spawnRate, node 목록, GPU handle, material/blend/depth, texture/model 경로, anchor/lifecycle, velocity/scale/rotation/color, lifetime/atlas/UV 값 등이 모인다. 실행 중 GPU handle과 저장 경로 문자열은 목적이 다르다.

[FxGraph](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/FX/Graph/FxGraph.h:67)는 node/edge/user parameter/emitter graph를 표현하고 JSON read/write API를 제공한다. [CFxGraphCompiler::Compile](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/FX/Exec/FxExecPlan.cpp:332)은 validator 결과를 먼저 얻은 뒤 stage별 실행 step을 생성한다. 하지만 이번 조사에서 Engine/Client 제품 루프의 호출까지 입증한 것은 아니다. graph/compiler 클래스의 존재를 Niagara 전체 기능 대응 또는 현재 모든 WFX의 실행 경로라고 일반화하지 않는다.

### 메모리 resource cache와 영구 데이터

[CResourceCache](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/Resource/ResourceCache.h:10)는 texture를 unique_ptr map, model을 shared_ptr map에 보관한다. [LoadTexture](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Resource/ResourceCache.cpp:14), [LoadModel](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Resource/ResourceCache.cpp:47)은 key 조회로 중복 로드를 피한다. 이것은 메모리 재사용이며 원본 파일 저장이나 cook이 아니다.

[CModel::LoadModel](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Resource/Model.cpp:971)은 요청 모델에서 WMesh 경로를 resolve하고 WMesh를 기반으로 mesh resource를 만든다. 캐시→파일 load→CPU model→GPU buffer의 계층을 구분하여 설명한다.

게임플레이 JSON은 [Build-LoLDefinitionPack.py](C:/Users/tnest/Desktop/WintersEngine/Tools/LoLData/Build-LoLDefinitionPack.py:3547)의 schema/domain validation과 code generation을 통과한다. source 목록은 [3346행](C:/Users/tnest/Desktop/WintersEngine/Tools/LoLData/Build-LoLDefinitionPack.py:3346), schema validator는 [817행](C:/Users/tnest/Desktop/WintersEngine/Tools/LoLData/Build-LoLDefinitionPack.py:817), manifest 생성은 [2239행](C:/Users/tnest/Desktop/WintersEngine/Tools/LoLData/Build-LoLDefinitionPack.py:2239)에 있다.

[GameplayDefinitionPack](C:/Users/tnest/Desktop/WintersEngine/Shared/GameSim/Definitions/GameplayDefinitionPack.h:13)은 manifest, 각 definition 배열의 const 포인터/count, FindChampion/FindSkill 등의 조회 API로 구성된다. Server는 이 pack을 TickContext에 주입한다. `F4 현재 저장 JSON → generated output`의 값 권위는 [데이터 구조 문서](C:/Users/tnest/Desktop/WintersEngine/.md/architecture/WINTERS_DATA_ARCHITECTURE.md:1)의 canonical authoring 규칙을 따른다. 그 문서의 7월 수치나 미완료 목록을 10월 현재 실측으로 재인용하지 않는다.

계정/경기 기록의 장기 보존은 또 다른 계층이다. 예를 들어 Replay의 [Service::CreateUpload](C:/Users/tnest/Desktop/WintersEngine/Services/internal/replay/service.go:45), [Repository::ReserveUpload](C:/Users/tnest/Desktop/WintersEngine/Services/internal/replay/repository.go:48), [S3Storage::CreateMultipartUpload](C:/Users/tnest/Desktop/WintersEngine/Services/internal/replay/s3_storage.go:79)가 업무 절차, DB metadata, object storage를 나눈다. [GetAuthorized](C:/Users/tnest/Desktop/WintersEngine/Services/internal/replay/repository.go:143)는 사용자별 조회 경계를 가진다. 따라서 "Save"라는 한 단어로 JSON authoring, GPU cache, binary cook, DB 영속 저장을 묶지 않는다.

## 6. 렌더러와 profiler

### RHI와 scene renderer

[IRHIDevice](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/RHI/IRHIDevice.h:12)는 frame, resource, command list, pipeline, bind group API를 노출한다. 일부 함수의 기본 구현은 빈 handle/null 반환이므로 interface의 존재만으로 모든 backend 지원 완료를 뜻하지 않는다.

[RenderWorldSnapshot](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/Renderer/RenderWorldSnapshot.h:59)은 view와 mesh/FX/debug 배열이다. mesh item은 world matrix, mesh slice, texture/sampler handle, tint, depth-write 값을 담는다. [CRHISceneRenderer::Render](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Renderer/RHISceneRenderer.cpp:399)는 command list를 받고 frame constants를 갱신한 뒤 mesh를 순회하며 layout/depth에 맞는 pipeline과 per-object constants/bind group을 설정한다. 이는 관측된 submit loop다. UE RDG처럼 pass dependency에서 수명·barrier·aliasing·culling을 자동 컴파일하는 시스템이라고 부르면 안 된다.

LoL 기본 DX11과 RHI 이관은 공존한다. [CEngineApp 공개 getter](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/Framework/CEngineApp.h:30)에 DX11Shader/DX11Pipeline이 남아 있다는 점도 숨기지 않는다. 제품 render path별 도달성은 다음 심층 조사에서 별도 추적한다.

### CPU 측정

[CProfileScope](C:/Users/tnest/Desktop/WintersEngine/Engine/Include/ProfilerAPI.h:37)는 생성/소멸 시 push/pop을 호출하는 RAII 계측이다. [CCPUProfiler::PushScope](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Core/Profiler/CPUProfiler.cpp:333)와 [PopScope](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Core/Profiler/CPUProfiler.cpp:345)는 QPC 시각, depth, thread ID를 기록하고 mutex로 event/stat 병합을 보호한다. stack은 [fiber-aware 저장](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Core/Profiler/CPUProfiler.cpp:124)을 사용한다.

[ProfilerTypes](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/Core/Profiler/ProfilerTypes.h:6)에 event/stat/counter/history 상한이 있다. raw tree event는 실제 push 경로에서 프레임당 1024개까지, scope stats는 256개까지이며 초과는 dropped 계수로 남는다. history의 raw events는 최근 64개 record만 보존하고 앞부분은 summary만 남긴다. [raw eviction](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Core/Profiler/CPUProfiler.cpp:297). 전체 캡처 길이 동안 모든 raw scope를 보존한다고 설명하면 안 된다.

### GPU와 저장

DX11은 [disjoint/timestamp query 생성](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/RHI/DX11/CDX11Device.cpp:998)과 [GetData](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/RHI/DX11/CDX11Device.cpp:1027)로 GPU 시간을 구한다. frame record에는 CPU frame 번호와 `gpuSourceRhiFrame`을 따로 기록한다. GPU 완료 시점과 CPU 화면 frame이 다를 수 있기 때문이다. [frame record](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/Core/Profiler/ProfilerTypes.h:44).

Overlay는 [비동기 save future](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Manager/Profiler/ProfilerOverlay.cpp:207)를 소유하고 [Save_DisplayFrameToJson](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Manager/Profiler/ProfilerOverlay.cpp:475), [Save_TimelineToJson](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Manager/Profiler/ProfilerOverlay.cpp:617)을 통해 raw event와 thread ID 등을 저장한다. 여기서 확인한 것은 측정 도구 구조다. FPS 향상 또는 최적화 수치는 이 조사에서 측정하지 않았다. 성능 포트폴리오 수치는 Release+profiling, 동일 조건, 여러 번의 재현 측정으로 별도 검증해야 한다.

## 7. Unreal과 비교할 때의 정확한 대응

다음 표는 로컬 UE 소스 확보 전 조사한 공식 문서 지도다. 버전 선택이 열리는 문서는 5.7로 고정했고 API/UObject/Cook/Insights 일부 페이지는 당시 공식 사이트의 5.8 제목으로 표시했다. 이 문서 표의 버전과 새로 확인한 **로컬 UE 5.8.3 구현**을 구분한다. 실제 코드 근거는 이어지는 7.1~7.8에 기록한다.

| 주제 | Winters에서 확인한 구현 | Unreal 공식 구조 / 비교에서 배울 점 |
|---|---|---|
| 빌드 단위 | legacy vcxproj와 CMake target이 공존하며 filters는 화면 구성 | UE5.7은 `Build.cs`/`Target.cs`가 build graph를 만들고 IDE solution은 생성된 편집 뷰다. Runtime/Editor module 타입과 dependency 공개 범위를 비교한다. [Modules](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-modules?application_version=5.7) |
| 객체 수명 | unique_ptr/shared_ptr, entity handle, component store, raw binding pointer | UE5.8 UObject는 reflection metadata, CDO, object creation, GC를 함께 제공한다. C++ 객체/저장 타입/반사 타입이 같은 문제가 아니라는 점을 비교한다. [Objects](https://dev.epicgames.com/documentation/en-us/unreal-engine/objects-in-unreal-engine) |
| 시퀀스 asset | `CSequenceAsset`, typed key vector, string binding, JSON | UE5.8 `UMovieSceneSequence`는 UObject 계열이며 compiled data, GetMovieScene, object binding/locator, Serialize API를 가진다. [UMovieSceneSequence](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/MovieScene/UMovieSceneSequence) |
| 시퀀스 재생 | `CSequencePlayer`의 double 초 단위 clock, 구간 통과 event | UE5.8 `UMovieSceneSequencePlayer`에는 frame time/rate, playback settings, time controller, binding, reverse/restore API가 있다. 같은 이름의 Play라도 평가/복구/바인딩 계약을 비교해야 한다. [Player](https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/MovieScene/UMovieSceneSequencePlayer) |
| asset 목록 | 파일 경로와 resource cache, 영역별 catalog | UE5.7 Asset Registry는 로드 전 metadata를 비동기로 수집하여 FAssetData 목록과 Content Browser에 제공한다. 메모리에 asset을 올리는 cache와 검색 metadata registry의 역할 차이를 설명한다. [Asset Registry](https://dev.epicgames.com/documentation/en-us/unreal-engine/asset-registry-in-unreal-engine?application_version=5.7) |
| cook | custom WMesh 등과 JSON→definition generated output | UE5.8 Cook commandlet은 대상 플랫폼용 content 변환을 수행한다. JSON 값 codegen과 platform asset cook을 동일하다고 하지 않고 입력·의존성·산출물·검증 단계를 대응시킨다. [Content Cooking](https://dev.epicgames.com/documentation/en-us/unreal-engine/cooking-content-in-unreal-engine) |
| 렌더링 | RHI resource handle와 명령 API, snapshot mesh submit loop | UE5.7 RDG는 CreateTexture/CreateBuffer, AddPass, Execute 및 parameter metadata로 dependency를 얻고 pass/resource 수명과 barrier 등을 처리한다. RHI abstraction과 render graph scheduling을 별도 계층으로 비교한다. [RDG](https://dev.epicgames.com/documentation/en-us/unreal-engine/render-dependency-graph-in-unreal-engine?application_version=5.7) |
| 측정 | bounded QPC raw event/stat/counter, GPU query, JSON frame/timeline | UE5.8 TraceLog/TraceAnalysis, Trace Server, Insights UI는 capture/store/analysis를 분리한다. CPU/GPU뿐 아니라 Memory/Net/Load/RDG channel이 존재한다. [Trace](https://dev.epicgames.com/documentation/en-us/unreal-engine/trace-in-unreal-engine-5), [Insights](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-insights-in-unreal-engine) |

이번 추가 조사에서는 CoreUObject의 객체 수명, Engine의 Actor/Component/World/Camera, CinematicCamera, AIModule과 BehaviorTreeEditor의 호출을 확인했다. MovieScene/Sequencer, RenderCore/RHI, Trace/Insights의 상세 비교는 각 담당 코드 가이드에서 다룬다. 다음 비교의 완료 증거는 파일·선언·호출 관계이며, Unreal 빌드·에디터 실행·PIE·화면·성능 검증은 수행하지 않았다.

### 7.1. UObject와 우리 객체: 수명을 누가 끝내는가

UE [GarbageCollection.cpp의 CollectGarbage](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/CoreUObject/Private/UObject/GarbageCollection.cpp:6366)는 GC lock을 얻고 내부 수집을 실행한다. [MarkObjectsAsUnreachable](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/CoreUObject/Private/UObject/GarbageCollection.cpp:4495)는 reachability 상태를 준비하고 root·cluster에서 도달 가능한 객체를 추적할 출발점을 마련한다. [PerformReachabilityAnalysis](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/CoreUObject/Private/UObject/GarbageCollection.cpp:4643)와 [IncrementalPurgeGarbage](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/CoreUObject/Private/UObject/GarbageCollection.cpp:4768)는 도달성 판정과 정리를 분리한다. [UObject::ConditionalBeginDestroy](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/CoreUObject/Private/UObject/Obj.cpp:1299)는 destruction flag와 BeginDestroy 호출을 관리한다.

LostArk의 현재 객체 수명은 manual AddRef/Release가 아니다. [CGameObject::m_Components](C:/Users/tnest/Desktop/LostArk/Engine/Public/GameObject.h:55)는 component tag를 key로 하는 `map<wstring, shared_ptr<CComponent>>`, [CLayer::m_GameObjects](C:/Users/tnest/Desktop/LostArk/Engine/Public/Layer.h:30)는 `list<shared_ptr<CGameObject>>`다. [Remove_GameObject](C:/Users/tnest/Desktop/LostArk/Engine/Private/Layer.cpp:87)는 phase 등록과 Layer의 보유 참조를 제거한다. 다른 shared_ptr 보유자가 남았는지와 객체 소멸은 별개의 문제다.

Winters [EntityHandle](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/ECS/Entity.h:17)은 ID와 generation을 64-bit 값으로 묶어 재사용된 slot을 이전 객체로 오인하지 않도록 한다. [CWorld::DestroyEntity](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/ECS/World.cpp:24)는 transform 부모·자식 관계를 정리하고 모든 component store에서 해당 entity를 제거한 뒤 entity manager를 갱신한다. handle은 생존 여부를 검사하는 identity이며 객체를 살려 두는 shared_ptr가 아니다.

공통 문제는 객체 수명과 유효한 참조 유지다. UE의 참조 추적 GC, LostArk의 스마트 포인터 소유권, Winters의 명시적 ECS 제거는 해결 방식이 다르다. UObject GC가 모든 C++ 메모리를 수집한다거나 `TObjectPtr`가 `shared_ptr`와 같은 참조 계수라고 설명하지 않는다. UE의 [FWeakObjectPtr](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/CoreUObject/Public/UObject/WeakObjectPtr.h:290)도 유효성 검사 계약을 갖지만 Winters EntityHandle과 대상 저장소·수명 규칙까지 같지는 않다.

### 7.2. Actor/Component와 Prototype/Clone, ECS store

UE [AActor::RootComponent](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Classes/GameFramework/Actor.h:1022)는 `UPROPERTY`로 선언된 `TObjectPtr<USceneComponent>`이며 actor의 공간 기준을 정한다. [OwnedComponents](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Classes/GameFramework/Actor.h:4331)는 actor가 소유한 component 집합이다. [UActorComponent::RegisterComponentWithWorld](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/Components/ActorComponent.cpp:1967)는 world·owner·중복 등록 상태를 검사하고 초기화 및 등록을 연결한다. 객체가 존재하는 것과 world에서 갱신·렌더링되는 component로 등록된 것은 다르다.

UE의 생성 진입점 [UWorld::SpawnActor](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/LevelActor.cpp:456)와 제거 진입점 [DestroyActor](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/LevelActor.cpp:839)를 함께 읽는다. 제거 시 component unregister와 garbage 표시, tick function 등록 해제가 이어진다([1051행](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/LevelActor.cpp:1051)). DestroyActor 호출을 즉시 C++ delete와 동일하게 설명하지 않는다.

LostArk [CGameObject::Add_Component](C:/Users/tnest/Desktop/LostArk/Engine/Public/GameObject.h:62)는 `Clone_Prototype`으로 component를 얻고 tag map과 typed shared_ptr 출력에 연결한다. [Object_Manager.cpp](C:/Users/tnest/Desktop/LostArk/Engine/Private/Object_Manager.cpp:70)는 game object prototype을 clone한 뒤 Layer에 넣는다. Winters는 [ComponentStore](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/ECS/ComponentStore.h:76)의 sparse index·dense entity 목록·연속 `vector<T>`와 [CWorld::m_mapStores](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/ECS/World.h:229)의 type별 store 소유권을 사용한다.

이 비교의 질문은 “어떤 component를 어디에 저장하고, 누가 생성·등록·제거하는가”다. UE의 reflection·Actor lifecycle 전체, LostArk의 Prototype/Clone/Layer, Winters의 ECS store를 단순 이름 대응으로 합치지 않는다.

### 7.3. UWorld::Tick, GameInstance update, Winters scheduler

UE [UWorld::Tick](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/LevelTick.cpp:1502)은 world 갱신의 실행 소유자다. [RunTickGroup 호출](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/LevelTick.cpp:1750)은 PrePhysics → StartPhysics → DuringPhysics → EndPhysics → PostPhysics로 이어지고, DuringPhysics는 완료를 기다리지 않는 호출을 사용한다(:1765). 이는 tick의 물리 전후 관계와 대기 시점을 드러낸다.

LostArk [CGameInstance::Update_Engine](C:/Users/tnest/Desktop/LostArk/Engine/Private/GameInstance.cpp:192)은 object Priority_Update, camera refresh, object Update, physics, Post_Physics_Update, Level Update, Late_Update를 순서대로 연결한다. 함수 호출 순서를 설명할 수 있지만 이 사실만으로 UE의 TickFunction dependency와 task scheduling을 구현했다고 부를 수는 없다.

Winters [CSystemSchedular::Execute](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/ECS/SystemScheduler.cpp:85)는 phase별 실행 plan의 batch를 순회한다. 작은 batch 또는 job system 부재 시 순차 호출하고(:102), 병렬 batch는 job을 제출한 뒤 counter를 기다린다(:113). [CWorld](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/ECS/World.h:229)의 데이터 보관과 scheduler의 실행 책임을 구분한다. `World`라는 이름이 같아도 UE UWorld의 전체 역할과 같지 않다.

### 7.4. Level 교체와 동적 스트리밍의 경계

UE [UWorld::PersistentLevel](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Classes/Engine/World.h:951)과 [StreamingLevels](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Classes/Engine/World.h:999)는 하나의 world 안에서 level들을 관리한다. [ULevel::Actors](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Classes/Engine/Level.h:429)는 actor 목록, [OwningWorld](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Classes/Engine/Level.h:457)는 소속 world를 가리킨다. [UWorld::UpdateLevelStreaming](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/World.cpp:5024)과 [UWorldPartitionStreamingPolicy::UpdateStreamingState](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/WorldPartition/WorldPartitionStreamingPolicy.cpp:271)는 streaming 상태를 실제로 갱신하는 코드다.

LostArk [CLevel_Manager::Change_Level](C:/Users/tnest/Desktop/LostArk/Engine/Private/Level_Manager.cpp:14)은 이전 level resource 정리 후 `unique_ptr<CLevel>` 한 개를 교체한다. [CLIENT_LEVEL_DESCRIPTOR::MapLoadScope](C:/Users/tnest/Desktop/LostArk/Client/Public/LevelRegistry.h:37)는 제품 level의 로드 범위를 선언한다. 맵 진입 범위 선택, 화면 밖 물체의 culling, 실행 중 level/cell streaming은 서로 다른 문제다. 현재 MapLoadScope를 UE World Partition과 동등한 동적 셀 스트리밍으로 소개하지 않는다. Winters의 ECS CWorld에 대해서도 이번 조사로 level package나 streaming 구현 여부까지 판정하지 않는다.

### 7.5. 최종 카메라 시점을 결정하는 호출자

UE는 [APlayerCameraManager::DoUpdateCamera](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/PlayerCameraManager.cpp:881)에서 view target을 갱신한다. [UpdateViewTargetInternal](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/PlayerCameraManager.cpp:338)은 Blueprint camera 결과를 사용하거나 target의 `CalcCamera`를 호출한다. [AActor::CalcCamera](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/Actor.cpp:3688)는 active CameraComponent의 `GetCameraView`를 선택하고, CameraManager는 현재·pending `FMinimalViewInfo`를 blend한다([950행](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/PlayerCameraManager.cpp:950)).

LostArk [CCamera::Begin_PresentationOverride](C:/Users/tnest/Desktop/LostArk/Engine/Private/Camera.cpp:57)의 owner ID는 현재 카메라 override를 소유한 연출을 식별하고 priority는 다른 연출의 선점을 판정한다. 이전 world pose와 FOV를 저장하므로 종료 복귀 상태도 이 계층이 소유한다. [Update_PipeLine](C:/Users/tnest/Desktop/LostArk/Engine/Private/Camera.cpp:198)은 최종 world matrix의 역행렬로 View를 만들고 FOV·aspect·near/far로 Projection을 만들어 GameInstance에 전달한다.

Winters [CCamera::Ready](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Renderer/CCamera.cpp:6)는 eye/at/up와 projection 값을 설정하고 [RecalcView](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Renderer/CCamera.cpp:92)는 LookAt matrix를 계산한다. 세 구현 모두 최종 시점을 결정하지만 UE의 ViewTarget, LostArk의 owner/priority override, Winters의 camera/resolver를 동일한 관리자 API라고 설명하지 않는다.

### 7.6. CineCamera lens와 카메라 키 저작 도구

UE [UCineCameraComponent::GetCameraView](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/CinematicCamera/Private/CineCameraComponent.cpp:593)는 derived data 계산 → 일반 CameraView → lens 갱신 → near clip·off-center projection 적용 순서다. [GetHorizontalFieldOfView](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/CinematicCamera/Private/CineCameraComponent.cpp:288)는 filmback의 sensor width와 focal length, overscan을 사용한다. 위치·회전·FOV key뿐 아니라 렌즈의 의미를 보관하는 모델이다.

LostArk [CameraTool](C:/Users/tnest/Desktop/LostArk/Client/Private/CameraTool.cpp:359)은 Valtan/Kouku Area source 선택과 Reload·Save·Validate를 ImGui로 제공한다. UI가 값을 편집한 뒤 [CValtanCinematicCameraController::Sample_Cue](C:/Users/tnest/Desktop/LostArk/Client/Private/ValtanCinematicCameraController.cpp:273)가 cue의 pose를 계산하며 [VALTAN_CINEMATIC_CAMERA_POSE](C:/Users/tnest/Desktop/LostArk/Client/Public/ValtanCinematicCameraDocument.h:22)는 FOVY를 보관한다. Winters [SeqCameraKey](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/Cinematic/CSequenceAsset.h:28)는 위치·회전·FOV key이고 [CSequencePlayer::EvaluateCamera](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/Cinematic/CSequencePlayer.cpp:204)가 resolver를 통해 camera에 적용한다.

공통 구현은 시간에서 pose/FOV를 샘플링하는 부분이다. 위 LostArk/Winters 경로를 확인한 것만으로 UE CineCamera의 filmback·focal length·aperture·focus 모델 전체를 구현했다고 주장하지 않는다. 우리 도구의 Save/Validate와 실제 재생기의 연결을 먼저 설명하고, 렌즈 모델은 별도 비교 대상으로 둔다.

### 7.7. AIController와 서버 Brain의 입력·출력

UE [AAIController::OnPossess](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/AIModule/Private/AIController.cpp:484)는 Pawn 소유 lifecycle의 일부다. [RunBehaviorTree](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/AIModule/Private/AIController.cpp:1001)는 Blackboard asset 호환성을 검사하고 필요 시 `UBehaviorTreeComponent`를 생성·등록한 뒤 BrainComponent로 연결하여 `StartTree`를 호출한다(:1030~1039). possession, Blackboard 데이터, tree 실행기의 책임이 분리된다.

LostArk [CMonsterBrain::Update](C:/Users/tnest/Desktop/LostArk/Server/Private/MonsterBrain.cpp:250)는 SERVER_WORLD_ENTITY, player map, gameplay catalog, navigation, collision, fixed dt, server tick을 입력으로 받는다. 현재 상태와 target을 판정하고 상태·경로 및 damage event 출력을 갱신한다. [IDLE/CHASE 분기](C:/Users/tnest/Desktop/LostArk/Server/Private/MonsterBrain.cpp:331)는 공격 범위에서 PATTERN_WINDUP으로 전환하고 [GameRoom_BossSimulation](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_BossSimulation.cpp:3073)이 이 Brain을 호출한다. 이 경로는 typed 서버 상태 머신이며 UE Behavior Tree graph라고 부르지 않는다.

Winters 권위 서버는 [CServerAICommandProducer::Execute](C:/Users/tnest/Desktop/WintersEngine/Server/Private/Game/ServerAICommandProducer.cpp:24)에서 `CChampionAISystem::Execute(world, TickContext, outCommands)`를 호출한다. [ChampionAIBrain.cpp](C:/Users/tnest/Desktop/WintersEngine/Shared/GameSim/Systems/ChampionAI/ChampionAIBrain.cpp:15)의 retreat/champion/farm score 판단은 이 Shared AI 쪽 근거다. 아래 Engine BT 클래스가 존재한다는 사실을 서버의 실제 command 생산 경로로 대체해서 설명하지 않는다.

### 7.8. Blackboard, BT 실행기, graph editor를 따로 비교하기

UE [UBehaviorTreeComponent::StartTree](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/AIModule/Private/BehaviorTree/BehaviorTreeComponent.cpp:240)와 [TickComponent](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/AIModule/Private/BehaviorTree/BehaviorTreeComponent.cpp:1698)는 instance stack·node memory·실행 요청을 관리한다. [UBlackboardComponent::RegisterObserver](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/AIModule/Classes/BehaviorTree/BlackboardComponent.h:73)는 key 변경 관찰자를 연결한다. [ValueMemory/ValueOffsets](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/AIModule/Classes/BehaviorTree/BlackboardComponent.h:245)는 key 값을 보관하며 setter의 변경 경로는 observer 통지까지 연결된다(:361). 매 프레임 전체 tree를 처음부터 순회하는 구현 하나로 축약하지 않는다.

Winters [CBTSelector::Tick와 CBTSequence::Tick](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/AI/BehaviorTree.cpp:6)은 자식의 Running/Success/Failure를 검사한다. [CBlackboard](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/AI/Blackboard.h:16)는 `unordered_map<string, variant<bool,int,float,Vec3,string,uint64>>`를 사용한다. [CBehaviorTreeSystem::Execute](C:/Users/tnest/Desktop/WintersEngine/Engine/Private/ECS/Systems/BehaviorTreeSystem.cpp:9)는 BotComponent와 BlackboardComponent를 함께 조회하고 누적 시간이 [TICK_INTERVAL=0.2초](C:/Users/tnest/Desktop/WintersEngine/Engine/Public/ECS/Systems/BehaviorTreeSystem.h:40)에 도달하면 BTContext를 구성해 Tick한다. **실제 등록은 [Scene_InGameLifecycle.cpp](C:/Users/tnest/Desktop/WintersEngine/Client/Private/Scene/Scene_InGameLifecycle.cpp:493)의 `!m_bNetworkAuthoritativeGameplay` 조건 아래**이므로 이 BT를 현재 권위 서버의 공통 AI runtime이라고 소개하지 않는다.

UE 도구는 [FBehaviorTreeEditor::InitBehaviorTreeEditor](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Editor/BehaviorTreeEditor/Private/BehaviorTreeEditor.cpp:219), graph widget(:568), debugger 생성(:288), [SaveAsset_Execute](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Editor/BehaviorTreeEditor/Private/BehaviorTreeEditor.cpp:1391), pin 변경의 transaction(:1303)까지 별도 코드로 연결된다. 노드 실행 알고리즘, graph authoring, asset 저장, undo, 실행 중 debugger는 각각 증명해야 하는 기능이다. LostArk ImGui 도구의 저장·검증·서버 승인 기능을 이 기준으로 설명하되, UI 라이브러리 차이만으로 툴의 구현 원리를 판단하지 않는다.

이 추가 비교에서 확인한 것은 세 저장소의 해당 코드 경로와 데이터 소유권이다. 누락 기능 전체 목록, 실제 UE editor 조작 절차, GC pause나 AI 처리량의 수치는 조사하지 않았다. 후속 영상에서는 한 객체의 생성·제거, 카메라 override, AI 입력·결과를 실제 실행 증거로 확인하여 코드 설명과 구분한다.

## 8. Epic–GitHub 재접근 절차

현재는 `C:/Users/tnest/Desktop/UnrealEngine`에 소스가 확보되어 있으며 7절은 그 소스를 읽은 결과다. 아래 절차는 계정 재연결이나 다른 PC에서 다시 받을 때의 참고로 보존한다. 이 로컬 파일 확인과 브라우저에서 사용자 계정의 조직 권한을 직접 검증하는 것은 구분한다. 이전에 연동했다면 로그인한 GitHub 계정으로 [EpicGames/UnrealEngine](https://github.com/EpicGames/UnrealEngine)을 먼저 연다.

접근이 안 될 때 공식 순서는 Epic 계정의 **APPS & ACCOUNTS → Accounts → GitHub Connect**, 계정 연결, **Authorize EpicGames**, GitHub 이메일의 **Join @EpicGames**다. 공식 안내상 초대는 7일 안에 수락해야 한다. 예전 연동 상태만으로 현재 조직 접근이 유지된다고 가정하지 않는다. [Epic 공식 소스 접근 안내](https://www.unrealengine.com/ue-on-github?lang=en-US).

권한이 확인되면 원하는 UE release/commit을 고정하여 소스를 받는다. 현재 공식 페이지의 기본 API 문서가 5.8이라고 해서 예전 연구용 5.7.4와 동일 코드를 받았다고 기록하지 않는다. [Epic 다운로드 안내](https://dev.epicgames.com/documentation/en-us/unreal-engine/downloading-source-code-in-unreal-engine).

## 9. 다음 기술서에서 먼저 보완할 근거

1. VS 필터마다 `화면 위치 → 실파일 → 클래스 → 호출자 → 데이터 → 저장 대상 → 검증` 카드 한 장을 만든다.
2. Winters의 cinematic runtime, editor validator UI, FX graph compiler를 각각 분리하고 제품 호출자까지 추적한다.
3. 저장 계약을 JSON parse, semantic validation, staged replacement, atomic disk write, runtime reload, server acknowledgement로 나눠 각 경로의 실제 보장만 체크한다.
4. LoL DX11과 Elden RHI path의 renderer/asset 지원을 나눠 실측한다. interface 선언만 보고 지원 행렬을 채우지 않는다.
5. profiler 영상은 코드 설명과 실제 capture evidence를 같이 둔다. dropped/omitted event, raw history 제한, GPU frame attribution도 보여준다.
6. 자기소개서에는 확인한 구현 원리·책임 경계·해결한 문제와 재현 증거를 쓴다. Unreal 동급, 전체 AAA 엔진 구현, 모든 graph/runtime 연결 완료 같은 미검증 표현은 쓰지 않는다.

이 조사로 소스·데이터·VS 필터를 읽는 기준과 객체 수명·카메라·World·AI의 실제 3자 비교 근거를 추가했다. 전체 함수/멤버/알고리즘 사전, 다른 도메인까지 확대한 3자 비교, 실제 툴 화면과 영상 타임코드, 200페이지 PDF의 본문·도판은 후속 산출물이다.
