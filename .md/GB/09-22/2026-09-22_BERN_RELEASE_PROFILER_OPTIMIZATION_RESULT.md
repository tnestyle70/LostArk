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
