# 2026-08-22 창술사 D·F V1 family 확장 결과

branch: `codex/lance-df-v1-restore`

선행 결과: [`창술사 D·F Product Effect 복원 결과`](2026-08-22_LANCEMASTER_D_F_PRODUCT_EFFECT_RESTORATION_RESULT.md)

정본 계약: [`Effect Family Runtime ABI 복원 가이드 6a`](../../TEAM/EFFECT_FAMILY_RUNTIME_ABI_RESTORATION_GUIDE.md),
[`V1 마스터 계획 0.5 / 0.6`](2026-08-22_FOUR_CHARACTER_VALTAN_EFFECT_V1_FULL_MIGRATION_MASTER_PLAN.md)

## 1. 이번 변경의 판정 기준

사용자가 확정한 이름만 사용한다.

```text
V1_COMPLETE
= 올바른 carrier + family별 RT0 Base HLSL + texture/channel/scalar/DynamicParameter 배선
+ blend/depth + attachment/timing + Effect Tool 편집·저장 + 사용자 육안 승인

NATIVE_PARITY   (별도 backlog, 완료 조건 아님)
= native VF/BasePass, 원본 ShaderMap permutation, exact child cooked variant,
  MRT 0/2/3/4/5, Distortion 외 scene feedback, hardware sampler 전수 parity
```

이 문서가 닫는 범위는 `V1_COMPLETE`의 앞 다섯 항목이다. `Effect Tool 편집·저장`은 기존 저작
경로를 그대로 쓰고, `사용자 육안 승인`은 `USER_REVIEW_PENDING`으로 남는다.

## 2. 무엇이 문제였나

D·F의 242개 grouped 행을 원본 parent material로 분해하면 23개 family가 나온다. grouped translucent
경로는 이 23개를 하나의 공용 UV pan, 공용 gray coverage 추정, 공용 additive/alpha로 축약한다.
그래서 다음이 구조적으로 표현되지 않았다.

- `fx_mm_basic_01`의 전용 `alpha_tex`가 coverage가 아니라 두 번째 색 소스로 취급된다.
- 같은 family의 독립된 두 `uv_noise` 도메인(tiling·panning·intensity가 각각 다름)이 하나로 합쳐진다.
- `fx_k_me_flowtrail_01_ts_tr`가 diff v-tile `0.2`, opacity v-tile `1.0`로 저작돼 있는데 공용 UV
  scale 하나로 무너진다. `diff_str 4~8`, `opacity_str 2~7`도 사라진다.
- `fx_m.` 패키지로 해석된 lensflare 자식이 이미 구현된 typed family와 같은 parent인데도 grouped에 남는다.

## 3. 구현한 것

### 3.1 evidence 정본

RT0 Base 재구성의 1차 증거는 `Data/Effects/Contracts/effect-family-manifest.v1.json`의 parent
material evidence다. family별 `blendMode`, `twoSided`, `isMasked`, texture parameter의 이름·group·기본
texture, scalar parameter의 이름·group·기본값을 소유한다. 이 corpus에는 해당 family의 DXBC가 없으므로
fidelity는 `BOUNDED_TRANSLATED`이며 `SOURCE_EXACT`로 기록하지 않는다.

### 3.2 family 확장 3건

| family | 방식 | opcode | D·F 행 | corpus 행 |
|---|---|---|---:|---:|
| `ARTIST_LENSFLARE01` | 기존 typed family에 두 번째 package 경로 alias | 28 (재사용) | 34 | 105 |
| `MM_BASIC01` | 신규 typed family (`fx_mm_basic_01_ad` + `_tr`) | 39 | 26 | 186 |
| `FLOWTRAIL01` | 신규 typed family (`fx_k_me_flowtrail_01_ts_tr`) | 40 | 48 | 53 |

lensflare alias의 근거는 이름이 비슷하다는 것이 아니라 parent evidence가 동일하다는 실측이다.

