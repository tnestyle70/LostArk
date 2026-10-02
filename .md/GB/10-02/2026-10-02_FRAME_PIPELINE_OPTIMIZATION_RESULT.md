# 프레임 계측·CPU·draw 제출 통합 최적화 결과

## G00. 현재 완료 상태

2026-10-03 PR 통합 확인: 아래 최초 작성 시점 이후 정식 Debug Product 빌드가
2026-10-02 17:40 KST에 성공했다. `out/BuildPipeline/runs/20261002T084006819Z-debug-product.json`의
Engine·Shared·Server·Client와 게시 runtime 입력 검사가 PASS이며, 외부
`Build-Debug-GuideNavMovie-Exit.json`의 SourceVerified도 true다. PR 준비 시 현재 입력47개의
SHA가 해당 빌드 대기의 검토본과 모두 일치했다. 따라서 아래의 Debug 제품 링크·설치 대기는
해소됐다. Release Product는 아직 실행 중이며 성공으로 기록하지 않는다. 사용자 화면·FPS는 미확인이다.
추가로 Profiler6개, Effect validator56개, Release Packaging22개, LAN contract3개
단위 검사와 변경 JSON/XML parse, `git diff --check`를 다시 통과했다.

성능 개선의 최우선은 베른의 카메라 이동 중 가시성 검사와 draw 준비·제출이다.
캐릭터 Movie의 동일 animation 입력 재사용은 병렬로 검증하며, CPU 개선 후 남는 GPU 비용을 다시 측정한다.
그 전에 느린 구간이 저장창 밖으로 빠졌는지와 raw scope 누락을 명확하게 드러내야 한다.

| 작업 | 현재 상태 | 아직 확인하지 않은 것 |
|---|---|---|
| profiler Self 신뢰도·저장 범위 | 5개 H/CPP 소스 반영, 실제 함수 검증·Debug/Release TU 컴파일 완료 | 제품 링크·설치·사용자 UI |
| 베른 visibility-only 평가 | 3개 H/CPP 소스 반영, 실제 배치 수치 검사·Debug/Release TU 컴파일 완료 | 제품 링크·설치·사용자 화면·FPS |
| Movie channel sample reuse | 다른 세션의 소스 변경을 보존하고 독립 검증 완료 | 제품 CModel/GPU 통합과 같은 장면 FPS |
| JSON 분석 CLI | 소스 작성, 단위 검사 6개·실제 캡처 3개 재계산 완료 | 새 EXE에서의 변경 후 캡처 |
| 정적 draw 준비·공간 후보·고정 경로 cache·GPU 일괄 처리 | 실제 코드와 공식 자료에 근거한 단계별 계획 | 후속 구현 |

통합 구조·후속 설계·적용 전 전체 코드는
[구현 계획](2026-10-02_FRAME_PIPELINE_OPTIMIZATION_IMPLEMENTATION_PLAN.md)에 있다.
기존 다른 세션의 변경을 되돌리거나 재작성하지 않았으며, 현재 실행 중인 Client에 소스가
자동으로 반영됐다고 보지 않는다. 아래 독립 수치 실험은 게임 FPS 측정과 구분한다.

## G01. 저장된 JSON이 증명하는 범위

입력은 `Client/Bin/ProfilerCaptures`의 다음 세 파일이다.

- `베른_기본캐릭터시점_20261002_154924_085_frame12644_29032_5.json`
- `무비_프레임측정_베른_20261002_154837_417_frame12503_29032_4.json`
- `무비_프레임측정_도화가_20261002_152019_331_frame11529_29032_2.json`

| 저장 구간 | 평균 interval | interval 기반 FPS | CPU raw 누락 |
|---|---:|---:|---:|
| 베른 기본 120프레임 | 76.127ms | 13.14 | 0 |
| 베른 무비 120프레임 | 126.165ms | 7.93 | 79프레임, 1,584,603 scope |
| 도화가 무비 120프레임 | 48.994ms | 20.41 | 0 |

