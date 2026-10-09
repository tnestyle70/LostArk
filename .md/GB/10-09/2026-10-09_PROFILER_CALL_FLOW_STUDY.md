# Profiler 호출 흐름과 성능 수치 읽기

2026년 10월 9일 현재 코드와 기존 저장 캡처를 읽은 학습 자료다. 제품 코드를 변경하거나 Client를 실행해 새 성능을 측정한 결과는 아니다. 아래 원본 링크의 줄 번호는 작성 당시 기준이며 한국어 주석 사본의 줄 번호와 다르다.

## G00 먼저 구분할 세 가지

**Profiler는 어디에서 시간이 걸렸는지 기록하고, 최적화 코드는 실제 일을 줄이며, Benchmark는 조건을 맞춰 전후를 비교한다.** Profiler를 구현했다는 사실만으로 FPS 개선을 증명하지 않는다. 현재 제품은 외부 Google profiler를 호출하는 경로가 아니라 `CProfiler`의 Windows QPC·D3D11 query 계측과 `CProfilerTool`의 표시·저장 경로를 사용한다.

이 Profiler는 코드에 직접 넣은 scope를 재는 계측형이다. 모든 함수의 call stack을 자동 수집하는 sampling profiler도, 락 순환을 추적하는 deadlock detector도 아니다. `Long operations`는 8ms 이상 걸린 **완료된** scope를 남긴다. 데드락으로 끝나지 않은 scope의 원인이나 락 소유자를 이 표만으로 알아낼 수 없다. Chase–Lev deque나 job scheduling은 일을 나누는 다른 계층이고 Profiler는 그 일이 실행되는 구간에 붙는 관측 도구다.

읽기 순서는 다음과 같다.

| 읽을 파일 | 소유하는 책임 | 먼저 볼 함수 |
|---|---|---|
| [Client.cpp 원본](C:/Users/tnest/Desktop/LostArk/Client/Default/Client.cpp:374) | 프레임 시작과 끝 | 메인 루프의 `Begin_Frame → Update → Render → End_Frame` |
| [Profiler.h 원본](C:/Users/tnest/Desktop/LostArk/Engine/Public/Profiler.h:301) | 표본·프레임·query·집계 계약 | `FProfilerFrame`, `CProfilerScope`, `CProfilerGpuScope` |
| [Profiler.cpp 원본](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:103) | 시간 수집과 보관 | `Begin_Frame`, `End_Scope`, `Resolve_GpuFrames` |
| [Renderer.cpp 원본](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:890) | 실제 렌더 pass의 계측 지점 | `Draw` 안의 `Render.Shadow`, `Render.NonBlend` 등 |
| [ProfilerTool.cpp 원본](C:/Users/tnest/Desktop/LostArk/Client/Private/ProfilerTool.cpp:664) | F7 표·그래프·저장 요청 | `Refresh`, `Render`, `Request_Save` |
| [ProfilerCaptureIO.cpp 원본](C:/Users/tnest/Desktop/LostArk/Client/Private/ProfilerCaptureIO.cpp:821) | 불변 snapshot의 JSON 저장 | `BeginSave`, `Make_NamedPath` |
| [RenderingBenchmark.cpp 원본](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingBenchmark.cpp:703) | 조건과 구간을 통제한 A/B | `Prepare_OptimizationPair → Begin → Update → Finalize` |

주석을 보면서 읽으려면 [Profiler.h 학습 사본](C:/Users/tnest/Desktop/LostArk/Study/CodeWalkthrough/Reference/Engine/Public/Profiler.h:445), [Profiler.cpp 학습 사본](C:/Users/tnest/Desktop/LostArk/Study/CodeWalkthrough/Reference/Engine/Private/Profiler.cpp:107)을 연다. 원본 코드를 생략하지 않고 설명 주석만 추가·번역했다. 제품의 컴파일 입력으로 대체하는 파일이 아니다. [검증 manifest](C:/Users/tnest/Desktop/LostArk/Study/CodeWalkthrough/profiler-copy-manifest.json)에 원본·사본 SHA256, 줄 수, 주석 외 토큰 동일성을 기록했다.

## G01 H에서 알아야 할 상태와 단위

`FProfilerScopeSample`은 CPU scope 하나다. `NameId`는 이름을 중복 저장하지 않기 위한 ID, `ThreadId`는 실제 실행 스레드, `Depth`는 그 스레드 안의 중첩 깊이다. 시작과 끝은 ms가 아니라 QPC tick이다. `FProfilerGpuScopeSample`은 GPU query의 시작·끝을 해당 GPU 프레임 시작 기준 ms로 변환한 표본이다. CPU QPC와 GPU timestamp의 절대 원점은 다르다.

