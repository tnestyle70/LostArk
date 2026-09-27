# 2026-09-26 마하라카 영상 결함 3건 수정 RESULT

> 09-26 후속 원본 재조사: 아래 최초 보고의 **지형 복원 완료**, **모코모코 모델 부재/삭제**
> 결론은 유지하지 않는다. headroom은 표시용 근사이며, MN_ISMP_00 모델·클립·배치가
> 실제 원본에서 발견됐다. 현재 반영/미완료 상태는 이 문서 끝의 G05를 기준으로 한다.

대상 Area: `LV_OCN_EVENTIS_MHP` (2021 마하라카 파라다이스 섬), 브랜치 `codex/main-ship-maharaka-0926`, HEAD `a84bbcd4`.

사용자가 제시한 3건:

1. 카메라 보는 방향으로 물에 하얀 것이 비친다 → 원인 찾고 안 나오게 수정
2. 바닥에 노란색·초록색 재질이 이상하게 나온다 (직전 수정에서 전혀 안 고쳐졌다) → 원본 확인 후 복원 또는 제거
3. 밖의 바다는 괜찮은데 워터팡 아레나·레인보우 익스프레스 쪽 물이 제대로 안 나온다, 모코모코 어트랙션은 아예 없는 것 같다 → 원본 데이터 전체를 다시 읽고 원인 규명 후 해결

---

## 1. 바닥 노랑·초록 (항목 2) — 원인 확정 후 수정, 설치 완료

### 소비 경로 증거 (직전 F fork 실패 반복 방지)

지형 타일의 diffuse 입력이 무엇인지 코드·파일로 확정했다.

- 설치된 타일 폴더에는 `baked_diffuse.png`, `baked_normal.png`, 그리고 레이어 원본 DDS 2개만 있다.
  `baked_diffuse.dds`는 **존재하지 않는다.**
  `Client/Bin/Resources/Map/LV_OCN_EVENTIS_MHP_LAND/Landscape/MAP_19DDE51893D1_LAND01_LC_01170/textures/`
- 그 타일의 `.wmodel`(`WINT` v1) 안의 UTF-16LE 텍스처 경로 문자열은 정확히 4개이고,
  그중 diffuse는 `textures/baked_diffuse.png`다.
- 로더: `Engine/Private/Material.cpp:268-281` 이 확장자를 소문자로 비교해 `.dds`면 DDS 로더, `.tga`면 TGA
  로더, **그 외는 `CreateWICTextureFromFileEx`** 로 읽는다. 색 슬롯은 `WIC_LOADER_FORCE_SRGB`.
  → PNG가 실제로 읽히는 입력이다.
- `mapmaterials.json`에는 `LAND01` 자산 행이 0개다. 즉 지형은 재질 override 없이 WModel 내부
  텍스처 참조만 사용하므로 재질 문서를 고치는 것은 지형에 영향이 없다.

### 근본 원인

`Data/Maps/Imported/...` 가 아니라 굽기 도구 쪽이 원인이다.
원본 마스터 재질
`C:\LostArkExtract\LV_OCN_EVENTIS_MHP_20260919\Landscape\SourceRaw\MasterMaterial\master_material.json`
(`lv_ocn_eventis_mhp_land_01_mi`, parent `efmaster_material_lv_prologue.landscape.landscape_base`)의
레이어 값:

| 레이어 | brightness | color | 텍스처 |
|---|---|---|---|
| layer03 | 4.0 | (1.000, 0.684, 0.094) 주황·노랑 | `lv_common_ground_06_d.dds` |
| layer04 | 4.0 | (0.075, 0.646, 0.149) 초록 | `lv_common_ground_06_d.dds` |
| layer01/02/05/06/cliff | ≤1.0 | 모래·흙 계열 | 각 sand 텍스처 |

원본은 HDR 조명 패스에서 1.0을 넘는 값을 다시 톤매핑으로 압축한다. 우리 굽기는 평범한 8bit
albedo PNG를 쓰므로 1.0 초과분이 전부 바이트 clamp에 날아가서, brightness 4.0 레이어의 텍셀
대부분이 같은 천장에 붙고 **질감이 사라진 형광 단색 덩어리**가 된다. layer03은 노랑, layer04는
초록 — 사용자가 본 그 두 색이 정확히 이 두 레이어다.

직전 F fork는 `adjusted_layer_diffuse`에 레이어별 hard clamp(`min(1, max(0, x))`)를 넣었는데,
clamp는 포화된 **색상비**(1.0, 0.68, 0.09)를 그대로 보존하므로 형광이 그대로 남았다. 실측으로
확인했다(아래 표의 "이전" 열).

### 수정

`Tools/LandscapeExtractor/extract_ue3_landscape.py` 세 군데:

1. `layer_headroom_scale(layer)` 신규 함수 — 해당 레이어 자기 텍스처의 모든 텍셀에
   desaturation·brightness·color를 적용했을 때의 최대 채널값 `peak`을 구하고, `peak>1`이면 `1/peak`을
   반환한다(그 이하면 1.0).
2. `build_layer_sources` 반환 직전에 각 레이어에 `headroomScale`을 채운다.
3. `adjusted_layer_diffuse`의 clamp를 `min(1, max(0, unclamped[c] * scale))`로 바꾼다.

효과: 레이어가 저작한 **색조는 유지**하면서 최상위 텍셀이 정확히 1.0에 놓이므로 clamp로 사라지던
질감이 전부 살아난다. 이미 범위 안인 레이어는 scale=1.0으로 **바이트 단위 무변경**이다.

계산된 headroomScale: layer01 1.0, layer02 0.6711, layer03 0.4554, layer04 0.6945, layer05 1.0,
layer06 1.0, layercliff 1.0.
보정 후 평균 albedo: layer02 (0.34,0.45,0.12), layer03 (0.66,0.43,0.06), layer04 (0.08,0.62,0.14).

파일은 LF 전용·BOM 없음을 유지했고 3929 → 3957줄. 기존 단위 테스트
`python -m unittest test_extract_ue3_landscape` 23건 통과.
백업 `out/MaharakaFix20260926/backup/extract_ue3_landscape.py.before-headroom`.

### 재굽기 후 실측 (형광 픽셀 비율 %, 이전 → 이후)

판정 기준: 노랑 `r>=200 and g>=130 and b<=90`, 초록 `g>=150 and r<=110 and b<=110`,
클립 `채널 중 하나가 정확히 255`.

