# 베른 컷신·기본 이동의 제출 비용 최적화 결과

## G00. 판정과 캡처 근거

현재 가장 큰 계측 구간은 CPU의 불투명 맵 제출이다. 원본 RNM·static-shadow texture가
배치를 나누고, 각 배치에서 재질 바인딩·FX pass 적용·DrawIndexedInstanced가 반복된다.
컷신은 기본 이동보다 Map.Batch.Draw 호출이4.127배이며 한 호출의 CPU 비용은 비슷하다.
GPU timestamp만으로 GPU 사용률을 판단하거나 화질을 낮춰야 한다고 결론 내리지 않았다.

입력은 다음 두 Debug JSON이다. RTX4070,1920×1080, Debug D3D layer·iterator debug2,
Detailed OFF, debugger 미부착 조건이다.

- `Client/Bin/ProfilerCaptures/베른성_컷신_20261004_050800_808_frame104_45964_0.json`
- `Client/Bin/ProfilerCaptures/베른성_기본이동_20261004_051021_021_frame496_45964_1.json`

| 평균 항목 | 컷신 | 기본 이동 |
|---|---:|---:|
| 보존 프레임 | 104 | 120 |
| frame interval | 118.676ms | 40.650ms |
| interval 역수 FPS | 8.43 | 24.60 |
| CPU frame | 117.329ms | 39.821ms |
| GPU timestamp frame | 117.412ms | 40.663ms |
| CPU Render.NonBlend | 59.311ms | 13.476ms |
| Map.Batch.Render | 50.829ms | 11.582ms |
| Map.Batch.Draw | 33.059ms | 7.319ms |
| Map.Batch.Draw 호출 | 2405.5 | 582.9 |
| Map.Batch.Material | 8.685ms | 2.123ms |
| Map.Batch.Pass | 7.171ms | 1.657ms |
| Map.Batch.Visibility | 4.162ms | 2.732ms |
| Ambient.Advance | 13.856ms | 2.890ms |
| NONBLEND PS 호출 | 1000.56만 | 1165.27만 |
| NONBLEND VS 호출 | 322.69만 | 82.30만 |

첫 컷신 frame의 interval은 미계측이며 interval 평균은103개다. 컷신4프레임에서 CPU scope
1313개가 누락되어 그 프레임의 Self는 판정에서 제외했다. 위 cpuWork는 보존된 고정 집계다.
두 캡처의 GPU timestamp는 모두 유효하지만 CPU 제출 공백도 포함한다. 픽셀 호출은 컷신이
더 적고 제출 수·VS 호출은 더 많다. Present0.331/0.036ms, frame gap0.815/0.853ms와
VRAM4.57~4.61GiB/예산10.98GiB는 이 캡처의 주된 시간 증가를 설명하지 않는다.

원래 Profiler.cpp의 독립 Debug 계측 비교에서2406회 제출당 collector 비용은 약1.01ms,
frame 경계 약0.15ms였다. 전체 Map.Batch.Draw33.06ms를 profiler 자체 비용으로 설명할
근거는 없었다. 이 비교는 실제 게임의 driver 내부 대기 원인을 분리한 측정은 아니다.

근거는 `out/BernOptimization20261004/capture-analysis-summary.json`, `analysis-report.txt`,
`out/BernProfilerOverhead20261004/benchmark.log`에 보존했다.

## G01. 배치 분할 원인과 적용 범위

설치 catalog16743개, 재질23200행을 조사했다. geometry·profile·전체 material에서 asset ID만
제외한 동일성으로는 authored-visible 배치16286개 중11개만 줄어든다. 조명 texture path도
구분하는 기존 방식에서는 geometry 이름만 합쳐도 핵심 분할이 남는다.

RNM average/directional·static-shadow의 세 texture만 원래 SRV bank로 전달한다. 현재 범위는
최종 카메라로 준비된 NONBLEND 인접 prefix4~8개, 같은 공유 단일 정적 mesh·비조명 재질·
profile·시계·mirroring, 생성 LOD 없음이다. override·morph·다중 mesh·다른 family·surface
진단 수집 중에는 기존 draw를 사용한다. 표시 순서를 재정렬하지 않는다.

자료 전체의 이름순 인접 조건에서 authored-visible1103개 배치가 감소 가능한 후보였다.
이는 실제 카메라 큐의 절감 개수나 FPS 개선 측정값이 아니다. 모든 variant를 합치는 기능이나
컷신 단일 자리 FPS의 완전 해결을 이 수치만으로 선언하지 않는다.

## G02. 구현한 파일과 실패 경계

