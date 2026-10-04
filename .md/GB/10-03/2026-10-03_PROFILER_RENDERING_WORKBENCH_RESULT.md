# Profiler 상세 계측·프레임 비교와 Rendering Workbench 구현 결과

## G00. 반영 범위와 Git 동기화

2026-10-03, 최초 확정 범위는 **확장 설계 + Profiler 구현**이었다. 이후 사용자가 프레임 차이·
쿠크 2관문/빙고의 구조 비교와 Workbench의 실제 A/B·수치 튜닝·개념 설명까지 구현을 요청했다.
`GPU GEN`은 `GPU Gems` 자료 모음으로 확인했다.
최초 Profiler는 실제 Engine 수집, Debug F7 표시, JSON 저장까지 연결했다.
추가 요청의 반영 범위는 아래 후속 절에서 구분한다.
[구현 계획서](2026-10-03_PROFILER_RENDERING_WORKBENCH_IMPLEMENTATION_PLAN.md)에 현재 옵션 실험과
새 renderer 기반 도입을 분리했다. 첫 병합 이후 추가한 상용 엔진 비교·메모리·시간축과
화면 공간 GI/SSR의 현재 상태는 아래 G10 이후에 구분한다. Lumen/DXR/Nanite 전체 구현은 아니다.

사용자의 후속 요청에 따라 `origin/main`을 fetch하고 `dc3a482d424b5d1317a1adb196843e3a97253388`
기준으로 **`GB/Advanced-Rendering-Profiler-WorkBench`** 브랜치를 만들었다.
`abeac743a`의 차원술사 가이드 건물 출입 동행·연결된 착지 수정, Bern Navigation 복구,
Movie 성능 및 Profiler CaptureWindow·SelfComplete 변경을 함께 수신했다.
기존 Profiler 작업과 겹친 네 파일은 양쪽 계약을 보존해 병합했다.

동기화 전에 기존 추적 변경과 미추적 자료를 별도 사본 및
`safety/2026-10-03-profiler-before-main-sync` stash
(`18f8a3a12e6270f98941ae17ec6aabf84e012c9d`)로 보존했다. 기존 미추적 파일 18개의
원본 바이트와 shader 입력 664개의 바이트·수정 시각이 동기화 과정에서 유지됐음을 확인했다.
사용자 `.gitignore`, 개인 문서, 저작 backup·retired 자료는 이 기능의 변경에 포함하지 않는다.

## G01. 실제 구현한 계측

| 경계 | 반영 내용 | 수치의 의미와 한계 |
|---|---|---|
| 한 프레임 시간 | CPU 시작·종료 tick, 이전 CPU 시간, 프레임 밖 간격 | 시작 간격 = 이전 CPU 처리 + 이전 종료부터 현재 시작까지의 간격. 현재 CPU 시간과 잘못 합산하지 않는다. |
| CPU 병목 | 기본 main-thread Self 내림차순, inclusive·worker 선택, 계측된 시간 합집합과 나머지 표시 | 중첩 구간과 worker를 프레임 시간에 중복 합산하지 않는다. 누락 표본이 있으면 SelfComplete를 따르고 Self는 `--`, 정렬은 inclusive로 전환한다. |
| GPU 병목 | 패스별 전체·Self·P95·최대, IA 정점·primitive, VS·PS 호출 | timestamp와 query가 유효한 프레임만 집계한다. GPU elapsed는 순수 연산 포화율이나 FLOPs가 아니다. |
| 패스 작업량 | GPU scope begin/end의 draw·instanced draw·instance·index·mesh 누계 차이 | CPU에서 제출한 작업량이며 GPU 결과의 원래 frame ID에 귀속한다. 부모 패스는 자식 제출도 포함한다. |
| 메시 | CMesh 실제 draw 지점의 호출·인스턴스·인덱스·프레임 고유 mesh 수 | 일반 draw의 mesh instance는 1이다. index는 instance를 곱한 제출량이며 최종 가시 삼각형 수가 아니다. |
| 직접광 | `Render.Lights.WorldReceivers`, `Render.Lights.CharacterReceivers` CPU/GPU 구간 | 월드와 초상에서 사용하는 실제 Light_Manager 경계를 분리한다. 개별 광원의 shader 명령 비용을 산출한 것은 아니다. |
| JSON | 기존 `LostArkProfilerCapture.v3`에 시간·패스·mesh 필드와 `measurementSemantics` 추가 | 기존 필드·CaptureWindow·비동기 원자 저장을 보존한다. 비정상 시간값이면 이전 저장 파일을 유지한다. |

고유 mesh 집계는 프레임마다 고정 hash table을 재사용한다. 최대 16,384개 이후에는
고유 개수를 하한값으로 표시하고 누락 수를 기록하며 제출 총량은 계속 누적한다.
메시 주소는 프레임 내부 중복 제거에만 사용하고 JSON으로 내보내지 않는다.
추가 counter 다섯 개를 포함해 총 88개 counter의 저장 이름과 enum 크기를 정적으로 확인한다.

기존 캡처 OFF, GPU query ring, 비동기 DONOTFLUSH 읽기와 이력 상한을 유지한다.
draw마다 query·동기 readback을 추가하지 않았다. 기본 고유 mesh 집계는 고정 table이며,
추가 상세 목록은 bounded vector와 최초 이름 등록 비용이 있으므로 상세 OFF와 구분해 측정한다.
패스별 index에는 기존 indirect의 실제 실행량을 추측해 넣지 않는다. 기존 indirect LOD0 상한은
예약 counter이며 현재 제출 생산자가 없어 미계측이다. Engine 밖 ImGui/DirectXTK 내부 draw는 이 제출 counter에 포함되지 않는다.

