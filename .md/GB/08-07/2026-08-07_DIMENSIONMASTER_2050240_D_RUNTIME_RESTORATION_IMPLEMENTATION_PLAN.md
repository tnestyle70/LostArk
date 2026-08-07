# 차원술사 D 2050240 Cascade 런타임 복원 구현 계획

## 1. 목표와 완료 경계

현재 정본은 `D = 2050240 / 경계 돌파`이며 Animation 정본의 순차 clip은
`pc_sp_m_00_sk_sk_telekinesisthrust_01 -> pc_sp_m_00_sk_sk_telekinesisthrust_04`다.
두 번째 clip의 source stage offset `0.5s`도 현재 action recipe와 일치한다. 이번 변경은 이
Animation 계약과 Particle timeline을 고정한 채, 원본 추측 없이 재현 가능한 런타임 결함만 교정한다.

완료 조건은 다음과 같다.

- D complete Effect의 기존 `b_wp_swm_m_2/follow` cue와 element별 snapshot/follow attachment는
  이미 복원된 Animation 계약으로 간주하고 변경하지 않는다.
- D의 mesh particle 8개도 sprite particle과 같은 source material profile, scalar/vector,
  Dynamic Parameter 계약을 소비한다.
- 3 FPS에서도 1초의 wall/action 시간이 지난 뒤 Effect simulation이 구조적으로 뒤처지지 않는다.
- 지원하지 않는 runtime shader profile, Dynamic Parameter semantic, SubUV mode는 publish/load
  stage에서 fail-closed한다.
- 하네스와 결과 문서는 `D=2050240`, `A=2050210`을 명시하고 51개 element 중 실제 GPU
  presentation이 없는 Light 2개와 ScreenPost 3개를 stage PASS와 분리한다.

이번 변경은 원본 Material expression topology를 발명하지 않는다. 현재 D의 21개 material
profile group은 모두 `RECONSTRUCTED_PROFILE`이고 `runtime_exact=0`이므로, generic profile 16개와
Light/Post/Camera/Sound/Afterimage/Visibility는 후속 source 복원 및 수동 A/B 경계로 남긴다.

## 2. 현재 실측

### Animation과 cue 정본

- `Data/Animation/Authored/DimensionMaster/DimensionMaster.skillbindings.json`
  - 2050240은 두 clip을 위 순서로 소유한다.
- `Data/Effects/Imported/DimensionMaster/Converted/skill.2050240.action-cue-recipe.json`
  - source stage 0은 `0.0s`, stage 1은 `0.5s`다.
- `Data/Animation/Authored/DimensionMaster/DimensionMaster.animevents`
  - complete Effect cue는 복원된 Animation 계약인 `b_wp_swm_m_2` follow로 시작한다.
  - 이번 작업은 cue anchor나 animation timing을 다시 조정하지 않는다.

### Material과 renderer

- D는 `46 Particle + 2 Light + 3 ScreenPost`다.
- Particle 46개 중 mesh particle은 8개다.
- `CEffectDocumentRenderer::Bind_MaterialInputs`는 source profile uniform을 particle shader에만
  bind한다.
- `Render_Particles`의 mesh 분기는 `EFFECT_EVALUATED_PARTICLE::vDynamicParameter`를 버린다.
- `Shader_VtxEffectMeshPreview.hlsl`은 source profile 분기 없이 `Shade_Effect`만 호출하고 가짜
  directional light를 곱한다.
- 21개 profile group은 모두 reconstructed이며 generic profile 16개, 32 occurrence는 원본
  opacity/render-state 식이 없다. 불투명 base DDS를 쓰는 6개 sprite는 full-quad alpha가
  수학적으로 남는다.

### 자동 검증의 현재 의미

- executor/runtime harness는 parse, distribution, timeline, stage를 검증하지만 D3D draw pixel은
  검증하지 않는다.
- Light/ScreenPost는 codec과 stage를 통과하지만 renderer가 `S_FALSE`로 건너뛴다.
- startup smoke는 Lobby 생존 검사이며 D를 재생하지 않는다.

따라서 기존 PASS는 변환·게시·stage 계약 PASS이고 GPU 원본 복원 PASS가 아니다.

## 3. 변경 파일

