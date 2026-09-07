# Character Select·쿠크 바닥 재질 복구 구현 계획

작성일: 2026-09-07

상태: 사용자 구현 요청에 따른 G01~G04 연결·배포·자동 검증을 마쳤다. G05 사용자 화면 확인과 남은 경계는 대응
[FLOOR_MATERIAL_RECOVERY_RESULT](2026-09-07_FLOOR_MATERIAL_RECOVERY_RESULT.md)에 기록한다.
이 문서는 적용 범위와 연결 원칙을 소유한다. 2026-09-08 사용자 원본·모작·줌 비교 요청과
원본에 가깝게 복구하자는 지시에 따라 G06의 Character Select 범위를 다시 구체화했다.
현재는 원본 배치/PBR/입력 조사와 복구 방안까지 진행했으며, G06 제품 반영·화면 검증 완료가 아니다.

## G00. 첫 목표와 비교 기준

첫 목표는 **쿠크의 실제 바닥 두 재질에서 원본 입력이 픽셀 계산까지 전달되게 하고,
무슨 입력 때문에 화면이 바뀌는지 사용자가 따로 확인할 수 있게 만드는 것**이다.

쉽게 말하면, 현재는 바닥 그림 파일을 읽어도 표면의 반짝임을 회색 한 값으로 줄이고,
원본 반사 입력은 일반 바닥 계산에서 사용하지 않는다. Rendering Tool의 밝기나 채도를
움직여도 이 빠진 계산이 생기지는 않는다. 먼저 입력과 계산을 연결해야 조절할 대상이 생긴다.
이 결함들이 전체 탈색감에서 차지하는 비율은 아직 실제 장면으로 측정하지 않았다.

진행 순서는 다음으로 고정한다.

1. 다른 구현 세션의 변경을 포함한 실행 기준을 확정하고 같은 위치·카메라·조명·후처리를 유지한다.
2. 쿠크의 아래·위 바닥을 각각 실제 원본 material과 연결한다.
3. 원본에서 켜진 기능과 해당 계산을 확인해 기존 Material 경로로 운반한다.
4. 고유색·normal·specular·환경 반사 기여를 분리하고 현재 계산/복구 계산을 비교한다.
5. 쿠크의 변화와 기존 Character Select 표현 보존을 사용자가 확인한다.
6. Character Select의 원본 재질을 별도로 확보해 같은 작업 방식을 적용한다.
7. 그 뒤 장면 조명·후처리와 이펙트를 조정한다.

이 문서의 코드 기준은 `codex/kouku-gate-pattern-bundles`,
HEAD `13fa34d7df9f37d1c697ddefe1e5e7aebfdafef9`의 **dirty working copy**다.
다른 세션에서 MapTool, MainApp, RenderingProfileService, map publisher, 조명·RenderingProfiles
등을 수정 중이다. 구현자는 작업 시작 시 최신 diff와 실제 호출 경계를 다시 대조한다.
현재 숫자로 프로필 전체를 되돌리거나 이 문서의 기준점을 실행 정본으로 강제하지 않는다.

완료 증거는 세 가지로 구분한다. 입력 연결은 데이터·바인딩 검증, 계산 실행은 shader·GPU 수치 검증,
원본에 가까워졌는지는 사용자의 실제 장면 비교로 판단한다.

## G01. 바닥 identity와 원본에서 확인한 내용

### G01-1. 쿠크의 두 바닥

조사 기준의 `g1.saydon`과 `g3.saydon`은 `(-0.07, 1.32, 942.33)`을 사용한다.
별도 `g1.kouku` 배치 `(22, -0.05, -62)`까지 같은 바닥이라고 가정하지 않는다.
다른 세션에서 관문 배치가 변경되면 아래 stable placement와 실제 진입 위치를 다시 연결한다.

| 대상 | Stable asset ID | Placement ID | 실제 배치 override의 원본 material |
|---|---|---|---|
| 아래 바닥 | `MAP_61FA80B72343_BG_RAD_KOUKUSATON_FLOOR08_SM_OVR_04DCDAA75173` | `12451899878577673092` | `lv_lut_midnightc.mat.bg_rad_koukusaton_floor09b_mi` |
| 위 바닥 | `MAP_0BE6EBEE4924_BG_RAD_KOUKUSATON_FLOOR08A_SM_OVR_84D65232B541` | `10472891450652540488` | `lv_lut_midnightc.mat.bg_rad_koukusaton_floor08b_mi` |

현재 WModel 안의 material 이름은 각각 `SLOT_000_bg_rad_koukusaton_floor08_mi`,
`SLOT_000_bg_rad_koukusaton_floor08a_mi`다. 저작 데이터의 runtime material join은 이 실제 이름을
사용하고, 원본 provenance에는 위 배치 override의 이름을 기록한다. 둘을 혼동하지 않는다.

| 원본 입력 | 아래 floor09b | 위 floor08b | 현재 제품 입력 |
|---|---:|---:|---|
| Parent family | `bg_seamless-specular_msk` | `bg_base_msk` | 일반 map opaque 계산 |
| Diffuse brightness | 0.6 | 0.7 | 원본 scalar 전달 없음 |
| Normal intensity | 0.5 | 0.5 | 원본 scalar 전달 없음 |
| Specular intensity | 0.2 | 0.2 | 공통 profile 1 |
| Specular power | 30 | 100 | 공통 profile 50 |
| Reflection intensity | 0.2 | 0.2 | 일반 opaque 반사 입력 미소비 |
| Normal / reflection / specular 기능 | 켜짐 | 켜짐 | 원본 기능별 선택 미전달 |
| Specular texture 기능 | **켜짐** | **꺼짐** | 설치된 S 슬롯 유무로 선택 |
| Diffuse saturation 기능 | 꺼짐 | 켜짐 | 원본 분기 미전달 |
| Bump / emissive 기능 | 꺼짐 | 꺼짐 | 파일 존재와 활성 기능을 구분해야 함 |

