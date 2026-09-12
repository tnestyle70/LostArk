# 맵·캐릭터 렌더링 성능 개선 결과

## G00. 현재 완료 상태

`codex/map-character-render-performance`, 시작 HEAD `a7205863`에서 맵 그림자 culling, 단순 shadow shader 및 캐릭터 shadow/장비 포즈 중복 준비를 수정했다. Debug Product의 Engine→Shared→Server→Client 빌드·링크·배포가 2026-09-12 16:34:50 KST에 완료됐다. 작업 소스는 미커밋 상태이며 runtime 데이터와 Resources payload는 변경하지 않았다.

현재 완료 범위는 소스 수정·수치 검증·제품 빌드다. 새 게임의 FPS, 실제 맵 그림자와 캐릭터 외형은 사용자 확인 전이다. Client/UI 실행·조작·화면 캡처는 수행하지 않았다.

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
