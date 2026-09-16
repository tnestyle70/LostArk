# 차원술사 T 소환 조명 제외와 V 초기 사각형 진단 결과

기준일: 2026-09-15. 작업 범위는 T 소환 한 cue와 V camera attachment 17개다. Product build와 Client/UI 실행·캡처는 하지 않았다. 사용자 화면 통과는 아직 없다.

## G00 완료 상태

T `dimension_summon`의 맵 direct/ambient/specular 색 기여를 제거하는 전용 animated pass11을 연결했다. 기존 MASKED threshold 0.3, CModel skinning, NONBLEND stage, depth/pick 출력은 유지한다. 사용자 요청에 따른 unlit 정책 변경이며 원본 조명 모델과 동일하다는 주장은 아니다.

V의 원본 Required `buselocalspace=true`와 현재 DIRECT 저작값 false가 충돌한 카메라 요소 17개를 true로 복구했다. 실제 Catalog stage와 CEffectPlayback에서 카메라 +1m 이동을 기존 입자가 따라가는 것을 확인했다. 이 결함은 확정·수정되었으나 **사용자가 말한 초기 사각형의 최종 원인과 해결은 아직 확정하지 않았다.**

## G01 실제 입력과 소비자

| 경계 | 실제 파일과 위치 |
|---|---|
| T/V 입력 | `Data/Balance/PlayerSkills.json:2354`, `:2377`의 skill2050500 T /2050520 V |
| clip | `Data/Animation/Authored/DimensionMaster/DimensionMaster.skillbindings.json:71`, `:77`의 DimensionPrison /TimeWave |
| effect asset | 같은 폴더 `DimensionMaster.animevents:1523`, `:1542`의 unified /full.restore |
| T exact cue | `Client/Private/Effect_DocumentRenderer_MaterialHelpers.cpp:654`에서 문서, dimension_summon cue, DimensionMaster_DimensionSummon.wmodel, dimensionprison clip, MASKED를 함께 식별 |
| T 실제 pass | `Client/Private/Effect_DocumentRenderer_Rendering.cpp:447`, `:523`에서 위 식별 결과만 pass11 선택 |
| V 제품 payload | `Data/Effects/EffectCatalog.json:302`의 DIRECT_AUTHORED_DOCUMENT |
| V 실제 local 소비자 | `Client/Private/Effect_Playback.cpp:8280`의 SourceEmitterWorld 및 같은 재생 경로의 CurrentRoot/SpawnRoot 선택 |

V에는 reconstructed 경로가 source localSpace를 덮어쓰는 `Effect_ReconstructedExecution.cpp:5266`이 있지만, 이 제품 자산은 DIRECT이므로 이 경로로 교정되지 않는다. 실제 catalog Capture_ProductLoadStageRequest→Stage_ProductLoadTarget으로 원본 저작 문서를 읽어 이 구분을 검사했다.

## G02 T 구현과 수치 검증

`Client/Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl:360`의 새 PS는 기존 Evaluate_Material의 coverage를 읽고 diffuse RGB를 emissive RGB로 옮긴다. diffuse RGB, normal alpha specular, material specular, source character/geometry metadata와 depth zw marker는0이다. `:828`의 pass11은 기존0~10 뒤에 추가했으며 기존 pass 이름·순서는 보존했다. 새 uniform이나 지속 상태를 추가하지 않았다.

현재 맵 조명 합성의 diffuse*shade+specular 항은0이고 재질 RGB는 emissive 항으로 전달된다. 장면 후처리와 fog 전체를 우회하는 화면 overlay는 아니다. 일반 캐릭터·맵·다른 effect model cue의 조명 정책은 바꾸지 않았다.

| 검증 | 실제 결과 |
|---|---|
| 변경 renderer C++ 최소 컴파일 | out 격리 CL 성공 |
| 실제 새 PS FXC ps_5_0 compile | 성공. 기존 shared include loop-shadow / Evaluate_Material 관련 경고는 남음 |
| 실제 PS WARP 8MRT | 합성 legacy texture RGB(0.2,0.35,0.1), alpha0.29 discard; alpha0.30/0.80 유지 |
| 조명 불변 | 실제 PS 출력에 현재 deferred 항을 흰색/푸른색/붉은색 shade로 평가해 같은 RGB 유지 |
| pass 계약 | 기존0~10 이름·순서 동일,11만 추가 |

WARP는 실제 새 pixel shader를 실행했지만 합성 입력을 사용했다. 실제 소환 모델의 사용자 화면 판정으로 대체하지 않는다.

진단 자료는 `out/DimensionMasterTVReview20260915/verify_t.py`, `t-unlit-warp.json`, `t-unlit.disasm.txt`, `contract-check.json`에 있다.

## G03 V 카메라 추적 결함 복구

`Data/Effects/Authored/effect.dimensionmaster.skill.2050520.full.restore.effect.json`의 현재43 elements 중 camera_view17개만 `detail.particle.localSpace`를 false→true로 바꿨다. 모든17개의 같은 sourceRecipe Required module은 원본 buselocalspace=true다. 나머지 JSON 값은 이전 파일과 구조적으로 완전히 같다. bloomIntensity0, 현재 요소 선택, 타이밍과 재질·배율은 보존했다.

