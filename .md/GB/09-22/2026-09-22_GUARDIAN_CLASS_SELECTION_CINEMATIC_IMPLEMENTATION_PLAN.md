# 가디언나이트 클래스 선택 연출 구현 계획

## G00. 원본과 현재 기준점

사용자는 원작의 클래스 선택 화면처럼 등장, 시간 정지에 가까운 동작, 떠오르는 파편,
카메라 이동이 이어지는 반복 연출의 구현을 요청했다. 첫 대상은 가디언나이트다.
첨부한 차원술사와 도화가 화면은 같은 기능의 후속 비교 자료다.

원본 `LV_LOBBY_CLASSSELECT_SCENE01`의 `ChangeClass9`는 Matinee75(export702)를
재생한다. 길이는31.505365초이며 종료 후 Matinee74(export701)의23.002918초 반복으로
이어진다. 배경은 `LV_LOBBY_CLASSSELECT_SL10`이다. SL00의11개 검은 원판/팔각별은
별도의 정지 캐릭터 미리보기 자리다. 사용자가 근처로 옮긴22배치의 위치·재질은 보존한다.
이 검은 표면을 밝은 중앙 무대 장식의 복원 대상으로 취급하지 않는다.

본체와 용의 소개용 기본 clip은 설치돼 있다. 컷신 배우는 Shadow body, wing, HR00 상하의,
DK02 face, DL56 hair, prop06, b_wp_1의 HR00 weapon이다. 일반 PCPreview702 의상후보와
같은 조합이 아니므로 후보 전체를 대신 연결하지 않는다. 현재 active catalog 연결과
Resources 후보 존재, source shader 준비, 실제 소비자 성공을 각각 확인한다.

시작 브랜치는 `GB/collider-pattern-bug-fix`이며 기존 코드·맵·재질의 미커밋 변경이 많다.
변경 직전 bytes를 보존하고 이 기능의 추가 경계만 반영한다. Client/UI 실행과 화면 캡처는
하지 않으며 첨부 이미지 분석과 사용자의 최종 화면 판정을 구분한다.

## G01. 원본 데이터와 재사용 경로

원본 actor/component/attachment, 카메라 Director cut·FOV·roll, animation의 실제 clip 시간,
visibility, particle toggle/time scale, material/light 곡선과 sound를 같은 source graph에서
회수한다. UE 좌표계와 cm 단위는 기존 map/model 변환 경로로 한 번만 변환한다.
카메라 곡선과 정지한 배우의 애니메이션 시계를 같은 속도로 진행한다고 가정하지 않는다.

SL10 정적 배경은 기존 LevelPlacementExtractor, source material compiler,
ModelAssetConverter와 Map publisher를 사용한다. 원본 visibility와 geometry/material
미지원을 구분한다. 현재 SL00 authoring은 최신 stable ID 기준으로 병합하며 기존804배치를
덮어쓰지 않는다. 새 리소스는 Resources 상대 ID로 연결하고 GBResources에도 전달한다.

배우·파편·소리는 기존 `CWorldSequenceDocument -> CWorldSequencePlayer ->
CWorldSequenceObject -> CModel/CMaterial` 경로로 재생한다. 독립 PSC의 별도 particle age는
기존 EffectPresentationService world-root 경로로 전달한다. 원본 반복 구간과 등장 구간의
자원은 공유하되 clock과 시작 상태는 구분한다. camera 값은 기존 `CEffectRecoveryCamera`
parser/sampler와 `CCamera_Free`의 presentation owner를 사용한다. 필요해진 source 시간·곡선
입력은 기존 문서/소비자에 추가하며 별도의 모델·파티클 렌더러를 만들지 않는다.

## G02. 클래스 선택의 연출 소유자

새 `Client/Public/ClassSelectionPresentation.h`와
`Client/Private/ClassSelectionPresentation.cpp`는 클래스별 연출 문서, 등장→반복 상태,
기존 WorldSequence player와 camera owner의 수명을 소유한다. gameplay authority, socket,
플레이어 위치와 skill command는 소유하지 않는다. 원본 캐릭터를 보여주는 presentation
actor와 Server의 실제 플레이어를 구분한다.

`CLevel_CharacterSelect`의 클래스 category 선택에서 연출을 요청하고 준비 성공 후 camera와
장면 표시를 인계한다. subclass thumbnail의 실제 class 변경은 기존 typed command 경계를
유지한다. 다른 클래스, 미리보기 종료, customization, Level 이탈·disconnect에서 자기 연출과
camera만 정리한다. 재생 중 gameplay mouse/key command는 제출하지 않으며 원복 후 기존
Server Arena의 조작을 되돌린다. 도구에서 중지·재시작할 수 있게 상태와 명령을 제공한다.

Loader의 기존 Area/model/Effect preparation과 연결하며 누락·손상된 선택 연출은 기존
선택 화면과 gameplay를 보존한 채 상태를 표시한다. 새 C++ 파일은 Client.vcxproj와
Client.vcxproj.filters의 물리 폴더에 맞게 등록한다. 새 authoring JSON은96.DataFiles None
항목과 기존 domain publisher의 검증·게시 경계에 포함한다.

## G03. 재질과 배경·연출 입력의 소비

정적 배경은 배치별 원본 RNM/environment와 named material을 유지한다. 컷신 부품은 정확한
source mesh/MIC 및 animation donor를 결합하고 실제 skeleton/rest/preScale을 대조한다.
얼굴·눈·머리·뿔·무기의 누락을 일반 의상 native shader 준비 성공으로 대신하지 않는다.
원본 material/light 곡선은 기존 Shader/Material/MapLight 입력으로 연결하고 종료 시 원복한다.