```text
fx_m_mi_00.fx_m.fx_c_pa_lensflare_01_ad   vs   fx_m.fx_c_pa_lensflare_01_ad
objectName            fx_c_pa_lensflare_01_ad       동일
expressionSlots       46                            동일
staticSwitchCount     0                             동일
referencedTextureCount 1                            동일
textureParameters     lensflaretexture -> fx_c_glow_006   동일
scalarParameters      select 0.0 / desaturation 0.0 / depthbaisalpha -50.0   동일
```

차이는 package 해석 경로와 render profile(one-sided 전용 대 one/two-sided 혼합)뿐이다. sidedness는
raster state이지 다른 식이 아니므로 opcode 28의 carrier 계약이 이미 둘 다 허용한다.

### 3.3 새 HLSL 두 개

profile 39와 40을 `Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli`의 기존 typed source-profile
chain에 추가했다. 두 식 모두 element의 authored 문서를 바꾸지 않고 `sourceProfile` 증거에서
runtime이 직접 유도한다.

`39 fx_mm_basic_01_ad / _tr`

- lane 0 `emissive_tex` 필수, lane 1 `alpha_tex`, lane 2 `uv_noise_01_tex`, lane 3 `uv_noise_02_tex` 선택
- 두 noise 도메인이 각자 tiling·panning·intensity를 가지고 emissive UV를 offset한다
- `alpha_tex`가 있으면 그 lane이 coverage 정본이고, 없으면 simple_01과 같은 상황(BC1 alpha 1.0)이므로
  luminance와 radial envelope으로 카드 경계를 제거한다
- `emissive_power`, `emissive_desaturation`, `uv_scale`, `distortion_intensity`만 배선한다

`40 fx_k_me_flowtrail_01_ts_tr`

- lane 0 `diff_tex`, lane 1 `opacity_tex` 필수, lane 2 `noise_tex` 선택
- diff와 opacity가 각자 tile·center·rotation을 가진 별도 UV 도메인이고 noise가 둘 다 offset한다
- radiance는 `pow(diff, diff_pow) * diff_str`, coverage는 `opacity.r * opacity_str`

### 3.4 구현하지 않고 backlog로 남긴 것

증거가 닫히지 않는 scalar는 구현하지 않았다. 추측으로 채우면 coverage와 형태가 근거 없이 바뀐다.

| family | 미구현 scalar | 이유 |
|---|---|---|
| `MM_BASIC01` | `fresnel_power`, `edge_power`, `edge_intensity`, `depth_alpha_bias`, `camera_distance`, `world_normal_intensity` | scene depth와 world normal 입력이 RT0 base pass에 없다 |
| `MM_BASIC01` | `uv_rotation_speed`, `uv_rotation_angle`/`uv_rotation_algle` | 29개 static switch 상태가 추출에 없어 회전 gate를 알 수 없다. 무조건 적용하면 전 occurrence가 회전한다 |
| `FLOWTRAIL01` | `wave_str`, `wave_tile`, `wave_pan_speed`, `wave_noise_str` | corpus의 어떤 child도 `wave_tile`·`wave_pan_speed`를 override하지 않고 parent expression graph가 증거에 없다 |
| `FLOWTRAIL01` | `cameravec_pow` | camera vector 입력이 없다 |

## 4. 결과 수치

### 4.1 D·F의 typed 전환

| skill | grouped 행 | 변경 전 typed-known | 변경 후 typed-known | 비율 |
|---|---:|---:|---:|---:|
| D 34110 반월섬 | 74 | 26 | 47 | 64% |
| F 34150 맹룡열파 | 168 | 27 | 102 | 61% |
| 합계 | 242 | 53 | 149 | 62% |

변경 후 D의 typed 구성은 `SIMPLE01 15 / MM_BASIC01 14 / ARTIST_LENSFLARE01 7 / WATERTRAIL 6 /
MISSILETRAIL 5`, F는 `FLOWTRAIL01 48 / ARTIST_LENSFLARE01 27 / MM_BASIC01 12 / SIMPLE01 9 /
ARTIST_LIGHTFLARE01 3 / ARTIST_MM_DISSOLVE01 3`이다.

