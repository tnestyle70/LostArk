# Release Effect 로딩 응답성과 실제 진행률 구현 계획

기준일: 2026-08-27

구현 브랜치: `codex/release-effect-loading-responsiveness`

구현 기준 HEAD: `88dfd58abfb28fc52078d3bcf72f2b4d35eee7b1`

구현 시작 시 `origin/main`도 같은 commit이었다. 앞선 원인 조사는 병합 완료된
`codex/team-lan-shared-server`의 `1664b0896816911bf72108f197d35d0ad1a816ab`에서 수행했지만,
실제 코드는 최신 `main`에서 만든 위 전용 브랜치에만 반영한다.

## 0. 결론과 이번 구현 경계

Release Character Select가 `CHARACTER SELECT: visual map` 화면에서 멈춘 것처럼 보이고 Windows가
`응답 없음`으로 표시한 직접 원인은 Server 접속이나 `127.0.0.1`, `E_FAIL` 승인 거부가 아니다.
Server 승인을 받은 뒤 Client가 Loading Level에 진입했고, 그 뒤 메인 스레드가 Product Effect의
renderer core를 처음 만들면서 큰 Effect HLSL 여섯 개를 동기 최적화 컴파일했다.

현재 `CLoader` 자체는 Debug와 Release 모두 worker thread에서 돈다. 그러나 Product Effect 준비는
`CMainApp::Update()`가 메인 스레드에서 실행한다. 기존의 "frame당 target 하나" 계약은 target 사이에서만
양보할 뿐, 첫 target 내부의 catalog parse, renderer core 여섯 shader 생성, document/resource 준비는
중간에 양보하지 않는다. Win32 message pump는 `Update()`가 반환해야 다시 실행되므로 이 한 호출이
길어지면 화면, 진행 바, 창 이동과 종료 메시지가 모두 멈춘다.

Debug와 Release의 thread 구조가 다른 것은 아니다. 차이는 `CShader::Initialize_Prototype()`가 Debug에서는
최적화를 생략하고 Release에서는 `D3DCOMPILE_OPTIMIZATION_LEVEL1`로 같은 HLSL을 런타임 재컴파일한다는
점이다. 현재 PC에서 첫 Effect shader 단독 probe는 Debug 계열이 약 25초였고, Release 계열은 2분을
넘겨 수동 중단됐다. 반면 Client 빌드는 이미 구성별 `fx_5_0` CSO를 만들고 있고
`Client/Bin/Release/Shader_VtxEffectMeshPreview.cso`도 존재한다. 런타임만 이 결과물을 무시하고 있다.

이번 구현은 다음 계약을 한 수직 슬라이스로 닫는다.

1. Debug/Release 제품 실행은 빌드된 CSO를 실행 파일 옆에서 읽고, 정상 경로에서 HLSL source compile을
   호출하지 않는다.
2. Loading 중 긴 파일 읽기, JSON parse/validate와 immutable resource manifest 생성은 기존 Loader
   worker의 수명 계약을 재사용한다.
3. D3D11 immediate context와 process-global catalog/queue/cache commit은 메인 스레드에 남기되,
   한 frame에 제한된 하나의 작은 commit unit만 처리한다.
4. Loading UI는 worker를 기다리지 않고 실제 phase, 완료/전체, 현재 Effect ID와 경과 시간을 poll한다.
   알 수 없는 작업을 시간 경과만으로 90%라고 표시하지 않는다.
5. Effect presentation 하나의 실패, unavailable target과 등록 실패는 기존 activation-ready 의미대로
   격리하고 Character Select의 Server gameplay 진입을 막지 않는다. Level resource 자체의 실패만 기존
   Level 실패 계약으로 Lobby에 복귀한다.

### 0.1 구현 중 확정된 설계 결정

초기 계획의 CPU-only stage와 main resumable resource cursor는 실제 계측 뒤 다음 구조로 대체했다. 아래
G02/G03/G05는 이 확정 구조를 기준으로 읽는다.

- worker는 document parse/validate뿐 아니라 D3D11 device-only renderer core/target resource stage까지
  수행한다. shared immediate context method는 호출하지 않으며 전달되는 context pointer는 commit 시 exact
  identity를 확인하기 위한 token이다.
- target document/projection, prepared-target index, receipt와 queue terminal state는 main만 publish한다.
  revision-independent device-scoped immutable renderer core/failure cache는 중복 core build를 막는 예외로
  worker가 mutex 아래 populate할 수 있다.
- command 방향은 latest-value single-slot mailbox, result 방향은 capacity-2 channel이다. 한 target result를
  보낸 worker는 main의 exact `TARGET_COMMIT_ACK` 전까지 다음 target으로 진행하지 않는다.
- main은 frame당 target result 하나를 소비한다. 긴 shader/resource 생성은 worker에서 끝났으므로 main에는
  catalog exact commit, renderer generation/device/core CAS 뒤 pointer swap, per-key receipt와 queue token
  commit만 남긴다. shader 1/6 또는 resource N/M main cursor는 만들지 않는다.
- catalog commit 뒤 renderer CAS가 실패하면 exact catalog commit receipt가 이번 삽입만 rollback한다.
  budget/duration/overlay receipt는 전역 map 전체 snapshot 대신 key별 node/value를 미리 stage한다.
- renderer probe의 `targetCommitMaximumMicroseconds`는 renderer CAS/swap만 측정한다. 전체 main result consume
  시간으로 해석하지 않으며 실제 창 응답성과 frame pacing은 사용자 Release smoke에서 별도 판정한다.

`Advance_ProductCuePreparation()` 전체를 `std::async` 또는 별도 thread로 감싸는 방식은 사용하지 않는다.
현재 Effect catalog와 presentation receipt는 main-thread 전역 상태이고, renderer cache mutex도 존재하며,
shared immediate context는 multi-thread protected가 아니다. 현재 함수를 통째로 worker에서 호출하면 data
race가 생기거나 메인 스레드가 같은 cache lock을 기다리면서 다시 멈출 수 있다.

## 1. 목표, 완료 조건과 제외 범위

### 1.1 완료 조건

- Debug와 Release의 제품 shader 로드 계약이 동일하다. 구성에 맞는 module-adjacent CSO만 사용한다.
- Release 정상 실행에서 `D3DX11CompileEffectFromFile` 호출 횟수는 0이다.
- CSO 누락, 빈 파일, 손상 bytecode와 technique/pass/input-layout 불일치는 정확한 논리 HLSL ID,
  실제 CSO 경로, byte 수와 HRESULT를 남기고 빠르게 실패한다. HLSL source compile로 조용히 우회하지 않는다.
