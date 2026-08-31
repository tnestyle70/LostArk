# 차원술사 유리·물 파티클 12경로 Visual Lab 구현 계획

기준일: 2026-08-31

작업 브랜치: `codex/dimensionmaster-vfx-ten-paths`

격리 작업 트리: `C:\Users\user\Desktop\CodexWorkTree\LostArk-dimensionmaster-vfx-lab`

계획 성격: 원본 데이터 복원이 아닌 `PROJECT_TUNED_APPROX` 프로젝트 저작 시각 실험

최종 화면 판정자: 사용자

## 0. 결론

현재 보이는 원형은 shader 상수가 부족해서가 아니라 **원형 billboard가 형상을 소유하고 있고,
유리에는 조각별 3축 운동이 없으며, 물에는 입자별 궤적 꼬리가 없기 때문**이다. 원형 카드 위에
crack, Fresnel, refraction 수식을 아무리 추가해도 결과는 정교한 원형 카드다.

이번 작업은 `CB0[22]`, `CB2[4]`, `t0..t7`을 복원하거나 기존 차원술사 F/W 문서와 물 opcode
1003을 다시 조합하지 않는다. 그 증거는 다음 현상을 식별하는 **추론 자료**로만 쓴다.

```text
원본 evidence                     프로젝트에서 얻을 의미
t0 crack normal                  균열 방향 왜곡과 표면 normal
t1 atypical mask                 비대칭 coverage와 edge 분포
t2 scene depth                   교차부 soft fade/contact edge
t3 inner hole                    깨진 막의 aperture/내부 공백
t4 dust                          미세 glint와 파편 밀도
t5 environment                   굴절·반사 색
t6 aura                          rim/emissive 보조층
t7 environment overlay           시선 의존 반사 overlay
CB0/CB2                          GPU 운반 ABI의 증거; Tool 조절 이름이 아님
```

Effect Tool에는 raw register나 100개 상수를 노출하지 않는다. 사용자가 직접 조절할 것은
`Shape`, `Motion`, `Surface`, `Timing` 네 그룹의 의미 파라미터이며 **한 candidate 화면에는 최대
12개**만 보인다. 한 번에 모든 조합을 시험하지 않고, 고정 seed의 제한된 preset sweep으로 약
24~30회 육안 비교 후 탈락시킨다.

## 1. 현재 코드·데이터에서 확인한 사실

### 1.1 현재 원형이 나오는 이유

- 차원술사 F 2050230의 실제 mesh carrier는 `fm_m_sphere_004.wmodel`,
  `fm_d_helix_017.wmodel` 중심이고 나머지는 sprite다.
- 차원술사 W clip2의 8개 element는 전부 sprite다.
- W clip3의 mesh 4개도 같은 `fm_m_helix_011.wmodel`을 사용하며 나머지는 sprite다.
- 직전 물 canary는 6개 billboard particle이고 particle별 history나 trail owner가 없다.
- `fx_d_fragment_005.dds`, `fx_a_fragment_001.dds`, `fx_o_glass_01.dds`의 alpha는 전 픽셀
  255이며 실제 파편/균열 정보는 RGB에 있다. generic alpha coverage로 읽으면 full card가 된다.
- 기존 물 program은 carrier UV 중심의 구면 normal/Fresnel을 만들기 때문에 carrier가 원형이면
  최종 silhouette도 원형이다.

### 1.2 새 실험에 쓸 수 있는 미사용 geometry oracle

`Client/Bin/Resources/Effect/DimensionMaster/Meshes/fx_m_glass_01.wmodel`은 현재 authored Effect에서
참조되지 않는다. 실측 결과 198 vertices, 522 indices, 174 triangles이며 triangle adjacency가
24개의 disconnected geometry island로 나뉜다. 이 파일이 F/W 원본 carrier라고 단정하지 않지만,
“진짜 불규칙 mesh silhouette가 문제를 해결하는가”를 가장 싸게 판별하는 신규 canary로 쓴다.

Resource pack은 Git에 넣지 않는다. 실험 문서에는 다음처럼 Resources-relative ID만 저장하며,
물리 파일은 팀 Drive 관리 위치에 둔다.

```text
Effect/DimensionMaster/Meshes/fx_m_glass_01.wmodel
Effect/DimensionMaster/Textures/FX_TEX_HIGH_03/fx_h_brokenglass_02_1.dds
Effect/DimensionMaster/Textures/FX_TEX_HIGH_03/fx_h_brokenglass_11_1.dds
Effect/Valtan/Meshes/FX_SM_01/fm_m_trail_002.wmodel
Effect/Valtan/Textures/FX_TEX_05/fx_m_spatter_001_xyclamp.dds
Effect/Valtan/Textures/FX_TEX_03/fx_e_fluid_006.dds
```

Valtan trail mesh는 칼날처럼 읽힐 위험이 있으므로 빠른 탈락 후보일 뿐 기본 채택안이 아니다.

격리 worktree의 `Client/Bin/Resources`에는 이 Drive-owned payload가 없다. P0 구현/validator/build와
사용자 실행은 원본 작업 폴더의 실제 pack을 명시적으로 가리킨다. 파일을 worktree나 Git으로 복제하지
않는다.

```powershell
$env:LOSTARK_RESOURCE_ROOT = 'C:\Users\user\Desktop\LostArk\Client\Bin\Resources'
```

명시한 root에서 위 여섯 exact ID의 물리 파일을 모두 `Test-Path`로 확인한 뒤 canary를 stage한다.

### 1.3 기존 경로 중 재사용할 것과 만들지 않을 것

재사용한다.

- `CAnimationEffectCueDocument -> CEffectPresentationService` 제품 cue 경계
- `CModel -> CMaterial` 통합 model 경로
- `EFFECT_DETAIL_DESC::Particle`, `Trail`의 authoring/codec 경계
- `CEffectPlayback`의 fixed-step particle simulation
- `CEffectDocumentRenderer`의 Mesh/Sprite/Particle/Trail draw adapter
- `CModel::Render_Instanced`
- `CEffect_Tool::Render_Detail`, draft preview, Apply/Save rollback
- 현재 compiled shader closure와 WARP probe

만들지 않는다.

- Character 전용 두 번째 Effect manager/runtime
- EffectV2를 Character에 병렬로 붙이는 우회 경로
- class/skill ID를 switch하는 전용 renderer
- 임의 HLSL path나 shader graph bytecode를 JSON에서 로드하는 구조
- raw `CB0[22]`, `CB2[4]`, `t0..t7` 편집 UI
- 12개 실험을 동시에 F에 겹쳐 재생하는 cue