`FProfilerFrame`은 CPU 값과 GPU 값의 도착 시점이 다른 봉투다. CPU 값은 이번 `End_Frame`에서 확정하지만 GPU는 몇 프레임 뒤에 결과가 생긴다. `GpuValid=false`인 상태에서 `GpuFrameMs`의 초기값 0을 읽으면 잘못된 결론을 낸다. `GpuStatus`의 `Unsupported`, `Pending`, `Valid`, `Disjoint`, `Dropped`, `Error`를 함께 본다. [원본 구조](C:/Users/tnest/Desktop/LostArk/Engine/Public/Profiler.h:202)

| 상태 | 필요한 이유 | 소유·접근 경계 |
|---|---|---|
| `m_Enabled / m_Collecting` | UI의 요청과 이번 프레임에 실제 적용한 상태를 분리 | 요청은 atomic, `Begin_Frame`에서 확정 |
| `m_CaptureEpoch` | Reset 이전에 시작한 worker 표본을 새 캡처에서 제외 | 시작 때 저장하고 종료 때 재검사 |
| `t_OpenScopes` | 스레드마다 다른 중첩 관계 유지 | TLS, 시작한 스레드에서 같은 token을 종료 |
| `m_PendingScopes` | 각 스레드에서 완료한 CPU scope를 프레임에 전달 | `m_Mutex`로 보호 |
| `m_CurrentFrame / m_History` | 수집 중 프레임과 완료 기록을 분리 | 메인 소유, history 접근은 잠금 |
| `FGpuQuerySlot` | CPU를 멈추지 않고 GPU 결과를 나중에 읽기 | immediate-context 메인 스레드 |
| `FrameNumber / SubmittedPollFrame` | 원래 제출 프레임과 실제 루프 경계를 구분 | Capture 정지 중에도 poll 번호 증가 |

`EProfilerCounter`의 draw·indices·bytes·cache hit는 시간과 별개다. 생산자에서 값을 실제로 증가시켰는지도 확인해야 한다. `EProfilerWork`는 상세 raw scope를 끄더라도 자주 호출되는 메인 작업의 고정 enum별 호출 수와 경과 시간을 누적하는 경로다. 그 시간도 부모와 자식이 겹칠 수 있다. [Begin_Work](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:440)

## G02 한 프레임을 호출 순서대로 따라가기

```text
Client 메인 루프
  Windows 메시지 처리 / resize / 최소화 대기
  Begin_Frame
    Capture·Reset·상세 계측 요청 확정
    CPU 시작 QPC / frame interval 계산
    저빈도 메모리 관측 / GPU 시작 query 제출
  CPU scope Client.Update
    MainApp::Update → Engine update와 게임·입력·복제 처리
  CPU scope Client.Render
    MainApp::Render
      Render.BeginFrame
      Renderer::Draw의 pass별 CPU scope + GPU scope
      ImGui 등 도구 제출
      Graphic_Device::Present → swapChain.Present(0, 0)
  End_Frame
    CPU 끝 QPC → 완료 CPU 표본 수거
    GPU 끝 query 제출 → history에 commit
    이전 프레임 GPU query 회수
  사용자 FPS 제한
```

이 순서의 출발점은 [Client.cpp:374](C:/Users/tnest/Desktop/LostArk/Client/Default/Client.cpp:374)다. `Timer_60`이라는 기존 이름이 남아 있지만 현재 루프를 항상 60Hz로 제한하는 코드라는 뜻은 아니다. FPS 제한은 `End_Frame` 뒤의 `Limit_FrameRate`에 있다.

[Begin_Frame](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:103)은 UI 요청을 프레임 경계에 적용한다. 중간부터 Capture를 켜 부모 없이 자식만 기록하는 일을 줄인다. 수집을 다시 시작할 때 이전 시작·끝 tick을 비우므로 Capture 정지 기간이 첫 interval에 섞이지 않는다.

[GameInstance update](C:/Users/tnest/Desktop/LostArk/Engine/Private/GameInstance.cpp:212)에서는 `Engine.PriorityUpdate`, `Engine.Camera.Update`, `Engine.ObjectUpdate`, `Engine.Physics`, `Engine.PostPhysicsUpdate`, `Engine.LevelUpdate`, `Engine.LateUpdate`로 비용을 나눈다. [Renderer pass](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:914)에는 동일한 `Render.Shadow` 이름의 CPU scope와 GPU scope를 나란히 둔다. 같은 이름이어도 하나는 CPU 쪽 함수 경과 시간, 다른 하나는 GPU 명령열의 timestamp 구간이다.