- Loading 중 메인 스레드는 worker 완료를 wait/join하지 않는다. Win32 message pump, loading animation과
  progress snapshot poll이 계속 진행된다.
- 이번에 추가하는 Effect stage worker API는 target catalog/queue/prepared index를 직접 변경하지 않고
  immediate context method를 호출하지 않는다. device-scoped immutable renderer core cache만 위 예외를 따른다.
- shader/resource 생성은 worker의 완성된 target stage에 포함한다. 메인은 frame당 result 하나를 try-pop하고
  exact catalog/renderer/receipt/queue commit만 수행한다. renderer CAS/swap unit이 cold Release 100ms를
  넘으면 구현 완료로 인정하지 않는다. 전체 main result consume은 이 probe와 구분한다.
- Level resource와 Effect 진행 상태를 동시에 볼 수 있다. determinate 단계만 실제 완료/전체를 표시하고,
  opaque 단계는 spinner/heartbeat와 경과 시간을 표시한다.
- 100%는 Level worker 성공과 선택 target 집합의 `prepared`, `failed`, `unavailable` 또는 격리된
  registration failure terminal 상태, 남은 main commit cursor 없음이 모두 확인된 뒤에만 표시한다.
- fresh pull 뒤 정본 Debug/Release 빌드를 수행하면 필요한 CSO가 각 실행 파일 옆에 생성·복사된다.
  CSO 자체는 Git에 커밋하지 않는다.
- Release Character Select 첫 진입, 두 번째 같은 revision 진입, Debug 진입을 사용자가 직접 확인할 수
  있고, 자동 검증과 사용자 수동 관찰을 RESULT에서 분리한다.

### 1.2 제외 범위

- Server protocol, `127.0.0.1:7777`, world approval, socket handoff 계약은 변경하지 않는다.
- Character Select visual map, Effect의 모양·색·타이밍과 resource pack 내용은 변경하지 않는다.
- Release 전용 thread 경로나 Debug 전용 정상 fallback을 만들지 않는다.
- 새 범용 Thread Manager, 두 번째 Loader, background HLSL compiler와 영구 shader cache를 만들지 않는다.
- `C1041` PDB 점유 오류는 병렬/중복 빌드 프로세스의 별도 문제이므로 이번 범위에 넣지 않는다.
- Client/UI를 에이전트가 실행하거나 화면 PASS를 대신 판정하지 않는다.

## 2. 현재 호출 흐름과 구조적 결함

현재 정지 경로는 다음과 같다.

```text
Win32 message pump
→ CMainApp::Update
→ CEffectPresentationService::Advance_ProductCuePreparation
→ CEffectCatalog::Find                         // 최초 문서 parse와 전역 map 변경
→ CEffectDocumentRenderer::Prepare_VisualProgramTarget
→ Acquire_RendererCore
→ Build_RendererCore                           // Effect shader 여섯 개 연속 생성
→ CShader::Create
→ D3DX11CompileEffectFromFile                  // Release 최적화 compile, 중간 yield 없음
```

`CLevel_Loading`의 현재 진행 바도 원인을 가린다.

- Loader worker가 실행 중이면 실제 작업량과 무관하게 90%를 향해 시간 기반으로 채운다.
- Loader worker가 끝나기 전에는 Effect 준비 상태를 표시하지 않는다.
- 메인 스레드가 Effect compile에 들어가면 마지막 Loader 문자열인 `visual map`만 남는다.
- Effect queue의 완료 수는 target 전체 prepare가 끝나야 증가하므로 긴 target 내부 상태가 보이지 않는다.

renderer core의 현재 cache lock 범위도 줄여야 한다. core가 없을 때 여섯 shader를 만드는 전체 구간에서
전역 mutex를 잡고 있으므로, worker화를 하더라도 main의 조회가 같은 lock을 기다릴 수 있다. cache 조회,
candidate build, publish를 분리해 실제 mutex 보유 구간을 double-check와 pointer 교체로 제한한다.

