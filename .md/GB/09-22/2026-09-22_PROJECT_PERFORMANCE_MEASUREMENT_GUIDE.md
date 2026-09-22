# 프로젝트 공통 성능 계측·최적화 가이드

기준일: 2026-09-22. 실제 코드 위치, 베른 Profiler JSON, 이번 변경의 소스 상태를 기준으로 한다.
이 문서는 계측 방법과 조사 대상을 소유한다. 구현 범위는
[구현 계획](2026-09-22_BERN_RELEASE_PROFILER_OPTIMIZATION_IMPLEMENTATION_PLAN.md),
컴파일·자동 검사·적용 여부와 남은 사용자 검증은 대응 RESULT를 정본으로 삼는다.

## G00. 베른 캡처가 보여 준 우선순위

이번 최적화는 **공통 map batching/culling, CModel/CMesh LOD, Renderer, navigation 경로**를 대상으로 한다.
베른만 object를 숨기거나 발탄만 임의 거리에서 simulation을 멈추는 정책을 추가하는 작업이 아니다.
같은 경로를 쓰는 베른·발탄·Character Select·쿠크에 동일한 알고리즘과 admission 조건이 적용된다.
단, 기존 Area의 map load scope, 조명·카메라·저작 데이터와 재질별 LOD 허용 여부는 서로 다르다.
따라서 공통 코드가 개선되어도 각 장면의 이득과 시각적 적합성은 별도로 확인해야 한다.

원본은 [Client/Bin/ProfilerCaptures](C:/Users/user/Desktop/LostArk/Client/Bin/ProfilerCaptures)의
한국어 `베른_*.json` 5개다. 전체 파일에는 카메라 이동과 이전 장면이 섞여 있다.
특히 바다 파일 앞부분에는 풀줌아웃 구간이 남아 있다. 이름만 보고 전체 파일 평균을 A/B 비교하면 안 된다.
분모와 선택 구간은 [분석 보고서](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/CAPTURE_ANALYSIS.md),
[선택 구간 JSON](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/selected_summary.json)에 있다.

| 선택 구간 | 완료 CPU 프레임 / 유효 GPU 프레임 | 평균 frame interval | 평균 처리율 | draw / frame | map visible |
|---|---:|---:|---:|---:|---:|
| 일반, 15027–15146 | 120 / 116 | 46.359 ms | 21.571 FPS | 1,107 | 1,025 |
| 풀줌아웃, 16179–16262 | 84 / 80 | **376.519 ms** | **2.656 FPS** | **19,761** | **39,009** |
| 바다, 17317–17436 | 120 / 116 | 35.433 ms | 28.222 FPS | 244 | 13 |
| 바닥줌인, 19483–19602 | 120 / 116 | 39.817 ms | 25.115 FPS | 427 | 125 |
| 파일명 `그람자절반`, 21643–21762 | 120 / 116 | 50.530 ms | 19.790 FPS | 897 | 658 |

처리율은 `1000 / 평균 frameIntervalMs`다. 프레임별 FPS를 산술평균한 값이 아니다.
원본 분석 보고서의 percentile은 `(N-1)×p` 위치의 선형 보간이고, 이번 JSON exporter의
`summary.percentileMethod`는 `nearest-rank`다. p95/p99를 재비교할 때 동일한 계산 정의로 다시 집계한다.
위 구간은 같은 설정의 통제된 A/B가 아니다. 원본에는 build configuration·카메라·해상도·그림자 설정의
확인 가능한 metadata가 없으므로 Release 기준 성능 또는 그림자 절반 설정의 효과라고 단정할 수 없다.

풀줌아웃의 `Render.NonBlend`는 CPU 306.125 ms, GPU elapsed 320.890 ms다.
CPU와 GPU는 겹쳐 실행되므로 두 값을 더하지 않는다. 이 구간은 17,762 instanced draw,
46,562,757 direct indices/frame, map batch 16,421개를 제출했다. **인스턴싱이 없었던 것이 아니라,
인스턴싱 후에도 제출 batch와 mesh가 매우 많았다.** 기존 LOD도 있었지만 이 캡처의 indirect draw는 0이다.
따라서 우선순위는 많은 작은 batch의 CPU 제출·재질 바인딩·driver 비용과 geometry/LOD다.

전체 유효 `Render.Shadow` 표본에서 PS/VS invocation은 모두 0이었다.
이는 그 scope 안에서 raster draw가 관측되지 않았다는 뜻이다. baked shadow나 기존 depth cache,
환경 조명까지 존재하지 않았다는 뜻은 아니다. 이번 자료로 shadow 해상도를 낮추면 몇 FPS 오른다고 계산하지 않는다.

기존 CPU scope 누적 누락은 수백만~수천만 건이다. 줌아웃의 실제 instanced draw 17,762개에 비해
`Map.Batch.Mesh.Submit` 표본은 약 1,847회만 남아 있었다. 누락된 자식 시간이 parent self로 보일 수 있다.
이 상태에서 leaf 순위만 보고 병목을 확정하거나 scope를 더 많이 넣는 것은 잘못된 방향이다.

## G01. Profiler.h → MainApp → ProfilerTool → JSON의 측정 계약

| 실제 소유 파일 | 책임 | 이번 소스 변경의 범위 |
|---|---|---|
| [Engine/Public/Profiler.h](C:/Users/user/Desktop/LostArk/Engine/Public/Profiler.h), [Profiler.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Profiler.cpp) | frame·scope·counter·GPU query의 단일 소유자 | 프레임별 CPU 누락, 선택적 상세 scope, 공통 map/LOD/shadow/light counter. 수집 정책 전환은 frame 경계에서 처리한다. |
| [MainApp.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MainApp.cpp) | 기존 ImGui 안에 Profiler 창 연결 | Debug/Release 공통 F7, 창 숨김과 계측 중단 분리. F1 개발 도구는 Debug 전용이다. |
| [ProfilerTool.cpp](C:/Users/user/Desktop/LostArk/Client/Private/ProfilerTool.cpp) | Capture/Reset/이름/Save·표시 | 상세 draw scope 기본 비활성, 선택한 최근 frame window 저장, 누락 경고. |
| [ProfilerCaptureIO.cpp](C:/Users/user/Desktop/LostArk/Client/Private/ProfilerCaptureIO.cpp) | snapshot을 worker에서 안전하게 JSON 기록 | build/device/viewport/camera/export 시점 설정, 분포 요약, GPU 유효 표본 수. |

표의 소스 반영은 사용자 화면 검증 완료를 뜻하지 않는다. 최신 빌드와 검사 결과는 대응 RESULT에서 확인한다.
F7은 창을 여는 키이고, JSON 기록은 창의 **Save JSON** 버튼으로 실행한다.

계측은 아래 세 가지를 함께 둔다.

| 계측 종류 | 무엇을 답하는가 | 넣는 위치와 제한 |
|---|---|---|
| CPU scope | 이 단계의 경과 시간이 얼마인가 | 호출자 기준 phase/batch 앞뒤에 `CProfilerScope`. 동적 asset 이름을 scope 이름으로 계속 생성하지 않는다. |
| GPU timestamp / pipeline query | 제출된 pass 구간이 얼마나 걸리고 VS/PS 작업량이 얼마인가 | 기존 `Renderer::Draw`의 pass 경계를 사용한다. immediate context를 소유한 thread에서만 query를 발행·회수한다. |
| 작업량 counter | 시간이 커진 이유가 건수·크기·실패 때문인가 | 실제 성공/실패 지점에서 개수·bytes·선택 결과를 기록한다. 자료형에 enum만 추가하면 아직 계측된 것이 아니다. |

한 가지를 더 세분할 때는 `상위 시간 + 처리 건수 + 입력 크기 + 실패/제외 이유`를 한 묶음으로 추가한다.
예를 들어 culling은 candidate/visible, LOD는 원본/선택 indices, particle은 alive/spawn/death,
navigation은 query/expanded/path length를 함께 본다. `시간만 2배`보다 `처리량은 같은데 ns/item이 2배`가
구현 결함을 더 잘 가리킨다.

`CProfilerDetailScope`는 per-draw 상세 진단용이다. 기본 비교는 이를 끄고 pass timing과 counter로 한다.
상세를 켠 캡처와 끈 캡처를 같은 조건으로 합산하지 않는다. 현재 CPU scope cap, GPU scope cap,
history 길이의 제한은 [Profiler.h](C:/Users/user/Desktop/LostArk/Engine/Public/Profiler.h)에서 확인한다.
cap을 단순히 크게 만드는 것보다 빈번한 지점을 thread별 누적으로 합치고 끝에서 한 번 반영하는 편이 먼저다.
이 thread별 누적 확장은 아래 표에서 **제안**으로 분리했다.

JSON의 CPU partial은 계측 overflow가 있었음을 나타낸다. 상세 scope를 의도적으로 끈 것은 overflow가 아니다.
`cpuScopesWithinBudget`가 참이어도 모든 함수에 scope가 있다는 뜻은 아니다.
GPU pending/invalid/disjoint는 0 ms가 아니며 유효 평균의 분모에 넣지 않는다.
`PSInvocations`는 pixel shader 호출 수이고 ALU instruction 수나 GPU 사용률이 아니다.
GPU timestamp에 CPU의 명령 공급 공백이 포함될 수 있으므로 CPU와 GPU elapsed가 비슷하다고 GPU 연산 포화로 단정하지 않는다.

## G02. Debug와 Release에서 동일 장면을 기록하는 순서

1. 필요한 구성을 정상 Product Build로 준비하고 사용자가 해당 Client를 실행한다. Release는 debugger 없이
   단독 실행한 값을 제품 기준으로 삼는다. Debug는 오류·자료구조·계측 정확성 검증과 개발 체감 개선용이다.
   두 구성을 동시에 실행하면 CPU/GPU/메모리 자원을 서로 점유하므로 비교할 때는 한 Client씩 실행한다.
2. 서버 승인 절차로 대상 Level에 들어간다. 같은 character, 위치, 카메라 view/projection, zoom, 해상도,
   창 상태, frame cap/VSync, render settings, 활성 NPC/monster/particle 수를 맞춘다.
   CPU/GPU 모델, driver, 전원 상태, 소스 revision과 authoring/runtime revision도 실험 기록에 남긴다.
3. warmup 후 첫 로드·shader 생성·texture upload가 안정되는 것을 확인한다. 초기 제안은 20–30초지만,
   고정 시간보다 실제 load/prewarm scope가 사라지고 작업량이 일정해지는 조건을 우선한다.
