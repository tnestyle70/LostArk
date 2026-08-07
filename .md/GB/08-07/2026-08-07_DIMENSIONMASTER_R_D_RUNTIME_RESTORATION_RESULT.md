# 차원술사 R/D Cascade 런타임 복원 결과

## 1. 결론

이번 화면은 하나의 원인으로 설명되지 않는다.

1. 정상 원본 R 레퍼런스는 현재 canonical `R=2050180 / foldcut`이다.
2. 깨진 툴 캡처는 구 후보 `2050190`을 선택한 화면이므로 동일 스킬 A/B가 아니다.
3. 캡처의 흰 덩어리, 보라/검정 사각면은 billboard 자체가 생긴 버그가 아니라 원래
   particle carrier인 sprite/mesh plane의 alpha, mask, Dynamic Parameter, texture role이 완성되지
   않아 carrier 외곽이 노출된 결과다.
4. 2050240은 현재 `D`이며, 어제 완료한 LUT, Vector3 stride, XYZ lock, Color/SubUV 중복 제거,
   sprite size 계약은 수치 실행 복원이다. Material expression과 Light/Post까지 pixel-equivalent로
   복원됐다는 뜻은 아니다.

Animation 작업자가 완료한 D의 `b_wp_swm_m_2/follow` cue, clip 순서
`telekinesisthrust_01 -> telekinesisthrust_04`, stage offset `0.5s`는 그대로 보존했다. 조사 중 제기된
top-level root anchor 가설은 기존 계약과 충돌하므로 폐기했고, 잠시 적용했던 한 줄은 원복해
`DimensionMaster.animevents`의 최종 diff가 0임을 확인했다.

## 2. 이미지와 정본 식별

| 구분 | 실제 의미 |
|---|---|
| 사용자가 제공한 정상 원본 R 이미지 | `2050180 / foldcut`의 짧고 넓은 보라색 검광, 잔광, 파편 |
| 깨진 툴 캡처의 선택 행 | 과거 후보 `2050190`, current roster가 아님 |
| current R | `2050180` |
| current A | `2050210` |
| current D | `2050240` |

따라서 구 `2050190` 캡처를 `2050180` 원본과 픽셀 단위로 직접 비교하면 안 된다. 다만 구 문서도
source material profile 없이 fallback을 타므로 불투명 판과 흰 덩어리가 나타나는 렌더 결함의 증거로는
유효하다.

다른 활성 작업의 R 전용 분석과도 같은 결론을 교차 확인했다. 공통 Renderer/Playback/Codec/HLSL은
이 변경에서만 수정하고, 그 작업에는 canonical R의 emitter/reference 매칭만 맡겨 충돌을 피했다.

## 3. 재현된 런타임 결함과 수정

### Mesh-backed Particle Material 실행

D의 Particle 46개 중 8개는 mesh-backed다. 기존 경로는 다음 값을 sprite particle shader에만
bind했고 mesh shader에는 전달하지 않았다.

- source shader profile
- scalar/vector parameter
- Dynamic Parameter semantic과 particle별 값

mesh shader는 원본 Effect profile 대신 임의 directional light를 곱한 generic `Shade_Effect`를
실행했다. 이 경로에서는 얇은 검광이나 마스크만 보여야 하는 mesh plane 전체가 흰색, 보라색 또는
검정색 판으로 보일 수 있다.

`CEffectDocumentRenderer`는 mesh shader에도 source profile 입력을 bind하고 mesh particle별
Dynamic Parameter를 전달하도록 교정했다. `Shader_VtxEffectMeshPreview.hlsl`도 particle과 같은
`Shade_EffectParticle` 경로를 사용한다. 일반 mesh와 afterimage에는 identity dynamic 값을 사용한다.

### Material 계약 fail-closed

기존 codec/publisher는 source profile의 stable ID 존재만 검사했다. renderer가 모르는 profile,
Dynamic Parameter semantic, SubUV mode도 draw 직전까지 통과할 수 있었다. 현재 실제 지원 집합을
C++와 publisher 양쪽에서 검사하고 알 수 없는 값은 load/publish stage에서 거부한다.

