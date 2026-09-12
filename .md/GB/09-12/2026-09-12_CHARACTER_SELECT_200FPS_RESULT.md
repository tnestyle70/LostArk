# Character Select 200fps / 도화가 궁극기 60fps 개선 결과

## G00. 현재 상태와 측정 경계

소스 기준점은 `3260ba7e`, 작업 브랜치는 `codex/character-select-200fps`다. 사용자가 제공한 기본 `_1`과 도화가 궁극기 `_2` 캡처를 분석하고 아래 최적화 및 추가 계측을 반영했다. 실제 새 Client 실행에서 기본 200fps·궁극기 60fps를 달성했는지는 아직 측정하지 않았다. Client/UI 실행과 화면 검증은 사용자가 수행한다. 이 문서의 수치 비교는 명시한 headless fixture에 한정한다.

작업 도중 KoukuSaydon의 데이터·도구·effect carrier 등 다른 변경이 작업 폴더에 추가됐다. 해당 변경은 수정하거나 되돌리지 않았으며, 최종 통합 빌드는 현재 작업 폴더의 변경을 함께 소비한다. `Level_KakulSaydonArena.cpp`의 이번 작업 범위는 기존 공통 FPS 제목 helper 호출이다. 소유권이 다른 변경을 함께 stage/commit하지 않는다. 과거 PR #363/#364는 이미 병합됐으며 이번 변경은 그 이후 성능 작업이다.

## G01. 캡처에서 확인한 병목

두 JSON의 중복 233프레임을 합산하지 않았다. `_1`에는 작은 effect burst도 있어 효과·Profiler refresh/save를 제외한 1,064프레임을 기본 상태로 비교했다. 평균 CPU 11.958ms, 프레임 간격 12.563ms이며, Map.Batch.BindAndDraw self 2.559ms, Shadow self 2.032ms, SkinPalette.Build 0.769ms, Animation.Channels.Update 0.502ms다. 기본 상태에서 모델 하나만 애니메이션을 갱신하고 실제 제출하며, 숨겨진 모델 갱신은 관측되지 않았다.

궁극기로 추정되는 무거운 발생은 6331프레임의 Spawn.Commit self 39.799ms부터다. `_2` 전체 평균을 궁극기 비용으로 사용하지 않았다. 6441~6474프레임의 평균 CPU는 48.577ms, Service.Update 25.772ms, Particle.Update 13.022ms, Spawn 7.345ms, FrameRebuild 4.972ms, Occurrence.Render 9.186ms다. 긴 프레임 이후 fixed-step 따라잡기 횟수가 늘어나 비용을 증폭했다. 무거운 구간은 6331~6474의 144프레임·약 4.19초이며 비용이 증가하는 중에 캡처가 끝났다. 전체 궁극기 수명이나 최악값을 모두 포착한 자료는 아니다.

기본 상태 전체 GPU pipeline에는 평균 8,470만 PS invocation이 관측됐다. 이는 ALU 연산 수가 아니라 [D3D11 pipeline statistics의 픽셀 셰이더 호출 수](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_query_data_pipeline_statistics)다. GPU Shadow elapsed가 늦게 실행되는 CPU Profiler refresh 시간까지 따라 증가하는 정황이 있어 6.805ms를 순수 그림자 shader 비용으로 단정하지 않았다. Long Operations의 main 표시는 임계값 이상 완료 호출의 위치이며 전체 프로세스의 thread 목록이 아니다.

원본 분석: `out/CharacterSelect200Fps20260912/captures/README.md`, `summary.json`, `analyze.py`.

## G02. 동일 포즈의 palette 재사용

`CModel`이 clone별 palette와 pose revision을 소유한다. Play/명시 pose/secondary motion/다른 모델 포즈 복사 등 combined pose가 갱신되는 지점에서 무효화한다. 공유 `CMesh`는 결과 포즈를 소유하지 않는다. WModel의 전체 skeleton palette는 같은 모델의 mesh끼리 공유하고, Assimp mesh의 개별 bone subset/offset은 별도 palette로 유지한다. shader upload는 계속 수행한다.

