# 2026-08-15 locked 1,007 전수 원인 규명과 family 단위 admission 분석

이 문서는 4직업 101개 `.unified` 후보의 portable Particle fail-closed 1,007행을 element 목록이 아니라
**원인 축 / parent Material family 축**으로 다시 세운 조사 보고다. 실측 입력은 현재 디스크의 다음 산출물이며
추정은 사용하지 않았다.

> **상태 주의**: 이 문서의 1,007은 현재 디스크에 보존한 기존 101 baseline의 조사 분모다.
> Wave 0 evidence와 Codex admission/Lance 경계를 적용한 최신 no-write projection은 Full 2,793 /
> Authoring Approximate 722 / Hard 973이며 아직 101문서에 쓰지 않았다. 다음 실행 순서와 현재 수치는
> `2026-08-15_AUTHORING_PRODUCT_GATE_SPLIT_CODEX_HANDOFF.md`를 우선한다. 이 문서는 family histogram의
> 원본 조사 근거로 보존하며 새 projection 수치로 일괄 치환하지 않는다.

- `Data/Effects/AuthoredCorrections/Generated/FourClassCombat.track-a-restoration-receipt.json`
- `Data/Effects/AuthoredCorrections/Generated/FourClassCombat.source-material-contract.json`
- `Data/Effects/Authored/*.unified.effect.json` (102 문서 / 4,810 element)
- `Data/Effects/Imported/DimensionMaster/ActionSource/DimensionMaster.source-material-evidence.json`
- `C:/Users/user/Desktop/Resource_LostArk/05_Reports/EffectExtraction/DIMENSIONMASTER/materials/*.materials.json`
- `C:/Users/user/Desktop/Resource_LostArk/01_Extracted/**/Material3/*.props.txt`
- `Tools/LevelPlacementExtractor/build_effect_source_material_contract.py`
- `Client/Public/Effect_MaterialTemplate.h`, `Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli`

---

## 1. 결론 세 줄

1. locked 1,007은 "DDS를 몰라서"가 아니다. **1,007 중 884행(88%)은 parent Material의 선언 evidence
   (group/renderState/collected parameters)가 compiler 입력에 아예 없어서** typed family로 라우팅되지
   못한 것이고, 나머지 123행은 material identity 자체가 compile 대상에서 빠진 것이다.
2. 그 evidence는 **원리적으로 못 얻는 정보가 아니다.** 이미 DimensionMaster 경로에서 89개 parent에 대해
   `UModel Material3 props` 방식으로 확보돼 있고, 필요한 source UPK 1,813개는 전부 로컬에 있다.
   locked를 지배하는 상위 51개 parent만 닫으면 **688행(68%)** 이 한 축에서 풀린다.
3. 단 하나 **영구히 파싱으로 얻을 수 없는 것은 material의 arithmetic(expression 그래프)** 이다.
   cooked UPK에서 stripped됐고(Artist 31470 실측: null slot 1,803 / unresolved edge 502),
   ShaderCache는 존재하지만 미해독(state-key direct match 0/23)이다.
   따라서 family evaluator는 **사람이 재구성**해야 하며, 이 부분만은 "추출"이 아니라 "구현"이다.

---

## 2. 분모 재확정 — 1,007이 정확히 무엇인가

`FourClassCombat.track-a-restoration-receipt.json`의 `counts` 실측값이다.

```text
source Particle corpus                4,846
  - typed exclusion                     159   (현재 101 target 밖 / event join 없음)
= strict mapped Particle              4,687
  - source-preserved deferred           199   (Cascade module 미지원, recipe 자체 비활성)
= portable Particle                   4,488
  - portable admitted                 3,481
  - portable FAIL-CLOSED              1,007   ← 이 문서의 대상
```

Particle 밖의 잠금은 별도 분모다. 합치지 않는다.

| 계열 | 수 | 상태 |
|---|---:|---|
| portable Particle fail-closed | 1,007 | 이 문서 |
| source-preserved deferred (Cascade module) | 199 | §4.3 |
| Decal missing-Base | 33 | `AUTHORING_INCOMPLETE_LOCKED`, 사용자 Base 지정으로 해제 |
| AnimationTrail | 11 | materialize 완료, native Stage/Draw 대기 |
| `CHARACTER_AFTERIMAGE` cue | 72 / 29 문서 | receipt-only, ghost pose runtime 부재 |

직업별 locked 분포다.

| class | admitted | locked | locked 비율 |
|---|---:|---:|---:|
| LANCE_MASTER | 1,603 | 381 | 19.2% |
| WARLORD | 963 | 234 | 19.6% |
| DIMENSIONMASTER | 561 | 206 | 26.9% |
| ARTIST | 354 | 186 | 34.4% |
| 합계 | 3,481 | 1,007 | 22.4% |