[End_Frame](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:157)은 먼저 CPU 끝 tick을 찍는다. 그 뒤 수행하는 완료 표본 이동·history commit·GPU 결과 회수 전체가 `CpuFrameMs`에 포함되는 것은 아니다. 그 일부 비용은 다음 시작까지의 gap에 나타난다. 따라서 CPU frame 시간과 실제 프레임 간격을 둘 다 저장한다.

## G03 CPU ms, frame interval, Present를 구분하기

[Query_Tick](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:1009)은 `QueryPerformanceCounter`를 호출한다. 초기화 때 얻은 QPC 주파수로 변환한다.

```text
CPU scope ms = (EndTick - BeginTick) × 1000 / TicksPerSecond
CpuFrameMs[N] = End[N] - Begin[N]을 ms로 변환
FrameIntervalMs[N] = Begin[N] - Begin[N-1]을 ms로 변환
FrameGapMs[N] = Begin[N] - End[N-1]을 ms로 변환
FrameIntervalMs[N] = PreviousCpuFrameMs[N] + FrameGapMs[N]
```

마지막 식의 CPU는 **이전 프레임**이다. 이번 `CpuFrameMs[N] + FrameGapMs[N]`가 아니다. 초기화 직후 첫 interval 0은 이전 시작점이 없다는 표시이므로 처리율 계산에서 제외한다. 처리율은 같은 구간에서 `1000 / 평균 interval(ms)`로 설명하고, 프레임별 순간 FPS의 산술평균과 섞지 않는다.

QPC는 구간 안의 경과 시간을 잰다. CPU가 명령어를 실행한 시간뿐 아니라 lock 대기, 동기 파일 I/O, driver 대기도 들어갈 수 있다. `[CPU] 20ms`라는 값만으로 “CPU 연산을 20ms 했다”라고 말하지 않는다.

[Graphic_Device::Present](C:/Users/tnest/Desktop/LostArk/Engine/Private/Graphic_Device.cpp:185)는 `Render.Present` CPU scope 안에서 `Present(0, 0)`을 호출한다. 여기서 12ms가 나왔다면 화면 제출 함수가 돌아올 때까지 12ms가 지났다는 뜻이다. 명시적 sync interval이 0이어도 그 사실만으로 driver·GPU queue·표시 계층의 대기가 없다고 결론 내릴 수 없다. `CpuFrameMs - Present`는 Present 밖의 경과 시간을 설명하는 보조값이며 순수 CPU 사용시간은 아니다. 다른 대기도 남아 있을 수 있다.

`FrameGapMs`는 message 처리·FPS 제한·최소화 대기·프레임 정리 등 Profiler 프레임 바깥 비용이 포함될 수 있다. 파일을 읽은 함수나 특정 wait를 별도로 계측하지 않았다면 gap 전체를 어느 한 함수의 비용으로 확정하지 않는다.

## G04 CPU scope의 수집과 inclusive/self

CPU RAII인 [CProfilerScope](C:/Users/tnest/Desktop/LostArk/Engine/Public/Profiler.h:630)는 생성 때 `Begin_Scope`, 블록을 벗어날 때 `End_Scope`를 호출한다. 조기 `return`에서도 정상 소멸하면 종료 표본이 남는다. token은 TLS stack 인덱스이므로 다른 스레드에 넘겨 닫는 계약은 아니다.

[Begin_Scope](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:294)는 이름·깊이·시작 tick·capture epoch를 기록한다. [End_Scope](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:316)는 종료 tick과 thread ID를 넣고 잠금 아래 완료 큐에 추가한다. worker가 mutex를 기다리는 사이 Reset될 수 있으므로 잠금 획득 뒤에도 epoch를 다시 확인한다.

worker가 여러 프레임에 걸친 일이라면 그 scope는 **종료한 프레임에 귀속**된다. 80ms짜리 Loader scope가 5ms짜리 메인 프레임에 붙어도 메인 프레임이 80ms라는 뜻은 아니다. worker와 main은 시간상 겹칠 수 있다.

가령 `Client.Render`가 12ms이고 내부 `Render.Draw`가 8ms, `Render.Present`가 3ms라면 부모 inclusive 12ms 안에 이미 자식 11ms가 들어 있다. 세 수를 더한 23ms는 실제 프레임 시간이 아니다. 계측된 자식이 완전하다면 Render self는 1ms다. self에는 아직 별도 scope를 붙이지 않은 실제 작업도 포함되므로 “아무 일도 하지 않은 시간”이 아니다.

[Get_ScopeAggregates](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:781)는 이름과 스레드별로 집계한다. 완료 순서가 자식 → 부모라는 점을 사용해 반복 정렬 비용을 줄인다. 선택 창에 dropped scope가 있거나 scope가 프레임 경계를 넘으면 self 완전성을 보장하지 않는다. UI의 `--`를 0ms로 바꾸어 순위를 매기면 안 된다. CPU main self, worker inclusive, 전체 frame elapsed는 서로 다른 질문의 답이다.