- `Engine/Public/GameObject.h`, `Engine/Private/GameObject.cpp`, `Engine/Private/Renderer.cpp`:
  기존 NONBLEND 큐에 opt-in 인접 제출을 연결했다. 기본 객체는 한 개를 그리며 잘못된 소비
  개수와 실패는 MRT·큐를 정리하고 프레임 실패로 반환한다.
- `Engine/Public/Model.h`, `Engine/Private/Model.cpp`, `Engine/Public/Material.h`,
  `Engine/Private/Material.cpp`: 공유 geometry·pretransform·실제 전체 material 값을 비교한다.
  padding memcmp를 사용하지 않으며 비조명 SRV·legacy texture·tint·override를 검증한다.
  원래 조명 SRV를 최대8개로 바인딩하고 성공한 마지막 단계에 bank count를 켠다.
- `Client/Public/MapStaticBatchObject.h`, `Client/Private/MapStaticBatchObject.cpp`:
  원래 batch 가시 payload의 순서를 유지해 전용 재사용 buffer에 복사한다. 복사본의
  directionalScale.w만 bank index로 쓴다. 준비 실패는 draw 전에 원래 Render로 복귀하고,
  제출 뒤 실패에는 중복 draw를 하지 않는다. 기존 visibility·shadow·placement는 보존했다.
- `Client/Public/MapAssetRenderUtils.h`, `Client/Private/MapAssetRenderUtils.cpp`:
  기존 surface 진단 lease의 활성 상태를 읽는 getter를 추가했다.
- `Client/Bin/ShaderFiles/Shader_VtxMeshMapInstance.hlsl`, `Shader_MapMaterialSurface.hlsli`,
  `Shader_StaticShadowMap.hlsli`: 원래 RNM·signed-distance shadow 식을 공유하며 실제 SRV의
  크기·mip·포맷·sRGB·sampler를 유지한다. 기존 pass0~26을 유지하고 bank용27~29만 추가했다.
  일반 SOURCE_BG24~26은 별도 shader로 컴파일하여 추가24개 SRV 비용을 전파하지 않는다.
  StaticShadow include는 `Engine/Bin/ShaderFiles/Shader_StaticShadowMap.hlsli` 정본에도
  반영했다. 첫 Product 시도에서 PrepareEngineSdk가 원래 Engine include로 Client 수정본을
  덮어써 helper 미정의 오류가 발생했으며, 정본을 수정해 배포 시 유지되도록 교정했다.

새 제품 C++ 파일·project/filter 변경·자료 재게시·Resources 변환은 없다. 렌더링 옵션과
팀장 저장값을 바꾸지 않았다. 기존 파일 인코딩·줄바꿈과 다른 세션의 변경을 보존했다.

## G03. 기본 이동의 실패 탐색

캡처 frame404/408/412의 Navigation.AStar는34.831/34.306/40.534ms이며16384노드 상한까지
확장하고 경로가 없었다. 설치 Bern navgrid의 목표는 주변49m대 바닥과 연결되지 않은
높이52.049m의62셀 영역이었다. 도착 셀의 walkable만으로 현재 바닥과 연결됐다고 볼 수 없다.

`Engine/Public/PathFinder.h`, `Engine/Private/PathFinder.cpp`에서 목표의 incoming Can_Step
간선을 최대128셀 역탐색한다. 출발점 없이 연결 성분을 완전히 소진했을 때만 UNREACHABLE을
반환한다. 출발점 발견·예산 초과는 새 generation에서 기존 A*를 실행한다. 동적 blocker·높이·
대각선 조건과 성공 경로의 선택·기존16384 상한·Server 명령을 유지한다.

원래 생산 CPP 전체를 사용한 독립 Debug/Release 비교를 각각2034사례 실행했다.
설치 Bern의18개 요청, height/diagonal/blocker 변경,127·128·129셀 경계와 random2000개다.
정상 경로·hash·확장 노드는 동일하고 false success0, 증명된 실패370개만 조기에 반환했다.
캡처의 세 실패는 A* 확장16384→0이다.

독립 Debug median 실패36.393ms→0.0236ms, 정상27.028μs→60.062μs였다. 정상 요청에는
약33μs probe 비용이 추가된다. Release 실패4.560ms→2.167μs, 정상1.216→4.968μs다.
이 수치를 게임 FPS 개선으로 환산하지 않는다. 근거:
`out/BernOptimization20261004/navigation/verification.json`.

## G04. 검증 상태

완료된 자동 검증:

- 변경 Engine CPP4개와 Client MapStaticBatchObject CPP의 실제 Debug 집중 컴파일 통과.
- 실제 Material 타입·비교/바인딩 본문과 WARP SRV를 사용한206검사 통과. Shader setter는
  fixture 경계이며 실제 GPU sampling 검증과 구분한다.
