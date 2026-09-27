# 2026-09-26 마하라카 G03·G04 RESULT — 렌더링 소비자 비교와 원본 RNM 입력 연결

실행 저장소 `C:/Users/USER/source/졸업팀폴/LostArk`, branch `codex/main-ship-maharaka-0926`,
HEAD `a84bbcd45ff995c70bf847c683f1d159cf1821a6`. 후보 폴더
`out/MaharakaContinuation_20260926_193455/G03_G04/`.

설계서 `.md/GB/09-26/2026-09-26_MAHARAKA_SOURCE_RESTORATION_HANDOFF.md` 의 G03 전체와 G04 전체만 수행했다.
G01/G02/G05~G12은 이 작업 범위가 아니다.

## 1. 이번 G에서 실제 바꾼 파일

**제품 파일을 하나도 바꾸지 않았다.** `Data/**`, `Client/Bin/Resources/**`,
`Client/Bin/DataFiles/**`, C++·shader·project 파일을 수정하지 않았다. 조사·추출·비교표만 생성했다.

새로 만든 파일은 전부 후보 폴더와 이 문서다.

| 경로 | 내용 |
|---|---|
| `out/…/G03_G04/ComponentLighting/SL01.component-lighting.json` + `SL01.receipt.json` | SL01 package component RNM 증거 |
| 같은 폴더 `LAND01.*`, `PS.*` | 나머지 두 package |
| `out/…/G03_G04/rnm_join_summary.json` | RNM ↔ 게시 배치 ↔ 자산 ↔ 재질 행 조인 결과 |
| `out/…/G03_G04/scene_profile_readonly_compare.json` | scene profile 읽기 전용 비교 |
| `out/…/G03_G04/rendering_gap_table.md` | G03 실측 비교표 |

기존 사용자 미커밋 변경 보존: 작업 시작 시 `git status --short` 161항목 / `git diff --stat`
75 files changed, 10356 insertions(+), 121 deletions(-). `git reset/checkout --/stash/add`,
commit, push를 실행하지 않았고 빌드도 하지 않았다(Visual Studio pid 30616 실행 중).

## 2. 원본 근거

### 2-1. 물리 ↔ 논리 package 쌍 (추측 아님, 실제 manifest에서 확정)

`C:/LostArkExtract/LV_OCN_EVENTIS_MHP_20260919/Placements/placement_manifest.json`
schemaVersion 3, `coordinateSystem: UE3-native`.

| logicalPackage | physicalPackage | 배치 | source 가시 / 숨김 |
|---|---|---|---|
| `LV_OCN_EVENTIS_MHP_SL01` | `645QRFK5UT4TKQLJ5FDEY5KJ63A.upk` (9,783,266 bytes) | 3823 | 3822 / 1 |
| `LV_OCN_EVENTIS_MHP_LAND01` | `867STHM7WV6VMSNL7HFG07M83MO5C.upk` (10,333,812 bytes) | 82 | 82 / 0 |
| `LV_OCN_EVENTIS_MHP_PS` | `423OPDI3SR2RIOJH3DBCW3IWH.upk` (690,230 bytes) | 244 | **1 / 243** |

`sourceVisibilityBasis` 는 세 package 모두 `ue3-instance-archetype-cdo`.
LAND01 의 물리 package 는 설계서가 말한 source Landscape 패키지와 같은 파일이다.
ShaderCache `9XUFAXIP8BXBAP1NIEG66EF.upk` (270,965,156 bytes) 는 `ReleasePC` 루트에 실재하며
`Packages` 하위가 아님을 확인했다.

### 2-2. 게시 배치의 조인 키

`Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.mapplacements` 는 헤더
`LOSTARK_MAP_PLACEMENTS 2 "LV_OCN_EVENTIS_MHP" 4651`, 탭이 아니라 **공백 구분 + 인용 필드**다.
4651행 전부 2열이 `<logicalPackage>:export:<N>` 형식이며 component 추출기 출력의
`sourcePlacementId` 와 문자열이 그대로 일치한다. 논리 package 분포는
SL01 3820 / LAND01 830 / PS 1.

