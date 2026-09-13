# 맵·캐릭터 렌더링 성능 개선 결과

## G00. 현재 완료 상태

`codex/map-character-render-performance`, 시작 HEAD `a7205863`에서 맵 그림자 culling, 단순 shadow shader 및 캐릭터 shadow/장비 포즈 중복 준비를 수정했다. Debug Product의 Engine→Shared→Server→Client 빌드·링크·배포가 2026-09-12 16:34:50 KST에 완료됐다. 첫 작업 소스는 사용자 commit `aac4fbdd`에 포함됐다. runtime 데이터와 Resources payload는 변경하지 않았다.

첫 작업은 소스 수정·수치 검증·제품 빌드 및 사용자의 Character Select 130fps 관찰까지 확인됐다. 쿠크 후속 변경은 G06 이후에 별도로 기록하며, 쿠크 최종 FPS와 화면 검증은 아직 사용자 확인 전이다. Client/UI 실행·조작·화면 캡처는 수행하지 않았다.

## G01. 사용자가 저장한 최신 캡처

입력은 `Client/Bin/ProfilerCaptures/profiler_20260912_161101_142_frame390_8384_0.json`이다. 캐릭터 두 개는 실제 update/submission되며 사용자가 Effect Tool에서 추가로 띄운 조건이다. 총 390프레임 중 GPU valid는 386개이고 dropped scope/frame은 0이다. background Effect 준비가 끝난 frame 230 이후 열 프레임을 제외한 241–386, 146프레임을 안정 비교 구간으로 선택했다. 이는 분석상의 제외 구간이며 제품의 warmup 완료 신호가 아니다.

| 측정 | 안정 구간 평균 |
|---|---:|
| 실제 frame interval | 15.381ms, 약 65.0fps |
| CPU frame | 15.262ms |
| Client.Render inclusive | 10.879ms, CPU의 71.3% |
| 직접 계측된 맵 CPU self 합계 | 3.722ms |
| Map.Batch.Mesh.Submit self | 1.929ms / 98회 |
| 두 모델 animation | 1.232ms |
| SkinPalette.Build | 0.190ms / 6회 |
| GPU Shadow elapsed | 8.511ms |
| Shadow PS / VS invocation | 약 13.24M / 1.497M |

interval p95는 17.905ms다. inclusive/self 항목은 중첩되므로 위 표의 시간을 합산하지 않는다. Shadow GPU elapsed는 뒤에 제출되는 CPU NonBlend와 높은 상관(r=0.832)을 보여 순수 GPU shader 시간으로 확정하지 않았다. map visibility self는 0.006ms로 현재 고정 시점의 culling 계산 자체는 주요 병목이 아니다.

Alt+V에서 scene backdrop이 활성화되면 기존 `Effect_Object`→scene environment replacement 계약으로 맵 본체·그림자를 생략하고, 연출 카메라도 바뀔 수 있다. FPS 상승을 설명할 실제 경로지만 최신 캡처에는 effect playback이 없어 사용자 관찰의 정확한 연출과 A/B 비교는 미확인이다.

분석기와 상세 분모: `out/MapCharacterPerf20260912/analyze_capture.py`, `capture_summary.json`, `README.md`.

## G02. 맵 그림자 후보 제거

`CGameInstance::Get_ShadowLightTransform`이 CShadow의 실제 view/projection을 읽기 전용으로 제공한다. 맵 utility는 별도 light revision과 보수적인 world sphere 검사를 사용한다. scene camera visibility를 그림자에 재사용하지 않는다. frame provider가 light를 바꿀 수 있어 최종 Render_Shadow에서 후보를 준비하며, authored visibility와 castsShadow가 없는 batch는 queue에서 제외한다.

light 행렬 또는 placement/visibility가 바뀔 때만 후보를 다시 계산하고 같은 GPU payload이면 upload를 생략한다. invalid bounds/행렬은 기존 제출을 유지한다. 실패한 Map은 CPU payload/revision을 교체하지 않아 다음 호출에서 재시도한다. 실제 shadow rasterizer의 DepthClipEnable=true도 별도 D3D device에서 확인했다.

설치 WModel 정점 bounds와 같은 Area placement를 실제 새 predicate로 검사했다. authored-visible 후보 746→410, index-instance 상한 3,267,222→2,302,659(-29.5%), group 66→54, submesh draw 상한 104→87이다. 이 값은 per-material castsShadow 제외 전 후보 상한이며 제품의 실제 제출 counter 또는 FPS가 아니다.

실제 함수 본문의 `/Od /RTC1`, `/O2` 검사 각각 100,079개를 통과했다. WARP instance-buffer Map/readback, 두 번의 실패 주입 뒤 재시도와 이전 CPU/GPU payload 보존, light/visibility 변경·동일 cache·invalid 입력을 포함하며 D3D debug 오류는 0이다. 최종 CPP 두 파일도 focused Debug 컴파일했다. 증거는 `out/MapCharacterPerf20260912/culling/README.md`, `result.json`, `probe-Od.run.log`, `probe-O2.run.log`이다.

## G03. 맵 그림자의 불필요한 shader 단계

`Shader_VtxMeshMapInstance`의 기존 pass 0~17은 그대로 두고 18~20의 단순 alpha shadow, 21~23의 opaque depth-only pass를 추가했다. family 0~5는 tangent frame과 baked lighting을 준비하지 않는다. source material이 활성화된 opaque family 3/4/5만 null PS를 사용한다. source 설정을 끄면 legacy diffuse alpha를 검사하며, 복잡한 native family는 기존 12~14를 사용한다.

기존 pass와 새 pass를 같은 binder로 비교한 하드웨어·WARP 각각 1,296조건, 1,327,104 depth 값의 불일치는 0이다. alpha cutoff, UV 변환, tint, mirror, cull mode, source on/off와 오염된 직전 shader 상태를 포함했다. instanced null surface 경로도 추가 확인했다. 실제 CShader admission과 모든 새 pass input layout 생성에 성공했으며 D3D 오류는 0이다. fixture는 procedural triangle과 identity/mirror transform이므로 전체 장면 화면 검증을 대신하지 않는다.

하드웨어 pipeline query에서 family 3/4/5의 한 삼각형 PS 호출은 1,024→0, VS 호출은 3→3이었다. family 0~2의 alpha 검사 PS는 유지된다. compiled VS instruction slot은 63→15(simple)/13(opaque), simple PS는 142→51이다. instruction slot과 이 작은 fixture를 GPU frame 개선율로 환산하지 않는다.