- 실제 Client 인접 제출·upload 본문 비교34검사 통과: 순서·원본 payload 보존·reuse·거부 조건·
  준비 실패 복귀·draw 실패 뒤 중복 제출 금지,2~3개 제외와4개 이상 병합을 포함한다.
- PathFinder 생산 CPP의 Debug/Release2034사례씩 통과.
- 최종 bank 분리 HLSL의 FXC fx_5_0 /O1 컴파일 통과.
- 실제 설치된 library-prop RNM 두 재질을 생산 CMaterial.cpp로 초기화한 WARP 검사 통과.
  공통 legacy/native SRV는 공유하고 조명 SRV는 다른 상태에서 병합 가능했다. 대표 재질만
  100회 바인딩한 뒤에도 입력 동등성을 유지했다.
- 실제 FX11 VS/PS·192byte instance와8개 RGBA32_FLOAT MRT로 WARP 및 RTX4070을 각각
  검사했다. Debug D3D layer ON, normal/specular×baked×shadow 조합64개 출력 비교 모두
  bitwise 동일, maxAbs0, D3D error/warning0이었다. 원본 texture는4~64px의 서로 다른
  크기·전체 mip·linear/sRGB SRV를 사용했다. 이는 사용자 장면 전체 화면 비교는 아니다.
- 실제 shader reflection에서 기존/수정 ordinary pass24 모두 bank SRV0개와 bank count
  미사용, bank pass27만 bank SRV24개와 count 사용임을 확인했다.
- 정상 Product가 생성한 실제 `Client/Bin/Debug/Shader_VtxMeshMapInstance.cso`도 RTX4070에서
  직접 로드했다.64개 MRT 비교는 bitwise 동일/maxAbs0이며 D3D error/warning0, ordinary/bank
  reflection도 동일하게 통과했다. 생성본292399bytes, SHA256
  `1d16829ee3a25abcb25e2e663a67a2fa79ef64e952b9509e35f45f0cc29839b7`이다.
  근거는 `out/BernOptimization20261004/gpu-parity/product-cso/verification.json`이다.
- 최종 성능 비교는 제품 빌드가 끝난 뒤 HLSL 컴파일 없이 수행했다. baseline도 O1으로 맞추고
  실제 Product CSO를 사용했다. RTX4070 Debug layer ON,31회 prefix/모드 교차×64group이며
  일반 draw와 bank의 resource bind+FX Apply+draw CPU 중앙값은 아래와 같다.

| 인접 개수 | 수정 shader의 일반 draw | bank1draw | 적용 판단 |
|---|---:|---:|---|
| 2 | 18.736μs | 34.669μs | 회귀, 기존 draw 유지 |
| 3 | 28.319μs | 30.641μs | 회귀, 기존 draw 유지 |
| 4 | 37.063μs | 29.205μs | 병합 허용 |
| 8 | 77.042μs | 17.245μs | 병합 허용 |

작은 묶음도 이득이라고 가정하지 않고 Client 최소 병합 수를4개로 확정했다. GPU 완료 대기·
비교·CPU instance 합성/upload·전체 재질 상수 바인딩은 측정 밖이므로 게임 FPS 개선율로
쓰지 않는다. bank는 실제 제품처럼 첫 재질의 ordinary SRV3개 설정과8slot 배열3개를
포함했다. D3D 오류·경고는0이다. 근거:
`out/BernOptimization20261004/gpu-parity/hardware-prefix-stable/verification.json`.

증거는 `out/BernDrawEngine20261004`, `out/BernLightingBank20261004/client`,
`out/BernOptimization20261004/gpu-parity/{warp,hardware}`에 보존한다. 정상 Debug Product
전체 빌드·배포는 `out/BuildPipeline/runs/20261003T211650083Z-debug-product.json`에서 PASS이며
필수 runtime 입력 누락/검사 오류는0이다. Engine DLL과 import library의 원본·배포 hash도
일치한다. 최소 병합 수4개 보정도 최종 증분 Product에서 PASS했다:
`out/BuildPipeline/runs/20261003T212432338Z-debug-product.json`.
마지막 Client 단계는10.239초, OBJ1개·EXE1개만 갱신했고 추가 CSO 컴파일은0개다.
최종 실행 파일은 `Client/Bin/Debug/Client.exe`다. 새 Client의 같은 두 구간 캡처·화면 판정은
아직 실행하지 않았다. Client/Server를 에이전트가 실행하거나 UI를 조작하지 않았다.

## G05. Ambient.Advance의 별도 반복 비용