베른 무비 앞 20프레임은 233.544ms(4.28FPS)이며 전체 저장 구간의 최대 interval도 281.489ms다.
500~1000ms에 해당하는 1~2FPS 구간은 이 파일에 없다. 사용자 관찰을 부정하는 뜻이 아니라,
현재 저장본으로 그 구간의 정확한 원인·비율을 확정할 수 없다는 뜻이다.

베른 기본 → 저장된 무비 앞 20프레임의 비교는 visible instance 1,035→8,101,
전체 draw 1,089→5,178, `Map.Batch.Render` 21.613→105.496ms다. 이동 카메라는
기본 시점에서 재사용되던 15,794개 batch 가시성 cache를 다시 평가한다. 카메라의 위치·방향이
실제 표시 대상과 제출량을 크게 바꾸는 근거다. 다만 CPU Render 경과에는 driver 대기도 들어갈 수 있어
105ms 전체를 순수 C++ 계산이라고 단정하지 않는다.

베른 기본의 Update는 17.203ms, Render는 58.205ms다. Render에는 world 외에도 환경 effect 갱신,
ImGui, text, Present가 포함된다. 도화가의 Lights 9.520ms를 베른에 옮겨 원인으로 쓰지 않는다.
GPU frame timestamp와 중첩 pass를 더하지 않으며, GPU timestamp 경과를 GPU 사용률로 해석하지 않는다.

## G02. Profiler와 분석 도구 반영

`Engine/Public/Profiler.h`, `Engine/Private/Profiler.cpp`, `Client/Public/ProfilerTool.h`,
`Client/Private/ProfilerTool.cpp`, `Client/Private/ProfilerCaptureIO.cpp`를 반영했다.

선택 구간에 CPU scope drop이 있으면 aggregate의 `SelfComplete=false`로 표시하고 UI는
Self 숫자 대신 `--`와 원인을 보여준다. 기존 inclusive/self 계산값 자체는 바꾸지 않았다.
v3 JSON의 `captureWindow`에는 requested/retained/saved/excluded/evicted 개수와 frame 범위,
선택창 밖 retained 구간의 최대 interval이 들어간다. eviction은 실제 ring 제거 수를 누적한다.
프레임 번호 차이로 누락/기록량을 추정하지 않으며 Reset으로 초기화한다.

MSVC v143 14.44 `/O2 /MDd /D_DEBUG`로 실제 `Profiler.cpp`와 `ProfilerCaptureIO.cpp`를
좁은 Win32 경계 shim과 컴파일·실행했다. 21개 계약 검사를 통과했다. 300개 보유/120개 저장,
선택창 밖 900ms 프레임, 1205개 기록/5개 eviction, Reset 후 frame 번호가 9000에서 계속되는 경우,
9000 scopes 중 8192개 보유/808개 drop, partial/clean 구간 전환을 포함한다.
실제 베른 무비 264행과 기본 260행의 기존 시간·호출수·thread 값은 원본과 모두 같다.
결정적 JSON 5개는 process identity를 제외한 기존 필드가 같으며 새 필드가 parse되고 수가 일치한다.

실제 vcxproj를 MSBuild로 평가하여 include/define/최적화/CRT 설정을 추출한 뒤,
Profiler.cpp·ProfilerTool.cpp·ProfilerCaptureIO.cpp를 Debug/Release 각각 compile-only로 검사했다.
6개 TU 모두 통과했다. 기존 forced include와 C++20을 유지하고 PCH만 끈 채 후보 Engine/Public을
우선했으며 object/PDB는 out에만 생성했다. 제품·SDK 출력과 실행 프로세스는 변경하지 않았다.
기존 C4819 경고는 남아 있으며, 이 검사는 최종 링크나 제품 실행의 성공을 뜻하지 않는다.

1200 history에서 최근 120개 저장 범위를 조회하는 추가 비용은 독립 실험 중앙값 3.17µs/회다.
UI는 표시 중 0.5초마다 집계하며, 이 조회는 raw scope 복사·정렬을 하지 않는다.
기존 raw 집계와 이 작은 범위 조회의 비용을 혼동하지 않는다.

