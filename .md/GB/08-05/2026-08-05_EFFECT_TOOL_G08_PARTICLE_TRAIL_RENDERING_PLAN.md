# Effect Tool G08 Particle·Trail·AfterImage와 실제 렌더링 계획

## G08-00. 이 문서의 기준

- 이전 단계: [G07 레퍼런스 워크스페이스와 월드 Preview](2026-08-05_EFFECT_TOOL_G07_REFERENCE_WORKSPACE_WORLD_PREVIEW_PLAN.md)
- 코드 설명 형식 정본: [G06 Shader와 남은 반영 가이드](2026-08-05_EFFECT_TOOL_G06_SHADER_REMAINING_GUIDE.md)
- 다음 단계: [G09 Effect admission·runtime·DimensionMaster](2026-08-05_EFFECT_TOOL_G09_RUNTIME_DIMENSIONMASTER_PLAN.md)
- 원본 자산 조사 근거: [차원술사 이펙트 추출 결과](../08-03/2026-08-03_DIMENSIONMASTER_EFFECT_EXTRACTION_RESULT.md)

G08의 본질은 G07의 같은 `EFFECT_DOCUMENT_DESC → Effect playback → Effect renderer` 경로에
Particle, Trail, AfterImage를 실제 구현하는 것이다. Particle/Trail을 Rect 한 장으로 대신하지 않는다.
현재 Renderer에 이미 있는 `Target_SceneHDR`, `Target_Distortion`, Bloom chain을 사용하며 같은 역할의
두 번째 HDR/Bloom renderer를 만들지 않는다.

## G08-01. 종료 화면과 종료 증거

G08이 끝나면 Effect Tool에서 다음 결과가 월드에 실제로 나타난다.

```text
Particle  spawn rate/burst/lifetime/velocity/acceleration/size/color로 여러 instance가 움직인다.
Trail     움직이는 root 또는 bone의 시간별 위치를 이어 ribbon geometry가 생긴다.
AfterImage Mesh/Sprite의 과거 transform copy가 정해진 간격과 수명으로 사라진다.
HDR       emissiveIntensity > 1의 RGB가 clamp되지 않고 Bloom으로 번진다.
Distortion noise/base UV offset이 Target_Distortion에 기록되어 최종 scene을 굴절시킨다.
Decal     scene depth로 복원한 실제 object/ground 표면에 project되고 빈 공중 Rect로 보이지 않는다.
```

같은 Document, 같은 random seed, 같은 sample time은 Tool seek와 순차 재생에서 같은 Particle 배치를 만든다.

## G08-02. Authoring schema v5

`EFFECT_AUTHORING_FORMAT_VERSION`은 `5u`로 올리고 최소 지원 version은 `3u`를 유지한다.
v3/v4를 읽으면 새 G08 block은 아래 default로 stage한다. 저장하면 v5로 기록한다.

기존 `EFFECT_TRANSFORM_DESC`에는 레퍼런스의 `Velocity_Start`에 대응하는
`float3_t vVelocityPerSecond = {0.f, 0.f, 0.f};`를 추가한다. 이 값은 Effect root 기준 local
meter/second이며 Particle 개별 velocity와 다른, Element 전체의 이동 속도다.

### `EFFECT_LINEAR_LERP_DESC`

Element lifetime의 normalized time `0..1`에서 시작값과 끝값을 선형 보간한다.

| 필드 | 단위와 의미 |
|---|---|
| `bPosition` | true면 `Transform.vPosition → vEndPosition`을 보간한다. |
| `vEndPosition` | Effect root 기준 local meter |
| `bRotation` | true면 시작 rotation degree → end degree를 보간한다. |
| `vEndRotationDegrees` | local Euler degree |
| `bRevolution` | true면 시작 revolution degree/second → end revolution을 보간한다. |
| `vEndRevolutionDegreesPerSecond` | local degree/second |
| `bScale` | true면 시작 scale → end scale을 보간한다. |
| `vEndScale` | 배율; 각 축은 0보다 커야 한다. |
| `bVelocity` | true면 `Transform.vVelocityPerSecond → vEndVelocityPerSecond`를 보간한다. |
| `vEndVelocityPerSecond` | Element 전체의 local meter/second |
| `bColorOffset` | true면 시작 color offset RGBA → end offset을 보간한다. |
| `vEndColorOffset` | shader에 더하는 signed RGBA offset |
| `bColorMultiply` | true면 시작 multiply RGBA → end RGBA를 보간한다. |
| `vEndColorMultiply` | HDR를 위해 RGB는 1보다 클 수 있고 alpha만 0..1이다. |
| `bEmissiveIntensity` | true면 시작 emissive → end emissive를 보간한다. |
| `fEndEmissiveIntensity` | 0 이상 HDR 배율 |

