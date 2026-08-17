# 차원술사 W family의 cooked shader cache join 결과

branch: `feature/effect-cooked-shader-recovery-and-transform-fix`
선행: `2026-08-17_EFFECT_COOKED_SHADER_BYTECODE_RECOVERY_IMPLEMENTATION_PLAN.md`

## 1. 현재 결론

차원술사 W의 cooked exact 경로는 G03-5 source-value uniform/texture closure까지 열렸고,
CustomParticle과 Helix/SpriteWave 두 MIC는 **사용자가 명시적으로 켜는 Authored preview canary**까지
연결됐다. 다만 기존의 **“부모 family 5/5 JOIN, uniform expression 정답표 완료”**, 뒤이은
**“4 family exact-map, Helix 미완료”**, 그리고 **“raw DXBC는 offline에서만 사용”** 판정은 현재 정본보다
이전 단계다. 현재 자동 증거로 확정된 것은 다음이다.

```text
target / occurrence                                          6 / 10
W MIC static set이 정확히 하나의 material map을 선택       5 / 6
선택 map의 uniform-expression set 구조 decode              5 / 6
구조적 pixel reference가 지목한 packed DXBC exact 추출     5 / 6
  EXACT  Glasshole / Crackhole / FluidNinja / CustomParticle / SpriteWave
  BLOCK  Slice (MIC에 static permutation resource가 없음)
exact native shader-object binding                           5 / 5
raw exact PS structural fixed-input WARP replay              5 / 5
source-value uniform CB0 expression closure                  5 / 5
source-exact native scalar-group packing                     0 / 5
source-exact texture binding                                 5 / 5
uniform texture binding / unique effective DDS              24 / 23
runtime DimensionMaster DDS parity                           4 / 5
source-exact sampler/filter/address/color-space              0 / 5
content-addressed exact DXBC variant                         5 / 5
Authored preview candidate                                   2 / 5
  CustomParticle / Helix-SpriteWave
source-value replay admission                                0 / 5
actual VF/pass admission                                    0 / 5
native binding runtime admission                            0 / 5
visual admission                                            false
Product publish/runtime admission                            false
```

따라서 현재 결과는 **6 target / 10 occurrence 중 exact 5 target의 G03-5 증거 폐쇄와 두 authoring canary**다.
다섯 exact target은 map, uniform-expression set, 구조적으로 유일한 pixel shader reference, packed DXBC와
native scalar/vector/texture binding wire를 닫은 뒤 raw 원본 PS를 합성 입력으로 실제 WARP 실행했다.
G03-5는 MIC override precedence와 uniform expression 값을 평가하고 exact texture register/resource
identity를 닫았다. 그러나 scalar-group lane/padding과 sampler/color-space, engine-owned CB row, 원본
emitter의 실제 VF/pass는 source exact가 아니다. canary도 exact cooked PS를 approximate carrier/CB0 일부/
sampler로 실행하는 **혼합 fidelity 실험 경로**이며 Product runtime 또는 bit-for-bit material 복원을 뜻하지
않는다. 사용자 first pixel과 형태 판정도 아직 미실행이다.

## 2. 증거 단계 정의

이 문서에서는 서로 다른 완료 단계를 다음처럼 부른다.

```text
PARENT_PRESENT
  base Material state GUID가 cache에 하나 이상의 map context를 가진다.

EXACT_ONE_MAP
  W MIC의 effective FStaticParameterSet이 engine-equality로 map 하나만 선택한다.

STRUCTURAL_VF_PASS_EXACT_DXBC
  exact map의 renderer-family VF 후보들이 공유하는 유일한 pixel shader reference를 고르고,
  그 reference가 지목한 packed DXBC container를 exact hash로 닫는다.
  실제 emitter VF/pass 선택을 admit했다는 뜻은 아니다.

EXACT_NATIVE_SHADER_OBJECT_BINDING
  shader type FName과 shader ID로 native shader-object 하나를 고르고, object-local
  scalar/vector/texture wire를 유일하게 닫는다. 실제 emitter VF/pass admission과는 별개다.

STRUCTURAL_FIXED_INPUT_REPLAY
  raw exact PS, signature-compatible synthetic carrier, native CB/SRV/sampler wire와 전체 MRT를
  deterministic synthetic fixture로 WARP 실행한다. 원본 MIC 값이나 실제 VF/pass를 의미하지 않는다.

SOURCE_VALUE_UNIFORM_AND_TEXTURE_CLOSURE
  MIC hierarchy override와 uniform expression을 평가하고 texture register/resource identity를 닫는다.
  native scalar-group packing과 source sampler/color-space가 열려 있으면 full source-value replay가 아니다.

ACTUAL_VF_PASS_ADMISSION
  원본 emitter ABI로 실제 vertex factory와 pass를 선택한다.

EXACT_BINDING_REPLAY_RUNTIME
  native uniform/register/texture/sampler binding의 의미, offline replay와 runtime bridge가 닫힌다.

AUTHORING_EXACT_CANARY
  사용자가 open Authored document에서 명시적으로 켠 경우에만 exact cooked PS를 제한된 MIC에 실행한다.
  candidate는 admission이 아니며 Product, actual VF/pass와 visual 상태를 올리지 않는다.

PRODUCT_VISUAL
  공용 runtime family ABI, Product 문서, draw와 사용자 수동 화면 판정까지 닫힌다.
```

