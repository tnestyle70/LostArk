# Profiler 상세 계측과 Rendering Workbench 확장 구현 계획

## G00. 목표와 현재 기준

2026-10-03, 최초 조사 HEAD `a0ff0185cc7fea171835cde15777c0f520d9d0ae`.
사용자의 Git 동기화 요청 후 `origin/main`의 `dc3a482d4`를 기준으로
`GB/Advanced-Rendering-Profiler-WorkBench` 브랜치를 만들었다. `abeac743a`의 차원술사
가이드 이동·착지, 베른 nav 복구, Movie 개선 및 Profiler 저장 범위·SelfComplete 계약을 보존한다.
사용자는 확장 설계와 Profiler 구현을 함께 요청했고, 이후 이전 프레임·쿠크 2관문·빙고의 구조
비교와 Rendering Workbench의 실제 A/B·변수 튜닝·개념 설명까지 구현 범위를 넓혔다.
`GPU GEN`은 사용자가 `GPU Gems` 자료 모음으로 명확히 했다. 이번 구현은 기존 Engine Profiler와
Debug F7 창, 현재 실행 가능한 렌더 경로의 세션 실험을 확장한다. 새로운 ray tracing backend와
UE 전용 renderer의 도입은 필요한 기반과 실제 구현 상태를 구분하는 단계 설계다.
Client/UI 실행과 최종 화면 판정은 사용자가 수행한다.

현재 `Engine/Public/Profiler.h`, `Engine/Private/Profiler.cpp`는 CPU inclusive/self,
GPU timestamp와 pipeline statistics, draw/index/instance 누계, 8-slot 비동기 query ring을
소유한다. `Client/Private/ProfilerTool.cpp`는 영어 표와 inclusive 정렬을 주로 사용한다.
현재 draw 누계는 Engine의 계측된 제출 경로이며 DirectXTK 내부 호출까지 모두 포괄하지 않는다.
mesh 제출 횟수, 고유 mesh, GPU 패스별 작업량과 순수 하위 구간 제외 비용을 구분해야 한다.
CPU 상세 표본 상한은 현재 8,192개이며 과거 문서의 4,096개와 다르다.

`RenderingBenchmark.cpp`는 이미 프레임 CPU/GPU 분포, 카메라·해상도·조명·설정 fingerprint와
source-material A/B를 제공한다. 현재 비교 허용 계약은 source-material 선택만 달라도 되는
형태이므로 SSAO, GI 등 임의 변수 비교는 실험변수 계약을 먼저 확장해야 한다.
저장된 `RenderingProfiles.json`과 게시 데이터, 사용자 video 설정은 이번 작업에서 변경하지 않는다.

## G01. Engine Profiler의 프레임·패스·메시 계약

수정 파일은 `Engine/Public/Profiler.h`, `Engine/Private/Profiler.cpp`와 실제 mesh 제출을
소유한 `Engine/Public/Mesh.h`, `Engine/Private/Mesh.cpp`다.
`Engine/Private/Light_Manager.cpp`는 일반·캐릭터 수광체의 CPU/GPU 구간을 추가하여
월드와 초상에서 재사용하는 실제 직접광 비용을 노출한다. 기존 VIBuffer draw 계수를 재사용하고 중복 계수하지 않는다.
기존 파일만 확장하므로 새 project/filter 등록은 필요하지 않다.
`Client/Default/Client.vcxproj`의 기존 ProfilerTool 항목에 `/utf-8`과 해당 파일만의
PCH 해제를 적용하여 기존 UTF-8 BOM 없는 한글 문자열의 실행 인코딩을 보장한다.

- CPU 프레임 경계와 이전 프레임 처리에 대응하는 프레임 밖 간격을 기록한다. 현재 CPU 처리와
  이전 프레임 간격을 잘못 빼서 잔여 시간을 만들지 않는다.
- GPU scope begin/end에서 main-thread 제출 누계 차이를 보존하고 GPU 결과의 원래 frame ID에
  연결한다. 부모 패스의 draw/index는 자식을 포함한다. CPU 제출 수는 GPU 결과 대기와 구분한다.
- GPU 하위 구간을 제외한 Self 값을 계산해 병목 정렬에 사용한다. 불완전 query 프레임은
  완전한 순위·분포에서 제외하고 상태와 누락 수를 유지한다.
- 제출된 mesh 호출 수와 고유 mesh geometry 수, 인스턴스 적용 인덱스 수를 구분한다.
  내부 주소는 프레임 중복 제거에만 사용하고 저장 ID로 내보내지 않는다. 상한 초과도 표시한다.
- 상세 모드에서 actual CMesh draw의 순서·표시 이름·GPU 패스·material slot·정점·
  draw당 index·instance를 최대512개 기록한다. 초과 수와 전체 제출 counter는 별도 보존한다.
  mesh 이름은 stable asset/placement ID가 아니며 개별 draw GPU 시간을 측정한 것처럼 표시하지 않는다.
- pipeline IA 입력·primitive·VS·PS 수를 정확한 명칭으로 노출한다. PS 호출 수를 광원
  수학 연산 횟수, GPU 점유율 또는 화면에 최종 보인 픽셀 수라고 부르지 않는다.
- Capture off no-op, query 미지원/지연/실패, 기존 CPU 수집 유지, DONOTFLUSH와 bounded
  메모리를 보존한다. GPU 결과를 기다리는 동기 readback이나 draw별 timestamp는 추가하지 않는다.

## G02. 한국어 Profiler와 병목 순위

`Client/Public/ProfilerTool.h`, `Client/Private/ProfilerTool.cpp`를 확장한다.
영문 scope ID는 로그·코드 검색을 위해 유지하고 한국어 역할 설명과 단위를 함께 표시한다.

첫 화면은 프레임 해석과 CPU/GPU 병목이다. CPU는 Self 내림차순을 기본으로 하고 inclusive
정렬도 선택할 수 있게 한다. 선택 범위의 CPU 표본이 누락되면 upstream의
SelfComplete 판정을 유지해 자체 시간은 `--`로 표시하고 전체 비용 순으로 전환한다. Main thread와 worker는 분리해서 읽으며 중첩 부모·자식과
CPU/GPU를 합산하지 않는다. World.Render/Render.World 아래 기존 패스 및 상세 계측을
드러내고 상세 모드의 오버헤드와 미관측·누락 상태를 명시한다.

GPU 표는 전체/자체 ms, P95, draw, instance, index, mesh 제출과 shader invocation을 같은
유효 프레임 분모로 보여준다. 조명은 direct lighting pass, 그림자 생성, SSAO, 간접 합성,
후처리 비용을 구분하고 각 광원 내부 연산으로 분해할 수 없는 범위를 설명한다.

작업량 표는 한국어 명칭으로 전 항목을 제공한다. 알려진 draw와 미계측 외부 라이브러리,
indirect upper bound, unique geometry와 scene object, 제출과 최종 가시성을 명확히 구분한다.
기존 한글 ImGui font atlas를 재사용한다. 새 렌더 옵션을 자동 활성화하지 않는다.

## G03. JSON과 검증

`Client/Private/ProfilerCaptureIO.cpp`의 기존 v3 저장에 additive 필드로 같은 단위와 상태를
기록한다. immutable snapshot, 비동기 저장, 임시파일 원자 교체와 실패 보존을 유지한다.
기존 필드를 삭제하거나 의미를 변경하지 않는다.