Static switch는 원본 MIC native 데이터에서 읽은 값이다. 텍스처 이름만으로 추정한 설정이 아니다.
위 바닥은 opacity texture 기능도 꺼져 있다. 설치된 O/E/S 파일을 모두 활성화하는 방식은
원본 복구와 다르다. 두 재질의 실제 mask는 Diffuse.A < 0.3333이면 discard다. BasePass와 LightPass에서 확인했다.

아래 바닥은 `S.rgb * specularColor.rgb * specularIntensity`를 사용한다. 위 바닥은 S texture를
끄고 **밝기·채도 조정 전 D.rgb**를 같은 specular 입력으로 사용한다. 일반 shader가 RGB를
휘도 한 값으로 줄이는 손실을 여섯 번째 MRT로 제거한다.

Normal.A는 **반사 강도의 mask**이며 roughness가 아니다. Normal.RG에서 Z를 먼저 복원한 뒤
XY에 normalIntensity를 곱해 정규화한다. ORM과 Normal.B는 이번 두 재질의 표면식에서 사용하지 않는다.
D/S/reflection RGB는 원본 Texture 기본값을 따라 sRGB, Normal은 명시된 linear다.

근거: [쿠크 대응 자료](/C:/Users/user/Desktop/LostArk/out/RenderingAudit20260907/floor_kouku_mapping.json),
[확정된 원본 식과 binding](/C:/Users/user/Desktop/LostArk/out/FloorMaterialRestore20260907/source_equations.md).

### G01-2. 구현 전 조사에서 확정한 식과 남은 경계

legacy normal row 29 bytes와 numbered FName을 보존한 decoder로 두 MIC에 대해 원본 cache의
material map을 각각 하나로 특정했다. LocalVertexFactory의 BasePass/DirectionalLight PS와
공유 VS를 읽었다. 선택된 pass의 식이며 원본 실장면의 lightmap pass 선택까지 증명하지는 않는다.

```text
rawXY = N.rg * 2 - 1
rawZ = sqrt(max(1 - dot(rawXY, rawXY), 0)) + 0.00001
Nt = normalize(float3(rawXY * normalIntensity * vertexAlphaFactor, rawZ))

09b baseDiffuse = D.rgb * diffuseColor.rgb * diffuseBrightness
08b baseDiffuse = lerp(dot(D.rgb, (0.3,0.59,0.11)), D.rgb, diffuseSaturation)
                 * diffuseColor.rgb * diffuseBrightness
q = R.rgb * reflectionColor.rgb * (1 + reflectionContrast) - reflectionContrast
w = N.a * reflectionIntensity
surfaceDiffuse = clamp((baseDiffuse + w*saturate(q)) * (1 - w*saturate(-q)), 0, 999)
```

반사는 Emissive 가산이 아니라 조명 전 Diffuse 변경이다. reflection debug는 이 변경의 절댓값이다.
08b reflectionUV는 `lerp(meshUV, (reflect(-Vt,Nt).xy+0.5)*0.5, 0.75)`다.
09b는 source-world reflection.xy*tiling에 CB0 엔진 좌표.xy*0.0003을 더한다. 그 엔진 좌표의
정체는 아직 확인되지 않았다. 이번 구현은 **해당 offset을 0으로 둔 근사**임을 표시한다.
임의로 Actor/Camera position을 원본 정답이라고 정하지 않는다.

08b의 vertexAlphaFactor는 원본 COLOR0.a다. 해당 추출 mesh에는 COLOR0가 없어서 기본값 1을
사용한다. native 배치의 override-color 기본 의미는 별도 미확정 경계로 남긴다.
09b는 vertexAlphaFactor=1이다. 두 설치 WModel은 tangent.w가 모두 +1로 손실되어 있으므로
기존 geometry cooker로 source glTF의 handedness를 보존한다. material section, 정점 position/N/UV/T.xyz,
index 순서와 실제 bounds/scale은 보존하고, 기존 embedded bounds의 1/100 단위 오류만 교정한다.
정상 복구된 TBN은 추가 binormal 반전 없이 사용한다. 기존 미선택 shader 계산은 유지한다.

현재 deferred Phong 광원식, 직접·간접 조명, 원본 lightmap 및 후처리는 별도의 근사 경계다.
이번 완료 범위는 표면 입력·확인된 식·RGB 저장·실제 draw와 light 소비·진단 연결이다.
게임 전체 shader/ABI 또는 원본 화면 일치 완료로 표현하지 않는다.

## G02. 데이터가 실제 Material에 도착하는 경로

선택할 구조는 **Area별 재질 저작 JSON → 기존 map publisher/catalog → CModel → CMaterial**이다.
WMA2/3 바이너리 변경이나 모델 전체 재변환을 첫 작업에 요구하지 않는다.

```text
Data/Maps/Authoring/<AreaId>/<AreaId>.mapmaterials.json
  → Publish-MapAuthoring의 검증·staging·교체
  → CMapAssetCatalog의 Area별 재질 입력
  → Loader / MapTool의 MODEL_ASSET_LOAD_DESC
  → CModel::Ready_BinaryModel에서 material 이름으로 합성
  → CMaterial이 texture와 표면 파라미터 소유
  → MapAssetRenderUtils가 static / instanced 양쪽 shader에 전달
```

JSON의 저장 key는 `assetId + materialName`이다. 배치별 pointer나 vector index로 저장하지 않는다.
원본 material path는 provenance이며 설치 WModel의 material 이름을 대신하는 lookup key가 아니다.
원본 parent default·child override·활성 switch를 합성한 값 중 실제 shader 소비자가 있는 항목만
runtime 계약에 추가한다. 나중에 쓸 미소비 필드를 미리 채우지 않는다.

