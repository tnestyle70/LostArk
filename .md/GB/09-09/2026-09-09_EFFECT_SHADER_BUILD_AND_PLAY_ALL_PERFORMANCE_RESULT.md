# Effect 셰이더 분리와 Play All 성능 개선 결과

기준일: 2026-09-09. 브랜치 `codex/dimensionmaster-tool-round3`의 기존 공유 변경을 보존했다.
같은 주제의 IMPLEMENTATION_PLAN이 범위 정본이다. 이 결과는 이번 성능 구조 변경만 다루며
차원술사 R/A/S의 원본 복원 내역은 Round2 RESULT G36을 따른다.

## G00. 복원 기준 빌드

분리 전 최종 Debug Product는 성공했다. receipt는
`out/BuildPipeline/runs/20260909T102254939Z-debug-product.json`이며 Engine/Shared/Server/Client가
모두 성공했고 누락 runtime input은0이다. 전체 소요는4,232,240ms, **70분32.240초**였다.
검토했던9개 입력 SHA와 실제 Engine→Client DLL 배포 일치를 확인한 뒤 성능 변경을 적용했다.
이 결과를 새 구조의 최종 빌드 성공으로 재사용하지 않는다.

기준 EXE, Mesh/Particle/Rect CSO와 DLL은
`out/EffectBuildPlayAllOptimization20260909/Baseline/Binaries/`에 보존했다.
입력·산출물 SHA 및 시간은 `Baseline/final_restore_build.json`에 있다.

## G01. 재질군별 셰이더와 빌드

generic2개와 native34개를 합친 Mesh/Particle FX36개 및 준비된 family 선택 경로를 구현했다.
최종 native-only 구조의 실제36개 carrier FxCompile은 **120,678ms, 2분0.678초**에 성공했다.
앞선 범용 front-end를 포함한 첫 분리본의 성공 배치는24분46.48초였다. 첫 분리본은
138,746,824byte로 기존2개60,247,327byte보다 커져, 물리 분리만으로 완료하지 않고 중복 제거를
추가했다. 최종 산출물 크기와 증분 검증은 아래 통합 항목에서 확인한다.

첫 분리본6개 실제 Q/V/WR CSO와 기존 대형 Mesh/Particle FX를 RTX4070 headless D3D11로
직접 비교했다. Q51 24/V63 24/WR208158/WR260549행에 raw/alpha/additive를 적용한
2,265조건, RT0+RT1 총18,554,880픽셀이 RGBA bit-exact였고 nonfinite0이었다.
WR208 검은 내부95,159/밝은 경계15,038픽셀, WR260 검은 내부4,668/밝은 경계20,807픽셀도
포함했다. 테스트8개 CSO와 실제 읽은6개 CSO SHA는 보존했고 검사 중 변경되지 않았다.
`FullFxDifferential/FULL_FX_RESULT.md`와 `full_fx_result.json`이 근거다.

같은 native differential의 WARP 실행은 기준 대형 PS의 첫 draw JIT가 CPU504.516/384.063초에도
끝나지 않아 경로가 확인된 검사 프로세스만 중단했다. 이 WARP 실행은 미완료이며 PASS가 아니다.
위 출력 일치는 같은 FX와 입력을 사용한 실제 hardware readback 결과다. 기존 별도의 WARP
buffer/generic 검사는 이 실행과 구분한다.

첫 분리 후에도 native FX에 큰 범용 PS front-end가 남는 중복을 확인했다. 실제 Has_*와 후반
resource staging·override·slot 재귀를 추적했고,224개 Authored 문서의 native1619행에서
Execution.enabled=true는0이었다. 현재 native와4개 범용 backend가 동시에 켜지는 입력은
허용되지 않는다. 이 근거로 native 전용 PS와 packet bind를 더 줄이는 추가 단계를 반영했다.
Generic 경로는 유지하고 모순된 mixed flags는 실제 draw 선택 경계에서 실패시킨다.
최종 native34개 FX의 실제 전처리 결과에서 VS_MAIN/PS_MAIN 함수 호출 closure를 조사했으며,
생략한 legacy23개 globals와 범용 backend globals의 reachable read는0이었다.
원본639개 native 함수 본문 SHA는 같고 중복 물리 owner는0이다.
`G01NativeOnly/reachable_native_globals.json`과 `g01_formula_and_xml_validation.json`이 근거다.

최종36개 CSO 합계는87,641,232byte로 첫 분리본보다 줄었지만 기존 대형2개보다 약45.5% 크다.
공통 VS/pass와 FX 객체를 여러 프로그램이 각각 소유하는 대가다. 기존 device별 공유 Core에서
한 번 준비하고 draw 중 생성하지 않는다.36개 실제 Effect 생성·반사·input layout 검사는
648.98ms였으며 이것을 Client 전체 로딩 시간으로 표현하지 않는다.

최종 무변경 FxCompile은0.310초/FXC0회였고57개 Client CSO의 SHA와 수정 시각이 모두 같았다.
ALTV192 물리 include에 소유권 설명 주석을 남기자 MeshALTV192/ParticleALTV192/NativeScreenPost
정확히3개만1.243초에 재컴파일했고 나머지54개는 보존됐다. 원본 함수 본문은 변하지 않았다.
`G01NativeOnly/incremental_result.json`이 실제 tracking·before/after 증거다.

Compiled closure는 Engine2+Client57의 활성 producer59개와 Client consumer58개를 확인했다.
실제 family36개 pass/layout/선택 packet/SRV/backend isolation, 기존 V1/V2와 유리 RT0/RT1 및
잘못된 유리 입력의 무출력, Resources 경계8조건도 통과했다.
`G01NativeOnly/compiled_closure.log`, `warp_result.json`에 결과를 보존했다.

