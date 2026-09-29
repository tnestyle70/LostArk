# 마하라카 물·재질 복원 RESULT

대상 Area `LV_OCN_EVENTIS_MHP` (Lobby `Maharaka` → `LEVEL::MAHARAKA`).
조사 시작 2026-09-25 19:35 KST, 이 문서 파일은 20:11에 처음 만들었고 이후 갱신했다. 2026-09-25 21:47부터 21:59까지 실제 저장소 반영·게시를 수행했다(사용자가 "빌드는 하지 말고 문제 해결과 퍼블리셔만 먼저 끝내라"고 명시).

사용자 요청: 화면(영상)의 마하라카가 원작 조감도와 다르고 물이 안 보인다. 베른이 물을 어떻게 표현하는지 보고 마하라카에 적용하고, 빠진 재질과 나머지도 가능한 만큼 복원한다.

**상태: 데이터 반영·게시·C++ 진단 로그 패치가 전부 끝났다. 빌드는 하지 않았다(사용자가 VS로 직접 한다).** 아래는 실제로 적용하고 검증한 결과다. Client와 게임 UI는 실행하지 않았다. 화면이 원작과 같은지는 판정하지 않는다(사용자 몫). 아래 수치는 파일·문서·게시 검사와 C++ 계약(Parse_MaterialOverrides) 대조 결과다.

## 1. 관찰 (사용자 첨부 이미지 두 개, 열람·분석)

원작 조감도(`스크린샷 2026-09-25 192953.png`)에서 보이는 것: 진한 청록 물 풀과 바다, 가운데 분홍·보라 줄무늬 원형 워터팡 아레나와 노랑 원판, 아레나 둘레 분수, 배 모양 바이킹 레스토랑, 무지개 슬라이드, 모코모코 어트랙션, 야자수, 흰 모래 길, 초록·노랑 원형 정원.

현재 화면 영상(`마하라카.mp4`, 1912×1080 5.7초 171프레임에서 10프레임 추출, `out/MaharakaWater20260925/frames/`)에서 보이는 것:
- 바다가 파랗지 않다. 섬 밖 전체가 흰색·연보라 구름결에 어두운 청록 얼룩이 섞인 한 장짜리 무늬로 크게 늘어나 보인다.
- 섬 가운데 풀 자리도 같은 청록·흰 소용돌이 무늬다. 파란 물이 아니다.
- 아레나 원판과 바닥이 회백색이고 분홍·보라 줄무늬가 없다. 오른쪽 아래 넓은 판에는 노란 얼룩이 있다.
- 식생(야자수)과 배 식당 바닥 타일은 색이 있다. 왼쪽 위 바다에 색 없는 노르스름한 직사각 판이 하나 떠 있다.
- 분수, 슬라이드의 물, 원작 조감도의 채도는 보이지 않는다.

## 2. 베른이 물을 표현하는 방식 (사실)

근거: `.md/GB/09-11/2026-09-11_BERN_NATIVE_WATER_IMPLEMENTATION_RESULT.md`, `.md/GB/08-25/2026-08-25_BERN_WATER_MATERIAL_RESTORATION_RESULT.md`, `Client/Public/SourceMapWaterMaterialParameters.h`, `Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.mapmaterials.json`(직접 열어 family 개수 집계).
- 베른의 물은 `mapwater.json`(옛 물 표현 문서)만으로 그려지지 않는다. 물 재질마다 `mapmaterials.json`에 `family = source.map.water-38 … water-43` 행이 있고(베른 물 family 행 83개), 원본 픽셀 셰이더 여섯 종(프로그램 38~43)이 원본 MIC 값으로 그린다. depth fade, Fresnel, 반사, 굴절이 그 셰이더 안에서 계산된다.
- 그 물은 반투명 forward 경로다. 불투명 장면을 `Target_EffectSceneColor`에 복사한 뒤 반투명 물이 읽는다.
- 베른 `mapwater.json`은 행이 1개뿐인 보조 문서다.

## 3. 마하라카 물이 안 보이는 원인

