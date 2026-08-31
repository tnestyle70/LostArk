# 차원술사 유리·물 파티클 12경로 Visual Lab 비평 결과

기준일: 2026-08-31

대상 계획: `2026-08-31_DIMENSIONMASTER_GLASS_WATER_TWELVE_PATH_VISUAL_LAB_PLAN.md`

구현 상태: 계획·비평만 완료, 코드와 Product 데이터는 미변경

## 1. 비평 구성

서로 다른 관점의 읽기 전용 비평자를 사용했다.

| 비평 역할 | 집중 영역 |
|---|---|
| Rendering/Geometry | carrier, topology, shader 식, mesh/particle/trail renderer, GPU 비용 |
| Motion/VFX Art | silhouette, 파열 인과, head-tail 문법, preset과 육안 탈락 기준 |
| Pipeline/Validation | Effect Tool/codec/runtime 소유권, Product 격리, rollback, 자동 gate |

비평자의 주장을 결론으로 그대로 복사하지 않고 현재 코드, authored 문서, resource를 다시 대조했다.

## 2. 재현된 공통 근거

### 2.1 원형 문제는 shader 상수보다 carrier 문제다

세 비평자가 독립적으로 같은 결론을 냈다. 현재 F/W는 sphere/helix mesh와 sprite 비중이 높고,
직전 물 canary는 billboard 6개뿐이다. 원형 UV로 hemisphere normal과 Fresnel을 계산하면 표면은
물처럼 반짝일 수 있어도 silhouette는 계속 원이다.

판정: 채택. 모든 surface 작업보다 flat silhouette gate를 먼저 둔다.

### 2.2 실제 24-island glass mesh가 미사용 상태다

`fx_m_glass_01.wmodel`은 authored 문서 참조가 없고 198 vertices, 522 indices, 174 triangles,
24 disconnected component로 재현됐다. 원본 F/W asset이라고 단정할 근거는 없지만 geometry oracle로
충분하다.

판정: 채택. G1을 첫 glass canary로 둔다. `SOURCE_EXACT`라고 부르지 않는다.

### 2.3 현재 Trail은 물방울별 꼬리가 아니다

현재 authored Trail은 element 중심의 point history를 만든다. particle마다 태어난 시점과 경로가
다른 droplet tail에는 stable spawn serial과 개별 bounded history가 필요하다. 같은 fixed tick에 여러
particle이 생길 수 있으므로 `iSpawnSimulationStep`만 identity로 사용할 수 없다.

판정: 채택. G10에 emitter-local monotonic spawn serial과 history ownership을 요구한다.

### 2.4 현재 Tool의 raw parameter UI는 project-tuned 조절에 부적합하다

`CEffect_Tool::Render_AuthoringMaterialParameters`는 compiler-declared scalar/vector를 모아 각각
`-100000..100000` 범위로 직접 보여 준다. 이 상태에서 CB evidence를 수십 개 scalar로 옮기면
사용자는 의미 없는 수치를 조절하게 된다.

판정: 채택. project-tuned technique에는 semantic control schema를 사용하고 candidate당 최대 12개
control만 노출한다. 전체 후보가 공유하는 용어 수가 더 많아도 한 화면에 12개를 넘기지 않는다.

## 3. 12개 방법별 비평 판정

점수는 1~5다. 시각 정보량은 높을수록 좋고, 구현 비용과 위험은 높을수록 불리하다.

| G | 방법 | 시각 정보량 | 비용 | 위험 | 비평 판정 |
|---|---|---:|---:|---:|---|
| 1 | 24-island rigid cluster | 5 | 2 | 2 | 반드시 첫 실행. 개별 분리는 없으므로 geometry oracle로 한정 |
| 2 | split + instanced shard bank | 5 | 4 | 3 | glass 최종 유력. 새 loader 대신 CModel/CMaterial/Render_Instanced 재사용 |
| 3 | Voronoi extruded bank | 4 | 4 | 3 | 원본 무관 대조군. 균일 polygon confetti 위험 |
| 4 | per-island WPO | 5 | 5 | 4 | 한 draw 장점. island metadata vertex ABI 비용을 G2와 비교 |
| 5 | Geometry Shader burst | 3 | 4 | 5 | 연구용. 종이 삼각형과 D3D11 GS 비용 때문에 빠른 kill 허용 |
| 6 | bone-per-shard clip | 5 | 5 | 3 | 정밀 timing fallback. 반복성·저작비가 큼 |
| 7 | analytic 2D polygon SDF | 4 | 3 | 3 | shader-only 대조군. 3D prism이라고 부르지 않음 |
| 8 | velocity pear SDF / teardrop mesh | 5 | 3 | 2 | 반드시 첫 water 실행. 같은 tuple의 carrier A/B |
| 9 | head + analytic tail mesh | 4 | 3 | 3 | history 전 빠른 A/B. seam/칼날이면 즉시 탈락 |
| 10 | per-particle history ribbon | 5 | 4 | 4 | water 최종 유력. identity/history/buffer harness 필수 |
| 11 | swept tube tail | 5 | 5 | 5 | ribbon 옆면 소실 대안. twist/vertex 비용이 큼 |
| 12 | metaball head-tail volume | 5 | 5 | 5 | 매끄럽지만 마지막 연구안. framegraph 확장을 먼저 하지 않음 |

