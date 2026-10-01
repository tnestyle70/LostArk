# 베른 캡처 기반 공통 최적화·Release Profiler 결과

기준일: 2026-09-22. 현재 작업 브랜치는 `GB/collider-pattern-bug-fix`다. 기존 다른 세션의
렌더링·게임플레이 변경을 유지했다. 이 문서는 실제 반영과 검증을 소유하며, 조사 후보와
반복 측정 방법은 [프로젝트 성능 계측 가이드](2026-09-22_PROJECT_PERFORMANCE_MEASUREMENT_GUIDE.md),
구현 범위는 [PLAN](2026-09-22_BERN_RELEASE_PROFILER_OPTIMIZATION_IMPLEMENTATION_PLAN.md)에 둔다.

## G00. 원본 캡처에서 확인한 병목

한국어 베른 JSON 5개, 약 2.15 GB를 분석했다. 카메라가 섞인 전체 평균과 고정 구간을 분리했다.
풀줌아웃의 마지막 84프레임(16179–16262)은 visible instance 39,009개가 유지되는 구간이다.

| 고정 줌아웃 측정 | 값 |
|---|---:|
| 평균 frame interval / 처리율 | 376.519 ms / 2.656 FPS |
| frame interval p50 / p95 / p99 | 368.491 / 407.705 / 416.336 ms |
| draw / instanced draw | 19,761 / 17,762 회/프레임 |
| 제출 indices | 46,562,757 / 프레임 |
| Render.NonBlend CPU / GPU elapsed | 306.125 / 320.890 ms |
| CPU Update | 17.997 ms |

원본 분석의 백분위는 선형보간이며, 새 JSON summary는 `percentileMethod=nearest-rank`를
명시한다. 프레임 수가 적을 때 두 방법의 수치를 같은 정의처럼 비교하지 않는다.

이미 인스턴싱을 사용하지만 batch와 submesh 제출이 많았다. 기존 LOD도 존재했으나 이 구간의
indirect draw는 0이다. 원본에 build/camera/settings metadata가 없어 이 값을 Release 성능으로
확정하지 않는다. CPU/GPU elapsed를 더하거나 GPU elapsed만으로 연산 포화를 단정하지 않는다.
모든 파일의 유효 Shadow scope PS/VS는 0이므로 파일명만으로 그림자 설정 A/B를 계산하지 않았다.

CPU raw scope는 8,192개 제한에 걸렸다. 실제 instanced draw 17,762회에 비해 기록된
Map.Batch.Mesh.Submit은 약 1,847회였다. 따라서 누락된 leaf의 순위나 parent self를 완전한
함수별 비용처럼 해석하지 않았다. [선택 구간 원문](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/selected_summary.json),
[분석 기록](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/CAPTURE_ANALYSIS.md)에 분모가 있다.

## G01. Debug/Release 공통 계측 반영

`MainApp → CProfilerTool → ProfilerCaptureIO`를 기존 ImGui에 연결했다. **F7로 창을 열고
Save JSON으로 저장한다.** 첫 F7은 수집을 시작한다. 창 숨김은 수집 중단이 아니며 저장 완료
회수도 창이 닫힌 상태에서 계속된다. Release가 사용하는 Character/AnimationTargetService 헤더도
Debug include 경계 밖으로 옮겨 실제 Release 최소 컴파일을 통과시켰다.

- 기본 저장은 선택한 최근 120프레임, 옵션은 최대 1,200프레임이다. Snapshot 자체가 필요한
  window만 복사한다. 한국어 이름·동명 고유 파일·비동기 저장·원자적 실패 보존을 유지했다.
- JSON v3에 build/compiler/iterator debug/device debug layer/adapter/viewport/camera/level/
  shadow와 후처리 설정을 추가했다. `sampledAtExport`는 저장 시점이며 과거 모든 프레임의 상태가 아니다.
- frame interval·CPU·유효 GPU의 평균/p50/p95/p99/max, 초과 프레임, pending/partial과
  window의 CPU 누락을 기록한다. 첫 interval 0과 invalid GPU는 해당 분포의 분모에서 제외한다.
- per-draw CPU detail은 기본 비활성이다. frame별 detail 모드와 droppedCpuScopes를 저장하고
  누적값과 구별한다. `cpuScopesWithinBudget`는 overflow 여부이며 전체 함수 계측의 완전성이 아니다.
- Capture/pause/Reset/detail 전환은 프레임 경계에서 반영한다. Reset은 이전 interval 기준점을
  비우며 worker의 오래된 scope는 capture epoch와 mutex 이후 재검사로 새 history에 섞이지 않게 한다.
- 실제 소비 지점에 culling 후보/통과, LOD0/1/2·원본/제출 index·생성된 LOD 가용성,
  shadow cache hit/miss·static/dynamic 제출, light record/draw/upload bytes를 연결했다.
  light record는 receiver pass별 제출 수이며 고유 광원 수가 아니다.

`Engine/Public/Profiler.h`의 counter는 기존 ordinal 뒤에 추가했다. 공개 헤더 변경 때문에
실제 제품 배포 시 Engine과 Client를 함께 빌드하고 SDK를 갱신해야 한다.

## G02. 공통 static mesh LOD 반영

`StaticMeshLod → Mesh → MapStaticBatchObject`에서 CPU에 있는 scalar 화면 오차 입력을
compute dispatch·UAV·indirect로 왕복하던 경로를 CPU 선택과 direct instanced draw로 바꿨다.
생성 geometry, 현재 draw 재질의 admission, 기존 0.25px 한계와 0.9 안전 여유는 유지했다.
invalid/near 조건은 원본으로 돌아간다. compute 비용 때문에 있던 index×instance 294,912
제한을 제거했으며 geometry 생성의 24,576~3,145,728 indices와 최대 1,048,576 vertices 경계는 유지했다.

넓게 퍼진 instance들을 하나의 world sphere로 감쌀 때 가로 폭이 가까운 깊이로 해석되어
LOD0에 묶이는 문제를 고쳤다. 각 visible instance의 보수적인 view depth/XY envelope를 사용한다.
finite/scale·부동소수점 바깥쪽 여유를 검사하고 성공한 payload와 camera revision에만 commit한다.
늦게 확정되는 camera/shadow provider보다 앞에서 임의로 cull하는 변경은 하지 않았다.

기존 compute shader는 참조 파일로 남기고 FxCompile/filter·Client 필수 배포 등록을 정리했다.
새 영구 C++ 파일은 없다. 베른 전용 분기 없이 같은 map/model/mesh 경로에 적용된다.

| 120 draw 고정 fixture, GPU 중앙값 | 원본 LOD0 | 기존 compute LOD | 새 CPU 선택 LOD |
|---|---:|---:|---:|
| 24,576 indices × 3 instances | 0.471 ms | 1.022 ms | 0.201 ms |
| 98,304 indices × 3 instances | 1.518 ms | 1.345 ms | 0.475 ms |

이 값은 Debug CRT `/Od /RTC1`의 **독립 D3D fixture**이며 게임 FPS가 아니다. `/O2`에서도
재확인했다. 기존 GPU/새 CPU 선택 960조건 일치, 생성 index buffer·동일 LOD 출력 일치,
view bounds 500,000점 포함 검사와 D3D debug 오류 0을 확인했다. 원본 LOD0와 단순화 geometry는
동일 geometry라는 주장이 아니며 수치 출력 차이와 사용자 시각 판정을 구분했다.
실제 설치 submesh 4개 표본 중 1개는 148,224→102,456 indices로 감소하고 3개는 감소하지 않았다.
모든 에셋에서 LOD가 생성되거나 큰 이득을 낸다고 주장하지 않는다.
[LOD 명령·증거](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/lod/README.md)

## G03. ServerNavigation::Find_Path 반영

매 query마다 grid 전체 costs/parents/closed 배열을 할당·초기화하던 경로를 thread-local
generation scratch로 바꿨다. 실제 방문 cell만 초기화하고 generation wrap은 전체 state를 비운다.
재진입은 임시 scratch, thread 간은 독립 저장소다. region dispatch, heap tie·경로·Server 권위는 유지한다.
최대 100만 cell에서 배열 payload 12 MB/thread를 보유하는 메모리 비용이 있다.

| 실제 navgrid의 warm 짧은 query | Release 이전→이후, µs/query |
|---|---:|
| Bern | 735.168 → 5.749 |
| Valtan | 76.843 → 4.620 |
| Character Select | 13.585 → 12.020 |
| Kouku StartFine | 8.494 → 9.654 |
| Kouku Gate3Fine | 12.053 → 5.923 |

9개 실제 navgrid, Debug/Release 각각 837개 sequential 및 720개 다중 thread 경로·expanded·smooth
비교가 일치했다. blocker/void/reload 70개와 wrap 70개도 통과했다. 실제 CPP의 기존 nav metrics
probe를 Debug/Release에서 다시 통과시켰다. 작은 StartFine의 Release 중앙값은 느려졌다.
첫 allocation은 timing 밖이며 공유 개발 PC의 함수 microbenchmark다. Server tick·Client FPS의
동일 배율 개선을 뜻하지 않는다. [전체 수치·메모리 경계](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/navigation/NAV_OPTIMIZATION_RESULT.md)

## G04. 실행한 컴파일·계측 검증