증거: `out/MapCharacterPerf20260912/shadow/parity-hardware.log`, `parity-warp.log`, `shadow_probe.cpp`, `shader.dump.txt`. 최종 배포 CSO는 이 수치 비교의 CSO와 SHA-256이 일치한다.

## G04. 캐릭터 그림자·장비 포즈

`Part_Body`, skinned `Part_Equipment` shadow는 실제 unchanged shader가 소비하는 diffuse만 기존 CModel→CMaterial 경로로 준비한다. texture override와 diffuse 실패 처리를 유지하며 static socket 장비의 surface binder는 유지했다. 캐릭터 GPU 픽셀 비교는 하지 않았고, 실제 shader dependency와 caller 연결을 확인했다.

`CModel::Pose_BonesFrom`는 weak source owner와 source/destination pose revision이 모두 일치하면 shadow/본체의 중복 복사와 palette 무효화를 생략한다. 같은 프레임 local 수정, source 변경·같은 주소 재사용, clone 초기 상태와 revision wrap를 검증했다.

실제 Model/Part_Body/Part_Equipment CPP의 Debug 컴파일 및 CPU 8,764 검사에서 행렬이 bitwise 일치했다. 351/352본·6mesh·2pass fixture에서 복사704→352회, palette build2→1회, median0.103093→0.052530ms였다. bone/shader stand-in을 쓴 실제 함수 검사이며 게임 프레임 수치는 아니다. 증거는 `out/MapCharacterRenderPerformance20260912/character/receipt.json`, `README.md`, `pose_result.json`이다.

## G05. 제품 빌드와 다음 측정

사용자가 Client/Server를 종료한 뒤 PID 부재를 확인하고 정본 Product Debug 빌드를 수행했다. 종료 코드0, 소요104.406초다. 기록은 `out/MapCharacterPerf20260912/product-build.log`, `out/BuildPipeline/runs/20260912T073450945Z-debug-product.json`이다. 기존 문자 집합·외부 PDB·shader 경고는 남아 있어 경고0 빌드는 아니다.

Engine DLL 배포본, GameInstance/Model/Shadow SDK header와 원본, 검증 CSO와 배포 CSO의 일치를 확인했다. 기존 Engine/Client project/filter XML 네 파일 parse와 `git diff --check`를 통과했다. 새 C++ 파일·프로젝트 등록·저장 JSON 변경은 없다. domain publish와 광역 진단은 실행하지 않았다. `out/MapCharacterPerf20260912/deployment-check.json`에 배포 대조를 기록했다.

새 Client는 `Client/Bin/Debug/Client.exe`이며 16:34:50 KST 빌드다. LAN 설정은 server-host, debugger `Server + Client` profile, Client endpoint `192.168.0.14:7777`이다. 보고 시 Server CMD와 Client 프로세스는 종료 상태다.

사용자는 Debug x64의 Server + Client profile을 Ctrl+F5로 시작하고 Lobby→Character Select로 진입한다. 이전과 같은 캐릭터 두 개·카메라·해상도·도구 조건에서 F1→Open Composition Profiler→Capture로 수집하고 Save JSON한다. 맵 그림자 누락이나 캐릭터 변화 유무도 직접 확인한다. Alt+V 구간은 별도로 기록한다. 평상시 title FPS 비교는 도구 창을 닫은 조건도 구분한다.

남은 큰 후보는 opaque map submit의 평균19.68µs/회다. 같은 wrapper를 사용하는 shadow6.53µs/회와 차이가 있고 submit 내부에는 Map/GetData/Flush가 없다. opaque shader의 21 texture·8 MRT에 따른 Debug D3D validation/driver 비용은 추가 분리가 필요하다. 다음 사용자 캡처에서 이번 shadow 개선 후 비중을 확인한 뒤 이 제출 경로와 캐릭터 animation을 이어 최적화한다.


## G06. 사용자 Character Select 확인과 쿠크 후속 기준

사용자는 첫 제품 빌드를 직접 실행하고 “character select 지금 130 깔끔하게 나와”라고 확인했다. 사용자 관찰 FPS이며 동일 조건의 재캡처나 자동 visual PASS로 대체하지 않는다. 첫 구현은 사용자의 `aac4fbdd` commit에 포함됐으며 쿠크 후속 시작 시 worktree는 clean이다.

쿠크 후속 입력은 `Client/Bin/ProfilerCaptures/profiler_20260912_164743_351_frame104_63936_0.json`이다. 104프레임 중GPU valid100, pending4, drop0다. UI 전환 영향을 제외한54–100(47프레임,약0.865초)의 interval18.397ms/54.36fps, CPU17.983ms, 맵 BindAndDraw5.057ms(Material2.149/Submit2.499/Apply0.303ms,119회)를 확인했다. 상시Effect5개 render1.712ms와 update0.473ms가 있다. 맵1234batch,962visibility,97actual drawing batch,117visible instance이며 현재 shadow는 실제 PS/VS0이다. 분석 정본은 `out/KoukuRenderPerf20260912/README.md`, `capture_summary.json`이다.

이후 구현·빌드·쿠크 개선 FPS는 아직 검증 전이며 위 Character Select 확인과 구분한다.


## G07. 닫힌 재질 진단 창의 반복 작업 제거

Rendering Workbench의 실제 목록 조회가 1초 수집 lease를 갱신한다. 조회가 없으면 RecordSurfaceBinding은 시각과 atomic deadline만 검사하고, mutex·32개 문자열 목록의 검색/삭제/추가를 수행하지 않는다. 기존 level 교체, 1초 만료,32row 상한과 source on/off 갱신은 유지되며 pane의 안내도 수집 시점을 설명한다.

실제 이전/수정 진단 함수 본문을 추출한 fixture에서 활성 수집 목록, source toggle, level 변경, 만료 및 재열기를 확인했다. 실제 파일과 동일한 Debug CRT `/O2`에서 합성119개 식별자×1000프레임×7회 median은 기존0.420555ms/frame, 닫힌 pane0.0004161ms/frame이었다. 시간/level을 공급한 단위 fixture이며 게임 FPS가 아니다. `out/KoukuRenderPerf20260912/diagnostics/{prepare.py,probe.cpp,result.json,source_receipt.json}`에 기록했다.

MapAssetRenderUtils와 MapStaticBatchObject의 focused Debug 컴파일은 통과했다. 전용 shader 및 다른 변경을 포함하는 최종 Product 빌드는 별도 확인한다.