### G1 비평

24개 island가 한 rigid cluster이므로 “여러 조각이 폭발한다”는 최종 답은 아니다. 하지만 기존 원형
card와 같은 material을 사용해도 silhouette가 달라지는지 가장 싸게 분리한다. 이 canary를 건너뛰고
G2부터 구현하면 geometry와 runtime 확장의 효과를 분리할 수 없다.

### G2 비평

현재 mesh particle은 particle마다 `Render_Mesh`를 반복한다. `CModel::Render_Instanced`는 존재하지만
Effect instance payload에 color/dynamic/quat/float3 scale이 모두 닫혔다고 가정하면 안 된다. 필요한
최소 instance ABI만 survivor에서 추가해야 한다.

### G3 비평

Voronoi cell을 runtime 매 frame 생성하면 안 된다. offline cook 또는 Tool stage 1회 생성 후 immutable
geometry로 써야 한다. 생성 seed/aspect/thickness가 너무 균일하면 유리가 아니라 색종이로 읽힌다.

### G4 비평

기존 WModel vertex contract가 island ID와 pivot을 보존하지 않으므로 기존 asset만 shader에 넣어서
구현할 수 없다. cook/vertex ABI 변경 비용이 G2 instancing보다 크면 탈락시킨다.

### G5 비평

primitive 단위 움직임은 connected shard가 아니라 triangle을 찢는다. silhouette 연구값은 있지만
Product 기본 구조로 승격할 가능성은 낮다. broad renderer 프로젝트를 새로 만들지 않는다.

### G6 비평

고정 clip은 짧은 F/W 연출에서 높은 미감을 줄 수 있지만 매 사용마다 같은 파편이 같은 길로 간다.
seed 다양성보다 정교한 timing이 중요한지 확인하는 대조군이다.

### G7 비평

SDF가 polygon coverage를 만들 수는 있어도 quad의 3D parallax/depth를 완전히 대체하지 못한다.
shader 계산만으로 원형을 깨는 최저비용 대조군으로 사용하고 근접 카메라를 합격 기준에 넣는다.

### G8 비평

현재 velocity alignment가 “회전”만 한다면 늘어난 head/tail 비율은 생기지 않는다. 같은 semantic
tuple의 analytic pear SDF card와 preauthored teardrop mesh를 A/B하고 speed stretch, head pivot,
transverse compression을 같이 닫아야 한다. zero/projected velocity fallback도 필요하다.

### G9 비평

Valtan trail mesh를 그대로 쓰면 검격/칼날처럼 보일 수 있다. 재사용은 빠른 탈락 실험으로만 허용하고,
좋지 않은 결과를 texture/색 수십 회 조절로 살리지 않는다.

### G10 비평

기존 element Trail state를 droplet마다 복제하는 식의 무제한 container는 금지한다. 최대 particle,
history points, upload bytes를 bounded 값으로 고정하고 모든 strip을 가능한 한 한 dynamic draw로 묶는다.

### G11 비평

parallel-transport frame을 쓰지 않으면 tube가 궤적에서 뒤틀린다. G10 ribbon이 실제 카메라에서
사라지는 증거가 있을 때만 구현한다.

### G12 비평

metaball이 이긴다는 증거 전에 전용 half-resolution framegraph나 deferred pass를 범용 renderer로
추가하면 과도한 확장이다. oriented proxy 안의 bounded 실험으로 시작하고 GPU gate에서 빠르게
탈락시킨다.

## 4. 원본 수식과 Effect Detail 방향에 대한 판정

사용자의 방향 전환은 맞다.

```text
원본 복원 목표
  raw register와 exact ShaderMap을 닫아 원본 pixel을 재현

이번 Visual Lab 목표
  원본 증거에서 필요한 현상을 추출하고 현재 엔진에 맞는 typed geometry/motion/surface를
  PROJECT_TUNED_APPROX로 저작
```

`CB0[22]`, `CB2[4]`, `t0..t7`은 다음 이유로 직접 UI가 되면 안 된다.

