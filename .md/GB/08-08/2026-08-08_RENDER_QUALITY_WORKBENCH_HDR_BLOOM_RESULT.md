# 2026-08-08 Rendering Quality Workbench / HDR·Bloom 결과서

## 1. 결론

Character Select, Bern, Valtan, Effect가 공유하는 기존 `CRenderer` 경로에 첫 렌더 품질
튜닝 슬라이스를 통합했다.

```text
Deferred G-Buffer
→ FP16 Light Accumulation
→ FP16 SceneHDR / Distortion
→ typed Screen Post
→ configurable half-resolution Bloom
→ configurable Hable Tone Mapping + optional FXAA
→ display-space UI / ImGui
```

F1 `LostArk Developer Tools`에서 `Rendering Workbench`를 열면 Bloom, Tone Mapping, FXAA를
실시간으로 조절할 수 있다. 값 변경은 전체 검증 후 다음 프레임부터 Character Select,
Valtan, Effect에 공통 적용된다.

이번 결과를 PBR, IBL, Forward+, GI, DoF 구현 완료라고 부르지 않는다. 현재 완료된 것은
기존 legacy Deferred renderer의 HDR/Bloom/Tone Mapping/FXAA 첫 수직 슬라이스다.

## 2. 구현 내용

### 2.1 HDR Light 보존

- `Target_Shade`: `R8G8B8A8_UNORM` → `R16G16B16A16_FLOAT`
- `Target_Specular`: `R16G16B16A16_UNORM` → `R16G16B16A16_FLOAT`

이전에는 Light 결과가 SceneHDR에 합쳐지기 전에 1로 잘렸다. 이제 1보다 큰 diffuse/specular
highlight가 SceneHDR와 Bloom Extract까지 보존된다. Emissive와 Effect는 기존 FP16 경로를
계속 사용한다.

### 2.2 Typed runtime 설정

`RENDER_QUALITY_SETTINGS`를 Engine public 계약으로 추가했다.

```text
Bloom Enabled
Bloom Threshold / Soft Knee / Intensity / Scatter
Exposure / Hable White Point / Display Gamma
FXAA Enabled / Blend / Edge Threshold / Edge Threshold Min
```

기본값은 기존 HLSL 하드코딩과 같다.

```text
Bloom true / 1.0 / 0.5 / 0.8 / 1.0
Exposure 2.0 / White Point 11.2 / Gamma 2.2
FXAA false / 0.75 / 0.166 / 0.0833
```

Renderer는 모든 float의 finite/range를 검사한 뒤 전체 설정을 한 번에 교체한다. 실패하면
기존 active 설정을 유지한다. Client는 Renderer 포인터를 직접 소유하지 않고
`CGameInstance::Get/Apply_RenderQualitySettings`를 사용한다.

### 2.3 Bloom/Hable

- Bloom OFF면 Extract/Horizontal/Vertical 세 pass를 실행하지 않는다.
- Final은 Bloom OFF일 때 stale BloomResult를 사용하지 않는다.
- Threshold와 Soft Knee가 bright-pass를 결정한다.
- Scatter가 half-resolution blur의 texel step을 조절한다.
- Intensity, Exposure, White Point, Gamma가 Final resolve에서 uniform으로 적용된다.
- UI/ImGui는 Tone Mapping 이후에 렌더되므로 world exposure와 FXAA의 영향을 받지 않는다.

### 2.4 실제 FXAA

FXAA는 이름만 추가한 checkbox가 아니다.

- Tone Mapping과 gamma가 적용된 중앙/대각 sample의 luminance를 계산한다.
- local contrast가 edge threshold보다 작으면 즉시 원본을 유지한다.
- edge 방향을 구해 제한된 span 안에서 두 단계 sample을 합성한다.
- 결과 luminance가 이웃 범위를 벗어나면 보수적인 blend를 선택한다.
- 기본값은 OFF라 기존 화면을 유지한다.

별도 renderer나 중첩 MRT를 만들지 않고 기존 Final pass에서 평가한다. UI는 그 뒤에 그려져
글자가 FXAA로 흐려지지 않는다.

### 2.5 F1 Rendering Workbench

새 전역 기능키를 추가하지 않았다.

