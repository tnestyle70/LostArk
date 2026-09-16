# 차원술사 T 소환 조명 제외와 V 초기 사각형 진단 구현 계획

기준일: 2026-09-15. 공유 `GB/Rendering-Restore` 작업 트리의 다른 변경은 유지한다.

## G06. 초기 2초 유리 파편의 원본 distortion 연결

사용자가 초기 2초의 바닥과 유리 파편으로 증상을 구체화했다. native66의 localcrack 원본에는
별도 distortion PS `9aa5e61191a9654290657484e8c9cae6`와 VS `e520045fc771e74d9a67d28b8621c46c`가 있다.
현재 base-color PS와 독립적으로 누락된 이 38명령 패스를 같은 native66의 Distortion MRT에 연결한다.
원본 shader object의 두 배열 후보는 동일 직렬화 shader class의 기존 검증 37개 모두의 배열 시작
152와 대조해 scalar group4로 식별했다. 이 중21개는 같은 VS다. cb0[1].y는 원본 distortion scalar,
cb0[0]은 기존 같은 VS distortion 소비 경로의 particle dynamic parameter를 사용한다.
깊이는 기존 native mesh distortion의 centimetre view-depth 변환을 재사용한다.
원본 distortion discard는 왜곡 기여만0으로 만들고 기존 base color를 지우지 않는다.
원본 DXBC와 후보 pixel shader를 동일 수치 입력으로 WARP 대조하고, 실제 Effect shader를 컴파일한다.
바닥 sprite의 전체 사각형 원인이나 사용자 화면 동등성까지 이 변경으로 확정하지 않는다.

## G00 현재 연결과 변경 경계

`PlayerSkills.json`의 T=2050500과 V=2050520은 각각 DimensionPrison과 TimeWave clip에 연결된다. animevents는 T의 `effect.dimensionmaster.skill.2050500.unified`, V의 `effect.dimensionmaster.skill.2050520.full.restore`를 재생한다. T `dimension_summon` cue는 설치된 `Character/DimensionMaster/DimensionMaster_DimensionSummon.wmodel`을 CModel 골격 경로로 그린다. V 현재 HEAD/작업 트리는 43 elements이며 과거 RESULT의 64 elements와 다르므로 현재 저작 요소 선택을 보존한다.

T는 정확한 문서/cue/model/clip/MASKED 식별로 NONBLEND GBuffer stage에 한 번 제출되지만 일반 animated pass0의 diffuse/specular를 출력하여 맵 조명의 영향을 받는다. 사용자 요청에 따라 이 소환 cue만 unlit 출력으로 바꾼다. 이것은 원본 조명 모델 복원을 주장하는 변경이 아니다. skinning, mask threshold0.3, depth write와 제출 stage는 유지한다.

V에는 이미 occurrence마다 최신 SceneHDR snapshot을 갱신하는09-10 수정이 있다. 현재 DIRECT_AUTHORED_DOCUMENT 소비자에서 camera_view 17개의 detail localSpace가 false이지만 동일 요소의 원본 Required buselocalspace는 true다. 09-14 전체 캐릭터 localSpace 해제 정책 이후 현재 요청에 필요한 V camera occurrence만 예외 복구한다. 초기0초 cam_01의 native69 두 요소도 포함된다. 첫 사각형의 최종 원인은 이 카메라 추적 결함과 분리하고, native shader/EPAL_Z 원본 basis가 확정되지 않은 방향 보정은 넣지 않는다.

## G01 수정 파일과 함수

| 파일 | 변경 |
|---|---|
| `Client/Private/Effect_DocumentRenderer_Rendering.cpp` | `Render_ModelCues`의 이미 존재하는 exact T predicate가 animated pass11을 선택한다. |
| `Client/Bin/ShaderFiles/Shader_VtxAnimMeshBinary.hlsl` | `PS_MAIN_EFFECT_MODEL_CUE_UNLIT`와 마지막 pass11을 추가한다. 기존0~10pass ID를 유지한다. |
| `Data/Effects/Authored/effect.dimensionmaster.skill.2050520.full.restore.effect.json` | 현재 43개 중 `camera_view` 17개에 한해 Required 원본 `buselocalspace=true`를 `detail.particle.localSpace`에 복구한다. |

unlit PS는 기존 `Evaluate_Material(input,true,0.3,frontFace)`로 coverage와 재질을 읽고 diffuse radiance를 emissive carrier로 전달한다. diffuse RGB, normal alpha specular mask, material specular와 source character metadata를0으로 만든다. depth xy와 pick XYZ는 유지하고 marker는 legacy0으로 남겨 모든 맵 direct/ambient/specular의 색 기여가0이 된다. 새 shader uniform이 없어서 공유 CShader의 다음 draw에 정책이 누출되지 않는다.

새 C++ 파일과 public 구조체/enum은 없다. 기존 등록 파일만 수정하므로 vcxproj/filter 추가는 필요 없다.

## G02 검증과 사용자 화면 확인

변경 source만 out 격리 C++ 최소 컴파일, 실제 FXC의 shader compile을 수행한다. exact T 선택 및 pass0~10 보존을 확인하고, 가능한 최소 WARP 진단으로 두 map-light 색에서 unlit GBuffer/radiance가 불변인지를 확인한다. V는 실제 Catalog Capture→Stage와 current CEffectPlayback으로 17개가 카메라 1m 이동을 따라가는지 이전 false 대조군과 비교한다. Required 원본 true와 기타 JSON 값의 완전 보존을 검사한다. 초기 cam_01 두 요소의 실제 Make_ParticleSpriteWorld 법선도 확인하고, source expected basis가 확인되지 않으면 사각형 해결로 기록하지 않는다. UTF-8 JSON parse와 `git diff --check`를 수행한다.

Product build, Client/UI 실행과 캡처는 하지 않는다. 사용자가 빌드 후 Lobby→Character Select→차원술사에서 T 소환의 푸른 조명 잔존과 V 처음 사각형을 최종 판정한다. source 수치 검증과 화면 통과를 RESULT에서 분리한다.
