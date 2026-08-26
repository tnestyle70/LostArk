# 2026-08-25 베른성 시각 복원 단계 계획

이 문서는 외부에서 받은 "베른성 8단계 해결안"을 현재 저장소의 실제 코드·데이터로
재현한 뒤, 프레임워크 계약을 깨지 않는 순서로 다시 세운 계획이다.
받은 제안을 그대로 옮기지 않는다. 실측으로 확인된 항목만 남기고, 사실과 다른 항목은
근거와 함께 교정한다.

정본 규칙: `AGENTS.md`, `CLAUDE.md`, `.md/GB/local.md`, `.md/GB/계획서작성규칙.md`.
화면 판정 경계: 이 계획의 모든 시각 결과는 사용자가 직접 Client에서 확인하고 판정한다.
에이전트는 데이터·코드 변경과 구조적 검증까지만 수행한다.

---

## 1. 현재 실제 반영 상태 (2026-08-25 실측)

### 1.1 베른 장면 조명은 방향광 1개이고 그림자가 꺼져 있다

`Data/Rendering/Authored/RenderingProfiles.json`의 `scene.bern.neutral-day.v1`:

```text
light.type      = directional
light.direction = (0.5, -1, 0.5, 0)
light.diffuse   = (0.8, 0.8, 0.8, 1)
light.ambient   = (0.25, 0.25, 0.25, 1)
light.specular  = (0.5, 0.5, 0.5, 1)
shadow.enabled  = false
```

이 profile을 `Client/Private/LevelRegistry.cpp:103`이 `LEVEL::BERN`에 묶는다.
`CRenderingProfileService::Commit_Resolved`(`Client/Private/RenderingProfileService.cpp:666`)가
`CGameInstance::Add_Light(Profile.Light)`를 한 번만 호출하고,
`CLight_Manager::Add_Light`(`Engine/Private/Light_Manager.cpp:50`)는 받은 desc 하나로
`Replace_SceneLights`를 호출한다. 즉 베른의 scene light는 정확히 1개, 그림자는 off다.

받은 진단의 `전체가 어둡고 입체감 부족`, `건축물 그림자 없음`은 여기서 전부 설명된다.
라이트맵이나 신규 광원 시스템이 아니라 이미 존재하는 authoring 값 문제다.

### 1.2 엔진 광원 타입은 DIRECTIONAL과 POINT뿐이고 상한이 세 겹으로 걸려 있다

```text
Engine/Public/Engine_Enum.h:50               enum class LIGHT { DIRECTIONAL, POINT, END };
Engine/Private/Light_Manager.cpp:58          scene light 16개 초과 시 E_INVALIDARG
Engine/Private/Presentation_Manager.cpp:8    MAX_TRANSIENT_LIGHTS = 64
Engine/Private/Presentation_Manager.cpp:302  LIGHT::POINT 이외는 전부 거부
Client/Public/MapLightDocument.h:29          MAX_LIGHT_COUNT = 64
Tools/MapPipeline/Publish-MapAuthoring.ps1:172  lights 배열 1..64 강제
```

`CLight::Render_Desc`는 광원 하나당 화면 사각형 1패스를 그린다
(`CLight_Manager::Render_Lights`, `Engine/Private/Light_Manager.cpp:69`).

따라서 원본 광원 326개는 현재 런타임에 그대로 올릴 수 없다.
Spot / Dominant Spot / Sky Light는 타입 자체가 없고, Point만 추려도 64개가 상한이다.
326개를 억지로 올리면 프레임마다 전체 화면 블렌드 패스가 326번 발생한다.

### 1.3 Masked foliage는 이미 동작하고 있다 — 실제로 빠진 것은 양면 렌더링이다

받은 제안 3번 `foliage 전용 Masked 패스를 만듭니다`는 현재 코드와 맞지 않는다.
deferred 픽셀 셰이더가 이미 alpha clip을 수행한다.

```text
Client/Bin/ShaderFiles/Shader_VtxMeshBinary.hlsl:113       if (diffuse.a < 0.3f) discard;
Client/Bin/ShaderFiles/Shader_VtxMeshMapInstance.hlsl:171  같은 discard
```

그리고 베른 나뭇잎 텍스처는 알파를 가진 DXT5다.

```text
MAP_E4BA13439D67_BG_ATM_TREE_GARDENTREE01C_SM_YSI/textures/
  bg_atm_tree_needletree01_da_kyo.dds    DXT5
  bg_atm_tree_needletree01a_da_ksy.dds   DXT5
  bg_atm_tree_needletree01a_n_ksy.dds    ATI2 (normal)
```