## G08. 쿠크 source BG 재질·맵 셰이더

family8이 source 설정과 DEFERRED 조건을 만족하고 명시적 diffuse override가 없을 때 기존 CModel→CMaterial 경로에서 미사용 legacy 입력을 건너뛴다. diffuse mirror/reset, native texture/flags, RNM, static shadow와 시간 입력은 유지한다. 일반 소품의 binary PS는 family 분기 이전에도 opacity dither를 적용하므로 g_Opacity는 이 경로에도 보존했다. static batch의 source-enabled family8 mesh만 같은 shader의24–26 pass를 선택하고 나머지는 기존 pass를 유지한다. 한 batch 안에서도 mesh별로 결정한다.

실제 CShader/FX와 추출한 이전/새 CMaterial·utility binder를 연결한504조건,17,031,168 float 값 비교를 통과했다. generic binary와 instanced shader, source on/off, native texture override 및 명시적 diffuse override, DEFERRED 외 profile, mirror/cull, opacity dither, emissive, RNM/staticshadow와 오염된 선행 상태를 포함했다. 최대 절대차1.49012e-08, 기준1e-5 상대 허용범위 이내이며 D3D error0다. null/model/mesh 및 필수 diffuse 실패도 보존했다. 해당 BG fixture의 model binding 호출8→4,20,000회 binder7회 median52.7611→36.1214ms였다. 이것은 실제 CModel의 facade·합성 texture를 쓰고 CShader까지 `/O2`로 컴파일한 fixture라 제품 Debug 시간으로 환산하지 않는다. `out/KoukuRenderPerf20260912/binder`에 원본 추출기·코드·로그·receipt를 기록했다.

shader는 기존0–23 pass/VS/일반 PS를 유지하고 source BG 계산·간접광 helper를 공유하는 전용 PS를 추가했다. 하드웨어 Debug/nondebug 각각6,912조건에서8 MRT+D32가 bit-exact이며 nonfinite/coverage mismatch/D3D error0다. WARP는 fixture를 넓히기 전6,800조건을 통과했다. 기존 일반 shader는 간접광/emissive helper 추출로 컴파일된 명령 수가1,713→1,723으로 변했지만 수치 비교는 통과했다. 전용 PS는326명령,texture21→9,sampler7→6이다.

16²×128draw×11교대 측정에서 Debug CPU DrawInstanced14.292→7.977µs, Effects Apply2.427→1.659µs였다. **넓은256² 조건의 GPU elapsed는11.857→13.929µs(+17.5%)로 증가했다.** nondebug에서도 비슷한 증가가 있어 GPU 개선으로 보고하지 않는다. PS/VS invocation은 양쪽 동일하다. branch/guard 후보는 안정적인 GPU 우세가 없어 제품에 포함하지 않았다. 현재 사용자 캡처의 CPU 제출 비용을 줄이는326명령 후보는 포함하되, 이 선택의 실제 frame 효과는 다음 쿠크 캡처에서 확인한다. 실행 중 다른 GPU 작업의 영향과 timestamp의 command 공급 간격도 분리되지 않았다.

최종 검증 CSO SHA256은 `6d8bbeef4ae5eec2511dbee1526a194a0b670672f11e585c132db896501cde43`이다. 전체 shader 근거는 `out/KoukuRenderPerf20260912/shader/README.md`, `summary.json`, `final_receipt.json`이다. 제품 빌드·배포 후 이 CSO와의 일치를 별도로 확인한다.

## G09. 상시 native particle 공통 입력

Effect_DocumentRenderer의 실제 ARTIST/PARTICLE registry 항목, loaded shader와 profile 범위가 일치할 때만 미사용 generic12 raw/flag+8 texture 입력을 건너뛴다. native packet/time/source textures/clamp, scene 입력과 공통 UV/color/clip/emissive는 유지한다. 공통 bind36→16회이며 move_destination sprite11개에 적용한다. mesh2356, 다른 family와 fixed carrier는 기존 경로다. 캡처의 sprite35–40회/frame가 같다면700–800 bind가 줄어드는 구조이며 새 프레임의 측정값은 아니다.

실제 CPP Debug 컴파일, old/new binder의 native88/기타637조건,24,340개 값·24,160개 first-failure 비교를 통과했다. 기존 실제 ARTIST particle CSO22개,110pass/220VS·PS reflection에서 생략20입력의 미사용을 확인했다. 실제 원본 DDS/parameter/clamp/pass를 공급한33조건은 hardware와 WARP 각각1,081,344 float 성분이 bitwise 일치하고 nonzero 출력도 있었다. `out/KoukuRenderPerf20260912/effect/README.md`, `receipt.json`을 따른다.

## G10. 렌더 제출 큐

기존 list 큐를 vector로 교체해 pass clear 후 capacity를 재사용하고 GameInstance→Renderer→queue에서 소유한 shared_ptr을 move한다. callback append 중 iterator/reference를 유지하지 않는 index 순회로 기존 same-pass 추가 순서와 owning 수명을 보존한다. sorted BLEND snapshot·failure clear·shadow off 정리도 유지한다. peak capacity는 Renderer 종료까지 보유하지만 객체 참조는 pass clear에서 즉시 해제한다.

실제 이전/새 메서드를 추출한 `/MDd /Od /RTC1`, `/MDd /O2` 각각4,794검사를 통과했다. 1,924제출×200frame는 기존384,800할당/해제, 예열 후 새 큐0/0이다. 합성 총 CPU 시간은 `/Od`109.834→29.667ms, `/O2`67.418→11.207ms이며 D3D/profiler stand-in을 사용하므로 게임 frame 값이 아니다. Renderer/GameInstance의 focused Debug 컴파일도 통과했다. 근거는 `out/KoukuRenderPerf20260912/queue/README.md`와 `source_receipt.json`이다.

## G11. 현재 남은 실행 확인

사용자가 Client/Server 종료를 알린 뒤 두 프로세스와 기존 MSBuild 부재를 확인하고 최종 Product Debug 빌드를 실행했다. Engine→Shared→Server→Client 전 단계 PASS, exit0,187.589초로2026-09-12 17:21:37 KST에 완료됐다. 기존 문자 집합·PDB·shader 경고는 남아 있어 warning0은 아니다. 에이전트는 Client/UI 실행·조작·프로세스 종료를 수행하지 않았다. 사용자는 소스와 관련 문서를 `5168899d`로 commit/push했으며, 해당 commit의9개 제품 소스와 빌드 전 검증 소스 hash는 모두 일치한다.