**확정한 사실 (코드·데이터로 확인)**
1. 마하라카는 `mapmaterials.json`이 없다. 그래서 물 10행이 전부 옛 경로 `mapwater.json → Bind_WaterShaderResources → PS_MAIN_WATER`(Shader_VtxMeshBinary pass 15~17)로만 그려진다. `Client/Private/MapAssetObject.cpp:249-262`.
2. 마하라카 바다의 원본 부모 재질은 `specialresource.mat.ocean_trn`이다. 베른의 물 여섯 종(preset_waterbase_trn 계열)과 다른 마스터다. 이 재질을 그리는 프로그램은 저장소에 없다(`Client/Public`, `Client/Private`, `Engine`, `Tools/LevelPlacementExtractor`, `Tools/MapPipeline` 검색 결과 없음). 원본 값에는 `sky_color`(0.12, 1.0, 0.95 청록), `fresnel_color`, `texture_sky`, `wave_*`, `base_distortion` 등 옛 물 문서가 읽지 못하는 항이 있다(`out/MaharakaWater20260925/analysis/ocean_trn_handoff.json`).
3. 옛 PS는 표면색을 메시의 diffuse 텍스처에서 읽는다. 바다 모델이 실제로 참조하는 텍스처는 `fx_c_water_001.dds`와 `t_snow_normal.dds`다(WModel의 UTF-16 문자열로 확인). `fx_c_water_001`은 흰 거품에 어두운 청록 얼룩이 섞인 무늬이고(디코드해서 확인, `out/.../analysis/*.png`), 영상의 바다 무늬와 닮았다.
4. 바다·풀 물 평면의 `opacity`(0.03, 2.0, 1.0)는 원본에서 depth fade 비율로 쓰이는 값인데, 옛 PS는 그냥 알파로 읽는다. 알파가 1로 포화하는 평면(2.0, 1.0)은 불투명 면이 되고 0.03 평면은 알파 약 0.17로 거의 안 보인다(코드 `PS_MAIN_WATER`의 식에서 계산).
5. 풀 바닥 같은 색 있는 재질도 색이 없다: `POOL01` 모델이 실제로 참조하는 텍스처는 엔진 기본 회색 `diffuse.dds`(128,128,128)와 `normal.png`다(WModel 문자열로 확인). 원본 MIC의 `diffuse_color (1.0, 0.458, 0.158)`, `diffuse_brightness 2.0`은 런타임이 읽는 재질에 없다. 이전 RESULT(`2026-09-25_MAHARAKA_WATERBOMB_RESTORATION_RESULT.md` 7절)의 설치 receipt는 `materialComplete 0/382`로 같은 사실을 적고 있다(그 수치는 이번에 다시 재지 않았다).

**추론 (화면으로 확인하지 못함)**
- 사용자가 본 "베른 때와 같은 원인"은 맞다: 원본 재질 파라미터가 런타임에 연결되지 않았다.
- 화면 전체의 회백색은 재질 색 누락에 더해, 마하라카가 중립 환경(팀장님 규칙으로 환경 프로필을 제거한 상태)이고 원본 그림자 맵 조명이 없는 것과도 관련이 있을 수 있다. 얼마나 기여하는지는 모른다.
- 바다 평면 `LV_MODULE_WATER02_512`(배율 100)의 한 변 크기는 이름의 "512"와 배율로 추정한 512 m다. 모델 경계를 측정하지 않았다. 거품 무늬가 그 넓이로 늘어난다는 판단은 이 추정에 의존한다.

**미확인**: 화면에서 실제로 어떻게 보이는지.

## 4. 재질 복원 경로: 이번에 새로 확인하고 out/에 만든 것

이전 RESULT는 "입력 manifest를 만드는 도구가 저장소에 없고 ShadowMap2D 때문에 막혔다"고 적었다. 재검토 결과:
- `Tools/LevelPlacementExtractor/build_source_map_materials.py`는 deferred 계열(`bg_base_opa/msk`, simple, overlay, foliage, PBR)만 컴파일한다. translucent·water·sky 프로그램(`source.map.translucent-N`, `water-N`)을 만드는 도구는 저장소에 없다(팀장 쪽 도구).
- 조명 evidence가 `UNSUPPORTED_NATIVE_LAYOUT`(ShadowMap2D)인 슬롯도 컴파일러가 `bakedLighting` 없이(`lightingEvidence = source-absent`) 받아 준다. 이번에는 그림자 맵을 소비하지 않고 재질 값만 연결한다. 이 라벨은 컴파일러 운반용이며 원본에 조명이 없다는 뜻이 아니다. receipt의 `projectApproximations`에 적었다.
- 입력 manifest를 만드는 어댑터를 **새로 작성했다. 파일 위치는 `out/MaharakaWater20260925/tools/build_source_map_material_inputs.py`이며 저장소 `Tools/`에는 아직 없다.** 게이트 뒤에 `Tools/LevelPlacementExtractor/`로 설치할지는 6절과 9절에서 정한다(설치 예정이며 지금은 설치하지 않았다). 이전 스테이징(`C:\LostArkExtract\LV_OCN_EVENTIS_MHP_20260919`)의 parameters·mip 카탈로그·재쿡 manifest를 그대로 입력으로 쓴다.
- 엔진 기본 텍스처(`efmaster_material_prologue.tex.normal/flat_gray/flat_normalmap`)는 설치본이 PNG라 컴파일러가 받지 못했다. 원본 패키지에서 UModel로 꺼내 내용을 확인했다(모두 2×2 균일색 PF_A8R8G8B8, 밉 2단, `normal`·`flat_normalmap`은 `srgb=false`, `flat_gray`는 기본 srgb). 같은 값으로 DDS를 만들었다.
- **게이트 전에 만든 새 파일(기존 파일 덮어쓰기 없음)**: `Client/Bin/Resources/Map/LV_OCN_EVENTIS_MHP_SOURCE_MATERIALS/EngineDefaults/` 아래 148바이트 DDS 3개, 2026-09-25 19:59 생성. `68be1aa40f15_flat_normalmap.dds`, `74ab8c1a3b74_flat_gray.dds`, `ef8bb9743550_normal.dds`. Resources는 Git 비추적이고 새 폴더라 병합 작업과 겹치지 않는다. 다른 Pillow 디코더로 열어 값을 확인했다.