즉 잎 카드의 배경은 이미 잘려 나간다. 남아 있는 실제 결함은 cullMode다.
현재 베른 catalog의 모든 행이 `Opaque Back`이라 잎 카드의 뒷면이 통째로 사라진다.
한 장짜리 잎 평면은 카메라 반대편에서 보면 없어지므로 밀도가 절반으로 보이고
남은 면만 강조되어 덩어리처럼 보인다.

### 1.4 배치별 재질 오버라이드는 이미 wmodel에 구워져 있다 (단, 메시당 1세트)

`MAP_E4BA13439D67_BG_ATM_TREE_GARDENTREE01C_SM_YSI.wmodel` 내부 머티리얼 이름:

```text
offset 11083  bg_atm_tree_needletree01_mi_kyo
offset 15838  bg_atm_tree_needletree01a_mi_ksy
```

받은 진단이 `정원나무 메시 -> 베른 전용 needletree..._01, _02`라고 말한 그 오버라이드가
이미 추출 시점에 적용되어 있다. 따라서 `배치별 재질 정보가 사라졌습니다`는 전면적으로는
사실이 아니다. 정확한 한계는 다음이다.

- 하나의 asset ID = 하나의 `.wmodel` = 하나의 머티리얼 세트다.
- 같은 메시를 서로 다른 재질로 쓰는 배치가 있으면 그중 한 세트만 남는다.
- SL06 카탈로그에서 같은 label이 두 asset ID를 갖는 경우는 1건뿐이다
  (`bg_ber_raniat_pillar02_sm_psy`).

즉 `MAP_PLACEMENT_RECORD`에 `materialSetId`를 추가해야 할 만큼의 근거는 아직 없다.
이 계약 변경은 `Client/Public/MapPlacementDocument.h:14`의 저장 포맷,
`CMapPlacementDocument::Read/Write`, publisher, MapTool 저장 경로를 전부 건드린다.
LFS로 관리되는 50,017행 placement 문서를 다시 쓰는 작업이므로 근거 없이 시작하지 않는다.

### 1.5 이미 존재하는 안전한 재질 조정 레버 — renderprofiles.json

```text
Data/Maps/Imported/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.renderprofiles.json
  -> Tools/LevelPlacementExtractor/build_bern_castle_shards.py:66
  -> Tools/LevelPlacementExtractor/build_maptool_scene.py:274 render_profile_text()
  -> .mapassets 행 꼬리 컬럼
  -> Client/Private/MapAssetCatalog.cpp:476 파싱
  -> MAP_ASSET_RENDER_PROFILE
  -> Client/Private/MapAssetRenderUtils.cpp Select_Pass / Bind_Material
```

`.mapassets` 행 꼬리 컬럼 순서는 다음과 같다.

```text
renderMode cullMode uvScale.x uvScale.y uvSpeed.x uvSpeed.y
opacity emissiveIntensity specularIntensity specularPower
colorTint.x colorTint.y colorTint.z colorTint.w opacityPower
```

현재 베른의 모든 행이 기본값이다.

```text
Opaque Back 1 1 0 0 1 1 1 50 1 1 1 1 1
```

허용 범위(`Client/Private/MapAssetCatalog.cpp:93 IsValidRenderProfile`,
`build_maptool_scene.py:292`)는 다음과 같다.

```text
renderMode        Opaque | Alpha | Sky | Additive
cullMode          Back | Front | None      (None = 양면)
opacity           0..1
opacityPower      0.01..64
emissiveIntensity >= 0
specularIntensity >= 0   (상한 없음)
specularPower     >= 1
colorTint         각 성분 >= 0
```

specular는 deferred G-buffer에 그대로 실린다.

```text
Shader_VtxMeshBinary.hlsl:154  output.vNormal.w = specularMask (= g_SpecularIntensity)
Shader_VtxMeshBinary.hlsl:157  output.vDepth.z  = g_SpecularPower
```

따라서 RUMINAWATER의 `specular_intensity 10 / specular_power 100`은
C++ 한 줄 없이 지금 바로 반영할 수 있다. 반사 환경맵만 별도 문제다.

### 1.6 RUMINAWATER 실제 배치

```text
Data/Maps/Imported/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE_SL06.mapassets:58
Data/Maps/Imported/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE_SL06.mapplacements:2924~2927
```

SL06의 4개는 분수 하나를 둘러싼 배치다.

```text
(84.56, 52.98, -156.56)
(91.28, 52.98, -156.56)
(87.92, 52.98, -153.20)   rot y-quat 0.3137
(87.92, 52.98, -159.92)
```

Area 전체로는 `MAP_2D6A6B1587D0_BG_RHD_BREEZE_RUMINAWATER01_SM_ASJ` 9개다.
같은 지점의 `MAP_43A8B6503530_LV_MODULE_WATER01_1024`는 마지막 컬럼이 `0`,
즉 이미 visible=false로 숨겨져 있다.

