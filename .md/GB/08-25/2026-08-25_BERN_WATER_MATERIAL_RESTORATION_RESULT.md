# 베른성 물 머티리얼 복원 결과

작업일: 2026-08-25
브랜치: `worktree-bw` (origin/main 기준)
Area: `LV_BER_BERNCASTLE`

## 1. 한 줄 결론

베른성의 큰 회색 판은 원본이 `BLEND_Translucent`인 물을 프레임워크가 `Opaque`로 그리고 있었기
때문이다. 원인은 추출 파이프라인이 MaterialInstanceConstant 체인의 텍스처 역할·스칼라·벡터·BlendMode를
버린 것이며, 이번 작업은 그 계약을 보존하도록 파이프라인을 고치고, 회수한 값을 별도 물 표현 문서로
publish해, 전용 Water pass가 그 값으로 그리게 연결했다.

### 2026-08-26 런타임 계약 교정

최초 구현은 `CMapAssetObject`가 실제로 복제하는 `Shader_VtxMeshBinary.hlsl`이 아니라
정적 배치 전용 `Shader_VtxMeshMapInstance.hlsl`에만 물 변수와 pass를 추가했다. 그 결과 베른과
발탄 진입 후 첫 BLEND map object가 존재하지 않는 물 변수를 bind하면서 `E_FAIL`을 반환했고
Client main loop가 종료됐다. 교정 내용과 검증 증거는
`../08-26/2026-08-26_MAP_WATER_RENDER_CONTRACT_CRASH_FIX_RESULT.md`가 정본이다.

## 2. 진단에서 실측으로 확정한 사실

`Data/Maps/Imported/LV_BER_BERNCASTLE/*.mapassets`와 원본 `.props.txt` 체인을 직접 대조한 결과다.

- 베른의 물 후보 에셋은 5종이고 배치는 visible 21개다. 전부 `Opaque Back`으로 등록돼 있었다.
  - `LV_BER_BERNCASTLE_WATER01_SM` SL03 1개
  - `BG_RHD_BREEZE_RUMINAWATER01_SM_ASJ` SL00 2 + SL06 7
  - `BG_LUT_MEMONAST_HOLYWATERPOLLUTION01_SM_YSI` SL01 8 + SL07 2 + SL09 1
- `LV_MODULE_WATER01_1024` / `LV_MODULE_WATER02_512`는 머티리얼 이름이 `dummy_material_0`이고
  텍스처가 0장이다. 배치 11개는 모두 `visible=0`이다. 켜면 회색 판이 생기므로 계속 숨긴다.
- `LV_BER_BERNCASTLE_WATER01_SM`의 `.wmodel`에는 `crystal_bluegray_blur.dds`와 `t_snow_normal.dds`가
  UTF-16 경로로 정상 바인딩돼 있었다. 즉 1×1 회색 폴백이 아니라 실제 청회색 blur를 불투명하게
  칠하고 있었다.
- 원본 체인은 4단계이며 자식이 부모를 덮어쓴다.

```text
lv_atm_koilsv_water_01_mi        texture_diffuse=crystal_bluegray_blur
                                 texture_reflection=lv_common_wet_04_d   <- 최종값
  └ preset_water_riverwindless   (스칼라/벡터만)
     └ preset_water              texture_normal=t_snow_normal
                                 detail_texture_normal=waterbump_tex
                                 texture_reflection=reflection_forest01  <- 자식이 덮어씀
        └ preset_waterbase_trn   BlendMode=BLEND_Translucent, TwoSided=false
                                 bDisableDepthTest=false, bIsMasked=false
                                 CollectedTextureParameters: texture_foam=fx_c_water_001,
                                 texture_opacity=voyage_mask_texture 등
```

- `Renderer.cpp`의 `Render_Blend()`는 우선순위는커녕 카메라 거리 정렬조차 없이 삽입 순서로만 그렸다.
- `Render_Blend()`는 `Begin_MRT("MRT_SceneHDR")` 안에서 `SCENE_HDR_RT0_SCENE_COLOR_RT1_DISTORTION`
  계약으로 실행되고, `Shader_Deferred.hlsl`이 `g_DistortionTexture.rg`를 ±0.05로 clamp해
  `SceneHDR.Sample(uv + distortion)`으로 적용한다. 굴절은 새 렌더 타깃 없이 이 경로로 가능하다.

