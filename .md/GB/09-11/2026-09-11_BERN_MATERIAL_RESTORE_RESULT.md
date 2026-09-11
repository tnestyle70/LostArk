# 베른 원본 재질·모델 입력 통합 결과

작성일: 2026-09-11. 대응 계획은 `.md/GB/08-25/2026-08-25_BERN_CASTLE_VISUAL_FIDELITY_STAGED_PLAN.md`다. 이 문서는 실제 코드와 원본 입력의 연결 상태, 실행한 자동 검증을 기록한다. Area 최종 Publish/Check, Product6 결과와 해당 제품 DLL/CSO의 실제 모델 바인딩을 확인했다. 사용자가 직접 수행하는 Client 화면 판정은 아직 기록하지 않는다.

## 1. 원본 identity와 대상

원본 StaticMeshActor 32,324개와 instanced foliage 17,651개를 합쳐 49,975개 native 배치를 조사했다. 고유 StaticMesh 961개, material signature 1,216개, source MIC 845개, 실제 material slot 1,797개다. 기존 landscape 42개와 합친 authoring 배치는 50,017개다.

native section 순서와 index 개수, component의 nullable override, parent chain, selected shader cache를 연결했다. 기존 WMAT 이름과 실제 source override가 달랐던 범위는 221개 asset의 14,607 slot-occurrence였다. 원본 Bern 전용 나무·수로 재질을 일반 needletree/Koilsv leaf 이름으로 대체하지 않는다.

동일 MIC가 두 WMAT entry에 반복된 2모델은 고유 named override 하나를 두 material entry 모두에 적용한다. source identity가 서로 다른데 이름만 같은 경우는 writer가 거부한다. 따라서 실제 slot 수와 JSON 고유 행 수는 서로 다르다.

원본 navmesh cul01/02는 component `hiddengame=true`인 336개 배치다. 현재 authoring에도 모두 비가시 상태였으며 이를 유지한다. 해당 두 MIC의 6개 material slot은 source hidden 대상이다.

## 2. 실제 구현

- 기존 `CMapAssetCatalog → MODEL_ASSET_LOAD_DESC → CModel → CMaterial` 경로에 source 입력을 연결했다. 별도 모델 런타임을 만들지 않았다.
- material별 renderMode/cullMode/Shadow를 저장·검증하고 MapAssetObject의 기존 CModel mesh를 해당 render group에 제출한다. generic GameObject의 기본 Render 동작은 유지한다.
- 같은 physical WModel/pre-transform의 material·RNM variant는 CModel mesh buffer를 공유한다. 재질과 atlas는 별도 CMaterial에 연결하되 같은 SRV는 weak cache로 재사용한다.
- BG8에 original normal·specular·2D reflection·emissive·bump·mask·UV·address와 subspecular/rim/panning을 연결했다. 원본 floor01의 normalIntensity=-1도 실제 식의 signed 입력으로 보존한다.
- overlay7은 source normal/overlay normal 유무, vertex paint, direction, height inversion, detail normal, mask와 별도 specular를 처리한다. 기존 사용자 승인 쿠크 바닥의 기본 branch는 유지한다.
- source foliage9/grass10, snowice11/vertexblend12/wet13은 각각 native 입력을 사용한다. 물17 MIC, 기타 forward48 MIC, monster5 MIC는 선택된 native program의 파라미터와 texture slot을 실제 CMaterial에 바인딩한다.
- forward material은 SourceCharacter deferred frame의 256-row 등록을 사용하지 않는다. Base/Baked/Light 계산, scene depth/color, 시간은 기존 forward pass에서 소비한다.
- publisher는 모든 선택 texture의 Resources-relative 경로와 실물, finite 수치, strict fields, color space, named WMAT 존재를 검사한다. bakedLighting의 optional staticShadow와 lightChannel/atlas UV를 검증하며 기존 transaction 경계를 유지한다.

Resources 입력은 `Client/Bin/Resources/Map/LV_BER_BERNCASTLE/SourceMaterials` 등의 기존 Map 하위에 설치했다. 추출 원본과 Resources binary를 Git 추적 대상으로 추가하지 않는다.

## 3. 정적 UV2와 geometry 호환성

`lv_rhd_queen.mesh.sky_mirror_sm`의 UV2는 UV0/UV1과 다르다. 원본 triangle-corner join으로 확인한 2,208개 정점의 UV2를 WMSH 1.4에 보존했다. evidence bit 17, optional UV2와 UV1, payload stride·finite 값을 reader에서 함께 검증한다. sky의 source program61은 `requiredExtraUVMask=2`로 실제 UV2 없는 모델을 거부한다.