현재 W는 Glasshole, Crackhole, FluidNinja, CustomParticle, SpriteWave 다섯 target이
`SOURCE_VALUE_UNIFORM_AND_TEXTURE_CLOSURE`, Slice가 `PARENT_PRESENT` 뒤에서 blocked다.
CustomParticle과 SpriteWave만 `AUTHORING_EXACT_CANARY` 후보이며, source-value full replay,
`ACTUAL_VF_PASS_ADMISSION`과 `EXACT_BINDING_REPLAY_RUNTIME`은 다섯 target 모두 false다.

## 3. 정본과 부모 Material 증거

### 3.1 cache 정본

live 설치본은 패치로 identity가 바뀌므로 exact 추출에는 사용하지 않는다. 정본은 pinned official
v974다.

```text
C:\Users\user\Desktop\Resource_LostArk\01_Extracted\Effect\
ARTIST\31470_TrackA_20260812\OfficialRefShaderCacheV974\
EV2LG3OVEH3HGV7THTFFTM7TOKMCC.v974.upk
```

live drift는 `INSTALLED_DRIFTED_PINNED_AUTHORITATIVE`로 진단한다. pinned cache와 live cache를
동시에 exact identity로 요구하지 않는다.

### 3.2 부모 Material state GUID

export serial의 tagged property stream 끝을 `end`라 할 때 `tail = serial[end:]`, state GUID는
`tail[16:32]` 규칙으로 추출했다.

```text
fx_j_pa_glasshole_02_tr        bb634bf8d8a89d41adfd3342cc9145ca
fx_k_crackholev2_01            26b6a1dbfee74a418a4876ac4ef7687a
fx_k_pa_fluidninja_01_tr       627fa9642bae264fb3921dffeb6bb6b7
fx_j_pa_customparticle_01_ad   d2f51aa37d6b3a4a9c19b5633c1e81ba
fx_m_pa_spritewave_01_ad       14aed7b23e72c045a3f00fdc2bfbee05
fx_j_pa_slice_01_tr            f49ffbfb01911043acee05efb2ac34b2
```

여섯 GUID 모두 pinned cache에서 parse 가능한 `FMaterialShaderMap` context를 가진다. 이것은
`PARENT_PRESENT 6/6`이며 W MIC가 어느 map을 쓰는지는 다음 절의 static-set equality가 판정한다.

### 3.3 map context 구조 검산

base GUID의 raw hit가 map 수보다 많은 이유를 바이트 구조로 확인했다. map마다 static set이
header와 반복 영역에 두 번 있고, 추가 한 건은 parent-default map의 `opaqueIdentity`다.

```text
base                raw  parseable static sets  map contexts
Glasshole             7            6                 3
CustomParticle       19           18                 9
Slice                 7            6                 3
Crackhole            11           10                 5
FluidNinja            7            6                 3
SpriteWave           87           86                43

invariant  parseable == 2 * map contexts
           raw       == parseable + 1
```

그러므로 raw hit 수를 permutation 수나 exact join 수로 사용하지 않는다.

## 4. W G03-1~G03-5 exact map, DXBC, binding과 source-value closure 결과

pinned v974 logical cache scan start는 `613,360,985 + 8`이다. MIC package의 NameTable로
`FStaticParameterSet`을 파싱하고, cache package의 NameTable로 map static set을 파싱한 뒤
engine-equivalent seal을 비교했다.

```text
G03-1  exact MIC map + uniform-expression set
G03-2  structural VF/pass reference + packed exact DXBC
G03-3  native shader-object + scalar/vector/texture wire
G03-4  raw exact PS structural fixed-input WARP replay
G03-5  source-value uniform evaluation + exact texture/sampler evidence
```

```text
target manifest
  Data/Effects/Imported/DimensionMaster/Materials/
  skill.2050120.clip3.exact-shader-targets.json

G03-3 receipt
  Data/Effects/Imported/DimensionMaster/Materials/
  skill.2050120.clip3.exact-material-maps.receipt.json

receipt result
  PASS_G03_3_EXPECTED_EXACT_NATIVE_BINDINGS_AND_SLICE_BLOCKED
```