renderer shape 기준은 sprite 808 / mesh 199다.

---

## 3. 원인 축 histogram

### 3.1 blocker 다중 라벨 (한 element가 여러 이유를 가질 수 있음)

```text
676  SOURCE_PROFILE_FALLBACK_BLOCKED
127  MISSING_EXECUTABLE_DRAWABLE_CONTRACT
123  SOURCE_PROFILE_NOT_COMPILED
 78  SOURCE_PROFILE_RECONSTRUCTED_STANDARD_SAFE_BASE
 77  MISSING_EXACT_MESH_MODEL
 30  SOURCE_PROFILE_GROUPED_RESOURCE_CONTRACT
 29  NON_EXACT_NAMED_TEXTURE_ALIAS
 20  SOURCE_PROFILE_LOCAL_CRACK_RESOURCE_CONTRACT
 11  SOURCE_MATERIAL_ENGINE_BUILTIN_POLICY_NOT_AUTHORED
  5  NON_EXACT_SOURCE_MATERIAL:AMBIGUOUS_MATERIAL_PATH
  3  SOURCE_PROFILE_FINITE_RESOURCE_CONTRACT
  1  SOURCE_PROFILE_BLACKLINE_RESOURCE_CONTRACT
```

blocker 개수 분포는 1개 867행, 2개 107행, 3개 33행이다.

### 3.2 단독 blocker — "이 하나만 닫으면 그 행이 열린다"

이 표가 admission 계획의 실제 우선순위다.

| 단독 blocker | 행 수 | 의미 |
|---|---:|---|
| `SOURCE_PROFILE_FALLBACK_BLOCKED` | **650** | parent family를 typed profile로 못 골랐다 |
| `SOURCE_PROFILE_RECONSTRUCTED_STANDARD_SAFE_BASE` | 78 | profile은 정했으나 안전한 Base lane이 없다 |
| `SOURCE_PROFILE_NOT_COMPILED` | 73 | material identity가 compile 대상에 없다 |
| `NON_EXACT_NAMED_TEXTURE_ALIAS` | 22 | named texture가 exact가 아니라 alias다 |
| `SOURCE_PROFILE_GROUPED_RESOURCE_CONTRACT` | 21 | grouped 실행 조건(base/mask/emissive) 미충족 |
| `SOURCE_PROFILE_LOCAL_CRACK_RESOURCE_CONTRACT` | 20 | local-crack의 normal/reflection/dissolve/mesh 4종 미충족 |
| `SOURCE_PROFILE_FINITE_RESOURCE_CONTRACT` | 2 | finite profile 필수 리소스 부족 |
| `SOURCE_PROFILE_BLACKLINE_RESOURCE_CONTRACT` | 1 | blackline mask+dissolve 미충족 |
| (복합 2~3개) | 140 | 대부분 mesh 모델 부재가 동반된다 |

복합 조합 상위는 다음과 같다.

```text
54  MISSING_EXACT_MESH_MODEL + MISSING_EXECUTABLE_DRAWABLE_CONTRACT
40  MISSING_EXECUTABLE_DRAWABLE_CONTRACT + SOURCE_PROFILE_NOT_COMPILED
14  MISSING_EXACT_MESH_MODEL + MISSING_EXECUTABLE_DRAWABLE_CONTRACT + SOURCE_PROFILE_FALLBACK_BLOCKED
10  MISSING_EXECUTABLE_DRAWABLE_CONTRACT + ENGINE_BUILTIN_POLICY + NOT_COMPILED
 9  MISSING_EXACT_MESH_MODEL + MISSING_EXECUTABLE_DRAWABLE_CONTRACT + GROUPED_RESOURCE_CONTRACT
```

### 3.3 결정 경로 추적 — 676 FALLBACK_BLOCKED가 왜 생기는가

`build_effect_source_material_contract.py`의 실제 코드 경로다.

```text
runtime_shader_profile_id(parent_path, source_path)
  └─ exact_profiles: 하드코딩 dict 12개 항목, parent 경로 문자열 exact/suffix 매치
     └─ 매치 → typed profile 확정
     └─ 미매치 → "" 반환
           ↓
grouped_translucent_selection(...)
  ├─ primary_names := parent의 collectedTextureParameters 중
  │                   source_group_slot(group) ∈ {mask, emission, base} 인 파라미터명
  ├─ primary_names 있고 해당 runtime asset 해결됨 → grouped-translucent.v1  (ADMIT)
  ├─ primary_names 있는데 asset 미해결      → MISSING_GROUPED_TRANSPARENT_RUNTIME_RESOURCE
  ├─ primary_names 없음 && emitter에 safe base/mask/emissive DDS 있음 → grouped-translucent.v1 (ADMIT)
  └─ 그 외                                  → UNKNOWN_GROUPED_TRANSPARENT_INPUT  (BLOCK)
```