## G02. 한국어 화면과 사용 순서

Debug F7의 첫 탭에 `한 프레임 해석`을 두고 CPU/GPU 비용, 프레임 밖 간격, 메시와 조명
작업량의 의미를 함께 표시한다. CPU·GPU·작업량·긴 작업 표와 저장 범위 안내는 한국어로
표현하고, `World.Render` 등 원래 scope ID는 코드 검색을 위해 유지했다.
검색은 한국어 설명과 영문 scope ID 모두 사용한다.
생산자가 없는 texture cache counter는 0을 측정 성공처럼 표시하지 않고 미계측으로 안내한다.

`ProfilerTool.cpp`의 기존 UTF-8 BOM 없는 파일 인코딩을 유지하고 해당 컴파일 항목에
`/utf-8`, PCH 미사용을 설정했다. 새 기법 사전 header와 프로젝트 등록은 G05에 기록한다.
Release의 F7 비활성 계약과 저장된 rendering quality는 변경하지 않았다.

사용자 확인 순서는 다음과 같다.

1. 새 Debug Client에서 F7을 열고 `수집(Capture)`을 켠다.
2. 같은 장면·카메라에서 수집한다. 계측 오버헤드를 비교할 때는 상세 수집과 창 표시 상태도 고정한다.
3. `한 프레임 해석`, CPU 병목, GPU 병목 및 패스 작업량에서 시간과 제출량을 확인한다.
4. 이름 있는 JSON을 저장하고 저장 범위·잘린 이력·GPU pending 상태를 함께 확인한다.

Client 실행·UI 조작·화면 판정은 수행하지 않았다. 위 입력 순서와 한글 font 표시·창 배치의
최종 확인은 사용자가 직접 한다. 자동 WARP 검증은 실제 게임 FPS나 조명 품질 검증이 아니다.

## G03. 최초 Profiler 구현 checkpoint 검증

| 검증 | 실제 결과 | 근거 |
|---|---|---|
| Debug Product 정상 증분 빌드 | Engine → Shared → Server → Client 모두 PASS, exit 0 | `out/BuildPipeline/runs/20261002T192648304Z-debug-product.json`, `out/Profiler20261003/product-debug.log` |
| 기존 Profiler 분석기 | Python unittest 6개 PASS | `python -m unittest discover -s Tools/Profiler -p 'test_*.py'` |
| Bern Navigation 공식 Validate | root, Bern, Bern2, Bern3, BernSea 모두 PASS | `out/Profiler20261003/nav-validate.log` |
| 수신한 nav 파일 보존 | 변경된 source/runtime 8개 파일이 동기화 HEAD와 동일 | `out/Profiler20261003/final-structure.json` |
| 프로젝트 구조 | Client vcxproj·filters XML parse, 기존 항목의 UTF-8/PCH metadata 확인 | 같은 구조 receipt |

Product 빌드는 약 185초 걸렸고, 실제 산출물은 Engine OBJ 38개·binary 2개,
Client OBJ 228개·binary 2개가 갱신됐다. 강제 Clean/Rebuild나 전체 데이터 publish는 하지 않았다.
기존 헤더의 코드 페이지 경고와 DirectXTK debug PDB 부재 경고는 남았고 컴파일·링크 오류는 없다.
Product의 파일·Navigation 참조 및 Item/Valtan catalog 검사는 전체 runtime·실제 플레이 검증을 뜻하지 않는다.

동기화 전 실제 Engine Profiler/CaptureIO를 연결한 headless WARP probe의 30개 검사가 통과했다.
중첩 Self, 동일 이름 합산과 부재 프레임, 지연 GPU의 원래 frame 귀속, 실제 IA 입력 9개·primitive 3개,
mesh 제출/instance/index, 고유 수 상한, worker no-op, capture OFF drain, reset/deferred reset,
이전 CPU+gap 관계와 JSON roundtrip을 확인했다. NaN 저장 실패 시 이전 파일 보존·임시파일 0개도 확인했다.
동기화 후 동일 제품 소스로 재컴파일한 추가 회귀 결과는 아래 G03-1에 기록한다.

## G03-1. 동기화 후 계측·저장 회귀

동기화된 실제 Engine/CaptureIO를 재컴파일하고 **41개 검사, failures 0**을 확인했다.
CPU scope overflow가 있는 선택 범위는 모든 이름의 SelfComplete가 false이며, 누락 프레임을
제외한 깨끗한 마지막1프레임에서는 true로 복구된다. CaptureWindow의0/1개 선택·이력 제외·최대
간격·경계·초기화와 JSON의 captureWindow+measurementSemantics 동시 보존도 통과했다.

device 없는 CPU 저장 검증에서1,202프레임을 생성하여 retained1,200/evicted2를 확인했다.
초기 GPU를 붙인 eviction stress는 지연되어 소유한 out probe만 종료한 뒤 CPU 전용으로 교체했다.
이 검사를 GPU eviction 성공이라고 기록하지 않는다.
근거는 `out/Profiler20261003/post-sync/post_sync_receipt.json`, `run_probe.log`,
`capture_json_receipt.json`이다. 이것은 추가 비교 UI/reader와 Workbench 변경 전의 계측 checkpoint다.