`Tools/Profiler/analyze_capture.py`는 원본을 변경하지 않고 frame 선택·percentile·CPU/cpuWork·
valid GPU pass·작업량·입력 SHA를 JSON으로 내보낸다. pending/invalid GPU를 0으로 평균하지 않고,
중첩 GPU 구간은 union 경과를 따로 산출한다. 불완전 raw CPU의 Self는 계산하지 않는다.
GPU valid/status가 모순인 입력은 거부한다. 기존 입력이나 기존 분석 파일을 덮어쓰지 않는다.
실제 3개 캡처의 재계산은 위 G01과 일치했다.

재현과 증거:

- `Tools/Profiler/README.md`, `Tools/Profiler/test_analyze_capture.py` — 6개 unittest PASS.
- `out/FrameOptimization20261002/profiler/verification.json` — 실제 함수·호환성 검증.
- `out/FrameOptimization20261002/profiler/source_hashes.json`, `install_receipt.json` — 원본·후보·반영 SHA.
- `out/FrameOptimization20261002/profiler/project_compile/verification.json` — 실제 프로젝트 설정 6개 TU 컴파일.
- `out/FrameOptimization20261002/analysis/final-three-captures.json` — 최종 분석기 재계산.
- `out/FrameOptimization20261002/analysis/reviewer-validation.json` — 독립 리뷰; 이후 GPU status 모순 거부 검사 추가.

## G02-1. 베른 가시성 평가 반영

`MapAssetRenderUtils.h/.cpp`와 `MapStaticBatchObject.cpp`에 반영했다. 두 batch/instance
소비자가 표시 여부만 요청하면 첫 확실한 분리면에서 검사를 끝내고 상세 plane 진단 배열을 만들지 않는다.
기본 FULL 호출, diagnostics, 큰 입력의 float overflow 가능성은 원래 전체 평가를 유지한다.
margin·tolerance 연산 순서·reject grace·bypass·오류 fallback·instance 순서·GPU payload는 유지했다.

설치된 베른 50,021 placement, 1,276 WModel의 1,629,486 vertices를 읽고 실제 Loader의
model preScale 0.01과 placement TRS, 원래 batch envelope 함수를 적용했다.
16,425개 초기 군 중 determinant 조건으로 한 군이 fallback되어 실제 캡처와 같은
16,424 batch/1,222 fallback, authored-visible 15,794 batch bounds가 재구성됐다.
카메라 16키와 저장 시점 2개 행렬, 순방향/역방향/정지/점프·경계·비정상 입력·grace·진단을
대조한 5,283,322검사가 모두 일치했다. 이는 실제 Movie 전 구간을 Client로 재생한 검사는 아니다.

| 실제 배치 bounds 독립 CPU 실험 | 이전 | 후보 |
|---|---:|---:|
| 568,584회 호출, 교대 9회 중앙값 | 83.403ms | 71.415ms |

위 두 중앙값의 차이는 약 14.37%다. paired 속도비는 0.950~1.452로 변동했고 9쌍 중 8쌍에서
후보가 빨랐다. 이는 가시성 함수의 한 실험이며 프레임당 절감 ms나 게임 FPS 증가율이 아니다.
독립 실험을 15,794회 검사량으로 나누면 차이는 약 0.33ms다. 실제 runtime의 다른 비용을 포함하지
않으며, 정지 카메라는 기존 cache hit가 대부분이므로 이 변경의 효과가 거의 없을 수도 있다.
initial fixture는 model preScale이 빠져 있었으므로 그 이전 benchmark 파일은 성능 근거에서 제외했다.

