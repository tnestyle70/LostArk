## P01. 프로파일링·최적화 — 느린 곳을 어떻게 입증했나

대응: 기술소개서 PPT 9~10쪽. **계측기를 만들었다는 사실과 성능을 개선했다는 주장은 분리한다.** 이번 카드의 첫 방어 대상은 `CProfiler`가 만든 숫자의 의미다. 이어서 최적화 한 항목의 실제 작업 감소를 검증한다.

**⓪ 코드 확인 — 다섯 군데를 보고 와라.**

| 읽을 곳 | 직접 찾아서 적을 것 |
|---|---|
| [프레임 계측 시작](C:/Users/tnest/Desktop/LostArk/Client/Default/Client.cpp:374) → [Begin_Frame / End_Frame](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:103) | Update·Render·Present·FPS 제한 중 CPU 측정 안과 밖에 있는 구간. `FrameIntervalMs`, `PreviousCpuFrameMs`, `FrameGapMs`의 기준점 세 개. |
| [Begin_Scope / End_Scope](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:294) | 시작 스레드의 TLS, 완료 표본 큐, mutex, capture epoch의 역할. worker가 두 프레임에 걸쳐 실행하면 어느 프레임에 기록되는지. |
| [GPU ring 상수](C:/Users/tnest/Desktop/LostArk/Engine/Public/Profiler.h:435) → [Begin_GpuFrame / Resolve_GpuFrames](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:1119) | 제출 프레임, poll 프레임, Pending slot, `GetData` 결과, `Disjoint`와 frequency를 상태표로 연결한다. |
| [GPU 집계](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:884) → [Benchmark 백분위](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingBenchmark.cpp:487) | 평균·p95의 분모와 산식. 미측정 프레임과 해당 pass 호출이 없는 유효 프레임을 어떻게 구분하는지. |
| [A/B 준비](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingBenchmark.cpp:703) → [Begin / Update / Finalize](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingBenchmark.cpp:1087) | warmup·표본 구간·조건 변경·GPU 회수 대기·raw 저장·원래값 복원의 경계. 비교 대상 이외 어떤 조건을 고정하는지. |

읽기 보조: [10-09 호출 흐름 학습 기록](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_PROFILER_CALL_FLOW_STUDY.md). 설계·완료 경계: [10-05 A/B PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-05/2026-10-05_RENDERING_OPTIMIZATION_AB_IMPLEMENTATION_PLAN.md), [10-05 A/B RESULT](C:/Users/tnest/Desktop/LostArk/.md/GB/10-05/2026-10-05_RENDERING_OPTIMIZATION_AB_RESULT.md). 당시 RESULT의 Release F7 미노출 설명은 후속 변경 이전 기록이다. 현재 F7/Capture 진입은 현재 코드와 [10-07 RESULT](C:/Users/tnest/Desktop/LostArk/.md/GB/10-07/2026-10-07_RELEASE_MOVIE_PROFILER_RESULT.md)를 따른다.

**① 문제 — 15초**

- 플레이 화면이 느리다. 정확히 어느 구간이 늦은지 모르는 상태에서, FPS 하나로 결정할 수 없는 것은 무엇인가?
- CPU 제출·GPU 실행·Present 대기·프레임 밖 제한 중 네가 구분해야 하는 것은 무엇인가? 한 원인으로 단정하려면 추가로 어떤 관측이 필요한가?
- **제약:** 이 칸에서 `QPC`, `query ring`, `p95`를 해결책 목록으로 나열하지 마라. 먼저 무엇을 알아내야 하는지 말해라. 실제 FPS 개선을 주장한다면 동일 조건의 원본 캡처 경로를 제시해라. 아직 없으면 미측정이라고 해라.

**② 직접적인 해법의 한계 — 20초**

