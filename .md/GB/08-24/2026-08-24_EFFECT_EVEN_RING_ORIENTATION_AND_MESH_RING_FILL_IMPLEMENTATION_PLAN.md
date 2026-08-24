# Effect Even Ring Orientation 및 Mesh Ring Fill 구현 계획

## 1. 목표

Effect Tool의 수동 Particle 저작 경로에 다음 두 계약을 추가한다.

1. Sprite Particle을 고정 burst cohort 안에서 링 둘레에 균등 배치하고, 각 Sprite의 로컬 자세를 방사/접선 방향으로 독립 회전한다.
2. `fm_d_ring_008.wmodel`처럼 raw UV의 V축이 안쪽에서 바깥쪽 링 단면을 나타내는 수동 Mesh Particle에 고정 크기 Ring Fill을 적용한다. 메쉬 Transform/Scale은 움직이지 않고 재질의 가시 영역만 `0 -> 1`로 공개한다.

기존 문서는 새 필드를 생략하면 `random + fixed orientation + ring fill disabled`로 완전히 같은 동작을 유지한다. SOURCE recipe, reconstructed/source material, standalone Mesh/Sprite, Opaque render profile에는 새 계약을 적용하지 않는다.

## 2. 소유권과 데이터 계약

### 2.1 Sprite Particle 링 배치

`EFFECT_PARTICLE_SPAWN_SHAPE_DESC`가 배치 분포를 소유한다.

```cpp
enum class EFFECT_PARTICLE_SPAWN_DISTRIBUTION : uint8_t
{
    RANDOM,
    EVEN,
    END
};

struct EFFECT_PARTICLE_SPAWN_SHAPE_DESC final
{
    EFFECT_PARTICLE_SPAWN_SHAPE eKind = EFFECT_PARTICLE_SPAWN_SHAPE::POINT;
    EFFECT_PARTICLE_SPAWN_DISTRIBUTION eDistribution =
        EFFECT_PARTICLE_SPAWN_DISTRIBUTION::RANDOM;
    f32_t fRadius = 0.f;
    f32_t fInnerRadius = 0.f;
    float3_t vExtents = { 0.f, 0.f, 0.f };
    f32_t fArcDegrees = 360.f;
};
```

JSON은 non-default일 때 기존 `particle.spawnShape`에 `"distribution": "even"`을 기록한다. EVEN은 이번 실제 소비자에 맞춰 `manual Particle + ring + spawnRatePerSecond=0 + burstCount>=2`로 닫는다. 360도에서는 끝점을 중복하지 않고 `i / N`, 부분 arc에서는 양 끝점을 포함하는 `i / (N - 1)`을 사용한다.

### 2.2 Sprite Particle 개별 자세

`EFFECT_PARTICLE_DESC`가 초기 자세를 별도 소유한다.

```cpp
enum class EFFECT_PARTICLE_ORIENTATION_MODE : uint8_t
{
    FIXED,
    GROUND_RADIAL_OUTWARD,
    GROUND_RADIAL_INWARD,
    GROUND_TANGENT_CLOCKWISE,
    GROUND_TANGENT_COUNTER_CLOCKWISE,
    END
};

struct EFFECT_PARTICLE_INITIAL_ORIENTATION_DESC final
{
    EFFECT_PARTICLE_ORIENTATION_MODE eMode =
        EFFECT_PARTICLE_ORIENTATION_MODE::FIXED;
    f32_t fOffsetDegrees = 0.f;
};
```

JSON은 non-default일 때 `particle.initialOrientation`을 기록한다. non-FIXED는 `manual Sprite Particle + Ring + localSpace=true + billboard=false`만 허용한다. Spawn sampler는 최종 위치에서 `atan2`를 역산하지 않고, 링 azimuth를 sample 결과에 함께 보존하여 initial-position offset과 무관한 자세를 만든다.

### 2.3 Mesh Particle Ring Fill

Mesh 전용 재질 제어는 `EFFECT_MESH_DETAIL_DESC::RingFill`이 소유한다.

```cpp
enum class EFFECT_RING_FILL_DIRECTION : uint8_t
{
    INNER_TO_OUTER,
    OUTER_TO_INNER,
    END
};

struct EFFECT_MESH_RING_FILL_DESC final
{
    bool_t bEnabled = false;
    f32_t fProgress = 1.f;
    EFFECT_RING_FILL_DIRECTION eDirection =
        EFFECT_RING_FILL_DIRECTION::INNER_TO_OUTER;
    f32_t fFeather = 0.05f;
    bool_t bInvert = false;
};
```