| 타일 | 노랑 | 초록 | 255 클립 | 평균 RGB |
|---|---|---|---|---|
| LC_01170 | 10.17 → **0.27** | 14.78 → **9.64** | 7.82 → **0.00** | (140,163,105) → (128,138,100) |
| LC_01173 | 15.10 → **0.42** | 4.40 → **2.88** | 13.01 → **0.00** | (156,157,110) → (142,134,106) |
| LC_01169 | 4.95 → **0.14** | 0.01 → 0.06 | 3.98 → **0.00** | (149,143,120) → (144,135,119) |
| LC_01171 | 0.00 → 0.00 | 0.89 → 0.58 | 0.10 → 0.00 | (144,139,128) → (144,138,127) |
| LC_01175 | 0.00 → 0.00 | 0.22 → 0.11 | 0.02 → 0.00 | 무변화 |
| LC_01180 | 0.14 → 0.00 | 0.03 → 0.01 | 0.04 → 0.00 | 무변화 |
| LC_01172 / 01179 | 0.00 | 0.00~0.01 | 0.00 | 무변화 |
| 나머지 정상 8타일 | 0.00 | 0.00 | 0.00 | **바이트 동일, 색 안 죽음** |

- 16타일 전부에서 **255 클립이 0.00%로 소멸**했다.
- 더 나빠진 타일 없음.
- 정상이던 8타일은 diffuse가 바이트 단위로 동일해 교체 대상에서 제외됐다.

### 정직한 한계 (사용자 판정 필요)

- LC_01170의 남은 "초록 9.64%"는 형광 clip이 아니라 **layer04가 원본에서 실제로 저작한 초록 색조**다.
  채도 P90은 이전 0.88 / 이후 0.88로 같고, 달라진 것은 clip 소멸과 질감 복원이다. 즉 그 구역은
  여전히 초록·노랑으로 보인다. 원본 마스터가 그 두 레이어를 brightness 4.0 + 노랑/초록 tint로
  저작했기 때문이며, 이것이 "복원"의 정답이다.
- 사용자가 색 자체를 원하지 않으면 두 번째 선택지(주변 바닥으로 덮어 없애기)를 별도로 지시해야
  한다. 그건 원본 복원이 아니라 제거이므로 승인 없이 하지 않았다.

### 교체·백업

교체된 파일 8개 (`baked_normal.png`은 변화 없어 미교체):

```
Client/Bin/Resources/Map/LV_OCN_EVENTIS_MHP_LAND/Landscape/<타일>/textures/baked_diffuse.png
  MAP_0AFFAE225908_LAND01_LC_01172   70792 -> 70750
  MAP_19DDE51893D1_LAND01_LC_01170  116788 -> 115552
  MAP_4CBD4C4B4315_LAND01_LC_01175   66912 -> 66781
  MAP_86DAC42217AC_LAND01_LC_01173   99693 -> 99598
  MAP_A6038D5C62B9_LAND01_LC_01179   76624 -> 76611
  MAP_B239E97CFE1A_LAND01_LC_01171   68439 -> 68321
  MAP_E082280F30AF_LAND01_LC_01180   79522 -> 79212
  MAP_F4D937137796_LAND01_LC_01169  104733 -> 104594
```

백업: `out/MaharakaFix20260926/backup/landscape_tiles_before_headroom/<타일>/baked_diffuse.png`

LAND01 타일 정합성: placements가 참조하는 타일 16개 = 설치된 타일 16개, 미설치 0, 미참조 0.
재굽기는 46타일을 생성했지만 이 Area가 쓰지 않는 30타일은 설치하지 않았다.

---

## 2. 물에 비치는 흰색 (항목 1) + 아레나·슬라이드 물 (항목 3 앞부분) — 같은 원인, 수정 완료

두 증상은 **하나의 원인**이다.

### 소비 경로 증거

물 자산은 후보 경로가 두 개 있어서 어느 쪽이 실제 입력인지 먼저 확정했다.

- 후보 A: `Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.mapwater.json`
  (`lostark.map-water-presentation` v1, 10행) → `CMapAssetCatalog::Load_WaterPresentation`
  (`MapAssetCatalog.cpp:2267`) → `MAP_ASSET_WATER_PROFILE` → `Bind_WaterShaderResources`
  (`MapAssetObject.cpp:630`) → `Shader_VtxMeshBinary.hlsl` 레거시 물 패스.
- 후보 B: `mapmaterials.json`의 `family: "source.map.water-4x.v1"` 행
  → `SourceCharacterMaterial::Configure`가 `source.map.water-` 접두사를 분기
  (`Client/Public/SourceCharacterMaterialParameters.h:62`)
  → `SourceMapWaterMaterial::Configure`가 `staged.program`과 상수 슬롯을 채움
  (`Client/Public/SourceMapWaterMaterialParameters.h`)
  → `CModel::Apply_MaterialOverrides` (`Engine/Private/Model.cpp:1822`)로 모델 surface에 적용.

확정: 7개 물 자산은 `.mapassets`의 `renderMode`가 `Water`라 카탈로그 profile은 WATER지만,
`CMapAssetObject::Get_MaterialRenderProfile` (`MapAssetObject.cpp:38-61`)이 **재질 surface의
renderMode로 덮어쓴다.** 우리 물 행은 `renderMode: "translucent"`이므로 profile은 TRANSLUCENT가
되고, `MapAssetObject.cpp:249`의 `bWater = (renderMode == WATER && m_bHasWaterProfile)`가 false가
되어 **레거시 물 패스는 실행되지 않는다.** 대신 `MapAssetObject.cpp:161-164`가
`family == SOURCE_CHARACTER && program 38..43`를 보고 `Request_SceneColorSnapshot()`을 호출하고,
`Bind_Material`이 그 program 상수를 바인딩한다.

→ **후보 B(mapmaterials)가 실제 소비 경로**이고, 이 7개에 대한 `mapwater.json` 행은 불활성이다
(다만 `MapAssetCatalog.cpp:2434-2448`의 양방향 검사 때문에 행 자체는 남겨야 한다).

### 근본 원인

`Tools/LevelPlacementExtractor/author_ocean_water_rows.py`가 7개 행을
`FAMILY = 'source.map.water-41.v1'`로 손저작했다. 이 스크립트 docstring이 스스로 밝히고 있다.

> the ocean master (specialresource.mat.ocean_trn) has no native program in this project …
> so the ocean MICs are expressed through source.map.water-41.v1 as a **PROJECT APPROXIMATION**