컴파일 결과(재쿡 변형 자산 382종, 슬롯 기준):
- 컴파일된 재질 행 319개(bg-source-opaque-masked 307, bg_base_opa_overlay 12), 자산 241종. 한 자산의 슬롯이 하나라도 못 만들어지면 그 자산은 통째로 옛 재질 경로에 남긴다(scene 승인이 자산 단위로 묶기 때문). 자산이 쓰이는 배치 3,833개 중 2,582개가 묶인다.
- 못 묶인 이유 상위(배치 수 기준): 텍스처가 설치본에서 PNG(비압축 A8R8G8B8 노멀맵, 약 118 배치), 식생 vertex wind 검증 바인딩 없음(약 750 배치), null 재질 슬롯(211), translucent 마스터 `bg_base_trn`·`depthtest`·`molding_trn`·`preset_flag_vertical` 프로그램 없음(약 200), 미지원 static switch `use_wind`/`use_linear_blend`(약 100), 노멀 텍스처 null. 자세한 표는 `out/MaharakaWater20260925/materials_v2/slot_report.json`.
- 재질 행의 값 범위 확인: diffuseBrightness 0.8~7.0, specularIntensity 0~30, 노멀 텍스처와 색공간 문제 없음, 이미시브 6행.
- 컴파일러가 만든 행을 C++ 파서 제약(`Parse_MaterialOverrides`의 두 family 분기)을 손으로 옮긴 검사기로 확인: 319행 오류 0(`out/.../tools/check_rows_like_cpp.py`). 이 검사기는 진짜 C++ 파서가 아니다.

바다 물 근사(데이터만): `author_ocean_water_rows.py`가 ocean 재질 7종 자산을 베른 물 프로그램 41(`source.map.water-41.v1`) 행으로 덧붙인다. ocean MIC와 preset-water는 texture lane 4개(normal, detail normal, diffuse, reflection)와 대부분의 parameter 이름을 공유한다. 겹치는 값은 ocean MIC 값을 그대로 복사했고 `diffuse_color`는 `sky_color × sky_intensity`, `reflection_color`는 `fresnel_color`(원본 fresnel 항이 켜진 경우), 모듈 평면의 `diffuse_tiling`은 약 6 m 거품 타일로 정했다. **이것은 ocean_trn 셰이더가 아니다. 프로젝트 근사다.** 강 재질(`river_water_mi` chain) 3행은 원본 그대로 둔다.

## 5. 지금까지 확정한 것 요약

**확정된 사실**
- 마하라카는 재질 문서가 없어서 원본 색·밝기·specular·노멀 세기·이미시브가 하나도 연결되지 않았다.
- 바다는 옛 물 경로로만 그려지고, 그 경로는 ocean_trn의 색항(sky_color, fresnel_color)과 depth fade 의미를 읽지 못한다.
- 베른과 같은 방식(재질 문서 + 원본 물 프로그램)은 마하라카의 바다에는 그대로 못 쓴다: 프로그램이 다른 마스터용이다.
- deferred 재질 319행을 원본 파라미터로 컴파일할 수 있고(자산 241종), 게시기 Validate/Publish/Check가 통과한다(아래 7절).