최종 native-only FX로도 기존 대형 FX와 같은2,265조건/18,554,880픽셀을 다시 비교해 RGBA
bit-exact, nonfinite0을 확인했다. 입력 packet/time/color/dynamic/scene/geometry는 첫 검사와
같으며, 사라진 범용0 flags만 설정 대상에서 제외했다. 첫 분리본 수치 PASS를 재사용하지 않았다.

설치 MSBuild의 실제 MultiProcFXC 경로는 별도 작은 FX 프로젝트에서 확인했다.
2개 FX 실행시간130/131ms가 전체 task172ms 안에 겹쳐 실행됐다. 변경 없는 재빌드는0 FXC였고,
한 leaf include 변경은 해당1개 CSO만 갱신했다. 근거는 `FxcParallel/result.json`이다.
이 작은 검사의 시간을 제품 전체의 개선율로 사용하지 않는다.

실제 pass PS reflection에서도 Mesh 정적 명령49,448개가 Q9,291/V10,131/WR14,875개로,
Particle67,240개가 Q14,878/V15,719/WR20,463개로 줄었다. temp register는 Mesh17→15/14/17,
Particle은17로 같았다. 실행되는 특정 profile의 GPU 시간이나 FPS는 이 수치와 다르다.
Debug bytecode 크기는 debug metadata를 포함하므로 GPU 실행 코드 크기로 표현하지 않는다.
근거는 `FullFxDifferential/pass_static_metrics.csv`다. 이는 중간 분리본 지표다.
최종 native-only PS 명령은 Mesh Q1,355/V2,195/WR6,939,
Particle Q1,354/V2,195/WR6,939다. 최종 temp register는 두 carrier 모두Q15/V12/WR17이며
최종 측정은 `FullFxDifferential/pass_static_metrics_final.csv`에 있다.

## G02. CShader 변수와 pass 조회

`Engine/Public/Shader.h`, `Engine/Private/Shader.cpp`에 실제 반영했다. Effect 생성 때 이름을
소유하는 불변 해시 테이블과 typed variable/pass 포인터를 준비하고 기존 Bind와 Begin이 사용한다.
같은 Effect를 공유하는 Clone은 캐시도 공유한다. uniform 값은 캐시하지 않는다. 재초기화는
새 Effect와 캐시를 함께 commit하며 실패하면 기존 Effect와 payload를 보존한다.

실제 변경 전후 CShader.cpp를 각각 컴파일하고 같은 RectPreview CSO를 D3D11 WARP로 열었다.
raw/matrix/resource bind, 전체 reflected global 이름의 공개 Bind 반환값,100개 없는 이름,
잘못된 type/pass, 실제 Begin의 VS 설정, Clone 갱신, 실패/성공 재초기화 및 이전 Clone 수명이
모두 통과했다. UI/Client를 실행하지 않았다. 근거는 `ShaderCache/`의 소스·컴파일·실행 로그다.

동일 Debug 실제 소스에서30만 bind를 반복하는 기준/변경3쌍을 교대 측정했다. 각 실행의 warmup을
제외한 중앙값을 다시 비교하면 **51.8532ms→10.97615ms**, thread cycle은
**129,368,068→27,303,213**이었다. 이 바인딩 구간의 시간 감소는 약78.8%이며 Client 전체 FPS가 아니다.
`ShaderCache/comparison_runs.json`에 모든 반복을 보존했다. 최초 후보의 Debug STL 해시 lookup은
오히려 느려서 제품에 적용하지 않았고 현재 불변 슬롯 테이블로 교체했다.

## G03. Playback CPU

Rebuild_Frame의 source sprite 표시값·mesh 판정·type pre-rotation을 요소당 한 번 계산하도록
옮겼다. local-space 역행렬도 요소당 재사용하되 world-space birth root는 입자별로 유지했다.
HistoryUpdate와 FrameRebuild를 기존 Profiler scope에 연결했다.

이 첫 변경은 실제 source Playback의5개 문서598요소,4,954개 순서 있는 frame 및 내부 RNG/state
checkpoint가 기준과 같았다. R23/S26/차원술사 AltV318/도화가 V69/창술사 Alt1 162요소가 대상이다.
추가로 조사한 도화가 Alt2와 창술사 Alt3 문서는 기준·변경 모두 같은 기존 unsupported module로
거부됐다. 현재 restored Effect Tool의 Load→Validate_Drawable→Stage도 같은 거부 경계다.
이2개를 새 회귀나 성공 문서로 집계하지 않으며 validation을 완화하거나 데이터를 삭제하지 않았다.

실제 Profiler로 큰 두 문서의240 HistoryUpdate를 확인했을 때 FixedStep이 각89.8%/90.9%였다.
FXC 동시 부하의 scope 비중 자료이며 정식 wall time 개선율은 아니다. 이를 바탕으로 실제 staged
document의 update module19종 class·enabled·원본 index를 준비하도록 CPP/H에 반영했다.
이전 prepared bundle의 value-only 편집 계약을 보존하며 vector field 리소스는 공유한다.
prepared owner를 교체하기 전에 immutable document shared_ptr를 값으로 보존해 수명을 유지한다.
모듈 수식·분포·literal fallback과 RNG·실행 순서는 바꾸지 않았다.

최종 소스에서도 기존4,954 checkpoint가 전부 일치했다. prepared sole-owner 이동, 외부 owner가
없는 immutable document, enabled on/off 편집 재사용, 실제 Solo 필터와 loop720 checkpoint도
전부 일치하여 **최종5,674 checkpoint 차이0**이다. 최소 C++ 컴파일과 diff 검사도 성공했다.
`CPU/final_source_receipt.json`, `CPU/executor/frame_comparison_final.json`이 최종 SHA와 근거다.
준비를 측정 밖에서 완료한 동일 public Stage/HistoryUpdate 경로를 기준·변경3쌍으로 교대 측정했다.