즉 두 개의 게이트가 전부다.

- **게이트 1**: parent 경로 문자열이 12개 표에 있는가. (family 축)
- **게이트 2**: parent가 선언한 texture parameter의 **group 문자열**이 base/mask/emission으로 읽히는가,
  아니면 emitter 자체에 안전한 Base/Mask/Emissive DDS가 붙어 있는가. (carrier 축)

`FourClassCombat.source-material-contract.json`의 blocked identity 182개 중
**162개(= 4,777 문서에서 688 occurrence)의 사유가 `UNKNOWN_GROUPED_TRANSPARENT_INPUT`** 이다.
나머지는 `MISSING_CLASS_LOCAL_FINITE_PROFILE_RESOURCE` 16, `ENGINE_MATERIAL_POLICY_NOT_AUTHORED` 2,
`LOCAL_CRACK_NAMED_TEXTURE_OR_SAMPLING_CONTRACT_INCOMPLETE` 2다.

### 3.4 게이트 2가 실패하는 진짜 이유 — 실측

162개 blocked identity가 실제로 들고 있는 필드를 세면 다음과 같다.

```text
162/162  parentMaterialPath          있음
 93/162  scalars > 0                 있음  (MIC override)
 59/162  currentResourceBindings > 0 있음
 56/162  textures > 0                있음  (MIC override)
 39/162  vectors > 0                 있음  (MIC override)
  0/162  staticSwitches > 0          없음
  0/162  renderState                 없음
  0/162  collectedTextureParameters  없음   ← 게이트 2의 입력이 통째로 빈다
  0/162  referencedTextures          없음
```

더 중요한 것은 이것이 blocked identity만의 문제가 아니라는 점이다.
**aggregate contract의 769개 material identity 전부가** 다음 상태다.

```text
769/769  materialResourceDecodeStatus = "NOT_CAPTURED"
769/769  expressionCoverage.topologyStatus = "NOT_CAPTURED"
769/769  renderState = null
769/769  semanticStatus = "RECONSTRUCTED_PROFILE"
```

즉 4직업 aggregate compiler는 **parent Material 선언 evidence를 한 건도 입력받지 않은 상태로 돌았다.**
지금 admitted된 3,481행 중 2,948행이 `grouped-translucent.v1`인 이유가 여기 있다.
그 admission은 family 재현이 아니라 "emitter에 붙은 DDS를 generic carrier로 그린다"는 근사다.

> 이 사실은 admitted 3,481을 원본 재현으로 읽으면 안 된다는 뜻이기도 하다.
> 현재 typed family로 실제 라우팅된 것은 aura 142 + circle 106 + shine 51 + center-glow 46 +
> missiletrail 34 + ring 15 + dot 12 + linearflow 12 + distortion 8 + slice 4 + blackline 4 =
> **434행**뿐이고, 이것도 parent 경로 **이름 문자열**로 고른 것이지 evidence로 고른 것이 아니다.

---

## 4. family 축 histogram

### 4.1 locked 1,007의 parent Material family 분포

numeric variant를 병합한 family 기준이다. 상위 20개가 65%, 상위 40개가 82%를 덮는다.

```text
locked 1,007

  123  (parent 없음 / compile 안 됨)
   81  bfx_d_pa_shine_ad
   44  fx_m_pa_missiletrail_tr
   38  fx_d_pa_flare_ad
   37  bfx_i_pa_thunder_ad
   33  fx_a_pa_firework_ad
   31  fx_k_me_makeflow_tr
   31  fx_k_pa_veldust_tr
   28  fx_m_basemaster_tr
   28  fx_j_me_localcrack_tr
   23  fx_c_pa_smokeseq_tr
   22  fx_b_pa_cd_tr
   22  fx_b_pa_smoke_tr
   21  fx_f_pa_wind_tr
   17  fx_d_pa_ring_ad
   17  fx_c_pa_glitter_tr
   16  fx_d_pa_glow_ad
   15  fx_d_pa_shine_tr
   14  fx_j_pa_circledisort_ad
   14  bfx_d_pa_circ_tr
   13  fx_d_pa_master_ad
   13  fx_a_pa_lightning04_ad
   11  fx_m_pa_trail_tr
   11  bfx_i_pa_backglow_cl_tr
   10  bfx_i_pa_backglow_tr
    9  fx_o_pa_circledisort_ad
    9  fx_mm_basic_tr
    9  fx_j_me_cubesample_tr
    9  fx_m_me_splitline_tr
    9  fx_h_pa_tearsurface_tr
   ...  (총 112 family)
```