- CPU 타이머로 `Render()` 앞뒤만 재면 GPU가 그 그림을 완성하는 데 걸린 시간을 얻는가? 그 함수의 반환과 GPU 완료가 같은 사건인지부터 답해라.
- GPU 결과가 나올 때까지 즉시 반복 조회하면 숫자는 얻을 수 있다. 그런데 그 관측이 원래 실행 흐름에 무엇을 바꾸는가?
- **가정 계산:** 부모 Render가 12ms, 내부 Draw가 8ms, Present가 3ms다. 합계 23ms라고 보고해도 되는가? 표본이 완전하다는 조건에서 부모 self를 구하고, 그 값이 순수 CPU 연산시간인지는 별도로 답해라.

**③ 메커니즘 — 25초**

- 현재 ring 크기 8, 최소 회수 지연 4는 어디에 정의돼 있는가? **가정:** Frame 100에서 제출한 query가 104에서도 준비되지 않았다. 104에서는 무엇을 보존하고, 108에서 같은 slot이 필요하면 무엇을 포기하는가?
- 결과가 뒤늦게 도착했을 때 현재 CPU 프레임에 붙이면 왜 틀리는가? 제출 번호와 poll 번호를 둘 다 저장하는 이유, Capture를 껐을 때도 회수할 수 있는 경로를 설명해라.
- `Pending`, `Disjoint`, `Error`, `Valid` 중 어떤 상태만 GPU 시간 표본으로 쓸 수 있는가? timestamp 차이와 frequency의 단위를 말하고 ms 변환식을 써라.

**④ 대안과 대조 — 15초**

- 계측 scope를 심는 현재 방식과 호출 스택을 주기적으로 표본 수집하는 방식은 각각 어떤 질문에 답하기 좋은가? 현재 `Long operations`만 보고 끝나지 않은 데드락의 소유자를 찾았다고 주장할 수 있는가?
- A 한 번, B 한 번과 A1/B1/B2/A2는 무엇이 다른가? ABBA와 warmup을 넣어도 움직이는 NPC·이펙트·컷신의 완전한 동일 재생까지 보장되는가?
- GPU pass 시간, draw 수, PS invocation 수는 서로 대체 가능한 지표인가? draw 감소만 확인했을 때 허용되는 결론과 아직 허용되지 않는 결론을 하나씩 말해라.

**⑤ 대가 — 15초**

- 결과 지연과 표본 누락을 감수하고 무엇을 유지했나? query ring을 키우면 무엇이 늘며, 느린 GPU 문제를 해결한 것이 되는가?
- CPU QPC·이름 조회·완료 큐 잠금·패널 집계·GPU query 자체 비용을 어떻게 비교 조건에 반영하겠는가? 계측 ON에서만 개선이 보이면 다음에 무엇을 확인하겠는가?
- Benchmark를 끝낼 때 저장된 렌더링 설정을 덮지 않고 세션이 소유한 값만 복원해야 하는 이유는 무엇인가? 비교 중 다른 주체가 그 값을 바꾸면 누구의 값을 보존하는가?

**직접 구현·확인 — 숫자를 읽는 함수를 하나 만들어라.**

제품 코드를 건드리지 않는 별도 연습에서 `{status, durationMs}` 목록을 받아 유효 표본 수·평균·nearest-rank p95를 반환하는 작은 함수를 작성한다. 구현 전에 기대값을 적는다.

- **가정 입력:** Valid `[1, 2, 3, 100]ms`와 Pending 한 개, Disjoint 한 개. 미유효 두 개를 0ms로 섞지 말고 결과를 계산해라.
- 유효 표본 0개, 유효한 0ms 한 개, 같은 pass가 한 프레임에 두 번 호출되는 경우를 구분해라. 측정 불가와 0ms를 같은 반환값으로 숨기지 마라.
- 같은 네 값으로 Benchmark의 선형보간 p95도 직접 계산하고 현재 두 경로가 왜 다른 수를 만드는지 설명해라. 어느 쪽을 썼는지 보고서에 기록해라.

그다음 사용자가 직접 만든 캡처 또는 이미 있는 캡처 한 개에서 같은 구간을 재집계한다. 원본 파일·선택 frame 범위·유효/누락 수·계산법·예상과 실제 차이를 남긴다. 이 연습의 성공을 실제 GPU 계측 성공이나 게임 FPS 개선으로 기록하지 않는다. 제품 실행과 화면 조작은 사용자가 한다.