## 3. 구현한 것

### 3.1 추출 파이프라인 (`Tools/BernCastlePipeline/build_bern_castle_assets.py`)

- `parse_material_document()`를 새로 만들어 텍스처 파라미터 외에 Scalar/Vector 파라미터,
  마스터의 `CollectedTextureParameters`, `BlendMode`/`TwoSided`/`bIsMasked`/`bDisableDepthTest`/
  `OpacityMaskClipValue`를 읽는다. 기존 `parse_material_props()`는 이 함수를 감싸는 호환 래퍼로 남겨
  기존 테스트를 깨지 않는다.
- `material_contract()`가 체인을 root-first로 접는다. 마스터의 collected 텍스처가 기본값을 깔고
  자식의 `TextureParameterValues`가 덮어쓴다. 기존 `material_texture_roles()`는 래퍼로 유지한다.
- `AUXILIARY_TEXTURE_PARAMETERS`를 신설했다. `.wmodel`에 슬롯이 없는 reflection / detailNormal /
  foam은 wmodel로 remap하지 않고 에셋 옆에 복사한 뒤 receipt에 역할로 기록한다.
- receipt를 `schemaVersion 2`로 올리고 `receipt_is_valid()`가 구 스키마를 미완료로 취급하게 했다.
  UModel이 내보내지 못한 텍스처는 실패시키지 않고 `missingTextures`로 남긴다.
- `--asset-id` 필터를 `extract`/`all`/`cook`에 추가했다. 인벤토리의 950 에셋 게이트는 그대로 두고
  실제 작업 목록만 좁힌다. 부분 선택일 때는 runtime manifest 재생성을 건너뛴다.

### 3.2 물 표현 문서 (`Tools/MapPipeline/build_map_water_presentation.py` 신규)

source receipt에서 물 계약을 읽어 authoring 문서를 만든다. 이름이 아니라 **마스터의 BlendMode**로
물을 판정한다. 모든 머티리얼이 Translucent인 에셋만 `waters[]`에 넣고, 머티리얼마다 BlendMode가
다른 에셋은 사유와 함께 `deferred[]`에 기록한다. `--apply-render-mode`는 admission된 에셋의 catalog
renderMode만 `Water`로 바꾸며, 행의 나머지 서식은 건드리지 않는다.

### 3.3 데이터

- `Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapwater.json` 신규
  (`lostark.map-water-presentation` v1)
- `Data/Maps/MapCatalog.json`에 `sourceWater`/`water` 쌍 선언
- `LV_BER_BERNCASTLE_SL03.mapassets`의 `WATER01_SM` 행이 `Opaque Back` -> `Water Back`
- `Tools/MapPipeline/Publish-MapAuthoring.ps1`이 기존 lights 쌍과 같은 방식으로 물 문서를 strict
  validate하고 같은 트랜잭션으로 publish한다

### 3.4 런타임

- `MAP_ASSET_RENDER_MODE::WATER` 추가, catalog가 `"Water"` 토큰을 파싱
- `MapAssetRenderUtils::Select_Pass()`의 modeOffset에 WATER=15 추가. 기존 pass 0~14는 이동 없음
- `CMapAssetCatalog`가 `MAP_ASSET_WATER_PROFILE`과 `Load_WaterPresentation()`을 소유한다.
  **양방향 fail-closed**: WATER 에셋에 행이 없거나, 행이 있는데 WATER가 아니면 Area 로드가 실패한다
- `CMapAssetObject`가 water 프로파일을 받아 draw 직전에 스칼라/벡터를 바인딩하고 draw 후 identity로
  되돌린다. 공유 FX11 effect를 쓰므로 다음 오브젝트가 물 값을 물려받지 못하게 한다