그 전제가 틀렸다. `ocean_trn`의 정확한 program은 이 프로젝트에 **이미 있다**: `water-42`.
증거 — 베른의 `MAP_6D4F71329FE9_BG_SCD_RHD_FLOOR07_SM_KHB` 행이 `source.map.water-42.v1`이고,
그 MIC 체인(`bg_yor_ugarden_canal01_02_mi_khg` → `river_water_mi` → `ocean_trn`)의 마스터 props
파일이 마하라카 3개 MIC와 **같은 파일**이다(`A643D1D45E9B__ocean_trn.props.txt`).
또 `water-42`는 `ocean_trn`의 `sky_color / sky_power / sky_intensity / sky_tiling_panning /
fresnel_color / fresnel_tiling / reflection_power / diffuse_tiling_panning /
distortion_intensity / mask_distortion_intensity / diffuse_saturation`를 소비하는 유일한 family다.

41로 근사하면서 스크립트가 한 짓(docstring에 명시됨):

- `reflection_power`(물03 = 1.5), `sky_*`, `fresnel_color`, `fresnel_tiling`,
  `distortion_intensity`, `mask_distortion_intensity`, `diffuse_saturation`를 **버렸다**.
- `diffuse_color ← sky_color × sky_intensity` 로 **바꿔 넣었다**.
- `reflection_color ← fresnel_color` 로 **바꿔 넣었다**.

결과:

- 수영장 물 `water03_mi`는 `fresnel_intensity`가 0이라 `reflection_color`가 원본값
  (0.867, 0.873, 1.000) — **거의 흰색** — 그대로 들어갔고, `reflection_intensity`는 원본의 20이
  그대로 들어갔다. 그런데 41 program에는 그 20을 눌러주는 `reflection_power`(1.5)도, 하늘·프레넬
  혼합도 없다. 시선 의존 항에 거의 흰색 × 20 → **카메라 방향으로 따라오는 하얀 번짐**.
- 바깥 바다 `water_mi_01 / water_mi_03`은 `fresnel_intensity`가 1200으로 활성이라
  `reflection_color`에 파란 `fresnel_color`(0, 0.111, 0.736)가 들어갔고 `reflection_intensity`가
  0.5뿐이어서 **우연히 그럴듯하게 보였다.**

즉 사용자가 관찰한 "바깥 바다는 괜찮고 아레나·슬라이드 물만 이상하다"가 이 분기와 정확히 일치한다.

참고로 그 program의 `texture_diffuse` 기본값은 `fx_c_water_001`(흰 거품)이고, 물 메시 WModel의
diffuse도 같은 파일이다. 레거시 물 패스(`Shader_VtxMeshBinary.hlsl`)는 그 거품을 수면 전체에
diffuse로 그리는 알려진 결함이 있는데, 위에서 확인한 대로 우리 7개 자산은 그 패스를 타지 않는다.

### 수정

`Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapmaterials.json`의 물 7행을
`source.map.water-41.v1` → `source.map.water-42.v1`로 재저작하고, 원본 props에서 읽은 값을 넣었다.

- 파라미터 30개 — MIC override는 원본 `.props.txt` 값 그대로, 미지정 항목은 `ocean_trn` 마스터의
  `CollectedScalarParameters` / `CollectedVectorParameters` 기본값(예 `mask_distortion_intensity`
  -0.007, `diffuse_saturation` 1.0, `diffuse_tiling_panning` [1,1,0.1,0.1]). `selectioncolor`는
  베른 검증행과 동일하게 [0,0,0,1].
- 텍스처 7개 — `expressionIndex` 순서는 베른 검증행의 MIC 체인을 해석해 1:1로 확정했다.
  `0 texture_normal(linear) / 1 detail_texture_normal(linear) / 2 texture_sky / 3 texture_reflection /
  4 texture_fresnel / 5 texture_diffuse / 6 texture_diffuse_mask`.
  7개 물 자산 폴더에 필요한 DDS 8종이 이미 전부 설치되어 있어 각 자산 자기 폴더를 참조한다.
  스크립트가 파일 실재를 assert로 검사했다.
- `renderMode: translucent`, `cullMode: back` 유지.

| 자산 | 원본 MIC | 위치 | 용도 |
|---|---|---|---|
| `MAP_7BA4AC84CD3A_LV_MODULE_WATER02_512_OVR_53F9EEC1FCA4` | water03_mi | (75.0, 20.2, -991.0) | 워터팡 아레나 수면 |
| `MAP_7BA4AC84CD3A_LV_MODULE_WATER02_512_OVR_E67B61F41453` | water_mi_03 | (75.0, 20.2, -991.0) | 같은 자리 하층 |
| `MAP_7BA4AC84CD3A_LV_MODULE_WATER02_512_OVR_3CB26CF55D22` | water_mi_01 | (75.1, 19.6, -993.3) scale 100 | 바깥 바다 |
| `MAP_3394CA48094A_..._WATER02_SM_OVR_53F9EEC1FCA4` | water03_mi | (84.5, 30.3, -956.1) | 레인보우 익스프레스 슬라이드 물 |
| `MAP_EB275526D3D5_..._FLOOR02_SM_OVR_53F9EEC1FCA4` | water03_mi | (75.0, 31.2, -990.6) | 상단 수조 |
| `MAP_C1B966AD13A1_..._WATER03_SM_OVR_53F9EEC1FCA4` | water03_mi | (77.3, 20.8, -961.1) | 수영장 |
| `MAP_EA9C52CC4464_LV_BER_BERNILF_FLOOR01_SM_OVR_53F9EEC1FCA4` | water03_mi | (48.1, 20.6, -1013.9) | 수영장 |

### program 41 과 42 는 상수 레이아웃이 다르다 (핵심 증거)

`Client/Bin/ShaderFiles/Shader_SourceMapWaterPrograms.hlsli` 는 원본 컴파일 셰이더를 옮긴
어셈블리형 HLSL 이다. `SourceMapWater42`(1137행~)와 `SourceMapWater42Baked`(2607행~)가 program 42,
`SourceMapWater41` 이 program 41 이고, `Shader_SourceMapForwardPrograms.hlsli:940` 이
`case 42u: g_HasBakedLighting!=0 ? SourceMapWater42Baked : SourceMapWater42` 로 분기한다.

사용하는 cb0 슬롯을 세어 보면

- `SourceMapWater41` : `source[1] .. source[13]`
- `SourceMapWater42` : `source[1] .. source[20]`