캐릭터 두 모델의 animation0.811ms 중 channel update0.760ms는 확인했으나 캡처가 모델 ID/caller를 노출하지 않아 이를 쿠크 보스 몸체로 단정할 수 없다. 이번 캡처의 Transition.Build는0회다. CNpc의 hammer clip 이름 조회와 transition 중복 행렬 계산은 조사 후보이며 이 후속에 추가 구현하지 않았다. 조명은 기존 camera frustum culling을 사용한다. 세 관문·연출·각 보스 전부의 실제 FPS/외형 검증 완료라고 보고하지 않는다. 사용자가 다음 쿠크 기본 상태와 다른 관문/연출의 같은 조건 캡처로 남은 비용을 확인한다.


빌드 기록은 `out/KoukuRenderPerf20260912/product-build.log`, `out/BuildPipeline/runs/20260912T082137790Z-debug-product.json`이다. Engine DLL 원본과 Client 배포본, Renderer SDK header와 원본, 최종 검증 map CSO와 Client 배포 CSO가 각각 SHA256 일치했다. 실제 배포된 binary/instanced CSO로 binder504조건을 다시 실행해17,031,168값 비교 최대절대차1.49012e-08, D3D error0를 확인했다.9개 제품 파일의 인코딩/BOM/CRLF, project/filter XML4개와 `git diff --check`도 확인했다. `deployment-check.json`과 `binder/run-deployed-hardware.log`가 증거다. 원본 Data/Resources와 domain publisher는 이 최적화에서 변경/실행하지 않았다.

최종 Client는 `Client/Bin/Debug/Client.exe`(17:21:37 KST), Engine DLL은17:18:34 KST다. Server는 up-to-date15:08:28 빌드를 유지한다. LAN은server-host이므로 사용자는 Debug x64 `Server + Client` profile을 Ctrl+F5로 시작하고 Lobby→KoukuSaydon에 진입한다. 같은 카메라·해상도·도구 조건에서 F1→Open Composition Profiler→Capture→Save JSON으로 후속 수치를 제공한다. 실제 쿠크 FPS 및 세 관문/연출 화면 확인은 아직 사용자가 수행할 단계다.

기존 PR #368은 이미 merged여서 같은 브랜치의 추가 push가 새 PR을 만들지 않는다. 사용자가 새 PR 생성·merge를 직접 하겠다고 명시했으므로 에이전트는 PR 생성/merge를 수행하지 않았다. 최종 빌드 결과를 기록한 이 RESULT의 후속 문서 수정만 로컬에 남긴다.


## 2026-09-12 G10–G11: 100fps·Alt+V 40fps 후속 진행

사용자가 PR369를 merge한 뒤 process82448에서 저장한 세 캡처를 분석했다. 시작251–548은 interval13.914ms(71.87fps), 3관문2161–2416은18.488ms(54.09fps), Alt+V3918–3953은38.396ms(26.04fps)다. 새 캡처의 맵 material/draw와 화면 조건은 이전 것과 달라 isolated A/B로 부르지 않는다. Character Select130fps는 사용자의 앞선 관찰이며 대응 최신 JSON이 없다.

시작→3관문의 맵 BindAndDraw는1.688→1.259ms/86→57mesh, 조명 CPU는1.576→3.420ms, PS는11.330M→23.741M다. Alt+V는 조명 CPU7.664ms/GPU8.688ms/PS192.60M로, 직전 PS24.52M의약7.86배다. Animation.Channels4.130ms, Sprite.InstanceBuild3.787ms, FrameRebuild1.615ms도 함께 조사했다. 마지막 두 파일은 각각1200frame ring이며 frame2420/3957은 끝번호다. 누적 CPU scope drop2761의 frame별 위치는 포맷에 없어 인증할 수 없다. 동기 모델 로드 frame1624의1.681초 spike, pendingGPU4frame, 별도Present대기구간은 일반FPS와 분리했다. NonBlend GPU timestamp에는 CPU feed gap 가능성이 있어 순수 map shader cost로 확정하지 않는다.

### 이번 반영 범위

조명은 기존 CRenderer→CLight_Manager→CVIBuffer 경로에서 scene 다음 transient의 순서와 receiver 필터를 유지하고, 같은 종류가 연속된 부분만 instanced draw로 묶는다. 16 scene+384 transient의 400개 record, 112byte stride를 44,800byte constant buffer에 한 번 공급한다. 첫 scene directional의 shadow/static-channel 소비와 source stencil은 그대로다. Engine 정본 Shader_Deferred의 기존 0–21 pass를 유지하고 22–27을 추가했으며 Client 사본도 일치한다. 화면 quad의 면적·해상도·조명 수·밝기는 줄이지 않았다.

Alt+V sprite는 한 draw 안의 카메라 inverse와 동일 owner/profile의 native velocity 분류를 재사용한다. particle의 수·수명·정렬·행렬 계산 순서는 유지한다. ModelCue는 외부에서 pose를 쓰지 않는 private CModel clone에 한정해 같은 animation index와 정확히 같은 요청 tick의 explicit seek 결과를 재사용한다. clip 변경·blend·nonfinite tick은 재사용하지 않으며 world/root/cue transform은 hit에서도 갱신한다.

CAnimation은 실제 재생한 clip에만 clone별 scale/rotation/translation cursor를 할당한다. 공유 CChannel 키는 변경하지 않고, cursor 구간 밖 seek·역재생·loop·중복 timestamp는 같은 upper_bound 탐색으로 돌아간다. 검증 후 Engine.vcxproj의 Channel.cpp/Animation.cpp 두 항목만 Debug x64 /O2를 사용하도록 설정했다. /MDd·_DEBUG·checked iterator와 정밀 부동소수점 계약을 유지하며 이 두 파일의 최적화된 stepping/지역변수 관찰은 제한된다.

맵은 동일 Bind_Material 호출 안의 SurfaceLighting·family8 emissive-time 중복을 제거한다. native texture register 이름은 고정 문자열을 사용한다. NPC는 실제 VtxAnimMeshBinary shader의 native base draw에서만 미사용 legacy 입력을 건너뛰며 diffuse admission/reset, native texture override, 독립 hit/skill emissive 4필드와 source material row는 유지한다. outline과 다른 caller는 기존 전체 binder를 사용한다. shared Effect의 객체별 uniform cache는 추가하지 않았다.