## G04. 현대 기법의 개념·확장 설계

계획서에는 현재 D3D11 renderer와 저장 profile의 실제 소비 경로, 본질 기준의 임시 세션 적용,
owner/generation에 따른 복원, A/B 변수 whitelist·조건 fingerprint, 반복·warmup·GPU pending,
수치 sweep와 결과 비교 계약을 정리했다. 현대 기법은 개선하는 현상·연산 비용·조절 변수·필요
입력·현재 지원 여부와 공식 자료를 연결했다.

GI의 baked/probe/screen-space/ray 방식, Lumen, SSR·planar·ray reflection, CSM·VSM,
GTAO, clustered/Forward+·MegaLights, Nanite, TAA·TSR·업스케일링, volumetric,
재질·투명·후처리와 path tracing을 서로 구분한다. GI는 문제 영역이고 Lumen은 여러 입력·추적·
cache·시간 누적을 묶은 구현이라는 점을 명시했다. 현재 엔진에서 명칭만 같은 toggle로 대체하지 않는다.

현재 rendering 설정·profile·게시 데이터는 그대로 유지했다. 본질 기준과 일반 변수 실험 도구의
추가 구현은 후속 절로 기록한다. 새 현대 기법의 GPU 구현과 실제 게임 성능 비교는 별도 범위다.
팀 LAN 계약은 수신한 main에서도 2026-10-02에 만료됐으므로 만료 우회나 endpoint 임의 변경을 하지 않았다.

## G05. 실제 draw 목록과 개념 사전

사용자의 “어떤 draw call이 불리는가” 요청에 맞춰 `CMesh::Render/Render_Instanced` 실제 제출
지점에서 상세 표본을 추가했다. `FProfilerMeshDrawSample`은 가장 안쪽의 계측된 GPU 패스,
메시 표시 이름, 재질 슬롯, geometry 정점, 인스턴스당 index와 instance를 보관한다.
GPU 패스가 없거나 패스 누락이 있으면 미관측으로 표시한다. 이름을 global asset/placement
identity로 사용하지 않고, raw pointer는 내보내지 않는다.

상세 수집 OFF에서는 이 목록을 만들지 않는다. ON에서는 프레임당512개까지 순서대로 기록하고
`DroppedMeshDraws`에 초과 수를 기록한다. 총 draw/mesh/index counter는 상한 뒤에도 계속
계수한다. 개별 draw timestamp는 추가하지 않았고 개별 draw GPU ms를 추정해 표시하지 않는다.

실제 Profiler를 연결한 별도 WARP probe **11개 검사, failures0**:
OFF 총량 보존·trace 없음, ON 중첩 패스/이름,512상한+초과2, 인스턴스당 index·material slot,
패스 없음, worker/null/zero-instance 무시, 다음 프레임 초기화와 reset을 확인했다.
근거는 `out/Profiler20261003/compile_mesh_trace.log`, `run_mesh_trace.log`다.

`RenderingTechniqueGuide.h`는 Workbench에서 검색·선택하는 읽기 전용 기법 사전이다.
GPU Gems, PBR·RNM·IBL, SSAO/GTAO·PCF, SSGI·probe GI·SSR·Lumen, RTX/DXR·path tracing,
TAA/TSR·DLSS/FSR/XeSS·ray reconstruction·frame generation, 다광원·Nanite·VSM,
volume·투명/OIT·재질/SSS·DOF·VRS·파티클·굴절·texture streaming·지형 기법을 설명한다.
각 항목은 현재 적용 범위, 조절 변수, 연산 부담, A/B 관찰법과 공식 URL을 함께 제공한다.
미구현 기술은 추가 입력/패스 또는 backend/SDK가 필요하다고 표시하며 실행 토글을 만들지 않았다.

기존 파일들의 UTF-8/원래 BOM 상태와 CRLF를 보존했고 새 header는 UTF-8 BOM 없이 작성했다.
Client 프로젝트와 기존 Rendering filter에 header를 등록하고 호출 CPP의 UTF-8 실행 인코딩을 명시했다.

## G06. 실제 SSAO·PCF 품질 변수

`Engine_RenderTypes.h`, `Renderer.cpp`, `Shadow.cpp`, `Shader_Deferred.hlsl`의 기존 경로를 확장했다.
SSAO sample 수4/8/12는 고정 수로 specialize한 kernel을 uniform branch로 고른다.
PCF radius0/1/2는 directional shadow와 dynamic baked shadow 비교에1/9/25개 위치를 사용한다.
후자는 위치당 complete/static depth를 각각 읽어2/18/50 fetch다. 기본12/radius1, source hair의
별도 filter 및 baked asset은 유지한다. 범위 밖 값은 renderer/shadow commit 전에 거부한다.

실제 HLSL을 headless D3D11 WARP의32×32 RGBA32Float target에서 비교했다.
기존 SSAO12·PCF radius1·dynamic baked radius1은 모두 **bitwise 동일, 최대 차이0**이었다.
SSAO4/8/12는 구별되는 finite 출력을 만들고 PCF0/1/2는 CPU tap oracle과 일치했다.
uniform SSAO 선택과 bounded PCF loop가 실제 bytecode에 존재함도 확인했다.
실제 전체 `fx_5_0`과 SSAO `ps_5_0` 컴파일도 통과했다.