**추론**
- 재질 연결이 회백색 문제의 큰 부분을 줄일 가능성이 높지만, 환경(태양·하늘·안개·후처리)과 그림자 맵이 없는 점이 남는다.
- 프로그램 41 근사가 실제 바다처럼 보일지는 화면을 봐야 안다.

**미확인**: 모든 화면 결과.

## 6. 저장소에 실제로 반영한 것 (2026-09-25 21:47~21:59)

사용자가 "다른 작업(병합, 실패 수정) 대기 없이 지금 반영하고 퍼블리셔까지 돌려라, 빌드는 나중에 VS로 직접 한다"고 확정해 게이트를 건너뛰고 바로 반영했다. 시작 시점 `out/MaharakaWater20260925/git_status_start_C.txt`에 기록. 실행 순서:

1. 백업+반영: `python out/MaharakaWater20260925/tools/apply_live.py`(compare-and-swap, 실패 시 자동 원복 로직 있음). 바뀐 파일:
   - `Data/Maps/MapCatalog.json`: 마하라카 행에 `sourceMaterials`/`materials` 두 줄 추가. 새 크기 14,497 bytes, sha 앞 16자 `2704f59002d9788a`.
   - `Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapmaterials.json`(새 파일, 326행 = 컴파일 319행 + 바다 근사 7행): 610,873 bytes, sha `c527013f3938d2a6`.
   - `Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapwater.json`(revision 1 → 2, 10행 중 7행 `PROJECT_AUTHORED`, 3행 `SOURCE_MATERIAL_EXACT`): 17,538 bytes, sha `481e9c730f00479a`.
   - 백업은 `out/MaharakaWater20260925/backup/`에 원본 8개 파일과 `manifest.json`으로 있다. 되돌리려면 `python out/MaharakaWater20260925/tools/apply_live.py --restore`.
2. 게시(마하라카 Area만, 빌드 잠금 사용): `Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP` Validate(22초)→Publish(4초)→Check(4초), 전부 exit 0. `PlacementCount 4651, FileCount 5` 동일 유지. 바뀐 게시 출력:
   - `Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.mapassets`: 머리줄만 `LOSTARK_MAP_ASSET_CATALOG 5 "LV_OCN_EVENTIS_MHP" 406 "LV_OCN_EVENTIS_MHP.mapmaterials.json"`로 바뀌고 나머지 406행은 그대로(220,586→220,625 bytes, 헤더 줄 길이 차이뿐).
   - `Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.mapmaterials.json`(새 파일, 저작본과 바이트 동일 610,873/`c527013f3938d2a6`), `.mapwater.json`(저작본과 동일 17,538/`481e9c730f00479a`).
   - `.mapplacements`(1,175,993 bytes, sha `1b169867e05c982b`)와 `.maplights.json`(16,961 bytes, sha `3d69ad796d3ed27d`)은 게시 전후 바이트가 완전히 같음을 확인(무변경).
   - 렌더링 프로필 게시(`Publish-RenderingProfiles.ps1`)는 실행하지 않았다.
3. C++ 진단 로그(`Client/Private/Level_Development.cpp`, 9,101→10,210 bytes): include 한 줄(`EffectFailureDiagnostic.h`)과 두 로그를 추가했다. 실패 경로에 `map.area.load-failed`(`m_MapRuntime.Get_Status()`), 마하라카 진입 성공 시 `map.water.loaded area=… assets=… waterAssets=… waterRows=… materialAssets=…`를 `Client/Default/EffectFailure.user.log`(`Write_EffectFailureDiagnostic`, 기존 채널 재사용)에 남긴다. 사용한 심볼(`CMapAssetCatalog::Get_Entries/Find_Water/Get_AreaId`, `MAP_ASSET_ENTRY::renderProfile.renderMode/materialOverrides`, `MAP_ASSET_RENDER_MODE::WATER`)을 각각 헤더에서 실재 확인했다. 백업은 `out/MaharakaWater20260925/cpp/Level_Development.cpp.before`(패치 전 원본, sha `f9739b55f79f10b2`). **빌드는 하지 않았다.** 대신 `cl /Zs`(구문 검사만, 산출물 없음, `/std:c++20` — 프로젝트의 실제 `LanguageStandard`와 맞춤)로 확인: EXITCODE=0, 실제 오류 0건(남은 경고는 기존 파일들의 한글 주석에 대한 C4819 코드페이지 경고뿐, 내 패치와 무관). 이것은 문법 확인일 뿐 링크·전체 빌드가 아니다.
4. 새 Resources 전달 목록 생성(`out/.../tools/make_distribution.py` 실행): `Resource_Distribution_2026-09-25_MaharakaMaterials.txt`, `Copy_ResourceDistribution_2026-09-25_MaharakaMaterials.ps1`(3개 파일, 444 bytes) 생성 완료, 저장소 루트에 있다. **복사 스크립트는 실행하지 않았다.**
5. 도구 설치: `Tools/LevelPlacementExtractor/build_source_map_material_inputs.py`, `author_ocean_water_rows.py` 설치(둘 다 python 구문 확인 완료), `README.md`에 두 절 추가(CRLF·no-BOM 유지, 파일 끝에 이어붙임, `git diff --check` 클린). `.vcxproj` 등록은 필요 없다(순수 python 스크립트).