컷신의13.856ms 평균에는 초반15프레임의 fixed-step 따라잡기가 집중된다. 이15프레임의
Ambient 평균은45.867ms이며, 완전 계측된 frame5의43개 효과는24개×9단계와19개×60단계를
실행했다. 해당 Ambient45.355ms 중 Step43.650ms이고 ParticleUpdate28.531ms,
Spawn8.680ms, Rebuild1.340ms였다. 단순히 효과 개수가 늘어난 현상은 아니다.

이미 쌓인 accumulator와 긴 frame의 반복 시뮬레이션이2차 비용을 만들지만 최초 backlog
유입 시점·asset ID는 이 캡처에 없다. offscreen catch-up 또는 특정 effect를 원인으로 확정하지
않았다. MAX_CATCH_UP_STEPS60을 임의로 낮추거나 시뮬레이션을 생략하면 age·spawn·수명·
부착 타이밍이 바뀔 수 있어 이번 소스 변경에 포함하지 않았다. draw 개선 후 재캡처에서도
누적이 남는지 확인한 뒤 같은 상태·age 결과를 보존하는 범위에서 별도로 줄여야 한다.
근거: `out/BernProfilerOverhead20261004/ambient-analysis.json`.

## G06. 남은 확인

사용자가 새 Debug Client에서 같은 컷신·기본 이동을 다시 저장해 실제 Map.Batch.Draw 호출,
Material·Pass·NonBlend 시간과 frame interval을 비교해야 한다. 원래 RNM·그림자·표시 순서와
이동 입력도 함께 확인한다. 단일 mesh/noLOD 범위 밖의 배치와 Ambient.Advance 비용은
남아 있으며 전체 컷신의 목표 FPS 달성을 주장하지 않는다.

## G07. 사용자 12:45 새 캡처의 저장 범위와 병목 한계

사용자 파일 `Client/Bin/ProfilerCaptures/베른_컷신_20261004_124545_628_frame170_44204_0.json`
SHA256은 `725be83e133ffa8ac1b72401c5891d35a6c7645c3403754cda7ed4f5c307e746`이다.
170개 수집 history 중 JSON 상세는 최근 120개(frame 51~170)만 포함한다. 앞쪽 50개는
수집 불가가 아니라 저장 window에서 제외됐다. 제외 구간 최대 interval 1122.9942ms가 있어
약 1FPS 순간이 있었음은 확인되지만, 그 구간의 느린 frame 수·연속성·scope 원인은 파일에 없다.
사용자가 관찰한 10개 이상 느린 frame이 모두 보존됐다고 주장하지 않는다. 저FPS라서
수집할 수 없다는 설명은 잘못이며 Profiler 기본 저장 window와 분석 window 결합이 원인이다.

저장된 120개만의 평균 interval은 126.316ms(7.92FPS), 최대 277.492ms이다. CPU frame 평균
123.362ms, Map.Batch.Render 53.062ms(내부 Draw 34.627/Material 9.291/Pass 7.727),
fallback Map.Object.Render 10.684ms, Ambient.Advance 18.207ms가 측정됐다. GPU query는
120/120 valid지만 CPU scope는 10개 frame에서 22,182개가 누락됐다. detailed=false로
per-mesh GPU 원인은 없다. navigationQueries는 모두 0이며 Present 평균 0.146ms다.
이 구간의 지속 비용을 제외된 1FPS 구간의 확정 원인으로 대신 기록하지 않는다.

저장된 메모리 표본은 VRAM budget 최대 46.75%, 최소 물리 RAM 여유 31.9GiB다. 이후
World Level 편집 클릭 시 EXE 종료를 이 캡처로 OOM이라 확정할 수 없다. 해당 시점 새 dump,
WER 또는 종료원인 로그도 발견되지 않았다. 편집에서 Bern 재질 JSON 95,342,906byte를
UI thread에서 다시 전체 파싱하는 부담은 별도 소스 경로로 확인했고 F1 도구 수정에서 다룬다.

재현 분석과 05:08/05:10 이전 캡처 비교는
`out/BernCliffFrameInvestigation20261004/performance/findings-and-source-consumers.json`,
`comparison-summary.json`, `analyze_current_capture.py`에 보존했다. 이 조사는 원래 1FPS
구간의 원인을 확정하거나 성능 개선 완료를 입증하지 않는다. 저장 범위 개선 후 동일 구간의
전체 상세 캡처가 필요하다. 에이전트는 Client를 실행하거나 rendering option을 변경하지 않았다.

## G08. 사용자 14:34 전체 캡처에서 확인한 연속 1FPS 구간

