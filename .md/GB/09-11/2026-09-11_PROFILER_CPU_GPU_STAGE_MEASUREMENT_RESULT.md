# Composition Profiler CPU·GPU 계측과 F1 패널 결과

## G00. 구현 범위

기존 Engine CProfiler를 확장하고 F1 **Open Composition Profiler**에 같은 데이터를 연결했다.
실제 C++ 호출 지점 기준 CPU 116개 이름, GPU 22개 이름을 패널에 등록했다. 이는 계측 가능한
구간 목록이며 매 프레임 138개가 모두 실행된다는 뜻은 아니다. 미관측은 `not observed / --`,
미연결 texture cache counter는 `N/A`로 표시한다. 개별 particle/bone마다 query를 발행하지 않는다.

기존 ALT V 캡처의 24프레임 CPU 평균90.230ms, Particle.Simulate30.788ms와 미분류 Update/Render
self26.576ms는 [확장 전 분석](2026-09-11_WARLORD_ALT_V_PROFILER_ANALYSIS_REPORT.md)의 수치다.
이번 작업은 그 잔여 비용을 다음 캡처에서 구분하기 위한 계측이며 JobSystem/work stealing/fiber,
animation culling이나 GPU 최적화를 구현해 FPS가 개선됐다는 결과는 아니다.

## G01. 패널과 읽는 방법

`Client/Private/ProfilerTool.cpp`와 기존 Tool owner를 확장했다. CPU/GPU 데이터를 별도 UI runtime에
복제하지 않고 Engine 집계 API로 0.5초마다 읽는다. 현재 scope 이름은 실제 호출 지점에서 추출해
사전등록하며 추후 관측된 새 scope도 집계 표에 표시한다. 이름 등록은 sample을 만들어내지 않는다.

- 상단: 최근 실제 Begin_Frame 간격/FPS, 선택 창 CPU 평균/최대, 유효한 GPU frame 평균/최대.
- CPU sections: 구간·thread·평균 ms/frame·Self ms/frame·최대 단일 call·호출/frame. 큰 비용순.
- GPU passes: 같은 CPU history 창 안에서 완전한 GPU frame을 분모로 평균·프레임 최대·P95·호출 수.
  같은 이름의 복수 호출은 프레임별 합산한다. 해당 pass가 없는 유효 프레임은 0으로 분모에 포함한다.
- Last resolved frame intervals: 실제 원래 GPU frame의 개별 begin/end/elapsed와 중첩 depth.
- Workload: 실제 draw/instance/queue/map/navigation/copy counters, animation 평가와 성공한 draw의 join,
  GPU pipeline invocation 수. 최신 CPU frame과 마지막 resolved GPU frame 번호를 분리한다.
- Long operations: 8ms 이상 완료 scope. worker가 여러 frame에 걸치면 완료 frame에 귀속된다.

Frame interval은 메시지 처리·60Hz gate 및 profiler 프레임 정리 사이의 간격도 포함한다. CpuFrameMs는
기존 Begin/End 측정 경계이며 End의 query 회수·history commit 전체를 포함하지 않는다.
CPU Avg는 자식을 포함하고 Self는 같은 thread의 자식을 제외한다. 부모와 자식을 합산하지 않는다.
GPU timestamp는 CPU 명령 공급을 기다린 시간도 포함할 수 있는 GPU 타임라인 경과 시간이며
GPU busy%, occupancy, VRAM 사용량이 아니다. CPU animation/navigation 자체에 GPU 시간이 있는
것처럼 표시하지 않는다. GPU skinning과 map draw는 해당 draw pass의 GPU 비용에 포함된다.
draw별 shader·model·material GPU 시간, exact frustum-out animation 판정은 이번 패널에 없다.

## G02. GPU 수집·표시 품질

`Profiler.h/.cpp`의 기존 8-slot frame ring 안에 frame당128개 pass begin/end query를 미리 만든다.
실제 immediate-context owner인 main thread에서만 발행하며 query 생성과 blocking GetData를
프레임 경로에 추가하지 않는다. 최소4개의 실제 frame-loop 호출 뒤 DONOTFLUSH로 조회한다.
capture frame ID와 query polling frame serial을 분리하여 Capture를 꺼도 CPU 빈 기록 없이
이미 발행한 GPU query를 계속 회수한다. 캡처 정지 직후 Save하면 아직 pending인 결과가 남을 수 있다.