## 7. 검증 (실제 반영·게시 결과에 대해 실행)

`out/.../tools/verify_live.py`를 반영·게시가 끝난 실제 트리에 대해 실행: **PASS 9건, FAIL 1건**(runtime 5개 파일이 격리 scratch 게시 결과와 바이트 동일, MapCatalog pair, 텍스처 참조 전부 존재, 물 asset/row 10대10 일치, mapassets 헤더 v5).
FAIL은 `rows pass the C++ constraint port`(검사기 `check_rows_like_cpp.py`가 바다 근사 7행을 "unexpected family"로 거부)였다. **이것을 실제 C++ 계약으로 재조사했다**(사실):
- 이 검사기는 애초에 `bg-source-opaque-masked`/`bg_base_opa_overlay` 두 family만 손으로 옮긴 부분 포트이고 native family(`source.*`)는 다루지 않는다(스코프 한계이지 결함 신고가 아니다).
- 실제 파서(`Client/Private/MapAssetCatalog.cpp` 668행 `family.starts_with("source.")` → `SourceCharacterMaterial::Configure` → `SourceMapWaterMaterial::Configure`, `Client/Public/SourceMapWaterMaterialParameters.h` 106~139행 water-41 분기)를 직접 읽고 별도 검사기(`C:\Users\USER\.claude\jobs\46aea322\tmp\check_ocean_rows_real_contract.py`)로 7행 전부를 재검증했다: parameter 20개 키 정확히 일치(`selectioncolor`~`specular_power`), texture 4개가 expressionIndex 0~3을 정확히 채움(요구 mask `baseTextureMask|lightTextureMask`=15), 참조 텍스처 8개(4개×상호참조 등) 전부 `Client/Bin/Resources`에 실재, top-level 필드는 `renderMode`(값 `"translucent"`)와 `cullMode`(값 `"back"`)를 포함해 전부 유효(이 둘은 `MapAssetCatalog.cpp` 533행에서 native-family exactFields 검사 전에 분리되므로 native 필드셋과 충돌하지 않는다). **재검사 결과 errors 0.** 베른의 실제 water-41 행(15개)과 필드 형태를 직접 대조해 같은 스키마임도 확인했다.
- 결론: 바다 근사 7행은 실제 C++ 파서 계약을 만족한다(코드 대조로 확인, 실제 실행·빌드로 확인한 것은 아님). `verify_live.py`의 FAIL은 검사기 스코프 한계이지 데이터 결함이 아니다.
확인하지 못한 것: 진짜 C++ 파서를 통한 실제 로드(빌드하지 않았다), 프로그램 41 근사 행이 화면에서 실제로 어떻게 보이는지, 어떤 화면도.

## 8. 렌더링 규칙 때문에 못 하는 것과 대안

- **`ocean_trn` 전용 프로그램**: 원본 pixel shader를 HLSL로 옮긴 native 프로그램 추가(Shader_SourceMapWaterPrograms 계열, CSO 변형 그룹, C++ 파라미터 패커)가 필요하다. `Client/Bin/ShaderFiles`와 렌더링 코드는 팀장님 정본이라 **보류, 팀장님께 요청 필요**.
- **환경(태양·하늘·안개·후처리)**: 원본 값은 이전 RESULT 3-1절에 있다. 렌더링 옵션이므로 적용하지 않고 값만 넘긴다.
- **지도 데칼 25개**: 런타임 소비자(렌더 경로)가 없다. 렌더링 영역이라 보류.
- **데이터 레이어만으로 가능한 대안(6절 1번에 준비함)**: (가) 옛 물 문서 값 재사상(`PROJECT_AUTHORED`), (나) 베른 프로그램 41에 ocean MIC 값을 넣은 근사 행. 두 대안 모두 셰이더·렌더러·`Data/Rendering`·렌더링 옵션 값을 바꾸지 않고 Area 데이터(`mapwater.json`, `mapmaterials.json`)만 바꾼다. 따라서 렌더링 규칙 위반은 아니라고 판단한다. 다만 프로그램 41을 다른 마스터의 재질에 쓰는 것이 팀장님의 재질 규칙에 맞는지는 확인이 필요하다.
- **원작과 달라지는 점**: ocean_trn의 wave 정점 이동(`use_wave`, 강도 25와 -10), `sky_*` 텍스처 겹침, 원본의 정확한 depth fade 곡선, 굴절 세부가 빠진다. 프로그램 41 행에서 원본과 다른 항이 색과 거품 타일링뿐이라는 보장은 없다. 색이 어둡거나 과포화일 수 있다. 되돌리기는 반영 전 백업 복원이거나 덧붙인 7행 삭제 후 재게시다.