### 1.7 베른에는 광원 문서 자체가 선언되어 있지 않다

```text
Data/Maps/MapCatalog.json:5~20    LV_BER_BERNCASTLE 항목에 sourceLights/lights 없음
Data/Maps/MapCatalog.json:26,30   LV_LUT_HEARTRB_ED 만 pair 보유
Client/Private/Level_Bern.cpp:38  m_MapRuntime.Load_Area 만 호출
Client/Private/Level_ValtanArena.cpp:217  발탄만 CMapLightPresentationRuntime 사용
```

`Publish-MapAuthoring.ps1:254`는 `sourceLights`와 `lights` 중 하나만 선언하는 것을 거부한다.
`Publish-MapAuthoring.ps1:519`가 shard-set을 지원하므로 베른도 같은 publisher로 처리된다.

### 1.8 추출기 텍스처 역할 표

`Tools/BernCastlePipeline/build_bern_castle_assets.py:29`의
`TEXTURE_PARAMETER_TO_SWITCH`는 diffuse / basecolor / albedo / normal / specular /
emissive / opacity / orm / metallic / roughness / ao를 처리한다.

받은 진단의 `현재 추출기는 diffuse·normal·specular까지만 인식`은 사실이 아니다.
실제로 빠진 항목은 `texture_reflection` 하나뿐이다.

---

## 2. 받은 8단계 제안에 대한 판정

| 받은 제안 | 판정 | 근거 |
|---|---|---|
| 1. texture_reflection 추출 추가 | 보류(G5) | 소비할 셰이더 슬롯·큐브맵 샘플러가 없다. 추출만으로는 화면이 바뀌지 않는다 |
| 2. 머티리얼 단위 렌더 방식 분리 | 거부(현 시점) | 두 번째 런타임 재질 경로가 된다. asset 단위 renderprofiles.json이 이미 정본이다 |
| 3. foliage 전용 Masked 패스 | 이미 구현됨 | Shader_VtxMeshBinary.hlsl:113, Shader_VtxMeshMapInstance.hlsl:171. 실제 결함은 cullMode |
| 4. 배치별 materialSetId 계약 | 보류 + 근거 필요 | 메시별 재질 세트는 이미 wmodel에 구워져 있다. 중복 label은 SL06에서 1건 |
| 5. RUMINAWATER 환경 반사 | 부분 채택(G3/G5) | specular 10/100과 tint는 지금 가능, 환경맵 반사는 G5 |
| 6. 베른 광원 publish·런타임 연결 | 채택(G4), 상한 준수 | 326개 불가. POINT만, 64개 이하. Spot/Sky는 Engine public 변경 필요 |
| 7. 안개·바람 환경 문서 | 보류(G6) | 소비자 없음. Engine 포스트/정점 경로 신규 계약이 먼저 필요 |
| 8. 라이트맵·그림자 복원 | 절반 채택(G1) | 방향광 그림자는 이미 있고 꺼져 있을 뿐이다. 라이트맵 베이크는 별도 |

받은 제안이 놓친 가장 큰 항목은 베른 scene profile의 그림자가 꺼져 있다는 사실이다.
이것이 코드 0줄로 고칠 수 있는 최대 효과 항목이다.

---

## 3. G 경계와 순서

효과 대비 위험이 낮은 순서다. 각 G는 독립적으로 되돌릴 수 있다.

```text
G1  베른 scene artistic profile   코드 0줄, JSON 1개          그림자·명암·색온도
G2  foliage cullMode = None       코드 0줄, JSON 1개          나무 밀도·덩어리감
G3  물·거울 재질 profile           코드 0줄, 같은 JSON         RUMINAWATER 광택
G4  베른 point light 레이어        Client 소량 + JSON+catalog  국소 조명
G5  환경 반사 슬롯                 Engine public + 셰이더      진짜 하늘 반사
G6  안개·바람                      Engine 신규 계약            공간 깊이
```

G1~G3은 전부 `Data`만 바꾸며 Engine public header를 건드리지 않으므로
`UpdateLib.bat` 재실행이 필요 없다. G4부터 Client 재빌드가 필요하다.

---

## 4. 파일 목록

| 구분 | 경로 | G | 역할 |
|---|---|---|---|
| 수정 | `Data/Rendering/Authored/RenderingProfiles.json` | G1 | 베른 방향광·그림자 authoring |
| 수정 | `Data/Maps/Imported/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.renderprofiles.json` | G2,G3 | asset별 cull/재질 override |
| 추가 | `Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.maplights.json` | G4 | 베른 point light 정본 |
| 수정 | `Data/Maps/MapCatalog.json` | G4 | sourceLights/lights pair 선언 |
| 수정 | `Client/Public/Level_Bern.h` | G4 | 런타임 멤버 선언 |
| 수정 | `Client/Private/Level_Bern.cpp` | G4 | CMapLightPresentationRuntime 연결 |