이전09-14 `CHARACTER_EFFECT_AND_ARENA_RECOVERY` G01의 사용자 전체 캐릭터 localSpace 해제에서 이 camera attachment까지 false가 되었다. 새 요청의 원본 기반 복구를 위해 V 카메라17개에 한해 예외를 적용했으며 actor-root 등 다른 영역은 되돌리지 않았다.

첫 cam_01 carrier는 elements13/14의 `authored.source-particle.full-v.dddd26700e1cd32033e7`, `authored.source-particle.full-v.c8727f95fd8234a116d6`다. source `FX_PC_SWP_02.Par_R_SWP_TimeWave_00_cam_01` emitter16/17이며,0초 native69 `fx_o_pa_splitline_02_ad`를 사용한다. 첫 요소는 JSON1732/1753줄, 두 번째1861/1882줄에 있다.

원본 action2050520 notify006은 PlayCameraParticleEffect, 시작0초, duration0.7188959718이다. 기존 추출 `out/DimensionMasterV20260912/source_action_2050520.json:173`의 serializedPayload SHA256은 `dfa1a4cd1fd9162ba1849b1f04792a65c91d74af05e1dc37cb8899b893caa93e`다. 현재 socket은 position(0,0,0.5), rotation(0,-90,0), follow=true다.

현재 `Effect_Playback.cpp`를 out에 새로 컴파일했다. 실제 catalog Capture/Stage 문서에서 각 camera 요소를 따로 Stage_Document한 뒤 실제 fixed-step 재생을 사용했다. 카메라 수치 앵커를 +1m 이동시킨 프레임에서 수정 전 false 대조군의 기존 입자 중심 ΔX=0, 수정 후 true는 ΔX=1이다. SourceEmitterWorld도 +1m 이동한다.17/17 통과했으며 첫 입자 시점0.016667/0.716667/1.58333초 묶음을 모두 포함한다.

이는 실제 playback 소비자가 바뀐 증거다. 수치 카메라 앵커이며 실제 Client 화면 캡처나 사용자의 카메라 동작 확인은 아니다. 진단은 `v_camera_playback.cpp`, `v_camera_playback.json`, `v_probe.ps1`에 있다.

## G04 초기 사각형의 남은 경계

기존09-10 per-occurrence SceneHDR snapshot 갱신은 현재 renderer에 연결되어 있으므로 같은 수정을 반복하지 않았다. 첫 native69 두 요소의 원본 compiled PS GUID `5dfee80075c74444bffc1d810344820c`를 설치 ShaderCache에서 새로 추출해 현재 VNative69와 대조했다. 일정한 SceneColor/noise texture, UV·particle color·dynamic 입력 48조건의 실제 WARP 출력은 최대 절대 오차 0이었다(`out/DimensionMasterVMaterialReview20260915/v69-existing-warp.json`). 따라서 이 범위의 알파·색 계산은 일치한다. 공간적으로 변화하는 SceneColor sampling과 원본 camera basis는 이 검증에 포함되지 않는다. SceneColor 출력이 사각형을 만든다는 추정만으로 shader alpha나 mask를 수정하지 않았다.

EPAL_Z는 현재 playback에서 AXIS_POSITIVE_Y로 변환된다(`Effect_Playback.cpp:1686`). sprite geometry의 local-axis 적용은 `SourceTransformTrack`이 있을 때만 열린다(`Effect_DocumentRenderer_GeometryHelpers.cpp:282`). 현재 V camera 요소는 이 track이 없어 localSpace=true 복구 후에도 sprite 법선은 world +Y다.

실제 Make_ParticleSpriteWorld 본문을 그대로 out에 추출해 두 초기 요소를 45° pitch /30° yaw 수치 카메라로 평가했다. 수정 전과 localSpace=true 후 모두 법선(0,1,0), dot(cameraForward)=-0.707107이다. SourceTransformTrack 조건을 진단용 복사본에서만 열어 emitter basis를 적용하면 법선(0.353553,0.707107,0.612373), dot=0이 되어 옆면으로 눕는다. `v_axis_basis.json`, `particle_sprite_world_exact.cpp`와 본문 hash에 기록했다.

따라서 track 조건 제거만을 복구로 적용하지 않았다. 원본 PlayCameraParticleEffect의 camera/UE 좌표 basis와 축 잠금 최종 소비자를 더 확인해야 한다. 이 수치는 현재와 가상 수정의 출력이고 원본 expected basis를 검증한 것이 아니다. GeometryHelpers 제품 코드는 변경하지 않았다.

## G05 최종 확인과 사용자 경계

변경 source 최소 컴파일, shader compile/WARP, JSON parse 및17개 원본 bool 일치/나머지 값 보존, 실제 Catalog/Playback17개, geometry 법선2개, `git diff --check`를 실행했다. Playback probe의 기존 EngineSDK 헤더 인코딩 경고와 link EDITANDCONTINUE/OPT:ICF 경고는 남지만 실행은0으로 종료했다.