shader/WARP20개와 validation20개, 합계 **40개 검사, failures0**이다. validation은 제품
Renderer 검증 함수를 그대로 추출해 실행하고 실제 Shadow Apply의 invalid/disabled/기존상태
보존을 확인했다. 이 국소 검사는 전체 Renderer 실행이나 실제 GPU 성능 향상 판정이 아니다.
근거는 `out/Profiler20261003/shader-quality/shader_warp_receipt.json`,
`validation_receipt.json`, `shader_probe.log`, `validation_probe.log`, `compile_shader.log`다.

Product의 기존 `Engine/Bin/ShaderFiles → EngineSDK/hlsl → Client/Bin/ShaderFiles` 배포가
추적 중인 Client deferred HLSL 사본도 갱신했다. Engine 정본과 바이트 hash가 같은 소스 사본을
함께 전달하며 컴파일된 CSO·EngineSDK·EXE/DLL은 커밋에서 제외한다.

## G07. 이전 프레임과 쿠크 2관문·빙고 기준 비교

Debug F7의 `프레임 변화` 탭에서 `최신 완료 프레임 따라가기` 또는 `현재 창 다시 가져오기`로
완료 프레임을 가져오고 `보관 프레임 위치`를 선택한다. 선택 B와 바로 이전 보관 A의 실제 frame
number·GPU 상태를 비교한다. 번호가 연속이 아니면 그 사실을 표시한다. `CPU·GPU 비용`,
`draw·메시·광원·컬링 작업량`, `선택 프레임 메시 draw`를 분리해 시간과 작업량을 함께 본다.
고정 CpuWork 이름·호출·시간 및 Animation 갱신/미제출 모델·시간·누락도 상세 OFF에서 비교한다.

`기준 A/B`에서 `A 이름`/`B 이름`을 쿠크 2관문·빙고 등으로 정하고 `현재 수집창 → A/B`로
각 장면을 보관한다. 기준은 새 수집·Reset 후에도 실행 중 유지된다. `저장 JSON`에서 파일을
고르고 `선택 JSON → A/B`로 과거 캡처를 읽는다. 상단 저장은 버튼을 누른 시점의 수집창이며
이미 보관한 A/B의 별도 export는 아니다. 시간·작업량 표는 B−A 증가순으로 정렬한다.

`비교 조건·수집 범위`는 장면·카메라·viewport·FPS cap·실효 품질·환경/LUT 식별자·상세 계측과
GPU 유효 분모를 보여준다. Profiler 기준의 조건은 보관/저장 시점 표본이므로 과거 모든 프레임의
동일 조건을 보장하지 않는다. 이 비교는 장면의 구조 차이를 찾는 용도이며 자동 인과 판정이 아니다.
쿠크 2관문과 빙고의 실제 새 캡처·게임 FPS 비교는 사용자가 수행할 항목으로 남아 있다.

Reader는 즉시 자식 regular JSON만 읽고 최대32MiB·1,200프레임·150,000 scope·깊이24·파싱값
150만 개로 제한한다. 파일 identity 재검증, 비정상 수치·형식 거부와 임시 결과의 완성 후 교체로
실패 시 기존 기준을 보존한다. 이전 형식의 없는 필드는 미계측이며 0으로 대체하지 않는다.
동기식 파일 읽기·분석 프레임은 성능 실험에서 제외하도록 안내한다.

최종 native 비교 회귀 **66개, failures0**은 NameId/main-thread 번호 변경, CPU self/상세OFF,
고정 작업·Animation, 인접 frame과 창 평균 분리, GPU 원래 frame/pending/partial 분모,
메시·설정 roundtrip, 구형 필드 부재, 비정상 입력·제한·파일 identity 실패 시 기준 보존을 확인했다.
형제 overlap·depth 건너뜀·부모 없는 depth는 수정 전3개 실패를 재현한 뒤 self 미계측으로 보완했다.
부모 범위 이탈도 미계측이며 inclusive 관측은 유지한다. 전체 파일을 임의 거절하지 않는다.
기존 native41개와 실제 Save_Json roundtrip도 최신 writer로 통과했다. 최종 hierarchy 보완은 reader
내부 변경이며 그 뒤66개를 다시 통과했다. 근거는 `out/Profiler20261003/comparison/`의
`comparison_receipt.json`, `comparison_hierarchy_before.log`, `comparison_run.log`,
`regression_run.log`, `verify_capture_json.py`, `compile_ui.log`다.

## G08. 실제 세션 A/B·반복·수치 sweep

Workbench의 `현재 품질을 A로 보관하고 실험 시작`은 현재 실효값을 A/B에 보관한다.
32개 typed 변수를 기존 renderer 경로에 임시 적용하며, 본질 기준 B는 SSAO·Bloom·FXAA·LUT·fog를
끄고 재질·직접광·baked/IBL·그림자·tone·접근성 설정을 보존한다. 본질 기준이 간접광 전체 OFF나
모든 장면의 정답이라는 뜻은 아니다. SSAO4/8/12·PCF0/1/2는 실제 shader 품질 실험이다.

A/B1~8회 반복은 AB/BA 순서를 교대한다. 준비0~120프레임 후10~900프레임을 측정하고 GPU 결과는
최대64프레임 추가 poll 후 pending으로 남긴다. 단일 변수 sweep은 A 기준과 최대9개 값을 같은
순서로 측정한다. 연속값2~9점, bool0/1, SSAO4/8/12, PCF0/1/2의 유효값만 허용한다.
CPU/GPU와 interval의 평균·중앙·P95·P99·최대, draw·mesh·index·패스 차이를 기록한다.