| 문서 | HistoryUpdate 평균 시간의3회 중앙값: 기준→변경 | 감소 | thread cycle 감소 |
|---|---:|---:|---:|
| 차원술사 R |1.592→0.820ms|48.5%|48.4%|
| 차원술사 S |7.537→2.400ms|68.2%|68.0%|
| 차원술사 Alt+V |12.764→6.159ms|51.7%|51.4%|
| 도화가 V |11.918→4.004ms|66.4%|65.7%|
| 창술사 Alt+V clip1 |11.230→4.624ms|58.8%|58.5%|

마지막 한 쌍은20:08:47–20:12:14에 FXC/CL/MSBuild와 native WARP JIT가 없는 구간에서 실행했다.
이 쌍의 cycle 감소도48.4/68.1/52.5/62.6/53.4%로 같은 방향이다. 처음 두 쌍의 동시 부하는
별도로 기록했다. GetThreadTimes가 같은 코드의 반복에서 크게 흔들려 그 OS-reported CPU 값은
단독 개선율 근거로 쓰지 않았다. 위 표는 wall time과 QueryThreadCycleTime의 일치 근거다.
`CPU/executor/final_timing_summary.json`, `quiet_timing_summary.json`에 모든 값을 보존했다.
Debug headless Playback 측정으로, GPU 제출·전체 Client frame이나 실제 FPS의 측정은 아니다.

## G04. 입자 업로드와 SceneColor 판정

`VIBuffer_ParticleRect.h/.cpp`는 기존2048 capacity와136-byte payload를 유지하며 미사용 tail에
WRITE_NO_OVERWRITE로 append한다. 첫 upload 또는 tail 부족만 WRITE_DISCARD다. 활성 slice의
byte offset을 IA에 연결하고 draw startInstance0은 유지했다. cursor/offset은 Map 성공 뒤 commit한다.
empty/Map 실패는 기존 count0, oversize는 기존 slice/count 보존과 E_INVALIDARG를 유지한다.
frame 경계에서 cursor를 임의 초기화하지 않으며 Clone은 독립 buffer를 만든다.

실제 현재 class를 WARP에서57번 draw해2,651입자의15,906 stream-output 정점 전체 payload와
순서가 byte-exact였다.9회 시작 slice,47회 append, exact-fill/wrap, empty, oversize, Clone,
재초기화를 포함했다. 강제 device removal/Map 실패 주입은 하지 않았다.
기존 Profiler에 실제 누락된 제출을 연결했고 DrawCalls57/InstancedDrawCalls57/Instances2651/
Indices15906이 GPU 결과와 일치했다.

Effect_Object의 SceneColor predicate는 Late_Update 호출 안에서 같은 요소의 결과를 재사용한다.
입자별 alpha·visibility·preview 선택은 그대로다.5문서598요소의2,400조건에서 결과가 모두 같았고
positive115조건을 포함했다. predicate 계산은63,062→2,220회로 줄었으며 입자별 eligibility
150,711회는 전후 같다. 호출을 넘겨 캐시하지 않아 문서 편집 후 stale 판정을 남기지 않는다.

현재3개 파일의 최소 CPP 컴파일과 diff 검사는 성공했다. 자세한 실제 소스 SHA와 근거는
`GpuSubmission/G04_RESULT.md`, `GpuSubmission/g04_result.json`에 있다. WARP 시간이나 반복 횟수
감소를 실제 GPU 프레임 시간 개선율로 표현하지 않는다.

G02/G04의 현재 Engine 전체 Debug 빌드는18,869ms에 성공했다.
`engine_prebuild_result.json`에4개 소스 SHA를 고정했으며, 현재 Client로 SDK/DLL을 배포하고
최종 링크하는 것은 아래 Product 통합 단계다. 기존 PhysX PDB 미제공 경고는 남아 있다.

## G05. 통합과 사용자 확인 경계

새 구조의 실제 분리 FX 증분 범위·출력 비교와 최종 Debug Product가 모두 완료됐다.
`out/BuildPipeline/runs/20260909T112138322Z-debug-product.json`의 Engine/Shared/Server/Client
모든 단계가 성공했고 누락 runtime input은0이다. 최종 Product는119,666ms, **1분59.666초**였으며
Client C++ 재컴파일·링크와 Engine SDK/DLL 배포를 포함한다. 앞서 성공한 FX tracking을 재사용해
이 Product에서 FXC 실행은0회였다. 따라서 별도36 FX 재컴파일120.678초와 이 Product 시간을
구분하며, 최초 clean 전체 빌드가 항상2분이라는 뜻으로 해석하지 않는다.

현재 실행 파일은 `Client/Bin/Debug/Client.exe`,49,940,992byte,20:21:37 KST 갱신이다.
SHA256은 `15C867839350238CF0D0C8B23873ECE8EBF15D3905A45B6D38BCE89F5A0195F5`다.
Engine과 Client에 배포된 Engine.dll SHA가 같고, 검증된 G01 소스66개/CSO36개, Shader/Playback/
VB/Object 및 최종 native 수치 검사의 모든 입력 SHA가 현재 파일과 같다.
`out/EffectBuildPlayAllOptimization20260909/final_integration_result.json`에 이 대조를 보존했다.

변경 project/filter XML2개와 PowerShell 구문 parse, 저장소 전체 `git diff --check`가 성공했다.
Native 입력 전수조사의224 JSON parse도 성공했으며 이번 성능 변경은 저작 JSON/Resources를
수정하지 않았다. 기존 C4819/C4828 인코딩 및 third-party PDB 미제공 경고는 남아 있으나 새 빌드
오류는 없다. Release bytecode 빌드와 전체 world/network 진단, publisher는 실행하지 않았다.

최종 확인 시 Server/Client/FXC/MSBuild 프로세스는 모두 종료 상태다. 이 PC의 팀 LAN 설정은
server-host이므로 **Debug|x64 → Server + Client profile → Ctrl+F5**로 사용자가 실행한다.
Client/Server/UI 자율 실행·조작·캡처는 하지 않았다. 실제 Play All FPS와 R/A/S의 시각 판정은
사용자가 Server + Client Ctrl+F5 → Lobby → Character Select → 차원술사 → F1 Effect Tool에서 수행한다.
입자·mesh·시간·draw 순서·수식을 줄여 성능을 만드는 변경은 하지 않는다.