1. register에는 material 값뿐 아니라 view/fog/engine carrier 값이 섞일 수 있다.
2. 원작의 단위, exposure, gamma, blend, depth 조건과 현재 renderer 조건이 다르다.
3. scene depth는 renderer-owned input이고 environment DDS는 필요한 program의 typed asset lane이므로
   둘을 원본 register 순서대로 같은 종류의 texture slot으로 복제할 수 없다.
4. 100개의 raw scalar도 원형 carrier와 tail 부재를 고치지 못한다.
5. 의미 없는 숫자는 비교, 회귀, 사용자 선택을 불가능하게 한다.

비평 결과, 저장 계층은 다음으로 확정한다.

| 의미 | 저장 owner | Tool 표시 |
|---|---|---|
| shard count/aspect/thickness/rotation | typed `Detail.Particle`/carrier field | Shape/Motion |
| spawn/lifetime/burst timing | `Detail.Timing`과 particle timing | Timing |
| Fresnel/absorption/refraction/edge | `Material.Execution` semantic name | Surface |
| packed index/register/texture lane | material program/layout registry | 표시 안 함 |
| label/range/unit/step/default | visual control schema | 해당 group widget |

visual schema의 target은 문자열 JSON path가 아니라 닫힌 enum이어야 한다. material semantic과 packed
lane의 join은 material program registry가 계속 소유하고 schema가 lane 번호를 복제하지 않는다.

## 5. 비평으로 채택한 구조적 제한

### 5.1 Product 격리

- broad exploration에서는 `EffectCatalog.json`, F/W authored effect, animevent, skillbinding을 수정하지 않는다.
- lab manifest/candidate/preset은 별도 `Data/Effects/Experiments/DimensionMasterGlassWater/` 경계에 둔다.
- Effect Tool이 `parse → validate → all-resource/program stage → single process-local commit`으로 읽는다.
- 한 row라도 prepare 실패하면 새 overlay 전체를 commit하지 않고 이전 preview 또는 Product baseline을 유지한다.
- 최종 survivor만 별도 변경 단위에서 cue로 승격한다.

### 5.2 ABI 제한

- texture lane은 4개 이하를 우선하고 현재 hard limit인 6개를 넘기지 않는다.
- 원본 t0..t7 때문에 선제적으로 8 lane/global bind slot을 만들지 않는다.
- candidate/preset마다 opcode를 하나씩 만들지 않는다. equation/topology family만 program identity를 가진다.
- preset은 data이며 shader ABI가 아니다.
- particle history는 G10이 생존한 뒤 opt-in sidecar로 추가하고 모든 particle state를 먼저 비대화하지 않는다.

### 5.3 예산과 gate

초기 lab hard bound는 다음과 같이 잡되 실제 baseline 계측으로 조정한다.

```text
candidate controls       <= 12
texture lanes preferred  <= 4, hard <= 6
active particles         <= 64
life                     <= 1.2s
world extent             <= 3.5m
dynamic geometry         <= 512 vertices / 1536 indices
dynamic upload           <= 64 KiB/frame
draws per candidate      target <= 4, hard <= 6
CPU simulation p95       target <= 0.20ms, hard <= 0.50ms
```

이 숫자는 visual PASS가 아니라 runaway 실험을 막는 lab budget이다. survivor의 GPU 시간은 실제
1080p manual 실행에서 별도로 계측한다.

## 6. 비평에서 기각하거나 보류한 제안

### 6.1 F에 12개를 동시에 합성

기각. overdraw, exposure, timing이 섞여 어느 방식이 좋은지 판별할 수 없다. 한 번에 하나 또는
동일 조건 grid만 허용한다.

### 6.2 최초부터 Product F/W action마다 자동 A/B runner 삽입

보류. process-local overlay와 정확한 action tick dedupe는 유용하지만 P0 silhouette 비교는 기존 Effect
Tool의 synchronized preview로 더 작게 닫을 수 있다. Tool 비교가 불충분하다는 사용자 관찰이 있을 때
동일 `CEffectPresentationService::Spawn` 경계의 Debug-only runner를 추가한다. 별도 presentation service는
만들지 않는다.

### 6.3 원본의 8개 texture slot을 모두 이식

기각. 첫 canary의 glass는 crack/normal/mask 2~4 lane, water는 shape/normal 1~2 lane으로 충분하다.
scene input은 asset lane과 분리한다.

### 6.4 100개 수식의 full Cartesian sweep

기각. projected size/count/coverage/travel을 정규화한 단일 질문 A/B 뒤 survivor × motion 3종,
winner × surface 3종의 staged funnel을 쓴다. pairwise/orthogonal sample 외의 무작위 대량 튜닝은
하지 않는다.

### 6.5 새 EffectV2 Character runtime

기각. EffectV2의 trail/particle 구조는 참고할 수 있지만 현재 Character 제품 cue는 기존
`CEffectPresentationService/CEffectPlayback/CEffectDocumentRenderer`를 소비한다. 두 번째 런타임은
데이터와 failure owner를 둘로 만든다.

