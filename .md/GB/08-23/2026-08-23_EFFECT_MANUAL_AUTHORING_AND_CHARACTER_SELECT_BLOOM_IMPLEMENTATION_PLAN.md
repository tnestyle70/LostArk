# 2026-08-23 Effect 수동 저작과 Character Select Bloom 구현 계획서

## 목표

도화가 F의 Product/저작 데이터는 유지하고, 나머지 캐릭터와 Valtan Effect를 사용자가 Effect Tool에서 직접 손튜닝할 수 있는 공통 작업면을 만든다. 저작 화면을 가리던 세 개의 실험용 Canary 실행 경로는 전용 런타임·셰이더·도구까지 제거한다.

```text
기존 SourceRecipe Element
  원본 module/curve 유지
  Effect Detail에서 실제로 소비되는 authored overlay와 source multiplier만 편집

새 Manual Element
  Create Element로 생성
  Effect Detail의 spawn/lifetime/start-end size/motion을 직접 편집

Character Select
  Valtan과 같은 scene Bloom multiplier로 네 캐릭터 Effect의 HDR halo를 비교
```

## 현재 실측

- 전역 Bloom은 활성화되어 있고 base intensity는 `2.6099999`다.
- Character Select의 `bloomIntensityMultiplier`는 `0`, Valtan은 `0.85`다.
- generic Effect shader는 RGB를 1에 clamp하지 않고 FP16 Scene HDR에 전달한다.
- Effect Detail의 `Bloom Intensity`는 scene Bloom 세기가 아니라 Emissive DDS의 HDR 가산 세기다.
- SourceRecipe Particle은 source module이 spawn, particle lifetime, motion, size와 DynamicParameter를 소유한다.
- `Detail.Particle.SourceScale` 중 Count/Size/Life/Rotation은 source 평가 결과에 직접 적용된다. Speed와 Alpha는 source base 값을 조정하지만 뒤의 absolute Velocity/ColorOverLife module이 최종값을 다시 소유할 수 있다.
- SourceScale의 SpawnDelay 값은 portable scheduler가 소비하지 않으므로 working control로 노출하면 안 된다. Element의 clip 배치는 `Timing > Start Delay`가 소유한다.
- 현재 Effect Detail은 상위 `Size`와 `Advanced Authoring > Type Detail`에서 같은 Model Import Scale, Start/End Size, Decal Size를 중복 표시한다. SourceRecipe에서도 먹지 않는 Start/End Size를 상위에서 편집할 수 있어 UI와 runtime 소유권이 불일치한다.

## G01. Character Select Bloom 검증 기준

수정 파일:

- `Data/Rendering/Authored/RenderingProfiles.json`
- publisher가 생성하는 `Client/Bin/DataFiles/Rendering/RenderingProfiles.runtime.json`

변경:

- revision `8 -> 9`
- `scene.character-select.warm-high-key.v1.bloomIntensityMultiplier`를 `0 -> 0.85`
- Character Select의 light, exposure, shadow profile은 유지한다.

C++과 map shader는 수정하지 않는다. `LevelRegistry`는 이미 Character Select profile을 선택하고 `CRenderingProfileService`가 `global bloom intensity * scene multiplier`를 계산한다.

## G02. Effect Detail 수동 저작 작업면 통일

수정 파일:

