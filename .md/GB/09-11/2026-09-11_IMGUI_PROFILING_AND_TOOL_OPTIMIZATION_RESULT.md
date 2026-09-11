# ImGui 툴 계측·최적화와 빌드 오류 수정 결과

## G00. 반영 범위와 판단

기존 Engine CProfiler를 확장해 F1 각 툴의 CPU 처리와 ImGui backend 업로드·제출·별도 창 Present를 구분했다. F1 → `Open Composition Profiler`의 첫 번째 `ImGui` 탭과 JSON이 같은 계측을 소비한다. 새 profiler나 렌더러를 만들지 않았다.

이번 범위는 ImGui 계측, 확인된 중복 작업 제거, 사용 종료 보스 튜닝 패널 제거, 관련 컴파일 오류 수정이다. 기존 맵·애니메이션·파티클·navigation 계측은 유지한다. JobSystem/fiber/work-stealing deque 도입, 애니메이션 culling 정책 변경, GPU particle 전환은 이번 변경에 포함하지 않았다.

후속 F1 Action Workbench 첫 열기 assertion 조사에서 확인한 WORLD lane 배열 범위 결함과
수정은 G07에 기록한다. G01~G06의 계측 개수·빌드 인계는 앞선 ImGui 작업 시점의 기록이며,
후속 소스 전체가 그 빌드로 검증됐다는 의미가 아니다. 최신 통합 Debug Product 빌드는
2026-09-11 23:23:36 KST에 Engine/Shared/Server/Client 모두 PASS했고,
Workbench를 다시 여는 사용자 화면 확인은 **USER_PENDING**이다.
후속 Profiler 이름 등록 cache와 Capture 수집/패널 비용 비교는 G08~G09를 따른다.
사용자 결정은 버그 수정 EXE 직접 검증이 먼저다. PR merge/main sync와 외부 리소스 공유는
그 이후로 보류하며, 로컬 `Desktop/GB_Resources` 생성·검증을 외부 공유 완료로 기록하지 않는다.