Unsupported/Pending/Valid/Disjoint/Dropped/Error와 pass 지원 여부를 구분한다. 상한 초과·미완료
scope는 drop으로 남기고 부분 GPU frame은 pass 평균/P95 분모에서 제외한다. 전체 frame query만
지원되는 장치에서는 전체 GPU 평균은 유지하고 pass 미지원 상태를 표시한다. 누적 CPU/GPU/model
drop 수를 패널과 JSON에 노출한다. CPU frame당4096개 scope 제한은 유지한다.

Renderer는 Draw, Shadow, NonBlend, SSAO, Lights, SceneHDR, ScreenPosts, Bloom, Final,
DisplayOverlays, UI, Debug, Priority, Combined, NonLight, Blend 및 SceneColorCopy를 계측한다.
MainApp은 BeginFrame, Portraits, ImGui.BackendSubmit, BossShowcase, UIText의 GPU 구간을 추가한다.
SceneColor snapshot은 초기 호출뿐 아니라 occurrence refresh가 호출하는 실제 copy 함수 안에서
측정한다. SceneColorCopies는 성공한 복사 횟수이고 bytes는 RGBA16F source의 논리 payload이며
read+write 메모리 버스 트래픽이나 실제 allocation 크기가 아니다. 기존 copy 순서는 유지한다.

## G03. JSON 저장

`LostArkProfilerCapture.v3`는 기존 필드와 함께 frame interval, GPU 상태/구간/depth/offset/drop,
animation join 및 누락 수를 기록한다. immutable Snapshot만 main에서 복사하고 JSON 직렬화,
파일 쓰기, 큰 snapshot 메모리 해제는 전용 저장 worker가 수행한다. 복사 비용은
`Profiler.Capture.Snapshot`, 패널 집계 비용은 `Profiler.Panel.Refresh`로 다음 완료 frame에 남는다.

같은 디렉터리의 고유 임시 파일을 완전히 작성한 뒤 atomic replace한다. 실패·취소 시 기존 파일을
보존한다. 동시 저장은 하나이고 UI를 닫아도 작업과 결과를 보존하며 다시 열면 완료를 표시한다.
종료는 협력 취소·CancelSynchronousIo와 최대5초 wait 후 기존 Loader와 같은 process fail-fast
정책을 따른다. worker만 강제 종료하고 프로세스를 계속 실행하지 않는다.

class/skill/카메라/viewport/build identity 자동 correlation과 서버 navigation 통계의 Client 전송은
이번 schema에 없다. 서버 비용은 별도의 실제 Server RoomPerf 로그로 읽는다.

## G04. 검증과 실행 준비

GPU WARP 수치 검증: 원래 frame 귀속, 최소4frame 지연, 같은 이름의 복수 copy 합산·P95,
128개 scope 상한, partial frame 제외, worker GPU 호출 거부, disabled no-op, frame interval
최초/재개 reset, model update→성공 draw join 및16384 key 상한을 확인했다. Capture off 뒤
실제 poll-loop를 진행해 이미 발행한40개 GPU frame이 모두 회수됨을 확인했다.

실제 ProfilerCaptureIO.cpp를 별도 out object로 컴파일해 GPU 상태6종, UTF-8/escaping,
23 counters/animation drop, 동기·비동기 JSON 결과 일치, 중복 저장 거부, 잘못된 경로·NaN의
기존 파일 보존, 종료 취소와 임시 파일 정리를 확인했다. UI는 독립 코드 검토에서 발견한
frame GPU만 지원할 때 평균을 숨기는 조건을 수정했다.