### 4.2 부분 개방 family — 같은 family가 열리기도 잠기기도 한다

이것이 현재 구조가 취약한 결정적 증거다. family가 아니라 **정확한 경로 문자열**로 고르기 때문이다.

| family | admitted | locked |
|---|---:|---:|
| `bfx_d_pa_shine_ad` | 65 | 81 |
| `fx_m_pa_missiletrail_tr` | 35 | 44 |
| `fx_k_me_makeflow_tr` | 188 | 31 |
| `fx_d_pa_master_ad` | 112 | 13 |
| `fx_d_pa_ring_ad` | 35 | 17 |
| `fx_mm_basic_tr` | 133 | 9 |
| `fx_m_pa_trail_tr` | 58 | 11 |
| `bfx_d_pa_circ_ad` | 106 | 5 |
| `fx_d_pa_flare_ad` | 4 | 38 |
| `fx_m_basemaster_tr` | 5 | 28 |

48개 family가 양쪽에 동시에 나타난다. 예를 들어 표에 있는 `bfx_m.bfx_d_pa_circ_01_ad`는
`circle.v1`로 열리지만 같은 family의 `bfx_m.bfx_d_pa_circ_01_tr`(translucent variant)는 표에 없어 잠긴다.
`fx_m.fx_d_pa_ring_11_tr`는 표에 있고 `fx_m_mi_03.fx_m.fx_d_pa_ring_12_ad`는 없다.

### 4.3 source-preserved deferred 199 — 별도 축 (Cascade module)

이건 material 문제가 아니라 **파티클 모듈 실행기** 문제다.

```text
199 deferred

  81  particlemoduletypedataribbon        (Cascade Ribbon — 이번 범위에서 계속 보류)
  47  efparticlemodulelocationemitter     (LOSTARK 커스텀 모듈)
  21  particlemodulelocationemitter
  19  particlemoduleorbit                 (chain/option 조합 미지원)
  10  particlemodulecollision
   5  particlemodulemeshmaterial
   4  particlemoduleeventreceiverspawn
   3  particlemodulesizemultiplyvelocity
   3  local vector field asset 없음/불안전
   2  particlemoduleeventgenerator
   2  particlemodulesubuv (Family/cardinality)
   2  particlemodulesubuvmovie
```

Ribbon 81을 제외하면 **118행이 emitter-location / orbit / collision 계열 4~5개 모듈 구현으로 열린다.**

---

## 5. Track A는 실제로 무엇을 뽑았고 무엇을 못 뽑았는가

이 절이 "이미 다 파싱한 거 아니야?"에 대한 실측 답이다. 결론은 **레이어마다 다르다**.

### 5.1 확보된 것

| 데이터 | 상태 | 근거 |
|---|---|---|
| MIC(child) scalar/vector/texture override | **전량 확보** | `.materials.json` 24,633 rows (material 2,752 + MIC 21,880), MIC 21,876개가 parent 링크 보유 |
| MIC → parent Material 경로 | **전량 확보** | 위 동일 |
| element별 exact DDS 바인딩 | **확보** | receipt: exact named texture 6,911 / rebase 1,967 / manifest 104 / unresolved 1,469 |
| Cascade module recipe (burst/lifetime/velocity/color/size/subUV) | **확보** | 4,488 portable, `sourceRecipe.modules` |
| DynamicParameter semantics | **확보** | 4,563/4,563 element에 populated (Cascade 측 정보) |
| subUV mode | **확보** | linear_blend 276 / random_flip_square 262 / none 4,025 |
| parent Material 선언 evidence (group/default/renderState) | **89개 parent만** | `DimensionMaster.source-material-evidence.json`, captureMethod = `UModel parent Material3 props` |
| parent Material 선언 evidence (Artist 31470 전용 심층) | **23 family** | BlendMode 23/23, LightingModel 21/23, TwoSided 9/23, scalar 342, vector 19, texture 71, parent default 297, static switch 94 |

확보된 89개 parent evidence의 품질은 다음과 같다.

```text
89/89  renderState.blendMode          (BLEND_Translucent / BLEND_Additive)
89/89  renderState.twoSided
89/89  renderState.disableDepthTest
89/89  referencedTextures > 0
87/89  collectedScalarParameters > 0
83/89  collectedTextureParameters > 0  ← group 문자열 포함
57/89  collectedVectorParameters > 0
 0/89  collectedStaticSwitchParameters ← 미확보
 0/89  renderState.lightingModel       ← 미확보
 0/89  renderState.usesDistortion      ← 미확보
```