```text
target          carrier         occurrence  MIC switches  map offset   uniform V/S/T  DXBC bytes  result
Glasshole       SpriteParticle       2            6        917,549,496     13/56/7       7,584    EXACT
Crackhole       SpriteParticle       1           17        905,951,377      8/43/6       5,940    EXACT
FluidNinja      SpriteParticle       1            3        872,844,808     10/41/5       4,592    EXACT
CustomParticle  SpriteParticle       1           13        889,014,114      4/30/3       4,184    EXACT
SpriteWave      MeshParticle         4           19        891,666,233     10/46/3       3,060    EXACT
Slice           SpriteParticle       1            -        -                      -           -    BLOCKED
```

```text
Glasshole       e2ba1c1ef87cdd52cc74a8e661f8613d0b17f2cc7b9b1d0d7ab6ed80ec6e775b
Crackhole       a04dc3d21a95d5f2299d6bc8d07f65bd7cde640be52ed0df362b4a71b0c680e6
FluidNinja      9c4706c3b6a36cb5a5eac40d8e871f2b67351265cf211f624614c842f0334b8e
CustomParticle  340169d8b9146fd280bbc1b509aa312e6afa71573dfe12cf4e89cb0136431ae3
SpriteWave      7917664ff360df0f6305bf8d6c41a8c506872a62e09d26beb2b8fd8d7164d76c
```

`targetCount = 6`, `occurrenceCount = 10`, `exactMaterialShaderMapCount = 5`,
`uniformExpressionSetDecodedCount = 5`, `exactPixelShaderDxbcCount = 5`,
`uniqueExactPixelShaderDxbcCount = 5`, `exactNativeShaderObjectBindingCount = 5`다. 다섯 exact row는
각각 유일하며 multi-map ambiguity가 없고, parent-default fallback은 0건이다.

SpriteParticle target의 structural VF 후보 수는 Glasshole/CustomParticle 2개,
Crackhole/FluidNinja 4개다. 각 target 안에서는 모든 후보가 같은 BasePass pixel shader reference를
공유하므로 DXBC는 하나로 닫혔다. SpriteWave는 `flocalvertexfactory` 후보 하나다. 그러나 다섯 row
모두 `actualVfPassAdmission = false`다. G03-3이 native shader-object binding을 닫았어도 원본 emitter
VF ABI를 아직 증명하지 않았기 때문이다.

### 4.1 G03-3 native shader-object table과 wire closure

RefShaderCache의 code section 뒤에는 native shader-object 265,979개가 있다. extractor는 각 object의
serialized absolute end-pointer를 다음 cursor로 사용해 table 전체를 monotonic contiguous하게
탐색했고, 마지막 object가 가리킨 `804,723,481`에서 정확히 끝났다. object size, object index 또는
binding array offset을 family별 상수로 두지 않았다.

각 exact pixel reference는 shader type FName과 shader ID로 native object 하나에 join했다. 그 object
전 범위에서 scalar/vector/texture wire 후보를 offset-free scan하고, uniform-expression 분모와 DXBC의
CB0/texture/sampler declaration까지 함께 닫히는 후보 하나만 admit했다.

```text
target          native object  material sample pairs                                  engine extra
Glasshole          262,733     t0/s1 t1/s2 t3/s3 t4/s4 t5/s5 t6/s6 t7/s7             t2/s0
Crackhole          233,960     t0/s0 t1/s1 t2/s2 t3/s4 t4/s3 t5/s5                   -
FluidNinja         159,660     t0/s1 t1/s2 t2/s4 t3/s5 t5/s3                         t4/s0
CustomParticle     197,194     t0/s2 t1/s1 t2/s0                                     -
SpriteWave         207,274     t0/s1 t1/s0 t2/s2                                     -
```

Glasshole의 `t2/s0`과 FluidNinja의 `t4/s0`은 material uniform texture가 소유하지 않는 engine sample
pair다. 이를 임의 material slot에 끼워 맞추지 않고 `unownedEngineSamplePairs`로 분리했다. 다섯 row의
native binding은 exact지만 `runtimeAdmission`과 `actualVfPassAdmission`은 모두 false다.

### 4.2 G03-4 raw DXBC structural fixed-input replay

```text
G03-4 tool
  Tools/EffectPipeline/replay_ue3_material_pixel_shaders.py

G03-4 receipt
  Data/Effects/Imported/DimensionMaster/Materials/
  skill.2050120.clip3.structural-fixed-input-replay.receipt.json

receipt result
  PASS_G03_4_STRUCTURAL_FIXED_INPUT_REPLAY_SOURCE_VALUE_REPLAY_BLOCKED
```