G1~G4 모두 신규 C++ 파일이 없으므로 `.vcxproj`/`.filters` 등록 변경이 없다.

---

## 5. G1 — 베른 scene artistic profile

### 5.1 왜 이것이 첫 단계인가

받은 진단표의 `전체가 어둡고 입체감 부족`과 `건축물 그림자 없음` 두 행이 한 번에 닫힌다.
이 값은 F1 Developer Tools에서 실시간으로 조정되고, 저장 경로가 이미 완결되어 있다.

```text
F1 -> Rendering 패널 -> Active Scene Artistic Profile
  Light Direction / Diffuse RGB / Ambient RGB / Specular RGB
  Directional Shadow Enabled / Focus / Distance / Coverage / Near / Far
  Depth Bias / Normal Bias / Strength
-> Save Authored   (Data/Rendering/Authored/RenderingProfiles.json 교체)
-> Publish Runtime (검증 후 runtime 승격)
-> Reload Runtime  (원자적 commit)
```

구현 위치는 `Client/Private/MainApp.cpp:3670~3780`이다.

### 5.2 적용 위치

파일: `Data/Rendering/Authored/RenderingProfiles.json`
작업: 블록 교체
기준점: `"profileId": "scene.bern.neutral-day.v1"` 객체 전체
같은 변경에서 문서 최상단 `revision`을 9에서 10으로 올린다.

### 5.3 교체 블록

```json
  {
   "profileId": "scene.bern.neutral-day.v1",
   "exposureMultiplier": 1,
   "bloomIntensityMultiplier": 1,
   "light": {
    "type": "directional",
    "direction": [0.42, -1, 0.35, 0],
    "diffuse": [1.02, 0.98, 0.88, 1],
    "ambient": [0.22, 0.26, 0.34, 1],
    "specular": [0.9, 0.93, 1, 1]
   },
   "shadow": {
    "enabled": true,
    "focus": [88, 52, -157],
    "distance": 120,
    "orthographicWidth": 150,
    "orthographicHeight": 150,
    "near": 1,
    "far": 320,
    "depthBias": 0.0015,
    "normalBias": 0.03,
    "strength": 0.72
   }
  },
```

값의 의도는 다음과 같다.

- `direction`은 원본 화면의 그림자 방향에 맞춘 시작값이다. 사용자가 F1에서 최종 조정한다.
- `diffuse`가 회색 0.8에서 약간 따뜻한 1.0대로 올라간다. 석조 건물의 대비를 만든다.
- `ambient`가 회색 0.25에서 푸른 쪽 (0.22, 0.26, 0.34)으로 이동한다.
  받은 진단의 `푸른 분위기 부족`은 안개 없이 ambient 색온도만으로도 상당 부분 잡힌다.
- `specular` 0.9대가 필요한 이유는, G3의 RUMINAWATER specularIntensity 10이
  실제로 반응하려면 scene light의 specular가 살아 있어야 하기 때문이다.
- `focus`는 1.6절에서 실측한 분수 광장 중심 (87.92, 52.98, -156.56)을 반올림한 값이다.
- `orthographicWidth/Height 150`은 그림자 맵이 고정 2048이므로
  150m 커버리지에서 텍셀당 약 7.3cm가 된다는 계산에서 나온 시작값이다.
- `far 320`은 distance 120과 커버리지 대각선을 포함한다.

이 수치는 시작점이며 최종 값이 아니다.
사용자가 F1에서 조정한 뒤 Save Authored를 누르면 이 파일이 갱신된다.

### 5.4 검증

```text
1. JSON parse 확인
2. powershell -ExecutionPolicy Bypass -File Tools/RenderingPipeline/Publish-RenderingProfiles.ps1
3. Client 실행 -> Lobby -> Bern 진입
4. F1 -> Rendering -> Directional Shadow Enabled 체크 상태 확인
5. 사용자 육안 판정 후 F1에서 조정, Save Authored, Publish Runtime, Reload Runtime
```

---

## 6. G2 — foliage cullMode 양면 전환

### 6.1 근거

1.3절에서 확인한 대로 alpha clip은 이미 동작한다. 현재 `Back` cull이므로
잎 카드 평면의 뒷면이 사라진다. `None`으로 바꾸면 양면이 그려진다.

`Select_Pass`(`Client/Private/MapAssetRenderUtils.cpp:19`)가
TWO_SIDED를 pass offset +2로 보내고, deferred 계열에서는 TwoSidedOpaquePass가
받는다(`Shader_VtxMeshBinary.hlsl:422`, `Shader_VtxMeshMapInstance.hlsl:401`).
alpha clip은 같은 PS_MAIN을 쓰므로 깊이·그림자 기록이 그대로 유지된다.