## 2. 시각 문제를 네 층으로 분리한다

```text
Shape   : silhouette와 실제 두께 — 원인가, 파편인가, 물방울인가
Motion  : 위치·속도·회전·궤적 이력 — 깨지는가, 날아가는가, 따라오는가
Surface : normal·Fresnel·흡수·굴절·edge — 유리/물 재질로 읽히는가
Timing  : 막 형성, 파열, 비행, splash, 소멸 — 사건의 인과가 읽히는가
```

Surface는 Shape를 대체할 수 없다. 따라서 모든 실험은 flat unlit coverage부터 통과해야 normal,
Fresnel, refraction을 켠다.

### 2.1 공통 운동식

파편과 물방울의 중심 운동은 fixed step에서 다음 ballistic 식을 기준으로 한다.

```text
p(t + dt) = p(t) + v(t) * dt
v(t + dt) = (v(t) + a * dt) * exp(-drag * dt)
```

유리 mesh의 회전은 한 축 roll이 아니라 quaternion 누적으로 처리한다.

```text
speed = |omega|
q(t + dt) = speed < epsilon
  ? q(t)
  : normalize(axisAngle(omega / speed, speed * dt) * q(t))
World     = Scale(aspect, thickness) * Rotate(q) * Translate(p)
```

같은 seed와 같은 fixed-step 시간에서 같은 결과가 나와야 Tool scrub과 자동 harness가 일치한다.
zero angular velocity도 quaternion NaN 없이 기존 방향을 유지해야 한다.

### 2.2 공통 유리 표면식

실제 mesh tangent frame을 기준으로 detail normal을 섞는다. `[0,1]` normal texture를 world normal과
직접 섞지 않는다.

```text
detailTS.xy = (normalSample.xy * 2 - 1) * normalStrength
detailTS.z  = sqrt(saturate(1 - dot(detailTS.xy, detailTS.xy)))
combinedTS  = BlendRNM(baseNormalTS, normalize(detailTS))
N           = normalize(TBN * combinedTS)
NoV = saturate(abs(dot(N, V)))
F  = F0 + (1 - F0) * pow(1 - NoV, fresnelPower)
T  = exp(-absorptionRGB * thickness)
RT0.rgb  = bodyTint * T
RT0.rgb += environmentSample * F
RT0.rgb += edgeColor * pow(1 - NoV, edgePower) * edgeGain
RT0.a    = coverage
RT1.xy   = clamp(N.xy * refractionStrength * thickness,
                 -maximumDistortion, maximumDistortion)
```

Effect material은 RT0에 local radiance/coverage를, RT1에 bounded signed distortion만 기록한다. 실제
scene color sample과 굴절 합성은 기존 Deferred distortion resolve가 소유한다. 첫 canary에서는
RT1도 끄고 edge/environment DDS만으로 material 가설을 판별한다. Scene depth는 기존 frame resource를
안전하게 bind할 수 있을 때만 renderer-owned system input이며, environment DDS는 필요한 candidate의
typed texture lane이다.

### 2.3 공통 물 표면식

```text
waterTS.xy = (normalSample.xy * 2 - 1) * normalStrength
waterTS.z  = sqrt(saturate(1 - dot(waterTS.xy, waterTS.xy)))
Nwater = normalize(TBN * BlendRNM(baseNormalTS, normalize(waterTS)))
F      = F0 + (1 - F0) * pow(1 - saturate(dot(Nwater, V)), fresnelPower)
T      = exp(-absorptionRGB * thickness)
RT0.rgb = bodyTint * T + highlightColor * F * highlightGain
RT0.a   = coverage
RT1.xy  = clamp(Nwater.xy * refractionStrength * thickness,
                -maximumDistortion, maximumDistortion)
foam   = smoothstep(foamThreshold, foamThreshold + foamWidth, curvatureOrBreakup)
```

물의 정체성은 이 식보다 먼저 head/tail silhouette에서 나온다.

## 3. Effect Detail 의미 파라미터 계약

### 3.1 저장 책임

- carrier/resource는 기존 `ResourceBindings`가 소유한다.
- 형상과 운동은 `EFFECT_DETAIL_DESC::Mesh/Sprite/Particle/Trail`의 typed field가 소유한다.
- 시간은 `Detail.Timing`과 particle/trail lifetime field가 소유한다.
- 표면은 `Material.Execution`의 semantic named parameter가 소유한다.
- `packedIndex`와 register는 HLSL transport ABI이며 Tool 화면에 노출하지 않는다.
- Tool은 네 그룹으로 함께 보여주되 저장 owner는 섞지 않는다.

현재 `CEffect_Tool::Render_AuthoringMaterialParameters`는 모든 scalar/vector를
`-100000..100000` 범위로 노출한다. project-tuned program에는 다음 metadata를 가진 profile별 control
descriptor를 추가한다.

```text
stable semantic name
group: Shape | Motion | Surface | Timing
label / tooltip / unit
minimum / maximum / step / default
scalar or color/vector kind
closed target kind
```

`closed target kind`는 문자열 JSON path가 아니라 `CARRIER_SHAPE`, `PARTICLE_MOTION`,
`ELEMENT_TIMING`, `MATERIAL_SCALAR`, `MATERIAL_VECTOR` 같은 닫힌 enum이다. material semantic name을
packed lane으로 바꾸는 책임은 기존 material program/layout registry에 남기며 visual schema가 lane
번호를 복제하지 않는다. UI와 `CEffectDocumentCodec` validator가 같은 descriptor를 소비한다.
unknown name, duplicate name, range 밖 값, NaN/Inf, packet count 불일치는 fail closed하고 기존
preview/document를 유지한다. Tool은 schema를 그려 staged mutation만 제출하고 renderer는 준비된
packed snapshot만 소비한다. runtime에서 semantic string을 매 frame lookup하지 않는다.

### 3.2 사용자에게 보일 최소 조절 항목

유리 Shape/Motion:

```text
burstCount, lifetimeMinMax, radialSpeedMinMax, lift, gravity, drag
aspectMinMax, thicknessMinMax, angularVelocityXYZMinMax
```

유리 Surface:

```text
tint, fresnelPower, edgeGain, edgePower, absorption
roughness, refractionStrength, crackScale, dissolveEdge
```