새 `베른_컷신_20261004_143459_089_frame339_77492_0.json`은 119,073,515byte이며 SHA256은
`9bd24cf8fd3e875ce44d087b1fa67793fed1f3b21d9638c36cb7e38eb8b1050a`다. 보유/저장339개,
저장창 제외0/퇴출0으로 이번에는 앞쪽 저FPS frame도 모두 포함한다. interval frame2~16의
15개가 연속으로 1.040~2.071초, 합계18.514초, 평균1.234초(약0.81FPS)다.
interval N은 CPU frame N-1에 대응한다. 이 등식을15개 전부 확인했으며 frame gap 평균은
0.884ms로 프레임 사이 대기가 초 단위 지연을 설명하지 않는다.

| CPU 원인 frame1~15의 별도 누적 계측 | 프레임당 평균 | 평균 CPU frame 대비 |
|---|---:|---:|
| 전체 CPU frame | 1,233.399ms | 100% |
| Ambient.Advance | 573.308ms | 46.48% |
| Map.Batch.Render | 338.094ms | 27.41% |
| Map.Object.Render | 62.568ms | 5.07% |

Map.Batch.Render 안에는 Draw196.730ms, Material99.426ms, Pass32.899ms가 포함된다.
부모와 자식 비용을 다시 더하지 않는다. 배경 효과 갱신은43~47회, batch render는5,602~6,757회,
batch draw/material/pass는각각8,366~9,973회다. 후기frame200~339는 Ambient.Advance가37회/
3.969ms, Map.Batch.Render가528회/16.663ms, 전체 CPU가47.050ms로 낮아진다.
저FPS15개와 회복된140개를 섞은 평균으로 원인을 설명하지 않는다.

실제 느린 구간의 저장된 FixedStep은 Effect.Playback.Update 내부에서 발생한다.
Update 밖 FixedStep0, HistoryUpdate0이며 frame1~76에 ordinary ambient Update의60step
상한 도달이 관측된다. frame18은 새 wall delta487.982ms(약29step)인데31개 effect가60step,
frame60은254.671ms(약15step)인데37개 effect가60step을 수행한다. 새 delta만 처리한 것이
아니라 이전부터 남은 시뮬레이션 시간이 계속 소모되는 증거다. 후기frame200/300은37개 effect가
각각2~3step 수준이다. Prewarm 평균0.0923ms와 retained FixedStep의 부모를 함께 확인해
반복 prewarm/Seek가 주원인이라는 가설과 구별했다.
또한 frame72~76은 CPU scope drop0인데 새 interval155~171ms(약9~10step)에 비해 여러
ambient 효과가60step을 수행한다. 따라서 backlog 증거는 상세 scope가 누락된 frame에만
의존하지 않는다. frame1~15의 Ambient 시간 중99.893%는 일반 Playback.Update 내부였다.

소스의 Timer_60은 실제 wall delta를 넘기고 Effect_PresentationService의 ambient tick은 이를
playback rate와 곱해 사용한다. 숨긴 동안의 delta는 overwrite되므로 offscreen 전체 시간을
쌓았다가 복귀 때 재생하는 경로와 다르다. Effect_Playback의 일반 Update는 누적기에 시간을
더한 뒤1/60초 step을 effect당 최대60번 처리하고 미처리 잔량을 남긴다. 따라서 느린 frame이
다음 frame의 더 많은 따라잡기 작업으로 이어지고, 그 작업이 다시 frame을 늘리는 반복 경로가
현재 코드와 실제 계측에 함께 나타난다. 많은 맵 draw 제출도 같은 구간의 독립적인 큰 비용이다.
최초 지연을 무엇이 시작했는지는 frame1이 이미60step 상한 상태여서 이 파일만으로 단정하지 않는다.

GPU339개 모두 valid이고 pending/partial GPU scope0이다. 하지만 GPU timestamp는 CPU의
명령 공급 대기도 포함하므로 GPU elapsed만으로 shader 포화나 특정 지형 재질을 지목하지 않는다.
CPU 상세 scope는 전체71개 frame에서272,850개, 저FPS 원인15개에서88,608개가 capacity로
누락됐다. **완료 frame 누락과 상세 scope 누락은 다르다.** 위 cpuWork는 별도 누적 계측이라
표의 전체 작업 비용은 확인 가능하다. 누락된 자식의 정확한 self 비용이나 effect별 완전한 호출
횟수는 주장하지 않는다. 60개가 실제 저장된 개별 Update는60step 상한 도달의 직접 근거다.
effect asset/placement ID와 accumulator 잔량은 현재 capture에 없어 개별 emitter 지목은 남는다.