다섯 raw cooked PS는 모두 `ID3D11Device::CreatePixelShader`를 통과했다. 도구는 각 PS의 ISGN으로
signature-compatible fullscreen carrier VS를 생성하고, 컴파일된 OSGN을 PS ISGN의 semantic,
component type과 mask에 다시 대조한다. target마다 shader input 8개와 rasterizer-owned
`SV_IsFrontFace` 1개가 닫혔다.

native CB0 slot과 material/engine sample pair는 live DXBC declaration 및 실제 sample pair와 다시
대조했다. Glass/Fluid의 SceneDepth와 Glass/Crack/Fluid의 CB2는 source-exact 값이 아니라 명시적
synthetic engine fixture로 공급했다. 전체 MRT는 다음처럼 검증했다.

```text
Glasshole / Crackhole / FluidNinja  o0,o2,o3,o4,o5 written, o1 sentinel 유지
CustomParticle / SpriteWave        o0 written
RT0 nonzero                         5 / 5
external opacity cb0[0].x sensitivity 5 / 5
```

Glass/Crack/Fluid는 외부 opacity 변화가 RT0 alpha를 바꿨다. additive CustomParticle/SpriteWave는
원본 PS가 `o0.w=0`을 고정하므로 RT0 RGB 변화로 입력 소비를 증명했다. 이 차이를 공통 alpha PASS로
왜곡하지 않았다.

fixture의 상수, 1x1 RGBA32F texture와 point-clamp sampler는 구조 검증용 합성 값이다. 따라서
`structuralFixedInputReplayAdmission=true`만 올렸고 다음 세 blocker를 명시했다.

```text
UNIFORM_EXPRESSION_FOLDEDMATH_ORDINAL_0_SEMANTICS_UNPROVEN
PARENT_DEFAULT_TEXTURE_CLOSURE_INCOMPLETE
SOURCE_SAMPLER_FILTER_AND_ADDRESS_EVIDENCE_INCOMPLETE
```

`sourceValueReplayAdmission`, actual VF/pass, runtime과 visual admission은 모두 false다.

### 4.3 G03-5 source-value uniform CB0 expression closure

```text
tool
  Tools/EffectPipeline/evaluate_ue3_material_uniform_expressions.py

targets
  Data/Effects/Imported/DimensionMaster/Materials/
  skill.2050120.clip3.source-value-uniform-evaluation.targets.json

receipt
  Data/Effects/Imported/DimensionMaster/Materials/
  skill.2050120.clip3.source-value-uniform-evaluation.receipt.json

result
  PASS_G03_5_SOURCE_VALUE_UNIFORM_CB0_CLOSED_TEXTURE_SAMPLER_REPLAY_OPEN
```

다섯 exact MIC의 root-to-leaf scalar/vector override precedence와 selected map uniform expression을
IEEE754 float32 연산 순서로 평가했다. FoldedMath ordinal 0은 source-proven add, ordinal 2는 multiply다.
Periodic은 `x - floor(x)`가 아니라 UE3의 `x - trunc(x)`로 교정했고 negative input 회귀로 고정했다.

```text
sourceValueUniformCb0ClosureCount        5 / 5
sourceValueReplayAdmissionCount          0 / 5
actualVfPassAdmissionCount               0 / 5
runtimeAdmissionCount                    0 / 5
visualAdmissionCount                     0 / 5
```

여기서 uniform expression 값의 closure와 native scalar group packing은 다른 증거다. vector row는
source expression 순서와 값으로 고정할 수 있지만 scalar group의 lane order/padding source ABI는 아직
증명되지 않았다. 따라서 variant packet의 scalar-group row는
`PROJECT_PREVIEW_APPROXIMATE_SCALAR_GROUP_PACKING_UNPROVEN`이며, 이 결과를 full source-value replay나
bit-for-bit material 복원으로 부르지 않는다.

### 4.4 G03-5 exact texture binding과 sampler evidence

```text
tool
  Tools/EffectPipeline/extract_ue3_material_texture_sampler_closure.py

receipt
  Data/Effects/Imported/DimensionMaster/Materials/
  skill.2050120.clip3.exact-texture-sampler-closure.receipt.json

result
  PASS_G03_5_EXACT_TEXTURE_BINDINGS_SAMPLER_AND_RUNTIME_DDS_BLOCKERS_EXPLICIT
```

```text
exact target                              5
uniform texture binding                  24
unique effective texture                23
source-exact texture binding target       5 / 5
runtime DimensionMaster DDS parity target 4 / 5
source-exact sampler target               0 / 5
source-value texture/sampler target        0 / 5
```

uniform texture expression, native t/s wire, full FName MIC override와 source Texture2D export identity는
닫혔다. CustomParticle과 Helix/SpriteWave는 canary에 필요한 DDS 3/3이 각각 runtime parity다.
반면 Glasshole에는 runtime DimensionMaster DDS가 빠진 binding이 있어 전체 parity가 false다.