그리고 그 group 어휘가 게이트 2의 실제 열쇠다.

```text
32  uv_noise            -> noise
31  01_alpha            -> mask
22  emissive            -> emission
20  None                -> (라우팅 실패)
18  08_uvdistort        -> noise
16  02_emission         -> emission
15  dissolve            -> dissolve
12  maintex             -> (라우팅 실패)   ← 사실상 base인데 못 읽음
10  05_specullar        -> (라우팅 실패)
 9  diff                -> (라우팅 실패)   ← 사실상 diffuse인데 못 읽음
 9  alpha               -> mask
 8  cracknormal         -> (라우팅 실패)
 8  09_mask             -> mask
 7  opacity             -> mask
 6  noise               -> noise
 5  02_diffuse          -> base
 4  flow                -> (라우팅 실패)
 4  01_diffuse          -> base
 ...
```

`source_group_slot()`은 `alpha/opacity/mask/dissolve/uvdistort/uv_noise/noise/emission/emissive/diffuse/base/color`
substring만 안다. **`maintex`, `diff`, `flow`, `in`, `01_map`, `01_main` 같은 실재 group을 놓친다.**
즉 evidence가 있는 89개 parent에서도 라우팅 손실이 발생한다.

### 5.2 확보되지 않은 것 — 재추출로 얻을 수 있는 것

| 항목 | 규모 | 재추출 가능성 |
|---|---:|---|
| locked를 지배하는 parent Material3 props | **101 parent → 650 element** | **가능**. 필요한 source UPK가 `00_SourcePackages`에 1,813개 전량 존재하고, `parentSourcePhysicalPackage`가 blocked identity 182개 중 85개에 이미 기록돼 있다 |
| 전체 corpus 기준 parent props | 305 parent 중 285개 미확보 | 가능 (동일 경로) |
| `CollectedStaticSwitchParameters` | 4,562/4,563 element에서 빈 배열 | 가능. Artist 31470 심층 추출에서는 94행 확보에 성공했다 |
| `LightingModel` / `bUseOneLayerDistortion` / `OpacityMaskClipValue` | DM props run 0/89 | 가능. Artist 31470 raw UPK 경로에서는 확보됐다 |
| sampler descriptor (AddressU/V, sRGB, Filter) | Artist 31470 기준 exact 0/72 | 부분적. 다수 필드가 export에서 omitted라 class default 승격 금지 상태 |
| MIC `FStaticParameterSet` selected values | Artist 31470 94행 중 exact 23 / 미증명 43 / entry 없음 28 | 부분적 |

파일 형식 차이도 명확하다.

```text
.materials.json (schema v2, "reconstructed")   →  parent + textures{name,texture} + scalars + vectors
                                                   group 없음, renderState 없음, staticSwitch 없음, 그래프 없음

Material3/*.props.txt (UModel)                 →  + collectedTexture/Scalar/Vector/StaticSwitchParameters
                                                   + group 문자열
                                                   + renderState(BlendMode/TwoSided/DisableDepthTest)
                                                   + referencedTextures
```

현재 4직업 aggregate compiler는 **앞쪽(.materials.json)만 입력**받는다. 그래서 769/769 `NOT_CAPTURED`가 나온다.

### 5.3 확보 불가 — 영구 결손

**material의 arithmetic(expression 그래프)은 cooked 배포본에서 복원할 수 없다.**
Artist 31470 심층 조사의 실측이 근거다.

```text
raw graph expression export            925
cooked-stripped null expression slot   1,803
unresolved graph edge                    502
surviving resolved graph edge            125
23/23 family graphProvenance         = RECONSTRUCTED_GRAPH  (sourceExactGraph = false)
implemented evaluator                =   0
independent numeric oracle           =   0
```

우회로였던 ShaderCache도 닫혀 있다.

```text
installed ShaderCache export              1,596  (전부 class `shadercache`)
선택 context candidate                       11
23 state key → 1,596 serial exact 16-byte 매치   0 / 23
상태  SHADERCACHE_PRESENT_DECODER_PENDING
blocker MATERIAL_SHADER_MAP_KEY_UNRESOLVED
```

DimensionMaster props 경로의 `expressionCoverage`가 `nullCount = 0`으로 보이는 것은 그래프가 온전하다는
증거가 아니다. UModel props 출력이 null slot을 아예 인쇄하지 않아 89/89 전부 `entryCount == nonNullCount`가
되는 **capture artifact**다. 두 경로를 교차하면 "노드 이름과 개수는 보이지만 연결과 연산은 없다"가 실상이다.