남은 확인은 실제 장면의 CPU/GPU frame time과 시각 결과다. 대표4개 shader profile의 수치 일치와
전체 family 입력 계약 성공을 모든 native profile의 최종 화면 일치로 확대하지 않는다.
현재 일부 문서의 기존 미지원 모듈 거부와 S 주변 screw의 원본 occurrence 미확정 경계도 별개이며,
이번 성능 개선으로 원본 복원 완료나 지원 범위 확대를 주장하지 않는다.

원본 mip12개 DDS의 Resources/Drive 전달 경계는 이전 G36과 같다. Git binary 추가와 무관한
공유 dirty 파일의 stage/commit/push는 하지 않았다.

## G06. Alt V318 full 재생 후속 실측과 초기 준비 개선 — 2026-09-10

사용자가 중단된 문서를 `effect.dimensionmaster.skill.2050540.full.restore` 전체로 확인했다.
14,269,056-byte JSON의318요소(sprite221/mesh97)를 유지한 채 기존 Playback과 실제 native CSO를
수치 실행했다. 전체 Client FPS를 측정하거나 사용자가 본 중단을 자동 재현했다는 뜻은 아니다.

### 적용한 개선: 일반 저작 준비의 불필요한 canonical 재직렬화 생략

F1의 Create_AuthoringOccurrence → CEffectObject::Stage_Document → Renderer 준비 경로는
이미 Drawable 검증을 끝낸다. 일반 Stage_PrevalidatedDocument는 값 편집을 허용해 canonical
digest를 사용하지 않는다. 그럼에도 sourceRecipe 전체를 Serialize → JSON parse → canonical
hash로 다시 처리하고 있었다.

Playback 준비의 기본값은 계속 hash 생성이며, 일반 검증 완료 호출자 둘만 생략을 명시한다.
immutable document는 flag와 무관하게 hash를 생성하고 visual/reconstructed 준비도 유지한다.
Loader/Codec 검증, resource signature, typed identity, 실패 시 기존 객체 보존은 그대로다.
제거한 것은 semantic validation이나 일반 입력 size gate가 아니라 중복 canonical 변환이다.

| 단계 | 변경 전3회 중앙값 | 변경 후3회 중앙값 |
|---|---:|---:|
| JSON Load |4,249.70ms|4,236.01ms|
| Playback resource prepare |6,306.02ms|1.2858ms|
| Stage |69.20ms|65.90ms|

저작 준비의 중복 변환 약6.3초를 없앴다. JSON 읽기 약4.2초와 모델·texture·FX 준비, 실제 렌더링은
별도 비용으로 남는다. 모든 frame/RNG/state 검사2,060개 CSV가 byte-exact였다. 모듈 평가, 난수,
입자 수와 시간은 생략하지 않았다. 현재 변경이 재생 중 simulation을 가속했다는 개선율은 내지 않는다.

### 재생 중 비용과 중단 경로

962개60Hz step에서130,389 particle records, 최대710개 live particle, mesh 최대426개,
활성 요소 최대187개였다. 기준 HistoryUpdate는 평균5.797ms/p95 20.287ms/p99 22.271ms/
최대59.836ms다. 전체 기간에는 빈 tail도 포함되므로 평균으로 밀집 구간의60FPS를 주장하지 않는다.
이 headless 값은 전체 Client frame이나 GPU 시간이 아니다.

Update_WithTransformHistory가60step보다 큰 누적 시간을 거부하고 Sequencer의 Sample 실패가
Stop으로 이어지는 실제 경로를 확인했다.1.1초를 주입해 거부 문구와 기존 state 보존을3회 재현했다.
사용자 화면의 오류 문구와 frame log가 없어 이 경로가 실제 중단의 원인이라고 확정하지는 않는다.
catch-up 한도를 늘리거나 delta를 버려 clock 불일치를 감추는 변경은 하지 않았다.

### GPU·후처리 조사와 수백 요소를 유지하는 방법

RTX4070에서 실제 ALTV064/128/192 family의 sprite/mesh6종을 두 번의 별도 process로 실행했다.
396개 draw 표본 모두 disjoint, 비유한 픽셀, D3D debug error가0이었다.64² 단일 fixture의 GPU
timestamp는 약.008~.011ms였다. 최초 CPU 벽시간55~433ms 중54~427ms는 bind·Apply·Draw·
완료 polling 밖의 미계측 구간이었다.

query Begin/End/Flush를 나눠 다시 측정하니 큰 잔여 지연은 재현되지 않았다. 두 번째 first Flush는
.538~.763ms, warm 중앙값은.058~.074ms였다. FX/Shader/Layout 준비64.8~173.0ms는 남았다.
영구 driver cache/JIT와 일치하는 양상이지만 최초 gap 내부 기록이 없어 원인은 미확정이다.
driver cache를 지우지 않았으며 ALTV145는 고정 fixture 출력이0이어서 fragment 비교도 제한된다.
작은 단일 draw 수치로 전체 viewport의 겹침이나318요소의 FPS를 예측하지 않는다.

현재 sprite는 이미 element span별 instanced draw다. mesh는 live particle마다 Render_Mesh에서
같은 재질·texture·상수 준비와 submesh draw를 반복한다. 따라서 최대426개 mesh는 실제 제출이
증폭되는 근거다. SceneColor는 활성 요청에 따라 프레임당 최대1회 복사한다. particle마다 화면을
복사하는 구조는 아니며 ALTV178 starting capture도 현재318문서에는 없다.

