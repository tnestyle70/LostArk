# LostArk 렌더링·물·스킬 재질·Profiler 코드 가이드

[전체 코드 지도](2026-10-03_VISUAL_STUDIO_CODE_ATLAS.md) · [최종 VS 필터 트리](2026-10-03_VISUAL_STUDIO_FILTER_TREE.md)

조사일: 2026-10-03. 작업 트리의 main 통합 후 코드 기준이며 origin/main의 rendering 기준은 c7de2091(PR #507)이다. 이 조사에서 C++/HLSL·런타임 데이터·빌드 설정을 바꾸거나 Client/UI를 실행하지 않았다. 별도 위임받은 Engine의 `.vcxproj.filters` 탐색 분류는 G08에 기록했다. 이 문서는 전체 함수 전수 해설의 완료본이 아니라, 후속 코드 수업과 구조 정리의 실제 진입점 및 확인한 경계를 기록한다.

## G01. 한 프레임이 화면이 되는 실제 호출 순서

진입점은 [Engine/Private/Renderer.cpp:865](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:865)의 `CRenderer::Draw`다. 객체가 renderer queue에 자신을 제출하고 Renderer가 pass별로 `Render_Group`을 부른다. `Renderer`가 게임 skill이나 effect JSON을 직접 해석하는 구조는 아니다.

```text
CRenderer::Draw
  → Presentation.Submit_FrameProviders
  → Ready_ScenePostTargets
  → Submit_FinalCameraObjects
  → Render_Shadow
  → Render_Portraits (초상 view; 일부 공통 RT와 조명/최종합성 재사용)
  → Render_NonBlend → MRT_GameObject의 G-buffer
  → Capture_PickingDepth
  → Render_SSAO (설정 ON일 때)
  → Render_Lights → MRT_LightAcc
  → Begin MRT_SceneHDR
      → Render_Priority
      → Render_Combined
      → Render_ScreenSpaceLighting (SSGI/SSR ON일 때)
      → Render_NonLight
      → Render_SceneReplacements
      → Capture_SceneColorSnapshot (요청 시)
      → Render_Blend
      → SCENE_UI
  → End MRT_SceneHDR
  → Render_ScreenPosts
  → Render_Bloom (설정 ON일 때)
  → Render_Final
  → Render_DisplayOverlays
  → Render_UI
  → Render_Debug (제출된 debug component가 없으면 즉시 반환)
  → Render_Picking
```

[Renderer.cpp:354](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:354)의 `MRT_GameObject`에는 Diffuse, Normal, Depth, PickPos, Emissive, MaterialSpecular, CharacterSurface, CharacterGeometry가 등록된다. [Renderer.cpp:372](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:372)의 `MRT_LightAcc`는 Shade와 Specular다. 불투명 표면 정보와 광원 결과를 분리한 deferred 경로에 투명/특수 source material의 forward 경로가 결합되어 있다.

`Render_Combined`는 G-buffer와 조명 결과를 HDR scene에 합성한다. `Render_Blend` 이후에는 픽셀별 깊이에 재질 정보를 모두 남기는 일반 G-buffer 방식으로 모든 투명을 다루지 않는다. 따라서 뒤의 SSGI/SSR을 물·스킬 투명까지 포괄하는 기능이라고 말하면 틀린다.

주요 파일 책임:

| 파일 | 책임 |
|---|---|
| [Renderer.cpp:1228](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:1228) `Render_NonBlend` | 불투명 queue를 MRT에 제출 |
| [Renderer.cpp:1264](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:1264) `Render_SSAO` | depth/normal 기반 AO 및 blur |
| [Renderer.cpp:1500](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:1500) `Render_Lights` | 월드·character receiver 조명 분리 |
| [Renderer.cpp:1615](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:1615) `Render_Combined` | 표면과 조명 결과 합성 |
| [Renderer.cpp:1868](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:1868) `Capture_SceneColorSnapshot` | scene-color와 bloom의 안전한 읽기용 snapshot |
| [Renderer.cpp:2195](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:2195) `Render_ScreenPosts` | distortion/native screen effect 등 화면 연출 |
| [Renderer.cpp:2409](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:2409) `Render_Bloom` | 문서별 bloom 기여를 포함한 blur 합성 |
| [Renderer.cpp:2498](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:2498) `Render_Final` | 출력용 후처리/resolve |

## G02. Shader라는 C++ 객체와 GPU 프로그램은 다르다

[Engine/Private/Shader.cpp:210](C:/Users/tnest/Desktop/LostArk/Engine/Private/Shader.cpp:210)의 `CShader::Initialize_Prototype`는 실행 중 HLSL 소스를 편집·컴파일하지 않는다. 전달된 논리 HLSL 파일명에서 실행 module 옆의 `.cso` 경로를 구하고, bytecode를 읽은 다음 `D3DX11CreateEffectFromMemory`(250행)로 FX11 effect를 만든다. pass의 vertex signature에 맞춰 `ID3D11Device::CreateInputLayout`(385행)을 준비한다.

`Bind_RawValue`, `Bind_Matrix`, `Bind_Texture`는 CPU의 값/행렬/SRV를 FX11 변수에 연결한다. [Shader.cpp:674](C:/Users/tnest/Desktop/LostArk/Engine/Private/Shader.cpp:674)의 `Begin(passIndex)`는 범위를 검증하고 필요한 source program group을 선택한 뒤 `IASetInputLayout`과 FX11 `Apply`를 실행한다. draw는 별도 `CModel/CMesh/VIBuffer`가 수행한다.

`PROGRAM_VARIANTS`(175행)는 program 범위별 shader와 변수 복사 연결을 가진다. `VARIABLE_COPY`의 revision은 같은 값을 매번 복사하는 비용을 줄이기 위한 캐시다. HLSL 파일이 많아도 그것이 각각 새로운 gameplay class라는 뜻은 아니다. carrier·program group·render state 조합이 build-time permutation과 pass로 분리된다.

`CShader` clone은 FX11 effect 상태를 공유하는 경로가 있으므로 쓰고 난 material override를 복구해야 한다. 예: [MapAssetObject.cpp:333](C:/Users/tnest/Desktop/LostArk/Client/Private/MapAssetObject.cpp:333) 부근은 draw 실패 뒤에도 vortex/water 입력을 identity로 reset한다. 상태 누출은 다음 오브젝트의 잘못된 색·변형으로 보일 수 있다.

## G03. 물은 두 코드 경로를 구분해야 한다

### 범용 mapwater profile 경로

[Client/Public/MapAssetCatalog.h:69](C:/Users/tnest/Desktop/LostArk/Client/Public/MapAssetCatalog.h:69)의 `MAP_ASSET_WATER_PROFILE`은 불투명도, Fresnel, distortion, normal/reflection 세기, UV tiling/panning, 색, 보조 texture 이름 등을 보관한다.

[MapAssetCatalog.cpp:2339](C:/Users/tnest/Desktop/LostArk/Client/Private/MapAssetCatalog.cpp:2339)의 `Load_WaterPresentation`은 runtime `<Area>.mapwater.json` 또는 authoring override의 문서를 읽는다. schema `lostark.map-water-presentation`, version 1, area 일치, 최대 64행, 필수 유한 수치/벡터, 중복 asset ID와 WATER asset ↔ water row 양방향 대응을 검사한 후 staged map을 보관한다. **이 함수는 진입 시 기존 `m_WaterProfiles`를 clear한다. 함수 하나를 근거로 이 전체 loader가 기존 catalog까지 항상 보존한다고 서술하면 안 된다.**

[MapAssetObject.cpp:263](C:/Users/tnest/Desktop/LostArk/Client/Private/MapAssetObject.cpp:263)의 `Render_Group`은 material별 render profile로 pass를 선택한다. WATER지만 profile이 없으면 TRANSLUCENT로 낮추는 분기가 있고, WATER일 때만 `Bind_WaterShaderResources(true)`를 호출한다. 이후 `Bind_Material → Shader::Begin → Model::Render` 순서다.

[Shader_VtxMeshBinary.hlsl:613](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl:613)의 식:

```text
UV(t) = UV × tiling.xy + panning.zw × elapsedTime
N_tangent = normalize(float3(sampleNormal.xy × intensity, 1))
N_world = normalize(N_tangent × tangentBasis)
fresnel = saturate(pow(1 - saturate(dot(N_world,V)), power) × intensity)
alpha0 = saturate(pow(saturate(opacity), opacityPower))
alpha = (alpha0 + (1-alpha0) × fresnel) × colorTint.a
distortion.xy = N_tangent.xy × screenDistortionIntensity × 0.05
```

출력은 color, distortion, bloom 기여다. reflection texture가 있다면 diffuse에 반사색 × 반사세기 × Fresnel을 더하는 코드가 있다. 그러나 현재 [MapAssetObject.cpp:733](C:/Users/tnest/Desktop/LostArk/Client/Private/MapAssetObject.cpp:733)는 **detailNormal/reflection의 보조 SRV를 이 경로가 소유하지 않아 두 has flag를 0으로 고정**한다. JSON의 texture 이름 존재와 실제 SRV 바인딩 성공은 같은 증거가 아니다. 이 범용 물을 완전한 ocean simulation 또는 SSR 물 반사라고 설명하면 안 된다.

### source water family 38~43 경로

이 경로는 위 범용 PS_WATER의 식으로 요약할 수 없다.

```text
mapmaterials의 source family·parameter·texture binding
 → SourceCharacterMaterialParameters_Generated.inl
 → SourceMapWaterMaterial::Configure
 → MODEL_SOURCE_CHARACTER_PARAMETERS
 → CModel / CMaterial
 → MapAssetRenderUtils::Bind_Material
 → Shader_SourceMapForwardPrograms::EvaluateSourceMapWater
 → SourceMapWater38/39/40/41/42/43 또는 Baked permutation
```

[SourceMapWaterMaterialParameters.h:14](C:/Users/tnest/Desktop/LostArk/Client/Public/SourceMapWaterMaterialParameters.h:14)의 `Configure`는 문자열 family를 program ID와 texture mask 및 float4 constant lane으로 변환한다. 파라미터를 찾지 못하거나 입력 중 미소비 이름이 남으면 false, 성공 때만 `result=staged`다. constant는 임의 slider 배열이 아니라 원본 프로그램 ABI를 재구성한 배치다.

[Engine/Public/BinaryAsset/ModelAssetData.h:64](C:/Users/tnest/Desktop/LostArk/Engine/Public/BinaryAsset/ModelAssetData.h:64)의 source material 계약은 64개 float4 base/light constant, 16개 texture lane, program/mask를 가진다. [Material.cpp:479](C:/Users/tnest/Desktop/LostArk/Engine/Private/Material.cpp:479)의 초기화는 texture SRV·baked average/directional·static shadow 등을 준비하고, [Material.cpp:948](C:/Users/tnest/Desktop/LostArk/Engine/Private/Material.cpp:948)의 `Bind_SourceCharacterInputs`는 실제 program/constant/texture를 shader에 바인딩한다.

[MapAssetObject.cpp:176](C:/Users/tnest/Desktop/LostArk/Client/Private/MapAssetObject.cpp:176)은 source water 38~43이 보이면 scene-color snapshot을 요청한다. [MapAssetRenderUtils.cpp:1153](C:/Users/tnest/Desktop/LostArk/Client/Private/MapAssetRenderUtils.cpp:1153) 부근은 depth, scene color, fog, ambient, direct scene lights와 baked 입력을 공급한다.

[Shader_SourceMapForwardPrograms.hlsli:910](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_SourceMapForwardPrograms.hlsli:910)은 현재 프로젝트의 meter/world basis를 source centimeter/basis로 변환하고 vertex color, UV, tangent view, fog, lightmap UV 등을 `SOURCE_CHARACTER_NATIVE_INPUT`에 채운다. program 42 ocean은 translated-world position과 camera origin을 구분한다. program 43은 별도 varying 순서를 사용한다. 40~43은 baked lighting 유무로 다른 함수에 dispatch한다.

[Shader_SourceMapWaterPrograms.hlsli:9](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_SourceMapWaterPrograms.hlsli:9)부터 실제 translated program이 있으며 38, 39, 40, 41, 42, 43과 40~43 Baked가 나뉜다. 해설은 선택한 실제 배치의 materialName/family/program/texture mask부터 고정한 다음 해당 식을 읽어야 한다. 현재 문서에는 한 물 표면의 전체 원본 DXBC 수치 parity를 새로 검증한 것으로 기록하지 않는다.

## G04. 스킬 이펙트의 다섯 책임

```text
skill/pattern cue
 → EffectAssetId
 → stable element/occurrence
    1. Composition: 언제·어디서·얼마나 오래
    2. Carrier: sprite/mesh/decal/trail/light/screen/model geometry와 simulation
    3. Material descriptor: texture·channel·constant·sampler·정책
    4. Material program: UV·색·coverage·dissolve·distortion 계산식
    5. Renderer adapter: VF/pass/RT/state/scene input 연결
 → GPU draw / 화면 연출
```

스킬당 HLSL 하나, element당 shader 하나라는 관계가 아니다. 같은 프로그램은 texture/scalar descriptor를 바꿔 여러 스킬이 재사용한다. 같은 그림 texture를 sprite에 넣었다고 바닥에 투영되는 decal carrier가 되지도 않는다.

실제 자료구조:

| 선언 | 변수 의미 |
|---|---|
| [Effect_AuthoringDocument.h:2074](C:/Users/tnest/Desktop/LostArk/Client/Public/Effect_AuthoringDocument.h:2074) `EFFECT_DOCUMENT_DESC` | stable EffectAssetId, format version, bloomIntensity, particle system, model cues, owner controls, element 배열 |
| [Effect_AuthoringDocument.h:1515](C:/Users/tnest/Desktop/LostArk/Client/Public/Effect_AuthoringDocument.h:1515) `EFFECT_ELEMENT_DESC` | stable elementId/groupId/sourceNode, visible, kind, runtime carrier, composition layer, resource bindings, material, attachment/inheritance, source tracks, authoring overrides |
| [Effect_AuthoringDocument.h:382](C:/Users/tnest/Desktop/LostArk/Client/Public/Effect_AuthoringDocument.h:382) `EFFECT_MATERIAL_EXECUTION_DESC` | enabled/failClosed/fidelity/backend/opcode/pass, texture lane와 sampler, scalar/vector, consumed/suppressed mask, render state |
| [Effect_DocumentRenderer.h:239](C:/Users/tnest/Desktop/LostArk/Client/Public/Effect_DocumentRenderer.h:239) `CEffectDocumentRenderer` | 준비된 document/resource, geometry adapter와 GPU 제출 수명 |

`UnboundSourceResources`는 source에 존재하지만 현재 material slot에 대응하지 않은 참조 보존 배열이다. 그 배열에 파일이 있다는 이유로 GPU에 바인딩되거나 화면에 나오는 것은 아니다. `bFailClosed`는 미해결 source를 일반 흰색/generic shader로 조용히 대체하지 않는 경계다. `bAuthoringApproximate`도 완전 복원 표식이 아니며 그 용도의 명시적 admission이 별도다.

[Effect_DocumentRenderer_Staging.cpp:276](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_Staging.cpp:276)의 `Stage_Document`와 [같은 파일:87](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_Staging.cpp:87)의 `Stage_PreparedInternal`이 준비된 resource를 stage한다. [Effect_DocumentRenderer_ResourceStaging.cpp:102](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_ResourceStaging.cpp:102)는 element별 texture/model/material execution을 준비한다. 임의 JSON HLSL 경로를 실행 중 compile하는 범용 material graph editor가 아니다.

[Effect_DocumentRenderer_MaterialBinding.cpp:326](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_MaterialBinding.cpp:326)의 `Bind_MaterialInputs`가 family별 typed packet과 scene inputs를 실제 shader에 연결한다. source scene depth/color/bloom은 375~380행, RuntimeMaterialV2 mask/count/scalar/input lane은 616행 이후, StandardColorV1은 714행 이후를 보면 된다.

[Effect_DocumentRenderer_Rendering.cpp:1361](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_DocumentRenderer_Rendering.cpp:1361)의 `Render`는 WorldMark와 Normal composition phase를 분리한다. `Render_CompositionPhase`는 descriptor/evaluated frame 일관성, GPU occurrence 수, admission을 검증하고 모델·element·particle·trail을 제출한다. 1697/1729/1748행의 `Render_Element`, `Render_Particles`, `Render_Trails`가 실제 geometry adapter 분기다. ScreenPost는 `Build_NativeScreenPost`(280행)에서 typed presentation output으로 다뤄진다.

### 저장과 활성화

정본은 `Data/Effects/Authored/*.effect.json`이며 `EffectCatalog.json`이 Product admission을 결정한다. `EffectResourceTree.json`은 저작 브라우저의 분류/표시 이름이다. 분류 폴더가 runtime 보스 identity나 admission을 결정하지 않는다. Effect에는 별도 generated runtime effect JSON을 만들지 않는 현재 계약이 있다.

[Effect_Tool_DocumentIo.cpp:173](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Tool_DocumentIo.cpp:173)의 `Try_SaveDocument`는 문서 종류와 Product/registry binding의 freshness를 검사하고 [302행](C:/Users/tnest/Desktop/LostArk/Client/Private/Effect_Tool_DocumentIo.cpp:302)의 `Save_AtomicIfUnchanged`를 사용한다. runtime 준비/activation 실패 시 319행 이후 저장한 canonical bytes를 기준으로 이전 파일 rollback을 시도한다. 성공 후 새 Product spawn은 새 immutable resource를 사용하며 이미 살아 있는 occurrence는 이전 resource를 유지한다(432행). 디스크 저장과 모든 살아 있는 이펙트 즉시 교체는 다른 일이다.

### 복원 수준을 부르는 정확한 방법

원작 Lost Ark 내부 엔진의 전체 source를 가진 것이 아니다. cooked UE3 ShaderMap/DXBC, material instance, emitter/module 등 증거에서 식/ABI를 재구성한다. [EFFECT_FAMILY_RUNTIME_ABI_RESTORATION_GUIDE.md:248](C:/Users/tnest/Desktop/LostArk/.md/TEAM/EFFECT_FAMILY_RUNTIME_ABI_RESTORATION_GUIDE.md:248)의 `V1_COMPLETE`와 `NATIVE_PARITY`를 구분해야 한다. typed RT0/carrier/input/timing/save/user visual approval이 닫힌 것과 native VF/MRT/sampler/scene feedback 전체 parity는 동일한 성과가 아니다.

## G05. PR #507 SSGI/SSR은 무엇을 실제 구현했는가

[Engine/Public/Engine_RenderTypes.h:133](C:/Users/tnest/Desktop/LostArk/Engine/Public/Engine_RenderTypes.h:133)의 quality 설정은 SSGI/SSR 기본 OFF다. [Renderer.cpp:1686](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:1686)의 `Render_ScreenSpaceLighting`은 `Render_Combined` 뒤, NonLight/scene replacement/투명 앞에 들어간다. shader 준비 실패 시 이전 설정을 보존하며 OFF에는 신규 shader load/pass/copy가 없다.

실제 식은 [Shader_ScreenSpaceLighting.hlsl:75](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_ScreenSpaceLighting.hlsl:75)의 `Receiver`, [96행](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_ScreenSpaceLighting.hlsl:96)의 `TraceScreen`, [146행](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_ScreenSpaceLighting.hlsl:146)의 `PS_SSGI`, [180행](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_ScreenSpaceLighting.hlsl:180)의 `PS_SSR`이다.

- Receiver는 `depth.w == 3`인 MapPBR뿐이다. depth.x=NDC, depth.y=viewZ/1000, normal.xyz=0~1 encoding, normal.a=roughness, material.rgb=F0, material.a=metallic을 소비한다.
- SSGI: 법선의 cosine-weighted 반구에 4/8/16 ray를 만들고 각 ray를 8개 고정 간격으로 depth 교차 검사한다. 화면 radiance를 거리감쇠, albedo, `(1-metallic)`, strength로 곱해 더한다.
- SSR: `reflect(viewIncident, normal)` 방향을 16/32/64 단계로 추적한다. 교차점 radiance에 Schlick Fresnel `F0+(1-F0)(1-N·V)^5`, `(1-roughness)^2`, 화면 가장자리 fade, strength를 곱해 더한다.
- 두 pass가 같은 원본 opaque HDR radiance를 읽는다. GI 결과를 SSR의 새 radiance로 재입력하지 않는다.
- ping-pong 임시 자원으로 수행하고 활성 pass가 모두 성공한 뒤 원본 HDR/bloom에 copy를 제출한다. copy 전 pass 실패는 원본을 건드리지 않는다. RT1 distortion과 유한한 기존 alpha/bloom을 보존하고 증가한 bright-pass 차이만 더한다. GPU device loss까지 데이터 원자성으로 보장하는 저장 transaction은 아니다.
- strength 0은 ray 계산을 조기 종료하지만 pass와 copy 고정비는 남는다.

이는 현재 화면의 가산 조명 실험이다. offscreen/가려진 geometry, 다중 bounce, motion vector/history, temporal denoising, Lumen Surface Cache, DXR BLAS/TLAS가 없다. 기존 baked/RNM/IBL을 물리적으로 대체하거나 에너지를 보존하는 GI라고 표현하지 않는다. fixed-step이 얇은 면을 놓칠 수 있고 camera/screen 경계에 의존한다.

관련 [2026-10-03_PROFILER_RENDERING_WORKBENCH_RESULT.md](C:/Users/tnest/Desktop/LostArk/.md/GB/10-03/2026-10-03_PROFILER_RENDERING_WORKBENCH_RESULT.md)는 shader/WARP 수치·state 복원 검증과 Debug build 증거를 남겼으나 사용자 실제 GPU/한국어 UI/쿠크 장면 최종 화면 검증과 Release 전체 Product를 완료로 기록하지 않는다. 이번 조사는 그 검사를 새로 실행한 것이 아니다.

## G06. Rendering Workbench의 실험과 저장

[RenderingProfileService.cpp:648](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingProfileService.cpp:648)의 `Experiment_Fields`는 41개 typed whitelist다. [720행](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingProfileService.cpp:720)의 `Set_ExperimentPreview`가 값·mask·owner 조건을 검증해 다음 완전한 frame에 적용할 overlay를 stage한다. [755행](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingProfileService.cpp:755)에서 기존 overlay를 복구하고 일반 profile/environment/video/presentation 값을 적용한 뒤 [797행](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingProfileService.cpp:797)의 실험 overlay를 마지막에 적용한다.

`CRenderingBenchmark`는 현재값을 A/B로 보관하고 준비 frame 후 측정, AB/BA 교대 반복과 단일 변수 sweep을 수행한다. 카메라/viewport/profile/region/light/video/debugger/detail capture 등 공통 조건 fingerprint가 다르면 run을 비교에서 제외한다. animation/server gameplay를 결정적으로 재생하는 도구는 아니므로 측정은 탐색적이다. `RenderingTechniqueGuide`는 읽기 전용 개념 사전이며 명칭이 보인다고 그 기술이 구현된 것은 아니다.

세션 실험은 rendering 원본 JSON을 저장하지 않는다. 종료/창 닫기/level·region·profile·video 변경 시 자신이 소유한 필드만 복구한다. 이에 비해 [RenderingProfileService.cpp:2238](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingProfileService.cpp:2238)의 `Save_Authored`는 최신 authoring catalog와 변경 필드를 merge하고 backup/temporary/ReplaceFile로 저장하는 별도 명령이다. 실험 종료 복구를 디스크 rollback이라고 설명하지 않는다.

[RenderingBenchmark.cpp:1586](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingBenchmark.cpp:1586)은 `LostArkRenderingBenchmark.v2` 결과를 저장하며 1703행에서 완성된 임시 파일을 `MoveFileExW`로 이동한다. `REPLACE_EXISTING`을 지정하지 않아 이름 충돌 시 기존 capture를 유지한다. 저장 위치는 실행 module의 부모 기준 `../BenchmarkCaptures`다. benchmark capture JSON과 rendering profile JSON은 용도와 쓰기 권한이 다르다.

## G07. Profiler의 숫자가 실제로 뜻하는 것

수집 정본은 [Engine/Public/Profiler.h](C:/Users/tnest/Desktop/LostArk/Engine/Public/Profiler.h), [Engine/Private/Profiler.cpp](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp)다. UI는 Client `ProfilerTool`, 저장/reader는 `ProfilerCaptureIO`다.

| 구조·함수 | 의미 |
|---|---|
| `FProfilerFrame` | frame 번호·CPU/QPC·GPU 상태·scope·counter·mesh draw·memory snapshot |
| `FProfilerScopeSample` | CPU 이름 ID/thread/depth/시간; thread별 중첩 |
| `FProfilerGpuScopeSample` | GPU timestamp interval과 scope begin/end 사이 제출량 차이 |
| `FProfilerMeshDrawSample` | 표시용 mesh/pass 이름 ID, material slot, vertex/index/instance 수; stable asset identity가 아님 |
| `FProfilerMemoryStats` | process/system/DXGI local/nonlocal usage/budget와 각각의 valid flag |
| `FProfilerCaptureWindow` | 요청/보유/저장/제외/퇴출 frame과 범위 |
| [Profiler.cpp:103](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:103) `Begin_Frame` | 새 frame 수집·이전 interval·메모리 low-rate 표본·GPU query 시작 |
| [Profiler.cpp:157](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:157) `End_Frame` | CPU 종료·GPU 종료·history commit |
| [Profiler.cpp:1176](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:1176) `Resolve_GpuFrames` | `D3D11_ASYNC_GETDATA_DONOTFLUSH`로 비동기 query 결과를 원래 frame에 귀속 |
| [Profiler.cpp:552](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:552) `Record_MeshSubmitted` | draw submission 작업량, frame 고유 mesh 수, 선택적 상세 trace |

고정 상한은 GPU query ring 8, GPU scope/frame 128, pipeline scope/frame 8, 고유 mesh 16,384, 상세 draw/frame 512, history 1,200이다. 고유 mesh 포인터는 그 frame의 중복 제거에 쓰며 export identity가 아니다. 상세 draw가 상한을 넘어도 총 제출량 counter는 계속 계수한다.

`frame N interval = Begin(N)-Begin(N-1) = PreviousCpuFrameMs + FrameGapMs`다. `CpuFrameMs(N)`를 같은 interval과 더하면 안 된다. CPU self는 같은 thread의 자식을 뺀 값이며 누락/잘못된 중첩이 있으면 N/A가 맞다. GPU pending을 0ms로 취급하거나 GPU elapsed를 utilization/FLOPs라고 부르면 안 된다. CPU QPC와 GPU timestamp lane의 원점도 서로 달라 chart x좌표만으로 인과 관계를 단정하지 않는다.

[Profiler.cpp:214](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:214)의 `Sample_Memory`는 Capture 중 약 1Hz로 K32GetProcessMemoryInfo/K32GetPerformanceInfo/DXGI QueryVideoMemoryInfo를 호출한다. resource별 texture memory 귀속이나 1초 사이 모든 peak를 잡는 기능은 아니다. local+nonlocal 또는 process+system을 단순 합산할 수 없다.

[Material.cpp:286](C:/Users/tnest/Desktop/LostArk/Engine/Private/Material.cpp:286)의 `LoadSharedTexture`에서 request/cache-hit/new-SRV producer가 연결된다. SRV 새 생성 누적을 현재 resident texture 개수로 해석하면 안 된다. 모든 texture loader의 총량도 아니다.

저장은 [ProfilerCaptureIO.cpp:762](C:/Users/tnest/Desktop/LostArk/Client/Private/ProfilerCaptureIO.cpp:762)의 동기 `Save_Json`과 [797행](C:/Users/tnest/Desktop/LostArk/Client/Private/ProfilerCaptureIO.cpp:797)의 UI용 exporter worker를 구분한다. schema는 `LostArkProfilerCapture.v3`(462행)이며 완성된 임시 파일을 `MoveFileExW`(659행)로 commit한다. 동기 API는 기존 파일 교체를 허용하지만 `BeginSave` worker는 `ReplaceExisting=false`로 호출해 이름 충돌 시 기존 capture를 보존한다. 저장 위치는 [867행](C:/Users/tnest/Desktop/LostArk/Client/Private/ProfilerCaptureIO.cpp:867)의 실행 module 부모 기준 `../ProfilerCaptures`다. 비교 reader는 [1481행](C:/Users/tnest/Desktop/LostArk/Client/Private/ProfilerCaptureIO.cpp:1481)부터 v1/v2/v3를 해석하고 없는 capability는 N/A로 보존한다. 실제 장면의 frame time을 연구할 때 export window 안에 관심 frame이 있는지 먼저 확인한다.

## G08. VS 탐색 분류의 책임 기준

아래 표는 파일의 소유 책임을 설명한다. 최종 Solution Explorer의 정확한 숫자·표기는 이번 통합 정리의 최종 filter 목록을 정본으로 삼는다. 이 표가 이전 filter 상태를 현재 상태로 고정하거나 물리 폴더 이동을 제안하는 것은 아니다. `Effect_DocumentRenderer*`는 Product에서도 쓰이는 renderer이고, `SourceMapWaterMaterialParameters`는 map source family를 변환한다는 사실이 분류의 기준이다.

| 논리 책임 | 파일 묶음 | 이유 |
|---|---|---|
| Presentation / Effects / Documents | Effect_AuthoringDocument, Codec, Validator, SourceCatalog | schema·identity·parse/validation의 runtime 계약 |
| Presentation / Effects / Runtime | evaluator, reconstructed runtime, component/object 재생 | 시간·simulation·수명 |
| Presentation / Effects / Renderer | Effect_DocumentRenderer* 및 내부 header | stage·material binding·geometry 제출 |
| Presentation / Effects / Materials | Effect_MaterialTemplate, MaterialProgramRegistry, family Material classes | program/layout·typed material ABI |
| Presentation / Rendering / Profiles | RenderingProfileService와 runtime profile 자료형 | 프레임별 실효값 적용과 authoring service |
| Presentation / Rendering / SourceMaterials / Map | SourceMapWaterMaterialParameters, SourceMap* parameter 변환 | source map 물·표면 program lane |
| Tools / Effect / Editing, Playback, Resources, IO | Effect_Tool_* | 실제 ImGui 저작·선택·저장 조작 |
| Tools / Sequencer / Action, Composition, Animation | CharacterActionWorkbench, Valtan/Kouku ActionWorkbench, EffectCompositionWorkbench, BoneAnimationWorkbench | 사용자 저작 창/세션 종류 |
| Tools / Rendering | RenderingBenchmark, RenderingTechniqueGuide, RenderingReferenceGuide | 임시 실험·기법 설명 UI |
| Tools / Profiler | ProfilerTool, ProfilerCaptureIO | Debug 시각화·capture 저장/비교 |
| ShaderFiles / Deferred, Map, Character, Effect, Post | 실제 shader include/compile item | shader 프로그램의 입력·출력 영역별 탐색 |

Engine의 Profiler/Renderer/Material은 기존 공용 실행 계층에 유지했다. Engine의 `03.ShaderFiles`는 Common/Deferred/SourceCharacter/SourceMap/Geometry 하위로 shader 98개를 분류하고, PCH 2항목은 `98.Default`로 분류했다. 이 변경은 Include·item type·빌드 metadata를 유지하는 탐색 분류다. `Data`는 현재 팀 계약대로 Client `96.DataFiles`의 `None` item만 유지한다. source material 이름에 Character가 있더라도 source water를 포함한 공통 adapter의 역사적 이름이므로 용도 분류와 심볼 이름을 혼동하지 않는다.

Effect native shader에도 같은 주의가 필요하다. [Shader_EffectArtistNativeSelectedGroup3968.hlsli:184](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_EffectArtistNativeSelectedGroup3968.hlsli:184)는 실제로 `KoukuNativeGroup3968`을 include한다. SelectedGroup 51개를 조사하면 Artist 6개, Kouku 44개, World+Kouku 혼합 1개다. 혼합인 [SelectedGroup2304:182](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_EffectArtistNativeSelectedGroup2304.hlsli:182)는 WorldNative와 Kouku2304를 함께 묶는다. `ArtistNativePrograms`는 Artist/Vehicle/World/Kouku 프로그램과 dispatch를 모으고, `ArtistNativeInputs/Common`은 공유 입력·자료구조·sampling ABI다. 역사적 `Artist` 접두어만으로 도화가 전용 필터에 배치하지 않는다. 검토용 [basename→family 사전](C:/Users/tnest/Desktop/LostArk/out/CodeAtlas/client-shader-filter-overrides.json)과 [실제 include 증거](C:/Users/tnest/Desktop/LostArk/out/CodeAtlas/client-shader-filter-evidence.json)에 118개 override를 기록했다.

## G09. 후속 영상·기술 문서에 필요한 확인 단위

1. 장면 하나의 물 배치 stable ID를 고정하고 material family/program/texture lane/constant/sample된 scene input을 끝까지 따라간다.
2. 스킬 하나의 실제 quick-slot → skill binding → cue → EffectAssetId → stable element → evaluated frame → render adapter → HLSL → RT를 연결한다. 프로그램 목록의 개수와 source-exact 승인 개수를 같은 숫자로 소개하지 않는다.
3. Tools 설명은 선택/편집/preview/save/publish/runtime activation을 각각 다른 이벤트로 그린다. disk/immutable resource/active occurrence의 세 상태를 별도 표시한다.
4. Profiler 캡처는 scene·camera·viewport·quality·capture detail 조건을 기록한다. CPU self/worker/GPU pending/draw 작업량/메모리 상태를 분리한다.
5. Unreal 비교는 본 repository의 cooked UE3 복원 evidence, 로컬 Unreal5 source, LostArk 프로젝트의 현재 구현을 구분해 쓴다. 별도 WintersEngine 저장소와도 실제 소스를 비교하며, 현재 프로젝트를 그 저장소 자체로 부르지 않는다. UE5 내부가 곧 Lost Ark 원본 구현이라고 서술하지 않는다.

이 조사의 결과만으로 새로운 visual PASS나 게임 FPS 향상을 선언하지 않는다. 코드 연결, 기존 RESULT의 실행 증거, 사용자가 직접 확인할 실제 화면을 분리해 포트폴리오의 문장을 작성한다.

## G10. UE 5.8.3 실제 소스로 연결한 렌더링·Profiler 비교

비교 대상은 `C:/Users/tnest/Desktop/UnrealEngine`의 source checkout이다. [Build.version:2](C:/Users/tnest/Desktop/UnrealEngine/Engine/Build/Build.version:2)의 버전은 5.8.3이며 조사한 Git HEAD는 `396c9f059903aed5fec78ecd3d437a40c6415368`이다. 아래 링크는 이 PC의 해당 checkout 기준이다. 다른 PC에서는 같은 버전과 commit을 먼저 맞추고 심볼로 다시 찾는다. Launcher 설치본·소스 빌드 성공·실행 중인 Editor 버전을 이 파일의 존재만으로 확인한 것은 아니다.

이번 G는 기존 LostArk 구현을 설명하기 위한 소스 비교다. C++/HLSL이나 데이터 계약을 추가하는 구현 PLAN이 아니며, 기존 G01~G09의 설명과 검증 경계를 유지한다. UE와 LostArk의 UI 실행, 실제 GPU 캡처, 동일 장면 성능 비교는 수행하지 않았다. 아래에서 말하는 차이는 코드가 소유하는 책임과 관측 범위이며 성능 우열이나 화면 품질 판정이 아니다.

### G10-01. DeferredShadingRenderer.h / cpp — 프레임을 구성하는 owner

[DeferredShadingRenderer.h:260](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Renderer/Private/DeferredShadingRenderer.h:260)의 `FDeferredShadingSceneRenderer`는 `FSceneRenderer`를 상속하고 [377행](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Renderer/Private/DeferredShadingRenderer.h:377)에 `Render(FRDGBuilder&, const FSceneRenderUpdateInputs*)`를 선언한다. CPP의 [Render:1823](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Renderer/Private/DeferredShadingRenderer.cpp:1823)는 view의 상태를 scene에 연결하고 기능 조건을 판정하며 전달받은 graph builder로 렌더링 작업을 구성한다. `Views`는 각 시점의 렌더링 입력이고 `ViewState`는 프레임 사이에 이어지는 view 상태다.

LostArk의 대응 진입점은 [CRenderer::Draw:865](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:865)다. G01의 순서대로 queue와 render target을 소비하고 각 pass를 직접 호출한다. 비교 설명은 “둘 다 한 프레임의 렌더링을 조율하지만 pass와 자원 의존성을 표현하는 방법이 다르다”로 시작한다. 함수 길이 또는 pass 개수만으로 엔진의 성능을 비교하지 않는다.

### G10-02. RenderGraphBuilder.h / cpp — pass parameter에서 자원 사용을 계산

[RenderGraphBuilder.h:45](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/RenderCore/Public/RenderGraphBuilder.h:45)는 RDG parameter로부터 barrier와 lifetime을 도출하는 계약을 설명한다. [AddPass:221](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/RenderCore/Public/RenderGraphBuilder.h:221)는 pass 이름, parameter 구조체, `ERDGPassFlags`, 실행 lambda를 받는다. parameter는 shader 값만 담는 꾸러미가 아니라 그 pass가 읽고 쓰는 자원을 graph가 알아내는 연결점이다.

[FRDGBuilder::Compile:1327](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/RenderCore/Private/RenderGraphBuilder.cpp:1327)는 pass dependency, 참조 수와 culling을 처리하고, [Execute:1766](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/RenderCore/Private/RenderGraphBuilder.cpp:1766)는 graph 실행과 자원 정리를 조직한다. `Passes`, texture/buffer reference count, async setup task는 실행 순서와 자원 사용의 자료구조다. LostArk는 [Begin_MRT 호출:954](C:/Users/tnest/Desktop/LostArk/Engine/Private/Renderer.cpp:954), 대응 End와 SRV/copy 순서를 각 함수에서 명시한다. 현재 renderer를 RDG 구현이라고 소개하지 않는다. 후속 확장 대상으로 pass의 읽기·쓰기 선언, lifetime 검증과 사용하지 않는 pass 제거를 구체적으로 설명할 수 있다.

### G10-03. DynamicRHI.h / cpp — 플랫폼 GPU API를 감싸는 경계

[DynamicRHI.h:198](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/RHI/Public/DynamicRHI.h:198)의 `FDynamicRHI`는 backend 경계다. [RHIInit:291](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/RHI/Private/DynamicRHI.cpp:291)는 플랫폼 shader 정보를 초기화하고 `PlatformCreateDynamicRHI`로 구현을 선택한 뒤 `GDynamicRHI->Init()`을 호출한다. 따라서 renderer의 pass 구성과 플랫폼 device 초기화가 별도 책임으로 나뉜다.

LostArk의 [CGraphic_Device::Initialize:17](C:/Users/tnest/Desktop/LostArk/Engine/Private/Graphic_Device.cpp:17)는 [D3D11CreateDevice:39](C:/Users/tnest/Desktop/LostArk/Engine/Private/Graphic_Device.cpp:39)를 직접 호출하고 device/context를 나머지 렌더링 경로에 전달한다. 현재 D3D11 상태·resource 바인딩을 이해한 성과와 여러 GPU API를 포괄하는 RHI abstraction을 만든 성과를 구분한다.

### G10-04. MaterialShared.h / cpp — 재질 표현을 shader map으로 변환

[MaterialShared.h:3151](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Public/MaterialShared.h:3151)의 `FMaterial::BeginCompileShaderMap`은 shader map ID, static parameter set, precompile mode와 target platform을 받는다. [CPP:3714](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/Materials/MaterialShared.cpp:3714)의 흐름은 새 `FMaterialShaderMap` 생성 → `Translate` → uniform/compiler environment 구성 → `Compile`이다. shader map ID는 컴파일할 변형의 identity이고 target/quality/static parameter는 어떤 GPU 프로그램이 필요한지를 결정한다. [Material.cpp:2618](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/Materials/Material.cpp:2618)의 cooked build 경로는 새 컴파일 대신 이미 로드한 shader map을 등록한다.

LostArk의 [CShader::Initialize_Prototype:210](C:/Users/tnest/Desktop/LostArk/Engine/Private/Shader.cpp:210)는 사전 CSO를 읽어 FX11 effect를 만들고, [CMaterial::Bind_SourceCharacterInputs:948](C:/Users/tnest/Desktop/LostArk/Engine/Private/Material.cpp:948)는 기존 program ABI에 typed 입력을 바인딩한다. source family별 식과 입력 복원을 범용 재질 그래프를 HLSL로 번역하는 compiler와 동일하게 부르지 않는다.

### G10-05. ShaderCompiler.cpp — 컴파일 요청과 worker 결과의 수명

[FShaderCompilingManager::SubmitJobs:1519](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/ShaderCompiler/ShaderCompiler.cpp:1519)는 shader compile job 묶음을 제출한다. [LaunchWorker:1671](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/ShaderCompiler/ShaderCompiler.cpp:1671)는 작업 디렉터리와 입력·출력 파일을 받아 worker process를 만들고, [ProcessAsyncResults:2865](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/ShaderCompiler/ShaderCompiler.cpp:2865)는 완료된 결과를 처리한다. 컴파일 요청, 실행 중인 worker, material이 사용할 결과는 같은 상태가 아니다.

LostArk의 shader build와 런타임 CSO 로딩도 분리해서 보여준다. G02의 `CShader`는 에디터 재질 변경을 받아 worker compile job과 shader map 교체를 모두 소유하는 객체가 아니다. 이미 준비된 CSO를 선택하고 FX11 변수·pass를 연결하는 구현을 설명하는 것이 정확하다.

### G10-06. LumenScreenProbeGather.cpp — 화면 ray만으로 설명할 수 없는 GI 입력

[RenderLumenScreenProbeGather:2169](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Renderer/Private/Lumen/LumenScreenProbeGather.cpp:2169)는 `FPreviousViewInfo`, `FLumenMeshSDFGridParameters`, radiance-cache interpolation parameter 등을 받는다. 이 함수 안에서 importance sampling ray 생성:2624 → trace용 RDG texture/UAV 준비:2637 → `TraceScreenProbes`:2647 → `FilterScreenProbes`:2661 → `UpdateHistoryScreenProbeGather`:2734로 이어진다. previous-view/history는 프레임 간 상태이고 radiance cache/SDF는 현재 화면 color/depth와 별도인 입력이다. 설정과 경로에 따른 사용 여부를 구분한다.

LostArk의 [PS_SSGI:146](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_ScreenSpaceLighting.hlsl:146)는 MapPBR receiver에 한해 현재 opaque radiance와 depth를 추적하는 가산 조명이다. G05에서 확인한 4/8/16 ray와 8-step 검사, offscreen·history·temporal denoising 부재를 함께 제시한다. “Lumen 완성” 대신 “화면 공간 GI를 구현하고 화면 의존성과 샘플 수의 비용을 분석했다”는 설명이 현재 코드에 맞는다.

### G10-07. ScreenSpaceRayTracing.cpp — SSR의 품질·이전 프레임·후속 처리

[RenderScreenSpaceReflections:1018](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Renderer/Private/ScreenSpaceRayTracing.cpp:1018)는 `FRDGBuilder`, scene texture, view, SSR quality, denoiser 입력 출력, SingleLayerWater 여부를 받는다. [1032행](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Renderer/Private/ScreenSpaceRayTracing.cpp:1032)부터 previous custom SSR input 또는 half-resolution temporal history를 쓰는 조건 분기가 있다. [IsSSRTemporalPassRequired:213](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Renderer/Private/ScreenSpaceRayTracing.cpp:213)는 AA 방식과 설정에 따라 temporal pass 필요 여부를 판정한다. 모든 설정에서 같은 후속 처리가 실행된다는 뜻은 아니다.

LostArk의 [PS_SSR:180](C:/Users/tnest/Desktop/LostArk/Client/Bin/ShaderFiles/Shader_ScreenSpaceLighting.hlsl:180)는 fixed-step trace 결과에 Fresnel, roughness, 화면 경계 감쇠를 곱한다. 현재 경로는 opaque MapPBR용이므로 물·스킬의 투명 표면 전체 반사 또는 temporal SSR과 구분한다. 화면 바깥으로 반사 대상을 이동하는 시연은 이 제한을 설명하는 관측 항목이며, 이번 조사에서 결과를 촬영한 것은 아니다.

### G10-08. EditorViewportClient.cpp / Scalability.cpp — 표시 실험과 저장 상태

[FEditorViewportClient::SetViewMode:6466](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Editor/UnrealEd/Private/EditorViewportClient.cpp:6466)는 view mode parameter를 초기화하고 perspective/orthographic mode에 맞춰 `ApplyViewMode`와 show flags를 갱신한 뒤 viewport를 invalidate한다. [Scalability::SaveState:1218](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Engine/Private/Scalability.cpp:1218)는 임시 quality 상태이면 backup quality를 선택하고 `ScalabilityGroups`의 해상도·AA·shadow·GI·reflection 등을 INI에 기록한다. view mode 전환, quality 변경과 설정 저장은 서로 다른 함수의 책임이다.

LostArk는 G06의 [Set_ExperimentPreview:720](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingProfileService.cpp:720)와 [Save_Authored:2238](C:/Users/tnest/Desktop/LostArk/Client/Private/RenderingProfileService.cpp:2238)를 비교 진입점으로 삼는다. Workbench의 [Save Authored / Publish Runtime 버튼:15435](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:15435), 세션 overlay, benchmark capture JSON을 별도로 보여준다. UE editor viewport/scalability 전체와 동일한 도구라고 부르기보다 실험값의 수명과 저장 owner를 직접 설계한 부분을 설명한다.

### G10-09. CpuProfilerTrace.h / cpp — 이벤트를 계측 stream으로 내보내기

[CpuProfilerTrace.h:107](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Core/Public/ProfilingDebugging/CpuProfilerTrace.h:107)의 `OutputBeginEvent(uint32 SpecId)`는 등록된 event specification ID를 받는다. [CPP:247](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/Core/Private/ProfilingDebugging/CpuProfilerTrace.cpp:247)는 begin event를 기록하고 `EventBatchV3` 등의 trace schema로 수집한다. [FCpuProfilerAnalyzer::OnAnalysisBegin:57](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Developer/TraceServices/Private/Analyzers/CpuProfilerTraceAnalysis.cpp:57)는 event 종류를 analyzer에 route하며 이후 thread timeline의 begin/end로 복원한다.

LostArk의 [CProfiler::Begin_Scope:294](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:294)와 `End_Scope`:316은 이름 ID, thread, 중첩 깊이와 QPC 시간을 frame sample로 모은다. 자체 CPU 계측과 Self 계산은 구현된 비교 대상이다. trace stream 분석 계층까지 모두 가진 것으로 확장해 설명하지 않는다. worker 시간과 main-thread 시간을 합쳐 frame elapsed처럼 표시하지 않는 이유도 G07의 규칙과 함께 설명한다.

### G10-10. GpuProfilerTrace.h / cpp — queue 시간과 fence 관계

[GpuProfilerTrace.h:77](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/RHI/Public/GpuProfilerTrace.h:77)의 `FGpuProfilerTrace`는 queue별 event 출력을 선언한다. [BeginWork:134](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Runtime/RHI/Private/GpuProfilerTrace.cpp:134)는 `QueueId`, GPU timestamp와 CPU timestamp를 기록하고, `TraceWait`:149와 `SignalFence`:165, `WaitFence`:173은 대기 구간과 queue 사이 fence 관계를 전달한다. 이 자료는 단순 pass duration보다 더 넓은 실행 관계를 분석할 근거다.

LostArk의 [Resolve_GpuFrames:1176](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:1176)는 D3D11 timestamp/disjoint query를 `DONOTFLUSH`로 읽어 원래 frame에 귀속한다. 제출 counter와 elapsed 측정은 제공하지만 현재 profiler에 UE의 queue/fence 관계 trace를 대응시키지 않는다. GPU pending, disjoint, 누락을 0ms와 구분하는 현재 구현부터 보여준다.

### G10-11. AnalysisService.cpp / TimingProfiler.h·cpp — 수집·분석·UI 분리

[FAnalysisService::StartAnalysis:304](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Developer/TraceServices/Private/AnalysisService.cpp:304)는 file stream을 열고 analysis session과 thread/frame/counter provider를 구성한다. [TimingProfiler.h:107](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Developer/TraceServices/Public/TraceServices/Model/TimingProfiler.h:107)의 `FCreateAggregationParams`는 집계 조건을 전달하고, [FTimingProfilerProvider::CreateAggregation:893](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Developer/TraceServices/Private/Model/TimingProfiler.cpp:893)은 선택한 timeline과 구간을 집계한다. UI는 [FTimingProfilerManager::SpawnTab:215](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Developer/TraceInsights/Private/Insights/TimingProfiler/TimingProfilerManager.cpp:215)에서 별도 Slate window로 생성된다.

LostArk는 [CProfilerTool::Render:799](C:/Users/tnest/Desktop/LostArk/Client/Private/ProfilerTool.cpp:799)와 [ProfilerCaptureIO의 비동기 exporter:797](C:/Users/tnest/Desktop/LostArk/Client/Private/ProfilerCaptureIO.cpp:797)를 연결한다. 게임 내 ImGui 표시, 유한 history, JSON 저장/reader를 구현했다. Unreal Insights와의 비교는 “같은 숫자를 보여주는 창”에 그치지 않고 수집 자료구조 → 저장 → 분석 → 시각화의 각 owner를 대조한다. 저장 실패·이름 충돌·캡처 범위 표기까지 현재 구현의 증거로 삼는다.

### G10-12. AllocationsAnalysis.cpp — 메모리 표본과 할당 수명은 다른 관측

[FAllocationsAnalyzer::OnAnalysisBegin:46](C:/Users/tnest/Desktop/UnrealEngine/Engine/Source/Developer/TraceServices/Private/Analyzers/AllocationsAnalysis.cpp:46)은 Memory의 Alloc/Free/Realloc 및 system/video 이벤트를 analyzer로 route한다. 이러한 이벤트 기반 분석은 할당과 해제의 관계를 보존할 수 있는 입력 계약이다. 필요한 trace channel과 생산자가 실제 실행에서 활성화됐는지는 별도 캡처로 확인해야 한다.

LostArk의 [CProfiler::Sample_Memory:214](C:/Users/tnest/Desktop/LostArk/Engine/Private/Profiler.cpp:214)는 약 1Hz의 process/system/DXGI 사용량과 budget 표본이다. 이를 texture별 소유량, 할당별 lifetime, 누수 원인의 callstack 분석으로 소개하지 않는다. G07의 texture cache request/hit/new-SRV counter도 관측한 생산자의 범위와 누적값 의미를 함께 표시한다.

## G11. 비교 영상·기술소개서에서 사용자가 직접 확인할 장면

아래는 다음 촬영의 확인 항목이며 이번 조사에서 완료된 실행 결과가 아니다. Client와 Editor의 실행·조작은 사용자가 수행한다. 기존 팀 rendering 저장값은 유지하고 허용된 세션 실험을 사용한다.

| 시연 순서 | 화면에서 확인할 내용 | 함께 보여줄 코드·저장 증거 |
|---|---|---|
| 한 프레임 추적 | LostArk의 동일 장면에서 주요 pass와 GPU pending/valid 상태를 확인 | G01의 `Draw` 순서, G10-01~02의 UE renderer/RDG owner |
| Rendering Workbench A/B | camera·viewport·quality·capture detail 조건을 고정하고 한 변수만 변경. 준비 frame과 비교 제외 조건을 기록 | 세션 overlay와 복구, benchmark fingerprint·JSON. gameplay 결정적 replay 검증으로 확대하지 않기 |
| SSGI/SSR 한계 | 반사 대상이 화면 밖으로 나가는 경우, camera 이동, 얇은 표면·화면 경계에서 결과 관찰 | `TraceScreen`의 fixed step과 receiver 조건. UE의 history/SDF/cache 입력과 구조 비교 |
| 재질 입력 추적 | 물 또는 스킬 하나를 골라 material family/program/texture/constant와 최종 pixel을 연결 | `CMaterial`/`CShader` 바인딩과 UE의 Translate→shader map compile 단계 차이 |
| Profiler capture | CPU inclusive/self, GPU elapsed, draw 작업량을 같은 의미의 항목끼리 읽고 이름 있는 capture 저장 | CaptureWindow와 pending/누락 표기, JSON schema·원자 저장. Insights는 실제 trace를 열어 해당 channel 수집 여부부터 확인 |
| 메모리 읽기 | process/VRAM 사용량 변화와 valid flag를 관찰 | 1Hz 표본의 한계. UE allocation trace를 사용할 때는 allocation/free 이벤트 수집 조건을 별도로 기록 |

두 엔진의 ms를 직접 비교하려면 scene·geometry/material·해상도·quality·GPU/driver·빌드 구성·계측 상태가 동등한지 먼저 기록한다. 현재 두 프로젝트에서 같은 장면을 같은 작업량으로 렌더링했다는 증거는 없다. 이번 기술소개서에는 구현 원리와 확인 가능한 범위의 차이를 쓰고, FPS 향상·UE 대비 성능 우열·시각적 parity는 실제 측정과 사용자 화면 확인 뒤에만 추가한다.