| 검사 | 현재 증거 |
|---|---|
| Profiler/Renderer/Light_Manager/MainApp/ProfilerTool/ProfilerCaptureIO/NetworkManager actual CPP | Debug `/Od /RTC1 /MDd`, Release `/O2 /MD` isolated compile PASS |
| StaticMeshLod/Mesh/MapStaticBatchObject/MapAssetRenderUtils actual CPP | Debug/Release focused compile PASS; LOD fixture `/Od`, `/O2` PASS |
| Profiler CPU 경계 | Debug/Release startup·reset·pause·resume·epoch·detail·frame별 loss·bounded snapshot PASS |
| GPU WARP query | Debug/Release 40frame 중 valid36/pending4, pause 중 회수·overflow·model sample cap PASS |
| JSON/file 저장 | Debug/Release sync/async·한국어·1,000 고유 이름·no-replace·실패 보존·single-flight·shutdown PASS |
| exporter 독립 검증 | actual CPP로 생성한 Debug/Release JSON 10개 strict parse, counter 68개·nearest-rank·empty/null·GPU 분모·context/async copy·window drop PASS |
| 파일 구조 | 변경 project/filter XML 4개·endpoint JSON parse 및 전체 `git diff --check` PASS; 화면 밖 최적화 C++ 8개는 기존 UTF-8/BOM 유무/CRLF 유지(Kouku는 원래 BOM 있음) |
| symlink fixture | 생성 권한 오류 1314로 두 항목 SKIP; PASS로 계산하지 않음 |
| 제품 EXE/DLL 정상 Build | 공유 Engine/Shared/Server Debug 정상 Build 성공(18:46), SDK Profiler.h 최신 hash 일치. Client Debug 정상 Build 진행 중; 전체 link/배포와 Product Release는 미완료 |
| 실제 Client F7·게임 FPS·화면 | USER_PENDING. Client/UI 자율 실행하지 않음 |

20,000개의 detail scope를 생성하는 독립 probe에서 off/on의 프레임당 계측 비용은
Debug CRT `/O2` 0.0235/3.7696 ms, Release `/O2` 0.0125/2.0796 ms였다.
on은 8,192개 기록·11,808개 누락, off는 의도적 미수집·누락 0이다. 이 값 역시 전체 게임 성능이 아니다.
실행 스크립트와 로그는 [검증 폴더](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922)에 있다.

## G05. LAN 정본과 사용자가 확인할 결과

요청한 데스크탑 주소 `192.168.0.22:7777`를 TeamLanEndpoint.json, Client 기본 endpoint·x64
Debug/Release debugger 설정, AGENTS/CLAUDE와 팀 네트워크 문서에 반영했다.
Sync-TeamLanEndpoint 결과는 실제 Wi-Fi 2 주소와 일치하는 `server-host`, LocalSubnet 방화벽
준비 완료였다. probe는 `not-listening`이며 Server 실행·접속 성공으로 기록하지 않았다.
Server bind `0.0.0.0`과 격리 harness loopback은 유지했다. 기존 portable ZIP의 .14 주소는
과거 산출물의 설명이므로 현재 소스 .22가 그 ZIP을 자동 변경하지 않는다는 안내를 덧붙였다.

최신 제품 빌드 후 사용자가 한 Client씩 실행하여 같은 조건의 Bern 일반/줌아웃,
Valtan·Character Select·Kouku 캡처를 반복한다. F7의 detail off로 워밍업 후 Reset,
창을 숨긴 측정, Capture 중단·GPU 결과 회수, 이름과 Save JSON 순서로 기록한다.
LOD 생성/선택률, 제출 index, draw, culling 후보/통과, NonBlend CPU/GPU, p95/p99와
시각 차이를 비교해야 실제 이득과 다음 병목을 확정할 수 있다.

DOD/SoA와 OOP는 서로 반대 개념이 아니다. 이 작업에서 OOP→DOD 전체 변환이나 fiber scheduler를
구현·측정하지 않았다. 기존 줌아웃 Update 18 ms만 2배 빨라진다는 가정이면 전체는 약
376.52→367.52 ms, 2.66→2.72 FPS다. 이는 Amdahl 계산 예시이며 실제 최적화 결과가 아니다.
현재 큰 비용인 렌더 제출부터 줄이고 worker 준비 시간·main join·실제 frame p95를 함께 비교한다.

## G06. 호환 배칭과 재질 API 비용

SR_MinecraftDungeons의 stage 경로는 월드 정점 bake·4×6 atlas·내부면 제거 뒤 실제 한 번의
DrawIndexedPrimitive를 사용한다. 전체 프레임이 한 draw라는 뜻은 아니다. Bern의 정적 mesh는
이미 instancing을 사용하며 baked/RNM/static-shadow texture 차이가 material variant를 나눈다.

현재 50,017 placement/15,589 used asset을 정적으로 묶으면 asset+mirror 16,422그룹이다.
기존 캡처의 runtime batch 16,421과 차이 1개는 아직 원인을 확정하지 않았다. 완전히 동일한
geometry/material을 합쳐 제거 가능한 175그룹은 모두 숨김 CUL_BOX placement이므로 실제 draw
개선으로 계산하지 않는다. RNM·shadow를 texture array index로 옮기는 가상 grouping은 3,008이나,
포맷·크기·mip bank와 submesh 분할 전 수치다. 이번 변경에서 texture array를 구현하지 않았다.
공간 단위 culling을 유지하며 호환 재질을 묶는 구조가 후속 후보다.

이번에는 MapAssetRenderUtils의 INSTANCED 입력 모드를 명시하여 vertex instance가 이미 제공하는
값과 존재하지 않는 character 변수에 대한 중복 binder 요청만 생략했다. 기본 OBJECT 경로는 같다.
10 material family, 2,520조건, 85,155,840개 float 비교에서 최대 차이 0, D3D 오류 0이었다.

| 20,000회 binder, 7회 ABBA 중앙값 | 이전 | 이후 |
|---|---:|---:|
| `/Od`, Debug CRT | 80.271 ms | 78.057 ms |
| `/O2`, Debug CRT | 46.115 ms | 42.725 ms |
| material raw API 요청 수 | 63 | 54 |
| warm 실제 Effects11 SetRawValue | 8 | 8 |

캐시가 이미 있으므로 API 요청 감소를 실제 GPU upload 9회 감소로 설명하지 않는다.
[배칭 조사·수치 검증](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/batching/README.md)

## G07. Bern 환경 표현·이동 표식·광원의 화면 밖 작업

실제 Bern에는 resident 이동 표식이 없고, 91개의 ambient SOURCE_LOOP placement가 거리만 검사한
뒤 카메라 뒤에서도 서비스의 Advance_Preview와 일반 Late_Update를 수행했다. Valtan의 이동 표식
5개는 이미 외부 clock·final-camera culling을 사용해 숨은 재생을 중단한다. 기존 자동 중복 Update
문제가 있었던 것으로 기록하지 않는다. 다만 8m carrier bound에 16/32m의 추가 여유가 붙어 있었다.

공통 service helper로 Valtan/Kouku 표식의 여유를 비활성 0m/활성 0.5m로 줄였다. 표식 원본과
설치 mesh SHA가 기존 실측과 일치하며 당시 850 step·2 loop의 최대 carrier 3.310587m보다 8m가 크다.
이번 소스로 추출한 marker lifecycle 61검사, frustum 기하 48,034검사를 통과했다.
scaled/sheared/non-finite/non-affine camera는 sprite billboard 확대 가능성이 있어 fail-open한다. 과거 actual
playback 결과와 이번 검사를 구분하며 사용자 화면 검증은 남아 있다.
[표식 검토](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/marker_review/README.md)

일반 환경 효과는 레벨명 분기 없이 LEVEL_ACTIVE + SOURCE_LOOP만 offscreen pause를 요청한다.
CEffectPlayback이 source module·분포·수명·속도·가속·크기·pivot·root를 해석하여 전체 sprite 범위를
계산할 수 있을 때만 적용한다. 외부 owner/anchor, mesh/trail/light/screen/control, 미지원 module은
기존 재생으로 남긴다. Root가 바뀌면 그 occurrence는 이후 계속 fail-open한다.

admitted occurrence는 Engine 자동 Late_Update 제출을 차단하고 최종 camera가 확정된 MainApp
Render 직전에 한 번만 갱신·제출한다. 화면 밖에서는 입자와 RNG 상태를 유지하되 시각적 시계를
멈추며 숨은 시간을 누적하거나 재진입 때 몰아서 replay하지 않는다. 따라서 연속 숨은 재생과
ambient 위상은 달라진다. Server trigger·이동·combat clock과 externally sampled 표식은 이
pause 대상이 아니다. 같은 조건의 모든 맵이 공통 경로를 소비한다.

Light_Manager는 최종 camera의 보수적 plane과 POINT/SPOT 영향 sphere를 pass마다 한 번 만든
검사 경로로 비교해 완전히 밖인 광원만 pack/upload 이전에 생략한다. directional·경계 교차·
불확실한 camera는 유지하고 원본 scene/transient 목록, forward 입력, shadow caster를 보존한다.
Debug/Release의 기하 76,991 assertion 및 8,948개 survivor record/순서/flag 비교를 통과했다.
[광원 검증](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/light-culling/RESULT.md)

추가 counter는 `lightCullingCandidates/Rejected`, `effectBoundsCandidates/Culled`,
`effectMarkerSamples`, `effectMarkerHistoryRequests`, `effectAmbientSuspended/Advanced`다.
marker history는 entry/reentry 요청 수이며 내부 rewind/large-gap rebuild 전체 횟수가 아니다.
`Effect.LevelPresentation.VisibleUpdate`로 최종 camera 단계의 환경 갱신 비용을 분리한다.
새 service·map effect·두 arena·MainApp·ProfilerTool·CaptureIO의 Debug/Release 최소 컴파일 14건을
통과했다. 실제 Update/Submit/Update_WorldRoot 추출 lifecycle의 Debug/Release 32검사도 통과했다.
121 hidden frame, initial seek·visible dt·중복 final phase 방지·무catchup 재진입·root 무효화·
fallback/owner/level/failure 정리를 검사했다. 상태 보존은 기존 draw distance 안에서 카메라에만
숨은 경우에 한한다. [lifecycle 근거](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/ambient-lifecycle/RESULT.md).
static source bound의 최종 실제 재생 검증도 완료했다. 베른 11개 문서 중 sprite 10개,
91개 placement 중 85개가 적용 가능하고 mesh 혼합 1개 문서/6개 placement는 기존 경로를 유지한다.
실제 placement TRS의 bound 반경은 최소 3.848m, 중앙값 24.266m, p95 49.933m, 최대 84.969m다.
큰 폭포·안개의 원본 분포는 줄이지 않았으므로 적용 가능 85개가 매 프레임 모두 culled된다는 뜻은 아니다.