> 따라서 사용자가 정리한 모델이 정확하다. 병목은 raw evidence 부족이 아니라
> `UE3 material semantics → Winters typed profile` 번역 계층이며, 그 번역기의 **산술 부분은
> 추출 결과가 아니라 재구성 결과**일 수밖에 없다. 다만 그 재구성의 입력(파라미터/그룹/렌더 상태)은
> 지금 상당수가 비어 있고, 그건 재추출로 채울 수 있다.

---

## 6. 현재 런타임이 가진 실행 family

`Client/Public/Effect_MaterialTemplate.h`의 허용 profile은 15개다.

```text
effect.ue3.grouped-translucent.v1     (범용 carrier)
effect.ue3.reconstructed-standard.v1  (범용 standard)
effect.ue3.fallback-blocked.v1        (차단 표식)
effect.ue3.circle.v1        effect.ue3.dot.v1          effect.ue3.ring.v1
effect.ue3.aura.v1          effect.ue3.shine.v1        effect.ue3.slice.v1
effect.ue3.blackline-aura.v1           effect.ue3.linearflow-02.v1
effect.ue3.one-layer-distortion.v1     effect.ue3.missiletrail-01.v1
effect.ue3.local-crack.v1              effect.ue3.procedural-center-glow.v1
```

`Shader_EffectCommon.hlsli`(956줄)는 family별 전용 uniform과 분기를 가진 typed 셰이더다
(`g_LinearFlowParameters[16]`, `g_LocalCrackParameters[5]`, `g_LocalCrackOutColor/InColor/ReflectionColor` 등).
범용 material VM이 아니다. 따라서 새 family 추가는 다음 세 곳을 함께 닫아야 한다.

1. `runtime_shader_profile_id()` 매핑 (현재 12개 exact 경로)
2. `Effect_MaterialTemplate.h`의 profile 허용 목록 + 리소스 계약 함수
3. `Shader_EffectCommon.hlsli`의 evaluator 분기

`Is_EffectFiniteProfileResourceContractSatisfied()`의 현재 계약도 기록해 둔다.

```text
shine.v1                  : base && mask
blackline-aura.v1         : mask && dissolve
slice.v1                  : base
missiletrail-01.v1        : base && noise && mask && dissolve && mesh
local-crack.v1            : 항상 false  ← 명시적으로 잠겨 있음 (locked 20행의 원인)
procedural-center-glow.v1 : 항상 true
```

---

## 7. admission 계획 — locked 1,007을 어떤 순서로 떨어뜨리는가

각 wave는 독립적으로 검증 가능하고, 앞 wave가 실패해도 뒤 wave가 막히지 않도록 구성했다.

### Wave 0 — 번역기 입력을 만든다 (재추출)

**대상**: locked 884행(parent 있음) 전체의 근본 입력.

1. `Tools/LevelPlacementExtractor/extract_umodel_material_dependencies.py` 경로로 4직업 전체
   parent Material3 props를 뽑는다. 대상 package는 `parentSourcePhysicalPackage`가 기록된 85개
   identity에서 직접 얻고, 나머지는 `.materials.json`의 `logical_package → UPK` 역인덱스로 해결한다.
   미확보 parent 101개가 몰린 package는 `fx_m` 65 / `bfx_m_mi_00` 6 / `bfx_m` 7 / `fx_m_mi_j_00` 6 /
   `fx_m_mi_03` 12 / `fx_m_mi_00` 4 / `fx_m_mi_02` 6 / `fx_mastermaterial` 6이다.
2. `build_dimensionmaster_source_material_evidence.py`의 `characterClass == "DIMENSIONMASTER"`
   하드 게이트(141행)를 제거하고 4직업 공용 builder로 일반화한다. 출력은 class별
   `<Class>.source-material-evidence.json`으로 나눠 evidence provenance를 섞지 않는다.
3. `build_four_class_source_material_contract.py`가 `.materials.json`뿐 아니라 이 evidence를
   입력으로 받게 한다. 성공 판정은 `materialResourceDecodeStatus != NOT_CAPTURED`,
   `renderState != null`의 identity 수가 0에서 증가하는 것이다.

**예상 효과**: 자체로 admit을 늘리지 않는다. Wave 1~3의 전제조건이다.
**검증**: `--check` 재생성 일치, evidence SHA pin, 기존 3,481 admitted 무회귀.

### Wave 1 — group 라우팅 어휘 확장 (가장 싼 이득)

**대상**: 게이트 2의 substring 표.

`source_group_slot()`에 실측 group 어휘를 추가한다. 단 **이름 추측이 아니라 실제 관측된 group만**
넣고, 각 항목에 관측 근거(어느 parent props에서 몇 회)를 남긴다.