물 Shape/Motion/Tail:

```text
headRadius, stretchBySpeed, maxStretch, transverseCompression
historySeconds, sampleInterval, tailWidthRatio, taperExponent
breakupTime, disconnectSpeed
```

물 Surface:

```text
tint, absorption, fresnelPower, rimGain
normalStrength, refractionStrength, foamThreshold, foamWidth
```

한 후보 화면에는 실제 필요한 6~12개만 보인다. 사용하지 않는 lane은 숨기고 packet에도 싣지 않는다.

## 4. 12개 canary

모든 canary는 기존 F/W cue에 연결하지 않은 별도 authored Effect ID로 시작한다. 동일 seed, 동일 카메라,
동일 시간 scrub에서 하나씩 또는 최대 3×3 grid로 비교한다.

G1, G5, G6, G7, G8, G9, G12는 서로 다른 carrier/material 가설이다. G2/G4는 “개별 shard”의
instancing/WPO backend A/B이고 G3는 G2와 같은 runtime에 넣는 “원본 split 대 procedural content”
asset 생성 A/B다. G10/G11은 같은 history trajectory의 ribbon/tube 단면 A/B다. 즉 12개를 전부
서로 다른 renderer라고 과장하지 않고, 시각 가설과 구현 backend 가설을 함께 비교한다.

### G1. 24-island rigid glass cluster

가설: 원형 문제의 1차 원인은 shader가 아니라 carrier다.

- `fx_m_glass_01.wmodel` 전체를 하나의 MeshParticle로 렌더한다.
- flat coverage에서 불규칙 외곽이 즉시 읽히는지 본다.
- 그 뒤 3축 회전, Fresnel edge, 약한 refraction을 순서대로 켠다.
- 장점: 기존 `CModel -> CMaterial` 경로로 가장 싸게 geometry 가설을 검증한다.
- 한계: 24 island가 하나의 rigid body라 폭발 후 개별 파편으로 흩어지지 않는다.
- 탈락: 100~300ms 구간에 비원형 plate silhouette가 읽히지 않거나 한 덩어리 장식물로만 읽힌다.
- 우선순위: P0, 첫 glass canary.

### G2. connected-island split + instanced shard bank

가설: 실제 24 island를 개별 rigid shard로 분리하면 최종 유리 파편감을 얻을 수 있다.

- offline resource tool에서 component를 최대 4~6개 shape/material cohort로 묶은 shard bank를 만든다.
- particle별 stable spawn serial, position, quaternion, float3 scale/aspect를 instance payload로 만든다.
- `CModel::Render_Instanced(iMeshIndex, ...)`는 동일 CModel의 동일 mesh만 묶으므로 cohort당 한 draw로
  제출한다. 서로 다른 8~24 WModel을 한 draw로 간주하지 않는다.
- 기존 `VTXMESHINSTANCE`에는 World/InvTranspose만 있으므로 color/dynamic/life가 필요하면 Effect 전용
  bounded instance payload/input layout과 instanced mesh shader를 같은 변경에서 추가한다.
- 큰 shard 5~7개와 작은 shard 20~35개를 별도 density/lifetime로 조합할 수 있다.
- 장점: 실제 parallax, 두께, 3축 tumble이 생긴다.
- 한계: Drive resource 전달과 Effect용 instance ABI가 필요하다.
- 탈락: G1보다 silhouette/운동 개선이 작거나 draw/instance 경계가 안정적으로 닫히지 않는다.
- 우선순위: P1, glass 최종 유력안.

### G3. seeded Voronoi extruded shard bank — G2 content A/B

가설: 원본 resource와 무관한 procedural geometry가 더 제어 가능한 파편 비율을 준다.

- 2D Voronoi cell을 생성해 3~7각형으로 정리하고 얇게 extrude/bevel한다.
- 12~20개 seed bank를 offline 생성하여 WModel로 cook한다.
- aspect, thickness, bevel, cell irregularity를 generator 입력으로 제한한다.
- 장점: `PROJECT_TUNED_APPROX` 방향이 명확하고 F/W 화면에 맞춘 aspect 제어가 쉽다.
- 한계: 균일한 cell은 값싼 polygon confetti로 보일 수 있다.
- 탈락: 세 shape preset이 모두 종이 조각/꽃가루로 읽힌다.
- 우선순위: P2.

### G4. single cluster per-island WPO fracture

가설: island ID를 vertex에 보존하면 한 draw에서 각 조각을 분리할 수 있다.

- cook 단계에서 island ID와 shard pivot을 UV2/COLOR 또는 전용 vertex lane에 bake한다.
- VS에서 `hash(islandId, seed)`로 initial translation, axis, angular velocity를 결정한다.
- 위치는 G4에서만 별도 무감쇠 식을 쓰지 않고 공통 fixed-step drag recurrence로 CPU/typed packet에
  준비한다. VS는 준비된 shard transform으로 `p' = pivot + R_i(p-pivot) + offset_i`만 수행한다.
- 장점: 24개 조각이 한 draw에서 deterministic하게 분리된다.
- 한계: 현재 VTXMESH가 필요한 island metadata를 끝까지 보존하는지 확인하고 계약을 확장해야 한다.
- 탈락: 새 vertex ABI가 G2 instancing보다 복잡하면서 성능 이득이 작다.
- 우선순위: P2 연구.

### G5. triangle Geometry Shader shard burst

가설: 별도 shard asset 없이 primitive 단위 각진 silhouette를 빠르게 만들 수 있다.

- impact proxy mesh의 triangle마다 primitive seed를 만든다.
- GS에서 triangle 중심 기준 translation/rotation과 edge barycentric를 출력한다.
- 현재 Effect pass의 GS가 NULL이므로 실제 G5 구현은 전용 GS input/output, pass binding, CSO closure와
  state rollback을 한 변경에서 닫아야 한다. 기존 mesh shader에 GS가 있다고 가정하지 않는다.
- 장점: asset generator 없이 완전히 다른 pipeline 가설을 검증한다.
- 한계: triangle 단위라 얇고 떨리는 종이처럼 보이기 쉽고 D3D11 GS 비용과 디버깅 부담이 크다.
- 탈락: flat silhouette 단계에서 안정된 판형이 아니라 노이즈 삼각형으로 읽힌다.
- 우선순위: P3 연구, Product 기본안 아님.

### G6. bone-per-shard prefractured animation

