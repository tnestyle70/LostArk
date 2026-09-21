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
