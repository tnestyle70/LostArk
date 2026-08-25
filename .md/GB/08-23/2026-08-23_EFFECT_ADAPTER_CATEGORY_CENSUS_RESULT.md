# 2026-08-23 Effect Adapter 카테고리 전수 조사 결과

base: `origin/main@d6b27084` (PR #172)

입력 정본: `Data/Effects/Contracts/effect-tuple-cohort-inventory.v1.json` (PR #170),
`Data/Effects/MaterialPrograms/effect-material-program-registry.v1.json` (PR #168/#172),
`Client/Public/Effect_MaterialProgramRegistry.h`, `Client/Public/Effect_AuthoringDocument.h`

CSV 시트: [`census/`](census/)

이 문서는 4캐릭터와 Valtan authored Effect **416 문서 / 7,566 occurrence** 전수를
`carrier × renderProfile` = **Adapter 카테고리**로 분류한 실측이다. runtime 구현도
Product 승격도 아니며, 사용자 육안 판정은 하나도 기록하지 않는다.

---

## 1. 결론 네 줄

1. Product에 실제로 실린 **2,554 occurrence 전부가 16개 Adapter 카테고리 안에 들어간다.**
   상위 5개가 89.2%다.
2. 그중 compiled adapter가 존재하는 카테고리는 **1개**(`SPRITE / alpha_two_sided`)이고,
   실제 binding은 **1행**(도화가 F `sprite.2b3dc6842507e910`)이다.
3. 도화가 F 17행은 7개 카테고리를 덮는다(Product의 62.7%). **그러나 additive 4종을 하나도
   갖고 있지 않다.** 도화가 F만으로 검증하면 Product의 **34.3%(876행)** 가 표본 밖이다.
4. 도화가 F의 12개 tuple cohort 총원은 **19행이고 전부 Artist 도메인**이다. cohort 기구로는
   Valtan·타 클래스가 한 행도 딸려오지 않는다. 수평 확장이 실제로 일어나는 축은
   **Adapter 하나뿐**이다.

---

## 2. 용어 — 세 ID가 각각 무엇을 소유하는가

`Client/Public/Effect_MaterialProgramRegistry.h`의
`EFFECT_RESOLVED_MATERIAL_PROGRAM_BINDING`이 정본이다.

```text
binding key = (effectAssetId, elementId)          ← occurrence 한 행
  ├─ programId     어떤 수학식으로 pixel을 계산하는가   (backend + opcode)
  ├─ layoutId      그 식의 입력을 GPU에 어떻게 배선하는가 (t#/s#, scalar/vector packing, mask)
  ├─ descriptorId  그 배선에 어떤 값을 넣는가            (DDS asset, sampler, scalar 값)
  └─ adapterId     그 식을 어느 몸·pass·state로 draw하는가 (carrier, VF, MRT, blend/depth/cull)
```

네 개의 성질이 다르다.

| ID | 재사용 범위 | 누가 소유 | 확장 비용 |
|---|---|---|---|
| `adapterId` | **가장 넓다.** carrier×state가 같으면 전부 공유 | **C++ compiled allowlist** | carrier당 1회 C++ 작업 |
| `programId` | equation이 같은 occurrence 집합 | registry JSON + HLSL evaluator | opcode당 1회 HLSL 작업 |
| `layoutId` | ABI packet이 같은 집합 | registry JSON | packet당 1회 |
| `descriptorId` | **occurrence 1:1** | registry JSON | 행당 데이터만 |

Adapter는 JSON이 **고를 수만** 있고 저작할 수 없다. `Effect_MaterialProgramRegistry.h`
주석 그대로다 — *"Registry JSON can select eAdapterId, but cannot author or replace any field
below."* 그래서 Mesh/Decal은 데이터가 없어서가 아니라 **compiled adapter가 없어서** 막혀 있다.

현재 compiled 실체는 정확히 하나다.

```cpp
enum class EFFECT_COMPILED_MATERIAL_ADAPTER_ID : uint8_t {
    SPRITE_PARTICLE_SCENE_COLOR_RT0_ZERO_DISTORTION_RT1_ALPHA_TWO_SIDED_V1,
    END,
};
enum class EFFECT_COMPILED_MATERIAL_CARRIER : uint8_t { SPRITE_PARTICLE, END, };
```

그리고 `Materialize_Binding`은 `adapterId != sprite || backend != RUNTIME_MATERIAL_V2 ||
opcode != 6 || executionVersion != 1`이면 전부 거부한다.

---

## 3. Product 기준 Adapter 카테고리 우선순위 (전수)

[`census/08_product_adapter_priority.csv`](census/08_product_adapter_priority.csv)

| # | carrier | renderProfile | Product행 | 누적 | 도화가F | typed준비 | compiled |
|---:|---|---|---:|---:|---:|---:|:---:|
| 1 | SPRITE | alpha_two_sided | 601 | 23.5% | 4 | 6 | **YES** |
| 2 | SPRITE | **additive_one_sided** | 501 | 43.1% | **0** | 0 | NO |
| 3 | MESH | alpha_two_sided | 481 | 62.0% | 5 | 7 | NO |
| 4 | SPRITE | alpha_one_sided | 422 | 78.5% | 2 | 4 | NO |
| 5 | SPRITE | **additive_two_sided** | 273 | 89.2% | **0** | 12 | NO |
| 6 | MESH | alpha_one_sided | 83 | 92.4% | 2 | 14 | NO |
| 7 | MESH | **additive_one_sided** | 76 | 95.4% | **0** | 0 | NO |
| 8 | DECAL | **alpha_two_sided** | 72 | 98.2% | **0** | 0 | NO |
| 9 | MESH | **additive_two_sided** | 26 | 99.3% | **0** | 0 | NO |
| 10 | RIBBON | alpha_two_sided | 10 | 99.6% | 1 | 2 | NO |
| 11 | DECAL | alpha_one_sided | 2 | 99.7% | 2 | 2 | NO |
| 12~14 | RIBBON | additive_two / alpha_one / additive_one | 5 | 99.9% | 0 | 0 | NO |
| 15 | MESH | opaque_back_depth_write | 1 | 100.0% | 1 | 1 | NO |
| 16 | (Light presentation) | — | 1 | 100.0% | 0 | 0 | N/A |

`EFFECT_RENDER_PROFILE`은 5종이고(`Effect_AuthoringDocument.h:85`) carrier 4종과 곱해
실사용 16칸이 나온다. **renderProfile → render state 삼중항은 이미 코드에 확정돼 있다**
(`Client/Private/Effect_DocumentRenderer.cpp:7570-7595`).

| renderProfile | Rasterizer | DepthStencil | Blend |
|---|---|---|---|
| `alpha_two_sided_depth_read` | `RS_Cull_None` | `DSS_ReadOnly` | `BS_EffectAlpha` |
| `additive_two_sided_depth_read` | `RS_Cull_None` | `DSS_ReadOnly` | `BS_EffectAdditive` |
| `alpha_one_sided_depth_read` | `RS_Default` | `DSS_ReadOnly` | `BS_EffectAlpha` |
| `additive_one_sided_depth_read` | `RS_Default` | `DSS_ReadOnly` | `BS_EffectAdditive` |
| `opaque_back_depth_write` | `RS_Default` | `DSS_Default` | `BS_EffectOpaque` |

즉 **state는 5종 모두 이미 있고 검증까지 걸려 있다.** compiled adapter desc가 alpha_two_sided
하나만 채워져 있을 뿐이다. `BS_EffectAdditive`도 legacy typed 경로에서 이미 쓰이고 있다
(`Effect_DocumentRenderer.cpp:1586, 1811, 7582, 7592`). 따라서 같은 SPRITE carrier에서
renderProfile만 늘리는 작업은 **새 state를 만드는 일이 아니라 adapter enum과 desc 항목을
4개 추가하는 일**이다.

---

## 4. 도화가 F 17행 — golden control의 실제 배선

[`census/04_artistF_31470_golden_control.csv`](census/04_artistF_31470_golden_control.csv)

`Data/Effects/Authored/effect.artist.skill.31470.unified.effect.json`은 **17 element**다.
`Effect_Artist31470ShaderRegistry.cpp`의 **35행**과 섞지 않는다. 다만 그 registry를 "legacy"라고
부르면 틀린다. **지금도 컴파일되고 실행 중이다** (`Client.vcxproj:193`,
`Effect_DocumentRenderer.cpp:8553` `Validate_Artist31470ShaderRegistry()`).
두 숫자는 세는 대상이 다르다.

```text
35 = 원본 UPK source emitter 행 (FX_PC_SDM_07 … particlespriteemitter_N)
17 = authored runtime element 행
```

17행 전부가 `program=TYPED_RUNTIME_PROGRAM_DECLARED`, `layout=TYPED_PACKET_CLOSED`,
`adapter=TYPED_STATIC_DISPATCH_CANDIDATE`, `descriptor=TYPED_VALUES_CLOSED`다.
즉 **데이터는 이미 다 닫혀 있고, 막힌 곳은 compiled adapter와 registry binding뿐이다.**

| carrier | renderProfile | opcode | elementId | binding |
|---|---|---:|---|:---:|
| SPRITE | alpha_two | **6** | `sprite.2b3dc6842507e910` | **BOUND** |
| SPRITE | alpha_two | 6 | `sprite.c65181324417a1a8` | — |
| SPRITE | alpha_two | 11 | `sprite.55a9fb0e7c1b18c1` | — |
| SPRITE | alpha_two | 13 | `sprite.ecd6e1e27fae5476` | — |
| SPRITE | alpha_one | 6 | `sprite.038c80fb6ea02c1a` | — |
| SPRITE | alpha_one | 6 | `sprite.bfa4e874bafe2a43` | — |
| MESH | alpha_two | 2 | `mesh.a502a64a218e7619` | — |
| MESH | alpha_two | 3 | `mesh.062366ee9f9655d3` | — |
| MESH | alpha_two | 7 | `mesh.6ded917f52544378` | — |
| MESH | alpha_two | 7 | `mesh.8876da26a729cca0` | — |
| MESH | alpha_two | 8 | `mesh.646163c341579b56` | — |
| MESH | alpha_one | 1 | `mesh.827d0c725afc9c37` | — |
| MESH | alpha_one | 1 | `mesh.8165115d36cd1a8a` | — |
| MESH | opaque_back | 1 | `mesh.cc04feee8a36940b` | — |
| DECAL | alpha_one | 14 | `decal.f3b5c3b63b4a7e34` | — |
| DECAL | alpha_one | 14 | `decal.6f78bff02c657a14` | — |
| RIBBON | alpha_two | 9 | `ribbon.a6fe27caa16b2630` | — |

현재 bound된 1행의 완전한 tuple은 다음과 같다. 나머지 16행이 따라야 할 형식이다.

```text
programId    effect.program.runtime-material-v2.opcode-6.artist-f-sprite.v1
             backend=runtimeMaterialV2  opcode=6
layoutId     effect.layout.runtime-material-v2.artist-f-sprite.v1
             lane.0 sparkle_tex          t0 / s5 / srgb
             lane.1 edgedeco.texture01   t1 / s6 / srgb
             scalar 3개 (packedIndex 0,1,2), vector 0개
             inputCount 6, inputConsumedMask [55,0], inputSuppressedMask [8,0]
             dynamicConsumedMask 15, particleColorPolicy 3
descriptorId effect.descriptor.artist-f.sprite-2b3dc6842507e910.v1
             lane.0 = Effect/Artist/Textures/fx_d_noise_003.dds  (linear/wrap)
             lane.1 = Effect/Artist/Textures/fx_c_noise_002.dds  (linear/wrap)
             scalar.0=30  scalar.1=1.5  scalar.2=0.1
adapterId    effect.adapter.sprite-particle.scene-color-rt0.zero-distortion-rt1.alpha-two-sided.v1
             shader   Shader_VtxEffectParticle.hlsl / VTXEFFECT_PARTICLE
             pass 1,  MRT_SceneHDR
             RT0 SV_TARGET0 sceneColor,  RT1 SV_TARGET1 distortion(결정적 0)
             RS_Cull_None / DSS_ReadOnly / BS_EffectAlpha,  stencilRef 0
```

**도화가 F가 갖지 않은 것: additive 4종 전부, DECAL alpha_two.**
이 다섯 칸이 Product의 34.3% + 2.8%다. golden control로서는 훌륭하지만
**additive 표본이 0이라 blend 경로를 증명하지 못한다.**

---

## 4-1. 지금 registry는 화면을 바꾸지 않는다 — 검증된 그림자다

이 절이 이 문서에서 가장 중요하다. `Effect_DocumentRenderer.cpp:15215-15235`를 보면
binding이 있을 때 하는 일은 다음이 전부다.

```cpp
if (... || !CEffectMaterialProgramRegistry::Is_ExecutionBitExact(
        pMaterialProgramBinding->Execution, Element.Material.Execution))
{
    strOutError = "Bound material-program packet differs from inline golden mirror: " ...
    return false;
}
RegistryMaterializedElement = Element;
RegistryMaterializedElement.Material.Execution = pMaterialProgramBinding->Execution;
```

registry Execution이 authored inline Execution과 **bit-exact일 것을 요구한 뒤 그 동일한 값을
다시 대입한다.** 오류 메시지 자체가 `inline golden mirror`라고 말한다. 그리고 실제 draw state는
여전히 inline `Element.Material.eRenderProfile`에서 나온다
(`Select_Pass`, `Effect_DocumentRenderer.cpp:7556`, `:8000`의 renderProfile switch).

즉 **bound element와 unbound element의 staged 결과가 같다. 현재 registry는 픽셀을 0개 바꾼다.**

이것은 결함이 아니라 PR #168이 의도한 Binding 0 단계다. 그러나 결과적으로 다음이 성립한다.

- adapter enum을 늘려도, binding을 늘려도 **그것만으로는 화면이 바뀌지 않는다.**
- 따라서 어떤 확장 작업보다 먼저 **registry를 mirror에서 authority로 뒤집어야 한다.**

---

## 5. 수평 확장이 cohort로 일어나지 않는 이유

[`census/06_artistF_cohort_reach.csv`](census/06_artistF_cohort_reach.csv)

`tupleCohortId = (programCandidateId, layoutIdentityId, adapterCandidateId)`다. 도화가 F
17행은 12개 cohort로 갈리고, 그 12개 cohort의 **총원은 19행이며 전부 Artist**다.
비-도화가F는 단 2행이다.

구조적 원인은 다음이다.

```text
TYPED_EXECUTION_COHORT     19개 / 50 occurrence   layoutIdentity = "layout.*"
NATIVE_EVIDENCE_COHORT    321개 / 1,974 occurrence layoutIdentity = "layout-evidence.*"
```

typed lane과 native-evidence lane은 layout identity 접두사부터 다르므로 **정의상 같은
cohort에 들어갈 수 없다**(`build_effect_tuple_cohort_inventory.py:2571-2591`, cohort id는
`{programCandidateId, layoutIdentityId, adapterCandidateId}`의 sha256). 19개 typed cohort는
전부 단일 도메인이다. 따라서 "도화가 F에서 검증한 cohort를 Valtan·4캐릭터로 확장"은 현재
데이터에서 **0행**을 반환한다.

그리고 더 큰 사실이 있다. cohort에 들어간 occurrence는 2,024행뿐이고
**나머지 5,542행(73.2%)은 `tupleCohortId = null`이다.**

```text
tupleCohortId 보유    2,024   (typed 50 + native 1,974)
tupleCohortId null    5,542   그중 PRODUCT_JOIN_CLOSED 1,764
  DXBC_FAMILY_REPRESENTATIVE_ONLY  3,671
  NO_PROGRAM_EVIDENCE              1,055
  BOUNDED_SOURCE_PROFILE_ONLY        525
```

즉 **현재 출하 중인 Product의 상당수가 경쟁 lane에 있는 게 아니라 아무 cohort에도 없다.**
cohort 확장을 확장 전략의 중심에 두면 안 되는 이유가 이것이다.

promotion 경로도 없다. `build_effect_tuple_cohort_inventory.py:4169-4170`이
`representativeProgramPromotion: "FORBIDDEN"`, `namedAbiTypedPacketPromotion: "FORBIDDEN"`로
못박아 두었고, `Publish-Effects.ps1`은 cohort inventory를 읽지도 않는다.

실제로 확장되는 축은 Adapter다.

```text
도화가 F 검증  →  adapter(carrier × state)가 증명됨   →  같은 카테고리 7,435 occurrence가 그 adapter를 재사용
             →  programId/layoutId는 재사용되지 않음   →  occurrence마다 별도 폐쇄 필요
```

---

## 6. 아직 얻어야 하는 정보 (전수)

[`census/07_required_information_matrix.csv`](census/07_required_information_matrix.csv)

| 남은 occurrence | blocker | 얻어야 하는 정보 |
|---:|---|---|
| 7,457 | `COMPILED_DRAW_DISPATCH_UNPROVEN` | carrier별 compiled adapter + 실제 draw 증거 |
| 7,457 | `VERTEX_FACTORY_UNPROVEN` | carrier별 VF/vertex layout 확정 |
| 5,819 | `STAGE_INPUT_SEMANTICS_UNPROVEN` | VS→PS stage input 의미 |
| 5,603 | `RUNTIME_PACKET_NOT_MATERIALIZED` | typed layout packet |
| 5,603 | `SAMPLER_STATE_UNPROVEN` | lane별 sampler state |
| 5,603 | `SCALAR_VECTOR_PACKING_UNRESOLVED` | cb0 scalar/vector packed index |
| 5,565 | `TEXTURE_REGISTER_SAMPLER_TOPOLOGY_UNMATERIALIZED` | DXBC signature의 t#/s# |
| 5,012 | `PRODUCT_CONSUMER_ABSENT` | Product cue 미포함 (material이 아니라 composition 문제) |
| 4,392 | `SCENE_INPUTS_UNPROVEN` | scene depth/color 필요 여부 |
| 3,856 | `OUTPUT_TOPOLOGY_MRT_UNPROVEN` | RT0/RT1 MRT 의미 |
| 3,543 | `OCCURRENCE_STATIC_PERMUTATION_NOT_EXTRACTED` | occurrence별 static set → exact permutation |
| 3,153 | `CURRENT_PACKET_CAPACITY_EXCEEDED` | layout count cap 상향 |
| 2,539 | `WPO_VERTEX_PROGRAM_UNPROVEN` | WPO는 VS라서 PS DXBC로 안 열림 |
| 1,804 | `NAMED_ABI_BLOCKED` 외 2종 | parent family named ABI 폐쇄 |
| 758 | `DXBC_EXTRACTION_BLOCKED` | 미추출 DXBC (구 V2 대상) |
| 558 | `CHILD_PARENT_RESOLUTION_BLOCKED` | child→parent join BLOCKED 잔여 |

### 분모에서 빼야 하는 3,256행

`LEGACY_STANDALONE_SPRITE`(1,902)와 `STANDALONE_MESH`(1,354) 합계 **3,256행은 복원 대상이
아니다.** 전수 확인 결과 다음이다.

```text
sourceRendererShape = null          3,256 / 3,256
sourceTypeDataClasses = []          전 행
sourceProfileEnabled = false        3,216 / 3,256
elementId 형식                       authored.approx.s004.sprite02
                                     authored.approx.s012.mesh01
PRODUCT_JOIN_CLOSED                  3 행
```

**원본에서 유래한 occurrence가 아니라 프로젝트가 손으로 만든 V0 근사치**다. 복원할 원본이
없으므로 분모에 넣으면 목표가 3,256행 부풀려진다. §3의 Product 기준 표는 이미 이들을
제외하고 있다.

---

## 7. 이 문서가 admission하지 않는 것

- runtime 변경 없음. C++·HLSL 한 줄도 바꾸지 않았다.
- `runtimeVerified`는 전 cohort `false`다. compiled draw 증거가 아니다.
- Product 승격 없음. 육안 판정 없음. 사용자 서면 관찰 전에는 어떤 visual PASS도 기록하지 않는다.
- 카테고리 분류는 authored `renderProfile` 기준이다. authored renderProfile이 원본 blend와
  같다는 증거는 별도이며 `Effect_AuthoringDocument.h:1277`은 이를 *"compiler's bounded
  fallback when source evidence was"* 없을 때 쓰는 값이라고 명시한다. **즉 renderProfile
  자체가 bounded 증거일 수 있다.** additive/alpha 판정을 source로 재확인하는 것은 별도 lane이다.

---

## 8. 다음 단위 제안

§3의 Product 우선순위는 **전략 순서**다. 그러나 오늘 실제로 bind 가능한 것은
`program + layout + descriptor`가 모두 닫힌 **typed lane 50행뿐**이다. 두 층을 분리한다.

### 오늘 bind 가능한 typed lane 전수 (50행)

| carrier | renderProfile | typed | Product | 도메인 |
|---|---|---:|---:|---|
| MESH | alpha_one | **14** | **14** | **LanceMaster 12 / Artist 2** |
| SPRITE | additive_two | 12 | 12 | Artist 12 |
| MESH | alpha_two | 7 | 7 | Artist 7 |
| SPRITE | alpha_two | 6 | 6 | Artist 6 |
| SPRITE | alpha_one | 4 | 4 | Artist 2 / DimensionMaster 2 |
| MESH | additive_one | 2 | 0 | Warlord 2 |
| RIBBON / DECAL / MESH-opaque | — | 5 | 5 | Artist 5 |

**주의: Product #2인 SPRITE additive_one(501행)은 typed가 0행이다.** 물량은 가장 크지만
지금 착수할 수 있는 occurrence가 하나도 없다. 먼저 그 카테고리 안에서 program/layout 증거를
닫는 별도 작업이 필요하다.

### 순서

```
S0  registry를 mirror에서 authority로 뒤집는다        ← 반드시 먼저
    Effect_DocumentRenderer.cpp:15215-15235의 Is_ExecutionBitExact 동등 gate를
    registry 우선으로 바꾸고, draw state를 adapter desc에서 받게 한다.
    이게 없으면 아래 전부가 화면에 안 나온다. (§4-1)

S1  sprite.c65181324417a1a8 binding 추가                 C++ 무변경, 데이터만
    같은 cohort.29510c4e26f6261, structuralDescriptorReuseCandidate=true.
    descriptor 재사용 경로가 build/validate/publish/resolve를 통과하는지 증명.
    단 S0 전에는 픽셀 증명이 아니라 데이터 경로 증명이다.

S2  MESH carrier compiled adapter 신규 (alpha_one)
    → typed 14행, 그중 12행이 LanceMaster이고 전부 Product.
    → typed lane에서 유일하게 cross-domain이면서 가장 큰 버킷.
    → EFFECT_COMPILED_MATERIAL_CARRIER에 두 번째 carrier가 생긴다.
      Mesh/Decal이 언젠가 실행되기 위한 구조적 전제조건.
    → 필요한 것: mesh VF/vertex layout, modelPreScale 0.01, pass index

S3  SPRITE renderProfile 확장 (additive_two 12 typed → additive_one/alpha_one)
    → state 삼중항은 이미 코드에 있음. adapter enum/desc 추가 + opcode gate 해제.

S4  DECAL carrier compiled adapter
    → 필요한 것: LocalDecal projector VF, scene depth 입력
```

각 단계는 default-off canary → D/R 빌드 → `Run-EffectRenderContractHarness.ps1`(PR #171)
→ **사용자 SOLO 육안** → V0 A/B 순서를 지킨다. 에이전트는 화면을 판정하지 않는다.

### 레버리지가 아닌 것

- **DXBC 추가 추출.** 계산식은 이미 병목이 아니다. 병목은 dispatch closure다.
- **cohort 확장.** namespace 경계상 lane을 못 넘고, 73.2%는 cohort 자체가 없다.
