# Character Select 기본 200fps / 도화가 궁극기 60fps 개선 계획

## G00. 목표와 측정 기준

사용자가 제공한 `profiler_20260912_005512_789_frame5507_58888_1.json`은 약 76fps 기본 상태, `profiler_20260912_005528_327_frame6474_58888_2.json`은 도화가 궁극기를 포함한다. 기본 프레임 간격 5ms 이하, 궁극기 16.7ms 이하를 목표로 한다. 두 캡처의 중복 233프레임은 합산하지 않으며 효과가 있는 구간과 Profiler refresh/save 비용을 분리한다. 시작 소스는 사용자의 측정 빌드에 대응하는 `3260ba7e`, 브랜치는 `codex/character-select-200fps`다. 해당 성능 파일들은 최신 origin/main과 동일하며 다른 효과 변경 65파일을 임의로 섞지 않는다.

기본 캡처의 효과·refresh/save 제외 1,064프레임 평균 CPU 11.958ms에서 Map.Batch.BindAndDraw self 2.559ms, Render.Shadow self 2.032ms, SkinPalette.Build 0.769ms가 확인됐다. GPU Shadow 6.805ms는 GPU 명령 사이 CPU 제출 공백도 포함할 수 있어 순수 GPU 실행 비용으로 단정하지 않는다. 궁극기 6331프레임 Spawn.Commit self 39.799ms 이후, 6441~6474프레임 평균 CPU 48.577ms, Particle fixed-step 20.752ms와 FrameRebuild 4.972ms, Particle.Render 8.281ms가 확인됐다. 파티클 비용 증가가 fixed-step 따라잡기 횟수를 늘리는 증폭도 조사한다.

## G01. 그림자 재질 준비

`CMapAssetRenderUtils`, `CMapStaticBatchObject`, `CMapAssetObject`에서 그림자가 소비하지 않는 표면 조명·재질 상수를 준비하는 비용을 줄인다. 현재 full material binding을 쓰는 그림자 호출자에 공통 shadow 전용 경로를 연결한다. 알파·UV·tint 의미가 확인된 family만 빠른 경로를 사용하고 복잡한 native/BG/foliage 등은 기존 경로를 유지한다. 그림자 해상도, draw 범위, 알파 판정과 재질 표현은 유지한다. 실제 shader reflection과 이전/이후 depth readback으로 상태 잔류와 cutout 경계를 확인한다.

## G02. 동일 포즈의 skin palette 재계산

`CModel`이 포즈와 모델별 palette cache를 소유한다. 공유 `CMesh`에 마지막 모델 포즈를 저장하지 않는다. combined pose가 실제 갱신되는 Play_Animation, Refresh_BoneCombinedMatrices, Pose_BonesFrom 및 초기화 경로에서 revision을 무효화한다. 동일 모델·동일 pose의 shadow/opaque 재제출은 palette 계산을 재사용하고 shader upload는 계속 수행한다. WModel의 모든 mesh가 같은 전체 skeleton inverse-bind/index 순서를 사용하는 경우 같은 모델 안에서 palette를 공유한다. Assimp의 mesh별 bone/offset 경로는 mesh별 cache로 분리한다. clone, 같은 프레임의 여러 pose 갱신, 다른 모델 교차 제출과 secondary motion을 검사한다.

## G03. 도화가 파티클의 반복 분류와 프레임 재구성

`Effect_Playback.h/.cpp`의 실제 spawn module 순서와 RNG 소비 순서를 보존하면서 stage에서 불변 source class/module 종류와 family flag를 준비한다. 매 particle birth마다 문자열 class 비교·enabled literal 조회·같은 module 검색을 반복하지 않는다. 현재 class 비교 자체는 string_view를 사용하므로 접미사 문자열 allocation이 있다는 뜻은 아니다. orbit가 정확히 0인 경우에만 FrameRebuild의 항등 회전을 생략하며 nonzero orbit 연산은 유지한다. `Effect_Distribution.cpp`는 operation 0/1의 사용되지 않는 maximum table/key 보간만 생략하고 minimum 계산과 random operation 2/3/4는 유지한다. 전체 playback을 일정 step으로 실행해 이전/이후 particle 필드와 RNG 결과를 대조하고 실제 spawn/update/rebuild 비용을 별도로 비교한다. 입자 수, lifetime, simulation step, 표시 품질을 줄여 목표를 맞추지 않는다.