| 파일 | 변경 책임 |
|---|---|
| `Client/Public/Effect_DocumentRenderer.h` | mesh particle draw에 per-particle Dynamic Parameter를 전달하는 private 계약 추가 |
| `Client/Private/Effect_DocumentRenderer.cpp` | mesh shader에도 source profile을 bind하고 mesh particle dynamic 값을 넘김 |
| `Client/Bin/ShaderFiles/Shader_VtxEffectMeshPreview.hlsl` | mesh particle이 `Shade_EffectParticle` 계약을 사용하고 emissive/unlit 기준으로 출력 |
| `Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli` | distortion 0을 강제로 0.01로 올리는 fallback 제거 |
| `Client/Public/Effect_MaterialTemplate.h` | renderer가 실제 지원하는 profile/semantic/SubUV allow-list의 C++ 정본 제공 |
| `Client/Private/Effect_DocumentCodec.cpp` | unknown runtime material 계약을 load/validate stage에서 거부 |
| `Tools/EffectPipeline/Publish-Effects.ps1` | 같은 public JSON 계약을 publish 전에 거부 |
| `Client/Private/Effect_Playback.cpp` | 3 FPS의 20 fixed step을 처리할 수 있도록 bounded catch-up 계약 교정 |
| `Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp` | A/D exact roster, D 표기, unknown material 계약, 3 FPS 동기화 회귀 검사 |
| `Tools/ProjectAudit/Test-EffectToolFinal.ps1` | D의 Particle GPU coverage와 미구현 presentation 수를 분리 보고 |
| 대응 `*_RESULT.md` | 실제 검증과 미완료 Material/Light/Post/Camera/Sound 경계 기록 |

새 C++ 파일은 추가하지 않으므로 `.vcxproj`와 `.vcxproj.filters` 등록 변경은 없다. 공유 작업트리에서
수정 중인 Client project 파일과 Valtan 변경은 건드리지 않는다.

## 4. G별 구현

### G01. D identity와 Animation 계약 보존

`DimensionMaster.animevents`의 2050240 asset cue는 기존 `b_wp_swm_m_2/follow`를 유지한다.
Animation 작업자가 완료한 anchor, clip 순서, timing은 이번 Material/renderer 복원의 수정 대상이
아니다.

하네스는 current roster에서 `A=2050210`, `D=2050240`을 동시에 검사하고 stale 후보
`2050190/2050550`이 current Effect roster에 들어오지 못하게 한다. 과거 `DimensionMaster A` 테스트
문구와 지역 변수도 D로 교정한다.

### G02. Mesh Particle material 실행

`Render_Mesh`에 optional Dynamic Parameter 입력을 추가한다. 일반 mesh element와 afterimage는
identity parameter를 사용하고, `Render_Particles`의 mesh 분기는 각 particle의
`vDynamicParameter`를 넘긴다.

`Bind_MaterialInputs`는 particle shader와 mesh shader에 동일한 source profile/scalar/vector/semantic
uniform을 bind한다. Mesh shader는 uniform Dynamic Parameter와 함께 `Shade_EffectParticle`을 호출한다.
원본 이펙트 material이 emissive/unlit인 현재 계약에 맞춰 임의 directional light는 제거한다.

`one-layer-distortion`은 authoring intensity가 0이면 출력도 0이어야 한다. `max(..., 0.01)` 강제값을
제거해 데이터 계약을 그대로 따른다.

### G03. Material runtime registry fail-closed

지원 profile은 현재 renderer의 여섯 값으로 제한한다.

```text
effect.ue3.reconstructed-standard.v1
effect.ue3.circle.v1
effect.ue3.dot.v1
effect.ue3.ring.v1
effect.ue3.aura.v1
effect.ue3.one-layer-distortion.v1
```

Dynamic Parameter semantic은 `unbound/opacity/emissive/dissolve/uv_pan/distortion/radial_size`,
SubUV mode는 `none/psuvim_linear_blend/psuvim_linear_blend_random_flip_square`만 허용한다.
Codec과 publisher 양쪽이 같은 집합을 검증한다. 잘못된 ID는 실제 draw까지 지연하지 않고 기존
Document/Catalog를 유지한 채 stage를 실패시킨다.

### G04. 3 FPS fixed-step 동기화