## 3. 수정 파일과 역할

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/Shader.cpp` | module-adjacent CSO resolve/read, `D3DX11CreateEffectFromMemory`, bytecode/technique/pass/signature/input-layout fail-closed 검증과 source compiler 제거 |
| 필요 시 수정 | `C:/Users/user/Desktop/LostArk/Engine/Public/Model.h` | main-block admission을 넘는 model CPU parse가 확인될 때만 file stage와 device-only create API 분리 |
| 필요 시 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/Model.cpp` | worker CPU payload와 main/device-only resource 생성 분리 |
| 필요 시 수정 | `C:/Users/user/Desktop/LostArk/Engine/Public/Texture.h` | main-block admission을 넘는 texture file/decode가 확인될 때만 memory stage API 보강 |
| 필요 시 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/Texture.cpp` | staged texture byte/decode와 device upload 분리 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj` | 실제 제품 `CShader` 소비 HLSL의 Debug/Release `FxCompile` closure, 새 Effect load job H/CPP 등록, direct Client build의 Engine CSO fail-fast/copy |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Default/Client.vcxproj.filters` | 추가 `FxCompile`과 새 Effect load job H/CPP를 기존 물리 filter에 등록 |
| 수정 | `C:/Users/user/Desktop/LostArk/UpdateLib.bat` | Engine의 `Shader_Cell.cso`, `Shader_Deferred.cso`를 구성별 Client 실행 폴더에 복사 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Public/Effect_LoadPreparationJob.h` | dependency-free job epoch, target owner claim, bounded result/rebase channel과 progress lane 계약 |
| 추가 | `C:/Users/user/Desktop/LostArk/Client/Private/Effect_LoadPreparationJob.cpp` | cancellable fixed-capacity channel, owner release와 state transition 구현 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Loader.h` | immutable Effect stage request/result, 실제 loading phase와 thread-safe progress snapshot 계약 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Loader.cpp` | Level resource 뒤 parse/validate와 device-only Effect target stage 실행, exact ACK, 취소와 terminal publish |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Effect_Catalog.h` | main이 만드는 immutable target source snapshot과 worker용 parse/validate stage API |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Effect_Catalog.cpp` | 익명 namespace의 source/path/parser를 전역 commit 없는 CPU stage와 main commit으로 분리 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Effect_ProductPrewarmQueue.h` | Loading owner, prevalidated no-fail front token과 기존 prepared/failed/unavailable/registration-isolated terminal probe 계약 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Effect_ProductPrewarmQueue.cpp` | Loading/non-loading 중복 소비 방지, front reservation/commit과 기존 activation-ready 의미 보존 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Effect_PresentationService.h` | Loading request capture, worker bundle stage, main exact-commit과 progress API |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Effect_PresentationService.cpp` | queue/per-key receipt의 main-thread 소유, catalog rollback, stale result 폐기와 non-loading 경로 유지 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Effect_DocumentRenderer.h` | device-only complete target stage와 generation/device/core CAS swap commit 분리 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Effect_DocumentRenderer.cpp` | compiled core/target resource worker stage, short cache lock, target pointer-swap commit |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Public/Level_Loading.h` | fake percentage 대신 두 progress snapshot을 합성하는 화면 상태 |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/Level_Loading.cpp` | Level/Effect 동시 상태, determinate/indeterminate 표시와 정확한 activation gate |
| 수정 | `C:/Users/user/Desktop/LostArk/Client/Private/MainApp.cpp` | Loading 밖 기존 incremental 준비 경로와 worker Loading 경로의 상호 배제, worker join 뒤 cache 해제 순서 |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/Build/Test-CompiledShaderClosure.ps1` | product source literal → `FxCompile` → 구성별 module-adjacent CSO closure와 basename 충돌 검사 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/Build/Invoke-BuildAndRegression.ps1` | Debug/Release 빌드 뒤 compiled shader closure, EffectRender와 PointLightFalloff harness build/run gate 실행 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/EffectRenderContractHarness/Private/EffectRenderContractHarness.cpp` | CSO load/failure, worker ownership, staged commit, progress, stale revision와 rollback 실행 검증 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/EffectRenderContractHarness/Default/EffectRenderContractHarness.vcxproj` | production job/queue seam link와 구성별 검증 CSO를 harness 실행 폴더에 fail-fast 배포 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/EffectRenderContractHarness/Default/EffectRenderContractHarness.vcxproj.filters` | production link와 shader build item을 기존 filter에 등록 |
| 필요 시 수정 | `C:/Users/user/Desktop/LostArk/Tools/PointLightFalloffContractHarness/Private/PointLightFalloffContractHarness.cpp` | module-adjacent Deferred CSO path/time assertion이 기존 render 판정만으로 부족할 때 보강 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/PointLightFalloffContractHarness/Default/PointLightFalloffContractHarness.vcxproj` | Debug/Release `Shader_Deferred.cso` 존재 검사와 harness target directory 배포 |
| 필요 시 수정 | `C:/Users/user/Desktop/LostArk/Tools/PointLightFalloffContractHarness/Default/PointLightFalloffContractHarness.vcxproj.filters` | shader build item을 filter에 노출할 때만 등록 |
| 검증 후 수정 | `C:/Users/user/Desktop/LostArk/CLAUDE.md` | fresh pull/build의 CSO 배포와 Loading 상태 사용법 정본 갱신 |
| 검증 후 수정 | `C:/Users/user/Desktop/LostArk/.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md` | Product Effect 준비 실패 격리와 Loading presentation 계약 중 팀 소비자에게 필요한 내용만 갱신 |
| 추가 | `C:/Users/user/Desktop/LostArk/.md/GB/08-27/2026-08-27_RELEASE_EFFECT_LOADING_RESPONSIVENESS_IMPLEMENTATION_RESULT.md` | 실제 diff, 계측, 자동 검증, 사용자 수동 검증과 미완료 경계 기록 |

새 C++ 파일은 dependency-free `Effect_LoadPreparationJob.h/.cpp` 한 쌍으로 제한한다. Loader와 Effect
Presentation이라는 실제 소비자가 있고, full Client를 끌어오지 않고 fixed-capacity channel, epoch와 owner
release를 harness로 검증하기 위한 seam이다. 두 파일은 Client project/filters와 EffectRender harness에
모두 등록한다. 새 C++는 UTF-8 BOM 없이, 새 PowerShell은 UTF-8로 저장하며 기존 C++ 파일은 현재 인코딩을
유지한다.

## 4. 데이터와 thread ownership 계약

### 4.1 immutable discovery request와 target epoch

`CLevel_Loading::Initialize()`는 Loader thread를 시작하기 전에 target JSON을 직접 읽지 않는다. main은
다음 bounded discovery input과 immutable catalog source snapshot만 만든다.

- next level과 level registry identity
- selected class와 그 class/level의 stable cue/projectile source registration
- 현재 Effect catalog revision
- Loading instance가 발급한 monotonic `jobEpoch`
- ProjectDataRoot의 canonical absolute root와 `Effects/Authored/...` ProjectDataRoot-relative source index
- 정확한 `shared_ptr<const CEffectMaterialProgramRegistry>`와 그 generation
- 실제 `ID3D11Device*` COM identity
- target별 기존 catalog document/projection identity와 prepared record identity를 나중에 capture할 수 있는
  immutable source-registration handle

Effect document source path와 runtime asset ID를 섞지 않는다. Product Effect JSON은
`CProjectDataRoot` 아래 `Data/Effects/Authored`에서 읽는 ProjectDataRoot-relative 문서이고, 문서가 참조하는
model/texture/vector-field만 `Client/Bin/Resources` 상대 asset ID다.

Loader worker의 첫 Effect 단계가 cue/projectile 문서를 bounded read/parse하여 dedupe된 available Product
Effect ID와 unavailable ID/root reason을 discovery result로 반환한다. main은 그 result와 current catalog
source snapshot을 CAS 검증하고 available target을 queue에 transactionally claim한다. claim 성공 뒤
`ACCEPT_DISCOVERY` mailbox 명령으로 exact immutable target epoch를 worker에 보내야 실제 Effect document
stage가 시작된다. unavailable ID는 target vector에서 조용히 버리지 않고 discovery failure receipt와
progress failed count에 남긴다.

`CLoader::Create()`는 최초 immutable discovery request를 값으로 받고 그 뒤에는 4.4의 typed mailbox로만
새 immutable target epoch를 받는다. worker가 참조 중인 vector를 수정하는 setter는 만들지 않는다.

여기서 identity를 하나의 `generation` 숫자로 합치지 않는다. renderer prepared-cache generation은 다른
target commit에도 증가하므로 job 전체 stale 판정에 쓰지 않는다. commit은 `jobEpoch`, catalog revision,
device pointer identity, exact material registry shared pointer/generation, stable Effect ID/source path, 기존
catalog document/projection identity와 그 target의 prepared record identity를 각각 비교한다. 같은 catalog
revision에서 Debug Save가 document pointer만 교체한 경우도 stale candidate가 새 문서를 덮지 못한다.

### 4.2 새 Effect stage의 worker 소유 상태