- `Client/Private/Effect_Tool.cpp`
- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_DocumentCodec.cpp`
- `Client/Private/Effect_Playback.cpp`
- `Tools/EffectPipeline/Publish-Effects.ps1`
- `Tools/EffectPipeline/Test-EffectPipeline.ps1`
- `Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py`
- `Tools/EffectRenderContractHarness/Private/EffectRenderContractHarness.cpp`

변경:

1. SourceRecipe 안내를 “아래 값 전체 무시”에서 실제 소유권 설명으로 교정한다.
2. SourceRecipe Mesh/Sprite/Decal carrier의 상위 Size 영역은 먹지 않는 Start/End Size 대신 실제 runtime이 소비하는 Source Playback Tuning 6축(Count/Size/Life/Speed/Rotation/Alpha)을 한 번만 표시한다. SpawnDelay는 노출하지 않는다.
3. Manual Particle만 Start/End Size와 authored spawn/lifetime/motion 입력을 표시한다.
4. Model Import Scale, Decal Size/Depth, Particle Start/End Size, Trail Start/End Width의 Type Detail 중복 표시를 제거한다.
5. compiler-owned SourceRecipe에서는 먹지 않는 raw spawn/lifetime/motion/DynamicParameter 입력을 숨기고, 실제로 소비되는 Max Particles, Random Seed, Local Space, sprite Billboard/Roll, Target Attractor와 resource/color/transform/timing overlay는 유지한다.
6. SourceRecipe를 자동으로 끄거나 source module stack을 변조하지 않는다. 도화가 F 전용 material/program/attachment 경로도 변경하지 않는다.
7. Count 배율을 문서 입자 예산과 event queue 상한에도 반영하고, Life 배율을 declared preview tail 계산에도 반영한다. Tool preview duration은 source emitter delay/duration/loop와 scaled particle tail을 runtime과 같은 순서로 계산한다. source module의 실제 lifetime 분포를 새 값으로 오인하지 않고 authored declared tail 경계로만 사용한다.
8. Effect Detail의 `Bloom Intensity`는 `Emissive Intensity (HDR)`로 이름을 바꾸고, scene Bloom과 별개라는 설명을 표시한다.
9. Source Decal은 runtime이 무시하는 `Detail.Decal.vSize`를 숨기고 source `Size x`와 실제 overlay인 Projection Depth만 표시한다.

## G03. 저작용 Canary 실행 경로 퇴역

범위:

- Effect Tool의 `Exact Cooked Canary`
- `Translated Glasshole02 Canary`
- `Translated Valtan Core-Three Canary`
- 위 세 UI가 전용으로 호출하던 Effect Object/Document Renderer 실행 상태와 draw 분기
- 전용 bridge/wrapper shader, Valtan canary runtime, runtime-canary materializer/replay/test/receipt 및 프로젝트 등록

원칙:

- UI만 숨기지 않고 호출자부터 backend와 전용 asset까지 제거한다.
- Product Effect 경로, generic source-material/profile 경로, 공용 DDS/WModel, offline source 증거와 번역 근거는 유지한다.
- 도화가 F의 material-program/attachment/preview 격리 경로는 변경하지 않는다.

## 검증

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File `
  Tools/RenderingPipeline/Publish-RenderingProfiles.ps1 -Mode Validate

powershell -NoProfile -ExecutionPolicy Bypass -File `
  Tools/RenderingPipeline/Publish-RenderingProfiles.ps1 -Mode Publish

powershell -NoProfile -ExecutionPolicy Bypass -File `
  Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate `
  -ResourceRoot Client/Bin/Resources

powershell -NoProfile -ExecutionPolicy Bypass -File `
  Tools/EffectPipeline/Test-EffectPipeline.ps1

python -B -m unittest `
  Tools.EffectPipeline.test_effect_tool_valtan_saved_rows `
  Tools.EffectPipeline.test_valtan_model_view_composition

powershell -NoProfile -ExecutionPolicy Bypass -File `
  Tools/EffectPipeline/Test-EffectDataProjectRegistration.ps1

powershell -NoProfile -ExecutionPolicy Bypass -File `
  Tools/EffectRenderContractHarness/Run-EffectRenderContractHarness.ps1 `
  -Configuration Debug -ExpectedBindingCount 171

powershell -NoProfile -ExecutionPolicy Bypass -File `
  Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug

git diff --check
```

사용자 수동 검증:

1. Character Select 재진입 또는 F1 Rendering Workbench `Reload Runtime`.
2. 같은 Mesh/Sprite Particle에서 Bloom ON/OFF가 core 색은 유지하고 halo만 바꾸는지 확인.
3. SourceRecipe Element에서 Count/Size/Life multiplier가 재시작 뒤 반영되는지 확인.
4. Manual Element에서 Start/End Size가 독립적으로 반영되는지 확인.
5. 도화가 F가 기존 화면과 동일하게 유지되는지 확인.
6. Effect Detail에서 세 Canary panel과 중복 Size/Type control이 사라졌는지 확인.

자동 검증은 visual fidelity PASS를 대신하지 않는다.