다음 구조 개선 대상은 같은 occurrence의 mesh particle을 instance stream으로 묶는 것이다.
기존 CModel/CMaterial 안에서 동일 submesh, native family, pass, texture/sampler, capture를
쓰는 연속 구간만 묶는다. world/normal/tangent, color, dynamic, life/time, SubUV는 instance별로
전달하고 mirror culling과 투명 입력 순서를 보존한다. 고정 material/texture binding은 그 구간의
밖으로 옮길 수 있다. 별도 model runtime을 만들거나 원본 요소를 삭제할 필요는 없다.

이 mesh instancing은 구체적인 후속 설계이며 이번 제품 구현 완료로 기록하지 않는다. 같은 픽셀을
여러 번 그리는 비용은 draw 묶기만으로 줄지 않으므로 실제 GPU frame 측정과 사용자 화면 확인이
필요하다. 긴 JSON Load 역시 남은 초기 비용이다.

### 검증과 현재 실행 상태

CPU 원문은 `out/DimensionMasterRDSAltV20260910/CPU/`, GPU 세부 표와 한계는 같은 작업의
`GPU/renderer_analysis.md`에 있다. 제품 최소 C++ 컴파일과 현재 객체의 별도 EXE 링크는 성공했다.
R/D/S/AltV 네 문서406행의 Save 왕복과 visible405개 Solo도 통과했다. D의 기존 hidden1은 유지했다.
공용 준비 호출 경계와 operation4를 독립 검토했으며 재현 가능한 회귀 지적은 없었다.

새 EXE는 RDS RESULT G37과 같다. 현재 Client PID55640이 실행 중이라 정식 EXE 교체와 사용자의
실제 재생 확인은 남았다. 이전 G05의 종료 상태와 EXE 시각은 당시 증거이며 현재 상태로 쓰지 않는다.
Client/UI 실행·종료·조작·캡처는 수행하지 않았다.

최종 통합 확인: 변경 JSON과 Alt V 총4개 parse, 현재 Client project/filter XML2개 parse,
저장소 전체 git diff --check가 모두 성공했다. 검증한 제품 소스5개·변경 JSON3개의 SHA와
현재 파일이 일치하며 새 EXE SHA도 receipt와 같다. Resources 추가와 Git stage/commit/push는 없다.

## G07. 광원 후속 요청과 Alt V318 전수 구조 감사 — 2026-09-10

**요소를 삭제하지 않고 줄일 수 있는 반복 비용을 확인했다. 다만 지금318행이 원본의 실제 발생량까지
그대로 재생되는 상태는 아니다.** 이번 작업은 원본 입력·현재 Playback·geometry·shader 소비의
전수 감사와 후속 구현 설계다. G06의 준비시간 개선에 더해 mesh instancing이 제품에 적용됐다거나
실제 Client FPS가 개선됐다고 기록하지 않는다. 광원106문서와 Artist 꽃밭은3class RESULT G14에
별도로 정리했다.

### 현재318행의 입력과 실제 발생

`effect.dimensionmaster.skill.2050540.full.restore`는 sprite221/mesh97, native program120종,
DDS166개, static WModel19종을 참조한다. family별175/112/31행은 ALTV064/128/192에 해당한다.
모델17종은 단일 submesh, crack032/037 두 종은2submesh다. SceneColor를 요청하는 행은34개지만
현재 renderer는 요청이 있을 때 frame당1회 snapshot을 만든다. particle마다 화면을 복사하는 구조는
아니다. DDS 실물 파일 합계14,301,528bytes는 파일 크기이며 전체 Client VRAM 실측값은 아니다.

현재 제품 Playback의 out 사본과 기존 root/anchor transform fixture로962개60Hz step을
재생했다.16.0333초까지 계산했으며 마지막 비어 있지 않은 frame은11.9833초다.
130,389 particle records, 최대710개 live particle, mesh 최대426개, 활성 element 최대187개였다.
이 수는 동일한 probe 입력의 결과이며 실제 사용자의 아레나 frame capture가 아니다.

| 전수 확인 | 결과 |
|---|---:|
| 실제 발생한 element |308/318|
| Spawn_Particles의 생성 요청 |3,951|
| 실제 수용한 생성 |1,854|
| 현재 capacity로 거절된 생성 |2,097,8개 element|
| 기존 frame/RNG/state checkpoint 대조 |2,060개 byte-exact|

`Tools/LevelPlacementExtractor/build_imported_effect_documents.py`의
`MAX_PARTICLES_PER_IMPORTED_ELEMENT=64`가 source PeakActiveParticles를64로 자른다.
현재318행의 maxParticles는 conversion receipt와 모두 일치하고 sourceScale.count도 모두1이다.
그러므로 이번 차이는 사용자 수량 trim이 아니라 importer의 BUDGET_CLAMP에서 비롯된다.
실제 첫 burst1500은64만 수용해1,436개가 잘렸다. source peak1507→64인 이 행 외에도
원본 peak271/271/251/261/124→64인 행과 rate 누적 손실을 확인했다.

현재 코덱은 element cap1~2048 및 scaled document cap합8192를 허용하고 현재 문서의 합은2649다.
64는 엔진이나 Direct3D가 수백 입자를 못 그린다는 한도가 아니다. 다만 source 요청 수와 실제
원작 화면의 동시 입자 수는 원본 quality/LOD/action 조건까지 같아야 비교할 수 있으므로 같다고
가정하지 않는다. cap을 원본에 맞게 회수할 때는 발생 수·RNG·메모리·최대 동시 비용도 다시 측정해야
한다. 이번 최적화 계산은 현재 cap의 출력 기준이다.

발생하지 않은10행 중8행은 source emitter duration0에 적용한 typed .1초 창보다 저장된
burst .15~.9초가 늦다. 현재 코드와 저작값 조합에서 그 burst까지 도달하지 않는 것을 확인했다.
나머지2행은 burst 없음/rate0이다. 원본 duration0의 상속·loop 의미까지 해독됐다는 주장은 하지
않으며, 이8행은 원본 clock을 추가 회수할 복원 경계다. 요소를 목록에서 유지하는 것과 발생이
실제로 복원되는 것은 별개다.