검증 증거: `out/ProfilerCoverage20260911/scope_catalog_coverage.json`,
`gpu_profiler_probe_receipt.json`, `capture_io_compile.log`, `capture_io_run.log`,
`capture_io_output/capture_valid.json`, `server_nav_metrics_probe_receipt.json`.
통합 Debug Product build는 2026-09-11 19:20:48 KST에 exit0으로 완료했다. Engine/Shared/Server/Client
네 프로젝트 compile/link/deploy가 모두 PASS이며 약171초가 걸렸다. 최초 시도에서 SceneColor byte
counter의 std::max 두 곳이 Win32 매크로와 충돌한 것을 `(std::max)(...)`로 고친 뒤 재빌드했다.
기존 C4819/C4828, shader X4000, DirectXTK PDB LNK4099 경고는 남지만 최종 컴파일·링크 오류는 없다.

명령: `powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`.
결과 receipt: `out/BuildPipeline/runs/20260911T102048234Z-debug-product.json`.
최종 로그: `out/ProfilerCoverage20260911/product_build_retry.log`.
`Client/Bin/Debug/Client.exe` 53,383,680 bytes, `Server/Bin/Debug/Server.exe` 13,198,848 bytes이며
둘 다 x64 PE를 확인했다. Engine.dll 배포 원본/Client 복사본 SHA256과 Profiler SDK header가 일치한다.
근거는 `out/ProfilerCoverage20260911/product_binary_verification.json`이다. 전체 git diff --check,
World JSON과 project XML 검사도 PASS. Client/UI는 실행하거나 캡처하지 않았다.

## G05. 사용 경로와 남은 검증

Visual Studio **Server + Client** profile → **Ctrl+F5** → Lobby에서 원하는 아레나 진입 →
**F1 → Open Composition Profiler → Capture**. 동일 카메라에서 idle과 ALT V를 각각 수집하고
GPU 결과를 기다린 뒤 Save JSON으로 비교한다. F1 도구 자체 비용을 분리하려면 패널을 닫은
상태에서도 Capture를 유지하고 재생한 다음 열어 저장한다.

아직 실행하지 않은 것은 실제 게임의 새 profiler capture, 계측 오버헤드 A/B, 최적화 전후 FPS,
사용자의 Client 화면 확인이다. 빌드와 수치 검증을 실제 화면 PASS로 대신하지 않는다.

## G06. CPU 호출 구간 구현 결과

기존 CProfilerScope 경로에 54개 계측 지점, 51개 서로 다른 이름을 연결했다. 이 수에는 기존 `Effect.Particle.Simulate` 두 지점의 Spawn/Update 이름 분리가 포함된다. 새 particle/bone/node별 scope는 추가하지 않았다. 전체 이름 목록은 `out/ProfilerCoverage20260911/cpu_scope_changes.json`에 있으며 기존 scope와 함께 도구의 사전등록 목록으로 소비한다.

| 파일 | 실제 측정 구간 |
|---|---|
| `Client/Private/MainApp.cpp` | InputAndUI.Update, Network.DrainAndDispatch, Engine.Update, Presentation.Prepare, Effect.ProductGroups.Update, DebugTools.Update, LevelAndEnvironment.Update; Render.BeginFrame/Portraits/World/BossShowcase/UIText, ImGui.BuildAndSubmit/BackendSubmit |
| `Client/Private/Effect_Playback.cpp` | FixedStep 안의 전체 emitter spawn 루프와 update 루프를 Effect.Particle.Spawn/Update로 분리. 기존 FixedStep/FrameRebuild/History 유지 |
| `Client/Private/Effect_DocumentRenderer.cpp` | Effect.Material.Bind; Mesh/Rect/Decal.Render; Mesh.BindAndDraw/DrawSubmission/InstanceBuild/InstanceUpload; Sprite.DepthSort/InstanceBuild/InstanceUpload/DrawSubmission |
| `Client/Private/MapStaticBatchObject.cpp` | Map.Batch.Visibility/CullAndPack/InstanceUpload/ShadowPrepare/ShadowUpload/BindAndDraw |
| `Engine/Private/Animation.cpp` | Animation.Channels.Update/Sample 전체 channel batch |
| `Engine/Private/Model.cpp` | Animation.Play/Blend/Transition.Build/Bones.Combine/History.Sample/History.DebugVerify |
| `Engine/Private/Mesh.cpp` | Animation.SkinPalette.Build/Bind |
| `Engine/Private/Graphic_Device.cpp` | Render.Present의 실제 SwapChain.Present CPU 호출 |
| `Engine/Private/NavPathFollower.cpp` | Navigation.Request/Follow |
| `Engine/Private/Navigation.cpp` | Navigation.FindPath/Simplify/RoundCorners |
| `Engine/Private/PathFinder.cpp` | Navigation.AStar 전체 탐색 |