선택 변수만 제외한 공통 조건과 실제 조건을 별도로 보관한다. 카메라·해상도·profile/region·
light·Video·debugger·Profiler 상세 상태가 바뀌면 비교를 제외하며 GPU pending을0ms로 비교하지
않는다. animation·Server gameplay를 결정적으로 재생하는 기능은 없으므로 안정된 입력의 run도
탐색 측정이다. 결과는 `LostArkRenderingBenchmark.v2` JSON으로 비동기 저장한다.

ProfileService가 매 프레임 기존 overlay를 복원한 뒤 원래 환경/Video/연출 경로를 적용하고
새 overlay를 마지막에 적용한다. 종료·창 닫기·Level/region/profile/Video 변경 시 실험을 해제한다.
복원은 소유 필드와 실험 때문에 정규화된 보조값만 처리하고 다른 편집·새 owner를 보존한다.
실험 중 기존 저작/저장/publish 컨트롤은 비활성화한다. catalog와 저작·게시 JSON은 바꾸지 않았다.

Workbench native contract **39개**, 조건 fingerprint **11개**, 합계 **50개, failures0**을 확인했다.
32필드 검증·단일 mask·다른 품질/접근성/fog/그림자 pose/PBR 편집 보존·새 PBR owner·sample
경계/P99·GPU pending·partial scope·JSON 새 파일 저장을 검사했다. Shadow OFF의 descriptor
정규화는 비기본width80·눈/목표·bias를 이용해 OFF→ON→OFF와 최신 보조값 편집 보존을 확인했다.
sweep는 A와 같은 값도 동일 mask를 유지하고, 측정 종료 후 사용자가 새로 시작한 F7 수집은 나중의
실험 종료가 끄지 않는다. 조건 검사에는 debugger·상세 수집·viewport·카메라와 shadow 정규화
기준이 포함된다. 소유 CPP 두 개의 Debug/Release 집중 컴파일도 각각 통과했다.

이 검증은 제품 함수를 사용하는 out 전용 probe와 stub 상태로 경계를 확인한 국소 검증이다.
실제 게임 장면의 프레임 성능·수동 슬라이더 조작·화면 품질 PASS로 확대하지 않는다.
JSON9개 검사도 schema·한국어/control escape·32필드·pending·delta/null·무효 run 제외·scope를
확인했다. 근거는 `out/Profiler20261003/workbench/`의 `validation_receipt.json`, `probe.log`,
`fingerprint_probe.log`, `compile.log`, `compile_release.log`, `probe_final.json`이다.

## G09. 확장 전체의 최종 검증과 남은 확인

최종 소스의 `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`는
**PASS, exit0, 353,025ms**다. Engine·Shared·Server·Client 모두 통과했고 변경한 deferred shader와
공유 파생 shader를 정상 최적화 옵션으로 컴파일·배포했다. Client 단계는 OBJ159·CSO29·binary2
갱신을 보고했다. 근거는 `out/BuildPipeline/runs/20261002T202358617Z-debug-product.json`과
`out/Profiler20261003/product-final-debug.log`다. 앞의 G03 최초 checkpoint를 대체하는 최종
통합 컴파일 증거이며, 실제 게임 플레이 검증은 아니다.

기존 Effects deprecated·일부 shader 잠재 미초기화/pow 경고, 기존 헤더의 코드 페이지 경고와
DirectXTK debug PDB 부재 경고가 남았다. 컴파일·링크 오류는 없고 경고를 숨기기 위해 제품
옵션을 바꾸지 않았다. Build runner는 파일·Navigation 참조·Item/Valtan catalog를 확인했으며
전체 데이터 publish나 Client/Server 실행을 하지 않았다.

최종 Python Profiler 분석기6개, Client project/filters XML parse와 guide 단일 등록,
한국어 CPP의 UTF-8/PCH metadata, 기존 C++/HLSL 인코딩·BOM·줄바꿈 보존, 양쪽 deferred source
hash 동일 및 `git diff --check`를 확인했다. 구조 근거는
`out/Profiler20261003/final-extension-structure.json`이다. Data와 Client/Server DataFiles는
변경하지 않았고 사용자 `.gitignore`·개인 문서·backup/retired 파일을 보존했다.

사용자 화면 확인은 Debug F7의 한국어 글꼴·표·이전 프레임, 쿠크2관문/빙고의 명명 A/B 저장·
불러오기와 Workbench의 A/B·sweep·닫기 복원 순서다. 현재 같은 조건에서 새로 측정한 두 장면의
성능 차이, GPU 장치별 품질/속도 개선, Lumen/DXR/Nanite 같은 새 renderer 구현은 완료로
기록하지 않는다. 이번 변경은 상세 계측·구조 비교·현재 renderer의 실제 수치 실험과 현대 기법
설명/도입 설계를 제공한다.

## G10. 첫 병합 확인과 상용 도구 비교 확장

