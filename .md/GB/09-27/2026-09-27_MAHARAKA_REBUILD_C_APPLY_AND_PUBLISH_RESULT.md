# 2026-09-27 마하라카 C단계 — 변종 집합 적용과 게시 결과

B단계가 `out/`에만 만들어 둔 RNM 변종 집합을 정본에 적용하고 게시한 기록이다.
적용 전에 보존 계약을 파일로 고정했고, 적용 후 같은 계약으로 다시 검사했다.
A·B단계는 `2026-09-27_MAHARAKA_REBUILD_A_MATERIAL_INPUTS_RESULT.md`,
`2026-09-27_MAHARAKA_REBUILD_B_RNM_VARIANTS_RESULT.md`를 따른다.

## 1. 실제로 바뀐 수치

| 항목 | 이전 | 이후 |
|---|---|---|
| 자산 카탈로그 행 | 408 | 867 |
| 재질 행 | 329 | 947 |
| 그중 `bakedLighting` 보유 | 142 | 931 |
| `placementLighting` 인스턴스 | 231 | 2,822 |
| placement 행 | 4,669 | 4,669 (변경 없음) |
| 변종으로 재지정된 placement | — | 1,496 |
| `renderprofiles.json` 행 | 307 | 317 |

게시 헤더는 `LOSTARK_MAP_ASSET_CATALOG 5 "LV_OCN_EVENTIS_MHP" 408` →
`... 867`로 바뀌었고 `LOSTARK_MAP_PLACEMENTS 2 "LV_OCN_EVENTIS_MHP" 4669`는 그대로다.

## 2. 보존 계약과 검사 결과

적용 전에 `out/MaharakaRebuild_C/preservation-manifest.json`으로 다음을 고정했다.
재질 행 329개의 행별 sha16, `bakedLighting` 보유 키 142개, 물 family 키 10개,
`placementLighting` 키 231개, `mapwater` assetId 10개, 게시 헤더 2개.

적용 후 같은 매니페스트로 검사한 결과다.

| 검사 | 결과 |
|---|---|
| 보존 대상 재질 키 329개 중 사라진 것 | 0 |
| 물 10행 값이 sha16까지 동일 | 그대로 |
| 기존 `bakedLighting` 142행 유지 (필드 소실·값 변경) | 0 / 0 |
| 기존 `placementLighting` 231키 중 누락 | 0 |
| 기존 카탈로그 408행이 새 867행 안에서 바이트 동일 | 408/408 |
| placement ID 집합 동일 | 동일 |
| 변경된 placement 1,496행 중 assetId 외 필드가 바뀐 것 | 0 |
| 물 자산에 생긴 변종 | 0 |
| `placementLighting.assetId` != 해당 placement의 assetId | 0 |
| 게시본이 참조하는 DDS 527개 중 디스크에 없는 것 | 0 |
| 게시 카탈로그 wmodel 867개 중 없는 것 | 0 |

`Publish-MapAuthoring.ps1 -AreaId LV_OCN_EVENTIS_MHP`를 `Validate` → `Publish` → `Check`
순서로 실행해 셋 다 통과했다. 게시 placement sha256은
`2789e2f3ce2bc0880bd0bfbde2bf245594cd1d2e17e73da016844949c793f9ac`다.

적용 전 `out/MaharakaRebuild_C/backup/`에 Authoring 4개, Imported 5개, DataFiles 6개와
`backup-sha256.txt`를 남겼고, 적용 직전에 각 대상이 백업 시점과 sha256이 같은지 확인한 뒤
임시 파일 + `os.replace`로 교체했다.

## 3. B가 스스로 제외한 구멍을 줄인 것

B는 "재질 행이 없어서 조명을 붙일 자리가 없는 자산 107개, 인스턴스 1,008개"를 제외했다.
그 사유를 전수 분류하다가 하나가 실제 도구 버그임을 확인했다.

`source_map_surface.py`와 `source_map_surface_extra.py`가 flicker가 켜진 재질에서
`v['emissive_intensitymin']`을 strict 조회한다. 이 파라미터는 base material
`bg.base.bg_base_opa`의 scalar parameter인데 `DefaultValue` property가 직렬화에서 생략돼 있다.
추출기는 이를 `DEFAULT_PROPERTY_ABSENT`로 기록하며, 같은 base의 형제 스칼라
`uv_rotate`, `uv_move_x`, `uv_move_y`는 이미 `v.get(name, 0)`으로 읽고 있었다.
즉 값이 없는 것이 아니라 클래스 기본값 0이 적용되는 경우인데 한 줄만 strict였고,
그 KeyError가 재질 전체를 버렸다. 추출 파라미터에서 이 이름은 248개 재질이 absent,
3개 재질만 MIC override(0.3 / 2.0 / 1.0)를 갖는다.

