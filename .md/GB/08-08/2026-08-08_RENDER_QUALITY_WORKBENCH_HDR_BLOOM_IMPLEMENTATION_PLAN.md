# 2026-08-08 Rendering Quality Workbench / HDR·Bloom 구현 계획서

## 1. 결론

이번 작업은 상용 엔진 기능 이름을 체크박스로 나열하는 작업이 아니다. 현재 제품 런타임에
실제로 존재하는 하나의 공용 렌더 경로를 먼저 계측·조절 가능하게 만든다.

```text
Character Select / Bern / Valtan / Effect
→ 공용 CRenderer
→ Deferred G-Buffer
→ Light Accumulation
→ FP16 Scene HDR + Effect Distortion
→ typed Screen Post
→ half-resolution Bloom
→ Hable Tone Mapping + FXAA(optional)
→ display-space UI / ImGui
```

1차 구현 완료 범위는 다음과 같다.

- Light accumulation의 HDR 값이 SceneHDR 전에 잘리지 않도록 Shade/Specular RT를 FP16으로 교정한다.
- 기존 하드코딩 Bloom/Hable 값을 검증된 `RENDER_QUALITY_SETTINGS`로 승격한다.
- Bloom On/Off, Threshold, Soft Knee, Intensity, Scatter를 실시간 조절한다.
- Exposure, White Point, Gamma를 실시간 조절한다.
- Final pass에서 실제 화면 edge를 평가하는 FXAA를 구현하고 기본값은 OFF로 둔다.
- 새 기능키 없이 F1 `LostArk Developer Tools` 안에 별도 `Rendering Workbench`를 제공한다.
- 기존 값으로 Reset하면 이번 변경 전 화면과 같은 수치 계약으로 돌아간다.

PBR, IBL, Forward+, DoF, SSAO, SSR, TAA는 이번 구현 완료 범위가 아니다. 각 기능은
실제 G-Buffer/RT/셰이더 소비 경로와 GPU A/B가 생길 때만 완료라고 부른다.

## 2. 현재 코드 실측

### 2.1 이미 존재하는 기능

| 단계 | 현재 구현 |
|---|---|
| Geometry | Diffuse / Normal / Depth / PickPos / Emissive MRT |
| Lighting | fullscreen Directional / Point, legacy diffuse + specular |
| HDR | `Target_SceneHDR`, `Target_Emissive`, `Target_Distortion` FP16 |
| Screen Post | RGB Noise / Zoom Blur / Film Noise typed pass |
| Bloom | 반해상도 R11G11B10F Extract / Ping / Result, 9-tap H/V blur |
| Tone Mapping | Hable, exposure multiplier, white point, gamma |
| UI | tone mapping 이후 display space에서 렌더 |

현재 셰이더 고정값은 다음과 같다.

```text
Bloom Threshold   1.0
Bloom Soft Knee   0.5
Bloom Intensity   0.8
Bloom Scatter     1.0 (현재 texel step)
Exposure          2.0
White Point      11.2
Gamma             2.2
FXAA               OFF / 구현 없음
```

### 2.2 이번에 확인한 결함

- `Target_Shade`가 `R8G8B8A8_UNORM`, `Target_Specular`가
  `R16G16B16A16_UNORM`이므로 1보다 큰 Light 결과가 SceneHDR 전에 잘린다.
- Bloom과 Tone Mapping 값이 HLSL 상수라 Character Select, Valtan, Effect에서 같은 조건의
  수치 A/B를 할 수 없다.
- FXAA 경로가 없다. 이름만 UI에 추가하지 않고 실제 Final pixel shader가 주변 LDR 결과를
  평가하도록 구현해야 한다.
- PBR용 metallic/roughness/AO는 일부 자산 reader에 있어도 G-Buffer와 lighting shader가
  소비하지 않는다.
- IBL irradiance/prefiltered cubemap/BRDF LUT, DoF CoC/near/far blur, Forward+ tile light list는
  현재 제품 경로에 없다.

## 3. 레퍼런스 판정