첫 구현 commit `398c2cb23b8358a3a84a824dc7c77cc0c053eba8`은
[PR505](https://github.com/tnestyle70/LostArk/pull/505)로 main에 병합됐다.
merge commit은 `8fdb3d4e6aa8c763f94aa28c683a80d7a8c84397`, 시각은2026-10-03 05:49:28 KST다.
검증한 feature tree와 merge tree가 같음을 확인했다. 이후 같은 요청의 상용 그래픽스·Profiler 비교
추가 구현을 위해 기능 브랜치를 이 main으로 fast-forward했다. 첫 병합이 아래 추가 구현까지
포함한 것처럼 설명하지 않는다. 후속 병합 근거는 최종 절에서 따로 기록한다.

`RenderingReferenceGuide.h`는 Profiler와 Workbench가 공유하는 읽기 전용 비교표다. 각 항목에
상용 개념, 현재 구현, 부족한 구조, 필요한 입력, 수치 의미, 현재 결론낼 수 없는 사항, 검증 기준,
실험 경로와 공식 문서 주소를 둔다. UE의 모든 cvar나 동일 성능을 인증하는 표가 아니다.
실제 렌더러/계측이 없는 기능에는 조작 가능한 가짜 활성 스위치를 만들지 않았다.

## G11. 시간축·병목 후보·실제 메모리

Debug F7의 `타임라인`은 최근 최대120개 완료 프레임에서 CPU thread/depth·GPU pass를 선택하고
확대·이동한다. 이벤트의 전체/self 시간·cross-frame·draw·IA/VS/PS 및 누락 상태를 표시한다.
CPU QPC와 GPU timestamp는 독립 원점이며 두 lane의 좌표를 CPU/GPU 인과로 설명하지 않는다.
`병목 후보·다음 실험`은 프레임 예산 초과, 프레임 틈, 많은 제출, 갱신 후 미제출 animation과
관측 부족을 근거와 함께 안내한다. 추정은 후보이며 driver·queue·GPU 점유율의 확정 원인이 아니다.

`RAM·GPU 메모리`는 Capture 중 최대1Hz로 실제 process private commit/working set/lifetime peak,
system commit/limit/available, 현재 device adapter node0의 DXGI local/nonlocal usage/budget을 읽는다.
API 실패는 각 valid flag의 N/A이고 정상0은0으로 유지한다. 표본 frame·QPC·PID·adapter LUID·age를
남기고 Reset 뒤 과거 표본을 새 관측으로 돌리지 않는다. 샘플 비용은 `Profiler.Memory.Sample`이다.
중복 표본을 chart/캡처 평균에서 제거하며 main-thread stall이나1초 사이의 순간 peak는 놓칠 수 있다.
DXGI budget은 고정 VRAM 용량이 아니며 local과nonlocal, process와system을 더하지 않는다.

`CMaterial.LoadSharedTexture`의 실제 경로에 request 시도·기존 weak-cache SRV 재사용·새 SRV 생성
3개 producer를 연결했다. 요청과 worker 완료가 다른 프레임에 잡힐 수 있다. 새 생성 누계는 현재
상주 texture 수가 아니고 다른 texture loader 전체를 포괄하지 않는다. v3 capability metadata가 없는
과거 캡처는 이 세 항목도 N/A로 읽는다. ContentHits/EstimatedGpuBytes는 계속 미계측이다.

시간축 순수 helper46개와 최신 ProfilerTool 집중 컴파일, 메모리 실제 native/WARP49개,
out-only OS/DXGI 실패 주입7개, memory JSON9개, 기존 캡처 비교66개와 cross-frame29개 회귀가 통과했다.
기본 선택은 최신 GPU 완료의 원래 프레임이며 최신 CPU와의 지연을 표시한다. GPU가 늦게 도착해도
계속 pending만 보지 않는다. 전환·수동 선택은 각각 최신 CPU·고정 snapshot 의미를 유지한다.
worker의 자식 종료와 부모 종료가 다른 프레임에 걸치거나 경계/이벤트가 누락되면 Self를 N/A로
표시한다. timeline은 영향을 받은 thread에 한정하고 집계/캡처 비교는 불완전 구간을 보수적으로 처리한다.
실제 shared-cache helper를 사용하는 WARP probe16개는 빈 경로/누락/decoder 실패·srgb/linear key·
weak owner 소멸 후 재생성·8개 동시 요청의 단일생성·Capture OFF를 확인했다. file decoder만 fixture로
대체했고 GPU SRV와 cache helper는 실제 실행했다. 새로운 scene 또는 UI를 실행한 결과가 아니다.
근거는 `out/Profiler20261003/timeline/`, `memory/memory_validation_receipt.json`,
`memory/regression/comparison_receipt.json`, `unreal/texture-cache-receipt.json`이다.

## G12. 설명과 실제 실험을 연결한 Workbench

세션 whitelist는41개 필드이고 원인별 recipe31개가 같은 `CRenderingProfileService` 경로를 사용한다.
recipe는 목표·볼 GPU pass/CPU·draw·비용/화질 한계와 전제조건을 설명한다. `기존 B를 이 단일 변수로
대체하고 적용`과 `이 변수의 sweep 범위만 준비`는 별도 명령이다. 다른 옵션을 자동으로 켜지 않는다.
공통 상용 비교표24항목과 GPU Gems/기법 사전은 실제 상태·누락한 기반을 함께 표시한다.

기본 OFF인 SSGI를 먼저 B에서 ON으로 적용한 뒤 `현재 B를 새 A 기준으로 채택`하면 그 상태에서
samples4/8/16만 바꾸는 실험을 준비할 수 있다. 채택한 공통 기준은 종료 복원의 소유 mask에는
포함하고, A/B에서 제외할 실험 변수 mask에는 넣지 않는다. 새 experiment ID로 이전 기준의 결과와
자동 비교하지 않는다. 종료·profile/level 변경은 원래 OFF로 복구하고 무관한 gamma 등의 편집을 유지한다.
source material OFF에서도 manual ON은 수신면 없는 패스 고정비 실험으로 허용한다. recipe는
가시 MapPBR 수신면이 없거나 FINAL view가 아니면 영상 기여를 확인할 수 없음을 안내한다.

조건이 달라져 비교에서 제외된 run은 opaque fingerprint와 함께 이름·이전/이후 값을 보인다.
JSON에 common/actual/changed condition fields와 recipe/goal/metric/confidence 설명을 남긴다.
광원 이름 상세는8개로 제한하고 전체 raw fingerprint/hash는 모든 광원을 계속 포함한다.
GPU pending과 partial scope를0ms로 비교하지 않는 기존 분모 계약은 유지한다.

실제 Benchmark+Service의 Debug/Release 집중 컴파일, native 계약114개, fingerprint22개,
recipe/기준 채택/거부·종료·profile/level 복원24개, JSON12개가 모두 통과했다(합계172).
새9필드의 복원, PBR mask 범위 분리, SSGI/SSR discrete 허용값, 공통 GI ON 조건을 fingerprint에서
숨기지 않는 것, source/debug view 변화, 명명 조건 차이와 한글 저장을 확인했다.
근거는 `out/Profiler20261003/commercial/validation_receipt.json`과 같은 폴더의 probe 로그다.

## G13. 실제 SSGI·SSR 렌더링

`Shader_ScreenSpaceLighting.hlsl`은 기존 Deferred pass index를 변경하지 않는 독립 FX5 프로그램이다.
Renderer는 불투명 `Render_Combined` 뒤, NonLight·scene replacement·투명 기여 전에 실행한다.
SSGI는 MapPBR 수신점의 cosine-weighted 반구4/8/16 rays에서 각8단계 depth 교차를 찾고,
원본 opaque HDR radiance를 albedo·비금속 diffuse 비율·거리 가중으로 더한다. SSR은 반사 방향을
16/32/64 단계로 찾고 현재 F0·roughness·화면 경계 가중을 적용해 radiance를 더한다.

두 패스는 같은 원본 HDR radiance를 사용한다. GI 결과를 SSR의 새 radiance로 다시 넣지 않는다.
기존 scene-post ping-pong 자원을 재사용하며 두 패스 성공 후에만 원본 HDR·Bloom에 복사한다.
Bloom은 추가된 radiance의 bright-pass 증가분만 더하고 기존 alpha·Bloom과 RT1 distortion을 보존한다.
모든 경로에서 원래 MRT·DSV·viewport를 복구하고 PS/effect SRV를 해제한다. shader는 최초 명시적
ON 적용 전에 준비하며 준비 실패는 이전 설정을 유지한다. 기본 OFF에는 새 패스·copy·shader load가 없다.

Profiler의 `Render.SSGI`, `Render.SSR`은 실제 CPU/GPU 범위와 draw/pipeline query를 사용한다.
`Render.ScreenSpaceLighting.Copy`는 결과 복사를 분리한다. PSInvocations는 처리 픽셀 호출이며
실제 ray hit/교차 수가 아니다. 강도0은 trace를 조기 종료하지만 패스와 복사는 남는다.
엔진 수치 검증은 강도0..2, GI 반경0.1..20, SSR 거리0.1..100·두께0.01..2와 discrete 값만 허용한다.
허용 경계/범위 밖/NaN/Inf를 포함한 실제 validator67개가 통과했다.

실제 FX5/O1 및 두 PS5/O1 컴파일, WARP 픽셀46개가 통과했다. visible wall에서 양의 GI bounce와
SSR hit, 강도 비례·sample/step 변화, 배경 clear(1,1,1,0) false hit 방지, self/offscreen/miss,
family0/1/2/4/5/6 보존, 비정상 입력·finite·alpha/Bloom 보존을 확인했다. 실제 Renderer 함수와
FX11을 묶은 WARP 통합49개도 통과했다. OFF/GI/SSR/둘다의 draw/copy, 두 번째 pass 실패 시 원본
HDR/Bloom 무변경, uniform 실패, 크기 불일치, resize, MRT4개·DSV·viewport2개 복원,
PS128slot·effect7개 SRV 해제와 debug layer warning/error0을 확인했다.
근거는 `out/Profiler20261003/screen-space/screen_space_validation_receipt.json`,
`integration/integration_receipt.json`, `unreal/quality/validation_receipt.json`이다.

이 구현은 기존 baked GI·RNM·IBL을 유지하는 현재 화면의 가산 실험이다. 물리적으로 에너지를
보존하는 GI/IBL 교체가 아니고, 화면 밖·가려진 면·다중 bounce·motion vector/history·시간 누적
노이즈 제거·Lumen Surface Cache·DXR BLAS/TLAS를 구현하지 않았다. full-resolution/고정 간격 추적의
노이즈·얇은 면 누락·화면 경계·camera 의존성과 실제 GPU 비용은 사용자 화면 A/B로 판정해야 한다.


## G14. 후속 통합 빌드·배포 검증과 남은 확인

후속 전체 C++/새 shader의 Debug Product 빌드는 PASS(exit0,99,288ms)다.
근거는 `out/BuildPipeline/runs/20261002T214120646Z-debug-product.json`이다. 이후 실제 실행 경로
확인에서 새 CSO의 Client 명시 배포 목록 누락을 발견해 Client project와 BuildDomains의 필수 산출물·
배포 pair를 연결했다. 최종 증분 Product는 **PASS(exit0,3,437ms)**이고 Client에 CSO1개를 배포했다.
최종 근거는 `out/BuildPipeline/runs/20261002T214721506Z-debug-product.json`과
`out/Profiler20261003/product-commercial-deploy-debug.log`다. Engine/Client CSO는 모두38,382bytes,
SHA256 `0fec9b186605dd8fde1c2b225da114bae97ef60434be22515db4f59eb3b3907d`로 일치한다.
양쪽 HLSL source도 동일하다. native binary/CSO/EngineSDK는 Git에 넣지 않는다.

기존 빌드 계약 Python39개는30 PASS·9 FAIL이다. 실패한9개를 이번 변경의 입력인 Client project와
BuildDomains의 HEAD 원문을 메모리에서 대입해 재검사했으며 동일하게 실패했다. 기존 harness 옵션,
Effect MSBuild item·runner 길이, output-guard fixture의 누락 module, publisher/world/Navigation
기대값 불일치다. 이 결과를 전체 PASS로 기록하거나 본 기능에서 무관한 빌드 체계를 수정하지 않는다.
근거는 `out/Profiler20261003/unreal/build-contract-results.json`이다.
새 셰이더의 Debug compile/deploy, 새 JSON/XML parse와 Engine/Client 해시 검증은 통과했다.

최종 구조/인코딩·등록·저장 Data 무변경·diff 검증은
`out/Profiler20261003/unreal/final-structure.json`에 기록한다. 기존 코드 페이지·외부 라이브러리
경고는 남아 있다. 실행하지 않은 Release 전체 Product, 실제 GPU 장치의 화질/속도, 쿠크2관문·빙고의
새 캡처, 한국어 UI 화면은 검증 완료로 기록하지 않는다. Client/Server를 자율 실행하지 않았다.

병합 전에는 push한 정확한 commit의 Engine/Profiler/배포, Workbench/Service/guide와 Profiler UI를
독립 검토하고 PR을 통해 main에 병합한다. commit 이후 생기는 review/merge SHA와 tree 대조 근거는
`out/Profiler20261003/commercial/`의 최종 review·merge 기록 및 연결 PR에서 확인한다.
사용자는 Debug F1 Workbench에서 SSGI 또는 SSR을 B로 ON → 새 A 채택 → sample/step sweep을
사용하고, F7에서 실제 pass·timeline·메모리와 명명 baseline 차이를 함께 확인한다.


## G15. 10-04 이름 있는 캡처의 기본 저장 범위 교정

`m_bSaveWindowOnly=true`와 분석창120개가 `Request_Save`의 Snapshot에 함께 쓰여,170개를 보관해도 기본 저장은 최근120개에 제한됐다. 기존 `captureWindow.excludedRetainedFrames`와 퇴출 경고는 정확했으나 기본 선택이 전체 수집 구간을 저장한다는 기대와 달랐다. 저장 범위 제한의 초기값을OFF로 바꾸고, 명시적으로 제한했을 때의 프레임 수를 분석·표시 범위와 별도로 분리했다. 저장 예정 표시와 실제 Snapshot은 `Save_FrameWindow`를 함께 사용한다.

저장 시작·완료 상태에는 실제 그 snapshot의 frame ID 범위·보관 중 제외·초기화 이후 퇴출과 저장 GPU pending/drop, CPU/GPU scope drop을 남긴다. 저장 도중 live history를 초기화해도 완료 메시지의 분모는 바뀌지 않는다. 최대1,200개 이력·기존JSON schema·단일 Snapshot→move exporter·비동기 원자 저장은 유지했다. 새 파일·project/filter·Engine 변경은 없다.

실제 `ProfilerTool.cpp` translation unit 집중 컴파일이 통과했다. 저장 관련 함수 원문을 발췌한 격리unit과 실제 Engine Profiler·CaptureIO를 연결한17개 검사가 통과했다. 기본170개 전부 저장, 명시120개/50개 제외, 분석범위1→900 변경의 독립성, 명시 범위 clamp,1,202개 중1,200개 보관/2개 퇴출, 저장 중 live 초기화 후170개 요약 보존을 확인했다. 생성JSON13개 assertion으로 frame 수·제외/퇴출·oldest1000ms fixture 보존·GPU pending/drop과 GPU 평균null을 검증했다. CPU전용probe의 clock과 GPU상태는 명시fixture이며 `Sample_Context`만 빈 context seam으로 대체했다. 실제GPU 측정·Client/UI 실행은 아니다.

근거는 `out/BernCliffFrameInvestigation20261004/profiler-save/verification-receipt.json`, `run-probe.log`, `all-170.json`, `explicit-120.json`, `retained-1200.json`과 같은 조사폴더의 `ProfilerCaptures`에 있다. H/CPP의UTF-8 BOM없음·CRLF를 유지했고 scoped `git diff --check`를 통과했다. 기존SDK codepage 경고가 있으며 제품 통합빌드·배포와 사용자F7 화면은 이 절의 완료 범위가 아니다.