### 2-3. 대표 component 전체 연결 (끊김 없음)

```
sourcePlacementId  LV_OCN_EVENTIS_MHP_SL01:export:5265
componentObjectPath …persistentlevel.<actor>.staticmeshcomponent_…  (actorExportIndex0 기록됨)
sourceMesh         bg_lut_lucastle_g.mesh.bg_lut_lucastle_houseinborder01_sm_phs
assetId            MAP_640F6740C1FA_BG_LUT_LUCASTLE_HOUSEINBORDER01_SM_PHS
lightMapKind       2  (RNM_TEXTURE_LIGHTMAP)
bakedLightGuids    c0bb17497749e244848520b99b0cea75
average            LV_OCN_EVENTIS_MHP_SL01.normalizedaveragecolor0_58   (lightmaptexture2d)
directional        LV_OCN_EVENTIS_MHP_SL01.directionalmaxcomponent0_58  (lightmaptexture2d)
averageScale       [1.0, 1.0, 1.0]
directionalScale   [1.0, 1.0344204902648926, 1.0]
coordinateScale    [0.029296875, 0.05859375]
coordinateBias     [0.8291015625, 0.501953125]
hasColor           false  (COLOR0 정점 0개)
nativeTailCompletelyConsumed  true
```

RNM lightmap 텍스처는 **레벨 package 내부 export**다(모델 UPK가 아님). 설계서 G04-02 2번과 일치한다.

WModel 확인: 같은 assetId 가 Bern 과 Maharaka 두 runtime root 에 모두 설치돼 있다.
둘 다 `WINT 1.2` (= `WINT_UV1_VERSION_MINOR`, 정적 UV1 보유), 파일 14,720 bytes,
`WUVS` tail 없음. **SHA256 은 서로 다르다** (Bern `d741d485d6882b33…`, Maharaka `c1431bf502907078…`).
같은 source mesh 라도 lightmap UV1 이 Area별로 다르다는 뜻이다.

## 3. 상태 구분

| 단계 | 상태 |
|---|---|
| 추출 완료 | RNM component 증거 4149건 추출 (아래 수치) |
| 후보 생성 | 조인 요약·비교표 생성 |
| live 설치 | **없음** |
| 게시 | **없음** |
| 빌드 | **없음** (VS 실행 중이라 금지) |
| 제품 소비자 연결 | **없음** |

## 4. 실행한 명령과 결과

### 4-1. CLI 도움말 (제품 변경 없음)

세 도구 모두 `--help` exit 0.

- `extract_source_map_component_lighting.py` 필수: `--package`, `--logical-package`, `--output`, `--receipt`, 선택 `--area-id`.
  설명: "Extract v868 StaticMeshComponent RNM and instance environment evidence."
- `build_source_map_materials.py` 필수: `--input`, `--resources-root`, `--output`, `--receipt`.
- `build_source_map_material_inputs.py` 필수 11개 (`--area-id --parameters --runtime-manifest
  --mip-catalog --mip-results --mip-chains --asset-inventory --imported-catalog --resources-root
  --new-resources-root --output-dir`), 선택 `--runtime-area-root --engine-default-dir`.

### 4-2. component lighting 추출 — exit 2, PASS 아님

세 실행 모두 `PARTIAL_UNSUPPORTED` / exit **2**. 설계서대로 PASS로 기록하지 않는다.

| package | component | RNM_TEXTURE_LIGHTMAP | UNSUPPORTED_NATIVE_LAYOUT | NO_LOD_LIGHTING_DATA | failure | skipped |
|---|---|---|---|---|---|---|
| SL01 | 3823 | 725 | 3027 | 71 | 0 | 3 |
| LAND01 | 82 | 1 | 81 | 0 | 0 | 0 |
| PS | 244 | 3 | 240 | 1 | 0 | 0 |
| 합계 | **4149** | **729** | **3348** | **72** | 0 | 3 |

