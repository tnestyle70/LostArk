# 2026-08-29 쿠크세이튼 리소스 추출 및 물리 반영 결과

## 1. 결론

`C:/Users/user/Desktop/LostArk` 원본 checkout에서 다섯 잔여 항목을 다시 실측하고, 추출 가능한 물리 dependency closure와 canonical 경로 정리를 완료했다. 원본에 없는 animation이나 현재 `CMaterial`이 표현하지 않는 material lane은 다른 이름 또는 diffuse/emissive로 위조하지 않았다.

| 항목 | 최종 상태 | 근거 |
|---|---|---|
| `MN_RPCZ_00` Action clip 27개 | 원본 추출 누락이 아님을 확인하고 닫음 | package export/import와 canonical PSA 12개 전수 대조 결과 compatible source clip 0개다. 27개는 `.loa`의 generic STAND 13, MOVE 8, DIE 6 조건 분기다. |
| `WP_MN_RPCT_07/08` cooked animation | canonical zero-animation으로 닫고 mesh/material WModel 4개 설치 | 두 package 모두 `AnimSet/AnimSequence` export/import가 0개다. exact skeleton identity를 유지한 0-animation WModel만 cook했다. |
| 맵 texture 37개 asset, 38개 slot | 물리 dependency closure 완료, runtime 표현 경계 분리 | exact texture dependency는 292/292, physical missing 0이다. 남은 38 slot은 authored null 23, UModel empty `Material3` 7, unsupported procedural spotlight 8이다. |
| 물리 Map root와 MapCatalog 불일치 | 완료 | 물리 root를 `Map/LV_LUT_MIDNIGHTC_ED`로 무손실 이동했다. MapCatalog의 기존 canonical root와 일치한다. |
| catalog 경로가 Sound만 포함 | 완료 | canonical map root에 map payload 2,555개가 있고 `Sound` 하위 폴더는 없다. Sound는 별도 `Sound/KakulSaydon`에 유지했다. |

이 결과는 extraction/authoring/Development geometry preview 범위를 닫는다. Product Level admission, navigation/gameplay/server/client binding, 제품 audio binding과 완전한 material shader fidelity는 이번 요청처럼 새 runtime 코드를 연결하지 않은 상태로 남긴다.

## 2. 정본 identity와 경로

- Area/catalog ID: `LV_LUT_MIDNIGHTC_ED`
- map runtime asset root: `Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED`
- runtime asset ID prefix: `Map/LV_LUT_MIDNIGHTC_ED/...`
- map source catalog: `Data/Maps/Imported/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.mapassets`
- placement authoring: `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.mapplacements`
- published Development data: `Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.mapassets`, `LV_LUT_MIDNIGHTC_ED.mapplacements`
- Sound physical root: `Client/Bin/Resources/Sound/KakulSaydon`
- Character physical root: `Client/Bin/Resources/Character/KakulSaydon/<canonical-package>`
- Effect physical root: `Client/Bin/Resources/Effect/KakulSaydon`

`KakulSaydon`은 사람이 읽는 collection alias로만 유지한다. 맵의 추출 정본 이름은 `LV_LUT_MIDNIGHTC_ED`이므로 `Map/KakulSaydon` alias는 exact-empty 상태에서 제거했고 다시 만들지 않았다. `Map/LV_LUT_MIDNIGHTC_ED/Sound`도 만들지 않았다.

MapCatalog는 생성 가능한 asset 정의와 published placement 문서의 경로를 가리킨다. `Data/Maps/Authoring`은 MapTool이 저장하는 transform instance 정본이고, publisher만 이를 `Client/Bin/DataFiles/Map`으로 검증·복사한다. 기존 MapCatalog 행과 MapTool의 `KakulSaydon / MidnightC ED` Development 선택기는 이미 존재했으므로 이번 작업에서 새 runtime 또는 selector 코드를 추가하지 않았다.

## 3. Animation closure

### 3.1 `MN_RPCZ_00`

- source package: 6,009,358 bytes, SHA-256 `fb87460ad33011c96ed24d13363e9341c31a3b47d2f83ca5d09975bb635be627`
- source export: `AnimSet=4`, `AnimSequence=119`; external animation import 0
- body PSA: main 86 + evt2 5 = unique 91
- additional 23개 sequence set: 23개 모두 main body identity와 중복
- rope PSA: 별도 22-bone rig 5개
- 기존 WModel: 28,541,364 bytes, SHA-256 `8d5a66797b7300f5a69038ab9c218ffc1c95aeed659b2ee96b193d762083d63e`, animation 91, skeleton 있음
- Action unique name 97개 중 WModel과 exact intersection 70개, generic condition name 27개
- canonical PSA 12개에서 26개 이름은 존재하지 않음
- 유일한 이름 후보 `Idle_Normal_1_1`은 `MN_RPCT_01`의 60-bone rig에만 있고 `MN_RPCZ_00`은 100 bones, 공유 bone 38개라 호환되지 않음

