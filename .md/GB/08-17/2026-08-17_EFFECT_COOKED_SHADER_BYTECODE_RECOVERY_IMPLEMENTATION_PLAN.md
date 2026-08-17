# Effect Cooked Shader Bytecode 회수 구현 계획

branch: `feature/effect-cooked-shader-recovery-and-transform-fix`
선행 문서: `2026-08-17_EFFECT_CASCADE_MATERIAL_GRAPH_RECONSTRUCTION_IMPLEMENTATION_PLAN.md`
대응 결과: `2026-08-17_EFFECT_DIMENSIONMASTER_SHADERCACHE_JOIN_RESULT.md`

## 1. 목표와 현재 판정

목표는 도화가 31470에서 증명한 cooked shader 회수 절차를 스킬 전용 구현으로 복제하지 않고,
**class-neutral family-variant extractor**로 일반화해 차원술사 W를 첫 fixture로 닫는 것이다.

여기서 재사용 단위는 스킬도, base family 이름 하나만도 아니다. exact variant key는 다음 다섯 축이다.

```text
base material family
+ effective FStaticParameterSet
+ shader platform
+ vertex factory
+ pass / shader type
```

같은 family라도 static switch, vertex factory 또는 pass가 다르면 다른 compiled variant다. 반대로
위 key가 같으면 스킬과 클래스가 달라도 같은 family evaluator와 shader variant를 재사용한다.
`skillId == 2050120` 같은 분기나 차원술사 W 전용 HLSL은 만들지 않는다.

현재 판정은 G03-5 source-value uniform/texture closure와 authoring-only exact canary 구현을 기준으로
다음과 같다.

```text
Artist 31470   frozen exact oracle 존재
               occurrence 22 중 21 EXACT DXBC, 1 unresolved
               offline replay와 source HLSL 수치 replay까지 증명
               actual VF/pass, native binding, Product/visual admission은 false

DimensionMaster W
               target 6 / occurrence 10
               Glasshole / Crackhole / FluidNinja / CustomParticle / SpriteWave
               exact map + uniform-expression set + packed pixel DXBC 5/6
               exact native shader-object binding 5/5
               raw exact PS + signature carrier + native wire synthetic WARP replay 5/5
               source-value uniform CB0 expression closure 5/5
               source-exact texture binding 5/5 (24 binding / 23 unique DDS)
               runtime DDS parity 4/5, source-exact sampler 0/5
               Slice는 MIC static set이 없어 parent-default 규칙의 ABI 근거 전까지 BLOCKED
               sourceValueReplayAdmission 0/5
               actualVfPassAdmission 0/5
               runtimeAdmission 0/5, visualAdmission false

저작 화면 canary CustomParticle + Helix/SpriteWave 두 MIC만 명시적 후보
               cooked PS는 exact, vector CB0/texture binding은 exact
               scalar-group packing, sampler/color-space, carrier bridge는 approximate
               기본 OFF, Authored preview 전용, 실패·미일치 Element는 family-lite 유지

제품 화면       Product 연결·publish·runtime admission은 모두 false
수동 화면       아직 사용자 first pixel/visual PASS가 아니다
다음 gate       사용자 수동 canary 판정 뒤 family/permutation cohort 확대 여부 결정
```

다섯 exact target의 DXBC는 exact MIC map의 구조적 renderer-family 후보들이 공유하는 유일한
pixel shader reference에서 추출했다. G03-3은 cache의 native shader-object 265,979개를 serialized
absolute end-pointer로 순차 탐색하고, shader type FName과 shader ID로 다섯 reference를 각각 정확히
하나의 object 및 native scalar/vector/texture wire와 결합했다. wire 후보는 object 내부의 고정 offset을
가정하지 않고 전 범위를 검색해 유일성으로 선택했다. 다만 이것은 원본 emitter가 실제로 선택한
vertex factory와 pass를 admit한 결과는 아니다. G03-4는 이 exact PS와 native wire를 합성 상수·텍스처,
동적 signature carrier와 전체 MRT를 사용해 WARP에서 5/5 구조 재생했다. G03-5는 MIC hierarchy와
uniform expression을 평가해 source-value CB0 expression을 5/5 닫고, 24개 texture binding과 23개
고유 DDS identity를 회수했다. 다만 native scalar-group lane/padding, source revision의 sampler/filter/
address/color-space, engine-owned CB row와 actual VF/pass는 열려 있다. 따라서 원본 cooked PS를 직접
실행하는 경로는 Product가 아니라 **명시적으로 켜는 Authored preview canary**에만 한정한다.