이 단계는 두 점 linear lerp만 소유한다. arbitrary curve/key editor를 먼저 만들지 않는다.
velocity가 선형으로 변할 때 위치 offset은 매 frame delta 누적값이 아니라 normalized lifetime의 해석식으로
계산해 Seek와 순차 재생이 같게 한다.

```text
velocity(t) = startVelocity + (endVelocity - startVelocity) * t
positionOffset(t) = lifetime * (startVelocity * t + 0.5 * (endVelocity - startVelocity) * t²)
```

### `EFFECT_PARTICLE_DESC`

| 필드 | default | 유효 조건 |
|---|---:|---|
| `iMaxParticles` | 256 | 1..2048 |
| `fSpawnRatePerSecond` | 20 | 0..2048 |
| `iBurstCount` | 0 | 0..2048, maxParticles 이하 |
| `iRandomSeed` | 1 | 0은 허용하지 않고 같은 값은 같은 결과 |
| `vLifeTimeSeconds` | `{0.5, 1.0}` | min > 0, max >= min, max <= 30 |
| `vInitialVelocityMin` | `{-0.5, 1.0, -0.5}` | local meter/second |
| `vInitialVelocityMax` | `{0.5, 2.0, 0.5}` | 각 축 max >= min |
| `vAcceleration` | `{0,-1,0}` | local meter/second² |
| `vStartSize` | `{0.2,0.2}` | meter, 두 축 > 0 |
| `vEndSize` | `{0,0}` | meter, 두 축 >= 0 |
| `bLocalSpace` | true | true면 root 이동을 계속 따라가고 false면 spawn world 위치를 유지 |
| `bBillboard` | true | camera right/up으로 Rect orientation 계산 |

`fSpawnRatePerSecond == 0`이고 `iBurstCount == 0`이면 유효하지만 아무 입자도 생기지 않는 명시적 설정이다.

### `EFFECT_TRAIL_DESC`

| 필드 | default | 의미 |
|---|---:|---|
| `iMaxPoints` | 64 | 2..256 history point cap |
| `fPointLifeTimeSeconds` | 0.35 | 한 점이 history에 남는 시간 |
| `fSampleIntervalSeconds` | 0.0167 | root/anchor를 다시 채집하는 최소 시간 |
| `fMinimumDistance` | 0.01 | 너무 가까운 중복 point를 막는 meter |
| `fStartWidth` | 0.2 | 새 point의 ribbon 폭 |
| `fEndWidth` | 0 | 수명 끝 point의 ribbon 폭 |
| `bFaceCamera` | true | view direction을 이용해 ribbon 양쪽 vertex를 만든다. |

레퍼런스의 `- / + Trail Vertex` UI는 별도 의미가 불분명한 배열을 만들지 않고 `iMaxPoints`를
한 단계씩 줄이거나 늘리는 편집 버튼으로 연결한다. 현재 사용 중인 point/생성 vertex 수는 옆에 read-only로 표시한다.

Trail은 Element의 local transform 뒤에 현재 effect root를 곱한 위치를 sample한다. bone 이름은 Effect asset이
소유하지 않는다. 어떤 bone을 root로 공급할지는 G09 Animation cue가 소유한다.

### `EFFECT_AFTERIMAGE_DESC`

기존 `Timing.fAfterImageSeconds`는 copy 하나의 수명으로 유지하고 다음 값만 추가한다.

| 필드 | default | 의미 |
|---|---:|---|
| `fSampleIntervalSeconds` | 0.05 | 과거 transform을 복사하는 간격 |
| `iMaxCopies` | 16 | 0..32, 0이면 기능 off |
| `fAlphaExponent` | 1 | 수명 normalized 값에 적용하는 fade 곡률 |