실제 이전/수정 함수의 424개 검사에서 행렬이 비트 단위로 일치했다. 서로 다른 모델의 교차 제출, clone, 1/7/351/512 bones, mesh별 offset, 같은 프레임의 pose 변경, costume-only bone, revision wrap를 포함했다. 351 bones·6 meshes·2 passes fixture는 build 12회에서 1회, Debug `/Od /MDd /RTC1` 계산 중앙값 0.517158ms에서 0.0402053ms로 줄었다. Shader 실제 업로드와 게임 프레임 전체 비용은 이 CPU fixture에 포함하지 않았다. Mesh/Model 실제 Debug CPP 컴파일도 통과했다.

증거: `out/CharacterSelect200Fps20260912/skin/receipt.json`, `prepare_probe.py`, `compile.ps1`.

## G03. 그림자 재질 준비와 맵 인스턴싱

두 맵 shadow 호출자가 `Bind_ShadowMaterial`을 사용한다. 검증한 legacy·PBR·specular family 0~5에서 그림자가 소비하지 않는 조명·표면 상수 준비를 생략한다. diffuse/source diffuse 선택, UV·mirror·tint·opacity dither·alpha cutoff와 stale native state reset은 보존한다. 그 외 복잡한 family는 기존 전체 binding을 사용한다.

실제 Shader와 원문에서 추출한 utility/material binding, 기존 제품 shadow CSO로 1,296사례·1,327,104 depth pixels가 `/Od`와 선택 `/O2` 모두 이전 결과와 일치했다. 실제 CPP 컴파일도 통과했다. 기본 맵은 이미 `(assetId, mirrored)` 기준 71개 batch로 묶여 있고 779 placements를 재현한다. 같은 WModel을 가리키는 일부 asset도 lightmap/environment override가 달라 무조건 합치지 않았다.

다음 캡처에서 Map.Batch와 Map.Shadow의 Material.Bind / Pass.Apply / Mesh.Submit을 각각 측정할 수 있다. 기존 부모 scope도 유지했다.

## G04. 캐릭터 조명의 픽셀 호출 제한

기존 source character material row마다 반복하던 fullscreen light pass 앞에 marker 5만 기록하는 mask를 한 번 만든다. 별도 D24S8를 사용해 EffectV2/Esther의 scene stencil을 보존한다. source 조명만 stencil==1에서 실행하고 기존 재질별 discard·광원 계산·FP16 additive 순서를 유지한다. 기존 shader pass 0~17은 유지하고 새 pass 18~21을 추가했다. 준비 실패·미지원 target은 한 번 진단하고 기존 조명 경로를 사용한다.

FP32 및 실제 FP16 light MRT의 4,788사례, 39,223,296 finite components에서 차이 0·nonfinite 0이다. scene depth/stencil과 mask depth도 보존했다. 크기 변경 성공/실패와 cache 재사용, 실패 시 기존 DSV 유지도 확인했다.

RTX 4070, 1280×720, 8 material rows×4 lights, marker 5 영역 1/16의 fixture에서는 mask 생성 비용을 포함해 PS 29,491,200→2,764,800(-90.6%)였다. 7회 GPU 중앙값은 directional 0.348064→0.079168ms, point 0.338976→0.074880ms, spot 0.348800→0.077120ms다. 화면 전체가 marker 5인 fixture에는 추가 mask 비용이 있어 모든 화면에서 빨라진다고 주장하지 않는다. 실제 Character Select의 면적과 절감량은 새 사용자 캡처가 필요하다.

증거: `out/CharacterSelect200Fps20260912/render/mask_probe.log` 및 해당 디렉터리 probe/compile 기록.

## G05. 파티클 계산과 worker

불변 spawn module 분류·flag를 stage에서 준비하고 사용하지 않는 최대 분포 interpolation, 0 orbit의 항등 회전, event generator가 없는 source의 반복 event scan을 제거했다. 불변 데이터 순회에서 Debug checked iterator 경합을 줄였으며 RNG·입자 수·수명·fixed step과 출력 순서를 유지한다.