원작 Character Select와 현재 화면의 가장 큰 차이는 Bloom 양이 아니다.

1. 환경광과 재질별 반사 응답
2. 노출과 Tone Mapping
3. 제한적으로 퍼지는 Emissive Bloom
4. 필요할 때만 사용하는 DoF

Valtan의 창·청색 결정·스킬 코어처럼 실제 HDR emissive인 픽셀은 Bloom 대상이다. 석재,
난간, 캐릭터의 일반 조명까지 Bloom으로 처리하면 표면 명암이나 간접광은 생기지 않고 밝은
면만 번져 검정과 문양이 무너진다.

첫 A/B 안전 시작 범위는 다음과 같다. 이 값은 원작 확정값이 아니라 고정 카메라 비교를 위한
보수적 탐색 범위다.

| 항목 | A/B 시작 범위 |
|---|---:|
| Exposure | `1.00 ~ 1.35`, 우선 상한 `1.50` |
| Bloom Threshold | `1.20 ~ 1.80` |
| Bloom Soft Knee | `0.30 ~ 0.60` |
| Bloom Intensity | `0.12 ~ 0.30` |
| Bloom Scatter | `0.75 ~ 1.50` |
| White Point / Gamma | 우선 `11.2 / 2.2` 유지 |
| FXAA | OFF/ON 동일 장면 edge 비교 |
| DoF | 1차 A/B에서는 OFF |

## 4. G별 구현 계획

### G01. Typed render quality 계약

`Engine/Public/Engine_Struct.h`에 POD 설정을 추가한다.

```cpp
typedef struct tagRenderQualitySettings
{
    bool_t bBloomEnabled = true;
    f32_t fBloomThreshold = 1.f;
    f32_t fBloomSoftKnee = 0.5f;
    f32_t fBloomIntensity = 0.8f;
    f32_t fBloomScatter = 1.f;
    f32_t fExposure = 2.f;
    f32_t fWhitePoint = 11.2f;
    f32_t fGamma = 2.2f;
    bool_t bFXAAEnabled = false;
    f32_t fFXAASubpixel = 0.75f;
    f32_t fFXAAEdgeThreshold = 0.166f;
    f32_t fFXAAEdgeThresholdMin = 0.0833f;
} RENDER_QUALITY_SETTINGS;
```

Renderer는 finite/range를 전부 검사한 뒤 한 번에 교체한다. 하나라도 잘못되면 active 설정을
변경하지 않는다. UI가 `CRenderer` 포인터를 직접 소유하지 않도록 `CGameInstance`의 typed
get/apply 경계만 공개한다.

검증 범위:

```text
Threshold       0.0 .. 64.0
Soft Knee       0.0 .. 1.0
Intensity       0.0 .. 16.0
Scatter         0.25 .. 4.0
Exposure        0.01 .. 32.0
White Point     1.0 .. 64.0
Gamma           1.0 .. 3.0
FXAA Subpixel   0.0 .. 1.0
FXAA Edge       0.0312 .. 0.333
FXAA Edge Min   0.0156 .. 0.0833
```

### G02. HDR Light와 Bloom/Hable shader 연결

- Shade와 Specular accumulation target을 FP16 FLOAT로 바꾼다.
- Bloom Extract는 `Threshold/SoftKnee`, blur는 `Scatter` uniform을 소비한다.
- Bloom OFF일 때 3개 half-resolution pass를 건너뛰고 Final은 bloom intensity 0을 사용한다.
- Final은 `Intensity/Exposure/WhitePoint/Gamma`를 uniform으로 소비한다.
- 기존 hardcoded default와 새 default는 정확히 같게 둔다.

Character Select, Valtan, Effect는 모두 같은 `CRenderer::Draw`를 사용하므로 별도 Preview
renderer나 두 번째 post 경로를 만들지 않는다.

### G03. 실제 FXAA

FXAA OFF는 기존 Final 직접 출력 경로를 유지한다. FXAA ON일 때만 Final pixel shader에서
톤매핑·감마가 적용된 중앙/주변 sample의 luminance contrast와 edge 방향을 평가해 subpixel
blend를 수행한다.

