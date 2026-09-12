# World Object 원본 재질·환경 반사 전달 구현 결과

## G00. 완료 상태와 실행 경계

World Object가 파생 static/skinned 모델을 사용할 때 명시한 원본 모델의 catalog 재질을 검증 후 전달하는 공통 경로를 연결했다. Showtime 좌·우 총 두 객체와 같은 원인으로 누락된 Bingo·outer_fire D/E/F 네 객체의 원본 재질 연결을 추가했다. 사용자의 마무리 지시에 따라 추가 네 소품의 신규 MODEL shader 확장은 조사 단계에서 중단했다.

Client/UI 실행과 화면 캡처는 하지 않았다. 이 문서의 PASS는 구조·데이터·컴파일 검증이며 화면 복원 승인이 아니다. 이 작업자는 공식 runtime publish와 제품 실행 파일 교체를 수행하지 않았으며 최종 통합 빌드·publish는 같은 작업의 상위 통합 단계가 수행한다.

## G01. 공통 소비 경로

| 파일 | 실제 변경 |
|---|---|
| `Client/Public/ActorCatalog.h`, `Client/Private/ActorCatalog.cpp` | `Build_DerivedModelLoadDescription`이 기존 exact source catalog descriptor를 stage하고 source/texture 실물, 실제 target decoder, material slot 유일성을 검증한다. 실패하면 이전 출력 descriptor를 유지한다. |
| `Client/Private/WorldSequencePlayer_Objects.cpp` | 위 helper를 실제 World Object 준비 경로에서 사용한다. 기존 map binding·inline profile 우선순위를 유지한다. |
| `Client/Public/WorldObjectTool.h`, `Client/Private/WorldObjectTool.cpp` | `Material Source Model` 입력은 별도 buffer에 두고 `Apply Material Source`에서 검증 후 저장한다. 새 모델과 기존 source slot이 불일치하면 모델 교체를 거절한다. |
| `Client/Public/WorldSequenceDocument.h` | 기존 `materialSourceModelAssetId`가 일반 파생 World Object에도 적용됨을 설명한다. 저장 버전·필드는 추가하지 않았다. |
| `Tools/ModelAssetConverter/apply_world_object_material_source.py` | 원본 모델 material source 적용, 파생 actor 기본 상속, exact map asset binding 적용을 공유한다. CLI는 검증 후보를 먼저 만들고 명시 `--apply`에서 authoring baseline을 재확인해 교체한다. runtime 출력은 거부한다. |
| `Tools/KoukuSaydonPipeline/build_source_sequences.py` | 파생 actor의 알려진 원본 catalog source를 기본 전달하고 총 둘·map 넷의 재생성에서도 명시 원본 대응을 유지한다. |
| `Tools/MapPipeline/Publish-MapAuthoring.ps1` | 명시 source model의 catalog override 존재·원본/target slot 일치·texture 입력 존재를 publish 교체 전에 확인한다. |

빈 source는 기존 해당 모델 자체의 catalog/embedded 경로를 유지한다. 정보가 없는 객체를 특정 program 26이나 다른 family로 추정하여 치환하지 않는다. 새 C++ 파일이 없어 project/filter 항목은 추가하지 않았다. 기존 파일 인코딩과 CRLF를 유지했다. 같은 파일에 이미 있던 다른 작업의 변경은 보존했다.

## G02. 데이터 반영

정본은 `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json`이며 revision 673→675다. 변경 전후 전체 JSON 비교에서 다음 여섯 resource의 재질 연결과 revision만 달라졌다.

| World Object | 원본 연결 |
|---|---|
| `world.object.kouku.saydon_showtime_gun_left` | `Character/KoukuSaton/WP_MN_RPCT_07/wp_mn_rpct_07l_sk.wmodel` |
| `world.object.kouku.saydon_showtime_gun_right` | 같은 LEFT 원본. 실제 양손 static mesh가 모두 `wp_mn_rpct_08_mi`이므로 손 이름으로 RIGHT 모델을 선택하지 않는다. |
| `world.object.kouku.bingo` | `MAP_5F1286085DD9_LV_LUT_MIDNIGHTC_FLOOR03_SM`의 기존 map material |
| `world.object.kouku.g3.outer_fire.d` | `MAP_CFEDE8067300_BG_RAD_KOUKUSATON_DECO24D_SM_KHB` |
| `world.object.kouku.g3.outer_fire.e` | `MAP_B71A2EC9D778_BG_RAD_KOUKUSATON_DECO24E_SM_KHB` |
| `world.object.kouku.g3.outer_fire.f` | `MAP_7AC8BB3D2FEE_BG_RAD_KOUKUSATON_DECO24F_SM_KHB` |

map 넷은 모두 기존 `bg-source-opaque-masked` family의 exact model/slot을 재사용한다. 이동 가능한 World Object에 위치별 baked lighting/static shadow를 복제하지 않는 기존 경로를 그대로 사용한다.

총 원본 MIC는 `wp_mn_rpct_07.mat.wp_mn_rpct_08_mi`, family는 `source.character.monster-6ff78ae19259.v1`, MODEL program은 26이다. normal/diffuse/specular, `hdr07_1`, `flat_black`, `statefx_default`, `brdf_beckmann_spec` 일곱 texture asset 모두 존재한다. source descriptor의 base/light constants와 16 texture 슬롯의 asset·색 공간이 파생 descriptor에서 동일함을 실제 C++ probe로 확인했다. 주요 원본 값은 IBL intensity 1, exposure 5, normal smooth 약 0.7, reflect LOD bias 80, metalicness power 약 0.2, roughness power 3, PBR specular intensity 10/power 6이다.