### 조명 검증과 남은 수치 한계

실제 old/new manager body의 72조합·9,192개 ordered record가 일치했다. receiver·shadow/static 소비·400개 capacity·401개 실패·null/invalid 입력을 포함하며 입력 검증 오류는 그리기 전에 실패한다. 기존 Renderer가 MRT 복구와 frame clear를 계속 소유한다. 변경 3CPP의 focused Debug compile과 Engine 정본 FXC /O1 compile이 통과했다. candidate/canonical CSO SHA256은 d83893904dc3f31a61d25aff92db8dc65116513a18e4428182215c0f1a64e601, 4,389,951byte다.

RTX4070 Debug device의 480조건, 조건당 FP16 2,048채널은 bitwise 일치, nonfinite/D3D error 0이다. debug layer를 끈 최초 검사에서는 15채널 차이가 있었고 그 실행의 최대오차·ULP는 기록되지 않았다. 후속 진단에서는 marker5/program17/specular G 한 채널이 0.027099609375→0.0270843505859375로 1 ULP, 절대차 0.0000152587890625였다. 이 한계값을 최초 15채널 전체의 상한으로 확장하지 않는다. candidate의 기존 generic wrapper 및 같은 instance 재호출에서도 차이가 관찰됐다. 별도 2개 프로세스에서 480조건×old/generic/instance 각4회 비교는 모두 0차이였으나, 최초 차이의 원인은 미확정이므로 nondebug 전 조건 bit-exact PASS로 기록하지 않는다.

실제 Product 설정(/Od manager·Light, /O2 CShader, Debug CRT/Effects11d)의 100회×7교대 median에서 동종 ALL 363light는 363→1draw, CPU7.885765→0.052345ms, SOURCE 400light는 400→2draw, 9.518315→0.076268ms였다. 종류가 계속 교대하는 최악조건은 ALL draw363 유지/8.065957→8.065072ms, SOURCE draw400 유지/8.336843→8.721233ms(+4.61%)다. 임의 sorting으로 blend 순서를 바꾸지 않으며 이 비용 경계도 유지한다. 256²·324point의 GPU1.917184→1.850496ms, PS21.233664M/VS1944는 동일했다. 따라서 pixel shading 양의 절감으로 보고하지 않으며 합성 draw 절감률을 게임 FPS로 환산하지 않는다. 상세 근거는 out/RenderTargets100Fps20260912/lights/receipt.json 및 README.md다.

### 입자·포즈·재질 검증

실제 Alt+V 두 문서 12시각의 2,026입자/239batch를 포함한 sprite 27,572개 입력·행렬 비교가 bitwise 일치했고 실패 입력3,066개도 동작이 같다. 카메라 조회2,026→239회, helper 격리 Debug median14.99948→2.57990ms다. 실제 설치 Alt+V clip2 section0–3의 private model 4개/각152bones를 사용한 544 pose 요청에서 local/combined bone행렬165,376개와 실제 socket행렬3,082개가 bitwise 일치했다. pose evaluation536→144회, 4모델×180frame×동일시각3회 synthetic median1,607.0648→574.9545ms다. renderer H/CPP focused compile도 통과했다. out/RenderTargets100Fps20260912/altv의 README/receipt가 근거다.

애니메이션은 별도 translation unit으로 기존 Channel/Animation /Od와 변경 /Od, Channel만 /O2, 둘 다 /O2를 각각 비교했다. bone store와 main은 공통 /Od, bone store는 실제 호출 경계를 맞춘 external noinline이며 profiler는 stand-in이다. 세 설정 각각649,002개 행렬이 bitwise 일치했고 clip clock·finished·seek·clone 결과도 같았다. 216channel×500frame×9교대 median은 각각59.7398→38.6606ms, 57.2653→27.1968ms, 58.6171→25.8106ms다. explicit duplicate 경계18개와 legacy combined-key768개를 포함한다. out/RenderTargets100Fps20260912/animation/README.md와 mixed/summary.json에 Debug ABI 및 분모를 기록했다.

맵 binder의 hardware504조건·17,031,168값은 최대차0, D3D error0다. native NPC binder는 실제 BossCatalog14family parameter와 animated binary CSO를 사용한 hardware/WARP 각각420조건·14,192,640값에서 최대차0, nonzero7,741,260, D3D error0였다. retained binding 7실패와 재시도도 일치한다. native bind49→31회, map 정적 shadow+RNM fixture7→6회이며 실제 게임 draw 수가 아니다. native20,000회 binder-only hardware median69.9897→46.3108ms다. legacy fallback의 hardware pick-position은 old→old에서도 달라 그 두 draw는 최종 hardware native 집계에서 제외했고 WARP에서는 통과했다. source-character forward map 분기는 call/consumer 분석까지이며 이 fixture의 GPU 인증 범위가 아니다. out/RenderTargets100Fps20260912/material/README.md와 receipt.json에 한계를 기록했다.

### 제품 빌드와 사용자 확인

소스 수정·최소 컴파일·위 수치 검증 및 Product Debug Engine→Shared→Server→Client 빌드·배포를 완료했다. 2026-09-12 18:20:39 KST, exit0, 총504.956초다. 새 Deferred FXC가 대부분의 시간을 사용했다. 기존 shader·PDB 경고는 남아 있으며 warning0은 아니다. 빌드 전 고정한20개 제품 파일의 SHA256은 빌드 후 모두 일치했다. Channel/Animation 실제 CL command의 /O2·/MDd·_DEBUG·/fp:precise와 /RTC 미사용을 확인했다. Engine DLL 원본/Client 배포본, 새 public SDK header4개, Engine→EngineSDK→Client Deferred HLSL, 수치 검증 CSO→Engine→Client CSO가 각각 동일하다. project/filter XML4개, 검증 receipt JSON4개와 전체 git diff --check도 통과했다.

빌드 기록은 out/RenderTargets100Fps20260912/product-build.log, product-logs, out/BuildPipeline/runs/20260912T092039173Z-debug-product.json이다. 20개 소스와 배포 검증은 prebuild-sources.json, check_deployment.py, deployment-check.json에 남겼다. 최종 Engine.dll은18:20:36 KST/8,245,760byte/SHA256 97114371b18261159bb8598ddccc88216ec2f30ab0381814a091167333558693이다. Client.exe는 다른 작업의18:11:36 build가 같은 최신 Client 소스를 이미 포함하여 up-to-date였고 SHA256 ec82dd7ce57d6d2ecefa2be83003d80c5cfffafea069f154b9cd1487d9ba953c,55,544,832byte다. Server도18:01:02/13,336,064byte의 최신 빌드를 유지했다. 원본 실행 폴더의 DLL과 Deferred CSO가 이번에 교체됐으므로 EXE 수정시각만으로 미반영으로 판단하지 않는다.