## 9. 아직 못 한 것과 이유

- 식생 vertex wind(41 슬롯, 약 750 배치): 원본 shader-map·vertex program 근거 추출물이 저장소에 없다.
- translucent 계열(약 30 슬롯, 약 200 배치): 해당 프로그램 매핑 도구가 저장소에 없다.
- 비압축 A8R8G8B8 노멀맵(약 118 배치): mip 회수 도구가 BC 형식만 지원한다. 원본 raw mip 기록을 직접 DDS로 쓰는 소도구를 만들면 가능할 것으로 보이나 이번에는 하지 않았다.
- ShadowMap2D·RNM 조명: Crunch 압축 해독기가 없다.
- 이미터 105개: Effect 문서가 있는 template은 `par_a_h_waterwave_001`, `par_d_fallmist_w3_002` 두 종(7개)뿐이고, `Level_Development`에는 지도 이펙트 런타임 연결(`CMapEffectPresentationRuntime`, Bern·Character Select만 호출)도 없다. 나머지 14종은 Effect 복원이 선행되어야 한다. 분수도 여기에 속한다.
- 환경 볼륨(brush 해독 못 함), 워터팡 아레나 존(57011)의 프롭·NPC·트리거: 별도 작업.
- 랜드스케이프(하단 오른쪽 흰 판의 노란 얼룩)는 원인을 조사하지 못했다.

## 10. 사용자 결정 (반영 시점 기준)

1. **바다 근사 반영 여부 → 반영함.** 사용자가 이미 확정(바다 근사를 반영하되 근사임을 명시하고 원복 방법을 남기라는 지시). 6·7절대로 반영·검증 완료.
2. **어댑터 저장소 설치 여부 → 설치함.** 사용자가 이미 확정. `Tools/LevelPlacementExtractor/build_source_map_material_inputs.py`, `author_ocean_water_rows.py` 설치, README 추가 완료.
3. **Drive 전달 여부 → 하지 않음(사용자 확정).** 대신 전달 목록 파일만 만들어 저장소 루트에 두었다(6절 4번). 파일 자체 전달은 다음 단계에서 사용자가 정한다.
4. **아직 남은 결정 — 팀장님께 물을 것 (이 fork는 결정할 수 없음):**
   - `ocean_trn` 전용 native 프로그램을 추가해 줄 수 있는지(8절). 그 전까지는 프로그램 41 근사가 최선이다.
   - 마하라카 환경 프로필(태양·하늘·안개·후처리)을 다시 넣어도 되는지, 아니면 팀장님이 별도로 관리할지.
   - 프로그램 41(베른 물 마스터)을 마하라카 바다(ocean_trn 원본)의 근사로 재사용하는 것이 재질 데이터 규칙에 맞는지. 7절에서 실제 C++ 계약은 만족함을 확인했지만, "다른 원본 마스터의 프로그램을 근사로 쓰는 것"이 팀 관례에 맞는지는 코드로 판단할 수 없다.

## 10-1. 작업 중 있었던 위험한 순간 (숨기지 않고 기록, 2건)