`hdr07_1`은 재질 소유 Texture2D IBL이다. Scene cubemap과 다른 입력이며 모든 material에 장면 반사를 강제로 연결하지 않았다. 기존 reflected static props 추출은 음수 scale geometry bake이고 광학 반사 복원과 별개다. 이 변경은 Resources binary payload를 새로 만들거나 교체하지 않았다.

313개 최종 분포는 map binding 282개, actor catalog 24개, inline profile 1개, placed sequence alias 2개, source override 없는 embedded material 4개다. alias 둘은 독립 모델을 소유하지 않으므로 재질 누락 객체가 아니다.

## G03. 실행한 검증

| 검증 | 결과·증거 |
|---|---|
| Python 문법 | 공통 apply 도구와 source sequence generator `py_compile` PASS |
| 최초 후보·실패 보존 | 9개 PASS: 총 연결 및 idempotence, traversal, catalog 없는 source, RIGHT slot 불일치, alias, target 누락, texture 누락, 다중 object 중간 실패에서 입력 보존 |
| 최종 map 연결·문서 보존 | 추가 10개 PASS: 여섯 binding 이외 semantic 동일, 각 연결 idempotence, 잘못된 map source·alias·기존 explicit binding 충돌 거부. `out/WorldObjectSourceMaterial20260912/validation.json` |
| 실제 Engine decoder와 CActorCatalog | 25개 PASS, failures 0. target material/원본 family·상수·texture 동일성과 실패 시 descriptor 보존. `material_probe.cpp`, `probe.log` |
| 최소 Client 컴파일 | `ActorCatalog.cpp`, `WorldObjectTool.cpp`, `WorldSequencePlayer_Objects.cpp` 세 TU out 전용 compile PASS. 기존 `Engine/VIBuffer_Rect.h`의 C4828 경고만 관찰. `compile_material.ps1`, `compile-material.log` |
| 실제 Area publisher 구조 검증 | `Publish-MapAuthoring.ps1 -AreaId LV_LUT_MIDNIGHTC_ED -Scope WorldSequences -Mode Validate` PASS. `map-validation-final.log` |
| 형식 검사 | 변경 JSON parse 및 소유 변경 파일 `git diff --check` PASS |

위 out 파일은 로컬 진단 산출물이며 소스 커밋 대상이 아니다. 일반 Model/World Object에 두 번째 런타임을 추가하지 않고 `CModel → CMaterial` 경로를 계속 사용한다.

## G04. 중단한 추가 네 소품

이 네 객체는 기존 embedded material을 유지한다. 원본을 찾았다는 사실과 제품 shader 등록 완료를 구분한다. 새로운 MODEL 85~88, SourceCharacter packing/dispatch, Engine program cap, BossCatalog override는 설치하지 않았다. 진행 중이던 out 전용 shader 추출도 종료했다.

| 객체 | 실제 원본과 현재 미완료 이유 |
|---|---|
| CuttingBlade | static slot `dummy_material_0`의 원본은 `mn_cngn_00.mat.mn_cngn_00_mi`. PBR Base MSK의 color variation+emissive 조합으로 Base `2b41020e9d64484f9b0c844745af2143` / Light `efdfb38a72332347af48b0fb8d607ac8`가 현재 MODEL에 미등록이다. 기존 가까운 family로 치환하지 않았다. |
| HornClown | body `mn_reup_04.mat.mn_reup_04_mi`는 기존 program 26과 Base/Light GUID가 같다. weapon `wp_mn_reup_01.mat.wp_mn_reup_01_mi`의 Base `7f3d666a6ef0984881ca1d6f72aa3a5a` / Light `dd1fcf291838dc48988c81901cd77e4b`는 미등록이다. 두 slot을 완전히 연결하기 전에 사용자 지시로 중단했다. |
| Trumpet | `wp_mn_reup_01.mat.wp_mn_reup_01-1_mi_dead`, legacy Monster Base MSK의 dead mask 조합. Base `fd7807729e65a14eae4074a3937bd61f` / Light `1d10aadedd169a4795c97d8a25fac8f5`가 미등록이며 PBR/IBL family로 바꾸지 않았다. |
| LaserCannon | `wp_mn_rhkp_06.mat.wp_mn_rhkp_06_mi_dead`, PBR Base MSK의 dead+emissive 조합. Base `231a7f149fd8054589dc4baf0825beb0` / Light `da7621e9468b8d4d9d69090a334b5828`가 미등록이다. Showtime 양손 총과 다른 모델이다. |

이후 재개 시 `out/WorldObjectSourceMaterial20260912/model-native/native_material_inputs.json`과 `horn-native/native_material_inputs.json`의 source MIC·static parameter set·effective numeric/texture closure를 사용해 기존 MODEL 생성·packing 경로에 필요한 정확한 pair를 연결해야 한다. 현재 out 조사물을 shader 설치 또는 시각 복원 완료로 해석하지 않는다.

## 최종 통합 확인

루트의 최종 Client 컴파일/링크·Server 빌드와 revision 352 공식 Kouku 4-domain 게시를 완료했다. 노란 3종은 Composition과 Client project에 등록했고 검증한 Decal CSO를 기본 Debug 경로에 반영했다. 상세 로그·중단 범위·사용자 실행 경로는 [통합 결과](2026-09-12_KOUKU_PARENT_PATTERN_TIMELINE_IMPLEMENTATION_RESULT.md#G03-배포와-사용자-확인)를 따른다. 사용자 화면 확인은 대기다.