```text
base    += maintex, diff, main, map   (관측 12 + 9 + 3 + 2)
noise   += flow, tex_flow, distortiontexture
mask    += (이미 alpha/opacity/mask 커버)
```

`05_specullar`, `cracknormal`, `13_rainbow`, `05_explode` 등은 base/mask/emission 어느 쪽도 아니므로
**추가하지 않는다.** 이것들을 억지로 매핑하면 §2의 fail-closed 계약이 깨진다.

**예상 효과**: 이미 evidence가 있는 43 parent / 234 locked element에서 우선 회수. Wave 0 이후엔
전체 884행에 같은 규칙이 적용된다.
**검증**: 어휘 추가 전후 admitted delta를 group별로 분해해 receipt에 기록. 새로 열린 행이
`grouped-translucent`로만 가는지, typed family로 가는지 구분.

### Wave 2 — family compiler를 exact 경로 표에서 family 규칙으로 승격

**대상**: `SOURCE_PROFILE_FALLBACK_BLOCKED` 단독 650행.

현재 12개 exact 문자열 표를 다음으로 바꾼다.

```text
(family stem, blend suffix, renderState.blendMode, 필수 group 집합) -> profile
```

`fx_d_pa_ring_11_tr`만이 아니라 `fx_d_pa_ring_*_{ad,tr}` 전체를 한 규칙으로 받되,
**Wave 0에서 얻은 renderState.blendMode와 collected parameter 집합이 그 family의 기준 서명과
일치할 때만** 승격한다. 이름만 같고 파라미터 서명이 다르면 계속 잠근다.

우선순위는 locked 가중치 순이다. 상위 10개 family만 닫아도 약 372행이다.

| 순위 | family | locked | 비고 |
|---:|---|---:|---|
| 1 | `bfx_d_pa_shine_ad` | 81 | 이미 65행이 열려 있는 부분개방 family |
| 2 | `fx_m_pa_missiletrail_tr` | 44 | `missiletrail-01.v1` 존재, 리소스 계약 5종이 과도 |
| 3 | `fx_d_pa_flare_ad` | 38 | 신규 evaluator |
| 4 | `bfx_i_pa_thunder_ad` | 37 | Warlord 전용 37/37 |
| 5 | `fx_a_pa_firework_ad` | 33 | Lance 28 + Warlord 5 |
| 6 | `fx_k_me_makeflow_tr` | 31 | 188행 이미 admitted |
| 7 | `fx_k_pa_veldust_tr` | 31 | Lance 전용 31/31 |
| 8 | `fx_m_basemaster_tr` | 28 | master material |
| 9 | `fx_j_me_localcrack_tr` | 28 | Wave 4에서 함께 처리 |
| 10 | `fx_c_pa_smokeseq_tr` | 23 | subUV 시퀀스 |

**예상 효과**: 상위 10 family 372행, 상위 20 family 약 532행.
**검증**: family별로 admitted delta와 `renderProfile`(blend/depth/cull) 값을 receipt에 남기고,
사용자 육안 검증 ledger에 family 단위 row를 추가한다.

### Wave 3 — carrier 계약 완화가 아니라 정확화

**대상**: `RECONSTRUCTED_STANDARD_SAFE_BASE` 78 + `GROUPED_RESOURCE_CONTRACT` 21 = 99행.

이 행들은 profile이 정해졌는데 base/mask/emissive lane이 비어서 막힌 것이다.
Wave 0의 `referencedTextures`와 parent default texture가 들어오면 **parent default를 lane으로 채울 수
있는지**가 판정 가능해진다. 지금은 그 판정 자체가 불가능해서 일괄 차단 중이다.

`blankwhite` base 금지와 normal/bump 금지 규칙은 유지한다. 이건 완화 대상이 아니다.

### Wave 4 — local-crack 잠금 해제

**대상**: `LOCAL_CRACK_RESOURCE_CONTRACT` 20행 (+ family 28행).

`Is_EffectFiniteProfileResourceContractSatisfied()`가 `local-crack.v1`에 대해 **무조건 false**를 반환한다.
셰이더(`g_LocalCrackParameters[5]`, out/in/reflection color)는 이미 구현돼 있으므로,
`Is_EffectLocalCrackResourceContractSatisfied(normal, reflection, dissolve, mesh)` 네 lane이
Wave 0 evidence로 채워지는지 확인한 뒤 게이트를 정상 계약으로 교체한다.

### Wave 5 — mesh drawable

**대상**: `MISSING_EXACT_MESH_MODEL` 77행 (locked mesh 199행 중 `meshResourceValid=false` 77).