아래 경계는 이번에 `CLoader` worker에 추가하는 Effect stage에 적용한다. 기존 Level resource Loader가 이미
device/context를 받아 map, character와 prototype을 준비하는 경로를 이번 작업에서 함께 재설계한다는 뜻이
아니다. 그 기존 경로의 존재도 새 Effect API가 context를 사용해도 된다는 근거로 삼지 않는다.

새 Effect stage는 다음만 소유한다.

- document와 CSO/resource file byte 읽기
- JSON parse와 schema/identity/path validation
- renderer projection과 resource manifest의 immutable candidate
- job epoch, Effect ID, catalog revision, device identity와 source identity가 포함된 stage result
- 성공 또는 원문이 보존된 실패 이유

새 Effect stage는 다음을 절대 하지 않는다.

- `CEffectCatalog` lazy `Find()`로 process-global map을 변경
- Product queue의 `Complete_Front()` 또는 prepared/failed set 변경
- renderer prepared cache, budget/duration/overlay receipt publish
- `ID3D11DeviceContext::Map/Unmap`, `Apply`, `IASet*`, draw와 pipeline state 변경
- `Change_Level`, socket, UI object 변경

새 Effect stage API에는 `ID3D11DeviceContext*`를 인자로 전달하지 않는다. 첫 구현에서는 Effect GPU
object도 만들지 않고 CPU/file candidate만 반환한다. 4.4의 job-scoped channel lock은 완성된 result
handle 하나를 move하는 동안만 잡고, parse, file I/O와 validation 중에는 잡지 않는다.

### 4.3 main thread 소유 상태

메인 스레드는 다음을 소유한다.

- catalog revision과 queue front 재검증
- CSO byte에서 Effect/device object 생성
- model/texture/buffer의 device resource 생성
- renderer prepared cache와 budget/duration/overlay receipt commit
- Product queue `prepared/failed` terminal 처리
- Loading UI snapshot 소비와 Level activation 요청

현재 D3D11 device는 `D3D11_CREATE_DEVICE_SINGLETHREADED` 없이 생성되므로 `ID3D11Device::Create*`를 worker에서
사용할 수 있다. 실제 구현은 cold stage가 main-block admission을 넘는 것을 확인한 뒤 renderer core와
target resource의 device-only candidate 생성까지 worker로 이동했다. worker API가 받는 context smart pointer는
commit의 exact identity token이며 immediate context method는 호출하지 않는다. main은 job epoch, catalog
revision, device/context/core/source identity를 다시 검증한 뒤 완성된 candidate map/session을 swap한다.

### 4.4 job-scoped channel, rebase와 owner release

각 immutable batch는 별도 `jobEpoch`를 가진다. result channel은 process-global 무제한 deque가 아니라
job-scoped capacity 2 SPSC channel로 고정한다. worker producer가 full channel에서 기다릴 때는
`cancel`, `rebase`, `close` predicate를 함께 보고, main은 절대 channel을 기다리지 않고 `try_pop`만 한다.
cancel/rebase/Level failure는 condition variable을 `notify_all`해 blocked producer를 깨운다.

Loader worker는 one-shot batch를 처리하고 바로 끝나지 않는다. 다음 bounded mailbox 명령까지 살아 있다.

```text
LOAD_LEVEL_RESOURCES
→ DISCOVER_EFFECT_TARGETS(jobEpoch N)
→ wait(ACCEPT_DISCOVERY exact target epoch | REBASE | CANCEL)
→ STAGE_EFFECT_EPOCH(jobEpoch N)
→ target result마다 wait(TARGET_COMMIT_ACK exact ID | REBASE | CLOSE | CANCEL)
→ EPOCH_STAGE_COMPLETE
→ wait(REBASE latest immutable epoch | CLOSE | CANCEL)
```

mailbox는 latest immutable epoch 하나만 보관한다. catalog revision drift가 생기면 main은 old result/commit
cursor를 stale로 폐기하고 old owner claim을 실패 없이 release한 뒤 새 revision batch를 `REBASE`로 보낸다.
덮어쓴 mailbox batch가 있으면 그 batch의 owner claim도 즉시 release한다. 새 epoch stage와 main terminal
commit이 끝나면 main이 `CLOSE`를 보내고 worker가 정상 종료한 뒤 Loader를 `SUCCEEDED`로 publish한다.

Product queue에는 `Claim_LoadingTargets(jobEpoch, targets)`와
`Release_LoadingOwner(jobEpoch, disposition)`에 해당하는 transaction을 둔다. claim은 기존 pending membership
위에 놓는 임시 owner이며 target을 별도 저장 계약으로 복제하지 않는다.

- 정상 prepared/failed/unavailable terminal: terminal 반영 뒤 claim release
- stale/rebase: 실패 latch 없이 claim release 후 새 epoch claim
- thread 생성 실패, cancel, disconnect, Level resource/device identity/worker-channel structural 실패:
  모든 claim release; 기존 pending target은 normal incremental owner가 다시 소비할 수 있음. 단, 현재
  Loading은 fatal로 닫고 같은 turn에 main incremental fallback을 실행하지 않음
- registration failure: claim을 만들지 못한 이유를 isolated terminal receipt로 보존

어떤 exit path에서도 Loading owner만 남은 pending ID가 생기지 않음을 dependency-free job harness로
검증한다.

## G00. 빌드된 CSO를 제품 runtime shader 정본으로 사용

`CShader`의 정상 제품 생성 경로는 받은 논리 HLSL 경로의 stem으로 `<실행 모듈 폴더>/<stem>.cso`를
resolve한다. 현재 작업 디렉터리가 `Client/Default`인지와 무관하도록 current working directory가 아니라
`GetModuleFileNameW(nullptr, ...)`로 실행 모듈 폴더를 구한다.

resolved 파일을 bounded byte vector로 읽고 `D3DX11CreateEffectFromMemory()`를 호출한다. 이후 기존
clone/Begin 계약은 유지하되 현재 초기화 코드의 unchecked 구간을 그대로 재사용하지 않는다.
`IsValid()`, `GetDesc()` HRESULT, technique/pass 개수, pass input signature의 null/0-byte 여부와
`CreateInputLayout()` 결과를 모두 검증한다. 다음 실패는 구분된 진단을 남긴다.

- CSO 없음 또는 읽기 실패
- 0-byte/truncated bytecode
- Effect bytecode 생성 실패
- technique 없음 또는 pass signature 불일치
- input layout 생성 실패

각 오류에는 configuration을 추측한 문자열이 아니라 실제 module path, 논리 HLSL path, resolved CSO path,
읽은 byte 수와 HRESULT가 포함된다. 정상 제품 경로는 어떤 실패에서도 HLSL compile로 fallback하지 않는다.
그 fallback은 Release의 수분짜리 정지를 다시 만들고 pull 받은 팀원의 배포 누락을 숨기기 때문이다.