수정 우선 대상은 일반 배경 이펙트의 누적시간 처리·한 frame 작업량과 이 구간의 맵 draw 제출이다.
단순히 전역 delta를 줄여 gameplay 시간을 바꾸거나 누적기를 버리는 변경은 이번 조사에서 하지
않았다. 원인 영역을 확인한 결과이며 성능 수정·개선 후 재측정 완료로 기록하지 않는다.
이 캡처의 export metadata는 Debug/D3D debug layer ON이지만 모든 과거 frame의 설정·카메라를
증명하지 않는다. Client/UI 실행·빌드·렌더링 옵션 변경도 하지 않았다.

근거는 `out/BernCutscene1Fps20261004_143459/summary.json`, `all-15-low-intervals.json`,
`all-frame-attribution.json`, `effect-step-parent-attribution.json`과 같은 폴더의 분석 스크립트다.

## G09. 배경 시각 시간의 지연 누적 방지

기존 deferred ambient admission에만 frame당 visual delta 최대0.1초를 적용했다. rate를 곱한
입력에서 제한하며 실제 visible Advance와 service elapsed에 같은 값을 넣는다. 초과분은 다음
frame으로 전달하지 않는다. 숨김 pause, 첫 Seek, owner/level 오류 제거와 render 제출은 기존 흐름이다.
정상적인1/60 fixed-step 적분·입자 age/spawn/RNG·source loop는 그대로이며 일반 Update,
transform history, authoring Seek, 전투와 Server의 시간은 변경하지 않았다.
기존 승인 후 root 편집으로 bounds만 무효가 된 인스턴스도 같은 배경 시각 시간 정책을 유지한다.

이 변경은 긴 frame 동안 배경 시각 시계를 느리게 한다. 건너뛴 실제 시간을 나중에 따라잡거나
입자 clock만 순간이동하지 않는다. 원본의 실제 시간 기준 phase와 동등하다는 주장은 하지 않는다.
Bern91개 SOURCE_LOOP 중 sprite85배치와 mesh 혼합6배치를 새롭게 합쳐 승인하지 않았으며,
이번 제한은 원래 static-sprite bounds를 통과한 경로에만 적용한다.

프로파일러에 다음4개 counter와 동일 UI 이름을 추가했다. 기존 counter 순서는 보존했다.

| JSON counter | 의미 |
|---|---|
| effectAmbientClampedUpdates | visible Advance 중 입력 시간이 제한된 효과 수 |
| effectAmbientDiscardedMicroseconds | 효과별 제외 시간의 합계. frame wall time·절약 CPU 시간이 아님 |
| effectAmbientFixedSteps | initial Seek·hidden frame을 제외한 실제 committed simulation step 합계 |
| effectAmbientMaxFixedSteps | 같은 frame의 단일 ambient Advance가 commit한 최대 step 수 |

step은 두 읽기 전용 getter를 통해 기존 uint64 simulation step의 전후 차이를 센다.
기존 Get_FixedStepClockSeconds는 accumulator까지 포함하므로 횟수로 환산하지 않았다.
새 counter는 raw CPU scope capacity와 독립적이며 capture의 measurementSemantics에도 단위를
기록했다. 비유한 입력은 visual delta0이며 제외 시간 표본을 만들지 않는다. 제외 시간 합계는
uint64 계측 용량을 넘을 때만 포화한다. 새로운 재생 runtime·제품 C++ 파일·리소스는 없다.

## G10. 카메라 속도 가설의 확인 범위

실제 입장 데이터는 `Data/Encounters/Bern/BernEntranceCamera.json`의16초/16key 곡선이며
FOVY60°다. 기본 follow의32.642°보다 같은 거리의 수직 범위를 약1.97배 넓게 본다.
Level_Bern은 이미 컷신 시간을 frame당0.1초로 제한한다.1FPS일 때 카메라는 실제1초에
컷신0.1초만 진행하므로 빠른 wall-clock 카메라 이동 때문에 메시가 밀린다는 가설은 확인되지 않았다.

맵 geometry는 입장 scope에서 준비되며 현재 카메라로 동기 가시성을 계산한다. Bern의3frame
reject grace로 직전 가시 대상이 잠시 함께 제출될 수는 있으나 이 비용의 비율은 기존 capture에 없다.
느린15frame의 가시 맵은 평균19,676개, draw10,597회이며 후반에는961개/1,057회다.
속도만 낮춰도 같은 pose의 FOV와 가시 대상 수는 줄지 않으므로 카메라·FOV·화질은 변경하지 않았다.
맵 제출 비용과 최초 지연 유발 요인은 후속 캡처에서 계속 구분해야 한다.

근거: `out/BernCutscene1Fps20261004_143459/camera-speed-readonly-audit.json`,
`ambient-visual-delta-source-receipt.json`. 현재 파일과 수치 모델의 결과이며 실제 캡처의
frame별 카메라 pose가 저장됐다는 뜻은 아니다.