`one-layer-distortion`은 authored intensity 0도 최소 0.01로 강제해 원본에 없는 왜곡을 만들었다.
현재는 0을 그대로 보존한다.

### 3 FPS fixed-step

기존 frame당 catch-up 상한 8 step은 3 FPS 입력 한 번에 필요한 약 20 step을 처리하지 못했다.
매 frame 약 0.2초씩 simulation backlog가 누적됐다. 상한을 1초 분량 60 step으로 늘리고
3/30/60/144 FPS의 sample time과 구조 snapshot이 같은지 회귀 검사한다.

## 4. 자동 검증 결과

```text
Effect source/import Python tests            32 PASS
Effect publish validate                      16 admitted Effects PASS
Skill binding fast harness                   failures 0
Effect executor fast harness                 failures 0
Effect runtime fast harness                  failures 0
3/30/60/144 FPS determinism                  PASS
Unknown profile/semantic/SubUV fail-closed   PASS
Effect Tool final harness                    PASS
ClientFrontendHarness x64 Debug build        errors 0
Client x64 Debug/HLSL build                  errors 0
ProjectAudit                                 76 checks PASS
targeted git diff --check                    PASS
```

Final audit의 정직한 D coverage는 다음과 같다.

```text
document elements                  51
GPU particle renderer coverage     46
unimplemented Light                 2
unimplemented ScreenPost            3
runtime-exact material profiles     0
reconstructed profile groups       21
```

## 5. 아직 복원되지 않은 것

이번 자동 검증은 원본 화면 복원 완료 판정이 아니다.

- D의 21개 material group은 전부 `RECONSTRUCTED_PROFILE`이고 `runtime_exact=0`이다.
- 16개 generic group, 32 occurrence는 원본 Material expression topology가 없다.
- 6개 sprite emitter는 불투명 base texture와 generic alpha 조합 때문에 full-quad alpha가 남는다.
- named texture parameter 50개, scalar 303개, vector 19개 중 상당수가 현재 profile 식에서 소비되지
  않는다.
- texture name heuristic은 normal/mask/emissive를 base slot으로 잘못 고를 수 있다.
- 원본 blend/depth/render-state가 capture되지 않은 항목은 추정값이다.
- Light 2와 ScreenPost 3은 parse/stage되지만 GPU presentation은 아직 없다.
- source action cue의 Particle 26, Sound 11, Character Afterimage 8, Visibility 2, Camera Shake 2는
  complete Effect renderer와 별도 실행 계약이 남아 있다.
- local-space source default, sort mode, 일부 axis-lock orientation은 별도 source 근거와 재생성이
  필요해 이번 변경에서 추측 수정하지 않았다.

따라서 현재 정확한 완료 표현은 “수치 변환과 공통 mesh Material 실행 결함을 교정했다”다.
“R/D 원본 이펙트가 완전히 복원됐다”는 판정은 canonical `R=2050180`, `D=2050240`을 같은 카메라와
같은 sample time에서 다시 캡처해 emitter별 A/B를 통과한 뒤에만 가능하다.

## 6. 다음 순서

1. 새 Debug Client에서 canonical `R=2050180`을 로드하고 구 `2050190`이 아닌지 확인한다.
2. R 원본 프레임과 `2050180`을 같은 카메라/sample time으로 캡처한다.
3. 흰 덩어리, 보라 판, 검정/흰 부채꼴을 각 emitter의 base/mask/normal/emissive/Dynamic Parameter에
   연결한다.
4. D의 21개 reconstructed group 중 generic 16개부터 source Material expression과 render state를
   복원한다.
5. Material A/B가 닫힌 뒤 Light, Camera Shake, ScreenPost, Sound, Afterimage, Visibility를 순서대로
   연결한다.

Animation anchor, clip, timing은 이 순서의 튜닝 변수가 아니다.