현재 repository에서 source compiler의 실제 consumer는 `CShader` 내부 한 곳뿐이고 별도 authoring hot
reload API는 없다. 따라서 미래용 `Create_FromSourceForAuthoring`을 만들지 않고 이번 변경에서
`D3DX11CompileEffectFromFile` 제품 경로를 제거한다. HLSL 변경은 Debug/Release MSBuild `FxCompile` 뒤 새
CSO로 반영한다. 별도 미래용 counter API를 추가하지 않고 closure gate가 production source/import에서
compiler 호출이 0인지 검사하며, runtime probe는 실제 compiled-load path/byte/time만 기록한다.

### G00-1. MSBuild closure

현재 Client project에 이미 main Effect shader 여섯 개를 포함한 다수 HLSL의 `FxCompile` 항목이 있다.
직접 `CShader::Create`가 소비하지만 현재 등록이 빠진 다음 V2 파일을 실제 consumer/configuration과
대조해 추가한다.

조사 시점에는 Engine Debug/Release의 Cell/Deferred CSO는 모두 존재하지만 `Client/Bin/Release`에는 두
파일이 모두 없고 Client Debug의 Deferred CSO는 현재 Engine Debug 출력과 크기가 다른 stale 파일이었다.
따라서 이 항목은 선택적 최적화가 아니라 fresh pull/direct build 배포 closure다.

- `Shader_EffectAnimMeshV2.hlsl`
- `Shader_EffectMeshV2.hlsl`
- `Shader_EffectRectV2.hlsl`
- `Shader_EffectParticleV2.hlsl`
- `Shader_EffectDecalV2.hlsl`
- `Shader_EffectTrailV2.hlsl`
- `Shader_VtxAnimMeshPreview_V2.hlsl`

각 항목은 `fx_5_0`으로 Debug/Release 구성별 output directory에 같은 stem의 `.cso`를 만든다. include된
`.hlsli`는 MSBuild/FXC dependency tracking을 유지하고 별도 runtime 파일로 취급하지 않는다.

Engine project가 생성하는 `Shader_Cell.cso`와 `Shader_Deferred.cso`는 `UpdateLib.bat Debug|Release`가
해당 Client module directory로 복사한다. 이것만으로는 Visual Studio의 Client project 직접 build가
닫히지 않으므로 `Client.vcxproj`의 기존 `PrepareEngineSdk`/`DeployClientRuntimeDependencies` target에도
구성별 Engine CSO 존재 `Error`와 `$(TargetDir)` copy를 넣는다. stale 파일이 남아도 `SkipUnchangedFiles`
판정이 source의 새 timestamp/hash를 반영하도록 구성별 Engine output을 항상 source로 쓴다.

전역 `CShader` 계약의 다른 실제 consumer인 `PointLightFalloffContractHarness`는
`Shader_Deferred.cso`를 harness target directory에 배포한 뒤 Debug/Release WARP 렌더 회귀를 통과해야
한다. EffectRenderContractHarness도 자신이 사용하는 Effect CSO를 target directory에 명시적으로
배포한다. Product shader call site와 `FxCompile` 항목 사이에 basename 충돌이 있으면 자동으로 한쪽을
선택하지 않고 build gate에서 실패시킨다.

`Test-CompiledShaderClosure.ps1`은 프로젝트 파일이 가진 `FxCompile`을 정본으로 사용한다. 별도 shader
manifest를 만들지 않는다. `-Configuration`은 `Debug|Release`만 받고 x64 조건과
`ExcludedFromBuild`를 실제 MSBuild 구성처럼 평가한다. 직접 literal consumer와 Effect V2 wrapper를 통해
선택되는 shader, project item, 예상 CSO stem, Client/Effect harness/PointLight harness별 module-adjacent
deployment의 1:1 closure를 검사한다. Engine 원본과 배포된 Cell/Deferred CSO는 non-empty뿐 아니라
SHA-256이 같아야 한다. CSO는 build artifact이므로 Git stage 대상이 아니다.

## G01. bounded discovery 뒤 정확한 Effect target epoch 확정

현재 `CLevel_Loading::Initialize()`의 순서를 다음과 같이 바꾼다.

```text
Server-approved next level과 selected class 확인
→ runtime catalog/source registration snapshot과 bounded discovery request 생성
→ CLoader 시작, worker가 cue/projectile 문서를 읽어 available/unavailable target discovery
→ main이 discovery/source identity 재검증, exact target queue claim
→ ACCEPT_DISCOVERY로 immutable target epoch 전달
→ worker가 exact target만 CPU stage
→ main은 Loading UI와 commit seam을 계속 update
```

Character Select discovery는 승인된 class의 cue/projectile source만, Valtan은 해당 level descriptor가
등록한 source만 읽는다. 전체 Effect catalog를 무조건 prewarm하지 않는다. discovery file에는 byte/row
bound와 elapsed probe를 적용한다. ID는 stable Effect asset ID로 dedupe하고 prototype tag, pointer, vector
index를 저장 경계로 쓰지 않는다. unavailable ID/count/root reason도 terminal receipt에 포함한다.

Loading 중 `CMainApp::Update()`는 같은 target을 기존 main-thread incremental queue에서 다시 parse/build하지
않는다. Loading job 소유 target은 Loader pipeline만 stage하고 main Loading commit seam만 commit한다.
Loading 밖에서 뒤늦게 등록된 target과 authoring reload는 기존 incremental path를 사용하되 두 경로가 같은
target/revision을 동시에 소유하지 않도록 owner state를 검사한다.

revision이 Loading 도중 바뀌면 old result를 실패로 latch하지 않는다. main이 result의 job epoch,
catalog revision, device/source identity와 현재 queue owner를 재검증하고 stale candidate/cursor를 폐기한다.
그 다음 4.4의 owner release와 bounded `REBASE` mailbox로 새 immutable epoch를 보낸다. 이전 revision의
visible prepared record는 새 target record가 commit될 때까지 보존한다.

## G02. worker complete target stage와 bounded main commit 분리

### G02-1. parse/validate와 device-only target stage

기존 `CEffectCatalog::Find()`와 `Prepare_VisualProgramTarget()` 안에 섞인 작업을 stage와 commit으로 나눈다.
worker는 process-global catalog를 다시 조회하지 않고 target epoch가 가진 immutable source index,
ProjectDataRoot-relative document path, exact material registry와 device/context identity를 사용한다. worker
stage는 document를 읽고 다음 완성 candidate를 만든다.