따라서 compatible source clip은 0개다. 기존 91-animation WModel은 변경하지 않았고, 27개를 이름 복제·bind pose·잘못된 donor retarget으로 만들지 않았다. 이 조건 분기를 실제 재생하려면 추출 문제가 아니라 별도 runtime fallback 정책이 필요하다.

### 3.2 `WP_MN_RPCT_07/08`

두 source package 모두 authored animation export/import가 0개이므로 0개가 canonical이다. 기존 ActorX importer → Blender FBX(`bake_anim=false`) → ModelAssetConverter 경로로 mesh, material, texture와 exact one-bone skeleton을 cook했다.

| WModel | Bone identity | Animation | Bytes | SHA-256 |
|---|---|---:|---:|---|
| `WP_MN_RPCT_07/wp_mn_rpct_07l_sk.wmodel` | `bone001` | 0 | 1,762,132 | `52588d7ac54c814fa9a1170f3ec3d9fb6f6647dda6160dac76c46d12ec3fde2d` |
| `WP_MN_RPCT_07/wp_mn_rpct_07r_sk.wmodel` | `bone001` | 0 | 1,763,108 | `7198ed739ac374c9ec81e700a0df4edd1b6ca8e0efde8d9ffc46c94efc1df7b3` |
| `WP_MN_RPCT_08/wp_mn_rpct_08_sk.wmodel` | `wp_mn_rpct_08_sk` | 0 | 210,996 | `7758df5ad5baa6a5fc9917241a0ccc958b72ca4793e47dc94c5a7519b2a311d8` |
| `WP_MN_RPCT_08/wp_mn_rpct_08_1_sk.wmodel` | `b_root` | 0 | 210,996 | `d19e27d7ad72474484531df4672acfdb07793e4ca0859dcbd3466b2a2c2f1b87` |

- `WP_MN_RPCT_07`: 5 files, 5,360,632 bytes
- `WP_MN_RPCT_08`: 6 files, 7,500,392 bytes
- canonical character package closure 8개 root: 63 files, 519,532,776 bytes, aggregate SHA-256 `b4cfaf6fe785a04aed2f9c8708c91fb46e074470e1038e7eaf56558c44a247d9`
- external reproducible staging: `C:/LostArkExtract/LV_LUT_MIDNIGHTC_ED_20260829/AnimationClosureAudit-20260830/ZeroAnimationCook`

`Character/KakulSaydon` 물리 폴더 전체는 사용자 소유 `WP_WGDH_02S`를 포함해 67 files, 520,669,903 bytes다. 위 canonical 63-file closure 계산에는 이 별도 사용자 payload를 넣지 않았고 삭제하거나 변경하지 않았다.

## 4. Map material과 배치 closure

기존 pipeline을 확장해 UE3 placement schema v2에 ordered `materialOverrides`, authored null slot과 signature를 보존하고, scene compile이 object path와 material signature 조합으로 올바른 WModel variant를 고르게 했다. material variant 단계는 exact named texture parameter와 UModel `ReferencedTextures`를 source identity 그대로 보존한다.

- catalog assets: 292, physical model missing 0
- placements: 2,951, missing asset reference 0
- exact texture dependency closure: complete 292, incomplete 0, physical missing texture 0
- dependency rows: total 2,578 = named parameter 2,244 + graph referenced 334
- physical 보존은 되었지만 현재 runtime에 bind되지 않는 rows: 1,313
- runtime-visible color: complete 255 assets, incomplete 37 assets/38 slots
- 38 slots: authored null 23 + UModel empty `Material3` 7 + procedural spotlight 8
- material fidelity: complete 0, incomplete 292. 이는 scalar/vector/render/aux texture semantic 전부를 현재 `CMaterial/WMaterial`이 표현하지 못한다는 별도 Product capability 상태다.

spotlight의 exact source texture `FX_TEX_02.fx_d_cloud_033`도 물리 closure에 포함했다. 다만 지원되지 않는 named lane을 false diffuse/emissive로 remap하지 않았다.

최종 canonical map payload:

- 2,555 files, 378,669,844 bytes
- owned payload 2,554 files + install receipt 1개
- WModel 292, DDS 1,968, TGA 294
- 신규 exact dependency 1,140 files = DDS 876 + TGA 264
- 기존 path 삭제 0, 기존 파일 SHA 변경 0
- runtime manifest SHA-256 `9f6b439d5025c693c1b0b3b370003ba0fab35da6ce808ef32dca412c7696f116`
- install receipt SHA-256 `60e9c37cf2dbe50db1db3831077af91b36571c0a328f6d2353dc23907b687391`
- physical aggregate SHA-256 `3be0ae99afc8f75cd3d3d06e67e8c249de728c8c7d132a1efea30eb74c2ad51b`
- admission state `geometry-preview-partial-material`

