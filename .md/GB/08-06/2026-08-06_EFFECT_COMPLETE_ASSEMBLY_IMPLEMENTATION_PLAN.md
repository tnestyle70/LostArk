# Effect 원본 실행 복원 구현 계획

## 1. 목표

차원술사 기본 캐릭터의 `BA1~BA4, Q, W, E, R, A, S, D, F, T, V`를 원본 UE3 Action/PSA/Cascade/Material 데이터에서 손실 없이 변환하고, 현재 `CEffectCatalog -> CEffectPresentationService -> CEffectObject` 제품 경로에서 실행한다.

완료 판정은 화면이 대략 비슷한 상태가 아니라 다음 계약을 모두 만족한 상태다.

```text
원본 emitter 누락          0
외부 module unresolved     0
unsupported module         0
approximation              0
missing resource           0
미실행 Action cue          0
animation/timeline drift   0
fallback white texture     0
Save/Reload 차이           0
```

PNG는 원본 시간·속도·수명을 추측하는 입력이 아니다. 원본 실행 복구 이후 렌더러·노출·블렌딩 차이를 확인하는 A/B 자료로만 사용한다.

기본 차원술사에는 Z source contract가 없으므로 시간/공간 specialization을 Z로 대체하지 않는다.

## 2. 확정 계층

```text
Skill Effect Assembly
├─ Body Animation Binding
├─ Timeline
├─ Particle Component Cue
├─ Model Cue
├─ Decal/Trail Cue
├─ Light Cue
├─ Camera Cue
├─ Screen Post Cue
└─ Sound Cue

Particle Component
├─ 원본 ParticleSystem identity
└─ Emitters
   ├─ Renderer: Sprite | Mesh | Decal | Ribbon | Light
   ├─ Resource/Material bindings
   └─ Module Stack
      ├─ Spawn/Lifetime
      ├─ Location
      ├─ Velocity/Force/Orbit
      ├─ Size/Rotation
      ├─ Color/Alpha
      ├─ SubUV
      └─ Dynamic Parameter/Event
```

`Particle Component`는 화면의 입자 한 개가 아니다. 원본 Cascade `ParticleSystem` 한 구성요소이며, 내부 Emitter가 실제 입자를 생성한다. 기존 97개 평면 layer를 사용자가 재조립하지 않는다.

## 3. 저장 계약

원본 재생성 가능 데이터와 사용자 보정을 분리한다.

```text
Data/Effects/Imported/...                 원본 정본
Data/Effects/Components/DimensionMaster/ 재사용 WFX component
Data/Effects/Authored/...                 Skill assembly와 override
Client/Bin/DataFiles/Effect/...           publisher가 만든 runtime catalog
```

Component 파일은 JSON 계약을 유지한다.

```text
DimensionMaster_S_00.particle.wfx.json
DimensionMaster_S_01.particle.wfx.json
...
```

완성 스킬 문서는 component asset ID, cue time, anchor, local transform과 override만 소유한다. publisher가 Component를 `parse -> validate -> stage -> compile -> commit`하여 기존 runtime Effect 문서로 병합한다. 중간 실패 시 기존 runtime catalog를 보존한다.

BA aggregate는 Tool 전체 검토용이고 제품 실행은 BA1~BA4 stage 문서를 사용한다.

## 4. Effect Tool UX

### 4.1 All Effects

```text
S | 찰나
├─ Body Animation
├─ Timeline
├─ Particle Components
│  ├─ DimensionMaster_S_00
│  │  ├─ Emitter 00 | Sprite
│  │  └─ Emitter 01 | Mesh
│  └─ DimensionMaster_S_01
├─ Model
├─ Decal/Trail
├─ Light
├─ Camera/Screen Post
└─ Sound
```

- Skill root: 전체 Effect 로드
- Component: 원본 cue time으로 seek하고 해당 구성요소 Solo
- Emitter: Component 내부 해당 Emitter Solo
- Module: Effect를 별도로 만들지 않고 Detail 편집 대상으로 선택

### 4.2 Data Files

- Skill assembly와 WFX component를 구분하여 표시한다.
- WFX 한 행은 Element 하나가 아니라 ParticleSystem component 하나다.
- WFX 선택 시 Emitter/Renderer/Module/Resource/원본 값/runtime 값/override를 표시한다.