최종 배포 Engine.dll을 별도 out/RenderTargets100Fps20260912/deployed-modelcue에 복사한 뒤 기존 headless Alt+V model-cue probe도 재실행했다. 실제4모델의544요청,165,376bone행렬·3,082socket행렬·8실패·16clone reset 비교가 다시 일치했다. 두 helper 모두 새 Engine을 사용하는 이번 median117.2315→41.3539ms는 pose-cache 비교이며, 이전 DLL의 다른 실행과 섞어 전체 성능 개선률로 환산하지 않는다. 새 DLL·probe·결과 hash는 deployed-modelcue/receipt.json에 기록한다. Client/UI 실행이나 화면 캡처는 하지 않았다.

공유 checkout의 다른 쿠크 연출·데이터·MainApp 변경을 보존했으며 stage/commit·branch 전환은 수행하지 않았다. 빌드 완료 시 실행 중 Client/Server는 다른 작업이 사용자의 요청으로 시작한 out/InteractiveRuntime/20260912_174839 복사본이다. 이 작업에서는 Client/Server 실행·조작·캡처·종료를 수행하지 않았다. 사용자는 이 이전 복사본의 Client/Server를 정상 종료하고 Visual Studio Debug x64 Server + Client profile을 Ctrl+F5로 시작한다. Lobby→KoukuSaydon의 같은 시작 위치·3관문 카메라·창술사 Alt+V를 같은 해상도와 도구 표시 조건으로 재측정한다. F1→Open Composition Profiler→Capture→Save JSON의 새 원본을 사용한다.

100fps/40fps 달성과 실제 캐릭터·맵·Alt+V 외형은 사용자 재캡처 전까지 미확인이다. source 반영, CPU/GPU fixture, Product 배포, 사용자의 화면/FPS 확인을 서로 대체하지 않는다.


## 2026-09-12 G12–G14 구조 진단: Debug 180fps

사용자는1280×720의 현재 Debug를180fps 목표로 선택했으며 Release 전환을 목표의 대안으로 삼지 않는다. 실행 중 EXE/DLL의 최신18:20 배포 일치는 직전 확인했고, ProfilerCaptures의 최신 JSON은 여전히17:31이다. 따라서 이번 읽기 전용 조사는 코드에 남은 구조를 확인한 것이며 새 FPS·현재 병목의 ms 순위를 확정하지 않았다.

확인된 구조는 다음과 같다. MapStaticBatchObject는 CPU sphere frustum과 assetId+mirror instancing을 갖지만 큐 등록 후 Render 시점에 판정하며 model 전체 구체가 통과하면 모든 mesh를 제출한다. MapAssetObject fallback은 cull 전에 재질 순회와 일부 snapshot 요청도 한다. 제품 경로에는 HZB/occlusion/compute cull/indirect draw 및 runtime geometry LOD chain이 없다. 카메라 밖 그림자 caster를 보존해야 하므로 최종 camera/light 확정 순서를 무시한 조기 제거는 금지한다.

Character/Npc의 CPU animation·combined bone·장비·부착점·morph/cloth 준비는 일반 camera visibility와 분리되어 있지 않다. VS의4weight GPU skinning은 이미 있으므로 GPU LOD만으로 이 CPU 비용이 사라지지 않는다. Effect는60Hz fixed simulation 외에 render frame마다 FrameRebuild/CPU instance 구성·upload를 수행한다. 독립 particle update의 제한된 worker 병렬화와 palette/pose/cursor/instance 저장소 재사용은 이미 구현되어 있으므로 이를 새 누락 기능으로 부르지 않는다.

조명 instancing은 각 광원의 전체 quad와 source material row마다 전체 light 순회를 유지한다. global stencil은 모든 marker-5 픽셀을 통과시키고 다른 row는 PS에서 discard한다. G-buffer는8 MRT의 논리 포맷 합84B/pixel이며 조명 누적은 추가16B/pixel이다. 이 값은 실제 DRAM bandwidth 측정이 아니다. PickPos는 geometric normal/flag도 소비하므로 이름만 보고 제거할 수 없다. SceneResolve/Final 등 후처리는 고정 비용 후보이며 최신 측정 전 주 병목으로 단정하지 않는다.

G12의 source local-light 범위 제한은 Deferred.hlsl의attenuation 계산616–625, native 이전 cutoff485–486, 최종 RGB/ambient 감쇠508/514–515를 근거로 한다. directional·forward·다른 map family로 확대하지 않는다. row별 fullscreen stencil 재생성은 현재1회 mask를R회로 늘려source 픽셀이 작거나light가 적으면 회귀하므로 첫 구현으로 선택하지 않았다. 계획은 기존 IMPLEMENTATION_PLAN의G12–G14에 추가했다. 이번 구조 진단 단계에서는 CPP/HLSL 변경·빌드·Client/UI 조작·새 성능 측정을 수행하지 않았다.


## G12–G14. 20:06 쿠크 캡처 후 현재 구현 범위

사용자는 Debug180fps 목표로 조명, 맵 가시성, 캐릭터 이펙트 CPU 준비, 재질 제출, GPU LOD를 함께 요청했고 이후 현재 진행 범위까지만 마무리하도록 지정했다. 새 범위나 Release 전환을 추가하지 않는다. 이번 round의 headless 증거는 `out/RenderStructure180Fps20260912/`에 둔다. 공유 checkout의 쿠크 연출·데이터 및 다른 UI 작업은 유지하며 자동 commit/stage/push하지 않는다.

### G12. 새 기준 측정과 광원 영역 제한

`profiler_20260912_200653_932_frame253_70792_0.json`은65프레임189–253, GPU완료61/pending4다. 시작 부분 frame189는 interval0이며 scope 일부가 CPU root 밖에 있어190–249의완료60프레임을 비교 창으로 둔다. 이0.795초 표본의 mean interval13.251268ms=75.464fps, CPU12.35717ms다. 장시간 안정 FPS나 Alt+V 재측정이 아니다. ImGui vertex7874.87, active window5/platform viewport3으로 개발 UI가 켜진 조건이다. 기존 analyzer 출력의 잘못된 `imguiVertices` spelling은 `imGuiVertices`로 교정했다.