큰 독립 emitter 갱신은 background worker 하나와 main이 분담한다. 실제 1/2/3 worker 비교에서 worker 셋의 안정적 이득이 없어 한 개로 제한했다. 512 particles·4 emitters 미만 및 portable/death event 경로는 직렬이다. Spawn/provider와 event dispatch는 기존 순서를 유지하며 worker join 이후에만 trail/frame rebuild/다음 step으로 진행한다. 작업은 emitter의 particle/RNG state 전체를 독점하고 큰 작업부터 분배한다. D3D 호출은 worker에서 하지 않는다.

pool의 실제 Windows thread 검사는 반복·동시 batch 230회, 정확히 한 번 실행, main 참여, 최대 두 executing lanes, 예외 전달과 반환 뒤 callback 없음이 통과했다. 실제 도화가 문서의 362 fixed steps에서 302,383 particle rows, peak 1,905, hash `63168e35e868c8f6`가 이전 직렬 결과와 최종 `/Od`·선택 `/O2` 모두 일치했다. 같은 문서 두 개의 독립 occurrence도 603,956 rows, peak 3,808, hash `786259d40924279f`가 일치했다. portable event/provider/재저작 24검사도 두 설정에서 통과했다.

worker 정책 비교의 중앙값은 한 문서에서 직렬 525.408ms→worker 하나 505.246ms, 두 문서에서 1,397.27ms→1,321.6ms였다. 이는 마지막 no-event scan 제거 전 같은 Playback 구현끼리 비교한 CPU fixture이며 worker의 추가 이득은 각각 약 3.8%·5.4%다. 최종 수치 동등성 실행의 선택 `/O2` 시간 598.496ms·1,258.42ms는 각 단일 실행이며 정책 비교 중앙값과 섞지 않는다. 큰 개선은 반복 계산 제거와 선택 컴파일 최적화에서 나왔고 worker 수나 합산 CPU 시간을 게임 FPS 향상으로 환산하지 않는다. 실제 캐릭터 본과 GPU 표현은 이 fixture에서 검증하지 않았다.

증거: `out/CharacterSelect200Fps20260912/artist/receipt.json`, `README.md`, `out/CharacterSelect200Fps20260912/workers/result.json`.

Product 빌드 후 같은 파일에 통합된 Kouku DirectLoc 변경까지 포함해 현재 Playback/Distribution `/O2`, pool `/Od`를 별도 출력 폴더에서 재컴파일·링크했다. Artist 1/2복사의 위 입자 수·전체 평가값 hash와 event/restage 24검사가 다시 모두 통과했고 검증 중 소스 hash가 유지됐다. 새 follow 단계는 worker join 이후 실행하며 Artist 문서에는 해당 DirectLoc 모듈이 없다. 이 재검증의 source snapshot과 receipt는 `out/CharacterSelect200Fps20260912/artist/post_integration/`에 보존했다. 해당 단일 실행 시간은 이전 벤치마크와 성능 비교하지 않는다.

## G06. Debug 설정과 Profiler

`Effect_Playback.cpp`, `Effect_Distribution.cpp`, `Profiler.cpp`, `Shader.cpp`, `MapAssetRenderUtils.cpp`만 Debug x64에서 `/O2 /Zi`를 사용하도록 지정했다. `_DEBUG`, Debug CRT, 기본 precise floating point와 D3D debug layer는 유지한다. 해당 파일의 `/RTC`와 Just My Code는 해제되며 줄 단위 stepping과 일부 local 변수 조회가 제한된다.

Profiler history의 scope 복사를 move로 바꾸고, mutex 안에서 pending worker 기록이 비어 있을 때 evicted buffer를 재사용한다. GPU 결과는 최근 프레임부터 찾는다. 캡처 pause 후에는 pending GPU 결과를 회수한 뒤 변경 없는 패널 재집계를 멈춘다. 실제 CPU probe에서 346 scopes/frame은 약 0.216→0.065ms, 120프레임×1,024 scopes 집계는 3.884→0.668ms였다. 이 비교는 선택 컴파일 최적화 효과를 포함하며 게임 FPS가 아니다.