### CPU 비용: 준비시간 외에도 반복 조회가 남아 있다

제품 Playback의16개 함수에 out 전용 scope를 넣고 동일962step을 측정했다.
총 HistoryUpdate6,579.38ms 중 Step4,994.82ms, Rebuild_Frame1,496.55ms였다.
아래 시간은 Debug와 scope 오버헤드를 포함하고 상위/하위 함수가 중첩되므로 행을 합산하지 않는다.

| 함수/작업 | 호출 수 | inclusive 시간 |
|---|---:|---:|
| Apply_SourceUpdateModules |130,389|3,074.61ms|
| Find_SourceDistribution |1,303,139|688.69ms|
| Evaluate_ModuleFloat / Vector |640,407 / 640,054|581.46 / 613.14ms|
| Prepare_SourceVectorFieldUpdates |28,976|378.15ms|
| Rebuild_Frame |962|1,496.55ms|

Update_Particles는318×962=305,916번 호출했다. 이 문서는 portable source events=false여서
Step의 조건부 전체 상태 복사는 실행하지 않는다. 발생하지 않은 복사 비용을 최적화 근거로 삼지
않았다. source update kind enum은 이미 준비되어 있지만 distribution/property 조회와 일부
불변 module/literal 검색, 빈 vector-field 준비 검사는 반복된다.

후속 CPU 구현은 기존 PREPARED_RESOURCES에 검증된 module/distribution/literal index와
필요 기능 flag를 준비하는 방향이다. uniform 분포의 난수 호출, emitter/particle 시간 선택과
curve 평가는 그대로 수행한다. 불변 값의 위치를 캐시하는 것과 난수/시간에 따라 달라지는 결과를
캐시하는 것을 구분한다. Rebuild_Frame의 packet 준비와 활성 구간 순회도 같은 frame/state 증거로
대조하며, 큰 catch-up delta를 버리거나 현재60step 제한만 늘려 중단을 감추지 않는다.

### mesh draw를 묶을 때 실제로 줄어드는 범위

실제 Playback의 프레임별 particle 출력에 현재 Render_Mesh의 determinant gate, submesh 수,
sprite span 제출 조건을 적용했다. 이하 수치는 **GPU draw를 실행해 센 값이 아닌 현재 renderer
조건으로 계산한 제출량**이다. geometry/상태 실패, 실제 viewport의 occlusion/overdraw는 이 계산의
측정 대상이 아니다.

962frame 전체는88,675회(sprite20,276+mesh68,399)이며, 가장 많은2.68333초 frame은577회다.
그 frame은 총605개 입자 중375개가 mesh이고, determinant gate를 통과한307개 mesh particle이
submesh 반복으로549회+sprite28회를 요구한다. 이 frame의 기하는143,472 triangle instances다.
mesh material packet 준비는157,184bytes, texture setter는2,763회다. 이는 호출량과 준비 데이터
크기이며 GPU upload/driver stall 실측값으로 바꾸어 부르지 않는다.

단일 submesh만 occurrence 내부에서 instancing하면 전체88,675→81,092회(8.55%감소), 기존
peak577→540회다. 많은 입자를 내는 crack032/037의7개 occurrence가2submesh여서 이 보수적안의
효과가 작다. 단순히 submesh별로 particle 전체를 묶으면 `p0(m0,m1),p1(m0,m1)`을
`m0(p0,p1),m1(p0,p1)`로 바꾸어 alpha 순서가 달라진다.

전수 상태 대조 결과 현재7개는 모두 ALTV145, 같은3개 texture 계약, alpha pass3/back-cull/
depth-read이며 source material slot override가 없다. 두 submesh의 WMAT diffuse 경로는 모두
비어 같은 기본색이고 해당 ALTV native PS는 g_BaseTexture도 소비하지 않는다. 따라서 현재의
effective state 조건에서는 **CModel 내부에서 submesh0→1의 기하를 그대로 이어 준비한 다음,
그 전체를 입자별 instance로 반복**하는 구조가 성립한다.

원본 indexed vertex attribute sequence와 결합 뒤 sequence는 두 모델 모두 byte-exact였다.
3개 입자의 논리 primitive 순서도 대조했다. source material slot0/1의 원래 index 범위는 별도로
보존한다. 결합 준비 버퍼의 추가 VB/IB 합은208,436bytes다. 기존 CMesh의 실제 runtime ABI는
VTXMESH68bytes와32-bit index다. 초기 감사의80bytes/16-bit 가정에 따른233,570bytes는 교정했다.
source indexed attribute와 primitive 순서, draw 수 계산에는 이 byte 예산 오류가 영향을 주지 않는다.
원본 asset이나 CModel/CMaterial
소유권을 없애지 않고 같은 CModel의 준비 geometry cache로 구현할 수 있다. 이후 slot override,
geometry 또는 실제 shader 입력이 바뀌면 동일 상태 여부를 재검증하고 해당 group만 해제해야 한다.

| 구조 |962frame 전체 계산 draw |기존577회 frame|해당 구조의 전체 frame 최고값|
|---|---:|---:|---:|
| 현재 |88,675|577|577|
| 단일 submesh만 instance |81,092|540|540|
| 같은 상태의 submesh 순서 결합 + occurrence 내부 instance |28,762|62|184|

마지막 안의67.56%는 **제출량의 계산상 감소율**이며 GPU/CPU frame time이나 FPS 개선율이 아니다.
같은 삼각형·픽셀 중첩은 계속 남는다. 원본 cap과 clock을 더 복구하면 제출량도 바뀐다.
이 geometry/instance shader 구현과 실제 GPU parity는 아직 적용·검증하지 않았다.

### 구현 순서와 검증 경계