MainApp의 새 구간은 기존 캡처의 Update/Render self 잔여 26.576ms를 다음 캡처에서 분해하기 위한 것이다. 아직 새 캡처로 이 잔여의 실제 원인이 확정된 것은 아니다. Update scope를 붙인 뒤 GPU가 빨라졌거나 FPS가 개선됐다고 해석하지 않는다.

`Effect.Mesh.BindAndDraw`는 재질/Shader.Begin/Draw를 포함한 mesh slot 반복문이며 `DrawSubmission`은 그 안의 실제 모델 제출 호출이다. material binding과 draw submission의 CPU 시간은 GPU shader 실행 시간이 아니다. `Animation.SkinPalette.Bind`도 CShader의 CPU 상수 설정 시간이며 실제 driver upload는 뒤의 Shader.Begin에서 일어날 수 있다. `Render.Present`는 driver 대기 등을 포함할 수 있는 CPU 경과 시간이다. Map upload cache hit에는 InstanceUpload 자식 sample이 생기지 않는다.

`Animation.History.DebugVerify`는 _DEBUG에서 수행하는 두 번째 combined-bone 계산과 memcmp 검증을 분리한다. 이 sample이 Release에서 없다는 것은 정상이며, 최초 history snapshot copy는 History.Sample의 나머지 범위에 포함된다. 기존 World marker locked-axis helper의 바이트는 CPU 계측 이전과 동일하게 보존했다.

F1 launcher의 실제 label은 `Client/Private/MainApp.cpp:9315`의 `Composition Profiler`이며 기존 toolButton의 `Open ` prefix를 통해 **Open Composition Profiler**로 표시된다. 기존 DEBUG_TOOL::PROFILER 및 단일 m_ProfilerTool 인스턴스를 그대로 사용한다.

### 업데이트했지만 제출하지 않은 모델

`CModel::Play_Animation`의 실제 evaluation 본체와 `Set_AnimationTransitionPose`에 CProfilerModelAnimationScope를 연결했다. 같은 프레임의 CModel Render/RenderInstanced/RenderOrderedStaticInstanced 실제 S_OK 제출에서 Record_ModelSubmitted를 호출한다. 여러 pass의 같은 model은 profiler 내부에서 중복 제거한다. pointer는 내부 join key로만 사용하고 capture ID로 내보내지 않는다.

이 값은 **Updated, not submitted**이다. 캐릭터/NPC/Valtan에서 애니메이션 평가 여부를 확실히 결정하는 기존 camera-frustum owner가 확인되지 않아 새 culling 판정이나 skip 규칙을 만들지 않았다. hidden presentation, tool preview, draw 실패 등도 포함될 수 있으므로 화면 밖 모델 수와 같지 않다. main thread의 활성 frame 및 같은 frame epoch만 집계한다. Play_Animation과 Set_AnimationTransitionPose는 서로 중첩 호출하지 않는다. history arbitrary sampling이나 cloth 등 모든 애니메이션 작업을 합산하는 값은 아니다.

### Client navigation과 Server 권위 navigation의 구분

Client의 Navigation.Request → CNavigation::Find_Path → CPathFinder::Find_Path는 **Client 프로세스의 Engine 경로 탐색**이다. 기존 NavigationQueries/ExpandedNodes/QueryMicroseconds/PathCells counter도 이 요청에서 쓴다. `Client/Private/Character.cpp:2301`은 replicated state가 있으면 network transform을 갱신하고 else에서만 로컬 path follower를 갱신한다. 제품 이동에서 Client Navigation sample이 없어도 Server가 실제 길을 찾고 있을 수 있다.