4149 는 placement manifest summary 의 `placementCount` 4149 와 일치한다.

조인 결과 (`rnm_join_summary.json`):

- RNM 729건 중 게시 배치와 조인되는 것 **725건**
- 영향 받는 `assetId` **147개**
- 그중 현재 `mapmaterials` 행이 있는 자산 **90개**, 없는 자산 **57개**
- 필요한 lightmap texture export **58개**

### 4-3. 회귀 2건 — 둘 다 통과

```
python -B -m unittest discover -s Tools/LandscapeExtractor -p test_extract_ue3_landscape.py
  → Ran 32 tests, OK, exit 0
python -B -m unittest discover -s Tools/LevelPlacementExtractor -p test_build_source_map_materials.py
  → Ran 13 tests, OK, exit 0
```

두 번째 실행 중 출력되는 `source map material build failed: source.material.zero: unsupported PBR
switches {'unsupported'}` 는 음성 테스트의 기대 출력이며 실패가 아니다.

## 5. G04-01 Landscape 계약 검증 결과

`Tools/LandscapeExtractor/extract_ue3_landscape.py` 의 `source_layer_contract` 경로는
키를 `{uvScale, diffuseBlend, normalBlend, shaderMapKey}` 로 정확히 요구하고(`:2455`),
`shaderMapKey` 를 64자 hex 로 검사하며(`:2457`), blend 값을 `weight|height` 로만 허용한다(`:2462`).

| 설계서 항목 | 코드 실측 | 판정 |
|---|---|---|
| source UV `(sectionBase+local)*0.1`, half-centred 회전 뒤 tiling | `source_landscape_uv` `:2401-2406` = `rotate_uv(x*scale-0.5, y*scale-0.5, rotation)` 후 `(u+0.5)*tiling` | 일치 |
| sRGB decode 후 linear bilinear wrap, alpha linear | `:2027-2033` "decode each texel BEFORE bilinear interpolation. Alpha …", `bilinear` `:2171` | 일치 |
| HeightBlend `saturate(2*paintWeight-1+diffuseAlpha)` 후 정규화 | `source_landscape_weights` `:2409-2419`, 분모 `max(sum, 0.0001)` | 일치 |
| layer별 서로 다른 diffuse/normal blend | `diffuseBlend`/`normalBlend` 를 분리해 layer별 mode 검사 | 일치 |
| rotation scalar 약 `3.1400001049` rad | 추출기에 **상수 없음**. `layer["rotation"]` 로 주입됨. `Landscape2/maharaka_landscape_manifest.json` 에 `rotation`·`uvScale`·`shaderMapKey` 문자열 **없음** | **unresolved** — 이 값이 저장된 문서 위치 미확인 |
| luma `.3,.59,.11` | `:2337` `red*0.299 + green*0.587 + blue*0.114`, `:2356` 기본값 `(0.299, 0.587, 0.114)`, layer 의 `luminanceWeights` 로 override 가능 | **불일치 가능** — 설계서 표기와 코드 기본값이 다르다. 원본 확인 전까지 바꾸지 않았다 |

`.3/.59/.11` 과 `.299/.587/.114` 는 UE3 `Luminance` 에서 실제로 쓰이는 두 계수 조합이 서로 다르다.
설계서 표기가 반올림인지, 코드 기본값이 잘못된 fallback 인지는 원본 DXBC 상수로 확정해야 한다.
해당 layer 데이터가 `luminanceWeights` 를 실제로 공급하면 기본값은 사용되지 않는다. 이번에 변경하지 않았다.