AfterImage는 Mesh와 Sprite만 지원한다. Particle/Trail/Decal에 값이 들어오면 Save/Load validation이 거부한다.

### `EFFECT_DETAIL_DESC` 연결

기존 `Transform`, `Color`, `UV`, `Timing`, `Mesh`, `Sprite`, `Decal` 뒤에 다음 block을 추가한다.

```text
EFFECT_LINEAR_LERP_DESC LinearLerp;
EFFECT_PARTICLE_DESC Particle;
EFFECT_TRAIL_DESC Trail;
EFFECT_AFTERIMAGE_DESC AfterImage;
```

### 레퍼런스 Effect Detail 라벨과 저장 필드

| 화면 라벨 | v5 저장 필드 또는 typed 상태 |
|---|---|
| `Lerp Position / Position_Start` | `Transform.vPosition`, `LinearLerp.bPosition`, `vEndPosition` |
| `Lerp Rotation / Rotation_Start` | `Transform.vRotationDegrees`, `bRotation`, `vEndRotationDegrees` |
| `Lerp Revolution / Revolution_Start` | `Transform.vRevolutionDegreesPerSecond`, `bRevolution`, end revolution |
| `Lerp Scaling / Scaling_Start` | `Transform.vScale`, `bScale`, `vEndScale` |
| `Lerp Velocity / Velocity_Start` | `Transform.vVelocityPerSecond`, `bVelocity`, end velocity |
| `Lerp ColorOffset / ColorOffset_Start` | `Color.vColorOffset`, `bColorOffset`, `vEndColorOffset` |
| `Color Clip` | `Color.fColorClip` |
| `Color Mul` | `Color.vColorMultiply` |
| `Bloom Intensity` | 기존 `Color.fEmissiveIntensity`; UI label만 레퍼런스와 맞춘다. |
| `Distortion Intensity` | `Color.fDistortionIntensity` |
| `Distortion On Base Material` | `Color.bDistortionOnBaseMaterial` |
| `Radial Time / Radial Intensity` | `Color.fRadialTime`, `Color.fRadialIntensity` |
| `UV Start / UV Speed` | `UV.vStart`, `UV.vSpeed` |
| `UV_Wave` | `UV.bWave`, amplitude, frequency |
| `IsSequence / IsLoop` | `UV.bSequence`, `UV.bLoop` |
| `UV_TileCount` | `UV.iTileColumns`, `UV.iTileRows` |
| `UV_TileIndex` | `UV.iTileIndex` |
| `Life Time` | `Timing.fLifeTimeSeconds` |
| `Start Delay Timer` | `Timing.fStartDelaySeconds` |
| `After Image Timer` | `Timing.fAfterImageSeconds`와 `AfterImage` block |
| `Dissolve Start` | `Timing.fDissolveStartNormalized` |
| `Billboard` | `Sprite.bBillboard` 또는 `Particle.bBillboard` |
| `Select Pass / Pass Name` | arbitrary string이 아니라 `EFFECT_RENDER_PROFILE`; `AlphaBlend` 등은 표시 label |
| `Trail Vertex -/+` | `Trail.iMaxPoints` 편집과 active point/vertex 진단 |

따라서 화면 라벨을 맞추더라도 Pass Name을 임의 문자열로 저장하거나 `BloomIntensity`라는 중복 필드를 만들지 않는다.

## G08-03. v5 JSON 모양

각 Element의 기존 `detail` object 안에 다음 네 object가 항상 저장된다. kind와 무관한 block도 default를
명시해 Save/Load 결과가 Document order나 parser fallback에 따라 바뀌지 않게 한다.