4. F7을 열고 이름을 `베른_풀줌아웃_Release_A_01`처럼 입력한다. **Detailed per-draw CPU scopes는 끈다.**
   Capture를 켜고 Reset한다. 수집/Reset 전환은 다음 frame 경계부터 적용되므로 완료 frame을 확인한다.
5. F7로 창을 숨기고 동일 카메라·동작 구간을 재현한다. 창을 숨겨도 Capture는 계속된다.
   정지 카메라, 고정 이동 경로, 동일 skill 순서, 동일 boss pattern을 각각 별도 실험으로 저장한다.
6. F7을 다시 열어 Capture를 끈다. 다음 frame 이후 정지가 반영되고 마지막 GPU query가 회수되는 시간을 준다.
   pending이 남으면 유효 표본 수와 상태를 함께 보존한다. 화면을 억지로 멈추거나 GPU를 동기 대기시키지 않는다.
7. `Frames`는 표시와 저장에 사용할 **최근 완료 frame 수**다. 기본 120, history 최대 1200이다.
   `Save only selected frame window`를 켜면 최근 선택 개수만, 끄면 유지된 전체 history를 저장한다.
   이것은 임의 시작/끝 frame 선택기가 아니다. 긴 이동 캡처는 전체를 저장한 뒤 일정 작업량 구간을 따로 분석한다.
8. Save JSON을 누르고 완료 메시지를 확인한다. 동일 조건에서 A/B를 교대로 3–5회 반복한다.
   평균·p50·p95·p99·max와 16.667/33.333/100 ms 초과 frame 수, 유효 표본 수를 비교한다.
   평균 차이가 반복 실행 간 흔들림보다 작으면 개선으로 채택하지 않는다.

현재 UI를 다시 열고 Capture를 끄는 과정은 끝부분에 UI frame을 조금 포함할 수 있다.
엄밀한 창 숨김 비교는 JSON에서 동일한 안정 구간을 선택하고 `ImGui.*` 작업량과 frame 번호를 함께 확인한다.
120frame은 60FPS에서 약 2초, 이 줌아웃 캡처의 속도에서는 약 45초다.
동작 재현은 frame 수뿐 아니라 실제 경과 시간과 이벤트 수를 맞춰야 한다.

metadata의 camera/settings는 **export 시점**에 한 번 읽은 값이다. history 모든 frame의 설정을 증명하지 않는다.
현재 자동 저장되지 않는 source revision, frame cap/VSync, worker 정책, 시나리오 seed, 서버 실행 조건은
실험 기록에 직접 남긴다. 저장 버튼 이후 JSON 파일 쓰기는 background worker에서 진행하지만
snapshot 복사와 메모리 사용은 여전히 측정 대상으로 본다.

인게임 화면은 사용자가 같은 이름의 이미지로 남겨 JSON과 연결한다. 최소한 같은 카메라 구도·HUD 상태·
가시 객체·LOD 경계·그림자·투명 이펙트가 확인되어야 한다. 이미지 한 장은 FPS 분포나 성능 원인을 증명하지 않는다.
성능 JSON과 화면 비교는 각각 수치와 시각적 회귀를 검증한다. 에이전트는 Client/UI를 대신 실행·조작하거나 캡처하지 않는다.