- parsed/validated Effect document와 stable identity
- stage 시작 당시 기존 catalog document/projection pointer identity
- visual projection과 scene budget candidate
- module-adjacent CSO에서 생성한 renderer core와 target별 shader/model/texture/vector-field device resource
- target별 duration/overlay 계산 candidate
- job epoch/catalog revision/device identity/material registry generation/source identity

resource file read, parse와 device create는 target stage timer에 포함한다. renderer core와 target stage 시간,
renderer swap 시간은 별도 probe로 남긴다. device-scoped immutable renderer core/failure cache는 worker가
populate할 수 있지만 catalog document/projection, prepared-target index, receipt와 queue terminal은 publish하지
않는다.

parse/validate 중 하나라도 실패하면 partial candidate를 publish하지 않고 실패 result만 반환한다. worker는
다음 target으로 계속 진행한다. cooperative cancel은 파일 사이, document element loop와 target 사이에서
확인한다.

### G02-2. exact target bundle commit

worker가 한 target의 무거운 resource stage를 완성하고 exact ACK를 기다리므로 main에는 다음 짧은 commit만
남긴다.

```text
→ queue front와 outcome prevalidate/reserve token
→ catalog exact identity commit
→ renderer generation/device/core CAS와 candidate map/session swap
→ per-key budget/duration/overlay receipt commit
→ no-fail Commit_PrevalidatedFront(token)
→ main reference release와 exact TARGET_COMMIT_ACK
```

한 frame은 result 하나만 try-pop한다. shader/resource cursor는 worker target stage로 대체했다. renderer
CAS/swap 소요는 구조화된 probe로 기록하고 cold Release 100ms를 넘으면 구현 blocker로 처리한다. catalog,
receipt와 queue까지 포함한 전체 main consume은 renderer probe와 구분하고 사용자 smoke에서 창 응답성과
frame pacing을 확인한다.

renderer core cache는 다음 transaction으로 바꾼다.

```text
짧은 lock: 현재 device identity와 기존 core pointer 확인
→ lock 해제
→ local candidate에 shader/resource unit을 순차 생성
→ 짧은 lock: device identity와 기존 core pointer 재확인/double-check
→ 이미 다른 candidate가 이겼으면 local candidate 폐기
→ 아니면 완성된 core shared pointer만 publish
```

shader 여섯 개 중 하나라도 실패하면 partial target을 prepared index에 넣지 않는다. 완성 target resource와
candidate map/session은 stage가 보관한다. 기존 budget/duration/overlay consumer 계약은 유지하되 전역 map
전체를 target마다 복사하지 않고 해당 key용 node/value만 commit 전에 준비한다. catalog가 새
document/projection을 삽입한 뒤 renderer CAS가 실패하면 exact commit receipt로 이번 삽입만 rollback한다.
이전 prepared target은 새 candidate map이 publish될 때까지 보존된다.

현재 `Complete_Front()`는 FIFO mismatch로 실패할 수 있으므로 record publish 뒤 그대로 호출하지 않는다.
모든 실패 가능 validation과 allocation이 끝난 뒤 queue가 `jobEpoch/front/outcome`을 검증·예약한
`PREWARM_FRONT_COMMIT_TOKEN`을 발급한다. token이 유효한 동안 해당 front mutation을 막고, main은
prepared-index pointer를 no-throw swap한 뒤 실패 불가능한 `Commit_PrevalidatedFront(token)`으로 terminal
state와 owner release를 끝낸다. cancel을 포함한 새 cancellation point를 token 발급과 이 두 no-fail
operation 사이에 두지 않는다. token prevalidation이 실패하면 아무 global publish도 하지 않는다.

target 실패 token은 단순 failed ID set 외에 catalog revision, Effect ID, root error code와 bounded 원문
메시지를 가진 failure receipt까지 미리 준비한다. 같은 shared core failure는 root receipt 하나를 참조해
로그 폭증을 막는다. 같은 revision의 duplicate는 prepared/pending/failed set으로 다시 stage하지 않는다.

## G03. 실제 progress snapshot과 Loading UI

`CLoader`는 기존 status mutex와 atomic state를 확장해 Level resource progress snapshot을 제공하고,
`CEffectLoadPreparationJob`은 worker-owned Effect progress snapshot을 제공한다. main은 둘을 copy만 하며
worker progress를 직접 수정하지 않는다. `CLevel_Loading`은 두 snapshot을 읽어 Level과 현재 Effect
target 상태를 함께 표시한다. snapshot은 최소 다음 의미를 가진다.

| 필드 | 의미 |
|---|---|
| `phase` | Level resource 단계 또는 `TARGET_STAGE`, `EPOCH_STAGE_COMPLETE`, `FAILED`, `CANCELLED` |
| `determinate` | 현재 단계가 실제 denominator를 아는지 여부 |
| `completed`, `total` | 실제 완료 unit 수. `determinate=false`일 때 가짜 값으로 채우지 않음 |
| `currentId` | 현재 map/resource/Effect의 stable ID 또는 logical shader ID |
| `status` | 짧고 사용자가 읽을 수 있는 현재 작업 |
| `elapsedMs` | 현재 phase 시작 뒤 경과 시간 |
| `isolatedFailureCount` | 현재 Effect epoch에서 격리된 target 실패 수 |

문자열과 복합 snapshot은 짧은 mutex scope에서 copy하고 count/state를 같은 snapshot lock으로 일관되게
읽는다. parse, file I/O, shader/device 생성과 cache commit 중 progress lock을 잡지 않는다.

화면은 Loader가 끝날 때까지 Effect 상태를 숨기지 않고 두 줄을 함께 표시한다.

```text
Level resources: visual map (indeterminate, 3.2s)
Effects: Preparing Product Effect document and device resources 8/41 - <effectAssetId>
```

opaque file/device API 내부 백분율은 알 수 없으므로 `현재 unit/전체 + 경과 시간 + heartbeat`만 표시한다.
기존의 worker 실행 여부를 근거로 90%를 채우는 코드는 제거한다. 모든 active phase가 determinate일 때만
실제 unit 합으로 progress bar를 만들고, 하나라도 denominator가 없으면 indeterminate animation을 쓴다.

Level activation 조건은 다음 conjunction 하나로 고정한다.

```text
Level Loader == SUCCEEDED
AND 모든 선택 Effect target == PREPARED, FAILED 또는 UNAVAILABLE
    또는 target registration failure가 기존 계약대로 isolated terminal
AND current jobEpoch의 result channel empty
AND EPOCH_STAGE_COMPLETE result와 worker close/terminal readiness 확인
```