1. 위2개 static model의 준비 geometry와 native mesh instance 입력을 기존 CModel/CMesh 안에
   연결한다. world/normal/tangent, color/dynamic, life/time, SubUV 및 투명 primitive 순서를 보존한다.
   기존 `CModel::Render_Instanced → CMesh::Render_Instanced`를 사용하되 instance stream slice를
   전달할 offset/start-instance 경계를 추가해야 한다. geometry와 native shader 입력의 실제 GPU
   출력 대조가 다음 검증이다. Direct3D는 indexed instance의 반복 수와 시작 위치를 제공한다.
   [Microsoft DrawIndexedInstanced](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-drawindexedinstanced)
2. source module의 준비 index와 불변 material/texture binding을 적용하고, 입자별로 달라지는
   값은 instance stream에 쓴다. 동적 vertex buffer는 사용 중인 범위를 덮지 않는 append 방식과
   DISCARD wrap을 사용한다. 현재 sprite 쪽의 기존 append/instancing을 다시 만들지 않는다.
   [Microsoft 동적 buffer 갱신 계약](https://learn.microsoft.com/en-us/windows/win32/direct3d11/how-to--use-dynamic-resources)
3. importer cap과 무발생8행의 원본 시간 계약을 별도로 복구하고 늘어난 원본 발생량으로 비용을
   다시 측정한다. source light와 Artist sibling provider 복구도 새 동시 부하에 포함해야 한다.
4. 실제 아레나 CPU/GPU frame, 큰 투명 면의 중첩과 전체 viewport shader 비용은 사용자의 재생
   확인으로 연결한다. 이번 작은 GPU fixture 또는 headless CPU 숫자를 최종 화면/FPS로 쓰지 않는다.

제품 문서/소스는 변경하지 않았다. out CPU scope probe와 실제 draw/cap 집계 probe가 각각
컴파일·실행 종료0이며 후자는 기존2,060 checkpoint와 byte-exact다. 현재 Alt V source JSON SHA는
`a25f86f9f24a0f89ada7597a130079da07e2f357e302cb9fcb7cb3b6a6b32d12`다.
자세한318행,120개 profile, 모든 geometry/texture와 cap 손실은
`out/FullRestoreLightAltVAudit20260910/AltV/`, CPU scope 결과는 같은 작업의 `CPU/`에 있다.
새 제품 EXE 배포·Client/UI 실행/조작/캡처와 visual PASS는 이번 감사에 포함하지 않았다.

## G08. full106문서 공통 재생 성능 구현 — 2026-09-10

사용자가 Alt V 외에도 수십 element의 Play에서 한 자리 FPS가 발생한다고 확인한 뒤 G07 설계를
제품 코드에 연결했다. 이번 완료 범위는 공통 CPU 조회·native mesh 제출 구조와 두 복원 결함의
소스 수정, 실제 빌드·수치 검증·실행 폴더 배포다. 사용자 실제 FPS와 visual fidelity는 미확인이다.

### 실제 변경

Playback은 Stage에서 각 update module의 최대4개 distribution index와 고정 literal을 준비한다.
입자마다 같은 문자열을 검색하던 Dynamic/Orbit/Location/Color/Size/Rotation/Camera/SubUV 경로는
현재 staged document의 index를 사용한다. uniform 난수 소비와 curve·emitter/particle 시간 평가는
그대로 남겼다. 분포 pointer를 외부 문서에 고정하지 않으며 값 편집·복사 뒤 재stage로 다시 준비한다.
빈 입자의 불필요한 갱신과 VectorField가 없는 element의 field 준비 호출도 생략한다.

Effect_Object는 매 프레임 반복하던 SceneColor 재질 검증을 문서 Stage로 이동했다. 실제7개
Playback 교체 경로 중 정상5개에서 준비하고 diagnostic2개에서 비운다. F1 값·Visible 편집은
기존 재stage를 사용하며 매 프레임의 Solo·가시성·입자 alpha 조건은 계속 평가한다.

Renderer는 native static mesh의 같은 occurrence를 연속 pass 구간별로 한 번에 제출한다.
입자별 World/Normal/Color/Dynamic/SubUV/LifeBlend는200-byte slot1 stream에 담는다.
기존 regular pass0~6을 보존하고 같은 상태와 pixel shader를 쓰는 instance pass7~13을 추가했다.
좌우 반전으로 cull pass가 바뀌면 그 지점에서만 나누며 전체 입자나 shader를 정렬하지 않는다.

같은 effective native material인 multi-submesh는 CModel 안에서 원본 기하·primitive 순서로
준비한다. `p0(m0,m1),p1(m0,m1)` 순서를 유지하고 원본 모델·source material slot은 보존한다.
기존 CMesh의 vertex68-byte/index32 ABI를 바꾸지 않았으며 두 균열 준비 버퍼의 추가 GPU 메모리는
208,436bytes다. 일반 map model에는 source CPU geometry 보존을 강제하지 않는다.
skinned/generic/compiled adapter/source slot override 또는 Engine의 미지원 준비 결과는 기존
Render_Mesh 경로를 사용한다. 실제 allocation/draw 실패는 기존 typed 실패 처리로 전달한다.

별도 복원 결함으로 native shader frontend가 새 Artist/Warlord/Lance profile을 clip하던 범위를
교정했다. full 문서에 해당하는 저장행은971개(Artist177/Lance352/Warlord442)이고 중복 검토본을
포함한다. registry 연결과 컴파일만으로 실제 native PS 진입을 보장할 수 없었던 결함이다.
원래 native switch와 unknown-profile clip은 유지한다. 실제 geometry의 UV1 보존 근거도 모든
native family가 소비하도록 연결했다. 이 수정은 이전보다 유효 표시량을 늘릴 수 있으므로
성능 전후 GPU 시간을 같은 그림량의 비교라고 단정하지 않는다.

또한 Light_Manager::Render_Lights의 조기 transient clear를 제거했다. 실제 두 호출자는 같은
Renderer::Draw 안에 있고 성공·실패 Clear_Frame이 프레임 종료를 소유한다. source-character
조명 전에 효과 광원이 사라지던 문제를 고쳤으나 source 광원23행과 초기값의 미복원은 별도다.

### CPU 재생 출력과 시간

수정 직전 Playback의 보존 사본과 수정본을 같은106 full 문서·seed·root/anchor fixture로
실행했다. 문서별 최대16초의60Hz history와8개 seek를 포함한29,974step/30,822 checkpoint의
frame·particle·RNG/state CSV가 byte-exact다. SHA256은
`5ae7c49ab9d7f4cdc4622c5657be9dd05d9126f2c092c14f16e6a800ee756066`이다.
가변 delta·실패 transform rollback·비정상 matrix·clock gate를 포함한 별도 Alt V2,060점도
이전 결과와 byte-exact이며 SHA256은
`c65f4d22137600ac2dc03a516274febfb8e98ca4d7e9585cf834d06a3aa8138a`다.

아래는 각각 한 번 실행한 **Debug headless HistoryUpdate 계산 합계**다. CSV/hash 작업은
측정 구간 밖이고 Client/renderer/UI는 실행하지 않았다. 반복 표본의 통계나 실제 FPS가 아니다.

| 문서 묶음 |문서 수|수정 전 합계|수정 후 합계|감소|
|---|---:|---:|---:|---:|
| Artist |19|12,377.73ms|4,976.55ms|59.79%|
| DimensionMaster |14|8,301.96ms|4,495.69ms|45.85%|
| Lance Master |46|10,851.92ms|5,077.06ms|53.22%|
| Warlord |27|6,900.70ms|3,465.59ms|49.78%|
| 전체 |106|38,432.31ms|18,014.89ms|53.13%|

DM Alt V318만의 같은962step은5,090.96→3,060.00ms,39.89% 감소다. 원본 cap·입자 수·clock은
이번 성능 변경에서 바꾸지 않았다. G07의 draw 감소67.56%는 여전히 제출량 계산이고 이 표의
CPU 감소나 실제 게임 FPS와 합산하거나 같은 값으로 부르지 않는다.

### 실제 기하·셰이더 검증

Engine WARP probe는 실제 crack032/037의 identity/nonuniform pretransform에서 원래/준비
VB byte와 rebased IB primitive 순서, slot range·cache hit·clone/morph invalidation·실패 시 기존
cache 보존·200-byte slice offset과 잘못된 buffer 범위를 확인했다. 총182검사 실패0이다.

별도 GPU probe는 production FX와 실제 source packet/DDS를 사용했다. 네 클래스12개 native
lane에 nonuniform/mirror/입자별 색·Dynamic/UV1/SubUV/Warlord WPO 입력을 적용하고 일반 draw와
instance draw를 비교했다. VS stream-output24개와 PS native-state/alpha-overlap48개의 필수
비교72개에서 nonfinite0, 허용 오차 초과0이며 최대 절대차는4.768371582e-7이다.
원래 submesh 순서를 의도적으로 바꾼 대조24개 중16개에서 차이를 검출해 투명 순서 검사의 감도를
확인했다. 이 대조군은 동일성을 요구하는72개에 포함하지 않는다. 결과의 tiny fixture와 수치
readback은 사용자 전체 viewport/FPS나 원본 게임과의 시각 일치를 판정하지 않는다.

최종12개 lane 중11개는 RT0의 양성 RGB 기여도 확인했다. WR258은 이 고정 UV/alpha fixture에서
RGB1e-7 기준을 넘는 기여가 확인되지 않아 일반/instance 입력·출력 동일성까지만 기록한다.
원본 emission/specular 값이 존재하므로 이 결과를 원래 검은 body라는 근거로 해석하지 않는다.

### 빌드·배포·남은 경계

Engine와 Client Debug|x64 전체 Build가 각각 종료0이고 새 shader CSO·Client.exe·Engine.dll을
Client/Bin/Debug에 배포했다. Engine DLL은 Engine/Bin/Debug 원본과 byte-exact다.
full JSON106개와 관련 project/filter 및 out probe XML8개의 parse, git diff --check도 통과했다.
빌드에는 기존 인코딩·생성 shader X4000·외부 library PDB 경고가 남으며 컴파일 오류는 없다.
최종 shader closure도 종료0이다.81개 활성 FxCompile producer/80개 Client consumer와58개
논리 family 프로그램의 실제 FX 생성·정확한 pass 순서·input layout·regular/instance VS 선택을
검사했다. native mesh14/generic mesh7/particle5 계약을 기존 ProductEffectShaderWarpProbe에
반영했고 PS1의 예전36개 고정값은 검증된 정본 실행 테이블 수와 비교하도록 바꿨다. 기존 texture/
parameter/backend isolation 및8개 resource-root/경계 검사도 유지·통과했다. WARP V1/V2는
각각1,352pixel이며 이 작은 fixture의 출력은 사용자 visual PASS가 아니다.

변경 제품13파일과 기존 shader 검사2파일의 원문·patch·SHA, CPU/GPU/기하 로그와 배포 receipt는
`out/FullRestorePerfImplementation20260910/`에 있다. 큰 dirty worktree의 다른 담당 변경을
정리하거나 자동 stage/commit하지 않았다. Client/Server 실행·조작·화면 캡처는 하지 않았다.

Load JSON 비용, 큰 투명 면의 pixel 중첩, 원본 burst/cap·무발생 emitter clock, Artist 꽃밭의
sibling provider, source 광원 초기값은 서로 다른 남은 범위다. 이번 개선으로 해당 원본 복원까지
완료됐다고 기록하지 않는다. 사용자는 Server+Client profile의 Ctrl+F5로 시작한 뒤 Character
Select에서 F1 Effect Tool의 같은 full 문서 전체를 Play해 화면과 FPS를 확인한다.