Landscape 설치 현황: `Client/Bin/Resources/Map/LV_OCN_EVENTIS_MHP_LAND/Landscape/` 에
컴포넌트 폴더 **16개**, `.wmodel` 16개, `baked_diffuse.png` 16개, normal PNG 16개 = PNG 32개.
`Landscape2/maharaka_landscape_manifest.json` 의 `validation` 은 `componentCountExpected: 46`,
`wmodelCount: 46`, `collisionHeightMismatchCount: 0`, 인접 edge `mismatchCount: 0` 이다.
**46 대 16 차이의 근거를 아직 확인하지 않았다.** 같은 manifest 의 `limitations` 첫 항목이
"baked diffuse/normal textures are display derivatives, not the UE3 material graph" 이므로
현재 PNG 로 native Landscape 동일성을 주장하지 않는다.

## 6. 반드시 피해야 하는 함정 (코드에서 확인)

`Tools/LevelPlacementExtractor/build_source_map_material_inputs.py` 는
`lightingEvidence='source-absent'` 를 **모든 slot 에 하드코딩**한다(`:259`).
같은 파일 `:288` 의 note 가 직접 이렇게 적어 놓았다.

> "lightingEvidence=source-absent is only the compiler carrier. The original components carry
> ShadowMap2D and RNM lightmaps that this build does not consume; baked lighting stays unbound."

그리고 `build_source_map_materials.py:374` 는 `lightingEvidence` 를
`('source-bound', 'source-absent')` 둘만 허용하고, `source-absent` 면
`bakedLighting`/`environment` 바인딩이 없어야 한다고 강제한다(`:377-378`).
즉 **`unresolved` 를 표현할 값이 schema 에 없다.**

따라서 이 adapter 를 그대로 마하라카에 돌리면 실제로는 RNM 이 있는 725 component 에 대해
"원본에 baked lighting 이 없다"는 **거짓 주장**을 데이터에 기록하게 된다. 설계서 G04-02 6번
("`source-absent` 가 확인된 경우만 baked 없음으로 기록, 미추출은 `unresolved`")과 정면으로 충돌한다.
이번 조사에서 이 adapter 를 실행하지 않았다.

## 7. Bern 이 증명한 목표 계약 (복사 대상이 아니라 형식 근거)

| 항목 | Bern 실측 |
|---|---|
| 고유 자산 | 16,515개 중 `_RNM_` 포함 **15,305개** |
| `bakedLighting` 보유 행 | 21,321개, 그중 `_RNM_` assetId **21,321 / 21,321 (100%)** |
| `bakedLighting` 구조 | `averageTexture`/`directionalTexture` = Resources 상대 DDS (`Map/Lighting/Bern/…dds`), `colorSpace: "linear"`, 선택 `staticShadow{texture, lightGuid, lightChannel, penumbraWidth, penumbraBasis, shadowExponent}` |
| `penumbraBasis` | `"PROJECT_ADAPTER"` — 원본이 아니라 프로젝트 근사임이 데이터에 표시돼 있다 |
| `placementLighting` 행 | 49,047개, 키 `sourcePlacementId`/`assetId`/`coordinateScale`/`coordinateBias`/`averageScale`/`directionalScale`… |
| lightmap DDS | `Client/Bin/Resources/Map/Lighting/Bern/` 에 7,043개 |

Maharaka 현재: `_RNM_` 자산 0개, `bakedLighting` 0행, `placementLighting` 0행,
`Map/Lighting/Maharaka/` 에 DDS 1개뿐이며 그것은 `lv_ocn_dookyis_lut.dds` (color grading LUT, lightmap 아님).

즉 **RNM 을 적용하려면 `_RNM_<hash>` variant 자산을 새로 cook 해야 한다.** 기존 자산 행에
`bakedLighting` 만 덧붙이는 방식은 Bern 의 실제 계약과 다르다. variant 마다 Area별 UV1 을 가진
별도 WModel 이 필요하고(대표 자산에서 Bern/Maharaka WModel SHA 가 다른 이유), 배치 147자산·725배치를
variant assetId 로 다시 가리켜야 한다.

## 8. import 수준의 근본 원인