```json
{
  "linearLerp": {
    "position": false,
    "endPosition": [0.0, 0.0, 0.0],
    "rotation": false,
    "endRotationDegrees": [0.0, 0.0, 0.0],
    "revolution": false,
    "endRevolutionDegreesPerSecond": [0.0, 0.0, 0.0],
    "scale": false,
    "endScale": [1.0, 1.0, 1.0],
    "velocity": false,
    "endVelocityPerSecond": [0.0, 0.0, 0.0],
    "colorOffset": false,
    "endColorOffset": [0.0, 0.0, 0.0, 0.0],
    "colorMultiply": false,
    "endColorMultiply": [1.0, 1.0, 1.0, 1.0],
    "emissiveIntensity": false,
    "endEmissiveIntensity": 1.0
  },
  "particle": {
    "maxParticles": 256,
    "spawnRatePerSecond": 20.0,
    "burstCount": 0,
    "randomSeed": 1,
    "lifeTimeSeconds": [0.5, 1.0],
    "initialVelocityMin": [-0.5, 1.0, -0.5],
    "initialVelocityMax": [0.5, 2.0, 0.5],
    "acceleration": [0.0, -1.0, 0.0],
    "startSize": [0.2, 0.2],
    "endSize": [0.0, 0.0],
    "localSpace": true,
    "billboard": true
  },
  "trail": {
    "maxPoints": 64,
    "pointLifeTimeSeconds": 0.35,
    "sampleIntervalSeconds": 0.016667,
    "minimumDistance": 0.01,
    "startWidth": 0.2,
    "endWidth": 0.0,
    "faceCamera": true
  },
  "afterImage": {
    "sampleIntervalSeconds": 0.05,
    "maxCopies": 16,
    "alphaExponent": 1.0
  }
}
```

## G08-04. 추가·수정 파일과 의존 관계

### Engine 새 파일

| 파일 | 한 줄 책임 |
|---|---|
| `Engine/Public/VIBuffer_ParticleRect.h` | dynamic particle instance buffer public 계약 |
| `Engine/Private/VIBuffer_ParticleRect.cpp` | Rect geometry와 `VTXINSTANCE_PARTICLE` array를 `WRITE_DISCARD`로 GPU에 올린다. |
| `Engine/Public/VIBuffer_DynamicTrail.h` | 매 frame 달라지는 ribbon vertex/index buffer 계약 |
| `Engine/Private/VIBuffer_DynamicTrail.cpp` | CPU가 만든 trail vertices를 dynamic VB/IB에 stage하고 draw한다. |

### Client 새 파일

| 파일 | 한 줄 책임 |
|---|---|
| `Client/Public/Effect_Playback.h` | Document 시간 평가와 particle/trail/afterimage runtime state 계약 |
| `Client/Private/Effect_Playback.cpp` | fixed-step simulation, seek/replay, evaluated frame 생성을 담당 |
| `Client/Bin/ShaderFiles/Shader_VtxEffectParticle.hlsl` | particle instance를 billboard 또는 local Rect로 그린다. |
| `Client/Bin/ShaderFiles/Shader_VtxEffectTrail.hlsl` | ribbon vertex의 UV/color를 SceneHDR/Distortion으로 출력한다. |
| `Client/Bin/ShaderFiles/Shader_VtxEffectDecal.hlsl` | scene depth의 world position을 decal local box로 역변환해 실제 표면에 투영한다. |

### 기존 파일

| 파일 | 이번 G 변경 |
|---|---|
| `Engine/Public/Engine_Struct.h` | `VTXTRAIL` input layout을 추가하고 기존 particle instance struct를 그대로 재사용한다. |
| `Client/Public/Effect_AuthoringDocument.h` | v5와 네 Detail block을 추가한다. |
| `Client/Public/Effect_DocumentRenderer.h` | evaluated particle/trail/afterimage frame과 projected decal draw 함수 추가 |
| `Client/Private/Effect_DocumentRenderer.cpp` | Particle/Trail buffer draw와 scene-depth Decal projection 연결 |
| `Client/Public/Effect_Object.h` | `CEffect_Playback`을 instance state로 소유한다. |
| `Client/Private/Effect_Object.cpp` | Update에서 playback을 진행하고 Render에서 evaluated frame을 그린다. |
| `Client/Public/Effect_Tool.h` | Particle/Trail/AfterImage panel session copy 추가 |
| `Client/Private/Effect_Tool.cpp` | kind별 detail editor와 v5 Save/Load/validation 추가 |
| `Shader_VtxEffectMeshPreview.hlsl` | HDR RGB clamp 제거와 distortion MRT output 추가 |
| `Shader_VtxEffectRectPreview.hlsl` | HDR RGB clamp 제거와 distortion MRT output 추가 |
| `Engine/Engine.vcxproj`, `.filters` | 새 Engine H/CPP 네 개 등록 |
| `Client/Default/Client.vcxproj`, `.filters` | playback H/CPP와 shader 등록 |
| `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | v5 block, buffer 등록, shader MRT 계약 검사 |

## G08-05. `Effect_Playback.h` 계약

### 이 파일이 소유하는 상태

`CEffect_DocumentRenderer`는 GPU draw만 소유한다. 살아 있는 Particle, trail history, afterimage history는
Effect instance마다 달라지므로 `CEffect_Playback`이 소유한다.

```text
EFFECT_PARTICLE_STATE
  position/velocity/age/lifetime/size/random 값을 갖는 살아 있는 입자 하나