## 2. Artist exact extractor가 확장되지 못했던 이유

### 2.1 처음부터 범용 라이브러리가 아니라 31470 canary였다

`extract_artist_31470_all_core_ref_shader_cache.py`는 cooked 경로가 실제로 가능한지를 한 스킬에서
끝까지 증명하기 위한 연구용 수직 슬라이스였다. 입력 manifest, schema, class, skill, target 분모,
recipe ID와 결과 count가 31470에 고정되어 있었다.

```text
characterClass  ARTIST
skillId         31470
target          recipe 18 / family 15 / occurrence 22
oracle          특정 recipe ID와 22-row denominator를 코드가 알고 있음
result schema   lostark.artist-31470.*
```

따라서 이름만 일반화하면 되는 도구가 아니었다. target manifest, effective static set, cache map,
shader reference, packed DXBC, replay receipt를 모두 class-neutral 입력과 결과 계약으로 분리해야 했다.

### 2.2 exact shader 증거와 제품 admission 사이에 열린 경계가 남아 있었다

Artist receipt가 증명한 것은 original DXBC 획득과 offline replay다. 다음 항목은 당시에도 false였다.

```text
actualVfPassSelection       false
registryAdmission           false
productRuntimeAdmission     false
visualFidelity              false
```

즉 실제 runtime vertex factory/pass 선택, shader-object binding ABI와 Product registry 연결이
닫히지 않았다. 이 상태에서 클래스를 늘리면 스킬별 증거 파일만 커지고 제품 픽셀은 늘지 않는다.

### 2.3 초기 0/23은 전투 캐시가 아니라 로비 캐시를 본 결과였다

초기 oracle이 검색한 ShaderCache 11개는 모두 `sc_lv_lobby_classselect_*`,
`sc_lv_customizingtool_classselect_*` 계열이었다. 전투 material을 그 분모에서 찾았으므로
`sourceBaseMaterialIdJoinCount = 0 / 23`이 나왔다.

이 수치가 회수 불가능으로 읽히면서 다른 클래스 확장의 우선순위가 낮아졌다. 이후 올바른
`EV2LG3OVEH3HGV7THTFFTM7TOKMCC` RefShaderCache와 pinned official v974 사본을 잡으면서
전제가 뒤집혔다. join 0은 알고리즘 결론이 아니라 먼저 분모 출처를 검증해야 하는 신호다.

### 2.4 canary 검증이 과대해졌고 authoring-first로 방향이 바뀌었다

Artist 한 스킬을 위해 builder, mutation test, admission receipt가 크게 확장됐지만 actual Product
pixel 경로는 여전히 닫히지 않았다. 같은 검증 스택을 클래스마다 복제하면 진행 속도만 떨어진다.
그래서 당시에는 cooked exact 확대보다 Effect Tool 저작, 표준 family와 실제 화면 작업을 먼저
진행하는 authoring-first 방향으로 전환했다.

현재는 W의 다섯 target이 pinned cache에서 exact map, uniform-expression set, packed pixel DXBC와
native shader-object binding까지 유일하게 닫힌 새 증거가 있으므로 class-neutral 확대의 비용 대비
가치가 생겼다.
다만 이것은 Artist canary를 다시 확장하거나 Artist의 Product admission을 완료했다는 뜻이 아니다.
Artist는 actual VF/pass와 native binding을 닫지 못한 채 authoring-first로 우선순위를 바꿨고,
현재 W 경로는 그 frozen binary primitive만 재사용한다. **Artist의 거대한 admission framework를
복제하지 않고**, 필요한 다섯 runtime 사실만 focused receipt로 닫는다.

```text
1  effective static set identity
2  정확히 하나의 material map과 구조적 uniform-expression set
3  structural VF/pass가 공유하는 pixel shader reference와 exact DXBC
4  exact native shader-object와 uniform/register/texture/sampler binding
5  actual VF/pass admission, offline replay와 runtime family ABI
```