같은 규칙으로 `.get(..., 0)`으로 고치고 근거를 주석으로 남겼다. 회귀 테스트 2개를 추가했다
(`test_absent_flicker_minimum_default_and_override`,
`test_opaque_flicker_minimum_uses_absent_default`). `test_build_source_map_materials.py`는
13 → 15 OK다.

그 결과 네온사인·간판·장식 소품 계열 자산 10개가 살아났다.

| 단계 | 슬롯 | 재질 행 | baked 행 | 변종 | placementLighting | 제외 자산 / 인스턴스 |
|---|---|---|---|---|---|---|
| B | 355 | 908 | 902 | 446 | 2,743 | 107 / 1,008 |
| C | 367 | 937 | 931 | 459 | 2,822 | 97 / 929 |

어댑터를 다시 돌린 결과는 B의 출력을 완전히 포함한다. 사라진 슬롯 0개, 값이 바뀐 기존 슬롯
0개, 텍스처 375 → 383(값 변경 0), `lightingEvidenceSummary` 동일. B가 쓴 `--mip-results`
파일이 디스크에 없어서 같은 근거인 `chains_b1/*.receipt.json` 565개에서
(`sourceObject`, `SOURCE_MIP_CHAIN_VALIDATED` → `recovered`)로 재구성했고, 그 재구성이
맞았다는 증거가 위의 "기존 슬롯 값 변경 0"이다.

새로 필요해진 텍스처 8개 중 7개는 이미 설치돼 있었고, 1개
(`Map/LV_OCN_EVENTIS_MHP_SOURCE_MATERIALS/EngineDefaults/726b99cf1d98_flat_white.dds`, 148 bytes)만
sha256을 확인한 뒤 **단일 파일 추가**로 설치했다. 폴더 전체 교체(`install_runtime_area`)는
쓰지 않았다. 그 경로는 09-27에 실제로 382개 디렉터리를 지운 이력이 있다.

## 4. 물 자산 renderMode를 source에 기록한 이유

물 자산 10개는 카탈로그에 `Water` 토큰을 갖고 있지만 `renderprofiles.json`에는 행이 없었다.
`build_maptool_scene.py`의 `render_profile_text()`는 `renderMode` 기본값이 `Opaque`이므로,
카탈로그를 다시 생성하면 그 10행이 조용히 `Opaque`로 강등된다.

10개 전부 `UE3 ImportTable exact` 근거를 가진 생성기 범위 내 자산임을 확인했고,
`{"assetId": ..., "renderMode": "Water"}` 한 필드만으로 현재 카탈로그 꼬리를 정확히
재현함을 대조한 뒤 추가했다. 기존 307행의 순서와 값은 그대로이고, 원본의 CRLF와
"말미 개행 없음"까지 유지해 앞 39,177바이트(99.9%)가 바이트 동일하다.

## 5. 남은 구멍 — 자산 97개 / 인스턴스 929개

조명을 붙일 재질 행이 없는 자산이다. 사유별 슬롯 수는 다음과 같다.

| 사유 | 슬롯 | 해소에 필요한 것 |
|---|---|---|
| `foliage vertex wind needs a verified binding` | 42 | 원본 vertex wind 바인딩을 실측·검증해야 한다 |
| `unsupported terminal bg_base_trn_depthtest` | 17 | 반투명 terminal을 publisher family까지 새로 연결 |
| `unsupported terminal ocean_trn` | 12 | 물 terminal (현재 물은 별도 family로 처리 중) |
| `unsupported terminal bg_base_trn` | 11 | 반투명 terminal |
| `null material slot` | 23 | 원본에 재질이 없는 슬롯. 해소 대상이 아니다 |
| `unhandled static switches use_wind / use_linear_blend / use_opacity_texture` | 11 | 각 스위치 분기를 원본 근거로 구현 |
| `unsupported terminal molding_trn / monster_base_opa / sky_opa / preset_flag_vertical_msk` | 6 | terminal별 개별 구현 |
| `invalid emissive bounds` | 1 | 값 범위 확인 |

전부 새 재질 terminal이나 vertex 프로그램을 구현하는 일이며, 값을 날조해 행을 만들면
조명 하나 얻는 대신 재질 전체가 틀어진다. 그래서 채우지 않고 사유를 남긴다.
`null material slot` 23개는 원본 그대로이므로 애초에 채울 대상이 아니다.

`b4_lightmap_batch.json`이 복구한 라이트맵 108개 중 92개가 실제로 참조되고 16개(8쌍)는
위 자산들이 제외돼서 쓰이지 않는다. 파일은 이미 설치돼 있으므로 위 terminal이 구현되면
추가 추출 없이 붙는다.

`ShadowMap2D`는 여전히 미해독이다(`pf_g8` uncompressed). 참조 목록만 기록하고 payload는
읽지 않으며, RNM 라이트맵은 그 참조 뒤에 직렬화되므로 파싱은 계속된다.

## 6. 빌드와 Drive 전달

`Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`를 마지막에 한 번 실행했다.
Engine / Shared / Server / Client 네 프로젝트 PASS, `missingRuntimeInputs`와
`invalidRuntimeInputs` 없음, Items·Valtan ClearRewards·Navigation 검사 전부 PASS다.
이번 구간은 C++/HLSL을 바꾸지 않았으므로 첫 실행의 출력은 0이었다.

그 뒤 `git diff --check` 경고를 없애려고 `MainApp.cpp` 한글 주석 5줄의 끝 공백을 지웠다가
빌드를 깼다(`C2065`·`C2737` 16건). 그 공백은 장식이 아니라 UTF-8 한글의 남는 바이트가
개행과 짝을 이뤄 개행을 삼키는 것을 막는 장치다. 5칸을 복원해 파일 크기를 원래대로
되돌리고 다시 빌드해 PASS했다(OBJ=1, binaries=1). 이 함정은 `.md/GB/gotchas.md`에 남겼다.
**그 5건 trailing whitespace 경고는 의도된 것이며 지우면 안 된다.**

Drive 전달 대상은 **1,869개 파일 / 935.3 MB**다.
`out/MaharakaDriveHandoff/resources-drive-manifest.json`과 같은 폴더의 목록 txt가 정본이며
`C:\Users\USER\OneDrive\바탕 화면\CY_Resources`에 Resources 상대 경로 그대로 복사했다.

| 그룹 | 개수 | 크기 | 내용 |
|---|---|---|---|
| `Map/LV_OCN_EVENTIS_MHP` | 1,592 | 844.4 MB | Area 모델과 그 모델이 들고 있는 텍스처 |
| `Character/NPC/Maharaka` | 30 | 32.7 MB | 모코모코·워터캐논 NPC |
| `Map/LV_OCN_EVENTIS_MHP_LAND` | 74 | 30.2 MB | 지형 모델·텍스처 |
| `Map/LV_OCN_EVENTIS_MHP_FOLIAGE` | 21 | 11.5 MB | 식생 |
| `Map/Lighting/Maharaka` | 136 | 8.7 MB | RNM 라이트맵 DDS와 LUT |
| `Sound/Maharaka` | 12 | 7.7 MB | 모코모코 물벼락 사운드 |
| `Map/LV_OCN_EVENTIS_MHP_SOURCE_MATERIALS` | 4 | 0.0 MB | source material 공용 입력·엔진 기본값 |

처음에는 이 목록을 "2026-09-26 00:00 이후 수정분" 시각 델타로 만들어 246개 / 89.1 MB라고 적었다.
**그 범위는 틀렸다.** 두 가지를 놓쳤다.

1. 그 전에 설치된 Area 자산을 전부 빼먹었다. 받는 PC에 이미 있다고 가정할 근거가 없다.
2. `.wmodel`을 ASCII로 훑어 내장 `.dds` 문자열이 0건이라고 판단했다. 실제로는
   `MODEL_MATERIAL_DATA`의 경로 필드가 `std::filesystem::path`(=`wchar_t`)라서 **UTF-16LE**로
   저장되고, 값도 Resources 루트가 아니라 **모델 폴더 상대**(`textures/<hash>_<name>.dds`)다.
   와이드 패턴으로 다시 읽으니 모델 410개에서 경로 1,285건이 나왔고, 텍스처 727개와 `.png` 60개가
   목록에서 빠져 있었다. 그대로 배포했으면 회색 메시로 떴을 것이다.

그래서 범위를 **참조 closure**로 바꿨다. 게시된 맵 문서(catalog / mapmaterials / mapwater /
maplights / mapmotions / placements) + 마하라카 NPC·사운드 카탈로그 + 각 모델 내장 텍스처 경로다.
참조 경로 1,805개 중 1,804개가 실제 파일이고(남은 1개는 MapCatalog의 Area 폴더 경로 문자열),
전부 staged이며 미해석 모델 텍스처는 0건이다.

검증은 staged 1,869개 전부에 대해 원본 존재와 크기 일치(불일치 0), 무작위 표본 120개와 8 MB 초과
파일 전부의 sha256 일치(불일치 0)다. 원본 `Client/Bin/Resources`(71,040 파일)는 무변경이다.

`converter.log.txt`, `converter.info.txt`, `.gltf`, `.bin`, 변환 영수증 `.json`은 제외했다.
런타임은 `.wmodel`과 `.dds`/`.tga`/`.png`만 열고, 이 중간물들은 애초에 runtime Resources에 있을
물건이 아니다(변환기가 남긴 것). 제외분은 2,442개 / 40.7 MB다.

## 7. 아직 사용자 확인이 남은 것

화면 판정은 하지 않았다. 게시된 데이터가 구조·참조·보존 검사를 통과했다는 것까지가
이 문서의 범위다. 실제 마하라카 진입 후 조명이 붙은 모습, 네온사인 flicker,
물 표면과 무대 색은 사용자가 직접 확인한다.