**이 카테고리의 후속 질문**

| 카드 | 다음에 직접 방어할 질문과 시작점 |
|---|---|
| P02 실제 최적화 한 개 | [MapStaticBatchObject](C:/Users/tnest/Desktop/LostArk/Client/Private/MapStaticBatchObject.cpp:702)의 instancing·동일 batch·lighting bank 중 하나를 골라라. 묶을 수 있는 조건과 묶으면 안 되는 재질/geometry/LOD 차이는 무엇인가? OFF가 원래 객체 생성 구조 전체를 되돌리는지, 같은 payload를 개별 제출하는지 확인해라. 개선 시간뿐 아니라 실제 제출 수와 남은 작업을 비교해라. |
| P03 캐시 | [CShader::Find_Variable](C:/Users/tnest/Desktop/LostArk/Engine/Private/Shader.cpp:631)가 캐시하는 것은 변수 주소인가 uniform 값인가? Effect 재초기화·Clone 공유에서 주소의 수명은 누가 지키는가? 캐시 hit 수만으로 이득을 주장할 수 있는가? |
| P04 CPU 비동기 수집 | Reset 이전에 열린 worker scope가 이후에 닫힌다. epoch 검사가 없으면 무슨 기록이 섞이는가? 종료 프레임 귀속의 장단점과 frame 전체 시간과의 차이는? |
| P05 병목 이동 | CPU 제출이 줄었지만 frame interval이 그대로다. 실패한 최적화인가? GPU/Present/프레임 제한·coverage 중 무엇을 다음에 확인할 것인가? |

**방어 완료 증거:** 다섯 칸 답변, 위 숫자의 독립 계산, 유효성 상태를 처리한 작은 구현, 코드 위치를 다시 찾은 기록. 성능 개선 주장은 별도로 동일 조건 A/B 원본 증거가 있어야 한다. `PPT에 적혀 있다`는 측정 근거가 아니다.

## C01. 빌드 시간·셰이더 컴파일 — 무엇을 덜 했기에 빨라졌나

대응: 기술소개서 PPT 19쪽. **증분 Build, 전체 Rebuild, FXC만 실행한 시간, 제품 컴파일·링크·배포 시간은 서로 다른 측정이다.** 이 카드에서는 현재 셰이더 분리와 증분 의존성부터 방어한다.

**⓪ 코드 확인 — 입력에서 배포까지 한 줄을 연결해라.**

| 읽을 곳 | 직접 찾아서 적을 것 |
|---|---|
| [Product runner의 Invoke-MSBuildProject](C:/Users/tnest/Desktop/LostArk/Tools/Build/Invoke-BuildAndRegression.ps1:244) | 실제 target·Configuration·Platform·toolchain·프로젝트 병렬도·로그 옵션. C++ 컴파일과 링크, shader compile 중 무엇을 receipt만으로 구분할 수 있는지. |
| [ProductToolchain.props](C:/Users/tnest/Desktop/LostArk/Tools/Build/ProductToolchain.props:1)와 [CppCompilation.props](C:/Users/tnest/Desktop/LostArk/Tools/Build/CppCompilation.props:1) | C++ `/MP`와 FXC 병렬도가 같은 설정인지. host architecture, PCH 대상과 예외가 각각 무엇인지. |
| [Kouku3584 Particle 진입](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_VtxEffectParticleKouku3584.hlsl:1) → [selected include](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_EffectArtistNativeSelectedGroup3584.hlsli:184) | carrier·group define이 어떤 물리 include와 case를 선택하는지. 공통 include와 한 group include의 변경 영향 차이. |
| [Client FxCompile 등록](C:/Users/tnest/Desktop/LostArk/Client/Default/Client.vcxproj:2223) → [PrepareEngineSdk와 CSO 배포](C:/Users/tnest/Desktop/LostArk/Client/Default/Client.vcxproj:1414) | 소스 등록, Engine 선행 산출물, `SkipUnchangedFiles`, module 옆 CSO가 이어지는 순서. `.filters`는 무엇을 소유하고 무엇을 결정하지 않는지. |
| [Get-BuildOutputSnapshot / Compare-BuildOutputSnapshot](C:/Users/tnest/Desktop/LostArk/Tools/Build/BuildIncrementalDiagnostics.psm1:3) | 현재 자동 기록이 크기·수정 시각인지 내용 hash인지. output writes 0이 보장하는 것과 보장하지 않는 것. |