`Network.DrainAndDispatch`는 Client의 CNetworkManager::Update CPU 구간이며 Server 연산 시간이나 왕복 지연이 아니다. Server는 `CGameRoom::Commit_MoveGoal`에서 CServerNavigation의 Find_Path/Smooth_Path를 호출하고, 이동 tick에서 Resolve_TraversalStep와 CServerCollisionSystem::Resolve_PlayerMove를 호출한다. 이번 Server 계측은 아래 별도 Server telemetry에 기록하며 Client CPU/GPU JSON 값으로 대신 표시하지 않는다.

## G07. Server navigation 실제 계측 확장

기존 서버 navigation 상태 진단과 성능 진단을 조사한 뒤 기존 CServerNavigation owner와 SERVER_ROOM_PERFORMANCE_METRICS를 확장했다. 수정한 파일은 아래 네 개다. Shared packet/Client replication 구조는 바꾸지 않았다.

- `Server/Public/ServerNavigation.h:51`: SERVER_NAVIGATION_QUERY_METRICS 및 PERFORMANCE_METRICS. 각 query 종류의 Calls, TotalNanoseconds, MaximumNanoseconds, ExpandedNodes, ReturnedPathPoints를 누적한다.
- `Server/Private/ServerNavigation.cpp:18`: 실제 query의 모든 정상/조기 반환을 감싸는 steady_clock RAII timer. 함수별 수학·탐색·성공/실패 결과는 유지한다.
- `Server/Private/ServerNavigation.cpp:80`: Get_PerformanceMetrics는 base grid와 detail region의 통계를 합친다. query timer는 region dispatch 뒤 실제 leaf에서만 시작해 같은 query를 두 번 세지 않는다.
- `Server/Public/GameRoom.h:123`, `Server/Private/GameRoom.cpp:1807`: 기존 room 성능 DTO에 Navigation snapshot을 넣고 room tick 완료 시 갱신한다. 기존 Get_PerformanceMetrics의 mutex 보호 snapshot 소비 경로를 유지한다.
- `Server/Private/GameRoom.cpp:2307`: 기존 `[RoomPerf]` 줄에 navigation 종류별 누적 값을 출력한다. 기존 log cadence/pressure/heartbeat 조건은 유지한다.

| Server stage | 본체와 의미 |
|---|---|
| FindPath | 실제 A* Find_Path. grid allocation, Resolve_Cell, 탐색, 경로 복원 및 반환까지 포함. Expanded는 중복 closed 항목을 제외하고 open에서 확정 pop한 수이며 goal pop을 포함한다. start==goal에서는 탐색 pop이 없어 0이다. |
| ReachablePath | Find_PathToReachablePointWithinRadius의 bounded BFS. Expanded는 frontier cursor에서 실제 처리한 노드 수이며 아직 처리하지 않은 발견 노드는 포함하지 않는다. |
| ProjectPoint | Project_Point와 Project_PointOnSameLevel 두 projection API의 합계. 실패 시에도 Calls와 시간은 기록한다. |
| SmoothPath | 정확한 goal/line-of-sight 검사와 string pulling, 반환 path 교체 전체. ReturnedPathPoints는 smoothing 이후 waypoint 수다. |
| TraversalStep | Resolve_TraversalStep의 ground sampling/height/support transition 검사 전체. |
| LineOfSight | 실제 Has_LineOfSight. SmoothPath와 일부 FindPath 내부에서도 호출하므로 부모와 inclusive 시간이 겹친다. |

로그 키 형식은 `Nav<Stage>Calls`, `Nav<Stage>TotalUs`, `Nav<Stage>MaxUs`, `Nav<Stage>Expanded`, `Nav<Stage>PathPoints`이다. Stage는 위 표 이름과 같다. **Load 이후 누적값**이므로 해당 줄의 tick 비용으로 읽지 않는다. 두 로그의 TotalUs/Calls 차이로 해당 구간 합계/평균을 계산할 수 있다. MaxUs는 Load 이후 최대이며 구간 최대가 아니다. 내부는 ns를 누적하고 로그에서만 정수 µs로 변환하므로 짧은 단일 query는 로그에서 0µs가 될 수 있다. PathPoints는 반환 waypoint 수 합계이며 metre 길이가 아니다. FindPath의 smoothing 이전 path 수와 SmoothPath의 이후 path 수를 따로 기록한다. path를 반환하지 않는 projection/traversal/LOS의 PathPoints와 탐색하지 않는 stage의 Expanded는 0이다.

