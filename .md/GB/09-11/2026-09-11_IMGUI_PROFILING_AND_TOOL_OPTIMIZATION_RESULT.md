# ImGui 툴 계측·최적화와 빌드 오류 수정 결과

## G00. 반영 범위와 판단

기존 Engine CProfiler를 확장해 F1 각 툴의 CPU 처리와 ImGui backend 업로드·제출·별도 창 Present를 구분했다. F1 → `Open Composition Profiler`의 첫 번째 `ImGui` 탭과 JSON이 같은 계측을 소비한다. 새 profiler나 렌더러를 만들지 않았다.

이번 범위는 ImGui 계측, 확인된 중복 작업 제거, 사용 종료 보스 튜닝 패널 제거, 관련 컴파일 오류 수정이다. 기존 맵·애니메이션·파티클·navigation 계측은 유지한다. JobSystem/fiber/work-stealing deque 도입, 애니메이션 culling 정책 변경, GPU particle 전환은 이번 변경에 포함하지 않았다.

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