## G05 GPU query를 기다리지 않고 회수하는 이유

[Create_GpuQueries](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:1061)는 미리 query를 만든다. `TIMESTAMP`는 명령열의 시각, `TIMESTAMP_DISJOINT`는 해당 구간의 주파수와 시계 유효성, `PIPELINE_STATISTICS`는 IA/VS/PS invocation 등의 작업량을 얻는다. hot path에서 매번 query 객체를 새로 만들지 않는다.

[Begin_GpuFrame](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:1119)은 `frameNumber % 8`로 slot을 고른다. 8개 ring을 사용하고 [Resolve_GpuFrames](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:1176)는 최소 4개 실제 루프 경계 뒤에 읽는다. `GetData(..., D3D11_ASYNC_GETDATA_DONOTFLUSH)`가 아직 준비되지 않았다고 답하면 그 프레임에서 기다리지 않고 다음 회수 기회로 넘긴다. slot이 아직 Pending인데 다시 필요하면 이전 결과를 덮지 않고 새 GPU 프레임을 Dropped로 기록한다.

```text
Frame 100: query를 GPU 명령열에 넣는다 → CPU는 계속 진행
Frame 104 이후: 준비됐다면 Frame 100 history를 찾아 GPU 결과를 붙인다
아직 미완료: Pending 유지 → 다음번에 다시 조회
disjoint/주파수0/잘못된 timestamp: 무효 상태 기록
```

GPU 시간은 `(end - begin) × 1000 / disjoint.Frequency`다. 완료되는 순간의 CPU 프레임에 붙이는 것이 아니라 원래 제출 프레임 번호를 사용한다. UI에서 최신 CPU 프레임과 마지막 유효 GPU 프레임이 달라도 오류가 아니다.

이 설계는 계측 때문에 매 프레임 GPU 동기 대기를 만드는 일을 피한다. 대신 최신 몇 프레임의 결과가 늦고 GPU가 크게 밀리면 표본이 빠질 수 있다. query 제출·회수 자체의 비용도 0은 아니다. 파이프라인 통계는 선택 pass에만 붙이며 최대 8개, GPU 시간 scope는 프레임당 최대 128개다.

GPU elapsed는 해당 timestamp 사이의 시간이다. GPU가 CPU의 다음 명령 공급을 기다린 공백까지 반영될 수 있어 GPU busy%, ALU 처리량, occupancy와 같지 않다. `PSInvocations`는 픽셀 셰이더 invocation 수이며 셰이더 명령 수가 아니다. draw와 indices 역시 계측된 Engine 제출량이지 최종 화면에 살아남은 geometry 수가 아니다. DirectXTK 내부 draw 등 계측하지 않은 경로를 자동으로 포함하지 않는다.

GPU self는 immediate-context의 계측된 직접 자식 구간을 한 번씩 차감한다. 부모/자식 inclusive를 더하지 않고 CPU와 GPU 시간도 더하지 않는다. CPU/GPU 타임라인을 가로로 나란히 놓은 것만으로 두 작업의 정확한 인과·동시 시점을 증명하지 않는다.

## G06 누락, p95, 계측 비용을 읽는 규칙

| 항목 | 현재 경계 | 해석 |
|---|---:|---|
| CPU raw scope | 프레임당 최대 8,192개, TLS 중첩 최대 64개 | 누락 수를 보고 self 완전성 판단 |
| CPU 상세 scope | 기본 비활성, 명시적으로 켬 | disabled와 dropped는 다르며 dropped 0도 모든 함수 계측 완료라는 뜻은 아님 |
| GPU scope | 프레임당 최대 128개 | 일부 누락이면 완전한 pass 집계 분모에서 제외 |
| GPU query ring / 회수 지연 | 8개 / 최소 4 loop frame | Pending·Dropped는 측정 0ms가 아님 |
| mesh 상세 목록 | 최대 512개 | 이름은 표시 label, 개별 draw GPU 시간 없음 |
| 완료 프레임 history | 최대 1,200개 | 오래된 hitch는 자동 퇴출될 수 있음 |
| long operation | 8ms 이상 완료 scope, 최대 256개 | deadlock detector나 무제한 tracing이 아님 |

[Get_GpuScopeAggregates](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:884)는 GPU 유효·scope 지원·scope 누락 0인 프레임만 선택한다. 같은 pass가 한 프레임에 여러 번 실행되면 먼저 합친다. 완전한 프레임에 pass 호출이 아예 없으면 그 pass의 0ms로 넣는다. 이것은 **프레임 전체가 미측정인 Pending을 0ms로 넣는 것과 다르다.** 시간과 pass 작업량이 같은 프레임 집합을 쓰도록 한 선택이다.