- UI/ImGui는 Final 뒤에 렌더되므로 FXAA 대상이 아니다.
- 단순 blur checkbox가 아니라 edge threshold와 local contrast early-out이 있는 실제 pass다.
- 첫 구현은 같은 Final pass에서 LDR 결과를 재평가하여 별도 renderer나 중첩 MRT를 만들지 않는다.
- TAA/history/motion vector를 FXAA 완료로 주장하지 않는다.

### G04. F1 Rendering Workbench

`CMainApp`의 기존 F1 Developer Tools에 `Rendering Workbench`를 추가한다.

```text
Pipeline (read-only)
  legacy_deferred_v1
  SceneHDR FP16 / Bloom half-res / Hable

Bloom
  Enabled, Threshold, Soft Knee, Intensity, Scatter

Tone Mapping
  Exposure, White Point, Gamma

Anti-Aliasing
  FXAA Enabled, Subpixel, Edge Threshold, Edge Threshold Min

Actions
  Apply Live, Reset Legacy Defaults, Reference A/B Start
```

슬라이더는 draft만 편집하고 `Apply Live`가 전체 검증 후 active 설정을 교체한다. MainApp의
ImGui는 world draw 뒤에 실행되므로 적용은 자연스럽게 다음 프레임부터 보인다. 실패 시 기존
화면과 active 값은 유지하고 이유를 표시한다.

이번 1차는 process session 전역 설정이다. Authored JSON 저장/publish는 GPU A/B로 유효한
필드와 범위를 확정한 다음 G05로 분리한다. 임시 값을 제품 정본으로 자동 저장하지 않는다.

### G05. 후속 저장 계약

GPU A/B 뒤 필요한 경우 다음 구조로 별도 구현한다.

```text
Data/Rendering/Authored/render.quality.default.json
→ validate / canonical / stale-save guard
→ Tools/RenderingPipeline publisher
→ Client/Bin/DataFiles/Rendering/render.quality.default.json
→ startup parse / validate / stage / commit
```

Save, Publish, Runtime Restart는 서로 다른 상태로 표시한다. 매 프레임 JSON을 읽지 않는다.

## 5. 후속 렌더링 로드맵

| 단계 | 실제 완료 조건 |
|---|---|
| RT Debug | Diffuse/Normal/Shade/Specular/Emissive/SceneHDR/Bloom/Final을 선택 캡처 |
| Color grading | 3D LUT 또는 검증된 2D LUT sampling과 before/after |
| SSAO | Depth/Normal 기반 AO RT, blur/composite, contact 영역 A/B |
| DoF | Depth 기반 CoC, near/far 분리 blur/composite, focus distance A/B |
| Shadow | 현재 과대 shadow RT 축소/설정화, CSM split + PCF correctness |
| PBR | G-Buffer metallic/roughness/AO, GGX NDF/geometry/Fresnel, energy conservation |
| IBL | diffuse irradiance + prefiltered specular cube + BRDF LUT split-sum |
| Forward+ | compute tile light cull, light list, 다광원 correctness/perf |
| TAA/SSR | motion/history 또는 Hi-Z 계약과 rejection/rollback |

PBR의 “두 종류 이미지를 나중에 결합”한다는 설명은 보통 IBL specular의 prefiltered environment와
BRDF integration LUT를 결합하는 split-sum 근사를 뜻한다. 현재 LostArk 엔진에는 그 두 입력과
GGX 소비 경로가 없으므로 이번 Bloom 작업으로 대신할 수 없다.

## 6. 자동 검증

1. 기본값이 기존 HLSL 상수와 일치하는지 소스 검사
2. NaN/Inf/음수/out-of-range apply가 실패하고 active 설정을 보존하는지 검사
3. Bloom OFF가 bloom pass를 건너뛰고 stale BloomResult를 intensity 0으로 무시하는지 검사
4. FXAA OFF/ON token과 shader 상수 바인딩 검사
5. Engine shader 정본과 Client 배포본 SHA 일치
6. Engine x64 Debug 빌드
7. `UpdateLib.bat Debug`
8. Client x64 Debug 빌드
9. 관련 audit, `git diff --check`