`/Od /RTC1 /MDd` actual playback에서 random root 40개 × 900 step 및 크기 모듈 순서 12개를
실행해 1,063,822개 particle row가 analytic bound 안에 들어왔다. 최대 observed/analytic은
0.965254이고 미지원 구성 22개는 fail-open했다. 실제 CPP의 Release `/O2 /MD` 컴파일도 통과했다.
생성된 bound는 표본 최대값이 아니라 source 분포·수명·운동을 해석한 상계다. 표본 검사는 그 계산과
실제 재생 소비자의 대조 증거이며 모든 미래 source module의 안전성을 대신 증명하지 않는다.
[bound 근거·제약·명령](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/bounds/BOUNDS_RESULT.md).
GPU pixel 및 제품 FPS 개선으로 대신 해석하지 않는다.


제품 빌드 확인 중 앞선 Client Debug 빌드가 stale SDK의 counter enum을 읽어 20개 compile 오류를
낸 기록도 확인했다. 이후 18:46 Engine 정상 Build가 최신 68-counter public header를 배포했고
Engine/Public과 EngineSDK/inc의 SHA256 일치를 확인했다. 현재 Client 재빌드가 진행 중이다.
header를 수동 복사하거나 SDK 이전 DLL과 새 header의 조합을 완료 상태로 취급하지 않는다.
[Engine 정상 Build 로그](C:/Users/user/Desktop/LostArk/out/CharacterSizeSave20260922/product-final-build/20260922T094624012Z-Engine-Debug.log).


마지막 점검 중 C: 공간 부족이 발생하여 이번 `out/BernProfiler20260922` 내부의 재생성 가능한
OBJ/PDB 152개(674,636,165 bytes)를 정리했다. 원본 캡처·소스·실행 로그·JSON receipt·probe EXE와
재현 스크립트는 보존했다. 이후 전체 `git diff --check`를 다시 실행해 통과했다. 개별 검증을 다시
실행할 때는 기존 object 재사용 단축 명령보다 각 full compile 스크립트를 먼저 사용한다.

## G08. 2026-10-01 Release 저장 캡처 조사

사용자 재부팅 후 최신 캡처는 `Client/Bin/ProfilerCaptures/베른성_20261001_012336_905_frame369_32536_0.json`이다.
Release/RTX4070/3840×2126, 120frames 중 GPU valid116/pending4, dropped0이다.
저장 시점 foreground, 사용자 FPS 제한0, Shadow OFF/SSAO ON/Bloom OFF/FXAA ON이며 설정은 변경하지 않았다.

| 캡처 | 화면 크기 | 평균 frame | GPU NonBlend | GPU Lights | CPU Present |
|---|---|---:|---:|---:|---:|
| 00:57 릴리즈_4k_성내부 | 3840×2160 | 34.394ms | 10.513ms | 19.649ms | 23.815ms |
| 01:02 릴리즈_도서관 | 2560×1440 | 48.933ms | 15.202ms | 3.488ms | 0.794ms |
| 01:23 베른성, 재부팅 후 | 3840×2126 | 25.579ms | 15.434ms | 4.818ms | 12.301ms |

최신 평균은39.094FPS, p9526.982ms, 최대28.738ms, GPU25.596ms다. CPU25.322ms 중
Present12.301ms를 제외한 구간은 약13.022ms다. 이전 캡처와는 camera/해상도/visible/lightdraw가
다르므로 재부팅이나 변경 코드의 개선율을 계산하지 않는다. 특히 도서관은4K 캡처가 아니다.
도서관 GPU frame49.674ms에는 계측된 top-level scope 바깥27.208ms가 있어 이를 전부 shader
연산으로 해석하지 않는다. 당시 별도 Debug shader build와 여러 실행 프로그램이 있었으나
개별 영향은 통제된 A/B로 측정하지 않았다.

## G09. 실제 맵 draw·LOD·ImGui 비용

최신 캡처는 전체 draw1,066회 중 instanced789회, light93회를 포함한다. Map.Batch.Render357회가
submesh 순회로 Map.Batch.Draw670회를 제출한다. 따라서 전체 draw를 고유 mesh 개수로 부르지 않는다.
등록 map placements50,021/map batch16,424/fallback1,222는 보유량이다. visible counter564도
fallback의 pass별 제출이 포함될 수 있어 고유 placement564개로 단정하지 않는다.
120frames 모두 visibility cache hit15,794/rebuild0/upload0이며 candidates0은 캐시 재사용 결과다.

전체 indices 약307.7만, VS 약125.1만, PS 약1억1,460.6만이다. NonBlend PS5,417.4만,
Lights PS2,173.7만이며 GPU NonBlend가 전체 GPU의60.3%다. CPU map material bind1.739ms,
pass apply0.385ms, Draw 호출0.166ms와 비교하면 단순 CPU draw 호출보다 GPU 불투명 재질·픽셀
처리가 우선 조사 대상이다. PS 횟수만으로 순수 overdraw·shader instruction 수를 확정할 수 없고,
material별 GPU attribution이나 동일 camera 해상도 A/B는 없다. LOD available/0/1/2 draw는
모두0으로 계측된 geometry 절감은 없지만 개별 admission 탈락 사유나 LOD 고장을 확정하지 않았다.

ImGui CPU NewFrame0.088ms + BuildAndSubmit0.241ms, GPU0.0135ms, draw4회다.
Profiler.Panel.Refresh는6frames에서 호출당1.087ms/전체 frame 평균0.054ms이며 부모에 포함된다.
F7이 열린 측정이므로 창을 닫은 상태나 OS background의 실측으로 대신하지 않는다.
소스상 tool build는 열린 창에서만 수행하며, ImGui frame/backend 자체는 별도 기본 비용이 있다.
제품 Party/Chat 업데이트 scope를 `UI.Runtime.Party.Update`/`UI.Runtime.Chat.Update`로 정정했다.

최신 캡처에는 Picking readback/bytes와 navigation query가 모두0이다. 이 구간은 재클릭 증상의
재현 증거가 아니다. 도서관 캡처는 Picking.MapWait27회/평균5.924ms/최대41.083ms였고,
navigation6회/평균0.020ms로 해당 입력에서는 GPU readback 대기를 먼저 줄일 근거가 있었다.
`PlayerController`는50ms 재전송 간격 안의 hold frame에서 피킹을 생략한다. 첫 클릭과 새 두 번째
물리 클릭은 즉시 exact pixel을 읽는다. 피킹 전49ms/피킹 후51ms였던 hold는 다음 frame로 늦출
수 있으므로 전송 시점까지 완전 동등하다고 주장하지 않는다.

Profiler JSON에는 export 시점 창/process foreground, minimized, 전경·배경·선택된 FPS 제한을
추가했다. 최신 캡처에서 새6필드를 확인했으며0은 checkbox에서 비활성인 제한을 뜻한다.
별도 minimized message wait와 과거 모든 frame의 상태는 이 필드로 측정하지 않는다.
G11 일반 local-light clip은 미반영이며 렌더 옵션·mesh·shader·LOD 품질 데이터는 수정하지 않았다.

분석 수치와 재생 fixture는 `out/RebootCapture20261001/bern_012336_analysis.json`,
`out/MovementAudit20261001/reclick_probe.cpp`와 `reclick_probe.jsonl`에 보존했다.

## G10. 저FPS 재클릭 보정 수정

일정40FPS, Server30Hz, 같은 방향150ms 간격 두 클릭, 편도0/25/50/100ms를 실제 helper로
재생했을 때 기존 코드에서도 RESET은0이었다. 입력만으로 위치를 직접 바꾸는 Server 경로는
없었으며, Bern Client/Server navigation16개 파일의 SHA256도 일치했다.
100/350/1000ms stall을 포함하면 projection과 residual 합산이 정상 속도의2배에 도달했다.

`CLocalMovePrediction::Update`는 최종 XZ 표시 이동을1.15×replicated speed(속도0의 정지 보정만1m/s 대체값)와
Engine frame delta를 한 번씩 소비하는 presentation clock의 frame budget으로 제한한다. ACK에서 시작한 보정 목표는 유지하며 각 frame의
남은 차이를 계속 따라간다. 같은 now의 재호출은 예산을 추가하지 않는다. pending local path는
기존 Character/NavPathFollower가 소유하며 기존0.1초 cap을 유지한다. 보정까지 고정100ms로
자르면 지속10FPS 미만에서 위치가 계속 밀리므로 보정 경로에는 그 cap을 두지 않는다. Server 속도나 명령·
snapshot 형식은 변경하지 않았다. genuine teleport/forced state/10m 초과 시각 오차 RESET은 유지한다.

350ms stall 후40FPS의 최대 한 frame 이동은0.1475m에서0.084813m로 줄고 RESET은0이었다.
속도 여유를 줄인 만큼 긴 stall 뒤 수렴은 느려진다. speed2.95m/s에서 약2.98m가 밀리면
최대 catch-up 여유는0.4425m/s이며 회복에 약6.7초 이상 걸릴 수 있다. 실제 화면 검증은 남았다.

Bern의 실제 합법 이동은 XZ0.0983353m/Y0.9999619m인데 기존 helper의3D 한계0.8483334m를
넘어 RESET이었다. 새 `CNavigation::Is_GroundedSegmentContinuous`는 양끝 동일 grid/layer,
실제 지면 Y와1mm 이내 일치, 기존 segment의 blocker/maxStepHeight를 확인한다.
Character는 할당 없는 callback으로 연결한다. helper는 snapshot 순서 검증 뒤, 기존3D 검사만
초과하고 XZ는 한계 안인 경우에만 이 증명을 요청한다. 실패·다른 층·수평 teleport는 RESET이다.
일반 경로에 A*나 새 이동 경로는 추가하지 않았다. 이 지형 결함이 사용자가 본 위치와 같은지는
좌표·입력 시점 로그가 없어 확정하지 않았다.

## G11. 후속 검증 증거와 실행 경계