p95는 평균이 아니라 느린 쪽 꼬리를 보기 위한 백분위다. 현재 저장 경로마다 계산법이 같지는 않다.

| 경로 | 계산 | 원본 |
|---|---|---|
| F7 GPU pass p95 | 정렬 후 `ceil(0.95 × N) - 1` 인덱스, nearest-rank | [Profiler.cpp:943](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:943) |
| Profiler JSON 분포·Python 분석 | 같은 nearest-rank | [ProfilerCaptureIO.cpp:206](C:/Users/tnest/Desktop/LostArk/Client/Private/ProfilerCaptureIO.cpp:206), [analyze_capture.py:13](C:/Users/tnest/Desktop/LostArk/Tools/Profiler/analyze_capture.py:13) |
| RenderingBenchmark 요약 | `0.95 × (N-1)`의 양옆 표본을 선형보간 | [RenderingBenchmark.cpp:487](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingBenchmark.cpp:487) |

예를 들어 정렬한 4개 표본이 `[1, 2, 3, 100]ms`면 nearest-rank p95는 100ms, 선형보간 p95는 85.45ms다. 계산 오류로 단정하기 전에 방법·표본 수·선택 프레임을 맞춰야 한다. 이 문서의 재집계는 nearest-rank다.

CPU 수집에는 QPC, 이름 조회, TLS 처리, 완료 큐 mutex와 메모리 비용이 있다. 패널에도 집계·표시 비용이 있어 `Profiler.Panel.Refresh` 자체를 기록한다. GPU query와 통계에도 비용이 있다. 비교할 때 양쪽의 Capture·detail·도구 표시 상태를 맞춘다. per-draw detail은 원인 탐색용으로 켜고, 실제 플레이 성능의 결론을 낼 때 상세 계측 영향도 별도로 확인한다.

메모리 수치는 [Sample_Memory](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:214)가 최대 초당 한 번 관측한다. private commit, working set, 시스템 commit, DXGI local/nonlocal budget usage를 구분하고 validity·sample age를 본다. 같은 sample을 공유하는 프레임들을 독립 메모리 측정처럼 평균내지 않는다. OS process peak는 캡처 구간 peak가 아니며 이 계측만으로 allocation stack, 누수 원인, 특정 텍스처의 residency를 찾지는 못한다.

## G07 F7에서 저장 JSON까지

[UpdateProfilerRuntime](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:11328)의 F7 처리는 창 표시만 바꾼다. 실제 수집은 [ProfilerTool::Render](C:/Users/tnest/Desktop/LostArk/Client/Private/ProfilerTool.cpp:915)의 `수집(Capture)`가 요청한다. 창을 닫아도 Capture는 계속된다. 초기화와 상세 옵션은 다음 프레임 경계에 적용한다.

`Refresh`는 Engine의 scope·GPU·frame 집계 API를 읽는다. JSON 저장에서는 [Request_Save](C:/Users/tnest/Desktop/LostArk/Client/Private/ProfilerTool.cpp:724)가 먼저 `profiler.Snapshot(...)`을 만든 뒤 저장 시점의 context를 붙인다. snapshot 복사는 메인 경로의 비용이며 JSON 파일 기록은 [BeginSave](C:/Users/tnest/Desktop/LostArk/Client/Private/ProfilerCaptureIO.cpp:821)의 별도 worker에서 수행한다. 모든 저장 비용이 백그라운드라는 뜻은 아니다.

기본 파일 위치는 실행 EXE 디렉터리의 상위 `ProfilerCaptures`다. 일반 Debug/Release 제품 배치에서는 [Client/Bin/ProfilerCaptures](C:/Users/tnest/Desktop/LostArk/Client/Bin/ProfilerCaptures)다. [Make_NamedPath](C:/Users/tnest/Desktop/LostArk/Client/Private/ProfilerCaptureIO.cpp:1016)가 이름·시각·프레임·프로세스·순번으로 새 파일명을 만든다. 분석·표시 frame 범위와 JSON 저장 범위는 별개이며 기본 저장은 보관 중 전체, 최대 1,200프레임이다.

`captureWindow`의 saved/retained/excluded/evicted를 먼저 본다. 이미 ring에서 빠진 프레임은 저장 범위를 넓혀도 복구되지 않는다. snapshot에 들어 있던 Pending query는 나중에 Engine에서 완료되더라도 이미 복사한 저장본에서 바뀌지 않는다. CPU 수집을 끈 뒤에도 메인 루프가 도는 동안 Engine은 남은 GPU query를 회수할 수 있지만, F7 저장 자체가 GPU 완료를 기다려 주는 API는 아니다.