### 6.2 대상 선정

배치 수 기준 상위 foliage asset이다. `_artree`가 붙었지만 실제로는 기둥·벽인
`bg_scd_mazea_pillar03_sm_artree`, `bg_ber_berncastle_wall02_sm_artree`,
`bg_ber_berncastle_coulmn01_sm_artree`는 제외했다.

```text
 8719  MAP_41CB42CA5056_BG_ATT_FOLIAGE_TOTGRASS03_SM_KSY
 3712  MAP_F114BB99842A_LV_ATM_LOGHILL_FOLIAGE01_SM
 1353  MAP_DAD60E49AC6A_BG_FOLIAGE_COMMONPLANT03_SM_KSY
 1203  MAP_9976C66E4100_BG_ANH_FOLIAGE_TE_F30_SM_OLD_01
 1153  MAP_E4BA13439D67_BG_ATM_TREE_GARDENTREE01C_SM_YSI
  844  MAP_A7CE965CE471_BG_ATM_TREE_GARDENTREE01D_SM_YSI
  833  MAP_2B73F7F74F45_BG_BER_COMMON_BERIVY01D_SM_KSY
  636  MAP_9A85926B83B3_BG_ATM_FOLIAGE_IVY11_SM_KSY
  414  MAP_2BB656F8F483_BG_ATM_FOLIAGE_IVY12_SM_KSY
  399  MAP_0B39DAEA62CA_LV_ATM_LOGHILL_FOLIAGE02_SM
  357  MAP_BCD69ACF8725_BG_BER_COMMON_BERIVY03C_SM_KSY
  300  MAP_97F1B98D54FE_BG_ANH_FOLIAGE_TE_F26_SM_YSI
  283  MAP_A4E9B7719B98_BG_RHD_TREE_MAPLE02C_SM_KSY
  275  MAP_79EB8ED6D468_BG_ATM_FOLIAGE_IVY05_SM_KSY
  270  MAP_5C49D0E5A7F2_BG_BER_COMMON_BERIVY03A_SM_KSY
  228  MAP_B852510B5B87_BG_ATM_TREE_GARDENTREE01A_SM_YSI
  178  MAP_F37B6FA03479_BG_ANH_TREE_MAPLETREE01E_SM_YSI
```

이 17개가 foliage 배치 29,426개 중 약 71%를 덮는다.
전체 273개 foliage asset을 한 번에 바꾸지 않는 이유는, 양면 전환이 픽셀 비용을
정확히 두 배로 만들기 때문이다. 상위군으로 효과와 비용을 먼저 실측한다.

### 6.3 성능 경계 — 반드시 함께 확인할 것

양면 렌더링은 foliage overdraw를 두 배로 만든다.
G2 적용 뒤 F1 Diagnostics의 smoothed FPS와 최근 frame time을 적용 전후로 기록한다.
프레임 시간이 눈에 띄게 악화되면 상위 5개만 남기고 나머지를 되돌린다.

---

## 7. G3 — 물·거울 재질 profile

### 7.1 대상

```text
9개  MAP_2D6A6B1587D0_BG_RHD_BREEZE_RUMINAWATER01_SM_ASJ   (분수 광장 패널)
1개  MAP_5387B1504BDD_LV_BER_BERNCASTLE_WATER01_SM         (중앙 물 평면)
3개  MAP_7BA4AC84CD3A_LV_MODULE_WATER02_512
```

`MAP_43A8B6503530_LV_MODULE_WATER01_1024` 8개는 이미 visible=0이므로 건드리지 않는다.

### 7.2 원본 재질에서 지금 반영 가능한 값

받은 진단이 인용한 원본 파라미터 중 현재 계약으로 표현 가능한 것만 옮긴다.

```text
specular_intensity = 10    -> specularIntensity 10
specular_power     = 100   -> specularPower 100
푸른 색상                  -> colorTint
reflection_intensity 0.05, reflection_contrast 1, texture_reflection ambientreflection_09
                           -> 표현할 슬롯 없음. G5로 이월
```

`specularIntensity`는 상한이 없고 `specularPower`는 1 이상이면 되므로
IsValidRenderProfile과 render_profile_text 양쪽을 통과한다.

### 7.3 G2 + G3 통합 교체본

파일: `Data/Maps/Imported/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.renderprofiles.json`
작업: 파일 전체 교체 (기존 sky_mirror_sm 항목은 그대로 보존한다)