새로 확인한11바닥의 미리보기 용도와 소개무대의 분리, 원본 검정 재질의 의미는
gotchas.md와 렌더링이펙트복원V2.md에 기록한다. 제품의 새 선택 연출 데이터 계약은
AREA_DATA_LAYER_GUIDE와 팀 gameplay interface 문서에만 반영한다.

## G04. 검증과 완료 구분

원본→후보의 actor/mesh/MIC/clip/track 연결과 카메라 cut·정지·반복 경계 시점을 검사한다.
실제 CModel과 기존 WorldSequence consumer로 모든 사용 리소스와 clip을 admission하고,
반복 재시작·빠른 클래스 교체·실패 rollback·Level cleanup에서 자원과 camera owner가 남지
않는지 확인한다. 변경한 JSON/XML parse, 최소 C++/shader 컴파일, 정식 domain publish/check,
필요한 Product Build와 git diff --check를 실행한다.

RESULT에는 source 조사, 후보 생성, 설치·게시, 소비자 검사, Product build와 사용자 화면
판정을 분리한다. 사용자 화면 확인 전 원작과 전체 시각 동등성을 완료로 기록하지 않는다.

## G05. 사용자 마무리 요청 시의 남은 목표

조사·코드·후보 검증을 닫고 제품 연출 데이터는 활성화하지 않는다. 다음 구현 대상은 동적
소품18개, SL10 정점 광원·바람·환경, 음향·camera shake·DOF·Bloom이며, 실제 설치·게시 전
GPU consumer 검증과 Product Build가 필요하다. 완료와 미완료의 정본은 대응 RESULT다.

## G06. 재생 가능 상태로 설치·연결하는 후속 요청

이전 G05의 비활성 종료 경계는 사용자의 재생 가능하게 마무리하라는 후속 요청으로 대체한다.
SL10 배경·배우 donor·4개 이펙트와 클래스 연출 manifest를 설치하고 정식 Map publisher를
통해 게시한다. 11개 SL00 미리보기 바닥의 현재 배치와 Server player 위치는 유지한다.
연출은 SL10 원본 절대 좌표로 재생하고 camera가 그 무대로 이동한다.

Action Workbench에 WORLD session을 추가해 동일 Level 연출 소유자에 Play/Restart,
Pause/Resume, Stop, Intro/Loop 구간 탐색을 위임한다. 등장→반복 전환용 모델 clone은
초기 준비하고 반복 중에는 player·이펙트 handle을 재사용한다. 일반 진행의 visual clock은
로딩·디버거 정지분을 한 프레임에 추격하지 않으며 Server simulation 시간에는 관여하지 않는다.

원본 body 0.4배속 후 끝 포즈, dragon 끝 포즈, ash 0.5배속, swing 0배속, water Lifetime=0과
계속 이동하는 camera를 각각의 source clock으로 재현한다. 반복 경계의 float 반올림으로
지속 파티클이 소멸하지 않도록 다음 구간까지 lifetime horizon을 확보한다. 최소 컴파일,
설치 데이터 실제 parser·consumer 검사, 정식 Area/WorldSequences publish/check와 Product
build를 확인하고, 전체 원작 동등성과 사용자 화면 확인은 별도로 기록한다.

G06의 실제 CModel 검증에서 발견된 donor WANM 총 key 상한 초과는 packed-float 값이 같은
TRS run 내부 키만 제거해 해결한다. 원본 key와 보간 곡선·구간 끝·skeleton hash를 보존하고
Engine 상한은 유지한다. hard-attachment 무기의 skinned type은 AnimControl 존재와 별개이므로
원본에 없는 bone clip track은 만들지 않고 기존 WORLD socket transform과 rest palette를 쓴다.

## G07. 사용자 실행에서 Play 비활성 재현 수정

실제 사용자 화면의 Class selection cinematics are not loaded 상태는 SL10 배경 준비 실패로
controller Initialize가 호출되지 않은 경로다. WARP의 실제 Loader 입력으로 SL10을 재현하고
CModel/Create_MaterialVariant 전체574개를 검증한다. native214..234/237+RNM은 Catalog와
CModel·Bind_SourceCharacterInputs에서 이미 지원하지만 CMaterial 초기 admission만 누락됐으므로
그 동일 범위를 반영한다. 조명 입력을 제거하거나 실패 asset을 숨겨 우회하지 않는다.
새 Engine DLL은 별도 위치에서 빌드·검증하고, 실행 중 Client가 점유한 DLL의 배포는 사용자의
종료 뒤 완료한다. 에이전트는 Client를 자동으로 종료하거나 UI를 조작하지 않는다.

## G08. 실제 연출 초기화와 Play 경계 검증

새 Engine DLL을 설치한 뒤에도 사용자 Play 실패가 계속됐다. 배경 모델 검증을 전체 연출
초기화 성공으로 대신하지 않고 실제 MapPlacementRuntime의 Layer 생성, WorldSequence의
Prepare_AreaLoad/Load_PreparedArea, ClassSelectionPresentation Initialize/Play를 이어 검사한다.

WorldSequence는 Area의 optional Deploy 두 문서가 모두 없으면 빈 대상 집합으로 준비한다.
한 문서만 있거나 손상된 문서는 계속 거부하고, Deploy가 없는 Area의 DEPLOY_PLACEMENT
binding도 기존 문서 검증에서 거부한다. Character Select에 가짜 빈 Deploy 파일을 만들지 않는다.
Level은 manifest·배경 준비 실패를 보존하고 Workbench와 F1이 같은 원인을 표시한다.

현재 실행 파일을 점유한 Client는 유지한 채 후보 EXE를 별도 출력에 빌드한다. 후보 검사 뒤
실제 파일 교체가 필요한 시점에만 종료를 안내한다. headless 실제 객체·재생 소비자 검사와
사용자의 화면 확인을 RESULT에 구분한다.