독립 코드 리뷰에서 overflow 안전성 상계, diagnostics FULL, 공통 grace 후처리와 draw 순서를 확인했다.
실제 Client project 설정으로 두 TU를 Debug/Release 각각 컴파일하여 4개 모두 통과했다.
최종 header의 설명 주석 한 줄만 컴파일 후 더 정확하게 고쳤고 실행 코드는 동일하다.
각 파일 UTF-8 no BOM/CRLF를 유지했고 적용 직전 원본 hash를 다시 확인했다.

증거는 `out/FrameOptimization20261002/map`의 `probe-result-final.json`, `fixture-receipt.json`,
`independent-source-review.md`, `project_compile/verification.json`, `source_hashes.json`,
`install_receipt.json`이다. 변경 후 제품 링크와 사용자 캡처는 아직 남아 있다.

## G03. Movie 포즈 재사용 독립 검증

다른 세션이 추가한 Animation sample reuse의 고정 소스와 설치 WModel 38개·80clip·3,177,101키로
실제 Animation/Channel/Bone 소스를 컴파일했다. 현재 구현은 ordered bone index와 SRT 키가 같은
경우 특정 시각의 local matrix 결과를 재사용한다. model별 root/blend/hierarchy와 재질별 draw는 남는다.

Debug/Release 각각 6,770,860검사, local/combined 행렬 6,681,648개의 비트 동등성,
두 스레드 20,000샘플을 통과했다. 역방향 seek, loop, 다른 시각의 cache 교체, clone clock 독립,
부분 SRT, 미갱신 본 보존, 외부 bone 수정·blend·pretransform을 검사했다.

| 37개 실제 animated actor의 채널 처리 독립 실험 | 이전 중앙값 | 재사용 중앙값 | 최초 74clip 등록 |
|---|---:|---:|---:|
| Debug | 5.975ms | 0.146ms | 418.8ms |
| Release | 5.109ms | 0.094ms | 350.7ms |

이는 채널 처리 fixture의 비용이며 전체 animation 또는 제품 FPS 개선량이 아니다.
최초 등록 비용이 늘어나므로 준비 시점도 함께 확인해야 한다. 제품 CModel 수명, GPU skin palette,
실제 draw, 설치 EXE와 화면은 이 수치 검사의 범위 밖이다.

기존 skeleton 전체 bytes 비교의 반복 그룹은 9개지만, 현재 캐시 입력인 ordered bone index/SRT 키는
32개 actor가 하나의 동일 그룹이다. rest/bind skeleton 전체를 공유하는 방식보다 입력 범위가 작다.
또한 38개 animated resource/7,718bones 중 7bones인 `a12245.p0`는 intro/loop animation track이 없다.
실제 Movie 배우의 평가량은 37개/7,711bones로 구분한다.

증거는 `out/FrameOptimization20261002/pose/independent-review-result.json`과
`review_snapshot_manifest.json`이다. 검증 당시 frozen Animation.cpp와 live의 차이는 include와
Evaluate scope 추가였다. 별도 세션 구현을 이번 세션의 새 구현으로 세지 않는다.

## G04. 사전 계산·LOD와 최적화 상한

고정 무비의 mesh/material 연결과 카메라 구간별 가시 후보는 미리 준비할 수 있다.
다만 keyframe 몇 지점만 검사한 목록은 중간 구간의 물체를 빠뜨릴 수 있다. 구간 전체의 보수적 후보,
camera/aspect/placement/material revision과 fallback이 필요하다. 계획 G08에 이를 명시했다.
공통 공간 후보 조회·정적 binding 재사용을 먼저 만들고, 무비 경로 cache를 그 위에 추가하는 순서다.

LOD는 이미 meshoptimizer로 정적 index를 생성하고 CPU가 화면 오차로 선택한다.
별도 설치는 필요하지 않다. 이전 GPU compute 선택은 작은 draw의 비용 때문에 CPU로 옮겼다.
원본 게임 패키지 전체의 LOD 보유 여부는 이 PC의 자료로 확정하지 않았다. 현재 decoder에는
원본 LOD chain 소비 계약이 없으며, 설치 모델의 감소 index는 엔진 생성 경로를 사용한다.

