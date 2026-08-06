# Effect 원본 실행 복원 상세 코드 계획

## G30. WFX Component 데이터 계약

### 수정·추가 파일

- `Client/Public/Effect_AuthoringDocument.h`
- `Client/Public/Effect_ComponentDocument.h`
- `Client/Private/Effect_ComponentCodec.cpp`
- `Client/Public/Effect_ComponentCodec.h`
- `Tools/LevelPlacementExtractor/build_effect_components.py`
- `Tools/EffectPipeline/Publish-Effects.ps1`
- `Data/Effects/Components/DimensionMaster/**`
- `Client/Default/Client.vcxproj`
- `Client/Default/Client.vcxproj.filters`

`EFFECT_COMPONENT_DESC`는 component asset ID, type, source identity, renderer별 Emitter와 원본 순서의 Module을 소유한다. `EFFECT_COMPONENT_CUE_DESC`는 component ID, start time, duration, anchor, transform, override만 소유한다.

codec은 stable ID, version, Resources-relative path, duplicate emitter/module, finite distribution, renderer/resource 호환성을 검증한다. parse 실패 시 출력 문서를 변경하지 않는다.

publisher는 모든 component를 먼저 stage하고 dependency DAG를 검증한 뒤 Skill assembly에 compile한다. compile 결과만 runtime catalog에 commit한다.

## G31. Particle Emitter/Module 실행

### 수정 파일

- `Client/Public/Effect_Playback.h`
- `Client/Private/Effect_Playback.cpp`
- `Client/Public/Effect_Distribution.h`
- `Client/Private/Effect_Distribution.cpp`
- `Engine/Public/VIBuffer_ParticleRect.h`
- `Client/Bin/ShaderFiles/Shader_VtxEffectParticle.hlsl`

`PARTICLE_STATE`는 position/base velocity/current velocity/acceleration/size/rotation/color/camera offset/SubUV/dynamic parameter/orbit payload/seed와 spawn root를 보존한다. Module은 source order로 Spawn과 Update phase를 실행한다. 동일 class가 여러 번 있어도 첫 항목만 찾지 않는다.

Emitter time은 delay, duration, loop, legacy emitter time을 구분한다. random은 stable component/emitter/particle seed로 결정하여 Restart/Seek가 동일하다.

Engine instance buffer는 DynamicParameter, SubUV frame, axis/camera-facing 정보를 shader에 전달한다. Engine public header 변경 후 UpdateLib와 Client를 모두 검증한다.

## G32. Material Instance와 procedural Material

### 수정·추가 파일

- `Client/Public/Effect_MaterialTemplate.h`
- `Client/Private/Effect_DocumentRenderer.cpp`
- `Client/Public/Effect_SourceMaterial.h`
- `Client/Private/Effect_SourceMaterial.cpp`
- Effect material shader files
- Material extraction Python/harness

`sourceMaterialPath`를 정본 identity로 사용하고 부모 chain과 named parameter를 보존한다. 흰 텍스처 fallback은 preview 완료로 판정하지 않는다. expression 지원 전에는 coverage가 `UNSUPPORTED_SOURCE_MATERIAL_EXECUTION`이고 제품 publish exact mode는 거부한다.

## G33. Light/Camera/Post/Sound channel

### 수정·추가 파일

- `Client/Public/Effect_PresentationChannel.h`
- `Client/Private/Effect_PresentationChannel.cpp`
- `Client/Public/Effect_DocumentRenderer.h`
- `Client/Private/Effect_DocumentRenderer.cpp`
- `Client/Public/Effect_Object.h`
- `Client/Private/Effect_Object.cpp`
- Action cue converter와 publisher

각 channel은 cue start/end, root/anchor follow 정책, source payload와 typed parameter를 보존한다. Stage에서 필요한 shader/model/sound resource를 모두 준비한 뒤 commit한다. 종료 시 이전 camera/post/light/character material 상태를 복원한다.

Sound는 기존 Audio service를 통해 재생하며 Effect 코드가 FMOD 경로를 직접 소유하지 않는다. Camera/Post는 제품 camera와 render pipeline의 typed command 경계를 사용한다.

## G34. Effect Tool 계층·동적 Detail·Resource UI

### 수정 파일

- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_Tool.cpp`

선택 identity는 `Skill/Component/Emitter/Module/Cue` stable ID다. vector index를 저장 ID로 사용하지 않는다.

All Effects는 assembly tree, Data Files는 assembly/component file tree를 표시한다. Component와 Emitter audition은 기존 preview를 복제하지 않고 같은 `CEffectObject` stage 경로를 사용한다.

Detail registry는 source class를 typed editor 함수에 매핑한다. 공통 필드와 kind/module 전용 필드를 분리하며 원본 값, runtime 평가값, override, coverage를 같은 카드에 표시한다.

Resource 상단은 현재 binding을 material/renderer metadata에서 동적으로 만든다. 하단 palette는 class domain과 resource kind로 필터링하며 기존 thumbnail cache를 재사용한다.

## G35. 차원술사 기본 runtime materialization

### 데이터

- BA aggregate 1개와 BA1~BA4 stage 4개
- Q/W/E/R/A/S/D/F/T/V aggregate
- Z 없음: `NO_BASE_SOURCE_CONTRACT`
- 시간/공간 specialization 미포함

Action cue 452개와 Cascade module 20,922개가 모두 typed executor에 연결되어야 exact다. Component assembly compile 전후의 emitter/module/distribution/literal/source hash를 대조한다.

## G36. Harness와 완료 판정

- Component codec 정상/잘못된 version/ID/path/duplicate/rollback
- flat v10 -> WFX -> compiled runtime identity
- source module order와 duplicate module 실행
- seeded distribution Restart/Seek determinism
- BA stage별 다른 component set
- Light/Post/Camera/Sound start/end cleanup
- procedural Material exact coverage
- Debug/Release Effect pipeline와 ClientFrontendHarness
- ProjectAudit와 `git diff --check`

`EXACT`만 완료다. `PARTIAL`, `APPROXIMATION`, `UNSUPPORTED`, `MISSING_RESOURCE`, `DEFAULT_NEEDS_TUNING`은 모두 미완료로 남긴다.