`Data/Maps/Imported/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.build.receipt.json`

- `sourceMaterialBuild: null` → **source material compiler 가 이 Area 에 한 번도 실행되지 않았다.**
- `runtimeMaterialAdmission` 382 자산 전부 `mode: "geometry-preview-partial-material"`,
  `originalVisualFidelityVerified: false` (382/382).
- 미지원 필드 상위: `renderFlag:blendMode`/`disableDepthTest`/`isMasked`/`opacityMaskClipValue`/`twoSided` 각 495건,
  `scalar:specular_intensity` 446, `scalar:specular_power` 413, `texture:texture_emissive` 380,
  `texture:texture_reflection` 375, `texture:texture_overlay_diffuse` 360, `scalar:diffuse_brightness` 301.
- `placementDirectories`: `C:/LostArkExtract/LV_OCN_EVENTIS_MHP_20260919/Placements_Admitted`.

현재 `mapmaterials` 329행의 family 분포는 `bg-source-opaque-masked` 307 / `bg_base_opa_overlay` 12 /
`source.map.water-42.v1` 10, 고유 assetId 251개다. 이 행들은 compiler 출력이 아니라 별 경로로 들어온 것이다.

## 9. mapmaterials stage-merge 방안 (아직 구현하지 않음)

329행 전체 교체를 하지 않는다. 다음을 지킨다.

1. 병합 키는 `(assetId, materialName)` 튜플 하나로 고정한다. 문서 내 중복 키가 이미 있는지
   먼저 세고(현재 329행 / 고유 assetId 251개이므로 한 자산이 여러 slot 행을 가진다),
   같은 키가 둘 이상이면 병합을 중단하고 근거를 보고한다. 마지막 값으로 조용히 덮지 않는다.
2. compiler 후보 중 `(assetId, materialName)` 가 기존 문서에 **없는 행만 추가**한다.
   기존에 있는 키는 필드별 diff 를 출력하고 승인 전까지 적용하지 않는다.
3. `source.map.water-42.v1` 10행과 `bg_base_opa_overlay` 12행, 사용자가 손댄 override 는
   병합 대상에서 제외 목록으로 고정한다.
4. `placementLighting` 은 현재 0행이므로 추가만 발생한다. 그래도 `sourcePlacementId` 가
   `.mapplacements` 에 실재하는지 4651행과 교차 검증하고, 없는 ID 는 거부한다.
5. RNM variant 가 필요한 147자산은 catalog(`.mapassets`)·배치(`.mapplacements`)·재질 행·조명 행을
   **한 트랜잭션 단위**로 바꾼다. 네 문서 중 하나만 바뀐 상태를 만들지 않는다.
6. 병합 전 원본 파일을 후보 폴더에 복사하고, 병합 후 행 수·기존 키 집합 불변을 assert 한다.

## 10. 사용자에게 확인받은 항목 / 받지 않은 항목

- 확인받은 항목: **없음**. 이번 G 는 화면 판정이 필요한 변경을 하지 않았다.
- 받지 않은 항목: RNM variant cook 진행 여부, `_RNM_` 자산 147개 추가에 따른 Resources 배포 범위,
  luma 계수 확정, 46 대 16 Landscape 컴포넌트 차이의 처리 방향.

## 11. 미해결 입력과 다음 조사 위치