```text
F1
→ LostArk Developer Tools
→ Rendering Workbench
```

제공 기능:

- 모든 수치의 live slider/checkbox
- `Reset Legacy Defaults`
- `Reference A/B Start`
- `Reload Active`
- 현재 pipeline과 viewport 표시
- invalid apply와 session-only 상태 표시

`Reference A/B Start`는 다음 보수적 탐색값이다.

```text
Threshold 1.4
Soft Knee 0.45
Intensity 0.20
Scatter 1.0
Exposure 1.20
White Point 11.2
Gamma 2.2
FXAA ON
```

원작 확정값이 아니며 고정 카메라 A/B의 시작점이다.

## 3. Valtan Bloom 판정

Valtan의 모든 은은한 빛을 Bloom으로 처리하면 안 된다.

Bloom이 담당하는 것:

- 발광 창
- 청색 결정
- Effect의 HDR emissive core
- 강한 specular highlight 주변의 제한된 halo

Bloom이 만들 수 없는 것:

- 표면 normal에 따른 명암
- indirect/environment light
- metallic/roughness별 반사
- contact shadow
- 인접 오브젝트를 실제로 비추는 light transport

따라서 석재·난간·수풀까지 BloomExtract에 많이 들어오면 Threshold/Exposure가 잘못된 것이다.

## 4. 자동 검증

| 검증 | 결과 |
|---|---|
| `Shader_Deferred.hlsl` FXC `fx_5_0` | PASS, 오류 0 |
| Rendering Workbench audit | PASS |
| Engine x64 Debug | PASS |
| `UpdateLib.bat Debug` | PASS |
| Client x64 Debug | PASS |
| Debug Client startup | PASS, 12초 생존 |
| Engine x64 Release | PASS |
| `UpdateLib.bat Release` | PASS |
| Client x64 Release | PASS |
| Release Client startup | PASS, 12초 생존 |
| Engine/Client Shader SHA256 | 동일 |
| `git diff --check` | PASS |

FXC의 `Effects deprecated for D3DCompiler_47` 경고는 기존 Effects11 사용 경고이며 셰이더
컴파일 오류가 아니다. 구현 중 발생했던 uninitialized 경고는 명시적 초기화로 제거했다.

## 5. 전체 ProjectAudit

전체 ProjectAudit은 이번 렌더 변경과 무관한 공유 작업트리 실패 두 건 때문에 종료 코드 1이다.

```text
projects.data-source-visibility
  expected=549 project=547 filters=547

effect.wfx-component-assembly
  WFX compile identity mismatch:
  effect.dimensionmaster.skill.2050120
```

같은 실행에서 Effect Tool final bundle은 PASS했다. 이번 작업은 DataFiles 프로젝트 등록과
차원술사 WFX/Assembly를 수정하지 않았으므로 다른 담당 변경을 섞어 자동 교정하지 않았다.

## 6. 수동 GPU 검증 상태

아직 수행하지 않았다. 빌드와 startup smoke를 시각 품질 PASS로 기록하지 않는다.

다음 순서로 사용자가 확인한다.

1. Debug Client를 `Client/Default`에서 실행한다.
2. Character Select idle에서 F1 → Rendering Workbench를 연다.
3. `Reset Legacy Defaults` 캡처를 남긴다.
4. Bloom을 OFF하고 석재 문양·캐릭터 실루엣을 비교한다.
5. `Reference A/B Start`를 적용한다.
6. FXAA OFF/ON으로 바닥 대각선과 캐릭터 외곽만 비교한다.
7. Valtan의 발광 창/결정에서 Threshold와 Intensity를 조절한다.
8. Effect Q/R/T 또는 A에서 흰 core가 퍼져도 보라 rim과 Mask 외곽이 유지되는지 확인한다.

판정 기준:

- 비발광 흰 바닥·석재가 BloomExtract에서 거의 사라진다.
- 검정과 문양이 회색 안개처럼 들리지 않는다.
- FXAA ON이 world geometry edge만 부드럽게 하고 HUD/ImGui는 흐리지 않는다.
- black frame, NaN, 화면 가장자리 wrap이 없다.

## 7. 명시적 미완료