```json
{
  "schemaVersion": 1,
  "areaId": "LV_BER_BERNCASTLE",
  "profiles": [
    {
      "assetId": "MAP_EDDEDF2CF6A1_SKY_MIRROR_SM",
      "renderMode": "Sky",
      "cullMode": "None",
      "colorTint": [0.12, 0.14, 0.18, 1.0]
    },
    {
      "assetId": "MAP_2D6A6B1587D0_BG_RHD_BREEZE_RUMINAWATER01_SM_ASJ",
      "renderMode": "Opaque",
      "cullMode": "Back",
      "specularIntensity": 10.0,
      "specularPower": 100.0,
      "colorTint": [0.62, 0.78, 1.0, 1.0]
    },
    {
      "assetId": "MAP_5387B1504BDD_LV_BER_BERNCASTLE_WATER01_SM",
      "renderMode": "Opaque",
      "cullMode": "Back",
      "specularIntensity": 8.0,
      "specularPower": 90.0,
      "colorTint": [0.66, 0.8, 1.0, 1.0]
    },
    {
      "assetId": "MAP_7BA4AC84CD3A_LV_MODULE_WATER02_512",
      "renderMode": "Opaque",
      "cullMode": "Back",
      "specularIntensity": 8.0,
      "specularPower": 90.0,
      "colorTint": [0.66, 0.8, 1.0, 1.0]
    },
    { "assetId": "MAP_41CB42CA5056_BG_ATT_FOLIAGE_TOTGRASS03_SM_KSY", "renderMode": "Opaque", "cullMode": "None" },
    { "assetId": "MAP_F114BB99842A_LV_ATM_LOGHILL_FOLIAGE01_SM", "renderMode": "Opaque", "cullMode": "None" },
    { "assetId": "MAP_DAD60E49AC6A_BG_FOLIAGE_COMMONPLANT03_SM_KSY", "renderMode": "Opaque", "cullMode": "None" },
    { "assetId": "MAP_9976C66E4100_BG_ANH_FOLIAGE_TE_F30_SM_OLD_01", "renderMode": "Opaque", "cullMode": "None" },
    { "assetId": "MAP_E4BA13439D67_BG_ATM_TREE_GARDENTREE01C_SM_YSI", "renderMode": "Opaque", "cullMode": "None" },
    { "assetId": "MAP_A7CE965CE471_BG_ATM_TREE_GARDENTREE01D_SM_YSI", "renderMode": "Opaque", "cullMode": "None" },
    { "assetId": "MAP_2B73F7F74F45_BG_BER_COMMON_BERIVY01D_SM_KSY", "renderMode": "Opaque", "cullMode": "None" },
    { "assetId": "MAP_9A85926B83B3_BG_ATM_FOLIAGE_IVY11_SM_KSY", "renderMode": "Opaque", "cullMode": "None" },
    { "assetId": "MAP_2BB656F8F483_BG_ATM_FOLIAGE_IVY12_SM_KSY", "renderMode": "Opaque", "cullMode": "None" },
    { "assetId": "MAP_0B39DAEA62CA_LV_ATM_LOGHILL_FOLIAGE02_SM", "renderMode": "Opaque", "cullMode": "None" },
    { "assetId": "MAP_BCD69ACF8725_BG_BER_COMMON_BERIVY03C_SM_KSY", "renderMode": "Opaque", "cullMode": "None" },
    { "assetId": "MAP_97F1B98D54FE_BG_ANH_FOLIAGE_TE_F26_SM_YSI", "renderMode": "Opaque", "cullMode": "None" },
    { "assetId": "MAP_A4E9B7719B98_BG_RHD_TREE_MAPLE02C_SM_KSY", "renderMode": "Opaque", "cullMode": "None" },
    { "assetId": "MAP_79EB8ED6D468_BG_ATM_FOLIAGE_IVY05_SM_KSY", "renderMode": "Opaque", "cullMode": "None" },
    { "assetId": "MAP_5C49D0E5A7F2_BG_BER_COMMON_BERIVY03A_SM_KSY", "renderMode": "Opaque", "cullMode": "None" },
    { "assetId": "MAP_B852510B5B87_BG_ATM_TREE_GARDENTREE01A_SM_YSI", "renderMode": "Opaque", "cullMode": "None" },
    { "assetId": "MAP_F37B6FA03479_BG_ANH_TREE_MAPLETREE01E_SM_YSI", "renderMode": "Opaque", "cullMode": "None" }
  ],
  "visibilityOverrides": []
}
```

### 7.4 재빌드·publish 순서와 선행 확인

renderprofiles.json은 shard 카탈로그 생성 입력이므로 shard를 다시 만들어야 한다.

```powershell
python Tools/LevelPlacementExtractor/build_bern_castle_shards.py --help
powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE
```