기준 문서: [현재 빌드 계약](C:/Users/tnest/Desktop/LostArk/CLAUDE.md:68), [셰이더 분리 PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/09-09/2026-09-09_EFFECT_SHADER_BUILD_AND_PLAY_ALL_PERFORMANCE_IMPLEMENTATION_PLAN.md), [RESULT의 G09 — 09-12 빌드 재발 방지](C:/Users/tnest/Desktop/LostArk/.md/GB/09-09/2026-09-09_EFFECT_SHADER_BUILD_AND_PLAY_ALL_PERFORMANCE_RESULT.md). RESULT에 기록된 당시 producer/family 개수를 현재 개수로 그대로 외우지 않는다.

**① 문제 — 15초**

- 개발자가 한 기능을 고친 뒤 실행 결과를 보기까지 어디에서 기다렸나? C++ 번역 단위, linker, FXC, 복사·publish 중 실제 병목으로 확인한 구간을 제시해라.
- “프로젝트가 크니까 오래 걸렸다”를 넘어서, 어떤 입력 변경이 필요 이상으로 큰 컴파일 범위를 만들었는가? 당시 기록의 원인과 현재 코드 분석을 구분해라.
- **제약:** 이 칸에 PCH·캐시·병렬화라는 해결책 이름부터 넣지 마라. 측정 범위와 기다린 작업을 먼저 말해라. 과거 로그를 읽었다는 것을 네가 그때 직접 측정했다고 바꾸지 마라.

**② 직접적인 해법의 한계 — 20초**

- 소스 변경 뒤 항상 Clean/Rebuild하면 오래된 산출물 문제를 피할 수 있다. 개발 반복에서는 어떤 유효한 결과까지 버리는가? 반대로 무조건 skip하면 어떤 변경을 놓치는가?
- 대형 셰이더를 한 파일에 두고 `#if`만 늘리는 것과 필요한 group을 물리 include로 나누는 것은 의존성 관점에서 무엇이 다른가?
- FXC 프로세스 수만 늘리면 한 개의 매우 큰 FX 작업도 그 수만큼 빨라지는가? 쪼갤 수 있는 작업 단위와 단일 작업의 시간, 메모리 압박을 따로 설명해라.

**③ 메커니즘 — 25초**

- Kouku3584 Particle의 define → selected group → native 함수·dispatch → carrier → CSO를 실제 파일로 연결해라. 함수 본문을 옮기는 일과 프로그램 선택의 의미를 바꾸는 일은 어떻게 구분해서 검증하는가?
- **가정:** 같은 파일을 그대로 둔 채 `OutDir` 문자열의 slash 표기만 바꿨다. 같은 물리 폴더라도 FXC 명령행 tracking에서 재컴파일될 수 있는 이유는 무엇인가? 현재 runner는 toolchain·tracking 전후를 어디에 남기는가?
- group leaf 한 개를 바꿨을 때 어떤 consumer만 다시 만들어져야 하는지 먼저 예측해라. Mesh/Particle만 확인하고 끝내면 Decal/Trail/ScreenPost의 어떤 누락을 놓칠 수 있는가?

**④ 대안과 대조 — 15초**

- 표준 라이브러리 PCH, `.cpp`의 헤더 의존성 축소, shader group 분리는 각각 어느 반복 작업을 줄이는가? 한 기법의 효과를 다른 단계의 개선이라고 보고하지 말아라.
- 여러 CSO가 같은 VS 소스를 include하는 것과 실제 GPU VS 객체 하나를 공유하는 것은 같은가? 코드 중복 감소·산출물 크기·런타임 객체 수를 따로 비교해라.
- Engine 빌드를 기다리는 dependency를 지우면 병렬성은 늘어 보인다. Client가 소비할 SDK 헤더·lib·CSO의 일관성은 누가 보장하는가?