- Rendering Workbench 값은 현재 process session 전역이며 JSON Save/Publish는 없다.
- RT별 ImGui thumbnail capture는 아직 없다.
- Character/Valtan/Deploy/Effect Model Cue의 animated material D/N/S/E 소비는 이번 후속
  슬라이스에서 연결했다. 다만 원본 MI의 flicker 함수는 남지 않아 Valtan 청록 발광은
  추출된 색·강도를 사용하는 `RECONSTRUCTED_PROFILE`이다.
- `LIGHT_DESC.vSpecular` 색은 이번 후속 슬라이스에서 legacy light shader까지 연결했다.
- PBR metallic/roughness/AO G-Buffer와 GGX BRDF는 없다.
- IBL irradiance/prefiltered cubemap/BRDF LUT는 없다.
- DoF, SSAO, SSR, CSM/PCF, TAA, Forward+는 없다.
- 기존 8192×4608 shadow color+DSV의 큰 메모리 사용은 별도 성능 슬라이스다.
- transient Light/Screen Post 다중 fullscreen pass 성능은 profiler A/B가 필요하다.

## 8. 다음 순서

1. 이번 Workbench로 Character Select와 Valtan 고정 카메라 A/B 수치를 확정한다.
2. Diffuse/Normal/Shade/Specular/Emissive/SceneHDR/Bloom/Final RT debug view를 추가한다.
3. 확정된 필드만 `Data/Rendering/Authored` JSON + publisher 계약으로 저장한다.
4. legacy light의 specular 색 소비와 Character/Valtan object emissive를 닫는다.
5. G-Buffer PBR → GGX → IBL split-sum 순서로 확장한다.
6. DoF/SSAO/Shadow는 각각 별도 RT와 GPU A/B로 닫는다.

Forward+는 현재 Deferred를 대체하는 품질 checkbox가 아니다. 다광원 부하가 실제 병목으로
측정된 뒤 compute tile culling과 light list를 갖춘 별도 구조 작업으로 판단한다.

## 9. Effect 중심 Scene Rendering 후속 통합

### 9.1 구현 결과

이 후속 슬라이스는 Effect를 최종 소비자로 두고 Character Select와 Valtan의 scene material,
light, shadow, post를 같은 HDR 합성 경로에 정렬했다.

```text
Opaque/Animated Mesh G-Buffer
  → persistent scene light
  → lit color에만 shadow 적용
  → object emissive 합성
  → Effect HDR Blend + Distortion
  → typed Effect Screen Post
  → Bloom
  → Hable / FXAA
  → UI / ImGui
```

- Effect의 Base/Mask/Dissolve/Noise/Emissive와 typed Light 23개, Screen Post 38개가
  최종 Bloom 이전에 들어가는 기존 경로를 유지했다.
- Rendering Workbench에서 typed Effect Light와 Screen Post를 각각 ON/OFF하고 최근 제출 수를
  확인할 수 있다. 따라서 A/B에서 Material 문제와 Light/Post 문제를 분리할 수 있다.
- WMA2 material name/hash를 런타임까지 보존하고, 공통 deferred material binder가 매 draw마다
  Diffuse/Normal/Specular/Emissive 존재 여부와 값을 명시적으로 설정한다. 이전 draw의 SRV나
  flag가 다음 mesh에 남는 stale-state도 차단한다.
- Valtan 본체 3 material과 무기 2 material은 원본 MI 증거의 청록색
  `(0.15, 1.5, 0.9)` 및 material별 intensity `5/15/10`을 stable material name으로 적용한다.
- Character body/equipment, NPC, animated Deploy, Effect Model Cue도 같은 D/N/S/E 소비 경계를
  사용한다. Valtan deploy static 경로는 기존 D/N/S/E 경로를 유지한다.
- shadow는 더 이상 object emissive를 0.3배로 죽이지 않는다. direct/ambient lit 결과에 shadow를
  적용한 뒤 emissive를 합성한다.
- Character Select는 warm high-key, Valtan은 cool low-key stable scene profile로 분리했다.
  최초 한 레벨이 등록한 전역 기본광을 이후 레벨이 계속 공유하던 함수-static 경로는 제거했다.
- persistent scene light 교체와 Effect transient light의 프레임 수명은 서로 분리했다.

### 9.2 자동 검증