Texture2D export가 `AddressX/Y`, `SRGB`, `Filter`를 생략한 경우 constructor default 후보만 알 수 있고,
`TF_Default`를 실제 filter로 해석할 source revision TextureLODSettings도 없다. 그래서 authoring canary는
sampler/filter/address/color-space를 `PROJECT_PREVIEW_APPROXIMATE`로 명시하고 source-exact sampler
admission을 올리지 않는다.

### 4.5 일반화 도구가 보존해야 할 파서 경계

```text
objectredirector shadow
  같은 object path의 material export와 redirector를 last-wins dict로 고르지 않는다.
  export index와 className을 함께 검증한다.

parent import/export
  parent가 음수 import index로 다른 package에 있는 경우와 양수 export로 같은 package에 있는
  경우를 모두 처리한다.

NameTable ownership
  MIC static set은 MIC package NameTable, cache map은 cache NameTable로 해석한다.

vertex factory count
  Artist의 expected 5/8을 W 정답으로 복제하지 않는다.
  W exact map 실측은 Glasshole 3, CustomParticle 3, Crackhole 6, FluidNinja 7, SpriteWave 7이다.
```

## 5. content-addressed exact variant와 Authored preview canary

### 5.1 variant materialization

```text
tool
  Tools/EffectPipeline/materialize_ue3_exact_cooked_shader_variants.py

contract
  Data/Effects/Contracts/ue3-exact-cooked-shader-variants.v1.json

blob root
  Data/Effects/CookedShaders/<full-sha256>.dxbc

result
  PASS_SOURCE_VARIANTS_MATERIALIZED_PREVIEW_CANDIDATES_ONLY
```

다섯 exact PS는 byte count와 full SHA-256을 파일명으로 갖는 content-addressed blob으로 고정됐다.
variant selector는 skill/class/occurrence가 아니라 다음 key다.

```text
base family + effective FStaticParameterSet + shader platform + VF + pass/shader type
```

contract는 5 variant를 담지만 authoring 후보는 다음 두 MIC뿐이다.

```text
CustomParticle
  variant  ue3.exact-cooked-ps.eb070deb6feaeb22d0e1c901
  MIC      fx_m_mi_j_00.fx_mi.fx_j_pa_customparticle_01_06_ad
  carrier  SpriteParticle

Helix / SpriteWave
  variant  ue3.exact-cooked-ps.42fbc42e48682c7c74a79457
  MIC      fx_m_mi_k_00.fx_mi.fx_k_me_spritewave_01_45_ad
  carrier  LocalMesh
```

`authoringPreviewCandidate=true`는 admission이 아니다. contract summary도 actual VF/pass 0,
Product runtime 0, visual 0을 유지하고 runtime DataFiles publish를 금지한다.

### 5.2 Effect Tool 실행 경계

Effect Tool은 project data root의 contract와 content-addressed DXBC를 lazy-load하고, 위 두
`sourceMaterialPath` MIC가 있는 open Authored document에서만 `Enable Exact Cooked Canary`를 허용한다.

```text
기본 상태             OFF
허용 입력             open Authored document + exact MIC match
Product Play          checkbox 비활성 / execution OFF
미일치 Element        기존 family-lite
load/stage/draw 실패  기존 family-lite 보존 + status에 이유
선택 key              sourceMaterialPath MIC exact match
lifecycle reset       새 document / active document 교체·discard / Product Play에서 OFF
```

renderer는 기존 `CEffectDocumentRenderer` 안에서 exact PS packet을 stage한다. 별도 runtime renderer,
차원술사 W 전용 HLSL 또는 skill ID switch는 없다. raw exact PS의 출력 alpha가 0인 additive target을
기존 SrcAlpha additive로 소거하지 않도록 canary 전용 ONE/ONE blend를 사용하고, draw 뒤 D3D11 상태를
복원한다.

fidelity 경계는 Tool 화면에도 그대로 표시된다.

```text
exact        cooked pixel DXBC
exact        vector CB0 expression row, texture register/resource/DDS parity
approximate  native scalar-group packing
approximate  sampler/filter/address/color-space
approximate  Sprite/LocalMesh carrier bridge와 actual VF/pass
false        source-value full replay, Product runtime, visual admission
```

### 5.3 현재 compile과 화면 상태

```text
PASS        exact runtime scaffold 포함 Client x64 Debug compile/link
PASS        Shader_EffectExactSpriteBridge.hlsl FXC compile
PASS        Shader_EffectExactLocalMeshBridge.hlsl FXC compile
PASS        Effect Tool checkbox/lazy-loader/lifecycle invalidation 포함 최종 Client x64 Debug link
             0 errors, 기존 C4819/LNK4099 warning 23건
미실행      Client/Effect Tool 자율 실행
미실행      사용자 CustomParticle/Helix first pixel 판정
```