## G04. 추가 계측과 후속 판단

현재 self 비용이 큰 재질 binding, particle render 준비와 profiler 프레임 종료 등은 필요하면 실제 호출 경계에 세부 scope를 추가한다. 새 scope는 기존 capture export와 도구 catalog를 통해 확인할 수 있게 한다. UDP, fiber, worker 도입은 transport나 독립 병렬 작업이 지배적인지 측정한 뒤 결정한다. 우선 불변 데이터 준비와 중복 계산 제거를 적용하고 실제 수치로 후속 후보를 선정한다.

## G05. 검증과 실행 경계

기존 C++ 인코딩과 줄바꿈을 유지한다. G06에서 추가하는 제품 C++ 파일은 Client vcxproj/filters에 함께 등록한다. 변경 CPP의 Debug 컴파일, 수치 동등성 검사, 필요한 shader 검사와 `git diff --check`를 실행한다. Engine public 변경 뒤 Engine→Client 순서로 Product 빌드를 진행한다. 실행 중인 사용자 Client/Server를 종료하지 않고 출력 잠금이 있으면 컴파일을 먼저 완료한 뒤 재빌드 가능한 상태를 알린다. Client 실행·조작·화면 캡처는 사용자만 수행한다. probe의 개선율을 실제 200fps/60fps 달성으로 기록하지 않으며 새 사용자 캡처로 프레임 간격과 긴 프레임을 재검증한다.

## G06. 고정 단계 파티클 갱신의 작업 풀

궁극기 후반에는 같은 프레임에 1~2개의 Playback만 존재하므로 occurrence 단위 분산보다 각 emitter의 독립적인 `Update_Particles`가 실제 병렬 작업 단위다. `CEffectPlayback::Step`은 기존 순서대로 모델·본 anchor, spawn과 첫 event dispatch를 완료한 뒤, 입자가 있는 emitter가 4개 이상이고 전체 입자가 512개 이상일 때만 작업 풀을 사용한다. `m_bHasPortableSourceEvents`가 참이거나 준비된 `SourceDeathEventGeneratorElementIds`가 비어 있지 않으면 기존 직렬 루프를 유지한다. 각 작업은 한 `ELEMENT_STATE`의 particle vector와 RNG·module map 전체를 독점한다. 이 단계에서 다른 emitter가 읽는 action-root, anchor, 문서와 준비된 vector field는 변경하지 않는다. 공유 event queue를 쓰는 갱신, provider particle을 읽는 spawn, trail와 frame append는 병렬 단계 밖에 유지한다.

`Client/Public/Effect_ParticleUpdatePool.h`와 `Client/Private/Effect_ParticleUpdatePool.cpp`에 실제 Effect 소비자 전용 동기 실행 함수를 추가한다. public 호출 계약은 `void Client::Run_EffectParticleUpdates(std::size_t count, void* context, void (*execute)(void*, std::size_t), Engine::CProfiler* profiler)`다. Playback은 재사용하는 element/state pointer 배열을 소유하고, 병렬 갱신에 한해서 입자 수와 module 수로 계산한 작업량이 큰 emitter부터 시작한다. RNG는 각 state에 독립적이며 frame row는 계속 문서 순서로 생성한다. 함수가 반환하기 전까지 context와 root를 보존한다. executor나 OS handle을 Playback에 저장하지 않으므로 transform-history의 stage/copy 계약을 유지한다.

풀은 전용 Windows thread pool과 work handle을 최초 사용 때 준비해 재사용한다. 실제 도화가 문서에서 worker 1/2/3개를 비교한 결과에 따라 백그라운드 callback은 1개로 제한하며, 호출한 main thread도 같은 atomic index queue를 처리한다. 논리 CPU가 2개 이하이면 직렬로 처리한다. 한 번에 하나의 batch만 받고 batch당 callback 1개만 제출하여 작업 대기열이 늘어나지 않는다. 준비 실패는 작업 시작 전 직렬 경로를 사용한다. 작업 중 예외가 발생하면 미할당 작업을 중단하고 이미 실행 중인 callback이 모두 끝난 뒤 원래 예외를 호출자에게 다시 전달한다. 일부 작업을 직렬로 재실행하지 않는다.

