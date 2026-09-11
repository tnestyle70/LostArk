# 쿠크 전체 맵 원본 재질 복구 결과

기준일: 2026-09-11. 구현 계획은 [기존 전체 맵 계획](../09-09/2026-09-09_KOUKU_FULL_MAP_MATERIAL_RESTORE_IMPLEMENTATION_PLAN.md)을 재사용했다. 이 문서는 정적 맵 재질과 실제 파서·draw 경로의 결과를 기록한다. 보스 원본 프로그램, 환경·광원·RNM 추출, 통합 Product 빌드와 사용자의 화면 판단은 해당 작업의 결과를 함께 확인한다.

## G00. 실제 연결 범위

적용 전 원본 catalog 330개에서 사용하던 310개 모델의 384개 슬롯을 조사했다. 377개 슬롯에 원본 재질을 연결했고, 남은 7개는 원본 lv_navimesh cul01/02를 참조하는 숨김 6개 에셋이다. 카드 형태의 숨김 1개 에셋도 실제 native override가 cul02이며 화면용 카드 재질을 임의로 적용하지 않았다. 숨김 에셋의 배치는 109개다.

RNM별 variant를 합친 최종 catalog는 1,301개, placement는 3,368개(visible 2,917개)다. 현재 사용 에셋 1,128개의 실제 installed WModel 사용 슬롯 1,539개 중 1,532개에 복원 행이 있으며, visible 미복원 슬롯·잉여 override·WModel census 오류는 모두 0이다. 기존 사용자가 승인한 바닥 2행은 JSON 의미 비교가 정확히 일치한다.

| 최종 mapmaterials family | 행 수 |
|---|---:|
| bg-source-opaque-masked / BG8 | 1,698 |
| bg_base_opa_overlay / overlay7 | 13 |
| 기존 승인 bg_base_msk / bg_seamless-specular_msk | 2 |
| source.character Mario 5개 | 5 |
| source.map translucent | 5 |
| source.map spotlight | 8 |
| source.map sky | 1 |
| 합계 | 1,732 |

정의는 1,275개 asset과 원본 MIC 219개를 참조한다. 실제 RNM texture를 가진 재질 행은 1,355개, placementLighting은 2,364개다. RNM 생성과 native geometry/atlas 설치는 조명 복원 작업에서 수행했다.

## G01. 원본 BG와 overlay의 소비자

Engine의 기존 CModel → CMaterial 경로에 SOURCE_BG_OPAQUE_MASKED=8을 추가했다. 원본 static switch에 따라 bump, alpha clip, normal strength, COLOR0.a, 별도 S, signed 2D reflection, simple/seamless specular cap, emissive flicker를 선택한다. 원본 UV 이동은 고정 offset을 유지하고 signed tiling과 flicker minimum 2.0을 그대로 허용했다.

MapAssetCatalog와 publisher는 동일한 formatVersion 2 필드·texture·branch를 검사한다. 원본에서 선택하지 않은 reflection/normal/S를 필수 입력으로 요구하지 않는다. MapAssetRenderUtils는 선택한 SRV·상수·RNM을 바인딩하며 MeshBinary와 MapInstance가 같은 표면 계산과 marker8을 사용한다. Deferred는 BG의 원본 specular cap 분기를 구분한다. 그림자 alpha도 실제 TBN/view로 계산한 parallax diffuse를 사용한다.

기존 overlay7에는 UV tiling과 별도 S 입력을 추가했다. 쿠크 floor18의 tableprop01d 재질은 원본 UV 3×3과 specular texture를 사용한다. 기존 Valtan overlay의 기본 입력은 유지한다.

source properties에서 확인한 DDS 201개를 `Client/Bin/Resources/Map/LV_LUT_MIDNIGHTC_ED/SourceMaterials`에 설치했다. 원본 top mip를 유지하고 기존 도구로 하위 mip를 생성했다. diffuse U mirror와 floor18 D의 V mirror, wall04 N의 U/V mirror도 실제 sampler에 연결했다. 마지막 두 texture의 추가 address 연결은 4개 원본 슬롯과 16개 RNM variant, 총 20행이다. sampler bit는 Deferred half-float 저장에 넣지 않아 불필요한 정수 정밀도 손실을 피한다.

## G02. 원본 forward와 Mario 경로

source.* 행은 SourceCharacterMaterial의 기존 strict parameter·texture packing을 재사용한다. source family6 에셋은 static batch 대상에서 제외하고 기존 CMapAssetObject → CModel로 제출한다. 원본 forward shader 구현과 입력은 별도 복원 작업에서 연결했다.

실제 C++ parser에서 native 19행의 렌더 모드는 Deferred 5(Mario), Alpha 5, Sky 1, Additive 8이다. 원래 Opaque로 남아 있던 forward 5개 asset의 catalog mode를 원본 blend에 맞춰 교정하여 실제 PS_MAIN_ALPHA/PS_MAIN_SKY 소비를 연결했다.

## G03. 실행한 자동 검증

| 검증 | 실제 결과 |
|---|---|
| Material.cpp, Model.cpp, MapAssetCatalog.cpp, MapAssetRenderUtils.cpp, MapPlacementRuntime.cpp 최소 /Zs | PASS, 기존 문자집합 경고 존재 |
| MapInstance fx_5_0 최신 sampler·shadow 포함 | PASS, helper의 기존 X4000 경고 존재 |
| Deferred fx_5_0 | PASS |
| MeshBinary/forward FX | 통합 담당이 컴파일 PASS를 확인; 최종 Product 재빌드 결과는 통합 RESULT 확인 |
| 원본 BasePass DXBC ↔ 실제 BG helper WARP | 198 MIC × alpha 4종 = 792 fixture, 실패 0, 최대 상대 오차 2.9802322387695312e-8 |
| 실제 CMapAssetCatalog::Load_Source + MapPlacementDocument::Read | 1,301 entry / 1,732 material / 3,368 placement / 2,364 RNM instance 전수 PASS |
| installed WModel material slot join | visible 미복원 0, 잉여 override 0, 파싱 오류 0 |
| 기존 승인 바닥 행 의미 비교 | 2/2 정확히 일치 |
| JSON 및 out probe project XML parse, git diff --check | PASS |

WARP 비교는 동일한 1×1 texture/CB 입력에서 diffuse·normal에 의한 reflection·alpha discard·원본 상수 packing을 검증한다. 원본 화면, 실제 시간별 장면, 비균일 texture의 UV/address/LOD, 최종 direct/RNM 영상의 동등성을 이 수치로 대신 판정하지 않는다.

재현 자료는 `out/KoukuFullMaterialRestore20260911`의 `final_material_coverage.json`, `bg_native_vs_product_helper_warp.json`, `catalog-native-run.log`, `compile-changed-cpp.log`, `mapinstance-fxc.log`, `source_sampler_remaining.json`에 있다. out 산출물과 Resources binary는 Git 소스에 추가하지 않는다.

## G04. 배포와 사용자 화면 확인

최종 Area publisher Publish/Check와 Product 빌드는 통합 담당이 진행한다. 에이전트는 Client·UI 실행, 캡처, visual PASS 판정을 하지 않았다. 기존 바닥 2개에 대한 사용자 승인 외에 이번 전체 맵 표면과 조명 조합의 화면 판단은 사용자가 직접 한다. 구조적으로 정적 공이나 맵 물체의 반사가 불가능한 상태가 아니며, 원본 입력이 연결된 native 모델 경로로 렌더링 준비를 진행한 상태다.