GPU 표와 v3 JSON에 선택된 여섯 패스의 PS/VS invocation을 연결했다. 기존 scope 필드는 유지하고 `pipelineValid`, `psInvocations`, `vsInvocations`를 추가했다. 프레임당 최대 8개의 선택 패스 pipeline query를 기존 8개 ring slot에서 재사용한다. 총 64개의 optional query이며 강제 Flush나 동기 GPU 대기를 추가하지 않았다. 미지원·미선택·불완전 통계는 `--`다. worker와 Join scope도 catalog에 연결했다.

실제 WARP/하드웨어 draw로 패스별/전체 수치 합계, 미선택 부모, 8-query 한도, truncated scope, pending/pause/resume을 검증했고 D3D debug 오류는 0이었다. 1,200프레임 CPU history의 buffer 재사용과 End/Commit 사이 worker 기록 보존도 통과했다. CaptureIO 실제 저장/비동기 저장/실패 보존과 기존 v3 호환성은 통과했고 OS 권한이 필요한 symlink 검사 두 개만 1314로 제외됐다.

증거: `out/CharacterSelect200Fps20260912/gpu-profiler/receipt.json`, `README.md`, `out/CharacterSelect200Fps20260912/profiler/*/run.log`.

## G07. 쿠크세이튼 FPS 표시

쿠크세이튼 Level의 Render 성공 경로에서 기존 공통 helper를 호출한다. Debug 창 제목은 `KoukuSaydon arena loading complete | FPS 200.0` 형식으로, 기존 Character Select/Valtan처럼 실제 smoothed FPS를 500ms 간격으로 갱신한다. 표시 예시의 200.0은 달성 수치가 아니다. 해당 CPP의 Debug 컴파일을 통과했다.

## G08. 빌드와 남은 사용자 확인

실제 Engine Debug x64 빌드와 사용자 종료 확인 후의 Product Debug 빌드 모두 종료 코드 0으로 통과했다. Product는 Engine → Shared → Server → Client 네 프로젝트를 컴파일·링크하고 SDK·shader·runtime DLL을 배포했다. 본 Product 실행은 총 729.242초이며 2026-09-12 01:52:58 KST에 Client EXE 생성을 완료했다. 빌드 runner가 확인하는 필수 runtime 입력 누락은 0개다. 별도 data publish와 광역 runtime 진단은 실행하지 않았다.

빌드 로그는 `out/CharacterSelect200Fps20260912/engine-build.log`, `product-build.log`, Product 결과는 `out/BuildPipeline/runs/20260911T165258874Z-debug-product.json`이다. `build-audit.json`에서 Engine DLL·Deferred CSO·Engine SDK Model/Profiler header의 배포본 일치를 확인했다. 실제 Product에서 새로 컴파일한 Binary/MapInstance/Deferred CSO도 수치 검증에 사용한 바이너리와 SHA256이 일치한다. 다섯 파일의 실제 CL command 기록은 `/O2 /Zi /MDd /fp:precise`이며 RTC/JMC가 없는 것을 확인했다. 변경 프로젝트 XML parse와 `git diff --check`도 통과했다. 외부 PDB 누락, shader 경고 및 기존 소스 문자 집합 경고가 남아 있으므로 경고 0 빌드는 아니다.

실행 파일은 `Client/Bin/Debug/Client.exe`, `Server/Bin/Debug/Server.exe`다. 에이전트가 제품 EXE를 실행하거나 화면을 조작하지 않았으며, 최종 Client 실행 성공과 FPS 수치는 사용자 확인 전이다.

사용자는 새 빌드로 Server+Client profile을 Ctrl+F5로 시작하고 Character Select 기본 상태와 도화가 궁극기 전체 구간을 각각 기록한다. 평상시 성능은 Profiler/도구 창을 닫은 상태도 함께 확인하고, 상세 캡처에서는 새 GPU PS/VS 표와 Map 단계·worker/Join scope를 사용한다. 1,200프레임 history는 200fps에서 약 6초이므로 긴 궁극기는 구간별로 나누어 저장한다. 실제 프레임 간격, 긴 프레임과 화면 품질을 함께 확인하기 전에는 200/60fps 달성이나 사용자 visual PASS로 기록하지 않는다.
