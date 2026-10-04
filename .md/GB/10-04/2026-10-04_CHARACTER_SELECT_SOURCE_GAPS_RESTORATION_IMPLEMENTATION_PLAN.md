# Character Select 원본 렌더링 입력·UI 누락 복원 구현 계획

## G00. 현재 실측과 이번 변경 범위

`LV_LOBBY_CLASSSELECT_SL00`의 원본 MIC 7개가 사용하는 재질 18행에는 식생 vertex 입력이 없다.
현재 native ShaderCache와 MIC static set을 다시 대조한 결과 5 MIC는 E4FE, 2 MIC는 1C39를 사용한다.
원본 `LV_LOBBY_PS`에는 WindDirectionalSource 하나가 있으며 component Strength는 0.699999988079071,
Speed는 2.0이다. 원본이 존재하는 이 Scene에 무풍 기본값을 넣지 않는다.

제품 UI가 참조하는 PNG 10개가 설치되지 않았다. 원본 EFUI_LOBBY, EFUI_CREATECHARACTER,
EFUI_SHAREIMAGE에서 CharacterSelect_IA/ID, CreateCharacter_I1/ID7, ShareImageV2_I46 atlas를 새로
추출했다. 기존 GFX 기반 crop 및 alpha 합성 레시피를 그대로 재사용한다. layout와 사용자 편집은 변경하지 않는다.

원본 회전 장식 component export 307/308 두 개에는 `EFActorMotionRotationAcyclic`, `AXIS_Z`,
`fMotionVel=+5/-5`가 존재한다. 이 class의 실행 구현은 native이며 `.u`에 실행 bytecode가 없다.
기존의 검증된 mapmotions를 소비할 Level 호출은 연결하고, 원본 장식 2행의 설치는 속도 단위·축 합성·
시간 owner의 근거가 확보된 경우에만 수행한다. 직렬화된 scalar를 임의로 degree/sec로 취급하지 않는다.

## G01. 원본 PNG 설치

변경 대상은 `Client/Bin/Resources/UI/CharacterSelect/`의 option/rename normal/hover 4개와
`Client/Bin/Resources/UI/ClassSelect/Common/`의 CreateCharacterCenterButton normal/hover,
TrialV2Button normal/hover, TrialModeBand, TrialModePlate 6개다.

원본 package identity와 export serial SHA를 기록하고 UModel의 명시적 CLI export만 사용한다.
원본 atlas에서 기존 `Tools/LpkPipeline/build_character_roster_ui.py` 및
`build_class_select_trial_ui.py`의 `build_art` 함수를 후보 출력에만 호출한다. layout 저장 함수는 호출하지 않는다.
후보 10개를 decode하여 유효 크기·alpha 및 pixel SHA를 확인한 뒤 최신 target hash와 원본 package hash를
다시 확인한다. 존재하는 target은 백업하고 같은 directory의 임시 파일에서 원자적으로 교체한다.
실패 시 이 작업이 설치한 파일만 원래 bytes로 rollback한다. 이미지 생성·추측 대체는 사용하지 않는다.

## G02. 원본 식생 입력 18행과 배치 입력 62행

공용 `source_foliage_wind.py`, `Shader_SourceFoliageWind.hlsli` 및 공용 binder는 Bern 복원 담당자가
소유한다. 이 작업은 7 MIC의 native uniform, 18 재질행에 해당하는 13 asset/5 물리 모델의 원본 mesh bounds,
62 배치의 owner position·primitive bounds, Scene wind의 actor/component/CDO 근거를 제공한다.
owner position은 실제 원본 Actor.location을 사용한다. collection actor의 9 component도 개별 배치로
식별하고, 같은 재질에 여러 owner가 있는 경우를 재질 공통 위치 하나로 합치지 않는다.

`placementWind`에는 stable sourcePlacementId/assetId와 UE3 좌표의 actorPositionSourceCm[3],
objectDimensionsAndRadiusSourceCm[4]만 넣는다. bounds는 원본 FBoxSphereBounds와 component/actor
복합 matrix로 계산한다. 원본 DLL의 TransformBy는 extent에 abs(matrix)를 적용하고 radius에는
최대 column norm을 사용한다. Static UpdateBounds의 +1 cm를 적용한 뒤 BoundsScale을 곱한다.
특수 userdata 0x8000 draw branch와 원작 전체 시간 owner는 확인 경계로 남긴다.

현재 source 모델의 cm→m preScale과 `[x,y,z]→[x,z,-y]` 좌표 변환을 중복하지 않는다.
원본 cooked vertex program의 instruction identity·uniform expression denominator를 확인하며
runtime에서 소비되는 Time lane은 기존 Renderer의 시간 owner를 사용한다. 재질의 다른 field와 baked lighting,
TRS, visibility, 사용자가 OFF로 설정한 Bloom/Fog/FXAA는 보존한다.

최신 저작·게시 문서를 다시 읽고 `(assetId, materialName)`의 18행에 `foliageWind`와 루트
`placementWind` 62행만 추가한다. 다른 field가 변경되지 않았음을 구조 대조하고 source와 target hash를
재검사한 뒤 backup/atomic install한다. 후속 원본 수식 정정은 이 두 소유 필드만 fresh merge한다.

## G03. 원본 반복 회전과 검증 경계

`CMapPlacementRuntime::Read_SelfMotions`, `Sample_SelfMotions`, `Load_SelfMotions`,
`Update_SelfMotions`가 기존 소비자다. 원본 native 단위가 확인되는 경우 새 manager를 만들지 않고
Area mapmotions 2행을 추가한다. `CLevel_CharacterSelect::Initialize/Update`에는 main과 성공적으로
불러온 background의 기존 mapmotions 소비를 연결한다. 현재 데이터가 없는 Area는 실패하지 않는다.
원본 native 구현을 확인할 수 없는 항목은 직렬화 사실과 남은 단위·시간 owner 경계를 RESULT에 기록한다.

이 복원 주제에는 새 C++ 파일과 project/filter 재등록이 필요하지 않다. 별도 사용자 지시로 추가한
WorldSceneTool animation module의 UI/미리보기 검증은 독립 F1 도구 주제에서 함께 검증한다.
Python 후보·설치 receipt, JSON parse, 18행의 원본 shader와 62행의 bounds/owner 대조, 무관 field 보존,
설치 PNG decode, GFX 원본 subimage 재대조, 읽기 전용 publisher Validate/Check,
독립 F1 animation TU의 /Zs, `git diff --check`를 수행한다.
실제 C++가 변경되면 root 담당자가 관련 Debug/Release 제품 빌드를 통합하여 검증한다.
Client·원작 게임·UI 실행과 최종 화면 판정은 수행하지 않는다.