Area의 재질 문서 선언은 기존 water layer처럼 `Data/Maps/MapCatalog.json`의
`sourceMaterials`와 `materials` 경로 쌍이 소유하도록 확장한다.
publisher는 선언 없는 저작 파일과 선언된 파일의 누락을 모두 거부한다.
필수 재질 문서 참조도 기존 runtime map catalog에 운반하여, runtime에서 파일이 사라진 경우를
미선언 Area로 오인하지 않게 한다. 해당 catalog reader/writer의 버전 호환과 MapTool의 source
로드도 같은 변경 단위에 포함한다. 별도의 범용 manifest를 추가하지 않는다.

| 변경할 파일·경계 | 새로 소유할 책임 |
|---|---|
| `Data/Maps/MapCatalog.json` | Area별 source/runtime 재질 문서 경로 선언 |
| `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.mapmaterials.json` 제안 | 대상 두 asset/material의 선택적 입력과 원본 family 구분 |
| `Tools/MapPipeline/Publish-MapAuthoring.ps1` | 선언·catalog 참조·새 문서를 함께 검증하고 기존 staging으로 runtime 교체 |
| `Client/Public/MapAssetCatalog.h`, `Client/Private/MapAssetCatalog.cpp` | catalog asset 참조 검증·임시 상태 구성·전체 성공 후 commit |
| `Engine/Public/BinaryAsset/ModelAssetData.h` | named material override, 실제 소비하는 표면 입력과 reflection 경로의 로드 계약 |
| `Engine/Public/Model.h`, `Engine/Private/Model.cpp` | decode 뒤 material 생성 전에 이름 일치 검증·입력 합성, mesh가 쓰는 재질 읽기 |
| `Engine/Public/Material.h`, `Engine/Private/Material.cpp` | 생성 때 확정된 표면 파라미터와 reflection SRV 수명 소유 |
| `Client/Private/Loader.cpp`의 `Ready_MapArea` | scoped map model 생성 시 descriptor로 해당 material 입력 전달 |
| `Client/Private/MapTool.cpp`의 `Admit_AuthoringPrototype` | 같은 catalog/descriptor를 사용해 제품과 같은 material 생성 |
| `Client/Public/MapAssetRenderUtils.h`, `Client/Private/MapAssetRenderUtils.cpp` | 실제 mesh material을 조회하여 draw마다 모든 입력과 기본값 바인딩 |

`CModel` clone은 `m_Materials`를 공유한다. 특정 placement를 조절하면서 공유 `CMaterial`을
나중에 변경하지 않는다. 배치마다 다른 원본 재질은 현재처럼 catalog asset variant로 구분한다.
MapTool의 debris·Deploy 모델 생성 경로는 이 일반 바닥 작업의 변경 대상이 아니다.

미선언 Area는 기존 동작을 사용한다. 선언된 문서의 중복 key·미등록 asset/material·잘못된 경로·
비정상 숫자·지원하지 않는 family는 오류 이유를 보존하고 stage를 취소한다.
필수 reflection 파일 누락을 기능 비활성화로 숨기지 않는다. 모델 단계에서 드러난 오류도
Loader/MapTool의 기존 실패 소비자까지 전달하여 기존 화면·정상 저작 상태를 보존한다.

재질 문서는 authoring 아래 저장하고 runtime 문서는 publisher만 교체한다.
새 C++ 파일은 첫 설계에 필요하지 않다. 새 JSON은 Client 프로젝트 `96.DataFiles`의 `None`
항목과 대응 filter 등록을 검토한다. 추가 header/HLSL include를 분리할 경우 실제 producer project와
filters 등록을 함께 처리하며 기존 물리 구조를 재배치하지 않는다.
이 optional layer가 실제 구현되면 팀 Area 데이터 가이드에도 지원 범위와 publish 경로를 갱신한다.

Reflection texture는 두 모델의 물리 `textures` 폴더에 이미 있다.
Resources-relative 위치는 각각 다음과 같다.

```text
Map/LV_LUT_MIDNIGHTC_ED/MAP_61FA80B72343_BG_RAD_KOUKUSATON_FLOOR08_SM_OVR_04DCDAA75173/textures/d89521db7863_ambientreflection_15.dds
Map/LV_LUT_MIDNIGHTC_ED/MAP_0BE6EBEE4924_BG_RAD_KOUKUSATON_FLOOR08A_SM_OVR_84D65232B541/textures/d89521db7863_ambientreflection_15.dds
```

구현 시 물리 위치와 loader의 경로 해석을 재검증한다. 신규 Resource 팩 생성·Drive 재배포·binary
Git 추가를 이 작업의 선행 조건으로 만들지 않는다. 이번 작업에서 Drive 전달 작업은 하지 않았다.

## G03. 셰이더에서 입력을 잃지 않는 저장·계산

`Shader_VtxMeshBinary.hlsl`과 `Shader_VtxMeshMapInstance.hlsl`은 같은 표면 입력과 식을 소비해야 한다.
일반 draw와 instancing 중 하나에만 반영하면 같은 바닥이 렌더 경로에 따라 달라진다.

Normal은 원본 encoding과 강도 적용식을 따라 decode한 뒤 기존 tangent/world 변환에 연결한다.
위 바닥은 mirrored scale이 있으므로 normal 방향과 handedness를 함께 확인한다.
`normal_intensity = 0.5`를 단순히 최종 normal 전체에 곱하면 이후 normalize에서 효과가
사라질 수 있으므로 숫자 전달 성공만으로 강도 복구가 됐다고 판단하지 않는다.

