# 현재 캐릭터와 선택 의상 재질 복원 구현 계획

## G00. 현재 소비 경로와 실측

현재 캐릭터 본체·기본 장비·무기와 선택 의상을 합쳐 설치 모델 332개, WMAT record 807개를 확인했다. 빈 미사용 record를 제외한 actual material key와 원본 package export를 대조한다. 기본 CharacterCatalog에는 native 재질이 있지만 선택 의상의 EquipmentPresentationService는 단순 경로 기반 CModel 생성만 수행한다. 따라서 같은 원본 재질을 선택해도 해당 source shader와 추가 texture 입력을 공급하지 못한다.

## G01. 기존 CActorCatalog 연결

CharacterCatalog formatVersion 4에 optional root `modelMaterialOverrides`를 추가한다. 기존 per-character override와 동일한 source material schema를 사용하되 선택 장비 `Character/<Class>/Equipment/` 경로만 받는다. 기본 body/equipment/weapon과 중복되는 ownership, 중복 material key, 잘못된 parameter/texture mask는 거부한다. root 행은 실제 기본 클래스 생성 목록을 늘리지 않는다.

ActorCatalog.cpp는 기존 material parser에 명시적인 최대 행 수를 전달하고 선택 장비 재질을 stage한다. Build_ModelLoadDescription은 기존 기본 캐릭터 소유권 다음에 선택 장비 descriptor를 조회한다. EquipmentPresentationService.cpp는 preTransform·본·prototype batch commit을 유지하면서 CModel::Create에 이 descriptor를 전달한다. 새 C++ 파일은 없으므로 프로젝트와 filters 등록은 필요 없다.

## G02. 원본 선택과 실제 리소스

실제 WMAT slot name을 원본 package export, MIC 상속, static shader map, GPU-skin Base/Light shader와 조인한다. shader 이름 유사성만으로 family를 선택하지 않는다. 기존 source program과 shader ID·uniform packing·texture expression index가 맞는 행만 재사용하며 새 permutation은 기존 translator와 renderer에서 검증한다. 누락 texture는 source object에서 추출하고 Resources-relative ID로 연결한다. 필요한 추가 UV가 없는 모델을 잘못된 family로 강제 로드하지 않는다.

## G03. 검증과 설치

최신 디스크 hash를 다시 확인하고 source/JSON 후보를 stable model/material key 단위로 병합한다. JSON parse·실제 참조 texture·slot·parameter mask를 검사하며 기존 CModel/CMaterial headless 소비자와 최소 Client 컴파일로 연결을 확인한다. root 세션이 통합 Debug 링크·배포를 수행한다. Client 실행·UI 조작·화면 판정은 사용자가 수행하며 source mapping 완료와 원작 외형 일치 판정을 구분한다.

## G04. 건슬링어·슬레이어 기본 의상

사용자가 지정한 두 기본 외형의 현재 설치 모델 14개에는 native override가 없다. GunSlinger 본체는
`pc_gn_f_00_sk`, Slayer 본체는 `pc_wr_f_00_sk`에서 왔으며 원본 export로 material identity를
확인한다. 같은 basename의 `PC_WBK_F_00.mat_high` 얼굴을 Slayer 몸에 임의 선택하지 않는다.

기본 body/equipment/weapon의 named slot마다 원본 MIC 상속, GPU-skin Base/Light shader와
uniform packing을 기존 family와 대조한다. 건슬링어 arm의 별도 permutation은 기존 translator로
program112를 생성하고 model admission·shader cohort84 범위를 함께 연결한다. 이미 수정 중인
program110/111과 선택 장비 서비스 변경은 보존한다. 새 C++ 파일은 없다.

눈과 머리의 추가 UV는 실제 원본 mesh의 같은 vertex에서 회수한다. 모델의 기존 geometry,
skin weights, skeleton, animation·event section은 보존한다. source texture는 정확한 원본을
`Character/SourceMaterials/<package>/`에 설치하고 기존 모델의 diffuse를 추정값으로 대체하지 않는다.
candidate의 CModel 생성·clone·shader binding을 확인한 뒤 최신 catalog의 대상 두 entry만 병합한다.
Resources 교체에는 기존 bytes 백업과 직전 hash 재확인·원자 교체를 사용한다.

## G05. 첫 이미지의 가디언나이트 기본·선택 장비

원본 `EFTable_PCPreview`의 702행과 `Item` 테이블, 각 LookInfo를 연결한 외형은 HR00 상·하의와 PC_DDK_02-5 팔·어깨·투구, WP_WDDK_04-5 무기다. 무기 LookInfo는 동일 WP_WDDK_04_SK geometry에 `WP_WDDK_04-05_MI`를 명시한다. 이름의 숫자 형식만으로 MIC를 추정하지 않는다.

정규화·cook한 다섯 의상과 동일 geometry 무기를 `Character/GuardianKnight/Equipment/class_select_hr00/`에 준비하고 기존 기본 장비는 `original_00/`에 보존한다. 각 여섯 slot의 새 외형·기존 외형을 총 12개의 stable visualSet으로 등록한다. CharacterCatalog는 다섯 기본 장비와 무기만 새 경로로 바꾸고 본체·날개·animation set을 유지한다. 실제 설치 골격의 `b_wp_1` 무기 socket과 기존 preTransform을 그대로 소비한다. 선택 catalog의 기존 221행과 최신 기본 재질 변경을 보존하며 리소스는 GBResources에도 같은 상대 경로로 복사한다.

## G06. 선택 장비 native ABI와 추가 UV

기존 source 재질과 정확히 일치하지 않는 Base/Light·uniform 조합은 program160~200으로 나누고,
기존110~112와 별도 cohort160/176/192에 배치한다. Character Select map208/210은 별도208
cohort로 통합한다. Engine/Client의 Base/Light leaf, animated/static/deferred wrapper와
`.vcxproj`·`.filters`를 함께 등록하고 dispatcher·CModel 허용 범위·CShader 변형 선택을 연결한다.
13개 translucent permutation은 기존 source forward 조명 경로를 공유한다. socket 무기의
static 모델도 같은 alpha pass를 사용하며 불필요한 본 palette 바인딩을 하지 않는다.

GPU-skin VS의 실제 varying을 읽어 hair UV1과 fur UV2를 구분한다. 원본 glTF와 설치 WModel의
position/UV0, normal·triangle corner를 조인해 WUVS에 추가하고 기존 geometry·skin·골격·애니메이션
bytes를 보존한다. 원본에도 UV1이 없는 program7 머리는 Base16.x/Light13.x의 two-tone 값이
모두0이고 range 분모가0이 아닐 때만 원본 출력의 UV1 비의존성을 이용한다. 가짜 UV를 생성하지
않으며 나중에 없는 UV를 요구하는 two-tone 변경은 CModel이 거절한다.

가디언 선택의 다섯 의상 slot은 두 outfit bundle로도 묶는다. 기존12개 개별 선택을 유지하면서
총14개 Guardian visualSet, 전체235개가 된다. CustomizingCostumes/Icons의 GuardianKnight
항목은 새 의상·기존 의상·새 무기·기존 무기 네 선택을 기존 소비 경로에 연결한다.