실제 Profiler와 CaptureIO를 사용하는 작은 비UI 검증으로 nested/self 합계, frame 귀속,
draw/index/instance 차이, 고유 mesh 중복 제거·상한, disabled/reset, query pending 및
JSON 유한값/직렬화를 확인한다. 임의의 게임 FPS를 측정값처럼 기록하지 않는다.
변경한 공개 Engine 헤더는 아래 Debug Product 빌드로 Engine 다음 Client까지 검증한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
git diff --check
```

사용자는 Debug Client에서 F7 → 수집(Capture) → 같은 장면과 카메라 → F7로 창 숨김 →
다시 열어 병목/작업량 확인 → JSON 저장 순서로 화면과 오버헤드를 판정한다.
동기화된 팀 LAN 기한도 2026-10-02에 만료됐으므로 이번 로컬 코드 작업에서 만료 우회 sync나 endpoint
변경을 하지 않는다. 제품 실행 준비 시 현재 endpoint 계약을 별도로 갱신해야 한다.

## G04. Rendering Workbench 후속 설계

G04~G08은 추가 요청으로 **실험 도구 구현 범위**에 포함됐다. 최종 반영·검증 범위는 RESULT를
따른다. G09~G11의 새로운 renderer 기법은 별도 기반이 필요한 단계 설계다. 현대 기법을 설명하는
항목이 있다는 사실만으로 해당 shader, GPU resource, asset pipeline이 구현됐다고 표시하지 않는다.
이 문서는 구현 범위와 호출 계약을 정하는 구현 계획서이며 변경 후 H/CPP 전문을 싣는 디테일
계획서는 아니다. 아래의 제안 타입·함수 이름은 새 계약이며 현재 존재하는 심볼과 구분한다.

### G04-1. 현재 경로를 기준으로 확장할 위치

| 현재 파일·기준 함수 | 현재 책임과 후속 확장 위치 |
|---|---|
| `Engine/Private/Graphic_Device.cpp`, `CGraphic_Device::Initialize` | `D3D11CreateDevice`로 현재 device를 생성한다. 현재 제품은 자체 D3D11 renderer이며 UE renderer를 포함하지 않는다. |
| `Engine/Public/Engine_RenderTypes.h` | `RENDER_QUALITY_SETTINGS`, `SHADOW_SETTINGS`, `HEIGHT_FOG_SETTINGS`, `MATERIAL_RENDER_SETTINGS`와 source tone/LUT, environment 입력 계약을 소유한다. 검증된 런타임 수치만 이 경계에 추가한다. |
| `Engine/Private/Renderer.cpp`, `CRenderer::Draw` | 기존 Shadow → Portraits → G-buffer → SSAO → Lights → HDR 합성/투명 → ScreenPosts → Bloom → Final → UI 경로에 패스와 계측을 연결한다. 별도 제품 renderer를 만들지 않는다. |
| `Client/Public/RenderingProfileService.h`, `Client/Private/RenderingProfileService.cpp` | 저장 profile, Level 품질 owner, camera region, 사용자 Video, presentation override를 해석한다. 실험 품질은 이 해석을 마친 값 위의 명시적 session 계층으로 적용한다. |
| `Client/Public/RenderingBenchmark.h`, `Client/Private/RenderingBenchmark.cpp` | 현재 source-material A/B, 조건 fingerprint, 프레임 수집과 JSON 저장을 소유한다. 일반 실험 명세·반복·변수 sweep를 여기에 확장한다. |
| `Client/Private/MainApp.cpp`, `CMainApp::RenderRenderingWorkbench` | 한국어 개념 설명·세션 후보 편집·실험 시작/취소 명령을 제출한다. UI가 renderer 내부 상태와 저작 JSON을 직접 바꾸지 않는다. |
| `Client/Private/UserSettingsDocument.cpp`, `CUserSettings::Apply_Video` | 밝기에 따른 gamma, Bloom/AA/SSAO OFF와 색각 보정을 실효 품질에 반영한다. 실험에서도 이 사용자 입력의 출처와 적용 결과를 보존한다. |

현재 `RenderingBenchmark::Render_PixelInputs`에는 direct diffuse/specular, RNM baked diffuse,
environment specular, cube diffuse 기여량과 normal 배율·roughness offset 비교가 있다.
`Shader_Deferred.hlsl`의 `Evaluate_MapSourcePBRDirect`/`Resolve_MapPBRLight`와
`Renderer::Render_Combined`에는 source PBR 및 환경 입력 소비가 연결돼 있다. 따라서 08-08
최초 문서의 “PBR/IBL 미구현”을 현재 전체 엔진의 상태로 복사하지 않는다. 기존 legacy 재질,
source PBR, source character와 forward family가 공존하며 새 기법의 적용 범위를 family별로 적는다.

### G04-2. 보존해야 하는 현재 저장 상태

조사 시점의 `Data/Rendering/Authored/RenderingProfiles.json`과 게시본은 revision **90**, profile
**29개**다. 아래는 파일의 저장 입력이며 실행 중 Video·region·presentation을 반영한 측정값이 아니다.

| 저장 owner | SSAO / Bloom / FXAA | base exposure / scene exposure 배율 | 비고 |
|---|---|---|---|
| globalQuality | ON / ON / ON | 약 0.73 / 해당 없음 | white point 1, gamma 약 1.905 |
| `scene.bern.neutral-day.v1` | ON / OFF / ON | 1 / 1 | source tone과 지역 입력을 별도 해석 |
| `scene.character-select.warm-high-key.v1` | ON / OFF / ON | 1 / 1 | 실제 선택 장면 품질 owner |
| `scene.kakulsaydon.g1.base.v1` | OFF / OFF / OFF | 2 / 0.5 | Bloom scene 배율 0. 패턴 scene 변경에도 Level 품질을 유지 |
| `scene.valtan.cool-low-key.v1` | OFF / OFF / ON | 약 0.73 / 1 | scene shadow와 fog는 별도 입력 |
| `scene.maharaka.source-day.v1` | ON / OFF / ON | 약 0.73 / 1 | RNM에 구운 광원과 UNBAKED receiver 분리 유지 |

쿠크의 region `kouku.ps.environment.55`에는 별도 SSAO OFF / FXAA OFF / Bloom ON / exposure 2가
있다. Level base만 비교해 이 값을 일괄 정리하지 않는다. Mario1~4의 현재 AA OFF, source·before
비교 profile, 사용자의 Video 선택, 안개 master와 region 값은 모두 보존 대상이다.
정본 위치와 publisher 경계는 `CLAUDE.md`의 “렌더링 옵션 정본”을 그대로 따른다.

### G04-3. “본질만 남긴 기준”을 구체적인 세션 값으로 정의

본질 기준은 기존 옵션이나 shader를 삭제하는 작업이 아니다. 사용자가 같은 장면에서 선택할 수
있는 **임시 진단 preset**이며 바뀌는 필드를 모두 표시한다. A는 진입 시점의 실효 입력을 보존한
`현재 품질`, B는 그 복사본에서 아래 항목만 바꾼 `표면·조명 기준`이다.

| 분류 | B의 기본 제안 | 이유·비교 해석 |
|---|---|---|
| 모델·재질·normal·roughness·metallic·alpha·emissive | A와 동일 | 재질을 다른 shading model로 바꾸면 후처리의 차이를 분리할 수 없다. |
| 직접광·receiver routing·RNM baked 조명·환경 반사 | A와 동일 | 현재 재질을 성립시키는 실제 조명 입력이다. RNM 제거를 원본 복원이나 무료 최적화로 부르지 않는다. |
| 방향광 shadow·static cache | A와 동일 | 형상 판단 기준을 유지한다. shadow 자체의 비용은 별도 실험으로 OFF/ON 비교한다. |
| tone mapping 방식·노출·white point·gamma | A의 실효값으로 고정 | HDR을 표시로 변환하는 단계는 남긴다. 노출을 올려 간접광의 유무를 숨기지 않는다. |
| SSAO·Bloom·FXAA | 임시 OFF | 접촉 차폐·영상 번짐·경계 보정의 영향을 하나씩 다시 추가해 확인한다. OFF만으로 저장된 quality가 바뀌지는 않는다. |
| 색보정 LUT·장면 desaturation·height fog | 임시 neutral/OFF | source tone curve와 LUT는 별개로 다룬다. fog OFF는 밀도 0 저장이 아니라 세션 gate다. |
| 사용자 색각 보정 | A와 동일 | 접근성 선택은 진단 preset이 자동 제거하지 않는다. 색각 보정 자체를 선택한 실험에서만 임시 비교한다. |
| Effect 형상·distortion·typed light/post·UI | A와 동일 | gameplay/presentation 내용을 preset이 무단 삭제하지 않는다. 정적 장면 실험에서 사용자가 재생을 멈추거나 개별 Effect pass를 독립 변수로 고른다. |

이 preset은 **GI가 전혀 없는 direct-only 결과가 아니다**. RNM과 환경광을 유지한다는 점을 이름
옆에 명시한다. 사용자가 `직접광만` 진단을 고르면 PBR에 한해 RNM/cube diffuse/environment
specular 기여를 각각 0으로 비교할 수 있지만 다른 material family의 내부 간접항까지 제거됐다고
표시하지 않는다. 미연결 family는 적용 대상 수와 `지원 안 됨`으로 드러낸다.

또한 contribution 0은 shader가 해당 계산을 생략한다는 보장이 없다. 화면 기여 비교와 성능용 pass
bypass를 별도로 표시하고, compiler·실제 GPU 비용으로 확인한 경우만 “계산 생략”이라고 적는다.
baseline 전체와 A의 차이는 여러 기능의 합성 차이다. 특정 기능 비용을 얻으려면 B-core에서 한
기능만 추가한 run끼리 비교해야 한다. preset의 권장 수치는 제품의 새 기본값이나 측정 결과가 아니다.

### G04-4. 화면 구성

Rendering Workbench의 첫 영역을 `현재 장면 / 실험 기준 / 비교 결과 / 기법 설명`으로 정리한다.
기존 Light Resources·Light Sequencer·Light Detail의 저작 책임은 유지한다.

- `현재 장면`: Level, active scene, Level quality owner, region, source revision, Video 적용 여부,
  실제 viewport와 내부 render 크기, 현재 실효 노출·AA·AO·Bloom·shadow·fog를 표시한다.
- `실험 기준`: A 보관, B를 A에서 복사, 본질 preset, 바뀐 필드 목록, A/B 전환, 복원 버튼을 둔다.
  저장할 저작 draft와 세션 B는 다른 배경·제목으로 구분한다.
- `비교 결과`: 실험 이름, 기준 조건 일치 상태, CPU/GPU Δms, p95/p99, draw/index/instance,
  shader 호출 및 병목 순위 변화를 표시한다. 측정 중 변경된 조건은 필드 이름으로 보여준다.
- `기법 설명`: 개념, 개선되는 현상, 비용이 생기는 곳, 조절 변수, 현재 지원 범위, 필요한 선행
  입력을 같은 행에서 읽게 한다. 이름만 있는 Lumen·Nanite 토글을 활성화하지 않는다.

현재 `MainApp.cpp`의 `FXAA (saved)`, `Reset Selected Quality Defaults`,
`Selected Reference A/B Start`는 `Update_Profile`을 통해 catalog draft를 바꾼다.
새 본질 기준 버튼은 이 호출을 재사용하지 않고 G05의 실험 session 명령을 호출해야 한다.

## G05. 실험 세션의 타입·적용 순서·복원 계약

### G05-1. 상태 owner와 새 계약

기존 `RenderingBenchmark.h`의 `RENDERING_BENCHMARK_RUN` 앞에 아래 실험 값 타입을 추가한다.
Engine은 실제 shader/resource 설정을, Client Benchmark는 실험 ID·A/B·진행 상태를 소유한다.
UI label, filesystem 경로, capture 목록을 `Engine_RenderTypes.h`에 넣지 않는다.

| 제안 계약 | 필드의 책임·단위·수명 |
|---|---|
| `RENDERING_EXPERIMENT_FIELD` | `quality.ssao.enabled`, `quality.bloom.intensity`, `shadow.enabled` 같은 enum과 고정 문자열 대응. 허용된 실제 필드만 선택하며 임의 JSON path를 setter로 사용하지 않는다. |
| `RENDERING_EXPERIMENT_SPEC` | Benchmark가 발급한 `experimentId`, 표시 이름, A/B variant ID, 독립 변수 목록, 반복 수, warm-up/측정 정책, 목표 FPS를 보관한다. 한 process session의 실험 정의다. |
| `RENDERING_EXPERIMENT_SNAPSHOT` | 실제 적용된 quality·shadow·fog·material contribution과 source/region/Video identity, 카메라·해상도·조명 상태를 값으로 보관한다. 포인터·구조체 padding을 저장하지 않는다. |
| `RENDERING_EXPERIMENT_OWNER` | Level, Level quality profile, active scene, region 및 generation을 기록한다. 같은 문자열 profile을 새로 Reload한 경우도 generation 변경으로 식별한다. |
| `RENDERING_EXPERIMENT_STATE` | `IDLE`, `STAGING`, `WARMUP`, `SAMPLING`, `RESOLVING_GPU`, `COMPLETED`, `CANCELLED`, `INVALID` 전이를 Benchmark가 소유한다. |
| `RENDERING_EXPERIMENT_RESULT` | variant·반복·sweep 값, 시작/끝 frame ID, 유효 표본·누락 수, 조건 snapshot, 시간 분포와 패스 작업량, 종료 이유를 보관한다. 완료한 값만 immutable export snapshot으로 만든다. |

현 `RENDERING_COMPARISON_OPTIONS`에는 Directional/LUT/Bloom/exposure만 있으므로 full quality를
저장할 문서처럼 사용하지 않는다. 기존 field override와 새 실험 override는 하나의 해석 함수에서
충돌을 검사한다. 동일 필드를 두 비교자가 소유하려 하면 시작을 거절하고 현재 소유자를 표시한다.
실험 시작이 기존 live 비교를 묵시적으로 지우지 않으며 “현재 실효 입력을 A로 보관”할 때 그
출처를 함께 기록한다.

### G05-2. 실제 적용 순서와 실패 rollback

`RenderingProfileService.cpp`의 `Apply_CameraEnvironment`를 확장할 때 유지·추가할 적용 순서는 다음과 같다. 새 실험 overlay와 아래 Try 함수는 후속 구현 대상이다.

```text
이전 프레임 presentation override 복원
→ 저장 Level/scene 품질 및 camera region 해석
→ scene multiplier와 사용자 Video 적용, region post-process 및 보간
→ 현재 cinematic/presentation 입력 적용
→ 기존 live comparison 및 새 실험의 충돌 없는 field overlay
→ CGameInstance의 typed Apply 경계
```

새 `Try_StageExperimentVariant`는 실효값의 복사본에 whitelist 필드만 변경한 뒤 기존 finite/range
검증과 기법 dependency/capability 검증을 수행한다. `Try_CommitExperimentVariant`는 필요한 GPU
resource까지 준비된 경우에만 프레임 경계에서 교체한다. SSAO bias가 radius 이상이거나 지원하지
않는 AA/GI 방식이면 commit 전 거절하고 어느 값이 잘못됐는지 표시한다. setter가 숫자를 조용히
clamp해서 요청값과 측정값을 다르게 만들지 않는다. UI가 제안하는 clamp도 변경 전후를 표시한다.

적용 중 quality→shadow→fog 중 하나가 실패하면 같은 프레임에서 이미 바꾼 **자신의 필드만**
직전 성공값으로 rollback한다. rollback 실패는 별도 실패 상태로 유지하고 측정을 중단하며,
기존 renderer의 frame 실패 처리로 넘긴다. 부분 적용된 장면을 성공한 B run으로 기록하지 않는다.
새 GPU resource의 staging 실패는 현재 resource를 폐기하지 않는다.

현재 `Restore_PresentationEnvironment`가 매 프레임 원래 quality를 복원한 뒤 exposure multiplier를
다시 적용하는 원리를 유지한다. B가 적용된 exposure를 다음 프레임 A로 재수집하거나 Video gamma를
두 번 곱하지 않는다. 동일 입력 1,000프레임에서 실효 수치가 누적되지 않는 focused 검증을 둔다.

사용자 Video OFF는 기본 실험의 상한으로 유지한다. 해당 기능을 반드시 ON으로 시험하려면 사용자가
실험 UI에서 그 Video gate까지 독립 변수로 명시해야 한다. 이때도 `CUserSettings::Commit`을 호출하지
않고 세션 preview임을 표시한다. user settings 파일은 수정하지 않는다.

### G05-3. 종료·외부 변경·저장 수명

`Update_RestorationPreview`의 기존 owner 확인을 확장한다. Workbench 닫기, Level 전환, scene
전환, Runtime Reload, Video 적용, region 변경은 실험을 종료하거나 무효화하는 원인이 된다.
직전 owner와 generation이 같은 경우만 자신이 적용한 필드를 복원한다. 다른 owner가 이미 값을
바꿨다면 이전 snapshot을 통째로 덮지 않고 override를 해제한 뒤 새 owner의 해석 결과를 유지한다.
동일 Level의 camera region 보간이 진행 중이면 기준선 준비를 완료하지 않는다.

capture 도중 창이 닫혀도 `CMainApp::Update`에서 취소·GPU pending 회수·저장 완료를 처리한다.
창 Render 함수만 lifecycle 소비자가 되어서는 안 된다. `Cancel`은 수집을 끝내지만 이미 완료한
다른 run이나 사용자의 기존 Profiler capture를 지우지 않는다. 현재 Benchmark의 `Reset_History`
사용은 일반 실험으로 확장할 때 run 시작 frame ID 방식으로 바꿔 공유 Profiler history 보존을 우선한다.
실험이 Profiler를 임시로 켰다면 그 활성화 generation도 보관한다. 사용자가 도중에 Capture/Reset을
바꾼 경우 run을 무효화하고 사용자의 새 선택을 유지하며, 시작 시점의 enabled 값으로 덮어쓰지 않는다.

`Save Authored`/`Publish Runtime`은 기존 profile catalog만 직렬화한다. B의 transient effective
값을 역수집하지 않는다. `실험 결과 저장`은 `Client/Bin/ProfilerCaptures`의 별도 이름 있는 JSON이며
저작 저장과 다르다. 후일 `B를 저작 후보로 복사`를 제공하더라도 필드 diff를 보여주는 명시적 사용자
명령으로만 stage하고, 기존 freshness·merge·atomic replace 계약을 그대로 거친다.

G05 종료 증거는 OFF/ON 반복·cancel·Level/region/Video/Reload owner 교체·apply 실패에서 원래
품질이 보존되는 비UI 검사다. Data 정본·게시본·user settings bytes 무변경도 확인한다. 실제 화면의
복원 여부는 사용자가 A/B/닫기를 조작해 판정한다.

## G06. 일반 A/B·프레임 예산·변수 sweep

### G06-1. “같은 조건”을 실제 변경 필드로 검증

현재 `RenderingBenchmark.cpp`의 `ComparisonConditions`는 source-material 선택만 제외하고
quality·카메라·광원 값 전체를 비교한다. 이를 그대로 두면 SSAO OFF/ON을 다른 조건으로 거절한다.
새 방식은 **전체 snapshot을 항상 저장**하고, experiment spec이 허용한 독립 변수만 제외한 공통
fingerprint를 추가로 만든다. 제외는 필드별 registry accessor에서 수행하며 문자열 prefix나
`quality 전체 무시` 같은 넓은 예외를 두지 않는다.

| 실험 | 허용하는 A/B 차이 | 계속 같아야 할 것 |
|---|---|---|
| SSAO 비용 | `quality.ssao.enabled` 하나 | radius/bias/intensity/power/fade와 모든 나머지 품질·장면 |
| Bloom 반경 sweep | `quality.bloom.scatter` 하나 | Bloom ON, threshold/intensity, tone/노출, 해상도 |
| material 복원 | `material.source.enabled` 하나 | 기존 A/B와 동일한 카메라·조명·품질·assets |
| GI 방식 비교 | `indirect.method`와 그 방식이 선언한 전용 parameters | 직접광, 재질, 노출, scenario, output 해상도. 방식별 내부 해상도 차이도 결과에 명시 |
| 본질 preset 비교 | 명시한 여러 field ID의 정확한 집합 | 전체 diff 일치 여부. 합성 결과이며 단일 기능 인과 비용으로 표시하지 않음 |

fingerprint에는 Level/scene/region/generation, authoring·runtime revision, material source mode,
shader build identity, camera view/projection, 실제 viewport, 내부 해상도, 조명 수와 모든 수치,
RNM/environment identity, typed Effect light/post 활성 상태, debug view, 실행 configuration,
adapter·driver 식별 가능 정보, Video frame cap·foreground 상태와 Present 정책을 기록한다.
현재 fingerprint에 없는 항목은 수집 reader를 연결한 뒤 비교 기준에 넣는다. 알 수 없는 값을 0이나
빈 문자열로 같은 조건 처리하지 않고 `미확인`으로 남긴다.

frame cap 또는 VSync로 결과가 제한되면 제한 상태를 유지한 채 표시한다. 이번 구현에서 저장 cap을
자동 해제하거나 swapchain을 변경하지 않는다. 기법 비용 실험에서 사용자가 제한을 조절했다면
A와 B 모두 같은 조건으로 새로 수집한다.

정적 장면은 카메라와 animation/action time을 고정한다. 실시간 gameplay는 Server simulation을
클라이언트 도구가 정지하거나 되돌리지 않는다. 동적 실험은 사용자가 재현 가능한 같은 scenario/
action time 경로를 선택했을 때 비교하고, 동일성 근거가 없으면 `탐색 측정`으로 표시한다.
카메라 경로 실험을 추가할 때는 현재 matrix의 단순 일치 대신 같은 path ID·revision·sample time을
비교한다. 현재 Benchmark에 deterministic camera replay가 있는 것처럼 표시하지 않는다.

### G06-2. 수집 상태와 temporal 기법

```text
IDLE
→ STAGING: 전체 후보·지원 여부·공통조건 검증, 자원 준비
→ WARMUP: 프레임 경계 적용, 필요한 history reset, 준비 신호와 정해진 프레임 수 충족
→ SAMPLING: 선택한 frame ID 구간을 기록, 매 프레임 조건 재확인
→ RESOLVING_GPU: CPU 표본 구간은 고정, 해당 GPU 결과를 비동기로 회수
→ COMPLETED: 유효 표본·누락·조건과 통계 commit
```

변수 변경, camera cut, resize, Level/scene 변경, motion-vector layout 변경은 temporal history의
invalidation 사유다. 새 기법의 history owner가 reset generation을 공개하고 Benchmark는 reset
완료를 확인한다. 기존 장면 resource를 모두 다시 로드하는 방식으로 warm-up하지 않는다.
SSGI/TAA/probe GI/volumetric의 cache 축적 시간이 다르므로 단일 “한 프레임 제외”로 안정화했다고
부르지 않는다. `고정 warm-up 프레임`, `history age`, `필수 resource ready` 조건을 함께 기록한다.

기본 제안은 warm-up 120프레임, 측정 600프레임, A/B 각각 3회다. 이는 재현을 위한 시작 정책이며
현재 장면의 안정화 측정값이나 통계적 유의성 보장이 아니다. 초기 cold resource cost는 별도 run으로
기록하고 warm steady-state에 섞지 않는다. sample count는 현재 최대 1,200 history와 GPU tail의
범위 안에서 검증하며, 여러 반복은 각 run 통계를 별도 보관해 history를 무한히 늘리지 않는다.

A/B 순서는 A→B, B→A를 교대해 일방적인 GPU 온도·cache 영향을 줄인다. 순서·warm/cold 분류·
각 반복의 분산을 남기고 가장 빠른 run만 대표값으로 고르지 않는다. temporal 안정화 중 외부 scene
입력이 바뀌면 다시 warm-up하거나 invalid로 종료한다. 무한 재시도하지 않고 명시적 사유를 표시한다.
GPU pending은 ring을 bounded 조회하며 `S_FALSE`를 0ms로 저장하지 않는다. 회수 정책의 제한을
넘긴 frame은 누락 상태로 남기고 CPU 결과와 유효 GPU 표본 수를 각각 보존한다.

### G06-3. 비용 표시와 비교식

| 지표 | 표시·계산 계약 |
|---|---|
| 실제 프레임 간격 | frame interval 평균·중앙값·p95·p99·최댓값. 실제 관측 FPS는 `1000 / 평균 interval ms`로 보조 표시 |
| CPU | main 처리·대기와 worker를 구분하고 CPU frame, self/inclusive pass 통계를 따로 표시 |
| GPU | 유효 query 표본의 pass별 inclusive/self ms와 pipeline 작업량. CPU 시간과 합산하지 않음 |
| 목표 예산 | 목표 30/60/120fps는 각각 약 33.33/16.67/8.33ms. 이는 수학적 예산이며 해당 GPU 성능 예측이 아님 |
| A/B 차이 | 동일 분모의 `B−A ms`, `B−A 작업량`, A가 0이 아닐 때만 상대 변화율. A=0의 백분율은 N/A |
| p99 | 완료한 원시 frame 표본에서 정의된 percentile 계산법으로 산출. 반복별 p99의 평균을 전체 표본 p99라고 부르지 않음 |
| 유효성 | CPU/GPU/각 pipeline counter의 유효 표본 수, dropped scope, pending/disjoint/미지원/미계측을 함께 표시 |
| 메모리 | 실제 소유한 RT/buffer/texture 할당의 logical bytes와 측정 가능한 residency/예산을 구분. trace buffer 예상량을 실측 VRAM으로 표시하지 않음 |

한 프레임 CPU와 GPU는 겹쳐 실행된다. `CPU 10ms + GPU 8ms = 18ms`로 FPS를 추정하지 않는다.
GPU timestamp elapsed에는 CPU 명령 공급 공백이 포함될 수 있어 GPU core 사용률과도 구분한다.
빛 비용은 광원 준비 CPU, shadow depth GPU, 직접 조명 GPU, RNM/환경 합성, 투명 조명, volume
조명을 따로 읽는다. `광원 수 × PSInvocations`는 정확한 light evaluation 횟수가 아니다.
기여가 같은 패스에 융합돼 있으면 A/B Δms로만 분리할 수 있음을 표시한다.

가장 비싼 pass는 GPU self 기준, 가장 비싼 CPU 작업은 thread별 self 기준으로 정렬한다.
계측한 자식 합과 부모 전체 사이의 잔여 시간은 `계측되지 않은 구간 / 패스 공통 처리`로 남기며
임의로 광원·draw에 균등 분배하지 않는다. 품질 평가는 사용자의 관찰 메모로 따로 기록한다.

### G06-4. 수치 sweep와 저장

첫 구현은 한 변수씩 선형 목록·명시 목록을 받는다. 예를 들어 SSAO radius는 사용자가 승인한
동일 단위 m의 목록을 넣고 다른 SSAO 값은 고정한다. 두 변수 grid는 이후 명시적인 조합 수·전체
예상 run 수를 보여줄 때만 제공한다. 입력값·실효값·단위·validation 결과를 모두 보관한다.

각 후보는 `stage → apply → temporal reset → warm-up → sample → pending resolve → run commit`
을 거친다. 중간 실패 시 해당 후보만 실패로 표시하고 성공했던 마지막 상태 또는 동일 owner의 A로
복원한다. 취소는 다음 프레임 경계에서 처리한다. 전체 화면 캡처·Client 자동 조작·Server gameplay
자동 반복을 sweep의 전제에 넣지 않는다. 사용자가 실행 중 도구에서 시작한 렌더 수치 실험만 자동
순환하며 결과 이미지의 최종 품질 판정은 사용자에게 남긴다.

`Save_Json`은 snapshot 생성과 파일 쓰기를 분리하고 기존 `ProfilerCaptureIO`의 비동기 저장·고유
파일 이름·오류 회수 방식을 재사용한다. benchmark JSON에는 schema/version, experiment/variant,
field diff, 전체 conditions, warm-up policy, frame 구간, raw sample 또는 capture 참조, 통계와
유효성을 저장한다. 기존 material A/B 결과를 읽을 때 없는 metadata는 추측해 채우지 않는다.

G06 종료는 실제 field-diff 검증, 허용 변수 외 변경 탐지, p99/동일 분모, temporal reset·pending
종료, cancel·파일 쓰기 실패에서 run 보존을 비UI 검사로 확인한다. 게임 FPS 개선은 사용자 실행의
동일 조건 캡처가 있어야 RESULT에 기록한다.

## G07. 현대 렌더링 기능군의 개념과 비교 변수

아래는 특정 UE 버전의 모든 CVar를 복제하는 목록이 아니라 화면을 만드는 기능군 전체를 살펴보는
개념 지도다. 수치 변수와 도입 순서는 이 프로젝트를 위한 설계 제안이며 고정 GPU 비용이나 최적값
주장이 아니다. 각 기능의 설명 버튼은 연결한 공식 문서와 자체 엔진의 지원 범위를 함께 표시한다.

### G07-1. 직접광·간접광·반사

**GI는 물체에 부딪힌 뒤 다른 표면을 밝히는 간접 조명을 다루는 범주이고, Lumen은 그 범주와
반사를 실시간으로 처리하는 UE 시스템이다.** Bloom·SSAO·환경 cubemap 중 하나를 켰다고 Lumen이
구현되지는 않는다. 현재 RNM은 이미 구워진 조명 입력이며 실시간 장면 변화의 bounce를 새로 풀지
않는다. [Epic GI 개요](https://dev.epicgames.com/documentation/en-us/unreal-engine/global-illumination-in-unreal-engine)

| 기능 | 무엇을 계산하는가 | 비교 변수와 비용 축 | 현재 지원·도입 전제 |
|---|---|---|---|
| 직접광 | 태양/Point/Spot에서 표면으로 바로 오는 diffuse·specular | 활성 광원 수, range/cone, 화면 점유, receiver, shadow 여부; light 제출 CPU·조명 GPU·PS 호출 | 현재 있음. source receiver 분리와 RNM 중복 가산 방지 유지 |
| PBR / GGX BRDF | 재질 roughness·metallic·normal에 따른 에너지 분배와 반사 | normal 강도·roughness·metallic, shader variant; texture bandwidth·BRDF 비용 | source PBR 있음. 모든 legacy·character를 동일 GGX로 바꾸는 전역 스위치가 아님 |
| Baked lightmap / RNM | 정적 geometry에 도달한 조명을 미리 계산한 texture | texel 밀도, bake sample·bounce, atlas/UV; bake 시간·용량·런타임 fetch | RNM 소비 있음. 신규 bake 시스템과 기존 추출 RNM은 별개 |
| IBL / reflection probe | 환경 영상을 diffuse·roughness별 specular로 근사 | cube 크기·mip·회전·강도·보정 범위·갱신 주기; 메모리/샘플 | source environment specular와 cube→Lambert SH 근사 있음. native SH 원본 packing 전체 동일성은 미완료 |
| Volumetric lightmap / irradiance probe | 공간의 조명 샘플을 보간해 움직이는 물체에 간접광 제공 | probe 간격·SH 차수·보간 범위; 메모리·빛샘 | 현재 일반 probe-volume 시스템 없음. 저장/streaming·보간·검증 필요 |
| SSGI | 화면 depth/normal/color에서 근거리 간접광 추정 | ray·step 수, 반경·해상도·history; tracing/denoise | G21에 현재 프레임 MapPBR 가산 실험 연결. 화면 밖 정보·history·denoising은 후속 기반 |
| 실시간 probe GI | probe 조명을 tracing으로 반복 갱신하고 공간 보간 | probe spacing, rays/probe, 업데이트 수/frame, hysteresis; trace·cache | 미구현. probe volume·scene tracing·relocation/leak 방지 설계 필요 |
| Lumen software tracing | screen trace, mesh/global distance field, surface cache를 결합한 GI·반사 | trace 거리·scene detail·cache 품질/갱신·GI quality | 자체 엔진에 없음. distance field asset·surface cache·history 전체 시스템 필요 |
| Lumen hardware tracing | ray tracing geometry와 surface cache 또는 hit lighting으로 GI·반사 | scene/BVH 갱신, ray budget, 반사 roughness·bounce·hit lighting | 자체 D3D11에서 DXR toggle 불가. RT 지원 backend와 scene 구조 선행 |
| SSR | 현재 화면의 depth·색으로 반사 ray를 추적 | max roughness·step·거리·해상도·edge fade; tracing/resolve | G21에 MapPBR 가산 실험 연결. 기존 IBL 보존, 화면 밖/가려진 표면·temporal 미지원 |
| Planar reflection | 평면에 대해 반사된 camera로 장면을 다시 그림 | 반사 해상도·clip plane·거리·대상; 추가 geometry/lighting pass | 미구현. 별도 카메라/target과 scope별 중복 비용 표기 필요 |
| Ray-traced reflection | scene geometry를 ray로 조회해 off-screen 반사 계산 | ray 수·bounce·roughness·denoising; BVH·trace·shade | 미구현. RT backend·material hit 평가·fallback 필요 |

probe·SSGI 방식의 원리와 한계는 [Volumetric Lightmaps](https://dev.epicgames.com/documentation/en-us/unreal-engine/volumetric-lightmaps-in-unreal-engine),
[Screen Space GI](https://dev.epicgames.com/documentation/en-us/unreal-engine/screen-space-global-illumination?application_version=4.27),
반사 방식은 [Reflection methods](https://dev.epicgames.com/documentation/unreal-engine/reflections-environment-in-unreal-engine?lang=en-US)를 따른다.
Lumen의 선행 구조·trace 선택은 [Lumen 기술 설명](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-technical-details-in-unreal-engine),
비용·확장성 해석은 [Lumen 성능 가이드](https://dev.epicgames.com/documentation/en-us/unreal-engine/lumen-performance-guide-for-unreal-engine)를 참고한다.

### G07-2. 그림자·차폐·다광원 렌더링

| 기능 | 무엇을 계산하는가 | 비교 변수와 비용 축 | 현재 지원·도입 전제 |
|---|---|---|---|
| Shadow map / PCF | 광원 depth와 receiver depth를 비교하고 주변 비교 샘플로 가장자리를 완화 | shadow 해상도·coverage·bias·filter 폭·caster 수; depth draw/메모리/샘플 | 2048² directional depth, PCF, static cache·dynamic caster 경로 있음 |
| CSM | camera 거리 구간별 방향광 shadow map을 나눠 해상도를 배분 | cascade 수·분할 거리·blend·각 해상도; 반복 caster draw | 미구현. camera frustum 분할·안정화·cascade별 culling 필요 |
| Contact shadow | 화면 depth로 작은 접촉 가림을 추적 | trace 거리·step·thickness; screen-space pass | 범용 기능 미구현. 장거리 shadow를 대체하지 않음 |
| Distance-field shadow / AO | geometry의 거리장을 따라 가림을 근사 | voxel/detail·trace step·거리; asset 용량·ray march | 범용 기능 미구현. 현재 source 정적 shadow texture와 다른 시스템 |
| VSM | 필요한 shadow page만 할당·생성·캐시하는 가상 shadow map | page budget·LOD bias·무효화·projection sample | 미구현. virtual address/page table·cache invalidation·GPU 관리 필요 |
| Ray-traced shadow | 표면에서 광원으로 ray를 보내 차폐와 penumbra 계산 | samples/pixel·광원 크기·거리·denoise | 미구현. RT backend와 동적 geometry 갱신 필요 |
| SSAO / GTAO | depth/normal 주변 차폐로 주로 간접광을 감쇠 | method·radius·sample·해상도·power·filter; pass GPU | 현재 half-resolution SSAO+resolve 있음. GTAO는 별도 식·검증 필요. GI의 bounce와 구분 |
| Deferred lighting | G-buffer를 기록한 뒤 조명을 적용 | MRT 형식/개수·해상도·light screen coverage; bandwidth·조명 pass | 현재 기반. 새 기능도 기존 G-buffer 계약 확장 우선 |
| Forward+ / clustered lighting | tile/3D cluster마다 영향 광원 목록을 만들어 필요한 광원만 평가 | tile 크기·Z slice·list cap; cull compute·list bandwidth·shading | 일반 cluster 시스템 미구현. opaque Deferred 유지 후 light culling만 개선하는 대안도 비교 |
| MegaLights / stochastic direct lighting | 제한된 ray/sample 예산으로 중요한 다수 직접광을 평가하고 복원 | samples·light importance·scene 복잡도·denoise; tracing·노이즈 | UE 구현 없음. GI와 다르며 무제한 무료 광원 기능이 아님 |

그림자 기능의 개념은 [Shadowing](https://dev.epicgames.com/documentation/en-us/unreal-engine/shadowing-in-unreal-engine),
[Virtual Shadow Maps](https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-shadow-maps-in-unreal-engine),
AO는 [Ambient Occlusion](https://dev.epicgames.com/documentation/en-us/unreal-engine/ambient-occlusion?application_version=4.27),
forward 구조는 [Forward Shading](https://dev.epicgames.com/documentation/unreal-engine/forward-shading-renderer-in-unreal-engine)을 참고한다.
MegaLights는 [공식 기능 설명](https://dev.epicgames.com/documentation/en-us/unreal-engine/megalights-in-unreal-engine)처럼 직접광과
노이즈 복원 비용을 따로 비교한다.

### G07-3. geometry·해상도·시간 재구성

| 기능 | 무엇을 개선하는가 | 비교 변수와 비용 축 | 현재 지원·도입 전제 |
|---|---|---|---|
| LOD / HLOD | 거리·화면 크기에 맞춰 geometry와 먼 배치를 단순화 | screen threshold·LOD·merge 범위; indices·draw·memory | 기존 CModel 경로를 확장할 후보. 일반 HLOD streaming은 새 asset 계약 필요 |
| Frustum/occlusion culling | 카메라 밖 또는 가려진 geometry 작업을 생략 | bounds·occlusion method·history; culling CPU/GPU 대 절감 draw | map visibility/shadow culling 있음. 일반 HZB/GPU culling을 구현 완료로 보지 않음 |
| Instancing / GPU-driven draw | 동일 geometry를 묶거나 GPU가 가시 목록·draw arguments 생성 | batch 크기·material split·instance 수; submit CPU·upload | map instancing 있음. GPU-driven 전체 scene과 구분 |
| Displacement / tessellation / WPO | WPO는 기존 정점을 이동하고 tessellation은 세분화한 geometry에 displacement 적용 | 분할 정도·변위 높이·거리 감쇠; primitive 증가·HS/DS/VS·bounds | source별 vertex 변형과 범용 tessellation을 구분. DX11 HS/DS·adjacency/edge 일치·culling/shadow bounds 검증 필요 |
| Nanite | 계층적 geometry cluster를 streaming/culling하고 화면에 맞게 raster | pixel error·streaming budget·material 다양성·overdraw | UE 구현 없음. 기존 LOD 옵션 추가 수준이 아닌 asset/geometry renderer 설계 |
| FXAA | 현재 영상의 경계를 필터링 | threshold·blend; final-pass samples·세부 blur | 현재 있음. Mario 포함 저장 OFF 보존 |
| MSAA | geometry coverage를 여러 위치에서 샘플링 | sample count·resolve; attachment memory/bandwidth | 현재 Deferred MRT 전체에서 지원하지 않음. G-buffer/투명/resolve 호환성 검토 선행 |
| TAA | jitter와 이전 프레임 정보를 모아 경계·반짝임 안정화 | jitter·history weight·clamp/rejection·sharpness | 미구현. velocity·previous matrix·reactive/disocclusion·reset 필요 |
| TAAU / TSR / temporal upscaler | 낮은 내부 해상도와 시간 정보를 출력 해상도로 재구성 | internal scale·history quality·sharpness; history·resolve | 자체 구현 및 UE TSR 없음. TSR 명칭을 단순 upscale shader에 사용하지 않음 |
| DLSS / FSR / XeSS 계열 upscaling | 외부 구현이 temporal 입력을 받아 해상도·안정성을 복원 | quality mode·입출력 크기·sharpness·SDK overhead | 현재 통합 없음. 기능별 SDK/API/hardware·velocity·depth·exposure 계약을 확인해 연결 |
| Frame generation | 렌더된 프레임 사이에 추가 표시 프레임을 생성 | 생성 비율·pacing·latency·UI 처리·GPU 추가비용 | 현재 없음. 생성된 표시 FPS와 game simulation/실제 렌더 FPS를 별도 기록 |
| 공간 upscaling | 한 프레임을 필터링해 해상도를 확대 | scale·filter·sharpness; resolve samples | 일반 render-scale 경로부터 필요. temporal history는 쓰지 않음 |
| Dynamic resolution | GPU 예산에 따라 내부 해상도를 변경 | 최소/최대 scale·target ms·적응 속도 | 미구현. A/B 기본은 고정 scale이며 동적 정책 자체를 시험할 때만 허용 |
| VRS | 영역별 shading rate를 줄여 픽셀 연산 절감 | rate·영역 기준·motion/edge 보호 | 현재 D3D11 backend에 없음. 지원 API와 hardware 확인 필요 |

geometry 기준은 [Instanced Static Mesh](https://dev.epicgames.com/documentation/en-us/unreal-engine/instanced-static-mesh-component-in-unreal-engine),
[Visibility and Occlusion](https://dev.epicgames.com/documentation/en-us/unreal-engine/visibility-and-occlusion-culling-in-unreal-engine),
[Nanite](https://dev.epicgames.com/documentation/en-us/unreal-engine/nanite-virtualized-geometry-in-unreal-engine)를 참고한다.
정점 이동과 bounds는 [Material Inputs](https://dev.epicgames.com/documentation/unreal-engine/material-inputs-in-unreal-engine?lang=en-US),
전통적인 tessellation 방식은 [UE4 World Displacement](https://dev.epicgames.com/documentation/en-us/unreal-engine/1.11---world-displacement?application_version=4.27)를
개념 근거로 삼는다. UE4의 이 방법을 최신 Nanite tessellation 구현과 동일하게 설명하지 않는다.
시간 재구성과 필요한 입력은 [Temporal upscalers](https://dev.epicgames.com/documentation/en-us/unreal-engine/temporal-upscalers-in-unreal-engine),
[TSR](https://dev.epicgames.com/documentation/unreal-engine/temporal-super-resolution-in-unreal-engine?lang=en-US)를 따른다.
frame generation은 [NVIDIA 통합 설명](https://developer.nvidia.com/blog/how-to-successfully-integrate-dlss-3/),
해상도 정책은 [Dynamic Resolution](https://dev.epicgames.com/documentation/unreal-engine/dynamic-resolution-in-unreal-engine?lang=en-US),
VRS의 API 경계는 [Microsoft VRS](https://learn.microsoft.com/en-us/windows/win32/direct3d12/vrs)를 참고한다.

### G07-4. 대기·투명·재질·후처리·참조 렌더링

| 기능 | 개념 | 비교 변수와 비용 축 | 현재 지원·도입 전제 |
|---|---|---|---|
| Height fog | 거리와 높이에 따라 안개색/투과를 합성 | density·height falloff·start distance·opacity | 현재 combine의 height/source exponential fog 있음. volumetric과 구분 |
| Volumetric fog | 시야 공간 volume의 산란·흡수·광원·그림자 적분 | XY grid·Z slice·distance·anisotropy·history·shadow | 미구현. froxel resource·volume light injection·temporal resolve 필요 |
| Sky atmosphere / aerial perspective | 대기의 파장별 산란과 시야 거리의 공기색 계산 | 태양 높이·산란/흡수 계수·LUT 해상도 | 원본 sky 재질과 다름. 새 atmosphere LUT/합성 경로 필요 |
| Volumetric cloud | 3D 밀도장을 ray march해 구름·자기 그림자 계산 | view/shadow step·trace distance·해상도·density | 미구현. volume asset·ray march·shadow·history 비용 별도 |
| Light shafts | 광원 주변의 빛줄기를 화면 또는 volume으로 근사 | sample·길이·occlusion·해상도 | 범용 기능 미구현. Effect mesh/sprite 연출과 물리 volume 구분 |
| 투명·refraction·OIT | alpha 합성·장면색 굴절·순서 의존 완화 | layer/overdraw·internal scale·OIT 방식·refraction copy | 현재 blend/distortion/source forward 있음. OIT는 알고리즘별 저장·정확도 계약 필요 |
| Water | 파형·normal·Fresnel·흡수·반사·굴절 합성 | wave/normal·reflection 방식·depth fade·step | source water family 있음. 현대 수면/SSR/volume을 자동 지원하지 않음 |
| SSS / hair / anisotropy / clear coat / layered material | 피부 내부 산란, 섬유 방향 반사, 표면 코팅층 등 재질별 반응 | 모델·lobe·sample·thickness·roughness; shader/permutation | 현재 source character family와 별개인 일반 shading model 확장. 일괄 material 치환 금지 |
| Substrate / BSDF layering | 여러 표면 반응을 조합하는 UE 재질 저작·표현 체계 | layer/lobe 복잡도·G-buffer 저장·단순화 예산 | UE Substrate 없음. 거칠기 slider 추가와 달리 재질 ABI·G-buffer·compiler 구조 변경 |
| Decal | 표면 위에 제한된 재질/색/normal 정보를 투영 | 대상 영역·채널·overdraw·draw | 기존 family별 decal 경로가 존재해도 전체 D-buffer/모든 채널 지원을 뜻하지 않음 |
| Texture streaming / virtual texturing | 필요한 mip·page를 선택해 texture 메모리와 대역폭 관리 | pool·mip bias·page size·upload budget | 기존 SRV/resource cache와 구분. 별도 residency·asset/feedback 구조 필요 |
| Denoising / ray reconstruction | 적은 ray 표본의 noisy 결과를 공간·시간 정보로 복원 | filter 폭·history weight·validation·sample count | 범용 RT denoiser 없음. 새 tracing 기법의 비용과 품질을 나눠 계측하며 history reset 공유 |
| Tone mapping / exposure / LUT | HDR→표시 변환, 밝기와 색 보정 | manual 노출·curve·white point·LUT·자동 적응 | Hable/source tone/LUT/manual exposure 있음. histogram 기반 auto exposure는 별도 기능 |
| Bloom | 밝은 영상의 확산 | threshold·knee·intensity·scatter·pyramid 해상도 | 현재 half-res extract/blur 있음. emissive가 주변 물체를 비추는 GI와 다름 |
| DoF / motion blur | 초점 밖 흐림·노출 시간 중 이동 흐림 | aperture/focus·CoC·shutter·velocity·sample | 범용 기능 미구현. velocity/depth와 history/foreground 경계 필요 |
| Lens effects / sharpening / vignette / grain | 표시 영상의 카메라·예술 보정 | 강도·threshold·sample·pass fusion | source typed post별 지원만 현재 사실. 새 효과는 독립 비용·OFF 기준 필요 |
| HDR display / output color | 표시장치 색역·전달함수·밝기에 맞춰 출력 | SDR/HDR·paper white·peak nits·gamut | SceneHDR FP16과 실제 HDR monitor 출력은 다른 기능. swapchain/output 계약 필요 |
| Path tracing | 많은 ray와 bounce를 누적한 품질 참조 | samples/pixel·bounce·denoise·scene | 자체 엔진에 없음. 실시간 프레임 목표와 별도인 offline/reference 경로로 설계 |

volume의 공식 근거는 [Volumetric Fog](https://dev.epicgames.com/documentation/en-us/unreal-engine/volumetric-fog-in-unreal-engine),
[Volumetric Cloud](https://dev.epicgames.com/documentation/unreal-engine/volumetric-cloud-component-in-unreal-engine?lang=en-US),
대기는 [Sky Atmosphere](https://dev.epicgames.com/documentation/en-us/unreal-engine/sky-atmosphere-component-in-unreal-engine),
투명 합성은 [Transparency](https://dev.epicgames.com/documentation/en-us/unreal-engine/using-transparency-in-unreal-engine-materials)를 참고한다.
재질 표현은 [Substrate](https://dev.epicgames.com/documentation/en-us/unreal-engine/substrate-materials-in-unreal-engine),
texture page 관리는 [Virtual Texturing](https://dev.epicgames.com/documentation/en-us/unreal-engine/virtual-texturing-in-unreal-engine),
ray reconstruction은 [NVIDIA DLSS 기술 구분](https://developer.nvidia.com/rtx/dlss)을 참고한다.
재질·후처리 기능은 [Post Process Effects](https://dev.epicgames.com/documentation/unreal-engine/post-process-effects-in-unreal-engine),
[Tone Mapping](https://dev.epicgames.com/documentation/unreal-engine/color-grading-and-the-filmic-tonemapper-in-unreal-engine?lang=en-US),
표시장치 출력은 [HDR Display Output](https://dev.epicgames.com/documentation/en-us/unreal-engine/high-dynamic-range-display-output-in-unreal-engine),
참조 렌더링은 [Path Tracer](https://dev.epicgames.com/documentation/en-us/unreal-engine/path-tracer-in-unreal-engine)를 참고한다.

### G07-5. 지원 상태를 사용자에게 정확하게 표시

기법 행은 `현재 사용 가능`, `입력은 있으나 부분 지원`, `설계만 완료`, `구현 필요`, `API/asset 전제
필요` 중 하나를 표시한다. 미지원 기능은 개념 설명과 필요한 선행조건을 열 수 있지만 Apply는
비활성화한다. 지원 안 됨을 OFF나 비용 0과 같은 값으로 기록하지 않는다.

2026-10-03 확인한 최신 Epic 문서는 Lumen software tracing에도 SM6를 명시하고 Nanite/VSM에는
DX12 SM6.6 atomic 또는 대응 Vulkan 기능을 요구한다. 과거 UE의 DX11 지원 설명을 자체 DX11
프로젝트에 그대로 적용하지 않는다. Lumen software tracing은 hardware RT 없이 동작할 수 있다는
뜻이며 “아무 D3D11 engine에서 checkbox 하나로 사용 가능”이라는 뜻이 아니다.
[플랫폼 요구사항](https://dev.epicgames.com/documentation/en-us/unreal-engine/hardware-and-software-specifications-for-unreal-engine)

## G08. 첫 확장 구현 — 기존 옵션의 비용을 닫는 Workbench

수정 대상은 `RenderingBenchmark.h/.cpp`, `RenderingProfileService.h/.cpp`,
`MainApp.cpp`, 필요 typed gate를 넣는 `Engine_RenderTypes.h`와 `Renderer.cpp`다.
G04~G06의 baseline/owner/experiment/state/JSON을 먼저 구현하고 현재 있는 SSAO/Bloom/FXAA/
shadow/fog/LUT만 capability registry에 활성 기능으로 등록한다. 기법 설명은 데이터 표와 현재
registry를 연결해 UI 설명과 실제 setter가 어긋나지 않게 한다.

첫 실험 순서는 현재 품질 A 확보 → B-core 적용 → B-core+SSAO → B-core+Bloom → shadow와
FXAA 단독 비교다. 현재 scene에 해당 기능이 없거나 Video gate가 막으면 unavailable 사유를
표시한다. 단일 변경 이외의 차이가 생긴 run은 자동 비교하지 않는다. 광원 수·RT 크기·CPU/GPU
패스 비용을 Profiler와 같은 이름·단위로 노출한다.

기존 source-material 비교는 별도 실험 preset으로 유지한다. source 복원과 현대화를 같은 A/B
라벨에 섞지 않는다. 새 C++ 파일 없이 시작할 수 있어 project/filter 변경은 필요하지 않다.
타입이 비대해져 분리할 경우 먼저 물리 파일을 정하고 Client project/filter에 필요한 항목만 등록한다.

종료 증거는 G05/G06 focused 검사, Engine→Client Debug Product compile, 변경 JSON/XML parse,
`git diff --check`다. 사용자는 F1 → Rendering Workbench → 현재 품질 보관 → 본질 기준 →
Capture A/B → 결과 저장 → 비교 종료 순서로 조작한다. 품질·복원·실제 FPS는 사용자 확인 전까지
미확인으로 RESULT에 남긴다.

## G09. DX11에서 가능한 화면 기법과 temporal 기반

계측 결과 AO 또는 반사 품질을 개선할 필요가 있으면 기존 SSAO 개선/GTAO와 SSR를 첫 후보로 둔다.
GTAO는 기존 SSAO input/resolve 경계를, SSR는 SceneHDR와 depth/normal/roughness를 재사용한다.
새 pass를 넣기 전에 현재 RT에 필요한 값과 정밀도가 있는지 확인하고 추가 MRT/format의 메모리·
대역폭 비용까지 A/B에 포함한다. 모든 재질에 없는 roughness를 임의 상수로 채워 지원 완료로 보지 않는다.

G21의 비시간누적 SSGI·SSR 실험 이후, TAA/TAAU와 간접광·반사의 시간 누적 버전은 공통 temporal 입력을 먼저 닫는다. Engine의 현재/이전 camera matrix와 object/
skinning transform으로 velocity를 만들고 camera cut·teleport·spawn·pose reset·resize 때 history를
무효화한다. 투명/particle의 velocity 또는 reactive mask 지원 범위를 표시한다. main scene과 portrait
처럼 서로 다른 view는 history를 공유하지 않는다. 생성·resize 실패는 기존 non-temporal 경로를 유지한다.

각 기법의 texture/history owner는 `CRenderer`다. Benchmark는 method와 quality를 typed 설정으로
제출하고 history generation·ready 상태만 읽는다. Engine GPU resource를 Client tool에서 직접
만들어 두 번째 렌더 경로를 만들지 않는다. 새로운 shader 파일을 추가하면 실제 Engine/Client shader
빌드·배포 입력과 프로젝트 항목을 함께 등록하고 셰이더 identity를 capture metadata에 기록한다.

종료는 OFF 회귀, resize/camera cut/spawn/가림 해제의 finite 결과와 bounded resource, pass별 GPU
표본을 확인하는 것이다. ghosting·노이즈·edge 누락은 사용자 시각 평가에 남긴다. 수치 오류가 없는
검사와 모든 장면의 화질 동등성을 동일하게 기록하지 않는다.

## G10. 다광원·그림자·volume의 선택적 확장

Profiler에서 light 준비/제출 CPU가 크면 현재 `Light_Manager`의 batch·mask·receiver를 먼저 살피고,
조명 GPU가 크면 screen coverage·overdraw·material family와 light culling을 비교한다. Forward+를
곧바로 전체 renderer 교체 목표로 삼지 않는다. 기존 Deferred 안에서 tiled/clustered list를 만드는
대안과 기존 light 경로를 동일 scene으로 비교한다.

cluster 구조는 bounds·tile/Z slice·maximum lights·overflow policy를 명시하고 overflow를 숨기며
광원을 버리지 않는다. 보수적 fallback을 사용하면 그 횟수·비용을 기록한다. compute build, list upload,
opaque lighting, source-character 전용 lighting, transparent lighting을 각각 계측한다.

shadow는 caster draw·cache invalidation·PCF 샘플 중 병목을 구분한 뒤 해상도/CSM/cache 개선을
선택한다. shadow 해상도를 낮춘 이득을 전체 scene 품질 동일로 단정하지 않는다. volumetric fog는
기존 height fog를 보존한 별도 method로 도입하고 froxel grid·거리·산란계수·light injection·history를
독립 scope로 측정한다. 모든 광원에 volumetric shadow를 자동 활성화하지 않는다.

이 G의 구현 순서는 측정 결과로 고르며 새 광원/scene 데이터를 임의로 만들지 않는다. 종료 조건은
동일 광원·caster 조건의 비용 차이, 실패 시 기존 방법 유지, source receiver·RNM 중복 방지 회귀,
사용자의 그림자/안개 품질 판정이다.

## G11. 동적 GI와 현대 UE 계열 구조의 도입 결정

현재 baked RNM과 환경 입력은 유지한 채 동적 GI의 목적을 먼저 고른다. 가까운 bounce 변화만
필요하면 SSGI의 한계를 공개하고, 화면 밖·실내 이동·동적 광원을 요구하면 probe 또는 scene tracing
구조를 검토한다. 새 간접광을 RNM 위에 중복 가산하지 않도록 receiver별 `baked 유지 / dynamic 대체 /
명시적 보충` 정책을 둔다. 정책 변경도 실험 변수로 기록한다.

자체 Lumen 유사 시스템은 다음 선행 항목이 모두 필요하다: geometry 표현과 streaming, distance
field 또는 acceleration structure, surface/material lighting cache, screen/scene trace 연결,
history·denoise·cache invalidation, diffuse/specular 합성, 디버그 view와 비용 계측. 이 가운데
일부만 구현했다면 그 기능 이름으로 보고하며 Lumen 구현 완료라고 부르지 않는다.

hardware ray tracing은 D3D12/Vulkan 등 지원 backend의 device/resource/synchronization,
acceleration structure·skin 갱신·material hit shader를 포함하는 별도 구조 변경이다. 이번 Profiler
작업에서 API 전환을 시작하지 않는다. Nanite는 geometry virtualization, VSM은 shadow page 관리,
MegaLights는 stochastic direct lighting이므로 GI와 각각 독립적인 선행조건·실험·완료 기준을 갖는다.
같은 이름의 UE 엔진 구현을 자체 엔진 기능으로 선언하지 않는다.

도입 결정서는 기대하는 장면 문제, 현재 GPU/CPU 병목, 지원 hardware/API, asset 변환량, 메모리
상한, fallback, 새 profiler scope, 사용자가 판정할 품질 조건을 적는다. 화면 개선 필요가 없거나
현재 frame budget에 맞지 않으면 설명용 항목으로 남겨도 된다. “더 현대적”이라는 이유만으로 모든
기법을 동시에 켜는 preset을 만들지 않는다.

## G12. 단계별 완료와 public 계약 갱신

| 단계 | 구현 완료라고 말할 수 있는 증거 | 별도로 남길 것 |
|---|---|---|
| G01~G03 Profiler | 실제 writer 연결, 통계/JSON 검증, Debug Product compile | 사용자 F7 화면·동일 조건 캡처 |
| G04~G07 설계 | 현재 소비자와 ownership에 맞는 이 문서, 공식 기술 근거 | 실제 도구 반영은 G08·RESULT로 확인, 신규 renderer와 구분 |
| G08 실험 UI/기존 옵션 | 세션 rollback, 조건 whitelist, warm-up/sample/pending, 결과 저장의 실제 연결 | 사용자의 A/B 품질 판단 |
| G09~G10 새 pass | shader/resource/소비자/계측·OFF 회귀·실패 보존 | 장면별 시각 품질과 실제 성능 분포 |
| G11 구조 도입 | 후속 요청에서 정한 구현 범위의 backend/asset/runtime/측정 연결 | Lumen/Nanite 등 명칭과 실제 달성 범위 차이 |

렌더 옵션을 저장할 public schema나 적용 우선순위가 바뀌는 시점에만 `CLAUDE.md`와 팀 계약을
갱신한다. 일시적인 benchmark 결과와 shader 수치는 대응 RESULT에 적고 AGENTS에 누적하지 않는다.
미구현 기술을 runtime JSON에 미리 넣거나 publisher가 소비하지 않는 값을 저장 성공으로 표시하지
않는다. G04 이후 실제 구현 시에는 변경 기능의 최소 컴파일·관련 실패 검증·JSON/XML parse·
`git diff --check`를 완료하고 실행한 항목만 RESULT에 남긴다.

## G13. 프레임·장면 간 구조 비교

사용자의 추가 요청은 “쿠크 2관문·빙고는 빠른데 다른 장면은 왜 느린가”를 비교할 수 있게
만드는 것이다. 프레임 시간을 하나의 평균으로만 읽지 않고 다음 관측 경계를 연결한다.

- 인접한 완료 프레임을 번호로 선택해 CPU 전체/간격·계산 구간·작업량의 차이를 읽는다.
  최신 CPU와 지연된 GPU를 서로 같은 프레임으로 간주하지 않는다.
- 사용자가 붙인 장면 이름으로 A/B를 보관하고 같은 실행에서 수집하거나 기존 JSON을 읽는다.
  `쿠크 2관문`, `빙고`는 비교 라벨이며 실제 측정이나 해당 장면 이동을 자동 수행하지 않는다.
- CPU는 캡처별 NameId를 직접 비교하지 않고 이름과 main/worker 역할을 키로 사용한다.
  전체·자체 시간과 호출 수가 함께 증가했는지 확인한다. Self 누락은 0으로 간주하지 않는다.
- GPU는 완료·유효·패스 누락이 없는 프레임 분모로 비교한다. 패스별 Δms와 draw·mesh·index·
  shader 호출 증감을 함께 보여주되 CPU/GPU 시간을 합치지 않는다.
- light records, culling candidate/reject, cache miss, instance upload와 scene copy처럼
  구조 차이를 드러내는 작업량을 비용 변화와 연결한다. 상관관계를 원인 확정으로 표현하지 않는다.

다른 scene 사이의 결과는 **구조 비교**다. 카메라·해상도·노출·quality·cap·상세계측·수집 구간이
같은 단일 변수 A/B와 구분한다. 조건이 다르면 그 차이를 보여주며, FPS 차이를 한 설정의 개선으로
단정하지 않는다. 비용 증가·감소 순위는 실제 표본에 기반하고 정해 둔 장면 우열을 하드코딩하지 않는다.

JSON reader는 파일 크기·프레임/행 상한과 finite 수치·schema·field 타입을 검증한 임시 snapshot을
만든 뒤 성공 시에만 A/B를 교체한다. 실패하면 기존 선택을 유지한다. 이전 v3에 새 mesh/draw/self
필드가 없으면 availability를 따로 보관해 미계측과 실제0을 구분한다. 기존 writer와 비동기 원자 저장은
유지한다. 임의 경로를 파일에 쓰거나 캡처 내용으로 명령을 실행하지 않는다.

수정 파일은 `ProfilerTool.h/.cpp`, `ProfilerCaptureIO.h/.cpp`의 기존 경로다. 종료 검증은
actual snapshot→JSON→bounded loader의 통계 일치, malformed/oversized/nonfinite 실패 보존,
서로 다른 NameId·GPU pending·SelfComplete 누락, 조건 차이와 프레임 분모를 다룬다.

## G14. 기법 사전과 현재 실행 가능 범위

`Client/Public/RenderingTechniqueGuide.h`는 Workbench 안에 읽기 전용 한국어 기법 사전을 제공한다.
새 header를 Client `.vcxproj`와 `.filters`의 기존 Rendering 분류에 등록한다. 호출자인
`RenderingBenchmark.cpp`는 `/utf-8`로 컴파일하고 기존 파일의 인코딩은 보존한다.
도구는 기법 이름·개념·현재 적용 범위·조절 변수·비용 발생 위치·A/B 관찰법·공식 자료 URL을 함께 표시한다.

GPU Gems는 알고리즘 자료 모음이다. 최초 조사 기준 구현에는 fixed12-sample SSAO, fixed3×3 PCF,
정적 shadow cache, 일부 source PBR·IBL·RNM, height fog·Bloom·FXAA·맵 instancing/CPU LOD가
있다. sample 수의 실제 실험 연결은 아래 G15로 확장한다. 고정 shadow 해상도를
이미 연결된 조절 변수처럼 표시하지 않는다.
indirect counter 선언만 있고 실제 producer가 없는 항목은 미계측이다.

기법 사전에서 Lumen·RTX/DXR·Nanite·VSM·MegaLights·DDGI·SSGI·SSR·TAA/TSR·DLSS/FSR/XeSS·
Ray Reconstruction·Frame Generation·volumetric·OIT·Substrate 등을 설명하되 미구현 기법은
`추가 패스·입력 필요` 또는 `기반·SDK 통합 필요`로 명시한다. 이 목록은 구현된 척하는 toggle이나
사용자 품질을 일괄 켜는 preset이 아니다. SDK/API 지원은 정확한 제품 버전과 장치 능력을 확인해야 한다.

실제 동작하는 수치 비교는 세션 실험의 whitelist에 연결된 필드만 제공한다. 같은 주제를 다루는
기존 기법과 UE/NVIDIA의 특정 구현을 동일하다고 주장하지 않는다. Lumen software 경로도 scene
표현과 cache를 요구하므로 RTX GPU 유무만으로 사용 가능 여부를 결정하지 않는다.

공식 GPU Gems 근거:
[전체 자료](https://developer.nvidia.com/gpugems),
[AO와 간접광](https://developer.nvidia.com/gpugems/gpugems2/part-ii-shading-lighting-and-shadows/chapter-14-dynamic-ambient-occlusion-and),
[volume 렌더링](https://developer.nvidia.com/gpugems/gpugems/part-vi-beyond-triangles/chapter-39-volume-rendering-techniques),
[화면 후처리 산란](https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-13-volumetric-light-scattering-post-process).
DXR의 실제 API 계약은 [Microsoft DirectX Raytracing](https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html)을 따른다.

## G15. GPU Gems 품질·비용 실험의 실제 shader 연결

현재 renderer의 품질 변수 중 실제 연산량을 바꾸는 SSAO sample 수와 PCF kernel 크기를 먼저
세션 실험에 연결한다. `RENDER_QUALITY_SETTINGS::iSSAOSampleCount`는4/8/12,
`SHADOW_SETTINGS::iPCFFilterRadius`는0/1/2를 지원한다. 기본12와1을 유지하여 진입만으로
기존 결과를 바꾸지 않는다. 저장 profile의 JSON schema와 기본 품질은 변경하지 않는다.

`Engine_RenderTypes.h`는 typed 입력, `Renderer.cpp`는 SSAO 검증·상수 bind,
`Shadow.cpp`는 shadow descriptor 검증·광원 shader bind를 소유한다. 실제 deferred shader는
SSAO loop 횟수와 PCF offset 범위를 소비한다. directional PCF와 dynamic baked shadow 비교
경로에 같은 kernel을 사용하고 source hair 고유 filter·baked asset은 변경하지 않는다.

PCF radius0/1/2는1/9/25개 kernel 위치다. dynamic baked shadow 비교는 위치마다 complete/static
depth를 읽으므로 texture fetch는2/18/50개다. sample count와 전체 조명 비용을 동일시하지 않는다.
SSAO는 sample 분포·정규화까지 해당 수에 맞추고 기존12개 설정의 결과를 기본 parity 기준으로 삼는다.

Workbench의 두 필드는 owner가 활성인 동안만 실효 설정 위에 적용하고, 상태·fingerprint·조건
whitelist·run 결과에 포함한다. 범위 밖 값은 commit 전에 거부한다. 검증은 실제 FX 컴파일,
headless WARP의 finite/default parity, 허용/거부 값과 loop 소비 연결로 수행한다.
화질·실제 GPU 장치 성능은 사용자의 동일조건 실험에서 판정한다.

## G16. 상용 엔진 비교를 실제 도구의 진단 흐름에 연결

PR505는 `8fdb3d4e6aa8c763f94aa28c683a80d7a8c84397`로 main에 병합됐다. 이후 사용자는
언리얼의 graphics/Profiler와 비교한 부족점도 개념과 함께 전부 툴에 반영하도록 요청했다.
이 후속 변경은 같은 브랜치를 해당 main으로 fast-forward한 뒤 진행하며 기존 저장값을 보존한다.

기존 G14는 렌더링 알고리즘 사전이며 상용 프로파일러의 시간축·메모리·task/wait·resource dependency
구조까지 보여주는 비교 도구는 아니다. `RenderingReferenceGuide.h`의 공용 catalog와 `Render()`를
Profiler와 Workbench 양쪽에서 소비한다. 한 항목은 상용 기능의 개념, 현재 구현, 실제 부족점,
필수 입력, 수치의 의미, 계측하지 못하는 범위, 검증 기준, 다음 실험과 공식 URL을 소유한다.
CPU/GPU timeline, scheduling/context switch, per-draw/resource inspection, RDG, memory allocation,
streaming, material complexity/overdraw, culling/geometry, GI/reflection/shadow, temporal/ray를 대조한다.

표시 상태는 구현·부분 지원·기반 필요를 구분한다. 부족점 설명이 있다고 미구현 renderer나
운영체제 추적까지 지원하는 것으로 표시하지 않는다. 실제 시간축·메모리 관측·기존 옵션 실험은
아래 G17~G19에서 연결하고, 나머지 항목은 필요한 증거와 검증 절차를 툴에서 확인한다.
새 header는 Client vcxproj와 기존 Rendering filter에 등록한다.

공식 비교 근거는 [Timing Insights](https://dev.epicgames.com/documentation/unreal-engine/timing-insights-in-unreal-engine),
[Context Switches](https://dev.epicgames.com/documentation/unreal-engine/context-switches-in-unreal-engine-5),
[Task Graph Insights](https://dev.epicgames.com/documentation/unreal-engine/task-graph-insights-in-unreal-engine-5),
[RDG](https://dev.epicgames.com/documentation/unreal-engine/render-dependency-graph-in-unreal-engine),
[Memory Insights](https://dev.epicgames.com/documentation/en-us/unreal-engine/memory-insights-in-unreal-engine)와
[GPUDump](https://dev.epicgames.com/documentation/unreal-engine/gpudump-viewer-tool-in-unreal-engine)다.
기존 메모리 조사 정본은 `../10-02/2026-10-02_FRAME_PIPELINE_OPTIMIZATION_IMPLEMENTATION_PLAN.md` G12이며,
그 문서의 과거 관측값을 현재 실행 측정값으로 재사용하지 않는다.

## G17. CPU·GPU 시간축과 근거 있는 병목 후보

`ProfilerTool.h/.cpp`는 기존 Engine snapshot을 bounded window로 읽어 완료 frame을 선택한다.
CPU는 thread/depth별 QPC 상대 구간, GPU는 해당 frame의 query 시작 기준 상대 구간을 표시한다.
두 축은 시계가 동기화됐다는 근거가 없으므로 같은 절대 시각처럼 합치지 않는다. 확대·이동·
구간 선택으로 이름·elapsed·확인 가능한 self·패스 draw를 보고 clipping·worker 프레임 경계·
누락·pending을 명시한다. 기존 범용 collector 대신 별도 계측 runtime을 만들지 않는다.

CPU/GPU 예산 초과, frame gap, 애니메이션 갱신 후 미제출, draw/index 제출량과 계측 누락을
근거로 병목 후보와 다음 실험을 안내한다. elapsed만으로 계산·스케줄러 대기·driver stall을
구분하거나 PS 호출만으로 overdraw·ALU 포화를 확정하지 않는다. 이런 원인에는 필요한 추가
trace를 안내하며 수집된 값 이상의 확신을 부여하지 않는다.

## G18. 메모리 관측과 texture cache의 실제 생산자

`Profiler.h/.cpp`는 Capture ON에서만 최대1Hz로 process private commit/working set/peak,
system commit/limit/available RAM, DXGI adapter node0 local/nonlocal usage/budget을 수집한다.
표본 frame/tick/age, process ID·adapter LUID·node와 각각의 validity를 DTO에 보존한다.
같은 OS 표본을 여러 frame에 재사용하므로 메모리 timeline은 sample frame으로 중복 제거한다.
GPU query가 실패해도 process/system 수치는 독립적으로 사용할 수 있고 실패값0은 N/A다.

Working set과 private commit, OS budget과 물리 VRAM, local/nonlocal·UMA를 구분한다.
프로세스 peak는 capture peak가 아니다. main-thread1Hz 표본은 stall 중 독립 샘플링이나 짧은
loading peak를 보장하지 않으며 allocation/free·callstack·asset owner별 점유는 별도 미지원이다.
JSON v3에 additive memory field를 저장하고 구형 캡처에서는 미계측으로 처리한다.

`Material.cpp::LoadSharedTexture`는 기존 TextureRequests/TexturePathHits/TextureUniqueSrvs의
실제 생산자를 연결한다. 각각 요청 시도, 성공한 shared-path 재사용, 성공한 새 SRV 생성 건수다.
마지막 값은 현재 resident SRV 수가 아니다. worker 요청과 완료는 서로 다른 관측 frame에 속할
수 있다. content-hash hit와 GPU 추정 bytes는 생산자가 없으므로 계속 미계측이다.
새 capability metadata로 같은 v3의 이전 미계측0과 새 관측0을 구분한다.

## G19. 원인별 실험과 조건 차이 설명

`RenderingBenchmark.h/.cpp`에 한 필드만 바꾸는 명시적 recipe를 둔다. SSAO on/sample/radius,
shadow on/PCF/strength, Bloom/FXAA, PBR direct/baked/IBL/cube/normal/roughness, fog와 노출/gamma는
기존 세션 명령을 소비하며 G21의 신규 조명 필드를 더해 총41개를 다룬다. 자동으로 다른 옵션을 켜지 않고 현재 전제조건과 화면 기여의
한계를 안내한다. 사용자가 기존 B 대체를 누를 때만 A를 기준으로 B를 준비·적용한다.
해당 변수의 sweep 준비도 기존 bounded 측정 경로로 보낸다.

각 recipe는 효과의 개념, 결과에서 볼 GPU pass/CPU/draw 작업량과 판정할 수 없는 원인을
설명한다. 비교 중 변경된 조건은 opaque fingerprint뿐 아니라 이름과 전후 값으로 보여주고
결과 JSON에도 남긴다. named 조건은 인과 증명이 아니며 동일 조건·부분 표본 계약은 유지한다.

기본 OFF 패스의 품질 실험은 사용자가 ON인 B를 새 A로 채택할 수 있게 연결한다. 원래 장면과
새 A의 차이는 공통 baseline ownership으로 복원하고, A/B 차이만 비교 제외 mask로 처리한다.
기준 채택 때 experiment ID를 갱신해 이전 기준의 결과를 자동으로 섞지 않는다.

## G20. 후속 검증과 남는 기반의 구분

시간축 좌표/clipping·병목 후보의 미계측 경계, process/DXGI 실패·rate-limit·stale age·JSON
roundtrip·legacy 부재, 실제 shared texture helper의 재사용/실패/동시 준비·계수, recipe의
단일 필드·전제조건·sweep·이름 있는 조건 차이를 native focused 검사한다. 최종 Engine public
header 변경은 Product Debug의 SDK·Client 컴파일까지 검증한다. Client/UI는 실행하지 않는다.

OS context switch·task dependency/callstack, per-draw GPU timestamp, RDG resource lifetime,
할당별 owner/free 추적, 중간 render-target dump, deterministic scene replay 및 새 Lumen/DXR/
Nanite/temporal renderer는 이번 연결만으로 구현됐다고 하지 않는다. 툴에서는 각 미흡점을
숨기지 않고 현재 지원 상태·필수 입력·추가 검증을 전부 확인할 수 있게 한다.

## G21. 실제 화면 공간 간접광과 반사 실험

사용자의 추가 요청에 따라 개념 카탈로그에서 멈추지 않고 D3D11의 현재 G-buffer를 소비하는 SSGI·SSR 패스를 구현한다. `Engine_RenderTypes.h`는 기본 OFF인 세션 수치 9개를 소유하고, `Renderer.cpp`는 불투명 합성 직후 별도 `Shader_ScreenSpaceLighting.hlsl`을 호출한다. 기존 Deferred pass 번호를 바꾸지 않는다. 새 shader는 Engine 프로젝트와 filters에 등록하고 정상 Product 배포가 실행 CSO와 Client 소스 사본을 전달한다.

SSGI는 MapPBR 수신점의 반구 방향에서 깊이 교차를 찾아 화면의 HDR radiance를 diffuse 반사율에 따라 더한다. SSR은 반사 방향의 깊이 교차에서 Fresnel·roughness 가중 radiance를 더한다. 기존 baked GI·IBL을 유지하는 가산 실험이므로 물리적으로 완결된 GI/환경 반사 교체로 설명하지 않는다. 원본 불투명 HDR을 두 실험의 공통 radiance 입력으로 사용하고 중간 결과를 별도 ping-pong target에 쌓아 두 패스 성공 후에만 SceneHDR·Bloom을 복사한다. 실패 시 원본 target은 보존하고 기존 MRT/DSV/viewport를 복구한다.

SSGI의 강도·반경·4/8/16 샘플, SSR의 강도·거리·thickness·16/32/64 단계를 Workbench 세션 소유권·A/B·sweep·named 조건에 연결한다. 데이터에 저장된 팀장 옵션은 변경하지 않는다. `Render.SSGI`, `Render.SSR`, 복사 패스를 분리 계측한다. 화면 밖·가려진 면·다중 bounce·시간 누적·denoising·Lumen Surface Cache·DXR BVH는 이 패스가 제공하지 않는다.

종료 증거는 FX5 컴파일, 실제 WARP 픽셀의 유효 교차·miss·강도 0·비수신 재질·finite 결과, 설정 범위/복원/캡처 조건 검증, Debug Product 빌드다. 실제 장면의 미관·노이즈·GPU 프레임 개선 판정은 사용자 화면 A/B로 남긴다.