최종 compile은 현재 Tool 통합 파일까지 포함한다. warning 23건은 기존 C4819/LNK4099이며 오류 0건이다.
이 결과는 실행 화면 또는 visual fidelity PASS가 아니다.

### 5.4 사용자 수동 확인 경로

```text
F1 -> Effect Tool -> Data Files -> DimensionMaster
-> effect.dimensionmaster.skill.2050120.clip3.unified
-> Open for Editing
-> Enable Exact Cooked Canary
-> matching element Solo -> Restart Preview
```

판정 순서는 CustomParticle 한 개, Helix/SpriteWave 네 개다.

```text
CustomParticle
  authored.source-particle.dc486c58e3b368e683fa8f03

Helix / SpriteWave
  authored.source-particle.0d483237146fa5d618d39a7f
  authored.source-particle.da2a381f72e08dbbe43e42f3
  authored.source-particle.0b38a1bdf89659a65e1ed571
  authored.source-particle.798b11b002a959023824d209
```

Product Play는 이 canary 경로가 아니다. 결과는 각각 `보임/안 보임`, 형태, 크기, 과노출 또는 카드
노출 여부로 기록한다. 사용자가 관찰을 적기 전에는 이 문서가 대신 PASS를 선언하지 않는다.

## 6. Slice가 blocked인 이유

W의 `fx_mi.fx_k_pa_slice_01_02_tr` export는 `property_end == serial_size`로 tail이 0바이트이고,
property stream에 `bHasStaticPermutationResource`가 없다. 즉 extract할 MIC
`FStaticParameterSet`이 없다.

UE3 의미상 이 MIC가 parent resource를 사용할 가능성이 높고, parent-default map 후보도 하나 있다.

```text
candidate  918,940,653 -> 918,943,009
switches   0
vf blocks  3
friendly   FX_J_Pa_Slice_01_Tr
```

그러나 이것은 아직 측정된 MIC key가 아니라 도입하려는 fallback 규칙이다. static set이 없다는
이유만으로 parent-default map을 성공 처리하면 silent identity fallback이 된다. EFEngine의
`UMaterialInstance::GetMaterialResource` / `FMaterialShaderMap::FindId` 선택 ABI 근거를 확보할
때까지 Slice는 `BLOCKED_MISSING_STATIC_SET_RULE`로 유지한다.

현재 family-lite Slice와 사용자의 `sourceScale.size = 0.25` 편집은 exact runtime admission 및
사용자 수동 승인 전까지 되돌리지 않는다.

## 7. raw uniform-expression window scan 정정

`Tools/EffectPipeline/extract_ue3_material_uniform_expressions.py`는 exact extractor가 아니다.
현재 docstring도 `RECONNAISSANCE ONLY`로 고정했다.

```text
현재 정찰 산출물
  Data/Effects/SourceCatalog/Reconnaissance/
  ue3-uniform-expression-window-scan.recon.json

삭제된 계약 위치
  Data/Effects/Contracts/ue3-uniform-expression-tables.v1.json
```

이 scanner는 첫 GUID hit 주변 고정 window를 4바이트씩 훑는다. 인접한 여러 material map이 같은
창에 들어오기 때문에 family별 expression count와 값은 한 map의 정답표가 아니라 합집합이다.

또한 다음 주장은 철회한다.

```text
철회  uniform expression 직렬화 순서가 곧 constant register 순서다.
철회  window에서 발견한 scalar 값이 W exact variant의 원본 기본값이다.
철회  texture 이름 목록만으로 alpha/distortion 합성식이 닫힌다.
철회  Slice texture가 세 번 보였으므로 한 map에서 독립 UV 세 개를 샘플한다.
```

scalar는 float4 packing과 미사용 제거를 거치고 vector/texture/sampler는 native binding table을
따로 가진다. G03-3은 선택된 map 하나의 shader-object binding wire와 DXBC register 선언을 함께
파싱해 다섯 exact native binding을 닫았다.

G03-2에서 decode한 `uniformExpressionSet`은 첫 GUID 주변 창이 아니라 exact MIC map 내부의 구조적
배열이므로 위 철회와 충돌하지 않는다. G03-3의 native wire도 object 전체 offset-free scan과 DXBC
declaration closure로 고른 구조적 증거다. G03-5 evaluator는 이 exact 구조와 MIC override로 값을
평가하지만, raw window 순서를 근거로 쓰지 않는다. scalar-group packing과 sampler, actual VF/pass가
열려 있으므로 여전히 Product 수치 정답이나 full source-value replay로 승격하지 않는다.

정찰에서 발견한 `in_opa_*`, `diff2_tile_*`, `cast_*` 같은 이름은 family 가치와 후보 의미를
찾는 데는 쓸 수 있다. 수치 적용이나 Product admission에는 쓰지 않는다.