Release 측정은 최적화된 실행 파일에서 조사하는 것이 기준이다. 외부 도구가 필요하면 Visual Studio
Performance Profiler의 CPU Usage로 함수 표본을, Memory/File I/O로 allocation과 로드를 별도 확인한다.
계측 자체 비용이 더 큰 instrumentation을 동시에 모두 켜지 않는다.
[Microsoft의 Release 성능 측정 안내](https://learn.microsoft.com/en-us/visualstudio/profiling/profiling-feature-tour?view=visualstudio).

## G03. Renderer·Map·Mesh에서 조사할 후보

아래의 **기존**은 producer가 코드에 있다는 뜻이며 해당 장면에서 병목으로 측정됐다는 뜻은 아니다.
**이번**은 이번 작업에서 추가·수정한 소스 범위이고, **제안**은 아직 별도 계측/실험이 필요하다.
우선 A는 지금 베른 증거와 직접 연결되는 항목, B는 반복 재현으로 확인할 항목, C는 해당 시나리오 발생 시 조사할 항목이다.
이 우선순위는 오류 severity가 아니다.

| 후보·우선순위 | 실제 위치와 현재 계측 | 추가로 기록할 수치 / 재현 workload | A/B 및 채택 조건 |
|---|---|---|---|
| 01. map batch 수·분할, A | [MapPlacementRuntime.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapPlacementRuntime.cpp) `Stage_PlacementRuntime`, `Is_BatchEligible`; 기존 batch/fallback counter | asset/material/spatial key별 batch 수, batch당 instance·mesh 수, 1-instance batch 비율. 고정 풀줌아웃 | 같은 visible instance로 key·spatial granularity 변경 후보 비교. batch 감소가 culling 정밀도 손실로 geometry를 늘리는지 함께 본다. |
| 02. culling과 visible pack, A | [MapStaticBatchObject.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapStaticBatchObject.cpp) `Upload_VisibleInstances`; 기존 `Map.Batch.Visibility/CullAndPack`, 이번 candidate/visible counter | tested/rejected/visible 수, ns/candidate, pack bytes, camera·transform revision 재사용률 | 정지/이동 카메라 각각 비교. spatial reject/cache 후보가 실제 visible ID 집합을 보존하는지 검증한다. |
| 03. LOD 선택·geometry, A | [StaticMeshLod.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/StaticMeshLod.cpp) `Select_Range`, [Mesh.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Mesh.cpp); 이번 직접 draw와 LOD별 counter | available draw, LOD0/1/2 draw, 원본/제출 indices. 줌 단계별 동일 camera | 이번 CPU range 선택은 이전 compute dispatch 비용을 제거한다. 선택 range·화면오차 동일성, 실제 VS/indices 감소와 frame을 함께 본다. LOD가 draw 수 자체를 줄이지는 않는다. |
| 04. LOD admission 사유, A | [MapAssetRenderUtils.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapAssetRenderUtils.cpp), [MapStaticBatchObject.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapStaticBatchObject.cpp) | **제안:** masked/blend/displacement/near-plane/no-generated-LOD별 거부 수, projected error·scale 분포 | 재질 원본·보수적 bounds를 유지한 채 안전한 admission만 확대한다. 이미 정상인 재질의 geometry를 일괄 강제하지 않는다. |
| 05. 재질·shader state 바인딩, A | [MapStaticBatchObject.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapStaticBatchObject.cpp), [Material.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Material.cpp), [Shader.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Shader.cpp); 기존 상세 Bind/Apply scope | **제안:** pass apply·CB/SRV 변경·동일 상태 재설정·program switch 수, bytes/draw | opaque 정렬·동일 상태 생략·material batch 후보. state cache는 다른 pass/ImGui 후 invalidation을 검증하며 source 재질 출력은 유지한다. |
| 06. CPU draw 제출·driver, A | [Renderer.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Renderer.cpp) `Render.NonBlend`, [Mesh.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Mesh.cpp) draw counter | CPU ms/draw, draw/mesh/instance 분모, VS/PS, 외부 CPU sampling의 driver stack | draw 감소와 LOD를 별도 A/B한다. GPU ms가 낮아져도 CPU 제출이 유지되면 batching 쪽을 계속 조사한다. |
| 07. instance upload·buffer growth, A | [MapStaticBatchObject.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapStaticBatchObject.cpp) `Ensure_InstanceCapacity`, `Upload_VisibleInstances`, `Upload_ShadowInstances`; 기존 상세 upload scope | **제안:** Map 시간, 업로드 bytes, realloc 수, capacity/high-water, 실제 dirty range | 정지 화면의 반복 upload 회피와 scratch 재사용 비교. camera/instance 변동 시 fresh data가 반드시 반영되어야 한다. |
| 08. fallback object·가상 호출 순회, B | [MapAssetObject.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapAssetObject.cpp) `Late_Update/Render_Group`; 기존 fallback·render submission counter | fallback 이유, 후보/실제 draw 수, object별 mesh 수, NS/object sampling | batch 불가 원인을 먼저 분리한다. source 재질·움직임 계약을 유지하는 범위의 batch 편입만 비교한다. |
| 09. shadow cache, B | [Renderer.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Renderer.cpp) `Render_Shadow`; 이번 `CacheAdmission/StaticBuild/CacheCopy/Dynamic`, hit/miss·caster counter | cache hit ratio, rebuild 이유, static/dynamic caster, copy bytes, 실제 GPU VS | 정지 카메라/움직이는 캐릭터/빛 이동을 분리한다. 이번 베른 표본은 shadow VS=0이므로 별도 실제 shadow draw 시나리오가 필요하다. |
| 10. shadow 품질·범위, B | [Shadow.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Shadow.cpp), [Renderer.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Renderer.cpp) | map resolution, light bounds, caster draw/indices, GPU depth 시간·cache memory | 해상도/범위 변경은 품질 실험으로 따로 표시한다. bake/캐시/동적 shadow를 구분하고 사용자 화면 승인 없이 기본값을 낮추지 않는다. |
| 11. light record 구성·전송, B | [Light_Manager.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Light_Manager.cpp) `Render_Lights`; 이번 `StageAndSubmit/UploadAndDraw`, records/draw/upload bytes | receiver pass별 light record 중복, local/directional/transient 수, CPU ms/record | 동일 light 입력에서 stage 재사용·정렬 후보를 비교한다. counter의 records는 고유 광원 수가 아니라 receiver pass 제출 기록 수다. |
| 12. light 계산·영향 범위, B | [MapLightPresentationRuntime.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapLightPresentationRuntime.cpp) `Submit_Presentation`, Renderer `Render.Lights` GPU/PS/VS | **제안:** candidate/admitted light, 화면 영향 면적, tile/volume별 light overlap | 동일 조명 결과를 유지하는 보수적 light cull 후보. tiled/clustered lighting은 이후 별도 설계이며 현재 적용 완료가 아니다. |
| 13. prebaked light·source material, B | [Material.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Material.cpp) `Bind_SurfaceLighting/Bind_StaticShadow`, [Shader_Deferred.hlsl](C:/Users/user/Desktop/LostArk/Engine/Bin/ShaderFiles/Shader_Deferred.hlsl) | **제안:** lightmap/IBL texture size·mip·샘플 수, material program별 GPU 비용 | baked 입력을 끄는 것은 원인 분리용 실험이다. static/dynamic 수광 계약을 유지하며 실제 runtime 연산·메모리 절감량을 비교한다. |
| 14. 투명·masked·overdraw, B | Renderer `Render.Blend`, [MapAssetObject.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapAssetObject.cpp) blend 정렬, effect renderer | PS/visible pixel, 투명 draw·화면 면적·sort 시간, alpha cutout coverage | 동일 effect 수에서 해상도 변경·분리 pass로 fill 비용을 확인한다. 투명 draw의 무조건 material 정렬은 화면 순서를 깨뜨릴 수 있다. |
| 15. SSAO·Bloom·final post, B | [Renderer.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Renderer.cpp) `Render.SSAO/Bloom/Final/ScreenPosts` | pass별 GPU ms, resolution, 샘플/반복 횟수·RT bytes | 한 pass씩 원인 분리 후 동일 품질 구현 비교. 끄거나 저해상도로 바꾼 결과는 품질 변경으로 기록한다. |
| 16. scene color copy·RT bandwidth, B | [Renderer.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Renderer.cpp) `Render.SceneColorCopy`, copy count/bytes | copy 소비자 수, duplicate copy, format·resolution, GPU elapsed | 동일 version의 scene color를 함께 쓰는 consumer만 copy 공유한다. refraction 시점이 다르면 합치지 않는다. |
| 17. Present·frame cap·OS 대기, B | [Graphic_Device.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Graphic_Device.cpp) `Render.Present`, [MainApp.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MainApp.cpp) `Limit_FrameRate` | Present CPU, frame interval, foreground/minimized, cap/VSync, queue latency | uncapped 기준과 제품 cap 조건을 따로 측정한다. cap sleep은 profiled CPU frame 밖이므로 CPU frame만으로 FPS를 계산하지 않는다. |

## G04. Animation·Effect·Object·UI에서 조사할 후보

| 후보·우선순위 | 실제 위치와 현재 계측 | 추가로 기록할 수치 / 재현 workload | A/B 및 채택 조건 |
|---|---|---|---|
| 18. animation 채널·본 결합, B | [Animation.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Animation.cpp) `Animation.Channels.Update/Sample`, [Model.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Model.cpp) `Animation.Play/Blend/Bones.Combine` | models·bones·channels/keyframes, ns/bone, sampling 횟수·clip cache hit | 동일 pose/time 입력의 hot array·키 검색 cache·batch 후보를 비교한다. 베른 선택 구간은 약 52개 모델과 3.7–3.9ms animation을 관측했다. |
| 19. 보이지 않는 animation·갱신 주기, B | Model `Begin/End_ModelAnimation` 연동과 submitted model 집계, [Character.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Character.cpp) | updated/not-submitted, camera visibility, socket/cue 소비 여부, cadence별 오차 | visual pose만 cadence 조절할 수 있는 대상부터 분리한다. Server action clock·notify·socket·gameplay를 화면 밖이라는 이유로 중단하지 않는다. |
| 20. skin palette·animation history, B | [Model.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Model.cpp) `SkinPalette.Build/Bind`, `Animation.History.Sample` | palette build 횟수/bytes, mesh 간 동일 pose 재사용, history count·sample cost | 같은 pose/mesh 반복의 palette 재사용, 필요한 history만 저장하는 후보. trail·rewind·socket 결과를 함께 검증한다. |
| 21. particle spawn·수명·메모리 pool, B | [Effect_Playback.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Effect_Playback.cpp) `Effect.Particle.Spawn/Update`; `vector<PARTICLE_STATE>` | **제안:** alive/spawn/death/capacity/growth, allocation/free bytes, pool hit/miss/high-water | 같은 seed·spawn/death 순서로 reserve·free list·hot/cold data 후보 비교. thread pool과 particle object/memory pool은 서로 다른 기능이다. |
| 22. particle module·고정 step, B | Effect playback `Effect.Playback.FixedStep/FrameRebuild`, `Update_Particles` | emitter별 particles×module 수, substep count, catch-up time·배제 이유 | 1/4/16 emitter와 적은/많은 particle workload. module dispatch 정리·constant cache·SoA 후보를 fixed-step 결과와 함께 비교한다. |
| 23. particle worker, B | [Effect_ParticleUpdatePool.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Effect_ParticleUpdatePool.cpp) `Run_EffectParticleUpdates`, 기존 `Worker/Join` | **제안:** enqueue→start, submission mutex wait, main/worker work, batch wall, jobs/lane·불균형 | 현재 main+보조 1 lane이 존재한다. serial과 worker를 같은 입력으로 비교하며 작은 batch는 serial 유지한다. 상세 조건은 G07. |
| 24. sprite/mesh instance 구성·sort, B | [Effect_DocumentRenderer_Particles.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_Particles.cpp) `Effect.Sprite.DepthSort/InstanceBuild/InstanceUpload/DrawSubmission`, mesh build/upload | particle 수·sort key·instance bytes·buffer growth·draw split 이유 | 동일 depth order에서 scratch 재사용·sort 생략 조건·합법적 instancing을 비교한다. alpha blending 순서 보존이 필수다. |
| 25. effect occurrence·준비·prewarm, B | [Effect_PresentationService.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Effect_PresentationService.cpp) `Effect.Prepare.*`, `Prewarm.Advance`; playback occurrence scopes | active/pending occurrence·spawn queue depth, 준비 cache hit·prewarm steps, runtime cold load | 처음 스킬과 반복 스킬을 분리한다. resource 준비를 공통 경로로 앞당기되 first-use hitch를 숨기려고 steady-state 결과에서만 제외하지 않는다. |
| 26. trail·afterimage·부착점, B | Effect playback와 [Effect_DocumentRenderer_Particles.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_Particles.cpp), 기존 trail/anchor scope | source bone 수, history points·triangles, CPU sampling·upload bytes | 같은 source curve·bone/time으로 cache·구조 변경을 비교한다. finite/count 통과를 실제 부착과 GPU 표시 성공으로 대신하지 않는다. |
| 27. Object/Layer 전체 순회, B | [Object_Manager.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Object_Manager.cpp), [Layer.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Layer.cpp); 기존 `Engine.ObjectUpdate/LateUpdate` | **제안:** phase별 방문/활성/실제 작업 object 수, type별 sample, virtual call·pointer miss sampling | no-op object의 phase 등록 제외와 type별 연속 작업 후보를 측정한다. 기존 Prototype/Clone/Layer 계약 안에서 hot loop를 개선한다. |
| 28. PhysX simulate·fetch wait, C | [Physics_Manager.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Physics_Manager.cpp) `Update`, `simulate/fetchResults(true)`; 기존 상위 `Engine.Physics` | **제안:** simulate/fetch 분리, actors/awake actors, substeps, query 수, step backlog | 같은 actor 상태에서 worker 수·sleeping·query cache 후보. Server combat 판정을 Client PhysX로 이동하지 않는다. |
| 29. 제품 UI·text·portrait, B | [UILayoutRuntime.cpp](C:/Users/user/Desktop/LostArk/Client/Private/UILayoutRuntime.cpp), [UI_Sprite.cpp](C:/Users/user/Desktop/LostArk/Client/Private/UI_Sprite.cpp), [CharacterPortraitRenderer.cpp](C:/Users/user/Desktop/LostArk/Client/Private/CharacterPortraitRenderer.cpp); 기존 `Render.UI/UIText/Portraits` | **제안:** visible widget·glyph·dirty layout, text/portrait cache hit, texture switch·draw 수 | 변하지 않는 rect/text/portrait의 dirty 갱신 후보. ImGui profiler 비용과 제품 UI 비용은 분리한다. |
| 30. ImGui와 Profiler 자기 비용, A | [ImGuiLayer.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/ImGuiLayer.cpp), [ProfilerTool.cpp](C:/Users/user/Desktop/LostArk/Client/Private/ProfilerTool.cpp); 기존 Build/Submit/Refresh·viewport/upload counter | capture off / capture on 창 숨김 / 창 열림 / 상세 on 네 조건, scope drop·QPC/lock sampling | 동일 화면에서 low-overhead 계측 비용을 먼저 구한다. 모든 draw를 세분화해 frame을 더 느리게 만드는 profiler 확장은 피한다. |

## G05. Navigation·Server·Network·로드와 메모리에서 조사할 후보

| 후보·우선순위 | 실제 위치와 현재 계측 | 추가로 기록할 수치 / 재현 workload | A/B 및 채택 조건 |
|---|---|---|---|
| 31. Client navigation, B | [NavPathFollower.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/NavPathFollower.cpp), [Navigation.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Navigation.cpp), [PathFinder.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/PathFinder.cpp); 기존 Request/AStar/Simplify/RoundCorners/Follow·query/expanded counter | query 결과, grid 크기/visited 비율, scratch bytes, repath 원인·동일 goal 중복 | 짧은/긴/불가능한 경로, 이동 spam, 장애물 모서리. Client prediction과 Server authoritative navigation 시간을 별도로 기록한다. |
| 32. Server A* scratch 초기화, A | [ServerNavigation.cpp](C:/Users/user/Desktop/LostArk/Server/Private/ServerNavigation.cpp) `Find_Path`; 기존 calls/total/max/expanded/path points | grid 크기와 query당 전체 초기화 bytes, touched cells, allocation count | 이번 소스에 generation-stamped per-thread scratch 재사용을 반영했다. 동일 path/expanded 결과·reload·invalid 경계를 확인했으며 실제 room tick 이득은 별도 측정한다. |
| 33. Server projection·LOS·smooth, B | ServerNavigation `ProjectPoint/TraversalStep/LineOfSight/FindPath/SmoothPath/ReachablePath` metrics | LOS calls/visited cells, path before/after, 실패 원인, 반복 projection, parent/child self | A*만 빨라져도 smooth/LOS가 남을 수 있다. 동일 장애물·support surface·경로 결과로 단계별 비교하며 inclusive를 중복 합산하지 않는다. |
| 34. Server tick·commands·AI·combat, B | [GameRoom.cpp](C:/Users/user/Desktop/LostArk/Server/Private/GameRoom.cpp) `Tick`, [GameRoom_PlayerSimulation.cpp](C:/Users/user/Desktop/LostArk/Server/Private/GameRoom_PlayerSimulation.cpp), [GameRoom_BossSimulation.cpp](C:/Users/user/Desktop/LostArk/Server/Private/GameRoom_BossSimulation.cpp) | 기존 tick last/max·ingress·scheduler. **제안:** command/player/trigger/world/boss/combat phase 분포, overlap candidate/accepted·oldest command age | 동일 command trace로 1/4 player, monster 수 증가, boss pattern 재현. authority·fixed tick·event order·private room 격리를 유지한다. |
| 35. snapshot 구축·직렬화·broadcast, B | [GameRoom_Replication.cpp](C:/Users/user/Desktop/LostArk/Server/Private/GameRoom_Replication.cpp) `Broadcast_WorldSnapshot`; 기존 encode/enqueue count·last/max·실패 | **제안:** build/encode/enqueue 분리, entity/event count, packet bytes/recipient, allocation | 같은 tick 결과에서 buffer 재사용·중복 encode 제거 후보. 이벤트·reliable lifecycle을 버려 byte 수만 줄이면 실패다. |
| 36. network queue·send·Client apply, B | [ClientSession.cpp](C:/Users/user/Desktop/LostArk/Server/Private/ClientSession.cpp), [NetworkManager.cpp](C:/Users/user/Desktop/LostArk/Client/Private/NetworkManager.cpp), [ClientReplication.cpp](C:/Users/user/Desktop/LostArk/Client/Private/ClientReplication.cpp) | 기존 queue/high-water/send last/max, `Network.DrainAndDispatch`. **제안:** enqueue→send, recv→apply, type별 parse time/bytes, oldest age·coalesced/drop delta | CPU 부하와 느린 수신자를 나눈다. RTT/OS 대기와 serialization CPU를 분리하고 typed failure·순서 보존을 검증한다. |
| 37. Loader·asset worker·동기 대기, C | [Loader.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Loader.cpp) `Loader.Map.*`, [Model.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Model.cpp) `Model.Load.*`, material worker/join | bytes/asset, queue→start, decode/create/upload/commit, worker active count, cache hit·IO wait | cold/warm load, 첫 입장/재입장, source revision 변화. GPU resource commit과 일반 파일 decode를 분리하고 cooperative cancel/rollback을 유지한다. |
| 38. texture cache·VRAM·shader cold build, C | [Material.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Material.cpp) `Texture.Cache.Lookup/SameKeyWait/Load.FileAndUpload`, [Shader.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Shader.cpp) build scopes, [UITextureCache.cpp](C:/Users/user/Desktop/LostArk/Client/Private/UITextureCache.cpp) | **제안:** hit/miss·unique SRV·estimated bytes·resident budget·eviction, program build/cache hit | 현재 texture counter 일부는 enum만 있고 UI는 N/A다. 0을 실제 hit/memory 0으로 해석하지 않는다. duplicate resource와 cold compilation부터 분리한다. |
| 39. heap·container·lock·cache locality, B | particle vectors, map pack scratch, navigation scratch, Profiler `m_Mutex`, Model material cache | **제안:** allocation/free 수·bytes, capacity growth, lock wait/hold, context switch, ns/item·cache miss 외부 sampling | pool/reserve/hot-cold/SoA는 해당 hot loop 하나씩 비교한다. 전체 객체를 바꾸기 전에 낭비 초기화·복사·중복 검색 제거를 우선한다. |
| 40. sound·진단 로그·저장 hitch, C | [Sound_Manager.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Sound/Sound_Manager.cpp), [ProfilerCaptureIO.cpp](C:/Users/user/Desktop/LostArk/Client/Private/ProfilerCaptureIO.cpp), Server diagnostic writer | **제안:** sound update/voice 수·first load, log write bytes/flush, snapshot copy/save wall·memory peak | 동일 audio/event 입력에서 lazy load·로그 폭주를 분리한다. 일상 frame과 저장 frame을 모두 보존하되 별도 시나리오로 보고한다. |

이 표는 실제 소스에서 확인한 **40개 조사 후보**다. 모든 후보가 이미 병목으로 판명됐거나 해결됐다는 목록이 아니다.
이번 자료로 먼저 확정된 것은 베른 줌아웃의 대량 NonBlend 제출과 profiler 표본 누락이다.
그 외 항목은 해당 workload에서 위 분모를 붙여 우선순위를 다시 정한다.

Server의 별도 성능 자료는 [ServerApp.cpp](C:/Users/user/Desktop/LostArk/Server/Private/ServerApp.cpp)의
`Tick_GameplaySimulations`와 RoomPerf writer로 수집된다. 현재 실행 파일 옆
`Diagnostics/server-room-perf-<pid>.log`에 bounded log를 남기고 GameRoom은 300tick마다 진단을 평가한다.
Client JSON의 navigation이 0이어도 remote Server의 navigation이 0이라는 뜻은 아니다.
Client frame 번호와 Server tick은 같은 시계가 아니므로 UTC·world/session·command sequence를 연결해 분석한다.
서로 다른 PC의 timestamp를 빼서 network latency를 구하려면 clock offset도 별도로 다뤄야 한다.

## G06. 이번 공통 수정과 아직 채택하지 않은 구조 변경

| 구분 | 현재 범위 | 완료 판정에서 아직 필요한 것 |
|---|---|---|
| 이번 소스 변경 | F7 Debug/Release 공유 Profiler, 상세 CPU scope 선택, frame별 누락·metadata·분포·작업량 | Debug/Release compile과 focused 검사 결과는 RESULT. 사용자 창 열기·숨기기·저장·JSON 재분석 필요 |
| 이번 소스 변경 | CPU에 이미 있는 projected error로 LOD range 선택 후 직접 indexed instanced draw. 기존 안전한 material admission과 품질 한계 유지 | 동일 선택·invalid fallback 자동 검증, 줌 구간별 실제 LOD 선택률·GPU/CPU 변화, 사용자 화면 회귀 검사 |
| 이번 소스 변경 | shadow cache/caster와 light stage/upload/draw 계측 | 실제 shadow raster draw가 있는 장면, 다양한 light 수의 고정 장면에서 counter 의미 확인 |
| 이번 소스 변경·수치 검증 | map instanced binder의 불필요한 character reset 5개와 per-instance와 중복된 global baked transform 요청 4개 생략 | 실제 shader 입력/출력과 기본 OBJECT 요청 trace 검증 통과. 제품 FPS와 사용자 화면은 새 캡처로 확인 |
| 이번 소스 변경·기하 검증 | deferred POINT/SPOT의 영향 sphere가 camera clip volume 전체 밖이면 해당 pass 제출 생략. 방향광·원본 목록·shadow 경로 유지 | 실제 CPP Debug/Release compile 및 76,650개 visible receiver 보존 검사 통과. 실제 게임 광원 수·GPU PS·frame p95는 새 캡처로 확인 |
| 이번 소스 변경·함수 검증 | Server A* scratch를 per-thread generation 방식으로 재사용하여 query마다 전체 grid 초기화 제거 | Debug/Release 실제 함수·경계 비교 통과. Product build와 실제 room tick·Client FPS는 별도 검증이며 이 함수 수치로 환산하지 않음 |
| 아직 제안 | 더 큰 batch 통합, material/state cache, animation cadence, particle memory pool/SoA, 추가 worker, tiled lighting, occlusion | 위 40개 후보의 측정으로 이득과 동일 결과를 확인한 뒤 개별 변경으로 채택 |

Level별 예외 분기를 넣어 숫자를 맞추는 방식은 사용하지 않는다. 공통 수정의 회귀 세트는 다음처럼 정한다.

| 장면 | 고정 workload | 공통 수정에서 확인할 것 |
|---|---|---|
| 베른 | 일반/풀줌아웃/바다/바닥, 동일 camera 정지 후 이동 | candidate/visible, batch/draw, LOD/indices, opaque CPU/GPU |
| 발탄 | 빈 아레나/동일 boss pattern/4인 effect | shadow cache, dynamic caster, animation/particle, Server combat·snapshot |
| Character Select | 같은 class·위치, class 변경 전후와 idle | 공통 map/material/LOD, background model animation, class 변경 resource hitch |
| 쿠크 | 같은 gate·pattern·카메라·effect 수 | map opaque, transparent overdraw, screen post/copy, pool·worker, network apply |

발탄·Character Select·쿠크는 이번 베른 JSON만으로 병목을 확정할 수 없다.
공통 경로의 결함 수정은 같이 적용하되 개선율은 각 장면의 새 캡처가 있어야 기록한다.

Server navigation은 실제 게시된 9개 grid의 동일 query를 변경 전/후 함수에 넣어 검증했다.
구성별 sequential 837개와 별도 thread 720개 path/expanded/smooth 비교, blocker·void·reload와
generation wrap 검사를 통과했다. 아래는 짧은 성공 경로 16개를 warmup 후 20회씩 7run 반복한
**query당 평균시간의 run 중앙값**이다. 최초 grid load와 scratch allocation은 제외했다.

| grid | Release 이전 → 이후 | Debug 이전 → 이후 |
|---|---:|---:|
| Bern, 319,200 cells | 735.168 → 5.749 µs/query | 4,582.770 → 47.227 µs/query |
| Bern2, 287,235 cells | 660.374 → 7.030 µs/query | 4,096.460 → 46.240 µs/query |
| Valtan, 122,304 cells | 76.843 → 4.620 µs/query | 1,402.250 → 55.428 µs/query |
| Character Select, 3,844 cells | 13.585 → 12.020 µs/query | 117.206 → 84.046 µs/query |
| Kouku StartFine, 1,216 cells | 8.494 → 9.654 µs/query | 69.072 → 50.705 µs/query |

큰 grid의 짧은 경로에서 매번 전체 배열을 초기화하던 비용이 줄었다.
작은 StartFine의 Release 중앙값은 오히려 1.161µs 증가했고 공유 개발 PC의 run 변동도 있다.
모든 경로 길이와 grid가 같은 배율로 빨라진다고 해석하지 않는다. scratch는 최대 grid 기준
12MB/thread를 유지하며 초기 성장 때 일시적 메모리 사용도 생긴다.
전체 조건·원본 run·실제 함수 compile 증거는
[NAV_OPTIMIZATION_RESULT](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/navigation/NAV_OPTIMIZATION_RESULT.md)에 있다.
이것은 함수 microbenchmark이며 베른의 NonBlend 306ms나 Client FPS가 해결됐다는 증거가 아니다.

map binder는 `INSTANCED`를 명시한 batch caller에서만 직접 raw-value 요청이 bind당 63 → 54회가 됐다.
기존 `CShader` 값 캐시 때문에 warm Effects11 `SetRawValue`는 8 → 8회, model/texture 호출은 6 → 6회다.
10 material family, 2,520개 경우에서 실제 MapInstance shader의 depth와 8 MRT를 비교해
85,155,840개 float의 최대 차이가 0이었다. 기본 `OBJECT`의 전체 ordered 요청 trace도 원본과 같았다.
이는 scalar Binary shader 전체의 화면 검증이나 실제 게임 FPS 측정을 대신하지 않는다.

실제 binder 추출 함수 20,000회, 7 ABBA round의 14 sample 중앙값은 `/Od` 80.2709 → 78.0567ms,
`/O2` 46.1146 → 42.7245ms였다. 이 두 하네스는 Debug CRT 및 `/O2` shader shim을 사용하므로
**제품 Debug/Release frame 수치가 아니다.** 실제 소스 소비자의 Debug/Release compile은 별도로 통과했다.
[조건·수치·hash receipt](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/batching/receipt.json),
[검증 범위](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/batching/README.md).

## G07. AoS·DOD·SoA·worker·fiber를 숫자로 비교하는 방법

AoS(Array of Structures)는 데이터 배열 형태이고 OOP는 코드의 책임과 호출 구조다.
`vector<PARTICLE_STATE>`를 가진 OOP 객체도 연속 AoS 데이터를 일괄 처리할 수 있다.
DOD는 실제 접근 패턴을 보고 필요한 데이터를 함께 처리하도록 설계하는 방식이며 SoA는 그 선택지 중 하나다.
모든 field를 한꺼번에 읽는 작은 record는 AoS가 유리할 수 있고, position/lifetime처럼 일부 field만 대량 처리하는
kernel은 SoA/hot-cold 분리가 유리할 수 있다. 변환·gather/scatter·작은 배열·분기·allocation 비용까지 포함해서 결정한다.

현재 particle 경로는 [Effect_Playback.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Effect_Playback.cpp)의
emitter별 particle vector를 사용하며, [Effect_ParticleUpdatePool.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Effect_ParticleUpdatePool.cpp)는
main thread와 보조 1 lane으로 독립 emitter를 처리한다. 현재 admission은 적어도 4 task와 512 particles이며,
portable source event/death-event generator 등 순서 의존 작업은 serial 경로를 유지한다.
spawn/provider 선택·event 처리 순서, emitter별 RNG, update 후 DirectLoc·collision 소비 순서를 보존해야 한다.

| 비교 variant | 동일하게 유지할 것 | 측정할 것 |
|---|---|---|
| 기존 AoS + serial | seed, fixed dt, emitter/module 입력, 생존·사망·event 순서 | batch wall, ns/particle, allocation bytes, 결과 hash/허용 수치 오차 |
| AoS + scratch/reserve·불필요 계산 제거 | 위 조건과 draw 입력 | kernel 향상과 메모리 절감. 구조 교체 전에 가장 작은 개선부터 확인 |
| hot/cold 또는 SoA/AoSoA + serial | 같은 작업량·결과, import/export 비용 포함 | SIMD 적용·cache miss·bytes/item, 전체 batch와 frame p95 |
| 동일 데이터 + worker 수 0/1/2/4 | 순서 독립 job만 분할, main 참여·join 포함 | queue/start 지연, jobs/lane, worker work sum, batch wall, join wait·CPU 점유 |
| fiber scheduler 후보 | 같은 worker 수·같은 dependency graph·동일 결과 | yield/resume/steal 수, stack memory, ready queue wait, critical-path idle 감소 |

variant별 함수만 따로 달리게 하지 말고 실제 소비자가 받는 동일 입력·출력까지 비교한다.
microbenchmark는 timer overhead보다 충분히 긴 반복을 하고, Release 최적화가 계산을 제거하지 않도록 결과를 소비한다.
실제 작은/중간/최대 workload와 빈·실패·불가능 경로도 포함한다. Debug/Release 결과는 별도 표로 남긴다.
전체 엔진 OOP와 DOD를 통째로 재작성해 비교하는 것보다 하나의 hot loop에서 요인을 분리하는 편이 인과관계를 확인하기 쉽다.

worker 병렬 구간의 판단식은 다음과 같다.

```text
Tserial = 동일 작업을 한 lane에서 수행한 wall time
Tparallel = setup + queue/start + 가장 늦게 끝난 lane의 시간 + join/commit
채택 조건 = Tparallel < Tserial, 결과 동일, frame p95/p99와 메모리·반응성 회귀 없음
```

worker의 CPU 시간을 전부 합한 값은 전체 일을 위해 사용한 자원량이다.
frame을 얼마나 빨리 끝냈는지는 가장 늦은 dependency가 결정하는 critical path로 판단한다.
GPU 제출 thread가 마지막 particle job을 기다린다면 해당 join이 중요하다. 반대로 GPU가 이미 오래 작업 중이고
worker가 그 안에 숨겨졌다면 worker kernel의 큰 개선이 FPS에 거의 반영되지 않을 수 있다.

아래 수치는 **실측 효과가 아닌 계산 예시**다.

| 가정 | serial | 계산한 parallel | 해석 |
|---|---:|---:|---|
| 병렬화 가능 작업 4 ms, 4 lane 균등, 추가 비용 0.4 ms | 4.000 ms | 1.400 ms | ideal 4배가 아니라 약 2.86배 |
| 같은 추가 비용에 작업 자체가 0.1 ms | 0.100 ms | 0.425 ms | 약 4.25배 느려짐. 작은 job을 만들수록 불리할 수 있음 |

frame 전체의 단순 민감도 계산은 `Tnew = T - P + P/s + overhead`다.
P는 전체 frame에서 실제 줄일 수 있는 비용이고 s는 그 부분의 배속이다.
CPU/GPU overlap이나 새로운 stall이 없다는 단순 가정이며, 실측 frame 예측식으로 무조건 쓰지 않는다.

베른 줌아웃의 관측 평균 `T=376.519 ms`, `Client.Update≈17.997 ms`를 P로 가정하면 다음과 같다.
이것은 Update 전체가 좋아진다는 매우 낙관적인 상한 예시이지, DOD나 worker의 예상 벤치마크 수치가 아니다.

| Update 가정 배속 | 계산 frame | 계산 FPS |
|---:|---:|---:|
| 현재 | 376.519 ms | 2.656 |
| 1.5배 | 370.520 ms | 2.699 |
| 2배 | 367.520 ms | 2.721 |
| 4배 | 363.021 ms | 2.755 |
| Update 비용을 전부 제거 | 358.522 ms | 2.789 |

그래서 지금 베른은 particle worker 수를 늘리는 것보다 306ms를 차지한 NonBlend 제출 경로를 먼저 확인한다.
draw batching으로 줄일 수 있는 CPU 구간과 LOD로 줄일 수 있는 geometry 구간도 각각 구분한다.

fiber는 application이 thread 위에서 직접 스케줄하는 실행 단위다. fiber 수를 늘려도 실행할 CPU core가 생기지 않는다.
대기 중인 job 대신 독립 job을 수행할 여지가 있을 때 스케줄링 효과를 비교할 수 있다.
[Microsoft Fibers 문서](https://learn.microsoft.com/en-us/windows/win32/procthread/fibers).
현재 Profiler의 CPU nesting은 thread-local stack이고 token은 시작 thread에서 끝내는 계약이다.
따라서 fiber migration을 도입한다면 yield/resume 구간의 scope·logical job ID·thread 이동을 추적하도록
계측부터 바꿔야 한다. 지금 scope를 다른 worker에서 그대로 종료시키면 올바른 시간표가 되지 않는다.

D3D11 draw를 기존 immediate context에 여러 worker가 동시에 제출하는 것은 이 실험의 구현 방법이 아니다.
deferred context는 thread별 command list 기록 경로이고 최종 playback은 immediate context에서 실행한다.
그 방식도 기록·state 재설정·command list 제출·driver 지원 비용을 포함해 비교해야 한다.
[Microsoft Immediate and Deferred Rendering](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-render-multi-thread-render).

## G08. Debug·Release 튜닝과 채택 기록

Debug의 checked iterator·검증·추가 도구 비용은 개발 체감의 별도 목표로 다룬다.
Debug에서 worker 수를 늘렸더니 느려졌다면 lock/checked-container/작은 job 비용을 먼저 측정한다.
오류 검사를 지워 Debug 숫자만 개선하기보다, 불필요한 container 작업과 매-frame allocation·중복 계산을 줄인다.
Release는 동일 소스·데이터의 최적화 build, debugger 없음, D3D debug layer 상태와 실제 compiler flags를 기록한다.
fast-math·전역 floating-point 정책·worker count 변경은 Server deterministic 결과·LOD 경계에 영향을 줄 수 있으므로
단일 성능 옵션으로 일괄 적용하지 않는다.

한 후보가 채택되려면 기록에 다음 항목이 함께 있어야 한다.

| 기록 | 필요한 내용 |
|---|---|
| 실행 조건 | source/data revision, Debug/Release, device/driver, resolution, camera, settings, seed, server 조건 |
| workload 동일성 | visible/candidate/particles/entities/queries/bytes 등 해당 단계의 분모, 실패·제외 이유 |
| 수치 | 3–5회 run별 mean/p50/p95/p99, frame 초과 수, scope drops, GPU 유효율, allocation/메모리 |
| 출력 동일성 | path/pose/event/state hash 또는 허용 오차, culling visible ID, 실패 rollback, 사용자 화면 판정 |
| 변경 효과 | kernel 개선 ms와 전체 frame/tick 개선 ms를 별도 기록, 품질 변경과 알고리즘 개선 구분 |
| 적용 범위 | 공통 소비자·영향 Level, 아직 측정하지 않은 장면과 unresolved case |

목표 60FPS는 frame budget 약 16.667ms, 30FPS는 33.333ms다.
평균만 맞추고 p95/p99 hitch를 남긴 결과는 안정적인 목표 달성이 아니다.
반대로 GPU가 유휴인 frame cap 구간의 낮은 CPU 사용률은 최적화 실패가 아니다.
이번 변경 이후 새 Debug/Release JSON에서 공통 경로별 비용과 실제 작업량을 다시 확인하고,
다음 변경은 가장 큰 **측정된 비용** 하나와 검증 가능한 동일 결과를 기준으로 선택한다.

## G09. SR_MinecraftDungeons의 1 draw를 베른에 적용할 수 있는가

**호환되는 정적 geometry를 합쳐 draw를 크게 줄이는 방향은 유효하다. 베른에는 culling과 batching을 함께 적용한다.**
`맵 전체 1 draw`를 목표 숫자로 고정하기보다, 보이지 않는 geometry를 버린 뒤 같은 state/material로 처리할 수 있는
구간을 충분히 큰 batch로 제출하는 것이 목표다. 아래의 SR 비교는 실제 로컬 코드를 읽은 결과이며,
베른의 spatial chunk merge·atlas/array 통합·GPU culling은 아직 구현 후보다.

### G09-01. SR의 실제 1 draw가 성립한 이유

[SR CBlockMgr.cpp](C:/Users/user/Desktop/SR_MinecraftDungeons/SR_Minecraft_Dungeons/Client/Code/CBlockMgr.cpp)의
`RebuildBatchMesh`는 `HasBlock(neighbor)`로 이웃 블록에 가려진 내부 면을 제외한다.
[SR CBatchBuffer.cpp](C:/Users/user/Desktop/SR_MinecraftDungeons/SR_Minecraft_Dungeons/Client/Code/CBatchBuffer.cpp)의
`Rebuild/WriteFace`는 남은 면의 정점을 월드 좌표로 굽고 block 종류에 맞는 atlas UV를 써서 한 VB/IB에 넣는다.
`CBlockMgr::Render_Stage`는 identity world, 공통 raster/depth 상태, texture 0을 설정한 다음 그 buffer를 한 번 그린다.
최종 [SR CVIBuffer.cpp](C:/Users/user/Desktop/SR_MinecraftDungeons/SR_Minecraft_Dungeons/Engine/Code/CVIBuffer.cpp)의
`Render_Buffer`에 D3D9 `DrawIndexedPrimitive` 한 번이 있다.

따라서 **블록 stage의 여러 블록을 한 draw로 제출했다는 기억은 코드와 일치한다.**
이 경로는 geometry merge와 texture atlas를 함께 쓴 구현이다. 호출마다 같은 mesh를 반복하는 instancing과는
다른 방식이고, 내부 면 제거도 이미 같이 수행했다. UI·캐릭터·물·다른 pass까지 전체 frame이 1 draw였다는
의미는 아니다. 확인한 `Render_Stage`는 atlas의 mip filter를 끄고 있으며, 그 설정을 베른에 그대로 옮기면
먼 거리 texture 품질과 aliasing 조건까지 달라진다.

### G09-02. 현재 베른의 batch는 무엇을 묶는가

[MapPlacementRuntime.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapPlacementRuntime.cpp)의
`Stage_PlacementRuntime`에서 현재 batch key는 정확히 `{assetId, mirrored}`다.
공간 chunk별 key는 아직 없다. `mirrored`는 signed scale 곱이 음수인 경우를 분리한다.
`Is_BatchEligible`는 deferred 경로와 호환되는 asset만 받아들이며 source character family,
override render mode·cull mode가 달라지는 항목은 이 instance 경로에 강제로 넣지 않는다.

[MapStaticBatchObject.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapStaticBatchObject.cpp)의 흐름은
`Upload_VisibleInstances → model의 mesh loop → Bind_Material → Shader::Begin → Render_Instanced`다.
같은 asset의 같은 mesh를 여러 transform으로 그리는 것은 묶이지만, 다른 mesh/material은 루프의 다른 draw다.
이미 culling은 batch bounds와 instance bounds를 보고 visible 목록을 만들며, camera/instance revision과
업로드 payload가 같으면 재사용한다. 이번에 바뀐 LOD도 이 공통 draw 경로에서 index range를 고른다.

현재 설치된 베른 mapset/material을 실제로 집계하면 placement 50,017개, 사용 asset ID 15,589개,
material row 23,153개, placement lighting row 49,047개다. 서로 다른 asset ID가 같은 geometry를 가리켜도
RNM average/directional lightmap과 static shadow texture·설정이 달라 draw를 합칠 수 없는 경우가 많다.
분석 결과는 [catalog_summary.json](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/batching/catalog_summary.json),
재현 코드는 [analyze_batch_keys.py](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/analyze_batch_keys.py)에 있다.

| 분류 기준 | 구조상 group 수 | 의미와 적용 상태 |
|---|---:|---|
| 현재 catalog eligibility + asset ID + mirror | 16,422 | 설치 데이터로 계산한 후보. 기존 캡처 batch 16,421과 1개 차이는 bounds/runtime 조건 등 추가 확인이 필요하며 억지로 일치시키지 않음 |
| geometry + 전체 material/profile + mirror 완전 일치 | 16,247 | 175개 차이는 저작 visibility가 모두 false인 CUL_BOX helper 중복군. visible draw 개선 근거가 아님 |
| geometry + mirror만 비교 | 1,647 | material 차이를 무시한 하한 탐색이며 이대로 병합하면 잘못된 lighting/재질을 사용함 |
| baked lighting 전체를 무시하고 나머지 material 비교 | 1,894 | group을 나누는 주원인이 baked lighting이라는 구조적 단서. lighting을 삭제하자는 제안이 아님 |
| baked texture ID만 array layer로 대체한다고 가정, scalar/shadow 정책 유지 | 3,008 | texture dimensions/format/mip별 bank 분할 전의 가상 호환군. 실제 array 구축·shader 구현·GPU 검증은 아직 없음 |

이 표는 **저작/설치 데이터의 구조상 그룹 수**이고 화면의 visible draw 수나 성능 개선율이 아니다.
대상 baked texture ID는 6,812개, 서로 다른 lighting texture+shadow 조합은 3,534개다.
따라서 asset ID 문자열만 geometry ID로 바꾸는 수정의 이득은 제한적이다.
실질적인 batch 확장은 lightmap/texture bank와 instance별 slice 입력을 보존하는 계약까지 조사해야 한다.
`CUL_BOX`라는 이름 자체로 가시성을 판정한 것이 아니라 집계 대상 placement의 실제 visible 값을 확인했다.

D3D11 `DrawIndexedInstanced`는 한 번 호출할 때 index range와 instance 수를 하나씩 받는다.
즉 현재 `CMesh`의 index range 하나를 여러 instance로 재사용하는 기능이다. 서로 다른 model의 vertex/index
범위와 서로 다른 shader state를 함수 하나가 자동으로 합쳐 주는 기능은 아니다.
[Microsoft DrawIndexedInstanced](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-drawindexedinstanced).

### G09-03. 하나로 합칠 수 있는 것과 먼저 통일해야 할 것

서로 다른 정적 mesh도 vertex/index를 공통 buffer에 재배치하고 transform을 미리 적용하면
**호환되는 opaque pass 일부를 한 indexed draw로 만드는 것은 가능하다.**
모든 source material을 같은 draw로 처리하려면 geometry merge 외에도 다음 조건을 해결해야 한다.

| 조건 | 베른에서 필요한 작업 | 실패하거나 느려질 수 있는 이유 |
|---|---|---|
| vertex layout·topology·index 주소 | 공통 vertex 형식과 index offset, 필요 시 32-bit index, transform/normal/tangent와 mirrored 방향 보존 | offset 오류·음수 scale·normal basis 손실, geometry 복제로 메모리 증가 |
| shader program·상수 | 같은 프로그램끼리 묶거나 vertex/instance material ID로 읽는 material table 설계 | 서로 다른 source program을 큰 통합 shader로 만들면 분기·register·texture fetch 비용 증가 |
| diffuse/normal/ORM/lightmap 등 texture | 호환 texture를 atlas 또는 array로 묶고 UV/slice·sampler·색 공간 계약 연결 | atlas mip bleeding·wrap/tiling·padding·UV derivative, array의 크기/format 통일 비용 |
| raster/depth/blend state | 같은 cull/depth/blend 조건의 draw로 나누기 | texture만 합쳐도 양면/단면·투명/불투명·depth-write 차이는 해결되지 않음 |
| source baked lighting | per-placement lightmap/static-shadow scale/bias와 source texture 참조 보존 | 월드 geometry만 합치고 placement별 lighting 입력을 버리면 색·그림자 회귀 |
| pass 소비자 | opaque, shadow, transparency, picking 등의 실제 소비 범위 분리 | opaque 1 draw가 shadow·후처리·UI까지 전체 frame 1 draw가 되는 것은 아님 |
| editor·runtime 변경 | placement stable ID→merged range 대응, visibility·suppression·수정·파괴 때 dirty rebuild | 전체 buffer 재생성 hitch, picking/저작 ID 상실, stale bounds |

Texture2DArray는 같은 array 안의 format·dimensions·mip 수가 같아야 한다.
다양한 크기의 source texture를 가장 큰 크기로 일괄 맞추면 메모리를 낭비할 수 있어 format/size군을 나누는
설계가 필요하다. atlas와 array의 선택 자체가 현재 재질을 통합할 수 있다는 증거는 아니다.
[Microsoft Texture Types](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-textures-intro).
D3D11 PS SRV는 최대 128 slot이라는 제약도 있으므로 맵의 모든 개별 texture를 그대로 무제한 bind하는 방법은 없다.
[Microsoft PSSetShaderResources](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-pssetshaderresources).

정적이라는 조건은 transform bake와 변경 빈도가 낮다는 이점이다.
재질 호환성·가시성·pass 차이까지 없애 주지는 않는다. 같은 고해상도 mesh를 여러 번 배치한 asset은
placement별 geometry를 전부 복제해 합치기보다 현재 instancing을 유지하는 편이 메모리에 유리할 수 있다.
반대로 반복이 적고 작은 mesh가 같은 재질 state를 공유하면 merge의 API 비용 절감 효과를 검토할 가치가 크다.

### G09-04. culling과 batching을 같이 사용하는 후보 흐름

```text
placement stable ID + source mesh/material + bounds
→ compatible pass/material state로 분류
→ 공간적으로 가까운 정적 항목을 chunk 후보로 묶기
→ chunk bounds culling
→ 필요할 때 chunk 내부 instance/submesh 정밀 culling
→ 동일 geometry 반복은 instancing, 작은 호환 geometry는 merge
→ 선택한 LOD의 visible 작업만 pass별 제출
```

이 흐름은 **후속 후보**다. 현재 key를 spatial chunk로 이미 바꿨다는 뜻은 아니다.
chunk 크기는 Level 이름으로 정하지 않고 bounds, triangle/vertex bytes, instance 수, material 호환성과
실제 camera workload에서 결정한다. 베른에서 검증한 공통 정책을 다른 map에도 같은 조건으로 적용한다.

| 방식 | 줄이는 비용 | 늘어날 수 있는 비용 / 적합한 경우 |
|---|---|---|
| 지금의 asset instancing + 정밀 culling | 반복 geometry 저장량과 instance별 draw, 불필요한 instance 제출 | asset/mesh/material 종류가 많으면 draw가 많이 남음 |
| 공간 chunk + 호환 material merge | 작은 mesh의 API·state 변경 수, chunk 단위 빠른 reject | chunk가 너무 작으면 draw 증가, 너무 크면 보이지 않는 geometry까지 제출 |
| 맵 전체 opaque merge | 이상적인 경우 opaque CPU draw 수를 크게 감소 | 전체 bounds가 늘 보이면 culling이 거의 작동하지 않음. 정밀 cull하려면 index 재구성·가시 목록 처리 비용이 별도로 필요 |
| GPU visible 목록 압축 + indirect | CPU cull/pack·가시 수 readback 회피, 큰 batch의 GPU 병렬 선별 | compute dispatch·UAV/SRV 전환·추가 buffer와 bandwidth, 작은 batch에서는 오히려 느려질 수 있음 |

한 거대한 mesh를 항상 제출한 뒤 vertex shader에서 화면 밖 항목을 치우는 것은 CPU에서 draw를 생략하는
것과 비용이 같지 않다. input fetch·vertex 처리 일부는 이미 실행된 뒤일 수 있다.
occlusion도 매 object 결과를 CPU가 동기 readback하면 stall이 생길 수 있어, 현재 병목을 확인하지 않고
프레임마다 blocking occlusion query를 추가하지 않는다. visibility 압축이나 지연된 conservative occlusion은
독립적인 후보로 측정한다.

chunk merge를 할 때 먼 작은 물체 하나 때문에 가까운 물체의 LOD가 낮아지거나,
가까운 큰 물체 하나 때문에 chunk 전체가 항상 LOD0가 되지 않는지도 본다.
이번 LOD는 **geometry량 감소**, merge는 **draw/state 수 감소**, culling은 **불필요한 제출 제거**가
주요 목적이다. 세 작업은 서로를 대체하지 않는다.

### G09-05. GPU instancing과 indirect가 API overhead를 해결하는 범위

현재 map `DrawIndexedInstanced`도 GPU에서 instance를 처리한다. 여기서 GPU-driven으로 더 확장한다는 것은
가시 목록·LOD·instance count 같은 draw 입력의 생성을 GPU에 옮기는 것이다.
컴퓨트에서 모든 draw의 인자를 만들었다고 CPU 호출 수가 자동으로 1이 되지는 않는다.

D3D11 `DrawIndexedInstancedIndirect`는 args buffer와 한 offset을 받는다. 그 위치의 인자는
`IndexCountPerInstance/InstanceCount/StartIndexLocation/BaseVertexLocation/StartInstanceLocation` 한 묶음이다.
표준 D3D11의 이 호출에는 여러 draw record를 한 번에 실행하는 count나 임의 pipeline state 변경 명령이 없다.
따라서 현재 API로 서로 다른 geometry/material group을 유지하면 보통 group마다 state 설정과 indirect 호출도 남는다.
이 판단은 API 인자 계약에 근거한 추론이며, GPU visible count 계산과 CPU readback 회피의 이득은 별도다.
[Microsoft DrawIndexedInstancedIndirect](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-id3d11devicecontext-drawindexedinstancedindirect),
[Indirect args 구조](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_draw_indexed_instanced_indirect_args).

여러 geometry를 한 호출로 처리하려면 merge 또는 shader가 공통 buffer에서 geometry/material을 찾아 읽는
구조까지 추가로 설계해야 한다. 그 경우에도 fixed-function state와 pass 경계, GPU의 추가 indirection 비용을
측정해야 한다. 이번 작업에서 제거한 draw별 LOD compute dispatch는 CPU에 이미 있는 소량 입력을 GPU에
다시 계산시키던 경로였다. 이 사실이 큰 visible 목록의 GPU culling까지 항상 불리하다는 뜻은 아니다.

### G09-06. 다음 구현을 고르는 계측 순서

1. 현재 low-overhead 캡처로 같은 일반/풀줌아웃 camera를 다시 기록한다. 기존 306.125ms CPU NonBlend에는
   상세 계측의 누락·간섭도 있으므로 이 값을 순수한 D3D11 API 비용이라고 단정하지 않는다.
2. CPU memory 안에서 asset/mesh/material/pass key와 instance 수를 집계한다. 1-instance draw 비율,
   같은 geometry 중복 asset ID, 동일 재질인데 분리된 mesh, state 변경 횟수, texture/CB bytes를 추가 조사한다.
   집계는 draw마다 JSON을 쓰지 않고 batch/frame 끝에서 누적한다.
3. atlas가 필요 없는 **동일 material/state의 작은 opaque mesh군**부터 하나의 bounded chunk 후보로 비교한다.
   source geometry·material·stable ID를 유지한 원본 A와 merged B를 만들어 compile/구조/화면·수치 검증한다.
4. 작은/중간/큰 chunk 후보에서 일반 시점·이동 카메라·풀줌아웃을 모두 측정한다.
   선택 기준은 draw 수 최소가 아니라 CPU submission, GPU VS/PS/elapsed, cull time, upload bytes,
   메모리와 전체 frame p95의 손익이다. 같은 카메라에서 원본 visible ID와 화면 결과를 함께 확인한다.
5. geometry/재질 통합 후에도 많은 instance의 cull/pack 시간이 남으면 큰 묶음의 GPU visible compaction을
   다음 A/B로 검토한다. CPU→GPU 이동으로 readback stall이나 작은 dispatch가 늘지 않는지 확인한다.
6. 후보가 이기면 공통 `CModel → CMaterial` 소비 경로·publisher·stable ID 계약으로 통합한다.
   별도 Bern 전용 모델 runtime을 추가하거나 원본 source 재질을 임의 단순화하지 않는다.

현재 자료는 draw 감소 실험의 우선순위가 높다는 근거다. 어떤 chunk 크기와 통합 재질군이 가장 빠른지,
최종 draw가 몇 개가 되는지, FPS가 얼마나 오르는지는 아직 측정 결과가 없다.
SR의 단일 atlas 블록 stage에서 검증된 원리를 출발점으로 사용하되 베른의 실제 재질·geometry·가시성 계약에
맞춰 **culling을 유지하면서 호환되는 draw를 묶는 것**을 다음 검증 단위로 삼는다.

## G10. 화면 밖 trigger effect·광원·그림자의 비용을 나누어 측정

화면 밖 effect의 emitter 중심이 보이지 않는다는 사실만으로 effect 전체를 멈출 수는 없다.
그 effect의 particle·mesh·trail이 화면 안으로 이어지거나, 광원 영향 범위와 그림자가 화면 안에
닿을 수 있다. trigger 활성·재생 시간·종료·Server gameplay와 Client의 draw 생략을 구분한다.
이 절은 현재 소스에서 확인한 광원·그림자 경계와 다음 측정 기준이다. 원본 베른 캡처만으로
화면 밖 trigger effect가 몇 ms를 차지했는지는 분리할 수 없다.

### G10-01. 광원은 emitter 위치가 아니라 영향 범위로 판단

| 현재 소스 | 실제 동작 | 안전한 개선·계측 후보 |
|---|---|---|
| [Effect_Object.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Effect_Object.cpp:1179) → [Presentation_Manager.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Presentation_Manager.cpp:355) | 평가된 effect light를 검증하고 transient 목록에 제출한다. 이 목록은 deferred와 forward 소비자가 함께 읽는다. | 목록 생성·생애는 유지하고 소비 pass에서 영향 범위를 판단한다. authored/active/submitted/culled light 수를 서로 다른 counter로 둔다. |
| [Light_Manager.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Light_Manager.cpp) | 기존에는 receiver/유효성만 검사한 뒤 scene+transient를 128-byte record로 pack하고 400개 단위로 반복 제출했다. 이번 변경은 pass당 최종 camera clip volume을 한 번 만들고 POINT/SPOT sphere 전체가 밖일 때 pack 전에 생략한다. | `LightCullingCandidates/LightCullingRejected`와 `LightRecords/LightUploadBytes/LightDrawCalls`를 비교한다. 후보/제외 건수는 재제출되는 receiver pass마다 누적되므로 unique light 수와 다르다. |
| [Shader_Deferred.hlsl](C:/Users/user/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_Deferred.hlsl:1835) | source-character local light는 range sphere를 감싼 AABB를 투영해 quad 외부를 clip한다. near plane에 걸치거나 입력이 불확실하면 전체 quad를 유지한다. ordinary light는 이 clip flag가 꺼져 있다. | 이미 GPU clip이 있는 source pass에서도 CPU 제출은 남는다. CPU cull과 ordinary local-light 화면 bounds 확장은 서로 다른 A/B로 측정한다. |
| [Renderer.cpp](C:/Users/user/Desktop/LostArk/Engine/Private/Renderer.cpp:1374) | 실제 보이는 source-character material row마다 light 목록을 다시 제출한다. | `unique active lights`와 `sum of submitted light records`를 구분한다. `visible source material rows × eligible lights` 증가가 CPU pack과 GPU PS 양쪽에 주는 비용을 기록한다. |

POINT의 보수적 bound는 `center = vPosition.xyz`, `radius = fRange`다. SPOT도 우선 같은 sphere를
사용하면 cone보다 느슨하지만 필요한 빛을 제거하지 않는다. shader의 local attenuation은 range 밖에서
0이 되며, 그 뒤 material 계산을 생략한다.
[attenuation 및 zero-energy 분기](C:/Users/user/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_Deferred.hlsl:678).
중심점만 frustum 검사하거나 카메라와 emitter 거리만 제한하면 화면 가장자리의 빛이 사라질 수 있다.

directional light는 무한 영향이므로 이 sphere cull에서 제외한다. camera가 sphere 안에 있거나
sphere가 near/side plane을 걸치면 유지한다. 행렬·bound가 유효하지 않을 때도 유지하는 fail-open이 필요하다.
clip plane 부호, 정규화, 경계 epsilon을 실제 camera 행렬 계약으로 검증한다. 이 최적화는 light의
업데이트나 transient 목록을 지우는 기능이 아니라 **현재 camera의 deferred 제출을 생략하는 기능**이다.
화면에 보이는 receiver를 비추는 화면 밖 광원은 계속 그려야 한다.

이번 구현은 6 plane을 double로 계산하고 5cm 여유와 float 행렬곱의 magnitude에 비례한 오차 여유를 둔다.
실제 CPP에서 추출한 helper로 perspective/orthographic/wide FOV, camera 내부, near·side·far 경계,
뒤쪽·큰 range·최대 1,000,000 좌표·NaN/Inf/singular/null 입력을 검사했다. Debug/Release 각각
76,991 assertion을 통과했고, 화면 안 receiver point를 포함한 76,650개 sphere를 모두 보존했다.
이는 기하 계약 검사다. 실제 light shader의 모든 pixel 비교나 Client 화면 판정은 별도다.
[light culling 검증 기록](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/light-culling/RESULT.md).

forward 소비자의 별도 한계도 있다. [MapAssetRenderUtils.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapAssetRenderUtils.cpp:22)의
forward light packet은 400개이고, optional receiver world sphere가 있으면 light sphere와의 교차로 거른다.
[Effect_DocumentRenderer_MaterialBinding.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_MaterialBinding.cpp:512)의
Guardian PBR 1122/1123은 80개 packet을 넘으면 실패한다. deferred의 반복 batch가 전체 light 수 제한을
없앴다는 사실을 이 forward 경로의 무제한 지원으로 설명하면 안 된다. 이 차이는 확인된 소스 경계이며,
실제 effect workload에서 상한 초과가 발생했다는 측정 근거는 아직 없다.

### G10-02. camera 밖 caster를 shadow에서 제거하지 않기

현재 map batch는 authored-visible caster를 shadow queue에 넣고, 최종 light의 view/projection을 기준으로
다시 거른다. camera visible 목록을 shadow 목록으로 재사용하지 않는다.
[MapStaticBatchObject::Late_Update](C:/Users/user/Desktop/LostArk/Client/Private/MapStaticBatchObject.cpp:229),
[Upload_ShadowInstances](C:/Users/user/Desktop/LostArk/Client/Private/MapStaticBatchObject.cpp:845),
[Intersects_ShadowCullSnapshot](C:/Users/user/Desktop/LostArk/Client/Private/MapAssetRenderUtils.cpp:478).

scalar map도 같은 light-volume 계약을 사용한다. morph 및 source program 43의 vertex displacement처럼
정적 model bounds를 벗어나는 경우는 해당 static-bound reject를 생략한다.
[MapAssetObject::Render_Shadow](C:/Users/user/Desktop/LostArk/Client/Private/MapAssetObject.cpp:281).
화면 밖 caster가 화면 안 receiver에 그림자를 만들 수 있으므로 effect나 mesh의 camera culling 결과를
shadow queue에 그대로 전파하면 안 된다. 기존 static-shadow cache hit와 light-volume reject를 먼저 측정하고,
추가 최적화는 보이는 receiver에 영향을 줄 수 있는 caster 범위를 보수적으로 유지해야 한다.

### G10-03. 같은 effect의 화면 안·경계·화면 밖 A/B

1. effect ID·seed·시간·카메라·조명·shadow 설정을 고정한다. 재생 중 camera만 움직이는 실험과
   camera 고정 후 effect 위치를 바꾸는 실험을 구분한다.
2. 화면 중앙, emitter만 화면 밖이지만 particle/trail은 보이는 경우, 광원 중심만 화면 밖인 경우,
   광원 sphere 전체가 화면 밖인 경우, camera가 광원 안인 경우, 화면 밖 caster의 그림자만 보이는 경우를 측정한다.
3. `Effect.Update/Playback/Evaluate`, bound 계산, particle simulation, CPU render prepare, upload bytes,
   instance/mesh draw, transient light records, light GPU elapsed/PS, shadow caster/draw/cache를 함께 기록한다.
   effect가 없거나 비활성인 프레임을 별도 baseline으로 둔다.
4. draw 생략과 simulation pause는 별도 변경으로 측정한다. draw만 생략하면 원본의 age·spawn·event를
   유지해야 한다. 장식용 ambient pause는 숨은 시간만큼 visual clock을 멈추는 정책이므로 연속 재생과
   위상이 달라질 수 있다. 다시 들어올 때 state·RNG를 보존하고 catch-up·재생성·과거 재연산이 없는지 검증한다.
5. 베른/발탄/캐릭터 선택/쿠크에서 같은 source family와 공통 consumer로 재검증한다.
   특정 Level 이름을 검사해 화면 밖 effect를 꺼 버리는 정책은 사용하지 않는다.

이 실험을 통과하기 전에는 화면 밖 effect의 CPU simulation 절감이나 광원 GPU 절감 비율을 추정치로
확정하지 않는다. `culled count`가 늘어도 bounds 계산·branch·cache 비용 때문에 전체 frame p95가
개선되지 않을 수 있으므로 최종 채택은 같은 조건의 Release 결과와 재진입 화면 검증으로 결정한다.

### G10-04. 이번 ambient pause의 소비 경계

[MapEffectPresentationRuntime.cpp](C:/Users/user/Desktop/LostArk/Client/Private/MapEffectPresentationRuntime.cpp:851)는
`LEVEL_ACTIVE && SOURCE_LOOP`에서만 `bAllowOffscreenPause`를 요청한다.
[Effect_PresentationService.cpp](C:/Users/user/Desktop/LostArk/Client/Private/Effect_PresentationService.cpp:5856)는
정적 visual sphere를 안전하게 구할 수 있고 character/boss/NPC anchor·외부 sample/provider가 없는 경우만
실제로 받아들인다. bound를 증명할 수 없는 source family는 기존 playback을 유지한다.
베른의 ambient 전체가 자동으로 이 최적화 대상이라는 뜻은 아니다.

admitted occurrence는 기존 Engine object 자동 update가 꺼진 채로 유지되며 자동 Late_Update 제출도 끈다.
service Update는 그 프레임의 delta를 보관하고, MainApp의 최종 camera 단계에서 visibility를 확인한 뒤
seek/advance와 render submission을 한 번만 수행한다. 첫 가시 프레임은 초기 seek만 수행한다.
숨은 동안에는 particle state를 삭제하거나 매 프레임 seek하지 않고 visual clock을 멈춘다.
재진입 시 마지막 state에서 현재 delta만 진행하므로 숨은 시간의 catch-up 비용이 없다.
이는 **장식용 ambient의 재생 위상 정책 변경**이며, 계속 재생한 원본과 절대 시간 위상이 같다는 주장은 하지 않는다.

world root가 바뀌면 이전 위치의 world-space particle이 남을 수 있으므로 그 occurrence의 static-bound
culling을 영구 해제한다. source loop 원본 목록, Server trigger, gameplay time, 외부 sample의 계약은 유지한다.
기존 draw-distance 바깥의 placement Stop/respawn 정책은 별도로 남아 있으므로 모든 거리에서 state가
계속 유지된다고 설명하지 않는다.

실제 service의 `Update/Submit_VisibleLevelPresentations/Update_WorldRoot`를 추출한 fake consumer
검사는 Debug/Release 각각 32 assertion을 통과했다. 121 hidden frames, 최초 seek, once-per-tick,
재진입·delta overwrite, root 변경 fail-open, unsupported normal/external fallback, owner·Level·실패 cleanup을
검사했다. 이 검사는 lifecycle 호출과 상태 소유권에 관한 것이며 실제 particle simulator·sphere 계산·GPU
출력 검증과는 별도다.
[독립 lifecycle 검토 기록](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/ambient-lifecycle/RESULT.md).


### G10-05. 이동 표식과 캡처 항목

Bern의 91 ambient SOURCE_LOOP는 draw distance 안에서는 카메라 뒤에서도 계산되었다.
Valtan 이동 표식 5개는 이미 외부 sample culling을 사용했다. 이번 변경은 표식의 8m sphere 위에
붙던 16/32m 여유를 0/0.5m로 줄이며 기존 history 재진입 경로를 유지한다. Bern에 같은 resident
이동 표식이 있다고 가정하지 않는다. 실제 draw-distance 정책의 Stop/respawn은 계속 적용된다.

| 새 측정값 | 해석 |
|---|---|
| `effectBoundsCandidates`, `effectBoundsCulled` | marker와 admitted ambient의 최종 camera sphere 검사/제외 횟수 |
| `effectAmbientSuspended`, `effectAmbientAdvanced` | 이번 frame에 계산을 건너뛴/진행한 admitted 환경 occurrence |
| `Effect.LevelPresentation.VisibleUpdate` | 최종 camera 단계의 검사·visible simulation·submit CPU 시간 |
| `effectMarkerSamples` | trigger marker 외부 sample 요청 수 |
| `effectMarkerHistoryRequests` | 최초 진입/재진입 history 요청 수. 내부 wrap·large-gap 재구축 전체 횟수는 아님 |
| `lightCullingCandidates`, `lightCullingRejected` | deferred receiver pass별 local light 후보/제외. unique authored light 수는 아님 |

동일 위치에서 camera를 회전해 suspended·culled 증가와 Effect.Playback/submit 감소가 같이 나타나는지
확인한다. 화면 가장자리의 분수·안개·광원, 표식의 진입/이탈 반복을 별도로 비교한다. count 개선과
CPU/GPU elapsed·p95/p99 개선을 분리하고 화면 판정을 함께 기록한다. source bound의 기하/particle
검증과 lifecycle fake-consumer 검증은 서로 다르며 실제 게임 FPS를 대신하지 않는다.

[marker 검증](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/marker_review/README.md).


최종 Bern admission은 11문서 중10개, 91placement 중85개다. mesh 혼합6개는 기존 재생이다.
실제 TRS의 bound 반경 중앙값24.266m/p9549.933m/최대84.969m이므로 화면 밖의85개가 항상 모두
제거된다는 뜻은 아니다. 실제 Debug playback의40root×900step 및 크기 순서12case에서
1,063,822 particle row가 analytic bound 안에 들어왔으며 미지원22case는 fail-open했다.
Release는 actual CPP `/O2 /MD` 최소 컴파일을 통과했다.
[정적 sprite bound 검증](C:/Users/user/Desktop/LostArk/out/BernProfiler20260922/bounds/BOUNDS_RESULT.md).