가설: W/F의 짧은 연출은 물리 다양성보다 미술가가 고정한 파열 타이밍이 더 중요할 수 있다.

- 12~24개 shard에 bone 하나씩을 주고 membrane→burst→settle clip을 저작한다.
- candidate Effect document가 `EFFECT_MODEL_CUE_DESC`를 소유하고 prepared CModel clip의 advance/hold는
  기존 Effect playback/document renderer가 수행한다. `CAnimationEffectCueDocument`는 Effect의
  action timing/anchor trigger만 소유한다.
- 장점: silhouette, 카메라 방향, 타이밍을 정확히 통제한다.
- 한계: 반복 재생이 눈에 띄며 seed 다양성이 없다. 현재 animated-model cue pass는 glass
  `Material.Execution`/RT1을 자동 소비하지 않으므로 G6은 우선 silhouette/timing-only다. survivor가
  되면 skinned effect-material adapter가 별도로 필요하다.
- 탈락: 동일 clip 반복이 실제 스킬 사용에서 즉시 인지되거나 asset 저작 비용이 G2보다 크다.
- 우선순위: P3 품질 fallback.

### G7. analytic 2D polygon SDF glass impostor

가설: 원형 alpha가 아닌 불규칙 polygon SDF라면 mesh 없이도 각진 silhouette를 얻을 수 있다.

- velocity-aligned quad 안에서 3~7개 half-plane 교집합으로 convex polygon SDF를 만든다.
- coverage는 `1 - smoothstep(0, max(fwidth(sdf), epsilon), sdf)`로 결정한다.
- 별도 local view ray/inverse proxy가 없는 상태에서 3D prism thickness/normal을 얻었다고 쓰지 않는다.
- 장점: seed/aspect/edge를 shader로 빠르게 sweep할 수 있다.
- 한계: camera-facing proxy라 근접/교차/depth에서 가짜가 드러난다.
- 탈락: 정면 외 카메라에서 card 느낌이 재발한다.
- 우선순위: P0 flat silhouette 비교, 최종안보다는 shader lab.

### G8. velocity-aligned pear head — SDF card / MeshParticle A/B

가설: 물은 구형 Fresnel보다 먼저 속도축에 정렬된 비대칭 3D head가 필요하다.

- 같은 semantic tuple로 analytic pear/capsule SDF card와 preauthored teardrop WModel을 먼저 A/B한다.
- SDF card는 기존 sprite particle carrier, WModel은 mesh particle carrier를 사용한다.
- teardrop WModel의 velocity basis/stretch는 `CEffectPlayback`이 particle World에 CPU로 넣는다.
  sphere rear-vertex pinch는 mesh shader와 velocity payload가 추가로 필요하므로 첫 mesh canary에서는
  사용하지 않는다.
- `stretch = clamp(1 + stretchBySpeed * |v|, 1, maxStretch)`를 쓴다.
- 길이축은 `headRadius * stretch`, 횡축은 부피 보존을 위해
  `headRadius / sqrt(stretch)`로 축소한다.
- SDF card만 뒤쪽 coverage를 analytic하게 pinch하고, mesh는 preauthored silhouette를 사용한다.
- 장점: 구현 대비 시각 정보량이 가장 크다.
- 한계: history가 없어 급회전하는 곡선 꼬리는 없다.
- 탈락: flat silhouette에서 원/알약으로만 읽히거나 진행 방향을 알 수 없다.
- 우선순위: P0, 첫 water canary.

### G9. compound head + analytic tail mesh

가설: 실제 history 전에도 한 particle state에서 head와 tail 두 geometry를 결합하면 목표를 근사한다.

- teardrop head와 -velocity 방향으로 정렬한 taper mesh를 같은 particle state로 렌더한다.
- 첫 구현은 local +X에 head와 tail이 함께 있는 하나의 compound WModel을 쓴다. mesh shader는 local
  longitudinal position에서 tail weight를 계산해 tail만 stretch/taper하므로 하나의 birth/state/resource다.
  동기화되지 않은 authored element 두 개를 같은 particle이라고 간주하지 않는다.
- tail length는 speed, width는 head radius, alpha는 normalized life에서 계산한다.
- head/tail 연결부는 겹침 구간과 같은 tint/normal policy로 숨긴다.
- 장점: history buffer 없이 빠르게 꼬리의 필요성을 검증한다.
- 한계: 꼬리가 휘지 않고 Valtan trail mesh는 칼날처럼 읽힐 수 있다.
- 탈락: seam, 칼날, 혜성으로 읽히며 물방울로 읽히지 않는다.
- 우선순위: P0 빠른 A/B, 빠른 kill 허용.

### G10. per-particle history ribbon tail + mesh head

가설: 사용자가 말한 “꼬리 달린 물방울”의 최종 구조는 입자별 과거 위치를 따르는 tapered ribbon이다.

- 각 birth에 emitter-local monotonic `spawnSerial`을 부여한다.
- 동일 tick 다중 spawn이 가능하므로 현재 `iSpawnSimulationStep`만 ID로 사용하지 않는다.
- history는 typed history carrier를 선택한 occurrence에만 opt-in sidecar로 할당하고 `spawnSerial`로
  join한다. 모든 `PARTICLE_STATE`를 먼저 비대화하거나 JSON/runtime packet에 history를 싣지 않는다.
- 각 살아 있는 droplet마다 6~10개 bounded position history를 fixed step으로 보존한다. seek/re-simulate는
  deterministic rebuild하고 birth/death/document swap에서 bounded clear한다.
- `u=saturate(age)`, finite positive exponent를 검증하고
  `w(u)=w0*pow(1-u,taperExponent)`로 꼬리 끝에서 0에 수렴시킨다.
- 여러 ribbon을 한 dynamic buffer에 concatenate하며 head와 첫 점을 연결한다.
- 기존 Trail은 element 중심 한 개를 sample하므로 그대로 쓰지 않고, 그 tessellation/shader 구조만 재사용한다.
- 장점: 중력과 방향 전환을 실제로 따라가며 목표와 가장 가깝다.
- 한계: stable ID, bounded eviction, re-simulate/rollback, buffer bounds harness가 필요하다.
- 탈락: tail이 head에서 분리되거나 point 순서/age가 역전되거나 성능 예산을 넘는다.
- 우선순위: P1, water 최종 유력안.

### G11. swept tube/capsule tail

가설: camera-facing ribbon의 옆면 소실을 피하려면 history를 3D tube로 감싸야 한다.

