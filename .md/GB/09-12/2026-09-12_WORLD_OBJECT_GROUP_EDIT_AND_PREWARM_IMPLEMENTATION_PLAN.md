# World Object 그룹 편집과 카드 사전 준비 구현 계획

## G00. 현재 정본과 목표

Object Tool의 Motion은 생성 개수·각 emission의 위치/Yaw/Delay·속도·자전·공전을 이미 소유한다.
Composition Box는 그 그룹의 Transform과 시간 구간을 소유한다. 이번 변경은 이 경계를 유지하면서
Box에서 정확한 Object/Motion을 열고, 생성 개수와 간격을 조절할 때 Collider/Logic 연결도 보존한다.
조커 첫 생성은 같은 WorldSequencePlayer의 Prototype/Clone 경로를 사전 준비하도록 확장한다.

`Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json`은 Object/Motion
정본이다. Pattern Composition과 독립 Sequence Composition은 이를 stable instance ID로 참조한다.
현재 갈고리 emission 18행은 Pattern 18·19의 Collider/Logic 각각 18행과 연결돼 있다. 배열의 중간
삭제·복제는 기존 `worldEmissionIndex`의 의미를 바꾸므로 Object 문서만 저장하면 안 된다.

## G01. Box에서 기존 Object/Motion 편집기로 연결

Workbench는 objectId와 motion instanceId를 담은 one-shot 요청을 소유하고 MainApp이 소비한다.
WorldObjectTool의 공개 선택 함수는 ID의 소유 관계를 확인한 뒤 기존 Select_Object/Select_State를
사용한다. 찾을 수 없는 항목은 기존 선택·미저장 문서를 유지한다. 열기 자체는 Save/Publish/Preview를
시작하지 않는다. Box에는 이미 갱신되는 iEmissionCount를 표시하고 Object 공통 편집과 초기/연결
Motion 편집을 각각 제공한다. Box 자체 Transform·시간·재생 속도는 기존 경로를 유지한다.

변경 파일은 `Client/Public/WorldObjectTool.h`, `Client/Private/WorldObjectTool.cpp`,
`Client/Public/KoukuSaydonActionWorkbench.h`, `Client/Private/KoukuSaydonActionWorkbench.cpp`의
요청 소비·Render_WorldBoxDetails와 `Client/Private/MainApp.cpp`의 도구 요청 전달 구간이다.

## G02. 생성 row 편집과 연관 Collider/Logic 보존

Object 편집 세션은 저장 기준 emission과 현재 row의 대응을 유지한다. 기존 row 수정은 같은 항목의
identity를 유지하고 복제는 원본 row의 provenance를 가진다. 개수 preset·간격·방향 편집은 이 대응을
통해 처리하며 index 이동을 단순 숫자 clamp로 숨기지 않는다.

사용자가 Object Save를 누르면 변경 Motion을 참조하는 모든 Composition WORLD를 찾고, 해당
WORLD Collider의 emission index와 window를 새 row에 연결한다. row 복제는 연결 Collider와
그 Collider의 Logic occurrence를 함께 복제하고 새 stable occurrence ID를 발급한다. row 삭제는
해당 Collider와 전용 Logic을 함께 정리한다. 공유된 Logic·hold·outcome·다른 target의 참조는
검증한 대응으로 유지하며 자동 대응할 수 없는 혼합 의존성은 이유를 표시하고 저장 전 상태를 보존한다.

Result/Contact가 직접 부르는 Motion, Contact target과 NEXT chain도 변경 참조에 포함한다.
기존 단일 Object 판정은 Count 1을 유지하고, indexed WORLD Collider는 명시 emission row를 요구한다.
Delay 변경이 WORLD box 수명을 벗어나면 그 box를 먼저 수정하도록 저장을 거부한다.

각 문서를 parse→validate→stage한 뒤 저장 직전 baseline을 다시 비교한다. 열려 있는 Composition에
미저장 편집 또는 Publish가 있으면 기존 편집을 덮어쓰지 않고 연관 저장을 거부한다. staging 실패,
외부 파일 변경, publish 실패는 성공으로 표시하지 않는다. Object/Composition의 기존 serializer와
publisher를 사용하며 별도 모델·물리 runtime을 만들지 않는다. 실제 Data 파일은 구현 중 자동 수정하지
않고, 실행 중 사용자가 명시적으로 Save하는 경로에서만 교체한다.

## G03. 조커 카드 생성 준비

현재 각 WORLD occurrence의 독립 player가 같은 카드 모델을 준비하고 실제 표시 시점에
CWorldSequenceObject와 CModel clone을 만든다. 기존 문서 batch·모델 cache와 Prepare_InstanceResources를
재사용해 중복 prototype 준비를 줄이고, 카드 occurrence의 필요한 clone을 시간 진행 전에 준비한다.
사전 준비된 객체는 표시와 gameplay 판정을 시작하지 않는다. Play는 기존 clock·anchor·Sample을
사용한다. 문서 교체·Stop·실패에서는 준비 객체를 기존 수명 계약으로 회수한다.