- 기존 `ClientPresentationPrimitiveContractTests.cpp`에 총 표시 이동/이동 중 수렴/정지 수렴/
  same-now/ground proof/순서 거부/실제 teleport 회귀를 추가하고 native Debug/Release 모두 PASS.
  지속5/8FPS 각각16초에서 RESET 없이4초 이후 오차0.25m 미만을 유지했다. 고정100ms cap의
  이전 후보는 새 지속 저FPS 검사에서 실패하여 그 후보를 적용하지 않았다.
  0.5m/s 감속40FPS200frames는 매frame0.014375m 상한을 지키고, speed0의0.3m 오차는0.3초 안에 정지 수렴했다.
  원본 header에 새 이동 budget 검사를 붙인 대조 실행은 해당 검사에서 실패해 기존 결함을 검출했다.
- 실제 새 helper + Character callback 원문 + Navigation query 원문 + 실제 NavGrid.cpp 통합17검사 PASS.
  설치 Bern1mstep, 겹친 층, detail/base 경계, 공중, 수직 teleport, 중간 blocked cell과 revision 보존을 확인했다.
- 실제 Navigation.cpp, Character.cpp와 PlayerController/ProfilerCaptureIO/ProfilerTool/MainApp까지
  총6개 CPP의 독립 Release 최소 컴파일 PASS. 제품 링크나 EngineSDK 배포를 대신하지 않는다.
  기존 header 인코딩 경고 C4819/C4828은 남으며 요청과 무관한 파일은 재인코딩하지 않았다.
- 피킹 gate9,600개 조건과 첫/빠른 재클릭/hold/deadzone/실패 상태 재시도 검증 PASS.
  실패는 실제 네트워크 장애 주입이 아닌 commit 이전 반환과 미갱신 상태 fixture 검사다.
- 최신 profiler JSON의 새6필드와 scope 이름 일치 확인. 소유 C++ BOM/CRLF 보존 및 전체 diff-check PASS.

증거는 `out/MovementAudit20261001/primitive_{debug,release}.log`,
`out/MovementAudit20261001/budget_mutation.log`, `out/MovementAudit20261001/client-compile/Character.log`,
`out/RebootCapture20261001/focused/validation.json`,
`%LOCALAPPDATA%/Temp/lostark-prediction-server-20261001/integration_probe.cpp`,
`out/MovementAudit20261001/ground-integration.log`에 보존했다.
검토 시점 Client/Server 실행 중으로 표준 Release Product build의 최종 교체를 위해 사용자 종료를
요청했다. Client 자율 실행·조작은 하지 않았다. 실제40FPS 재클릭 화면과 변경 후 profiler의 FPS
검증은 사용자가 수행하며, 기존 캡처를 변경 후 성능이나 증상 해결 증거로 쓰지 않는다.

## G12. 추가 발탄4K 캡처

`발탄_레이드_4k_20261001_014417_488_frame863_11668_0.json`은10,236,403bytes,
SHA256 `dc8ecbeec3ad6dbea3ccac1c19619443c7f5270200fd3ffd0e725c9b6c1cc2da`다.
120frames744–863/GPU valid116/pending4, dropped0, 상세CPU 계측OFF다. 저장 시점은
Release/RTX4070/3840×2160/foreground/FPS 제한0이며 Shadow ON/SSAO OFF/Bloom OFF/FXAA ON이다.
실행 Client PID11668은이번 prediction 수정 제품 빌드 이전 바이너리다.

| 시간 순서의 구간 | frame 평균 | 전체 draw 평균 | visible 평균 | CPU NonBlend | GPU NonBlend | GPU Blend |
|---|---:|---:|---:|---:|---:|---:|
| 744–773 | 17.259ms | 1,034 | 1,138 | 5.148ms | 9.499ms | 3.354ms |
| 774–803 | 17.658ms | 1,901 | 2,505 | 8.718ms | 9.194ms | 3.531ms |
| 804–833 | 21.002ms | 3,045 | 4,557 | 12.718ms | 11.769ms | 4.083ms |
| 834–863 | 30.299ms | 572 | 419 | 2.581ms | 11.471ms | 16.299ms |

전체 평균21.554ms(46.39FPS), p9539.977ms, 최대43.376ms다. 후반30frames는33FPS 수준이며
해당 GPU 유효26frames 평균33.463ms다. table은 안정 장면 A/B가 아닌 시간 흐름이며 GPU 평균은
각 구간 유효 표본만 사용한다. 카메라가 변하며 visibility rebuild4,036/cachehit0이 매frame 관측된다.

GPU 최악 frame848은43.638ms 중 Blend25.719ms, Blend PS523,881,920회/VS6,959회다.
이때 전체 draw454/map visible329/Blend submissions17, CPU NonBlend1.628ms/Present29.652ms다.
중간 구간보다 맵 draw가 줄었는데 후반이 더 느리므로 이 구간은 메시 개수보다 투명 렌더링의
픽셀 작업이 강한 병목 근거다. `CRenderer::Render_Blend`는 전체 BLEND object queue를 그리므로
물·투명 맵 표면·이펙트 중 어느 occurrence/asset이 원인인지 현재 GPU scope만으로 확정하지 않는다.
중간804–833 구간의 CPU 맵 제출 비용까지 없다고 주장하지 않는다.

전체 GPU 평균 NonBlend10.449/Blend6.490/Lights2.795ms다. Shadow는 static caster5,207개
등록량과 달리 cache hit120/120이며 GPU0.088ms다. scene color copy는6–10회/평균425.8MB,
GPU0.856ms이고 frame848은8회/530,841,600bytes다. ImGui CPU NewFrame+BuildAndSubmit0.337ms,
GPU0.0247ms/draw4회로 주요 하락 원인 근거가 없다. Picking readback/navigation query는0이다.

베른은 GPU NonBlend15.434ms가 중심인 반면 발탄 후반은 Blend가 중심이다. 두 장소의 체감40FPS를
동일 원인이나 PC 성능 보정·40FPS 제한으로 해석하지 않는다.01:46:20 별도 live nvidia-smi는
전체 GPU100%, VRAM10,318/12,282MiB, graphics2790MHz,57°C를 보여줬다. 캡처 이후 단일 시점이며
Client 단독 귀속이나 캡처 당시 사용률로 쓰지 않는다.
분석은 `out/RebootCapture20261001/valtan_014417_analysis.json`, 보조 샘플은
`out/RebootCapture20261001/gpu_live_014620.json`에 보존했다. 이번 발탄 추가 조사에서 렌더 소스·
품질 옵션·설치 데이터는 변경하지 않았다.

## G13. ACK 적용 위상과 최종 presentation clock 수정

사용자는60FPS에서도 빠른 동일 방향 재클릭 때 끊김을 보고했다. 실제 프레임 순서를 유지하고
Character Update 뒤 snapshot 적용까지8/12/15ms가 걸리는 조건을 추가하니 기존 helper는
60FPS/30Hz에서 속도가 각각0.6842↔1.3158,0.4375↔1.5625,0.1818↔1.8182배로 반복했다.
일반 FPS만 고정한 초기 재생은 이 처리 시간을 포함하지 않아 이 결함을 검출하지 못했다.
총 속도 상한만 추가한1차 후보도 늦은 ACK 위상에서 빠른 frame만 제한하여 지속 지연이 생겼다.

최종 `LocalMovePrediction.h`는 `m_receivedAt`/RTT/timeout을 wall clock으로 유지하며,
`m_presentationSeconds`/`m_snapshotPresentationAt`/`m_correctionPresentationAt`을 별도로 둔다.
Engine delta는 새 Update 시각마다 한 번만 누적한다. 같은 시각의 재호출은0초다.
snapshot projection과 보정 residual은 같은 presentation 시계에서 시작하므로 ACK 적용이
Object Update보다 늦어도 다음 frame 전체를 진행한다. ACK 직후 동일 시각 조회는 최신 yaw와
이동 상태를 반환하되 표시 위치와 frame 시계는 유지한다. Server simulation·packet·Engine 호출 순서는 그대로다.

최종 실제 제품 header와 영구 회귀의 Debug/Release 실행을 모두 통과했다.60FPS의
0/8/12/15ms ACK 위상과1/3/6ms Object jitter에서 steady 속도0.99998–1.00002배/RESET0,
누적 지연이 없었다.40FPS는30Hz 양자화의 잔여 가감속이 있지만 위상과 무관한 평균 속도로
수렴했다. 원본과1차 속도 상한 후보는 새 영구 phase 검사에서 각각 실패했다.
지속5/8FPS·감속·정지·pending 만료·실제 teleport·같은 frame 두 번 Update·ACK 즉시 위치
보존 검사도 포함한다. 실제 Character.cpp 최종 Release 최소 컴파일을 다시 통과했다.

짧은 다음 waypoint 밖을 임의 외삽하지 않으므로 코너 직전의 감속까지 제거하지는 않았다.
1m 간격90도 코너 fixture는60/40FPS 모두 RESET0, 최대 위치 오차 약0.193/0.205m였다.
현재 캡처의 Player layer 시작→Replication 시작은 Bern01:23 평균0.658ms/최대0.850ms,
Valtan01:44 평균0.785ms/최대1.568ms이고, 이전 도서관은평균2.883ms/최대50.129ms다.
이는 상위 scope 경계이며 정확한 prediction/개별 snapshot 적용 시각이 아니다.8/12/15ms
조건을 사용자의 실제 입력 순간 실측으로 바꾸어 설명하지 않는다.

사용자가01:58–01:59 직접 수행한 Engine→Client Release 빌드는1차 소스를 포함했다.
Character.obj01:58:24의 read tlog에서 LocalMovePrediction.h 의존성을 확인했고 Engine SDK의
새 Navigation API도 배포됐다.01:59:57 Client.exe/PID23052에는 이 최종 frame clock 수정이
포함되지 않았다. 그 실행에서 사용자가 보고한 끊김은 최종본 재현 판정이 아니다.

최종 증거: `out/MovementAudit20261001/movement_budget_validation.json`,
`ack_phase_original.jsonl`, `ack_phase_speedcap_only.jsonl`, `ack_presentation_clock.jsonl`,
`capture_phase_bounds.json`, `client-compile/Character.log`. 새 최종 제품 빌드와 사용자 화면 검증은 별도다.

## G14. Source BG의 불필요한 sample과 Effect 중복 복사 제거