- history tangent를 따라 parallel-transport frame을 만든다.
- 각 sample마다 6~8각 ring을 만들고 taper하여 head와 연결한다.
- 현재 `CVIBuffer_DynamicTrail`은 점당 2 vertex/segment당 6 index이고 normal/tangent가 없으므로
  그대로 쓸 수 없다. G11은 generalized dynamic tube buffer, normal-aware vertex layout/shader와
  bounded capacity를 명시적으로 추가한다.
- zero-length segment와 180도 turn은 이전 frame 유지/재초기화로 처리한다.
- 장점: 어느 카메라에서도 부피가 유지된다.
- 한계: twist, degenerate segment, vertex 수, translucent sort 비용이 크다.
- 탈락: G10 대비 실제 화면 개선이 작거나 twist artifact가 보인다.
- 우선순위: P3 연구.

### G12. SDF/metaball head-tail volume

가설: sphere head와 capsule tail의 smooth union이 가장 자연스러운 surface tension 형상을 만든다.

- oriented proxy 안에서 sphere SDF와 history capsule SDF를 smooth-min으로 합친다.
- per-droplet history는 bounded StructuredBuffer와 instance range로 PS에 전달한다. droplet별 constant
  draw로 우회하지 않는다.
- raymarch hit gradient로 normal을 얻고, Beer-Lambert thickness는 proxy entry→first-hit 거리가 아니라
  surface entry→exit의 두 교차 또는 명시적으로 검증한 bounded thickness 근사를 쓴다.
- head-tail seam이 없고 breakup 직전 목이 가늘어지는 값을 코드로 제어한다.
- 장점: 물 부피와 연결이 가장 매끄럽다.
- 한계: 투명 depth/sort와 raymarch step 비용이 크다.
- 탈락: 6개 droplet 기준 GPU budget을 넘거나 proxy 교차 artifact가 G10보다 심하다.
- 우선순위: P3 연구.

## 5. 독립 기법 위에 붙이는 composition layer

다음은 12개 중 하나를 대체하는 독립 renderer가 아니라 survivor에 붙이는 연출 문법이다.

### 5.1 유리: membrane → fracture → macro/micro shard

```text
0~70ms    crack/inner-hole membrane이 아주 짧게 형성
40~120ms  membrane aperture가 열리며 큰 shard 5~7개가 방향성을 가짐
70~300ms  작은 shard/glint 20~35개가 늦게 확산
300ms~    edge와 dust만 먼저 사라지고 큰 shard가 마지막에 dissolve
```

faceted shell은 100ms를 넘기면 다시 원형 방패처럼 읽히므로 anticipation 구간에만 허용한다.

### 5.2 물: head → tail → impact crown → microdrops

```text
flight     velocity-aligned head와 반대 방향 tail
impact     40~180ms 동안 충돌점에서 crown/radial sheet
breakup    작은 secondary droplet가 원래 속도+법선 방향으로 분리
settle     foam/rim은 짧게, 굵은 물 덩어리는 더 빨리 소멸
```

splash를 비행 중 상시 재생하지 않는다. 충돌 또는 burst 사건에서만 발생시킨다.

## 6. preset sweep — 100개 무작위 수식 대신 제한된 실험

### 6.1 고정 조건

- random seed 고정
- 1차 비교는 front 카메라와 F/W animation time 고정
- survivor는 같은 seed/time으로 `front / yaw 30도 / pitch 25도` 세 view에서 재검사
- water는 화면 평면 방향 속도와 화면 안쪽 30도 속도를 각각 검사
- `0, 50, 100, 200, 400ms` 절대 frame과 `spawn 직후 / snap+30ms / life 50% /
  pre-death` normalized frame을 함께 고정
- 한 번에 하나의 기법 또는 최대 3×3 비교 grid
- flat pass는 opaque constant color, texture/alpha modulation/emissive/refraction/distortion/bloom OFF,
  동일 depth/cull로 고정
- 비교 질문마다 projected bounding diameter를 viewport 높이의 15%로 맞추고 총 coverage는 ±10%,
  macro count와 200ms 중심 travel distance는 같게 맞춤
- G1처럼 내부 shape를 바꿀 수 없는 resource oracle은 projected diameter만 맞추고 별도 표시
- 3×3 tile도 tile viewport 기준 같은 pixel diameter를 보장

### 6.2 preset

아래 숫자는 원본 복원값이 아니라 첫 비교를 재현하기 위한 `PROJECT_TUNED_APPROX` 초기 tuple이다.

Glass Shape (`macroCount`, `aspect`, `thickness/width`):

```text
G-SHEET   6, 1.8~2.6, 0.04~0.07
G-NEEDLE  6, 3.5~5.0, 0.025~0.05
G-MIXED   6, sheet 3 + needle 3, 0.03~0.08
```

Glass Motion (`speed m/s`, `lift m/s`, `gravity m/s²`, `drag /s`, `spin deg/s`):

```text
RADIAL        3.0~5.0, 1.0, -4.0, 0.6, 360~720
FORWARD_FAN   4.0~6.0, 0.5, -5.0, 0.4, 420~840, forward arc 55도
IMPLODE_SNAP  0~60ms inward 1.5, 이후 outward 5.0~7.0, -4.0, 0.5, 540~900
```

Glass Surface (`fresnelPower`, `edgeGain`, `absorption RGB /m`,
`distortionStrength`, `maximumDistortion UV`):

```text
CLEAR   5.0, 0.8, (0.06,0.03,0.02), 0.006, 0.015
ARCANE  3.5, 1.4, (0.18,0.08,0.28), 0.008, 0.015
PRISM   4.5, 1.0, (0.03,0.04,0.08), 0.010, 0.015;
        chromaticSplit은 survivor 전까지 0, 최종 비교에서만 0.002
```

Water Shape (`headAspect`, `stretchBySpeed s/m`, `maxStretch`, `tail target in head D`):

```text
W-PEAR    2.0, 0.22, 3.0, 1.75D
W-NEEDLE  3.0, 0.35, 5.0, 3.00D
W-GLOB    1.25, 0.08, 1.8, 1.00D
```

Water Motion (`speed m/s`, `gravity m/s²`, `drag /s`, `lifetime s`):

```text
BURST  4.0~6.0, -2.0, 0.5, 0.45~0.65
HEAVY  2.0~3.5, -6.0, 0.3, 0.50~0.80
SPRAY  6.0~9.0, -4.0, 0.8, 0.30~0.55
```