| 미해결 | 확인된 검색 범위 | 다음 조사 위치 |
|---|---|---|
| `UNSUPPORTED_NATIVE_LAYOUT` 3348 component | 세 package 전부 추출기로 시도, failure 0 | 추출기의 v868 native layout 분기. 이 3348건이 RNM 을 가졌는지 아닌지는 **아직 모른다**. `source-absent` 로 기록하면 안 된다 |
| `NO_LOD_LIGHTING_DATA` 72 component | 같은 추출 | 이 72건만 현재 근거로 baked 없음 후보다. 그래도 LOD 0 이 실제로 lighting 을 안 갖는지 재확인 필요 |
| lightmap DDS 58 export | export 이름·package 확정 | 레벨 package 내 `lightmaptexture2d` export 를 DDS 로 뽑는 경로. 기존 `extract_ue3_texture_mips.py` 지원 범위 확인 |
| UV1 이 UV0 복제가 아님 | WModel 1.2 (UV1 채널 존재), Bern/Maharaka SHA 상이 | WMSH 정점 버퍼에서 UV0/UV1 실제 값 표본 비교. 숫자로 확인하지 않았다 |
| rotation scalar `3.1400001049` 저장 위치 | 추출기 코드, `Landscape2` manifest | `SourceRaw/MasterMaterial/master_material.json`, `DependencySerials` |
| luma 계수 | 추출기 `:2337`, `:2356` | 원본 선택 DXBC 상수. ShaderCache export 813 `sc_lv_ocn_eventis_mhp_land01` |
| Landscape 46 대 16 | manifest validation, 설치 폴더 | `Landscape`/`Landscape2`/`LandscapeIsland` 세 폴더의 area 귀속 |
| 원본 fog/skylight/post-process | 4149 component 전부 `INSTANCE_PROPERTY_ABSENT` | **레벨 액터** export. Bern region 이름이 `bern.ps.environment.106~110` 이므로 `LV_OCN_EVENTIS_MHP_PS` (243 hidden) 가 1순위 |
| 상시 맵 particle 원본 | MapCatalog 미선언, 문서 없음, 코드 경로 0건 | SL01/PS 레벨 액터의 particle 액터 export |

## 12. 다음 G 진행 근거

- G05(전용 scene profile·상시 particle·물)는 이 문서의 4-1/4-2 목록과 11절 표를 입력으로 바로 시작할 수 있다.
  단 `Data/Rendering/**` 수정과 `Publish-RenderingProfiles.ps1 -Mode Publish` 는 사용자 승인 전까지 금지 상태다.
- RNM 실제 적용은 `_RNM_` variant cook 이 선행이므로 G04 는 **미완료**다. 대표 component 의
  원본→증거 연결만 끝났고 후보 설치·게시·소비자 연결은 하지 않았다.
- G01/G02 와 독립적이므로 그쪽이 막혀도 위 미해결 항목은 계속 진행할 수 있다.

## 최종 재확인

명령 출력으로 다시 확인한 결과다.

1. **제품 변경 0** — `git status --short` 로 확인한 변경 파일 목록에 이번 작업이 만든 항목은
   후보 폴더(`out/` = ignore 대상)와 이 RESULT 문서뿐이다. `Data/`, `Client/Bin/Resources/`,
   `Client/Bin/DataFiles/`, `Engine/`, `Client/`, `Tools/` 파일을 쓰지 않았다.
2. **빌드 0** — MSBuild/cl/fxc 를 실행하지 않았다. VS(pid 30616)는 그대로 두었다.
3. **팀장 정본 무변경** — `Data/Rendering/Authored/RenderingProfiles.json` 은 읽기만 했고
   revision 85 그대로다. `Publish-RenderingProfiles.ps1` 을 실행하지 않았다.
4. **exit code 정직 기록** — component lighting 3회 모두 exit **2 (`PARTIAL_UNSUPPORTED`)** 이며
   PASS 로 쓰지 않았다. 회귀 2건은 실제로 exit 0 / `OK` (32 tests, 13 tests) 다.
5. **끝내지 못한 것** — UV1 이 UV0 복제가 아닌지 숫자 검증, 3348 unsupported component 의 RNM 유무,
   lightmap DDS 추출, `_RNM_` variant cook, mapmaterials 실제 병합, luma·rotation 상수 확정,
   Landscape 46/16 차이. 전부 11절에 미해결로 남겼고 완료로 표시하지 않았다.
6. **화면 판정 없음** — Client 를 실행·조작·캡처하지 않았고 visual PASS 를 기록하지 않았다.