`build_bern_castle_shards.py`는 원본 UE3 추출 manifest를 입력으로 요구한다.
그 입력이 로컬에 없으면 G2/G3는 shard 재생성 없이 적용할 수 없다.
그 경우 대안은 `.mapassets` 행 꼬리 컬럼을 직접 치환하는 것이지만,
`Data/Maps/Imported`는 추출 기준본이므로 임의 편집하지 않고 사용자에게 먼저 확인한다.
G2/G3 착수 전에 `--help` 출력으로 필요한 manifest 경로를 먼저 확정한다.

### 7.5 검증

```text
1. JSON parse
2. shard 재생성 후 .mapassets 행에 None / 10 / 100 이 실제로 들어갔는지 grep
3. Publish-MapAuthoring 성공, 중간 실패 시 전체 rollback 확인
4. Client Bern 진입, 카탈로그 로드 실패 로그 없음
5. F1 Diagnostics frame time 적용 전후 기록
6. 사용자 육안 판정
```

---

## 8. G4 — 베른 point light 레이어

### 8.1 반드시 지킬 상한

```text
타입   POINT 만  (Presentation_Manager.cpp:302)
개수   64 이하   (MapLightDocument.h:29, Publish-MapAuthoring.ps1:172, Presentation_Manager.cpp:8)
비용   광원 1개 = 전체 화면 블렌드 1패스
```

원본 326개 중 Point 279개에서 광장·정문 시야에 실제로 기여하는 64개 이하를 고른다.
선정 기준은 카메라가 실제로 가는 영역과의 거리이며, 선정 근거를 문서 provenance에 남긴다.

Spot 32개, Dominant Spot 11개, Sky Light 1개는 이 G의 범위가 아니다.
`enum class LIGHT`에 타입을 추가하는 것은 Engine public header 변경이고
`UpdateLib.bat` 재실행과 Client 전체 재빌드, deferred 조명 셰이더 변경을 동반한다.

### 8.2 문서 형식

발탄 문서와 같은 `lostark.map-light-presentation` v1을 그대로 쓴다.

파일: `Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.maplights.json`

```json
{
  "schema": "lostark.map-light-presentation",
  "formatVersion": 1,
  "areaId": "LV_BER_BERNCASTLE",
  "provenance": "SOURCE_INSTANCE_SELECTED_COURTYARD_SUBSET",
  "lights": []
}
```

lights 배열의 각 항목 형식은 다음과 같다.

```json
{
  "lightId": "light.bern.courtyard.pointlight_NNN",
  "sourceLevel": "LV_BER_BERNCASTLE_T_SL06",
  "sourceObjectId": "SL06:export:NNN:원본이름",
  "position": [0.0, 0.0, 0.0],
  "radiusMeters": 9.0,
  "falloffExponent": 2.0,
  "color": [1.0, 0.85, 0.6, 1.0],
  "brightness": 4.0
}
```

provenance 문자열은 값의 출처를 정확히 밝혀야 한다.
발탄이 `SOURCE_INSTANCE_EXACT_FALLOFF_INFERRED`를 쓰는 이유는
falloff가 원본 행에 직렬화되지 않아 class default에서 추론했기 때문이다.
베른도 추론한 필드가 있으면 문자열에 드러낸다.

### 8.3 MapCatalog pair

파일: `Data/Maps/MapCatalog.json`
작업: LV_BER_BERNCASTLE 객체에 두 줄 추가
기준점: `"sourcePlacements"` 바로 아래에 sourceLights, `"catalog"` 바로 아래에 lights

```json
      "sourceLights": "Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.maplights.json",
      "lights": "Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE.maplights.json",
```

`Publish-MapAuthoring.ps1:262~266`이 이 두 경로 문자열을 정확히 대조하므로
경로를 한 글자도 다르게 쓰면 publish가 실패한다.

### 8.4 Level_Bern 연결

`Client/Private/Level_ValtanArena.cpp:217~227`과 동일한 계약을 따른다.

파일: `Client/Public/Level_Bern.h`
작업: 멤버 추가
기준점: 기존 `CMapPlacementRuntime m_MapRuntime;` 선언 바로 아래
추가 대상: `shared_ptr<class CMapLightPresentationRuntime> m_pMapLightPresentation;`
필요한 이유: provider 등록이 유지되려면 Level이 소유해야 한다.

파일: `Client/Private/Level_Bern.cpp`
작업: 블록 추가
기준점: `m_MapRuntime.Load_Area(...)` 성공 분기 직후, `Ready_Layer_Camera` 호출 바로 위
연결되는 부분: CPresentation_Manager::Add_TransientLight -> CLight_Manager::Render_Lights

```cpp
	auto mapLightPresentation = make_shared<CMapLightPresentationRuntime>();
	if (!mapLightPresentation->Load_Runtime(pEntry->pMapAreaId))
	{
		OutputDebugStringA((
			"[Level_Bern][MapLight] " +
			mapLightPresentation->Get_Status() +
			"\n").c_str());
		m_MapRuntime.Clear();
		return E_FAIL;
	}
	m_pMapLightPresentation = std::move(mapLightPresentation);
```