76.127ms에서 60FPS의 16.667ms까지는 전체 프레임 약 78.1% 감소가 필요하다.
기본 시점의 animation 1.093ms만 전부 사라진다는 단순 산술도 약 13.33FPS이고,
Map.Batch.Render 21.613ms가 전부 사라진다는 산술은 약 18.34FPS다.
이는 경로 중첩·CPU/GPU 대기 변화가 없는 가상의 예이며 실제 예측이나 엔진의 최종 상한이 아니다.
최적화 10회라는 횟수로 상한을 정할 수 없고, 남은 critical path와 동일 품질의 작업량으로 판단한다.

## G05. 다음 동일 조건 측정

Debug에서 베른 기본/동일 입장 무비/도화가 동일 구간을 별도로 남긴다.
같은 창 크기·화질·전원·도구창 조건에서 Detailed OFF → Frames 1200 → Reset/수집 →
느린 구간 직후 Capture OFF → GPU pending 회수 → 서로 다른 이름 저장 순서다.
F7은 창 토글이며 창을 숨겨도 수집은 계속된다. Release에는 F7 profiler가 없다.

기존 세 파일은 Detailed ON이다. 새 Detailed OFF 캡처와 단순히 나눈 FPS 비율에는 계측 비용
차이가 섞이므로 최적화의 전후 개선율로 쓰지 않는다. 기존 바이너리의 OFF 기준 캡처가 없으면
이번 새 OFF 캡처를 후속 최적화의 기준으로 보관하고, 현재 변경의 효과는 별도 동일 조건 A/B가
확보될 때 확정한다.

변경 후 P50/P95/P99와 Update/Render/FinalCamera/NonBlend, cpuWork의 Material/Pass/Draw,
visible/draw/cache rebuild를 함께 비교한다. 입력과 품질이 같지 않으면 FPS 비율을 인과적 개선량으로
표현하지 않는다. Agent는 AGENTS의 경계에 따라 Client/UI를 실행·조작하지 않으며,
사용자의 화면 판정과 새 게임 캡처가 남은 검증이다.

## G06. 최종 검증과 제품 빌드 경계

변경한 C++ 8파일의 반영 SHA와 PLAN의 전체 코드 블록을 다시 대조했고 모두 일치했다.
검증 JSON 7개 parse, 신규 분석기 unittest 6개, tracked diff와 신규 PLAN/RESULT/Python의
whitespace 검사를 통과했다. 영구 새 C++ 파일·vcxproj/filter 등록 변경은 없다.
`CLAUDE.md`에는 additive captureWindow와 불완전 Self 표시 계약만 추가했고,
gotchas에는 저장 범위/누락/interval 시차 해석을 남겼다. 기존 다른 세션의 수정은 보존했다.

최종 제품 단계에서 `Tools/Build/ProductOutputGuard.psm1`의 실제 점유 검사를 실행했으며,
`Client/Bin/Debug/Client.exe` PID 29032와 `Server/Bin/Debug/Server.exe` PID 36548이 실행 중이라
표준 Product Build를 진행할 수 없음을 확인했다. 프로세스를 종료하거나 제품/SDK를 교체하지 않았다.
사용자에게 저장 후 종료를 요청한 상태다. 독립 compile-only 성공을 제품 링크·설치 성공으로
대신 기록하지 않는다. 종료 후 정상 증분 Debug → Release Product Build가 다음 단계다.