Water Surface (`fresnelPower`, `rimGain`, `normalStrength`,
`distortionStrength`, `maximumDistortion UV`):

```text
CLEAR     5.0, 0.8, 0.35, 0.005, 0.012
STYLIZED  3.5, 1.4, 0.55, 0.007, 0.012
DARK      4.0, 1.0, 0.45, 0.006, 0.012
```

full Cartesian product는 실행하지 않는다.

1. glass shape 질문: G1 resource oracle 1종과 G7의 G-SHEET/G-NEEDLE/G-MIXED 3종을 정규화해 비교
2. water head 질문: G8의 pear SDF card와 teardrop mesh를 W-PEAR 동일 tuple로 비교
3. tail 질문: 같은 G8 head에 G9 analytic tail을 OFF/ON으로 비교
4. 각 생존 shape × motion 3종: 6회
5. 최종 glass/water × surface 3종: 6회
6. 이후 G2/G10과 고비용 연구안을 한 변수 A/B로 비교

총 약 24~30회의 정보가 있는 육안 실험으로 줄인다. Tool sweep은 transient clone이며 tile을 선택해도
Apply 전에는 authored document를 바꾸지 않는다.

## 7. 구현 단계와 정확한 수정 경계

### G0. Visual Lab foundation

실험 data는 다음 Debug/Tool-only 경계에 둔다.

```text
Data/Effects/Experiments/DimensionMasterGlassWater/VisualLab.v1.json
```

이 manifest는 Product `EffectCatalog.json`, F/W authored effect, animevent, skillbinding에 등록하지
않는다. `CEffectDocumentCodec`은 parse/serialize, structural/semantic validation과 staged clone mutation만
소유한다. Effect Tool은 `Stage_DetailDraftPreview/Try_CommitDocument`를 호출하고,
`CEffectDocumentRenderer/CEffectCatalog`가 resource·shader prepare와 immutable prepared target commit을
소유한다. 모든 row를 process-local debug overlay로 stage/prewarm한 뒤 한 번에 commit한다. 어느
단계든 실패하면 staged clone을 폐기하고 이전 document/prepared pointer 또는 Product baseline을
유지한다. Save 뒤 commit 실패는 기존 CAS rollback 경계로 이전 bytes를 복원한다. 별도 catalog나
renderer가 아니다. exploration 시작과 종료에서 Product 네 종류 파일의 hash 불변을 focused harness로
확인한다.

manifest에는 12 candidate 외에 read-only Product F/W reference row를 둘 수 있지만 새 방식 수에는
세지 않는다. W reference는 lab row에서 기존 asset을 읽기만 하며 W Product cue를 F에 합성하지 않는다.

수정 후보:

- `Client/Public/Effect_AuthoringDocument.h`
  - project-tuned shape/motion typed field를 필요한 survivor만큼 추가한다.
- `Client/Private/Effect_DocumentCodec.cpp`
  - parse/serialize, structural/semantic validation, staged mutation에서 range/NaN/duplicate/unknown 거부를
    추가한다. resource prepare/commit을 codec에 넣지 않는다.
- `Client/Private/Effect_Tool.cpp`
  - `Render_Detail` 안에 Shape/Motion/Surface/Timing 그룹과 bounded sweep을 추가한다.
  - 기존 `Stage_DetailDraftPreview`와 rollback을 그대로 소비한다.
  - project-tuned lab candidate에서는 generic raw parameter UI 대신 visual control schema를 그린다.
- `Client/Public/Effect_Playback.h`, `Client/Private/Effect_Playback.cpp`
  - direct mesh 3축 rotation, velocity basis, 필요 시 spawnSerial/history를 추가한다.
- `Client/Public/Effect_DocumentRenderer.h`, `Client/Private/Effect_DocumentRenderer.cpp`
  - 선택된 canary의 batch/instance/ribbon 제출만 확장한다.
- `Client/Public/Effect_MaterialProgramRegistry.h`, `Client/Private/Effect_MaterialProgramRegistry.cpp`
  - semantic name과 layout/packed ABI의 유일한 join을 소유한다.
- `Client/Bin/ShaderFiles/Shader_VtxEffectParticle.hlsl`
  - G7/G8 SDF card처럼 sprite carrier로 닫히는 계산만 추가한다.
- `Client/Bin/ShaderFiles/Shader_VtxEffectMeshPreview.hlsl`
  - G1/G8 mesh World와 selected mesh surface 입력을 닫는다.
- G2/G5가 실제 survivor가 될 때만 Effect instanced mesh VS/input layout과 shard GS/pass CSO를
  각각 추가한다. 현재 mesh pass에 instancing/GS가 이미 있다고 가정하지 않는다.
- `Client/Bin/ShaderFiles/Shader_VtxEffectTrail.hlsl`
  - particle history ribbon survivor에서만 typed input을 추가한다.
- `Client/Bin/ShaderFiles/Shader_EffectUe3MaterialFamilies.hlsli`
  - 기존 opcode 번호를 재사용하지 않고, 서로 다른 equation/topology family의 survivor program만
    append-only 등록한다. candidate/preset마다 opcode를 만들지 않는다.
  - HLSL, `CEffectMaterialProgramRegistry` layout/descriptor/adapter, codec admission, renderer pass와
    compiled CSO closure를 같은 변경에서 닫는다. preset은 ABI가 아니다.
  - 기존 packet의 13 float4 scalar block, 3 float4 vector, texture hard 6 lane을 먼저 확장하지 않는다.

새 C++ 파일이 정말 필요한 경우에만 다음 역할 단위로 추가한다.

```text
Client/Public/Effect_VisualExperiment.h
Client/Private/Effect_VisualExperiment.cpp
Client/Public/Effect_VisualLabRunner.h
Client/Private/Effect_VisualLabRunner.cpp
```

`Effect_VisualExperiment`는 deterministic preset 생성/capture만 소유하며 semantic ABI나 runtime
candidate 선택을 소유하지 않는다. visual-control schema는 Tool/codec/registry가 함께 소비하는 Effect
authoring 계약이다. `Effect_VisualLabRunner`는 기존 `CEffectPresentationService::Spawn` 경계에서 F pair
순서와 action edge dedupe만 소유하는 얇은 Debug orchestration이며 두 번째 manager/runtime가 아니다.
추가 시
`Client.vcxproj`와 `Client.vcxproj.filters`에 물리 폴더와 일치하도록 등록하고 Debug/Release Client
compile로 검증한다. 후보 수만 나열하는 미래용 placeholder라면 만들지 않고 기존 Tool 파일에 둔다.