즉 두 program 의 상수 레이아웃이 다르다. `SourceMapWaterMaterialParameters.h` 도 41 은
`baseConstants[1..10]`, 42 는 `baseConstants[1..16]`(baked 는 33..48)을 채운다.
ocean_trn 의 값 30개를 41 로 보내면 **레지스터가 어긋난 채 해석된다**. 그래서 근사가
"파라미터 몇 개를 버린" 수준이 아니라 값이 다른 뜻으로 읽혔다.

반대로 42 는 원본 ocean_trn 컴파일 셰이더의 번역이므로, 원본 MIC 값을 그대로 넣는 것이
정의상 원본 출력을 재현하는 방법이다.

### 계약 검증

`SourceMapWaterMaterialParameters.h` 끝의 `if(!valid || consumed.size()!=parameters.size())return false;`
때문에 파라미터 집합이 정확히 일치해야 한다. 헤더의 water-42 블록에서 `parameter("…")` 이름을
추출해 대조한 결과 **30개, 7행 모두 정확히 일치**하고 베른 검증행과 키 집합이 동일하다.
(이 검사는 실패 시 재질 행 거부 → Area 로드 실패로 이어지므로, 만약 마하라카 진입이 실패하면
`out/MaharakaFix20260926/backup/LV_OCN_EVENTIS_MHP.mapmaterials.json.before-water42`로 되돌리면 된다.)

백업: `out/MaharakaFix20260926/backup/LV_OCN_EVENTIS_MHP.mapmaterials.json.before-water42`

### 재발 방지

`Tools/LevelPlacementExtractor/author_ocean_water_rows.py`는 손대지 않았다(이 작업의 요청 범위가
아니고, 파이프라인 단계도 아닌 1회성 저작 도구다). **다시 실행하면 이 수정이 water-41로 되돌아간다.**
그 스크립트의 `FAMILY = 'source.map.water-41.v1'`와 "ocean_trn has no native program" 전제가 틀렸다는
사실을 `gotchas.md`에 남길 것을 권한다.

---

## 3. 모코모코 어트랙션 (항목 3 뒷부분) — 원본에 없다. 우리가 빠뜨린 것이 아니다

사용자 관찰: "모코모코 어트렉션은 아예 너가 만들지도 않은거같은데".
조사 결과 **2021 MHP 레벨 패밀리 전체에 그 조형물 static mesh 가 존재하지 않는다.**

### 원본 배치 전수 토큰 검색 (직접 재현 확인)

`C:\LostArkExtract\LV_OCN_EVENTIS_MHP_20260919\Placements\*.placements.json` 3파일 전문에서
대소문자 무시로 센 결과:

| 토큰 | 건수 |
|---|---|
| moko / mococo / mcc | 0 / 0 / 0 |
| arena / pang / attract / cannon | 0 / 0 / 0 / 0 |
| coaster / carousel / ferris / rainbow / express | 0 / 0 / 0 / 0 / 0 |
| **slide** | **6** (자산 3종 × 2) |

주의: `bg_att_*` / `lv_att_*` 의 `att` 는 attraction 이 아니라 **아르데타인 대륙 코드**다
(`bg_att_eichmannl`, `bg_att_templet`, `lv_att_stern`). 어트랙션 근거로 쓸 수 없다.

### 레인보우 익스프레스는 이미 들어와 있다 (직접 확인)

`slide` 6건의 실체는 `bg_ocn_etc_g.mesh.bg_ocn_etc_decoslide01a/b/c_sm` 3개이고,
우리 저장소 배치에 같은 좌표로 **이미 있다.**

```
MAP_ED87CF80D720_BG_OCN_ETC_DECOSLIDE01A_SM_OVR_1D8FC6CEB1F9  (72.67, 20.31, -965.84)
MAP_A0D8DF5ADD24_BG_OCN_ETC_DECOSLIDE01B_SM                   (75.12, 20.31, -965.84)
MAP_713DC328BE42_BG_OCN_ETC_DECOSLIDE01C_SM                   (77.57, 20.31, -965.84)
```

`.wmodel` 도 세 폴더 모두 설치되어 있다. x 좌표가 선행 문서
`2026-09-26_MAHARAKA_SOURCE_FUNCTIONS_REPORT.md` 의 TrackMove 1~3번(72.63 / 75.10 / 77.54)과
4cm 이내로 일치한다. 슬라이드 상단 부속물(`bg_att_etc_roof01c_sm_ysi` 3, `lv_att_stern_fence02_sm` 6,
`ehm_pipe_a13/a14` 8)도 같은 x 대에 있다.

### 누락 배치는 어트랙션이 아니다

원본 4,149행 vs 저장소 4,651행. 원본에 있고 저장소에 없는 것은 **asset 9종 / 326행**이고,
`admission.exclusions.json` 수치(243 + 80 + 3 = 326)와 정확히 일치하며 그 외 누락은 없다.

| 레벨 | asset | 행 |
|---|---|---|
| PS | `lv_navimesh...cul_box_1 / _4 / _7 / _8` | 171 / 56 / 13 / 2 (컬링 박스) |
| PS | `lv_ocn_eventis_mhp_pool04_sm` | 1 (source-hidden) |
| LAND01 | `bg_rhd_stone_rock01_sm_ksr` / `bg_fat_stone_rock02_sm` / `rock01_sm` | 59 / 12 / 9 (레이싱 쪽) |
| SL01 | `bg_tot_movillage_decoprop07f_sm_artree` | 3 (glTF basis 퇴화) |

어트랙션 계열은 누락 0: `decoslide01a/b/c` 3/3, `gatedeco` 12/12, `gate01` 1/1,
`elevator*` 6/6, `robot02` 4/4, `pool04` 81/81, `tent01/02` 16/16.

정정: PS 의 실제 제외 기준은 y 규칙이 아니라 `sourceHidden` 이다
(manifest `sourceVisiblePlacementCount: 1 / sourceHiddenPlacementCount: 243`).

### 추출하지 않은 sub-level 11개 — 섬 쪽 손실은 없다

PS 패키지 name table 로 확인한 MHP streaming sublevel 은 13개다:
`LAND01, SL01, SL02, MUSIC, SOUNDSTREAM, ENVNPC03, SCENE02A, SCENE02B, SCENE03A, SCENE03B,
SCENE04A, SCENE06A, SCENE07A`. 우리는 `PS + LAND01 + SL01` 만 가져왔다.

- `SL02` (17.6MB, exports 9,881, 배치 3,934행): source y 0~34,656cm → **전부 레이싱 트랙**,
  island scope(y>50000) 0행. 섬 쪽으로 누락된 것이 없다. `slide` 히트는 SL01과 같은 자산.