JSON은 enabled일 때 `detail.mesh.ringFill`을 기록한다. `progress`와 lerp end는 `[0,1]`, feather는 `[0,0.5]`로 검증한다. `linearLerp.ringFillProgress`가 켜지면 enabled가 필수다. 이 기능은 generic `effect.standard`, manual Mesh Particle, non-Opaque profile에서만 허용한다.

`EFFECT_LINEAR_LERP_DESC`에는 다음 필드를 추가한다. 보간 clock은 fixed burst의 첫 simulation-step 출생 오프셋과 무관하게 완료 wave 경계에서 정확히 1에 도달하도록 Element-local Timing Life를 사용한다.

```cpp
bool_t bRingFillProgress = false;
f32_t fEndRingFillProgress = 1.f;
```

셰이더는 transformed/panned UV가 아니라 mesh carrier의 raw `TEXCOORD0.y`를 사용한다. `INNER_TO_OUTER`는 V low-to-high, `OUTER_TO_INNER`는 high-to-low이고 Invert는 모델 UV 방향 보정이다. `progress == 0`은 완전 비표시, `progress == 1`은 완전 표시로 특수 처리하며 중간 경계에 feather를 적용한다.

```hlsl
float radialV = saturate(rawCarrierUV.y);
if (g_RingFillInvert != 0u)
    radialV = 1.f - radialV;
if (g_RingFillDirection != 0u)
    radialV = 1.f - radialV;

float coverage = 1.f - smoothstep(
    g_RingFillProgress - g_RingFillFeather,
    g_RingFillProgress + g_RingFillFeather,
    radialV);
```

coverage는 Alpha/Additive 양쪽의 SrcAlpha blend에서 중복 감쇠되지 않도록 generic particle shader 최종 SceneColor alpha와 Distortion에 적용하고 낮은 coverage를 clip한다. Source/reconstructed/runtime-material 분기는 enabled validation과 constant 0으로 격리한다.

manual Ring Fill Mesh Particle에는 기존의 암시적 `1 - particleLife` alpha fade를 적용하지 않는다. 완료 링의 alpha는 Color/Lerp로 명시 저작하며, particle lifetime을 Element Timing Life보다 조금 길게 두면 완료 wave와 겹치는 짧은 hold를 만들 수 있다.

## 3. 구현 파일

- `Client/Public/Effect_AuthoringDocument.h`
  - 새 enum/descriptor/default 및 lerp 필드
- `Client/Public/Effect_Playback.h`
  - 링 위치와 azimuth를 함께 반환하는 spawn sample 구조
- `Client/Private/Effect_DocumentCodec.cpp`
  - optional read/write, enum token, validation, backward-compatible omission
- `Client/Private/Effect_Playback.cpp`
  - EVEN cohort angle, particle-local orientation, Ring Fill progress lerp
- `Client/Private/Effect_Tool.cpp`
  - Spawn Distribution, Initial Orientation, Ring Fill, Lerp Ring Fill UI
- `Client/Private/Effect_DocumentRenderer.cpp`
  - evaluated Ring Fill constant binding
- `Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli`
  - neutral-by-default Ring Fill constants와 coverage
- `Tools/EffectPipeline/Publish-Effects.ps1`
  - publisher schema/range/carrier validation
- `Tools/EffectPipeline/Test-EffectPipeline.ps1`
  - positive/negative fixture
- `Tools/EffectRenderContractHarness/Private/EffectRenderContractHarness.cpp`
  - codec default/round-trip/rejection, 16-point ring, orientation basis, lerp 및 WARP draw 회귀

기존 사용자 저작 문서와 runtime catalog는 수정하거나 Publish하지 않는다.

## 4. 실패 소비자와 검증

- 잘못된 enum, NaN/범위, unsupported carrier는 codec/publisher가 문서 admission 전에 거부한다.
- EVEN을 continuous spawn에 사용하거나 non-ring에 사용하면 실패한다.
- 개별 orientation을 billboard/source/mesh particle에 사용하면 실패한다.
- Ring Fill을 source recipe, standalone mesh, Sprite Particle, Opaque profile에 사용하면 실패한다.
- 저장/재로드 후 새 optional 필드가 유지되고, default 문서는 새 JSON을 쓰지 않는다.
- seek/reset 반복 시 같은 fixed burst 링과 같은 개별 자세를 재현한다.

검증 순서:

1. `EffectRenderContractHarness` Debug/Release build 및 실행
2. `Test-EffectPipeline.ps1`
3. `Publish-Effects.ps1 -Mode Validate`만 실행
4. Client x64 Debug Rebuild로 HLSL include 재컴파일
5. `Invoke-BuildAndRegression.ps1 -Configuration Debug`
6. `git diff --check`
7. 사용자가 Effect Tool에서 16개 링, 개별 radial/tangent orientation, Ring Fill 0/25/50/100%, timeline scrub과 저장/재로드를 육안 검증

## 5. 수동 저작 기준

- Sprite ring: Fixed Burst 16, Max Particles 16, Spawn Rate 0, Ring, inner radius=radius, arc 360, distribution Even, local space on, billboard off, desired orientation mode 및 offset.
- Mesh ring fill: manual Mesh Particle, `fm_d_ring_008.wmodel`, fixed burst 1, model pre-scale 0.01, non-Opaque Alpha/Additive profile, Ring Fill on, Progress 0, Direction Inner To Outer, Feather 0.05, Lerp Ring Fill on, End 1.
- 완료 wave: 같은 문서의 별도 element로 `fx_h_wave_01.dds`를 연결하고 Start Delay를 fill duration에 맞춘다. Fill shader가 다른 element를 자동 발생시키는 새 event 경로는 만들지 않는다.

## 6. G04 수동 Mesh Particle Scale 라이브 튜닝 회귀 수정

### 6.1 실측 원인

수동 Mesh Particle의 `Model Import Scale`은 `CModel::Create`에 전달되는 WModel pre-transform이므로 모델 리소스를 다시 준비해야 적용된다. 그러나 authoring `Stage_Document`의 GPU 리소스 재사용 판정인 `Resource_SignatureMatches`가 `Detail.Mesh.fModelPreScale`을 비교하지 않았다. 이 때문에 Effect Detail의 값과 저장 JSON은 바뀌어도 기존 scale로 생성된 `CModel`을 계속 재사용했다.

`Transform > Scaling`과 `Particle > Start/End Size`는 모델 로드 값이 아니라 매 sample의 particle world matrix를 소유한다. 이 두 축은 리소스를 재로딩하지 않고 playback restage만으로 즉시 반영되어야 한다. 같은 증상으로 오인되지 않도록 실제 제출 world matrix가 변하는지를 실행형 하네스에서 고정한다.

또한 새 manual Mesh/Mesh Particle draft가 WModel 단위 보정 `0.01`을 `Transform > Scaling`에 넣고 `Model Import Scale`은 `1`로 두고 있었다. Valtan 저작 계약인 `Model Import Scale=0.01`, `Transform Scale=identity`와 반대이며, 사용자가 Import Scale도 `0.01`로 맞추면 저장/재로드 후 두 값이 곱해져 `0.0001`이 된다. 새 draft만 올바른 축으로 고치고 기존 authored document는 자동 변환하지 않는다.

### 6.2 구현 계약

- `Client/Private/Effect_DocumentRenderer.cpp`
  - Element resource signature에 `Detail.Mesh.fModelPreScale`을 포함한다.
  - Transform scale, particle size, color, timing처럼 GPU resource identity가 아닌 값은 기존 재사용 경로를 유지한다.
- `Client/Private/Effect_Tool.cpp`
  - 새 manual Mesh/Mesh Particle은 `Model Import Scale=0.01`, `Transform Scale=(1,1,1)`로 시작한다.
  - 기존 authored document의 수치는 변경하지 않는다.
- `Tools/EffectRenderContractHarness/Private/EffectRenderContractHarness.cpp`
  - manual Mesh Particle을 동일 `CEffectObject`에 restage한다.
  - Transform scale 변경은 model disk load를 늘리지 않으면서 제출 world matrix를 바꿔야 한다.
  - Model Import Scale 변경은 resource rebuild/model load를 발생시켜야 한다.
  - 다시 같은 Import Scale로 Transform만 바꾸면 model resource를 재사용해야 한다.

### 6.3 검증 및 수동 판정

1. EffectRenderContractHarness Debug 실행
2. Client x64 Debug build
3. `Publish-Effects.ps1 -Mode Validate`
4. `git diff --check`
5. 사용자가 새 Mesh Particle에서 Model Import Scale, Transform Scaling XYZ, Start/End Size를 각각 바꾸고 라이브 프리뷰 및 저장/재로드 결과를 육안 판정한다.