**1차 (이전 fork, 조사 단계, 게이트 전):** 첫 scratch 루트를 `out/MaharakaWater20260925/scratch`에 만들었고 그 안에 라이브 Resources를 가리키는 junction이 있었다. 경로가 너무 길어 게시가 실패해 짧은 경로로 옮기면서, 정리하려고 `rm -rf out/MaharakaWater20260925/scratch`를 실행했다. 이 명령이 junction 안쪽 Resources를 지웠을 수 있어, 바로 뒤에 라이브 Resources를 셌다: `Map/LV_OCN_EVENTIS_MHP` 3,254개, `_FOLIAGE` 51개, `_LAND` 106개, `LV_BER_BERNCASTLE` 6,088개, `LV_LUT_MIDNIGHTC_ED` 8,182개로 이전 RESULT가 적은 설치 receipt 수(3,253 / 50 + receipt 각 1)와 맞았고 `Map` 아래 폴더 29개가 그대로였다. 파일 내용의 해시 대조까지는 하지 않았다. 이후 make_scratch.py는 junction을 `os.rmdir`로 먼저 끊고 라이브 폴더 존재를 assert하도록 고쳤다.

**2차 (이 fork, 정리 단계, 2026-09-25 21:57):** 격리 시험용 `C:\LostArkExtract\mhw\Client\Bin\Resources`에도 라이브 Resources를 가리키는 junction이 남아 있었다. 이번에는 `cmd /c rmdir`을 먼저 시도했으나 이 세션 환경에서 `/c` 인자가 경로로 잘못 해석되어 차단됐다(`Remove-Item on system path '/c' is blocked`, 실제 삭제는 발생하지 않음). 재귀 삭제를 쓰지 않고 `[System.IO.Directory]::Delete($path, $false)`(비재귀, junction 링크 자체만 제거)로 안전하게 끊었다. 제거 전후 라이브 `Client/Bin/Resources` 파일 수를 직접 세어 `MHP=3254 FOLIAGE=51 Lighting=8938`으로 완전히 동일함을 확인했고, `Get-ChildItem -Force -Recurse`로 `C:\LostArkExtract\mhw` 안에 남은 reparse point가 없음을 확인한 뒤에만 나머지(5.47MB, Data/Client 트리만, Resources 없음)를 재귀 삭제했다.

## 11. 렌더링 규칙 준수 확인 (최종)

`git diff cd58d12b -- Engine/Private/Renderer.cpp Engine/Public/Renderer.h Client/Bin/ShaderFiles Data/Rendering Client/Bin/DataFiles/Rendering` 출력이 비어 있다(2026-09-25 21:58, 반영·게시·C++ 패치를 모두 끝낸 뒤 재확인). 병합 커밋 `cd58d12b` 이후 내가 건드린 추적 파일은 정확히 7개다: `Data/Maps/MapCatalog.json`, `Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapwater.json`, `Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.mapassets`, 같은 폴더의 `.mapwater.json`, `Client/Private/Level_Development.cpp`, `Tools/LevelPlacementExtractor/README.md`. 새로 만든 추적 대상 파일(아직 `git add` 전, untracked)은 5개: `Data/Maps/Authoring/LV_OCN_EVENTIS_MHP/LV_OCN_EVENTIS_MHP.mapmaterials.json`, `Client/Bin/DataFiles/Map/LV_OCN_EVENTIS_MHP.mapmaterials.json`, `Tools/LevelPlacementExtractor/build_source_map_material_inputs.py`, `Tools/LevelPlacementExtractor/author_ocean_water_rows.py`, 이 문서. 이 목록 밖의 추적 파일 변경(`.md/GB/09-25/2026-09-25_MERGE_RESULT.md`, `Data/Balance/Profiles/Retail.balanceprofile.json`, `Server/Bin/DataFiles/World/KAKULSAYDON_ARENA.spawngroupsbootstrap`, `Server/Private/ServerGameplayContractTests_WorldPlayback.cpp`, `Tools/CompositionPipeline/*`, `Tools/GameplayPipeline/build_retail_balance_profile.py`, Kouku 관련 파일들)는 다른 두 fork(병합 마무리, 실패 원인 수정)가 동시에 만든 것이고 이 fork는 손대지 않았다.
팀장 리소스와 기존 마하라카 Resources 파일은 수정·삭제하지 않았다(정리 전후 파일 수 3,254 / 51 / 8,938 동일 확인, 11절 위 문단).

## 12. 사용자가 VS 빌드 뒤 확인하는 순서