WorldSequencePlayer H/CPP/Objects CPP와 WorldSequenceObject H/CPP를 확장한다. 실제 호출자는
Level_KakulSaydonArena의 입장 준비·저장 후 재로드와 Make_WorldSequenceTargets이며 통합 담당이
소유한다. PresentationPlayer와 독립 WORLD player는 전달받은 TARGET_SET의 같은 owner를 소비한다.

`TARGET_SET::objectPreparationOwner`는 호출 중에만 빌리는 Level owner다.
`Prewarm_ObjectInstances(instanceId, copies, targets)`는 기존 Prototype/Clone/Layer로 숨은 객체를
준비한다. 정상 카드 `world.object.instance.kouku.card` 6개와 조커
`world.object.instance.kouku.joker_card` 1개가 대상이다. Area/revision, modelAssetId,
preScale, animated, diffuse, source material/profile/map binding와 device/context/catalog가 일치할
때만 준비 모델을 공유한다. 실패는 false와 Get_Status 이유를 반환한다.

살아 있는 객체는 owner 포인터 대신 shared return token을 가진다. Stop/완료는 rest local bone
pose를 복원하고 풀로 반환한다. 문서 교체·Clear는 token을 비활성화하고 idle 객체를 제거하므로
예전 revision의 재생이 나중에 끝나도 새 풀에 돌아가지 않는다. 기존 6+1의 반복 재생은 clone을
계속 재사용하며, 동시에 그 이상 재생하는 경우의 추가 객체는 기존 생성·제거 경로를 사용한다.
풀은 exact objectId별로 유지하며 동일 model 입력을 가진 다른 actor와 clone 자체를 섞지 않는다.

## G04. 검증과 사용자 편집 경계

새 저장 계약은 현재 외곽불·갈고리·칼날의 실제 참조를 사용해 추가·삭제·복제·간격·Delay 변경 후
남은 index와 Collider/Logic 연결을 검사한다. 실패한 입력·외부 변경·미저장 Composition은 기존 파일과
draft를 보존하는지 확인한다. source encoding과 기존 카메라 성능 수정은 유지한다.

현재 사용자가 Visual Studio 빌드·Client 확인을 수행 중이므로 제품 빌드, EngineSDK/DLL/CSO 배포,
Client/UI 실행·조작·캡처를 하지 않는다. 최소 컴파일은 통합 담당과 조율해 실행한 범위만 RESULT에
기록한다. 구현 중 authoring JSON·Resources를 자동 저장하지 않는다. 화면 결과는 사용자가 판정한다.

## G05. 원본 시퀀스 배경의 중복 model 준비 제거

Source Sequence 배경은 MapTool의 target 중복 제약 때문에 266개 actor가 각자 objectResourceId를
가진다. 현재 `Prepare_ObjectResources`는 ID마다 `CModel::Create`를 반복한다. 실제
`WorldSequenceObject::Initialize`는 prototype을 Clone하고 CModel copy constructor는 Bone과
Animation을 각각 Clone하므로, 준비 prototype만 공유하고 재생 상태는 각 object에 유지할 수 있다.

WorldSequencePlayer의 기존 m_ObjectModels를 그대로 사용한다. 새 `Same_ObjectModelInputs`는
model asset ID, modelPreScale, animated, diffuse, material source/profile/map bindings만 비교한다.
ID·표시명·anchor·placement scale·Motion은 준비 model 입력이 아니므로 각 객체의 기존 저작 상태에
남는다. `Find_PreparedObjectModel`은 자기 cache에서 위 입력과 device/context/catalog가 같은
항목을 찾는다. `Find_SharedObjectModel`은 기존 owner 전용 의미를 유지하고 같은 Area/revision인
owner의 검색 함수를 호출한다. exact object ID를 먼저 찾은 뒤 다른 ID를 비교한다.

준비와 해당 clip 검증이 성공한 model은 owner 문서의 실제 일치 resource ID에 공유 저장해
다른 set/player의 준비가 같은 CModel을 재사용하도록 한다. owner가 다른 Area/revision이거나
해당 full input이 없으면 저장하지 않는다. 이미 존재하는 다른 owner entry를 덮어쓰지 않는다.
같은 local object ID가 다른 device/context/catalog에 준비된 상태면 reload가 필요하다는 오류를
반환해 stale GPU 입력을 사용하지 않는다. 문서 교체/Clear가 기존 cache를 비우는 수명을 유지한다.

수정은 WorldSequencePlayer.h와 WorldSequencePlayer_Objects.cpp다. 새 cache 전역이나 C++ 파일,
프로젝트 등록은 없다. G03 Joker pool의 exact object ID와 owner prototype 일치 검사를 유지한다.
실제 helper/lookup/publication 함수를 out CPU fixture에서 검사해 모든 model 입력·Area/revision·
device/context/catalog 격리, actor ID만 다른 재사용, 실패 후 기존 owner entry 보존을 확인한다.
최소 CPP 컴파일은 out에서 수행하며 제품 실행 파일을 덮어쓰지 않는다.