재현 명령은 다음과 같다. 실제 사용자 Client/UI 실행은 포함하지 않는다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release -Profile Product
```

동일 checkout에 다른 빌드가 진행 중이면 같은 출력에 동시에 빌드하지 않고 그 결과와 입력 범위를
확인한다. 이번 작업에서는 별도 자동 실행·예약 빌드·Client 재시작 작업을 추가하지 않았다.

## G07. 메모리 후속 조사: 압박은 확인, 과거 OOM 종료는 미확정

사용자의 메모리 질문에 따라 현재 프로세스와 기존 로그·소유 코드를 읽기 전용으로 조사했다.
Client를 재실행하거나 Bern을 재생하지 않았고, 이번 메모리 조사에서 제품 코드는 바꾸지 않았다.
관측 대상은 15:12:54에 시작한 기존 Debug Client PID 29032이며 위 소스 변경을 새 EXE로
설치한 뒤의 관측이 아니다. 메모리 조사와 계측 확장 순서는 PLAN G12에 추가했다.

### G07.1. 현재 OS 관측

`out/FrameOptimization20261002/memory-observation-20261002.json`은
2026-10-02 17:29:06 KST의 순차 OS 조회다. 조회 사이의 작은 시간 차이는 있으며 atomic snapshot은 아니다.

| 지표 | 원시 byte | 약 GiB/MiB | 의미 |
|---|---:|---:|---|
| OS 사용 가능 전체 물리 RAM | 16,127,729,664 | 15.02GiB | OS가 사용할 수 있는 전체 물리 용량 |
| 현재 가용 물리 RAM | 989,085,696 | 0.92GiB | 시스템 전체의 물리 여유 |
| Client PrivateMemorySize64 | 17,020,428,288 | 15.85GiB | Client 전용 commit; 전부 RAM 상주라는 뜻이 아님 |
| Client WorkingSet64 | 262,258,688 | 250.11MiB | 현재 RAM 상주 pageable 영역 |
| Client PeakWorkingSet64 | 5,683,388,416 | 5.29GiB | 이 process 수명의 working-set 최고값; 전체 commit 최고값이 아님 |
| 시스템 전체 commit | 49,862,369,280 | 46.44GiB | 모든 process·시스템을 포함한 commit |
| 시스템 commit limit | 53,764,173,824 | 50.07GiB | 현재 backing capacity 경계 |

시스템 commit 비율은 약92.7%다. 현재 전용 할당량과 낮은 가용 RAM을 함께 보면 메모리 여유를
성능 작업의 우선 조건으로 볼 근거가 있다. 낮은 WorkingSet만 보고 Client 메모리가 작다고 해석하지 않는다.
동시 수집한 GPU process counter의 한 adapter DedicatedUsage는 약5.43GiB이지만 DXGI Budget을
수집한 것은 아니므로 예산 초과나 GPU OOM으로 확정하지 않는다. CPU private와 GPU 값을 단순 합산하지 않는다.
시스템 page-read counter도 특정 Client의 fault나 그 비용을 증명하지 않는다.

### G07.2. 같은 process의 과거 이벤트 기록

`Client/Bin/Debug/Diagnostics/client-session-29032.jsonl`에서 메모리 필드만 추렸다.
선택한 원시 값과 읽은 시점 파일의 SHA는 `out/FrameOptimization20261002/memory-session-selected.json`에 있다.

| 시각 KST / 원본 행 | Client private | 관측 |
|---|---:|---|
| 15:45:47 / 47 | 8.961GiB | Bern entry.accepted |
| 15:47:26 / 51 | 15.181GiB | Bern session의 후속 관측 |
| 15:52:32 / 57 | 15.973GiB | 가용 RAM 0.709GiB |
| 16:25:58 / 91 | 15.901GiB | 가용 RAM 0.246GiB, Client WorkingSet 0.378GiB |

승인 이후 약7.01GiB 증가했으나 공통/캐릭터/기존 자원과 도구 상태가 포함돼 순수 Bern 자산 크기는 아니다.
시간에 따라 증가했다는 사실만으로 leak라고 판정하지 않는다. 이 로그는 세션 이벤트·stall·60초 heartbeat이며
로딩 중 최고값을 빠짐없이 기록하지 않는다. 현재 crash log는 비어 있고 확인한 exit log 두 건은 WM_QUIT/hr=0이다.
과거 사용자 경험과 일치하는 OOM 예외·실패 HRESULT·dump는 확보하지 못했다. 확인 범위의
RADAR_PRE_LEAK_64 이벤트도 누수/OOM 확정으로 쓰지 않는다. 빈 로그가 사용자의 과거 종료 경험을 반증하지도 않는다.

### G07.3. 실제 메모리 생존 구조

- `WModelDecoder.cpp:125`의 파일 bytes는 decode 결과와 겹치지만 decoder 반환 뒤 사라진다.
  `Model.cpp:2441`의 decoded asset은 mesh/material/animation 준비가 끝날 때까지 남는다.
- `Mesh.cpp:135`의 변환 정점 복사, `:164`의 CPU picking 정점/index/BVH와 GPU VB/IB를 구분한다.
  CPU picking은 업로드 후에도 보유한다. `StaticMeshLod.cpp:32`의 attribute/combined/reduced 임시 배열과
  생성된 추가 index buffer도 LOD의 메모리 비용이다.
- Loader는 같은 model path의 geometry를 공유하며 Clone도 mesh/material을 공유한다.
  CMaterial texture cache의 참조는 weak_ptr이고 same-key SRV도 공유한다. 이미 하는 공유를
  앞으로 새로 얻을 메모리 절감으로 다시 계산하지 않는다.
- `MapPlacementRuntime.cpp:233`의 area별 load-stage cache는 catalog/records를 복사하고,
  조회도 복사한다. runtime Clear와 수명이 다르다. 같은 area는 replace되므로 무한 누적이라고 단정하지 않는다.
- 공통 준비 callback 최대4와 별도 skinned-material worker 최대3은 작업 수 제한이다.
  완료된 map prototype staging과 진행 중 임시 작업의 byte 합계를 제한하는 budget은 아니다.
- 새 Loading worker가 시작된 뒤 이전 level 정리가 진행되어 짧은 중첩이 가능하다.
  이전 월드 전체를 로딩 내내 유지하는 구조는 아니다. Loader 취소의 5+5초 timeout fail-fast도
  allocation failure와 별개인 종료 경로다.

기존 09-16 Bern 2,000 variants/226 geometry 표본은 4 worker 2.441초, 8개 3.194초,
16개 4.463초였다. warm 파일 cache·고정 순서의 과거 일부 표본이고 현재 전체 로딩 성능은 아니다.
worker 수 증가가 자동으로 빨라지지 않는 기존 근거로만 사용한다.

### G07.4. Profiler도 메모리를 사용한다

현재 CProfiler JSON의 MapBatch/ImGui/SceneColor Bytes는 주로 전송 payload다. RAM 점유나
현재 상주 VRAM으로 읽으면 안 된다. TextureEstimatedGpuBytes는 writer가 연결되지 않아 N/A다.
메모리 기본 값은 별도 ClientSessionDiagnostic에서 부분적으로 기록하고 있으나 Profiler의 연속 메모리 timeline은 없다.

MSVC 14.44 x64 `/Zs` static_assert로 FProfilerScopeSample의 sizeof32/alignof8을 확인했다.
8192 scopes × 1200frames × 32bytes는 CPU scope 본문만300MiB이며 전체 Profiler의 실제 사용량이나
전체 할당 상한이 아니다. Snapshot은 frame/vector를 복사하고 exporter worker가 이 사본을 저장 완료까지
소유하므로 live history와 저장 중 사본이 겹친다. JSON은 파일로 streaming하며 전체 JSON 문자열을
추가로 보유한다고 가정하지 않는다. 중복 Save는 차단돼 있다.
따라서 Frames1200은 구간 보존에 도움이 되지만 메모리 비용도 있으며 Detailed OFF가 중요하다.
이번 컴파일은 문법 검사만 했고 큰 메모리를 할당하는 probe나 게임 부하 재현은 하지 않았다.

메모리 안정성의 다음 완료 조건은 phase별 peak와 owner별 잔존량, 같은 진입/퇴장 반복에서의 보유 정책,
allocation/GPU/timeout 실패 구분이다. 그 근거 없이 worker를 늘리거나 cache를 무한히 추가하지 않는다.