metadata의 camera·level·viewport·설정은 저장 시점 context다. 캡처된 모든 과거 프레임에서 같은 값이었다는 보증이 아니다. 긴 history에 Lobby·Loading·무비·게임 플레이가 섞일 수 있다. 혼합 구간 평균을 특정 장면의 성능이나 최적화 A/B로 발표하지 않는다.

## G08 최적화 Benchmark는 무엇을 추가로 통제하나

F7은 원인 관찰용 자유 캡처이고, 기존 `CRenderingBenchmark`의 최적화 모드는 한 항목의 A OFF/B ON을 비교한다. Debug F1 Rendering Workbench의 최적화 A/B와 Release F1 Optimization Benchmark가 같은 계측 경로를 사용한다.

1. [Prepare_OptimizationPair](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingBenchmark.cpp:703)가 선택 항목의 A/B와 세션 소유값을 준비한다.
2. [Begin](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingBenchmark.cpp:1087)이 Capture를 활성화하고 목표 프레임 범위를 정한다. 최적화 모드는 최소 2 warmup frame, 실제 표본은 10~900개다.
3. [Update](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingBenchmark.cpp:1137)가 warmup 뒤 camera·장면·build·adapter·viewport·비교 대상 외 설정과 실제 적용값을 보관·검사한다. GPU 결과는 Engine이 회수하고, 마지막 표본 뒤 최대 64frame의 제한된 tail까지 Pending을 관찰한다.
4. [Finalize](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingBenchmark.cpp:1245)가 정확한 frame 범위의 CPU·interval·GPU 분포와 draw/indices/counter/work/pass를 계산한다. 첫 interval은 이전 단계 경계에 걸칠 수 있어 제외한다.
5. 각 단계 raw를 최대 64MiB snapshot으로 따로 저장한 뒤 다음 warmup으로 간다. 전체 순서는 A1 → B1 → B2 → A2다. [raw 저장 경로](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingBenchmark.cpp:656)
6. [Build_ComparisonCost](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingBenchmark.cpp:973)는 A 두 번, B 두 번의 평균과 반복 간 차이를 보여 준다. 조건·반복 짝·실효값이 다르거나 최적화 raw 4개가 저장되지 않으면 유효한 완료 비교로 내지 않는다. GPU 비교는 모든 표본이 유효한지도 확인한다.

ABBA는 순서에 따른 시간 변화의 영향을 줄여 확인하려는 설계다. 반복이 두 번씩 있다는 사실만으로 통계적으로 유의한 개선을 증명하지 않는다. NPC·환경 이펙트·컷신 시간을 결정적으로 재생하는 시스템도 아니므로 고정 카메라만으로 작업량까지 완전히 같다고 보장하지 않는다. 대상 0개·worker work 0·cache hit 상태와 실제 작업량을 함께 봐야 한다.

이 모드는 현재 구현의 단일 runtime 스위치를 바꾼다. chunk 분할·geometry bake·LOD 생성 방식·과거 버전 전체를 되돌리는 것은 아니다. Debug와 Release, D3D debug layer 변경은 별도 프로세스 조건이다. 프로세스 재시작도 OS 파일 cache cold를 보장하지 않는다. 결과 저장 위치는 [BenchmarkCaptures 생성 코드](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingBenchmark.cpp:2918)를 따른다.

수치 발표에는 A/B 조건, 표본 수, mean·p95·p99, CPU/GPU 유효 분모, 적용 대상 수와 raw 경로를 붙인다. 예컨대 20→15ms는 시간 25% 감소이고 처리율은 50→66.67FPS로 약 33.33% 증가다. 이는 계산 예시이며 이 프로젝트의 실제 측정 결과가 아니다.

## G09 기존 증거에서 지금 말할 수 있는 것

작성 시점의 로컬 `Client/Bin/ProfilerCaptures`에는 10월 2일 5개와 10월 7일 8개, 총 13개 기존 JSON이 있었다. 로컬 `Client/Bin/BenchmarkCaptures` 폴더는 없었다. 저장 요약 fixture나 컴파일 성공을 실제 GPU A/B run으로 대신하지 않았다.

다음은 [기존 10월 7일 raw JSON](C:/Users/tnest/Desktop/LostArk/Client/Bin/ProfilerCaptures/profiler_20261007_112004_306_frame390_3504_7.json)을 새 실행 없이 읽어 다시 계산한 **전체 저장 구간 예시**다. 프레임 1~390이며 저장 시점 metadata는 Debug, AMD Radeon 840M, 1920×1080, D3D debug layer ON이다. 모든 과거 프레임에서 이 장면·조건이 유지됐다는 의미는 아니다.