## 3. 정본과 보존 경계

### 3.1 pinned official v974가 유일한 cache 정본이다

live 설치본은 2026-08-16 패치로 identity가 바뀌었다. 재현 가능한 join은 다음 pinned 사본만
사용한다.

```text
C:\Users\user\Desktop\Resource_LostArk\01_Extracted\Effect\
ARTIST\31470_TrackA_20260812\OfficialRefShaderCacheV974\
EV2LG3OVEH3HGV7THTFFTM7TOKMCC.v974.upk
```

live cache drift는 실패가 아니라 `INSTALLED_DRIFTED_PINNED_AUTHORITATIVE` 진단으로 기록한다.
installed와 pinned를 동시에 exact identity로 요구해 추출 전체를 막지 않는다.

### 3.2 Artist oracle은 수정하지 않고 회귀 기준으로 고정한다

기존 Artist extractor와 sealed receipt는 역사적 증거이자 golden oracle이다. 범용 도구는 이를
대체하거나 다시 쓰지 않는다. 같은 Artist target manifest를 입력했을 때 기존의 핵심 분모와
identity를 재현해야 한다.

```text
recipe             18
family             15
occurrence         22
EXACT / unresolved 21 / 1
```

receipt 전체 byte-for-byte 동일성을 강제하지 않는다. class-neutral schema로 바뀌어도 map key,
DXBC slice identity, binding closure와 replay 결과가 같아야 한다.

### 3.3 raw window scan은 정찰 자료다

`extract_ue3_material_uniform_expressions.py`의 GUID 주변 window scan은 다음 이유로 exact 계약이 아니다.

```text
한 창이 인접한 여러 material map을 섞는다.
uniform expression 직렬화 순서는 상수 register 순서가 아니다.
부모 GUID 존재는 W MIC permutation 선택을 증명하지 않는다.
```

따라서 그 산출물은 family 이름과 후보 파라미터를 발견하는 reconnaissance에만 쓴다. scalar/vector/
texture binding과 기본값의 정답표, renderer ABI, Product admission 근거로 승격하지 않는다.

## 4. 구현 구조

### 4.1 class-neutral target manifest

target row는 스킬 표시 이름이 아니라 원본 identity와 effective variant key를 소유한다.

```text
targetId
source package / object path / export class
base material package / object path / state GUID
effective FStaticParameterSet identity
shader platform
expected renderer geometry family
occurrence IDs (진단과 적용 대상)
```

object redirector와 material export가 같은 object path를 가질 수 있으므로 path last-wins lookup은
금지한다. export index와 class name을 함께 검증한다. MIC package와 RefShaderCache의 NameTable도
서로 바꿔 쓰지 않는다.

### 4.2 family, variant, MIC, occurrence, cue를 분리한다

```text
family evaluator
  Glasshole / Crackhole / FluidNinja / CustomParticle / Slice / SpriteWave 공용 수식

compiled variant
  family + effective static set + platform + VF + pass/shader type

MIC packet
  scalar / vector / texture override

occurrence
  particle count, life, transform, dynamic parameter, anchor

skill cue
  animation timing과 stage/clip binding
```

W는 이 구조의 첫 fixture일 뿐이다. 같은 variant key가 다른 스킬에서 나오면 새 셰이더를 만들지
않고 기존 variant를 참조한다.

### 4.3 원본 DXBC의 실행 경계 — 이전 offline-only 판정 정정

초기 계획의 “원본 DXBC는 offline oracle로만 사용한다”는 Product 안전 경계로는 맞지만, 현재
Authored preview canary의 실제 구현 상태를 설명하지 못한다. G03-4에서 raw DXBC 자체가 D3D11 WARP로
실행됨을 확인했고, 현재 Effect renderer에는 exact cooked pixel shader를 authoring에서만 실행하는
bounded bridge가 추가됐다.