경로 이동 전 1,415 files, 343,242,379 bytes와 이동 직후 파일 set의 relative path/size/SHA 차이는 0이었다. 그 뒤 위 1,140개 exact texture dependency만 추가했으며 기존 파일을 덮어쓰거나 삭제하지 않았다.

## 5. 함께 보존한 물리 리소스

| Root | Files | Bytes | 이번 마감에서의 상태 |
|---|---:|---:|---|
| `Map/LV_LUT_MIDNIGHTC_ED` | 2,555 | 378,669,844 | canonical relocation과 dependency 보강 완료 |
| `Sound/KakulSaydon` | 1,088 WAV | 1,056,169,208 | 별도 canonical sound root로 보존; map 아래 복사본 없음 |
| `Character/KakulSaydon` | 67 | 520,669,903 | canonical 63-file closure + 사용자 별도 payload 4개 보존 |
| `Effect/KakulSaydon` | 697 | 100,169,184 | 사용자 소유 closure 그대로 보존; Effect V2 변경 없음 |
| `UI/KakulSaydon` | 0 | 0 | 이번 다섯 항목에서 runtime UI payload를 만들거나 연결하지 않음 |

Sound의 direct playable closure는 1,088 WAV, unique media 1,087, 497 event identity이며 aggregate SHA-256은 `fae7e4515a2981860b0c99cf4a9b4e4c587dfa9fd509d1d539d986b6fcec3320`이다. Product audio binding은 추가하지 않았다.

## 6. 변경한 정본과 pipeline

- `Data/ResourceIntake/LV_LUT_MIDNIGHTC_ED.resource-intake.json`: canonical paths, animation source verdict, map material coverage, physical receipt와 Sound closure 갱신
- `Data/Maps/Imported/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.build.receipt.json`: 새 runtime manifest SHA 반영
- `Tools/LevelPlacementExtractor/extract_ue3_placements.py`: ordered material override/null/signature 추출
- `Tools/LevelPlacementExtractor/build_maptool_scene.py`: object path + material signature variant join과 forged signature 거부
- `Tools/LevelPlacementExtractor/build_map_material_variants.py`: exact source dependency와 runtime material variant 생성
- 위 pipeline의 focused test 3개

MapCatalog, MapTool selector, Client/Server runtime, project/filter, Effect V2는 변경하지 않았다.

## 7. 자동 검증

- 관련 JSON 5종 parse: PASS
- LevelPlacementExtractor focused Python tests: 38/38 PASS
- Kakul admission tests: 7/7 PASS
- model retime tests: 7/7 PASS
- WModel cook geometry tests: 6/6 PASS
- focused Python tests 합계: 58/58 PASS
- `Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Mode Validate`: PASS, 292 assets / 2,951 placements
- 같은 publisher `-Mode Check`: PASS
- Development geometry admission: PASS, 292 assets / 2,951 placements
- WModelGeometryContractHarness Debug: exit 0
- Client Debug x64 `ClCompile`, project references 제외: exit 0. 다른 세션의 dirty C++를 포함한 합산 tree 결과이며 이번 리소스 변경 단독 runtime 증거로 사용하지 않는다.
- map owned file identity: 2,554/2,554 PASS, SHA mismatch 0
- catalog physical model resolution: 292/292 PASS
- placement asset resolution: 2,951/2,951 PASS
- four new weapon WModel path/size/SHA와 skeleton identity: PASS
- old `Map/KakulSaydon`: 없음
- `Map/LV_LUT_MIDNIGHTC_ED/Sound`: 없음
- Kakul 관련 Map/Sound/Character/Effect/UI Resource scope: Git tracked 0, staged 0
- `git diff --check`: PASS

Product admission은 의도적으로 false다. navigation/gameplay/server/client/audio binding 등 42개 Product 요건은 extraction-only 범위에서 만들지 않았다.

## 8. 수동 검증과 남은 경계

- Client/UI 실행·조작: `NOT RUN`
- MapTool Development geometry preview 육안 판정: `MANUAL_NOT_RUN`
- map material first-pixel fidelity 판정: `MANUAL_NOT_RUN`
- Sound 재생/청각 판정: `MANUAL_NOT_RUN`
- Animation Tool clip 표시와 한글 display-name 번역 판정: `MANUAL_NOT_RUN`

자동 검증은 물리 identity, placement, parse, pipeline과 Development admission까지만 증명한다. 사용자가 직접 MapTool과 향후 통합 Animation Tool에서 시각·청각 결과를 확인해야 한다. `MN_RPCZ_00` 27개 generic 상태 fallback, 292개 material의 완전한 shader semantic, 제품 Sound/UI/Effect binding이 필요해지면 별도 runtime 수직 슬라이스로 진행해야 한다.

## 9. Git 경계

이번 Kakul Resource payload는 Git에 add/commit하지 않았다. 원본 checkout의 다른 세션 미커밋 변경과 사용자 리소스를 모두 보존했으며 reset, checkout, 일괄 정리를 수행하지 않았다. 공유 dirty tree이므로 자동 stage/commit도 하지 않았다.
