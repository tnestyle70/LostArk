# 2026-08-07 차원술사 Q/W/T/S 시각 복원 상세 코드 계획서

## 1. 변경 파일

| 파일 | 계약 |
|---|---|
| `Tools/LevelPlacementExtractor/build_imported_effect_documents.py` | Screen Post source 값과 runtime 실행 값을 분리 |
| `Tools/LevelPlacementExtractor/test_build_imported_effect_documents.py` | RGBNoise provenance와 FilmNoise fail-closed 회귀 |
| `Tools/LevelPlacementExtractor/build_effect_source_material_contract.py` | exact parent → finite runtime profile |
| `Tools/LevelPlacementExtractor/test_build_effect_source_material_contract.py` | exact/suffix/근사 이름 오승격 방지 |
| `Client/Public/Effect_MaterialTemplate.h` | 실행 가능한 profile whitelist |
| `Client/Private/Effect_DocumentRenderer.cpp` | profile ID → shader index 및 parameter staging |
| `Client/Private/Effect_DocumentCodec.cpp` | profile별 drawable resource 계약 검증 |
| `Client/Bin/ShaderFiles/Shader_Deferred.hlsl` | RGBNoise additive 포화 제거 |
| `Client/Bin/ShaderFiles/Shader_EffectCommon.hlsli` | profile 7~9의 유한 Material 식 |

## 2. G01 Screen Post 코드 계약

`screen_post_presentation`은 다음 의미를 보장한다.

```python
if subtype == "RGB_NOISE":
    intensity = max(0.0, dynamic_values.get("rgb_str", 0.0))
    secondary = 0.0
    runtime_enabled = True
elif subtype.startswith("ZOOM_BLUR"):
    intensity = max(0.0, dynamic_values.get("blurstrength", 0.0))
    secondary = 0.0
    runtime_enabled = True
else:
    intensity = 0.0
    secondary = 0.0
    runtime_enabled = False
```

`source_dynamic_parameters`가 만든 `powerx` parameter는 제거하지 않는다. Shader의 RGBNoise pass는 RGB channel offset만 실행하고 다음 additive 식은 제거한다.

```hlsl
// 금지: source graph 근거가 없는 화면 명도 노이즈
vColor += fNoise * g_fPresentationSecondaryIntensity *
    g_vPresentationTint.rgb;
```

## 3. G02 Profile 선택 코드 계약

`runtime_shader_profile_id`의 exact table에 다음을 추가한다.

```python
"fx_m.fx_f_pa_shine_01_0_tr": "effect.ue3.shine.v1",
"fx_m.fx_j_pa_blacklineaura_01_tr": "effect.ue3.blackline-aura.v1",
"fx_m.fx_j_me_localcrack_01_tr": "effect.ue3.local-crack.v1",
```

현재 함수의 full identity 또는 `identity.endswith("." + material_path)` 조건은 유지한다. substring/contains는 금지한다.

Runtime profile index는 다음과 같다.

```text
0 reconstructed-standard
1 circle
2 dot
3 ring
4 aura
5 one-layer-distortion
6 grouped-translucent
7 shine
8 blackline-aura
9 local-crack
```

Codec drawable 계약:

- Shine: sprite의 Base/Mask/Emissive 중 하나가 있어야 한다.
- Blackline Aura: mesh binding은 기존 mesh 규칙을 통과하고 Mask 또는 Dissolve가 있어야 한다.
- Local Crack: mesh binding과 Dissolve가 있어야 한다.
- 조건이 없으면 stage 실패 또는 generator fallback-blocked이며 white base를 발명하지 않는다.

## 4. G03 Shader 식

### 4.1 Shine

```hlsl
float2 ellipseP = (uv - 0.5f) * float2(1.f, 2.f);
float radial = pow(saturate(1.f - length(ellipseP) * 2.f), power);
float carrier = base.a * (hasMask ? mask.r : luminance(base.rgb));
float alpha = carrier * radial * opacity;
float3 rgb = (base.rgb + noise.rgb * noiseStrength) * noisecolor.rgb;
```

목적은 원본 Sprite 크기를 바꾸는 것이 아니라 카드의 불투명 사각 외곽을 제거하는 것이다.

### 4.2 Blackline Aura

```hlsl
float maskValue = hasMask ? mask.r : 0.f;
float dissolveValue = hasDissolve ? dissolve.r : 1.f;
float alpha = pow(saturate(maskValue), maskPower) * dissolveValue;
float3 rgb = lerp(diffColor.rgb, maskColor.rgb, maskValue);
```

Base는 색 carrier 보조로만 사용하고 alpha 근거로 강제하지 않는다.

### 4.3 Local Crack

```hlsl
float dissolveValue = hasDissolve ? dissolve.r : 0.f;
float alpha = smoothstep(threshold - softness,
                         threshold + softness,
                         dissolveValue);
float3 rgb = lerp(outColor.rgb, inColor.rgb, dissolveValue);
```

Mesh geometry는 원본 binding을 사용하고 dissolve texture가 없으면 그리지 않는다.

## 5. G04 테스트 목록

Python:

```text
test_screen_post_preserves_dynamic_parameter_provenance
test_film_noise_without_source_gain_is_fail_closed
test_exact_parent_profiles_map_to_finite_runtime_profiles
test_similar_parent_name_does_not_promote
test_local_crack_required_runtime_contract
```

데이터 감사:

```text
Q/W/T/S element kind count unchanged
RGBNoise secondaryIntensity == 0
FilmNoise enabled == false
new profile occurrence count > 0
normal/data as Base == 0
malformed LUT == 0
```

C++/실행:

```text
Effect document stage/load PASS
unsupported profile FAIL with reason
Client Debug build error 0
startup smoke survives 12 seconds
Effect Tool final harness PASS
ProjectAudit PASS or pre-existing unrelated failure isolated
```

## 6. 재생성 순서

1. Material evidence/contract 생성
2. Imported document materialize
3. base11 canonical promotion
4. Assembly/WFX compile
5. Publisher Validate
6. Runtime Catalog atomic publish
7. Client build/smoke

중간 실패 시 기존 Runtime Catalog를 유지하고 부분 publish를 완료로 기록하지 않는다.

## 7. 수동 캡처 계약

각 캡처에 다음 정보를 함께 남긴다.

```text
class = DimensionMaster
activeEffectId
sampleTimeSeconds
selectedEmitterId
screenPostEnabled
fixed camera transform
```

S는 2050220 원본을 새로 캡처하고, E는 `fm_h_box_01_1.wmodel` 후보의 `useModelMaterial=true` 경로를 Solo하여 모델 내장 Material 문제가 맞는지 분리한다.