### G1. 첫 silhouette quartet

- glass: G1 rigid cluster, G7 polygon SDF card
- water: G8 pear SDF/teardrop mesh head, G9 compound head-tail
- surface를 끈 flat coverage 비교만 구현한다.
- 이 단계에서 떨어진 방식에는 refraction/MRT/scene input을 구현하지 않는다.

### G2. motion survivor

- glass survivor에 3축 tumble와 ballistic preset을 연결한다.
- water survivor에 velocity basis, speed stretch, breakup timing을 연결한다.
- fixed seed/time replay와 NaN/degenerate velocity를 자동 검증한다.

### G3. surface survivor

- glass Fresnel/absorption/edge를 먼저, RT1 distortion은 나중에 연결한다.
- water normal/Fresnel/absorption/foam을 먼저 연결한다.
- 필요한 texture lane만 사용한다. 원본의 8 lane이라는 이유로 현재 authoring lane 수를 먼저 늘리지 않는다.
- scene depth는 기존 frame resource를 안전하게 bind할 수 있을 때만 renderer-owned system input으로
  연결하고, environment DDS는 candidate가 실제 소비할 때 typed texture lane으로 연결한다. scene color
  sample/굴절 합성은 Effect PS가 아니라 Deferred distortion resolve가 소유한다.

### G4. 최종 유력 구조

- glass: G2 split/instanced shards + membrane + macro/micro layer
- water: G10 per-particle history ribbon + G8 head + impact crown/microdrops
- G1/G8에서 shape 가설이 탈락하면 이 단계로 가지 않는다.

### G5. 고비용 연구안

- 사용자 요청의 최소 범위인 G1~G10은 모두 source-inferred MVP와 자동 gate까지 시도한다. 자동
  gate에서 KILLED된 후보는 깊은 surface/preset tuning을 중단하고 실패 증거를 보존한다.
- G11/G12는 앞선 열 개에서 해소되지 않은 명확한 결함이 있을 때 추가 구현한다.
- 구현 목적과 kill criterion이 없는 연구 코드는 Product shader closure에 넣지 않는다.

### G6. Product 불변 F A/B와 survivor 승격

- broad exploration 중 `EffectCatalog.json`, F/W authored document, `DimensionMaster.animevents`,
  skillbinding은 hash 불변이다.
- authoring은 F1 Effect Tool의 synchronized preview와 semantic controls를 쓴다. 새 전역 단축키나
  F2~F12 selector를 만들지 않는다.
- 실제 action context A/B는 authoritative `(skillId=2050230, iActionStartTick)` edge를 1회 dedupe한다.
  고정 순서에서 첫 F는 A=Product baseline, 둘째 F는 B=candidate SOLO, 이후 다음 pair로 간다.
  재시작하면 순서를 reset한다. B prepare/spawn 실패는 baseline을 바꾸지 않고 다음 pair로 진행한다.
- 사용자 서면 survivor 이후 integration B만 `baseline + candidate`로 비교한다.
- 선택된 glass 1개와 water 1개만 별도 변경 단위의 Product 후보로 stage한다. 실제 F/W Product cue
  교체는 별도 승인이다.

## 8. 자동 검증 계약

### 8.1 codec/data

- stable Effect/Element/Preset ID
- Product EffectCatalog/F/W authored/animevent/skillbinding hash 불변
- duplicate ID/parameter 거부
- unknown version/kind/profile 거부
- Resources-relative path, drive-qualified/`..` 거부
- min > max, 음수 lifetime/width, zero sample interval 거부
- NaN/Inf, packet count/mask/lane 불일치 거부
- 중간 resource/shader stage 실패 시 기존 preview/document 유지
- JSON parse와 `Validate-EffectSources.ps1`

### 8.2 simulation/geometry

- fixed seed + fixed step deterministic replay
- quaternion finite/normalized
- zero angular velocity에서 기존 quaternion 유지
- zero velocity에서 velocity basis fallback이 finite
- spawnSerial uniqueness, bounded history eviction
- history point age와 cumulative distance 단조 증가
- head-tail 첫 점 연결, tail end width ≤ head width
- instance count, dynamic buffer vertex/index bounds
- 죽은 particle의 history가 다음 birth에 섞이지 않음

초기 runaway 방지 budget은 candidate control 12개, texture lane preferred 4/hard 6, active particle
64, life 1.2초, world extent 3.5m, dynamic geometry 512 vertices/1536 indices, upload 64KiB/frame,
draw target 4/hard 6으로 둔다. 이 값은 미감 PASS가 아니라 lab 안전 경계이며 실제 baseline 계측으로
조정한다.

### 8.3 GPU

기존 compiled shader closure/WARP probe를 확장한다.

- 정확한 vertex carrier와 typed packet으로 RT0 nonzero/finite
- distortion을 쓸 때만 RT1 nonzero/finite
- time/velocity/semantic parameter 변경 시 출력 변화
- invalid opcode/layout/lane/mask/NaN은 draw를 제출하지 않고, 사전 clear한 WARP RT0/RT1 sentinel을
  변경하지 않음
- silhouette coverage가 0 또는 full-card로 고정되지 않음
- debug view: coverage, normal, Fresnel, thickness, distortion, velocity axis,
  tail age, overdraw/depth/sort

### 8.4 정본 명령

실제 구현 변경 단위에서 다음을 실행한다.

```powershell
$resourceRoot = 'C:\Users\user\Desktop\LostArk\Client\Bin\Resources'
$env:LOSTARK_RESOURCE_ROOT = $resourceRoot

powershell -NoProfile -ExecutionPolicy Bypass -File Tools/EffectPipeline/Validate-EffectSources.ps1 -RepositoryRoot C:\Users\user\Desktop\CodexWorkTree\LostArk-dimensionmaster-vfx-lab -ResourceRoot $resourceRoot

powershell -ExecutionPolicy Bypass -File Tools/Build/Test-CompiledShaderClosure.ps1 -Configuration Debug -Modules Product
powershell -ExecutionPolicy Bypass -File Tools/Build/Test-CompiledShaderClosure.ps1 -Configuration Release -Modules Product

powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile FullDiagnostic -ResourceRoot $resourceRoot
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release -Profile Product -ResourceRoot $resourceRoot

git diff --check
```