런타임 VTXMESH는 기존 member offset을 유지하고 끝에 UV2를 추가한 76-byte 구조다. 기존 정적 1.0/1.1/1.2와 skinned 1.3 decoding 경로를 유지하며, 1.4로 재변환한 물리 입력은 해당 sky 한 개다. 원본 1.2 sky와 새 1.4 sky를 같은 native C++ reader로 각각 성공적으로 읽었다.

최종 물리 이름 검증에서 `lv_matte.mesh.sky_mirror_sm`에 다른 package의 동명 UV2 sky가 설치된 오류를 발견했다. 조명 설치 작업을 exact asset ID 선택으로 교정하고 lv_matte를 자신의 검증된 geometry로 복원했다. 961개 물리 모델의 최종 named material 검증은 오류 0이다. 같은 leaf 이름만으로 자산을 선택하지 않는다.

## 4. 실행한 검증

| 검사 | 실행 결과와 범위 |
|---|---|
| 기본 BG actual helper vs native DXBC | 563 MIC × alpha 4 = 2,252 fixture, 실패 0, 최대 상대 오차 5.96e-8. 최신 BG helper 합류 후 재실행. uniform 1×1 texture로 Base 표면·normal-driven reflection·mask를 검사했으며 UV/address/LOD는 이 fixture 범위에 포함하지 않았다. |
| overlay actual helper vs native DXBC | 53 MIC의 Base/Light/Baked × vertex R/A 3 = 477 fixture, 실패 0, 최대 상대 오차 2.54e-7. Engine/Client source helper 동기화 뒤 재실행했다. |
| BG 추가 branch | 담당 구현의 Base/Baked/Light 399 fixture, panning 48 fixture, Deferred rim 4 fixture 통과 결과를 수신했다. |
| foliage/grass, special | 담당 구현이 원본 shader와 Base/Baked/Light 비교를 완료했다. special 336 fixture 및 interpactor BG8 21 fixture 통과 결과를 수신했다. |
| native C++ Catalog | water Baked 합류 후 최종 23 shards/16,741 assets와 23,153개 material 행을 실제 C++ Load_Source로 통과했다. baked 21,321행/shadow 16,421행이며, 실제 Read는 50,017 placement와 RNM 49,047개를 통과했다. |
| 실제 CModel/CMaterial | Product6 Engine DLL과 headless WARP에서 water Baked를 포함한 87개 family/program/baked/SDF/renderMode 조합의 대표 81모델을 생성하고 81개 material variant를 만들었다. 실제 143 mesh material의 family/program/lighting/render/cull가 일치했으며 오류 0이다. 동명 material 두 모델의 모든 slot도 검사했다. |
| 제품 CShader와 material 바인딩 | Product6 Binary 3,639,678 bytes 및 MapInstance 9,090,535 bytes CSO를 실제 CShader::Create로 읽었다. 실제 CModel의 native binding 80회, lighting binding 206회, special binding 18회와 Effects pass Apply 206회가 통과했다. native 종료 코드 0, 오류 0, 실행 71초이며 Engine DLL의 실제 로드 경로도 확인했다. Engine/UI 초기화·RT mock·Draw를 수행하지 않았으므로 전체 장면 렌더나 화면 판정으로 사용하지 않는다. |
| 물 Baked admission 추가 | water40~43의 C++/publisher 허용·SRV·UV·receiver 경계를 연결하고 최소 CPP 5개 검사를 통과했다. 원본 atlas pair가 있는 19행은 실제 C++ Catalog와 publisher 함수 모두 통과했다. 반대로 water39의 Baked 입력은 두 경로 모두 명시적으로 거부했다. |
| 물 Baked 실제 shader | 담당 구현의 water40~43 Baked PS 4개가 실제 forward/RNM 경로에 연결됐고, strict 원본 9 MIC와 DXBC 비교 162 fixture/165,888 pixels 통과 결과를 수신했다. 최종 데이터 재병합, actual C++ 파서 및 Product6 실제 Engine/CSO 바인딩까지 통과했다. |
| deferred native frame 상한 | 실제 native 653행 중 program33~65의 610행은 frame 등록에서 제외된다(609 forward 행과 조명 수용을 차단한 black 1행). monster80~83은 43 asset/43 WMAT CMaterial entry이며, 전체 80배치 중 visible 79배치가 사용하는 고유 asset은 34개다. placement의 CModel clone은 CMaterial shared_ptr를 공유하므로 어느 카메라에서도 Bern 정적 맵의 동시 등록 상한은 34개로 256 제한 미만이다. NPC/플레이어의 별도 재질은 이 정적 맵 상한에 포함하지 않았다. |
| actual publisher material parser | 기존 PowerShell 함수 본문으로 중간 1,787행의 실제 WMAT 이름·DDS·numeric 검사를 통과했다. 이후 최종 Area Validate와 water Baked 포함 Publish/Check 50 files/23 shards/50,017 placements 통과 결과를 확인했다. |
| UV2 cooker/reader | source payload 완전 비교, 1.4/1.2 두 입력 native C++ reader 통과. 새 sky 175,320 bytes/2,208 vertices, UV2 합계 1,103.66 및 1,095.29. |
| 최소 컴파일 | Material/Model/Catalog/MapAssetRenderUtils/MapPlacementRuntime `/Zs`, geometry reader `/Zs`, MapInstance 전체 FXC 통과. 최종 helper64/65와 water Baked 합류 후 통합 담당의 Product6 빌드 통과 결과를 수신했다. |
| 저장 형식 | 변경 publisher PowerShell AST parse, Python cooker parse, 관련 변경 `git diff --check` 통과. |