소멸·레벨 이탈 경로에서 `m_pMapLightPresentation->Clear()`와 `reset()`을
발탄과 같은 순서로 호출한다. 정확한 삽입 위치는 구현 직전에
Level_Bern.cpp의 현재 실패 rollback 흐름을 다시 읽고 확정한다.

Level_Bern.cpp는 기존 파일이므로 편집 전에 파일별 인코딩을 감지하고 그대로 보존한다.
한글 주석이 있으면 CP949일 수 있으므로 Edit 도구 대신 바이트 단위 편집을 사용한다.

### 8.5 검증

```text
1. maplights.json parse
2. powershell Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE
   -> pair 미선언 / 경로 불일치 / 65개 이상에서 실패하는지 음성 케이스 확인
3. Engine public header 변경 없음 -> UpdateLib 불필요, Client 재빌드만
4. Bern 진입 후 F1 Rendering 패널의 Last submitted Light 수가 문서 개수와 일치
5. maplights.json을 일부러 손상시켜 Bern 진입이 fail-closed 되는지 확인
6. 사용자 육안 판정
```

---

## 9. G5·G6 — 보류 항목과 보류 이유

### G5 환경 반사 슬롯

필요한 변경 범위:

```text
Tools/BernCastlePipeline/build_bern_castle_assets.py        texture_reflection 역할 추가
ModelAssetConverter                                         reflection remap 스위치
.wmodel 머티리얼 슬롯                                        신규 텍스처 타입
CModel / CMaterial                                          슬롯 로드
MAP_ASSET_RENDER_PROFILE                                    reflectionIntensity / contrast
.mapassets 행 형식 + CATALOG_VERSION                         컬럼 추가
Shader_VtxMeshBinary.hlsl / Shader_VtxMeshMapInstance.hlsl   큐브맵 샘플러와 반사 항
```

`.mapassets` 컬럼을 늘리면 CATALOG_VERSION이 올라가고 모든 Area의 카탈로그를
다시 생성해야 한다. 베른뿐 아니라 발탄·수련장·Character Select까지 영향을 받는다.
G1~G4 결과를 사용자가 판정한 뒤에 착수 여부를 결정한다.

### G6 안개·바람

현재 Engine에 height fog나 wind를 소비하는 계약이 없다.
안개는 `Shader_Deferred.hlsl` 최종 합성 단계에 depth 기반 항을 추가하는 신규 계약이며,
바람은 foliage 정점 애니메이션이므로 `Shader_VtxMeshMapInstance.hlsl`의 VS와
per-asset 파라미터가 함께 필요하다. 둘 다 소비자를 먼저 설계해야 한다.

### 라이트맵

원본 lightmap/shadowmap UV와 텍스처는 현재 `.wmodel` 형식에 슬롯이 없다.
G1의 방향광 그림자와 G4의 point light로 얻는 결과를 먼저 판정한 뒤,
그래도 부족하면 별도 수직 슬라이스로 계획한다.

---

## 10. 이 계획이 지키는 프레임워크 경계

- 같은 역할의 두 번째 런타임 경로를 만들지 않는다. 재질 조정은 기존
  renderprofiles.json -> .mapassets -> MAP_ASSET_RENDER_PROFILE 하나만 쓴다.
- MAP_PLACEMENT_RECORD와 placement 저장 형식을 바꾸지 않는다.
- `Client/Bin/DataFiles`는 publisher만 교체한다. 직접 편집하지 않는다.
- `Data/Maps/Imported`는 추출 기준본이다. renderprofiles.json은 원래 그 폴더에 있는
  override 문서이므로 예외지만, `.mapassets` 자체를 손으로 고치지 않는다.
- Engine public header는 G4까지 건드리지 않으므로 `UpdateLib.bat` 재실행이 없다.
- `.vcxproj`와 `.filters`는 변경하지 않는다. 신규 C++ 파일이 없다.
- 시각 판정은 전부 사용자가 한다. 에이전트는 빌드·parse·publish·수치 진단까지만 한다.

---

## 11. 구현 상태

| 항목 | 상태 |
|---|---|
| 현황 실측 | 완료 (2026-08-25) |
| 받은 8단계 제안 재현·판정 | 완료 |
| G1 데이터 변경 | 미적용 — 사용자 승인 대기 |
| G2/G3 데이터 변경 | 미적용 — shard 재생성 입력 확인 필요 |
| G4 Client 변경 | 미착수 |
| G5/G6 | 보류 |
| 자동 검증 | 미실행 |
| 사용자 육안 판정 | 미실행 |