현재 G-buffer는 0 Diffuse, 1 Normal+scalar specular, 2 Depth+specular power,
3 PickPosition, 4 Emissive의 다섯 target이다.
원본 계산이 RGB specular를 요구하므로 **재질 specular RGB용 여섯 번째 target을 추가**하고
방향광·로컬광의 deferred 소비까지 함께 연결한다.
기존 `Target_Specular`는 광원 계산의 누적 결과이므로 재질 입력 저장소로 재사용하지 않는다.

| 변경 파일 | 연결할 계산 |
|---|---|
| `Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl` | 복구 대상으로 선택된 일반 mesh의 표면 계산과 G-buffer 출력 |
| `Client/Bin/ShaderFiles/Shader_VtxMeshMapInstance.hlsl` | 동일한 instanced 표면 계산과 출력 |
| `Engine/Private/Renderer.cpp`와 필요한 renderer 선언 | 새 target 생성·clear·MRT 등록·shader SRV 전달·수명 관리 |
| `Engine/Bin/ShaderFiles/Shader_Deferred.hlsl` | 재질 RGB를 보존한 directional/local specular 계산과 기여 진단 |

호환성 선택은 `Depth.w = 0` 미선택 표면, `Depth.w = 1` 복구 계산, `Depth.w = 2` 대상 재질의 A/B 기존 계산이다.
2는 진단 대상을 유지하면서 기존 scalar specular를 사용한다.
새 target의 alpha만 검사하면 앞서 그린 바닥 위에 기존 character가 덮였을 때 바닥의 specular가
남아 잘못 소비될 수 있다. 기존 opaque writer의 Depth.w=0과 실제 깊이 점유를 함께 사용한다.
현재 Depth clear는 `(1,1,1,1)`이므로 새 marker 설계에서는 clear.w를 0으로 정리하고,
background 제외와 경계 픽셀의 point/load sampling도 검증한다.
재질 선택 marker와 새 specular RGB는 같은 픽셀에서 point/load로 읽는다.
marker만 point로 읽고 RGB를 선형 보간해 다른 표면의 값을 섞지 않는다.
`BS_DeferredEmissiveOverlay`는 새 target의 write mask도 0으로 두어 기존 표면 정보를 보존한다.

Reflection은 G01에서 확정한 좌표식·색 공간·연산 순서·합성 위치로 구현한다.
이름이 reflection이라는 이유로 임의의 screen UV를 사용하거나 Emissive에 더하지 않는다.
원본의 reflection 합성과 원본 `use_emissive` 기능은 별도로 확인한다.
G-buffer 저장 범위에서 강도가 잘리는지도 확인하고, 후처리 전 표면 기여로 검증한다.

이 단계는 renderer 전체를 PBR/ORM 방식으로 교체하는 작업이 아니다.
복구 대상으로 선택한 재질의 식을 기존 renderer에 연결하며, 선택되지 않은 표면은 기존 계산을 유지한다.

## G04. Rendering Workbench에서 비교하는 방법

기존 Workbench에 다음 세션 진단 기능을 연결한다. 파라미터 slider·Save는 이번 범위에 포함하지 않는다.

| 제안 항목 | 사용자가 확인할 내용 |
|---|---|
| 현재 계산 / 복구 계산 | 같은 장면에서 대상 재질만 A/B 전환 |
| 대상 재질 표시 | 현재 asset·material·활성 family와 입력 로드 상태 |
| 고유색 보기 | 조명·Bloom 이전 바닥 무늬와 색 입력 |
| Normal 보기 | 요철 방향·강도·mirrored 배치 차이 |
| Specular 기여 보기 | 직접 광원에 의한 반짝임 색과 분포 |
| Reflection 기여 보기 | 복구한 환경 반사 계산의 기여 |
| 최종 화면 | 위 입력이 실제 장면에서 합성된 결과 |

UI는 `MainApp.cpp`의 기존 Rendering Workbench와 필요한 선언에 연결한다.
선택 상태는 세션 진단 상태이며 Engine 렌더 설정과 MapAssetRenderUtils의 실제 소비까지 연결한다.
A/B는 모델을 바꾸거나 파일을 매번 읽지 않고, 준비된 입력의 계산 선택만 바꾼다.
A는 기존 모작 재질 계산, B는 복구 계산이다. 두 모드 모두 handedness가 교정된 동일 모델을 사용하므로
A가 수정 전 실행 파일의 픽셀까지 재현하는 것은 아니다. 원작 영상과의 비교는 별도로 한다.

첫 비교에서 Exposure·Gamma·White Point·Bloom·광원을 동시에 바꾸지 않는다.
특정 입력만 바꾸었을 때 어느 기여가 변했는지 먼저 본다.
조명 specular가 0인 경우 재질의 직접 specular를 바꿔도 그 광원 기여는 나오지 않으므로
기여 진단 또는 명시적으로 고정한 비교 광원으로 입력 연결을 확인한다.

첫 단계는 비교 UI까지로 제한하고, 원본 식과 소비가 확정된 파라미터만 이후 편집 항목으로 연다.
새 slider는 단위·유효 범위·저장 정본·실제 소비자를 함께 연결한다.

현재 일반 `Reload Runtime`을 material prototype 재생성 기능으로 안내하면 안 된다.
MapTool의 기존 prototype fingerprint는 모델 경로만 본다.
초기 재질 데이터 갱신은 Client 재시작을 적용 기준으로 삼고, Level 재진입만으로 새 material이
생성되는지는 실제 prototype 수명 확인 후 안내한다. 진단 A/B는 재시작 없이 동작하게 한다.

## G05. 사용자가 보는 순서와 종료 기준

정상 빌드·배포 후 사용자가 수행할 순서다. 에이전트는 Client/UI를 실행하거나 캡처하지 않는다.