다른 PC의 새 캡처가 없으므로 프레임 급락의 주원인과 개선 FPS는 아직 확정하지 않았다. ImGui API 자체는 widget마다 GPU draw를 수행하는 방식이 아니라 vertex/index와 묶인 draw command를 만든다. 실제 비용은 툴 데이터 처리, layout, 업로드·draw, OS 창과 Present 대기로 분리해서 읽어야 한다. [Dear ImGui 공식 설명](https://github.com/ocornut/imgui#how-it-works).

## G01. 현재 측정하는 부분

`Client/Private/MainApp.cpp`의 실제 각 툴 Build/Update, F1 hub 세부 패널, 도구 초기화와 구형 Profiler overlay/details에 CPU scope를 연결했다. `MainApp_SequenceViewer.cpp`도 Update/Build/Refresh를 구분한다. 툴이 보이지 않아도 필요한 재생·저장 작업은 기존 실행 조건을 유지한다.

`Engine/Private/ImGuiLayer.cpp`는 DX11/Win32/core NewFrame, drawdata 완성, platform update/render를 측정한다. DX11 backend는 다음을 측정한다.

| 구분 | 실제 계측 |
|---|---|
| CPU | Buffer 성장, Map, vertex/index/constant 업로드, D3D state backup/setup/restore, DrawIndexed 제출, texture update, device object 생성 |
| GPU timestamp | 실제 `ImGui.RenderDrawData`, 별도 platform viewport render 및 Present 구간 |
| 작업량 18종 | draw list, vertex, index, command, draw call, callback, render window, active window, viewport 수; vertex/index/constant/texture 업로드 bytes; buffer growth, texture create/update, device object build, Map 실패 |

기존 counter 23개 뒤에 18개를 추가해 총 41개이며 기존 순서를 유지한다. JSON은 `LostArkProfilerCapture.v3`에 additive field를 추가했다. GPU scope는 여러 viewport의 실제 호출을 합산하며 메인 drawdata를 별도로 중복 집계하지 않는다.

Composition Profiler의 정적 목록은 현재 소스의 이름 기준 CPU 183개, GPU 25개다. 실제 관측되지 않은 구간도 목록에서 찾을 수 있으며 미관측을 0ms로 표시하지 않는다. 동적 이름은 실제 수집된 scope registry로 표시한다. 정적 이름 목록은 프로그램의 모든 함수·외부 라이브러리·OS 동작이 개별 계측됐다는 의미가 아니다.

## G02. 제거한 낭비

| 변경 위치 | 적용 내용 | 확인된 범위 |
|---|---|---|
| Effect_Tool.cpp | family마다 반복 count/filter하던 목록을 프레임당 한 번 분류 | 230개 요소 문서에서 분류 predicate 호출 2,990~4,140회 → 230회; 원본 family/ID 순서 동일 |
| EffectAuthoringSequencer_Timeline.cpp | 화면 밖 box와 hit widget 제출 생략 | 활성 drag와 전체 scroll 영역 유지, border margin 반영 |
| ProfilerTool.cpp | CPU 행 목록을 갱신·필터 변경 때 재구성하고 clipper 적용 | 매 행 filter 문자열 할당 제거, 행 높이와 cache pointer 수명 리뷰 |
| MainApp.cpp / ProfilerTool | 구형 Details의 동기 JSON 저장을 기존 비동기 exporter로 통합 | 중복 저장 차단, 창을 닫아도 Update에서 완료 회수 |
| imgui_impl_dx11.cpp | 빈 drawdata에서 buffer 업로드와 state 작업 생략 | 필요한 texture create/update와 callback-only 목록은 보존 |
| imgui_impl_dx11.cpp | index buffer Map 실패 시 이미 Map한 vertex buffer 해제 | 실패 정리 경로 보완 |
| ImGuiLayer.cpp | main viewport만 있으면 platform render용 state 왕복 생략 | platform window 생성·파괴 update는 유지 |
| Engine.vcxproj | Debug x64 ImGui core 4개와 backend 2개만 /O2 /Zi 사용 | /MDd, _DEBUG, assert 유지. 다른 69개 compile item과 Release metadata 동일 |

마지막 항목은 ImGui 내부 코드의 디버깅·stepping이 최적화된 코드 기준으로 바뀌는 tradeoff가 있다. 위의 호출 수 감소와 빌드 옵션 변경을 실게임 FPS 개선 측정으로 대신하지 않는다.

## G03. 사용 종료 보스 위치·크기 튜닝 패널

사용자 설명에 따라 `RenderKoukuSaydonBossTuningControls`, 단독 caller, 전용 load/save/JSON patch helper와 상태를 제거했다. 기존 관문 선택·Server spawn/despawn·Bingo·Complete Play, CNpc 표현 코드와 저장된 NPC 수치는 유지한다.

삭제한 패널은 매 프레임 파일을 읽지는 않았다. 파일 읽기는 최초 열기/Reload/Save에서만 있었다. 평상시에는 프레임당 WorldEntities 선형 탐색 6회와 변화 없는 scale/yaw/offset/weapon setter가 발생했고, 일부 setter는 collider 또는 회전행렬을 다시 계산했다. 이 비용과 패널 widget 처리가 사라졌다. 이전 캡처에 해당 패널 단독 시간이 없어 몇 FPS가 개선되는지는 확정할 수 없다.

## G04. 컴파일 오류 원인과 수정

사용자가 보고한 `ImGuiVertexUploadBytes`는 `Engine/Public/Profiler.h`의 enum에 정상 선언돼 있었지만, 조사 당시 `EngineSDK/Inc/Profiler.h`에는 신규 counter 18개가 없는 구버전이 남아 있었다. Client는 이 SDK 헤더를 사용한다. 이 상태가 사용자 진단의 근거 있는 원인이며, 해당 선언 오류 자체가 이번 빌드 로그에서 재현된 것은 아니다.

실제 Product Debug 빌드에서 재현된 오류는 DX11 backend의 `IM_NEW` 세 곳에서 발생한 C2226 `ImNewWrapper`였다. 새 `Profiler.h` include가 `Engine_Defines.h`의 `_DEBUG`용 `new` 매크로를 유입시켜 ImGui placement-new 문법을 깨뜨렸다. backend의 Profiler include 전후에 `push_macro("new")` / `pop_macro("new")`를 적용해 기존 매크로 상태를 복원했다. Engine Debug 할당 정책을 전역 변경하지 않았다.

수정 뒤 Engine 빌드와 SDK 복사가 완료됐고 원본/SDK Profiler 헤더의 SHA256이 일치함을 확인했다. 선행 standalone backend 수치 probe는 비-Debug 구성이라 이 `_DEBUG` 전용 결함을 잡지 못했다. 실제 Debug Engine 빌드가 이 수정의 컴파일 증거다. 최종 Client EXE 빌드는 아래 사용자 인계 상태와 구분한다.

## G05. 자동 검증 증거

실행한 구조·수치 검사만 기록한다. `out/`은 Git 제외 진단 산출물이다.

| 검사 | 결과 / 증거 |
|---|---|
| 실제 Engine Profiler + DX11 backend headless WARP | 6 capture/6 GPU frame, 3 draw call·6 callback, vertex/index/constant 180/18/384 bytes; 빈 drawdata texture 갱신, callback-only, null hook, viewport state restore PASS. `out/ImGuiProfiler20260911/imgui_backend_probe_receipt.json` |
| JSON 실제 C++ sync/async exporter | 41 counter의 이름·값·순서, zero 보존, 동기/비동기 일치, Unicode, 단일 저장, 실패 시 기존 파일 보존, 취소·temp 정리 PASS. `out/ProfilerCoverage20260911/capture_io_run.log` |
| Effect 목록 | World 4/12개, Warlord 230/186개 문서의 family/ID 순서 PASS. `out/ImGuiToolPerformance20260911/family_order_check.json` |
| 변경 Effect 툴 2개 translation unit | Debug compile PASS. `out/ImGuiToolPerformance20260911/compile.log`, `timeline_compile.log` |
| 최종 Client 변경 3개 translation unit | 현재 SDK로 ProfilerTool.cpp, MainApp.cpp, MainApp_SequenceViewer.cpp Debug `/c` compile PASS, 19:56:13 KST. `out/ImGuiOptimization20260911/final_cpp_compile/compile.log`, `compile.rsp`. 고유 out/ object만 생성했고 EXE link 검증은 아님 |
| MainApp / Profiler 독립 리뷰 | scope wrapper 제어 흐름, CPU cache·clipper 수명 PASS; 숨김 상태 저장 Poll 결함 수정 재확인. `out/ImGuiOptimization20260911/main_and_profiler_readonly_review.json` |
| ImGui compile 설정 | MSBuild 실제 item metadata와 XML parse PASS. `out/ImGuiOptimization20260911/imgui_optimization_metadata_receipt.json` |
| 정적 scope 목록 | CPU 183 / GPU 25 이름 목록. `out/ImGuiOptimization20260911/scope_catalog_coverage.json` |
| 보스 튜닝 제거 | 삭제 경계, 전용 helper 참조, 유지할 caller와 인코딩 보존 PASS. `out/ImGuiOptimization20260911/boss_tuning_removal_receipt.json` |

### Product Debug 빌드와 사용자 인계

`powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`를 실행했다. 수정 후 Engine.dll, Shared.lib, Server.exe compile/link는 성공했다. ImGui 여섯 파일의 실제 CL 명령도 `/O2 /Zi /MDd`, RTC/JMC 없음으로 확인했다. SDK 헤더와 원본이 일치하며 `ImGuiVertexUploadBytes`가 양쪽에 존재한다. 증거는 `out/ImGuiOptimization20260911/debug_sdk_and_compiler_receipt.json`이다.

사용자가 EXE를 직접 빌드하겠다고 후속 요청했고, 실제 Visual Studio 18의 Framework.sln 빌드와 에이전트 Product 빌드가 같은 `Shader_VtxMeshBinary.cso`를 동시에 생성 중임을 process parent/command line으로 확인했다. 19:55:35 KST에 에이전트 소유 Product 프로세스 트리만 종료하고 사용자 devenv/MSBuild/x64 FXC는 유지했다. Client/UI는 실행하지 않았다.

따라서 이번 최종 상태는 **Engine/Shared/Server 빌드 PASS, Client 전체 compile/link는 사용자 Visual Studio 빌드에 인계**다. 통합 Product PASS나 새 Client.exe 생성 완료로 기록하지 않는다. 이때 남아 있던 Client.exe의 이전 timestamp는 최신 소스 빌드 증거가 아니다. 부분 빌드 로그는 `out/ImGuiOptimization20260911/product_build_fixed.log`, 인계 기록은 `product_build_handoff_receipt.json`이다.

인계 후 수정한 Client 세 파일을 고유 `out/` object 경로로 별도 Debug 컴파일해 모두 통과했다. 실제 Engine SDK include를 사용했으며 제품 object/PDB/EXE와 사용자의 빌드 프로세스는 변경하지 않았다. 최종 `git diff --check`와 변경 프로젝트 XML parse도 통과했다. 기존 셰이더·인코딩 경고는 남아 있으므로 warning-free 빌드로 표현하지 않는다.

## G06. 실행·성능 확인 경계

Client/UI를 자율 실행·조작하거나 화면 캡처하지 않았다. detached OS 창의 실사용 Present, 실제 adapter의 GPU 비용, 화면 밖 타임라인 drag 감각과 다른 팀 PC의 FPS는 사용자 실행에서 확인해야 한다.

Debug x64의 `Server + Client` profile을 Ctrl+F5로 시작하고 F1 → `Open Composition Profiler` → `ImGui`를 사용한다. 같은 장면에서 Capture를 켠 채 F1 닫힘, F1만 열림, 문제가 생기는 툴 열림, 툴을 별도 창으로 분리한 경우를 각각 수집하면 비교할 수 있다. `Save JSON`은 `Client/Bin/ProfilerCaptures`에 저장한다. 저장 중 툴을 닫아도 완료 상태를 회수한다.

CPU Self는 자식 구간을 제외한 비용이다. GPU timestamp는 해당 구간의 경과 시간이며 GPU utilization/VRAM/ALU·bandwidth 병목 카운터가 아니다. CPU와 GPU를 단순 합산하거나 GPU 구간 전체를 GPU가 일한 시간으로 단정하지 않는다. Map/GPU/Present 대기, 다른 프로세스 및 드라이버 영향을 구분해야 한다. Server navigation은 별도 Server `[RoomPerf]`의 Nav 필드로 확인한다.

현재 남은 우선순위는 다른 PC 캡처에서 Tool Build, DX11 업로드·제출, 별도 창 Present 중 실제 큰 구간을 고르는 것이다. 워로드 ALT_V의 이전 particle CPU 병목과 미분류 CPU 시간에 대한 판단은 대응 profiler 분석 보고서를 따른다. work-stealing 자체가 Render 제출이나 GPU/Present 대기를 병렬화하지는 않으므로 새 수치 없이 대규모 스케줄러를 해결책으로 확정하지 않았다.

## G07. Action Workbench 첫 열기 배열 범위 수정

사용자는 F1 Action Workbench를 열자 assertion으로 종료됐다고 보고했다. 현재 기본
`CSequencerTool` 세션은 VALTAN이고, `Begin_WorkbenchFrame` 이후 Sequencer pane이
`CValtanActionWorkbench::Render_Timeline`을 호출한다. 소스에서 확인한 확정 결함은
`TIMELINE_LANE`/표시 순서에는 WORLD까지 8개 lane이 있는데
`m_TimelineLaneSubrowCounts`는 여전히 `std::array<size_t,7>`이었다는 점이다.
WORLD의 index 7 조회는 범위 밖이며 MSVC Debug checked array에서는 assertion을 일으킨다.
같은 cache 크기를 사용하던 `Pack_TimelineSubrows`도 WORLD lane을 누락했다.

`ValtanActionWorkbench.h`는 cache 크기를 `TIMELINE_LANE::COUNT`로 정하고,
`ValtanActionWorkbench.cpp`는 표시 순서 배열의 실제 항목 수와 COUNT를 static_assert로
대조한다. cache는 표시 ordinal 대신 `eLane` 값으로 조회하며 배경색 순서에만 ordinal을 유지한다.
assertion을 끄거나 빈 ImGui ID 문제로 추측해 덮어쓰지 않았다. 이전 gotchas의 빈 draft
Selectable ID 보정은 현 소스에 이미 있고 이번 WORLD 배열 결함과 별개다.

git blame에서 고정 크기 7은 `8769ebab7`(08-31), WORLD 표시 추가는
`359412c4`(09-11 13:56)에 확인됐다. 사용자 assertion expression/file/line 또는 stack trace는
조사 시점에 제공되지 않았으므로 **실제 오류 대화상자와 동일한 assertion이라는 확정은 하지
않는다**. 첫 타임라인 표시에서 도달하는 확정 out-of-bounds를 수정한 것이다.

실제 ValtanActionWorkbench.cpp, ValtanActionWorkbench_Blueprint.cpp, SequencerTool.cpp의
out 전용 Debug focused compile과 diff-check가 통과했다. CPU probe는 현 public enum,
cache/표시 순서 선언과 실제 `Pack_TimelineSubrows` 본문을 사용하고 font width만 고정 fixture로
바꿨다. ImGui context/frame/draw는 생성하지 않았다.

- lane 8개/cache 8개, 빈 lane의 1 row 유지.
- WORLD의 겹친 box 3개가 3 rows로 배치되고 다른 lane의 겹친 box 2개가 2 rows로 배치.
- 입력 순서를 뒤집어도 row count 유지, 이전 index 7/size 7 불일치 확인.

근거는 `out/ActionWorkbenchAssert20260911/run.log`, `compile.log`,
`production_pack.inl`, `production_order.inl`, `production_cache.inl`이다. 원래 결함을 확인하려고
assertion 대화상자를 일부러 실행하지 않았고 Client/UI 실행·조작·캡처도 하지 않았다.
최신 통합 Debug Product 빌드는 2026-09-11 23:23:36 KST에 완료됐다. 사용자가 F1 → Action Workbench를 다시
열어 종료 없이 timeline이 표시되는지 확인해야 하며, CPU packing 통과를 첫 열기 visual PASS로
기록하지 않는다.

## G08. Profiler 이름 등록의 thread-local cache

`CProfiler::Intern_Name`은 함수 내부에서만 초기화되는 thread-local 캐시에 현재 Profiler instance ID와 최대512개의 소유 문자열→ID를 보관한다. transparent `std::string_view` 조회의 hit는 임시 문자열 할당과 공용 mutex를 생략한다. 새 이름, 다른 instance로 전환, 상한 초과 뒤의 miss는 기존 canonical 이름 표에서 ID를 확인한다. `Reset_History`는 이름 표와 ID를 보존한다. constructor가 발급하는 monotonic uint64 instance ID를 사용하므로 같은 주소에 새 Profiler를 생성해도 이전 ID를 쓰지 않는다. 기존 `End_Scope`의 완료 sample 등록 mutex는 유지했으며 완전한 lock-free profiler로 바꾼 것은 아니다.

실제 변경 전/후 `Profiler.h/.cpp` 전체를 같은 out CPU probe로 컴파일해 각각14개 검사를 통과했다. 이름 순서·반복·임시 문자열·empty/embedded-NUL, nested depth, 4개 thread의800개 sample, Reset·Capture off/on, 600개 이름의 cache eviction, 같은 주소의 새 instance를 확인했다. 다른 thread가 실제 profiler mutex를 잡은 중에도 최적화 후의 warmed lookup은 완료했다.

| 동일 x64 `/MDd /Od`, 5회 중앙값 | 변경 전 | 변경 후 |
|---|---:|---:|
| 반복 이름 등록 | 545.727 ns/call | 242.861 ns/call |
| Begin_Scope + End_Scope | 637.642 ns/pair | 317.397 ns/pair |
| 긴 이름10,000회 warmed 등록의 Debug CRT 할당 | 20,000 | 0 |

이 값은 CPU microbenchmark이며 실제 FPS가2배가 됐다는 결과가 아니다. cold load, GPU/Present 대기, thread 경합을 포함한 게임 프레임은 별도 비교가 필요하다. 실제 Profiler.cpp와 최신 ProfilerTool.cpp의 고유 out focused `/c`는 PASS다. constructor/member 변경으로 Engine ABI가 바뀌어 필요했던 Product rebuild/deploy도 최신 통합 Debug 빌드에서 완료했다. 근거는 `out/ProfilerScopeCache20260911/result.md`, `result.json`, baseline/after의 `run.log`다.

후속 Workbench·Profiler 변경을 포함한 통합 receipt는
`out/BuildPipeline/runs/20260911T142336444Z-debug-product.json`이다. 23:23:36 KST에
Engine/Shared/Server/Client 모두 PASS, 총 867183ms, `missingRuntimeInputs=[]`를 기록했다.
Engine 원본/Client 배포 DLL SHA256은
`E59B1122866CFBBA0DB633033337960765F6C3E4370134A7878808010A3AABE4`로 같고,
Profiler 원본/SDK header도 일치 확인됐다. 사용자 화면과 실게임 FPS는 USER_PENDING이며
PR merge/main sync·외부 리소스 공유 보류는 유지한다.

## G09. Capture와 패널 비용을 구분하는 사용자 수동 비교

같은 장면·카메라·해상도·스킬 동작과 동일 길이 구간을 사용하고 첫 로드/준비가 끝난 뒤 아래 네 상태를 비교한다. 에이전트는 Client/UI를 실행하거나 조작하지 않았다.

| Capture | F1/Profiler 패널 | 구분할 비용 |
|---|---|---|
| off | 닫힘 | 계측 수집과 패널을 뺀 기준 |
| on | 닫힘 | scope/counter/query 수집의 추가 비용 |
| on | 열림 | 집계·표시·ImGui 제출의 추가 비용 |
| off | 열림 | 새 수집 없이 패널을 표시하는 비용 |

Capture off에서는 Profiler history와 최근 CPU/FPS가 갱신되지 않으므로 남아 있는 패널 값이나 이전 JSON을 off 구간의 새 측정값으로 사용하지 않는다. off/on의 프레임 비교에는 사용자가 별도로 기록한 독립 프레임 간격/FPS가 필요하며, 그 값이 없으면 off 구간의 개선율은 미측정으로 남긴다. Capture를 끈 직후에는 이미 발행한 GPU query의 회수가 남을 수 있으므로 정지 직후의 과도 구간을 제외한다. Capture on 두 상태는 기록을 수집한 뒤 GPU pending이 해소된 구간과 drop 수를 확인해 비교하며, Save/Snapshot 순간은 비교 구간에서 제외한다. CPU Self와 GPU elapsed를 단순 합산하지 않는다.