## G11. 이번 수정의 검증과 남은 실행 확인

실제 Service.Update/Submit/helper, Object.Advance와 Playback.Update·getter 본문을 추출한
headless native fixture31개 검사가 통과했다. Step은 관찰 경계로 대체했으므로 호출 예산과
입력·시계 소비의 증거이며 particle 최종 좌표나 GPU 표시 동등성을 주장하지 않는다.
수명·spawn·RNG를 포함한 Playback.cpp와 Object.cpp는 실제 diff가 없다.

반복2초/1초 지연 뒤 정상1/60초 입력에서 이전 코드는60step과4.01667초 잔량을 유지했고,
새 경로는1step으로 돌아왔다.339,000회 소수 delta에서도 committed clock 오차0을 확인했다.
직전 잔량이 tick 경계에 가까운 경우에는7step이 가능하다. 실제 저장339개 interval을 초기
accumulator0에서 재생한 모델은 기존3,344step/상한60회 도달21frame, 수정1,417step/최대6회였다.
실제 캡처 시작 전 잔량과 effect별 활성 이력은 없으므로 이 수치를 캡처의 전체 호출 수나 FPS
개선율로 대신하지 않는다.

Step 실패를10회 주입하면 기존 accumulator가 남아 회복 때60step이 가능함도 확인했다.
따라서6~7step은 새 인스턴스에서 정상 Step 성공이 이어질 때의 범위이며 절대 상한이 아니다.
현재 정적 승인과 Bern11개 문서는 실패를 유발하는 model anchor/event 구성을 포함하지 않는다.
비정상 실패·회복 시 새 counter는 실제60회를 숨기지 않는다. 실패 시계와 일반 재생 계약을
이 최적화에서 임의 초기화하지 않았다. hidden/resume, initial Seek, stale owner/level,
render failure 제거, root 이동, rate 적용 후 제한과 일반 combat/history 비적용도 검사했다.

실제 Profiler.cpp·ProfilerCaptureIO.cpp·DataJson.cpp로 만든 headless exporter/importer는
native54개와 JSON/호환성33개 검사를 통과했다. 기존88개 ordinal/이름은 유지되고 새4개가
추가됐다. raw scope8192개 누락 중에도 counter가 저장된다. 이전v3의 없는 키는 미측정,
명시적0은 측정0으로 유지하며 malformed 입력은 이전 분석 상태를 보존한 채 거절한다.

정상 Debug Product 증분 compile/deploy가 성공했다:
`out/BuildPipeline/runs/20261004T062359755Z-debug-product.json`.
현재 checkout의 다른 세션 렌더링 변경도 포함한 빌드이며 이번 commit에는 이 기능의 변경만 묶는다.
기존 인코딩·형변환 경고는 남아 있고 새 Client/UI를 실행하지 않았다. diff-check도 통과했다.
Release 빌드와 실제 새 EXE의 컷신 FPS·장식 효과 화면 확인은 이번 검증에 포함하지 않는다.

검증 재료는 `out/BernAmbientCatchupFix20261004/production-body-fixture-receipt.json`,
`run_fixture.py`, `profiler/verification-receipt.json`에 보존한다. 사용자는 새 Debug EXE에서
F7 Capture를 켠 뒤 같은 입장 컷신을 저장하고4개 새 counter와 Ambient.Advance·Map.Batch.Render를
함께 비교한다. 맵 제출 비용과 최초 지연 원인이 모두 해결됐다고 결론내리지 않는다.

## G12. 사용자 컷신2 재측정: 반복 제한 적용과 남은 저FPS

`베른_컷신2_20261004_153522_285_frame217_80032_0.json`은217개 수집/저장, 제외/퇴출0,
CPU/GPU scope 누락0이다. 사용자는 컷신 재생만 저장했다고 확인했다. 별도로 F6로 비슷한
위치를 이동하면20FPS 이상이라는 관찰을 전달했지만, 이 파일에 F6 비교 구간이 있다는 뜻은 아니다.

interval frame2~25의24개가400ms 이상이고 대응 CPU1~24 평균은597.400ms다.
Map.Batch.Render292.591ms, Map.Object.Render55.172ms, Ambient.Advance65.367ms이며
렌더 전체475.997ms, 업데이트116.676ms다. Map.Batch.Draw164.477/Material92.319/
Pass28.014ms는 batch render의 자식이므로 다시 더하지 않는다. 느린 구간 batch render는
평균5,599회, 전체 draw9,771.75회, 가시 맵18,021.42개다. 후반 CPU198~217은57.487ms,
batch17.917ms/583회, draw1,200.45회와 가시 맵1,016개다. 양 끝은 동일 카메라 조건의 A/B가 아니다.