1. 사용자 PC의 `Server + Client` profile을 `Ctrl+F5`로 시작한다.
2. Lobby → KoukuSaydon → 기존 **1관문 - 세이튼** 이동을 사용한다. **3관문 - 세이튼**도 같은 복구 바닥 위치다. 원본 구역명 SL05의 별도 StageMarkers 경로를 화면 검증 경로로 안내하지 않는다.
3. F1 → Rendering Workbench에서 구현된 재질 비교 항목을 연다. 같은 카메라·해상도·시간 상태로 A/B를 전환한다.
4. 고유색 → Normal → Specular → Reflection → 최종 화면 순으로 확인한다.
5. Lobby → Character Select에서 기존 바닥·캐릭터·투명 이펙트가 공통 renderer 변경으로 깨지지 않았는지 본다.

쿠크 재질에만 선택 적용하는 첫 변경으로 Character Select의 원본 바닥까지 자동 복구되지는 않는다.
이 단계의 CS 확인은 공통 경로의 회귀 확인이다. CS 개선 비교는 G06의 별도 재질 연결 후 진행한다.

| 관찰 결과 | 다음으로 확인할 곳 |
|---|---|
| A/B 기여가 전혀 변하지 않음 | 실제 대상 asset/material·활성 분기·prototype 갱신·draw binding |
| Normal은 달라지나 반짝임이 없음 | 직접 광원의 specular 값·normal 방향·시선/광원 관계 |
| 기여 화면은 달라졌는데 최종 화면은 희게 뭉침 | HDR 합산·tone mapping·White Point·Bloom |
| static과 instanced에서 다름 | 입력 기본값·좌표 변환·두 shader 계산 일치 |
| 위 바닥만 변화가 다름 | S texture 비활성 분기·상하 겹침·mirrored transform·mask |
| 쿠크는 변하고 CS는 동일함 | 선택 적용 범위에 맞는 결과이며 CS 입력 복구를 별도 진행 |

자동 검증은 이름 기반 join과 실패 시 기존 상태 보존, texture 경로·색 공간·실제 바인딩,
shader 컴파일, MRT/legacy marker/overlay 계약, static·instanced 동일 입력을 포함한다.
새 target과 light 소비는 기존 관련 수치 probe로 확인하며 Client 화면 캡처나 광역 Effect oracle을
완료 조건으로 추가하지 않는다. 새 target의 비용은 기능 확인 뒤 기존 Benchmark로 측정한다.

구현된 기능에 필요한 최소 컴파일과 JSON/XML parse, `git diff --check` 후 최종 Product 빌드는
`Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`를 사용한다.
다른 세션의 빌드·publisher와 겹치지 않게 한다.
RESULT에는 구현·자동 검증·사용자 관찰을 분리하고, 사용자 확인 전 visual PASS를 기록하지 않는다.

## G06. Character Select와 이펙트로 확대하는 기준

2026-09-08 구현 상태: 아래 여섯 배치의 네 variant, 다섯 PBR 재질, UV1/lightmap·환경반사,
mip/filter와 기존 CS map-light 소비자 연결을 반영하고 Debug Product 및 관련 수치 검증을 마쳤다.
실제 결과와 근사는 대응 RESULT G08~G11을 따른다. 사용자 원본 PNG와 같은 맵 변형인지 및
최종 화면 유사도는 미확정이며, 이펙트 확대는 아직 실행하지 않았다.

### G06-1. 이번 비교의 목표와 원본 정본

사용자 원본 PNG의 밝은 석재·장식 구분과 세부 무늬, 모작 줌 비교에서 드러난 거리별 선명도를
복구 대상으로 삼는다. 모작 화면에 있는 잘못된 기본 재질을 조명색으로 보정하는 방식에서,
실제 원본 배치가 선택한 재질·텍스처·계산을 연결하는 방식으로 바꾼다.