이 판정은 기존 `Is_ProductPrewarmTargetActivationReady()`의 catalog-revision-current와
registration-failure-isolated 의미를 보존하고 새 boolean 우회 판정을 중복 구현하지 않는다. Effect 실패,
unavailable 또는 registration failure 수가 0이 아니면 warning/status를 남기되 presentation 격리 계약에
따라 activation은 허용한다. Level map/model/prototype 실패는 기존처럼 activation하지 않고 정확한 실패
이유로 Lobby 복귀 경로를 탄다.

이 readiness는 Level/class presentation 전환에만 적용한다. 기존 Valtan Debug audition spawn처럼
`all-prepared`를 요구하는 소비자는 failure-isolated Level readiness를 재사용하지 않고 strict prepared
policy를 유지한다. progress terminal count와 실제 소비 가능 여부를 하나의 universal boolean으로 합치지
않는다.

## G04. 취소, 종료와 실패 격리

새 Effect thread를 추가하지 않고 현재 Loader의 수명 계약을 재사용한다.

```text
cooperative cancel 요청
→ 5초 bounded join
→ CancelSynchronousIo
→ 추가 5초 bounded join
→ 그래도 남으면 ERROR_TIMEOUT 전용 fail-fast
```

`TerminateThread`로 worker만 죽이고 process를 계속 실행하지 않는다. CSO 전환이 먼저 구현되어야 하는
이유도 source compiler 한 호출은 cooperative cancel 지점이 없기 때문이다.

cancel 또는 Level 전환 폐기 시 main은 더 이상 새 commit unit을 시작하지 않는다. current job channel과
미완성 local candidate를 폐기하고 global prepared state는 그대로 둔 뒤 4.4의 owner release를 반드시
호출한다. worker가 늦게 반환한 result는 job epoch/catalog revision/device/source identity/owner state
검증에서 stale로 거부한다.

실패 분류는 다음과 같다.

- Level map/model/prototype/registry 실패: Loading 실패, activation 금지, 기존 Lobby 복구 계약 사용
- cue/projectile target discovery read/parse/validation 실패: 정상 empty로 취급하지 않고
  registration-failure-isolated receipt, Effect 없이 Level 진입 계속
- Effect document/path/schema 실패: 해당 target failed terminal, 다른 target과 Level 진입 계속
- Effect target unavailable 또는 등록 실패: 기존 activation-ready helper로 isolated terminal 처리하고
  catalog revision이 current가 될 때까지의 대기 의미를 보존
- Effect CSO/resource/device 생성 실패: 해당 target 또는 공유 core를 사용하는 target을 빠르게 failed
  terminal로 만들고 Level 진입 계속; 동일 오류를 target마다 수십 번 중복 출력하지 않음
- stale job epoch/catalog revision/source identity: 실패로 기록하지 않고 candidate 폐기, owner release와
  bounded rebase
- 실제 device pointer identity 변경: 같은 catalog revision에서 rearm을 약속하지 않고 Loading fatal로
  처리, 모든 owner release와 기존 prepared state 보존. device recreation 수직 슬라이스는 이번 범위가 아님
- result channel/worker invariant 위반: Effect target failure로 축소하지 않고 Loading fatal, 정확한 structural
  오류 보존, 모든 owner release, partial global commit 금지와 기존 Lobby 복구 경로

공유 renderer core 실패 시 아직 처리하지 않은 target을 동일한 root failure receipt로 terminal 처리해
무한 재시도를 막는다. queue failure key는 현재 계약대로 catalog revision에 묶고 명시적 catalog revision
변경에서만 다시 시도한다. device 변경에 따른 같은-revision failed-target rearm은 지원한다고 기록하지
않는다.

## G05. 자동 검증

### G05-1. compiled shader closure와 CShader contract

Debug와 Release 각각 다음을 검증한다.

1. 모든 직접 제품 `CShader` 논리 HLSL에 정확히 하나의 `FxCompile` producer가 있다.
2. 실행 모듈 옆에 non-empty CSO가 있고 basename 충돌이 없다.
3. `D3DX11CreateEffectFromMemory`, Effect `IsValid/GetDesc`, technique/pass count, non-empty input signature와
   input layout 생성이 성공한다.
4. production source/import에 `D3DX11CompileEffectFromFile`가 없고 runtime probe가 compiled-load
   path/byte/time을 보고한다.
5. missing, 0-byte, truncated와 잘못된 Effect bytecode가 빠르게 실패하고 HLSL fallback을 호출하지 않는다.
6. failed candidate가 이전 `CShader`/renderer core를 교체하지 않는다.
7. Client direct build와 `UpdateLib.bat`가 모두 Engine Cell/Deferred CSO를 구성에 맞게 갱신한다.
8. PointLightFalloffContractHarness가 module-adjacent Deferred CSO로 Debug/Release WARP 렌더를 통과한다.

### G05-2. async ownership, progress와 transaction harness

`EffectRenderContractHarness`에 fake/delayed stage seam과 실제 WARP/device fixture를 사용해 다음을 추가한다.

- worker를 의도적으로 지연해도 main tick이 wait하지 않고 progress heartbeat를 계속 읽음
- worker stage가 shared immediate context method를 호출하지 않고 전달된 context pointer를 identity로만 사용
- capacity 2 result channel full 상태에서 cancel/rebase가 blocked producer를 깨우며 main은 try-pop만 사용
- thread 생성 실패, cancel, disconnect, Level failure와 mailbox overwrite가 모든 Loading owner claim을 해제
- cue/projectile discovery가 worker에서 수행되고 available/unavailable ID와 root reason을 모두 보존
- request 순서, duplicate dedupe와 target별 FIFO terminal 처리
- worker stage 뒤 exact target ACK 전에는 다음 target으로 진행하지 않음
- compiled renderer core와 target resource 준비 시간, renderer CAS/swap 시간이 별도 probe로 기록되고
  renderer swap unit이 cold Release 100ms 이하임
- progress count가 단조 증가하고 `completed <= total`, terminal 전 100%가 되지 않음
- indeterminate phase를 fake 90%로 변환하지 않음
- target 하나의 parse/resource 실패가 다음 target과 기존 prepared target을 보존
- unavailable target과 registration failure가 기존 helper 의미대로 terminal 처리되어 무한 대기하지 않음
- core/target build 실패, cancel과 stale job/revision/source result가 prepared-target record를 publish하지 않음
- catalog insert 뒤 renderer stale-generation 거부가 exact catalog document/projection을 rollback함
- budget/duration/overlay receipt commit이 전역 map 전체 복사 없이 target key만 변경함
- revision 변경은 stale candidate를 failed로 latch하지 않고 old owner release 뒤 bounded mailbox의 새 epoch로
  재등록