## 8. Helix Mesh / SpriteWave 상태

차원술사 W에는 sprite family와 별도로 Helix mesh particle 네 occurrence가 있다.

```text
mesh          fm_m_helix_011.wmodel
MIC           fx_m_mi_k_00.fx_mi.fx_k_me_spritewave_01_45_ad
parent        fx_m_mi_m_00.fx_m.fx_m_pa_spritewave_01_ad
base texture  fx_m_spark_001.dds
geometry      Mesh Particle
```

이 대상은 Sprite family에 억지로 포함하지 않고 `SpriteWave + MeshParticle` variant로 분리했다.
네 occurrence가 target 하나를 공유하며, W MIC의 effective static set으로 exact map 하나,
`flocalvertexfactory` structural 후보 하나, 유일한 BasePass pixel shader reference와 3,060-byte
DXBC를 봉인했다. 따라서 SpriteWave는 이제 G03-2의 다섯 exact target과 10 occurrence denominator에
포함된다.

G03-3은 SpriteWave pixel reference를 native shader-object index `207,274`와 exact join하고
`t0/s1`, `t1/s0`, `t2/s2` wire를 유일하게 닫았다. 그래도 `actualVfPassAdmission`은 false다. source가
MeshParticle이고 structural 후보가 Local VF 하나라는 사실과 pixel native binding은 원본 emitter의
실제 VF/pass 선택 ABI를 대신하지 않는다. 현재 LocalMesh bridge로 Authored preview canary 후보에는
올랐지만 carrier는 approximate이고 Product runtime admission은 false다. canary를 껐거나 draw가
실패하면 기존 family-lite/runtime 경로를 유지한다.

## 9. Artist exact 경로와 현재 일반화 방향

Artist extractor가 멈춘 이유는 cooked 데이터가 없어서가 아니다.

```text
의도적 canary scope
  31470 manifest, schema, recipe ID, denominator가 hardcoded였다.

열린 runtime 경계
  actualVfPassSelection / registryAdmission / productRuntimeAdmission이 false였다.

잘못된 초기 분모
  전투 RefShaderCache가 아니라 로비·class-select cache 11개를 조사해 0/23이 나왔다.

과검증과 우선순위 전환
  한 스킬의 evidence/admission framework가 커졌고 Product pixel은 닫히지 않아
  Effect Tool authoring-first로 이동했다.
```

현재는 pinned v974와 W 5 exact map + uniform set + DXBC + native binding 증거가 있으므로 전제가
바뀌었다. 기존
Artist extractor와 receipt는 frozen oracle로 유지하고, 새 class-neutral extractor가 그 parser와
packed-code primitive를 import한다. Artist canary 자체의 actual VF/pass, native binding과 Product
admission은 여전히 false이며, 이번 W 결과가 그 중단된 admission을 소급해 완료하지 않는다.

재사용 key는 다음과 같다.

```text
base family + effective static set + platform + vertex factory + pass/shader type
```

W는 첫 fixture일 뿐이며 `skillId == 2050120` 또는 차원술사 전용 HLSL 분기를 만들지 않는다.
같은 key를 쓰는 다른 스킬과 클래스는 같은 compiled family variant를 재사용한다.

## 10. 남은 구현 순서

```text
완료       W target manifest 6 target / 10 occurrence
완료       exact map + uniform-expression set 5, Slice blocked 1
완료       structural pixel reference -> packed DXBC 5
완료       G03-3 native shader-object table + exact binding 5
완료       G03-4 raw exact PS structural fixed-input replay 5/5
완료       G03-5 source-value uniform CB0 expression closure 5/5
완료       G03-5 exact texture binding 5/5, 24 binding / 23 DDS
열림       source-exact native scalar-group packing 0/5
열림       source-exact sampler/filter/address/color-space 0/5
완료       content-addressed exact DXBC variant 5
완료       CustomParticle + Helix Authored preview canary 연결
완료       Tool 통합 뒤 최종 Client x64 Debug compile/link (0 errors)
대기       사용자 CustomParticle/Helix first pixel과 형태 판정
열림       actual VF/pass admission 0/5, Product runtime admission 0/5
후속       Slice parent-default ABI 근거 확보 또는 fail-closed 유지
후속       사용자 판정이 승인한 family/permutation cohort만 확대
```

각 family는 독립적으로 admit한다. 실패한 family만 현재 family-lite를 유지하며, 다른 family의 exact
진행을 막지 않는다.

### 사용자 판정 뒤 결정 트리