`Client/Bin/ShaderFiles/Shader_MapMaterialSurface.hlsli`의 두 재질 공통 flag를 명시적인
`[branch] if`로 변경했다. detail normal OFF는0, specular texture OFF는 diffuse.rgb를 유지하며
ON 수식·UV·sampler·재질 입력·coverage는 그대로다. 실제 PS_MAIN_SOURCE_BG의 FXC /O1
DXBC는 변경 전 t3/t2 sample 뒤 movc, 변경 후 if_nz 내부 sample임을 확인했다. instruction
slot343→347이므로 instruction 개수 감소나 GPU ms 개선을 이 검사로 주장하지 않는다.
설치 Bern BG13,228 material slot 중 normal ON/detail OFF12,979개, specular 계산에서
texture OFF3,866개가 해당하지만 이는 실제 frame의 가시 픽셀이나 GPU 시간 비율이 아니다.

`CEffectObject::Submit_RenderGroups`의 particle scan과 초기 scene snapshot 요청23줄을
삭제하고 occurrence 소비 계약 주석3줄을 남겼다. NORMAL/WORLD_MARK의
Render_CompositionPhase는 live scene-color를 읽는 occurrence의 afterimage/element/particle/trail
보다 먼저 refresh한다. SourceMaterialSlots의 scene 요구는 부모로 합쳐지고 fallback blocked/
suppressed는 draw도 생략한다. ModelCue의 deferred/masked는 live snapshot을 바인딩하지 않으며
frozen capture는 별도 실제 HDR/Bloom target을 사용한다. Map/World 요청, occurrence별
HDR/Bloom pair, 합성 순서는 바꾸지 않았다. 기존 cached staging metadata와 헤더 layout도 보존했다.

Effect만 초기 복사를 요청하던 frame에서는4K 전체화면 복사2회/논리 payload132,710,400bytes를
줄일 수 있다. Map/World가 요청한 frame은 초기 복사를 유지하므로 매frame 절감을 확정하지 않는다.
이는 발탄 최악 Blend25.719ms 전체를 해결한다는 뜻이 아니다. 더 큰 Bloom OFF 재평가 생략은
native material의 scene-dependent clip 동등성이 입증되지 않아 적용하지 않았다.

실제 Effect_Object.cpp 독립 Release 컴파일은 VS18 Insiders/MSVC14.44.35207/SDK26100,
/O2 /MD에서 exit0, object2,619,083bytes로 통과했다. 기존 include의 C4819만 남았다.
BOM 없는 UTF-8/CRLF와 다른 소비자11개 파일 hash 불변, diff-check를 확인했다.
`out/RebootCapture20261001/effect_snapshot/validation.json`, `Effect_Object.log`,
`Effect_Object.rsp`, `compile.cmd`가 검증 근거다. GPU 출력 비교·최종 Client 링크·화면·성능은
여기에 포함하지 않는다. 사용자가 최종 소스로 직접 Release를 빌드하고 화면을 확인하기로 했으며
실행 중인 Client/Server를 에이전트가 종료하거나 조작하지 않았다.
## G15. 추가 쿠크 레이드4K 캡처

`쿠크세이튼_40fps_20261001_021554_984_frame258_23052_0.json`은16,963,707bytes,
SHA256 `4edae40cd20b0c2487ffbe8b0f8226163c4ed86590bd00c97023fddbf50be0a2`다.
Release/RTX4070/3840×2126/foreground/전경·배경 제한0, 저장 시점 Shadow/SSAO/Bloom/FXAA는
모두 OFF다. 설정 metadata는 export 시점이며 과거 전체 frame 상태를 대신하지 않는다.
120frames139–258의 평균33.2796ms(30.05FPS), p9538.4931ms, 최대39.1254ms다.
GPU는139–254의116개만 유효하고 마지막4개는 pending이며 scope drop/partial은 없다.

| 시간 순서 구간 | frame 평균 | GPU 유효 수 | GPU 평균 | NonBlend | Lights | Blend |
|---|---:|---:|---:|---:|---:|---:|
| 139–168 |35.226ms|30|35.807ms|12.531ms|10.432ms|11.210ms|
| 169–198 |37.187ms|30|36.832ms|13.173ms|11.035ms|11.019ms|
| 199–228 |30.557ms|30|29.774ms|12.600ms|11.633ms|3.965ms|
| 229–258 |30.149ms|26|30.568ms|12.619ms|14.397ms|1.986ms|

전체 GPU33.3378ms 중 NonBlend12.7345/Lights11.7873/Blend7.2196ms 합계는95.2115%다.
후반 Blend가 줄어도 조명 비용이 증가해 약30ms가 남는다. CPU32.7583ms 중 Present20.9683ms를
제외하면11.7901ms다. GPU timestamp는 제출 공백을 포함할 수 있지만 이 시간 분포와 PS 작업량은
GPU 중심 병목의 강한 근거다. 고정 camera의 A/B나 해상도별 비교로 취급하지 않는다.

전체 draw 평균668과 맵 가시 인스턴스53–63/map submesh draw50–55를 구분한다.
VS 전체 약55.3만에 비해 전체 PS 약6.3150억, 조명 PS만 약3.8996억이다. 조명 record660.4개는
StageAndSubmit/UploadAndDraw52회 반복 제출에서 합한 값이며 고유 조명 수가 아니다.
light draw208은 매frame 같고 local candidate608.4/rejected0도 반복 소비된 분모다.
메시 개수만 늘어서30FPS라는 판정 대신 불투명 재질·조명·투명 픽셀 작업을 각각 조사한다.

ImGui는 CPU NewFrame0.124042+BuildAndSubmit0.261885=0.385927ms,
GPU backend0.010596ms/draw4다. backend와 같은 RenderDrawData scope를 중복 합산하지 않는다.
SceneColorCopy는 평균2.133회/GPU0.225695ms이고 마지막30frames는0이다. picking/navigation은0으로
이번 창을 빠른 재클릭 재현으로 취급하지 않는다. GPU 최악frame189는39.240832ms 중
NonBlend13.5024/Lights11.11552/Blend12.887232ms였다.

QPC→현재 wall clock anchor 환산의 근사 구간은02:15:50.979–54.974이며 isolated FXC 검증의
02:14:49 이후 실행과 겹친다. CPU 경합 가능성을 남기고 컴파일 없는 정상 baseline으로 확정하지 않는다.
PID23052의 시작01:59:58/EXE01:59:57.600은 최종 prediction02:03:48과
Effect snapshot02:13:18 수정 전이다. 따라서 수정 후 FPS나 끊김 해결 증거가 아니다.
분석·분모·한계는 `out/RebootCapture20261001/kouku_021554_analysis.json`에 보존했다.
이번 추가 캡처 분석으로 렌더 옵션·품질 데이터나 제품 소스를 추가 변경하지 않았다.
쿠크 조명 소비자를 추가 확인했다. Renderer는 ordinary pass 이후 이번 view에 등록된
CMaterial row마다 전체 light 목록을 다시 제출한다. 단일 view라면52회는 ordinary1+source51과
일치한다. row는 program 종류가 아닌 CMaterial 객체/입력별 단위이며 미등록 prototype을
일괄 순회하는 경로는 없다. Shader_Deferred는 depth marker/row ID를 normal·world reconstruction·
shadow보다 먼저 검사한다. 따라서 PS3.8996억을 모두 비싼 source shading 완료 횟수로 해석하지 않는다.
다만 row 등록은 GPU 가림/alpha 결과 전이므로 최종 픽셀이0인 제출 재질을 별도로 제거하지 않는다.
공통 source stencil은 있지만 row별 coverage 제한은 없다. 후속 방향은 이미 있는 early discard의
중복 추가가 아닌 row별 영역과 재제출 감소이며, 이번 소스에는 그 구조 변경을 적용하지 않았다.

## G16. 최종 소스 검증과 사용자 빌드 인계

Source BG의 실제 전후 PS 컴파일2회와 전체 Effect5개(MapInstance 및 Anim source group
001/009/017/025)는 PASS했다. 기본 Anim/Mesh의 전체 Effect는 변경하지 않은 모든 native program까지
포함해 최적화하는 별도 큰 검증이므로 task-owned FXC2개만 종료하고
`ABORTED_AS_OUT_OF_SCOPE_FULL_NATIVE_COMPILE`로 기록했다. 미시작52개와 구분하며59개 전체
컴파일 성공으로 기록하지 않는다. Release include tlog의59개 dependency 연결은 별도로 확인했고
프로젝트 /T fx_5_0 기본 optimization1과 검증 /O1이 일치한다. 최종 normal Release Product Build가
이59개 전체의 빌드·설치 검증을 담당한다. 실제 실행한 명령·종료코드·hash는
`out/MovementAudit20261001/source_bg_uniform_branch/audit-summary.json`,
`compile-receipt.json`, `branch-dxbc-snippets.json`, `release-include-tlog-evidence.json`에 남겼다.

최종 이동 native Debug/Release 회귀, navigation 통합17개와 변경 CPP7개의 독립 Release 최소
컴파일을 통과했다. 제품 소스 SHA256은 `out/MovementAudit20261001/final-source-manifest.json`에
기록했다. 최종 이동 clock·Shader_MapMaterialSurface·Effect_Object 소스 반영과 전체 diff-check를
확인했다. 수정 시점 이후 Client.exe 링크·GPU 화면 동등성·변경 후 FPS·사용자 이동 화면은 아직
확인하지 않았다. 사용자 요청에 따라 이후 최종 제품 빌드와 직접 실행은 사용자가 수행한다.
검증용 FXC는 종료했고 Client/Server 실행·종료·UI 조작은 하지 않았다.

## G17. 지속 재클릭과 독립된 Server clock·연속 이동

G13의 frame clock 수정은 유지하되 MOVE ACK RTT를 projection lead로 쓰던 의존성을 제거했다.
별도 읽기 검토에서 60FPS/150ms 지속 연타는 여전히 속도 0.924~1.084배가 반복됐으며,
첫 ACK lead를 고정한 진단도 40FPS의 수신 위상 문제를 해결하지 못했다. 첫 ACK pin만
적용하는 후보는 채택하지 않았다. G19 구현은 입력 명령, Server 시간 추정, frame 이동 소비를 분리한다.

