# 발탄 본체·무기·유령 재질과 중앙 바닥 구현 계획

## G00. 실측과 목표

현재 발탄 모델 5개의 실제 사용 재질은 11슬롯이며 BossCatalog에는 이 모델의 source material override가 없다. CValtanPresentationAssetService는 경로 기반 CModel 생성으로 동일 공통 override를 소비하지 않는다. 본체·분리 갑옷·도끼의 원본 5MIC는 이미 구현된 native Base/Directional program21과 정확히 같은 shader pair다. 유령의 3MIC는 별도 rim/cloud/opacity native pair이며 비충돌 program84를 추가한다. 33..65는 source-map forward 예약 범위이며 80..83은 static map monster다. 원본 MIC 상속·static shader map·texture expression 8개를 전부 읽었고 closure 실패는 없다.

## G01. 기존 CModel과 재질 입력

BossCatalog root modelMaterialOverrides에 발탄 11슬롯만 추가한다. 기존 쿠크 override와 boss row는 보존한다. Body·armor·weapon 생성은 CActorCatalog::Build_ModelLoadDescription에서 검증한 MODEL_ASSET_LOAD_DESC를 사용하며 donor animation은 계속 별도 animation 입력이다. 필수 body/weapon 실패는 prototype batch commit 이전에 거부하고 갑옷은 기존 plate 격리 정책을 유지한다.

SourceCharacterMaterialParameters.h의 새 family는 원본 named parameter를 strict packing한다. 양쪽 SourceCharacterPrograms.hlsli에는 program84의 원본 두 PS와 시간식만 추가하고 기존 program을 보존한다. 양쪽 SourceCharacterMaterial.hlsli는 유령의 원본 VS varying 배치·fog identity·opacity coverage를 기존 shared draw에 연결한다. sorted translucency는 해당 기존 draw가 제공하지 않으므로 원본 투명 정렬 완료로 판정하지 않는다.

## G02. 중앙 돌바닥 차이

첨부 화면에는 밝은 청록 균열 바닥, 어두운 중앙 원형 mesh와 바깥 돌이 구분된다. 09-08 복원은 정적 floor4+rock3에만 overlay·배치색·RNM을 연결했다. 현재 Deploy 정상/파괴 바닥, 중앙 하층과 주변 정적 floor의 원본 source ID·MIC·slot을 확인해 실제 다른 재질과 미복원 입력을 분리한다. 원본이 다른 두 재질을 임의로 한 재질로 통일하지 않는다.

중앙 원형 배치 `LV_LUT_HEARTRB_ED_SL00:export:1274`는 레벨의 명시적 두 MIC override를 사용한다. 현재 rain MIC 이름을 materialName 키로 유지하고 sourceMaterial과 실제 상속 입력을 올바른 `lv_lut_heartrb` MIC로 연결한다. 원본2MIC는 기존 stone MIC와 동일한 engine-equivalent static set이므로 기존 `bg_base_opa_overlay` 경로를 사용한다. 원본 native index stream과 GLTF corner를 전부 대응시켜 UV1·tangentW·component BGRA를 검증하고, BGRA를 RGBA로 한 번 변환한 전용 variant geometry에 저장한다. catalog에 variant 한 행을 추가하고 해당 stable placement만 교체하며 원본 RNM atlas pair와 배치별 scale/bias/coefficient를 연결한다. 신규 원본 D/N DDS 네 장만 추가하고 기존 overlay/RNM을 재사용한다.

## G03. 검증과 종료

원본 texture 26개를 정확 source object에서 가져오고 DDS mip를 준비해 Resources/Character/SourceMaterials/Valtan에 둔다. Resources는 Git 제외 입력이며 Drive 전달이 필요하다. 필요한 C++·shader 컴파일, JSON parse, strict packing과 실제 CModel create/clone/bind 검사를 기존 진단 실행 경로로 수행한다. 전체 Product 빌드는 root가 조율한다. 새 production C++ 파일은 없고 project/filter 등록 변경은 없다. Client/UI 실행·캡처와 최종 화면 판정은 사용자가 수행한다.


사용자의 최신 종료 범위:84 충돌 수정 뒤 Product 재빌드 결과가 나오면 종료한다. 따라서 그 이후 추가 CModel probe compile/run과 Client/UI 확인은 이번 세션에서 수행하지 않으며 RESULT에 미실행으로 남긴다.