맵은21 mesh submit과 BindAndDraw0.464ms, 전체 draw255.4회다. NONBLEND queue991개는 draw 수가 아니다. light CPU0.672ms, particle Render1.327ms, FrameRebuild0.207ms, ImGui BuildAndSubmit1.952ms가 측정됐다. NonBlend GPU5.228ms에는 CPU feed와 scheduling이 포함될 수 있어 순수 맵 shader 비용으로 확정하지 않는다. Blend PS0만으로 입자의 실제 화면 밖 원인을 확정하지 않는다. 같은 장면의 적용 후 FPS는 아직 사용자 재캡처 전이다.

Light_Manager와 Engine/Client Deferred shader는 source-character point/spot light만 range 구체를 감싸는 box로 투영해 quad clip distance를 제한한다. 기존 light record112byte의 예약 필드를 쓰고 directional/ordinary/near-plane·invalid 투영은 기존 full quad를 유지한다. 광원 수·밝기·range·반사 수학, source row와 blend 순서는 바꾸지 않았다. 새 VS SRV 없이 기존 projection에서2pixel margin을 구한다.

기존/수정 FX의1,998조건147,308,544 FP16채널 중52,272(0.0355%)가 다르고 최대 절댓값은0.001953125다. 최대 절댓값 예시는2.099609375→2.1015625(1half ULP), 가장 큰ULP1024는0.00006103515625→0.0001220703125라는 near-zero 값이다. clip을 끈333조건과 ordinary33조건은 차이0이고 검사한 양의 sphere 기여 영역의 누락도0이다. 활성 primitive clipping에서 차이가 생기는 것으로 좁혔으며 hardware interpolation이 정확한 원인이라고 증명하지는 않았다. 따라서 bit-exact/visual PASS로 기록하지 않는다.

1280×720,24point/5paired sample의 synthetic Debug fixture는 source16%에서 PS4.464M→1.803M, GPU1.107→0.603ms, source전체에서 PS23.040M→1.826M, GPU1.076→0.123ms였다. ordinary PS동일/GPU+0.29%, near fallback PS동일이다. source mask1회 포함 수치이고 실제 게임 FPS 향상으로 환산하지 않는다. 전체FXC, CPP focused Debug, source/include closure 일치와 D3D error0을 확인했다.

### G13. Effect CPU 준비와 최종 화면 밖 제출

Effect_Playback은60Hz fixed-step/root/anchor/document 상태가 그대로일 때 평가 결과를 재사용한다. frame provider는 계속 호출해 외부 변경·실패를 반영하고 reset/seek/새step은 무효화한다. 실제 문서4종의60/75/180Hz·정적/이동root/anchor/history8,784프레임464,402입자 행,180,547,411-byte 정규화 출력이 bitwise 동일했다. provider3,596회와39실패도 같다. Rebuild본체8,838→3,835,180Hz5-marker Debug fixture360프레임 중앙값463.1872→169.7018ms다. 현재75fps 장면에서 같은 절감률을 가정하지 않는다.

Effect_DocumentRenderer는 실제 실행 native ARTIST particle VS가 인정된 경우 최종billboard/SubUV quad의 clip XY를 검사한다. 확실한 화면 밖에서만 instance/material/upload/draw를 제외한다. 다른 shader family·mesh·invalid 계산은 기존 경로다. 크기/alpha/가려짐 추정으로 입자를 삭제하지 않았다. 실제22VS/110null-GS pass, hardware/WARP각1,936조건31,719,424 pixel component가 동일했고 제외1,100회/visible638조건을 포함한다. 이 검사는 coverage PS를 사용한 수치 증거이며 원본 material의 사용자 fidelity 판정이 아니다. helper Debug 평균비용은별도7×88,000후보 중앙518.944ns다. 실제scene제외율은미확인이다. 두 제품 CPP focused Debug compile이 통과했다.

### G13. Effect 전체가 공유하는 재질 입력 재사용

Shader H/CPP는 같은 Effect의 모든 Clone이 동일한 EFFECT_BINDINGS에 마지막 성공 입력을 기록한다. 작은 raw/matrix와 단일 SRV의 동일 재설정만 생략하며 종류 변경·array/큰 입력·실패는 해당 기록을 무효화한다. pass Apply는 항상 실행한다. 객체별 last-uniform이나 기기 상태 cache로 sibling 변경을 놓치지 않는다. Bind_Texture의 const ComPtr 참조로 call-entry의 추가 참조 증감도 줄였다. MainApp의 쿠크 Pattern/Bundle 두 목록은 기존 read-only vector를 참조하며 Reload 전에 stable 선택 ID를 복사하는 기존 계약을 유지한다.

main/binder는 /Od/RTC1, Shader만 기존 제품과 같은 /O2인 분리 TU로 검사했다. 모두 /MDd/_DEBUG/정밀 float다. map 504조건 17,031,168값의 depth+8MRT가 old/new Shader 간 bitwise 동일했고 Clone 교차 오염·ClearState·100 matrix/raw/array/SRV 교차도 통과했다. native 14 catalog family 420조건 14,192,640값은 WARP에서 bitwise 동일했다. hardware native는 emissive에 최대 4.76837158203125e-7(2 float ULP)의 차이가 있고, 변경 없는 old를 다시 실행해도 같은 최댓값 차이가 관찰됐다. 원인은 미확정이며 hardware bit-exact로 표기하지 않는다. 다른 7target/depth는 일치했다. 기존 7개 필수 binder 실패와 재시도도 보존했다.

같은 현재 binder 20,000회/7sample 중앙 CPU fixture는 map 86.4388→70.6844ms, native hardware 67.4787→59.3808ms였다. 반복 재질 fixture의 값이며 실제 frame 시간이 아니다. inherited 출력의 7→6 Model call/49→31 bind는 이전 binder 변경값으로 이번 Shader 절감량과 구분한다. 실제 Shader.cpp focused Debug 컴파일과 독립 소유권 리뷰를 완료했다.

### G14. LOD와 가시성의 유지 경계

새 StaticMeshLod는 기존 decoded 정적 mesh의 VB를 유지하고 meshoptimizer v1.0 고정 MIT source에서 19개 normal/tangent/binormal/UV0–2/RGBA 속성과 LockBorder로 index LOD를 생성한다. GPU가 visible batch 전체의 보수적 view bounds를 소비해 한 LOD를 선택하고 합쳐진 index range에 indirect draw 1회만 실행한다. 원본 IB/shadow LOD0, 작은 draw와 invalid/near fallback을 유지한다. 처음 opaque였던 shared mesh가 material variant로 교체되는 경계를 리뷰에서 찾아 실제 draw의 현재 재질을 다시 검사하도록 보완했다. opacity/masked/다른 family/morph와 unsupported projection은 기존 경로다.