```text
CustomParticle와 Helix 모두 유효
  -> 동일 family + static set + platform + VF + pass cohort로 확대

둘 중 하나만 유효
  -> 성공한 family만 확대
  -> 실패 family는 family-lite 유지

둘 다 무효 또는 화면상 이득 없음
  -> Exact Cooked Canary OFF
  -> family-lite renderer에서 element 눈 튜닝
  -> 필요한 스킬만 하나씩 수동 복원
```

어느 분기에서도 이번 canary만으로 Product publish 또는 visual PASS를 선언하지 않는다.

## 11. 검증 상태

### 구현 상태

```text
완료       부모 state GUID 6개 추출
완료       class-neutral target manifest와 G03-2 extractor
완료       W 6 target / 10 occurrence 분모 고정
완료       W MIC 5 target -> EXACT_MATERIAL_SHADER_MAP
완료       exact map uniform-expression set 5개 구조 decode
완료       structural pixel reference가 지목한 exact DXBC 5개 추출
완료       shader-object 265,979개 serialized end-pointer 순차 탐색
완료       다섯 exact DXBC -> native shader-object와 offset-free wire unique join
완료       Glasshole t2/s0, FluidNinja t4/s0 engine extra 분리
완료       Slice를 silent parent-default 없이 BLOCKED로 분류
완료       raw window scan을 reconnaissance로 강등
완료       G03-3 exact native shader-object binding 5/5
완료       G03-4 structural fixed-input WARP replay 5/5
완료       G03-5 source-value uniform CB0 expression closure 5/5
완료       G03-5 source-exact texture binding 5/5
완료       5 exact DXBC content-addressed variant materialization
완료       CustomParticle/Helix Authored preview canary 코드 연결
미완료     sourceValueReplayAdmission 0/5
미완료     source-exact native scalar-group packing 0/5
미완료     source-exact sampler/filter/address/color-space 0/5
미완료     actualVfPassAdmission 0/5
미완료     Product runtime admission/publish
```

### 자동 검증 상태

```text
실행함     pinned cache map context 구조와 2N+1 불변식 확인
실행함     W 다섯 MIC static-set equality unique-map 판정
실행함     exact map uniform-expression set decode 5건
실행함     packed descriptor/code blob/DXBC identity 5건 봉인
실행함     focused extractor unit test 20건 PASS
실행함     extractor --check PASS (exact=5, dxbc=5, native=5, blocked=1)
실행함     extractor/test py_compile PASS
실행함     target manifest와 G03-3 receipt JSON parse PASS
실행함     focused structural replay unit test 19건 PASS
실행함     raw DXBC structural replay --check PASS (targets=5)
실행함     G03-4 tool/test py_compile과 receipt JSON parse PASS
실행함     focused uniform evaluator unit test 7건 PASS
실행함     uniform evaluator --check PASS (targets=5, sourceReplay=0, runtime=0, visual=0)
실행함     focused texture/sampler closure unit test 5건 PASS
실행함     texture/sampler --check PASS (targets=5, bindings=24, textures=23, samplerExact=0)
실행함     focused exact variant materializer unit test 9건 PASS
실행함     variant materializer --check PASS (variants=5, blobs=5, previewCandidates=2, runtime=0)
실행함     exact runtime scaffold 포함 Client x64 Debug compile/link PASS
실행함     exact Sprite/LocalMesh bridge HLSL 두 파일 FXC compile PASS
실행함     raw scanner 계약 경로 제거와 정찰 경로 존재 확인
실행함     Tool checkbox/lazy-loader/lifecycle invalidation 포함 최종 Client x64 Debug link PASS
             (0 errors, 기존 C4819/LNK4099 warning 23건)
실행함     첫 사용자 canary 시도에서 Helix 설치가 `Exact preview CB0 is absent`로 fail-closed
확인함     Helix/Custom DXBC는 ISGN/OSGN/SHEX만 보존하고 RDEF는 제거된 컨테이너
수정함     완전한 RDEF-stripped 경우에만 sealed sidecar의 CB0/t/s 계약을 사용
             (Helix CB0 22행/352B, Custom CB0 13행/208B)
실행함     RDEF-stripped 수정 뒤 Client x64 Debug compile/link PASS
실행함     Client.vcxproj / Client.vcxproj.filters XML parse PASS
실행함     exact variant contract JSON parse PASS
실행함     git diff --check PASS
미실행     Artist frozen oracle class-neutral 회귀
미실행     Product publisher Validate/Publish
```

### 수동 검증 상태

```text
재검증 대기 첫 시도는 RDEF 메타데이터 오판으로 픽셀 draw 전에 차단됨
미실행     수정 빌드의 CustomParticle canary first pixel/형태 판정
미실행     수정 빌드의 Helix 네 occurrence canary first pixel/형태 판정
미실행     family/permutation cohort 확대 승인
```

사용자의 서면 판정 전에는 `visual PASS`, `복원 완료` 또는 occurrence 승인을 기록하지 않는다.