```text
Authored preview canary
  raw exact pixel DXBC 실행
  sourceMaterialPath MIC exact match
  CustomParticle Sprite carrier / Helix LocalMesh carrier
  명시적 checkbox, 기본 OFF
  실패 또는 미일치면 기존 family-lite로 fail-open

Product runtime
  exact cooked execution OFF
  publish/runtime/visual admission false
  ABI 검증 없는 raw DXBC 승격 금지
```

canary의 pixel shader bytecode는 exact지만 carrier VS와 원본 emitter VF/pass 선택은 exact가 아니다.
CB0 vector row와 texture register/DDS binding은 source evidence를 사용하고, scalar-group packing과
sampler/filter/address/color-space는 project-preview approximate다. 이 혼합 fidelity를 bit-for-bit
material 복원으로 부르지 않는다.

family별 canary가 실패하면 해당 family만 기존 family-lite로 남긴다. 특히 Slice는 exact variant 대상에서
제외하고 현재 family-lite 경로와 사용자의 `sourceScale.size = 0.25` 손튜닝을 그대로 보존한다.

## 5. G별 구현

### G00. 증거 단계 이름과 정본 고정 — 완료

- parent GUID hit, MIC exact map, exact shader, runtime exact를 서로 다른 상태로 정의한다.
- pinned v974를 정본으로 고정하고 live 설치본 drift를 진단으로 분리한다.
- raw window scan을 reconnaissance로 강등한다.

종료 증거: 대응 RESULT의 §1, §4.5, §6이 같은 상태 이름을 사용한다.

### G01. Artist oracle freeze와 class-neutral core 분리 — 부분 완료

- Artist 전용 파일을 그대로 보존한다.
- `extract_ue3_material_shader_maps.py`는 frozen Artist 도구의 static-set equality, package cursor,
  packed descriptor와 DXBC decode primitive를 import해 class-neutral target manifest로 실행한다.
- 새 extractor는 character, skill, family, offset과 denominator를 코드에 고정하지 않는다.
- Artist 전용 recipe ID, 31470 count와 result schema는 기존 파일에 그대로 남아 있으며 새 receipt로
  덮어쓰지 않았다.
- class-neutral 경로로 Artist 21 EXACT / 1 unresolved를 다시 재생하는 회귀는 아직 미실행이다.

종료 증거: 기존 Artist 22 occurrence의 21 EXACT / 1 unresolved와 기존 DXBC identity를 재현한다.

### G02. W family-variant target manifest — 완료

- `skill.2050120.clip3.exact-shader-targets.json`은 6 target과 10 occurrence를 고정한다.
- Helix mesh 네 occurrence는 `SpriteWave + MeshParticle` target 하나로 분리했다.
- 재사용 identity는 skill/class가 아니라 family + effective static set + platform + VF + pass/shader type다.

종료 증거: skill/class/occurrence는 진단 분모일 뿐 variant selector가 아니며, 스킬 전용 HLSL 분기가 없다.

### G03-1. exact MIC map과 uniform-expression set 선택 — 완료

- Glasshole, Crackhole, FluidNinja, CustomParticle, Helix SpriteWave 다섯 MIC는 engine-equality static set으로
  각각 유일한 material shader map을 선택했다.
- 선택된 map 내부의 uniform-expression set을 구조적으로 decode했다.
- Slice는 `bHasStaticPermutationResource`가 없고 tail이 0이므로 parent-default를 silent identity fallback으로
  적용하지 않았다. parent-default fallback은 0건이다.

종료 증거: 6 target / 10 occurrence, exact map 5, decoded uniform set 5, Slice blocked 1이다.

### G03-2. structural VF/pass reference와 packed DXBC — 완료

- SpriteParticle 네 target의 structural VF 후보가 공유하는 유일한 BasePass pixel shader reference를
  선택했다. SpriteWave는 `flocalvertexfactory` 후보 하나다.
- reference가 지목한 packed descriptor와 compressed blob을 풀어 서로 다른 DXBC container 5개를
  exact SHA-256과 byte count로 봉인했다.

종료 증거: exact pixel DXBC 5/5이며 Slice는 upstream blocked 상태를 유지한다.

### G03-3. native shader-object와 binding wire — 완료

