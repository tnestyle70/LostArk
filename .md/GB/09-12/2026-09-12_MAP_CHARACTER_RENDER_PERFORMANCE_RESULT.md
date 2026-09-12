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

쿠크 후속 source와 focused/numeric 검증은 준비됐고 최종 Product EXE·DLL 빌드는 실행 중 Client/Server 종료를 기다린다. 에이전트가 프로세스를 종료하거나 UI를 조작하지 않았다. 다른 작업의 KOUKU_SOURCE_SEQUENCE_RESTORE PLAN/RESULT 변경은 별도 작업으로 보존하며 자동 stage/commit하지 않는다.

캐릭터 두 모델의 animation0.811ms 중 channel update0.760ms는 확인했으나 캡처가 모델 ID/caller를 노출하지 않아 이를 쿠크 보스 몸체로 단정할 수 없다. 이번 캡처의 Transition.Build는0회다. CNpc의 hammer clip 이름 조회와 transition 중복 행렬 계산은 조사 후보이며 이 후속에 추가 구현하지 않았다. 조명은 기존 camera frustum culling을 사용한다. 세 관문·연출·각 보스 전부의 실제 FPS/외형 검증 완료라고 보고하지 않는다. 사용자가 다음 쿠크 기본 상태와 다른 관문/연출의 같은 조건 캡처로 남은 비용을 확인한다.