1. Visual Studio에서 `Framework.sln`을 열고 Product 빌드(Engine → Shared → Server → Client)를 실행한다. 이 fork는 컴파일을 하지 않았으므로 이번이 이 패치의 첫 실제 컴파일·링크다. `cl /Zs` 구문 검사만 통과한 상태이며 링크 오류 가능성을 배제하지 않는다.
2. Server, Client를 재시작한다(데이터는 이미 게시되어 있어 재게시는 필요 없다).
3. Lobby → `Maharaka`로 들어간다.
4. `Client/Default/EffectFailure.user.log`(가장 최근 프로세스 로그)에서 `map.water.loaded`로 시작하는 줄을 찾는다. 예상 값은 `area=LV_OCN_EVENTIS_MHP assets=406 waterAssets=10 waterRows=10 materialAssets=248`이다(재질이 붙은 asset 248종, 못 붙은 158종은 9절의 이유로 회색·기존 상태 유지). 이 줄이 없고 `map.area.load-failed`만 있으면 그 옆 상태 문구를 읽는다.
5. 화면에서 물 색과 재질(풀장·바다·아레나 바닥)이 이전과 어떻게 달라졌는지 직접 판정한다. 이 fork는 화면을 본 적이 없다.
6. 바다 근사(프로그램 41)가 이상하게 보이면(색이 과포화이거나 검게 보이는 등) `python out/MaharakaWater20260925/tools/apply_live.py --restore`로 재질·물 문서만 되돌리고 `Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP`를 다시 실행하면 이전 상태(재질 문서 없음, 옛 물 경로)로 복귀한다. `Data/Maps/MapCatalog.json`도 함께 되돌아간다.

## 13. 전달 목록 위치

- `Resource_Distribution_2026-09-25_MaharakaMaterials.txt` / `Copy_ResourceDistribution_2026-09-25_MaharakaMaterials.ps1`(저장소 루트, 새 파일 3개·444 bytes, 엔진 기본 텍스처 DDS). 이 3개 파일이 없는 PC는 재질 문서가 텍스처 누락으로 로드에 실패한다.
- 기존 마하라카 리소스(3,306개·약 914 MB)는 이전 `Resource_Distribution_2026-09-25_MaharakaRestore.txt` 목록을 그대로 쓴다(이번에 추가 변경 없음).
- 두 목록 모두 아직 Drive로 보내지 않았다(10절 3번).

## 14. 최종 재확인

**확인한 것(코드·파일·명령 결과로):**
- 반영(`apply_live.py`)과 게시(Validate/Publish/Check) 전부 exit 0, 무관 파일(`mapplacements`, `maplights.json`) 바이트 불변.
- 재질 문서 326행 전체가 실제 C++ 계약(`Parse_MaterialOverrides`)을 만족함을 두 경로로 확인: (a) 기존 부분 검사기(319 deferred 행, 0 오류), (b) 이번에 native water family 계약을 직접 코드에서 읽어 만든 재검사기로 7행 전부(0 오류).
- C++ 패치가 앵커 1회 일치로만 적용됐고 CRLF·ASCII가 그대로이며, 프로젝트의 실제 `/std:c++20`으로 구문 검사(EXITCODE=0)를 통과함.
- 렌더링 보호 파일(Renderer, ShaderFiles, Data/Rendering, DataFiles/Rendering) 무변경(diff 빈 값)을 반영 전/후 두 번 확인.
- 라이브 Resources가 이번 작업 전체 동안(1차 위험 순간 포함) 훼손되지 않았음을 파일 수 대조로 확인(정확한 hash 대조는 하지 않음, 미확인 항목에 있음).
- 새로 만든 도구(`build_source_map_material_inputs.py`, `author_ocean_water_rows.py`)가 python 구문 오류 없이 설치됨.

**확인하지 못한 것:**
- 실제 Debug Product 빌드와 링크(사용자가 VS로 진행 예정).
- Client 실행 화면 어떤 것도 — 물이 실제로 보이는지, 재질 색이 맞는지, 바다 근사가 자연스러운지는 전부 미확인.
- 라이브 Resources 파일의 내용 해시 대조(파일 개수만 확인).
- 재질이 붙지 않은 자산 158종(식생 wind, translucent, 비압축 노멀맵)과 이미터·분수·데칼·환경 볼륨·랜드스케이프 얼룩은 이번에 손대지 않았고 9절 그대로 남아 있다.
- 프로그램 41 근사가 팀장님의 재질 데이터 규칙에 맞는지에 대한 팀장님 본인의 판단.

MAHARAKA_APPLY_DONE: 데이터 반영, 마하라카 Area 게시(Validate/Publish/Check 전부 성공), C++ 진단 로그 패치와 구문 검사, 도구 설치, 전달 목록 생성, 임시 파일 정리, 렌더링 규칙 준수 재확인까지 전부 끝났다. 빌드는 의도적으로 하지 않았다(사용자가 VS로 직접 진행). 재질이 붙지 않은 자산과 이미터·데칼·환경은 9절에 적은 대로 이번 범위 밖이다.