main의 `Effect.Particle.Update` 범위 안에서 worker를 모두 합류한 뒤 두 번째 event dispatch, trail와 frame rebuild를 진행한다. 따라서 renderer와 다음 fixed step은 완성된 state만 소비한다. worker는 main이 넘긴 profiler pointer로 callback당 한 번의 CPU scope만 기록하고 D3D·GameInstance·owner/renderer callback을 호출하지 않는다. main이 자체 작업을 마친 뒤 기다리는 부분에는 별도 Join scope를 둔다. 풀 종료는 새 batch가 없는 상태에서 대기 callback을 취소하고 실행 callback을 합류한 뒤 handle을 닫으며, context가 살아 있는 callback을 남기지 않는다.

신규 H/CPP를 Client vcxproj와 filters에 등록하고 실제 Playback 호출까지 Debug 컴파일·링크한다. 격리된 수치 검증은 batch 반복 사용, index별 정확히 한 번 실행, 동시 lane 상한, main 보조 실행, 예외 전파와 callback drain을 확인한다. 실제 도화가 문서와 event 문서를 기존 직렬 결과와 비교하여 particle 값·RNG·event 순서가 유지되는지 검사한다. thread pool 성능과 실제 Client FPS 달성은 구분해 기록한다.

## G07. 측정된 파티클 계산 파일의 Debug 최적화

`Effect_Playback.cpp`, `Effect_Distribution.cpp` 두 계산 파일에 Debug x64의 `/O2`, `/Zi`, Just My Code 해제, `/RTC` 해제를 적용한다. `_DEBUG`, Debug CRT `/MDd`, 기본 precise floating point와 D3D debug layer는 유지한다. 다른 제품 파일의 Debug 설정과 Release 설정은 유지한다. 해당 두 파일에서 명령 재배치와 변수 생략으로 줄 단위 stepping/일부 local 조회가 제한되며, `/RTC`의 이 파일별 실행 검사는 제공되지 않는다.

적용 전 실제 세 문서 362 fixed steps에서 `/Od` 이전/수정과 `/O2` 수정의 모든 평가 필드 hash가 동일했다. 도화가 31930은 302,383 particle rows, DimensionMaster 2050540은 125,898 rows, 도화가 31470은 19,381 rows다. portable event/provider/재저작 24검사도 각 설정에서 통과했다. 도화가 31930의 세 번 median은 이전 2,497.65ms, 연산 정리 2,172.37ms, 선택 최적화 652.004ms이며 게임 프레임이 아닌 headless playback 측정이다. 이 검증 근거로 두 파일에 한정해 적용한다.

## G08. Profiler 종료 시 복사와 GPU 결과 검색

기본 상태의 `다음 FrameInterval - 현재 CpuFrameMs`는 평균 0.501ms다. 이 전체를 profiler 비용으로 단정하지 않으며, 코드에서 확인된 `Commit_CurrentFrame`의 전체 scope vector 복사를 move로 바꾼다. 가득 찬 history에서 제거할 프레임의 scope buffer는 mutex 안에서 pending buffer가 비어 있을 때만 다음 프레임용으로 재사용한다. 사이에 완료된 worker scope가 있다면 그대로 보존한다. GPU query 결과는 보통 최근 네 프레임에 대응하므로 동일한 frame-number 조건을 최근 history부터 검사해 1,200개 프레임을 앞에서부터 걷지 않는다. 캡처 스키마와 CPU/GPU 시간 의미는 유지한다. 이전/이후 실제 Profiler 구현의 history 순서·scope 수·worker completion과 CPU overhead를 검사한다.

## G09. 픽셀·정점 셰이더 호출의 패스별 계측

사용자가 제기한 픽셀 셰이더 수치와 실제 `_1`의 평균 8,470만 `PSInvocations`를 구분한다. 이것은 ALU 명령 수가 아니라 셰이더 호출 수다. 현재 전체 프레임 통계만으로 어느 패스의 픽셀이 반복되는지 알 수 없어 `Profiler`의 GPU scope에 선택적 pipeline query를 연결한다. `Renderer`의 Shadow, NonBlend, SSAO, Lights, Blend, UI 여섯 패스를 선택하고 프레임당 최대 여덟 query, 기존 여덟 프레임 ring 안에서 초기화·재사용한다. 런타임 query 생성, 강제 Flush와 동기 GPU 대기는 추가하지 않는다.