- `SCENE02A/02B/03A/03B/04A/06A/07A` + `ENVNPC03` (7KB~83KB): 메쉬 이름 0~11개
  (`bg_ocn_etc_deco01a~j`, `deco02/03/04`, `floor01a/b`)뿐이고 나머지는 컷신용 `mn_head_*`.
  **어느 scene 레벨에도 모코코 조형물 메쉬가 없다.** `mococo_cam` 은 `scene02b` 에 카메라 이름으로만
  존재하고, `scene_maharakap_waterpangstart` / `bgm_eventis_mhp_m05_scene_waterpangstart` 는
  `scene03b` 에 있다.
- `MUSIC`, `SOUNDSTREAM` 은 사운드.
- `lv_ocn_eventis_amf` / `lv_ocn_eventis_wtf` 는 **재질 패키지 참조이고 레벨 참조가 아니다.**
  `EFTable_ZoneBase` 기준 `LV_OCN_EventIS_AMF_PS`=존 50113, `WTF_PS`=57007 로 **다른 이벤트 섬**이며,
  2022·2023 재출시판 `MHP22_PS`(57017/18/19) / `MHP23_PS`(57025/26) 는 별도 레벨이다.
  설치본 33,911개 패키지에 `AMF_PS / WTF_PS / MHP22_PS / MHP23_PS` 의 `.upk` 가 **없다(삭제됨).**

### 워터팡 아레나 무대·워터캐논도 원본 메쉬가 없다

아레나 중심 proj (64.6, 19.9, -983) 반경 10m 안 배치 47개는 대부분
`lv_att_stern_fence01_sm` 링이고 **무대·원판·캐논 메쉬가 없다.**
SL01 의 움직이는 actor(`efmotionstaticmeshactor` 59 + `interpactor` 12) 전수 확인에도
회전 워터캐논 같은 메쉬가 없다.

### 결론

- **확인된 답**: 모코모코 어트랙션과 워터팡 아레나 무대·워터캐논의 static mesh 는 2021 MHP 레벨
  패밀리에 애초에 존재하지 않는다. 우리가 안 가져온 것이 아니다. 세 놀이기구 중 레인보우
  익스프레스는 이미 저장소·Resources 에 들어와 있다.
- **확인 불가 + 이유**: 남은 두 어트랙션의 형상 출처는 (a) 서버가 DeployData 로 스폰하는 Prop
  (존 57009/57011 Prop 레코드의 EFTable 정의 상당수가 삭제됨) 또는 (b) 2023 아크패스 문구가
  가리키는 별도 레벨 `MHP22_PS`/`MHP23_PS` 인데, (a) 는 모델 ID 정의가 테이블에 없고 (b) 는 해당
  `.upk` 가 설치본에 없어 둘 다 확인 불가다.

### 이 절의 검증 수준

직접 재현 확인: 토큰 전수 검색 건수, 저장소 배치의 `decoslide01a/b/c` 좌표 3개,
`admission.exclusions.json` 의 243/80/3.
미재현(조사 보고 인용): SL02 의 배치 3,934행·y 범위, SCENE 레벨 7개의 내용, name table 로
유도한 sublevel 13개 목록, `EFTable_ZoneBase` 의 존 번호, 설치본 패키지 수.

### 참고: admission 규칙 원문

`C:\LostArkExtract\LV_OCN_EVENTIS_MHP_20260919\admission.exclusions.json`:

- 규칙: `island scope = source world y > 50000 cm (Maharaka island; racing track lies near y 0..35000)`
- `LV_OCN_EVENTIS_MHP_PS`: 유지 1, 제외 243 (레이싱 트랙 y<500m 또는 source-hidden helper)
- `LV_OCN_EVENTIS_MHP_LAND01`: 유지 2, 제외 80 (레이싱 트랙)
- `LV_OCN_EVENTIS_MHP_SL01`: 유지 3820, 제외 3 (glTF normal/tangent basis 퇴화)

추출된 source level은 `PS`, `LAND01`, `SL01` 3개뿐이다 (`Placements/` 폴더 기준).

---

## 실행·검증 기록

- `python -m unittest test_extract_ue3_landscape` — 23건 PASS (LandscapeExtractor 변경 후)
- 지형 재굽기 46타일 완료, 16개 설치본과 대조 후 8개 교체
- `Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP -Mode Validate` — PASS
  (PlacementCount 4651, FileCount 5)
- `Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP -Mode Publish` — PASS
  (Sha256 `1b169867e05c982bcb164a886758af092991579d9603ec09558d1b5a921dc736`)
- 게시본 재파싱 확인: `Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.mapmaterials.json`
  water-42 7행 / water-41 0행

### 빌드 필요 여부

**C++·HLSL 변경 없음.** 변경된 것은 Python 굽기 도구, Resources PNG 8개, 저작 JSON 1개,
그리고 publisher가 생성한 런타임 JSON이다. 따라서 이번 수정만 확인하려면 재빌드가 필요하지 않고,
Client를 다시 실행하면 새 데이터·텍스처를 읽는다.

### 사용자 확인 절차

1. Client 실행 → Lobby → `Maharaka`
2. 섬 바닥에서 노랑·초록 구역이 형광 단색 덩어리가 아니라 질감 있는 흙·풀로 보이는지
3. 워터팡 아레나(대략 x 75, z -991)와 레인보우 익스프레스 슬라이드(x 84.5, y 30.3, z -956)의
   물에서 카메라를 돌렸을 때 하얀 번짐이 사라졌는지
4. 바깥 바다가 이전과 같이(또는 더 낫게) 보이는지 — 바다 2행도 41→42로 바뀌었으므로 회귀 확인 필요
5. 이상하면 위 백업 두 곳에서 되돌리면 된다

### 에이전트가 하지 않은 것 / 남은 것

- 화면 판정은 하지 않았다. Client를 실행·조작하지 않았고 캡처도 만들지 않았다. 위 3·4번은
  사용자의 육안 판정이 정본이다.
- 바닥 노랑·초록을 "주변 바닥으로 덮어 없애는" 두 번째 선택지는 원본 복원이 아니므로 승인 없이
  하지 않았다.
- `author_ocean_water_rows.py`의 잘못된 전제는 문서로만 남겼고 스크립트는 수정하지 않았다.
- 물 7행 중 바다 2행의 회귀 여부는 사용자 확인 전까지 미확정이다.

MAHARAKA_VIDEO_FIX_DONE

## G04. 사용자 스크린샷 이후 원본 재조사와 실제 반영

### 현재 판정

