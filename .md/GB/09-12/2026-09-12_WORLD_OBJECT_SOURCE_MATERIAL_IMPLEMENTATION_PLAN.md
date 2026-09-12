# World Object 원본 재질·환경 반사 전달 구현 계획

## G00. 현재 실측과 변경 범위

Showtime 좌·우 총은 `Effect/KoukuSaydon/WorldObjects/SaydonShowtimeGun/SaydonShowtimeGun.wmodel`의 `wp_mn_rpct_08_mi`를 사용하지만 재질 원본 연결이 없다. 같은 원본 MIC를 사용하는 `Character/KoukuSaton/WP_MN_RPCT_07/wp_mn_rpct_07l_sk.wmodel`은 BossCatalog에 이미 등록되어 있다. 원본 program 26의 normal, diffuse, specular mask, hdr07_1 2D IBL, flat black, state texture, BRDF LUT 일곱 물리 입력과 IBL·roughness·specular 수치가 모두 존재한다.

313개 World Object 중 map binding 278개, catalog 연결 22개, inline profile 1개가 있으며 명시 연결이 없는 12개에는 sequence alias 2개가 포함된다. 전체를 하나의 재질이나 환경으로 바꾸지 않는다. 재질 소유 2D IBL과 장면 소유 cubemap은 별개 입력이다. 음수 scale용 reflected static mesh bake는 geometry 반사만 처리하며 광학 반사를 복구하지 않는다.

## G01. 공통 원본 재질 전달

`Client/Public/ActorCatalog.h`, `Client/Private/ActorCatalog.cpp`의 기존 descriptor 경로에 `Build_DerivedModelLoadDescription`을 추가한다. 실제 소비자는 World Object runtime과 Object Tool의 명시 Apply다. Resources-relative target model과 선택적인 source model을 받고 기존 catalog parser로 원본 override 전체를 stage한다. source가 명시되면 override 존재, target WModel decode 및 원본 override slot의 유일한 대응을 검사한다. 실패하면 출력 descriptor를 보존한다. source가 비면 기존 exact-model catalog 동작을 유지한다. shader program, texture expression, scalar, 색 공간, IBL 입력을 재해석하거나 특정 family로 치환하지 않는다.

`Client/Private/WorldSequencePlayer_Objects.cpp`는 위 helper를 사용한다. 기존 map binding 및 inline profile의 우선순위와 prepared-model 공유는 유지한다. `Client/Public/WorldSequenceDocument.h`의 materialSourceModelAssetId 설명은 cinematic 전용에서 모든 파생 World Object용으로 정정한다. 새로운 저장 필드와 파일 형식은 추가하지 않는다.

## G02. Object Tool의 편집·실패 보존

`Client/Public/WorldObjectTool.h`, `Client/Private/WorldObjectTool.cpp`에 선택 object별 재질 원본 입력 buffer를 둔다. 입력은 Apply 전에 Document에 들어가지 않는다. Apply는 실제 catalog와 target slot 검증 후 해당 resource의 source ID만 commit한다. 빈 source는 해당 모델 자신의 catalog 재질을 사용한다. 별도 source가 없는 embedded material은 source 복원 완료라고 표시하지 않는다. 모델을 교체할 때 기존 명시 source가 새 target과 호환되지 않으면 모델 변경을 거절하고 기존 모델과 pattern을 보존한다.

## G03. 데이터·추출 generator·publisher

`Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json`의 총 좌·우 두 resource에 동일한 원본 LEFT 모델 ID를 연결한다. 이 명칭은 손 위치가 아니라 실제 static 모델의 `wp_mn_rpct_08_mi` 소유자다.

`Tools/KoukuSaydonPipeline/build_source_sequences.py`는 파생 actor 생성 시 원본 모델에 catalog override가 있으면 기존 source 모델 ID를 기본 전달한다. 저장된 명시 map binding/profile은 계속 유지하고 원본을 알 수 없는 항목을 이름으로 추측하지 않는다. 총의 재질 연결은 해당 source-model 관계를 명시하는 재사용 가능한 apply 절차로 유지한다.

`Tools/MapPipeline/Publish-MapAuthoring.ps1`는 기존 materialSourceModelAssetId 검증에서 실제 catalog의 source override가 존재하는지와 override slot이 target에 있는지 확인한다. 실패는 기존 publish 결과 교체 전에 발생한다. 광역 native family 확장과 기존 미복원 객체의 강제 실패는 범위에 넣지 않는다.

동일 누락인 Bingo 바닥과 outer_fire D/E/F 네 객체는 원래 imported map asset과 같은 모델이며 기존 map material family가 지원된다. `Tools/ModelAssetConverter/apply_world_object_material_source.py`의 `assign_map_material_source`가 exact asset→model 일치, 전체 named slot 대응, 지원 family와 기존 binding 충돌을 검사한 뒤 기존 `mapMaterialBindings`를 연결한다. 위 generator도 이 네 원본 대응을 유지한다. 위치별 baked/static-shadow 입력은 기존 movable World Object 소비자가 제거한다.

CuttingBlade, HornClown, Trumpet, LaserCannon은 원본 MIC와 Base/Light shader pair까지만 조사한다. 사용자의 마무리 지시에 따라 신규 MODEL family 설치는 중단하며 기존 embedded 재질을 보존한다. 이 조사 후보를 제품 복원 완료로 기록하지 않는다.

## G04. 검증과 완료 경계

JSON parse, 일곱 입력의 실물 존재, 원본 material/scalar/IBL/BRDF의 동일성, 총 둘과 map 넷 이외 데이터 semantic 보존, source 누락·target slot 불일치·잘못된 경로의 후보 실패 보존, Python/PowerShell 문법과 최소 Client 컴파일을 확인한다. 변경 C++는 기존 파일 안에 있으므로 project/filter 등록은 추가하지 않는다. 공식 publish는 root가 수행한다. Client/UI를 실행하거나 시각 완성을 대신 판정하지 않는다. 실행한 증거와 사용자 화면 확인 경계는 같은 이름의 RESULT에 기록한다.