- shader-object table 265,979개를 serialized absolute end-pointer로 순차 탐색했다.
- shader type FName과 shader ID로 다섯 reference를 각각 native shader-object 하나와 결합했다.
- object 전체 offset-free scan으로 scalar/vector/texture wire 후보 하나를 선택했다.
- Glasshole `t2/s0`, FluidNinja `t4/s0`은 material texture가 아닌 engine-owned sample pair로 분리했다.
- native binding 완료는 actual emitter VF/pass admission이 아니다. actual VF/pass, runtime, visual은 false다.

종료 증거: receipt stage `G03_3_EXACT_NATIVE_SHADER_OBJECT_BINDING_CLOSURE`, result
`PASS_G03_3_EXPECTED_EXACT_NATIVE_BINDINGS_AND_SLICE_BLOCKED`, exact native binding 5/5다.

자동 검증: focused extractor test 20건과 extractor `--check`가 PASS다.

### G03-4. structural fixed-input replay — 완료

- `replay_ue3_material_pixel_shaders.py`는 G03-3 receipt가 봉인한 다섯 raw PS DXBC를 그대로
  `CreatePixelShader`하고, 각 PS ISGN에서 signature-compatible fullscreen carrier VS를 동적으로 만든다.
- 컴파일된 carrier OSGN과 원본 PS ISGN의 semantic/type/mask를 다시 비교한다. rasterizer-owned
  `SV_IsFrontFace`는 별도 입력으로 분리한다.
- native CB0/SRV/sampler wire를 live DXBC declaration과 sample pair에 재검증한다. Glasshole과
  FluidNinja의 engine-owned SceneDepth pair, Glass/Crack/Fluid의 synthetic CB2도 분리해 기록한다.
- Glass/Crack/Fluid는 `o0,o2,o3,o4,o5`와 `o1` sentinel hole을, CustomParticle/SpriteWave는 `o0`을
  실제 WARP draw로 검증한다.
- 다섯 target 모두 RT0가 nonzero이고 UE3 external-opacity 입력 `cb0[0].x` 변화가 RT0에 영향을 준다.
  additive CustomParticle/SpriteWave는 원본 PS가 `o0.w=0`을 고정하므로 RGB 변화가 증거다.
- fixture는 deterministic synthetic 상수와 1x1 RGBA32F texture, point-clamp sampler다. 따라서
  `structuralFixedInputReplayAdmission=true`만 올리고 `sourceValueReplay`, actual VF/pass, runtime,
  visual admission은 모두 false로 유지한다.

종료 증거: receipt stage `G03_4_STRUCTURAL_FIXED_INPUT_REPLAY`, result
`PASS_G03_4_STRUCTURAL_FIXED_INPUT_REPLAY_SOURCE_VALUE_REPLAY_BLOCKED`, exact target 5/5 replay,
focused test 19건과 receipt `--check` PASS다.

### G03-5. source-value uniform과 texture/sampler closure — 완료, exact replay admission은 보류

- `evaluate_ue3_material_uniform_expressions.py`가 MIC root-to-leaf override precedence와 UE3 uniform
  expression을 float32 semantics로 평가했다. FoldedMath ordinal 0은 add, ordinal 2는 multiply로 닫혔다.
- Periodic은 floor가 아니라 `x - trunc(x)`인 UE3 semantics로 교정했고 negative input 회귀를 추가했다.
- source-value uniform CB0 expression closure는 5/5다. 그러나 native scalar-group lane order/padding은
  source ABI로 증명되지 않아 authoring packet의 scalar row는 approximate다.
- `extract_ue3_material_texture_sampler_closure.py`가 24 native texture binding, 23 unique effective DDS를
  닫았다. source-exact texture binding은 5/5, runtime DDS parity는 4/5다.
- source revision의 address/filter/color-space와 `TF_Default` TextureLODSettings는 닫히지 않아
  source-exact sampler는 0/5다.
- actual VF/pass, source-value full replay, Product runtime과 visual admission은 모두 false다.

종료 증거: uniform receipt result
`PASS_G03_5_SOURCE_VALUE_UNIFORM_CB0_CLOSED_TEXTURE_SAMPLER_REPLAY_OPEN`, texture receipt result
`PASS_G03_5_EXACT_TEXTURE_BINDINGS_SAMPLER_AND_RUNTIME_DDS_BLOCKERS_EXPLICIT`다. focused test는 uniform 7건,
texture/sampler 5건이 PASS다.