**부분 반영. 마하라카 전체 원본 복원 완료 아님.** 위 최초 완료 표기보다 이 항목이 우선한다.
사용자 이미지에서 노랑/초록 바닥 얼룩, 바위처럼 보이는 수면과 회색 무대가 남은 것을
관찰했다. clipping 감소나 JSON 파싱만으로 해당 결함이 해결됐다고 판정하지 않는다.

작업 폴더는 `C:/Users/USER/source/졸업팀폴/LostArk`다. 과거 Codex worktree와 혼동하지 않는다.
기존 dirty 브랜치의 배/카메라/NPC 수정은 되돌리거나 commit/stage하지 않았다.

### 원본을 다시 읽은 범위

- 원본 `C:/ProgramData/Smilegate/Games/LOSTARK/EFGame`의 LPK 8개 인덱스를 읽었다.
- data4의 LookInfo **59,888개 payload**를 MHP/Maharaka/Waterpang/ISMP/Mokoko/Mococo 참조로
  검사했다. 29개가 맞았지만 다른 지역의 모코코 자료도 포함하므로 전부 마하라카로 취급하지 않는다.
- 기존 복호화 테이블과 57009/57010/57011 Deploy 좌표, 실제 UPK mesh/AnimSet/재질을 대조했다.
- `out/MaharakaReaudit20260926`에 조사 후보를 보존했다. 원본 게임 컨테이너는 수정하지 않았다.
  이 검색은 모든 미해독 참조의 부재 증명이 아니다.

### 모코모코: 원본이 존재한다

`Npc 570910 -> EFDLChar_MN_ISMP_00.MN_ISMP_00 -> MN_ISMP_00` 연결을 확인했다.
물리 패키지는 `ReleasePC/Packages/9G1MHF9U1BZZE7AWQT64SSP.upk`다.

- skeletal mesh: `mn_ismp_00_sk`, `mn_ismp_00-1_sk` 두 종류.
- AnimSet: `mn_ismp_00_ani`, 실제 클립 10개.
- 클립: `att_battle_1_01`, `att_battle_2_01/02/03`, `att_battle_3_01/02/03/04`,
  `idle_normal_1`, `idle_battle_1`. 긴 반복 두 개는 각각 24초다.
- 기본 모델은 9개 joint/10개 clip의 WModel 후보까지 변환했다.
  후보: `out/MaharakaReaudit20260926/Resources/Map/LV_OCN_EVENTIS_MHP_Attractions/MN_ISMP_00/MN_ISMP_00.wmodel`.
- 섬 57009 Deploy actor 100/NPC 570910: 원본 cm `(7517.9,100726.2,1990.4)`,
  프로젝트 위치 `(75.179,19.904,-1007.262)` m. 원본 yaw -90도는 모델 basis와 별도 대조가 필요하다.
- 원본 세 MIC의 RefShaderCache GPU-skin Base/Light 프로그램과 uniform/texture 입력도 추출했다.
  `MokoNative/mn_ismp_00.mat.mn_ismp_00_mi.json`의 기본 재질은 설치된
  `source.character.monster-6ff78ae19259.v1`/program 26과 Base 642줄, Light 618줄,
  CPU configure 모두 재생성 EXACT다. 나머지 두 재질은 서로 다른 원본 프로그램이며 아직 연결하지 않았다.

**미설치:** 이 후보를 live Resources나 NpcCatalog/Gameplay에 등록하지 않았다. 모델/애니메이션
추출은 제품 재질·배치·AI 타이밍 적용과 다르다. 기존 NPC 생성 경로의 native descriptor 전달과
나머지 두 재질을 닫지 않고 회색/기본 재질로 설치해 원본 복원이라고 하지 않는다.
워터팡 무대·캐논의 전체 소스 연결도 미완료다. 이전의 "삭제됐다"는 단정은 철회한다.

### 지형: 밝기만의 문제가 아니었다

원본 `efmaster_material_lv_prologue.landscape.landscape_base` export를 직접 읽었다.

- layer01은 `LB_AlphaBlend`, layer02~07은 `LB_HeightBlend`다.
- 현재 MIC의 `alpha_modify=false`에서 Height 입력은 각 diffuse의 alpha channel이다.
- 현재 `bake_component_textures`는 Height 입력 없이 모든 weight를 정규화해 가중 평균한다.
- 원본 `use_layer02_normal/use_layer05_normal/use_layer06_normal=false` 등의 선택도
  단순 normal 혼합과 다르다. 현재 코드의 byte/255 색 계산·headroom·8bit 출력은
  원본 linear/HDR 재질을 그대로 실행하는 경로가 아니다.
- 보존된 그래프는 부분 그래프다. 사라진 edge를 추측해 채우거나 표현식 누락을 원본
  shader 자체의 삭제로 해석하지 않는다. source graph/parameters는 out의
  `landscape_graph.json`, `source_details.json`, `landscape_parameters.json`에 보존했다.

이번에는 Landscape PNG·지형 geometry·팀장 rendering 옵션을 변경하지 않았다.
HeightBlend/UV·색공간·원본 조명 및 runtime carrier 연결은 남은 작업이다.

### 이번에 실제 반영한 수정

1. `MapPlacementRuntime.cpp::Sample_SelfMotions`가 각 행에서 base로 reset하던 문제를 수정했다.
   placement별 한 번 reset 후 모든 행을 합성하고 한 번 commit한다. 기존 visibility 유지.
   마하라카 게시본 65행/59배치 중 두 회전축을 가진 6배치의 앞 축이 지워지던 경우에 해당한다.
   기존 주기 함수나 모션 데이터는 바꾸지 않았다.
2. `author_ocean_water_rows.py`의 잘못된 water-41 재생성 경로를 water-42 후보 생성으로 교정했다.
   실제 packer의 30개 parameter·7개 texture lane과 대조했고 source 값·색공간을 보존한다.
   입력 저작 파일과 동일한 출력 경로는 거부한다. 원본 부모가 같은 river-rock도 누락시키지 않는다.
3. 최신 저장본의 **기존 326개 행은 값 단위 무변경**으로 보존하고 다음 세 행만 추가했다.
   최종 authoring/runtime은 329개, water-42는 10개다.

| 추가 asset ID | 적용 placement 수 |
|---|---:|
| MAP_4C8A5424422D_BG_FAT_STONE_ROCK03_SM_OVR_1A2629364329 | 2 |
| MAP_557CAE338ABF_BG_FAT_STONE_ROCK02_SM_OVR_1A2629364329 | 7 |
| MAP_A98778E158B2_BG_FAT_STONE_ROCK01_SM_OVR_1A2629364329 | 4 |