선택한 scope의 `PipelineValid`, PS/VS 호출 수를 sample/aggregate, Profiler GPU 표와 JSON export의 선택 필드로 연결한다. query 미지원·오류·미선택은 0회 측정으로 위장하지 않고 unavailable로 표시한다. 완료되지 않은 query는 기존 bounded ring 정책을 따른다. CPU worker scope와 main join scope도 catalog에 노출한다. 실제 headless D3D draw를 서로 다른 크기로 두 번 제출해 패스별 PS/VS 값과 전체값의 관계, pending·중첩·초과·누락 scope와 JSON 호환성을 검사한다. GPU timestamp의 제출 공백 한계는 계속 명시한다.

## G10. Source character 조명의 stencil 범위 제한

`CRenderer`가 화면 크기의 별도 D24S8 texture/DSV를 소유한다. 일반 scene DSV는 EffectV2 outline과 Esther가 0xff stencil을 사용하므로 공유하거나 지우지 않는다. source material row가 있는 프레임에는 기존 `Target_Depth`의 marker 5인 픽셀만 stencil 1로 쓰는 fullscreen pass를 한 번 수행한다. 이 pass는 색상 target을 연결하지 않고 depth를 읽거나 쓰지 않는다. 기존 ordinary light는 유지하고 source row light만 별도 DSV의 stencil == 1에서 실행한다. 기존 row별 marker/id discard, light 계산, additive 제출 순서와 FP16 target은 유지한다.

`DEFERRED` 기존 pass 0~17을 유지하고 mask 18, source directional 19, point 20, spot 21을 뒤에 추가한다. source wrapper는 earlydepthstencil 속성과 기존 PS 함수 호출만 소유한다. `CLight_Manager`가 기존 SOURCE_CHARACTER receiver 여부와 mask 준비 성공을 `CLight::Render_Desc`에 전달해 pass를 고른다. public GPU context getter나 두 번째 material runtime을 추가하지 않는다. Renderer는 바인딩된 RTV/DSV를 보존하고 mask 및 source light 범위에서만 새 DSV를 사용한 뒤 복원한다. 화면 크기가 바뀌면 새 DSV를 stage한 후 교체한다. optional mask 준비가 실패하면 해당 크기에 대해 한 번 실패를 진단하고 기존 source row 제출을 유지한다.

수치 검증은 기존 deferred offscreen probe를 확장한다. 모든 기존 native program과 ordinary/mixed row, receiver, 배경, 빈 mask, 전체 mask, 픽셀 단위 mask에서 두 light MRT의 이전/이후 값을 비교한다. source row/light 개수를 반복한 동일 workload에서 pipeline PS invocation과 GPU timestamp 중앙값을 측정하고 mask 생성 비용을 포함한다. depth/stencil 보존과 source pass 뒤 원래 DSV 복원도 확인한다. 제품 Client나 UI는 실행하지 않는다.
## G11. 계측·binding의 Debug 비용

`Profiler.cpp`의 실제 CPU probe에서 346 scopes/frame이 `/Od` 0.216ms에서 `/O2` 0.065ms, 120프레임·1,024 scopes/frame 집계가 3.884ms에서 0.668ms로 줄었다. GPU timing은 이 CPU probe에 포함되지 않는다. `Shader.cpp`와 `MapAssetRenderUtils.cpp`의 실제 shader/binder 비교도 1,296 depth parity cases가 두 설정에서 동일했고 binding CPU 비용이 줄었다. 이 세 파일에도 G07과 같은 제한된 Debug x64 `/O2`, PDB, RTC/JMC 해제를 적용한다. Debug CRT·D3D layer·floating-point 정책은 유지한다. 캡처를 pause한 동안 데이터가 바뀌지 않는 Profiler 패널은 일정 간격마다 재집계하지 않으며, Capture/Reset/window 변경 시에는 즉시 갱신한다.

## G12. 쿠크세이튼 창 제목 FPS

사용자 추가 요청에 따라 `Level_KakulSaydonArena.cpp::Render`의 성공 경로에서 기존 `CMainApp::Update_DebugWindowTitleWithFps`를 호출한다. 기본 제목은 Loader가 쓰는 `KoukuSaydon arena loading complete`로 맞추고 Character Select/Valtan과 같은 500ms 갱신·ImGui smoothed FPS를 사용한다. 최종 표시 형식은 공통 helper의 `KoukuSaydon arena loading complete | FPS 200.0`이다. 기존 Debug 경계와 실패 반환, 제품 UI는 유지하며 새 timer/매 프레임 SetWindowText 경로를 만들지 않는다.