새 effectAmbientMaxFixedSteps는 전체 최대6, 느린24개도 모두6이다. 이전 지연 누적 방지의
작동은 실제 캡처로 확인했지만 컷신의 저FPS를 해결하지 못했다. 제한된 배경 효과의 갱신 비용도
0이 아니며 입장 직후의 많은 맵 제출 비용이 계속 남는다. CPU밖 frame gap 평균2.340ms와
Profiler.Panel.Refresh6.517ms·Memory.Sample4.698ms만으로597ms를 설명할 수 없다.
frame197은 interval0이므로 앞 frame의 CPU에 대응시키지 않는다.

카메라 source audit에서는 문서 load/Begin override의 매frame 반복이나 camera matrix revision의
같은 frame 내 폭증을 찾지 못했다. moving frame의 batch visibility rebuild는15,794회이고
가시성 판정이 전부 통과하는 fail-open 상태도 아니다. 실제 sampler/camera/frustum 본문을
사용한10,568pose 검사는 같은 pose/FOV에서 cinematic과 free의 view/projection 차이0을 보였다.
이 CPU 검사는 실제 동일경로 재생의 GPU 비용이나 FPS 동등성을 증명하지 않는다.

다음 비교는 사용자가 요청한 F1 재생 버튼으로 초기 입장과 이후 반복 재생의 동일 경로·FOV를
확보해 수행한다. 카메라 속도/FOV/화질을 바꾸거나 이번 결과를 단일 근본 원인 확정으로 기록하지 않는다.
근거는 `out/BernCutscene2Investigation20261004/independent/summary.json`,
`frame-rows.json`, `slow-interval-pairs.json` 및 `camera_render`의 source probe다.

## G13. F1 Bern 입장 컷신 반복 재생

Debug Bern의 F1 `Camera`에서 자유 이동 속도와 Reset 버튼 바로 아래에
`Start Bern Cutscene`을 추가했다. UI는 요청만 제출하고 다음 Level Update에서 현재 상태를
재검사한 뒤 기존 입장 컷신의 sampler·경로·FOV·재생 시간으로 실행한다. 자유 카메라 속도는
이 재생 시간을 바꾸지 않는다. 반복 재생과 같은 process의 Bern 재입장을 모두 지원한다.

재입장 때문에 cue가 준비되지 않은 경우에만 원본 문서를 임시 후보로 읽고 첫 pose까지 검증한
뒤 반영한다. 최초 자동 재생 latch를 초기화하거나 정본 JSON을 저장하지 않는다. 정상 종료와
ESC는 기존 camera override 종료 경로로 시작 전 pose/FOV와 follow/free 요청을 복원한다.
중복 재생, 다른 camera owner, 레벨 전환·캐릭터 복원, 연결 종료와 placement 편집 충돌은
기존 상태를 유지하고 이유를 표시한다. 새 제품 파일이나 프로젝트 등록은 필요하지 않았다.

실제 Can/Request/Consume/Ready/Update/End 본문, DataJson·Bern parser와 shared Sample_Cue를
실행한 native fixture25개 검사가 통과했다. 원본16key/16초, 반복·재입장, queued 요청의
비변경, 중복, ESC edge/held ESC, consume 직전 owner 변경, 잘못된 문서·첫 pose,
Begin/Apply 실패와 각 guard를 확인했다. Camera/Transform·network·ImGui는 경계 spy이므로
실제 화면·GPU 행렬이나 FPS 개선 증거로 대신하지 않는다. 기존 camera contract4개도 통과했다.
독립 소스 리뷰에서 추가 수정이 필요한 결함은 없었고 scoped diff-check도 통과했다.

Debug Product 증분 compile/deploy는37,763ms에 PASS했다:
`out/BuildPipeline/runs/20261004T065129859Z-debug-product.json`.
같은 checkout의 다른 세션 미커밋 렌더링 변경을 포함한 빌드이며 이번 commit은 컷신 재생
기능과 대응 문서만 포함한다. 검증 중 소스·정본 카메라 JSON hash는 유지됐다.
재현 자료는 `out/BernEntranceReplay20261004/run_native_fixture.py`,
`native-replay-fixture-receipt.json`과 `native-replay-fixture-run.log`다.

Client/UI와 Release 빌드는 실행하지 않았다. 사용자가 새 Debug 실행 파일에서 F7 Capture를
켠 뒤 F1 버튼으로 같은 경로를 재생하고 저장해 최초 입장과 비교한다. 이 변경은 비교용 재생
진입점이며2FPS의 원인을 해결했거나 반복 재생 FPS가 개선됐다고 기록하지 않는다.