세 행의 원본 MIC는 `lv_ber_kandad.mat.lv_ber_kandad_f_water_01`이다. 이미 설치된
`Client/Bin/Resources/Map/LV_OCN_EVENTIS_MHP/.../textures/*.dds`를 참조하므로 이 반영에는
새 Drive binary가 없다. source wave/distortion 분기·vertex 변형과 최종 외형까지 완료한 것은 아니다.
저작 파일 백업은 `out/MaharakaReaudit20260926/mapmaterials.before-install.json`이다.

### 실행한 검증

- 수정 water helper Python AST 및 actual header의 parameter 이름 30개/texture lane 7개 일치.
- 10개 후보의 source parameter 값과 Resources 경로 존재 확인. 기타 319개 후보 행 무변경.
- 별도 후보를 기존 publisher의 실제 WModel material-name/parameter/texture 검사로 검증: PASS.
- 최신 live 저장본 326행 전부 보존, 신규 3행/13배치, authoring/runtime 파일 일치 확인.
- Area `Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP`의 Validate → Publish → Check: PASS.
  4,651 placements, 게시 파일 5개. runtime JSON은 수동 편집하지 않았다.
- Debug Product 증분 Build: PASS. receipt `out/BuildPipeline/runs/20260926T074248577Z-debug-product.json`.
  Engine/Shared compile output 변화 0, Server OBJ 68/실행 파일 1, Client OBJ 52/실행 파일 1,
  CSO compile 변화 0. 다른 기존 dirty C++도 같은 Product Build에 포함됐다.
- 해당 build의 기본 runtime missing/invalid 입력 목록은 비어 있다. 실제 아레나 재생 검사는 아니다.
- 변경 파일 `git diff --check`: PASS. MapPlacementRuntime.cpp의 UTF-8 no-BOM/CRLF 유지.
  전체 dirty tree 검사는 기존 `MainApp.cpp`의 trailing whitespace 5줄을 보고했다.
  이번 작업에서 건드리지 않은 UI 변경이므로 정리하지 않았다.

### 사용자 확인과 남은 범위

에이전트는 Client/UI를 실행·조작·촬영하지 않았다. 화면 성공 판정은 미실시다.
새 실행 파일은 `Client/Bin/Debug/Client.exe`, 작업 디렉터리는 `Client/Default`다.
Server가 실행 중인 상태에서 사용자가 새 Client로 Lobby → Maharaka → F6 자유시점에서
바위 수면 13배치와 복수 축 소품의 움직임을 확인한다. F6 상태에서는 게임 이동 입력을 보내지 않는다.

지형 바닥·모코모코 live 설치·워터팡 전체 복원은 **미완료**다. 전체 복원이나 모든 원본 검색이
끝났다고 보고하지 않는다. 부분 graph를 정확한 compiled landscape shader/입력과 연결하고,
모코모코의 나머지 재질·배치·source 동작 소비자를 연결하는 일이 남아 있다.

## G05. 17:16 스크린샷 이후 지형 재생성·모코모코 제품 배치

### 현재 상태

**구현과 설치는 아래 범위만 완료했다. 전체 원본 복원/사용자 visual PASS는 아니다.**
사용자 이미지의 노랑·초록 얼룩이 유지된 것을 확인했다. 단순 headroom 감소를 다시
완료 근거로 삼지 않고 실제 레벨 ShaderCache의 Landscape 프로그램을 찾았다.

### 원본 Landscape의 실제 프로그램과 수정

공용 RefShaderCache의 196개 Landscape 후보에는 설치된 컴포넌트의 static key가 없었다.
그러나 이것은 원본 삭제가 아니다. `ReleasePC/9XUFAXIP8BXBAP1NIEG66EF.upk`의
레벨별 ShaderCache 1,596개 중 export 813 `sc_lv_ocn_eventis_mhp_land01`에 있었다.
descriptor 수와 code blob 수가 다른 packed 형식이므로 `extract_selected_packed_dxbc`로
읽었다. 현재 설치 지형 **16/16개가 실제 컴포넌트 static key와 일치**한다.

`Tools/LandscapeExtractor/extract_ue3_landscape.py`에 명시적으로 선택하는 source-layer
베이크 입력을 추가했다. 기존 호출자의 legacy 기본 경로는 바꾸지 않았다.

- 원본 UV: `(sectionBase + local) * 0.1`에 half-centred 회전을 적용한 뒤 tiling.
  기존 component 크기로 나누는 UV와 다르다. 원본 rotation scalar는 `3.14 radians` 계수다.
- source sRGB texture를 linear로 변환한 **뒤** bilinear wrap 필터링한다. alpha는 linear다.
- 원본 HeightBlend는 `saturate(2 * paintWeight - 1 + diffuseAlpha)`를 합산·정규화한다.
  diffuse와 normal의 blend 모드가 다르며 normal의 layer06/07은 raw weight다.
- 각 컴포넌트의 native static normal enable, luma `(0.3,0.59,0.11)`를 적용한다.
- 원본 밝기·색상값은 유지한다. 노랑·초록 tint 자체는 원본에 있으므로 임의로 지우지 않았다.
- 원본 PS에 없는 강제 경사 cliff overlay는 이 source-layer 경로에서 적용하지 않는다.

16개 diffuse/normal **32 PNG**를 재생성하여 기존 WModel이 읽는
`Client/Bin/Resources/Map/LV_OCN_EVENTIS_MHP_LAND/Landscape/<asset>/textures/`에 설치했다.
실제 파일의 설치 전 identity를 전부 대조하고 백업 후 교체했다. geometry/holes/placement와
팀장 rendering options는 변경하지 않았다. 512px source-layer 결과다.

후보/검증 자료는 `out/MaharakaReaudit20260926/`에 있다.
`terrain-native-layer-candidate.json`, `terrain-native-layer-installed.json`,
`landscape-level-map-*.json`, `landscape-level-matches.json`.
원래 파일 백업은 `TerrainBeforeColourInstall`, 후속 설치 직전 백업은
`TerrainBeforeNativeLayerInstall`이다. 지우거나 기존 백업을 덮어쓰지 않았다.

이것은 원본 diffuse/normal/UV 계산을 옮긴 **표시용 베이크**다. 전체 GPU material,
HDR, RNM/lightmaps, 동적 reflection, GPU derivative mip LOD 및 기존 별도 cliff geometry/UV는
동일성 검증이 끝나지 않았다. PNG를 열어본 것은 리소스 검사이지 게임 화면 PASS가 아니다.

### 모코모코와 워터캐논: 제품 모델·재질·배치 설치