`LocalMovePrediction.h`는 Server tick과 수신 시각의 offset을 관찰하고 Engine delta로
표시 시간을 한 번씩 진행한다. 현재 frame보다 앞선 최신 snapshot은 이전 sample과 같은
Server 시점으로 맞춘다. 수신 batch나 MOVE ACK 수가 clock을 추가 진행시키지 않는다.
수신 window의 최소값과 tick 단위 관측 불확실성을 사용하며 실제 지연 상승·하강에는
제한된 slew로 적응한다. 절대 one-way latency 측정은 아니다. 첫 양수 frame의 anchor는
초기 snapshot 조회 후 한동안 Update가 없던 5/8FPS fixture의 영구 시간 지연도 막는다.

입력은 목표와 sequence를 갱신한다. 동일 목표는 Character의 기존 path와 최초 pending
path 시간을 보존하고, 다른 거리의 같은 직선 목표도 남은 correction을 초기화하지 않는다.
`Update`→기존 follower의 허용 delta→`CompleteLocalPathFrame`으로 ACK 전후 모두 같은
frame budget을 소비한다. actual turn이 승인되기 전에는 이전 segment의 residual을 새 방향에
덧붙이지 않는다. known Server corner는 기존 waypoint를 먼저 지난 후 다음 segment로 진행한다.
기존 1.15배 총 XZ budget, 350ms freshness, 실제 teleport·forced state, 입력 거부와
navigation-proven 지면 연속성 검사는 유지했다. 프로토콜과 Server simulation은 변경하지 않았다.

### G17-1. native 자동 검증

기존 `ClientPresentationPrimitiveContractTests.cpp`를 복사본 없이 직접 컴파일한 Debug
(`/Od /MDd /RTC1`)와 Release (`/O2 /MD /DNDEBUG`) 전체가 PASS했다. 새 파일 등록은 없다.
명령은 `cmd /c out\MovementAudit20261001\run_primitive_debug.cmd`와
`cmd /c out\MovementAudit20261001\run_primitive_release.cmd`다.

| 검증 | 결과 |
|---|---|
| 40/60FPS × 수신 phase 0/8/15ms × 5 scenario | 30 trace PASS |
| scenario | 동일 목표 / 다른 거리의 동일 직선 / 지연 변화·batch / 90도 Server turn·body block / 다른 거리+지연·batch |
| 150ms 연타 대 single-click, 8초의 모든 frame | 최대 표시 위치 차이 0m; 클릭·ACK 직후 포함 |
| 2.0~2.5초 steady 절대 속도 | 40FPS 약 0.999994~1.000000배, 60FPS 약 0.999991~1.000000배 |
| steady 속도 회귀 조건 | 두 trace 동등성과 별도로 0.995~1.005배 Require |
| 지연 변화 | 전달 지연 25→225→25ms, 12ms jitter, 220ms packet batch |
| 실제 90도 입력·old ACK | 이전 X residual의 새 Z 경로 사선 유도 없음 |
| 알려진 corner·retarget | waypoint 경유, 첫 local frame 전/후 반대 목표 ACK의 오래된 cache 폐기 |
| tick wrap | UINT_MAX→1과 일반 tick trace 동등 |
| 기존 회귀 | 5/8FPS 수렴, 감속·속도0 정지, budget, 동일 frame 재호출, timeout·실제 teleport·지면 proof PASS |

로그는 `out/MovementAudit20261001/primitive_debug.log`, `primitive_release.log`,
집계·소스 hash는 `continuous_reclick_summary.json`에 보존했다. 기존 header를 유지하고
새 테스트 API만 연결한 out 전용 대조 실행은 40FPS/phase0의 첫 재클릭 직후 frame7에서
single과 0.0134248m 차이로 실패했다. `continuous_reclick_original.log`와
`continuous_reclick_original/LocalMovePrediction.h`에 원본 동작과 adapter를 보존했다.
최종 scoped diff-check, UTF-8(BOM 없음)/CRLF 유지도 확인했다.

### G17-2. 제품 반영과 남은 경계

위 native 검증은 실제 Bern 화면이나 최종 제품 링크의 성공을 대신하지 않는다. 사용자 화면
재현, 실제 nav/body 접촉의 시각 품질, 최신 profiler의 GPU 비용은 별도 판정이다. Client UI는
자율 실행·조작하지 않았다. 제품 Debug/Release 빌드는 이 native 소스 동결 뒤 별도로 진행한다.

동결 뒤 읽기 검토가 찾은 fast-ACK corner edge는 out 전용 후보로 재현했다. 새 목표 전송과
다음 local frame 사이에 ACK가 먼저 오고 Server가 이전 corner를 되짚어 가면 frozen 소스는
X0.018→0.036으로 기존 corner 쪽에 한 번 되돌아간다. 이전/새 Server segment의 방향이
달라진 cache를 폐기하는 후보는 X0.018→0.0012로 새 목표 쪽으로 간다. 각각 exit1/0이며
`corner_fast_ack_probe.cpp`, `corner_fast_ack_{frozen,candidate}.log`에 남겼다.
제품 빌드가 frozen 소스를 읽는 동안에는 out 후보만 검증했다. 해당 빌드 종료 후 기존 hash를
확인하고 이전/새 segment 방향 검증 3줄만 실제 Header에 반영했다. 첫 local frame 전 ACK를
영구 회귀에 추가한 최신 source Debug/Release가 모두 exit0/PASS이며 제품 최종 증분 빌드는
이 source 재동결 뒤 별도로 진행한다. 최종 Header SHA256은
`5bea248c108feb4bf74ca1115640d42851312cb0447b2b9f048f0502c7238c70`이다.

## G18. 최종 Debug/Release 제품 링크

이동 보완과 모든 최신 Client/Server 소스를 포함한 정상 Product Debug/Release가 모두 성공했다.
앞선 EXE 잠금·최종 링크/제품 빌드 대기는 해소됐으며 실행 파일과 실제 로그는
`../09-27/2026-09-27_GUIDE_AI_TOOL_IMPLEMENTATION_RESULT.md`의 G09에 기록했다.
이 결과는 기존 기능별 검증을 대체하거나 실제 Client 화면·다인 플레이·성능 확인으로 확대하지 않는다.

## G19. 2026-10-01 Debug 이동 캡처와 과거 Release의 독립 재검토

사용자는 실행 중인 Debug 약20FPS 이동이 의도대로 움직이며 Release의60FPS 아래 이동과
다르다고 보고했다. 이번 검토는 소스·기존 캡처·실행 파일·컴파일 추적을 읽었으며 제품 코드,
렌더 옵션, 실행 중 Client/Server와 통합 worktree를 변경하지 않았다. 새 빌드나 화면 검증도 하지 않았다.

### G19-1. 실제 비교 대상과 빌드 시점

현재 Client PID16728은 `Client/Bin/Debug/Client.exe`(05:39:05), 로드된 Engine은
같은 Debug 폴더의 DLL이다. Server PID57648도 Debug이며 실제 TCP 연결은
`192.168.0.22:7777`이다. Debug 전용 LocalServerEndpoint.user.json 분기가 존재하지만
이번 실행은127.0.0.1을 사용하지 않으므로 현재 Debug가 별도 loopback 경로라는 설명은 배제한다.
과거 Release의 실제 연결과 같은 서버였다는 사실까지 이 현재 socket으로 확정하지 않는다.

06:50 Debug 캡처는 PID16728, D3D debug layer ON,1920×1080이다. 비교한01:02 Release
도서관 캡처는 PID28900, debug layer OFF,2560×1440이다. Export metadata는 마지막 시점만
설명한다. Release 기록은 G13/G17 이동 시계 수정 전이고, Debug05:39와 설치 Release05:40은
수정 후다. 따라서 두 캡처를 동일 소스·동일 장면의 구성 A/B로 해석하면 안 된다.
현재 저장된 Release 캡처 중05:40 이후 기록은 확인하지 못했다.

실제 Character/PlayerController/ClientReplication CL command tlog는 Debug
`/Od /RTC1 /MDd /D _DEBUG`, Release `/O2 /Oi /GL /MD /D NDEBUG`이며 양쪽
`/fp:precise`다. 현재 LocalMovePrediction.h SHA256은 G17 최종값
`5bea248c108feb4bf74ca1115640d42851312cb0447b2b9f048f0502c7238c70`과 같다.
일반 이동·보정 수식에 구성별 분기는 확인되지 않았다. 최적화가 수식을 잘못 계산했다는
증거도 없다. 기존 양쪽 native 검사 결과는 실제 실행 시각의 동등성을 보장하지 않는다.

### G19-2. 관측된 시간 차이와 인과 경계

| 120frame 캡처 | Debug 이동06:50 | Release 도서관01:02 |
|---|---:|---:|
| 평균 FPS |18.96|20.44|
| frame interval 평균 / p95 / 최대 ms |52.738 /56.157 /58.651|48.933 /91.713 /132.401|
| frame interval 표준편차 ms |1.762|23.116|
| Picking.MapWait 호출 수 |63|27|
| MapWait 호출당 평균 / 최대 ms |0.210 /1.098|5.924 /41.083|

63/27은 readback 호출 수이며 별도의 물리 클릭 개수로 단정하지 않는다.
현재 호출 순서는 delta 측정 → network drain → Character/Object 이동 → Level snapshot 적용
→ Controller picking → LateUpdate camera → render다. 두 캡처의 모든 MapWait도 replication
이후다. 같은 frame의 이동을 계산한 뒤 GPU readback이 주 스레드를 막으면 표시가 늦어지고
그 대기가 다음 frame delta에 들어간다. 이것은 멈춤·뒤따르는 큰 이동의 구체적인 경로다.
Debug의 CPU 작업과 D3D 검사로 GPU가 먼저 준비될 수 있다는 설명은 가능한 원리이며,
해상도·장면·빌드까지 다른 두 자료로 그 원인 하나를 분리해 확정하지 않는다.