- `CMapPlacementRuntime`이 catalog에서 물 행을 찾아 DESC에 넣는다
- 셰이더에 `PS_MAIN_WATER`와 `WaterBack/Front/TwoSided` pass 3개 추가. RT0에 색, RT1에 화면공간
  왜곡을 쓴다. 노멀 2장 패닝, 프레넬, 반사, `diffuse_color` 틴트, opacity/opacity_power를 소비한다
- `Renderer::Render_Blend()`가 카메라 거리 내림차순(먼 것 먼저) 안정 정렬 후 그린다. 카메라 위치를
  못 얻으면 기존 삽입 순서로 fail-open한다

### 3.5 회수한 텍스처

재추출로 아래 5장을 되찾아 `Client/Bin/Resources`에 배치했다. `.wmodel`은 재cook 후에도 바이트
동일이므로 기존 산출물은 바뀌지 않았다.

```text
WATER01_SM/textures/lv_common_wet_04_d.dds        65,664
WATER01_SM/textures/waterbump_tex.dds          1,048,704
RUMINAWATER/textures/ambientreflection_09.dds      8,320
HOLYWATER/textures/reflection_forest01.dds       131,200
HOLYWATER/textures/t_waterwake_norm.dds          262,272
```

## 4. 실행한 자동 검증

```text
python -m py_compile build_bern_castle_assets.py                      PASS
python -m unittest test_build_bern_castle_assets.py                   3/3 PASS
python -m py_compile build_map_water_presentation.py                  PASS
파이프라인 extract --asset-id 5종                                      5/5 exported
파이프라인 cook --asset-id 5종                                         5/5 cooked, .wmodel 바이트 동일
접힌 계약 실측: 텍스처 6, 스칼라 14, 벡터 5, BlendMode=Translucent      확인
build_map_water_presentation 생성                                     waters 1 / deferred 1
build_map_water_presentation --apply-render-mode                      SL03 1행만 변경(126637->126636 bytes)
Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE                    PASS, 13 shards / 50,017 placements
런타임 문서 생성 확인                                                  Client/Bin/DataFiles/Map/*.mapwater.json
fxc /T fx_5_0 Shader_VtxMeshMapInstance.hlsl                          컴파일 성공 (X4717 deprecation 경고만)
MSBuild Engine.vcxproj Debug x64                                      Engine.dll 생성 성공
UpdateLib.bat Debug                                                   완료
git diff --check                                                      clean
```

## 5. 아직 검증하지 않은 것

- **Client x64 Debug 빌드.** 이 RESULT 작성 시점에 실행 중이다. 미검증 파일은
  `MapAssetCatalog.{h,cpp}`, `MapAssetObject.{h,cpp}`, `MapPlacementRuntime.cpp` 세 묶음이다.
- **화면 판정.** 사용자 전용이다. 에이전트는 Client를 실행하거나 캡처하지 않았고 어떤 것도
  visual PASS로 기록하지 않았다.
- 워크트리에는 `Client/Bin/Resources`가 없다(Git 제외). 워크트리 EXE로 실행하려면
  `LOSTARK_RESOURCE_ROOT`를 본체 체크아웃의 Resources로 지정해야 한다.

## 6. 의도적으로 남긴 경계

- **혼합 BlendMode 에셋.** `HOLYWATERPOLLUTION01`은 머티리얼 0이 Opaque, 1이 Translucent다.
  렌더 그룹과 pass는 오브젝트당 한 번 정해지므로 메시를 두 제출로 쪼개기 전에는 정확히 그릴 수 없다.
  문서에 `MIXED_MATERIAL_BLEND_MODES` 사유로 남기고 authored render mode를 유지했다. 배치 11개.
- **RUMINAWATER 9개는 물이 아니다.** 재추출 결과 두 머티리얼 모두 `BLEND_Opaque`다. 원본이 불투명이다.
- **보조 텍스처 미바인딩.** reflection / detailNormal은 회수·배치·문서화까지 했지만 런타임이 아직
  SRV를 소유하지 않아 `g_HasReflectionTexture`/`g_HasDetailNormalTexture`가 0이다. `CTexture`
  프로토타입 등록 경로를 여는 별도 슬라이스가 필요하다.