오차 0.25px는 quadric+attribute 지표를 투영한 선택 기준이며 엄밀한 최대 실루엣/재질 pixel 오차 보장은 아니다. Resources를 변경하거나 새 모델 runtime을 추가하지 않았고 사용자의 LOD 전환/fidelity 판정은 남아 있다. GPU 선택 index는 CPU 동기 readback 없이 별도 indirectDrawCalls/indirectIndexUpperBound 카운터로 기록하며 기존 indices는 direct 정확값만 담는다.

MapStaticBatch의 전체 bounds broad phase는 확실한 외부 batch만 개별 순회 전에 제외한다. 독립 검사 1,997,657건에서 30,000batch/1,620,000scale·shear 방향과 Bern grace3, 240frame history/visibility 변경/Map 실패의 기존 payload·LOD bounds 유지를 확인했다. broad reject 4,968건에서 기존 개별 판정의 visible child를 숨긴 경우는 0이다. 제품 통합 빌드와 최종 LOD 손익 기록은 아래 완료 기록으로 구분한다.


G14의 실제 생성/compute/indirect 최종 fixture는 원본 bytes와 LOD0 index를 보존하고 near→LOD0, invalid/empty 거부, D3D error0을 확인했다. 65,536 float pixel 비교의 최대 차이는1.19e-7이다. 큰 표본은98,304index×3instance에서 LOD2 index29,490을 선택했고,120draw GPU1.516→1.312ms/VS12,662,784→4,773,480이었다. 반면24,576index×3instance 작은 표본은 GPU0.471→0.989ms로 나빠졌다. 이를 근거로 실제 draw에서 원본 index×instance가294,912 이상일 때만 GPU 선택을 적용하며 Prepare의 S_OK만 indirect 성공으로 취급한다. 작은 draw는 기존 direct LOD0를 유지한다.

현재20:06 쿠크 캡처의 전체 frame indices 평균251,273.4는 이 draw별 기준보다 작다. 따라서 이 캡처 조건에는 새 GPU LOD가 활성화되지 않으며 이 장면의 FPS 개선 근거로 삼지 않는다. 설치3WModel/4submesh의 준비는 합계186.09ms이고,1개만148,224→102,456index로 감소했으며3개는 seam/attribute 제한 때문에 원본을 유지했다. 이는 LOD 생성 표본이지 현재 쿠크 draw admission이나180fps의 증거가 아니다. 추가 최적화 범위는 진행하지 않는다.

큰 LOD fixture에서도 CPU 제출 비용은120draw 기준 direct 약0.095–0.098ms에서 indirect 약0.428–0.647ms로 늘었다. 위 기준은 GPU 작업량·GPU 시간의 이득을 확인한 최소 조건이며 CPU/GPU 전체 frame이 빨라진다는 보장은 아니다. 현재 CPU 준비가 큰 쿠크 표본에 이를 강제로 켜지 않는다.


### G12–G14. 최종 Debug 제품 빌드와 배포 확인

2026-09-12 20:50:55 KST에 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product -BuildLogDirectory out/RenderStructure180Fps20260912/product-logs`가 exit0으로 완료됐다. Engine→Shared→Server→Client 컴파일·링크·SDK·shader·DLL 배포가 성공했다. 전체 Product receipt는 `out/BuildPipeline/runs/20260912T115055253Z-debug-product.json`, 로그는 `out/RenderStructure180Fps20260912/product-build.log`다. C4819/C4828 인코딩 경고와 외부 라이브러리 PDB LNK4099 경고는 남아 있으며 이를 무경고 빌드라고 보고하지 않는다.

빌드 시작에 기록한70개 source 입력은 종료 후 모두 동일했다. Engine→Client DLL, Deferred/MeshLod의 HLSL·CSO, 변경 public header9개의 EngineSDK 사본, 수치 fixture에서 검증한 두 CSO까지16쌍의 hash 일치를 확인했다. 변경 project/filter4개 XML parse와 `git diff --check`도 통과했다. 배포 상세는 `out/RenderStructure180Fps20260912/deployment-check.json`이다.

| 제품 출력 | 최종 상태 | SHA256 |
|---|---|---|
| Client/Bin/Debug/Client.exe |20:50:54,55,544,320byte|93cb81eb47f6640b565bcacfcf603534080e05dfa02fd4434d55423167ad6909|
| Client/Bin/Debug/Engine.dll |20:48:03,8,403,968byte|e6a90abfc0cd1589975c59adcb68c709770389544a775fd6f0f6ead750cb8992|
| Client/Bin/Debug/Shader_Deferred.cso |검증한candidate와동일|6693974efbec9aeccaf7091332fe3c906d3e9e7ea517352289d25e0357db153f|
| Client/Bin/Debug/Shader_MeshLod.cso |신규compute정상배포|8962f605a8ebd9eec41f963d14e508446583dd824ee52e866a2836d3b0772e80|

Data publisher와 광역 진단은 실행하지 않았다. 사용자의 runtime Data/Resources 원본도 이번 최적화에서 바꾸지 않았다. 최종 빌드 완료(20:50:55 KST) 뒤 사용자가 20:51:43 KST에 Client(PID 15004)와 Server(PID 80532)를 실행한 것을 read-only process 조회로 확인했다. Client 실행 경로는 Client/Bin/Debug/Client.exe이고 로드된 Engine 모듈은 Client/Bin/Debug/Engine.dll로, 방금 배포한 최종 Debug 경로와 일치한다. 근거는 out/RenderStructure180Fps20260912/running-process-check.json이다. 세션 시작 LAN 설정은server-host/192.168.0.14:7777로 완료됐으므로 사용자는 VS의Server+Client profile에서Ctrl+F5로 실행하며 Client 시작은 기존Lobby다. Client/UI 실행·조작·캡처와 visual 판정은 에이전트가 하지 않았다. 현재 진행 범위의 구현·자동 수치 진단·제품 빌드는 완료했고, 실제 FPS·Alt+V·조명과 LOD 외형의 사용자 확인은 미실행이다. 100/40/180fps 달성으로 기록하지 않는다.