Release의 모든 긴 정지를 picking으로 설명할 수는 없다. frame7427에는 picking이 없으며
Player layer→Replication 간50.129ms 중 WorldEntity.Update가48.523ms이고 NPC 작업 누계가
48.502ms다. frame7428은 MapStaticBatch.FinalCamera81.690ms,7429는 Lights81.024ms다.
세부 CPU scope가 비활성이므로 이 시간을 asset load나 특정 Map 호출로 다시 이름 붙이지 않는다.
최대 interval132.401ms인7387은 앞선7386 CPU frame81.243ms와 그 밖의 약51.157ms를
포함한다. 이전 CPU frame의 Present48.562ms도 확인된다. frame interval과 같은 번호의
CPU scope를 단순히 동일 시간 구간으로 합산하지 않는다. 계측 바깥 시간을 OS 경합이나
프로파일러 단일 원인으로 확정할 자료도 없다.

### G19-3. 적용 상태와 남은 확인

과거 수식 결함은 G13/G17의 실제 실패·수정 대조 로그로 증명됐지만, 최신 Release에서
같은 결함이 남았다는 근거로 재사용하지 않는다. 별도 통합 worktree
`C:/Users/user/.codex/worktrees/pr494-496-flexible-colosseum/LostArk`에는 요청 ID를 유지하는
비동기 이동 picking과 `Map(DO_NOT_WAIT)`가 구현돼 있다. Desktop의 현재 Debug/Release에는
그 변경이 아직 없으며, 통합본 구현·빌드와 사용자 화면 해결 판정은 별개다.

원인 확정에 남은 비교는 동일 소스 세대·같은 서버·장면·해상도의 Debug/Release 이동 캡처다.
현재 Profiler v3에는 frame별 표시 좌표, snapshot tick/ACK sequence, 보정 전후 좌표와
RESET 사유가 없다. 같은 조건에서도 위치 튐이 남으면 기존 Capture 수명 안에서 이 값을
최소 추가해야 프레임 정지와 실제 위치 보정을 구분할 수 있다. 새 계측은 이번 읽기 검토에서
구현하지 않았다. 수치 근거는 `out/MovementAudit20261001/capture_compare_readonly_0650.json`
및 원본 캡처이며, 핵심 frame별 귀속과 비교 요약은 같은 폴더의
`debug_release_comparison_0650.json`에 있다. 최종 화면 판정은 사용자가 수행한다.

## G21. 2026-10-01 실제 이동 소비의 세 가지 결함 수정

G19 이후 protocol132가 통합된 Desktop 소스에서 별도로 재현했다. 기존 캡처의 GPU 비용과
현재 이동 수식의 결함을 같은 원인으로 단정하지 않았다. 아래 변경은 Debug/Release 공통이다.

- `Character.cpp`의 미승인 이동이 helper의 허용 시간을 다시100ms로 자르는 처리를 제거했다.
  helper의 freshness350ms·외삽150ms·표시 속도115% 상한은 유지한다.
- `NavPathFollower.cpp`의2cm 이내 waypoint 무료 이동을 제거했다. 짧은 마지막 segment도
  frame 이동량에서 실제 거리를 소비하고 도착 전에 IDLE로 전환하지 않는다.
- `GameRoom_PlayerCommands.cpp`의 새 목표 경로를 임시 vector에 검색한다. 실패하면 기존
  경로·진행 index·목표·이동 상태를 보존하며 처리 sequence ACK는 진행한다. 성공 시에만
  새 경로를 commit한다. Client가 Server의 위치·통과 판정을 대신하지 않는다.

### G21-1. 실패와 수정 후 수치

`out/MotionAuditFollower20261001`의 frozen Engine DLL/lib와 실제 CNavigation/CTransform/
CNavPathFollower를 사용했다. 같은 helper에서 기존 follower와 최소 수정 후보를 비교했다.
제품 Client 실행이나 화면 대조 결과는 아니다.

| 재현 | 수정 전 | 수정 후 |
|---|---:|---:|
|2.95m/s,132.401ms, 미승인 경로|0.295m|0.390583m|
|2.95m/s,125ms, 미승인 경로|0.295m|0.368750m|
|40FPS,19mm 앞의 corner, follower 이동|0.092725m|0.073726m|
|0.5m/s,25ms,19mm 앞 도착점|19mm 이동·경로 종료|12.5mm 이동·경로 유지|

corner의 명목 frame 예산은0.07375m다. 수정 후 chord 거리는 회전 때문에 이 값보다 약간 작다.
수정 전 helper의 최종115% 제한이0.084812m로 숨기던 follower 과소/과대 진행까지 구분했다.
16.667/25/50/132.401ms, 같은 목표/retarget/90도 미승인 turn을 대조했다.

### G21-2. 실제 Server 명령과 snapshot 회귀

기존 `ServerGameplayContractTests_Navigation.cpp`와 runner에 `Run_MoveRetarget`를 추가했다.
실제 `Handle_Move -> Update_Players -> Broadcast_WorldSnapshot -> PacketReader`를 사용해
직선/우회 경로 각각 유효seq1 → 불가능한seq2 → 유효seq3을 검사한다. 실패한seq2의 snapshot은
ACK2와 기존 waypoint를 함께 유지하며 다음 simulation step도 계속 진행한다.
Release 제품의 `Server.exe --navigation-contract-test`는 exit0, `navigation failures : 0`이다.
로그는 `out/MovementCpuPick20261001-server-navigation.log`다. listener나 Client UI는 실행하지 않았다.

## G22. 일반 이동 클릭을 같은 frame의 CPU 표면 query로 변경

`PlayerController`의 이동용 GPU request/poll/cancel 상태를 제거했다. 현재 camera ray를
Level의 `CMapPlacementRuntime`에 전달하고 성공한 입력 frame에 기존 typed sink로 제출한다.
새 press는 즉시 새 목표를 평가하고 hold는 기존50ms 재전송·동일 목표 억제를 유지한다.
miss·미준비·실패는 현재 이동을 보존하며 GPU readback이나 임의 평면으로 fallback하지 않는다.
배 이동의 수면 plane과 저작 도구의 별도 GPU picking API는 유지한다.

### G22-1. 실제 연결과 수명

- `CMesh`의 기존 불변 pick geometry에 triangle ordinal/BVH를 prototype당 한 번 만든다.
  기존 model/mesh clone이 공유하고 query에서 정점 복사·asset load·geometry 재구축을 하지 않는다.
- `CModel::Try_PickStaticSurface`는 world affine inverse, 정규화된 ray, world 거리 한계와
  mirror determinant를 사용한다. BACK/FRONT/NONE은 실제 render pass 정책에서 전달한다.
- `MapStaticBatchObject`와 `MapAssetObject`는 현재 world bounds·transform·visible·stage/
  camera suppression을 읽는다. `MapPlacementRuntime`은 현재 load scope의 소유 객체만 모은다.
  별도 placement cache나 두 번째 map runtime은 없다.
- Bern/Valtan/Kouku/CharacterSelect/Development에 resolver를 연결했다. Development가
  Training/Maharaka/Colosseum의 동일 controller를 연결한다. 새 제품 H/CPP가 없어 project와
  filters 항목을 추가하지 않았다. Engine public header는 정상 Product build가 SDK에 배포한다.

정적 LOD0 geometry를 이동용 표면으로 사용하는 계약이다. masked 바닥과 cardmaze receiver를
포함하고 명시 foliage/grass·알려진 shader 변형·morph는 제외한다. shader alpha로 잘린 구멍,
GPU vertex 변형과 화면용 LOD의 픽셀 일치를 보장하지 않는다. Server의 XZ/navigation 층 판정은
그대로 유지한다. 렌더 옵션·해상도·Present·전역 frame timer를 바꾸지 않았다.

### G22-2. 입력·기하 검증

`Tools/MovementRegression/test_cpu_move_dispatch.py`는 실제 production dispatch와 admission
함수를 변경 없이 추출해 native로 컴파일한다. camera/map/typed-send만 대역이며 GPU API
대역은 없다. Debug/Release 모두27검사 PASS다. 같은 frame 제출·빠른 재클릭·release 이후
지연 명령 부재·throttle·miss 보존·NaN·송신 실패·배 이동 및5Level 연결을 확인했다.
실제 gameplay frame 실행이나 화면 성공을 대체하는 테스트는 아니다.

실제 설치 WModel3종(Kouku FLOOR01, Bern 실내 FLOOR01A, Bern FLOOR04)을 읽어 정본
Mesh/Model 함수를 native로 대조한19,200 query는 world-space brute reference와 불일치0,
query heap 할당0이었다. 음수/비균일 scale과 컬링을 포함한다. 화면 없는 D3D11 WARP의
12조합(BACK/FRONT/NONE×mirror×winding) raster 결과와 CPU cull 결과도 일치했다.
근거는 `out/FramePacingAudit20261001/cpu_pick/manifest.json`, `results.txt`, `warp_results.txt`다.

일반 이동의 GPU 완료 의존을 제거한 것이며 Debug/Release의 전체 GPU 실행 시간을 동일하게
만든다는 뜻은 아니다. 전체 게임 지연·GPU 병목·다인 조작감의 최종 판정에는 최신 동일 조건의
사용자 실행이 필요하다. Client/UI는 자율 실행·조작하지 않았다.

### G22-3. 반복 입력 비용과 실제 맵 크기 검증

후속 실사용에서 Map 밖 Deploy carrier 누락이 확인됐다. 발탄 파괴 바닥·난간과 쿠크 종이 다리의
현재 표시 모델/pose를 같은 CPU 이동 resolver에 합성한 수정과 Release 배포 검증은
[Deploy 피킹 결과](../10-01/2026-10-01_DEPLOY_CPU_MOVEMENT_PICKING_RESULT.md)에 기록한다.
아래의 기존 Map/Controller 검사는 이 Deploy 연결 누락을 검출한 근거가 아니다.

hold 표면 검색 시각을 송신 시각과 분리했다. 같은 목표·miss·송신 실패도 다음 hold 검색까지
50ms 간격을 지키며 새 press는 즉시 검색한다. 송신이 없다는 이유로 매 frame 검색하지 않는다.
`MapStaticBatchObject`는 기존 bounds가 유효하고 dirty가 아닐 때만 batch 전체를 먼저 배제한다.
dirty이면 instance 검사로 진행하며 입력 중 bounds 재구축이나 별도 cache를 만들지 않는다.