원본 모델 `MN_ISMP_00_SK`, `MN_ISMP_00-1_SK`를 기존 CModel/CNpc 경로에 연결했다.
각각 원본 9 joints, 10 clips를 가진다. 원본 NPC ModelSize 170%, 50%를 geometry/bind/animation
전체에 동일하게 적용했다. 기존 NPC의 no-RootNode centimetre preScale 경로를 사용한다.

Native 재질 3종 중 기본은 기존 program 26, 나머지는 원본에서 생성한 program
1526/1527이다. 각각 Base/Light disassembly 재생성 EXACT를 확인했다. common cohort 1472,
registry, parameter packer를 등록했고 NPC catalog도 기존 model-load descriptor를 전달하도록
수정했다. 두 번째 모델 런타임은 만들지 않았다.

`NpcCatalog.json` format 2의 optional root `modelMaterialOverrides`는 기존 공용 parser로
stage/validate한다. 정의에 없는 NPC 모델의 override를 거부하고, 다른 actor catalog와
중복 소유권도 기존 모호성 오류로 거부한다. 이전 NPC 행은 값 단위로 그대로 유지했다.

| placement ID | archetype | 위치(m), yaw |
|---|---|---|
| npc.maharaka.source57009.actor100 | NPC_MAHARAKA_MOKOMOKO | 75.179, 19.904, -1007.262 / 0도 |
| npc.maharaka.source57009.actor188 | NPC_MAHARAKA_WATERCANNON | 75.05, 22.36, -984.32 / -46.1도 |

yaw는 source yaw에 기존 NPC preY -90도를 보상한 값이다. 실제 방향은 사용자 검증 대기다.
`idle_normal_1`을 기본 재생하고 나머지 8 attack clip을 catalog action binding에 등록했다.
**AI 회전/물 발사 event를 자동 실행하도록 구현한 것은 아니다.** 임의의 반복 순서를 원본
일정처럼 대입하지 않았다. 원본 Deploy 57011 복제본은 현재 섬에 중복 배치하지 않았다.

추가 리소스(모두 Resources 상대 경로):

- `Character/NPC/Maharaka/MN_ISMP_00/MN_ISMP_00.wmodel`
- `Character/NPC/Maharaka/MN_ISMP_00-1/MN_ISMP_00-1.wmodel`
- `Character/NPC/Maharaka/Textures/*.dds` 10개. 기존 shared lookup texture 4개도 참조한다.
- 위 subtree의 converter fallback PNG/TGA도 모델 입력으로 보존했다.

물리 루트는 `C:/Users/USER/source/졸업팀폴/LostArk/Client/Bin/Resources/`다.
지형 PNG와 NPC subtree는 Git 제외 Resources이므로 팀 배포 시 별도 전달 대상이다.
이번 요청에서는 Drive 업로드나 Git push는 하지 않았다.

### 게시·빌드·검사

- World publisher에 `-WorldId MAHARAKA` scope를 추가했다. 해당 월드가 참조하는 encounter만
  profile 검증하고 실제 참조의 미해결 오류는 유지한다. ALL의 검증은 완화하지 않았다.
- World Validate/Publish: 6 placements PASS. 기존 playerSpawn 4개는 변경하지 않았다.
  `Server/Bin/DataFiles/World/MAHARAKA.worldbootstrap` 및 Client NPC presentation 게시 완료.
  Client presentation의 entries가 빈 것은 placement별 idle/behavior override가 없기 때문이다.
- Map publisher Check: PASS, 4,651 placements / 5 files. 추가 mapmaterials 변경은 없다.
- 설치 payload 재검사: 32 PNG identity/백업, 2 WModel/6 material slots/texture 존재 PASS.
- 2 WModel의 weighted bind 최대 오차 .000789/.000247, clip당 5시점 geometry finite 검사 PASS.
  이는 GPU tangent/화면 일치 검증을 대신하지 않는다.
- Landscape unit tests: **32 PASS**. 색공간, alpha, wrap, legacy sampling, source UV/HeightBlend,
  잘못된 mode/UV scale 거부 포함.
- 정본 `Invoke-BuildAndRegression.ps1 -Configuration Debug` Product 증분 빌드 PASS.
  receipt `out/BuildPipeline/runs/20260926T093149866Z-debug-product.json`.
  Engine 367,037ms / Client 1,165,016ms. Engine OBJ2/CSO23, Client OBJ8/CSO72.
  Shared/Server도 PASS. HLSL/외부 PDB 경고는 남았으나 compile/link 오류는 없었다.
  build 이후 Python의 입력 오류/metadata 설명을 보완했으며 C++/shader 변경은 없다.
- 변경 파일 `git diff --check`: PASS. JSON은 실제 publisher 및 설치 검사에서 다시 파싱했다.
  전체 dirty tree의 기존 MainApp.cpp 공백 문제와 무관한 수정은 유지했다.

### 추가 발견과 미완료 경계

`MN_ISMP_00.Action.loa`는 실제로 남아 있다. 36 actions, 182 stages, 20종 particle,
111 sound notifies가 있다. 물/얼굴 효과·사운드·AI 신호의 제품 연결은 미완료다.
`MN_ISTM_00` LookInfo는 `MN_Empty_00_SK`를 사용한다. 링의 20 NPC는 무대 메시로
사용할 근거가 없는 빈 controller다. 그 Action에도 별도 소스가 남아 있으나 전체 이벤트
실행기를 현재 NPC idle에 억지로 붙이지 않았다.

이전 보고서의 Prop ID를 Npc 테이블에서 조회한 결과는 잘못된 비교였다. 실제 Prop 테이블을
다시 조회하면 57011의 118 placements 중 39개에 현재 정의가 있고 79개는 이 테이블에서
찾지 못한다. 300004는 모델 없는 충돌 Prop이며 MN_KZDW_02-1 NPC가 아니다. 이것으로
무대 리소스 삭제를 단정할 수 없다. 무대 모델/재질/상태 전환의 정확한 연결은 미완료다.

### 사용자 실행·화면 확인

Client/Server는 실행하지 않았다. 종료 시 Client/Server process 없음, Visual Studio만 실행 중.
사용자가 `Server + Client` Debug profile로 시작하고 Lobby → Maharaka로 진입한다.
F6 자유시점에서 기존 얼룩 구간과 위 두 NPC 위치의 크기·방향·재질·idle을 확인한다.
**지형 최종 외형, 모코모코/캐논의 실제 첫 화면, 물/무대 전체 복원은 사용자 승인 전이다.**