OUT 증거는 `out/BernMaterialAudit20260911/`의 `material_variant_handoff.json`, `bern_material_rows.json`, `bg_native_vs_product_helper_warp.json`, `overlay_warp.json`, `catalog-native-1773.log`, `cmodel-material-admission.log`, `cmodel-material-binding-product6-native.log`, `material-binding-native-exit.json`, `material-binding-product-cso.json`, `material-binding-engine-dll.json`, `publisher-material-probe.log`, `installed_material_name_audit.json`, `native_frame_registration_audit.json`, `SkyNativeUV2/cook-result.json`에 있다. 통합 Product6 receipt는 `out/BuildPipeline/runs/20260910T185256023Z-debug-product.json`이다. 이 OUT 자료 자체를 제품 런타임 정본으로 사용하지 않는다.

## 5. 최종 연결 수와 남은 경계

source visible 대상 843 MIC를 1,789개 고유 named 행으로 연결했다. 실제 1,791 slot을 복원하며, 원본 hidden navmesh 2 MIC/6 slot은 가시 대상에서 제외했다. source texture 1,477개를 설치했고 RNM/색 variant 병합 후 authoring은 23,153개 material 행이다. water Baked를 포함한 최종 병합에서 RNM 49,047배치, SDF 40,247배치를 연결했다. 최초 병합의 NoConsumer는 asset 단위여서 mixed asset의 다른 BG 슬롯이 RNM을 소비하면 물 슬롯의 미연결을 보여 주지 못했으며, 현재는 slot 단위 집계를 함께 기록한다.

추가 source-slot 조사로 water40~43 Baked 대상 19 variant material 행/25 slot-occurrence를 연결했다. program별로 40은 7행/7개, 41은 11행/15개, 42는 1행/3개이며 43은 해당 원본 RNM 배치가 없다. 전체 RNM/SDF placement 증가는 각각 6개다. 38/39는 Baked를 소비하지 않으며, RNM이 있는 component의 해당 물 slot-occurrence는 각각 10/46개다. 그 밖의 source-slot 비소비는 hidden navmesh 335개와 unlit forward53 1개다. 이 수치를 asset 전체 RNM 비소비 개수와 혼동하지 않는다.

foliage/grass/interpactor의 원본 wind VS와 파라미터는 조사했지만 실제 vertex deformation 및 scene wind·mesh bounds CPU 입력은 연결하지 않았다. overlay의 sourceDirection은 정적 원본 material 방향만 소비한다. 동적 바람까지 복원한 것으로 기록하지 않는다.

Product6 빌드와 Area Publish/Check는 통과했으며 사용자 직접 실행 결과는 아직 없다. Agent가 Client/UI를 실행하거나 화면을 캡처하지 않았다.

원본 식의 수치 비교는 선택한 shader와 fixture에 대한 결과다. 원본 실행 장면 전체의 카메라·wind·빛 입력까지 동일하다는 뜻으로 사용하지 않는다. 사용자가 Bern에서 원본과 같은 광장·수로·나무·금속 구도를 직접 확인해야 하며, 현재 문서는 화면 일치나 visual PASS를 선언하지 않는다.