현재 SL00 복구의 원본 정본은 `LV_LOBBY_CLASSSELECT_SL00` component의 ordered Materials override,
`BG_PCSELECT15` 실제 MIC, parent와 선택 ShaderMap의 effective 입력이다.
모델 기본 MIC만으로 원본 배치 재질을 대신하지 않는다.
사용자 PNG가 SL00와 같은 맵/버전이라는 의미는 아니다. 관련 원본 패키지와 중앙 배치의 동일성은
별도로 대조하며, 근거 없이 bridge를 삭제하거나 기존 승인된 높이를 변경하지 않는다.
최신 이미지·원본/native·현재 runtime 대조는
[조사 RESULT G13](2026-09-07_RENDERING_MATERIAL_PIPELINE_AUDIT_RESULT.md#G13-사용자-모작줌-비교와-원본-배치-재질-재확인)과
[실제 PBR 조사](C:/Users/user/Desktop/LostArk/out/CharacterSelectVisualReview20260908/source_material_findings.md)를 따른다.

첫 대상은 BRIDGE01E의 410/490, 411/495와 FLOOR12의 336, 371, 총 여섯 배치의 네 override 조합이다.
source export 번호는 0-based이며 placement의 stable ID와 transform을 보존한다.
MAGICFLOOR03D와 이펙트는 이 첫 묶음의 결과가 확인된 다음 범위다.

### G06-2. 배치별 material variant와 정확한 입력

기존 `Tools/LevelPlacementExtractor/build_map_material_variants.py`의 ordered override/signature 경로로
같은 모델의 서로 다른 재질 조합을 별도 asset variant로 만든다. 같은 Material을 공유하는 clone을
실행 중 덮어쓰지 않는다. 410/490은 override slot 0만 지정돼 있으므로 slot 1의 기본 상속을 유지한다.

원본의 package-qualified D/N/detail-normal/reflection/ORM 입력을 추출하고 필요한 물리 자산만
Resources에 준비한다. 비슷한 filename으로 다른 package의 normal을 대체하지 않는다.
기존 CModel→CMaterial에 이 입력을 합성하며, 새 texture 경로가 필요하면 기존
`MODEL_MATERIAL_OVERRIDE`에 실제 소비 필드만 확장한다. 누락된 필수 입력은 stage 실패로 보고한다.

`MapCatalog.json`의 CS Area에 sourceMaterials/materials를 선언하고, 기존 mapmaterials publisher와
catalog v5 필수 참조 경로로 전달한다. 저작 mapplacements의 해당 여섯 asset 참조만 variant로
변경하며 기존에 승인된 아홉 Y 보정과 다른 배치, gameplay/navigation, 카메라 설정을 보존한다.
runtime 문서는 publisher가 생성한다. 새 .mapmaterials JSON은 Client의 96.DataFiles None/filter에 등록한다.

### G06-3. 원본 PBR 표면식과 진단

확인된 실제 family는 `bg_base_pbr_opa`, `bg_base_pbr_seamless_opa`다. 쿠크의 기존 두
specular/reflection program으로 대체하지 않는다. MODEL_SURFACE_FAMILY와 공통
`Shader_MapMaterialSurface.hlsli`에 실제 PBR 선택을 명시적으로 추가하고 일반/인스턴스 양쪽에서 소비한다.
현재 program 1 이외를 모두 program 2로 처리하는 else를 새 family의 fallback으로 사용하지 않는다.

다음 입력·계산을 한 기능으로 연결한다.

- 원본 diffuse 색공간, brightness/color/saturation 순서와 texture별 UV 선택.
- 원본 base normal RG에서 Z를 복원한 뒤 base/detail XY 강도와 vertex alpha를 적용하는 순서.
- 실제 ORM R/G/B의 AO/roughness/metallic 연산, intensity→SafePow→clamp와 원본 F0 계산.
- Normal.A를 mask로 쓰는 2D 반사 합성. 반사를 emissive로 바꾸지 않는다.
- 기존 G-buffer와 deferred 소비까지 roughness/metallic/F0 의미를 보존하는 명시적 PBR 경로.
- 선택한 표면의 Base color/Normal/Roughness/Metallic/반사 기여를 조명과 후처리에서 분리하는 진단.

336은 diffuse와 normal UV 배율이 다르고, 371 diffuse는 원본 SRGB=false다. 모든 슬롯에
같은 UV와 색공간을 강제하지 않는다. Source scalar 1.1~1.6을 최종 roughness 값으로 오해해
저장 단계에서 [0,1]로 잘라 버리지 않는다.

환경 cube·BRDF lookup·engine constant의 native binding과 ORM SRV 색공간은 자동으로
원본 정합 처리하지 않는다. 이 조사에서 확보한 표면식과 아직 확보하지 못한 엔진 입력의 경계는
RESULT에 남긴다. 원본식 일부를 연결한 단계와 실제 원작 전체 광원 결과를 같은 완료 상태로 기록하지 않는다.

### G06-4. 노멀 geometry와 mip/filter

새로 추출한 FLOOR12 source glTF와 기존 WModel을 기존 geometry cooker로 대조한다.
좌표/정점/UV/소재·index 순서를 보존하고 source handedness/COLOR0만 실제 근거대로 운반한다.
UV에서 재구성한 부호를 GLTF_PRESERVED 증거로 기록하지 않는다. BRIDGE도 같은 source 경로로
확인한 뒤 선택 자산을 교체한다. TEXCOORD_1과 lightmap 입력은 기존 UV0와 별도 계약이다.

원거리 잔선 문제에는 mip chain과 표면 sampler를 함께 준비한다. 원본 전체 mip을 확보할 수 있으면
그 입력을 보존하고, 생성 mip을 사용하면 프로젝트에서 생성한 축소본으로 기록한다.
BC1/BC5/TGA를 runtime context overload 하나로 자동 해결한다고 가정하지 않는다.
색상은 해당 SRGB 계약, normal·mask는 선형 데이터 의미를 유지하며 TGA alpha를 삭제하지 않는다.

선택된 CS 표면에만 필요한 sampler를 바인딩하고 legacy와 쿠크 A/B 입력을 임의 변경하지 않는다.
FXAA는 geometry 외곽의 계단을 줄이는 별도 비교 항목이며 mip chain을 대신하는 완료 조건이 아니다.

### G06-5. 원본 광원·환경 입력과 사용자 튜닝

원본 SL00에서 확인한 Point 6개·Spot 1개와 lightmap 자료를 출발점으로 사용한다.
빛 종류·위치·방향·색·반경·활성 상태·lighting channel을 조사한 뒤 기존 MapLight 데이터와
일반 runtime 소비자에 연결한다. 꺼진 광원과 static 바닥에 영향을 주지 않는 채널을 모두 켜지 않는다.
scene profile의 Directional을 원본에 존재했던 광원으로 기록하지 않는다.

원본 단위·거리 감쇠와 현재 조명식을 맞추고, lightmap/환경반사 자료를 별도 소비한다.
Texture2D 58개가 존재한다는 사실만으로 그 texture를 어느 mesh/UV로 읽어야 하는지까지
확보됐다고 취급하지 않는다. 원본 point/spot 정보 복구와 간접광/lightmap 복구는 서로 다른 검증 항목이다.

사용자가 현재 Directional Light Detail와 Scene Brightness를 조절하고 있으므로 그 저장값을
덮어쓰지 않는다. 재질과 원본 광원 비교가 준비되면 같은 카메라에서 기여를 분리해 사용자가 비교한다.
마지막에 노출·White Point·Bloom과 필요한 색 보정을 맞춘다.

### G06-6. 변경 파일과 검증 단위

| 기존 경계 | 필요한 연결 |
|---|---|
| material variant importer와 geometry cooker | 원본 ordered override, 선택 geometry/source texture 준비 |
| CS mapassets/mapplacements/mapmaterials + MapCatalog | stable 배치의 variant 참조와 필수 재질 데이터 |
| MapAssetCatalog/Publish-MapAuthoring | 새 입력·경로·family 검증, stage 실패 시 기존 데이터 유지 |
| ModelAssetData/Model/Material | immutable 표면 입력과 실제 D/N/detail/ORM/reflection SRV |
| MapAssetRenderUtils + 공통 surface HLSLI + 두 map shader | 일반/인스턴스의 동일 표면식, 명시적 program 분기와 상태 초기화 |
| Renderer/Deferred shader | PBR 입력을 잃지 않는 저장·광원 소비·진단 |
| MainApp Rendering Workbench | 선택 대상의 A/B와 기여 확인, 사용자 scene draft 보존 |
| CS maplights 및 기존 MapLight runtime | 원본 광원 선택·좌표·채널 연결과 shadow/environment 분리 |

자원 준비는 원본 package-qualified identity, 선택 texture의 전체 mip·색공간·alpha와 geometry를
확인한다. 입력 연결은 기존 publisher/catalog→CModel/Clone 검증을, shader 계산은 기존 focused
WARP 수치 경로를 재사용한다. 최소 컴파일·배포와 JSON/XML/diff 검사를 마친 뒤 사용자가
동일 위치/카메라와 원거리·근거리에서 바닥 무늬·금속 반응·잔선을 확인한다.
새 receipt framework나 별도 모델 runtime을 추가하지 않는다. 화면 유사도는 사용자 확인으로 종료한다.

### G06-7. 이펙트 확대

이펙트는 그 뒤 고정된 장면에서 발광색·밝기·알파·blend·Bloom 전후를 분리한다.
공통 렌더링 개선으로 함께 좋아질 부분은 있지만, 누락된 이펙트 층·시간 변화·왜곡·합성식은
이펙트의 해당 경로에서 복구해야 한다. 이동·스킬 반응 같은 조작감은 입력·Server·animation·camera
경계의 별도 작업으로 관리한다.

이번 계획의 원칙은 **화면에서 큰 재질 하나의 입력과 계산을 끝까지 연결하고,
그 결과를 확인한 다음 같은 방식으로 넓히는 것**이다.

## G07. 종료 기록

구현·자동 검증·수동 관찰·확대 범위는 대응 RESULT에서 분리한다.
다른 세션의 미완성 변경이 함께 있는 dirty working copy이므로 자동 stage/commit/push는 하지 않는다.
Resources의 쿠크 두 WModel과 CS 네 variant·texture·lighting 입력은 로컬 runtime 입력이며 Git에 추가하지 않는다.
다른 PC에 필요한 물리 폴더와 실제 Drive 전달 여부는 RESULT의 해당 단계에서 기록한다.

## G08. Character Select 전체 맵 확대

2026-09-08 사용자 승인에 따라 중앙 링·장식 → 주변 바닥·다리 → 기둥·배경 → 투명·발광 재질
순서로 SL00의 현재 803배치를 복구한다. 직전 여섯 배치의 사용자 색감 개선 관찰을 기준점으로 삼고,
사용자의 RenderingProfiles·카메라 및 승인된 아홉 Y 보정을 보존한다. Client/UI 실행과 화면 캡처는
에이전트의 작업에 포함하지 않는다. 자동 검증 뒤 실제 화면은 사용자가 판정한다.

### G08-1. 원본 배치와 재질 조합

기존 `extract_ue3_placements`와 `build_map_material_variants`로 원본 803배치, 55 mesh,
62 ordered-material 조합을 확인했다. 301배치에 override가 있고 502배치는 기본 상속이다.
override의 slot 순서와 null 상속을 보존하며, 모델의 기본 재질을 실제 배치 재질로 대체 추정하지 않는다.
기존 importer가 미해결 배치를 0개로 읽은 결과를 기준으로 원본 MIC/parent/선택 shader를 연결한다.

중앙 483/485의 실제 재질은 `bg_pcselect14.mat.bg_elg_kayangel_floor02a_mio_ksr`이고
parent는 `bg_seamless-specular_opa`다. 현재 `bg_gdogods_magicfloor01b_mi_ksr` 기본값에서
전환한다. 원본 opaque 분기에는 discard가 없으므로 임의 투명화로 중앙 구성 차이를 숨기지 않는다.

### G08-2. 기존 모델·라이트맵 경로 확장

원본 LOD 데이터는 LightMap2D 799배치(29 atlas 쌍), LightMap1D 1배치, LOD 데이터 없는
3배치로 읽혔다. 같은 mesh/material도 atlas·환경반사가 다르면 별도 immutable 입력이 필요하다.
실제 조명 입력까지 포함한 179그룹은 기존 단일 catalog의 512개 제한 안이다.

같은 ordered-material 조합은 기존 `_OVR_` ID를 사용한다. 그 조합 안에서 라이트맵 texture 쌍이나
환경반사 입력이 다른 그룹에는 안정적인 입력 signature로 `_LGT_` 파생 ID를 붙인다.
atlas 좌표·색 계수는 계속 stable sourcePlacementId의 placementLighting이 소유한다.
기존 여섯 배치의 네 ID와 입력은 일치하는 그룹에서 유지한다. 여러 위치의 조명을 하나로 덮어쓰거나
같은 CMaterial을 clone 생성 후 변경하지 않는다. Loader/MapTool/CModel 경로를 그대로 사용한다.

모델은 기존 exporter/ModelAssetConverter/geometry cooker를 재사용하며 현재 배치가 참조하는
55개 source mesh에 한정해 준비한다. UV1·tangent·COLOR0의 실제 출처와 반복 안정성을 확인한다.
원본과 다른 추출 결과를 source-preserved로 기록하지 않는다. 텍스처 색 공간과 alpha를 보존하고
생성 mip은 원본 mip과 구분한다. 설치는 준비된 자산과 authoring 연결을 검증한 뒤 진행한다.

### G08-3. 중앙 링 계산과 C++ 소비자

`MODEL_SURFACE_FAMILY::SOURCE_SPECULAR_OPAQUE=5`와 JSON family
`bg_seamless-specular_opa`를 추가한다. 기존 쿠크 program 1/2와 PBR 3/4를 유지한다.
marker 4는 source specular opaque 표면을 식별하며, UV tiling·D/N/S/reflection 입력과
원본 Blinn 직접광, RNM 라이트맵, 원본의 세 방향 기저 반사 합을 전달한다.
ORM/AO/PBR cube 계산을 이 재질에 임의 적용하지 않는다.

ModelAssetData/Model/Material은 실제 named input·경로·수치·UV1을 GPU 할당 전에 검증한다.
MapAssetCatalog와 Publish-MapAuthoring은 같은 JSON 계약과 실패 시 기존 상태 보존을 유지한다.
MapAssetRenderUtils는 새 family의 원본 texture·UV·조명 입력을 일반/인스턴스 양쪽에 전달한다.
공통 surface HLSLI, 두 map shader와 deferred가 원본 수식을 소비하며 HDR 표면값의 저장 범위도
수치 검사한다. source shader의 한 계열을 이름만 비슷한 다른 permutation에 자동 적용하지 않는다.

### G08-4. 확대 검증과 종료

각 실제 재질 계열의 입력과 수식을 조사한 뒤 기존 runtime에 연결한다. 투명·발광은 원본 blend,
opacity·시간·발광식을 해당 렌더 경로에서 확인한다. 별도 mock runtime이나 새 admission framework는
만들지 않는다. native LightMap1D와 원본 표시 상태는 실제 소비 근거로 처리하며 parser 성공을
화면 복구 완료로 기록하지 않는다.

기존 GPU 수치 probe와 native catalog/CModel 검증을 재사용해 새 family, 실제 자산 생성·바인딩,
clone 수명과 실패 시 기존 상태 보존을 확인한다. 변경 기능의 최소 컴파일과 Debug Product 배포,
해당 Area Validate/Publish/Check, JSON/XML parse와 diff 검사를 수행한다.
새 C++ 파일은 계획하지 않으며 등록된 기존 파일을 확장한다. 추가 authoring 문서가 필요하면
Client 프로젝트의 96.DataFiles None/filter 등록도 같은 변경에서 처리한다.

원본 PNG의 중앙 원판과 현재 십자 구성 차이는 자료·기하·표시 조건으로 별도 대조한다.
현재 map version/프레임 상태가 일치하지 않은 것을 조명 보정으로 해결됐다고 보고하지 않는다.
완료 보고에는 적용한 배치/재질 범위, 자동 검증과 사용자 관찰, 원본 엔진 입력의 근사를 분리한다.

### G08-5. 실제 기본·PBR 분기 확대

75개 실제 사용 재질의 native uniform/default/texture 선택을 확인했다. 기존 source specular와
다르게 기본 재질은 직접광 specular의 RGB 상한 2와 base-pass 세 방향 specular 합을 사용하지
않는다. SOURCE_BASE_OPAQUE=6, SOURCE_BASE_MASKED=7, SOURCE_PBR_MASKED=8로 구분한다.
원본 family 이름이 masked라도 실제 shader에 discard가 없는 재질은 opaque로 처리한다.
기존 쿠크 bg_base_msk program 2는 유지하며, 새 CS v2 데이터의 명시적인 alphaMode로 6/7을 선택한다.

surfaceFeatures는 실제 compiled 분기의 normal/detail/reflection/specular texture/emissive/flicker/
metallic diffuse tint/overlay/vertex paint 비트를 전달한다. 발광 texture·색·강도·UV와 시간 변화,
metallic diffuse 색·밝기, overlay texture·색·tiling·밝기·채도·sharpness·specular 강도를 기존
MODEL_SURFACE_PARAMETERS와 CMaterial에 추가하고, 일반·인스턴스 draw의 동일 binding에서 소비한다.
시간은 기존 draw elapsedTime을 사용한다. 원본 엔진의 origin phase 미확정은 0인 프로젝트 해석으로
명시한다. 비활성 texture 입력을 임의 dummy로 필수화하지 않으며 활성 비트의 실제 입력만 검증한다.

COLOR0를 소비하는 분기는 실제 mesh stream을 VS→PS로 운반한다. UModel의 두 번 같은 출력만으로
원본 색·tangent 부호를 확정하지 않는다. 실행마다 달라진 채널은 원본 packed stream으로 대조하며,
UV 미분으로 재구성한 tangent와 제외한 색 채널은 원본 보존 채널과 구분한다.

### G08-6. 사용자 승인 Git 통합

2026-09-08 추가 요청에 따라 연결·검증을 마친 변경과 완료된 다른 작업의 변경을 기능별로 정리하고,
origin/main의 실제 변경과 충돌을 해결한 뒤 PR·merge·로컬 pull 동기화를 수행한다. 위 G07의
자동 Git 미수행 기록은 추가 승인 전 상태다. Resources, 빌드 산출물과 개인 파일은 계속 제외한다.
공용 protocol과 world-sequence 메시지는 양쪽 필드·동작을 하나의 reader/writer로 통합하고,
수동 visual fidelity 미확인을 자동 PASS로 바꾸지 않는다.