| 검증 | 결과 |
|---|---|
| Engine x64 Debug | PASS, 오류 0 |
| `UpdateLib.bat Debug` | PASS |
| Client x64 Debug | PASS, 오류 0 |
| Debug Client startup | PASS, `Client/Default`에서 12초 생존 |
| Rendering Workbench audit | PASS |
| Effect Tool final audit | PASS, code 50 / documents 16 / resources 336 / palette 2667 |
| Effect runtime 자료 | Particle 46 / typed Light 2 / typed Post 3 / reconstructed groups 21 |
| Engine/Client deferred shader SHA | 동일 |

Effect Tool final audit의 `runtime-exact material=0`은 그대로다. 이번 작업은 Effect Material
원본 그래프를 자동 복원한 것이 아니라, 이미 복원·튜닝한 Effect가 scene HDR, object emissive,
Light/Post, Bloom에서 손실 없이 비교되도록 렌더 경계를 고친 작업이다.

전체 ProjectAudit은 공유 작업트리의 기존 두 실패만 남았다.

```text
projects.data-source-visibility: expected=549 project=547 filters=547
effect.wfx-component-assembly: expectedEffects=16 missingMappings=0, compiler traceback
```

### 9.3 수동 GPU A/B 절차

빌드와 startup smoke는 시각 품질 PASS가 아니다. 다음 네 묶음을 같은 해상도, 카메라, FOV,
애니메이션 frame, Effect sample time으로 캡처해야 한다.

1. Character Select idle: scene profile과 석재·금테·캐릭터의 명암을 확인한다.
2. Character Select DimensionMaster A: Effect Light ON/OFF, Post ON/OFF, Bloom ON/OFF를 각각
   비교하여 흰 core, 보라 rim, Mask 경계와 4연 검격 타이밍을 분리 판정한다.
3. Valtan boss idle: 본체 3 material과 무기 2 material의 청록 Emissive가
   `Target_Emissive`와 최종 Bloom에 나타나고 갈색 단색으로 뭉개지지 않는지 확인한다.
4. Valtan crystal/deploy: 청록색은 국소 accent로만 남고 바닥·석재 전체가 cyan 또는 Bloom으로
   뜨지 않는지 확인한다.

Workbench에서 Effect Light를 끄면 형상·Material은 유지되고 국소 조명만 사라져야 하며,
Screen Post를 끄면 mesh/sprite 형상은 유지되고 화면 왜곡만 사라져야 한다. Bloom을 끄면
발광 core와 색은 남고 halo만 사라져야 한다.

### 9.4 아직 완료가 아닌 것

- DimensionMaster Material `RUNTIME_EXACT`는 0이며 A의 4연 검격은 계속 Effect Detail에서
  개별 body/rim/highlight/afterimage를 GPU A/B 튜닝해야 한다.
- Valtan MI의 정확한 flicker curve가 없어 현재 청록 Emissive는 finite reconstructed profile이다.
- scene profile 값은 코드에 seed한 live baseline이며 아직 Authored JSON/publisher와 고정 화면
  수동 승인을 거치지 않았다.
- PBR metallic/roughness/AO G-Buffer, GGX, IBL, Forward+, DoF, SSAO, SSR, CSM/PCF, TAA는
  구현하지 않았다. 현재 완료 명칭은 `legacy Deferred + Effect-centered HDR integration`이다.
- shadow는 여전히 8192×4608 hard shadow이고 ambient 분리, PCF/CSM, 메모리 축소가 필요하다.
- Valtan base catalog 275행 중 물리 WModel 40개만 확인되어 235개 누락은 별도
  Resources hydration/merge blocker다. 셰이더 튜닝으로 해결할 수 없다.
- 레벨 전환 전체를 하나의 no-allocation render-state transaction으로 묶고 이전 프레임 제출을
  폐기하는 계약은 아직 완성하지 않았다. 알려진 scene profile은 성공한 level change 뒤 적용한다.

따라서 원작 UE3 로아와의 `100%`는 현재 주장하지 않는다. 완료 기준은 먼저 Effect 채널 누락 0,
fallback 0, 고정 A/B manifest 100%를 달성한 뒤, 장면별 노출·색·Bloom·shadow와 Effect 실루엣을
reference-condition perceptual match로 승인하는 것이다.