| 지표 | 분모 | 평균 | p95 nearest-rank |
|---|---:|---:|---:|
| CPU frame elapsed | 390 | 392.9417ms | 740.9111ms |
| 실제 frame interval | 첫 0 제외 389 | 395.0407ms | 743.5795ms |
| GPU frame elapsed | Valid 386 | 393.8455ms | 743.1212ms |

GPU Pending은 4개이고 CPU/GPU dropped scope 합은 각각 0, raw 상세 CPU 옵션은 390개 모두 OFF다. saved=retained=390, excluded=evicted=0이다. 이 표는 **CPU·GPU 분모가 다르고 Pending이 실제 존재한다는 증거**다. 최신 RTX4050 Release의 성능이나 새 최적화의 개선율로 사용하지 않는다. 10월 7일 8개 파일은 같은 process의 누적 캡처여서 프레임이 겹친다. 독립 실행 8회라고 합쳐 분석해서도 안 된다.

기존 문서의 수치도 용도를 분리해야 한다.

| 기록 | 확인한 내용 | 발표에서 지켜야 할 범위 |
|---|---|---|
| [09-11 계측 확장 결과](C:/Users/tnest/Desktop/LostArk/.md/GB/09-11/2026-09-11_PROFILER_CPU_GPU_STAGE_MEASUREMENT_RESULT.md:8) | 당시 CPU/GPU 구간 목록과 계측 확장 | 계측을 추가한 결과이지 FPS 개선 결과가 아님 |
| [09-22 Profiler 최적화 결과](C:/Users/tnest/Desktop/LostArk/.md/GB/09-22/2026-09-22_BERN_RELEASE_PROFILER_OPTIMIZATION_RESULT.md:8) | 과거 줌아웃 구간의 큰 제출량·NonBlend 비용·scope 상한 | 당시 build metadata 부재를 기록했고 Release 성능으로 확정하지 않음 |
| [10-05 최적화 A/B 구현 결과](C:/Users/tnest/Desktop/LostArk/.md/GB/10-05/2026-10-05_RENDERING_OPTIMIZATION_AB_RESULT.md:1) | 함수 fixture·원본 저장 경로·제품 빌드 검증 | 그 작업에서 새 Bern FPS 개선율은 측정하지 않았다고 명시 |
| [10-07 Release Profiler 결과](C:/Users/tnest/Desktop/LostArk/.md/GB/10-07/2026-10-07_RELEASE_MOVIE_PROFILER_RESULT.md:1) | Release F7, 저장 context, 비Bern 순회 제한과 컴파일 | 이전 Debug 캡처를 현재 Release 저FPS의 단독 원인으로 사용하지 않음 |

과거 문서의 F1 진입·60Hz gate·Release 노출 제한은 작성 당시 내용일 수 있다. 현재 사용 경로는 G02/G07의 실제 코드와 최신 [CLAUDE](C:/Users/tnest/Desktop/LostArk/CLAUDE.md:541)를 우선한다.

기술소개서의 “셰이더 50%”는 성능 개선율이 아니다. [프로젝트 구조 조사](C:/Users/tnest/Desktop/LostArk/.md/GB/10-07/2026-10-07_PROJECT_STRUCTURE_AUDIT_RESULT.md:34)의 당시 Git 추적 코드 확장자 bytes 156,924,941 중 HLSL/HLSLI 78,086,130bytes, 약 49.76%라는 규모 지표다. shader 실행시간·GPU 비중·최적화 이득과는 다른 분모다. 렌더링 기법과 실제 수치의 상세 설명은 [렌더링 기술소개서 해설](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_RENDERING_TECHNICAL_STUDY.md)을 함께 읽는다.

## G10 설계 선택의 장점과 비용

| 선택 | 선택 이유와 장점 | 비용·한계 |
|---|---|---|
| CPU RAII scope + QPC | 함수 블록과 측정 구간의 관계가 명확하고 조기 return에도 닫힘 | 수동 배치한 구간만 보이며 wait와 실행시간이 섞임 |
| TLS 중첩 + 완료 큐 | worker와 main의 중첩을 혼동하지 않고 한 history로 모음 | 완료 시 mutex, 프레임을 넘는 worker self 귀속 제한 |
| GPU 미리 만든 query ring | 매 프레임 GPU wait 없이 원래 프레임에 결과 연결 | 수 frame 지연, Pending/Dropped와 지원 여부 관리 필요 |
| bounded history/detail | 메모리·수집 비용의 무제한 증가 방지 | 오래된 hitch나 상세 표본 누락 가능 |
| 상세 계측 opt-in + 고정 work/counter | 일반 Capture에서 per-draw 비용을 줄이고 작업량은 관측 | 상세 OFF 상태에서 leaf 비용을 모두 알 수 없음 |
| snapshot 뒤 비동기 JSON | 파일 기록 동안 live history와 저장 데이터를 분리 | snapshot 복사는 메인 비용, 복사 뒤 GPU 결과는 저장본에 반영 안 됨 |
| 단일 항목 ABBA | 비교 대상을 좁히고 순서 영향과 반복 차이를 확인 | 동적 장면 결정적 replay·cold cache·통계적 신뢰구간을 보장하지 않음 |