## 7. 수동 GPU 검증

자동 빌드는 품질 PASS가 아니다. 사용자가 다음 고정 장면을 캡처한 뒤에만 시각 완료를 판정한다.

### Character Select

```text
A. Legacy Defaults
B. Bloom OFF
C. Exposure 1.0~1.35 + Bloom OFF
D. C + Threshold 1.2~1.8 / Intensity 0.12~0.30
E. D + FXAA ON
```

### Valtan

- 청색 결정과 발광 창은 halo가 생긴다.
- 석재·난간·수풀은 BloomExtract에서 거의 사라진다.
- 그림자와 검정이 회색 안개처럼 들리지 않는다.

### Effect

- 같은 Q/R/T 또는 A 검격에서 Bloom OFF/ON을 비교한다.
- Emissive 중심만 퍼지고 Mask 외곽과 보라색 rim은 유지된다.
- 화면 가장자리 wrap, black frame, NaN, UI blur가 없어야 한다.

## 8. 완료·미완료 표현

이번 슬라이스 완료 후 사용할 수 있는 표현:

> 공용 legacy Deferred 경로에서 HDR Light 보존, configurable Bloom/Hable, 실제 FXAA와
> F1 Rendering Workbench를 구현하고 Debug 빌드를 통과했다.

사용하면 안 되는 표현:

> UE5 렌더링, PBR, IBL, Forward+, GI, DoF, CSM, SSAO를 모두 구현했다.

GPU A/B 전에는 “원작과 동일한 퀄리티”도 완료라고 기록하지 않는다.

## 9. 08-08 확장 목표: Effect 중심 Scene Rendering

이번 확장은 장면 Bloom 튜닝이 주 목표가 아니다. 정본 흐름은 다음과 같다.

```text
Effect Detail / Authored Effect
→ Base / Noise / Mask / Emissive / Dissolve
→ Mesh / Sprite / Trail / Decal
→ typed transient Light / Screen Post
→ FP16 SceneHDR / Distortion
→ scene lighting / shadow와 합성
→ Bloom / Hable / FXAA
→ 고정 카메라 A/B
```

Effect의 HDR core, alpha silhouette, distortion, typed Light, Screen Post가 먼저 정확해야 한다.
Character Select와 Valtan의 장면 프로필은 같은 Effect를 서로 다른 분위기에서 망가뜨리지 않도록
받쳐 주는 기반이다. 비발광 지형과 캐릭터를 Bloom으로 밝히지 않는다.

### G06. 장면 프로필과 전환 transaction

- `CLIENT_LEVEL_DESCRIPTOR`는 stable scene profile ID를 소유한다.
- Loading/Lobby, Character Select, Bern, Valtan, Development가 모두 명시적 프로필을 가진다.
- 목표 프로필의 persistent light와 scene multiplier를 active 상태 밖에서 검증·stage한다.
- 목표 Level 생성과 `Change_Level`이 성공한 뒤에만 profile을 commit한다.
- 실패하면 이전 Level, active profile, persistent light, global Workbench 값을 유지한다.
- Effect의 frame transient Light는 persistent scene light 교체 대상이 아니다.
- 성공한 전환 프레임에는 이전 Level의 render submission을 폐기한 뒤 새 profile을 적용한다.

`RENDER_QUALITY_SETTINGS`의 FXAA, Gamma, Bloom On/Off와 사용자의 live 값은 process-global 기술
품질로 유지한다. Scene profile은 key/fill/ambient와 exposure/bloom multiplier만 소유하고 전역
Workbench 값을 덮지 않는다.

### G07. Shadow와 조명 합성 교정

현재 shadow 비교는 최종 `diffuse * shade + specular + emissive` 전체에 `0.3`을 곱한다. 이 구조는
그림자 안의 ambient와 Effect/오브젝트 emissive까지 죽인다. 변경 후 계약은 다음과 같다.