material이 아니라 **`.wmodel` 자산 부재**다. 별도 mesh 추출/변환 트랙이며 material compiler로 열리지 않는다.
locked mesh 199행 중 122행은 mesh는 유효하고 material만 막힌 것이므로 Wave 1~4에 포함된다.

### Wave 6 — compile 누락 identity

**대상**: `SOURCE_PROFILE_NOT_COMPILED` 123행.

세부 분해는 다음과 같다.

```text
 87  materialResolutionStatus = RESOLVED_EXACT_SOURCE_PACKAGE  ← 패키지는 찾았는데 compile 대상에서 누락
 27  UNRESOLVED_MATERIAL_PATH
  8  RESOLVED_UNIQUE_PATH
  1  RESOLVED_IDENTICAL_COPIES
그중 11행은 enginematerials.defaultparticle (engine builtin, 정책상 계속 차단)
class 분포: LANCE_MASTER 91 / ARTIST 16 / WARLORD 16
```

87행은 seed/compile 대상 선정 버그에 가깝다. `blockedMissingPhysicalPackageIdentityCount = 9`와
교차 확인하고, 누락 원인을 receipt에 코드로 남긴 뒤 compile 대상에 편입한다.
`enginematerials.*` 11행은 계속 fail-closed로 둔다.

### Wave 7 — Cascade module (별도 축, deferred 199)

Ribbon 81을 제외한 118행은 `efparticlemodulelocationemitter` 47 +
`particlemodulelocationemitter` 21 + `particlemoduleorbit` 19 + `particlemodulecollision` 10 +
`particlemodulemeshmaterial` 5 + 기타 16이다. emitter-location 계열 2개만 구현해도 68행이다.

### 누적 예상

| 단계 | 처리 대상 | 잔여 locked (예상) |
|---|---|---:|
| 현재 | — | 1,007 |
| Wave 1 (group 어휘) | 이미 evidence 있는 234행 중 라우팅 회복분 | ~900 |
| Wave 2 상위 10 family | 372 | ~530 |
| Wave 2 상위 20 family | +160 | ~370 |
| Wave 3 (carrier 정확화) | 99 | ~280 |
| Wave 4 (local-crack) | 20 | ~260 |
| Wave 6 (compile 누락) | 87 | ~175 |
| Wave 5 (mesh 자산) | 77 | ~100 |

잔여 약 100행은 `enginematerials` 11, ambiguous material path 5, non-exact alias 잔여,
그리고 exact evidence가 끝내 없는 행이다. 이건 잠근 상태가 정답이다.

---

## 8. 이 계획이 지켜야 할 경계

- generic fallback으로 보이게 만드는 admission 완화는 하지 않는다. Wave 1의 group 어휘 추가도
  **실제 관측된 group 문자열만** 대상으로 하고 각 항목에 관측 근거를 남긴다.
- DimensionMaster canonical profile 784개를 다른 캐릭터에 추정 복사하지 않는다. 각 class는
  자기 source receipt의 parent/texture/scalar/vector/dynamic evidence만 쓴다.
- family evaluator는 재구성물이다. `graphProvenance = RECONSTRUCTED_GRAPH`,
  `sourceExactGraph = false`를 계속 유지하고 exact로 승격하지 않는다.
- admitted 수치는 draw 증거가 아니다. `Stage_Document` 성공과 `Query_ParticleRuntimeProbe` 존재는
  픽셀 제출을 증명하지 않는다(§10.2 기록 유지).
- 최종 visual PASS는 사용자만 판정한다. 에이전트는 build, 구조화된 로그, 수치 진단, 실행 준비까지다.

## 9. 이 문서가 실행한 검증

읽기 전용 실측만 수행했다. 코드/데이터 변경은 없다.

- 102개 `.unified` 문서 파싱: 4,810 element (particle 4,716 / decal 82 / trail 12)
- restoration receipt의 `particleRows` 4,687행 전수 집계
- aggregate source-material-contract 769 identity 전수 집계
- `.materials.json` 30파일 / 24,633 material row 필드 스키마 확인
- `DimensionMaster.source-material-evidence.json` 89 parent evidence 품질 집계
- `01_Extracted/**/Material3/*.props.txt` 157파일 / 34 object 인덱싱
- `00_SourcePackages` 1,813 UPK 존재 확인 및 상위 parent package 6종 개별 확인
- `Effect_MaterialTemplate.h` profile 목록과 리소스 계약 함수, `Shader_EffectCommon.hlsli` family 분기 확인

미실행: publisher 재실행, compiler 수정, 빌드, Client 실행, 육안 검증.