timer는 query 경과 시간을 읽으며 preemption을 포함할 수 있다. per-node clock/scope를 넣지 않았고 A*/BFS의 필요한 expanded 정수 증가만 추가했다. 동일 stage의 여러 query를 합한 누적값이며 worker lane/개별 command correlation은 없다. m_PerformanceMetrics와 Get_PerformanceMetrics는 기존 navigation처럼 소유 room thread에서 사용한다. thread parallel navigation으로 바뀐다면 수집 상태의 동기화도 함께 바꿔야 한다.

### 기존 destruction diagnostics는 여전히 상태값

`Shared/Public/Network/PacketMessages.h:1623`의 WORLD_DESTRUCTION_RUNTIME_DIAGNOSTICS는 active wall collision count, active nav blocker region count, navigation revision, last event sequence 네 상태값이다. 이 이름의 counters는 CPU 시간이나 query 수가 아니다. `CGameRoom::Build_WorldDestructionDiagnostics`가 실제 collision/navigation owner의 현재 상태를 읽어 destruction full-sync/delta에 넣는다. `ClientReplication.cpp:456/606`은 projection 적용 성공 뒤 마지막 diagnostics를 저장하고 `Level_ValtanArena.cpp:1162` → ValtanBossTool → `MainApp.cpp:9063`의 debris 상태 표시가 소비한다. 각 Client frame에 성능 sample을 전달하는 구조가 아니다.

기존 SERVER_ROOM_PERFORMANCE_METRICS에는 전체 TickUs/TickMaxUs, snapshot encode/enqueue/session enqueue 시간과 ingress/outbound 압력 정보가 있었지만 navigation 단계 시간이 없었다. 이번 Navigation snapshot/RoomPerf 확장으로 실제 Server query 시간·작업량이 추가됐다. Server collision broad/narrow phase/sweep CPU 시간, runtime blocker transaction 및 개별 Sample_Position의 독립 시간은 이번 Server 세부 계측에 없다. 이 비용은 해당 caller 및 전체 room tick에 포함되며 Client profiler로 측정됐다고 주장하지 않는다.

## G08. CPU·Server 검증 세부

- CPU 11개 파일과 Server 4개 파일의 BOM/기존 줄바꿈 스타일을 보존했다. Server 네 파일은 bare LF 0을 확인했다. 기존 dirty 변경을 revert/stage/commit하지 않았다.
- 변경 파일 git diff --check PASS. Client/Engine 제품 전체 compile/link 결과는 통합 담당의 실제 빌드 결과를 별도로 기록한다.
- MSVC /std:c++20 /O2로 실제 ServerNavigation.cpp 단독 compile PASS.
- `out/ProfilerCoverage20260911/server_nav_metrics_probe_receipt.json`: 실제 public Load와 base/detail 8×8 navgrid를 통해 FindPath 성공2+실패1=Calls3, Expanded10, PathPoints8, detail region 중복계수 없음, BFS Expanded2/PathPoints2, ProjectPoint2/SmoothPath1/TraversalStep1/LOS3, Load 재호출 시 통계 reset을 검증했다. query total 18,500ns는 synthetic 입력 검증의 실행 수치이며 실제 게임 성능이 아니다. probe는 out 임시 검증물이고 별도 제품 runtime이나 permanent harness 파일을 추가하지 않았다.
- 계측 이후의 게임 캡처/FPS와 Client 화면 검증은 아직 이 하위 작업에서 수행하지 않았다. Client나 UI를 실행하거나 스크린샷을 생성하지 않았다.
- CPU scope frame당4096 제한은 남아 있다. scope를 particle/bone/node마다 넣지는 않았지만 effect occurrence/map batch/model 수가 많으면 sample이 떨어질 수 있어 dropped count를 함께 확인해야 한다. no sample은 지원되지 않는 구간/호출되지 않은 구간/수집 탈락을 확인하기 전까지 0ms로 확정하지 않는다.