### G04. content-addressed exact variant와 Authored preview canary — 구현 완료, 수동 판정 대기

- `materialize_ue3_exact_cooked_shader_variants.py`는 다섯 DXBC를 full SHA-256 이름으로
  `Data/Effects/CookedShaders`에 저장하고 class-neutral variant contract를 만든다.
- contract에는 다섯 variant가 있지만 `authoringPreviewCandidate=true`는 texture parity가 닫힌
  CustomParticle과 Helix/SpriteWave 두 MIC뿐이다.
- Effect Tool은 open Authored document에서만 `Enable Exact Cooked Canary`를 명시적으로 켤 수 있다.
  Product Play가 열려 있으면 비활성이고 기본값은 OFF다.
- renderer는 exact cooked PS, MIC-keyed CB0/SRV packet과 전용 Sprite/LocalMesh carrier를 기존 Effect
  renderer 안에서 사용한다. 새 runtime renderer나 skill ID 분기는 만들지 않는다.
- exact packet 실패, unknown MIC와 꺼진 상태는 기존 family-lite로 돌아간다.
- 새 document 생성, active document 교체/discard와 Product Play 진입은 canary 선택을 강제로 OFF한다.
- Slice는 canary 대상이 아니며 사용자의 `sourceScale.size = 0.25`를 보존한다.

구현 종료 증거: variant contract check와 focused test 9건, bridge shader compile, Client Debug compile/link다.
현재 자동 빌드와 사용자 수동 결과는 대응 RESULT에서 분리해 기록한다.

### G05. 사용자 수동 canary 판정과 다음 분기 — 대기

- 사용자가 CustomParticle 한 element와 Helix 네 element를 각각 solo/restart해 first pixel, 형태, 크기,
  알파와 과노출 여부를 판정한다.
- 둘 다 유효하면 같은 family/permutation cohort로 확대한다.
- 하나만 유효하면 그 family만 확대하고 다른 family는 family-lite로 유지한다.
- 둘 다 무효면 exact canary를 끄고 family-lite 눈 튜닝 또는 스킬별 수동 복원으로 전환한다.

Product publish와 runtime admission은 이번 canary 결과와 무관하게 별도 후속 gate다.

## 6. 완료 조건

### 구현 완료

```text
class-neutral target manifest와 extractor가 존재한다.
W의 admitted family variant가 exact map -> DXBC -> native binding -> structural replay로 닫힌다.
source-value uniform expression과 texture binding은 닫히고 남은 approximate 경계가 명시된다.
CustomParticle/Helix만 Authored preview canary로 연결되며 기본 OFF와 family-lite fallback을 유지한다.
같은 variant를 다른 스킬이 재사용할 수 있고 skillId 전용 분기가 없다.
```

### 자동 검증 완료

```text
Artist frozen oracle 회귀 재현
W focused exact receipt validation
variant materialization check / bridge shader compile / Client Debug build
git diff --check
```

Artist class-neutral oracle 회귀와 Product publisher Validate는 이번 authoring canary의 완료 증거가
아니며, 실행하지 않았다면 RESULT에 미실행으로 남긴다.

### 수동 검증 완료

```text
사용자가 직접 Effect Tool에서 CustomParticle과 Helix canary의 first pixel과 visual fidelity를 판정한다.
사용자의 서면 판정 전에는 visual PASS 또는 복원 완료로 기록하지 않는다.
```

## 7. 하지 않는 것

- 차원술사 W 또는 개별 skill ID 전용 HLSL.
- raw GUID window scan 결과를 register/default 정답으로 사용하는 것.
- static set이 없는 Slice를 근거 없이 parent-default map으로 성공 처리하는 것.
- original DXBC를 Product renderer 또는 publish 산출물에 admission 없이 연결하는 것.
- authoring preview candidate를 source-exact sampler, actual VF/pass 또는 Product admission으로 부르는 것.
- 다른 세션의 family-lite, authored occurrence, 사용자 손튜닝 값을 exact admission 전에 되돌리는 것.
- 사용자 육안 판정 없이 `visual PASS`를 기록하는 것.