Product profile만으로는 Effect source validation이 자동 실행되지 않으므로 validation 명령을 별도로
실행한다. 현재 obsolete Imported/Generated receipt에 묶인 baseline failure는 새 canary gate로
재사용하지 않고, 현재 제품 정본 validator와 새 Visual Lab harness를 사용한다.

## 9. 사용자 육안 판정 계약

에이전트는 Client/UI를 자율 실행·조작하거나 캡처하지 않는다. 자동 검증은 draw와 수치의 안전성을
증명할 뿐 미감을 PASS로 쓰지 않는다.

### 9.1 실행 전 LAN 경계

이 작업 트리에서 LAN sync는 로컬 endpoint를 확인했으나 현재 PC가 `server-host`이고 TCP 7777
LocalSubnet 방화벽 규칙이 누락되어 있었다. 실제 Server+Client 실행 전 관리자로 PowerShell을 열어
다음을 한 번 실행해야 한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Network/Sync-TeamLanEndpoint.ps1
```

### 9.2 Tool 판정 순서

1. 사용자가 elevated PowerShell의 LAN sync를 완료한 뒤 Server + Client profile을 직접 시작한다.
2. F1 Developer Tools의 Effect Tool에서 Visual Lab candidate를 선택한다.
3. `Flat Silhouette → Motion → Depth/Cull → Refraction Only → Edge Only → Full Surface` 순으로 본다.
4. 절대/normalized time과 front/yaw/pitch view를 같은 seed로 본다.
5. Shape Oracle, Motion Survivor, Final Composition 단계마다 서면 승격 또는 탈락 이유를 남긴다.
6. action-context 검증은 고정 F A/B pair 순서로 직접 누른다.
7. 선택된 두 후보만 차원술사 F/W animation synchronized integration preview로 본다.

### 9.3 최소 육안 합격 조건

Shape Oracle PASS:

- G1/G7 glass는 동일 projected size에서 원형/full card가 아닌 외곽을 보인다.
- G8 water head는 tail/splash가 없어도 bulbous mass와 진행 방향이 읽힌다.
- 이 단계는 다음 투자 여부만 결정하며 최종 visual PASS가 아니다.

Motion Survivor PASS:

- glass는 150~300ms에 macro shard 최소 3개가 서로 상대 위치와 회전축을 달리한다.
- water tail은 head diameter의 1.5~3.5배이며 끝 폭은 head의 0.35배 이하이다.
- head는 bulbous mass로 남고 폭/alpha는 head에서 tip으로 단조 taper한다.
- seam, 칼날 edge, bead chain, 분리 ghost가 없어야 한다. G10부터는 꼬리가 중력/방향 전환 궤적을
  따라야 한다.

Final Glass Composition PASS:

- 두 시점과 front/yaw view에서 facet parallax와 face/edge 밝기·굴절 방향이 바뀐다.
- 종이, 꽃가루, 불꽃, 돌, 얼음이 아니라 유리로 읽힌다.
- crack membrane은 짧은 인과를 만들며 원형 방패로 머물지 않는다.

Final Water Composition PASS:

- main droplet 최소 3개가 진행 방향을 읽을 수 있는 비대칭 head를 가진다.
- head/tail은 transparent sort pop, black/white card, 과한 halo 없이 glass와 다른 물 질감으로 읽힌다.
- splash crown은 core head/tail gate와 분리한다. ground collision 또는 명시적 breakup event 뒤
  40~180ms에 사건점에서 확장하고 비행 중 임의로 떠 있지 않는다.

## 10. 우선순위와 중단 규칙

| 순서 | 실험 | 목적 | 다음 단계 조건 |
|---|---|---|---|
| 1 | G1 | 실제 mesh가 원형을 깨는가 | flat silhouette 합격 |
| 2 | G8 | velocity head가 물방울로 읽히는가 | 방향성 합격 |
| 3 | G7 | shader-only 각진 impostor 비교 | 카메라 의존 허용 범위 확인 |
| 4 | G9 | history 없는 꼬리의 정보량 | seam/칼날 탈락 여부 |
| 5 | G2 | glass 개별 shard 최종 구조 | G1 생존 시만 |
| 6 | G10 | water history tail 최종 구조 | G8 생존·G9 부족 시만 |
| 7 | composition | membrane/micro/splash | 핵심 carrier 생존 후 |
| 8 | G3/G4/G5/G6/G11/G12 | 대체 연구 | 명시적 결함이 있을 때만 |

다음 경우 즉시 해당 경로를 중단한다.

- flat coverage가 원형/full card인 상태에서 surface 상수만 계속 조절함
- 하나의 F에 여러 후보를 겹쳐 어느 방식의 효과인지 분리할 수 없음
- source-exact 이름을 붙이기 위해 raw CB/texture lane 수를 복제함
- 새로운 manager/runtime가 기존 `CEffectPlayback/CEffectDocumentRenderer`와 같은 역할을 가짐
- 사용자 승인 전에 Product F/W cue를 교체함
- 자동 screenshot이나 draw success를 visual PASS로 기록함

## 11. 완료 정의

이 계획의 구현 완료는 다음을 모두 만족할 때다.

- G1~G10이 각각 독립 stable canary로 source-inferred MVP/자동 gate까지 시도되고 결과가
  `AUTO_PASS` 또는 근거 있는 `KILLED`로 추적된다.
- G11/G12는 앞선 열 개의 결과에 따라 `AUTO_PASS`, `KILLED`, 또는 명시적 `DEFERRED_WITH_REASON`으로
  추적된다.
- P0 quartet이 Tool에서 같은 seed/time으로 비교 가능하다.
- glass/water 후보가 자동 safety gate를 통과하고, 사용자가 Shape Oracle → Motion Survivor → Final
  Composition 단계에서 별도로 visual promotion한다. 자동화가 survivor를 선택하지 않는다.
- 의미 파라미터만 Effect Detail에 bounded control로 노출된다.
- invalid data와 중간 stage 실패가 기존 preview/document를 보존한다.
- Debug FullDiagnostic와 Release Product, Effect validator, shader closure/WARP가 통과한다.
- 사용자가 직접 육안 판정한 glass/water 후보만 F/W synchronized preview에 연결된다.

실제 Product F/W 교체와 `USER_APPROVED`는 이 Visual Lab 결과를 입력으로 하는 후속 변경 단위다.