### 4.2 carrier는 이미 원본 값이다

이번 확장은 carrier를 바꾸지 않는다. D·F의 carrier는 conversion이 원본 emitter에서 그대로 가져왔다.

| skill | mesh particle | sprite particle | decal | 근거 |
|---|---:|---:|---:|---|
| D 34110 | 19 | 64 | 5 | `sourceRecipe.rendererShape` |
| F 34150 | 75 | 111 | 0 | 동일 |

`FLOWTRAIL01` 48행은 전부 mesh particle + `alpha_two_sided_depth_read`이고 parent evidence의
`BLEND_Translucent / twoSided true`와 일치한다.

### 4.3 남은 grouped 행

| parent material | D | F | 다음 조치 |
|---|---:|---:|---|
| `fx_m_mi_m_00.fx_mi.fx_m_pa_trail_01_4_tr` | 5 | 24 | parent evidence에 texture/scalar parameter가 0개다. source intake부터 필요 |
| `fx_m.fx_k_me_makeflow_02_tr` | 0 | 17 | 18행 중 exact child 1행은 carrier cohort가 inherited `color_tex`까지 연결해 profile36으로 승격했다. 남은 17개 4-lane 자식은 alias만으로는 fail-closed |
| `fx_mastermaterial.fx_mm.fx_mm_distortion_01_ad` | 0 | 9 | evidence 있음. 다음 family 후보 |
| `fx_m_mi_02.fx_m.fx_j_me_flamesurface_01_ma` | 0 | 9 | `BLEND_Masked` — masked carrier 경계를 먼저 확인해야 한다 |
| `fx_mastermaterial.fx_mm.fx_mm_simple_03_tr` | 8 | 0 | evidence 있음. `SIMPLE01`과 lane 집합이 같아 확장 후보 |
| 기타 | 14 | 6 | occurrence 3개 이하 소규모 |

## 5. 자동 검증

| 검증 | 결과 |
|---|---|
| `Tools/EffectPipeline/audit_typed_source_profile_join.py` | classifier pair 38개, dead identity join 0개 |
| `Tools/EffectPipeline/test_audit_typed_source_profile_join.py` | 4 tests OK |
| `Publish-Effects.ps1 -Mode Validate` | (아래 6절) |
| `Invoke-BuildAndRegression.ps1 -Configuration Debug` | (아래 6절) |

`audit_typed_source_profile_join.py`는 이번에 추가한 실행형 계약이다. typed family는
`(profileId, parentMaterialPath)` exact 문자열 쌍으로 join하므로 literal 한 글자가 틀리면 family
전체가 조용히 grouped로 돌아가고 build error도 runtime error도 나지 않는다. 이 audit은 classifier의
모든 쌍이 authored corpus에서 최소 1개 element와 실제로 join되는지 확인한다.

## 6. 수동 검증 (사용자 전용)

에이전트는 Client를 실행·조작하거나 화면을 캡처하지 않았다.

```text
Lobby -> Character Select -> 창술사(Lance Master), 긴 창 스탠스
D 반월섬 : 0ms cast, 96ms spark, 490ms 지면 decal 2종, 676ms mesh trail 4종, 710ms 충격 spark
F 맹룡열파 : 180~576ms EarthQS trail(FLOWTRAIL01 48행), 776ms Ark, 1405ms DragonSwing
```

확인해야 할 것은 다음이다.

- FLOWTRAIL01 궤적이 사각 카드 없이 흐르는지, diff와 opacity가 서로 다른 속도로 흐르는지
- MM_BASIC01 행의 경계가 카드가 아니라 artwork 모양인지
- lensflare 27행이 과도하게 밝지 않은지
- 이전보다 나빠진 occurrence가 있는지

`manual first pixel`, `eye smoke`, `visual PASS`는 사용자의 서면 판정 전까지 기록하지 않는다.