현재 프레임당 8 step 제한은 3 FPS에서 `0.333s` 입력 중 `0.133s`만 소비해 매 프레임 지연을
누적한다. bounded catch-up 상한을 1초 분량인 60 step으로 교정하고, 하네스에서 `1/3s` update 세 번과
60 FPS update 60번의 sample time 및 구조 snapshot을 비교한다. debugger 정지처럼 1초를 넘는 backlog는
여전히 bounded 상태로 남기되, 실제 보고된 3 FPS에서는 action/animation clock과 같은 속도로 진행한다.

### G05. 실행 커버리지와 문서 정정

Final audit는 D에 대해 다음을 별도 수치로 낸다.

```text
document elements 51
particle renderer coverage 46
unimplemented light 2
unimplemented screenPost 3
runtime exact material profiles 0
reconstructed material profile groups 21
```

08-06의 2050240 문서는 historical `A` 표기가 현재 canonical D로 대체됐음을 RESULT에 명시한다.
`518 source Modules`는 보존/구조 accounting이며 518개 모두의 pixel-equivalent execution을 뜻하지
않는다고 교정한다.

### G06. 원본 Texture Parameter 역할 보존

canonical R/D의 source receipt와 `Resource_LostArk` Material dump를 emitter별로 교차한다. 현재
importer는 첫 texture를 parameter 의미와 무관하게 `base`로 강제하고, 이미 사용된 semantic slot의
다음 texture를 비어 있는 임의 slot로 넘긴다. 이 때문에 R
`foldcut_rib_01.particlespriteemitter_47`의 `normal_tex`가 Base Color가 되고 `refle_tex`가 Noise가
된다.

명시적인 이름만 다음처럼 분류한다.

```text
normal / bump                         -> noise
mask / opacity / alpha                -> mask
noise / distort                       -> noise
dissolve                              -> dissolve
emissive / glow / bloom               -> emissive
reflection / refle / environment      -> base
main / base / diffuse 및 미분류 layer -> base
```

첫 texture의 base 강제와 중복 semantic의 임의 fallback은 제거한다. base가 없고 mask만 있는
procedural material은 white base + source mask를 사용한다. expression topology를 모르는 복수 layer는
다른 의미의 slot로 가장하지 않고 receipt에 미표현 경계로 남긴다. 이 변경은 profile을
`RUNTIME_EXACT`로 승격하지 않는다.

### G07. 다음 A/B를 위한 Runtime Sample

선택 emitter의 정적 Source Material 정보만으로는 최종 draw 직전 particle alpha와 Dynamic Parameter를
알 수 없다. Playback의 현재 evaluated frame에서 다음 read-only probe를 제공한다.

```text
sample time / active particle count
pre-texture alpha min/max/first
Dynamic Parameter first/min/max
normalized life / raw SubImage index
```

Effect Tool은 선택 Element 아래에 이 값을 transient하게 표시하고 `Copy Runtime Probe`로 한 번만
복사한다. JSON에는 저장하지 않고 shader 결과도 바꾸지 않는다. Tool UI 병행 작업이 같은 파일을
수정 중이면 UI 연결은 merge 뒤로 미루고 Playback/Object probe와 harness만 먼저 닫는다.

## 5. 검증

다음 순서로 실제 실행한다.

```powershell
python -m unittest `
  Tools/LevelPlacementExtractor/test_build_effect_source_material_contract.py `
  Tools/LevelPlacementExtractor/test_build_imported_effect_documents.py `
  Tools/LevelPlacementExtractor/test_effect_extraction_tools.py `
  Tools/LevelPlacementExtractor/test_extract_umodel_material_dependencies.py `
  Tools/LevelPlacementExtractor/test_materialize_dimensionmaster_base_effects.py

powershell -ExecutionPolicy Bypass -File Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate
Tools/ClientFrontendHarness/Bin/Debug/ClientFrontendHarness.exe --skill-binding-fast
Tools/ClientFrontendHarness/Bin/Debug/ClientFrontendHarness.exe --effect-executor-fast
Tools/ClientFrontendHarness/Bin/Debug/ClientFrontendHarness.exe --effect-runtime-fast
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Test-EffectToolFinal.ps1
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
git diff --check
```

Renderer/HLSL 변경이므로 Client x64 Debug 빌드까지 수행한다. 수동 GPU 완료 판정은 canonical D를
동일 camera와 동일 시점으로 캡처해 plane 위치, mesh opacity/dynamic UV, full-quad 6개, Light/Post
미출력을 각각 분리 확인한 뒤에만 기록한다.