```text
direct diffuse × shadow
+ ambient/environment fill
+ specular × shadow
+ object/effect emissive (shadow 비대상)
```

- `LIGHT_DESC.vSpecular`를 실제 deferred shader에 전달한다.
- invalid light type, 0 방향벡터, 음수/비유한 색과 point range를 stage에서 거부한다.
- 8192×4608 RGBA32F single shadow target은 품질이 아니라 과도한 메모리 사용이다.
- 첫 교정은 shadow resolution, depth bias, strength를 설정화하고 PCF를 적용한다.
- CSM은 split별 light matrix와 atlas/array correctness를 갖춘 후속 독립 G로 유지한다.

### G08. Valtan WMA2와 애니메이션 재질

실측상 `Character/Valtan/MN_RPBF_01.wmodel`의 embedded material은 legacy WMAT이며 diffuse만
보존한다. 같은 리소스 폴더에는 normal/specular/emissive 원본이 존재하지만 현재 모델이 소비하지
않는다. 또한 `Shader_VtxAnimMeshBinary.hlsl`은 emissive를 항상 0으로 출력한다.

따라서 기존 mesh/skeleton/27 animation을 유지하면서 다음을 수행한다.

- 원본 FBX material name과 texture family를 근거로 WMA2를 생성한다.
- base/normal/specular/emissive를 material index별로 명시한다.
- 기존 `.wmesh/.wskel/.wanim`을 그대로 pack하여 candidate `.wmodel`을 만든다.
- `ModelAssetConverter info`와 runtime material probe로 슬롯과 파일 존재를 확인한 뒤 원자 교체한다.
- Valtan body shader가 normal/specular/emissive를 G-buffer에 출력하도록 연결한다.
- emissive intensity는 HDR core를 만들며 Bloom은 그 결과만 제한적으로 확산한다.

### G09. Effect 중심 Rendering Workbench

F1 `Rendering Workbench`는 장면 post만 조절하는 창으로 끝내지 않는다. 다음 read/write 상태를
분리해 표시한다.

```text
Global technical quality
  Bloom / Hable / FXAA

Active scene profile
  profile ID / key / ambient / specular / shadow / scene multipliers

Effect presentation diagnostics
  selected Effect ID / sample time
  HDR effect layer / transient Light / Screen Post counts
  Effect Light ON/OFF / Screen Post ON/OFF / Bloom ON/OFF

Render target diagnostics
  Diffuse / Normal / Shade / Specular / Emissive
  SceneHDR / Distortion / Bloom Extract / Bloom / Final
```

Effect ON/OFF는 Authored 문서를 변경하지 않는 preview 진단 기능이다. Save/Publish/Runtime 적용과
혼동하지 않는다. Effect Detail의 layer 값은 계속 해당 문서가 소유한다.

### G10. 100% 판정 경계

원본 UE3 parent material graph, MI 상속, dynamic parameter curve, 카메라/HDR/LUT metadata가 없는
상태에서 픽셀 100% 동일을 주장하지 않는다. 완료 명칭은 `고정 레퍼런스 조건 지각적 정합`이다.

- Effect resource/channel/profile 적용률 100%, silent fallback 0
- fixed camera/FOV/sample/animation/post manifest 보유율 100%
- unintended opaque card, non-emissive Bloom leak, normal-to-base 출력 0
- neutral ROI exposure 차이 0.15 EV 이하
- neutral median DeltaE 5 이하, 95 percentile 12 이하
- low-frequency lighting SSIM 0.90 이상
- Effect silhouette/material/timing과 Character Select/Valtan 분위기 수동 승인

PBR/GGX와 IBL은 지원 checkbox로 먼저 선언하지 않는다. 원본 spec/gloss 계약과 WMA2의
metallic/roughness/AO 증거를 분리한 뒤 G-buffer consumer, BRDF, irradiance/prefilter cube, BRDF LUT가
실제 연결되고 GPU A/B를 통과할 때만 완료로 기록한다.