EFFECT_TRAIL_POINT
  world position/sample age/width를 갖는 ribbon history point 하나

EFFECT_AFTERIMAGE_SAMPLE
  과거 world matrix/sample age를 갖는 copy 하나

EFFECT_EVALUATED_ELEMENT
  renderer가 이번 frame에 그릴 Mesh/Rect/Particle/Trail instance array

EFFECT_EVALUATED_FRAME
  Document Elements와 같은 stable ID 순서로 평가된 draw packet
```

### public 함수

| 함수 | 실제 호출자와 책임 |
|---|---|
| `Stage_Document` | `CEffectObject`; 새 Document의 runtime state를 local stage 후 commit |
| `Reset` | Tool Reset/새 gameplay spawn; clock과 모든 history를 0으로 초기화 |
| `Update(delta, rootWorld)` | 순차 재생; 1/60초 fixed step으로 simulation 진행 |
| `Seek(sampleTime, rootWorld)` | Tool scrub; 0에서 deterministic replay해 같은 상태 복원 |
| `Get_Frame` | Document renderer가 읽는 immutable evaluated frame 반환 |
| `Is_Finished` | 모든 Element lifetime과 살아 있는 history가 끝났는지 반환 |

### 시간 불변식

- simulation fixed step은 `1/60`초다.
- 누적 초를 직접 비교하지 않고 integer simulation step count로 목표 step을 계산한다.
- 한 frame catch-up은 최대 8 step이며 초과 시간은 버리지 않고 accumulator에 남긴다.
- Tool이 과거 시간으로 이동하면 `Reset → fixed-step replay`한다.
- `iRandomSeed + stable Element ID hash`로 emitter seed를 만든다.
- random 결과에 pointer, vector index, 현재 wall clock을 사용하지 않는다.

## G08-06. Particle 내부 흐름

### `Emit_Particles`

한 줄 책임: spawn accumulator와 burst 규칙으로 이번 fixed step에 태어날 입자를 결정한다.

```text
Element local time가 0을 처음 통과
→ burstCount만큼 spawn 시도
→ spawnRate * fixedDelta를 accumulator에 누적
→ 정수 부분만 spawn count로 소비
→ maxParticles cap까지 state 생성
→ seed RNG로 lifetime/velocity를 min-max 사이에서 선택
```

### `Update_Particles`

한 줄 책임: 살아 있는 입자의 velocity, position, age, size/color를 한 fixed step 진행한다.

```text
velocity += acceleration * dt
→ position += velocity * dt
→ age += dt
→ normalizedAge = age / lifetime
→ size/color lerp
→ age >= lifetime인 입자 제거
→ localSpace 여부에 따라 instance world matrix 계산
```

## G08-07. Trail 내부 흐름

### `Sample_Trail`

한 줄 책임: current effect root가 시간·거리 조건을 넘었을 때 history point 하나를 추가한다.

```text
sampleInterval 경과 확인
→ Element local * rootWorld에서 current position 계산
→ 마지막 point와 minimumDistance 비교
→ 조건 충족 시 point push
→ age > pointLifetime 제거
→ maxPoints 초과 시 가장 오래된 point 제거
```

### `Build_TrailGeometry`

한 줄 책임: history point마다 좌우 vertex를 만들어 하나의 triangle strip용 index array로 변환한다.

```text
인접 point 방향 계산
→ camera-facing이면 cross(viewDirection, tangent)으로 side 계산
→ age normalized로 width와 alpha 계산
→ left/right vertex와 누적거리 기반 U 생성
→ DynamicTrail buffer Update
```

point가 2개 미만이면 draw하지 않는다. 0 길이 tangent는 이전 유효 방향을 사용하고 처음부터 0이면 skip한다.

## G08-08. AfterImage 내부 흐름

```text
Mesh/Sprite Element가 활성이고 afterImageSeconds > 0, maxCopies > 0
→ sampleInterval마다 현재 Element world matrix 저장
→ sample age 증가
→ age / afterImageSeconds로 alpha fade 계산
→ 만료 copy 제거
→ 원본 Element 뒤에 오래된 copy부터 draw
```

AfterImage는 GPU texture나 model을 다시 load하지 않고 원본 Element resource cache를 공유한다.

## G08-09. GPU buffer 계약

### `CVIBuffer_ParticleRect`

- 기존 `CVIBuffer_Instance`와 `VTXINSTANCE_PARTICLE_RECT` input layout을 재사용한다.
- instance buffer는 `D3D11_USAGE_DYNAMIC + D3D11_CPU_ACCESS_WRITE`다.
- `Update_Instances`는 count가 capacity를 넘으면 실패하며 조용히 잘라내지 않는다.
- `Map(WRITE_DISCARD) → memcpy → Unmap` 뒤 `m_iNumInstances`를 commit한다.

### `CVIBuffer_DynamicTrail`

- `VTXTRAIL`은 `position`, `texcoord`, `color`만 갖는다.
- vertex/index 최대치는 validator의 `iMaxPoints`에서 계산한다.
- stage 중 vertex나 index 생성 실패 시 이전 GPU count를 0으로 만들고 잘못된 geometry를 재사용하지 않는다.

Engine public header가 바뀌므로 `Engine Debug/Release → UpdateLib Debug/Release → Client` 순서를 지킨다.

## G08-10. HLSL 출력 계약

### HDR

G06 shader의 최종 `return saturate(color)`는 RGB까지 1로 잘라 Bloom 에너지를 없앤다. G08에서는 다음 규칙으로 교체한다.

```text
output.SceneColor.rgb = max(color.rgb, 0.0);
output.SceneColor.a = saturate(color.a);
```

`emissiveIntensity`는 RGB에 곱하며 1보다 큰 값이 `Target_SceneHDR`에 그대로 남아야 한다.
Bloom은 기존 `CRenderer::Render_Bloom`이 SceneHDR에서 추출한다.

### Distortion

Effect pixel shader output은 두 target을 갖는다.

```text
SV_Target0  SceneHDR color
SV_Target1  signed distortion RG offset, BA는 0
```

`bDistortionOnBaseMaterial`이면 base texture RG, 아니면 noise texture RG를 `[-1,1]`로 바꾼 뒤
`fDistortionIntensity`를 곱한다. distortion이 0이면 정확히 `{0,0,0,0}`을 쓴다.
최종 scene sampling과 tone mapping은 기존 `Shader_Deferred.hlsl`의 FINAL pass 하나만 담당한다.

### Projected Decal

`Shader_VtxEffectDecal.hlsl`은 full-screen `CVIBuffer_Rect`를 그리되 모든 pixel을 쓰지 않는다.

```text
Target_Depth sample
→ 기존 Deferred와 같은 ProjInverse/ViewInverse 계산으로 world position 복원
→ world position * inverse(decal element world matrix)
→ local X/Z가 Decal.vSize 절반, local Y가 Decal.fDepth 절반 안인지 검사
→ 바깥 pixel clip
→ local X/Z를 0..1 UV로 변환
→ Base/Noise/Mask/Emissive/Dissolve와 Color/UV/Pass 적용
→ SceneHDR와 Distortion MRT 출력
```

`Target_Depth`는 GBuffer가 끝난 뒤 read-only SRV로 bind한다. Effect renderer는 새로운 depth renderer를 만들지 않는다.
off-screen 단독 Preview에는 scene depth가 없으므로 Decal의 정답 화면은 가운데 Model View이며,
작은 진단 Preview는 `World depth required`를 표시한다.

## G08-11. Tool panel 배치

`Effect Detail`의 공통 Transform/Color/UV/Timing 아래에 kind별 panel만 표시한다.

```text
Mesh/Sprite  Linear Lerp + AfterImage
Particle     Linear Lerp + Particle
Trail        Linear Lerp + Trail
Decal        Linear Lerp + Size/Depth + Projected Surface 진단
```

UI buffer를 움직이는 동안 Active Document를 매 pixel 변경하지 않는다. local `StagedDetail`을 편집하고
`Apply Detail`에서 전체 validation과 playback stage가 성공한 뒤 Element copy를 commit한다.

## G08-12. validation과 실패 보존

Save/Load/Apply는 다음을 검사한다.

- 숫자가 finite인지, min/max와 cap이 맞는지
- Particle resource slot에 Base Texture가 있는지
- Trail resource slot에 Base Texture가 있는지
- Decal resource slot에 Base Texture가 있고 size/depth가 양수인지
- AfterImage가 Mesh/Sprite에만 사용되는지
- random seed가 0이 아닌지
- total `maxParticles` 합이 Document당 8192 이하인지
- total trail points가 Document당 2048 이하인지
- total afterimage copies가 Document당 256 이하인지

실패 시 Active Document, current playback, GPU buffer, world Effect를 전부 기존 상태로 유지하고
Element ID와 field 이름을 포함한 이유를 표시한다.

## G08-13. 구현 순서

```text
G08-1  v5 struct/default/Save/Load/validation
G08-2  Effect_Playback fixed-step과 deterministic seek
G08-3  ParticleRect Engine buffer와 Particle HLSL
G08-4  DynamicTrail Engine buffer와 Trail HLSL
G08-5  AfterImage history와 shared model/texture draw
G08-6  Target_Depth world reconstruction 기반 projected Decal HLSL
G08-7  Mesh/Rect/Particle/Trail/Decal의 SceneHDR + Distortion MRT 출력
G08-8  레퍼런스 Lerp/Color/UV/Timing/Pass/Trail Vertex panel과 Apply rollback
G08-9  budget, ProjectAudit, build, runtime smoke
```

## G08-14. project/filter 등록

Engine은 새 H/CPP 네 개를 현재 VIBuffer Header/Source 필터에 등록한다. Client는
`Effect_Playback.h/.cpp`를 기존 Effect Tool 필터에, Particle/Trail/Decal HLSL 세 개를 기존 Shader 항목 근처에 등록한다.
물리 폴더가 정본이며 기존 항목의 필터를 옮기지 않는다.

## G08-15. 검증

### 자동 검증

```powershell
msbuild Engine/Default/Engine.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64
.\UpdateLib.bat Debug
msbuild Client/Default/Client.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64
msbuild Engine/Default/Engine.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64
.\UpdateLib.bat Release
msbuild Client/Default/Client.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
git diff --check
```

### deterministic harness cases

```text
같은 v5 Document + seed를 30/60/144 FPS delta로 2초 평가 → particle state hash 동일
2초 순차 재생과 Seek(2.0) → evaluated frame hash 동일
wrong version / NaN / min>max / cap 초과 / kind 불일치 → load 거부, 이전 frame 유지
buffer update 중간 실패 → instance count 0 또는 이전 committed snapshot 유지
```

### 수동 smoke

```text
Particle: burst 32와 spawn rate 50이 여러 Rect instance로 보임
Trail: DimensionMaster root/bone 이동을 따라 ribbon이 이어지고 멈추면 수명 뒤 사라짐
AfterImage: moving Mesh copy가 maxCopies를 넘지 않고 fade
HDR: emissive 1과 8의 Bloom 차이가 분명함
Distortion: intensity 0에서는 변화 없음, 양수에서는 effect 주변만 scene 굴절
Decal: thumbnail click 뒤 지면과 object 표면에 projection되고 camera를 움직여도 같은 world 영역에 유지
Save/Discard/Load 뒤 seed와 결과 재현
```

## G08-16. G08에서 하지 않는 것

- 459개 legacy Cascade 후보를 자동 변환하거나 미지원 module을 조용히 버리지 않는다.
- UE3 VectorField, CameraOffset, arbitrary material expression을 발명하지 않는다.
- fullscreen RGB split/zoom blur와 Alt+V camera sequence는 별도 범위다.
- gameplay effect spawn과 Animation cue는 G09 전까지 제품 완료로 부르지 않는다.
