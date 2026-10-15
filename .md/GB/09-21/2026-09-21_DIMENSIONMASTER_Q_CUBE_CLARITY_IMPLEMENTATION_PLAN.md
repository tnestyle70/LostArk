# 차원술사 Q·R·V 재질 적응과 T 복구 구현 계획

## G00. 현재 입력과 적용 범위

후속 사용자 지시는 현재 렌더링이 올바르다는 전제에서 Q 황금 큐브, R 검격의 가시성,
V 시작의 큰 하늘색 유리를 수정하는 것이다. 정상 판정을 받은 ALT V와 A 검격은 기준으로
보존한다. 전역 tone/LUT·조명·Bloom 또는 별도 독립 색 합성 경로를 변경하지 않는다.
아래 G01~G04는 앞선 Q 후보의 구현과 검증 범위이며 R·V는 실제 재질별 원인을 구분한다.
추가 요청한 T는 환경 조명 수신을 먼저 복구하고 원본 이펙트 구성 복원은 별도로 대조한다.

사용자 요청은 Q의 초기 깔끔한 큐브 표현을 스킬 재질 계산만으로 유지하는 것이다. 장면 조명,
노출, tone/LUT, 전역·문서 Bloom 설정은 수정하지 않는다. 기존 큐브의 mesh, 굴절 UV, 모서리,
배치, 입자 수·수명·속도와 Q의 검격·파편은 보존한다.

현재 브랜치는 `codex/spider-pattern-fear-sound`이며 다른 작업의 큰 dirty 변경이 있다.
브랜치를 전환하거나 자동 stage/commit하지 않고 이번 파일의 필요한 부분만 수정한다.
Q 비교본 `effect.dimensionmaster.skill.2050100.restore`는 두 요소이며 사용자가 저장한
`bloomIntensity=16`을 보존한다. 실제 animevent는 다섯 요소의
`effect.dimensionmaster.skill.2050100.tuning.restore`를 사용한다. 두 문서에서 같은 stable ID
`authored.source-particle.b4984eeabefb0fc93783ecda`의 CubeSample 하나만 보정을 켠다.
`full.restore`, R/A의 동일 재질과 다른 모든 스킬은 기존 계산을 유지한다.

현재 CubeSample은 HDR 배경을 읽어 `lerp(5*C^5,C,facing)`에 색 배율을 곱한다.
Q의 color는 `[4,3.79999995,3,1]`, edge strength는2, edge exponent는30이다.
밝은 입력에서는 본체가 모서리와 함께 tone shoulder로 압축된다. 종료 기준은 전체 Bloom을
사용하지 않는 수치 조건에서 밝은 배경의 본체와 모서리 차이를 확보하고, opt-off와 다른 스킬의
기존 계산을 유지하며, 제품 빌드 후 사용자가 실제 화면을 판정할 수 있게 하는 것이다.

## G01. 기존 material scalar와 shader 연결

| 파일 | 변경 책임 |
|---|---|
| `Client/Private/Effect_DocumentCodec_MaterialValidation.cpp` | CubeSample의 optional `project_clarity_strength`를 유한한0~1로 검증 |
| `Client/Private/Effect_DocumentRenderer_ResourceStaging.cpp` | 기존 `vSourceScalars0.w`에 이름으로 값을 stage; 생략0; 실패 시 기존 resource 보존 |
| `Client/Bin/ShaderFiles/Shader_EffectCubeSampleScene.hlsli` | 기존 함수 안에서 명시 보정분의 본체 highlight와 투과 coverage를 계산 |
| Q restore/tuning authored JSON | 해당 cube scalar에 `group=Project tuning`, `value=1` 추가 |

새 C++ 파일, enum, persistent state, CBuffer, shader program ID는 만들지 않는다.
기존 source scalar parse/serialize, 이름별 material UI, authoring override와 staging signature가
이 값을 전달한다. 이 값은 원본 MIC 파라미터가 아닌 프로젝트 보정이다. 0 또는 생략은 기존
shader 분기로 돌아간다. generic scalar UI에서0~1로 편집하며 범위 밖 값은 검증에서 거절한다.

흐름은 authored named scalar → codec 검증 → `Stage_ElementResource`의 local staged float4 →
기존 `Bind_Common`의 float4 binding → `Shade_EffectCubeSampleScene`이다. 새로운 renderer나
class/skill ID 분기를 추가하지 않는다. 기존 프로젝트에 등록된 CPP/HLSLI만 수정하므로
vcxproj/filters 추가는 없다.

## G02. 1차 CubeSample 계산 — 사용자 화면 판정 후 G04로 대체

원본 출력은 그대로 계산하고 강도0이면 기존 결과를 반환한다. 보정이 켜지면 실제 SceneHDR
샘플로 다음 값을 한 번 정한다. 동일 fragment의 Bloom 보조 평가에서도 이 실제 장면 기준을
사용해 배경 기여와 자체 색의 분리를 유지한다.

- 실제 장면 max RGB의0.15~0.8 구간에서 보정을 부드럽게 활성화한다. 어두운 배경의 기존
  본체·반사 표현은 유지한다.
- tint와 Emissive까지 포함한 본체 RGB의 peak0.35 위를 부드럽게 압축한다. 상한은
  `0.65 + 0.45*facing`으로 하여 옆면과 정면의 밝기 차이를 남긴다. RGB 공통 배율을 써 색 비율을
  유지하며 채널별 saturate로 흰색을 만들지 않는다.
- caustic은 본체와 분리하고 밝은 장면에서 같은 방식으로 peak0.75에 접근하게 한다.
  기존 edge mask·색·강도는 유지하여 모서리의 밝기 여유를 확보한다.
- 실제 배경 max RGB0.5~4에서 `a + w*a*(1-a)`의 보정 coverage로 점진 전환한다.
  fade의0/1은 그대로이며 clip 판정은 보정 전 원본 alpha를 사용한다. 중간 fade의 투과율만
  달라지고 발생 시간·수명·depth/cull/blend 상태는 바꾸지 않는다.
- 보정분의 scene 전달은 실제 장면에서 고정한 transmission gain을 SceneColor 또는 기존
  SceneBloom에 곱한다. 원본 출력과 보정 출력을 강도0~1로 보간한다. 강도0은 기존 보조 평가도
  유지하고 강도1의 보정분은 선형으로 Bloom을 운반한다. 배경 HDR에서 Bloom을 재생성하지 않는다.

이것은 원본 shader 식의 재복원 주장이 아니라 요청한 밝은 배경 가독성을 위한 선택적 저작
보정이다. 실제 화면의 미적 판정과 수치 대비 검사를 구분한다.

## G03. 후보 검증과 반영

기존 파일의 byte 인코딩·개행을 확인하고 보존한다. 데이터는 out에 후보를 만들고 정확히 두
stable element의 scalar 추가만 있는지 구조 비교한다. 저장·재Parse와 허용값0/1 및 음수·1초과
거절을 확인한다. 원본/다른 문서는 변경하지 않는다.

창 없는 focused shader 수치 검사에서 강도0의 원본 동일성, 어두운/중간/밝은/유색 HDR 입력,
정면/옆면, 중심/edge, alpha0/0.6/1, SceneBloom0과 운반값을 확인한다. 실제 production include를
컴파일하며 별도 영구 harness/project는 추가하지 않는다. 수정 CPP와 generic Mesh shader를
컴파일하고 정상 Product Debug 증분 빌드로 설치한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
git diff --check
```

Client/Server가 현재 제품 출력을 점유하고 있으므로 후보 검증을 마친 뒤 필요한 종료를 안내한다.
사용자가 편집 중인 Q의 최종 데이터 교체는 AGENTS의 편집 보존 계약에 따라 저장본 적용 기준을
한 번 확인한다. 승인 뒤 최신 디스크를 다시 읽어 stable ID와 scalar 필드만 병합하고 hash 재확인,
backup과 원자적 교체를 수행한다. 사용자 Reload·프로세스 종료·실행은 자동 수행하지 않는다.

사용자는 제품 빌드 후 Effect Tool V1의 `이펙트_차원술사Q` 및 Q_튜닝을 다시 열거나 Reload하고,
Lobby → Character Select → 차원술사의 Q로 확인한다. 세 맵의 최종 선명도 판정은 사용자에게
남기며, 실제 적용·컴파일·수치 검증·미확인 화면은 대응 RESULT에 분리 기록한다.

## G04. 사용자 화면 피드백에 따른 황금색 자체 발광

1차 수정은 면의 밝기를 억제하면서 황금색 면 발광을 만들지 못했다. 현 단계는 G02의
caustic 압축을 제거하고 배경 흡수와 자체 발광에 원본 금색을 연결한다. 이전 scalar1이 저장된 Q 두 문서만 그대로 사용하며
JSON/C++/전역 rendering 설정은 추가 변경하지 않는다.

`Shader_EffectCubeSampleScene.hlsli`에서 배경 굴절분만 기존 bounded transmission으로
전달한다. 이를 큐브 자체 발광에는 적용하지 않는다. 원본 particle RGB `(7,6,1)`을 최대
채널로 나눈 chroma `(1,6/7,1/7)`를 배경 transmission에 곱해 금색 흡수를 만든다. 같은 chroma를
particle RGB와 MIC color `(4,3.8,3,1)`의 곱에도 적용하고
`0.0875 + 0.0525*grazing*grazing`으로 독립 면 발광을 만든다.
실제 Q의 정면 발광은 `(2.45,1.71,0.0375)`, 측면은 `(3.92,2.736,0.06)`의 HDR이다. 이 face 계수는
요청한 밝은 황금색을 위한 PROJECT_TUNED이며 원본 shader 식의 복원으로 분류하지 않는다.

원래 caustic과 edge를 별도로 더한다. 원본 alpha0.6은 밝은 배경에서 최대0.84까지 강화하되
fade0/1과 원본 clip을 유지한다. 회색 HDR4에서 alpha0.6의 배경 잔여만1.6이라 source tone이
blue를 약0.9까지 올리므로, 자체 발광과 함께 배경 누출도 줄여야 색이 남는다. 발광 RGB에
alpha를 중복 적용하지 않는다. 자체 발광은 SceneColor/Bloom/black의 세 평가에 같은 값으로
더해 `F(SceneBloom)-F(black)+Write_SceneBloom(F(black))`의 기존 분리 계약을 사용한다.
문서 Bloom intensity가 자기 발광에 적용되고 Bloom off에서도 황금색 자체는 남는다.
강도0은 기존 RGB/alpha를 유지한다. 새 파일·프로젝트 등록·CBuffer는 없다.

Character Select의 현재 source tone/toe0.5와 중성 grading은 활성 원본 자료와 맞는다.
멀리 떨어진 별도 source volume의 금빛 Bloom tint/LUT를 중앙 아레나에 전역 적용하지 않는다.
directional OFF는 baked 배경과 tone을 끄지 않으며, LUT OFF도 source tone을 끄지 않는다.

기존 focused GPU 검사를 재사용하되 실제 Q의 `(7,6,1,0.6)`과 원본 caustic DDS를 읽고
면 중앙·모서리·시선 각도를 포함한다. SceneBloom0에서도 자체 발광과 Bloom이 양수인지,
intensity0/1.3/16의 RGB 불변/Bloom 응답, opt0 원본 동일성, alpha끝값/범위/clip, SceneBloom 선형 운반,
현재 Character Select tone 이후 황금색 성분을 검사한다. 단순R>G>B 대신 회색 배경0.5/1에서
center R−B≥0.2, 배경4에서≥0.12인 표본 기준을 둔다. 이는 수정 전의 백색화를 검출하기 위한
수치 조건이며 실제 맵의 미적 승인이나 극단적인 HDR16 이상의 대비 보장은 아니다. 최종 시각 판정은 사용자에게 남긴다.
소스 반영 후 정상 Product Debug Build로 CSO를 설치한다. 실행 파일 잠금 해제에 필요한
사용자 종료만 안내하며 Q 데이터 저장 승인을 다시 요구하지 않는다.

## G05. Q의 면과 테두리 대비를 함께 유지

G04 최초 금색 후보의 face gain1은 현재 tone에서 면 중앙까지 포화시켜 테두리 대비를
줄였다. 원본/D6/최초 금색 후보를 현재 CS·Bern·G3 포함7개 profile/region으로 비교한 뒤,
face gain0.35를 선택했다. 배경 transmission, chroma, alpha, 원래 caustic/edge는 그대로 두고
위 G04의 face 계수만 낮춘다. CS 회색 배경0.5/1/4에서 정면 금색 R−B는0.343/0.242/0.170,
edge−center 밝기차는0.090/0.083/0.051이다. facing0/0.5에서도 기존 금색 기준을 통과했다.
Bern/G3의 밝은 배경4는 테두리 밝기차0.030/0.026이며 모든 맵의 동일한 대비를 보장하지 않는다.
최종 실제 source include를 재검사하고 Product Build로 배포한다. 별도 맵 보정은 만들지 않는다.

## G06. V 시작 유리의 면 투과와 가장자리 보존

V 시작 세 파편은 `effect.ue3.v-fx-j-me-localcrack-01-07-tr-native.v1`이다.
자체 normal/reflection/noise 텍스처를 사용하며 기본색에서 SceneColor를 직접 읽지 않는다.
선택 PS 본문은 정상 ALT V native145와 같다. 현재 DDS를 통과한 raw RGB는 검사 표본에서
R0.198~3.648/G0.355~3.737/B1.297~5.814이며 원본 particle B500을 RT0B500으로 해석하지 않는다.
단순 emissive 감쇠는 같은 alpha로 반사·모서리까지 어둡게 만들어 유리 대신 어두운 면이 된다.

`Data/Effects/Authored/effect.dimensionmaster.skill.2050520.full.restore.effect.json`의 아래
세 stable element에서 기존 named scalar `fresnel_pow` 값만0.200000003→0.5로 조정한다.

- `authored.source-particle.full-v.462eea59a1d0912fa5a1`
- `authored.source-particle.full-v.18ce1c993cce94e337e5`
- `authored.source-particle.full-v.ebad38e09d7a635e4e66`

현재 codec→native parameter builder→VNative66의 기존 source[11].x가 이 값을 소비한다.
면의 opacity는 낮추되 grazing rim의 alpha1과 source fade0을 유지한다. 실제 shader/DDS 표본의
viewZ0.9/0.5/0.1에서 coverage는0.138/0.690/1에서0.007/0.395/1로 바뀐다.
색곡선·텍스처·emissive·크기·위치·재생 시점·distortion·다른 occurrence는 그대로 둔다.
이 세 재질의 PROJECT_TUNED 투명도 조정이며 원본 MIC의0.2 자체가 잘못됐다고 기록하지 않는다.

후보는 최신 저장본의 해당 필드만 병합해 만들고 무관한 전체 구조 동일성, 실제 codec admission,
설치 mesh winding과 native shader/DDS GPU 표본을 확인한다. 교체 직전 파일 hash를 재확인하고
백업·원자 교체하며 실패하면 자기 변경만 되돌린다. 새 C++/shader/프로젝트 등록은 없다.

## G07. T 소환체의 환경 조명 복구와 우선 검증 빌드

사용자는 T가 환경 렌더링을 다시 받도록 명시했다. `Render_ModelCues`에서 기존
`Is_DimensionSummonCharacterSurfaceCue`가 선택하는 정확한 T 소환체만 animated pass11에서
pass0으로 돌린다. 기존 MASKED_SURFACE, NONBLEND 제출과 clip0.3은 유지한다. 두 pass는
RS/DSS/BS/VS가 같고 pass0의 PS_MAIN이 정상 조명용 GBuffer를 생성한다. 다른 cue와
공통 renderer, shader pass 번호, 전역 tone/LUT/Bloom은 수정하지 않는다.

`Client/Private/Effect_DocumentRenderer_Rendering.cpp`의 변경 코드는 다음과 같다.

```cpp
// The exact T summon keeps its depth/mask path and receives normal map lighting.
if (bCharacterSurface)
    iPass = 0u;
```

새 C++ 파일이나 프로젝트 등록은 없다. 기존 파일의 인코딩과 CRLF를 유지하고 Product Debug
증분 빌드로 컴파일·설치한다. 현재 T unified 문서의23개 요소와 소환체를 전부 원본으로
복원한 것으로 기록하지 않는다. full restore의 원본 구성 대조와 R 검격 조사는 별도로 남으며,
사용자가 요청한 빠른 화면 검증을 위해 Q/V와 확인된 T 환경 조명 변경을 먼저 빌드한다.

## G08. 2026-10-12 CubeSample의 배경 증폭 Bloom 누락 수정

현재 Q의 emissive 바인딩과 G05 면 발광은 살아 있다. 그러나 Mesh carrier의
`F(SceneBloom)-F(black)+Write_SceneBloom(F(black))`는 배경을 재질이 증폭하여
새로 threshold를 넘긴 부분을 누락한다. R/A 보라 큐브의 정면 중앙에서 배경 RGB0.4,
MIC `(2,.65,4)`, emissive1이면 출력 RGB는 `(.8,.26,1.6)`인데 배경 Bloom0과
`F(black)=0` 때문에 Bloom은0이다. 이 조건은 밝기 상수 조정 전에 재현할 대상이다.

기존 `Shader_EffectCubeSampleScene.hlsli`의 SceneColor/alpha/clip 계산을 보존하고
같은 현재 배경에서 얻은 RGB 전달 계수 T로 별도 RT2를 만든다. C는 배경 HDR,
B는 이미 소유자별 배율이 반영된 배경 Bloom, X는 배율 없는 `Extract_SceneBloom`,
I는 현재 문서의 Bloom 배율이다. 이번 프로젝트 합성 정책은 다음과 같다.

```text
RT2.rgb = clamp(T * B + I * max(X(F(C)) - T * X(C), 0), 0, 60000)
RT2.a = RT0.a
```

기존 배경 Bloom은 T로 한 번 전달하며, 재질 때문에 추가된 bright-pass만 문서 배율을
받는다. 항등 전달(T1, 자체 발광0)은 B를 유지하고 억제된 배경 Bloom을 재생성하지 않는다.
검은 배경에서는 기존 자체 발광 추출과 같다. C^5에 B를 대입하지 않으므로 다른 스킬의
Bloom 배율을5승으로 왜곡하지 않는다. 이는 UE3 원본의 미확인 보조 MRT 복원 주장이 아니다.

### 파일과 적용 위치

- `Client/Bin/ShaderFiles/Shader_EffectCubeSampleScene.hlsli`: 기존 refracted 계산 직후
  `bloomTransmission`을 계산하고, clarity 블록의 기존 `transmission`을 같은 strength로
  혼합한다. clip 이후 RT2를 계산한다.
- `Client/Bin/ShaderFiles/Shader_EffectMeshFamilyCarrier.hlsli`: generic family의 PS_MAIN에서
  invocation-local `g_CubeSampleBloomWritten`을 초기화하고, 실제 CubeSample 함수가
  RT2를 쓴 경우 그 출력을 반환한다. 기존 상위 material backend 선택 순서는 유지한다.

새 파일·CBuffer·public pass·프로젝트 등록·JSON 변경은 없다. Q 실제 cue의 tuning 문서는
root Bloom 생략 기본1.3이고 비교용 restore만16이라는 차이를 유지한다. 적용 대상은
Q의 금색과 R/A의 보라 큐브가 공유하는 CubeSample이다. C는 비음수 HDR radiance 기준이며
기존 abs(C)^5가 처리하는 음수 입력까지 같은 물리적 분해라고 설명하지 않는다.

### 교체 가능한 추가 코드

CubeSample include 아래의 invocation-local 표식:

```hlsl
static bool g_CubeSampleBloomWritten = false;
```

`refracted` 선언 바로 아래:

```hlsl
float3 bloomTransmission = (5.f * power2 * power2 * grazing + facing) *
    g_SourceVector0.rgb * g_SourceVector0.a * g_EmissiveIntensity;
```

clarity 블록의 `transmission` 선언 바로 아래:

```hlsl
bloomTransmission = (1.f - strength) * bloomTransmission +
    strength * transmission;
```

기존 clip 다음, return 바로 위:

```hlsl
const float3 sceneBloom = g_EffectSceneBloomTexture.Sample(
    LinearClampUVSampler, sceneUV).rgb;
const float3 newBloom = max(Extract_SceneBloom(output.SceneColor.rgb) -
    bloomTransmission * Extract_SceneBloom(scene), 0.f);
const float bloomIntensity = clamp(g_fEffectBloomIntensity >= 0.f ?
    g_fEffectBloomIntensity : g_fSceneBloomIntensity, 0.f, 16.f);
output.BloomContribution = float4(min(max(bloomTransmission * sceneBloom,
    0.f) + newBloom * bloomIntensity, 60000.f), output.SceneColor.a);
g_CubeSampleBloomWritten = true;
```

Mesh carrier의 `PS_MAIN` 함수 전문:

```hlsl
EFFECT_PS_OUT PS_MAIN(VS_OUT input, bool frontFace : SV_IsFrontFace)
{
    g_EffectSceneReadMode = 0u;
    g_EffectSceneSampleUsed = false;
#if EFFECT_SHADER_FAMILY == 0
    g_CubeSampleBloomWritten = false;
#endif
    EFFECT_PS_OUT output = PS_MATERIAL(input, frontFace);
#if EFFECT_SHADER_FAMILY == 0
    if (g_CubeSampleBloomWritten)
        return output;
#endif
    if (!g_EffectSceneSampleUsed)
        return Write_EffectBloom(output);
    g_EffectSceneReadMode = 1u;
    const EFFECT_PS_OUT transported = PS_MATERIAL(input, frontFace);
    g_EffectSceneReadMode = 2u;
    const EFFECT_PS_OUT emission = PS_MATERIAL(input, frontFace);
    g_EffectSceneReadMode = 0u;
    output.BloomContribution = float4(transported.SceneColor.rgb - emission.SceneColor.rgb +
        Write_SceneBloom(emission.SceneColor).rgb, output.SceneColor.a);
    return output;
}
```

### 검증

production include의 수정 전후 창 없는 GPU 수치 비교로 누락 재현, RT0·alpha 보존,
검은 배경의 자체 발광, 항등 전달, 배경 Bloom 선형 전달, intensity0과 alpha coverage를
확인한다. 임시 산출물은 `out/CubeBloom20261012`에만 둔다. 정상 Product Debug Build로
소비 CSO를 갱신하고 `git diff --check`를 확인한다. 공통 carrier include 변경으로
의존 native FX도 정상 증분 컴파일될 수 있다. Client/UI는 실행하지 않는다.
전역 Bloom OFF에서 halo가 나타난다거나 source tone/LUT 이후 색이 모든 맵에서 같다고
보장하지 않는다. 최종 화면은 사용자가 실제 Q와 보라 큐브에서 판정한다.

## G17. 2026-10-12 Q/R/A 국소 색 응답

### 목표와 연결 범위

초기 Q2 성공 기록과 첫 Git 보존본의 native CubeSample, Hable exposure .6351,
white point 1, gamma 1.905를 색 기여분의 기준으로 사용한다. 과거 스크린샷 파일은
확보하지 못했으므로 화면의 완전 동일 복원을 주장하지 않는다. 현재 전역 프로필과
LUT, Bloom/FXAA 상태는 저장하거나 변경하지 않는다.

`Bind_MaterialInputs`가 실제 Q/R/A 문서 ID와 여섯 element ID 및 source profile42를
모두 확인하고 실제 blend가 one/two-sided alpha depth-read인 경우에만 적용한다.
generic mesh의 매 draw에서 선택 flag를 다시 쓰고, 선택된 경우에만
Renderer가 소유한 현재 exposure/source curve/graded LUT를 read-only 바인딩한다.
Engine은 skill ID를 알지 않으며 `Render_Final`과 동일한 curve packing을 사용한다.

### 수식과 실패 경계

현재 굴절 전 pixel 배경 D, native 재질 S0, 현재 재질 S, 원래 alpha a0와 현재 alpha a,
과거 Hable H, 현재 tone/LUT G에 대해 목표는 다음과 같다.

```text
target = saturate(G(D) + H(lerp(D,S0,a0)) - H(D))
G(lerp(D,S',a)) ~= target, 0 <= S' <= 60000
```

bounded tone-space solve는 최대8회, RGB와 휘도 가중 오차를 최소화한다. source tone의
shoulder 위는 rational inverse를 사용하고 아래만 bounded bisection한다. alpha0,
비대상 draw, source-post OFF, invalid input은 기존 재질을 반환한다. 기존보다 가중
오차가 커지는 해도 사용하지 않는다. LUT의 비가역 색역과 같은 occurrence 안의 고정
snapshot 겹침은 근사 경계로 남는다. LUT lookup 비용은 별도 검증한다.

### 파일 역할과 검증

- 새 `Shader_EffectCubeColorResponse.hlsli`: 역사 기준 H, 현재 G, bounded solver를 소유한다.
- `Shader_EffectCubeSampleScene.hlsli`: native RT0를 보관하고 기존 RT2 계산을 완료한 뒤 RT0 RGB만 보정한다. alpha/clip/굴절 UV/RT2는 보존한다.
- `Effect_DocumentRenderer_MaterialBinding.cpp`: exact identity selector와 draw별 flag 초기화.
- `Renderer.h/.cpp`, `GameInstance.h/.cpp`: 현재 renderer-owned 색 변환의 read-only shader binding.
- Client project/filter: 새 HLSLI를 기존 include 항목 옆에 등록한다. 새 C++ 파일은 없다.

실제 제품 include의 GPU solver, 실제 DDS LUT bake, HDR16 alpha blend를 검사한다.
global 데이터 hash 보존, 비대상/alpha0 identity, finite/range, 1/2/5겹침과 edge 대비를
확인한 뒤 Product Debug/Release를 정상 증분 Build한다. Client/UI는 실행하지 않는다.
실제 반영 전문과 검증 결과는 구현이 확정된 다음 이 G와 RESULT에 동기화한다.
## G18. 2026-10-13 사용자 화면 확인 후 Q/R/A 발광 강화

Q는 사용자가 복구를 확인했으며 발광을 크게 올려 달라고 요청했다. R/A는 이전처럼 보인다는
관찰을 받았다. 현재 실제 cue부터 generic CubeSample까지 exact selector 연결은 정상이다.
R/A는 clarity0으로 면 자체 발광이 없고, G17의 clarity 이전 native 기준은 저작한 면 발광을
목표에서 제외한다. 단순 emissive 증가로 검은 면의0을 바꾸거나 밝은 C^5 포화를 풀 수 없다.

### 수정 파일과 값

- `Data/Effects/Authored/effect.dimensionmaster.skill.2050100.tuning.restore.effect.json`:
  `authored.source-particle.b4984eeabefb0fc93783ecda`의 `detail.color.emissiveIntensity`를1→4.
- `Data/Effects/Authored/effect.dimensionmaster.skill.2050180.tuning.restore.effect.json`:
  `authored.tuning.dimensionmaster.r.purple-glass-sequence`의 같은 필드를1→4,
  `material.sourceProfile.scalars`의 `project_clarity_strength`를생략0→1.
- `Data/Effects/Authored/effect.dimensionmaster.skill.2050210.a1.full.restore.effect.json`:
  `authored.composition.dimensionmaster.a.hit-1-purple-cube`부터 hit-4까지 같은 두 필드를변경.
- `Shader_EffectCubeSampleScene.hlsli::Shade_EffectCubeSampleScene`:
  clarity 이전의 임시 nativeColor를 제거하고 최종 저작 RGB/alpha를 색 응답에 전달한다.
- `Shader_EffectCubeColorResponse.hlsli::CubeColor_Resolve`:
  별도의 nativeMaterial/nativeAlpha 입력을 제거하고 현재 저작 RGB/alpha로 historical target을
  계산한다. solver·LUT·오차 보호는 그대로다. RT2는 이 색 응답 전에 기존 식으로 계산한다.

새 목표는 `saturate(G(D) + H(lerp(D,Sauthored,a)) - H(D))`다. 원본 계산만의 복원에서
현재 저작한 면 발광·투과·coverage를 존중하는 PROJECT_TUNED 색 응답으로 바뀐다.
Q의 금색 면도 의도적으로 강화된다. R/A의 기존 clarity는 밝은 배경의 중간 alpha를
최대0.7→0.91로 올리며 fade0/1과 clip은 보존한다. 이를 단순 RGB만 변경한 것으로 쓰지 않는다.

### 보존과 검증

전역 rendering profile, LUT, Bloom, Resources, geometry·발생시간·수명·particle color와
다른 element는 유지한다. 새 C++/constant/schema/project 등록은 없다. 이미 존재하는 scalar의
codec0~1 검증과 staging SourceScalars0.w, 기존 emissive GPU binding을 재사용한다.
사용자의 반영 지시를 최신 디스크 저장본 적용 승인으로 삼고, 후보를 구조 비교한 뒤 직전 hash
확인·백업·원자 교체한다. 이 direct-authored 문서는 publish하거나 다른 폴더로 복사하지 않는다.
현재 메모리 draft의 Reload는 자동 수행하지 않는다.

검증은 세 문서의 기존 validator, exact 필드 차이, 실제 shader focused GPU, Debug/Release
Product Build와 설치 CSO를 구분한다. 과거 out 검증 파일은 현재 없어 재사용하지 않는다.
실제 화면 판정은 사용자에게 남긴다. 아래 코드 전문은 G18 최종 반영 소스에 동기화한다.

빌드 중 별도 작업의 `Client/Public/BossLogicFlowView.h:8`에서 잘린
`#include <string_vie`가 확인되어 `#include <string_view>` 한 줄만 복구했다.
기존 UTF-8 BOM과 해당 파일의 다른 변경을 보존한다. 큐브 기능의 C++ 확장은 아니다.

### G18 반영 코드 전문

아래 두 HLSLI는 G18의 최종 큐브 소스 전문이다. 마지막 헤더는 빌드를 막은 include 한 줄만 복구한 당시 사본이며, 나머지 변경은 다른 작업의 것이다. G17의 과거 전체 파일 사본과 구분한다.

#### C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_EffectCubeSampleScene.hlsli

SHA256: `be4a7f82fe588669e6a6a5b50971883dd0c19756f9a019467d0a72b407b1e770`

```hlsl
#ifndef EFFECT_CUBESAMPLE_SCENE_HLSLI
#define EFFECT_CUBESAMPLE_SCENE_HLSLI
#include "Shader_EffectSceneColorInput.hlsli"
#include "Shader_EffectCubeColorResponse.hlsli"

// 실제 CubeSample 분기가 쓴 RT2만 carrier가 보존한다. 픽셀 호출 사이에는 공유되지 않는다.
static bool g_CubeSampleBloomWritten = false;

float CubeSample_SourcePow(float value, float exponent)
{
    // Native log/exp branch returns zero even for the near-zero ^0 case.
    return abs(value) < 1.0e-6f ? 0.f : pow(abs(value), exponent);
}

// Limit only transmitted background radiance. Authored self-emission and
// the original caustic/edge terms retain their HDR range.
float CubeSample_ClarityTransmissionScale(float3 radiance, float ceiling)
{
    const float peak = max(radiance.r, max(radiance.g, radiance.b));
    const float excess = max(peak - 0.35f, 0.f);
    const float shoulder = ceiling - 0.35f;
    const float compressed = 0.35f + shoulder * (excess / (excess + shoulder));
    return peak > 0.35f ? compressed / peak : 1.f;
}

EFFECT_PS_OUT Shade_EffectCubeSampleScene(float2 meshUV,
    float2 pixelPosition, float3 tangentView, float4 particleColor)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    uint width = 0u;
    uint height = 0u;
    g_EffectSceneColorTexture.GetDimensions(width, height);
    if (g_SourceTextureMask != 1u || width == 0u || height == 0u ||
        !all(isfinite(tangentView)) || dot(tangentView, tangentView) < 1.0e-12f)
    {
        clip(-1.f);
        return output;
    }

    // PS f548f39885bbfe42ac405ccd46dc2919: selected unclamped CubeSample.
    // SceneHDR snapshot and particleColor replace engine-owned scene/prefix
    // inputs. Native fog and auxiliary MRT2-5 are outside this RT0 adapter.
    const float3 V = normalize(tangentView);
    const float facing = abs(V.z);
    const float grazing = 1.f - facing;
    const float2 sceneUV = pixelPosition / float2(width, height) +
        0.1f * g_SourceVector1.x * grazing * V.yx;
    const float3 scene = Read_EffectSceneColor(
        LinearClampUVSampler, sceneUV).rgb;
    const float3 powerBase = max(abs(scene), 1.0e-6f);
    const float3 power2 = powerBase * powerBase;
    const float3 power5 = power2 * power2 * powerBase;
    const float3 refracted = lerp(5.f * power5, scene, facing);
    // 가중된 Bloom을 C^5에 대입하지 않고, 현재 HDR 배경에서 정한 RGB 전달 계수를 쓴다.
    float3 bloomTransmission = (5.f * power2 * power2 * grazing + facing) *
        g_SourceVector0.rgb * g_SourceVector0.a * g_EmissiveIntensity;
    const float3 caustic = Sample_SourceTexture0(V.xy * 3.f).rgb *
        CubeSample_SourcePow(grazing, 7.f) * g_SourceScalars0.x;
    const float2 edgeUV = 2.f * (meshUV - 0.5f);
    const float edge = min(1.f,
        CubeSample_SourcePow(edgeUV.x, g_SourceScalars0.y) +
        CubeSample_SourcePow(edgeUV.y, g_SourceScalars0.y)) * g_SourceScalars0.z;

    // Mesh particle RGB owns the edge term; multiplying all body RGB by it
    // would apply the source color-over-life gain to the wrong expression.
    output.SceneColor.rgb = ((refracted + caustic) *
        g_SourceVector0.rgb * g_SourceVector0.a + edge * particleColor.rgb) *
        g_EmissiveIntensity;
    const float sourceAlpha = saturate(particleColor.a);
    output.SceneColor.a = sourceAlpha;
    if (g_SourceScalars0.w > 0.f)
    {
        // 보정된 색과 Bloom 전달은 같은 현재 HDR 배경에서 정한 계수를 소비한다.
        const float strength = saturate(g_SourceScalars0.w);
        const float3 actualScene = min(max(g_EffectSceneColorTexture.Sample(
            LinearClampUVSampler, sceneUV).rgb, 0.f), 60000.f);
        const float scenePeak = max(actualScene.r, max(actualScene.g, actualScene.b));
        const float sceneWeight = smoothstep(0.15f, 0.8f, scenePeak);
        const float3 actualSquared = actualScene * actualScene;
        const float3 nativeTransmission = (5.f * actualSquared * actualSquared *
            grazing + facing) * g_SourceVector0.rgb *
            g_SourceVector0.a * g_EmissiveIntensity;
        const float bodyScale = CubeSample_ClarityTransmissionScale(
            actualScene * nativeTransmission, 0.65f + 0.45f * facing);
        // Source-tone compression can wash a pale HDR tint to white. Use the
        // authored particle chroma for absorption as well as self-emission.
        const float3 particleRadiance = max(particleColor.rgb, 0.f);
        const float particlePeak = max(particleRadiance.r,
            max(particleRadiance.g, particleRadiance.b));
        const float3 particleChroma = particleRadiance / max(particlePeak, 1.e-4f);
        const float3 transmission = nativeTransmission * particleChroma *
            ((1.f - sceneWeight) + sceneWeight * bodyScale);
        bloomTransmission = (1.f - strength) * bloomTransmission +
            strength * transmission;
        const float3 nativeCaustic = caustic * g_SourceVector0.rgb *
            g_SourceVector0.a * g_EmissiveIntensity;
        // The source HDR particle color supplies the gold hue. This authored
        // face term is independent of SceneColor, so it remains in F(black)
        // and participates in the existing per-effect bloom extraction.
        const float faceEmissionWeight = 0.0875f + 0.0525f * grazing * grazing;
        const float3 faceEmission = particleRadiance * particleChroma * g_SourceVector0.rgb *
            g_SourceVector0.a * g_EmissiveIntensity * faceEmissionWeight;
        const float3 clearColor = scene * transmission + nativeCaustic +
            edge * particleColor.rgb * g_EmissiveIntensity + faceEmission;
        // strength1에서 큰 C^5 항끼리의 뺄셈 상쇄 없이 보정 색을 선택한다.
        output.SceneColor.rgb = (1.f - strength) * output.SceneColor.rgb +
            strength * clearColor;
        // Reduce unfiltered background leakage only on bright scenes. Preserve
        // fade endpoints and the original alpha-test threshold below.
        output.SceneColor.a += strength * smoothstep(0.5f, 4.f, scenePeak) *
            sourceAlpha * (1.f - sourceAlpha);
    }
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    if (g_ColorClip > 0.f)
        clip(sourceAlpha - g_ColorClip);
    // 기존 배경 Bloom은 한 번 전달하고, 재질 증폭으로 새로 생긴 bright-pass만
    // 현재 문서 배율을 받는다. 항등 전달에서는 억제된 배경 Bloom이 되살아나지 않는다.
    const float3 sceneBloom = g_EffectSceneBloomTexture.Sample(
        LinearClampUVSampler, sceneUV).rgb;
    const float3 newBloom = max(Extract_SceneBloom(output.SceneColor.rgb) -
        bloomTransmission * Extract_SceneBloom(scene), 0.f);
    const float bloomIntensity = clamp(g_fEffectBloomIntensity >= 0.f ?
        g_fEffectBloomIntensity : g_fSceneBloomIntensity, 0.f, 16.f);
    output.BloomContribution = float4(min(max(bloomTransmission * sceneBloom,
        0.f) + newBloom * bloomIntensity, 60000.f), output.SceneColor.a);
    g_CubeSampleBloomWritten = true;
    // 저작한 면 발광과 coverage까지 목표에 포함해야 색 응답이 그 보정을 다시 지우지 않는다.
    // Bloom은 위의 저작 HDR 에너지로 이미 계산했으며 이 단계는 RT0 RGB만 바꾼다.
    [branch] if (g_CubeColorResponseEnabled != 0)
        output.SceneColor.rgb = CubeColor_Resolve(output.SceneColor.rgb,
            g_EffectSceneColorTexture.SampleLevel(LinearClampUVSampler,
                pixelPosition / float2(width, height), 0.f).rgb,
            output.SceneColor.a);
    return output;
}

#endif
```

#### C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_EffectCubeColorResponse.hlsli

SHA256: `9c8ee99e9497034b03dc4fbce8e01d435b9e060b0996244811245ebeb8a1047e`

```hlsl
#ifndef EFFECT_CUBE_COLOR_RESPONSE_HLSLI
#define EFFECT_CUBE_COLOR_RESPONSE_HLSLI

// Renderer 소유의 현재 변환을 읽기만 한다. Client가 exact Q/R/A draw마다 선택을 갱신한다.
int g_CubeColorResponseEnabled = 0;
int g_iSourcePostProcessEnabled = 0;
float4 g_vSourceToneCurve = float4(.22f, 1.22f, .298f, 1.f);
float g_fSourceToneToe = 1.f;
float g_fToneMapExposure = 1.f;
Texture2D g_SourceGradingLut;

bool CubeColor_IsFinite(float3 value)
{
    // FXC의 fast-math가 isfinite를 제거해도 HDR 경계의 NaN/INF 방어는 남긴다.
    return all((asuint(value) & 0x7f800000u) != 0x7f800000u);
}

float3 CubeColor_Tone(float3 hdr)
{
    // Final의 Sanitize_HDR와 같은 노출 전후 범위다.
    const float3 color = clamp(clamp(hdr, 0.f, 60000.f) * g_fToneMapExposure, 0.f, 60000.f);
    const float3 rational = color / abs(color + g_vSourceToneCurve.x) * g_vSourceToneCurve.y;
    const float3 linearGamma = pow(max(color * g_vSourceToneCurve.w, 0.f), 1.f / 2.2f);
    const float3 shoulder = saturate((color - g_vSourceToneCurve.z) * 10000.f);
    return saturate(lerp(lerp(linearGamma, rational, shoulder), rational, saturate(g_fSourceToneToe)));
}

float3 CubeColor_Grade(float3 color)
{
    color = saturate(color);
    const float slice = floor(color.b * 14.9999f);
    const float2 uv = float2(slice / 16.f + color.r * (15.f / 256.f) + .5f / 256.f,
        color.g * (15.f / 16.f) + .5f / 16.f);
    const float3 low = g_SourceGradingLut.SampleLevel(LinearClampUVSampler, uv, 0.f).rgb;
    const float3 high = g_SourceGradingLut.SampleLevel(LinearClampUVSampler, uv + float2(1.f / 16.f, 0.f), 0.f).rgb;
    return lerp(low, high, color.b * 15.f - slice);
}

float3 CubeColor_Hable(float3 color)
{
    return ((color * (.15f * color + .05f) + .004f) /
        (color * (.15f * color + .50f) + .06f)) - .02f / .30f;
}

// 09-08 Q2 이후 첫 보존본의 Hable 설정이다. 현재 장면 옵션을 과거 값으로 되돌리지 않는다.
float3 CubeColor_Historical(float3 hdr)
{
    return pow(saturate(CubeColor_Hable(clamp(hdr, 0.f, 60000.f) * .6351f) /
        CubeColor_Hable(1.f.xxx)), 1.f / 1.905f);
}

float CubeColor_Metric(float3 a, float3 b)
{
    const float3 luma = float3(.299f, .587f, .114f);
    return dot(a, b) + 16.f * dot(a, luma) * dot(b, luma);
}

float CubeColor_Error(float3 a, float3 b)
{
    return CubeColor_Metric(a - b, a - b);
}

// shoulder 위는 유리함수의 역식으로 즉시 복원한다. toe 혼합 구간만 작은 HDR 구간에서 찾는다.
float3 CubeColor_InverseTone(float3 tone)
{
    const float exposure = max(g_fToneMapExposure, 1.e-6f);
    const float boundary = max(g_vSourceToneCurve.z + .0001f, 0.f) / exposure;
    const float3 threshold = CubeColor_Tone(boundary.xxx);
    const float3 rational = tone * g_vSourceToneCurve.x /
        max(g_vSourceToneCurve.y - tone, 1.e-6f) / exposure;
    if (all(tone >= threshold))
        return min(rational, 60000.f);
    float3 low = 0.f;
    float3 high = min(boundary, 60000.f).xxx;
    [loop] for (uint iteration = 0u; iteration < 20u; ++iteration)
    {
        const float3 middle = .5f * (low + high);
        const float3 mapped = CubeColor_Tone(middle);
        low = float3(mapped.x < tone.x ? middle.x : low.x,
            mapped.y < tone.y ? middle.y : low.y, mapped.z < tone.z ? middle.z : low.z);
        high = float3(mapped.x < tone.x ? high.x : middle.x,
            mapped.y < tone.y ? high.y : middle.y, mapped.z < tone.z ? high.z : middle.z);
    }
    const float3 bisected = .5f * (low + high);
    return float3(tone.x >= threshold.x ? rational.x : bisected.x,
        tone.y >= threshold.y ? rational.y : bisected.y,
        tone.z >= threshold.z ? rational.z : bisected.z);
}

// 현재 저작한 면 발광/투과/alpha의 표시색 기여분을 과거 tone 기준으로 맞춘다.
// clarity 이전 색을 목표로 쓰면 저작한 자체 발광이 다시 제거되므로 실제 합성 입력을 사용한다.
// 굴절 배경, alpha, RT2 소유권은 호출자에 남긴다.
// LUT가 비가역이므로 제한된 최근접 해이며, 같은 occurrence의 고정 snapshot 겹침까지 정확한 역합성은 아니다.
float3 CubeColor_Resolve(float3 currentMaterial, float3 background, float alpha)
{
    [branch] if (g_CubeColorResponseEnabled == 0 || g_iSourcePostProcessEnabled == 0 ||
        alpha <= 1.e-5f || g_fToneMapExposure <= 0.f ||
        !CubeColor_IsFinite(currentMaterial) || !CubeColor_IsFinite(background))
        return currentMaterial;

    background = clamp(background, 0.f, 60000.f);
    const float3 target = saturate(CubeColor_Grade(CubeColor_Tone(background)) +
        CubeColor_Historical(lerp(background, currentMaterial, saturate(alpha))) -
        CubeColor_Historical(background));
    const float3 fixedBackground = background * (1.f - saturate(alpha));
    const float3 lower = CubeColor_Tone(fixedBackground);
    const float3 upper = CubeColor_Tone(fixedBackground + alpha * 60000.f);
    float3 tone = clamp(CubeColor_Tone(lerp(background, currentMaterial, alpha)), lower, upper);
    float3 actual = CubeColor_Grade(tone);
    const float baselineError = CubeColor_Error(actual, target);
    float bestError = baselineError;
    const float3 secondSeed = clamp(target, lower, upper);
    const float3 secondActual = CubeColor_Grade(secondSeed);
    const float secondError = CubeColor_Error(secondActual, target);
    if (secondError < bestError)
    {
        tone = secondSeed;
        actual = secondActual;
        bestError = secondError;
    }

    // 최대8회 damped Gauss-Newton. LUT 미분/역행렬이 불안정한 셀에서도 범위와 기존 오차를 지킨다.
    [loop] for (uint iteration = 0u; iteration < 8u && bestError > 1.e-7f; ++iteration)
    {
        const float3 delta = float3(tone.x > .99f ? -.01f : .01f,
            tone.y > .99f ? -.01f : .01f, tone.z > .99f ? -.01f : .01f);
        const float3 j0 = (CubeColor_Grade(tone + float3(delta.x, 0.f, 0.f)) - actual) / delta.x;
        const float3 j1 = (CubeColor_Grade(tone + float3(0.f, delta.y, 0.f)) - actual) / delta.y;
        const float3 j2 = (CubeColor_Grade(tone + float3(0.f, 0.f, delta.z)) - actual) / delta.z;
        const float3 h0 = float3(CubeColor_Metric(j0, j0) + .0001f, CubeColor_Metric(j0, j1), CubeColor_Metric(j0, j2));
        const float3 h1 = float3(h0.y, CubeColor_Metric(j1, j1) + .0001f, CubeColor_Metric(j1, j2));
        const float3 h2 = float3(h0.z, h1.z, CubeColor_Metric(j2, j2) + .0001f);
        const float3 residual = target - actual;
        const float3 rhs = float3(CubeColor_Metric(j0, residual), CubeColor_Metric(j1, residual), CubeColor_Metric(j2, residual));
        const float determinant = dot(h0, cross(h1, h2));
        const float3 step = clamp(float3(dot(rhs, cross(h1, h2)), dot(h0, cross(rhs, h2)),
            dot(h0, cross(h1, rhs))) / max(determinant, 1.e-12f), -.25f, .25f);
        float3 nextTone = tone;
        float3 nextActual = actual;
        float nextError = bestError;
        [unroll] for (uint trial = 0u; trial < 3u; ++trial)
        {
            const float gain = trial == 0u ? 1.f : (trial == 1u ? .5f : .25f);
            const float3 candidate = clamp(tone + gain * step, lower, upper);
            const float3 candidateActual = CubeColor_Grade(candidate);
            const float candidateError = CubeColor_Error(candidateActual, target);
            if (candidateError < nextError)
            {
                nextTone = candidate;
                nextActual = candidateActual;
                nextError = candidateError;
            }
        }
        if (nextError >= bestError)
            break;
        tone = nextTone;
        actual = nextActual;
        bestError = nextError;
    }

    const float3 material = clamp((CubeColor_InverseTone(tone) - fixedBackground) / alpha, 0.f, 60000.f);
    const float3 resolved = CubeColor_Grade(CubeColor_Tone(lerp(background, material, alpha)));
    return CubeColor_IsFinite(material) && CubeColor_Error(resolved, target) <= baselineError ? material : currentMaterial;
}

#endif
```

#### C:/Users/tnest/Desktop/LostArk/Client/Public/BossLogicFlowView.h

SHA256: `bcdc97ee707625c20870d014c7247352f61574c4c83556a547ad811d8ebc1d46`

```cpp
#pragma once

#include "ActionCompositionGraphModel.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Client
{
	struct VALTAN_PATTERN_VIEW;

	// 원본 Pattern 그래프 노드에 clip 이름과 Counter 표시 진단을 덧붙인 읽기 전용 값이다.
	// 실제 gameplay 분기 정의와 실행은 이 표시 모델이 소유하지 않는다.
	struct BOSS_LOGIC_FLOW_NODE_VIEW final
	{
		std::size_t iGraphNodeIndex = ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
		std::string strStageId;
		std::string strActionId;
		std::string strSequenceRole;
		std::string strStageKind;
		std::vector<std::string> ClipNames;
		// 저작 조건의 사람이 읽는 요약이다. proxy/response/status/hit 필드를 표시할 뿐 이 문자열로 gameplay를 실행하지 않는다.
		std::vector<std::string> ConditionSummaries;
		bool bCounterWindow = false;
		bool bCounterProxy = false;
		bool bCounterContractIncomplete = false;
	};

	// Graph.Edges와 같은 순서의 표시 속성이다. authored COUNTER_HIT 및 닫힌 Counter의 TIMEOUT 색상 등을 결정한다.
	struct BOSS_LOGIC_FLOW_EDGE_VIEW final
	{
		std::size_t iGraphEdgeIndex = ACTION_COMPOSITION_GRAPH_INVALID_INDEX;
		bool bAuthored = false;
		bool bCounterSuccess = false;
		bool bCounterTimeout = false;
	};

	struct BOSS_LOGIC_FLOW_VIEW final
	{
		std::string strPatternId;
		std::string strPatternDisplayName;
		// 어느 Stage든 정확한 COUNTER_HIT 분기가 있으면 true다. Sequence UI badge용 사실이며 아래의 더 엄격한 노드 Counter 조합 진단과 구분한다.
		bool bHasCounterHitBranch = false;
		ACTION_COMPOSITION_GRAPH_SNAPSHOT Graph;
		std::vector<BOSS_LOGIC_FLOW_NODE_VIEW> Nodes;
		std::vector<BOSS_LOGIC_FLOW_EDGE_VIEW> Edges;
	};

	// 원본을 바꾸지 않는 값 변환이다. 임시 view를 완성해 마지막에 교체하며 명시적 false는 이전 InOutView를 보존한다.
	class CBossLogicFlowViewModel final
	{
	public:
		// 분기 이름 존재만 확인한다. counterable ENTER/EXIT 쌍이나 target의 유효성 검증은 Project와 원본 admission이 담당한다.
		[[nodiscard]] static bool Has_CounterHitBranch(
			const VALTAN_PATTERN_VIEW& Pattern) noexcept;
		static bool Project(
			const VALTAN_PATTERN_VIEW& Pattern,
			std::uint64_t iSourceGeneration,
			BOSS_LOGIC_FLOW_VIEW& InOutView,
			ACTION_COMPOSITION_GRAPH_ERROR& OutError);
	};

	enum class BOSS_LOGIC_FLOW_SELECTION_KIND : std::uint8_t
	{
		NONE,
		STAGE,
		BRANCH,
	};

	// 캔버스 선택을 소유자에게 stable 문자열로 반환한다. 화면 내부 포인터나 vector index를 선택의 영구 식별자로 넘기지 않는다.
	struct BOSS_LOGIC_FLOW_SELECTION final
	{
		BOSS_LOGIC_FLOW_SELECTION_KIND eKind =
			BOSS_LOGIC_FLOW_SELECTION_KIND::NONE;
		std::string strPatternId;
		std::string strStageId;
		std::string strActionId;
		std::string strOutcome;
		std::string strTargetActionId;
		std::string strTargetPatternId;
		bool bTerminal = false;
		bool bAuthored = false;

		void Clear();
	};

	// Client가 이미 복제받은 보스 cursor에서 뽑은 관찰 입력이다. 네트워크를 별도로 읽거나 두 번째 gameplay 실행 상태를 만들지 않는다.
	struct BOSS_LOGIC_FLOW_LIVE_SNAPSHOT final
	{
		bool bValid = false;
		std::uint32_t iServerTick = 0u;
		std::uint32_t iPatternSequence = 0u;
		std::string strPatternId;
		std::string strActionId;
	};

	// 연속된 두 authoritative cursor와 일치하는 유일한 그래프 간선으로 추론한 표시 기록이다.
	// Server가 보낸 outcome 영수증이 아니며 gameplay 입력이나 Pattern 선택 명령으로 사용하지 않는다.
	struct BOSS_LOGIC_FLOW_OBSERVED_EDGE final
	{
		std::uint64_t iObservationSequence = 0u;
		std::uint64_t iSourceGeneration = 0u;
		std::uint32_t iSourceServerTick = 0u;
		std::uint32_t iTargetServerTick = 0u;
		std::uint32_t iSourcePatternSequence = 0u;
		std::uint32_t iTargetPatternSequence = 0u;
		std::string strSourcePatternId;
		std::string strSourceStageId;
		std::string strSourceActionId;
		std::string strOutcome;
		std::string strTargetActionId;
		std::string strTargetPatternId;
		bool bTerminal = false;
		bool bCrossPattern = false;
	};

	// 이미 승인된 Client snapshot을 관찰한다. 입력이 무효하면 anchor를 비우고, tick 공백/모호한 전이는 기록 없이 현재 snapshot을 새 anchor로 삼는다.
	// 과거 tick은 버리고 최대 32개 기록만 소유한다. 관찰 실패가 기존 history까지 지우지는 않는다.
	class CBossLogicFlowObservedEdgeResolver final
	{
	public:
		static constexpr std::size_t MAX_HISTORY_COUNT = 32u;

		// ValtanBossTool이 HUD의 복제 cursor와 같은 source generation의 view를 전달한다. view는 값으로 복사하며 포인터를 보관하지 않는다.
		// 출력 포인터가 있으면 매 호출 처음 비운다. false도 anchor를 변경할 수 있으며 true만 history에 새 기록을 추가한다.
		bool Observe(
			const BOSS_LOGIC_FLOW_LIVE_SNAPSHOT& Snapshot,
			const BOSS_LOGIC_FLOW_VIEW* pCurrentView,
			BOSS_LOGIC_FLOW_OBSERVED_EDGE* pOutObserved = nullptr);
		// anchor, history와 관찰 번호를 함께 초기화한다. 일반 Observe 실패와 달리 이 명시적 호출은 과거 기록도 지운다.
		void Reset();
		// 반환 참조의 소유자는 이 resolver다. Observe/Reset 뒤에는 원소 포인터와 iterator를 계속 사용하지 않는다.
		[[nodiscard]] const std::vector<BOSS_LOGIC_FLOW_OBSERVED_EDGE>&
			Get_History() const noexcept
		{
			return m_History;
		}

	private:
		void Clear_Anchor();

		// 다음 전이를 대조할 anchor를 독립 값으로 소유한다. 내부 동기화는 없으므로 같은 인스턴스의 갱신/조회는 호출자가 직렬화한다.
		BOSS_LOGIC_FLOW_LIVE_SNAPSHOT m_PreviousSnapshot;
		BOSS_LOGIC_FLOW_VIEW m_PreviousView;
		std::vector<BOSS_LOGIC_FLOW_OBSERVED_EDGE> m_History;
		std::uint64_t m_iNextObservationSequence = 1u;
		bool m_bHasAnchor = false;
	};

	// UI가 소유하는 pan/zoom과 Fit 요청이다. Pattern/source generation 변경 시 Renderer가 다시 맞춤을 요청한다.
	struct BOSS_LOGIC_FLOW_CANVAS_STATE final
	{
		std::string strPatternId;
		std::uint64_t iSourceGeneration = ~std::uint64_t{ 0u };
		float fPanX = 0.f;
		float fPanY = 0.f;
		float fZoom = 1.f;
		bool bFitRequested = true;
	};

	// string_view와 pObservedEdge는 Render 호출 동안만 차용한다. 데이터 수명은 호출자가 유지한다.
	struct BOSS_LOGIC_FLOW_RENDER_CONTEXT final
	{
		std::string_view strSelectedStageId;
		std::string_view strLiveActionId;
		const BOSS_LOGIC_FLOW_OBSERVED_EDGE* pObservedEdge = nullptr;
		bool bLivePattern = false;
		bool bAllowSelection = false;
		float fMinimumCanvasHeight = 320.f;
	};

	// Valtan Boss Tool이 live identity를 전달하는 ImGui 읽기 전용 canvas다. 클릭 선택은 명시적으로 허용한 경우만 반환한다.
	// 네트워크나 gameplay를 직접 변경하지 않으며 true는 stable 선택 한 건을 출력했다는 뜻이다.
	class CBossLogicFlowRenderer final
	{
	public:
		static bool Render(
			const BOSS_LOGIC_FLOW_VIEW& View,
			const BOSS_LOGIC_FLOW_RENDER_CONTEXT& Context,
			BOSS_LOGIC_FLOW_CANVAS_STATE& InOutState,
			BOSS_LOGIC_FLOW_SELECTION& OutSelection);
	};
}
```

#### 저작 데이터의 정확한 교체 블록

각 stable element의 detail.color 전체 블록은 다음과 같다. 다른 요소에는 적용하지 않는다.

`C:/Users/tnest/Desktop/LostArk/Data/Effects/Authored/effect.dimensionmaster.skill.2050100.tuning.restore.effect.json`

- `authored.source-particle.b4984eeabefb0fc93783ecda`

```json
{
  "offset": [
    0,
    0,
    0,
    0
  ],
  "multiply": [
    1,
    1,
    1,
    1
  ],
  "clip": 0,
  "emissiveIntensity": 4,
  "distortionIntensity": 0,
  "distortionOnBaseMaterial": false,
  "radialTime": 0,
  "radialIntensity": 0
}
```
`C:/Users/tnest/Desktop/LostArk/Data/Effects/Authored/effect.dimensionmaster.skill.2050180.tuning.restore.effect.json`

- `authored.tuning.dimensionmaster.r.purple-glass-sequence`

```json
{
  "offset": [
    0,
    0,
    0,
    0
  ],
  "multiply": [
    1,
    1,
    1,
    1
  ],
  "clip": 0,
  "emissiveIntensity": 4,
  "distortionIntensity": 0,
  "distortionOnBaseMaterial": false,
  "radialTime": 0,
  "radialIntensity": 0
}
```

같은 요소의 material.sourceProfile.scalars 전체 배열:

```json
[
  {
    "name": "edge_str",
    "group": "None",
    "value": 2
  },
  {
    "name": "edge_line",
    "group": "None",
    "value": 30
  },
  {
    "name": "spec_str",
    "group": "None",
    "value": 1
  },
  {
    "name": "project_clarity_strength",
    "group": "Project tuning",
    "value": 1
  }
]
```
`C:/Users/tnest/Desktop/LostArk/Data/Effects/Authored/effect.dimensionmaster.skill.2050210.a1.full.restore.effect.json`

- `authored.composition.dimensionmaster.a.hit-1-purple-cube`
- `authored.composition.dimensionmaster.a.hit-2-purple-cube`
- `authored.composition.dimensionmaster.a.hit-3-purple-cube`
- `authored.composition.dimensionmaster.a.hit-4-purple-cube`

```json
{
  "offset": [
    0,
    0,
    0,
    0
  ],
  "multiply": [
    1,
    1,
    1,
    1
  ],
  "clip": 0,
  "emissiveIntensity": 4,
  "distortionIntensity": 0,
  "distortionOnBaseMaterial": false,
  "radialTime": 0,
  "radialIntensity": 0
}
```

같은 요소의 material.sourceProfile.scalars 전체 배열:

```json
[
  {
    "name": "edge_str",
    "group": "None",
    "value": 2
  },
  {
    "name": "edge_line",
    "group": "None",
    "value": 30
  },
  {
    "name": "spec_str",
    "group": "None",
    "value": 1
  },
  {
    "name": "project_clarity_strength",
    "group": "Project tuning",
    "value": 1
  }
]
```

### G17 반영 코드 전문

이 전문은 아래 기록 시점의 실제 파일을 기계적으로 동기화한 코드 정본이다. 문서 사본의 행 끝 공백만 정규화하며 원본 C++은 변경하지 않는다. 기존 무관한 변경도 현재 파일과 대조할 수 있도록 보존하며, 이 G의 변경 범위는 위 설명과 실제 diff로 구분한다. SHA256은 원본 파일의 byte 기준이다.

#### C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_EffectCubeColorResponse.hlsli

CubeColor_Resolve, 역사 기준과 현재 tone/LUT의 국소 색 기여 비교 및 제한된 solver.

SHA256: `792a20762fd3bdad84ca66e1d5ff6aeb110ea0355b5b01262adf49aa33743ee2`

```hlsl
#ifndef EFFECT_CUBE_COLOR_RESPONSE_HLSLI
#define EFFECT_CUBE_COLOR_RESPONSE_HLSLI

// Renderer 소유의 현재 변환을 읽기만 한다. Client가 exact Q/R/A draw마다 선택을 갱신한다.
int g_CubeColorResponseEnabled = 0;
int g_iSourcePostProcessEnabled = 0;
float4 g_vSourceToneCurve = float4(.22f, 1.22f, .298f, 1.f);
float g_fSourceToneToe = 1.f;
float g_fToneMapExposure = 1.f;
Texture2D g_SourceGradingLut;

bool CubeColor_IsFinite(float3 value)
{
    // FXC의 fast-math가 isfinite를 제거해도 HDR 경계의 NaN/INF 방어는 남긴다.
    return all((asuint(value) & 0x7f800000u) != 0x7f800000u);
}

float3 CubeColor_Tone(float3 hdr)
{
    // Final의 Sanitize_HDR와 같은 노출 전후 범위다.
    const float3 color = clamp(clamp(hdr, 0.f, 60000.f) * g_fToneMapExposure, 0.f, 60000.f);
    const float3 rational = color / abs(color + g_vSourceToneCurve.x) * g_vSourceToneCurve.y;
    const float3 linearGamma = pow(max(color * g_vSourceToneCurve.w, 0.f), 1.f / 2.2f);
    const float3 shoulder = saturate((color - g_vSourceToneCurve.z) * 10000.f);
    return saturate(lerp(lerp(linearGamma, rational, shoulder), rational, saturate(g_fSourceToneToe)));
}

float3 CubeColor_Grade(float3 color)
{
    color = saturate(color);
    const float slice = floor(color.b * 14.9999f);
    const float2 uv = float2(slice / 16.f + color.r * (15.f / 256.f) + .5f / 256.f,
        color.g * (15.f / 16.f) + .5f / 16.f);
    const float3 low = g_SourceGradingLut.SampleLevel(LinearClampUVSampler, uv, 0.f).rgb;
    const float3 high = g_SourceGradingLut.SampleLevel(LinearClampUVSampler, uv + float2(1.f / 16.f, 0.f), 0.f).rgb;
    return lerp(low, high, color.b * 15.f - slice);
}

float3 CubeColor_Hable(float3 color)
{
    return ((color * (.15f * color + .05f) + .004f) /
        (color * (.15f * color + .50f) + .06f)) - .02f / .30f;
}

// 09-08 Q2 이후 첫 보존본의 Hable 설정이다. 현재 장면 옵션을 과거 값으로 되돌리지 않는다.
float3 CubeColor_Historical(float3 hdr)
{
    return pow(saturate(CubeColor_Hable(clamp(hdr, 0.f, 60000.f) * .6351f) /
        CubeColor_Hable(1.f.xxx)), 1.f / 1.905f);
}

float CubeColor_Metric(float3 a, float3 b)
{
    const float3 luma = float3(.299f, .587f, .114f);
    return dot(a, b) + 16.f * dot(a, luma) * dot(b, luma);
}

float CubeColor_Error(float3 a, float3 b)
{
    return CubeColor_Metric(a - b, a - b);
}

// shoulder 위는 유리함수의 역식으로 즉시 복원한다. toe 혼합 구간만 작은 HDR 구간에서 찾는다.
float3 CubeColor_InverseTone(float3 tone)
{
    const float exposure = max(g_fToneMapExposure, 1.e-6f);
    const float boundary = max(g_vSourceToneCurve.z + .0001f, 0.f) / exposure;
    const float3 threshold = CubeColor_Tone(boundary.xxx);
    const float3 rational = tone * g_vSourceToneCurve.x /
        max(g_vSourceToneCurve.y - tone, 1.e-6f) / exposure;
    if (all(tone >= threshold))
        return min(rational, 60000.f);
    float3 low = 0.f;
    float3 high = min(boundary, 60000.f).xxx;
    [loop] for (uint iteration = 0u; iteration < 20u; ++iteration)
    {
        const float3 middle = .5f * (low + high);
        const float3 mapped = CubeColor_Tone(middle);
        low = float3(mapped.x < tone.x ? middle.x : low.x,
            mapped.y < tone.y ? middle.y : low.y, mapped.z < tone.z ? middle.z : low.z);
        high = float3(mapped.x < tone.x ? high.x : middle.x,
            mapped.y < tone.y ? high.y : middle.y, mapped.z < tone.z ? high.z : middle.z);
    }
    const float3 bisected = .5f * (low + high);
    return float3(tone.x >= threshold.x ? rational.x : bisected.x,
        tone.y >= threshold.y ? rational.y : bisected.y,
        tone.z >= threshold.z ? rational.z : bisected.z);
}

// alpha-over 뒤의 표시색 기여분만 과거 기준으로 맞춘다. 굴절 배경, alpha, RT2 소유권은 호출자에 남긴다.
// LUT가 비가역이므로 제한된 최근접 해이며, 같은 occurrence의 고정 snapshot 겹침까지 정확한 역합성은 아니다.
float3 CubeColor_Resolve(float3 currentMaterial, float3 nativeMaterial, float3 background,
    float alpha, float nativeAlpha)
{
    [branch] if (g_CubeColorResponseEnabled == 0 || g_iSourcePostProcessEnabled == 0 ||
        alpha <= 1.e-5f || g_fToneMapExposure <= 0.f ||
        !CubeColor_IsFinite(currentMaterial) || !CubeColor_IsFinite(nativeMaterial) || !CubeColor_IsFinite(background))
        return currentMaterial;

    background = clamp(background, 0.f, 60000.f);
    const float3 target = saturate(CubeColor_Grade(CubeColor_Tone(background)) +
        CubeColor_Historical(lerp(background, nativeMaterial, saturate(nativeAlpha))) -
        CubeColor_Historical(background));
    const float3 fixedBackground = background * (1.f - saturate(alpha));
    const float3 lower = CubeColor_Tone(fixedBackground);
    const float3 upper = CubeColor_Tone(fixedBackground + alpha * 60000.f);
    float3 tone = clamp(CubeColor_Tone(lerp(background, currentMaterial, alpha)), lower, upper);
    float3 actual = CubeColor_Grade(tone);
    const float baselineError = CubeColor_Error(actual, target);
    float bestError = baselineError;
    const float3 secondSeed = clamp(target, lower, upper);
    const float3 secondActual = CubeColor_Grade(secondSeed);
    const float secondError = CubeColor_Error(secondActual, target);
    if (secondError < bestError)
    {
        tone = secondSeed;
        actual = secondActual;
        bestError = secondError;
    }

    // 최대8회 damped Gauss-Newton. LUT 미분/역행렬이 불안정한 셀에서도 범위와 기존 오차를 지킨다.
    [loop] for (uint iteration = 0u; iteration < 8u && bestError > 1.e-7f; ++iteration)
    {
        const float3 delta = float3(tone.x > .99f ? -.01f : .01f,
            tone.y > .99f ? -.01f : .01f, tone.z > .99f ? -.01f : .01f);
        const float3 j0 = (CubeColor_Grade(tone + float3(delta.x, 0.f, 0.f)) - actual) / delta.x;
        const float3 j1 = (CubeColor_Grade(tone + float3(0.f, delta.y, 0.f)) - actual) / delta.y;
        const float3 j2 = (CubeColor_Grade(tone + float3(0.f, 0.f, delta.z)) - actual) / delta.z;
        const float3 h0 = float3(CubeColor_Metric(j0, j0) + .0001f, CubeColor_Metric(j0, j1), CubeColor_Metric(j0, j2));
        const float3 h1 = float3(h0.y, CubeColor_Metric(j1, j1) + .0001f, CubeColor_Metric(j1, j2));
        const float3 h2 = float3(h0.z, h1.z, CubeColor_Metric(j2, j2) + .0001f);
        const float3 residual = target - actual;
        const float3 rhs = float3(CubeColor_Metric(j0, residual), CubeColor_Metric(j1, residual), CubeColor_Metric(j2, residual));
        const float determinant = dot(h0, cross(h1, h2));
        const float3 step = clamp(float3(dot(rhs, cross(h1, h2)), dot(h0, cross(rhs, h2)),
            dot(h0, cross(h1, rhs))) / max(determinant, 1.e-12f), -.25f, .25f);
        float3 nextTone = tone;
        float3 nextActual = actual;
        float nextError = bestError;
        [unroll] for (uint trial = 0u; trial < 3u; ++trial)
        {
            const float gain = trial == 0u ? 1.f : (trial == 1u ? .5f : .25f);
            const float3 candidate = clamp(tone + gain * step, lower, upper);
            const float3 candidateActual = CubeColor_Grade(candidate);
            const float candidateError = CubeColor_Error(candidateActual, target);
            if (candidateError < nextError)
            {
                nextTone = candidate;
                nextActual = candidateActual;
                nextError = candidateError;
            }
        }
        if (nextError >= bestError)
            break;
        tone = nextTone;
        actual = nextActual;
        bestError = nextError;
    }

    const float3 material = clamp((CubeColor_InverseTone(tone) - fixedBackground) / alpha, 0.f, 60000.f);
    const float3 resolved = CubeColor_Grade(CubeColor_Tone(lerp(background, material, alpha)));
    return CubeColor_IsFinite(material) && CubeColor_Error(resolved, target) <= baselineError ? material : currentMaterial;
}

#endif
```

#### C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_EffectCubeSampleScene.hlsli

Shade_EffectCubeSampleScene, native RGB 보관과 RT2 계산 뒤 RT0 RGB 보정 호출.

SHA256: `35f9a932a39bbbda9f0b95fd6d8049cd0f56c91131ce88624821919f920529c5`

```hlsl
#ifndef EFFECT_CUBESAMPLE_SCENE_HLSLI
#define EFFECT_CUBESAMPLE_SCENE_HLSLI
#include "Shader_EffectSceneColorInput.hlsli"
#include "Shader_EffectCubeColorResponse.hlsli"

// 실제 CubeSample 분기가 쓴 RT2만 carrier가 보존한다. 픽셀 호출 사이에는 공유되지 않는다.
static bool g_CubeSampleBloomWritten = false;

float CubeSample_SourcePow(float value, float exponent)
{
    // Native log/exp branch returns zero even for the near-zero ^0 case.
    return abs(value) < 1.0e-6f ? 0.f : pow(abs(value), exponent);
}

// Limit only transmitted background radiance. Authored self-emission and
// the original caustic/edge terms retain their HDR range.
float CubeSample_ClarityTransmissionScale(float3 radiance, float ceiling)
{
    const float peak = max(radiance.r, max(radiance.g, radiance.b));
    const float excess = max(peak - 0.35f, 0.f);
    const float shoulder = ceiling - 0.35f;
    const float compressed = 0.35f + shoulder * (excess / (excess + shoulder));
    return peak > 0.35f ? compressed / peak : 1.f;
}

EFFECT_PS_OUT Shade_EffectCubeSampleScene(float2 meshUV,
    float2 pixelPosition, float3 tangentView, float4 particleColor)
{
    EFFECT_PS_OUT output = (EFFECT_PS_OUT)0;
    uint width = 0u;
    uint height = 0u;
    g_EffectSceneColorTexture.GetDimensions(width, height);
    if (g_SourceTextureMask != 1u || width == 0u || height == 0u ||
        !all(isfinite(tangentView)) || dot(tangentView, tangentView) < 1.0e-12f)
    {
        clip(-1.f);
        return output;
    }

    // PS f548f39885bbfe42ac405ccd46dc2919: selected unclamped CubeSample.
    // SceneHDR snapshot and particleColor replace engine-owned scene/prefix
    // inputs. Native fog and auxiliary MRT2-5 are outside this RT0 adapter.
    const float3 V = normalize(tangentView);
    const float facing = abs(V.z);
    const float grazing = 1.f - facing;
    const float2 sceneUV = pixelPosition / float2(width, height) +
        0.1f * g_SourceVector1.x * grazing * V.yx;
    const float3 scene = Read_EffectSceneColor(
        LinearClampUVSampler, sceneUV).rgb;
    const float3 powerBase = max(abs(scene), 1.0e-6f);
    const float3 power2 = powerBase * powerBase;
    const float3 power5 = power2 * power2 * powerBase;
    const float3 refracted = lerp(5.f * power5, scene, facing);
    // 가중된 Bloom을 C^5에 대입하지 않고, 현재 HDR 배경에서 정한 RGB 전달 계수를 쓴다.
    float3 bloomTransmission = (5.f * power2 * power2 * grazing + facing) *
        g_SourceVector0.rgb * g_SourceVector0.a * g_EmissiveIntensity;
    const float3 caustic = Sample_SourceTexture0(V.xy * 3.f).rgb *
        CubeSample_SourcePow(grazing, 7.f) * g_SourceScalars0.x;
    const float2 edgeUV = 2.f * (meshUV - 0.5f);
    const float edge = min(1.f,
        CubeSample_SourcePow(edgeUV.x, g_SourceScalars0.y) +
        CubeSample_SourcePow(edgeUV.y, g_SourceScalars0.y)) * g_SourceScalars0.z;

    // Mesh particle RGB owns the edge term; multiplying all body RGB by it
    // would apply the source color-over-life gain to the wrong expression.
    output.SceneColor.rgb = ((refracted + caustic) *
        g_SourceVector0.rgb * g_SourceVector0.a + edge * particleColor.rgb) *
        g_EmissiveIntensity;
    const float sourceAlpha = saturate(particleColor.a);
    const float3 nativeColor = output.SceneColor.rgb;
    output.SceneColor.a = sourceAlpha;
    if (g_SourceScalars0.w > 0.f)
    {
        // 보정된 색과 Bloom 전달은 같은 현재 HDR 배경에서 정한 계수를 소비한다.
        const float strength = saturate(g_SourceScalars0.w);
        const float3 actualScene = min(max(g_EffectSceneColorTexture.Sample(
            LinearClampUVSampler, sceneUV).rgb, 0.f), 60000.f);
        const float scenePeak = max(actualScene.r, max(actualScene.g, actualScene.b));
        const float sceneWeight = smoothstep(0.15f, 0.8f, scenePeak);
        const float3 actualSquared = actualScene * actualScene;
        const float3 nativeTransmission = (5.f * actualSquared * actualSquared *
            grazing + facing) * g_SourceVector0.rgb *
            g_SourceVector0.a * g_EmissiveIntensity;
        const float bodyScale = CubeSample_ClarityTransmissionScale(
            actualScene * nativeTransmission, 0.65f + 0.45f * facing);
        // Source-tone compression can wash a pale HDR tint to white. Use the
        // authored particle chroma for absorption as well as self-emission.
        const float3 particleRadiance = max(particleColor.rgb, 0.f);
        const float particlePeak = max(particleRadiance.r,
            max(particleRadiance.g, particleRadiance.b));
        const float3 particleChroma = particleRadiance / max(particlePeak, 1.e-4f);
        const float3 transmission = nativeTransmission * particleChroma *
            ((1.f - sceneWeight) + sceneWeight * bodyScale);
        bloomTransmission = (1.f - strength) * bloomTransmission +
            strength * transmission;
        const float3 nativeCaustic = caustic * g_SourceVector0.rgb *
            g_SourceVector0.a * g_EmissiveIntensity;
        // The source HDR particle color supplies the gold hue. This authored
        // face term is independent of SceneColor, so it remains in F(black)
        // and participates in the existing per-effect bloom extraction.
        const float faceEmissionWeight = 0.0875f + 0.0525f * grazing * grazing;
        const float3 faceEmission = particleRadiance * particleChroma * g_SourceVector0.rgb *
            g_SourceVector0.a * g_EmissiveIntensity * faceEmissionWeight;
        const float3 clearColor = scene * transmission + nativeCaustic +
            edge * particleColor.rgb * g_EmissiveIntensity + faceEmission;
        // strength1에서 큰 C^5 항끼리의 뺄셈 상쇄 없이 보정 색을 선택한다.
        output.SceneColor.rgb = (1.f - strength) * output.SceneColor.rgb +
            strength * clearColor;
        // Reduce unfiltered background leakage only on bright scenes. Preserve
        // fade endpoints and the original alpha-test threshold below.
        output.SceneColor.a += strength * smoothstep(0.5f, 4.f, scenePeak) *
            sourceAlpha * (1.f - sourceAlpha);
    }
    output.Distortion = float4(0.f, 0.f, 0.f, 0.f);
    if (g_ColorClip > 0.f)
        clip(sourceAlpha - g_ColorClip);
    // 기존 배경 Bloom은 한 번 전달하고, 재질 증폭으로 새로 생긴 bright-pass만
    // 현재 문서 배율을 받는다. 항등 전달에서는 억제된 배경 Bloom이 되살아나지 않는다.
    const float3 sceneBloom = g_EffectSceneBloomTexture.Sample(
        LinearClampUVSampler, sceneUV).rgb;
    const float3 newBloom = max(Extract_SceneBloom(output.SceneColor.rgb) -
        bloomTransmission * Extract_SceneBloom(scene), 0.f);
    const float bloomIntensity = clamp(g_fEffectBloomIntensity >= 0.f ?
        g_fEffectBloomIntensity : g_fSceneBloomIntensity, 0.f, 16.f);
    output.BloomContribution = float4(min(max(bloomTransmission * sceneBloom,
        0.f) + newBloom * bloomIntensity, 60000.f), output.SceneColor.a);
    g_CubeSampleBloomWritten = true;
    // Bloom은 기존 HDR 에너지로 이미 계산했다. 색 응답 보정은 RT0 RGB에만 적용한다.
    [branch] if (g_CubeColorResponseEnabled != 0)
        output.SceneColor.rgb = CubeColor_Resolve(output.SceneColor.rgb, nativeColor,
            g_EffectSceneColorTexture.SampleLevel(LinearClampUVSampler,
                pixelPosition / float2(width, height), 0.f).rgb,
            output.SceneColor.a, sourceAlpha);
    return output;
}

#endif
```

#### C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_MaterialBinding.cpp

Bind_MaterialInputs, 실제 문서/요소/profile/blend 조건에 따른 매 draw 선택과 바인딩 실패 전파.

SHA256: `2d4a210bf51bbe0bebb99281e946c2df0097778debacb27c4d04fd83eda45d1f`

```cpp
#include "Effect_DocumentRenderer_Internal.h"
#include "Effect_ArtistMaterial.h"
#include "GameInstance.h"
#include "Presentation_Manager.h"
#include "Profiler.h"
#include "Shader.h"
#include "VIBuffer_DynamicTrail.h"
#include "VIBuffer_ParticleRect.h"
#include <d3d11sdklayers.h>
#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <optional>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Model.h"
#include "Engine_RenderTypes.h"
#include "VIBuffer_Rect.h"

const Client::CEffectDocumentRenderer::ELEMENT_RESOURCE*
Client::CEffectDocumentRenderer::Find_Resource(
	const std::string& strElementId) const
{
	if (nullptr == m_pPreparedDocument)
		return nullptr;
	const auto Iterator =
		m_pPreparedDocument->ElementResources.find(strElementId);
	return Iterator == m_pPreparedDocument->ElementResources.end() ?
		nullptr : &Iterator->second;
}

uint32_t Client::CEffectDocumentRenderer::Select_Pass(
	const EFFECT_RENDER_PROFILE eProfile) const
{
	switch (eProfile)
	{
	case EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE: return 0u;
	case EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ: return 1u;
	case EFFECT_RENDER_PROFILE::ADDITIVE_TWO_SIDED_DEPTH_READ: return 2u;
	case EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ: return 3u;
	case EFFECT_RENDER_PROFILE::ADDITIVE_ONE_SIDED_DEPTH_READ: return 4u;
	case EFFECT_RENDER_PROFILE::MULTIPLY_ONE_SIDED_DEPTH_READ: return 5u;
	case EFFECT_RENDER_PROFILE::END:
	default: return UINT32_MAX;
	}
}

HRESULT Client::CEffectDocumentRenderer::Fail_RenderOperation(
	std::string strOperation,
	const HRESULT hResult,
	const bool_t bObjectLocal)
{
	if (m_strRenderFailureDetail.empty())
		m_strRenderFailureDetail = std::move(strOperation);
	m_bLastRenderFailureObjectLocal =
		m_bLastRenderFailureObjectLocal || bObjectLocal;
	return FAILED(hResult) ? hResult : E_FAIL;
}

#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
void Client::CEffectDocumentRenderer::Record_TestMaterialBinding()
{
	if (nullptr == m_pActiveOccurrenceStats)
		return;
	++m_pActiveOccurrenceStats->iMaterialBindCount;
	/* Bind_MaterialInputs commits five standard SRVs and seven source SRVs.
	   Missing optional lanes are deliberately bound to typed white/black
	   resources, so every successful call has the same complete shader seam. */
	m_pActiveOccurrenceStats->iTextureSrvBindCount += 12u;
}

void Client::CEffectDocumentRenderer::Record_TestSamplerBinding()
{
	if (nullptr != m_pActiveOccurrenceStats)
		++m_pActiveOccurrenceStats->iSamplerBindCount;
}

void Client::CEffectDocumentRenderer::Record_TestShaderPassApplication()
{
	if (nullptr != m_pActiveOccurrenceStats)
	{
		++m_pActiveOccurrenceStats->iShaderPassApplyCount;
		/* CShader::Begin applies the selected Effect pass, including its compiled
		   LinearSampler state. RuntimeMaterialV2 may then replace that PS sampler;
		   its successful override is recorded independently below. */
		++m_pActiveOccurrenceStats->iSamplerBindCount;
	}
}

void Client::CEffectDocumentRenderer::Record_TestGeometryUpload()
{
	if (nullptr != m_pActiveOccurrenceStats)
		++m_pActiveOccurrenceStats->iGeometryUploadCount;
}

void Client::CEffectDocumentRenderer::Record_TestTrailGeometryUpload(
	const std::span<const Engine::VTXEFFECT_TRAIL> Vertices)
{
	if (nullptr == m_pActiveOccurrenceStats)
		return;
	++m_pActiveOccurrenceStats->iGeometryUploadCount;
	m_pActiveOccurrenceStats->iFinalTrailUploadedVertexCount += Vertices.size();
	for (size_t iVertex = 0u; iVertex + 1u < Vertices.size(); iVertex += 2u)
	{
		const float3_t& Left = Vertices[iVertex].vPosition;
		const float3_t& Right = Vertices[iVertex + 1u].vPosition;
		const float3_t Center = {
			(Left.x + Right.x) * 0.5f,
			(Left.y + Right.y) * 0.5f,
			(Left.z + Right.z) * 0.5f };
		const f32_t DeltaX = Right.x - Left.x;
		const f32_t DeltaY = Right.y - Left.y;
		const f32_t DeltaZ = Right.z - Left.z;
		const f32_t Width = std::sqrt(
			DeltaX * DeltaX + DeltaY * DeltaY + DeltaZ * DeltaZ);
		if (!m_pActiveOccurrenceStats->bHasFinalTrailPairCenter)
		{
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMin = Center;
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMax = Center;
			m_pActiveOccurrenceStats->fFinalTrailPairWidthMin = Width;
			m_pActiveOccurrenceStats->fFinalTrailPairWidthMax = Width;
			m_pActiveOccurrenceStats->bHasFinalTrailPairCenter = true;
			continue;
		}
		m_pActiveOccurrenceStats->vFinalTrailPairCenterMin.x = (std::min)(
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMin.x, Center.x);
		m_pActiveOccurrenceStats->vFinalTrailPairCenterMin.y = (std::min)(
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMin.y, Center.y);
		m_pActiveOccurrenceStats->vFinalTrailPairCenterMin.z = (std::min)(
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMin.z, Center.z);
		m_pActiveOccurrenceStats->vFinalTrailPairCenterMax.x = (std::max)(
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMax.x, Center.x);
		m_pActiveOccurrenceStats->vFinalTrailPairCenterMax.y = (std::max)(
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMax.y, Center.y);
		m_pActiveOccurrenceStats->vFinalTrailPairCenterMax.z = (std::max)(
			m_pActiveOccurrenceStats->vFinalTrailPairCenterMax.z, Center.z);
		m_pActiveOccurrenceStats->fFinalTrailPairWidthMin = (std::min)(
			m_pActiveOccurrenceStats->fFinalTrailPairWidthMin, Width);
		m_pActiveOccurrenceStats->fFinalTrailPairWidthMax = (std::max)(
			m_pActiveOccurrenceStats->fFinalTrailPairWidthMax, Width);
	}
}

void Client::CEffectDocumentRenderer::Record_TestVIBufferBinding()
{
	if (nullptr != m_pActiveOccurrenceStats)
		++m_pActiveOccurrenceStats->iVIBufferBindCount;
}

void Client::CEffectDocumentRenderer::Record_TestDrawSelection(
	const EFFECT_GPU_RENDER_CARRIER eCarrier,
	const uint32_t iSelectedPassIndex)
{
	if (nullptr == m_pActiveOccurrenceStats)
		return;
	if (0u == m_pActiveOccurrenceStats->iDrawSelectionCount)
	{
		m_pActiveOccurrenceStats->eCarrier = eCarrier;
		m_pActiveOccurrenceStats->iSelectedPassIndex = iSelectedPassIndex;
	}
	else if (m_pActiveOccurrenceStats->eCarrier != eCarrier ||
		m_pActiveOccurrenceStats->iSelectedPassIndex != iSelectedPassIndex)
	{
		m_pActiveOccurrenceStats->bDrawSelectionDiverged = true;
	}
	if (EFFECT_GPU_RENDER_CARRIER::END == eCarrier ||
		UINT32_MAX == iSelectedPassIndex)
	{
		m_pActiveOccurrenceStats->bDrawSelectionDiverged = true;
	}
	++m_pActiveOccurrenceStats->iDrawSelectionCount;
}

void Client::CEffectDocumentRenderer::
	Record_TestCompiledAdapterPipelineValidation()
{
	if (nullptr != m_pActiveOccurrenceStats)
		++m_pActiveOccurrenceStats->iCompiledAdapterPipelineValidationCount;
}

namespace
{
	void Extend_SubmittedPosition(
		Client::EFFECT_GPU_RENDER_OCCURRENCE_STATS& Stats,
		const float3_t& Position)
	{
		if (!Stats.bHasSubmittedPosition)
		{
			Stats.vSubmittedPositionMin = Position;
			Stats.vSubmittedPositionMax = Position;
			Stats.bHasSubmittedPosition = true;
			return;
		}
		Stats.vSubmittedPositionMin.x =
			(std::min)(Stats.vSubmittedPositionMin.x, Position.x);
		Stats.vSubmittedPositionMin.y =
			(std::min)(Stats.vSubmittedPositionMin.y, Position.y);
		Stats.vSubmittedPositionMin.z =
			(std::min)(Stats.vSubmittedPositionMin.z, Position.z);
		Stats.vSubmittedPositionMax.x =
			(std::max)(Stats.vSubmittedPositionMax.x, Position.x);
		Stats.vSubmittedPositionMax.y =
			(std::max)(Stats.vSubmittedPositionMax.y, Position.y);
		Stats.vSubmittedPositionMax.z =
			(std::max)(Stats.vSubmittedPositionMax.z, Position.z);
	}
}

void Client::CEffectDocumentRenderer::Record_TestIssuedDraw(
	const float4x4_t& World)
{
	if (nullptr == m_pActiveOccurrenceStats)
		return;
	if (UINT64_MAX == m_pActiveOccurrenceStats->iFirstIssuedDrawOrdinal)
		m_pActiveOccurrenceStats->iFirstIssuedDrawOrdinal =
			m_iTestIssuedDrawOrdinal;
	++m_iTestIssuedDrawOrdinal;
	++m_pActiveOccurrenceStats->iVIBufferDrawCount;
	++m_pActiveOccurrenceStats->iIssuedDrawCallCount;
	if (!m_pActiveOccurrenceStats->bHasFirstSubmittedParticleWorld)
	{
		m_pActiveOccurrenceStats->FirstSubmittedParticleWorld = World;
		m_pActiveOccurrenceStats->bHasFirstSubmittedParticleWorld = true;
	}
	Extend_SubmittedPosition(*m_pActiveOccurrenceStats,
		{ World._41, World._42, World._43 });
}

void Client::CEffectDocumentRenderer::Record_TestIssuedDraw(
	const std::span<const Engine::VTXEFFECT_PARTICLE> Instances)
{
	if (nullptr == m_pActiveOccurrenceStats)
		return;
	if (UINT64_MAX == m_pActiveOccurrenceStats->iFirstIssuedDrawOrdinal)
		m_pActiveOccurrenceStats->iFirstIssuedDrawOrdinal =
			m_iTestIssuedDrawOrdinal;
	++m_iTestIssuedDrawOrdinal;
	++m_pActiveOccurrenceStats->iVIBufferDrawCount;
	++m_pActiveOccurrenceStats->iIssuedDrawCallCount;
	for (const Engine::VTXEFFECT_PARTICLE& Instance : Instances)
	{
		if (!m_pActiveOccurrenceStats->bHasFirstSubmittedParticleWorld)
		{
			m_pActiveOccurrenceStats->FirstSubmittedParticleWorld = Instance.World;
			m_pActiveOccurrenceStats->bHasFirstSubmittedParticleWorld = true;
		}
		Extend_SubmittedPosition(*m_pActiveOccurrenceStats,
			{ Instance.World._41, Instance.World._42, Instance.World._43 });
	}
}

void Client::CEffectDocumentRenderer::Record_TestIssuedDraw(
	const std::span<const Engine::VTXEFFECT_TRAIL> Vertices)
{
	if (nullptr == m_pActiveOccurrenceStats)
		return;
	if (UINT64_MAX == m_pActiveOccurrenceStats->iFirstIssuedDrawOrdinal)
		m_pActiveOccurrenceStats->iFirstIssuedDrawOrdinal =
			m_iTestIssuedDrawOrdinal;
	++m_iTestIssuedDrawOrdinal;
	++m_pActiveOccurrenceStats->iVIBufferDrawCount;
	++m_pActiveOccurrenceStats->iIssuedDrawCallCount;
	for (const Engine::VTXEFFECT_TRAIL& Vertex : Vertices)
		Extend_SubmittedPosition(*m_pActiveOccurrenceStats, Vertex.vPosition);
}
#endif

HRESULT Client::CEffectDocumentRenderer::Bind_Common(
	const shared_ptr<Engine::CShader>& pShader,
	const EFFECT_EVALUATED_ELEMENT& Element,
	const ELEMENT_RESOURCE& Resource,
	const f32_t fAlphaScale,
	const EFFECT_MATERIAL_DESC* pMaterialOverride,
    const EFFECT_SHADER_PROGRAM_DESC* pShaderProgram)
{
	return Bind_Common(pShader, *Element.pElement, Element.Color,
		Element.fLocalTimeSeconds, Element.fNormalizedLife,
		Resource, fAlphaScale, pMaterialOverride, pShaderProgram);
}

HRESULT Client::CEffectDocumentRenderer::Bind_Common(
	const shared_ptr<Engine::CShader>& pShader,
	const EFFECT_ELEMENT_DESC& Element,
	const EFFECT_COLOR_DESC& Color,
	const f32_t fLocalTimeSeconds,
	const f32_t fNormalizedLife,
	const ELEMENT_RESOURCE& Resource,
	const f32_t fAlphaScale,
	const EFFECT_MATERIAL_DESC* pMaterialOverride,
    const EFFECT_SHADER_PROGRAM_DESC* pShaderProgram)
{
	if (nullptr == pShader)
		return Fail_RenderOperation(
			"Common shader bind failed: shader is null.", E_POINTER);
	HRESULT hResult = pShader->Bind_Matrix("g_ViewMatrix",
		CGameInstance::Get().Get_Transform(D3DTS::VIEW));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Common shader bind failed: g_ViewMatrix.", hResult);
	hResult = pShader->Bind_Matrix("g_ProjMatrix",
		CGameInstance::Get().Get_Transform(D3DTS::PROJ));
	if (FAILED(hResult))
		return Fail_RenderOperation(
			"Common shader bind failed: g_ProjMatrix.", hResult);
	if ((nullptr != pShaderProgram && pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::PARTICLE))
	{
		const float4_t CameraPosition = *CGameInstance::Get().Get_CamPosition();
		hResult = pShader->Bind_RawValue("g_CameraPosition",
			&CameraPosition, sizeof(CameraPosition));
		if (FAILED(hResult))
			return Fail_RenderOperation(
				"Particle camera-position shader binding failed.", hResult);
	}
	return Bind_MaterialInputs(pShader, Element, Color,
		fLocalTimeSeconds, fNormalizedLife, Resource, fAlphaScale, pMaterialOverride, pShaderProgram);
}

HRESULT Client::CEffectDocumentRenderer::Bind_MaterialInputs(
	const shared_ptr<Engine::CShader>& pShader,
	const EFFECT_ELEMENT_DESC& Element,
	const EFFECT_COLOR_DESC& Color,
	const f32_t fLocalTimeSeconds,
	const f32_t fNormalizedLife,
	const ELEMENT_RESOURCE& Resource,
	const f32_t fAlphaScale,
	const EFFECT_MATERIAL_DESC* pMaterialOverride,
    const EFFECT_SHADER_PROGRAM_DESC* pShaderProgram)
{
	Engine::CProfilerScope cpuPhaseScope(CGameInstance::Get().Get_Profiler(), "Effect.Material.Bind");
	if (FAILED(Bind_BloomInputs(pShader))) return E_FAIL;
	const EFFECT_MATERIAL_DESC& Material = nullptr != pMaterialOverride ?
		*pMaterialOverride : Element.Material;
	if (nullptr == pShader)
		return Fail_RenderOperation(
			"Material bind failed: shader is null.", E_INVALIDARG);
	HRESULT hFirstBindFailure = S_OK;
	const auto BindFailed = [&hFirstBindFailure](const HRESULT hResult)
	{
		if (SUCCEEDED(hResult))
			return false;
		if (SUCCEEDED(hFirstBindFailure))
			hFirstBindFailure = hResult;
		return true;
	};
    std::array<float4_t, 32u> animatedArtistParameters;
    const auto* artistParameters = &Resource.ArtistSourceMaterialParameters;
    if (!Resource.ArtistMaterialTrackBindings.empty())
    {
        if (!Element.SourceTransformTrack)
            return Fail_RenderOperation("Native material parameter track is unavailable.", E_INVALIDARG);
        animatedArtistParameters = Resource.ArtistSourceMaterialParameters;
        if (!Apply_ArtistMaterialTrackSamples(*Element.SourceTransformTrack, Resource.ArtistMaterialTrackBindings,
            m_fSourceTrackSampleSeconds, animatedArtistParameters))
            return Fail_RenderOperation("Native material parameter track sample is invalid.", E_INVALIDARG);
        artistParameters = &animatedArtistParameters;
    }
    // Fixed decal/trail carriers share the admitted native parameter packet.
    const bool bKoukuFixedNative = nullptr == pShaderProgram &&
        nullptr != Find_ArtistNativeCarrierProgram(Resource.iSourceMaterialProfile) &&
        (pShader == m_pDecalShader || pShader == m_pTrailShader);
    if (bKoukuFixedNative)
    {
        if (BindFailed(pShader->Bind_RawValue("g_ArtistSourceMaterialParameters",
                artistParameters->data(), sizeof(*artistParameters))) ||
            BindFailed(pShader->Bind_RawValue("g_ArtistSourceMaterialTime", &fLocalTimeSeconds, sizeof(fLocalTimeSeconds))))
            return Fail_RenderOperation("Kouku native fixed-carrier parameter binding failed.", hFirstBindFailure);
        if (Resource.bSourceRequiresSceneDepth &&
            BindFailed(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Depth"), pShader, "g_EffectSceneDepthTexture")))
            return Fail_RenderOperation("Kouku fixed-carrier source depth is unavailable.", hFirstBindFailure);
        if (Resource.bSourceRequiresSceneColor &&
            (BindFailed(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_EffectSceneColor"), pShader, "g_EffectSceneColorTexture")) ||
             BindFailed(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_EffectSceneBloom"), pShader, "g_EffectSceneBloomTexture"))))
            return Fail_RenderOperation("Kouku fixed-carrier source scene color is unavailable.", hFirstBindFailure);
        if (pShader == m_pTrailShader)
        {
            // Native ribbon distortion projects source world centimeters once.
            const matrix_t SourceToClient = XMMatrixSet(.01f,0.f,0.f,0.f,
                0.f,0.f,-.01f,0.f, 0.f,.01f,0.f,0.f, 0.f,0.f,0.f,1.f);
            float4x4_t SourceProjection;
            XMStoreFloat4x4(&SourceProjection, SourceToClient *
                XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::VIEW)) *
                XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::PROJ)));
            for (auto& Row : SourceProjection.m) for (auto& Value : Row) Value *= 100.f;
            if (BindFailed(pShader->Bind_Matrix("g_KoukuSourceProjection", &SourceProjection)))
                return Fail_RenderOperation("Kouku ribbon source projection binding failed.", hFirstBindFailure);
        }
        if (pShader == m_pDecalShader)
        {
            // Match the existing native model/mesh adapter: committed scene ambient;
            // UE skylight hemispheres have no scene owner and remain disabled.
            float4_t Ambient{0.f, 0.f, 0.f, 1.f};
            for (const auto& Light : CGameInstance::Get().Get_SceneLights())
                if (Light.eType == LIGHT::DIRECTIONAL)
                { Ambient.x += Light.vAmbient.x; Ambient.y += Light.vAmbient.y; Ambient.z += Light.vAmbient.z; }
            if (BindFailed(pShader->Bind_RawValue("g_KoukuDecalAmbient", &Ambient, sizeof(Ambient))))
                return Fail_RenderOperation("Kouku decal ambient binding failed.", hFirstBindFailure);
        }
    }
    const bool_t bGenericCarrierShader = nullptr != pShaderProgram &&
        pShaderProgram->eFamily == EFFECT_SHADER_FAMILY::GENERIC;
    // The native ARTIST particle programs consume their source packet and
    // shared UV/color inputs, but never the generic material textures/effects.
    // Match the selected executable, not an authored native-profile flag: mesh
    // and fixed carriers continue to receive the complete common packet.
    const bool_t bNativeArtistParticle = nullptr != pShaderProgram &&
        pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::PARTICLE &&
        pShaderProgram->eFamily == EFFECT_SHADER_FAMILY::ARTIST &&
        pShaderProgram == Get_EffectShaderProgram(Resource.iShaderProgramIndex) &&
        Resource.iShaderProgramIndex < m_ShaderPrograms.size() &&
        pShader == m_ShaderPrograms[Resource.iShaderProgramIndex] &&
        Resource.iSourceMaterialProfile >= pShaderProgram->iFirstProfile &&
        Resource.iSourceMaterialProfile <= pShaderProgram->iLastProfile;
	if ((nullptr != pShaderProgram && pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::MESH) || (nullptr != pShaderProgram && pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::PARTICLE))
	{
        const auto BindNativePacket = [&](const char* pParameters, const char* pTime,
            const auto& Parameters)
        {
            return BindFailed(pShader->Bind_RawValue(pParameters, Parameters.data(), static_cast<uint32_t>(sizeof(Parameters)))) ||
                BindFailed(pShader->Bind_RawValue(pTime, &fLocalTimeSeconds, sizeof(fLocalTimeSeconds)));
        };
        bool NativeBindFailed = false;
        switch (pShaderProgram->eFamily)
        {
        case EFFECT_SHADER_FAMILY::GENERIC: break;
        case EFFECT_SHADER_FAMILY::DIMENSIONMASTER_Q:
            NativeBindFailed = BindNativePacket("g_QSourceMaterialParameters", "g_QSourceMaterialTime", Resource.QSourceMaterialParameters); break;
        case EFFECT_SHADER_FAMILY::DIMENSIONMASTER_V:
        case EFFECT_SHADER_FAMILY::DIMENSIONMASTER_WR:
        case EFFECT_SHADER_FAMILY::DIMENSIONMASTER_SD:
            NativeBindFailed = BindNativePacket("g_VSourceMaterialParameters", "g_VSourceMaterialTime", Resource.VSourceMaterialParameters); break;
        case EFFECT_SHADER_FAMILY::DIMENSIONMASTER_ALTV:
            NativeBindFailed = BindNativePacket("g_ALTVSourceMaterialParameters", "g_ALTVSourceMaterialTime", Resource.ALTVSourceMaterialParameters); break;
        case EFFECT_SHADER_FAMILY::ARTIST:
        {
            NativeBindFailed = BindNativePacket("g_ArtistSourceMaterialParameters", "g_ArtistSourceMaterialTime", *artistParameters);
            if (nullptr != Find_ArtistNativeCarrierProgram(Resource.iSourceMaterialProfile))
            {
                float4_t SceneAmbient{0.f, 0.f, 0.f, 1.f};
                for (const auto& Light : CGameInstance::Get().Get_SceneLights())
                    if (Light.eType == LIGHT::DIRECTIONAL)
                    { SceneAmbient.x += Light.vAmbient.x; SceneAmbient.y += Light.vAmbient.y; SceneAmbient.z += Light.vAmbient.z; }
                if (Resource.iSourceMaterialProfile == 2893u &&
                    pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::MESH)
                {
                    // Native lit paper exports material MRTs; this forward carrier
                    // must receive direct lighting in addition to its RT0 ambient.
                    std::array<float4_t, 16> Directions{}, Colors{};
                    uint32_t Count = 0u;
                    for (const auto& Light : CGameInstance::Get().Get_SceneLights())
                    {
                        if (Light.eType != LIGHT::DIRECTIONAL ||
                            Light.eReceiver == LIGHT_RECEIVER::SOURCE_CHARACTER)
                            continue;
                        if (Count == Directions.size()) { NativeBindFailed = true; break; }
                        // Client to UE source basis; source world/view use it too.
                        Directions[Count] = { Light.vDirection.x, -Light.vDirection.z, Light.vDirection.y, 0.f };
                        Colors[Count] = Light.vDiffuse;
                        ++Count;
                    }
                    NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_KoukuDoveDirectionalCount", &Count, sizeof(Count))) || NativeBindFailed;
                    NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_KoukuDoveDirectionalDirections", Directions.data(), sizeof(Directions))) || NativeBindFailed;
                    NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_KoukuDoveDirectionalColors", Colors.data(), sizeof(Colors))) || NativeBindFailed;
                }
                NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_KoukuSourceAmbient", &SceneAmbient, sizeof(SceneAmbient))) || NativeBindFailed;
                NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_KoukuSourceActorPosition", &m_vSourceActorPosition, sizeof(m_vSourceActorPosition))) || NativeBindFailed;
                // UE source world is centimeters in X,Z,-Y. Source clip W is
                // likewise centimeters, matching the native depth adapters.
                const matrix_t SourceToClient = XMMatrixSet(.01f,0.f,0.f,0.f,
                    0.f,0.f,-.01f,0.f, 0.f,.01f,0.f,0.f, 0.f,0.f,0.f,1.f);
                float4x4_t SourceProjection;
                XMStoreFloat4x4(&SourceProjection, SourceToClient *
                    XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::VIEW)) *
                    XMLoadFloat4x4(CGameInstance::Get().Get_Transform(D3DTS::PROJ)));
                for (auto& Row : SourceProjection.m) for (auto& Value : Row) Value *= 100.f;
                NativeBindFailed = BindFailed(pShader->Bind_Matrix("g_KoukuSourceProjection", &SourceProjection)) || NativeBindFailed;
            }
            break;
        }
        case EFFECT_SHADER_FAMILY::LANCE_MASTER:
        {
            NativeBindFailed = BindNativePacket("g_LanceVASourceMaterialParameters", "g_LanceVASourceMaterialTime", Resource.LanceVASourceMaterialParameters);
            if (pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::MESH &&
                Resource.iSourceMaterialProfile >= 782u && Resource.iSourceMaterialProfile <= 800u)
            {
                // The authored ALT V static backdrop consumes the committed scene
                // ambient through the same native lighting input as model cues.
                float4_t SceneAmbient{};
                for (const auto& Light : CGameInstance::Get().Get_SceneLights())
                    if (Light.eType == LIGHT::DIRECTIONAL)
                    { SceneAmbient.x += Light.vAmbient.x; SceneAmbient.y += Light.vAmbient.y; SceneAmbient.z += Light.vAmbient.z; }
                NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_LanceVAStaticAmbient", &SceneAmbient, sizeof(SceneAmbient))) || NativeBindFailed;
            }
            break;
        }
        case EFFECT_SHADER_FAMILY::WARLORD:
        {
            NativeBindFailed = BindNativePacket("g_WarlordSourceMaterialParameters", "g_WarlordSourceMaterialTime", Resource.VSourceMaterialParameters);
            float4_t SceneAmbient{0.f, 0.f, 0.f, 1.f};
            const float4_t NoHemisphere{};
            for (const auto& Light : CGameInstance::Get().Get_SceneLights())
                if (Light.eType == LIGHT::DIRECTIONAL)
                { SceneAmbient.x += Light.vAmbient.x; SceneAmbient.y += Light.vAmbient.y; SceneAmbient.z += Light.vAmbient.z; }
            NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_WarlordSkyUpper", &NoHemisphere, sizeof(NoHemisphere))) || NativeBindFailed;
            NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_WarlordSkyLower", &NoHemisphere, sizeof(NoHemisphere))) || NativeBindFailed;
            NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_WarlordAmbient", &SceneAmbient, sizeof(SceneAmbient))) || NativeBindFailed;
            if (Resource.iSourceMaterialProfile == 1122u || Resource.iSourceMaterialProfile == 1123u)
            {
                // Current committed contracts: at most 16 scene + 64 transient lights.
                // Preserve the same scene-then-transient order as CLight_Manager::Render_Lights.
                std::array<float4_t, 80> Directions{};
                std::array<float4_t, 80> Positions{};
                std::array<float4_t, 80> Colors{};
                std::array<float4_t, 80> SpotCones{};
                uint32_t iLightCount = 0u;
                const auto AppendLight = [&](const LIGHT_DESC& Light) -> HRESULT
                {
                    if (iLightCount == Directions.size())
                        return Fail_RenderOperation("Guardian PBR light packet exceeds the 16 scene + 64 transient contract.", E_INVALIDARG);
                    float4_t Direction{};
                    if (Light.eType == LIGHT::DIRECTIONAL || Light.eType == LIGHT::SPOT)
                    {
                        const float fLengthSquared = Light.vDirection.x * Light.vDirection.x +
                            Light.vDirection.y * Light.vDirection.y + Light.vDirection.z * Light.vDirection.z;
                        if (!std::isfinite(fLengthSquared) || fLengthSquared <= 1.e-12f)
                            return Fail_RenderOperation("Guardian PBR light has an invalid direction.", E_INVALIDARG);
                        const float fInverseLength = 1.f / std::sqrt(fLengthSquared);
                        const float fSign = Light.eType == LIGHT::DIRECTIONAL ? -1.f : 1.f;
                        Direction = float4_t(Light.vDirection.x * fInverseLength * fSign,
                            Light.vDirection.y * fInverseLength * fSign, Light.vDirection.z * fInverseLength * fSign, 0.f);
                    }
                    else if (Light.eType != LIGHT::POINT)
                        return Fail_RenderOperation("Guardian PBR light has an unsupported type.", E_INVALIDARG);
                    Direction.w = Light.eType == LIGHT::DIRECTIONAL ? 0.f : (Light.eType == LIGHT::POINT ? 1.f : 2.f);
                    Directions[iLightCount] = Direction;
                    Positions[iLightCount] = float4_t(Light.vPosition.x, Light.vPosition.y, Light.vPosition.z, Light.fRange);
                    Colors[iLightCount] = float4_t(Light.vDiffuse.x, Light.vDiffuse.y, Light.vDiffuse.z, Light.fFalloffExponent);
                    SpotCones[iLightCount] = float4_t(Light.fSpotInnerCos, Light.fSpotOuterCos, 0.f, 0.f);
                    ++iLightCount;
                    return S_OK;
                };
                for (const auto& Light : CGameInstance::Get().Get_SceneLights())
                    if (FAILED(AppendLight(Light)))
                        return E_INVALIDARG;
                for (const auto& Light : CPresentation_Manager::Get().Get_TransientLights())
                    if (FAILED(AppendLight(Light)))
                        return E_INVALIDARG;
                NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_WarlordGuardianLightCount", &iLightCount, sizeof(iLightCount))) || NativeBindFailed;
                NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_WarlordGuardianDirections", Directions.data(), static_cast<uint32_t>(sizeof(Directions)))) || NativeBindFailed;
                NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_WarlordGuardianPositions", Positions.data(), static_cast<uint32_t>(sizeof(Positions)))) || NativeBindFailed;
                NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_WarlordGuardianColors", Colors.data(), static_cast<uint32_t>(sizeof(Colors)))) || NativeBindFailed;
                NativeBindFailed = BindFailed(pShader->Bind_RawValue("g_WarlordGuardianSpotCones", SpotCones.data(), static_cast<uint32_t>(sizeof(SpotCones)))) || NativeBindFailed;
            }
            break;
        }
        default: return Fail_RenderOperation("Native shader family is invalid.", E_INVALIDARG, true);
        }
        if (NativeBindFailed)
            return Fail_RenderOperation("Selected native family material parameter binding failed.", hFirstBindFailure);
        if (bGenericCarrierShader)
        {
		if (BindFailed(pShader->Bind_RawValue(
			"g_ReconstructedMaterialEvaluatorEnabled",
			&Resource.iReconstructedMaterialEvaluatorEnabled,
			sizeof(Resource.iReconstructedMaterialEvaluatorEnabled))) ||
			BindFailed(pShader->Bind_RawValue(
				"g_ReconstructedMaterialFeatureMask",
				&Resource.iReconstructedMaterialFeatureMask,
				sizeof(Resource.iReconstructedMaterialFeatureMask))) ||
			BindFailed(pShader->Bind_RawValue("g_ReconstructedUVScale",
				&Resource.vReconstructedUVScale,
				sizeof(Resource.vReconstructedUVScale))) ||
			BindFailed(pShader->Bind_RawValue("g_ReconstructedPanRotationAux",
				&Resource.vReconstructedPanRotationAux,
				sizeof(Resource.vReconstructedPanRotationAux))) ||
			BindFailed(pShader->Bind_RawValue("g_ReconstructedColor",
				&Resource.vReconstructedColor,
				sizeof(Resource.vReconstructedColor))) ||
			BindFailed(pShader->Bind_RawValue("g_ReconstructedParams0",
				&Resource.vReconstructedParams0,
				sizeof(Resource.vReconstructedParams0))) ||
			BindFailed(pShader->Bind_RawValue("g_ReconstructedParams1",
				&Resource.vReconstructedParams1,
				sizeof(Resource.vReconstructedParams1))) ||
			BindFailed(pShader->Bind_RawValue("g_ArtistVisualV4Opcode",
				&Resource.iArtistVisualV4Opcode,
				sizeof(Resource.iArtistVisualV4Opcode))) ||
			BindFailed(pShader->Bind_RawValue("g_ArtistVisualV4TextureMask",
				&Resource.iArtistVisualV4TextureMask,
				sizeof(Resource.iArtistVisualV4TextureMask))) ||
			BindFailed(pShader->Bind_RawValue("g_ArtistVisualV4Params",
				Resource.ArtistVisualV4Params.data(),
				sizeof(Resource.ArtistVisualV4Params))) ||
			BindFailed(pShader->Bind_RawValue("g_ArtistVisualV4Colors",
				Resource.ArtistVisualV4Colors.data(),
				sizeof(Resource.ArtistVisualV4Colors))))
		{
			return Fail_RenderOperation(
				"Material bind failed: reconstructed evaluator block.",
				hFirstBindFailure);
		}
        }
	}
	const bool_t bRuntimeMaterialV2Shader = bGenericCarrierShader || pShader == m_pDecalShader ||
		pShader == m_pTrailShader || pShader == m_pRectShader;
	if (bRuntimeMaterialV2Shader)
	{
		if (
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2Enabled",
				&Resource.iRuntimeMaterialV2Enabled,
				sizeof(Resource.iRuntimeMaterialV2Enabled))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2Opcode",
				&Resource.iRuntimeMaterialV2Opcode,
				sizeof(Resource.iRuntimeMaterialV2Opcode))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2TextureLaneCount",
				&Resource.iRuntimeMaterialV2TextureLaneCount,
				sizeof(Resource.iRuntimeMaterialV2TextureLaneCount))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2TextureMask",
				&Resource.iRuntimeMaterialV2TextureMask,
				sizeof(Resource.iRuntimeMaterialV2TextureMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2DynamicConsumedMask",
				&Resource.iRuntimeMaterialV2DynamicConsumedMask,
				sizeof(Resource.iRuntimeMaterialV2DynamicConsumedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2DynamicSuppressedMask",
				&Resource.iRuntimeMaterialV2DynamicSuppressedMask,
				sizeof(Resource.iRuntimeMaterialV2DynamicSuppressedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2ParticleColorPolicy",
				&Resource.iRuntimeMaterialV2ParticleColorPolicy,
				sizeof(Resource.iRuntimeMaterialV2ParticleColorPolicy))) ||
			BindFailed(pShader->Bind_RawValue(
				"g_RuntimeMaterialV2ParticleColorConsumedMask",
				&Resource.iRuntimeMaterialV2ParticleColorConsumedMask,
				sizeof(Resource.iRuntimeMaterialV2ParticleColorConsumedMask))) ||
			BindFailed(pShader->Bind_RawValue(
				"g_RuntimeMaterialV2ParticleColorSuppressedMask",
				&Resource.iRuntimeMaterialV2ParticleColorSuppressedMask,
				sizeof(Resource.iRuntimeMaterialV2ParticleColorSuppressedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2ScalarCount",
				&Resource.iRuntimeMaterialV2ScalarCount,
				sizeof(Resource.iRuntimeMaterialV2ScalarCount))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2VectorCount",
				&Resource.iRuntimeMaterialV2VectorCount,
				sizeof(Resource.iRuntimeMaterialV2VectorCount))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2InputCount",
				&Resource.iRuntimeMaterialV2InputCount,
				sizeof(Resource.iRuntimeMaterialV2InputCount))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2InputConsumedMask",
				Resource.RuntimeMaterialV2InputConsumedMask.data(),
				sizeof(Resource.RuntimeMaterialV2InputConsumedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2InputSuppressedMask",
				Resource.RuntimeMaterialV2InputSuppressedMask.data(),
				sizeof(Resource.RuntimeMaterialV2InputSuppressedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2StaticInputCount",
				&Resource.iRuntimeMaterialV2StaticInputCount,
				sizeof(Resource.iRuntimeMaterialV2StaticInputCount))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2StaticSelectedMask",
				&Resource.iRuntimeMaterialV2StaticSelectedMask,
				sizeof(Resource.iRuntimeMaterialV2StaticSelectedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2StaticConsumedMask",
				&Resource.iRuntimeMaterialV2StaticConsumedMask,
				sizeof(Resource.iRuntimeMaterialV2StaticConsumedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2StaticSuppressedMask",
				&Resource.iRuntimeMaterialV2StaticSuppressedMask,
				sizeof(Resource.iRuntimeMaterialV2StaticSuppressedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2RenderInputCount",
				&Resource.iRuntimeMaterialV2RenderInputCount,
				sizeof(Resource.iRuntimeMaterialV2RenderInputCount))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2RenderConsumedMask",
				&Resource.iRuntimeMaterialV2RenderConsumedMask,
				sizeof(Resource.iRuntimeMaterialV2RenderConsumedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2RenderSuppressedMask",
				&Resource.iRuntimeMaterialV2RenderSuppressedMask,
				sizeof(Resource.iRuntimeMaterialV2RenderSuppressedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2ScalarBlocks",
				Resource.RuntimeMaterialV2ScalarBlocks.data(),
				sizeof(Resource.RuntimeMaterialV2ScalarBlocks))) ||
			BindFailed(pShader->Bind_RawValue(
				"g_RuntimeMaterialV2VectorComponentConsumedMask",
				Resource.RuntimeMaterialV2VectorComponentConsumedMask.data(),
				sizeof(Resource.RuntimeMaterialV2VectorComponentConsumedMask))) ||
			BindFailed(pShader->Bind_RawValue(
				"g_RuntimeMaterialV2VectorComponentSuppressedMask",
				Resource.RuntimeMaterialV2VectorComponentSuppressedMask.data(),
				sizeof(Resource.RuntimeMaterialV2VectorComponentSuppressedMask))) ||
			BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2NormalizedLife",
				&fNormalizedLife, sizeof(fNormalizedLife))) ||
			BindFailed(pShader->Bind_RawValue("g_EffectLocalTime",
				&fLocalTimeSeconds, sizeof(fLocalTimeSeconds))))
		{
			return Fail_RenderOperation(
				"Material bind failed: RuntimeMaterialV2 scalar block.",
				hFirstBindFailure);
		}
	}
	if (bRuntimeMaterialV2Shader &&
		BindFailed(pShader->Bind_RawValue("g_RuntimeMaterialV2Vectors",
			Resource.RuntimeMaterialV2Vectors.data(),
			sizeof(Resource.RuntimeMaterialV2Vectors))))
	{
		return Fail_RenderOperation(
			"Material bind failed: RuntimeMaterialV2 vector block.",
			hFirstBindFailure);
	}
	const bool_t bStandardColorV1Shader = bGenericCarrierShader ||
		pShader == m_pDecalShader || pShader == m_pTrailShader;
	if (bStandardColorV1Shader &&
		(BindFailed(pShader->Bind_RawValue("g_StandardColorV1Enabled",
			&Resource.iStandardColorV1Enabled,
			sizeof(Resource.iStandardColorV1Enabled))) ||
		 BindFailed(pShader->Bind_RawValue("g_StandardColorV1Header",
			Resource.StandardColorV1Header.data(),
			sizeof(Resource.StandardColorV1Header))) ||
		 BindFailed(pShader->Bind_RawValue("g_StandardColorV1BaseCoverage",
			Resource.StandardColorV1BaseCoverage.data(),
			sizeof(Resource.StandardColorV1BaseCoverage))) ||
		 BindFailed(pShader->Bind_RawValue("g_StandardColorV1Dissolve",
			Resource.StandardColorV1Dissolve.data(),
			sizeof(Resource.StandardColorV1Dissolve))) ||
		 BindFailed(pShader->Bind_RawValue("g_StandardColorV1Policies",
			Resource.StandardColorV1Policies.data(),
			sizeof(Resource.StandardColorV1Policies))) ||
		 BindFailed(pShader->Bind_RawValue("g_StandardColorV1Scalars",
			&Resource.vStandardColorV1Scalars,
			sizeof(Resource.vStandardColorV1Scalars)))))
	{
		return Fail_RenderOperation(
			"Material bind failed: StandardColorV1 packet.",
			hFirstBindFailure);
	}
	const std::string_view strSourceSubUVMode = SourceLiteralString(
		Element, "interpolationmethod");
	const uint32_t iSourceSubUVColumns = static_cast<uint32_t>((std::max)(
		1.f, SourceLiteralNumber(Element, "subimages_horizontal", 1.f)));
	const uint32_t iSourceSubUVRows = static_cast<uint32_t>((std::max)(
		1.f, SourceLiteralNumber(Element, "subimages_vertical", 1.f)));
	const uint64_t iSourceSubUVFrameCount =
		static_cast<uint64_t>(iSourceSubUVColumns) * iSourceSubUVRows;
	const bool_t bSourceSubUV = Element.SourceRecipe.bEnabled &&
		iSourceSubUVFrameCount > 1u && iSourceSubUVFrameCount <= UINT32_MAX &&
		!strSourceSubUVMode.empty() && strSourceSubUVMode != "none" &&
		strSourceSubUVMode != "psuvim_none";
	const bool_t bParticleLifeSubUV = (nullptr != pShaderProgram && pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::PARTICLE) &&
		!Element.SourceRecipe.bEnabled && Element.Detail.Particle.bSubUVOverLife;
	const bool_t bParticleOwnsAtlas = bSourceSubUV || bParticleLifeSubUV;

	float2_t UVOffset(
		Element.Detail.UV.vStart.x + Element.Detail.UV.vSpeed.x * fLocalTimeSeconds,
		Element.Detail.UV.vStart.y + Element.Detail.UV.vSpeed.y * fLocalTimeSeconds);
	if (Element.Detail.UV.bWave)
	{
		const f32_t Wave = std::sin(
			XM_2PI * Element.Detail.UV.fWaveFrequency * fLocalTimeSeconds);
		UVOffset.x += Element.Detail.UV.vWaveAmplitude.x * Wave;
		UVOffset.y += Element.Detail.UV.vWaveAmplitude.y * Wave;
	}
	int32_t iTileIndex = Element.Detail.UV.iTileIndex;
	if (!bParticleOwnsAtlas && Element.Detail.UV.bSequence)
	{
		const int32_t iTileCount = Element.Detail.UV.iTileColumns *
			Element.Detail.UV.iTileRows;
		const int32_t iFrame = static_cast<int32_t>(
			fLocalTimeSeconds / Element.Detail.UV.fSequenceTerm) +
			Element.Detail.UV.iTileIndex;
		iTileIndex = Element.Detail.UV.bLoop ?
			((iFrame % iTileCount) + iTileCount) % iTileCount :
			std::clamp(iFrame, 0, iTileCount - 1);
	}
	const float2_t UVScale = bParticleOwnsAtlas ? float2_t(1.f, 1.f) : float2_t(
		1.f / Element.Detail.UV.iTileColumns,
		1.f / Element.Detail.UV.iTileRows);
	if (!bParticleOwnsAtlas)
	{
		UVOffset.x += static_cast<f32_t>(
			iTileIndex % Element.Detail.UV.iTileColumns) * UVScale.x;
		UVOffset.y += static_cast<f32_t>(
			iTileIndex / Element.Detail.UV.iTileColumns) * UVScale.y;
	}
	const f32_t Dissolve = bNativeArtistParticle || Element.Detail.Timing.fDissolveStartNormalized >= 1.f ?
		0.f : std::clamp(
			(fNormalizedLife - Element.Detail.Timing.fDissolveStartNormalized) /
			(1.f - Element.Detail.Timing.fDissolveStartNormalized), 0.f, 1.f);

	const float4_t AuthoredColorMultiply = bNativeArtistParticle ? float4_t{} :
		Evaluate_CommonColor(Element, fNormalizedLife).vColorMultiply;
	float4_t ColorMultiply = Color.vColorMultiply;
	ColorMultiply.w *= fAlphaScale;
	const uint32_t iHasNoise = !bNativeArtistParticle && nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::NOISE_TEXTURE) ? 1u : 0u;
	const uint32_t iHasMask = !bNativeArtistParticle && nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::MASK_TEXTURE) ? 1u : 0u;
	const uint32_t iHasEmissive = !bNativeArtistParticle && nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE) ? 1u : 0u;
	const uint32_t iHasDissolve = !bNativeArtistParticle && nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE) ? 1u : 0u;
	const uint32_t iHasBase2 = !bNativeArtistParticle && nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::BASE2_TEXTURE) ? 1u : 0u;
	const uint32_t iHasMask2 = !bNativeArtistParticle && nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::MASK2_TEXTURE) ? 1u : 0u;
	const uint32_t iHasNoise2 = !bNativeArtistParticle && nullptr != Find_Texture(
		Resource.Textures, EFFECT_RESOURCE_SLOT::NOISE2_TEXTURE) ? 1u : 0u;
	const uint32_t iDistortionOnBase =
		Element.Detail.Color.bDistortionOnBaseMaterial ? 1u : 0u;
	if ((bGenericCarrierShader || pShader == m_pTrailShader) &&
		(BindFailed(pShader->Bind_RawValue("g_SourceMaterialProfile",
			&Resource.iSourceMaterialProfile,
			sizeof(Resource.iSourceMaterialProfile))) ||
		BindFailed(pShader->Bind_RawValue("g_AuthoredColorMultiply",
			&AuthoredColorMultiply, sizeof(AuthoredColorMultiply))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceScalars0",
			&Resource.vSourceScalars0,
			sizeof(Resource.vSourceScalars0))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceScalars1",
			&Resource.vSourceScalars1,
			sizeof(Resource.vSourceScalars1))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceVector0",
			&Resource.vSourceVector0,
			sizeof(Resource.vSourceVector0))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceVector1",
			&Resource.vSourceVector1,
			sizeof(Resource.vSourceVector1))) ||
		BindFailed(pShader->Bind_RawValue("g_TypedTrailParameters",
			Resource.TypedTrailParameters.data(),
			sizeof(Resource.TypedTrailParameters))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceTextureMask",
			&Resource.iSourceTextureMask,
			sizeof(Resource.iSourceTextureMask))) ||
		BindFailed(pShader->Bind_RawValue("g_LinearFlowParameters",
			Resource.LinearFlowParameters.data(),
			sizeof(Resource.LinearFlowParameters))) ||
		BindFailed(pShader->Bind_RawValue("g_LinearFlowMaskAColor",
			&Resource.vLinearFlowMaskAColor,
			sizeof(Resource.vLinearFlowMaskAColor))) ||
		BindFailed(pShader->Bind_RawValue("g_LinearFlowMaskBColor",
			&Resource.vLinearFlowMaskBColor,
			sizeof(Resource.vLinearFlowMaskBColor))) ||
		BindFailed(pShader->Bind_RawValue("g_BlacklineParameters",
			Resource.BlacklineParameters.data(),
			sizeof(Resource.BlacklineParameters))) ||
		BindFailed(pShader->Bind_RawValue("g_BlacklineDiffuseColor",
			&Resource.vBlacklineDiffuseColor,
			sizeof(Resource.vBlacklineDiffuseColor))) ||
		BindFailed(pShader->Bind_RawValue("g_BlacklineMaskColor",
			&Resource.vBlacklineMaskColor,
			sizeof(Resource.vBlacklineMaskColor))) ||
		BindFailed(pShader->Bind_RawValue("g_LocalCrackParameters",
			Resource.LocalCrackParameters.data(),
			sizeof(Resource.LocalCrackParameters))) ||
		BindFailed(pShader->Bind_RawValue("g_LocalCrackOutColor",
			&Resource.vLocalCrackOutColor,
			sizeof(Resource.vLocalCrackOutColor))) ||
		BindFailed(pShader->Bind_RawValue("g_LocalCrackInColor",
			&Resource.vLocalCrackInColor,
			sizeof(Resource.vLocalCrackInColor))) ||
		BindFailed(pShader->Bind_RawValue("g_LocalCrackReflectionColor",
			&Resource.vLocalCrackReflectionColor,
			sizeof(Resource.vLocalCrackReflectionColor))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceTextureClampUMask",
			&Resource.iSourceTextureClampUMask,
			sizeof(Resource.iSourceTextureClampUMask))) ||
		BindFailed(pShader->Bind_RawValue("g_SourceTextureClampVMask",
			&Resource.iSourceTextureClampVMask,
			sizeof(Resource.iSourceTextureClampVMask))) ||
		BindFailed(pShader->Bind_RawValue("g_GroupedUVScalePan",
			&Resource.GroupedConstants.vUVScalePan,
			sizeof(Resource.GroupedConstants.vUVScalePan))) ||
		BindFailed(pShader->Bind_RawValue("g_GroupedAlphaEmissive",
			&Resource.GroupedConstants.vAlphaEmissive,
			sizeof(Resource.GroupedConstants.vAlphaEmissive))) ||
		BindFailed(pShader->Bind_RawValue("g_GroupedNoiseDissolve",
			&Resource.GroupedConstants.vNoiseDissolve,
			sizeof(Resource.GroupedConstants.vNoiseDissolve))) ||
		BindFailed(pShader->Bind_RawValue("g_GroupedTint",
			&Resource.GroupedConstants.vTint,
			sizeof(Resource.GroupedConstants.vTint))) ||
		BindFailed(pShader->Bind_RawValue("g_GroupedMaterialFlags",
			&Resource.GroupedConstants.iFlags,
			sizeof(Resource.GroupedConstants.iFlags))) ||
		BindFailed(pShader->Bind_RawValue("g_EffectLocalTime",
			&fLocalTimeSeconds, sizeof(fLocalTimeSeconds))) ||
		BindFailed(pShader->Bind_RawValue("g_DynamicParameterSemantics",
			Resource.DynamicParameterSemantics.data(),
			sizeof(Resource.DynamicParameterSemantics)))))
	{
		return Fail_RenderOperation(
			"Material bind failed: source-profile block.",
			hFirstBindFailure);
	}
    if (((nullptr != pShaderProgram && !bGenericCarrierShader) || pShader == m_pDecalShader) &&
        (BindFailed(pShader->Bind_RawValue("g_SourceMaterialProfile",
            &Resource.iSourceMaterialProfile, sizeof(Resource.iSourceMaterialProfile))) ||
         BindFailed(pShader->Bind_RawValue("g_SourceTextureMask",
            &Resource.iSourceTextureMask, sizeof(Resource.iSourceTextureMask))) ||
         BindFailed(pShader->Bind_RawValue("g_SourceTextureClampUMask",
            &Resource.iSourceTextureClampUMask, sizeof(Resource.iSourceTextureClampUMask))) ||
         BindFailed(pShader->Bind_RawValue("g_SourceTextureClampVMask",
            &Resource.iSourceTextureClampVMask, sizeof(Resource.iSourceTextureClampVMask)))))
    {
        return Fail_RenderOperation(
            "Material bind failed: native source texture profile block.", hFirstBindFailure);
    }
	if (pShader == m_pRectShader && 0u != Resource.iRuntimeMaterialV2Enabled &&
		BindFailed(pShader->Bind_RawValue("g_SourceTextureMask",
			&Resource.iSourceTextureMask,
			sizeof(Resource.iSourceTextureMask))))
	{
		return Fail_RenderOperation(
			"Material bind failed: typed sprite rect source texture mask.",
			hFirstBindFailure);
	}
	if (bGenericCarrierShader && pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::MESH)
	{
		// 공유 shader의 이전 draw 상태가 다른 CubeSample 재질로 새지 않도록 매번 선택한다.
		const auto& effectId = Get_StagedDocument().strEffectAssetId;
		const auto& elementId = Element.strElementId;
		const bool alphaOver = Material.eRenderProfile == EFFECT_RENDER_PROFILE::ALPHA_ONE_SIDED_DEPTH_READ ||
			Material.eRenderProfile == EFFECT_RENDER_PROFILE::ALPHA_TWO_SIDED_DEPTH_READ;
		const bool selectedCube = alphaOver && Resource.iSourceMaterialProfile == 42u &&
			((effectId == "effect.dimensionmaster.skill.2050100.tuning.restore" &&
			  elementId == "authored.source-particle.b4984eeabefb0fc93783ecda") ||
			 (effectId == "effect.dimensionmaster.skill.2050180.tuning.restore" &&
			  elementId == "authored.tuning.dimensionmaster.r.purple-glass-sequence") ||
			 (effectId == "effect.dimensionmaster.skill.2050210.a1.full.restore" &&
			  (elementId == "authored.composition.dimensionmaster.a.hit-1-purple-cube" ||
			   elementId == "authored.composition.dimensionmaster.a.hit-2-purple-cube" ||
			   elementId == "authored.composition.dimensionmaster.a.hit-3-purple-cube" ||
			   elementId == "authored.composition.dimensionmaster.a.hit-4-purple-cube")));
		const int32_t cubeColorResponse = selectedCube ? 1 : 0;
		if (BindFailed(pShader->Bind_RawValue("g_CubeColorResponseEnabled",
				&cubeColorResponse, sizeof(cubeColorResponse))) ||
			(selectedCube && BindFailed(CGameInstance::Get().Bind_ActiveSceneColorTransform(pShader.get()))))
			return Fail_RenderOperation("Cube color response binding failed.", hFirstBindFailure);
		uint32_t iRingFillEnabled = 0u;
		uint32_t iRingFillDirection = 0u;
		uint32_t iRingFillInvert = 0u;
		f32_t fRingFillProgress = 1.f;
		f32_t fRingFillFeather = 0.f;
		const EFFECT_MESH_RING_FILL_DESC& RingFill =
			Element.Detail.Mesh.RingFill;
		const bool_t bGenericManualMeshParticle =
			RingFill.bEnabled &&
			Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			nullptr != Find_Binding(
				Element, EFFECT_RESOURCE_SLOT::MESH_MODEL) &&
			nullptr != Resource.pModel &&
			!Element.SourceRecipe.bEnabled &&
			Material.strTemplateId ==
				EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			Material.eRenderProfile !=
				EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE &&
			!Material.SourceMaterial.bEnabled &&
			!Material.Execution.bEnabled &&
			0u == Resource.iSourceMaterialProfile &&
			0u == Resource.iReconstructedMaterialEvaluatorEnabled &&
			0u == Resource.iArtistVisualV4Opcode &&
			0u == Resource.iRuntimeMaterialV2Enabled &&
			0u == Resource.iStandardColorV1Enabled;
		if (bGenericManualMeshParticle)
		{
			iRingFillEnabled = 1u;
			iRingFillDirection = static_cast<uint32_t>(RingFill.eDirection);
			iRingFillInvert = RingFill.bInvert ? 1u : 0u;
			fRingFillProgress = RingFill.fProgress;
			const EFFECT_LINEAR_LERP_DESC& LinearLerp =
				Element.Detail.LinearLerp;
			if (LinearLerp.bRingFillProgress)
			{
				/* Ring Fill is an Element timeline, not an implicit particle fade.
				   Fixed bursts are born on the first simulation step, so particle
				   age cannot reach one at the authored completion-wave boundary. */
				const f32_t fLifeT = Element.Detail.Timing.fLifeTimeSeconds > 0.f ?
					std::clamp(fLocalTimeSeconds /
						Element.Detail.Timing.fLifeTimeSeconds, 0.f, 1.f) :
					std::clamp(fNormalizedLife, 0.f, 1.f);
				fRingFillProgress +=
					(LinearLerp.fEndRingFillProgress - fRingFillProgress) *
					fLifeT;
			}
			fRingFillProgress = std::clamp(fRingFillProgress, 0.f, 1.f);
			fRingFillFeather = std::clamp(RingFill.fFeather, 0.f, 0.5f);
		}
		if (BindFailed(pShader->Bind_RawValue("g_RingFillEnabled",
				&iRingFillEnabled, sizeof(iRingFillEnabled))) ||
			BindFailed(pShader->Bind_RawValue("g_RingFillDirection",
				&iRingFillDirection, sizeof(iRingFillDirection))) ||
			BindFailed(pShader->Bind_RawValue("g_RingFillInvert",
				&iRingFillInvert, sizeof(iRingFillInvert))) ||
			BindFailed(pShader->Bind_RawValue("g_RingFillProgress",
				&fRingFillProgress, sizeof(fRingFillProgress))) ||
			BindFailed(pShader->Bind_RawValue("g_RingFillFeather",
				&fRingFillFeather, sizeof(fRingFillFeather))))
		{
			return Fail_RenderOperation(
				"Material bind failed: generic mesh Ring Fill block.",
				hFirstBindFailure);
		}
	}
	if ((bGenericCarrierShader && pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::PARTICLE) || pShader == m_pRectShader)
	{
		uint32_t iLinearRevealEnabled = 0u;
		uint32_t iLinearRevealAxis = 1u;
		uint32_t iLinearRevealInvert = 1u;
		f32_t fLinearRevealStartSeconds = 0.f;
		f32_t fLinearRevealDurationSeconds = 0.55f;
		f32_t fLinearRevealEdgeWidth = 0.045f;
		f32_t fLinearRevealSoftness = 0.03f;
		float4_t vLinearRevealEdgeColor = { 1.f, 1.f, 1.f, 1.f };
		f32_t fLinearRevealEdgeEmissive = 7.f;
		const EFFECT_LINEAR_REVEAL_DESC& LinearReveal =
			Element.Detail.Sprite.LinearReveal;
		const bool_t bDirectHandAuthored = Element.strSourceNode.empty() ||
			Element.strSourceNode.starts_with("authored-copy:");
		const bool_t bExpectedSpriteCarrier =
			((nullptr != pShaderProgram && pShaderProgram->eCarrier == EFFECT_SHADER_CARRIER::PARTICLE) &&
			 Element.eKind == EFFECT_ELEMENT_KIND::PARTICLE &&
			 nullptr == Find_Binding(Element, EFFECT_RESOURCE_SLOT::MESH_MODEL) &&
			 nullptr == Resource.pModel) ||
			(pShader == m_pRectShader &&
			 Element.eKind == EFFECT_ELEMENT_KIND::SPRITE);
		const bool_t bGenericManualSprite = LinearReveal.bEnabled &&
			bExpectedSpriteCarrier && bDirectHandAuthored &&
			Element.Renderer.eType == EFFECT_RENDERER_TYPE::END &&
			!Element.SourceRecipe.bEnabled &&
			!Element.SourcePresentation.bEnabled &&
			Material.strTemplateId ==
				EFFECT_STANDARD_MATERIAL_TEMPLATE_ID &&
			Material.strSourceMaterialPath.empty() &&
			!Material.SourceMaterial.bEnabled &&
			!Material.Execution.bEnabled &&
			!Material.Execution.bFailClosed &&
			!Material.Execution.bAuthoringApproximate &&
			Material.eRenderProfile !=
				EFFECT_RENDER_PROFILE::OPAQUE_BACK_DEPTH_WRITE &&
			0u == Resource.iSourceMaterialProfile &&
			0u == Resource.iReconstructedMaterialEvaluatorEnabled &&
			0u == Resource.iArtistVisualV4Opcode &&
			0u == Resource.iRuntimeMaterialV2Enabled &&
			0u == Resource.iStandardColorV1Enabled;
		if (bGenericManualSprite)
		{
			iLinearRevealEnabled = 1u;
			iLinearRevealAxis = static_cast<uint32_t>(LinearReveal.eAxis);
			iLinearRevealInvert = LinearReveal.bInvert ? 1u : 0u;
			fLinearRevealStartSeconds = std::clamp(
				LinearReveal.fStartSeconds, 0.f, 30.f);
			fLinearRevealDurationSeconds = std::clamp(
				LinearReveal.fDurationSeconds, 0.0001f, 30.f);
			fLinearRevealEdgeWidth = std::clamp(
				LinearReveal.fEdgeWidth, 0.f, 0.5f);
			fLinearRevealSoftness = std::clamp(
				LinearReveal.fSoftness, 0.f,
				(std::max)(0.f, 0.5f - fLinearRevealEdgeWidth));
			vLinearRevealEdgeColor =
			{
				std::clamp(LinearReveal.vEdgeColor.x, 0.f, 1.f),
				std::clamp(LinearReveal.vEdgeColor.y, 0.f, 1.f),
				std::clamp(LinearReveal.vEdgeColor.z, 0.f, 1.f),
				std::clamp(LinearReveal.vEdgeColor.w, 0.f, 1.f)
			};
			fLinearRevealEdgeEmissive = std::clamp(
				LinearReveal.fEdgeEmissive, 0.f, 100.f);
		}
		if (BindFailed(pShader->Bind_RawValue("g_LinearRevealEnabled",
				&iLinearRevealEnabled, sizeof(iLinearRevealEnabled))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealAxis",
				&iLinearRevealAxis, sizeof(iLinearRevealAxis))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealInvert",
				&iLinearRevealInvert, sizeof(iLinearRevealInvert))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealStartSeconds",
				&fLinearRevealStartSeconds,
				sizeof(fLinearRevealStartSeconds))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealDurationSeconds",
				&fLinearRevealDurationSeconds,
				sizeof(fLinearRevealDurationSeconds))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealEdgeWidth",
				&fLinearRevealEdgeWidth, sizeof(fLinearRevealEdgeWidth))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealSoftness",
				&fLinearRevealSoftness, sizeof(fLinearRevealSoftness))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealEdgeColor",
				&vLinearRevealEdgeColor, sizeof(vLinearRevealEdgeColor))) ||
			BindFailed(pShader->Bind_RawValue("g_LinearRevealEdgeEmissive",
				&fLinearRevealEdgeEmissive,
				sizeof(fLinearRevealEdgeEmissive))))
		{
			return Fail_RenderOperation(
				"Material bind failed: generic sprite Linear Reveal block.",
				hFirstBindFailure);
		}
	}

	const bool_t bBindFailed =
		BindFailed(pShader->Bind_RawValue("g_UVScale", &UVScale, sizeof(UVScale))) ||
		BindFailed(pShader->Bind_RawValue("g_UVOffset", &UVOffset, sizeof(UVOffset))) ||
		BindFailed(pShader->Bind_RawValue("g_ColorOffset", &Color.vColorOffset, sizeof(Color.vColorOffset))) ||
		BindFailed(pShader->Bind_RawValue("g_ColorMultiply", &ColorMultiply, sizeof(ColorMultiply))) ||
		BindFailed(pShader->Bind_RawValue("g_ColorClip", &Color.fColorClip, sizeof(Color.fColorClip))) ||
		BindFailed(pShader->Bind_RawValue("g_EmissiveIntensity", &Color.fEmissiveIntensity, sizeof(Color.fEmissiveIntensity))) ||
		(!bNativeArtistParticle && (
		BindFailed(pShader->Bind_RawValue("g_DistortionIntensity", &Color.fDistortionIntensity, sizeof(Color.fDistortionIntensity))) ||
		BindFailed(pShader->Bind_RawValue("g_DistortionOnBaseMaterial", &iDistortionOnBase, sizeof(iDistortionOnBase))) ||
		BindFailed(pShader->Bind_RawValue("g_RadialTime", &Color.fRadialTime, sizeof(Color.fRadialTime))) ||
		BindFailed(pShader->Bind_RawValue("g_RadialIntensity", &Color.fRadialIntensity, sizeof(Color.fRadialIntensity))) ||
		BindFailed(pShader->Bind_RawValue("g_DissolveAmount", &Dissolve, sizeof(Dissolve))) ||
		BindFailed(pShader->Bind_RawValue("g_HasNoise", &iHasNoise, sizeof(iHasNoise))) ||
		BindFailed(pShader->Bind_RawValue("g_HasMask", &iHasMask, sizeof(iHasMask))) ||
		BindFailed(pShader->Bind_RawValue("g_HasEmissive", &iHasEmissive, sizeof(iHasEmissive))) ||
		BindFailed(pShader->Bind_RawValue("g_HasDissolve", &iHasDissolve, sizeof(iHasDissolve))) ||
		BindFailed(pShader->Bind_RawValue("g_HasBase2", &iHasBase2, sizeof(iHasBase2))) ||
		BindFailed(pShader->Bind_RawValue("g_HasMask2", &iHasMask2, sizeof(iHasMask2))) ||
		BindFailed(pShader->Bind_RawValue("g_HasNoise2", &iHasNoise2, sizeof(iHasNoise2))) ||
		BindFailed(pShader->Bind_Texture("g_Base2Texture", iHasBase2 ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::BASE2_TEXTURE) : m_pWhiteTexture)) ||
		BindFailed(pShader->Bind_Texture("g_Mask2Texture", iHasMask2 ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::MASK2_TEXTURE) : m_pWhiteTexture)) ||
		BindFailed(pShader->Bind_Texture("g_Noise2Texture", iHasNoise2 ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::NOISE2_TEXTURE) : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_BaseTexture",
			nullptr != Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::BASE_TEXTURE) ?
				Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::BASE_TEXTURE) :
				m_pWhiteTexture)) ||
		BindFailed(pShader->Bind_Texture("g_NoiseTexture", iHasNoise ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::NOISE_TEXTURE) : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_MaskTexture", iHasMask ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::MASK_TEXTURE) : m_pWhiteTexture)) ||
		BindFailed(pShader->Bind_Texture("g_EmissiveTexture", iHasEmissive ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::EMISSIVE_TEXTURE) : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_DissolveTexture", iHasDissolve ?
			Find_Texture(Resource.Textures, EFFECT_RESOURCE_SLOT::DISSOLVE_TEXTURE) : m_pBlackTexture)))) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture0",
			Resource.SourceTextures[0] ? Resource.SourceTextures[0] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture1",
			Resource.SourceTextures[1] ? Resource.SourceTextures[1] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture2",
			Resource.SourceTextures[2] ? Resource.SourceTextures[2] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture3",
			Resource.SourceTextures[3] ? Resource.SourceTextures[3] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture4",
			Resource.SourceTextures[4] ? Resource.SourceTextures[4] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture5",
			Resource.SourceTextures[5] ? Resource.SourceTextures[5] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture6",
			Resource.SourceTextures[6] ? Resource.SourceTextures[6] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture7",
			Resource.SourceTextures[7] ? Resource.SourceTextures[7] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture8",
			Resource.SourceTextures[8] ? Resource.SourceTextures[8] : m_pBlackTexture)) ||
		BindFailed(pShader->Bind_Texture("g_SourceTexture9",
			Resource.SourceTextures[9] ? Resource.SourceTextures[9] : m_pBlackTexture));
	if (bBindFailed)
	{
		return Fail_RenderOperation(
			"Material bind failed: common constants/textures block.",
			hFirstBindFailure);
	}
#if defined(LOSTARK_EFFECT_RECONSTRUCTED_EXECUTION_TESTS)
	Record_TestMaterialBinding();
#endif
	return S_OK;
}
```

#### C:/Users/tnest/Desktop/LostArk/Engine/Public/Renderer.h

Bind_ActiveSceneColorTransform 선언, 현재 renderer-owned 변환의 read-only shader binding.

SHA256: `d6553262809fa22981fe21cb540868c9481b337a8e5c3964c8738d3bbbf4c4b2`

```cpp
#pragma once

#include "GameObject.h"
#include "Engine_RenderTypes.h"
#include "Target_Manager.h"

/* 화면에 그려져야할 객체들을 그리는 순서에 따른 그룹별로 모아둔다. */
/* 모아둔 순서대로 객체들의 드로우콜을 수행해준다.*/

NS_BEGIN(Engine)

class CGpuSceneVisibility;
class CGpuVisibilityHiZ;
struct PRESENTATION_SCREEN_POST_DESC;
struct PRESENTATION_SCREEN_OVERLAY_DESC;

class CRenderer final
{
private:
	CRenderer(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	~CRenderer();

public:
    struct VIEWPORT_RESIZE_STATE final
    {
        uint32_t Width = 0u, Height = 0u;
        CTarget_Manager::RESIZE_STATE Targets;
        ComPtr<ID3D11DepthStencilView> BloomDSV, SSAODSV, SourceLightMaskDSV;
        ComPtr<ID3D11Texture2D> ScenePostTextures[2];
        ComPtr<ID3D11RenderTargetView> ScenePostRTVs[2], SceneBloomPostRTVs[2];
        ComPtr<ID3D11ShaderResourceView> ScenePostSRVs[2], SceneBloomPostSRVs[2];
    };
    HRESULT Stage_ViewportResize(uint32_t width, uint32_t height,
        const CTarget_Manager& targets, VIEWPORT_RESIZE_STATE& output) const;
    void Commit_ViewportResize(CTarget_Manager& targets, VIEWPORT_RESIZE_STATE& staged) noexcept;

	HRESULT Initialize();
	HRESULT Add_RenderObject(RENDERGROUP eRenderGroupID, shared_ptr<CGameObject> pRenderObject);
	HRESULT Draw(bool staticOcclusionAllowed = false);
    // Level 소유 장면을 weak 참조하므로 등록 자체가 언로드를 막지 않는다.
    void Register_GpuSceneVisibility(const shared_ptr<CGpuSceneVisibility>& scene);
    void Unregister_GpuSceneVisibility(const CGpuSceneVisibility* scene);
    MAP_VISIBILITY_SETTINGS Get_MapVisibilitySettings() const { return m_MapVisibility; }
    HRESULT Apply_MapVisibilitySettings(const MAP_VISIBILITY_SETTINGS& settings);
    RENDER_OPTIMIZATION_SETTINGS Get_RenderOptimizationSettings() const { return m_RenderOptimization; }
    HRESULT Apply_RenderOptimizationSettings(const RENDER_OPTIMIZATION_SETTINGS& settings);

	/* One off-screen portrait to draw through the deferred path before the world fills the
	G-buffer this frame. The caller owns the destination texture and the draw itself: Engine
	never learns what a character is, it only binds the portrait camera, runs the callback
	against the same G-buffer the world uses and resolves the same lighting, so a portrait
	cannot drift away from the field look when a material program is added. */
	struct PORTRAIT_REQUEST final
	{
		/* Receives the tone-mapped colour. Render_Final resolves the scene into it through a
		full-screen quad, so the portrait is a resample of the scene rather than a crop and the
		destination may be any size. Must outlive Draw. */
		ComPtr<ID3D11RenderTargetView>		pDestination = { nullptr };
		uint32_t							iWidth = 0u;
		uint32_t							iHeight = 0u;
		/* The portrait camera, already built by the caller. */
		float4x4_t							ViewMatrix{};
		float4x4_t							ProjMatrix{};
		/* Draws the subject with the G-buffer bound. A failure skips only this request. */
		function<HRESULT()>					DrawSubject;
		/* Reuses subject forward passes after combine. The optional blend state
		changes only coverage accumulation for the offscreen UI texture. */
		function<HRESULT(RENDERGROUP, ID3D11BlendState*)> DrawForward;
	};

	/* Queued for the next Draw and cleared by it, so a window that stops asking stops being
	drawn with no teardown of its own. */
	HRESULT Request_Portrait(PORTRAIT_REQUEST Request);
	// A visible refractive occurrence requests the pre-translucency colour once.
	void Request_SceneColorSnapshot() { m_bSceneColorSnapshotRequested = true; }
	HRESULT Refresh_SceneColorSnapshot();
    void Request_SceneEnvironmentReplacement() { m_bSceneEnvironmentReplaced = true; }
    bool_t Is_SceneEnvironmentReplaced() const { return m_bSceneEnvironmentReplaced; }
	const RENDER_QUALITY_SETTINGS& Get_RenderQualitySettings() const { return m_RenderQualitySettings; }
	HRESULT Apply_RenderQualitySettings(const RENDER_QUALITY_SETTINGS& Settings);
	const MATERIAL_RENDER_SETTINGS& Get_MaterialRenderSettings() const { return m_MaterialRenderSettings; }
	HRESULT Apply_MaterialRenderSettings(const MATERIAL_RENDER_SETTINGS& settings);
	const HEIGHT_FOG_SETTINGS& Get_HeightFogSettings() const { return m_HeightFogSettings; }
	HRESULT Apply_HeightFog(const HEIGHT_FOG_SETTINGS& Settings);
	HRESULT Bind_HeightFog(class CShader* shader) const;
	// 최종 합성과 같은 현재 tone/LUT를 차용하며 저장 설정이나 LUT 수명을 바꾸지 않는다.
	HRESULT Bind_ActiveSceneColorTransform(class CShader* shader) const;
	HRESULT Stage_RenderEnvironment(const wstring_t& cubePath, const float4_t& color,
		const float4_t& rotationIntensity, RENDER_ENVIRONMENT_STATE& outState, bool_t forceReload = false) const;
	void Commit_RenderEnvironment(const RENDER_ENVIRONMENT_STATE& state);
	RENDER_ENVIRONMENT_STATE Get_RenderEnvironment() const;

	/* The renderer owns the clock the deferred fog drifts on so no caller
	   has to feed a time value into every screen pass. */
	void Advance_PresentationClock(f32_t fTimeDelta);

	HRESULT Add_DebugComponent(shared_ptr<CComponent> pDebugComponent);

private:
	ComPtr<ID3D11Device>					m_pDevice = { nullptr };
	ComPtr<ID3D11DeviceContext>				m_pContext = { nullptr };
	ComPtr<ID3D11Texture2D>				m_pShadowDepthTexture = { nullptr };
	ComPtr<ID3D11DepthStencilView>			m_pShadowDSV = { nullptr };
	ComPtr<ID3D11ShaderResourceView>		m_pShadowSRV = { nullptr };
	/* Frame queue, not a lookup: filled by Request_Portrait and emptied by Draw. */
	vector<PORTRAIT_REQUEST>				m_PortraitRequests;
	ComPtr<ID3D11BlendState> m_pPortraitBlendState;
    struct STATIC_SHADOW_CASTER final
    {
        weak_ptr<CGameObject> Owner;
        const CGameObject* Identity = nullptr;
        uint64_t Revision = 0u;
    };
    // Weak ownership prevents a cached map from retaining an unloaded Level.
    ComPtr<ID3D11Texture2D> m_pStaticShadowDepthTexture;
    ComPtr<ID3D11ShaderResourceView> m_pStaticShadowSRV;
    vector<STATIC_SHADOW_CASTER> m_StaticShadowCasters;
    vector<STATIC_SHADOW_CASTER> m_CandidateStaticShadowCasters;
    vector<uint8_t> m_StaticShadowFlags;
    float4x4_t m_StaticShadowView{}, m_StaticShadowProjection{};
    bool_t m_bStaticShadowCacheValid = false;
    bool_t m_bStaticShadowSourceMaterials = false;
	ComPtr<ID3D11DepthStencilView>			m_pBloomDSV = { nullptr };
	ComPtr<ID3D11DepthStencilView>			m_pSSAODSV = { nullptr };
	ComPtr<ID3D11DepthStencilView> m_pSourceLightMaskDSV;
	uint32_t m_iSourceLightMaskWidth = 0u, m_iSourceLightMaskHeight = 0u;
	uint32_t m_iSourceLightMaskFailedWidth = 0u, m_iSourceLightMaskFailedHeight = 0u;
	// An opt-in late pass updates the picking target without clearing opaque picks.
	ComPtr<ID3D11Texture2D> m_pPickingTexture;
	ComPtr<ID3D11RenderTargetView> m_pPickingRTV;
	ComPtr<ID3D11Texture2D> m_pPickingDepthTexture;
	ComPtr<ID3D11DepthStencilView> m_pPickingDSV;
	D3D11_TEXTURE2D_DESC m_PickingDepthDesc{};
	D3D11_DEPTH_STENCIL_VIEW_DESC m_PickingDSVDesc{};
	bool_t m_bPickingDepthCaptured = false;
	vector<shared_ptr<CGameObject>>		m_RenderObjects[ETOUI(RENDERGROUP::END)];

    std::vector<weak_ptr<CGpuSceneVisibility>> m_GpuScenes;
    std::unique_ptr<CGpuVisibilityHiZ> m_GpuHiZ;
    uint64_t m_GpuVisibilityFrame = 0u;
    bool m_GpuHiZUnavailable = false;
    MAP_VISIBILITY_SETTINGS m_MapVisibility{};
    RENDER_OPTIMIZATION_SETTINGS m_RenderOptimization{};
    std::unique_ptr<class COcclusionCuller> m_OcclusionCuller;
    struct OCCLUSION_CANDIDATE final
    {
        size_t Index = 0u;
        CGameObject::STATIC_OCCLUSION_DESC Desc{};
        float Score = 0.f;
    };
    vector<OCCLUSION_CANDIDATE> m_OcclusionCandidates;
    vector<size_t> m_OccluderOrder;
    vector<uint8_t> m_OcclusionHidden;
    vector<shared_ptr<CGameObject>> m_OcclusionVisibleQueue;
    struct OCCLUSION_CACHE_ROW final
    {
        weak_ptr<CGameObject> Owner;
        CGameObject::STATIC_OCCLUSION_DESC Desc{};
        bool Hidden = false;
    };
    vector<OCCLUSION_CACHE_ROW> m_OcclusionCache;
    float4x4_t m_OcclusionView{}, m_OcclusionProjection{};
    float2_t m_OcclusionViewport{};
    uint64_t m_OcclusionSettingsRevision = 0u;
    bool m_OcclusionCacheValid = false;
	shared_ptr<class CVIBuffer_Rect>		m_pVIBuffer = { nullptr };
	shared_ptr<class CShader>				m_pShader = { nullptr };
    shared_ptr<class CShader> m_pScreenSpaceLightingShader;
    ComPtr<ID3D11Texture2D> m_pSSGIHalfTexture;
    ComPtr<ID3D11RenderTargetView> m_pSSGIHalfRTV;
    ComPtr<ID3D11ShaderResourceView> m_pSSGIHalfSRV;
    uint32_t m_iSSGIHalfWidth = 0u, m_iSSGIHalfHeight = 0u;

	float4x4_t								m_WorldMatrix{}, m_ViewMatrix{}, m_ProjMatrix{};
	uint32_t								m_iShadowMapSize = 2048u;
	float2_t								m_vShadowTexelSize = {};
	uint32_t								m_iBloomWidth = {};
	uint32_t								m_iBloomHeight = {};
	float2_t								m_vBloomTexelSize = {};
	uint32_t								m_iSSAOWidth = {};
	uint32_t								m_iSSAOHeight = {};
	float2_t								m_vSSAOTexelSize = {};
	ComPtr<ID3D11Texture2D>				m_pScenePostTextures[2];
	ComPtr<ID3D11RenderTargetView>		m_pScenePostRTVs[2];
	ComPtr<ID3D11ShaderResourceView>	m_pScenePostSRVs[2];
	ComPtr<ID3D11RenderTargetView> m_pSceneBloomPostRTVs[2];
	ComPtr<ID3D11ShaderResourceView> m_pSceneBloomPostSRVs[2];
	uint32_t								m_iScenePostWidth = {};
	uint32_t								m_iScenePostHeight = {};
	uint32_t								m_iScenePostFinalTarget = {};
	struct SOURCE_GRADING_CACHE_ENTRY final
	{
		SOURCE_POST_PROCESS_SETTINGS Settings;
		f32_t fGamma = 2.2f;
		ComPtr<ID3D11ShaderResourceView> pTexture;
	};
	vector<SOURCE_GRADING_CACHE_ENTRY> m_SourceGradingCache;
	ComPtr<ID3D11ShaderResourceView> m_pSourceGradingLut;
	RENDER_QUALITY_SETTINGS				m_RenderQualitySettings = {};
	MATERIAL_RENDER_SETTINGS m_MaterialRenderSettings = {};
	RENDER_ENVIRONMENT_STATE m_RenderEnvironment;
	HEIGHT_FOG_SETTINGS				m_HeightFogSettings = {};
	bool_t m_bSceneColorSnapshotRequested = false;
    bool_t m_bSceneEnvironmentReplaced = false;
	f32_t							m_fPresentationClock = 0.f;

	list<shared_ptr<CComponent>>			m_DebugComponent;

private:
	HRESULT Render_Priority();
	HRESULT Render_Shadow();
	HRESULT Render_NonBlend(bool staticOcclusionAllowed);
    void Cull_StaticOcclusion();
    bool Prepare_GpuSceneVisibility(bool staticOcclusionAllowed);
	HRESULT Render_SSAO();
	HRESULT Render_SSAOPass(const wstring_t& strMRTTag, DEFERRED ePass);
	HRESULT Render_Lights();
	HRESULT Render_Portraits();
	HRESULT Render_Combined(bool_t bPortrait = false);
    HRESULT Render_ScreenSpaceLighting();
    HRESULT Ready_SSGIHalfTarget(uint32_t width, uint32_t height);
	HRESULT Render_NonLight();
	HRESULT Render_Blend();
	HRESULT Capture_SceneColorSnapshot();
	HRESULT Render_SceneReplacements();
	HRESULT Render_ScreenPosts();
	HRESULT Render_DisplayOverlays();
	HRESULT Render_ScreenOverlay(const PRESENTATION_SCREEN_OVERLAY_DESC& Overlay);
	HRESULT Render_ScreenPostPass(
		ComPtr<ID3D11ShaderResourceView> pSourceSRV,
		ComPtr<ID3D11RenderTargetView> pDestinationRTV,
		uint32_t iPassIndex,
		ComPtr<ID3D11ShaderResourceView> pBloomSourceSRV,
		ComPtr<ID3D11RenderTargetView> pBloomDestinationRTV,
		const PRESENTATION_SCREEN_POST_DESC* pPostDesc = nullptr);
	HRESULT Render_Bloom();
	HRESULT Render_BloomPass(const wstring_t& strMRTTag,
		ComPtr<ID3D11ShaderResourceView> pSourceSRV, DEFERRED ePass);
	HRESULT Render_Final(bool_t bPortrait = false);
	HRESULT Render_UI();
	HRESULT Capture_PickingDepth();
	HRESULT Render_Picking();

private:
	HRESULT Stage_SourceGradingLut(const SOURCE_POST_PROCESS_SETTINGS& source, f32_t gamma,
		ComPtr<ID3D11ShaderResourceView>& output);
	HRESULT Ready_Shadow_Resources();
	HRESULT Ready_SourceLightMask(uint32_t width, uint32_t height);
	HRESULT Ready_Bloom_DSV();
	HRESULT Ready_SSAO_DSV();
	HRESULT Ready_ScenePostTargets(uint32_t iWidth, uint32_t iHeight);
	void SetUp_ViewportDesc(uint32_t iWidth, uint32_t iHeight);

private:
	HRESULT Render_Debug();



public:
	static unique_ptr<CRenderer> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);

};

NS_END
```

#### C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp

PackSourceToneCurve와 Bind_ActiveSceneColorTransform, Final과 동일한 현재 설정/LUT를 제공.

SHA256: `8f79d0b829239c14edca1c7f016eeca269fbcc0ea4a4a125af670a7c280804bd`

```cpp
#include "Renderer.h"
#include "GpuSceneVisibility.h"
#include "OcclusionCuller.h"
#include "BlendSortKey.h"
#pragma push_macro("new")
#undef new
#include "DirectXTK/DDSTextureLoader.h"
#pragma pop_macro("new")
#include "Engine_RenderTypes.h"
#include "Engine_VertexTypes.h"
#include "Material.h"
#include "Render_OutputContract.h"
#include "Profiler.h"

#include <cmath>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <typeinfo>
#include <filesystem>

namespace
{
	void WriteRendererFailure(
        const char* stage, const HRESULT result, const char* objectType = nullptr) noexcept
    {
        try
        {
            static unsigned int records = 0u;
            if (records >= 256u) return;
            ++records;
            wchar_t modulePath[32768]{};
            const DWORD length = GetModuleFileNameW(nullptr, modulePath, 32768u);
            std::filesystem::path path = "RendererExit.user.log";
            if (length > 0u && length < 32768u)
                path = std::filesystem::path(modulePath).parent_path().parent_path().parent_path() /
                    L"Default" / L"RendererExit.user.log";
            std::error_code error;
            const auto bytes = std::filesystem::file_size(path, error);
            const bool rotate = !error && bytes >= 1024u * 1024u;
            std::ofstream output(path, std::ios::binary | (rotate ? std::ios::trunc : std::ios::app));
            if (!output) return;
            const auto unixMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            output << "utc_ms=" << unixMs << " pid=" << GetCurrentProcessId() << " tick=" << GetTickCount64()
                << " stage=" << (stage ? stage : "unknown") << " hr=0x" << std::hex
                << std::uppercase << static_cast<unsigned long>(result) << std::dec;
            if (objectType) output << " object=" << objectType;
            output << '\n';
        }
        catch (...) { }
    }
}
#include "GameInstance.h"
#include "Presentation_Manager.h"
#include "Transform.h"

#include <algorithm>
#include <cfloat>
#include <cstring>
#include <cmath>

namespace
{
	constexpr uint32_t DEFERRED_PASS_SCENE_RESOLVE =
		ETOUI(DEFERRED::SCENE_RESOLVE);
	constexpr uint32_t DEFERRED_PASS_RGB_NOISE =
		ETOUI(DEFERRED::PRESENTATION_RGB_NOISE);
	constexpr uint32_t DEFERRED_PASS_ZOOM_BLUR =
		ETOUI(DEFERRED::PRESENTATION_ZOOM_BLUR);
	constexpr uint32_t DEFERRED_PASS_FILM_NOISE =
		ETOUI(DEFERRED::PRESENTATION_FILM_NOISE);
	constexpr uint32_t DEFERRED_PASS_TEXTURED_OVERLAY =
		PRESENTATION_TEXTURED_OVERLAY_PASS_INDEX;
	constexpr uint32_t DEFERRED_PASS_CHROMATIC_ABERRATION =
		ETOUI(DEFERRED::PRESENTATION_CHROMATIC_ABERRATION);

	static_assert(8u == DEFERRED_PASS_SCENE_RESOLVE);
	static_assert(9u == DEFERRED_PASS_RGB_NOISE);
	static_assert(10u == DEFERRED_PASS_ZOOM_BLUR);
	static_assert(11u == DEFERRED_PASS_FILM_NOISE);
	static_assert(14u == DEFERRED_PASS_TEXTURED_OVERLAY);
	static_assert(12u == ETOUI(DEFERRED::SSAO_RAW));
	static_assert(13u == ETOUI(DEFERRED::SSAO_BLUR));
	static_assert(15u == DEFERRED_PASS_CHROMATIC_ABERRATION);
	static_assert(16u == ETOUI(DEFERRED::SPOT));
	static_assert(17u == ETOUI(DEFERRED::PRESENTATION_DISPLAY_OVERLAY));
	static_assert(18u == ETOUI(DEFERRED::SOURCE_LIGHT_MASK));
	static_assert(19u == ETOUI(DEFERRED::SOURCE_DIRECTIONAL));
	static_assert(20u == ETOUI(DEFERRED::SOURCE_POINT));
	static_assert(21u == ETOUI(DEFERRED::SOURCE_SPOT));
	static_assert(28u == ETOUI(DEFERRED::SOURCE_LUT_BAKE));
	static_assert(29u == ETOUI(DEFERRED::SOURCE_LUT_BAKE_NEUTRAL));
	static_assert(30u == ETOUI(DEFERRED::PORTRAIT_RESOLVE));
	static_assert(31u == ETOUI(DEFERRED::END));

	bool_t IsFiniteInRange(const f32_t fValue, const f32_t fMinimum,
		const f32_t fMaximum)
	{
		return std::isfinite(fValue) &&
			fValue >= fMinimum && fValue <= fMaximum;
	}


	bool_t IsValidSourcePostProcess(const SOURCE_POST_PROCESS_SETTINGS& source)
	{
		const auto vectorInRange = [](const float3_t& v, float minimum, float maximum)
		{ return IsFiniteInRange(v.x, minimum, maximum) && IsFiniteInRange(v.y, minimum, maximum) && IsFiniteInRange(v.z, minimum, maximum); };
		if (!IsFiniteInRange(source.fToneScale, 0.000001f, 64.f) ||
			!IsFiniteInRange(source.fToneRange, 0.000001f, 128.f) ||
			!IsFiniteInRange(source.fToneToe, 0.f, 1.f) ||
			!IsFiniteInRange(source.fDesaturation, 0.f, 1.f) ||
			!vectorInRange(source.vHighlights, 0.000001f, 64.f) ||
			!vectorInRange(source.vMidtones, 0.000001f, 64.f) ||
			!vectorInRange(source.vShadows, -64.f, 64.f) ||
			!vectorInRange(source.vColorize, 0.f, 64.f) || source.LutLayers.size() > 32u) return false;
		float weight = 0.f;
		for (const auto& layer : source.LutLayers)
		{
			if (!IsFiniteInRange(layer.fWeight, 0.f, 1.f)) return false;
			weight += layer.fWeight;
		}
		return source.LutLayers.empty() || fabsf(weight - 1.f) < 0.0001f;
	}

	// 최종 합성과 효과의 역보정이 같은 source tone 입력을 사용하도록 packing을 공유한다.
	float4_t PackSourceToneCurve(const SOURCE_POST_PROCESS_SETTINGS& source)
	{
		const float scale = (std::max)(0.000001f, source.fToneScale);
		const float inverseScale = 1.f / scale;
		const float curveA = .22f * inverseScale;
		const float curveB = 1.f / (source.fToneRange / (source.fToneRange + curveA));
		return float4_t(curveA, curveB, sqrtf(curveB * curveA * inverseScale) - curveA, scale);
	}

	bool_t SameSourceGrading(const SOURCE_POST_PROCESS_SETTINGS& a, const SOURCE_POST_PROCESS_SETTINGS& b)
	{
		const auto equal = [](const float3_t& x, const float3_t& y) { return x.x == y.x && x.y == y.y && x.z == y.z; };
		if (a.fDesaturation != b.fDesaturation || !equal(a.vHighlights,b.vHighlights) ||
			!equal(a.vMidtones,b.vMidtones) || !equal(a.vShadows,b.vShadows) ||
			!equal(a.vColorize,b.vColorize) || a.LutLayers.size() != b.LutLayers.size()) return false;
		for (size_t i = 0u; i < a.LutLayers.size(); ++i)
			if (a.LutLayers[i].pLut != b.LutLayers[i].pLut || a.LutLayers[i].fWeight != b.LutLayers[i].fWeight) return false;
		return true;
	}


	struct ScopedSourceLutState final
	{
		ID3D11DeviceContext* context;
		ID3D11RenderTargetView* targets[8]{};
		ComPtr<ID3D11DepthStencilView> depth;
		D3D11_VIEWPORT viewports[16]{}; UINT viewportCount=16u;
		ComPtr<ID3D11VertexShader> vs; ComPtr<ID3D11PixelShader> ps; ComPtr<ID3D11GeometryShader> gs;
		ComPtr<ID3D11InputLayout> layout; ComPtr<ID3D11Buffer> vertex,index;
		UINT stride=0u,offset=0u,indexOffset=0u; DXGI_FORMAT indexFormat=DXGI_FORMAT_UNKNOWN;
		D3D11_PRIMITIVE_TOPOLOGY topology=D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		ComPtr<ID3D11BlendState> blend; float blendFactor[4]{}; UINT sampleMask=0u;
		ComPtr<ID3D11DepthStencilState> depthState; UINT stencilRef=0u;
		ComPtr<ID3D11RasterizerState> raster;
		ID3D11ShaderResourceView* resources[128]{}; ID3D11SamplerState* samplers[16]{};
		ID3D11Buffer* psBuffers[14]{}; ID3D11Buffer* vsBuffers[14]{};
		explicit ScopedSourceLutState(ID3D11DeviceContext* value) : context(value)
		{
			context->OMGetRenderTargets(8u,targets,depth.GetAddressOf());
			context->RSGetViewports(&viewportCount,viewports);
			context->VSGetShader(vs.GetAddressOf(),nullptr,nullptr); context->PSGetShader(ps.GetAddressOf(),nullptr,nullptr);
			context->GSGetShader(gs.GetAddressOf(),nullptr,nullptr); context->IAGetInputLayout(layout.GetAddressOf());
			context->IAGetVertexBuffers(0u,1u,vertex.GetAddressOf(),&stride,&offset);
			context->IAGetIndexBuffer(index.GetAddressOf(),&indexFormat,&indexOffset); context->IAGetPrimitiveTopology(&topology);
			context->OMGetBlendState(blend.GetAddressOf(),blendFactor,&sampleMask);
			context->OMGetDepthStencilState(depthState.GetAddressOf(),&stencilRef); context->RSGetState(raster.GetAddressOf());
			context->PSGetShaderResources(0u,128u,resources); context->PSGetSamplers(0u,16u,samplers);
			context->PSGetConstantBuffers(0u,14u,psBuffers); context->VSGetConstantBuffers(0u,14u,vsBuffers);
		}
		~ScopedSourceLutState()
		{
			ID3D11ShaderResourceView* empty[128]{}; context->PSSetShaderResources(0u,128u,empty);
			context->OMSetRenderTargets(8u,targets,depth.Get()); context->RSSetViewports(viewportCount,viewports);
			context->VSSetShader(vs.Get(),nullptr,0u); context->PSSetShader(ps.Get(),nullptr,0u); context->GSSetShader(gs.Get(),nullptr,0u);
			context->IASetInputLayout(layout.Get()); ID3D11Buffer* vb=vertex.Get(); context->IASetVertexBuffers(0u,1u,&vb,&stride,&offset);
			context->IASetIndexBuffer(index.Get(),indexFormat,indexOffset); context->IASetPrimitiveTopology(topology);
			context->OMSetBlendState(blend.Get(),blendFactor,sampleMask); context->OMSetDepthStencilState(depthState.Get(),stencilRef);
			context->RSSetState(raster.Get()); context->PSSetShaderResources(0u,128u,resources); context->PSSetSamplers(0u,16u,samplers);
			context->PSSetConstantBuffers(0u,14u,psBuffers); context->VSSetConstantBuffers(0u,14u,vsBuffers);
			for(auto* p:targets) if(p)p->Release(); for(auto* p:resources) if(p)p->Release();
			for(auto* p:samplers) if(p)p->Release(); for(auto* p:psBuffers) if(p)p->Release(); for(auto* p:vsBuffers) if(p)p->Release();
		}
	};

	bool_t IsValidRenderQualitySettings(
		const RENDER_QUALITY_SETTINGS& Settings)
	{
		return
			Settings.iTextureMinMip <= 3u &&
			IsValidSourcePostProcess(Settings.SourcePostProcess) &&
            IsFiniteInRange(Settings.fSSGIStrength, 0.f, 2.f) &&
            IsFiniteInRange(Settings.fSSGIRadius, 0.1f, 20.f) &&
            (Settings.iSSGISampleCount == 4u || Settings.iSSGISampleCount == 8u || Settings.iSSGISampleCount == 16u) &&
            IsFiniteInRange(Settings.fSSRStrength, 0.f, 2.f) &&
            IsFiniteInRange(Settings.fSSRMaxDistance, 0.1f, 100.f) &&
            IsFiniteInRange(Settings.fSSRThickness, 0.01f, 2.f) &&
            (Settings.iSSRStepCount == 16u || Settings.iSSRStepCount == 32u || Settings.iSSRStepCount == 64u) &&
            (Settings.iSSAOSampleCount == 4u || Settings.iSSAOSampleCount == 8u ||
                Settings.iSSAOSampleCount == 12u) &&
			IsFiniteInRange(Settings.vBloomTint.x, 0.f, 1.f) &&
			IsFiniteInRange(Settings.vBloomTint.y, 0.f, 1.f) &&
			IsFiniteInRange(Settings.vBloomTint.z, 0.f, 1.f) &&
			IsFiniteInRange(Settings.vBloomTint.w, 1.f, 1.f) &&
			IsFiniteInRange(Settings.fSceneDesaturation, 0.f, 1.f) &&
			Settings.iColorFilterType >= 0 && Settings.iColorFilterType <= 3 &&
			IsFiniteInRange(Settings.fColorFilterStrength, 0.f, 1.f) &&
			IsFiniteInRange(Settings.fSSAORadius, 0.01f, 8.f) &&
			IsFiniteInRange(Settings.fSSAOBias, 0.f, 1.f) &&
			IsFiniteInRange(Settings.fSSAOIntensity, 0.f, 4.f) &&
			IsFiniteInRange(Settings.fSSAOPower, 0.1f, 8.f) &&
			IsFiniteInRange(Settings.fSSAODistanceFade, 1.f, 1000.f) &&
			Settings.fSSAOBias < Settings.fSSAORadius &&
			Settings.fSSAODistanceFade >= Settings.fSSAORadius &&
			IsFiniteInRange(Settings.fBloomThreshold, 0.f, 64.f) &&
			IsFiniteInRange(Settings.fBloomSoftKnee, 0.f, 1.f) &&
			IsFiniteInRange(Settings.fBloomIntensity, 0.f, 16.f) &&
			IsFiniteInRange(Settings.fBloomScatter, 0.25f, 4.f) &&
			IsFiniteInRange(Settings.fExposure, 0.01f, 32.f) &&
			IsFiniteInRange(Settings.fWhitePoint, 1.f, 64.f) &&
			IsFiniteInRange(Settings.fGamma, 1.f, 3.f) &&
			IsFiniteInRange(Settings.fFXAASubpixel, 0.f, 1.f) &&
			IsFiniteInRange(Settings.fFXAAEdgeThreshold, 0.0312f, 0.333f) &&
			IsFiniteInRange(Settings.fFXAAEdgeThresholdMin, 0.0156f, 0.0833f);
	}
}

CRenderer::CRenderer(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{

}

CRenderer::~CRenderer()
{
}

HRESULT CRenderer::Initialize()
{
	float2_t		vViewportSize = CGameInstance::Get().Get_ViewportSize();
	m_vShadowTexelSize = float2_t(
		1.f / static_cast<f32_t>(m_iShadowMapSize),
		1.f / static_cast<f32_t>(m_iShadowMapSize));
	m_iBloomWidth = max(1u, static_cast<uint32_t>(vViewportSize.x) / 2u);
	m_iBloomHeight = max(1u, static_cast<uint32_t>(vViewportSize.y) / 2u);
	m_vBloomTexelSize = float2_t(
		1.f / static_cast<f32_t>(m_iBloomWidth),
		1.f / static_cast<f32_t>(m_iBloomHeight));
	m_iSSAOWidth = max(1u, static_cast<uint32_t>(vViewportSize.x) / 2u);
	m_iSSAOHeight = max(1u, static_cast<uint32_t>(vViewportSize.y) / 2u);
	m_vSSAOTexelSize = float2_t(
		1.f / static_cast<f32_t>(m_iSSAOWidth),
		1.f / static_cast<f32_t>(m_iSSAOHeight));

	/* For.Target_Diffuse */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Diffuse"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R8G8B8A8_UNORM, float4_t(1.f, 1.f, 1.f, 0.f))))
		return E_FAIL;

	/* For.Target_Normal */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Normal"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_UNORM, float4_t(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;

	/* For.Target_Shade */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Shade"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_Depth */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Depth"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R32G32B32A32_FLOAT, float4_t(1.f, 1.f, 1.f, 0.f))))
		return E_FAIL;

	/* For.Target_Specular */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Specular"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* Material input RGB is separate from accumulated light specular. */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_MaterialSpecular"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

    // Source character direct lighting needs UV and its full tangent basis.
    if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_CharacterSurface"), vViewportSize.x, vViewportSize.y,
        DXGI_FORMAT_R32G32B32A32_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))) ||
        FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_CharacterGeometry"), vViewportSize.x, vViewportSize.y,
        DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
        return E_FAIL;

	/* For.Target_PickPos */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_PickPos"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R32G32B32A32_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_Emissive */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Emissive"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* For.Target_SceneHDR */
	/* Scene colour before tone mapping. FP16 so values above 1 survive. */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_SceneHDR"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	// One shared full-resolution contribution target for all effects, never one per skill.
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_SceneBloom"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* Refractive effects read this snapshot while SceneHDR remains the output. */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_EffectSceneColor"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_EffectSceneBloom"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* Signed offsets: RG for ordinary effects, BA for receiver-isolated noise. */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_Distortion"), vViewportSize.x, vViewportSize.y,
		DXGI_FORMAT_R16G16B16A16_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	/* Half-resolution ambient occlusion: raw estimate plus bilateral resolve. */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_SSAORaw"), m_iSSAOWidth, m_iSSAOHeight,
		DXGI_FORMAT_R16_FLOAT, float4_t(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_SSAOBlur"), m_iSSAOWidth, m_iSSAOHeight,
		DXGI_FORMAT_R16_FLOAT, float4_t(1.f, 1.f, 1.f, 1.f))))
		return E_FAIL;

	/* Half-resolution bloom chain. R11G11B10 preserves positive HDR energy. */
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_BloomExtract"), m_iBloomWidth, m_iBloomHeight,
		DXGI_FORMAT_R11G11B10_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_BloomPing"), m_iBloomWidth, m_iBloomHeight,
		DXGI_FORMAT_R11G11B10_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_RenderTarget(TEXT("Target_BloomResult"), m_iBloomWidth, m_iBloomHeight,
		DXGI_FORMAT_R11G11B10_FLOAT, float4_t(0.f, 0.f, 0.f, 0.f))))
		return E_FAIL;

	if (FAILED(Ready_Shadow_Resources()))
		return E_FAIL;
	if (FAILED(Ready_Bloom_DSV()))
		return E_FAIL;
	if (FAILED(Ready_SSAO_DSV()))
		return E_FAIL;
	if (FAILED(Ready_ScenePostTargets(
		static_cast<uint32_t>(vViewportSize.x),
		static_cast<uint32_t>(vViewportSize.y))))
	{
		return E_FAIL;
	}



	/* MRT_GameObject */
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Diffuse"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Normal"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Depth"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_PickPos"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_Emissive"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_MaterialSpecular"))))
		return E_FAIL;
    if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_CharacterSurface"))))
        return E_FAIL;
    if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_GameObject"), TEXT("Target_CharacterGeometry"))))
        return E_FAIL;

	/* MRT_LightAcc */
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_LightAcc"), TEXT("Target_Shade"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_LightAcc"), TEXT("Target_Specular"))))
		return E_FAIL;

	/* MRT_SceneHDR */
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_SceneHDR"), TEXT("Target_SceneHDR"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_SceneHDR"), TEXT("Target_Distortion"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_SceneHDR"), TEXT("Target_SceneBloom"))))
		return E_FAIL;

	/* Half-resolution bloom ping-pong targets. */
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_BloomExtract"), TEXT("Target_BloomExtract"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_BloomPing"), TEXT("Target_BloomPing"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_BloomResult"), TEXT("Target_BloomResult"))))
		return E_FAIL;

	/* Half-resolution SSAO raw and bilateral targets. */
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_SSAORaw"), TEXT("Target_SSAORaw"))))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Add_MRT(TEXT("MRT_SSAOBlur"), TEXT("Target_SSAOBlur"))))
		return E_FAIL;

	m_pVIBuffer = CVIBuffer_Rect::Create(m_pDevice, m_pContext);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Deferred.hlsl"), VTXTEX::Elements, VTXTEX::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(vViewportSize.x, vViewportSize.y, 1.f));
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix,
		XMMatrixOrthographicLH(vViewportSize.x, vViewportSize.y, 0.f, 1.f));

#ifdef _DEBUG
	//if (FAILED(CGameInstance::Get().Ready_RT_DebugDesc(TEXT("Target_Diffuse"), 150.f, 150.f, 300.f, 300.f)))
	//	return E_FAIL;
	//if (FAILED(CGameInstance::Get().Ready_RT_DebugDesc(TEXT("Target_Normal"), 150.f, 450.f, 300.f, 300.f)))
	//	return E_FAIL;
	//if (FAILED(CGameInstance::Get().Ready_RT_DebugDesc(TEXT("Target_Shade"), 450.f, 150.f, 300.f, 300.f)))
	//	return E_FAIL;
	//if (FAILED(CGameInstance::Get().Ready_RT_DebugDesc(TEXT("Target_Specular"), 450.f, 450.f, 300.f, 300.f)))
	//	return E_FAIL;
#endif

	return S_OK;
}

HRESULT CRenderer::Stage_ViewportResize(uint32_t width, uint32_t height,
    const CTarget_Manager& targets, VIEWPORT_RESIZE_STATE& output) const
{
    if (width == 0u || height == 0u || width > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION ||
        height > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION) return E_INVALIDARG;
    VIEWPORT_RESIZE_STATE staged;
    staged.Width = width;
    staged.Height = height;
    const uint32_t halfWidth = max(1u, width / 2u), halfHeight = max(1u, height / 2u);
    const wchar_t* fullTargets[] = {
        L"Target_Diffuse", L"Target_Normal", L"Target_Shade", L"Target_Depth",
        L"Target_Specular", L"Target_MaterialSpecular", L"Target_CharacterSurface",
        L"Target_CharacterGeometry", L"Target_PickPos", L"Target_Emissive",
        L"Target_SceneHDR", L"Target_SceneBloom", L"Target_EffectSceneColor",
        L"Target_EffectSceneBloom", L"Target_Distortion"
    };
    const wchar_t* halfTargets[] = {
        L"Target_SSAORaw", L"Target_SSAOBlur", L"Target_BloomExtract",
        L"Target_BloomPing", L"Target_BloomResult"
    };
    vector<CTarget_Manager::RESIZE_REQUEST> requests;
    requests.reserve(size(fullTargets) + size(halfTargets));
    for (const auto* tag : fullTargets) requests.push_back({tag, width, height});
    for (const auto* tag : halfTargets) requests.push_back({tag, halfWidth, halfHeight});
    HRESULT result = targets.Stage_Resize(requests, staged.Targets);
    if (FAILED(result)) return result;

    const auto stageDepth = [this](uint32_t w, uint32_t h,
        ComPtr<ID3D11DepthStencilView>& depth) -> HRESULT
    {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = w; desc.Height = h;
        desc.MipLevels = desc.ArraySize = desc.SampleDesc.Count = 1u;
        desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        ComPtr<ID3D11Texture2D> texture;
        HRESULT hr = m_pDevice->CreateTexture2D(&desc, nullptr, texture.GetAddressOf());
        if (SUCCEEDED(hr)) hr = m_pDevice->CreateDepthStencilView(texture.Get(), nullptr, depth.GetAddressOf());
        return hr;
    };
    result = stageDepth(halfWidth, halfHeight, staged.BloomDSV);
    if (SUCCEEDED(result)) result = stageDepth(halfWidth, halfHeight, staged.SSAODSV);
    if (SUCCEEDED(result)) result = stageDepth(width, height, staged.SourceLightMaskDSV);
    if (FAILED(result)) return result;

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width; desc.Height = height;
    desc.MipLevels = desc.ArraySize = desc.SampleDesc.Count = 1u;
    desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    for (size_t index = 0u; index < 2u; ++index)
    {
        ComPtr<ID3D11Texture2D> bloom;
        result = m_pDevice->CreateTexture2D(&desc, nullptr, staged.ScenePostTextures[index].GetAddressOf());
        if (SUCCEEDED(result)) result = m_pDevice->CreateRenderTargetView(staged.ScenePostTextures[index].Get(),
            nullptr, staged.ScenePostRTVs[index].GetAddressOf());
        if (SUCCEEDED(result)) result = m_pDevice->CreateShaderResourceView(staged.ScenePostTextures[index].Get(),
            nullptr, staged.ScenePostSRVs[index].GetAddressOf());
        if (SUCCEEDED(result)) result = m_pDevice->CreateTexture2D(&desc, nullptr, bloom.GetAddressOf());
        if (SUCCEEDED(result)) result = m_pDevice->CreateRenderTargetView(bloom.Get(), nullptr,
            staged.SceneBloomPostRTVs[index].GetAddressOf());
        if (SUCCEEDED(result)) result = m_pDevice->CreateShaderResourceView(bloom.Get(), nullptr,
            staged.SceneBloomPostSRVs[index].GetAddressOf());
        if (FAILED(result)) return result;
    }
    output = std::move(staged);
    return S_OK;
}

void CRenderer::Commit_ViewportResize(CTarget_Manager& targets, VIEWPORT_RESIZE_STATE& staged) noexcept
{
    const auto previous = CGameInstance::Get().Get_ViewportSize();
    targets.Commit_Resize(staged.Targets, static_cast<f32_t>(staged.Width) - previous.x,
        static_cast<f32_t>(staged.Height) - previous.y);
    m_pBloomDSV.Swap(staged.BloomDSV);
    m_pSSAODSV.Swap(staged.SSAODSV);
    m_pSourceLightMaskDSV.Swap(staged.SourceLightMaskDSV);
    m_iSourceLightMaskWidth = staged.Width; m_iSourceLightMaskHeight = staged.Height;
    m_iSourceLightMaskFailedWidth = m_iSourceLightMaskFailedHeight = 0u;
    m_pPickingRTV.Reset();
    m_pPickingTexture.Reset();
    m_pPickingDSV.Reset();
    m_pPickingDepthTexture.Reset();
    m_PickingDepthDesc = {};
    m_PickingDSVDesc = {};
    m_bPickingDepthCaptured = false;
    m_iBloomWidth = m_iSSAOWidth = max(1u, staged.Width / 2u);
    m_iBloomHeight = m_iSSAOHeight = max(1u, staged.Height / 2u);
    m_vBloomTexelSize = m_vSSAOTexelSize = float2_t(1.f / m_iBloomWidth, 1.f / m_iBloomHeight);
    for (size_t index = 0u; index < 2u; ++index)
    {
        m_pScenePostTextures[index].Swap(staged.ScenePostTextures[index]);
        m_pScenePostRTVs[index].Swap(staged.ScenePostRTVs[index]);
        m_pScenePostSRVs[index].Swap(staged.ScenePostSRVs[index]);
        m_pSceneBloomPostRTVs[index].Swap(staged.SceneBloomPostRTVs[index]);
        m_pSceneBloomPostSRVs[index].Swap(staged.SceneBloomPostSRVs[index]);
    }
    m_iScenePostWidth = staged.Width; m_iScenePostHeight = staged.Height;
    m_iScenePostFinalTarget = 0u;
    // Keep a pending scene-color capture request for the next draw at the new size.
    XMStoreFloat4x4(&m_WorldMatrix, XMMatrixScaling(static_cast<f32_t>(staged.Width),
        static_cast<f32_t>(staged.Height), 1.f));
    XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(static_cast<f32_t>(staged.Width),
        static_cast<f32_t>(staged.Height), 0.f, 1.f));
    // Quality, source LUTs, scene environment and fixed-resolution shadow resources survive.
}

HRESULT CRenderer::Request_Portrait(PORTRAIT_REQUEST Request)
{
	if (nullptr == Request.pDestination || 0u == Request.iWidth || 0u == Request.iHeight ||
		!Request.DrawSubject)
	{
		return E_INVALIDARG;
	}

	m_PortraitRequests.push_back(std::move(Request));

	return S_OK;
}

HRESULT CRenderer::Add_RenderObject(RENDERGROUP eRenderGroupID, shared_ptr<CGameObject> pRenderObject)
{
	if (nullptr == pRenderObject ||
		eRenderGroupID >= RENDERGROUP::END)
		return E_FAIL;

	m_RenderObjects[ETOUI(eRenderGroupID)].push_back(std::move(pRenderObject));
    if (CProfiler* profiler = CGameInstance::Get().Get_Profiler())
    {
        switch (eRenderGroupID)
        {
        case RENDERGROUP::PRIORITY: profiler->Add_Counter(EProfilerCounter::RenderSubmissionsPriority); break;
        case RENDERGROUP::SHADOW: profiler->Add_Counter(EProfilerCounter::RenderSubmissionsShadow); break;
        case RENDERGROUP::NONBLEND: profiler->Add_Counter(EProfilerCounter::RenderSubmissionsNonBlend); break;
        case RENDERGROUP::BLEND: profiler->Add_Counter(EProfilerCounter::RenderSubmissionsBlend); break;
        default: break; // Other queues are not included in these four counters.
        }
    }

	return S_OK;
}

HRESULT CRenderer::Apply_MaterialRenderSettings(const MATERIAL_RENDER_SETTINGS& settings)
{
	if (static_cast<uint32_t>(settings.eDebugView) >= static_cast<uint32_t>(MATERIAL_DEBUG_VIEW::END))
		return E_INVALIDARG;
	const auto& pbr = settings.MapPBR;
	for (const auto value : { pbr.vContributionScale.x, pbr.vContributionScale.y,
		pbr.vContributionScale.z, pbr.vContributionScale.w })
		if (!IsFiniteInRange(value, 0.f, 4.f)) return E_INVALIDARG;
	if (!IsFiniteInRange(pbr.vSurfaceParameters.x, 0.f, 4.f) ||
		!IsFiniteInRange(pbr.vSurfaceParameters.y, -1.f, 1.f) ||
        !IsFiniteInRange(pbr.fCubeDiffuseScale, 0.f, 4.f) ||
		(pbr.vSurfaceParameters.z != 0.f && pbr.vSurfaceParameters.z != 1.f) ||
		pbr.vSurfaceParameters.w != 0.f) return E_INVALIDARG;
	m_MaterialRenderSettings = settings;
	return S_OK;
}


HRESULT CRenderer::Stage_SourceGradingLut(const SOURCE_POST_PROCESS_SETTINGS& source, f32_t gamma,
	ComPtr<ID3D11ShaderResourceView>& output)
{
	if (!m_pShader || !m_pVIBuffer) return E_FAIL;
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width=256u; desc.Height=16u; desc.MipLevels=desc.ArraySize=desc.SampleDesc.Count=1u;
	desc.Format=DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.Usage=D3D11_USAGE_IMMUTABLE; desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
	std::array<ComPtr<ID3D11ShaderResourceView>,4u> inputs;
	ID3D11ShaderResourceView* inputViews[4]{};
	float4_t weights{};
	float neutralWeight=source.LutLayers.empty()?1.f:0.f;
	size_t inputIndex=0u;
	for (const auto& layer : source.LutLayers)
	{
		if (!layer.pLut) { neutralWeight+=layer.fWeight; continue; }
		if (inputIndex>=inputs.size()) return E_INVALIDARG;
		std::array<uint32_t,4096u> pixels{};
		for(size_t i=0u;i<pixels.size();++i)
		{
			const auto& p=layer.pLut->Pixels[i];
			if(!IsFiniteInRange(p.x,0.f,1.f)||!IsFiniteInRange(p.y,0.f,1.f)||!IsFiniteInRange(p.z,0.f,1.f)||!IsFiniteInRange(p.w,0.f,1.f))return E_INVALIDARG;
			const auto byte=[](float v){return static_cast<uint32_t>(std::nearbyintf(v*255.f));};
			pixels[i]=byte(p.z)|(byte(p.y)<<8u)|(byte(p.x)<<16u)|(byte(p.w)<<24u);
		}
		D3D11_SUBRESOURCE_DATA initial{}; initial.pSysMem=pixels.data(); initial.SysMemPitch=256u*sizeof(uint32_t);
		ComPtr<ID3D11Texture2D> texture;
		HRESULT result=m_pDevice->CreateTexture2D(&desc,&initial,texture.GetAddressOf());
		if(SUCCEEDED(result))result=m_pDevice->CreateShaderResourceView(texture.Get(),nullptr,inputs[inputIndex].GetAddressOf());
		if(FAILED(result))return result;
		inputViews[inputIndex]=inputs[inputIndex].Get();
		(&weights.x)[inputIndex]=layer.fWeight; ++inputIndex;
	}
	desc.Usage=D3D11_USAGE_DEFAULT; desc.BindFlags=D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
	ComPtr<ID3D11Texture2D> target;
	ComPtr<ID3D11RenderTargetView> rtv;
	ComPtr<ID3D11ShaderResourceView> staged;
	HRESULT result=m_pDevice->CreateTexture2D(&desc,nullptr,target.GetAddressOf());
	if(SUCCEEDED(result))result=m_pDevice->CreateRenderTargetView(target.Get(),nullptr,rtv.GetAddressOf());
	if(SUCCEEDED(result))result=m_pDevice->CreateShaderResourceView(target.Get(),nullptr,staged.GetAddressOf());
	if(FAILED(result))return result;
	const float d=source.fDesaturation;
	const float4_t parameters[7]={
		{source.vShadows.x,source.vShadows.y,source.vShadows.z,1.f-d},
		{1.f/source.vHighlights.x,1.f/source.vHighlights.y,1.f/source.vHighlights.z,0.f},
		{source.vMidtones.x,source.vMidtones.y,source.vMidtones.z,0.f},
		{.3f*d,.59f*d,.11f*d,0.f},
		{source.vColorize.x,source.vColorize.y,source.vColorize.z,0.f},
		{1.f,1.f,1.f,1.f/(std::max)(gamma,.0001f)},
		{0.f,0.f,0.f,0.f}};
	{
		ScopedSourceLutState restore(m_pContext.Get());
		ID3D11RenderTargetView* destination=rtv.Get();
		m_pContext->OMSetRenderTargets(1u,&destination,nullptr);
		SetUp_ViewportDesc(256u,16u);
		if(FAILED(m_pShader->Bind_Textures("g_SourceLutInputs",inputViews,4u)) ||
			FAILED(m_pShader->Bind_RawValue("g_vSourceLutWeights",&weights,sizeof(weights))) ||
			FAILED(m_pShader->Bind_RawValue("g_fSourceNeutralLutWeight",&neutralWeight,sizeof(neutralWeight))) ||
			FAILED(m_pShader->Bind_RawValue("g_vSourceGradingParameters",parameters,sizeof(parameters))) ||
			FAILED(m_pShader->Begin(ETOUI(inputIndex == 0u ? DEFERRED::SOURCE_LUT_BAKE_NEUTRAL : DEFERRED::SOURCE_LUT_BAKE))) ||
			FAILED(m_pVIBuffer->Bind_Resources()) || FAILED(m_pVIBuffer->Render())) result=E_FAIL;
		else result=m_pDevice->GetDeviceRemovedReason();
		ID3D11ShaderResourceView* empty[4]{};
		if(FAILED(m_pShader->Bind_Textures("g_SourceLutInputs",empty,4u)))result=E_FAIL;
	}
	if(FAILED(result))return result;
	output=std::move(staged);
	return S_OK;
}

HRESULT CRenderer::Apply_RenderQualitySettings(
	const RENDER_QUALITY_SETTINGS& Settings)
{
	if (!IsValidRenderQualitySettings(Settings))
		return E_INVALIDARG;

    // Load the optional program before committing any settings. A missing or
    // invalid program rejects the experiment while retaining the prior profile.
    if ((Settings.bSSGIEnabled || Settings.bSSREnabled) && !m_pScreenSpaceLightingShader)
    {
        auto stagedShader = CShader::Create(m_pDevice, m_pContext,
            TEXT("../Bin/ShaderFiles/Shader_ScreenSpaceLighting.hlsl"), VTXTEX::Elements, VTXTEX::iNumElements);
        if (!stagedShader) return E_FAIL;
        m_pScreenSpaceLightingShader = std::move(stagedShader);
    }

	ComPtr<ID3D11ShaderResourceView> stagedLut;
	if (Settings.SourcePostProcess.bEnabled)
	{
		for (auto entry = m_SourceGradingCache.begin(); entry != m_SourceGradingCache.end(); ++entry)
			if (entry->fGamma == Settings.fGamma && SameSourceGrading(entry->Settings, Settings.SourcePostProcess))
			{
				stagedLut = entry->pTexture;
				auto recent = *entry;
				m_SourceGradingCache.erase(entry);
				m_SourceGradingCache.push_back(std::move(recent));
				break;
			}
		if (!stagedLut)
		{
			const HRESULT result = Stage_SourceGradingLut(Settings.SourcePostProcess,
				Settings.fGamma, stagedLut);
			if (FAILED(result)) return result;
			// Keep the prior resource available for profile/region rollback. This
			// bounded cache retains immutable source pixels along with each key.
			m_SourceGradingCache.push_back({Settings.SourcePostProcess, Settings.fGamma, stagedLut});
			if (m_SourceGradingCache.size() > 8u) m_SourceGradingCache.erase(m_SourceGradingCache.begin());
		}
	}
	m_pSourceGradingLut = stagedLut;
	// Masked static casters sample the same surface mips as their color pass.
	if (m_RenderQualitySettings.iTextureMinMip != Settings.iTextureMinMip)
		m_bStaticShadowCacheValid = false;
	m_RenderQualitySettings = Settings;
	return S_OK;
}

HRESULT CRenderer::Bind_ActiveSceneColorTransform(CShader* shader) const
{
	if (nullptr == shader) return E_INVALIDARG;
	const auto& source = m_RenderQualitySettings.SourcePostProcess;
	const int32_t enabled = source.bEnabled ? 1 : 0;
	if (enabled != 0 && !m_pSourceGradingLut) return E_FAIL;
	const float4_t curve = PackSourceToneCurve(source);
	const float exposure = m_RenderQualitySettings.fExposure;
	// Render thread가 commit한 설정과 LUT를 함께 차용한다. 실패는 호출자의 draw를 중단하며,
	// source OFF도 매번 바인딩해 이전 occurrence의 활성 상태나 LUT가 남지 않게 한다.
	if (FAILED(shader->Bind_RawValue("g_iSourcePostProcessEnabled", &enabled, sizeof(enabled))) ||
		FAILED(shader->Bind_RawValue("g_vSourceToneCurve", &curve, sizeof(curve))) ||
		FAILED(shader->Bind_RawValue("g_fSourceToneToe", &source.fToneToe, sizeof(source.fToneToe))) ||
		FAILED(shader->Bind_RawValue("g_fToneMapExposure", &exposure, sizeof(exposure))) ||
		FAILED(shader->Bind_Texture("g_SourceGradingLut", m_pSourceGradingLut))) return E_FAIL;
	return S_OK;
}

namespace
{
	bool_t IsValidHeightFogSettings(const HEIGHT_FOG_SETTINGS& Settings)
	{
		const auto finite = [](const f32_t value) { return std::isfinite(value); };
		return finite(Settings.vColor.x) && finite(Settings.vColor.y) &&
			finite(Settings.vColor.z) && finite(Settings.vColor.w) &&
			Settings.vColor.x >= 0.f && Settings.vColor.y >= 0.f &&
			Settings.vColor.z >= 0.f &&
			finite(Settings.fDensity) && Settings.fDensity >= 0.f &&
			finite(Settings.fHeightFalloff) && Settings.fHeightFalloff > 0.f &&
			finite(Settings.fTopHeight) &&
			finite(Settings.fStartDistance) && Settings.fStartDistance >= 0.f &&
			finite(Settings.fMaximumOpacity) &&
			Settings.fMaximumOpacity >= 0.f && Settings.fMaximumOpacity <= 1.f &&
			finite(Settings.fDriftSpeed) && Settings.fDriftSpeed >= 0.f &&
			finite(Settings.fDriftHeightAmplitude) &&
			Settings.fDriftHeightAmplitude >= 0.f &&
			finite(Settings.fDriftDensityAmplitude) &&
			Settings.fDriftDensityAmplitude >= 0.f &&
			finite(Settings.fCoveragePercent) &&
			Settings.fCoveragePercent >= 0.f &&
			Settings.fCoveragePercent <= 1.f &&
			finite(Settings.fWindDirectionX) &&
			finite(Settings.fWindDirectionZ) &&
			finite(Settings.fWindSpeed) && Settings.fWindSpeed >= 0.f &&
			finite(Settings.fPatchScale) && Settings.fPatchScale > 0.f &&
			finite(Settings.fPatchSoftness) &&
			Settings.fPatchSoftness > 0.f &&
			Settings.fPatchSoftness <= 0.5f &&
            finite(Settings.vInscatteringColor.x) && Settings.vInscatteringColor.x >= 0.f &&
            finite(Settings.vInscatteringColor.y) && Settings.vInscatteringColor.y >= 0.f &&
            finite(Settings.vInscatteringColor.z) && Settings.vInscatteringColor.z >= 0.f &&
            finite(Settings.vFogLightDirection.x) && finite(Settings.vFogLightDirection.y) &&
            finite(Settings.vFogLightDirection.z) && finite(Settings.vFogLightDirection.w) &&
            Settings.vFogLightDirection.w >= -1.f && Settings.vFogLightDirection.w <= 1.f &&
            (!Settings.bSourceExponential ||
                Settings.vFogLightDirection.x * Settings.vFogLightDirection.x +
                Settings.vFogLightDirection.y * Settings.vFogLightDirection.y +
                Settings.vFogLightDirection.z * Settings.vFogLightDirection.z > 0.000001f);
	}
}

HRESULT CRenderer::Stage_RenderEnvironment(const wstring_t& cubePath,
    const float4_t& color, const float4_t& rotationIntensity,
    RENDER_ENVIRONMENT_STATE& outState, bool_t forceReload) const
{
    const float values[] = {color.x, color.y, color.z, color.w,
        rotationIntensity.x, rotationIntensity.y, rotationIntensity.z, rotationIntensity.w};
    for (float value : values) if (!std::isfinite(value)) return E_INVALIDARG;
    if (color.x < 0.f || color.y < 0.f || color.z < 0.f || color.w < 0.f ||
        color.x > 64.f || color.y > 64.f || color.z > 64.f || color.w > 64.f ||
        rotationIntensity.z < 0.f || rotationIntensity.z > 64.f ||
        rotationIntensity.w != 0.f ||
        std::abs(rotationIntensity.x * rotationIntensity.x +
            rotationIntensity.y * rotationIntensity.y - 1.f) > .001f) return E_INVALIDARG;
    RENDER_ENVIRONMENT_STATE staged;
    staged.strCubePath = cubePath;
    staged.vColor = color;
    staged.vRotationIntensity = rotationIntensity;
    if (!cubePath.empty() && !forceReload && cubePath == m_RenderEnvironment.strCubePath &&
        m_RenderEnvironment.pCube)
    {
        // Quality edits and mood changes in the same scene retain its staged cube.
        staged.pCube = m_RenderEnvironment.pCube;
    }
    else if (!cubePath.empty())
    {
        if (FAILED(CreateDDSTextureFromFileEx(m_pDevice.Get(), cubePath.c_str(), 0,
            D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0,
            DDS_LOADER_DEFAULT, nullptr, staged.pCube.GetAddressOf()))) return E_FAIL;
        D3D11_SHADER_RESOURCE_VIEW_DESC desc{};
        staged.pCube->GetDesc(&desc);
        if (desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURECUBE ||
            desc.TextureCube.MipLevels == 0u ||
            (desc.Format != DXGI_FORMAT_BC1_UNORM && desc.Format != DXGI_FORMAT_BC2_UNORM &&
             desc.Format != DXGI_FORMAT_BC3_UNORM && desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM &&
             desc.Format != DXGI_FORMAT_B8G8R8A8_UNORM))
            return E_INVALIDARG;
    }
    outState = std::move(staged);
    return S_OK;
}

void CRenderer::Commit_RenderEnvironment(const RENDER_ENVIRONMENT_STATE& state)
{
    m_RenderEnvironment = state;
}

RENDER_ENVIRONMENT_STATE CRenderer::Get_RenderEnvironment() const
{
    return m_RenderEnvironment;
}

HRESULT CRenderer::Bind_HeightFog(CShader* shader) const
{
    if (!shader) return E_INVALIDARG;
	const uint32_t iFogEnabled = m_HeightFogSettings.bEnabled ? 1u : 0u;
	const float2_t vFogWind(
		m_HeightFogSettings.fWindDirectionX,
		m_HeightFogSettings.fWindDirectionZ);
    const uint32_t sourceModel = m_HeightFogSettings.bSourceExponential ? 1u : 0u;
    if (FAILED(shader->Bind_RawValue("g_iSourceExponentialFog", &sourceModel, sizeof(sourceModel))) ||
        FAILED(shader->Bind_RawValue("g_vFogInscatteringColor", &m_HeightFogSettings.vInscatteringColor, sizeof(float4_t))) ||
        FAILED(shader->Bind_RawValue("g_vFogLightDirection", &m_HeightFogSettings.vFogLightDirection, sizeof(float4_t))) ||
		FAILED(shader->Bind_RawValue("g_iHeightFogEnabled",
			&iFogEnabled, sizeof(iFogEnabled))) ||
		FAILED(shader->Bind_RawValue("g_vHeightFogColor",
			&m_HeightFogSettings.vColor, sizeof(float4_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogDensity",
			&m_HeightFogSettings.fDensity, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogFalloff",
			&m_HeightFogSettings.fHeightFalloff, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogTopHeight",
			&m_HeightFogSettings.fTopHeight, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogStartDistance",
			&m_HeightFogSettings.fStartDistance, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogMaximumOpacity",
			&m_HeightFogSettings.fMaximumOpacity, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogDriftSpeed",
			&m_HeightFogSettings.fDriftSpeed, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogDriftHeight",
			&m_HeightFogSettings.fDriftHeightAmplitude, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fHeightFogDriftDensity",
			&m_HeightFogSettings.fDriftDensityAmplitude, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fFogCoverage",
			&m_HeightFogSettings.fCoveragePercent, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_vFogWindDirection",
			&vFogWind, sizeof(vFogWind))) ||
		FAILED(shader->Bind_RawValue("g_fFogWindSpeed",
			&m_HeightFogSettings.fWindSpeed, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fFogPatchScale",
			&m_HeightFogSettings.fPatchScale, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fFogPatchSoftness",
			&m_HeightFogSettings.fPatchSoftness, sizeof(f32_t))) ||
		FAILED(shader->Bind_RawValue("g_fPresentationClock",
			&m_fPresentationClock, sizeof(m_fPresentationClock))))
	{
		return E_FAIL;
	}
    return S_OK;
}

HRESULT CRenderer::Apply_HeightFog(const HEIGHT_FOG_SETTINGS& Settings)
{
	if (!IsValidHeightFogSettings(Settings))
		return E_INVALIDARG;

	m_HeightFogSettings = Settings;
	return S_OK;
}

void CRenderer::Advance_PresentationClock(f32_t fTimeDelta)
{
	if (!std::isfinite(fTimeDelta) || fTimeDelta < 0.f)
		return;

	/* Wrapping keeps the drift phase exact after long sessions instead of
	   letting float precision quantise the sine input. */
	constexpr f32_t CLOCK_WRAP_SECONDS = 3600.f;
	m_fPresentationClock = fmodf(
		m_fPresentationClock + fTimeDelta, CLOCK_WRAP_SECONDS);
}

HRESULT CRenderer::Draw(const bool staticOcclusionAllowed)
{
    m_bPickingDepthCaptured = false;
    // Priority sky and later forward water share this frame clock.
    CMaterial::Reset_SourceCharacterFrame(m_fPresentationClock);
	CPresentation_Manager& Presentation = CPresentation_Manager::Get();
	auto FailFrame = [this, &Presentation](
		const char* stage, const HRESULT hResult) -> HRESULT
	{
		WriteRendererFailure(stage, hResult);
        m_bPickingDepthCaptured = false;
        CMaterial::Reset_SourceCharacterFrame(m_fPresentationClock);
		m_bSceneColorSnapshotRequested = false;
        m_bSceneEnvironmentReplaced = false;
		Presentation.Clear_Frame();
		for (auto& RenderGroup : m_RenderObjects)
			RenderGroup.clear();
		return FAILED(hResult) ? hResult : E_FAIL;
	};
	CProfiler* const pProfiler = CGameInstance::Get().Get_Profiler();
	CProfilerScope drawScope(pProfiler, "Render.Draw");
	CProfilerGpuScope gpuDrawScope(pProfiler, "Render.Draw");
	HRESULT hResult = S_OK;
	{
		CProfilerScope scope(pProfiler, "Render.SubmitFrameProviders");
		hResult = Presentation.Submit_FrameProviders();
	}
	if (FAILED(hResult))
		return FailFrame("Submit_FrameProviders", hResult);
	const float2_t vViewportSize = CGameInstance::Get().Get_ViewportSize();
	if (vViewportSize.x <= 0.f || vViewportSize.y <= 0.f)
	{
		return FailFrame("Viewport", E_INVALIDARG);
	}
	hResult = Ready_ScenePostTargets(
		static_cast<uint32_t>(vViewportSize.x),
		static_cast<uint32_t>(vViewportSize.y));
	if (FAILED(hResult))
		return FailFrame("Ready_ScenePostTargets", hResult);
	{
		CProfilerScope scope(pProfiler, "Render.FinalCameraSubmission");
		CGameInstance::Get().Submit_FinalCameraObjects();
	}
	{
		CProfilerScope scope(pProfiler, "Render.Shadow");
		CProfilerGpuScope gpuScope(pProfiler, "Render.Shadow", true);
		hResult = Render_Shadow();
	}
	if (FAILED(hResult))
		return FailFrame("Render_Shadow", hResult);
	/* Portraits borrow the world G-buffer before the world fills it: Begin_MRT clears every
	target, so Render_NonBlend below starts from a clean sheet whatever a portrait wrote. A
	portrait failure is isolated -- the queue is dropped and the world frame continues. */
	{
		CProfilerScope scope(pProfiler, "Render.Portraits");
		CProfilerGpuScope gpuScope(pProfiler, "Render.Portraits", true);
		if (FAILED(Render_Portraits()))
			m_PortraitRequests.clear();
	}
	{
		CProfilerScope scope(pProfiler, "Render.NonBlend");
		CProfilerGpuScope gpuScope(pProfiler, "Render.NonBlend", true);
		hResult = Render_NonBlend(staticOcclusionAllowed);
	}
	if (FAILED(hResult))
		return FailFrame("Render_NonBlend", hResult);
	// Preserve only opaque depth before forward effects or UI can write to it.
	hResult = Capture_PickingDepth();
	if (FAILED(hResult))
		return FailFrame("Capture_PickingDepth", hResult);
	if (m_RenderQualitySettings.bSSAOEnabled)
	{
		CProfilerScope scope(pProfiler, "Render.SSAO");
		CProfilerGpuScope gpuScope(pProfiler, "Render.SSAO", true);
		hResult = Render_SSAO();
		if (FAILED(hResult))
			return FailFrame("Render_SSAO", hResult);
	}
	{
		CProfilerScope scope(pProfiler, "Render.Lights");
		CProfilerGpuScope gpuScope(pProfiler, "Render.Lights", true);
		hResult = Render_Lights();
	}
	if (FAILED(hResult))
		return FailFrame("Render_Lights", hResult);

	/* Scene colour is accumulated in FP16 so lighting and effect values above 1 */
	/* survive until tone mapping. Sky/background join the same target, or the   */
	/* final blit would overwrite them with black.                               */
	hResult = CGameInstance::Get().Begin_MRT(TEXT("MRT_SceneHDR"));
	if (FAILED(hResult))
		return FailFrame("Begin_MRT_SceneHDR", hResult);
	HRESULT hSceneResult = S_OK;
	HRESULT hEndSceneResult = S_OK;
	{
		CRenderOutputContractScope SceneOutputScope(
			RENDER_OUTPUT_CONTRACT::
			SCENE_HDR_RT0_SCENE_COLOR_RT1_DISTORTION,
			m_pContext.Get());
		CProfilerScope scope(pProfiler, "Render.SceneHDR");
		CProfilerGpuScope gpuScope(pProfiler, "Render.SceneHDR");
		hSceneResult = Render_Priority();
		if (SUCCEEDED(hSceneResult))
			hSceneResult = Render_Combined();
        if (SUCCEEDED(hSceneResult) && (m_RenderQualitySettings.bSSGIEnabled || m_RenderQualitySettings.bSSREnabled))
            hSceneResult = Render_ScreenSpaceLighting();
		if (SUCCEEDED(hSceneResult))
			hSceneResult = Render_NonLight();
		if (SUCCEEDED(hSceneResult))
			hSceneResult = Render_SceneReplacements();
		if (SUCCEEDED(hSceneResult) && m_bSceneColorSnapshotRequested)
		{
			hSceneResult = Capture_SceneColorSnapshot();
		}
		if (SUCCEEDED(hSceneResult))
			hSceneResult = Render_Blend();

		// Opt-in cinematic UI participates in scene post effects (e.g. glass).
		// Ordinary HUD/UI stays in display space after the final composite.
		for (const auto& object : m_RenderObjects[ETOUI(RENDERGROUP::SCENE_UI)])
            if (object && SUCCEEDED(hSceneResult))
            {
                // Presentation UI is optional; a failed sprite cannot discard the world frame.
                const HRESULT uiResult = object->Render_Group(RENDERGROUP::SCENE_UI);
                if (FAILED(uiResult)) WriteRendererFailure("Render_SceneUI_Isolated", uiResult);
            }
		m_RenderObjects[ETOUI(RENDERGROUP::SCENE_UI)].clear();

		/* Always restore the back-buffer/DSV pair after entering the HDR MRT. */
		hEndSceneResult = CGameInstance::Get().End_MRT();
	}
	if (FAILED(hSceneResult) || FAILED(hEndSceneResult))
		return FailFrame(
			FAILED(hSceneResult) ? "Render_Scene" : "End_MRT_SceneHDR",
			FAILED(hSceneResult) ? hSceneResult : hEndSceneResult);

	{
		CProfilerScope scope(pProfiler, "Render.ScreenPosts");
		CProfilerGpuScope gpuScope(pProfiler, "Render.ScreenPosts");
		hResult = Render_ScreenPosts();
	}
	if (FAILED(hResult))
		return FailFrame("Render_ScreenPosts", hResult);

	if (m_RenderQualitySettings.bBloomEnabled)
	{
		CProfilerScope scope(pProfiler, "Render.Bloom");
		CProfilerGpuScope gpuScope(pProfiler, "Render.Bloom");
		hResult = Render_Bloom();
		if (FAILED(hResult))
			return FailFrame("Render_Bloom", hResult);
	}

	/* The one and only place tone mapping and gamma are applied. */
	{
		CProfilerScope scope(pProfiler, "Render.Final");
		CProfilerGpuScope gpuScope(pProfiler, "Render.Final");
		hResult = Render_Final();
	}
	if (FAILED(hResult))
		return FailFrame("Render_Final", hResult);

	{
		CProfilerScope scope(pProfiler, "Render.DisplayOverlays");
		CProfilerGpuScope gpuScope(pProfiler, "Render.DisplayOverlays");
		hResult = Render_DisplayOverlays();
	}
	if (FAILED(hResult))
		return FailFrame("Render_DisplayOverlays", hResult);

	/* UI is authored in display space, so it stays out of the HDR target. */
	{
		CProfilerScope scope(pProfiler, "Render.UI");
		CProfilerGpuScope gpuScope(pProfiler, "Render.UI", true);
		hResult = Render_UI();
	}
	if (FAILED(hResult))
		return FailFrame("Render_UI", hResult);

	{
		CProfilerScope scope(pProfiler, "Render.Debug");
		CProfilerGpuScope gpuScope(pProfiler, "Render.Debug");
		hResult = Render_Debug();
	}
	if (FAILED(hResult))
		return FailFrame("Render_Debug", hResult);

	// PickPos also carries deferred surface data. All visual consumers must finish
	// before forward-only receivers replace its positions for the next input frame.
	{
		CProfilerScope scope(pProfiler, "Render.Picking");
		CProfilerGpuScope gpuScope(pProfiler, "Render.Picking");
		hResult = Render_Picking();
	}
	if (FAILED(hResult))
		return FailFrame("Render_Picking", hResult);

	m_bSceneColorSnapshotRequested = false;
    m_bSceneEnvironmentReplaced = false;
	Presentation.Clear_Frame();
	return S_OK;
}


HRESULT CRenderer::Add_DebugComponent(shared_ptr<CComponent> pDebugComponent)
{
	m_DebugComponent.push_back(pDebugComponent);
	return S_OK;
}


HRESULT CRenderer::Render_Priority()
{
    CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope cpuScope(profiler, "Render.Priority");
    CProfilerGpuScope gpuScope(profiler, "Render.Priority");
	// Index iteration permits callbacks to append without retaining vector
	// iterators or element references; the queue still owns every object.
	for (size_t renderIndex = 0; renderIndex < m_RenderObjects[ETOUI(RENDERGROUP::PRIORITY)].size(); ++renderIndex)
	{
		CGameObject* const pRenderObject = m_RenderObjects[ETOUI(RENDERGROUP::PRIORITY)][renderIndex].get();
		if (nullptr != pRenderObject)
			pRenderObject->Render_Group(RENDERGROUP::PRIORITY);
	}

	m_RenderObjects[ETOUI(RENDERGROUP::PRIORITY)].clear();

	return S_OK;
}

HRESULT CRenderer::Render_Shadow()
{
    auto& objects = m_RenderObjects[ETOUI(RENDERGROUP::SHADOW)];
    if (!m_pShadowDSV || !m_pShadowSRV || !m_pShadowDepthTexture || !m_iShadowMapSize)
    {
        m_bStaticShadowCacheValid = false;
        objects.clear();
        return E_FAIL;
    }

    auto& game = CGameInstance::Get();
    const auto* view = game.Get_ShadowLightTransform(D3DTS::VIEW);
    const auto* projection = game.Get_ShadowLightTransform(D3DTS::PROJ);
    const bool enabled = game.Is_ShadowLightEnabled();
    const bool sourceMaterials = m_MaterialRenderSettings.bUseSourceMaterials;
    const bool allowCache = m_RenderOptimization.StaticShadowCacheEnabled && enabled && view && projection &&
        m_pStaticShadowDepthTexture && !m_bSceneEnvironmentReplaced;
    m_CandidateStaticShadowCasters.clear();
    m_StaticShadowFlags.assign(objects.size(), 0u);
    if (allowCache)
    {
        CProfilerScope scope(game.Get_Profiler(), "Render.Shadow.CacheAdmission");
        for (size_t i = 0; i < objects.size(); ++i)
        {
            uint64_t revision = 0u;
            if (objects[i] && objects[i]->Try_GetStaticShadowRevision(revision) && revision)
            {
                m_StaticShadowFlags[i] = 1u;
                m_CandidateStaticShadowCasters.push_back({objects[i], objects[i].get(), revision});
            }
        }
    }
    const bool hasStatic = !m_CandidateStaticShadowCasters.empty();
    bool reuse = hasStatic && m_bStaticShadowCacheValid &&
        sourceMaterials == m_bStaticShadowSourceMaterials &&
        0 == std::memcmp(view, &m_StaticShadowView, sizeof(*view)) &&
        0 == std::memcmp(projection, &m_StaticShadowProjection, sizeof(*projection)) &&
        m_CandidateStaticShadowCasters.size() == m_StaticShadowCasters.size();
    for (size_t i = 0; reuse && i < m_StaticShadowCasters.size(); ++i)
    {
        const auto& previous = m_StaticShadowCasters[i];
        const auto& current = m_CandidateStaticShadowCasters[i];
        reuse = previous.Identity == current.Identity && previous.Revision == current.Revision &&
            !previous.Owner.owner_before(current.Owner) && !current.Owner.owner_before(previous.Owner);
    }
    if (enabled)
    {
        if (CProfiler* profiler = game.Get_Profiler())
        {
            profiler->Add_Counter(reuse ? EProfilerCounter::ShadowCacheHits : EProfilerCounter::ShadowCacheMisses);
            profiler->Add_Counter(EProfilerCounter::ShadowStaticCasters, m_CandidateStaticShadowCasters.size());
            profiler->Add_Counter(EProfilerCounter::ShadowDynamicCasters, objects.size() - m_CandidateStaticShadowCasters.size());
        }
    }
    if (!reuse) m_bStaticShadowCacheValid = false;
    if (!hasStatic) m_StaticShadowCasters.clear();

    D3D11_VIEWPORT originalViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
    UINT viewportCount = _countof(originalViewports);
    m_pContext->RSGetViewports(&viewportCount, originalViewports);
    if (FAILED(game.Begin_DepthOnly(m_pShadowDSV)))
    {
        m_bStaticShadowCacheValid = false;
        objects.clear();
        return E_FAIL;
    }

    // Keep the existing target manager's saved output pair. Only unbind the
    // depth target for CopyResource; beginning another pass would clear it.
    const auto copyDepth = [&](ID3D11Texture2D* destination, ID3D11Texture2D* source)
    {
        CProfilerScope scope(game.Get_Profiler(), "Render.Shadow.CacheCopy");
        CProfilerGpuScope gpuScope(game.Get_Profiler(), "Render.Shadow.CacheCopy");
        m_pContext->OMSetRenderTargets(0u, nullptr, nullptr);
        m_pContext->CopyResource(destination, source);
        m_pContext->OMSetRenderTargets(0u, nullptr, m_pShadowDSV.Get());
    };
    const auto drawObjects = [&](bool staticPass) -> HRESULT
    {
        for (size_t i = 0; i < objects.size(); ++i)
        {
            const bool isStatic = i < m_StaticShadowFlags.size() && m_StaticShadowFlags[i] != 0u;
            if (isStatic != staticPass) continue;
            // Hold this object across a callback that may append to the queue.
            const auto object = objects[i];
            if (object && FAILED(object->Render_Shadow()))
            {
                WriteRendererFailure("Render_Shadow_Object", E_FAIL, typeid(*object).name());
                return E_FAIL;
            }
        }
        return S_OK;
    };

    HRESULT result = S_OK;
    if (enabled)
    {
        SetUp_ViewportDesc(m_iShadowMapSize, m_iShadowMapSize);
        if (reuse)
            copyDepth(m_pShadowDepthTexture.Get(), m_pStaticShadowDepthTexture.Get());
        else if (hasStatic)
        {
            CProfilerScope scope(game.Get_Profiler(), "Render.Shadow.StaticBuild");
            CProfilerGpuScope gpuScope(game.Get_Profiler(), "Render.Shadow.StaticBuild");
            result = drawObjects(true);
            if (SUCCEEDED(result))
            {
                copyDepth(m_pStaticShadowDepthTexture.Get(), m_pShadowDepthTexture.Get());
                m_StaticShadowCasters.swap(m_CandidateStaticShadowCasters);
                m_StaticShadowView = *view;
                m_StaticShadowProjection = *projection;
                m_bStaticShadowSourceMaterials = sourceMaterials;
                m_bStaticShadowCacheValid = true;
            }
        }
        if (SUCCEEDED(result))
        {
            CProfilerScope scope(game.Get_Profiler(), "Render.Shadow.Dynamic");
            CProfilerGpuScope gpuScope(game.Get_Profiler(), "Render.Shadow.Dynamic");
            result = drawObjects(false);
        }
    }
    objects.clear();
    const HRESULT endResult = game.End_DepthOnly();
    m_pContext->RSSetViewports(viewportCount, viewportCount ? originalViewports : nullptr);
    if (FAILED(result) || FAILED(endResult))
    {
        m_bStaticShadowCacheValid = false;
        return E_FAIL;
    }
    return S_OK;
}

HRESULT CRenderer::Apply_RenderOptimizationSettings(const RENDER_OPTIMIZATION_SETTINGS& settings)
{
    if (settings.FrustumEnabled == m_RenderOptimization.FrustumEnabled &&
        settings.MeshLodEnabled == m_RenderOptimization.MeshLodEnabled &&
        settings.MapInstancingEnabled == m_RenderOptimization.MapInstancingEnabled &&
        settings.IdenticalBatchEnabled == m_RenderOptimization.IdenticalBatchEnabled &&
        settings.LightingBankEnabled == m_RenderOptimization.LightingBankEnabled &&
        settings.StaticShadowCacheEnabled == m_RenderOptimization.StaticShadowCacheEnabled &&
        settings.NpcPoseReuseEnabled == m_RenderOptimization.NpcPoseReuseEnabled &&
        settings.ParticleRootCacheEnabled == m_RenderOptimization.ParticleRootCacheEnabled &&
        settings.ParticleWorkersEnabled == m_RenderOptimization.ParticleWorkersEnabled) return S_OK;
    // The caller's revision is not an authority. Equal repeated applications
    // must preserve warm caches, including profile preview's per-frame update.
    const uint64_t revision = m_RenderOptimization.Revision + 1u;
    m_RenderOptimization = settings;
    m_RenderOptimization.Revision = revision ? revision : 1u;
    m_OcclusionCacheValid = false;
    m_bStaticShadowCacheValid = false;
    return S_OK;
}

HRESULT CRenderer::Apply_MapVisibilitySettings(const MAP_VISIBILITY_SETTINGS& settings)
{
    if (!std::isfinite(settings.DistanceScale) || settings.DistanceScale < .25f || settings.DistanceScale > 4.f ||
        !std::isfinite(settings.DistanceMaxPixels) || settings.DistanceMaxPixels < 4.f || settings.DistanceMaxPixels > 128.f)
        return E_INVALIDARG;
    if (settings.GpuVisibilityEnabled == m_MapVisibility.GpuVisibilityEnabled &&
        settings.GpuHiZEnabled == m_MapVisibility.GpuHiZEnabled &&
        settings.OcclusionEnabled == m_MapVisibility.OcclusionEnabled &&
        settings.DistanceEnabled == m_MapVisibility.DistanceEnabled &&
        settings.ParallelPreparationEnabled == m_MapVisibility.ParallelPreparationEnabled && settings.DistanceScale == m_MapVisibility.DistanceScale &&
        settings.DistanceMaxPixels == m_MapVisibility.DistanceMaxPixels) return S_OK;
    const uint64_t revision = m_MapVisibility.Revision + 1u;
    m_MapVisibility = settings;
    m_MapVisibility.Revision = revision ? revision : 1u;
    return S_OK;
}

void CRenderer::Cull_StaticOcclusion()
{
    auto& game = CGameInstance::Get();
    auto& objects = m_RenderObjects[ETOUI(RENDERGROUP::NONBLEND)];
    if (!m_MapVisibility.OcclusionEnabled || m_bSceneEnvironmentReplaced || objects.empty()) return;
    const auto* view = game.Get_Transform(D3DTS::VIEW);
    const auto* projection = game.Get_Transform(D3DTS::PROJ);
    const auto viewport = game.Get_ViewportSize();
    if (!view || !projection || viewport.x <= 0.f || viewport.y <= 0.f) return;
    CProfiler* profiler = game.Get_Profiler();
    CProfilerScope scope(profiler, "Render.MapOcclusion");
    try
    {
        m_OcclusionCandidates.clear(); m_OccluderOrder.clear();
        m_OcclusionHidden.assign(objects.size(), 0u);
        m_OcclusionVisibleQueue.clear();
        for (size_t i = 0; i < objects.size(); ++i)
        {
            if (!objects[i]) continue;
            CGameObject::STATIC_OCCLUSION_DESC desc{};
            if (!objects[i]->Try_GetStaticOcclusionDesc(desc) || !desc.Draws || !desc.Indices || !desc.Revision) continue;
            const vector_t minimum = XMLoadFloat3(&desc.BoundsMin), maximum = XMLoadFloat3(&desc.BoundsMax);
            if (XMVector3IsNaN(minimum) || XMVector3IsInfinite(minimum) || XMVector3IsNaN(maximum) ||
                XMVector3IsInfinite(maximum) || !XMVector3LessOrEqual(minimum, maximum)) continue;
            float3_t center{};
            XMStoreFloat3(&center, XMVector3TransformCoord((minimum + maximum) * .5f, XMLoadFloat4x4(view)));
            const float radius = XMVectorGetX(XMVector3Length(maximum - minimum)) * .5f;
            const float depth = (std::max)(.1f, center.z - radius);
            const float area = radius * radius * projection->_11 * projection->_22 / (depth * depth);
            const size_t slot = m_OcclusionCandidates.size();
            m_OcclusionCandidates.push_back({i, desc, area / std::sqrt(float((std::max)(1u, desc.OccluderTriangles)))});
            // Tiny screen coverage cannot repay rasterization overhead. Large
            // near-plane envelopes remain safe: the kernel clips/rejects them.
            if (desc.OccluderTriangles && std::isfinite(area) && area >= .002f) m_OccluderOrder.push_back(slot);
        }
        if (profiler) profiler->Add_Counter(EProfilerCounter::MapOcclusionCandidates, m_OcclusionCandidates.size());
        if (m_OcclusionCandidates.size() < 8u || m_OccluderOrder.empty()) return;
        bool reuse = m_OcclusionCacheValid && m_OcclusionCache.size() == m_OcclusionCandidates.size() &&
            m_OcclusionSettingsRevision == m_MapVisibility.Revision &&
            !std::memcmp(view, &m_OcclusionView, sizeof(*view)) && !std::memcmp(projection, &m_OcclusionProjection, sizeof(*projection)) &&
            viewport.x == m_OcclusionViewport.x && viewport.y == m_OcclusionViewport.y;
        for (size_t i = 0; reuse && i < m_OcclusionCandidates.size(); ++i)
        {
            const auto& candidate = m_OcclusionCandidates[i];
            const auto& previous = m_OcclusionCache[i];
            const auto& owner = objects[candidate.Index];
            reuse = !previous.Owner.owner_before(owner) && !owner.owner_before(previous.Owner) &&
                !std::memcmp(&candidate.Desc, &previous.Desc, sizeof(candidate.Desc));
        }
        if (reuse)
        {
            if (profiler) profiler->Add_Counter(EProfilerCounter::MapOcclusionCacheHits);
            for (size_t i = 0; i < m_OcclusionCandidates.size(); ++i)
                if (m_OcclusionCache[i].Hidden) m_OcclusionHidden[m_OcclusionCandidates[i].Index] = 1u;
        }
        else
        {
            m_OcclusionCacheValid = false;
            if (!m_OcclusionCuller) m_OcclusionCuller = std::make_unique<COcclusionCuller>();
            if (!m_OcclusionCuller->Begin(*view, *projection, uint32_t(viewport.x), uint32_t(viewport.y))) return;
            std::stable_sort(m_OccluderOrder.begin(), m_OccluderOrder.end(), [&](size_t a, size_t b) {
                return m_OcclusionCandidates[a].Score > m_OcclusionCandidates[b].Score;
            });
            constexpr uint32_t triangleBudget = 100000u, maximumOccluders = 64u;
            const auto started = std::chrono::steady_clock::now();
            uint32_t occluders = 0u;
            {
                CProfilerScope rasterScope(profiler, "Render.MapOcclusion.Rasterize");
                for (const size_t slot : m_OccluderOrder)
                {
                    if (occluders >= maximumOccluders || m_OcclusionCuller->GetInputTriangles() >= triangleBudget) break;
                    // A bounded partial depth buffer is always safe; it can only
                    // miss opportunities. Never carry an old camera's hidden set.
                    if (std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count() > 1.5) break;
                    const auto& candidate = m_OcclusionCandidates[slot];
                    const auto count = objects[candidate.Index]->Rasterize_StaticOccluder(*m_OcclusionCuller,
                        (std::min)(16384u, triangleBudget - m_OcclusionCuller->GetInputTriangles()));
                    if (count) { ++occluders; m_OcclusionHidden[candidate.Index] = 2u; }
                }
            }
            if (profiler)
            {
                profiler->Add_Counter(EProfilerCounter::MapOcclusionOccluders, occluders);
                profiler->Add_Counter(EProfilerCounter::MapOcclusionRasterizedTriangles, m_OcclusionCuller->GetRasterizedTriangles());
            }
            // Cache a visible-only result too when the same camera produced no depth.
        }
        uint64_t rejected = 0u, draws = 0u, indices = 0u, tested = 0u;
        {
            CProfilerScope queryScope(profiler, "Render.MapOcclusion.Test");
            for (const auto& candidate : m_OcclusionCandidates)
            {
                if (m_OcclusionHidden[candidate.Index] == 2u) continue;
                if (!reuse) ++tested;
                if (reuse ? m_OcclusionHidden[candidate.Index] != 1u :
                    !m_OcclusionCuller->TestBounds(candidate.Desc.BoundsMin, candidate.Desc.BoundsMax)) continue;
                m_OcclusionHidden[candidate.Index] = 1u;
                ++rejected; draws += candidate.Desc.Draws; indices += candidate.Desc.Indices;
            }
        }
        if (profiler)
        {
            profiler->Add_Counter(EProfilerCounter::MapOcclusionTested, tested);
        }
        if (!reuse)
        {
            m_OcclusionCache.clear(); m_OcclusionCache.reserve(m_OcclusionCandidates.size());
            for (const auto& candidate : m_OcclusionCandidates)
                m_OcclusionCache.push_back({objects[candidate.Index], candidate.Desc, m_OcclusionHidden[candidate.Index] == 1u});
            m_OcclusionView = *view; m_OcclusionProjection = *projection; m_OcclusionViewport = viewport;
            m_OcclusionSettingsRevision = m_MapVisibility.Revision;
            m_OcclusionCacheValid = true;
        }
        if (!rejected) return;
        // Commit one stable queue after every test. Original shadow/forward
        // queues and owners survive, and adjacent instance batching still runs.
        m_OcclusionVisibleQueue.reserve(objects.size() - size_t(rejected));
        for (size_t i = 0; i < objects.size(); ++i)
            if (m_OcclusionHidden[i] != 1u) m_OcclusionVisibleQueue.push_back(objects[i]);
        objects.swap(m_OcclusionVisibleQueue);
        m_OcclusionVisibleQueue.clear();
        if (profiler)
        {
            profiler->Add_Counter(EProfilerCounter::MapOcclusionRejectedBatches, rejected);
            profiler->Add_Counter(EProfilerCounter::MapOcclusionSourceDraws, draws);
            profiler->Add_Counter(EProfilerCounter::MapOcclusionRejectedIndices, indices);
        }
    }
    catch (...) { m_OcclusionVisibleQueue.clear(); } // Keep the original queue on preparation failure.
}

void CRenderer::Register_GpuSceneVisibility(const shared_ptr<CGpuSceneVisibility>& scene)
{
    if (!scene) return;
    for (auto it = m_GpuScenes.begin(); it != m_GpuScenes.end();)
    {
        auto existing = it->lock();
        if (!existing) it = m_GpuScenes.erase(it);
        else { if (existing == scene) return; ++it; }
    }
    m_GpuScenes.emplace_back(scene);
}

void CRenderer::Unregister_GpuSceneVisibility(const CGpuSceneVisibility* scene)
{
    std::erase_if(m_GpuScenes, [scene](const auto& weak)
    { const auto locked = weak.lock(); return !locked || locked.get() == scene; });
}

bool CRenderer::Prepare_GpuSceneVisibility(bool staticOcclusionAllowed)
{
    if (m_GpuScenes.empty()) return false;
    auto& game = CGameInstance::Get();
    CProfiler* profiler = game.Get_Profiler();
    CProfilerScope scope(profiler, "Render.GpuScene.Prepare");
    std::vector<shared_ptr<CGpuSceneVisibility>> scenes;
    for (auto it = m_GpuScenes.begin(); it != m_GpuScenes.end();)
    {
        if (auto scene = it->lock()) { scene->Invalidate_Frame(); scenes.push_back(std::move(scene)); ++it; }
        else it = m_GpuScenes.erase(it);
    }
    if (!m_MapVisibility.GpuVisibilityEnabled) return false;
    const auto* view = game.Get_Transform(D3DTS::VIEW);
    const auto* projection = game.Get_Transform(D3DTS::PROJ);
    const auto* camera = game.Get_CamPosition();
    const auto viewport = game.Get_ViewportSize();
    if (!view || !projection || !camera || viewport.x <= 0 || viewport.y <= 0) return false;
    CGpuSceneVisibility::FRAME frame;
    frame.View = *view; frame.Projection = *projection;
    frame.Camera = {camera->x,camera->y,camera->z};
    frame.Width = static_cast<uint32_t>(viewport.x); frame.Height = static_cast<uint32_t>(viewport.y);
    if (++m_GpuVisibilityFrame == 0u) ++m_GpuVisibilityFrame;
    frame.FrameId = m_GpuVisibilityFrame;
    frame.DistanceEnabled = m_MapVisibility.DistanceEnabled;
    frame.DistanceScale = m_MapVisibility.DistanceScale;
    frame.MeshLodEnabled = m_RenderOptimization.MeshLodEnabled;
    frame.FrustumEnabled = m_RenderOptimization.FrustumEnabled;
    frame.HiZEnabled = staticOcclusionAllowed && m_MapVisibility.OcclusionEnabled && m_MapVisibility.GpuHiZEnabled;
    std::erase_if(scenes,[&frame](const auto& scene) { return scene->Prepare_Frame(frame) != S_OK; });
    if (scenes.empty()) return false;

    ID3D11ShaderResourceView* depth = nullptr;
    uint32_t mipCount = 0u;
    uint64_t sourceCount = 0u, binCount = 0u;
    for (const auto& scene : scenes)
    { sourceCount += scene->Get_Statistics().SourceInstances; binCount += scene->Get_Statistics().Bins; }
    // 작은 작업에는 깊이 prepass 자체가 더 비싸므로 장면 크기와 실제 가림자 예산을 함께 제한한다.
    if (frame.HiZEnabled && sourceCount >= 128u && binCount >= 8u)
    {
        CProfilerScope depthScope(profiler,"Render.GpuScene.StaticDepthHiZ");
        CProfilerGpuScope depthGpuScope(profiler,"Render.GpuScene.StaticDepthHiZ",true);
        if (!m_GpuHiZ && !m_GpuHiZUnavailable)
        {
            auto candidate = std::make_unique<CGpuVisibilityHiZ>(m_pDevice.Get(),m_pContext.Get());
            if (SUCCEEDED(candidate->Initialize())) m_GpuHiZ = std::move(candidate);
            else m_GpuHiZUnavailable = true;
        }
        struct Candidate { shared_ptr<CGameObject> Object; CGameObject::STATIC_OCCLUSION_DESC Desc; float Score; };
        std::vector<Candidate> candidates;
        if (m_GpuHiZ)
        {
            for (const auto& object : m_RenderObjects[ETOUI(RENDERGROUP::NONBLEND)])
            {
                CGameObject::STATIC_OCCLUSION_DESC desc{};
                if (!object || !object->Try_GetVisibilityDepthDesc(desc) || !desc.OccluderTriangles || !desc.Draws || desc.Draws > 32u) continue;
                const float3_t center{(desc.BoundsMin.x+desc.BoundsMax.x)*.5f,(desc.BoundsMin.y+desc.BoundsMax.y)*.5f,(desc.BoundsMin.z+desc.BoundsMax.z)*.5f};
                const float dx = desc.BoundsMax.x-desc.BoundsMin.x, dy = desc.BoundsMax.y-desc.BoundsMin.y, dz = desc.BoundsMax.z-desc.BoundsMin.z;
                const float ex = center.x-camera->x, ey = center.y-camera->y, ez = center.z-camera->z;
                const float score = (dx*dx+dy*dy+dz*dz)/std::max(1.f,ex*ex+ey*ey+ez*ez)/std::sqrt(static_cast<float>(desc.OccluderTriangles));
                if (std::isfinite(score) && score > 0.f) candidates.push_back({object,desc,score});
            }
            std::stable_sort(candidates.begin(),candidates.end(),[](const auto& a,const auto& b) { return a.Score > b.Score; });
            if (!candidates.empty() && SUCCEEDED(m_GpuHiZ->Begin_Depth(frame.Width,frame.Height)))
            {
                uint32_t triangles = 0u, draws = 0u;
                bool valid = true;
                const auto begin = std::chrono::steady_clock::now();
                for (const auto& candidate : candidates)
                {
                    if (draws+candidate.Desc.Draws > 32u || triangles >= 80000u) continue;
                    uint32_t drawn = 0u;
                    const HRESULT hr = candidate.Object->Render_VisibilityDepth(80000u-triangles,drawn);
                    if (FAILED(hr) || drawn > 80000u-triangles) { valid = false; break; }
                    if (hr == S_OK && drawn) { triangles += drawn; draws += candidate.Desc.Draws; }
                    if (std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count() >= 2.) break;
                }
                if (m_GpuHiZ->End_Depth(valid && triangles != 0u) == S_OK)
                { depth = m_GpuHiZ->Get_SRV(); mipCount = m_GpuHiZ->Get_MipCount(); }
            }
        }
    }
    bool dispatched = false;
    {
        CProfilerScope dispatchScope(profiler,"Render.GpuScene.Dispatch");
        CProfilerGpuScope dispatchGpuScope(profiler,"Render.GpuScene.Dispatch");
        for (const auto& scene : scenes)
        {
            if (scene->Dispatch(frame,depth,mipCount) == S_OK) dispatched = true;
            else scene->Invalidate_Frame();
        }
    }
    return dispatched;
}

HRESULT CRenderer::Render_NonBlend(const bool staticOcclusionAllowed)
{
    // GPU 깊이와 CPU MOC를 같은 프레임에 중복 준비하지 않는다. GPU 실패는 기존 경로로 돌아간다.
    if (!Prepare_GpuSceneVisibility(staticOcclusionAllowed) && staticOcclusionAllowed) Cull_StaticOcclusion();
    CMaterial::Reset_SourceCharacterFrame(m_fPresentationClock);
	/* Diffuse + Normal */
	if (FAILED(CGameInstance::Get().Begin_MRT(TEXT("MRT_GameObject"))))
		return E_FAIL;

	auto& NonBlendObjects =
		m_RenderObjects[ETOUI(RENDERGROUP::NONBLEND)];
	auto& DeferredOverlayObjects =
		m_RenderObjects[ETOUI(RENDERGROUP::DEFERRED_OVERLAY)];
	for (size_t renderIndex = 0; renderIndex < NonBlendObjects.size();)
	{
		CGameObject* const pRenderObject = NonBlendObjects[renderIndex].get();
		if (nullptr == pRenderObject) { ++renderIndex; continue; }
		const auto remaining = std::span<const std::shared_ptr<CGameObject>>(
			NonBlendObjects.data() + renderIndex, NonBlendObjects.size() - renderIndex);
		size_t consumed = 0u;
		const HRESULT result = pRenderObject->Render_AdjacentNonBlend(remaining, consumed);
		if (FAILED(result) || consumed == 0u || consumed > remaining.size())
		{
			NonBlendObjects.clear();
			DeferredOverlayObjects.clear();
			(void)CGameInstance::Get().End_MRT();
			return FAILED(result) ? result : E_UNEXPECTED;
		}
		renderIndex += consumed;
	}

	/* Deferred overlays must run after every opaque object while the complete
	   game-object MRT, including Target_Emissive, is still bound. */
	for (size_t renderIndex = 0; renderIndex < DeferredOverlayObjects.size(); ++renderIndex)
	{
		CGameObject* const pRenderObject = DeferredOverlayObjects[renderIndex].get();
		if (nullptr != pRenderObject)
			pRenderObject->Render_DeferredOverlay();
	}

	NonBlendObjects.clear();
	DeferredOverlayObjects.clear();

	if (FAILED(CGameInstance::Get().End_MRT()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_SSAO()
{
	if (nullptr == m_pSSAODSV || 0u == m_iSSAOWidth || 0u == m_iSSAOHeight)
		return E_FAIL;

	D3D11_VIEWPORT OriginalViewports[
		D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
	UINT iViewportCount = _countof(OriginalViewports);
	m_pContext->RSGetViewports(&iViewportCount, OriginalViewports);
	SetUp_ViewportDesc(m_iSSAOWidth, m_iSSAOHeight);

	HRESULT hResult = Render_SSAOPass(
		TEXT("MRT_SSAORaw"), DEFERRED::SSAO_RAW);
	if (SUCCEEDED(hResult))
	{
		hResult = Render_SSAOPass(
			TEXT("MRT_SSAOBlur"), DEFERRED::SSAO_BLUR);
	}

	if (0u < iViewportCount)
		m_pContext->RSSetViewports(iViewportCount, OriginalViewports);
	else
		m_pContext->RSSetViewports(0u, nullptr);

	return hResult;
}

HRESULT CRenderer::Render_SSAOPass(
	const wstring_t& strMRTTag, const DEFERRED ePass)
{
	if (DEFERRED::SSAO_RAW != ePass && DEFERRED::SSAO_BLUR != ePass)
		return E_INVALIDARG;
	if (FAILED(CGameInstance::Get().Begin_MRT(strMRTTag, m_pSSAODSV)))
		return E_FAIL;

	HRESULT hResult = S_OK;
	HRESULT hBindAO = S_OK;
    const uint32_t horizonAOEnabled = m_RenderQualitySettings.bHorizonAOEnabled ? 1u : 0u;
	if (DEFERRED::SSAO_BLUR == ePass)
	{
		hBindAO = CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_SSAORaw"), m_pShader, "g_SSAOTexture");
	}

	if (FAILED(hBindAO) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_Depth"), m_pShader, "g_DepthTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_Normal"), m_pShader, "g_NormalTexture")) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(
            TEXT("Target_CharacterGeometry"), m_pShader, "g_CharacterGeometryTexture")) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vSSAOTexelSize", &m_vSSAOTexelSize,
			sizeof(m_vSSAOTexelSize))) ||
		FAILED(m_pShader->Bind_RawValue(
            "g_iSSAOSampleCount", &m_RenderQualitySettings.iSSAOSampleCount,
            sizeof(m_RenderQualitySettings.iSSAOSampleCount))) ||
        FAILED(m_pShader->Bind_RawValue(
            "g_iHorizonAOEnabled", &horizonAOEnabled, sizeof(horizonAOEnabled))) ||
        FAILED(m_pShader->Bind_RawValue(
			"g_fSSAORadius", &m_RenderQualitySettings.fSSAORadius,
			sizeof(m_RenderQualitySettings.fSSAORadius))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fSSAOBias", &m_RenderQualitySettings.fSSAOBias,
			sizeof(m_RenderQualitySettings.fSSAOBias))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fSSAOIntensity", &m_RenderQualitySettings.fSSAOIntensity,
			sizeof(m_RenderQualitySettings.fSSAOIntensity))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fSSAOPower", &m_RenderQualitySettings.fSSAOPower,
			sizeof(m_RenderQualitySettings.fSSAOPower))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fSSAODistanceFade", &m_RenderQualitySettings.fSSAODistanceFade,
			sizeof(m_RenderQualitySettings.fSSAODistanceFade))) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_CameraViewMatrix",
			CGameInstance::Get().Get_Transform(D3DTS::VIEW))) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_CameraProjMatrix",
			CGameInstance::Get().Get_Transform(D3DTS::PROJ))) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_ProjMatrixInverse",
			CGameInstance::Get().Get_InverseTransform(D3DTS::PROJ))) ||
		FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)) ||
		FAILED(m_pShader->Begin(ETOUI(ePass))) ||
		FAILED(m_pVIBuffer->Bind_Resources()) ||
		FAILED(m_pVIBuffer->Render()))
	{
		hResult = E_FAIL;
	}

	/* Balance Begin_MRT even when a shader bind or draw fails. */
	if (FAILED(CGameInstance::Get().End_MRT()))
		hResult = E_FAIL;

	return hResult;
}

/* A portrait is the deferred frame run once more with the portrait camera, before the world
fills the same targets. Every stage below the subject draw is the world's own, called rather
than copied, so the portrait lands on the same lighting, the same SourceCharacter light passes
and the same tone curve as the field. */
HRESULT CRenderer::Render_Portraits()
{
	if (m_PortraitRequests.empty())
		return S_OK;

	const float4x4_t PreviousView = *CGameInstance::Get().Get_Transform(D3DTS::VIEW);
	const float4x4_t PreviousProjection = *CGameInstance::Get().Get_Transform(D3DTS::PROJ);
	if (!m_pPortraitBlendState)
	{
		D3D11_BLEND_DESC Blend{};
		auto& Target = Blend.RenderTarget[0];
		Target.BlendEnable = TRUE;
		Target.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		Target.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		Target.BlendOp = D3D11_BLEND_OP_ADD;
		Target.SrcBlendAlpha = D3D11_BLEND_ONE;
		Target.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		Target.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		Target.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		if (FAILED(m_pDevice->CreateBlendState(&Blend, &m_pPortraitBlendState))) return E_FAIL;
	}
	uint32_t iPreviousViewportCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	D3D11_VIEWPORT PreviousViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
	m_pContext->RSGetViewports(&iPreviousViewportCount, PreviousViewports);
	/* The last resolve leaves a portrait target bound. Begin_MRT saves whatever is bound as the
	target it restores on End_MRT, so the world pass that follows would adopt the portrait
	texture as its back buffer and draw the rest of the frame into it. */
	ID3D11RenderTargetView* PreviousRTVs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
	ComPtr<ID3D11DepthStencilView> pPreviousDSV;
	m_pContext->OMGetRenderTargets(_countof(PreviousRTVs), PreviousRTVs, &pPreviousDSV);
	/* Target_SSAOBlur still holds the world's occlusion, computed from a G-buffer the portrait
	is about to overwrite, and a portrait is too small for its own SSAO pass to be worth a frame.
	The lighting pass reads this flag, so it is cleared for the duration. */
	const bool_t bPreviousSSAO = m_RenderQualitySettings.bSSAOEnabled;
	m_RenderQualitySettings.bSSAOEnabled = false;

	HRESULT hLastFailure = S_OK;
	for (const PORTRAIT_REQUEST& Request : m_PortraitRequests)
	{
		// Every request borrows the full-size targets. The preceding resolve changed
		// both viewport and depth binding to the small portrait destination.
		m_pContext->OMSetRenderTargets(_countof(PreviousRTVs), PreviousRTVs, pPreviousDSV.Get());
		SetUp_ViewportDesc(m_iScenePostWidth, m_iScenePostHeight);
		CGameInstance::Get().Set_Transform(D3DTS::VIEW, XMLoadFloat4x4(&Request.ViewMatrix));
		CGameInstance::Get().Set_Transform(D3DTS::PROJ, XMLoadFloat4x4(&Request.ProjMatrix));
		/* Lighting and the combine rebuild world position from the inverse matrices and the
		camera position, and those are derived once per frame from whatever was last set. Without
		this the portrait is shaded as if the world camera were still the one looking at it. */
		CGameInstance::Get().Refresh_CameraState();

		/* The frame depth buffer is shared. Handing it to Begin_MRT clears it, which gives the
		subject a clean depth test and keeps the previous portrait out of this one. */
		if (FAILED(CGameInstance::Get().Begin_MRT(TEXT("MRT_GameObject"), pPreviousDSV)))
		{
			hLastFailure = E_FAIL;
			continue;
		}
		const HRESULT hSubject = Request.DrawSubject();
		const HRESULT hEndGeometry = CGameInstance::Get().End_MRT();
		if (FAILED(hSubject) || FAILED(hEndGeometry))
		{
			hLastFailure = FAILED(hSubject) ? hSubject : hEndGeometry;
			continue;
		}

		/* Render_Lights itself, not a reduced copy of it. Its ordinary pass skips the marker the
		SourceCharacter materials write, so the mask and per-material passes that follow are the
		only thing that lights a playable body; without them the subject resolves to an unlit
		silhouette. It also consumes the source material frame list the subject draw just filled,
		which the world pass would otherwise inherit and light a second time. */
		const HRESULT hLights = Render_Lights();
		if (FAILED(hLights))
		{
			hLastFailure = hLights;
			continue;
		}

		/* Target_Shade and Target_Specular hold only the lighting terms; Render_Combined mixes
		them with the G-buffer into MRT_SceneHDR, which is still pre-tone-map FP16. */
		if (FAILED(CGameInstance::Get().Begin_MRT(TEXT("MRT_SceneHDR"))))
		{
			hLastFailure = E_FAIL;
			continue;
		}
		HRESULT hCombined = Render_Combined(true);
		// Use the same opaque-forward and translucent material programs as the field,
		// with the subject depth still bound. SceneHDR alpha accumulates UI coverage.
		if (SUCCEEDED(hCombined) && Request.DrawForward)
			hCombined = Request.DrawForward(RENDERGROUP::NONLIGHT, nullptr);
		if (SUCCEEDED(hCombined) && Request.DrawForward)
			hCombined = Request.DrawForward(RENDERGROUP::BLEND, m_pPortraitBlendState.Get());
		const HRESULT hEndScene = CGameInstance::Get().End_MRT();
		if (FAILED(hCombined) || FAILED(hEndScene))
		{
			hLastFailure = FAILED(hCombined) ? hCombined : hEndScene;
			continue;
		}

		/* The portrait resolve tone maps through a full-screen quad, so binding the caller's
		target and shrinking the viewport resamples the whole subject into it. That keeps the
		portrait on the field's tone curve without a second tone map path, and its own pass drops
		the pixels the subject never wrote so the UI frame shows through. */
		ID3D11RenderTargetView* pPortraitTarget = Request.pDestination.Get();
		m_pContext->OMSetRenderTargets(1u, &pPortraitTarget, nullptr);
		D3D11_VIEWPORT DestinationViewport{};
		DestinationViewport.TopLeftX = 0.f;
		DestinationViewport.TopLeftY = 0.f;
		DestinationViewport.Width = static_cast<f32_t>(Request.iWidth);
		DestinationViewport.Height = static_cast<f32_t>(Request.iHeight);
		DestinationViewport.MinDepth = 0.f;
		DestinationViewport.MaxDepth = 1.f;
		m_pContext->RSSetViewports(1u, &DestinationViewport);
		const HRESULT hResolve = Render_Final(true);
		if (FAILED(hResolve))
			hLastFailure = hResolve;
	}

	m_RenderQualitySettings.bSSAOEnabled = bPreviousSSAO;
	m_PortraitRequests.clear();
	CGameInstance::Get().Set_Transform(D3DTS::VIEW, XMLoadFloat4x4(&PreviousView));
	CGameInstance::Get().Set_Transform(D3DTS::PROJ, XMLoadFloat4x4(&PreviousProjection));
	CGameInstance::Get().Refresh_CameraState();
	m_pContext->OMSetRenderTargets(_countof(PreviousRTVs), PreviousRTVs, pPreviousDSV.Get());
	for (auto* pTarget : PreviousRTVs) if (pTarget) pTarget->Release();
	m_pContext->RSSetViewports(iPreviousViewportCount,
		iPreviousViewportCount ? PreviousViewports : nullptr);
	/* The world starts its own depth from scratch. Begin_MRT does not clear a depth buffer it
	was not handed, so the last subject would otherwise reject the geometry standing behind it. */
	if (pPreviousDSV)
		m_pContext->ClearDepthStencilView(pPreviousDSV.Get(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	return hLastFailure;
}

HRESULT CRenderer::Render_Lights()
{
	/* Shade */
	if (FAILED(CGameInstance::Get().Begin_MRT(TEXT("MRT_LightAcc"))))
		return E_FAIL;

	HRESULT hResult = S_OK;
    const uint32_t noSourceCharacter = 0u;
    const auto& pbrComparison = m_MaterialRenderSettings.MapPBR;
    const float4_t pbrContributions = pbrComparison.Is_Active(CGameInstance::Get().Get_CurrentLevelID()) ?
        pbrComparison.vContributionScale : float4_t(1.f, 1.f, 1.f, 1.f);
    const HRESULT hBindPBR = m_pShader->Bind_RawValue("g_MapPBRContributionScale", &pbrContributions,
        sizeof(pbrContributions));
	const bool_t bShadowEnabled =
		CGameInstance::Get().Is_ShadowLightEnabled();
	const uint32_t iSSAOEnabled =
		m_RenderQualitySettings.bSSAOEnabled ? 1u : 0u;
	HRESULT hBindAO = S_OK;
	if (0u != iSSAOEnabled)
	{
		hBindAO = CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_SSAOBlur"), m_pShader, "g_SSAOTexture");
	}
    if (FAILED(hBindPBR) || FAILED(m_pShader->Bind_RawValue("g_SourceCharacterProgram", &noSourceCharacter, sizeof(noSourceCharacter))) ||
        FAILED(m_pShader->Bind_RawValue("g_SourceCharacterRow", &noSourceCharacter, sizeof(noSourceCharacter))) ||
        FAILED(m_pShader->Bind_Matrix("g_SourceCharacterViewMatrix", CGameInstance::Get().Get_Transform(D3DTS::VIEW))) ||
        FAILED(m_pShader->Bind_Matrix("g_SourceCharacterProjMatrix", CGameInstance::Get().Get_Transform(D3DTS::PROJ))) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Emissive"), m_pShader, "g_EmissiveTexture")) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_CharacterSurface"), m_pShader, "g_CharacterSurfaceTexture")) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_CharacterGeometry"), m_pShader, "g_CharacterGeometryTexture")) ||
        FAILED(hBindAO) ||
		FAILED(m_pShader->Bind_Texture(
			"g_LightDepthTexture", m_pShadowSRV)) ||
		FAILED(CGameInstance::Get().
			Bind_ShadowLight_LightingResources(m_pShader)) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vShadowTexelSize", &m_vShadowTexelSize,
			sizeof(m_vShadowTexelSize))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_iSSAOEnabled", &iSSAOEnabled, sizeof(iSSAOEnabled))) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Normal"), m_pShader, "g_NormalTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_MaterialSpecular"), m_pShader, "g_MaterialSpecularTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_PickPos"), m_pShader, "g_GeometricNormalTexture")) ||
		FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInverse", CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW))) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInverse", CGameInstance::Get().Get_InverseTransform(D3DTS::PROJ))) ||
		FAILED(m_pShader->Bind_RawValue("g_vCamPosition", CGameInstance::Get().Get_CamPosition(), sizeof(float4_t))) ||
		FAILED(m_pVIBuffer->Bind_Resources()) ||
		FAILED(CGameInstance::Get().Render_Lights(
			m_pShader, m_pVIBuffer, bShadowEnabled)))
	{
		hResult = E_FAIL;
	}

    // The ordinary pass skips marker-5 pixels. Only materials submitted by
    // visible mesh draws get a source light pass; prototypes never enter here.
    const uint32_t sourceCount = CMaterial::Get_SourceCharacterFrameCount();
    if (SUCCEEDED(hResult) && sourceCount != 0u)
    {
        // Effect outlines own the scene stencil. Only this separate DSV receives
        // the marker-5 mask, and the original light targets are restored on failure.
        ID3D11RenderTargetView* rawTargets[2]{};
        ComPtr<ID3D11DepthStencilView> originalDepth;
        m_pContext->OMGetRenderTargets(2u, rawTargets, originalDepth.GetAddressOf());
        ComPtr<ID3D11RenderTargetView> targets[2];
        for (uint32_t index = 0u; index < 2u; ++index) targets[index].Attach(rawTargets[index]);
        D3D11_TEXTURE2D_DESC targetDesc{};
        if (targets[0])
        {
            ComPtr<ID3D11Resource> resource;
            ComPtr<ID3D11Texture2D> texture;
            targets[0]->GetResource(resource.GetAddressOf());
            if (resource && SUCCEEDED(resource.As(&texture))) texture->GetDesc(&targetDesc);
        }
        bool_t useSourceMask = false;
        if (targetDesc.SampleDesc.Count == 1u &&
            S_OK == Ready_SourceLightMask(targetDesc.Width, targetDesc.Height))
        {
            m_pContext->ClearDepthStencilView(m_pSourceLightMaskDSV.Get(), D3D11_CLEAR_STENCIL, 1.f, 0u);
            m_pContext->OMSetRenderTargets(0u, nullptr, m_pSourceLightMaskDSV.Get());
            HRESULT maskResult = m_pShader->Begin(ETOUI(DEFERRED::SOURCE_LIGHT_MASK));
            if (SUCCEEDED(maskResult)) maskResult = m_pVIBuffer->Render();
            useSourceMask = SUCCEEDED(maskResult);
            if (!useSourceMask)
            {
                m_iSourceLightMaskFailedWidth = targetDesc.Width;
                m_iSourceLightMaskFailedHeight = targetDesc.Height;
                WriteRendererFailure("SourceLightMask_DrawFallback", maskResult);
            }
            m_pContext->OMSetRenderTargets(2u, rawTargets,
                useSourceMask ? m_pSourceLightMaskDSV.Get() : originalDepth.Get());
        }
        for (uint32_t index = 0u; SUCCEEDED(hResult) && index < sourceCount; ++index)
        {
            if (FAILED(CMaterial::Bind_SourceCharacterLight(m_pShader, index)) ||
                FAILED(CGameInstance::Get().Render_Lights(m_pShader, m_pVIBuffer, bShadowEnabled,
                    LIGHT_RECEIVER::SOURCE_CHARACTER, useSourceMask))) hResult = E_FAIL;
        }
        m_pContext->OMSetRenderTargets(2u, rawTargets, originalDepth.Get());
    }
    if (FAILED(m_pShader->Bind_RawValue("g_SourceCharacterProgram", &noSourceCharacter, sizeof(noSourceCharacter))) ||
        FAILED(m_pShader->Bind_RawValue("g_SourceCharacterRow", &noSourceCharacter, sizeof(noSourceCharacter))))
        hResult = E_FAIL;
    CMaterial::Reset_SourceCharacterFrame(m_fPresentationClock);

	/* Always restore the back buffer even when a light bind or draw fails. */
	if (FAILED(CGameInstance::Get().End_MRT()))
		hResult = E_FAIL;

	return hResult;
}

HRESULT CRenderer::Render_Combined(bool_t bPortrait)
{
	const uint32_t portrait = bPortrait ? 1u : 0u;
	if (FAILED(m_pShader->Bind_RawValue("g_iPortraitCapture", &portrait, sizeof(portrait)))) return E_FAIL;
    CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope cpuScope(profiler, "Render.Combined");
    CProfilerGpuScope gpuScope(profiler, "Render.Combined");
    const uint32_t dynamicBakedShadow = CGameInstance::Get().Is_ShadowLightEnabled() &&
        m_bStaticShadowCacheValid && m_pStaticShadowSRV ? 1u : 0u;
    const auto& comparison = m_MaterialRenderSettings.MapPBR;
    const float diffuseScale = comparison.Is_Active(CGameInstance::Get().Get_CurrentLevelID()) ?
        comparison.fCubeDiffuseScale : 1.f;
    const float cubeDiffuseIntensity = m_RenderEnvironment.pCube && !m_RenderEnvironment.bUseSourcePBRIndirect ?
        m_RenderEnvironment.fDiffuseIntensity * m_RenderEnvironment.vRotationIntensity.z * diffuseScale : 0.f;
    if (FAILED(m_pShader->Bind_RawValue("g_CubeDiffuseSH", m_RenderEnvironment.vDiffuseSH.data(),
            sizeof(m_RenderEnvironment.vDiffuseSH))) ||
        FAILED(m_pShader->Bind_RawValue("g_CubeDiffuseIntensity", &cubeDiffuseIntensity, sizeof(cubeDiffuseIntensity))) ||
        FAILED(m_pShader->Bind_RawValue("g_CubeDiffuseColor", &m_RenderEnvironment.vColor, sizeof(float4_t))) ||
        FAILED(m_pShader->Bind_RawValue("g_CubeDiffuseRotation", &m_RenderEnvironment.vRotationIntensity, sizeof(float4_t))) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Normal"), m_pShader, "g_NormalTexture")) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_MaterialSpecular"), m_pShader, "g_MaterialSpecularTexture")))
        return E_FAIL;
    if (FAILED(m_pShader->Bind_RawValue("g_iDynamicBakedShadowEnabled",
            &dynamicBakedShadow, sizeof(dynamicBakedShadow))) ||
        FAILED(m_pShader->Bind_Texture("g_StaticLightDepthTexture",
            m_pStaticShadowSRV ? m_pStaticShadowSRV : m_pShadowSRV)) ||
        FAILED(m_pShader->Bind_Texture("g_LightDepthTexture", m_pShadowSRV)) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_CharacterGeometry"),
            m_pShader, "g_CharacterGeometryTexture")) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_PickPos"),
            m_pShader, "g_GeometricNormalTexture"))) return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Diffuse"), m_pShader, "g_DiffuseTexture")))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Shade"), m_pShader, "g_ShadeTexture")))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Specular"), m_pShader, "g_SpecularTexture")))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Emissive"), m_pShader, "g_EmissiveTexture")))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	/* Height fog reuses the world position the combine step can rebuild from
	   the depth target, so it needs the same inverse matrices and camera the
	   lighting pass already binds. */
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_Depth"), m_pShader, "g_DepthTexture")) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrixInverse",
			CGameInstance::Get().Get_InverseTransform(D3DTS::VIEW))) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrixInverse",
			CGameInstance::Get().Get_InverseTransform(D3DTS::PROJ))) ||
		FAILED(m_pShader->Bind_RawValue("g_vCamPosition",
			CGameInstance::Get().Get_CamPosition(), sizeof(float4_t))) ||
		FAILED(Bind_HeightFog(m_pShader.get()))) return E_FAIL;

	if (FAILED(m_pShader->Begin(ETOUI(DEFERRED::COMBINED))))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Resources()))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Ready_SSGIHalfTarget(uint32_t width, uint32_t height)
{
    if (width == 0u || height == 0u || width > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION ||
        height > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION) return E_INVALIDARG;
    const uint32_t halfWidth = (width + 1u) / 2u, halfHeight = (height + 1u) / 2u;
    if (m_iSSGIHalfWidth == halfWidth && m_iSSGIHalfHeight == halfHeight &&
        m_pSSGIHalfTexture && m_pSSGIHalfRTV && m_pSSGIHalfSRV) return S_OK;
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = halfWidth; desc.Height = halfHeight;
    desc.MipLevels = desc.ArraySize = desc.SampleDesc.Count = 1u;
    desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11RenderTargetView> target;
    ComPtr<ID3D11ShaderResourceView> resource;
    HRESULT result = m_pDevice->CreateTexture2D(&desc, nullptr, texture.GetAddressOf());
    if (SUCCEEDED(result)) result = m_pDevice->CreateRenderTargetView(texture.Get(), nullptr, target.GetAddressOf());
    if (SUCCEEDED(result)) result = m_pDevice->CreateShaderResourceView(texture.Get(), nullptr, resource.GetAddressOf());
    if (FAILED(result)) return result;
    // A resize only replaces this lazy scratch after every resource succeeds.
    m_pSSGIHalfTexture = std::move(texture);
    m_pSSGIHalfRTV = std::move(target);
    m_pSSGIHalfSRV = std::move(resource);
    m_iSSGIHalfWidth = halfWidth; m_iSSGIHalfHeight = halfHeight;
    return S_OK;
}

HRESULT CRenderer::Render_ScreenSpaceLighting()
{
    const auto& quality = m_RenderQualitySettings;
    if (!quality.bSSGIEnabled && !quality.bSSREnabled) return S_OK;
    if (!m_pScreenSpaceLightingShader || !m_pVIBuffer) return E_FAIL;
    auto& game = CGameInstance::Get();
    CProfiler* const profiler = game.Get_Profiler();
    CProfilerScope cpuScope(profiler, "Render.ScreenSpaceLighting");
    auto shader = m_pScreenSpaceLightingShader;
    const auto scene = game.Get_RT_SRV(TEXT("Target_SceneHDR"));
    const auto bloom = game.Get_RT_SRV(TEXT("Target_SceneBloom"));
    if (!scene || !bloom) return E_FAIL;

    // Validate every resource before changing the output bindings. Scratch
    // targets already participate in the transactional viewport resize path.
    ComPtr<ID3D11Resource> originalScene, originalBloom, scratchBloom[2];
    scene->GetResource(originalScene.GetAddressOf());
    bloom->GetResource(originalBloom.GetAddressOf());
    D3D11_TEXTURE2D_DESC sceneDesc{};
    ComPtr<ID3D11Texture2D> sceneTexture;
    if (FAILED(originalScene.As(&sceneTexture))) return E_FAIL;
    sceneTexture->GetDesc(&sceneDesc);
    const auto compatible = [&sceneDesc](ID3D11Resource* resource)
    {
        ComPtr<ID3D11Texture2D> texture;
        if (!resource || FAILED(resource->QueryInterface(IID_PPV_ARGS(texture.GetAddressOf())))) return false;
        D3D11_TEXTURE2D_DESC desc{};
        texture->GetDesc(&desc);
        return desc.Width == sceneDesc.Width && desc.Height == sceneDesc.Height &&
            desc.Format == DXGI_FORMAT_R16G16B16A16_FLOAT && desc.Format == sceneDesc.Format &&
            desc.MipLevels == 1u && desc.ArraySize == 1u && desc.SampleDesc.Count == 1u &&
            desc.SampleDesc.Quality == sceneDesc.SampleDesc.Quality;
    };
    if (!compatible(originalScene.Get()) || !compatible(originalBloom.Get())) return E_INVALIDARG;
    const bool halfSSGI = quality.bSSGIEnabled && quality.bSSGIHalfResolution;
    if (halfSSGI && FAILED(Ready_SSGIHalfTarget(sceneDesc.Width, sceneDesc.Height))) return E_FAIL;
    for (size_t index = 0; index < 2u; ++index)
    {
        if (!m_pScenePostRTVs[index] || !m_pScenePostSRVs[index] ||
            !m_pSceneBloomPostRTVs[index] || !m_pSceneBloomPostSRVs[index]) return E_FAIL;
        m_pSceneBloomPostSRVs[index]->GetResource(scratchBloom[index].GetAddressOf());
        if (!compatible(m_pScenePostTextures[index].Get()) || !compatible(scratchBloom[index].Get()) ||
            m_pScenePostTextures[index].Get() == originalScene.Get() ||
            scratchBloom[index].Get() == originalBloom.Get()) return E_INVALIDARG;
    }

    struct RestoreOutputs final
    {
        ID3D11DeviceContext* Context;
        ID3D11RenderTargetView* Outputs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
        ComPtr<ID3D11DepthStencilView> Depth;
        D3D11_VIEWPORT Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
        UINT ViewportCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
        shared_ptr<CShader> Shader;
        ~RestoreOutputs()
        {
            ID3D11ShaderResourceView* empty[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
            Context->PSSetShaderResources(0, _countof(empty), empty);
            for (const char* name : { "g_DepthTexture", "g_NormalTexture", "g_MaterialSpecularTexture",
                "g_DiffuseTexture", "g_RadianceTexture", "g_SceneHDRTexture", "g_SceneBloomTexture",
                "g_SSGIHalfTexture" })
                Shader->Bind_Texture(name, nullptr);
            Context->OMSetRenderTargets(_countof(Outputs), Outputs, Depth.Get());
            Context->RSSetViewports(ViewportCount, Viewports);
            for (auto* output : Outputs) if (output) output->Release();
        }
    } restore{m_pContext.Get()};
    restore.Shader = shader;
    m_pContext->OMGetRenderTargets(_countof(restore.Outputs), restore.Outputs, restore.Depth.GetAddressOf());
    m_pContext->RSGetViewports(&restore.ViewportCount, restore.Viewports);
    ComPtr<ID3D11Resource> activeScene, activeBloom;
    if (restore.Outputs[0]) restore.Outputs[0]->GetResource(activeScene.GetAddressOf());
    if (restore.Outputs[2]) restore.Outputs[2]->GetResource(activeBloom.GetAddressOf());
    if (activeScene.Get() != originalScene.Get() || activeBloom.Get() != originalBloom.Get()) return E_INVALIDARG;
    CRenderOutputContractScope temporaryOutput(RENDER_OUTPUT_CONTRACT::NONE, m_pContext.Get());
    const float2_t inverseSize(1.f / sceneDesc.Width, 1.f / sceneDesc.Height);
    const uint32_t bloomEnabled = quality.bBloomEnabled ? 1u : 0u;
    const uint32_t ssrRefinementEnabled = quality.bSSRRefinementEnabled ? 1u : 0u;
    const uint32_t ssrRoughnessFilterEnabled = quality.bSSRRoughnessFilterEnabled ? 1u : 0u;
    const auto bind = [&shader](const char* name, const auto& value)
    { return shader->Bind_RawValue(name, &value, sizeof(value)); };
    if (FAILED(shader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)) ||
        FAILED(shader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)) ||
        FAILED(shader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)) ||
        FAILED(shader->Bind_Matrix("g_CameraViewMatrix", game.Get_Transform(D3DTS::VIEW))) ||
        FAILED(shader->Bind_Matrix("g_CameraProjMatrix", game.Get_Transform(D3DTS::PROJ))) ||
        FAILED(shader->Bind_Matrix("g_CameraProjMatrixInverse", game.Get_InverseTransform(D3DTS::PROJ))) ||
        FAILED(game.Bind_RT_SRV(TEXT("Target_Depth"), shader, "g_DepthTexture")) ||
        FAILED(game.Bind_RT_SRV(TEXT("Target_Normal"), shader, "g_NormalTexture")) ||
        FAILED(game.Bind_RT_SRV(TEXT("Target_MaterialSpecular"), shader, "g_MaterialSpecularTexture")) ||
        FAILED(game.Bind_RT_SRV(TEXT("Target_Diffuse"), shader, "g_DiffuseTexture")) ||
        FAILED(shader->Bind_Texture("g_RadianceTexture", scene)) ||
        FAILED(bind("g_vInverseSceneSize", inverseSize)) ||
        FAILED(bind("g_iSSGIRayCount", quality.iSSGISampleCount)) ||
        FAILED(bind("g_fSSGIRadius", quality.fSSGIRadius)) ||
        FAILED(bind("g_fSSGIStrength", quality.fSSGIStrength)) ||
        FAILED(bind("g_iSSRStepCount", quality.iSSRStepCount)) ||
        FAILED(bind("g_iSSRRefinementEnabled", ssrRefinementEnabled)) ||
        FAILED(bind("g_iSSRRoughnessFilterEnabled", ssrRoughnessFilterEnabled)) ||
        FAILED(bind("g_fSSRMaxDistance", quality.fSSRMaxDistance)) ||
        FAILED(bind("g_fSSRThickness", quality.fSSRThickness)) ||
        FAILED(bind("g_fSSRStrength", quality.fSSRStrength)) ||
        FAILED(bind("g_iBloomEnabled", bloomEnabled)) ||
        FAILED(bind("g_fBloomThreshold", quality.fBloomThreshold)) ||
        FAILED(bind("g_fBloomSoftKnee", quality.fBloomSoftKnee)) ||
        FAILED(bind("g_fBloomIntensity", quality.fBloomIntensity))) return E_FAIL;

    ID3D11ShaderResourceView* empty[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
    auto baseScene = scene;
    auto baseBloom = bloom;
    uint32_t writtenTargets = 0u;
    SetUp_ViewportDesc(sceneDesc.Width, sceneDesc.Height);
    for (uint32_t pass = 0u; pass < 2u; ++pass)
    {
        if ((pass == 0u && !quality.bSSGIEnabled) || (pass == 1u && !quality.bSSREnabled)) continue;
        const char* name = pass == 0u ? "Render.SSGI" : "Render.SSR";
        CProfilerScope passCpu(profiler, name);
        CProfilerGpuScope passGpu(profiler, name, true);
        if (pass == 0u && halfSSGI)
        {
            CProfilerScope gatherCpu(profiler, "Render.SSGI.GatherHalf");
            CProfilerGpuScope gatherGpu(profiler, "Render.SSGI.GatherHalf", true);
            m_pContext->PSSetShaderResources(0, _countof(empty), empty);
            if (FAILED(shader->Bind_Texture("g_SSGIHalfTexture", nullptr))) return E_FAIL;
            ID3D11RenderTargetView* gatherTarget = m_pSSGIHalfRTV.Get();
            m_pContext->OMSetRenderTargets(1u, &gatherTarget, nullptr);
            SetUp_ViewportDesc(m_iSSGIHalfWidth, m_iSSGIHalfHeight);
            if (FAILED(shader->Begin(2u)) || FAILED(m_pVIBuffer->Bind_Resources()) ||
                FAILED(m_pVIBuffer->Render())) return E_FAIL;
            m_pContext->OMSetRenderTargets(0u, nullptr, nullptr);
            SetUp_ViewportDesc(sceneDesc.Width, sceneDesc.Height);
        }
        const uint32_t destination = writtenTargets;
        m_pContext->PSSetShaderResources(0, _countof(empty), empty);
        ID3D11RenderTargetView* outputs[3] = {
            m_pScenePostRTVs[destination].Get(), nullptr, m_pSceneBloomPostRTVs[destination].Get() };
        m_pContext->OMSetRenderTargets(3u, outputs, nullptr);
        const bool halfResolve = pass == 0u && halfSSGI;
        CProfilerScope resolveCpu(halfResolve ? profiler : nullptr, "Render.SSGI.ResolveHalf");
        CProfilerGpuScope resolveGpu(halfResolve ? profiler : nullptr, "Render.SSGI.ResolveHalf", true);
        if (FAILED(shader->Bind_Texture("g_SceneHDRTexture", baseScene)) ||
            FAILED(shader->Bind_Texture("g_SceneBloomTexture", baseBloom)) ||
            (halfResolve && FAILED(shader->Bind_Texture("g_SSGIHalfTexture", m_pSSGIHalfSRV))) ||
            FAILED(shader->Begin(halfResolve ? 3u : pass)) || FAILED(m_pVIBuffer->Bind_Resources()) ||
            FAILED(m_pVIBuffer->Render())) return E_FAIL;
        baseScene = m_pScenePostSRVs[destination];
        baseBloom = m_pSceneBloomPostSRVs[destination];
        ++writtenTargets;
    }
    m_pContext->OMSetRenderTargets(0, nullptr, nullptr);
    m_pContext->PSSetShaderResources(0, _countof(empty), empty);
    if (FAILED(m_pDevice->GetDeviceRemovedReason())) return E_FAIL;
    {
        CProfilerScope copyCpu(profiler, "Render.ScreenSpaceLighting.Copy");
        CProfilerGpuScope copyGpu(profiler, "Render.ScreenSpaceLighting.Copy");
        // Commit only after both passes succeed. The distortion target and all
        // authored lighting inputs remain unchanged.
        m_pContext->CopyResource(originalScene.Get(), m_pScenePostTextures[writtenTargets - 1u].Get());
        m_pContext->CopyResource(originalBloom.Get(), scratchBloom[writtenTargets - 1u].Get());
        if (profiler)
        {
            profiler->Add_Counter(EProfilerCounter::SceneColorCopies, 2u);
            profiler->Add_Counter(EProfilerCounter::SceneColorCopyBytes,
                static_cast<uint64_t>(sceneDesc.Width) * sceneDesc.Height * 16u);
        }
    }
    return m_pDevice->GetDeviceRemovedReason();
}


HRESULT CRenderer::Render_NonLight()
{
    CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope cpuScope(profiler, "Render.NonLight");
    CProfilerGpuScope gpuScope(profiler, "Render.NonLight");
	HRESULT hFirstFailure = S_OK;
	for (size_t renderIndex = 0; renderIndex < m_RenderObjects[ETOUI(RENDERGROUP::NONLIGHT)].size(); ++renderIndex)
	{
		CGameObject* const pRenderObject = m_RenderObjects[ETOUI(RENDERGROUP::NONLIGHT)][renderIndex].get();
		if (nullptr != pRenderObject)
		{
			const HRESULT hResult = pRenderObject->Render_Group(RENDERGROUP::NONLIGHT);
			if (FAILED(hResult) && SUCCEEDED(hFirstFailure))
			{
				WriteRendererFailure(
					"Render_NonLight_Object",
					hResult,
					typeid(*pRenderObject).name());
				hFirstFailure = hResult;
			}
		}
	}

	m_RenderObjects[ETOUI(RENDERGROUP::NONLIGHT)].clear();

	return hFirstFailure;
}

HRESULT CRenderer::Refresh_SceneColorSnapshot()
{
	if (CRenderOutputContract::Get_Active() !=
			RENDER_OUTPUT_CONTRACT::SCENE_HDR_RT0_SCENE_COLOR_RT1_DISTORTION ||
		!CRenderOutputContract::Matches_ActiveRenderTargets(m_pContext.Get()))
		return E_INVALIDARG;
	return Capture_SceneColorSnapshot();
}

HRESULT CRenderer::Capture_SceneColorSnapshot()
{
    CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope snapshotScope(profiler, "Render.SceneColorSnapshot");
	const auto sourceSRV = CGameInstance::Get().Get_RT_SRV(TEXT("Target_SceneHDR"));
	const auto snapshotSRV = CGameInstance::Get().Get_RT_SRV(TEXT("Target_EffectSceneColor"));
	if (!sourceSRV || !snapshotSRV)
		return E_FAIL;
	ComPtr<ID3D11Resource> sourceResource, snapshotResource;
	sourceSRV->GetResource(sourceResource.GetAddressOf());
	snapshotSRV->GetResource(snapshotResource.GetAddressOf());
	ComPtr<ID3D11Texture2D> sourceTexture, snapshotTexture;
	if (!sourceResource || !snapshotResource || sourceResource.Get() == snapshotResource.Get() ||
		FAILED(sourceResource.As(&sourceTexture)) || FAILED(snapshotResource.As(&snapshotTexture)))
		return E_FAIL;
	const auto bloomSnapshotSRV = CGameInstance::Get().Get_RT_SRV(TEXT("Target_EffectSceneBloom"));
	ComPtr<ID3D11Resource> bloomSnapshotResource;
	ComPtr<ID3D11Texture2D> bloomSnapshotTexture;
	if (!bloomSnapshotSRV) return E_FAIL;
	bloomSnapshotSRV->GetResource(bloomSnapshotResource.GetAddressOf());
	if (!bloomSnapshotResource || FAILED(bloomSnapshotResource.As(&bloomSnapshotTexture))) return E_FAIL;
	D3D11_TEXTURE2D_DESC sourceDesc{}, snapshotDesc{};
	sourceTexture->GetDesc(&sourceDesc);
	snapshotTexture->GetDesc(&snapshotDesc);
	if (sourceDesc.Width != snapshotDesc.Width || sourceDesc.Height != snapshotDesc.Height ||
		sourceDesc.Format != snapshotDesc.Format || sourceDesc.MipLevels != snapshotDesc.MipLevels ||
		sourceDesc.ArraySize != snapshotDesc.ArraySize ||
		sourceDesc.SampleDesc.Count != snapshotDesc.SampleDesc.Count ||
		sourceDesc.SampleDesc.Quality != snapshotDesc.SampleDesc.Quality)
		return E_INVALIDARG;

	// Copy outside the output binding, then restore all MRTs and the same DSV.
	// Begin_MRT would clear the accumulated scene, so it must not be used here.
	ID3D11RenderTargetView* outputs[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
	ComPtr<ID3D11DepthStencilView> depth;
	m_pContext->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
		outputs, depth.GetAddressOf());
	ComPtr<ID3D11Resource> activeSceneResource;
	if (outputs[0]) outputs[0]->GetResource(activeSceneResource.GetAddressOf());
	if (activeSceneResource.Get() != sourceResource.Get())
	{
		for (auto* output : outputs)
			if (output) output->Release();
		return E_INVALIDARG;
	}
	m_pContext->OMSetRenderTargets(0, nullptr, nullptr);
	ID3D11ShaderResourceView* emptySRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	m_pContext->PSSetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, emptySRVs);
    HRESULT result = E_FAIL;
    {
        CProfilerGpuScope copyScope(profiler, "Render.SceneColorCopy");
        result = CGameInstance::Get().Copy_RT_Resource(
            TEXT("Target_SceneHDR"), snapshotTexture);
        if (SUCCEEDED(result))
            result = CGameInstance::Get().Copy_RT_Resource(TEXT("Target_SceneBloom"), bloomSnapshotTexture);
    }
    if (SUCCEEDED(result) && profiler)
    {
        profiler->Add_Counter(EProfilerCounter::SceneColorCopies, 2u);
        // Both named targets are created as RGBA16F. Count the copied logical
        // payload, not read+write bus traffic or driver allocation overhead.
        uint64_t pixels = 0;
        uint32_t width = sourceDesc.Width;
        uint32_t height = sourceDesc.Height;
        for (uint32_t mip = 0; mip < sourceDesc.MipLevels; ++mip)
        {
            pixels += static_cast<uint64_t>(width) * height;
            width = (std::max)(1u, width / 2);
            height = (std::max)(1u, height / 2);
        }
        profiler->Add_Counter(EProfilerCounter::SceneColorCopyBytes,
            pixels * 16u * sourceDesc.ArraySize * sourceDesc.SampleDesc.Count);
    }
	m_pContext->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT,
		outputs, depth.Get());
	for (auto* output : outputs)
		if (output) output->Release();
	return result;
}

HRESULT CRenderer::Render_Blend()
{
    CProfiler* const profiler = CGameInstance::Get().Get_Profiler();
    CProfilerScope cpuScope(profiler, "Render.Blend");
    CProfilerGpuScope gpuScope(profiler, "Render.Blend", true);
	HRESULT hFirstFailure = S_OK;

	/* Translucent surfaces have to be drawn far to near or they overwrite each
	   other in submission order, which is what made overlapping water sheets
	   and their props flip depending on which placement happened to be added
	   first. The queue itself is left alone; only the draw order is sorted, so
	   nothing else that walks the render group sees a different sequence.

	   An object may request an earlier slot through Get_BlendSortPriority():
	   lower priorities draw first and equal priorities keep far-to-near order.
	   Without a camera every distance ties, so only the priority reorders and
	   stable_sort keeps the submission order.

	   The scratch buffer is a function-local static because rendering runs on
	   one thread and the alternative is a heap allocation every frame. */
	struct BLEND_ENTRY final
	{
		Engine::BLEND_SORT_KEY key;
		CGameObject* pObject;
	};
	static vector<BLEND_ENTRY> SortedBlendObjects;
	SortedBlendObjects.clear();
	auto& BlendObjects = m_RenderObjects[ETOUI(RENDERGROUP::BLEND)];
	SortedBlendObjects.reserve(BlendObjects.size());
	const float4_t* pCamPosition = CGameInstance::Get().Get_CamPosition();
	const vector_t vCamera =
		nullptr != pCamPosition ? XMLoadFloat4(pCamPosition) : XMVectorZero();
	for (size_t renderIndex = 0; renderIndex < BlendObjects.size(); ++renderIndex)
	{
		CGameObject* const pRenderObject = BlendObjects[renderIndex].get();
		if (nullptr == pRenderObject)
			continue;
		/* No transform is not an error here: the object still has to draw,
		   it just cannot be placed in the ordering, so it goes first. */
		f32_t fDistanceSquared = FLT_MAX;
		if (nullptr != pCamPosition)
		{
			const shared_ptr<CTransform> pTransform =
				dynamic_pointer_cast<CTransform>(
					pRenderObject->Get_Component(g_strTransformComTag));
			if (nullptr != pTransform)
			{
				const vector_t vDelta =
					pTransform->Get_State(STATE::POSITION) - vCamera;
				fDistanceSquared =
					XMVectorGetX(XMVector3LengthSq(vDelta));
				if (!std::isfinite(fDistanceSquared))
					fDistanceSquared = FLT_MAX;
			}
		}
		SortedBlendObjects.push_back(
			{ { pRenderObject->Get_BlendSortPriority(), fDistanceSquared }, pRenderObject });
	}
	std::stable_sort(
		SortedBlendObjects.begin(), SortedBlendObjects.end(),
		[](const BLEND_ENTRY& lhs, const BLEND_ENTRY& rhs)
		{
			return Engine::BlendSortBefore(lhs.key, rhs.key);
		});
	for (const BLEND_ENTRY& Entry : SortedBlendObjects)
	{
		const HRESULT hResult = Entry.pObject->Render_Group(RENDERGROUP::BLEND);
		if (FAILED(hResult) && SUCCEEDED(hFirstFailure))
		{
			WriteRendererFailure(
				"Render_Blend_Object",
				hResult,
				typeid(*Entry.pObject).name());
			hFirstFailure = hResult;
		}
	}
	BlendObjects.clear();
	SortedBlendObjects.clear();
	return hFirstFailure;
}

HRESULT CRenderer::Render_ScreenPostPass(
	ComPtr<ID3D11ShaderResourceView> pSourceSRV,
	ComPtr<ID3D11RenderTargetView> pDestinationRTV,
	const uint32_t iPassIndex,
	ComPtr<ID3D11ShaderResourceView> pBloomSourceSRV,
	ComPtr<ID3D11RenderTargetView> pBloomDestinationRTV,
	const PRESENTATION_SCREEN_POST_DESC* pPostDesc)
{
	if (nullptr == pDestinationRTV || nullptr == pBloomDestinationRTV)
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_fSceneBloomIntensity", &m_RenderQualitySettings.fBloomIntensity, sizeof(float))) ||
		FAILED(m_pShader->Bind_RawValue("g_fEffectBloomThreshold", &m_RenderQualitySettings.fBloomThreshold, sizeof(float))) ||
		FAILED(m_pShader->Bind_RawValue("g_fEffectBloomSoftKnee", &m_RenderQualitySettings.fBloomSoftKnee, sizeof(float))))
		return E_FAIL;
	if (nullptr != pSourceSRV)
	{
		ComPtr<ID3D11Resource> pSourceResource;
		ComPtr<ID3D11Resource> pDestinationResource;
		pSourceSRV->GetResource(&pSourceResource);
		pDestinationRTV->GetResource(&pDestinationResource);
		if (nullptr == pSourceResource || nullptr == pDestinationResource ||
			pSourceResource.Get() == pDestinationResource.Get())
		{
			return E_FAIL;
		}
	}

	ID3D11ShaderResourceView* pNullSRVs[
		D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	m_pContext->PSSetShaderResources(
		0u, _countof(pNullSRVs), pNullSRVs);
	ID3D11RenderTargetView* destinations[3] = {
		pDestinationRTV.Get(), nullptr, pBloomDestinationRTV.Get() };
	m_pContext->OMSetRenderTargets(3u, destinations, nullptr);
	const float4_t vClear{};
	m_pContext->ClearRenderTargetView(destinations[0], &vClear.x);
	m_pContext->ClearRenderTargetView(destinations[2], &vClear.x);

	if (nullptr != pPostDesc && nullptr != pPostDesc->pMaterial)
	{
		PRESENTATION_SCREEN_POST_MATERIAL_INPUT Input;
		Input.pSceneColor = pSourceSRV;
		Input.pSceneBloom = pBloomSourceSRV;
		Input.pSceneDepth = CGameInstance::Get().Get_RT_SRV(TEXT("Target_Depth"));
		Input.World = m_WorldMatrix;
		Input.View = m_ViewMatrix;
		Input.Projection = m_ProjMatrix;
		if (nullptr == Input.pSceneColor || nullptr == Input.pSceneBloom || nullptr == Input.pSceneDepth)
			return E_FAIL;
		const HRESULT hBind = pPostDesc->pMaterial->Bind(Input);
		if (hBind != S_OK)
			return hBind; // S_FALSE is a suppressed pass: do not draw stale shader state.
		if (FAILED(m_pVIBuffer->Bind_Resources()) || FAILED(m_pVIBuffer->Render()))
			return E_FAIL;
		return S_OK;
	}
	if (nullptr == pPostDesc)
	{
		if (FAILED(CGameInstance::Get().Bind_RT_SRV(
				TEXT("Target_SceneHDR"), m_pShader,
				"g_SceneHDRTexture")) ||
			FAILED(CGameInstance::Get().Bind_RT_SRV(
				TEXT("Target_Distortion"), m_pShader,
				"g_DistortionTexture")) ||
			FAILED(CGameInstance::Get().Bind_RT_SRV(
				TEXT("Target_Depth"), m_pShader, "g_DepthTexture")) ||
			FAILED(CGameInstance::Get().Bind_RT_SRV(
				TEXT("Target_PickPos"), m_pShader, "g_GeometricNormalTexture")) ||
			FAILED(CGameInstance::Get().Bind_RT_SRV(
				TEXT("Target_SceneBloom"), m_pShader, "g_SceneBloomTexture")))
		{
			return E_FAIL;
		}
	}
	else
	{
		if (nullptr == pSourceSRV || nullptr == pBloomSourceSRV ||
			FAILED(m_pShader->Bind_Texture("g_PostBloomTexture", pBloomSourceSRV)) ||
			FAILED(m_pShader->Bind_Texture(
				"g_PostProcessTexture", pSourceSRV)) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_fPresentationTime", &pPostDesc->fSampleTimeSeconds,
				sizeof(pPostDesc->fSampleTimeSeconds))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_fPresentationIntensity", &pPostDesc->fIntensity,
				sizeof(pPostDesc->fIntensity))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_fPresentationSecondaryIntensity",
				&pPostDesc->fSecondaryIntensity,
				sizeof(pPostDesc->fSecondaryIntensity))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_fPresentationFrequency", &pPostDesc->fFrequency,
				sizeof(pPostDesc->fFrequency))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_iPresentationSeed", &pPostDesc->iRandomSeed,
				sizeof(pPostDesc->iRandomSeed))) ||
			FAILED(m_pShader->Bind_RawValue(
				"g_vPresentationTint", &pPostDesc->vTint,
				sizeof(pPostDesc->vTint))))
		{
			return E_FAIL;
		}
	}

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)) ||
		FAILED(m_pShader->Begin(iPassIndex)) ||
		FAILED(m_pVIBuffer->Bind_Resources()) ||
		FAILED(m_pVIBuffer->Render()))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CRenderer::Render_SceneReplacements()
{
    const auto& posts = CPresentation_Manager::Get().Get_ScreenPosts();
    const bool hasReplacement = std::any_of(posts.begin(), posts.end(), [](const auto& post) {
        return post.pMaterial && post.pMaterial->Replaces_SceneBeforeBlend();
    });
    if (!hasReplacement) return S_OK;
    auto color = CGameInstance::Get().Get_RT_SRV(TEXT("Target_SceneHDR"));
    auto bloom = CGameInstance::Get().Get_RT_SRV(TEXT("Target_SceneBloom"));
    if (!color || !bloom) return E_FAIL;
    ID3D11RenderTargetView* saved[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT]{};
    ID3D11DepthStencilView* depth = nullptr;
    m_pContext->OMGetRenderTargets(_countof(saved), saved, &depth);
    HRESULT result = S_OK;
    for (const auto& post : posts)
    {
        if (!post.pMaterial || !post.pMaterial->Replaces_SceneBeforeBlend()) continue;
        result = Render_ScreenPostPass(color, m_pScenePostRTVs[0], 0u,
            bloom, m_pSceneBloomPostRTVs[0], &post);
        if (FAILED(result)) break;
        if (result == S_FALSE) continue;
        ID3D11ShaderResourceView* clear[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
        m_pContext->PSSetShaderResources(0u, _countof(clear), clear);
        m_pContext->OMSetRenderTargets(0u, nullptr, nullptr);
        const auto copy = [this](ID3D11ShaderResourceView* source, ID3D11ShaderResourceView* destination) {
            ComPtr<ID3D11Resource> sourceResource, destinationResource;
            source->GetResource(&sourceResource); destination->GetResource(&destinationResource);
            ComPtr<ID3D11Texture2D> sourceTexture, destinationTexture;
            if (!sourceResource || !destinationResource || sourceResource.Get() == destinationResource.Get() ||
                FAILED(sourceResource.As(&sourceTexture)) || FAILED(destinationResource.As(&destinationTexture))) return E_FAIL;
            D3D11_TEXTURE2D_DESC a{}, b{};
            sourceTexture->GetDesc(&a); destinationTexture->GetDesc(&b);
            if (a.Width != b.Width || a.Height != b.Height || a.Format != b.Format ||
                a.ArraySize != b.ArraySize || a.MipLevels != b.MipLevels ||
                a.SampleDesc.Count != b.SampleDesc.Count || a.SampleDesc.Quality != b.SampleDesc.Quality) return E_FAIL;
            m_pContext->CopyResource(destinationResource.Get(), sourceResource.Get());
            return S_OK;
        };
        result = copy(m_pScenePostSRVs[0].Get(), color.Get());
        if (SUCCEEDED(result)) result = copy(m_pSceneBloomPostSRVs[0].Get(), bloom.Get());
        if (FAILED(result)) break;
    }
    ID3D11ShaderResourceView* clear[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
    m_pContext->PSSetShaderResources(0u, _countof(clear), clear);
    m_pContext->OMSetRenderTargets(_countof(saved), saved, depth);
    for (auto* target : saved) if (target) target->Release();
    if (depth) depth->Release();
    return result;
}

HRESULT CRenderer::Render_ScreenPosts()
{
	CPresentation_Manager& Presentation = CPresentation_Manager::Get();
	ComPtr<ID3D11RenderTargetView> pOriginalRTV;
	ComPtr<ID3D11DepthStencilView> pOriginalDSV;
	m_pContext->OMGetRenderTargets(
		1u, pOriginalRTV.GetAddressOf(), pOriginalDSV.GetAddressOf());
	D3D11_VIEWPORT OriginalViewports[
		D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
	UINT iViewportCount = _countof(OriginalViewports);
	m_pContext->RSGetViewports(&iViewportCount, OriginalViewports);
	SetUp_ViewportDesc(m_iScenePostWidth, m_iScenePostHeight);

	HRESULT hResult = Render_ScreenPostPass(
		nullptr, m_pScenePostRTVs[0], DEFERRED_PASS_SCENE_RESOLVE,
		nullptr, m_pSceneBloomPostRTVs[0]);
	const vector<PRESENTATION_SCREEN_POST_DESC>& ScreenPosts =
		Presentation.Get_ScreenPosts();
	size_t composedPostCount = 0u;
	for (size_t iPost = 0u;
		SUCCEEDED(hResult) && iPost < ScreenPosts.size(); ++iPost)
	{
		const PRESENTATION_SCREEN_POST_DESC& Post = ScreenPosts[iPost];
		if (Post.pMaterial && Post.pMaterial->Replaces_SceneBeforeBlend()) continue;
		const PRESENTATION_SCREEN_POST_PLAN_STEP Step =
			Build_PresentationScreenPostPlanStep(composedPostCount);
		uint32_t iPassIndex = {};
		switch (Post.eProfile)
		{
		case PRESENTATION_SCREEN_POST_PROFILE::RGB_NOISE_RECONSTRUCTED:
			iPassIndex = DEFERRED_PASS_RGB_NOISE;
			break;
		case PRESENTATION_SCREEN_POST_PROFILE::ZOOM_BLUR_RECONSTRUCTED:
			iPassIndex = DEFERRED_PASS_ZOOM_BLUR;
			break;
		case PRESENTATION_SCREEN_POST_PROFILE::FILM_NOISE_RECONSTRUCTED:
			iPassIndex = DEFERRED_PASS_FILM_NOISE;
			break;
		case PRESENTATION_SCREEN_POST_PROFILE::CHROMATIC_ABERRATION_RECONSTRUCTED:
			iPassIndex = DEFERRED_PASS_CHROMATIC_ABERRATION;
			break;
		case PRESENTATION_SCREEN_POST_PROFILE::PREPARED_MATERIAL:
			iPassIndex = 0u;
			break;
		default:
			hResult = E_FAIL;
			continue;
		}
		hResult = Render_ScreenPostPass(
			m_pScenePostSRVs[Step.iSourceTarget],
			m_pScenePostRTVs[Step.iDestinationTarget],
			iPassIndex, m_pSceneBloomPostSRVs[Step.iSourceTarget],
			m_pSceneBloomPostRTVs[Step.iDestinationTarget], &Post);
		if (hResult == S_OK)
			++composedPostCount;
	}
	const vector<PRESENTATION_SCREEN_OVERLAY_DESC>& ScreenOverlays =
		Presentation.Get_ScreenOverlays();
	size_t hdrOverlayCount = 0u;
	for (size_t iOverlay = 0u;
		SUCCEEDED(hResult) && iOverlay < ScreenOverlays.size(); ++iOverlay)
	{
		const PRESENTATION_SCREEN_OVERLAY_DESC& Overlay =
			ScreenOverlays[iOverlay];
		if (Overlay.bDisplaySpace)
			continue;
		const PRESENTATION_SCREEN_POST_PLAN_STEP Step =
			Build_PresentationScreenOverlayPlanStep(
				composedPostCount, hdrOverlayCount++);
		ComPtr<ID3D11ShaderResourceView> pSourceSRV =
			m_pScenePostSRVs[Step.iSourceTarget];
		ComPtr<ID3D11RenderTargetView> pDestinationRTV =
			m_pScenePostRTVs[Step.iDestinationTarget];
		if (nullptr == pSourceSRV || nullptr == pDestinationRTV ||
			nullptr == Overlay.pTexture)
		{
			hResult = E_FAIL;
			break;
		}
		ComPtr<ID3D11Resource> pSourceResource;
		ComPtr<ID3D11Resource> pDestinationResource;
		ComPtr<ID3D11Resource> pOverlayResource;
		pSourceSRV->GetResource(&pSourceResource);
		pDestinationRTV->GetResource(&pDestinationResource);
		Overlay.pTexture->GetResource(&pOverlayResource);
		if (nullptr == pSourceResource || nullptr == pDestinationResource ||
			nullptr == pOverlayResource ||
			pSourceResource.Get() == pDestinationResource.Get() ||
			pOverlayResource.Get() == pDestinationResource.Get())
		{
			hResult = E_FAIL;
			break;
		}

		ID3D11ShaderResourceView* pNullSRVs[
			D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
		m_pContext->PSSetShaderResources(
			0u, _countof(pNullSRVs), pNullSRVs);
		ID3D11RenderTargetView* destinations[3] = { pDestinationRTV.Get(),
			nullptr, m_pSceneBloomPostRTVs[Step.iDestinationTarget].Get() };
		m_pContext->OMSetRenderTargets(3u, destinations, nullptr);
		const float4_t vClear{};
		m_pContext->ClearRenderTargetView(destinations[0], &vClear.x);
		m_pContext->ClearRenderTargetView(destinations[2], &vClear.x);
		if (FAILED(m_pShader->Bind_Texture("g_PostProcessTexture", pSourceSRV)) ||
			FAILED(m_pShader->Bind_Texture("g_PostBloomTexture", m_pSceneBloomPostSRVs[Step.iSourceTarget])) ||
			FAILED(Render_ScreenOverlay(Overlay)))
			hResult = E_FAIL;
	}
	m_iScenePostFinalTarget = PresentationScreenCompositionFinalTarget(
		composedPostCount, hdrOverlayCount);

	ID3D11ShaderResourceView* pNullSRVs[
		D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	m_pContext->PSSetShaderResources(
		0u, _countof(pNullSRVs), pNullSRVs);
	ID3D11RenderTargetView* pOriginal = pOriginalRTV.Get();
	m_pContext->OMSetRenderTargets(
		nullptr == pOriginal ? 0u : 1u,
		nullptr == pOriginal ? nullptr : &pOriginal,
		pOriginalDSV.Get());
	if (0u < iViewportCount)
		m_pContext->RSSetViewports(iViewportCount, OriginalViewports);
	else
		m_pContext->RSSetViewports(0u, nullptr);
	Presentation.Clear_ScreenPosts();
	return hResult;
}

HRESULT CRenderer::Render_ScreenOverlay(
	const PRESENTATION_SCREEN_OVERLAY_DESC& Overlay)
{
	const uint32_t iCoverageChannel =
		static_cast<uint32_t>(Overlay.eCoverageChannel);
	const uint32_t iFilter = static_cast<uint32_t>(Overlay.eFilter);
	const uint32_t iAddress = static_cast<uint32_t>(Overlay.eAddress);
	if (FAILED(m_pShader->Bind_RawValue("g_fEffectBloomIntensity", &Overlay.fBloomIntensity, sizeof(Overlay.fBloomIntensity))) ||
		FAILED(m_pShader->Bind_Texture(
			"g_PresentationOverlayTexture", Overlay.pTexture)) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fPresentationTime", &Overlay.fSampleTimeSeconds,
			sizeof(Overlay.fSampleTimeSeconds))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vPresentationOverlayPosition", &Overlay.vPosition,
			sizeof(Overlay.vPosition))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vPresentationOverlayScale", &Overlay.vScale,
			sizeof(Overlay.vScale))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fPresentationOverlayRotationDegrees",
			&Overlay.fRotationDegrees,
			sizeof(Overlay.fRotationDegrees))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fPresentationOverlayAngularVelocityDegreesPerSecond",
			&Overlay.fAngularVelocityDegreesPerSecond,
			sizeof(Overlay.fAngularVelocityDegreesPerSecond))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vPresentationOverlayUvDriftPerSecond",
			&Overlay.vUvDriftPerSecond,
			sizeof(Overlay.vUvDriftPerSecond))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vPresentationOverlayTint", &Overlay.vTint,
			sizeof(Overlay.vTint))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fPresentationOverlayAlpha", &Overlay.fAlpha,
			sizeof(Overlay.fAlpha))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_iPresentationOverlayCoverageChannel",
			&iCoverageChannel, sizeof(iCoverageChannel))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_iPresentationOverlayFilter", &iFilter,
			sizeof(iFilter))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_iPresentationOverlayAddress", &iAddress,
			sizeof(iAddress))) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_WorldMatrix", &m_WorldMatrix)) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_ViewMatrix", &m_ViewMatrix)) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_ProjMatrix", &m_ProjMatrix)) ||
		FAILED(m_pShader->Begin(Overlay.bDisplaySpace ?
			ETOUI(DEFERRED::PRESENTATION_DISPLAY_OVERLAY) :
			DEFERRED_PASS_TEXTURED_OVERLAY)) ||
		FAILED(m_pVIBuffer->Bind_Resources()) ||
		FAILED(m_pVIBuffer->Render()))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CRenderer::Render_DisplayOverlays()
{
	CPresentation_Manager& Presentation = CPresentation_Manager::Get();
	HRESULT result = S_OK;
	for (const auto& overlay : Presentation.Get_ScreenOverlays())
	{
		if (!overlay.bDisplaySpace)
			continue;
		// No scene SRV is sampled: alpha blend directly over the final display.
		if (FAILED(m_pShader->Bind_Texture("g_PostProcessTexture", nullptr)) ||
			FAILED(Render_ScreenOverlay(overlay)))
		{
			result = E_FAIL;
			break;
		}
	}
	ID3D11ShaderResourceView* nullSrvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	m_pContext->PSSetShaderResources(0u, _countof(nullSrvs), nullSrvs);
	Presentation.Clear_ScreenOverlays();
	return result;
}

HRESULT CRenderer::Render_Bloom()
{
	D3D11_VIEWPORT OriginalViewports[
		D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
	UINT iViewportCount = _countof(OriginalViewports);
	m_pContext->RSGetViewports(&iViewportCount, OriginalViewports);

	SetUp_ViewportDesc(m_iBloomWidth, m_iBloomHeight);

	HRESULT hResult = Render_BloomPass(
		TEXT("MRT_BloomExtract"),
		m_pSceneBloomPostSRVs[m_iScenePostFinalTarget],
		DEFERRED::BLOOM_EXTRACT);
	if (SUCCEEDED(hResult))
	{
		hResult = Render_BloomPass(
			TEXT("MRT_BloomPing"), nullptr,
			DEFERRED::BLOOM_BLUR_H);
	}
	if (SUCCEEDED(hResult))
	{
		hResult = Render_BloomPass(
			TEXT("MRT_BloomResult"), nullptr,
			DEFERRED::BLOOM_BLUR_V);
	}

	/* Restore every viewport exactly as it was before the half-resolution pass. */
	if (0 < iViewportCount)
		m_pContext->RSSetViewports(iViewportCount, OriginalViewports);
	else
		m_pContext->RSSetViewports(0, nullptr);

	return hResult;
}

HRESULT CRenderer::Render_BloomPass(const wstring_t& strMRTTag,
	ComPtr<ID3D11ShaderResourceView> pSourceSRV, DEFERRED ePass)
{
	if (FAILED(CGameInstance::Get().Begin_MRT(strMRTTag, m_pBloomDSV)))
		return E_FAIL;

	HRESULT hResult = S_OK;
	HRESULT hBindSource = S_OK;
	if (nullptr != pSourceSRV)
		hBindSource = m_pShader->Bind_Texture(
			"g_PostProcessTexture", pSourceSRV);
	else if (DEFERRED::BLOOM_BLUR_H == ePass)
		hBindSource = CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_BloomExtract"), m_pShader,
			"g_PostProcessTexture");
	else if (DEFERRED::BLOOM_BLUR_V == ePass)
		hBindSource = CGameInstance::Get().Bind_RT_SRV(
			TEXT("Target_BloomPing"), m_pShader,
			"g_PostProcessTexture");
	else
		hBindSource = E_FAIL;
	if (FAILED(hBindSource) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vBloomTexelSize", &m_vBloomTexelSize,
			sizeof(m_vBloomTexelSize))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fBloomThreshold", &m_RenderQualitySettings.fBloomThreshold,
			sizeof(m_RenderQualitySettings.fBloomThreshold))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fBloomSoftKnee", &m_RenderQualitySettings.fBloomSoftKnee,
			sizeof(m_RenderQualitySettings.fBloomSoftKnee))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fBloomScatter", &m_RenderQualitySettings.fBloomScatter,
			sizeof(m_RenderQualitySettings.fBloomScatter))) ||
		FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)) ||
		FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)) ||
		FAILED(m_pShader->Begin(ETOUI(ePass))) ||
		FAILED(m_pVIBuffer->Bind_Resources()) ||
		FAILED(m_pVIBuffer->Render()))
	{
		hResult = E_FAIL;
	}

	/* Keep render-target state balanced even if binding or drawing failed. */
	if (FAILED(CGameInstance::Get().End_MRT()))
		hResult = E_FAIL;

	return hResult;
}

/* bPortrait resolves an offscreen portrait instead of the frame: it reads Target_SceneHDR
directly because the screen post chain has not run for it, drops bloom and any material debug
view, and selects the pass that leaves the background transparent. */
HRESULT CRenderer::Render_Final(bool_t bPortrait)
{
	const uint32_t portrait = bPortrait ? 1u : 0u;
	if (FAILED(m_pShader->Bind_RawValue("g_iPortraitCapture", &portrait, sizeof(portrait)))) return E_FAIL;
	const uint32_t materialView = bPortrait ? 0u :
		static_cast<uint32_t>(m_MaterialRenderSettings.eDebugView);
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Shade"), m_pShader, "g_ShadeTexture"))) return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue("g_MaterialDebugView", &materialView, sizeof(materialView))) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Diffuse"), m_pShader, "g_DiffuseTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Normal"), m_pShader, "g_NormalTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Depth"), m_pShader, "g_DepthTexture")) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_CharacterSurface"), m_pShader, "g_CharacterSurfaceTexture")) ||
        FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_CharacterGeometry"), m_pShader, "g_CharacterGeometryTexture")) ||
		FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_Specular"), m_pShader, "g_SpecularTexture")))
		return E_FAIL;
	const uint32_t iBloomEnabled =
		!bPortrait && m_RenderQualitySettings.bBloomEnabled ? 1u : 0u;
	const uint32_t iFXAAEnabled =
		m_RenderQualitySettings.bFXAAEnabled ? 1u : 0u;
	const float2_t vInverseSceneSize = {
		1.f / static_cast<f32_t>((max)(1u, m_iScenePostWidth)),
		1.f / static_cast<f32_t>((max)(1u, m_iScenePostHeight)) };

	const auto& source = m_RenderQualitySettings.SourcePostProcess;
	const int32_t sourceEnabled = source.bEnabled ? 1 : 0;
	const float4_t sourceCurve = PackSourceToneCurve(source);
	if (FAILED(m_pShader->Bind_RawValue("g_iSourcePostProcessEnabled", &sourceEnabled, sizeof(sourceEnabled))) ||
		FAILED(m_pShader->Bind_RawValue("g_vSourceToneCurve", &sourceCurve, sizeof(sourceCurve))) ||
		FAILED(m_pShader->Bind_RawValue("g_fSourceToneToe", &source.fToneToe, sizeof(source.fToneToe))) ||
		FAILED(m_pShader->Bind_Texture("g_SourceGradingLut", m_pSourceGradingLut))) return E_FAIL;

	if (FAILED(bPortrait ?
			CGameInstance::Get().Bind_RT_SRV(TEXT("Target_SceneHDR"),
				m_pShader, "g_SceneHDRTexture") :
			m_pShader->Bind_Texture("g_SceneHDRTexture",
				m_pScenePostSRVs[m_iScenePostFinalTarget])))
		return E_FAIL;
	if (FAILED(CGameInstance::Get().Bind_RT_SRV(TEXT("Target_BloomResult"), m_pShader, "g_BloomTexture")))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_RawValue(
			"g_iBloomEnabled", &iBloomEnabled, sizeof(iBloomEnabled))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fToneMapExposure", &m_RenderQualitySettings.fExposure,
			sizeof(m_RenderQualitySettings.fExposure))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fToneMapWhitePoint", &m_RenderQualitySettings.fWhitePoint,
			sizeof(m_RenderQualitySettings.fWhitePoint))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fToneMapGamma", &m_RenderQualitySettings.fGamma,
			sizeof(m_RenderQualitySettings.fGamma))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vSceneBloomTint", &m_RenderQualitySettings.vBloomTint,
			sizeof(m_RenderQualitySettings.vBloomTint))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fSceneDesaturation", &m_RenderQualitySettings.fSceneDesaturation,
			sizeof(m_RenderQualitySettings.fSceneDesaturation))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_iColorFilterType", &m_RenderQualitySettings.iColorFilterType,
			sizeof(m_RenderQualitySettings.iColorFilterType))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fColorFilterStrength", &m_RenderQualitySettings.fColorFilterStrength,
			sizeof(m_RenderQualitySettings.fColorFilterStrength))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_iFXAAEnabled", &iFXAAEnabled, sizeof(iFXAAEnabled))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fFXAASubpixel", &m_RenderQualitySettings.fFXAASubpixel,
			sizeof(m_RenderQualitySettings.fFXAASubpixel))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fFXAAEdgeThreshold", &m_RenderQualitySettings.fFXAAEdgeThreshold,
			sizeof(m_RenderQualitySettings.fFXAAEdgeThreshold))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_fFXAAEdgeThresholdMin", &m_RenderQualitySettings.fFXAAEdgeThresholdMin,
			sizeof(m_RenderQualitySettings.fFXAAEdgeThresholdMin))) ||
		FAILED(m_pShader->Bind_RawValue(
			"g_vInverseSceneSize", &vInverseSceneSize,
			sizeof(vInverseSceneSize))))
	{
		return E_FAIL;
	}

	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pShader->Begin(ETOUI(bPortrait ?
			DEFERRED::PORTRAIT_RESOLVE : DEFERRED::FINAL))))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Resources()))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_UI()
{
	/* UI surfaces submit in the order their owners update, which is creation order and says
	nothing about which one is in front. Drawing follows Get_UISortLayer instead, and
	stable_sort keeps submission order inside one layer so a document's own slots still draw
	in their authored sequence. The scratch buffer is function-local static for the same
	reason the blend one is: rendering is single-threaded and this runs every frame. */
	static std::vector<CGameObject*> SortedUIObjects;
	SortedUIObjects.clear();
	SortedUIObjects.reserve(m_RenderObjects[ETOUI(RENDERGROUP::UI)].size());
	for (size_t renderIndex = 0; renderIndex < m_RenderObjects[ETOUI(RENDERGROUP::UI)].size(); ++renderIndex)
	{
		CGameObject* const pRenderObject = m_RenderObjects[ETOUI(RENDERGROUP::UI)][renderIndex].get();
		if (nullptr != pRenderObject)
			SortedUIObjects.push_back(pRenderObject);
	}
	std::stable_sort(SortedUIObjects.begin(), SortedUIObjects.end(),
		[](const CGameObject* pLeft, const CGameObject* pRight)
		{
			return pLeft->Get_UISortLayer() < pRight->Get_UISortLayer();
		});
	for (CGameObject* const pRenderObject : SortedUIObjects)
		pRenderObject->Render_Group(RENDERGROUP::UI);

	m_RenderObjects[ETOUI(RENDERGROUP::UI)].clear();

	return S_OK;
}

HRESULT CRenderer::Capture_PickingDepth()
{
	m_bPickingDepthCaptured = false;
	if (m_RenderObjects[ETOUI(RENDERGROUP::PICKING)].empty())
		return S_OK;

	ScopedSourceLutState restore(m_pContext.Get());
	if (!restore.depth)
		return E_FAIL;
	ComPtr<ID3D11Resource> resource;
	restore.depth->GetResource(resource.GetAddressOf());
	ComPtr<ID3D11Texture2D> texture;
	HRESULT result = resource.As(&texture);
	if (FAILED(result))
		return result;
	D3D11_TEXTURE2D_DESC desc{};
	texture->GetDesc(&desc);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = 0u;
	desc.MiscFlags = 0u;
	D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc{};
	restore.depth->GetDesc(&viewDesc);
	if (!m_pPickingDepthTexture || !m_pPickingDSV ||
		std::memcmp(&desc, &m_PickingDepthDesc, sizeof(desc)) != 0 ||
		std::memcmp(&viewDesc, &m_PickingDSVDesc, sizeof(viewDesc)) != 0)
	{
		ComPtr<ID3D11Texture2D> stagedTexture;
		ComPtr<ID3D11DepthStencilView> stagedView;
		result = m_pDevice->CreateTexture2D(&desc, nullptr, stagedTexture.GetAddressOf());
		if (SUCCEEDED(result))
			result = m_pDevice->CreateDepthStencilView(stagedTexture.Get(), &viewDesc,
				stagedView.GetAddressOf());
		if (FAILED(result))
			return result;
		m_pPickingDepthTexture = std::move(stagedTexture);
		m_pPickingDSV = std::move(stagedView);
		m_PickingDepthDesc = desc;
		m_PickingDSVDesc = viewDesc;
	}

	ID3D11ShaderResourceView* empty[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	m_pContext->PSSetShaderResources(0u, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, empty);
	m_pContext->OMSetRenderTargets(0u, nullptr, nullptr);
	m_pContext->CopyResource(m_pPickingDepthTexture.Get(), texture.Get());
	m_bPickingDepthCaptured = true;
	return S_OK;
}

HRESULT CRenderer::Render_Picking()
{
	auto& objects = m_RenderObjects[ETOUI(RENDERGROUP::PICKING)];
	if (objects.empty())
		return S_OK;

	const auto source = CGameInstance::Get().Get_RT_SRV(TEXT("Target_PickPos"));
	if (!source)
		return E_FAIL;
	ComPtr<ID3D11Resource> resource;
	source->GetResource(resource.GetAddressOf());
	ComPtr<ID3D11Texture2D> texture;
	HRESULT result = resource.As(&texture);
	if (FAILED(result))
		return result;
	D3D11_TEXTURE2D_DESC targetDesc{};
	texture->GetDesc(&targetDesc);
	if (texture.Get() != m_pPickingTexture.Get() || !m_pPickingRTV)
	{
		ComPtr<ID3D11RenderTargetView> view;
		result = m_pDevice->CreateRenderTargetView(texture.Get(), nullptr, view.GetAddressOf());
		if (FAILED(result))
			return result;
		m_pPickingTexture = texture;
		m_pPickingRTV = std::move(view);
	}

	if (!m_bPickingDepthCaptured || !m_pPickingDSV ||
		m_PickingDepthDesc.Width != targetDesc.Width ||
		m_PickingDepthDesc.Height != targetDesc.Height ||
		m_PickingDepthDesc.SampleDesc.Count != targetDesc.SampleDesc.Count ||
		m_PickingDepthDesc.SampleDesc.Quality != targetDesc.SampleDesc.Quality)
		return E_FAIL;

	ScopedSourceLutState restore(m_pContext.Get());
	// Begin_MRT clears every attached target, which would erase opaque picks.
	// The opaque-depth snapshot excludes later UI/effects; writes to this private
	// DSV resolve the nearest receiver without changing the visual scene's depth.
	ID3D11ShaderResourceView* empty[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]{};
	m_pContext->PSSetShaderResources(0u, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, empty);
	ID3D11RenderTargetView* target = m_pPickingRTV.Get();
	m_pContext->OMSetRenderTargets(1u, &target, m_pPickingDSV.Get());
	SetUp_ViewportDesc(targetDesc.Width, targetDesc.Height);
	for (const auto& object : objects)
	{
		if (!object)
			continue;
		result = object->Render_Group(RENDERGROUP::PICKING);
		if (FAILED(result))
			return result; // Draw's FailFrame clears every queue after state restoration.
	}
	objects.clear();
	m_bPickingDepthCaptured = false;
	return S_OK;
}

HRESULT CRenderer::Ready_SourceLightMask(uint32_t width, uint32_t height)
{
    if (width == 0u || height == 0u) return S_FALSE;
    if (width == m_iSourceLightMaskFailedWidth && height == m_iSourceLightMaskFailedHeight)
        return S_FALSE;
    if (m_pSourceLightMaskDSV && width == m_iSourceLightMaskWidth && height == m_iSourceLightMaskHeight)
        return S_OK;
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = desc.ArraySize = 1u;
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.SampleDesc.Count = 1u;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11DepthStencilView> depth;
    HRESULT result = m_pDevice->CreateTexture2D(&desc, nullptr, texture.GetAddressOf());
    if (SUCCEEDED(result)) result = m_pDevice->CreateDepthStencilView(texture.Get(), nullptr, depth.GetAddressOf());
    if (FAILED(result))
    {
        m_iSourceLightMaskFailedWidth = width;
        m_iSourceLightMaskFailedHeight = height;
        WriteRendererFailure("SourceLightMask_ResourceFallback", result);
        return S_FALSE;
    }
    m_pSourceLightMaskDSV = std::move(depth);
    m_iSourceLightMaskWidth = width;
    m_iSourceLightMaskHeight = height;
    return S_OK;
}

HRESULT CRenderer::Ready_Shadow_Resources()
{
	D3D11_TEXTURE2D_DESC TextureDesc{};
	TextureDesc.Width = m_iShadowMapSize;
	TextureDesc.Height = m_iShadowMapSize;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags =
		D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	ComPtr<ID3D11Texture2D> pStagedTexture;
	if (FAILED(m_pDevice->CreateTexture2D(
		&TextureDesc, nullptr, pStagedTexture.GetAddressOf())))
		return E_FAIL;

	D3D11_DEPTH_STENCIL_VIEW_DESC DSVDesc{};
	DSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
	DSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSVDesc.Texture2D.MipSlice = 0;
	ComPtr<ID3D11DepthStencilView> pStagedDSV;
	if (FAILED(m_pDevice->CreateDepthStencilView(
		pStagedTexture.Get(), &DSVDesc, pStagedDSV.GetAddressOf())))
		return E_FAIL;

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	SRVDesc.Texture2D.MipLevels = 1;
	ComPtr<ID3D11ShaderResourceView> pStagedSRV;
	if (FAILED(m_pDevice->CreateShaderResourceView(
		pStagedTexture.Get(), &SRVDesc, pStagedSRV.GetAddressOf())))
	{
		return E_FAIL;
	}

	m_pShadowDepthTexture = std::move(pStagedTexture);
	m_pShadowDSV = std::move(pStagedDSV);
	m_pShadowSRV = std::move(pStagedSRV);
    m_bStaticShadowCacheValid = false;
    m_StaticShadowCasters.clear();
    m_CandidateStaticShadowCasters.clear();
    m_pStaticShadowDepthTexture.Reset();
    m_pStaticShadowSRV.Reset();
    // Read the existing static cache to distinguish moving occluders from
    // shadows already baked into RNM. Failure preserves ordinary shadows.
    TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    if (SUCCEEDED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr,
        m_pStaticShadowDepthTexture.GetAddressOf())))
        (void)m_pDevice->CreateShaderResourceView(m_pStaticShadowDepthTexture.Get(),
            &SRVDesc, m_pStaticShadowSRV.GetAddressOf());
	return S_OK;
}

HRESULT CRenderer::Ready_Bloom_DSV()
{
	ComPtr<ID3D11Texture2D> pDepthStencilTexture = { nullptr };

	D3D11_TEXTURE2D_DESC TextureDesc{};
	TextureDesc.Width = m_iBloomWidth;
	TextureDesc.Height = m_iBloomHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	if (FAILED(m_pDevice->CreateTexture2D(
		&TextureDesc, nullptr, pDepthStencilTexture.GetAddressOf())))
	{
		return E_FAIL;
	}

	if (FAILED(m_pDevice->CreateDepthStencilView(
		pDepthStencilTexture.Get(), nullptr, m_pBloomDSV.GetAddressOf())))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CRenderer::Ready_SSAO_DSV()
{
	ComPtr<ID3D11Texture2D> pDepthStencilTexture = { nullptr };

	D3D11_TEXTURE2D_DESC TextureDesc{};
	TextureDesc.Width = m_iSSAOWidth;
	TextureDesc.Height = m_iSSAOHeight;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	if (FAILED(m_pDevice->CreateTexture2D(
		&TextureDesc, nullptr, pDepthStencilTexture.GetAddressOf())))
	{
		return E_FAIL;
	}

	if (FAILED(m_pDevice->CreateDepthStencilView(
		pDepthStencilTexture.Get(), nullptr, m_pSSAODSV.GetAddressOf())))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CRenderer::Ready_ScenePostTargets(
	const uint32_t iWidth, const uint32_t iHeight)
{
	if (0u == iWidth || 0u == iHeight)
		return E_FAIL;
	if (m_iScenePostWidth == iWidth && m_iScenePostHeight == iHeight &&
		nullptr != m_pScenePostTextures[0] &&
		nullptr != m_pScenePostTextures[1] &&
		nullptr != m_pScenePostRTVs[0] &&
		nullptr != m_pScenePostRTVs[1] &&
		nullptr != m_pScenePostSRVs[0] &&
		nullptr != m_pScenePostSRVs[1] &&
		nullptr != m_pSceneBloomPostSRVs[0] && nullptr != m_pSceneBloomPostSRVs[1])
	{
		return S_OK;
	}

	ComPtr<ID3D11Texture2D> StagedTextures[2];
	ComPtr<ID3D11RenderTargetView> StagedRTVs[2];
	ComPtr<ID3D11ShaderResourceView> StagedSRVs[2];
	ComPtr<ID3D11Texture2D> StagedBloomTextures[2];
	ComPtr<ID3D11RenderTargetView> StagedBloomRTVs[2];
	ComPtr<ID3D11ShaderResourceView> StagedBloomSRVs[2];
	D3D11_TEXTURE2D_DESC TextureDesc{};
	TextureDesc.Width = iWidth;
	TextureDesc.Height = iHeight;
	TextureDesc.MipLevels = 1u;
	TextureDesc.ArraySize = 1u;
	TextureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	TextureDesc.SampleDesc.Count = 1u;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags =
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	for (size_t iTarget = 0u; iTarget < 2u; ++iTarget)
	{
		if (FAILED(m_pDevice->CreateTexture2D(
				&TextureDesc, nullptr,
				StagedTextures[iTarget].GetAddressOf())) ||
			FAILED(m_pDevice->CreateRenderTargetView(
				StagedTextures[iTarget].Get(), nullptr,
				StagedRTVs[iTarget].GetAddressOf())) ||
			FAILED(m_pDevice->CreateShaderResourceView(
				StagedTextures[iTarget].Get(), nullptr,
				StagedSRVs[iTarget].GetAddressOf())) ||
			FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, StagedBloomTextures[iTarget].GetAddressOf())) ||
			FAILED(m_pDevice->CreateRenderTargetView(StagedBloomTextures[iTarget].Get(), nullptr, StagedBloomRTVs[iTarget].GetAddressOf())) ||
			FAILED(m_pDevice->CreateShaderResourceView(StagedBloomTextures[iTarget].Get(), nullptr, StagedBloomSRVs[iTarget].GetAddressOf())))
		{
			return E_FAIL;
		}
	}

	for (size_t iTarget = 0u; iTarget < 2u; ++iTarget)
	{
		m_pScenePostTextures[iTarget] = std::move(StagedTextures[iTarget]);
		m_pScenePostRTVs[iTarget] = std::move(StagedRTVs[iTarget]);
		m_pScenePostSRVs[iTarget] = std::move(StagedSRVs[iTarget]);
		m_pSceneBloomPostRTVs[iTarget] = std::move(StagedBloomRTVs[iTarget]);
		m_pSceneBloomPostSRVs[iTarget] = std::move(StagedBloomSRVs[iTarget]);
	}
	m_iScenePostWidth = iWidth;
	m_iScenePostHeight = iHeight;
	m_iScenePostFinalTarget = 0u;
	XMStoreFloat4x4(&m_WorldMatrix,
		XMMatrixScaling(static_cast<f32_t>(iWidth),
			static_cast<f32_t>(iHeight), 1.f));
	XMStoreFloat4x4(&m_ProjMatrix,
		XMMatrixOrthographicLH(static_cast<f32_t>(iWidth),
			static_cast<f32_t>(iHeight), 0.f, 1.f));
	return S_OK;
}

void CRenderer::SetUp_ViewportDesc(uint32_t iWidth, uint32_t iHeight)
{
	D3D11_VIEWPORT			ViewPortDesc;
	ZeroMemory(&ViewPortDesc, sizeof(D3D11_VIEWPORT));
	ViewPortDesc.TopLeftX = 0;
	ViewPortDesc.TopLeftY = 0;
	ViewPortDesc.Width = static_cast<f32_t>(iWidth);
	ViewPortDesc.Height = static_cast<f32_t>(iHeight);
	ViewPortDesc.MinDepth = 0.f;
	ViewPortDesc.MaxDepth = 1.f;

	m_pContext->RSSetViewports(1, &ViewPortDesc);
}


HRESULT CRenderer::Render_Debug()
{
	if (m_DebugComponent.empty())
		return S_OK;
	for (auto& pDebugComponent : m_DebugComponent)
	{
		if (nullptr != pDebugComponent)
			pDebugComponent->Render();
	}
	m_DebugComponent.clear();

	if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pVIBuffer->Bind_Resources()))
		return E_FAIL;

	//if (FAILED(CGameInstance::Get().Render_MRT(TEXT("MRT_GameObject"), m_pShader, m_pVIBuffer)))
	//	return E_FAIL;

	//if (FAILED(CGameInstance::Get().Render_MRT(TEXT("MRT_LightAcc"), m_pShader, m_pVIBuffer)))
	//	return E_FAIL;

	return S_OK;
}


unique_ptr<CRenderer> CRenderer::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CRenderer>(new CRenderer(pDevice, pContext));

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CRenderer");
		return nullptr;
	}

	return pInstance;
}
```

#### C:/Users/tnest/Desktop/LostArk/Engine/Public/GameInstance.h

Bind_ActiveSceneColorTransform의 facade 선언.

SHA256: `4c47c8cdfddcb655488e2162a12e2b1b8cb64197117f1df1c9212a475f2ca406`

```cpp
#pragma once

#include "Prototype_Manager.h"
#pragma push_macro("new")
#undef new
#include <DirectXColors.h>
#pragma pop_macro("new")
#include "Engine_RenderFwd.h"
#include "Sound/Sound_Manager.h"

NS_BEGIN(Engine)

class ENGINE_DLL CGameInstance final
{
	DECLARE_SINGLETON(CGameInstance)
	friend class CCamera;
private:
	CGameInstance();
public:
	~CGameInstance();

public:
	float2_t Get_ViewportSize() {
		return m_vViewportDesc;
	}

	f32_t Random(f32_t fMin = 0.f, f32_t fMax = 1.f) {
		return fMin + (static_cast<f32_t>(rand()) / RAND_MAX) * (fMax - fMin);
	}

public:
	HRESULT Initialize_Engine(const ENGINE_DESC& EngineDesc,
		ComPtr<ID3D11Device>& pOutDevice, ComPtr<ID3D11DeviceContext>& pOutContext);
	// Physical client pixels; call between frames, before update and rendering.
	HRESULT Resize_Viewport(uint32_t width, uint32_t height);
	HRESULT Set_FullscreenMode(bool fullscreen, uint32_t width, uint32_t height);
	void Update_Engine(f32_t fTimeDelta);
	void Submit_FinalCameraObjects();
	HRESULT Render_Begin(const float4_t* pClearColor);
	// The caller opts into static occlusion for this frame without changing session settings.
	HRESULT Render(bool staticOcclusionAllowed = false);
	HRESULT Render_End();
	HRESULT Clear_Resources(uint32_t iClearLevelID);

public: /* For.Graphic_Device */

public: /* For.Input_Device */
	int8_t	Get_DIKeyState(uint8_t byKeyID);
	bool_t Get_DIKeyPressed(uint8_t byKeyID);
	int8_t Get_DIKeyStateRaw(uint8_t byKeyID) const;
	bool_t Get_DIKeyPressedRaw(uint8_t byKeyID) const;
	int8_t	Get_DIMouseState(DIM eMouse);
	int8_t Get_DIMouseStateRaw(DIM eMouse) const;
	int32_t	Get_DIMouseMove(DIMM eMouseState);
	void SetInputBlocked(bool_t bKeyboardBlocked, bool_t bMouseBlocked);
	void SetMouseButtonBlocked(DIM eMouse, bool_t blocked);
	bool_t IsKeyboardInputBlocked() const;
	bool_t IsMouseInputBlocked() const;

#ifdef _WIN64
public: /* For.Sound_Manager */
	HRESULT Play_Sound(const wstring_t& strSoundFilePath, f32_t fVolume);
	uint64_t Play_SoundCue(const wstring_t& path, f32_t volume, uint32_t ageMs = 0u, bool_t paused = false, f32_t playbackRate = 1.f, uint32_t endMs = 0u);
	bool_t Get_SoundDurationMs(const wstring_t& path, uint32_t& durationMs);
	bool_t Is_SoundCueActive(uint64_t handle) const;
	void Pause_SoundCue(uint64_t handle, bool_t paused);
    void Set_SoundCuePlaybackRate(uint64_t handle, f32_t playbackRate);
	void Seek_SoundCue(uint64_t handle, uint32_t ageMs);
    // Reconcile a live cue with an externally owned clock without seeking every frame.
    // False means the caller's live cue has ended or become unavailable.
    bool_t Synchronize_SoundCue(uint64_t handle, uint32_t ageMs, uint32_t maxDriftMs = 100u);
	void Stop_SoundCue(uint64_t handle);

	HRESULT Play_LoopingSound(const wstring_t& strSoundFilePath, f32_t fVolume);
	void Stop_LoopingSound();
	HRESULT Play_Music(const wstring_t& strSoundFilePath, f32_t fVolume,
		bool_t bLoop = true);
	void Stop_Music();
	/* Mixer bus volume the system option window owns; 0..1, applied to sounds that
	are already playing on that bus. */
	HRESULT Apply_SoundCategoryVolume(SOUND_CATEGORY eCategory, f32_t fVolume);
	f32_t Get_SoundCategoryVolume(SOUND_CATEGORY eCategory) const;
	/* See CSound_Manager::Set_MuteOnFocusLoss (the sound-in-background option, inverted). */
	void Set_SoundMuteOnFocusLoss(bool_t bMute);
#endif


public: /* For.Timer_Manager */
	f32_t Get_TimeDelta(const wstring_t& strTimerTag);
	HRESULT Add_Timer(const wstring_t& strTimerTag);
	void Update_TimeDelta(const wstring_t& strTimerTag);

public: /* For.Level_Manager */
	HRESULT Change_Level(uint32_t iCurrentLevelID, unique_ptr<class CLevel> pNewLevel);
	uint32_t Get_CurrentLevelID() const;

public: /* For.Prototype_Manager */
	HRESULT Add_Prototype(uint32_t iLevelIndex, const wstring_t& strPrototypeTag, unique_ptr<class CPrototype> pPrototype);
	HRESULT Add_Prototypes(
		uint32_t iLevelIndex,
		vector<pair<wstring_t, unique_ptr<class CPrototype>>>&& prototypes);
	shared_ptr<CPrototype> Clone_Prototype(uint32_t iLevelIndex, const wstring_t& strPrototypeTag, void* pArg = nullptr);

public: /* For.Object_Manager */
	shared_ptr<class CGameObject> Get_GameObject(uint32_t iLevelIndex, const wstring_t& strLayerTag, uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(uint32_t iLevelIndex, const wstring_t& strLayerTag, const wstring_t& strComponentTag, uint32_t iIndex);
	shared_ptr<CComponent> Get_Component(uint32_t iLevelIndex, const wstring_t& strLayerTag, const wstring_t& strPartTag, const wstring_t& strComponentTag, uint32_t iIndex);
	HRESULT Add_GameObject_to_Layer(uint32_t iPrototypeLevelIndex, const wstring_t& strPrototypeTag, uint32_t iLayerLevelIndex, const wstring_t& strLayerTag, void* pArg = nullptr, shared_ptr<class CGameObject>* pOutGameObject = nullptr);
	HRESULT Remove_GameObject_from_Layer(uint32_t iLevelIndex, const wstring_t& strLayerTag, const shared_ptr<class CGameObject>& pGameObject);

public: /* Renderer */
	HRESULT Add_RenderObject(RENDERGROUP eRenderGroupID, shared_ptr<class CGameObject> pRenderObject);
    void Register_GpuSceneVisibility(const shared_ptr<class CGpuSceneVisibility>& scene);
    void Unregister_GpuSceneVisibility(const class CGpuSceneVisibility* scene);
	/* Queues one off-screen portrait for the next Draw, which renders it through the same
	deferred path the world uses and copies the resolved colour into pDestination. The pieces
	are passed separately so this header does not have to see CRenderer's request type. */
	HRESULT Request_Portrait(ComPtr<ID3D11RenderTargetView> pDestination,
		uint32_t iWidth, uint32_t iHeight,
		const float4x4_t& ViewMatrix, const float4x4_t& ProjMatrix,
		function<HRESULT()> DrawSubject,
		function<HRESULT(RENDERGROUP, ID3D11BlendState*)> DrawForward = {});
	void Request_SceneColorSnapshot();
    void Request_SceneEnvironmentReplacement();
    bool_t Is_SceneEnvironmentReplaced() const;
	HRESULT Refresh_SceneColorSnapshot();
	RENDER_QUALITY_SETTINGS Get_RenderQualitySettings() const;
    MAP_VISIBILITY_SETTINGS Get_MapVisibilitySettings() const;
    HRESULT Apply_MapVisibilitySettings(const MAP_VISIBILITY_SETTINGS& settings);
    RENDER_OPTIMIZATION_SETTINGS Get_RenderOptimizationSettings() const;
    HRESULT Apply_RenderOptimizationSettings(const RENDER_OPTIMIZATION_SETTINGS& settings);
	// Draw-time scalar view avoids copying the quality profile and LUT layers.
	uint32_t Get_TextureMinMip() const;
	HRESULT Apply_RenderQualitySettings(const RENDER_QUALITY_SETTINGS& Settings);
	MATERIAL_RENDER_SETTINGS Get_MaterialRenderSettings() const;
	HRESULT Apply_MaterialRenderSettings(const MATERIAL_RENDER_SETTINGS& settings);
	HEIGHT_FOG_SETTINGS Get_HeightFogSettings() const;
	HRESULT Apply_HeightFog(const HEIGHT_FOG_SETTINGS& Settings);
	HRESULT Bind_HeightFog(class CShader* shader) const;
	HRESULT Bind_ActiveSceneColorTransform(class CShader* shader) const;
	HRESULT Stage_RenderEnvironment(const wstring_t& cubePath, const float4_t& color,
		const float4_t& rotationIntensity, RENDER_ENVIRONMENT_STATE& outState, bool_t forceReload = false) const;
	void Commit_RenderEnvironment(const RENDER_ENVIRONMENT_STATE& state);
	RENDER_ENVIRONMENT_STATE Get_RenderEnvironment() const;

	HRESULT Add_DebugComponent(shared_ptr<CComponent> pDebugComponent);

public: /* For.PipeLine */
	void Set_Transform(D3DTS eType, fmatrix_t TransformMatrix);
	const float4_t* Get_CamPosition();
	const float4x4_t* Get_Transform(D3DTS eType);
	const float4x4_t* Get_InverseTransform(D3DTS eType);
	HRESULT Bind_CamPosition(shared_ptr<class CShader> pShader, const char_t* pConstantName);
	HRESULT Bind_Transform(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType);
	HRESULT Bind_InverseTransform(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType);
	/* Recomputes the inverse matrices and camera position from the transforms set above.
	Rendering an offscreen view mid-frame sets a second camera and has to derive them again. */
	void Refresh_CameraState();


public: /* For.Light */

	HRESULT Add_Light(const LIGHT_DESC& LightDesc);
	const vector<LIGHT_DESC>& Get_SceneLights() const;
	HRESULT Render_Lights(shared_ptr<class CShader> pShader,
		shared_ptr<class CVIBuffer_Rect> pVIBuffer,
		bool_t bEnableSceneDirectionalShadow,
        LIGHT_RECEIVER ePassReceiver = LIGHT_RECEIVER::ALL,
		bool_t bSourceLightMask = false);


public: /* For.Font_Manager */
	HRESULT Add_Font(const wstring& strFontTag, const tchar_t* pFontFilePath);
	void Draw_Text(const wstring& strFontTag, const tchar_t* pText, const float2_t& vPosition, fvector_t vColor = Colors::White, f32_t fRotation = 0.f, const float2_t& vOrigin = float2_t(0.f, 0.f), f32_t fScale = 1.f);
	float2_t Measure_Text(const wstring& strFontTag, const tchar_t* pText);
	/* See CFont_Manager::Set_ClipOutRect: screen rect Draw_Text refuses to draw over. */
	void Set_TextClipOutRect(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight);
	/* One more rect on top of the ones already set (CFont_Manager::Add_ClipOutRect). */
	void Add_TextClipOutRect(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight);
	void Clear_TextClipOutRect();
	/* See CFont_Manager::Set_ClipInRect: screen rect text must stay wholly inside. */
	void Set_TextClipInRect(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight);
	void Clear_TextClipInRect();

public: /* For.Target_Manager */
	HRESULT Add_RenderTarget(const wstring_t& strTargetTag, uint32_t iWidth, uint32_t iHeight, DXGI_FORMAT ePixelFormat, const float4_t& vClearColor);
	HRESULT Add_MRT(const wstring_t& strMRTTag, const wstring_t& strTargetTag);
	HRESULT Begin_MRT(const wstring_t& strMRTTag, ComPtr<ID3D11DepthStencilView> pDSV = nullptr);
	HRESULT End_MRT();
	HRESULT Begin_DepthOnly(ComPtr<ID3D11DepthStencilView> pDSV);
	HRESULT End_DepthOnly();
	HRESULT Bind_RT_SRV(const wstring_t& strTargetTag, shared_ptr<class CShader> pShader, const char_t* pConstantName);
	ComPtr<ID3D11ShaderResourceView> Get_RT_SRV(const wstring_t& strTargetTag) const;
	HRESULT Copy_RT_Resource(const wstring_t& strTargetTag, ComPtr<ID3D11Texture2D> pTexture2D);


#ifdef _DEBUG
public:
	HRESULT Ready_RT_DebugDesc(const wstring_t& strTargetTag, f32_t fX, f32_t fY, f32_t fSizeX, f32_t fSizeY);
	HRESULT Render_MRT(const wstring_t& strMRTTag, shared_ptr<class CShader> pShader, shared_ptr<class CVIBuffer_Rect> pVIBuffer);
#endif


public: /* For.Picking */
	bool_t Picking(float4_t& vOut);
	uint64_t Request_Picking();
	HRESULT Poll_Picking(uint64_t requestId, float4_t& vOut);
	void Cancel_Picking(uint64_t requestId);

public: /* For.Shadow */
	HRESULT Apply_Shadow_Light(const SHADOW_LIGHT_DESC& ShadowLightDesc);
	HRESULT Add_Shadow_Light(const SHADOW_LIGHT_DESC& ShadowLightDesc);
	bool_t Is_ShadowLightEnabled() const;
	const SHADOW_LIGHT_DESC& Get_ShadowLightDesc() const;
	const float4x4_t* Get_ShadowLightTransform(D3DTS eType) const;
	HRESULT Bind_ShadowLight_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType);
	HRESULT Bind_ShadowLight_LightingResources(shared_ptr<class CShader> pShader);

public: /* For.Frustum */
	void Update_Frustum_InLocalSpace(fmatrix_t WorldMatrix);
	bool_t isIn_Frustum_InWorldSpace(fvector_t vWorldPoint, f32_t fRange);
	bool_t isIn_Frustum_InLocalSpace(fvector_t vLocalPoint, f32_t fRange);

public: /* For.Profiler */
	class CProfiler* Get_Profiler() const { return m_pProfiler.get(); }

public: /* For.Physics */
	class CPhysics_Manager* Get_PhysicsManager() const
	{
		return m_pPhysics_Manager.get();
	}

private:
	unique_ptr<class CTimer_Manager>		m_pTimer_Manager = { nullptr };
	unique_ptr<class CGraphic_Device>		m_pGraphic_Device = { nullptr };
	unique_ptr<class CInput_Device>			m_pInput_Device = { nullptr };
#ifdef _WIN64
	unique_ptr<class CSound_Manager>		m_pSound_Manager = { nullptr };
#endif
	unique_ptr<class CLevel_Manager>		m_pLevel_Manager = { nullptr };
	unique_ptr<class CPrototype_Manager>	m_pPrototype_Manager = { nullptr };
	unique_ptr<class CObject_Manager>		m_pObject_Manager = { nullptr };
	unique_ptr<class CRenderer>				m_pRenderer = { nullptr };
	unique_ptr<class CPipeLine>				m_pPipeLine = { nullptr };
	unique_ptr<class CLight_Manager>		m_pLight_Manager = { nullptr };
	unique_ptr<class CFont_Manager>			m_pFont_Manager = { nullptr };
	unique_ptr<class CTarget_Manager>		m_pTarget_Manager = { nullptr };
	unique_ptr<class CPicking>				m_pPicking = { nullptr };
	unique_ptr<class CShadow>				m_pShadow = { nullptr };
	unique_ptr<class CFrustum>				m_pFrustum = { nullptr };
	unique_ptr<class CProfiler>				m_pProfiler = { nullptr };
	unique_ptr<class CPhysics_Manager>		m_pPhysics_Manager = { nullptr };

private:
	float2_t			m_vViewportDesc = {};

public:
	void Release_Engine();

};

NS_END
```

#### C:/Users/tnest/Desktop/LostArk/Engine/Private/GameInstance.cpp

Bind_ActiveSceneColorTransform은 Renderer에 위임하며 별도 상태를 소유하지 않는다.

SHA256: `f1f28f90ba21c91f25f5b88e6a68c91967e39162893e4b06f6bf313f0ff9dec6`

```cpp
#include "Input_Device.h" // Load DirectInput before Engine::POINT enters global lookup.
#include "GameInstance.h"
#include "Engine_InitTypes.h"
#include "Engine_RenderTypes.h"

#include "Shadow.h"
#include "Picking.h"
#include "Frustum.h"
#include "Renderer.h"
#include "PipeLine.h"
#include "Font_Manager.h"
#ifdef _WIN64
#include "Sound/Sound_Manager.h"
#endif
#include "Light_Manager.h"
#include "Timer_Manager.h"
#include "Level_Manager.h"
#include "Target_Manager.h"
#include "Object_Manager.h"
#include "Graphic_Device.h"
#include "Prototype_Manager.h"
#include "Profiler.h"
#include "Physics_Manager.h"

#include <atomic>

namespace
{
	std::atomic_bool g_isNonInteractiveErrorMode = false;
}

void Engine::Set_NonInteractiveErrorMode(const bool isEnabled)
{
	g_isNonInteractiveErrorMode.store(isEnabled, std::memory_order_release);
}

bool Engine::Is_NonInteractiveErrorMode()
{
	return g_isNonInteractiveErrorMode.load(std::memory_order_acquire);
}

int Engine::Show_EngineMessage(const wchar_t* pMessage)
{
	const wchar_t* pResolvedMessage = nullptr != pMessage ? pMessage : L"Engine error";

	if (Is_NonInteractiveErrorMode())
	{
		OutputDebugStringW(pResolvedMessage);
		OutputDebugStringW(L"\n");
		return IDOK;
	}

	return MessageBoxW(nullptr, pResolvedMessage, L"System Message", MB_OK);
}

CGameInstance::CGameInstance()
{
}

CGameInstance::~CGameInstance()
{
}

HRESULT CGameInstance::Initialize_Engine(const ENGINE_DESC& EngineDesc,
	ComPtr<ID3D11Device>& pOutDevice, ComPtr<ID3D11DeviceContext>& pOutContext)
{
	Set_NonInteractiveErrorMode(EngineDesc.bNonInteractiveErrors);
	pOutDevice.Reset();
	pOutContext.Reset();
	Release_Engine();
	const auto FailInitialization = [this, &pOutDevice, &pOutContext](
		const HRESULT hResult) -> HRESULT
	{
		Release_Engine();
		pOutDevice.Reset();
		pOutContext.Reset();
		return FAILED(hResult) ? hResult : E_FAIL;
	};

	//viewport
	m_vViewportDesc = float2_t(EngineDesc.iWinSizeX, EngineDesc.iWinSizeY);

	m_pGraphic_Device = CGraphic_Device::Create(EngineDesc.hWnd, EngineDesc.eWinMode,
		EngineDesc.eDriverType, EngineDesc.iWinSizeX, EngineDesc.iWinSizeY,
		pOutDevice, pOutContext);
	if (nullptr == m_pGraphic_Device)
		return FailInitialization(E_FAIL);

	m_pInput_Device = CInput_Device::Create(EngineDesc.hInstance, EngineDesc.hWnd);
	if (nullptr == m_pInput_Device)
		return FailInitialization(E_FAIL);

#ifdef _WIN64
	m_pSound_Manager = CSound_Manager::Create();
	if (nullptr == m_pSound_Manager)
		return FailInitialization(E_FAIL);
#endif

	m_pTarget_Manager = CTarget_Manager::Create(pOutDevice, pOutContext);
	if (nullptr == m_pTarget_Manager)
		return FailInitialization(E_FAIL);

	m_pRenderer = CRenderer::Create(pOutDevice, pOutContext);
	if (nullptr == m_pRenderer)
		return FailInitialization(E_FAIL);

	m_pTimer_Manager = CTimer_Manager::Create();
	if (nullptr == m_pTimer_Manager)
		return FailInitialization(E_FAIL);

	m_pPrototype_Manager = CPrototype_Manager::Create(EngineDesc.iNumLevels);
	if (nullptr == m_pPrototype_Manager)
		return FailInitialization(E_FAIL);

	m_pObject_Manager = CObject_Manager::Create(EngineDesc.iNumLevels);
	if (nullptr == m_pObject_Manager)
		return FailInitialization(E_FAIL);

	m_pLevel_Manager = CLevel_Manager::Create();
	if (nullptr == m_pLevel_Manager)
		return FailInitialization(E_FAIL);

	m_pPipeLine = CPipeLine::Create();
	if (nullptr == m_pPipeLine)
		return FailInitialization(E_FAIL);

	m_pLight_Manager = CLight_Manager::Create();
	if (nullptr == m_pLight_Manager)
		return FailInitialization(E_FAIL);

	m_pFont_Manager = CFont_Manager::Create(pOutDevice, pOutContext);
	if (nullptr == m_pFont_Manager)
		return FailInitialization(E_FAIL);

	m_pPicking = CPicking::Create(pOutDevice, pOutContext, EngineDesc.hWnd);
	if (nullptr == m_pPicking)
		return FailInitialization(E_FAIL);

	m_pShadow = CShadow::Create();
	if (nullptr == m_pShadow)
		return FailInitialization(E_FAIL);

	m_pFrustum = CFrustum::Create();
	if (nullptr == m_pFrustum)
		return FailInitialization(E_FAIL);

	m_pPhysics_Manager = CPhysics_Manager::Create(EngineDesc.iNumLevels);
	if (nullptr == m_pPhysics_Manager)
		return FailInitialization(E_FAIL);

	m_pProfiler = std::make_unique<CProfiler>();
	const HRESULT hProfilerResult =
		m_pProfiler->Initialize(pOutDevice, pOutContext);
	if (FAILED(hProfilerResult))
		return FailInitialization(hProfilerResult);

	return S_OK;
}

HRESULT CGameInstance::Resize_Viewport(uint32_t width, uint32_t height)
{
    if (!m_pGraphic_Device || !m_pTarget_Manager || !m_pRenderer || width == 0u || height == 0u ||
        width > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION || height > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION)
        return E_INVALIDARG;
    if (m_vViewportDesc.x == static_cast<f32_t>(width) && m_vViewportDesc.y == static_cast<f32_t>(height))
    {
        m_pTarget_Manager->Release_OutputReferences();
        return m_pGraphic_Device->Resize_BackBuffer(width, height);
    }
    try
    {
        CRenderer::VIEWPORT_RESIZE_STATE staged;
        HRESULT result = m_pRenderer->Stage_ViewportResize(width, height, *m_pTarget_Manager, staged);
        if (FAILED(result)) return result;
        m_pTarget_Manager->Release_OutputReferences();
        result = m_pGraphic_Device->Resize_BackBuffer(width, height);
        if (FAILED(result)) return result;
        // All allocations and fallible GPU work have completed. Publish one coherent size.
        m_pRenderer->Commit_ViewportResize(*m_pTarget_Manager, staged);
        m_vViewportDesc = float2_t(static_cast<f32_t>(width), static_cast<f32_t>(height));
        return S_OK;
    }
    catch (const std::bad_alloc&)
    {
        return E_OUTOFMEMORY;
    }
}

HRESULT CGameInstance::Set_FullscreenMode(bool fullscreen, uint32_t width, uint32_t height)
{
    return m_pGraphic_Device ? m_pGraphic_Device->Set_FullscreenMode(fullscreen, width, height) : E_FAIL;
}

void CGameInstance::Update_Engine(f32_t fTimeDelta)
{
	/* The deferred fog drifts on a renderer owned clock, advanced once per
	   frame so no screen pass has to be handed a time value. */
	m_pRenderer->Advance_PresentationClock(fTimeDelta);

	CProfiler* const pProfiler = m_pProfiler.get();
	{
		CProfilerScope scope(pProfiler, "Engine.Input.Update");
		m_pInput_Device->Update();
	}
#ifdef _WIN64
	{
		CProfilerScope scope(pProfiler, "Engine.Sound.Update");
		m_pSound_Manager->Update();
	}
#endif
	{
		CProfilerScope scope(pProfiler, "Engine.PriorityUpdate");
		m_pObject_Manager->Priority_Update(fTimeDelta);
	}

	{
		CProfilerScope scope(pProfiler, "Engine.Camera.Update");
		Refresh_CameraState();
	}

	{
		CProfilerScope scope(pProfiler, "Engine.ObjectUpdate");
		m_pObject_Manager->Update(fTimeDelta);
	}
	{
		CProfilerScope scope(pProfiler, "Engine.Physics");
		m_pPhysics_Manager->Update(fTimeDelta);
	}
	{
		CProfilerScope scope(pProfiler, "Engine.PostPhysicsUpdate");
		m_pObject_Manager->Post_Physics_Update(fTimeDelta);
	}

	/* Level gameplay can commit authoritative transforms and bind the
	follow camera. Run it before render submission and frustum culling. */
	{
		CProfilerScope scope(pProfiler, "Engine.LevelUpdate");
		m_pLevel_Manager->Update(fTimeDelta);
	}
	{
		CProfilerScope scope(pProfiler, "Engine.LateUpdate");
		m_pObject_Manager->Late_Update(fTimeDelta);
	}
}

void CGameInstance::Submit_FinalCameraObjects()
{
	m_pObject_Manager->Submit_FinalCamera();
}

void CGameInstance::Refresh_CameraState()
{
	m_pPipeLine->Update();
	m_pFrustum->Update_InWorldSpace();
}

HRESULT CGameInstance::Render_Begin(const float4_t* pClearColor)
{
	if (FAILED(m_pGraphic_Device->Clear_BackBuffer_View(pClearColor)))
		return E_FAIL;

	if (FAILED(m_pGraphic_Device->Clear_DepthStencil_View()))
		return E_FAIL;

	return S_OK;
}

HRESULT CGameInstance::Render(const bool staticOcclusionAllowed)
{
	const HRESULT hDrawResult = m_pRenderer->Draw(staticOcclusionAllowed);
	if (FAILED(hDrawResult))
		return hDrawResult;

	const HRESULT hLevelResult = m_pLevel_Manager->Render();
	if (FAILED(hLevelResult))
		return hLevelResult;

	return S_OK;
}

HRESULT CGameInstance::Render_End()
{
	return m_pGraphic_Device->Present();
}

HRESULT CGameInstance::Clear_Resources(uint32_t iClearLevelID)
{
	if (FAILED(m_pObject_Manager->Clear(iClearLevelID)))
		return E_FAIL;

	if (FAILED(m_pPrototype_Manager->Clear(iClearLevelID)))
		return E_FAIL;

	if (FAILED(m_pPhysics_Manager->Clear(iClearLevelID)))
		return E_FAIL;

	return S_OK;

}

int8_t CGameInstance::Get_DIKeyState(uint8_t byKeyID)
{
	return m_pInput_Device->Get_DIKeyState(byKeyID);
}

bool_t CGameInstance::Get_DIKeyPressed(uint8_t byKeyID)
{
	return m_pInput_Device->Get_DIKeyPressed(byKeyID);
}

int8_t CGameInstance::Get_DIKeyStateRaw(uint8_t byKeyID) const
{
	return nullptr != m_pInput_Device ?
		m_pInput_Device->Get_DIKeyStateRaw(byKeyID) : 0;
}

bool_t CGameInstance::Get_DIKeyPressedRaw(uint8_t byKeyID) const
{
	return nullptr != m_pInput_Device &&
		m_pInput_Device->Get_DIKeyPressedRaw(byKeyID);
}

int8_t CGameInstance::Get_DIMouseState(DIM eMouse)
{
	return m_pInput_Device->Get_DIMouseState(eMouse);
}

int8_t CGameInstance::Get_DIMouseStateRaw(DIM eMouse) const
{
	return nullptr != m_pInput_Device ?
		m_pInput_Device->Get_DIMouseStateRaw(eMouse) : 0;
}

int32_t CGameInstance::Get_DIMouseMove(DIMM eMouseState)
{
	return m_pInput_Device->Get_DIMouseMove(eMouseState);
}

void CGameInstance::SetInputBlocked(bool_t bKeyboardBlocked, bool_t bMouseBlocked)
{
	if (nullptr != m_pInput_Device)
		m_pInput_Device->SetInputBlocked(bKeyboardBlocked, bMouseBlocked);
}

void CGameInstance::SetMouseButtonBlocked(DIM eMouse, bool_t blocked)
{
	if (nullptr != m_pInput_Device)
		m_pInput_Device->SetMouseButtonBlocked(eMouse, blocked);
}

bool_t CGameInstance::IsKeyboardInputBlocked() const
{
	return nullptr != m_pInput_Device && m_pInput_Device->IsKeyboardInputBlocked();
}

bool_t CGameInstance::IsMouseInputBlocked() const
{
	return nullptr != m_pInput_Device && m_pInput_Device->IsMouseInputBlocked();
}

#ifdef _WIN64
uint64_t CGameInstance::Play_SoundCue(const wstring_t& path, f32_t volume, uint32_t ageMs, bool_t paused, f32_t playbackRate, uint32_t endMs)
{ return m_pSound_Manager ? m_pSound_Manager->Play_SoundCue(path, volume, ageMs, paused, playbackRate, endMs) : 0u; }
bool_t CGameInstance::Get_SoundDurationMs(const wstring_t& path, uint32_t& durationMs)
{ return m_pSound_Manager && m_pSound_Manager->Get_SoundDurationMs(path, durationMs); }
bool_t CGameInstance::Is_SoundCueActive(uint64_t handle) const
{ return m_pSound_Manager && m_pSound_Manager->Is_SoundCueActive(handle); }
void CGameInstance::Pause_SoundCue(uint64_t handle, bool_t paused)
{ if (m_pSound_Manager) m_pSound_Manager->Pause_SoundCue(handle, paused); }
void CGameInstance::Set_SoundCuePlaybackRate(uint64_t handle, f32_t rate)
{ if (m_pSound_Manager) m_pSound_Manager->Set_SoundCuePlaybackRate(handle, rate); }
void CGameInstance::Seek_SoundCue(uint64_t handle, uint32_t ageMs)
{ if (m_pSound_Manager) m_pSound_Manager->Seek_SoundCue(handle, ageMs); }
bool_t CGameInstance::Synchronize_SoundCue(uint64_t handle, uint32_t ageMs, uint32_t maxDriftMs)
{ return m_pSound_Manager && m_pSound_Manager->Synchronize_SoundCue(handle, ageMs, maxDriftMs); }
void CGameInstance::Stop_SoundCue(uint64_t handle)
{ if (m_pSound_Manager) m_pSound_Manager->Stop_SoundCue(handle); }

HRESULT CGameInstance::Play_Sound(const wstring_t& strSoundFilePath, f32_t fVolume)
{
	return m_pSound_Manager->Play_Sound(strSoundFilePath, fVolume);
}

HRESULT CGameInstance::Play_LoopingSound(const wstring_t& strSoundFilePath, f32_t fVolume)
{
	return nullptr != m_pSound_Manager ?
		m_pSound_Manager->Play_LoopingSound(strSoundFilePath, fVolume) : E_FAIL;
}

void CGameInstance::Stop_LoopingSound()
{
	if (nullptr != m_pSound_Manager)
		m_pSound_Manager->Stop_LoopingSound();
}

HRESULT CGameInstance::Play_Music(const wstring_t& strSoundFilePath,
	f32_t fVolume, const bool_t bLoop)
{
	return m_pSound_Manager->Play_Music(strSoundFilePath, fVolume, bLoop);
}

void CGameInstance::Stop_Music()
{
	m_pSound_Manager->Stop_Music();
}

HRESULT CGameInstance::Apply_SoundCategoryVolume(
	const SOUND_CATEGORY eCategory, const f32_t fVolume)
{
	return m_pSound_Manager->Apply_CategoryVolume(eCategory, fVolume);
}

f32_t CGameInstance::Get_SoundCategoryVolume(const SOUND_CATEGORY eCategory) const
{
	return m_pSound_Manager->Get_CategoryVolume(eCategory);
}

void CGameInstance::Set_SoundMuteOnFocusLoss(const bool_t bMute)
{
	m_pSound_Manager->Set_MuteOnFocusLoss(bMute);
}
#endif

f32_t CGameInstance::Get_TimeDelta(const wstring_t& strTimerTag)
{
	return m_pTimer_Manager->Get_TimeDelta(strTimerTag);
}

HRESULT CGameInstance::Add_Timer(const wstring_t& strTimerTag)
{
	return m_pTimer_Manager->Add_Timer(strTimerTag);
}

void CGameInstance::Update_TimeDelta(const wstring_t& strTimerTag)
{
	m_pTimer_Manager->Update_TimeDelta(strTimerTag);
}

HRESULT CGameInstance::Change_Level(uint32_t iCurrentLevelID, unique_ptr<class CLevel> pNewLevel)
{
	return m_pLevel_Manager->Change_Level(iCurrentLevelID, move(pNewLevel));
}

uint32_t CGameInstance::Get_CurrentLevelID() const
{
	return nullptr == m_pLevel_Manager ? 0u : m_pLevel_Manager->Get_CurrentLevelID();
}

HRESULT CGameInstance::Add_Prototype(uint32_t iLevelIndex, const wstring_t& strPrototypeTag, unique_ptr<class CPrototype> pPrototype)
{
	return m_pPrototype_Manager->Add_Prototype(iLevelIndex, strPrototypeTag, move(pPrototype));
}

HRESULT CGameInstance::Add_Prototypes(
	const uint32_t iLevelIndex,
	vector<pair<wstring_t, unique_ptr<CPrototype>>>&& prototypes)
{
	return m_pPrototype_Manager->Add_Prototypes(
		iLevelIndex,
		std::move(prototypes));
}

shared_ptr<CPrototype> CGameInstance::Clone_Prototype(uint32_t iLevelIndex, const wstring_t& strPrototypeTag, void* pArg)
{
	return m_pPrototype_Manager->Clone_Prototype(iLevelIndex, strPrototypeTag, pArg);
}

shared_ptr<CGameObject> CGameInstance::Get_GameObject(
	uint32_t iLevelIndex,
	const wstring_t& strLayerTag,
	uint32_t iIndex)
{
	return m_pObject_Manager->Get_GameObject(
		iLevelIndex,
		strLayerTag,
		iIndex);
}

shared_ptr<CComponent> CGameInstance::Get_Component(uint32_t iLevelIndex, const wstring_t& strLayerTag, const wstring_t& strComponentTag, uint32_t iIndex)
{
	return m_pObject_Manager->Get_Component(iLevelIndex, strLayerTag, strComponentTag, iIndex);
}

shared_ptr<CComponent> CGameInstance::Get_Component(uint32_t iLevelIndex, const wstring_t& strLayerTag, const wstring_t& strPartTag, const wstring_t& strComponentTag, uint32_t iIndex)
{
	return m_pObject_Manager->Get_Component(iLevelIndex, strLayerTag, strPartTag, strComponentTag, iIndex);
}

HRESULT CGameInstance::Add_GameObject_to_Layer(uint32_t iPrototypeLevelIndex, const wstring_t& strPrototypeTag, uint32_t iLayerLevelIndex, const wstring_t& strLayerTag, void* pArg, shared_ptr<CGameObject>* pOutGameObject)
{
	return m_pObject_Manager->Add_GameObject_to_Layer(iPrototypeLevelIndex, strPrototypeTag, iLayerLevelIndex, strLayerTag, pArg, pOutGameObject);
}

HRESULT CGameInstance::Remove_GameObject_from_Layer(uint32_t iLevelIndex, const wstring_t& strLayerTag, const shared_ptr<CGameObject>& pGameObject)
{
	return m_pObject_Manager->Remove_GameObject_from_Layer(iLevelIndex, strLayerTag, pGameObject);
}

HRESULT CGameInstance::Add_RenderObject(RENDERGROUP eRenderGroupID, shared_ptr<CGameObject> pRenderObject)
{
	return m_pRenderer->Add_RenderObject(eRenderGroupID, std::move(pRenderObject));
}

HRESULT CGameInstance::Request_Portrait(ComPtr<ID3D11RenderTargetView> pDestination,
	uint32_t iWidth, uint32_t iHeight,
	const float4x4_t& ViewMatrix, const float4x4_t& ProjMatrix,
	function<HRESULT()> DrawSubject,
	function<HRESULT(RENDERGROUP, ID3D11BlendState*)> DrawForward)
{
	if (nullptr == m_pRenderer)
		return E_FAIL;

	CRenderer::PORTRAIT_REQUEST		Request{};
	Request.pDestination = std::move(pDestination);
	Request.iWidth = iWidth;
	Request.iHeight = iHeight;
	Request.ViewMatrix = ViewMatrix;
	Request.ProjMatrix = ProjMatrix;
	Request.DrawSubject = std::move(DrawSubject);
	Request.DrawForward = std::move(DrawForward);

	return m_pRenderer->Request_Portrait(std::move(Request));
}

void CGameInstance::Request_SceneColorSnapshot()
{
	if (m_pRenderer)
		m_pRenderer->Request_SceneColorSnapshot();
}

HRESULT CGameInstance::Refresh_SceneColorSnapshot()
{
	return m_pRenderer ? m_pRenderer->Refresh_SceneColorSnapshot() : E_FAIL;
}

void CGameInstance::Request_SceneEnvironmentReplacement()
{
    if (m_pRenderer) m_pRenderer->Request_SceneEnvironmentReplacement();
}

bool_t CGameInstance::Is_SceneEnvironmentReplaced() const
{
    return m_pRenderer && m_pRenderer->Is_SceneEnvironmentReplaced();
}

void CGameInstance::Register_GpuSceneVisibility(const shared_ptr<CGpuSceneVisibility>& scene)
{
    if (m_pRenderer) m_pRenderer->Register_GpuSceneVisibility(scene);
}

void CGameInstance::Unregister_GpuSceneVisibility(const CGpuSceneVisibility* scene)
{
    if (m_pRenderer) m_pRenderer->Unregister_GpuSceneVisibility(scene);
}

MAP_VISIBILITY_SETTINGS CGameInstance::Get_MapVisibilitySettings() const
{
    return m_pRenderer ? m_pRenderer->Get_MapVisibilitySettings() : MAP_VISIBILITY_SETTINGS{};
}
HRESULT CGameInstance::Apply_MapVisibilitySettings(const MAP_VISIBILITY_SETTINGS& settings)
{
    return m_pRenderer ? m_pRenderer->Apply_MapVisibilitySettings(settings) : E_FAIL;
}

RENDER_OPTIMIZATION_SETTINGS CGameInstance::Get_RenderOptimizationSettings() const
{
    return m_pRenderer ? m_pRenderer->Get_RenderOptimizationSettings() : RENDER_OPTIMIZATION_SETTINGS{};
}
HRESULT CGameInstance::Apply_RenderOptimizationSettings(const RENDER_OPTIMIZATION_SETTINGS& settings)
{
    return m_pRenderer ? m_pRenderer->Apply_RenderOptimizationSettings(settings) : E_FAIL;
}

RENDER_QUALITY_SETTINGS CGameInstance::Get_RenderQualitySettings() const
{
	return m_pRenderer->Get_RenderQualitySettings();
}

uint32_t CGameInstance::Get_TextureMinMip() const
{
	return m_pRenderer ? m_pRenderer->Get_RenderQualitySettings().iTextureMinMip : 0u;
}

HRESULT CGameInstance::Apply_RenderQualitySettings(
	const RENDER_QUALITY_SETTINGS& Settings)
{
	return m_pRenderer->Apply_RenderQualitySettings(Settings);
}

MATERIAL_RENDER_SETTINGS CGameInstance::Get_MaterialRenderSettings() const
{
	return m_pRenderer->Get_MaterialRenderSettings();
}

HRESULT CGameInstance::Apply_MaterialRenderSettings(const MATERIAL_RENDER_SETTINGS& settings)
{
	return m_pRenderer->Apply_MaterialRenderSettings(settings);
}

HEIGHT_FOG_SETTINGS CGameInstance::Get_HeightFogSettings() const
{
	return m_pRenderer->Get_HeightFogSettings();
}

HRESULT CGameInstance::Stage_RenderEnvironment(const wstring_t& cubePath,
    const float4_t& color, const float4_t& rotationIntensity,
    RENDER_ENVIRONMENT_STATE& outState, bool_t forceReload) const
{
    return m_pRenderer ? m_pRenderer->Stage_RenderEnvironment(
        cubePath, color, rotationIntensity, outState, forceReload) : E_FAIL;
}

void CGameInstance::Commit_RenderEnvironment(const RENDER_ENVIRONMENT_STATE& state)
{
    if (m_pRenderer) m_pRenderer->Commit_RenderEnvironment(state);
}

RENDER_ENVIRONMENT_STATE CGameInstance::Get_RenderEnvironment() const
{
    return m_pRenderer ? m_pRenderer->Get_RenderEnvironment() : RENDER_ENVIRONMENT_STATE{};
}

HRESULT CGameInstance::Bind_HeightFog(CShader* shader) const
{
    return m_pRenderer->Bind_HeightFog(shader);
}

HRESULT CGameInstance::Bind_ActiveSceneColorTransform(CShader* shader) const
{
    return m_pRenderer ? m_pRenderer->Bind_ActiveSceneColorTransform(shader) : E_FAIL;
}

HRESULT CGameInstance::Apply_HeightFog(
	const HEIGHT_FOG_SETTINGS& Settings)
{
	return m_pRenderer->Apply_HeightFog(Settings);
}

HRESULT CGameInstance::Add_DebugComponent(shared_ptr<CComponent> pDebugComponent)
{
	return m_pRenderer->Add_DebugComponent(pDebugComponent);
}

void CGameInstance::Set_Transform(D3DTS eType, fmatrix_t TransformMatrix)
{
	m_pPipeLine->Set_Transform(eType, TransformMatrix);
}

const float4_t* CGameInstance::Get_CamPosition()
{
	return m_pPipeLine->Get_CamPosition();
}

const float4x4_t* CGameInstance::Get_Transform(D3DTS eType)
{
	return 	m_pPipeLine->Get_Transform(eType);
}

const float4x4_t* CGameInstance::Get_InverseTransform(D3DTS eType)
{
	return 	m_pPipeLine->Get_InverseTransform(eType);
}

HRESULT CGameInstance::Bind_CamPosition(shared_ptr<class CShader> pShader, const char_t* pConstantName)
{
	return 	m_pPipeLine->Bind_CamPos_ShaderResource(pShader, pConstantName);
}

HRESULT CGameInstance::Bind_Transform(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType)
{
	return 	m_pPipeLine->Bind_ShaderResource(pShader, pConstantName, eType);
}

HRESULT CGameInstance::Bind_InverseTransform(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType)
{
	return 	m_pPipeLine->Bind_Inverse_ShaderResource(pShader, pConstantName, eType);
}

HRESULT CGameInstance::Add_Light(const LIGHT_DESC& LightDesc)
{
	return m_pLight_Manager->Add_Light(LightDesc);
}

const vector<LIGHT_DESC>& CGameInstance::Get_SceneLights() const
{
	return m_pLight_Manager->Get_SceneLights();
}

HRESULT CGameInstance::Render_Lights(
	shared_ptr<class CShader> pShader,
	shared_ptr<class CVIBuffer_Rect> pVIBuffer,
	bool_t bEnableSceneDirectionalShadow, LIGHT_RECEIVER ePassReceiver, bool_t bSourceLightMask)
{
	return m_pLight_Manager->Render_Lights(
		pShader, pVIBuffer, bEnableSceneDirectionalShadow, ePassReceiver, bSourceLightMask);
}

HRESULT CGameInstance::Add_Font(const wstring& strFontTag, const tchar_t* pFontFilePath)
{
	return m_pFont_Manager->Add_Font(strFontTag, pFontFilePath);
}

void CGameInstance::Draw_Text(const wstring& strFontTag, const tchar_t* pText, const float2_t& vPosition, fvector_t vColor, f32_t fRotation, const float2_t& vOrigin, f32_t fScale)
{
	return m_pFont_Manager->Draw(strFontTag, pText, vPosition, vColor, fRotation, vOrigin, fScale);
}

float2_t CGameInstance::Measure_Text(const wstring& strFontTag, const tchar_t* pText)
{
	return m_pFont_Manager->Measure(strFontTag, pText);
}

void CGameInstance::Set_TextClipOutRect(f32_t fX, f32_t fY, f32_t fWidth, f32_t fHeight)
{
	m_pFont_Manager->Set_ClipOutRect(fX, fY, fWidth, fHeight);
}

void CGameInstance::Add_TextClipOutRect(
	const f32_t fX, const f32_t fY, const f32_t fWidth, const f32_t fHeight)
{
	m_pFont_Manager->Add_ClipOutRect(fX, fY, fWidth, fHeight);
}

void CGameInstance::Clear_TextClipOutRect()
{
	m_pFont_Manager->Clear_ClipOutRect();
}

void CGameInstance::Set_TextClipInRect(
	const f32_t fX, const f32_t fY, const f32_t fWidth, const f32_t fHeight)
{
	m_pFont_Manager->Set_ClipInRect(fX, fY, fWidth, fHeight);
}

void CGameInstance::Clear_TextClipInRect()
{
	m_pFont_Manager->Clear_ClipInRect();
}

HRESULT CGameInstance::Add_RenderTarget(const wstring_t& strTargetTag, uint32_t iWidth, uint32_t iHeight, DXGI_FORMAT ePixelFormat, const float4_t& vClearColor)
{
	return m_pTarget_Manager->Add_RenderTarget(strTargetTag, iWidth, iHeight, ePixelFormat, vClearColor);
}

HRESULT CGameInstance::Add_MRT(const wstring_t& strMRTTag, const wstring_t& strTargetTag)
{
	return m_pTarget_Manager->Add_MRT(strMRTTag, strTargetTag);
}

HRESULT CGameInstance::Begin_MRT(const wstring_t& strMRTTag, ComPtr<ID3D11DepthStencilView> pDSV)
{
	return m_pTarget_Manager->Begin_MRT(strMRTTag, pDSV);
}

HRESULT CGameInstance::End_MRT()
{
	return m_pTarget_Manager->End_MRT();
}

HRESULT CGameInstance::Begin_DepthOnly(
	ComPtr<ID3D11DepthStencilView> pDSV)
{
	return m_pTarget_Manager->Begin_DepthOnly(pDSV);
}

HRESULT CGameInstance::End_DepthOnly()
{
	return m_pTarget_Manager->End_DepthOnly();
}

HRESULT CGameInstance::Bind_RT_SRV(const wstring_t& strTargetTag, shared_ptr<class CShader> pShader, const char_t* pConstantName)
{
	return m_pTarget_Manager->Bind_SRV(strTargetTag, pShader, pConstantName);
}

ComPtr<ID3D11ShaderResourceView> CGameInstance::Get_RT_SRV(
	const wstring_t& strTargetTag) const
{
	return m_pTarget_Manager->Get_SRV(strTargetTag);
}

HRESULT CGameInstance::Copy_RT_Resource(const wstring_t& strTargetTag, ComPtr<ID3D11Texture2D> pTexture2D)
{
	return m_pTarget_Manager->Copy_Resource(strTargetTag, pTexture2D);
}

#ifdef _DEBUG

HRESULT CGameInstance::Ready_RT_DebugDesc(const wstring_t& strTargetTag, f32_t fX, f32_t fY, f32_t fSizeX, f32_t fSizeY)
{
	return m_pTarget_Manager->Ready_DebugDesc(strTargetTag, fX, fY, fSizeX, fSizeY);
}

HRESULT CGameInstance::Render_MRT(const wstring_t& strMRTTag, shared_ptr<class CShader> pShader, shared_ptr<class CVIBuffer_Rect> pVIBuffer)
{
	return m_pTarget_Manager->Render_MRT(strMRTTag, pShader, pVIBuffer);
}

#endif

bool_t CGameInstance::Picking(float4_t& vOut)
{
	return m_pPicking->Picking(vOut);
}

HRESULT CGameInstance::Apply_Shadow_Light(
	const SHADOW_LIGHT_DESC& ShadowLightDesc)
{
	return m_pShadow->Apply_Shadow_Light(ShadowLightDesc);
}

HRESULT CGameInstance::Add_Shadow_Light(
	const SHADOW_LIGHT_DESC& ShadowLightDesc)
{
	return Apply_Shadow_Light(ShadowLightDesc);
}

bool_t CGameInstance::Is_ShadowLightEnabled() const
{
	return m_pShadow->Is_Enabled();
}

const SHADOW_LIGHT_DESC& CGameInstance::Get_ShadowLightDesc() const
{
	return m_pShadow->Get_Desc();
}

const float4x4_t* CGameInstance::Get_ShadowLightTransform(D3DTS eType) const
{
	return m_pShadow ? m_pShadow->Get_Transform(eType) : nullptr;
}

HRESULT CGameInstance::Bind_ShadowLight_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName, D3DTS eType)
{
	return m_pShadow->Bind_ShaderResource(pShader, pConstantName, eType);
}

HRESULT CGameInstance::Bind_ShadowLight_LightingResources(
	shared_ptr<class CShader> pShader)
{
	return m_pShadow->Bind_LightingShaderResources(pShader);
}

void CGameInstance::Update_Frustum_InLocalSpace(fmatrix_t WorldMatrix)
{
	m_pFrustum->Update_InLocalSpace(WorldMatrix);
}

bool_t CGameInstance::isIn_Frustum_InLocalSpace(fvector_t vLocalPoint, f32_t fRange)
{
	return m_pFrustum->isIn_Frustum_InLocalSpace(vLocalPoint, fRange);
}

bool_t CGameInstance::isIn_Frustum_InWorldSpace(fvector_t vWorldPoint, f32_t fRange)
{
	return m_pFrustum->isIn_Frustum_InWorldSpace(vWorldPoint, fRange);
}

void CGameInstance::Release_Engine()
{
	/* A level can restore a presentation camera from its destructor. Keep the
	   pipeline, frustum and renderer alive until that teardown has completed;
	   otherwise CCamera::End_PresentationOverride writes its final view/proj
	   matrices through an already-destroyed CPipeLine. This also joins a
	   Loading level's worker before any service it may still reference. */
	m_pLevel_Manager.reset();
	m_pProfiler.reset();
	m_pFrustum.reset();
	m_pShadow.reset();
	m_pPicking.reset();
	m_pTarget_Manager.reset();
	m_pFont_Manager.reset();
	m_pLight_Manager.reset();
	m_pPipeLine.reset();
	m_pRenderer.reset();
	m_pObject_Manager.reset();
	m_pPhysics_Manager.reset();
	m_pPrototype_Manager.reset();
	m_pTimer_Manager.reset();
#ifdef _WIN64
	m_pSound_Manager.reset();
#endif
	m_pInput_Device.reset();
	if (nullptr != m_pGraphic_Device)
		m_pGraphic_Device->Shutdown();
	m_pGraphic_Device.reset();
}

uint64_t CGameInstance::Request_Picking()
{
    return m_pPicking ? m_pPicking->Request_Picking() : 0u;
}
HRESULT CGameInstance::Poll_Picking(const uint64_t requestId, float4_t& vOut)
{
    return m_pPicking ? m_pPicking->Poll_Picking(requestId, vOut) : E_ABORT;
}
void CGameInstance::Cancel_Picking(const uint64_t requestId)
{
    if (m_pPicking) m_pPicking->Cancel_Picking(requestId);
}
```

#### 프로젝트 등록

Client.vcxproj에서 기존 Shader_EffectCubeSampleScene.hlsli 항목 바로 아래:

```xml
    <None Include="..\Bin\ShaderFiles\Shader_EffectCubeColorResponse.hlsli" />
```

Client.vcxproj.filters에서 같은 기존 항목 다음:

```xml
    <None Include="..\Bin\ShaderFiles\Shader_EffectCubeColorResponse.hlsli">
      <Filter>97.ShaderFiles\02.Effects\01.CarriersAndCommon</Filter>
    </None>
```