실제 Bern mapset 50,021 placements / 1,276 unique models / 1,904,503 triangles의 bounds로
256개 ray를 검사했다. 후보 평균24.07·최대127은 변경 전후 같았다. batch bounds 검사 후
Release broadphase p50/p95는0.0693/0.0920ms, Debug는2.1603/3.3926ms였다.
수정 전은 각각0.3131/0.4351ms,12.0124/17.2173ms다. 이 수치는 bounds 구간만 측정했으며
실제 Level callback 순회·삼각형 검색·렌더링을 포함한 게임 frame 시간이 아니다.
BVH 추가 보유량 추정은 unique geometry 전체 약40.05MiB다.

실제 batch query의 visibility·nearest·dirty bounds·실패 출력 보존8조건과 query 할당0도
확인했다. 인위적으로50,021개 mesh를 같은 위치에 겹친 최악 입력은 Release p95 22.91ms,
Debug377.71ms로 비용이 남는다. 이 스트레스 입력을 실제 Bern 배치 성능으로 해석하거나
CPU 병목이 전혀 없다고 결론 내리지 않는다. 근거는
`out/FramePacingAudit20261001/cpu_pick/verification_summary.json`이다.

실제 Character 소비자와 설치 Engine DLL을 연결한 이동 회귀는 Debug/Release 각각
27조건·189검사 PASS다. 근거는 `out/MotionAuditFollower20261001/persistent-debug-final/result.json`,
`persistent-release/result.json`이며 기존 Debug DLL의 짧은 waypoint 결함도 검출했다.
Controller 최종27검사는 `out/CpuMoveDispatchRegression/final-Debug/result.json`,
`final-Release/result.json`에 보존했다.

### G20 후속. Character의 실제 capture 소비 연결

기존 capture ring/export에 `Character.cpp`의 실제 Update·snapshot 적용·local command 제출을
연결했다. 비활성 capture는 표본을 만들거나 추가 pose 조회를 하지 않는다. Frame의 before/after는
Update 전후 표시 위치이고 snapshot의 tick/sequence는 수신 Server tick/처리 ACK다.
Command Accepted는 local prediction 제출 성공이며 Server 승인이나 packet 전달 성공이 아니다.
Frame의 기본 tick/authority는0이므로 QPC상 앞선 snapshot과 연관해 해석한다.
실제 생산 함수 native21검사를 통과했다. 필드 계약과 근거는
`out/MovementCaptureHooks20261001/hooks_contract.txt`, `hook_probe.results.txt`에 있다.

## G23. 2026-10-01 08:23 베른 캡처 기반 조명 행 통합

사용자가 콜로세움 종료·폰트/UI 수정은 다른 채팅에서 진행하고 이 작업은 저장된 베른
Profiler를 기준으로 최적화하도록 범위를 변경했다. 이 G의 제품 변경은
`Engine/Private/Material.cpp`뿐이다. 기존 이동·피킹 변경은 이 G의 성과로 합산하지 않는다.

### G23-1. 저장된 캡처에서 확인한 병목

입력은 `Client/Bin/ProfilerCaptures/베른_33fps_20261001_082343_324_frame214_62428_0.json`이다.
SHA256은 `5dac9aa2e185c9219194da781841306a227a2e881b4adea40a0b630a3ecebab2`다.
Release/RTX 4070, 3840×2126, 120프레임 중 GPU 유효116·pending4이며 scope drop은 없다.
내보내기 시점 metadata의 foreground FPS 제한은0, SSAO/FXAA ON·Bloom/Shadow OFF다.
이 설정을 최적화 과정에서 변경하지 않았다.

| 항목 | 캡처 평균 |
|---|---:|
| Frame interval |30.565451ms, 약32.72FPS|
| Frame p95 |31.4856ms|
| GPU Lights |14.838206ms|
| GPU NonBlend |11.596467ms|
| GPU SSAO |1.8071ms|
| CPU Present |19.905ms|
| 광원 records / draw |1,497 / 717|

CPU frame30.169ms에서 Present를 제외하면 약10.264ms다. 이 캡처에는 이동 표본과
피킹 GPU readback이 없으므로 이동·AI 계산을 이번 병목으로 지목하지 않는다.

게시 광원315개 중 enabled·brightness 조건을 통과한299개를 캡처 카메라로 대조했다.
frustum을 통과한22개는 point16·spot6, receiver는 SOURCE_CHARACTER21·ALL1이다.
scene directional을 더한 source 경로23 records/11 연속 type 구간과 일반 경로2/2로
`65×23+2=1,497 records`, `65×11+2=717 draws`가120프레임 모두 일치한다.
일반 ALL spotlight는 near-plane을 걸쳐 기존 bounds가 fullscreen으로 열리므로 일반 광원에
clip flag만 확장하는 후보는 이 장면의 비용을 줄이지 못해 적용하지 않았다.

### G23-2. 실제 구현

기존 CMaterial frame registry에서 program·hasBakedLighting·lightTextureMask·전체64개
float4 lightConstants의 비트·사용 slot의 override 적용 후 SRV가 모두 같은 재질만 같은
조명 행으로 연결했다. 각 재질의 base constants와 base textures는 원래 재질에서 따로
bind한다. 광원 순서, shader, quality 옵션, Resources와 게시 데이터는 변경하지 않았다.

행 대표와 모든 alias는 frame 종료까지 shared_ptr를 보유한다. 이는 실제 CModel의
`use_count()>1` 기반 copy-on-write 조건과 frame 중 객체 수명을 보존한다. 동일 포인터의
빠른 lookup, forward 제외, bind 실패 시 미등록, Reset 해제와 24-bit 정확 정수 row 한계는
유지한다. 신규 제품 C++·public API·project/filter 항목은 없다.

설치된 베른 mapmaterials의 source.character43개 slot을 정확한 상수 비트·baked 여부와
texture asset/colorSpace 기준으로 분석하면8개 lighting state가 나온다. 이 값은 전체
catalog의 중복 후보이며 캡처의65개 실제 행을8개로 줄였다는 측정값이 아니다. 실제 frame의
SRV·override·가시 재질 목록은 새 사용자 캡처로 확인해야 한다.

### G23-3. 검증과 남은 실행 확인

기존 `SourceCharacterShaderVariantProbe.cpp`와 runner에 `MaterialLightRows` 선택 검증을
추가했다. 비공개 CMaterial은 실제 production translation unit을 직접 컴파일하고 CShader는
제품 Engine DLL을 사용한다. 격리 출력 폴더에서 실행하며 창을 만들거나 Client를 실행하지 않는다.

- 동일 clone과 base-only 변경은 행을 공유하고 실제 constant-buffer/SRV readback에서
  각자의 base 입력을 유지한다. 마지막 light 상수·signed zero 비트·program·mask·effective
  SRV 차이, baked 여부, 반복 bind, forward 제외, 실패한 등록과 Reset을 확인했다.
- alias 소유권을 유지해 CModel copy-on-write의 shared ownership 조건을 보존했다.
  300개 서로 다른 행은 실제 light constant-buffer readback으로 혼동 없이 구분했다.
- 수정 전 HEAD의 Material.cpp로 같은 회귀를 실행하면 동일 clone이 별도 행을 만드는
  검사에서 실패하며 수정 후 Release 후보는 통과한다.
- 베른 Devilstone program81의 실제 light constants와 변화하는6단계 synthetic mip texture를
  제품 deferred shader에 넣었다. 방향광/점광원/spot × 세 가지 row 경계 × UV 연속/불연속의
  18조건에서 별도2행과 공유1행의32×32 shade/specular 출력 최대 절대차는0이었다.
  finite·비영(非零) 조명 출력을 확인했다. 실제 설치 모델 화면·RTX 성능 측정은 아니다.
- 기존 All Release 검사도149program·14clone·9실패 입력·564light pass 등을 통과했다.
  기존 검증을 대체하지 않고 선택 검사를 추가했다. runner는 EXE 실행 실패가 이전 compiler의
  exit0을 상속하지 않도록 했으며 누락 EXE 실패·정상 실행·오류 설정 복원도 확인했다.

정상 VS Release 빌드 로그는 수정된 Material.cpp를08:42:58에 컴파일하고08:43:02에
Engine.dll로 링크한 것을 확인했다. Engine/Bin과 Client/Bin의 Release DLL hash도 같다.
별도 표준 Product runner 재검증은 실행 중인 Client PID59928·Server PID62144 때문에
output guard에서 컴파일 전에 중단됐다. 이를 Product runner PASS로 기록하지 않는다.
사용자 프로세스는 종료하지 않았다. 기존 인코딩·CRLF와 변경 파일의 diff 공백 검사는 통과했다.

분석·재현 근거는 `out/BernLightRows20261001`의 `audit.py`,
`capture-light-reconstruction.json`, `exact-light-key-analysis.json`,
`devilstone-representatives.json`, `Candidate/probe.log`, `Baseline/probe.log`,
`ExistingAllDefault/probe.log`, `release-build-evidence.json`에 있다.
최종 선택 검사의 소스·DLL hash와 출력 수치는 `Candidate/material-light-rows-result.json`,
`Baseline/material-light-rows-result.json`에 있다. runner의 기본 All 도구 체인 선택도 유지해
최종 기본 인자의 Release 검사 성공을 확인했다. 재현 명령은 다음과 같다.

```powershell
& Tools/RenderingPipeline/Test-SourceCharacterShaderVariants.ps1 -Configuration Release -Focus MaterialLightRows -VCToolsVersion 14.44 -OutputDirectory out/BernLightRows20261001/Candidate
```

표준 runner 중단 기록은 `out/BuildPipeline/runs/20260930T234527945Z-release-product.json`이다.

구현과 위 수치 검증은 완료했다. 같은 베른 위치·해상도·설정의 사용자 새 캡처에서
Lights 시간·광원 draw/records·전체 frame 시간을 비교하는 절차와 최종 화면 판정은 남아 있다.
이전01:23 캡처는 카메라가 달라 이번 변경의 전후 성능 비교로 사용하지 않는다.