### 4.3 동적 Effect Detail

공통 panel은 identity, provenance, coverage, timeline, anchor, transform, render profile을 표시한다. 선택 종류에 따라 ParticleSystem, Emitter, Spawn, Location, Velocity, Force, Size, Rotation, Color, SubUV, Material, Trail, Decal, Light, Camera, Screen Post, Sound editor를 동적으로 구성한다.

지원하지 않는 source class도 삭제하지 않는다. Generic Raw Module 카드에서 literal/distribution/reference를 읽을 수 있게 하고 coverage를 `UNSUPPORTED`로 유지한다.

### 4.4 Resource 영역

상단은 선택한 Renderer/Material의 현재 binding을 가변 슬롯으로 표시한다.

```text
Mesh | Material | Base | Noise | Mask | Emissive | Dissolve |
Normal | SubUV | 원본 named texture parameters ...
```

하단은 선택 class domain의 Mesh/Texture/Material library다. 선택한 상단 슬롯과 호환되는 항목만 bind하며 Mesh도 thumbnail을 제공한다. 고정 Base/Noise/Mask 5칸은 호환 UI로만 유지하고 내부 정본은 원본 parameter 이름이다.

## 5. Runtime 실행 범위

### Particle

- Required/Spawn/Lifetime 전체 distribution과 Burst/Loop/Delay
- Point/Box/Sphere/Cylinder/Circle/Direct/Emitter/Bone/Socket/Skeletal surface/Ground location
- Velocity/Velocity Over Life/Inherit Parent/Acceleration/Orbit/Vortex/Vector Field
- 2D·3D Size, Size Over Life, Sprite·Mesh Rotation, Rotation Rate, Axis Lock, Camera Facing
- Color/Alpha/Color Scale Over Life
- SubUV, Camera Offset, Spawn Per Unit, seeded random
- Ribbon, Event Generator/Receiver

### Material

- Material Instance parent chain
- Texture/Scalar/Vector/Static Switch
- Blend, depth, two-sided
- Panner/Rotator/Fresnel
- Emissive/Distortion/Dissolve/Noise/Mask
- Dynamic Particle Parameter
- texture parameter가 없는 procedural Material expression

### 별도 presentation channel

- animated Model Cue
- Point/Directional Light
- Camera Shake/Control
- RGB Noise/Zoom/FilmNoise/PostProcessChain
- Scene Capture
- Character visibility/material/afterimage
- Sound cue

이 채널을 Particle Element로 가장하지 않는다.

## 6. 구현 순서

1. WFX Component와 Skill Assembly JSON schema, codec, publisher, audit
2. 기존 v10 flat sourceRecipe를 Component/Emitter/Module 계층으로 무손실 변환
3. publisher가 Component를 기존 runtime `EFFECT_DOCUMENT_DESC`로 compile
4. Effect Tool All Effects/Data Files tree와 동적 Detail/Resource UI
5. Cascade module executor와 deterministic parity harness
6. Material graph/runtime parameter 실행
7. Light/Camera/Post/Sound/Character presentation channel
8. 차원술사 11 aggregate + BA1~BA4 runtime 연결
9. Debug/Release build, Effect pipeline, ClientFrontendHarness, ProjectAudit
10. 실제 GPU 재생 후 PNG A/B와 필요한 Authored override

## 7. 검증

- Python: source payload hash, Component 분해/재조립 identity, BA stage rebase, invalid ID/version/path/duplicate/rollback
- Publisher: missing component/resource, source hash mismatch, unsupported channel, 중간 실패 rollback
- C++ harness: distribution/module parity, deterministic seek/restart, Component compile identity, Light/Post command lifetime
- Runtime: body animation과 Effect clock 0초 동기화, T clip chain, BA1~BA4 격리, F Summon 30fps
- Build: Engine 변경 시 Debug/Release Engine -> UpdateLib -> Client
- `Tools/ProjectAudit/Invoke-ProjectAudit.ps1`
- `git diff --check`

수동 GPU 확인은 자동 PASS로 기록하지 않고 RESULT에 별도 항목으로 남긴다.