**⑤ 대가 — 15초**

- group을 더 잘게 나누면 컴파일 범위는 작아질 수 있다. 대신 늘어나는 project 등록·dispatch·CSO 수·공통 프로그램 중복·검증 부담 중 현재 실제로 지불한 것은 무엇인가?
- **기존 기록 해석:** RESULT의 2,123,966ms → 403,367ms는 어떤 Build 조건의 기록인가? 이 수치를 전체 C++ Clean/Rebuild 개선율이나 현재 PC 성능으로 바꿔 써도 되는가?
- 무변경 Build가 짧다는 사실만으로 신규 팀원의 최초 빌드가 짧아졌다고 할 수 있는가? 최초·무변경·CPP leaf·HLSLI leaf·공통 include 변경을 나눠야 하는 이유를 말해라.

**직접 구현·확인 — 재빌드 범위를 먼저 예측해라.**

제품 솔루션을 다시 빌드하기 전에 별도 작은 의존성 모형을 직접 작성한다. 다음은 제품 실측이 아닌 **연습용 가정 그래프**다.

```text
A.hlsl → common.hlsli, leafA.hlsli
B.hlsl → common.hlsli, leafB.hlsli
C.hlsl → leafA.hlsli
```

- 입력 파일 변경 목록을 받아 영향받는 출력 집합을 찾는 함수를 작성해라. `leafA` 변경, `common` 변경, 변경 없음, 누락 출력 각각의 기대 결과를 실행 전에 적어라.
- 다음에는 파일 내용이 같아도 compiler 옵션이 바뀌는 경우를 추가해라. input 파일 그래프만으로 충분한지, output identity에 어떤 설정을 포함할지 설명해라.
- 실제 프로젝트의 한 group을 같은 표로 옮겨서 연습의 생략 조건을 적어라. 전처리 macro, include search path, toolchain, 생성기, 복사 경로 중 무엇이 추가되는가?

이 연습은 dependency 이해 확인이다. 실제 FXC tracking을 검증한 것으로 쓰지 않는다. 실제 빌드 검증 단계에서는 같은 toolchain·같은 인자의 정상 `Build`를 사용하고, 예상 compiler 실행 범위와 진단 로그·변경 출력·시간을 대조한다. 현재 작업에서는 빌드·timestamp 변경·tracking 삭제를 실행하지 않았다.

**이 카테고리의 후속 질문**

| 카드 | 다음에 직접 방어할 질문 |
|---|---|
| C02 C++ 의존성 | 헤더의 선언 변경과 `.cpp` 내부 구현 변경은 어떤 TU를 무효화하는가? PCH에 자주 바뀌는 gameplay 헤더를 넣으면 어느 범위가 영향을 받는가? |
| C03 성공 증거 | compile 성공, link 성공, Engine DLL 배포 일치, 현재 runtime CSO consumer 확인은 각각 무엇을 증명하는가? 최소 TU compile 성공만으로 전체 제품 링크 성공을 말할 수 있는가? |
| C04 생성기 | 같은 내용을 재생성하면서 매번 파일을 쓰면 증분 빌드에 무엇이 생기는가? byte 동일성·mtime 보존·입력 누락 거부를 각각 왜 확인하는가? |
| C05 전달 | runtime ZIP의 EXE/DLL/CSO를 받는 것과 개발용 OBJ/PCH/tlog를 가진 것은 같은가? Data publish, Resources 설치, 컴파일, 실행 중 reload를 한 단계로 취급하면 어떤 오류가 생기는가? |

**방어 완료 증거:** 다섯 칸 답변, 실제 group의 consumer 표, 연습 구현의 예상/실제 출력, Build 수치의 정확한 조건 설명. PPT의 임의 예시 수치나 무변경 시간은 실제 개선율 증거로 사용하지 않는다.