## G11 면접에서 바로 쓸 수 있는 답변

**“Profiler는 어떻게 구현했나요?”**

> 프레임 시작·끝과 주요 update, 렌더 pass에 RAII scope를 넣고 CPU 경과 시간은 QPC로 기록했습니다. 스레드 ID와 중첩 깊이를 저장해서 부모 포함 시간과 계측된 자식을 뺀 self를 나눴습니다. GPU는 D3D11 timestamp와 disjoint query를 8개 ring으로 관리하고 최소 4프레임 뒤 비동기로 회수해 원래 제출 프레임에 연결했습니다. GPU를 기다리며 측정하지 않도록 했고, Pending이나 누락은 0ms로 계산하지 않았습니다.

**“CPU와 GPU 중 어디가 병목인지 어떻게 판단했나요?”**

> 전체 frame interval을 먼저 보고 CPU main self, Present 대기, 유효 GPU pass 시간과 작업량을 같이 봤습니다. CPU/GPU와 부모/자식 시간은 겹치므로 합산하지 않았습니다. NonBlend가 크면 draw·indices·VS/PS와 material 제출 비용을, Blend가 크면 투명 pass와 복사량을 확인하는 식으로 조사 범위를 좁혔습니다. GPU timestamp만으로 ALU 포화라고 단정하지는 않았습니다.

**“최적화가 효과 있었다는 근거는 무엇인가요?”**

> 같은 build·GPU·해상도·카메라와 다른 설정을 유지하고 한 항목만 OFF/ON으로 비교하도록 A1/B1/B2/A2 측정 경로를 만들었습니다. warmup을 제외하고 평균과 p95/p99, 실제 작업량, 유효 표본 수, 각 단계 raw JSON을 연결합니다. 다만 도구 구현과 테스트 성공은 실제 게임 개선율과 구분합니다. 지금 제시하는 수치는 해당 raw 캡처에서 확인한 구간만 말하고, 비교 조건이 다른 캡처나 아직 측정하지 않은 항목에는 개선율을 붙이지 않습니다.

**“단점이나 개선할 점은 무엇인가요?”**

> 수동 계측 밖의 함수는 자동으로 알 수 없고 CPU elapsed에는 대기가 포함됩니다. 상세 계측 자체도 비용이 있어서 최대 표본 수와 detail 옵션을 뒀습니다. GPU 결과는 늦게 오며 저장 시 Pending이 남을 수 있습니다. 다음으로 개선한다면 확정적 장면 재생과 더 많은 반복, 외부 CPU/GPU tracing과의 교차 확인을 추가하고 싶습니다. 이는 현재 구현된 기능과 구분해서 설명합니다.

**“데드락 프로파일링도 했나요?”**

> 현재 이 Profiler는 성능 구간 계측입니다. 완료되지 않은 락 대기 사이클이나 lock order를 추적하는 기능은 여기에 없습니다. 오래 걸린 완료 scope를 관찰할 수는 있지만 그것만으로 데드락 검출을 구현했다고 말하지는 않습니다. 별도 구현이나 dump·wait-chain 조사 증거가 있을 때 그 범위를 추가로 설명하겠습니다.

## G12 이번 자료의 검증 범위

현재 원본 Profiler 두 파일, Client 프레임 경계·F7·저장·Benchmark 호출자와 대응 PLAN/RESULT를 읽었다. 기존 JSON은 read-only로 parse하고 지정 구간의 분모·평균·nearest-rank p95를 재계산했다. 새 UI 조작, Client/Server 실행, GPU benchmark, 제품 코드·프로젝트 변경은 하지 않았다.

주석 사본은 UTF-8 BOM 없이 저장했다. 원본 `Profiler.h` 722줄 → 사본 750줄, 원본 `Profiler.cpp` 1,309줄 → 사본 1,355줄이다. 문자열·문자 literal을 보존하면서 C++ 주석과 공백만 제외한 토큰열을 비교해 각각 3,065개와 8,533개가 완전히 같음을 확인했고, 사본 작성 전후 원본 SHA256도 같았다. 이는 설명 사본의 비주석 코드 보존 검사이며 제품 재컴파일·실행 검증을 했다는 뜻은 아니다.