- **`texture_foam`/`texture_opacity` 미회수.** 마스터가 선언하지만 UModel이 이 에셋 export에서
  내보내지 않았다. receipt의 `missingTextures`에 기록돼 있다.
- **opacity 합성식은 재구성이다.** 원본 opacity 그래프가 cooked 패키지에 없다. 저작값 0.8 / 0.6은
  그대로 쓰고 프레넬로 가장자리를 닫는 방식은 `PROJECT_RECONSTRUCTED`이며 셰이더 주석에 명시했다.
- **폭포·안개 파티클 42개는 막혔다.** 매니페스트에 stable id·좌표·yaw·drawScale·
  `translucencySortPriority`(폭포 -2가 31, 100이 2 / 안개 -2가 9)·`maxDrawDistance`·
  `occlusionCulling`이 전부 있지만, 원본 Cascade가 프로젝트 Effect 형식으로 변환된 적이 없고
  `EffectCatalog.json` 276행에 Bern 항목이 0개다. 소비자 없는 문서를 만들지 않았다.
- **정렬은 거리 기준까지만.** `translucencySortPriority`는 파티클이 들어올 때 같이 열어야 한다.

## 7. 재현 명령

```powershell
python Tools/BernCastlePipeline/build_bern_castle_assets.py extract `
  --placements-dir <추출>/bern/placements --placements-dir <추출>/bern/placements_rest `
  --output-root <추출>/bern_full --umodel <umodel>/umodel_lostark_v7.exe `
  --package-root "<게임설치>/EFGame/ReleasePC/Packages" --workers 5 --force `
  --asset-id MAP_5387B1504BDD_LV_BER_BERNCASTLE_WATER01_SM

python Tools/MapPipeline/build_map_water_presentation.py `
  --area-id LV_BER_BERNCASTLE --catalog-dir Data/Maps/Imported/LV_BER_BERNCASTLE `
  --source-root <추출>/bern_full/source `
  --output Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapwater.json `
  --resource-root Client/Bin/Resources --apply-render-mode

powershell -ExecutionPolicy Bypass -File Tools/MapPipeline/Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE
```

`--check`를 붙이면 문서를 다시 만들지 않고 현재 문서가 source와 어긋났는지만 검사한다.

## 8. 2026-08-26 Client 렌더 회귀 교정

최초 RESULT의 Client 빌드 미검증 경계에서 실제 회귀가 확인됐다. `CMapAssetObject`는
`Shader_VtxMeshBinary.hlsl`을 소비하지만, Water 변수·pixel shader·pass는 instanced 전용
`Shader_VtxMeshMapInstance.hlsl`에만 추가돼 있었다. 그 결과 Water가 아닌 발탄 phase proxy도
identity Water 값을 바인딩하는 첫 draw에서 `E_FAIL`을 반환했고 Client가 level 5에서 종료됐다.

교정 내용:

- Binary shader에 동일한 Water ABI와 `PS_MAIN_WATER`를 추가했다.
- 기존 pass 0~14를 유지하고 Water pass를 15~17에 배치했다.
- 기존 `DeferredEmissiveOverlayPass`와 유일한 C++ 소비자를 함께 pass 18로 이동했다.
- 발탄의 검정 aperture/red ring/burgundy cloud `PresentationVortex` 분기는 변경하지 않았다.
- source-contract 테스트가 Water 바인딩 이름, pass 15~18 순서, Water base 15,
  emissive overlay 18, Valtan vortex profile 1~3을 함께 고정한다.

검증:

```text
fxc /T fx_5_0 Shader_VtxMeshBinary.hlsl                            PASS
python Tools/LevelPlacementExtractor/test_valtan_floor_emissive_contract.py
                                                                    7/7 PASS
MSBuild Client Debug|x64                                            PASS
MSBuild Client Release|x64                                          PASS
git diff --check                                                    PASS
```

Client 자율 실행과 화면 판정은 하지 않았다. 발탄 진입 시 신규 `RendererExit.user.log`가 생기지
않는지, vortex가 초록/청색 사각 카드 없이 보이는지, 베른 물과 발탄 균열 emissive가 유지되는지는
사용자 수동 smoke 대상으로 남는다.