## 7. 최종 실행 권고

```text
P0  G1 rigid glass cluster
    G8 velocity pear SDF / teardrop mesh head
    G7 analytic polygon SDF
    G9 compound head-tail

P1  G2 split/instanced shards
    G10 per-particle history ribbon
    membrane / macro-micro / impact crown composition

P2  G3 Voronoi bank
    G4 per-island WPO

P3  G5 GS triangles
    G6 bone clip
    G11 swept tube
    G12 metaball volume
```

사용자 요청의 “적어도 10개 시도”는 G1~G10을 각각 source-inferred MVP/자동 gate까지 도달시키는 것으로
해석한다. 자동 gate에서 KILLED된 후보는 깊은 surface tuning을 중단하지만 실패 증거와 이유는 lab
manifest/result에 보존한다. G11~G12는 첫 열 개에서 해소되지 않은 명확한 결함이 있을 때 실행하는
추가 연구안이다.

예상 최종 조합은 다음이지만 사용자 판정 전에는 선택으로 확정하지 않는다.

- glass: G1 또는 G2 + 짧은 crack membrane + dual-shell edge/refraction
- water: G8 head + G10 tail + impact crown/microdrops

## 8. 계획서 초안 재비평과 교정

세 비평자에게 작성된 계획서 전체를 다시 읽게 했다. 첫 아이디어 비평과 달리 두 번째 비평은
현재 plan 문장의 구현 가능성만 검사했다.

### 8.1 Motion/VFX Art 재비평

필수 교정:

- 단일 정면 카메라 대신 front/yaw/pitch survivor view
- projected diameter, macro count, total coverage, travel distance 정규화
- 이름뿐인 preset을 exact bounded tuple로 교체
- `Shape Oracle → Motion Survivor → Final Composition` 사용자 승격 분리
- 자동 safety gate와 사용자 visual promotion 분리
- water tail 길이 상한, seam/칼날/bead-chain 부정 조건
- splash crown을 core head/tail gate에서 분리

판정: 모두 채택하여 계획서에 반영했다.

### 8.2 Rendering/Geometry 재비평

필수 교정:

- zero angular velocity quaternion NaN 방지
- tangent-space normal decode/strength/RNM/TBN 순서
- G2의 동일 model/mesh cohort별 instancing과 Effect 전용 instance payload
- G4의 공통 drag recurrence 유지
- G5의 실제 GS pass/CSO closure
- G8의 sprite shader와 mesh World 경계 분리
- G9를 하나의 compound WModel/state로 고정
- G11의 generalized tube buffer/normal-aware layout 필요
- G12의 history StructuredBuffer와 entry→exit thickness

판정: 모두 채택했다. G3는 별도 renderer가 아니라 G2 content-generation A/B로, G10/G11은 같은
history의 ribbon/tube backend A/B로 분류를 교정했다.

### 8.3 Pipeline/Validation 재비평

필수 교정:

- 실제 fidelity 이름을 `PROJECT_TUNED_APPROX`로 통일
- Effect PS는 scene color를 직접 sample하지 않고 RT0 local radiance/coverage와 RT1 signed distortion만
  기록하며 Deferred resolve가 합성
- codec은 parse/validate/staged mutation, Tool/Renderer/Catalog는 prepare/commit/rollback 소유
- Product F/W/catalog/animevent/skillbinding hash 불변과 process-local overlay
- fixed F A/B runner는 기존 presentation spawn 경계를 소비하고 별도 manager가 아님
- candidate/preset별 opcode 금지, material registry와 CSO closure 원자적 변경
- invalid packet WARP는 zero write가 아니라 draw 미제출과 기존 sentinel 불변

판정: 모두 채택했다. 다만 비평자가 “firewall-ready”라고 보고한 항목은 재현되지 않았다. 정본 LAN
script를 다시 실행한 결과 현재 PC는 `server-host`이고 TCP 7777 LocalSubnet firewall rule이
missing/stale이므로 elevated PowerShell 실행이 여전히 필요하다. 실제 명령 출력을 우선해 계획서의
기존 blocker를 유지했다.

## 9. 비평 완료 상태

- 12개 canary의 시각 가설과 backend/content A/B 구분: 확인
- 원형 원인의 코드·resource 근거: 재현
- semantic Effect Detail 방향: 채택
- Product F/W 격리: 채택
- 자동 gate와 수동 visual gate 분리: 채택
- 코드 구현: 아직 시작하지 않음
- 사용자 visual admission: 아직 수행하지 않음

다음 작업은 P0 네 canary의 Tool flat silhouette와 Product 불변 F A/B vertical slice다.