사용자는 Product build 후 기존 Server 연결 상태에서 Lobby→Character Select→차원술사로 진입해 T 소환의 푸른 조명 영향과 V 시작 사각형을 확인한다. V 화면은 사각형 발생 시점과 camera/character 이동 여부가 확인되어야 다음 원인 분리가 가능하다. T/V 구현과 수치 통과를 화면 복원 완료로 기록하지 않는다.

공통 반복 방지 원리는 `gotchas.md`와 `렌더링이펙트복원V2.md`에 root가 통합한다. 전체 localSpace 정책을 바꿀 때 camera_view attachment의 원본 Required와 DIRECT/reconstructed 소비자를 분리하고, 소환 unlit은 exact cue/depth/mask를 유지한 MRT 출력 정책으로 제한한다.

## G06 유리 재질의 별도 원본 distortion pass 연결

사용자가 V의 첫 섬광·유리 파편·바닥 문제 범위를 시작 약 2초까지로 명확히 했다. localcrack 재질의 원본 `bUsesDistortion=true`와 별도 distortion PS `9aa5e61191a9654290657484e8c9cae6`를 확인했다. 기존 native66 기본 색 패스가 있어도 distortion 출력은 항상 0이었다. 원본 설치 ShaderCache `EV2LG3OVEH3HGV7THTFFTM7TOKMCC.upk`에서 해당 PS 바이트코드와 실제 serialized shader object를 추출했다.

원본 PS SHA256은 `cb21a8e5eb108ff1f70a60acdd6446d5ee387b922634cd1f5e4d2a096d478c1a`다. shader object의 binding 후보 두 개를 임의 선택하지 않았다. 같은 shader class의 기존 검증된 object 37개(같은 VS `e520045fc771e74d9a67d28b8621c46c` 21개 포함)의 직렬화 배열 위치 152와 대조해 scalar group4 → CB0[1]을 확인했다. 이때 CB0[1].y는 원본 distortion 10, CB0[0].w는 particle dynamic W다.

`Shader_EffectDimensionMasterVNative.hlsli`에 `VNative66Distortion`을 추가하고 native66에서만 기존 distortion MRT에 연결했다. 원본 38개 PS 명령의 시선 방향·5제곱 Fresnel·왜곡 강도·dynamic W·가림 깊이 검사를 옮겼다. 기존 depth Y의 view metres/1000을 cm로 바꾸고, 원본의 양/음 누적 RGBA를 기존 MRT의 signed XY로 전달한다. 원본 distortion discard는 distortion 기여 0으로만 처리하며 기본 유리 색 패스까지 clip하지 않는다. 기본색, 다른 profile, 블렌드와 전역 후처리는 변경하지 않았다.

원본 packed PS와 새 함수의 실제 WARP 출력은 합성 강도 4종, dynamic W 3종, 시선 3종, 깊이 3종의 108조건에서 최대 절대 오차 0이었다. 그중 13조건에서 실제 0이 아닌 distortion 출력이 나와 모두 버리는 검사만 통과한 결과가 아님도 확인했다. Mesh/Particle DimensionMasterV 전체 FXC `fx_5_0` 컴파일도 성공했다. 기존 공통 include의 X4000 경고는 남는다. 결과 CSO는 out에만 저장했고 현재 실행 중 Client의 로드된 shader를 교체하지 않았다.

독립 carrier 검토에서 현재 native66의 실제 세 mesh occurrence가 tangentView와 원본 dynamic W=1을 전달하며, RT1이 float signed RG / One+One으로 합성됨을 확인했다. 다만 최종 `Shader_Deferred::PS_MAIN_SCENE_RESOLVE`의 기존 공통 distortion clamp ±0.05는 유지된다. 이번 원본 PS의 일부 raw 누적값은 이 범위를 넘으므로 원본 PS 출력 일치가 최종 화면의 왜곡 크기까지 동등하다는 뜻은 아니다. 원본 최종 resolve를 확인하지 않은 채 모든 이펙트의 공통 상한을 제거하지 않았다.

유리 메시 `fm_d_crack_037`도 원본 UPK에서 새로 추출해 설치 WModel의 두 section 286/565 triangles 위치·노멀·기본 UV·winding이 일치함을 확인했다. 원본 vertex COLOR0는 설치본에 없지만 해당 기본색 PS가 particle color를 읽으므로 이것을 이번 화면 결함의 원인으로 단정하지 않았다. 카메라 occurrence는 약 0.7028초와 1.5701초로 사용자가 말한 범위에 들어간다.

근거는 `out/DimensionMasterVMaterialReview20260915/distortion_binding_class_closure.json`, `v66-distortion-warp.json`, `distortion-installation.json`, Mesh/Particle FXC log와 `out/DimensionMasterTVReview20260915/Glass/geometry-compare.json`이다. G04의 첫 native69 바닥 사각형과 원본 카메라 basis는 별도 미확정 항목이다. 이번 원본 distortion 연결 성공을 초기 2초 화면 전체 복원 완료로 취급하지 않는다. 새 shader의 제품 빌드·다음 Client 실행 반영과 최종 화면 확인이 남았다.