- 같은 catalog revision에서 document/projection pointer만 교체된 result가 CAS에서 stale로 거부됨
- queue front token prevalidation 실패는 prepared index를 publish하지 않고, 유효 token의 pointer swap 뒤
  terminal commit은 실패할 수 없음
- device identity 또는 result-channel invariant 변경은 same-revision Effect fallback/rearm이 아니라 owner
  release를 동반한 Loading fatal로 끝남
- 같은 revision의 두 번째 진입은 이미 prepared target을 다시 읽거나 생성하지 않음
- Loader 성공과 Effect terminal/commit-empty가 모두 충족되기 전 activation gate가 열리지 않음
- Level/class failure-isolated readiness와 Valtan audition all-prepared readiness가 서로 섞이지 않음

### G05-3. 정본 검증 명령

구현 뒤 실제 존재하는 현재 정본 명령만 사용한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Validate-EffectSources.ps1 `
    -RepositoryRoot C:/Users/user/Desktop/LostArk

powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Sync-EffectDataProject.ps1 -Check

powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 `
    -Configuration Debug

powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 `
    -Configuration Release

powershell -ExecutionPolicy Bypass -File Tools/Build/Test-CompiledShaderClosure.ps1 `
    -Configuration Debug

powershell -ExecutionPolicy Bypass -File Tools/Build/Test-CompiledShaderClosure.ps1 `
    -Configuration Release

git diff --check
```

`Test-CompiledShaderClosure.ps1`는 이번 구현에서 추가한 뒤 `Invoke-BuildAndRegression.ps1`에도 연결한다.
현재 full regression에 없는 EffectRenderContractHarness와 PointLightFalloffContractHarness의 구성별
project build와 runner 실행도 같은 script에 추가한다. 따라서 clean pull에서 기존 exe나 stale local CSO에
의존하지 않고 위 Debug/Release full 명령 하나가 두 harness를 build한 뒤 실행해야 한다. 별도 closure 실행은
focused failure 진단이다. project/filters XML parse, 새 `FxCompile` item의 Debug/Release output도 closure
gate에 포함한다.

full script의 shader 관련 순서는 `Engine build → UpdateLib → Client build → 두 harness build/deploy
→ compiled-shader closure/hash gate → 두 harness run`으로 고정한다. closure가 실패하면 stale 또는
부분 배포된 harness exe를 실행하지 않는다. Effect harness는 Client에 비링크 ProjectReference를 두어
직접/solution 병렬 빌드에서도 Client shader producer 완료 뒤에만 CSO glob을 평가한다.

full build 뒤 focused 재실행이 필요할 때만 다음 runner를 사용한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/EffectRenderContractHarness/Run-EffectRenderContractHarness.ps1 `
    -Configuration <Debug|Release>

powershell -ExecutionPolicy Bypass -File Tools/PointLightFalloffContractHarness/Run-PointLightFalloffContractHarness.ps1 `
    -Configuration <Debug|Release>
```

최신 `main`에서 코드 수정 전에 `Sync-EffectDataProject.ps1 -Check`를 먼저 실행해 baseline을 기록한다.
현재 조사 브랜치에서는 project/filters stale로 실패하므로 최신 `main`에서도 동일하면 이번 shader item
변경으로 조용히 덮지 않고 upstream project drift를 먼저 분리한다.

## G06. 사용자 Release smoke와 PR admission

에이전트는 Client를 실행하거나 UI를 대신 조작하지 않는다. 자동 검증이 모두 끝난 뒤 사용자가 정본 실행
경로로 확인한다.

1. `Server + Client` profile 또는 Server와 `Client/Bin/Release/Client.exe`를 정본 순서로 시작한다.
2. Lobby에서 Character Select를 누른다.
3. Loading 중 창 이동/종료 입력이 반응하고 `응답 없음`이 뜨지 않는지 확인한다.
4. Level resources와 Effect phase가 함께 보이며 target/total, current ID, 상태와 경과 시간이 실제로
   갱신되는지 확인한다.
5. Character Select visual map과 character 화면으로 최종 진입하는지 확인한다.
6. Lobby로 돌아와 같은 class/revision으로 두 번째 진입했을 때 prepared target을 재준비하지 않는지 본다.
7. 같은 경로를 Debug에서도 한 번 확인해 configuration별 동작 차이가 없는지 본다.
8. 테스트용 CSO 누락/손상은 harness 전용 복사본에서만 검증하고 실제 Client output을 훼손하지 않는다.

PR에는 다음 증거를 붙인다.

- Debug/Release full regression과 EffectRenderContractHarness 결과
- Debug/Release runtime source-compiler call site 0과 CSO closure/compiled-load 결과
- Release first-entry phase별 elapsed, target/resource count와 가장 긴 main commit unit 계측
- CSO 누락/손상, stale revision, cancel과 target 실패 격리 결과
- 사용자가 직접 관찰한 Release/Debug 화면 결과와 아직 확인하지 않은 항목의 명시적 분리

사용자의 서면 확인 전에는 `visual PASS`, `응답성 PASS`, Character Select 최종 화면 PASS로 기록하지 않는다.

## 5. 구현 순서와 중간 admission gate

1. **G00 CSO contract**를 먼저 구현하고 Debug/Release shader closure와 실패 harness를 통과시킨다.
   이 단계 전에는 cancel 불가능한 Release source compiler가 남아 있으므로 worker pipeline을 붙이지 않는다.
2. **G01 request snapshot**으로 Loader 시작 전 exact target ownership을 확정하고 Loading/non-loading queue의
   동시 소비를 막는다.
3. **G02 stage/commit split**을 구현한다. 실제 계측에 따라 parse/validate와 device-only target 준비를
   worker의 완성 stage로 이동하고, main에는 exact CAS/swap/per-key commit만 남긴다.
4. **G03 progress/activation**을 연결하고 fake 90%를 제거한다.
5. **G04 cancellation/failure** harness에서 partial commit과 무한 재시도가 없는지 확인한다.
6. **G05 Debug/Release full regression**을 통과한 뒤 RESULT와 public 문서를 실제 구현 상태로 갱신한다.
7. **G06 사용자 smoke** 결과를 받아 PR의 수동 admission 상태를 확정한다.

각 gate가 실패하면 다음 G로 넘어가지 않는다. 특히 CSO contract만으로 실제 Release 멈춤이 사라져도
progress/worker ownership과 target 내부 bounded commit이 닫히기 전에는 이번 계획 전체를 완료 처리하지
않는다.
