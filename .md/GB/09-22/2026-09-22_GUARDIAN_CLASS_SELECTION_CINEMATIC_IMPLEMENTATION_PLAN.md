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

## G09. 2026-09-24 F1 Character Select Movie

F1에 `Character Select Movie` 구역을 추가하고 기존 7 class 목록에서 Guardian Knight를 기본 선택한다. `CLevel_CharacterSelect::Render_ClassSelectMovieControls`가 현재 Level의 동일 CClassSelectionPresentation에 Play/Restart·Stop을 전달하고 Intro/Loop 시간과 준비 상태를 표시한다. MainApp은 공통 F1 구역에서 이 좁은 UI 진입점만 호출하며 별도 preview player와 새 미디어를 만들지 않는다.

Character Select가 아니면 `Enter Character Select to preview this movie`를 표시하고 객체를 생성하지 않는다. Server 입장 승인·customizing·create character·raid-entry preview 및 Level 전환의 기존 재생 가능 조건을 유지한다. UI의 class 선택은 연출 대상만 바꾸며 Server class-change 명령을 제출하지 않는다. 현재 manifest에 없는 class는 미설정 상태와 비활성 Play를 표시한다. 기존 음향·카메라·배우/FX 수명과 Stop 정리를 그대로 사용한다.

기존 Level_CharacterSelect.cpp/.h만 확장하므로 project/filter 추가는 없다. 설치 manifest·게시 WorldSequence·Resources 참조를 읽기 전용으로 검사하고 Debug/Release 컴파일은 통합 담당자가 수행한다. Client·영상·UI를 실행하지 않으며 기존 리소스 존재 검사를 실제 화면 재생 성공으로 대신하지 않는다.

## G10. 2026-09-24 직접 관람 경로와 Stop·재Play 검증

후속 요청은 Lobby의 Server 승인 Character Select 진입부터 F1 Character Select Movie의
Guardian Knight Play, 인트로와 loop, Stop 후 플레이어 시점과 입력 복구, 다시 Play의
0초 시작까지 단계별로 확인하고 안내하는 것이다. 현재 제품 소스와 설치 데이터를 다시 읽고
과거 다른 PC의 out 검사 결과를 현재 실행 결과로 재사용하지 않는다.

현재 저장 데이터의 참조 정합성과 실제 production 수명 함수의 Stop/Restart/실패 정리
경로를 별도 검사한다. 테스트 대역을 사용한 lifecycle 검사와 실제 모델·GPU·Level Initialize
검사는 구분한다. Client/UI는 실행하지 않는다. Play 버튼 활성화와 ready/intro/loop/stopped
상태 문구를 사용자 화면 확인 지점으로 정리한다.

Follow/Free 모드는 Play가 변경하지 않으므로 플레이어 시점 복귀 확인은 Follow에서 시작해
F1 Stop 버튼으로 끝낸다. F6는 중지와 카메라 모드 토글을 함께 일으킬 수 있어 Stop과
동일한 복귀 검사로 사용하지 않는다. 정상 Stop은 준비된 scene와 pool을 보존하며, Clear는
Level 이탈용으로 구분한다. 이번 요청만으로 자동 종료나 임의 재게시를 수행하지 않는다.

## G11. 2026-09-24 선택한 바닥 category의 공통 Movie Play

F1의 static Guardian Knight 선택을 Level 수명의 movie category 선택으로 교체한다.
`Data/Rendering/Authored/CharacterSelectFloorSwap.json`의 11개 ID·label·원본 바닥 ID를
읽기 전용으로 parse/validate/stage/commit하고 실제 SL00 배치가 유일하게 존재하는지 확인한다.
7개 playable class는 대응 category와 명시적으로 연결하며 나머지 4 category는 미연결 상태다.
선택 초기값은 현재 입장 class를 따르고 제품 category 클릭, F1, Action Composition WORLD가
같은 Level 선택을 소비한다. 이 상태는 Server player class와 별개이며 class-change를 보내지 않는다.

`Client/Public/Level_CharacterSelect.h`에 movie option, 선택 조회·갱신과
`Play_ClassCinematic(classId)` 계약을 추가한다. CPP의 Play는 선택 category·class ID를 검증하고
기존 Server Arena·customizing·전환 gate를 통과한 뒤 동일 presentation owner로 재생한다.
준비되지 않은 movie도 Play 요청 자체는 가능하며 선택한 category와 실제 준비 실패 이유를
표시한다. 배경 성공 조건과 scene admission은 유지하고 ready를 위조하지 않는다.
제품 category의 준비된 movie 자동 재생과 Debug 재생 버튼도 공통 entry를 사용한다.

11개 발판·Server player·컷신 actor·camera transform은 변경하지 않는다. 현재 Guardian은
원본 SL10 전체 무대 좌표로 관람한다. 기존 H/CPP만 수정하므로 project/filter 등록 추가는 없다.
UTF-8 BOM 없음·CRLF, 선택 경로와 실패 보존, git diff --check를 확인하고 통합 담당자가
Debug/Release 최소 컴파일을 실행한다. Client/UI 실행과 최종 화면 판정은 사용자가 수행한다.

## G12. 2026-09-24 바닥 카테고리와 WORLD 선택 재생

11개 바닥의 기존 CharacterSelectFloorSwap.json stable option과 source placement를 클래스 선택의 기준으로 사용한다. F1과 Action Composition WORLD는 Level의 동일한 movie 선택과 Play 명령을 소비하며 Server의 실제 클래스 선택을 요구하거나 변경하지 않는다. 현재 설치된 무비는 가디언나이트다. 다른 카테고리에서 이를 대신 재생하지 않고 해당 카테고리의 미연결 상태를 표시한다.

이번 위치 선택은 원본 WORLD 무대다. 옮겨진 SL00 바닥은 선택 기준으로 보존하고 SL10 배경·배우·카메라·FX는 함께 원본 좌표를 사용한다. WORLD의 고정 Guardian 문구/Play/Seek는 선택 대상으로 교체하고, 현재 활성 재생의 transport와 다음 선택을 구분한다.

MapPlacementRuntime은 stage 실패에서 구체적인 asset/source placement와 실패 단계를 보존한다. 실제 설치 모델·prototype·배치 소비자를 창 없는 검사로 확인하고 실패 원인을 수정한 뒤 정상 Product Build를 수행한다. 실행 중 Client는 에이전트가 종료하지 않으며 실제 화면 Play는 사용자 확인으로 남긴다. 기존 파일만 수정하므로 project/filter 추가는 없다.

## G13. 2026-09-25 scene별 배경 Area와 실패 격리

manifest scene의 optional `backgroundAreaId`를 읽고 누락 시 Level registry의 기존 SL10
presentation Area를 사용한다. 공통 parser가 반환한 고유 Area 목록을 Level이 기존
CMapPlacementRuntime의 Area별 owner로 준비하고 숨긴다. 같은 Area는 한 번만 준비하며
primary SL00와 같은 Area는 이미 로드된 m_MapRuntime을 공유하여 별도 배치를 만들지 않는다.

한 배경의 Load_Area 실패는 그 Area에만 보존하고 presentation Initialize의 공통 실패로
승격하지 않는다. Play는 선택 class의 admitted scene을 확인한 뒤 그 scene의 배경 준비만
검증한다. 활성 scene의 배경만 표시하고 Stop·Level 종료에서는 cinematic 배경만 숨기거나
정리한다. primary gameplay map은 cinematic Stop의 숨김 대상에 넣지 않는다.

Level_CharacterSelect CPP/H가 Area별 runtime·실패·가시성을 소유한다. Loader의 Area별
prototype 준비와 manifest parser·조회 API는 기존 담당자가 같은 변경 단위에서 연결한다.
원본 네 class의 배경이 아직 설치되지 않았으면 그 class 요청만 명시 실패하며 기존 Guardian은
자기 배경으로 재생 가능 조건을 유지한다. 기존 Resources·원본 WorldSequence는 변경하지 않는다.
현재 함수 본문·대역으로 배경 실패 격리, optional fallback, 같은 Area 공유, 재생 교체·Stop
가시성을 확인하고 실제 배경 모델·전체 연출 검증과 Product 빌드는 통합 담당자가 수행한다.

## G14. 2026-09-25 움직이는 PSC root와 기존 음성 소비자

후속 네 class의 moving/bone PSC는 기존 Effect world-root owner를 사용한다. effects[].rootKeys는
optional이며 phase source time의 WORLD meter TRS를 저장한다. position/scale 선형 보간과
정규화 quaternion shortest slerp를 사용하고 runtime에서 좌표 변환을 다시 하지 않는다.
기존 rootWorld만 가진 Guardian 입력은 기존 고정 root 경로를 유지한다.

particle age를 받는 기존 fixed-step transform provider는 같은 PSC의 intro와 loop clock을
역으로 조회해 과거 입자 생성 위치를 복원한다. 정지 age 중 본이 움직이는 경우에는 과거
simulation root와 현재 표시 root를 구분하고, 기존 Seek_WorldRoot의 optional 최종 root 값으로
history commit 뒤 Update(0) frame만 갱신한다. 이전 caller는 이 선택 값을 전달하지 않는다.
loopAgeDeltaMs는 생략 시 기존 clock span, 명시하면 그 값을 사용한다. 연속 양수 증분은
clock span과 일치해야 하며 0은 정지 또는 매 loop rewind epoch를 명시한다. visibility만으로
age를 0으로 바꾸지 않는다. 음수 원본 phase key는 producer가 0시점 선행 상태로 평가해 저장한다.
원본에 phase당39 PSC가 있으므로 Effect occurrence 상한을128로 조정하되 문서 크기 상한은 유지한다.

음성은 phase당 한 carrier template의 기존 WorldSequence soundTracks를 사용한다. 별도
ClassSelection audio 문서나 재생기를 만들지 않는다. staged player는 첫 Play 전에 pause하고,
성공 commit 뒤 Play/Seek의 목표 pause를 적용한다. Set_Paused는 실제 WORLD player의 소유
sound handle에도 전달한다. 비선형 phase clock의 PCM rate와 phase 밖으로 남는 음성 tail은
기존 계약을 먼저 확인하며 지원하지 않는 동작을 복원됐다고 표시하지 않는다.

ClassSelectionPresentation CPP/H와 Effect_PresentationService CPP/H의 기존 경로만 확장한다.
새 C++ 파일과 project/filter 등록은 없다. 초기화·clock 역매핑·epoch·plateau·최종 root·audio
pause/실패 보존을 실제 함수 기반 검사로 확인하고 정상 Debug Product와 Guardian 실제 native
재생 회귀를 수행한다. 네 class 데이터 설치·실제 GPU 표시·음성 청취는 별도 완료 경계다.


LIGHT_KEY의 optional radiusMeters는 track.radiusMeters를 상속하고 같은 phase source-time
보간으로 기존 transient light range에 전달한다. finite positive·기존 상한 검증을 유지한다.


## G15. 2026-09-25 원본 PSC parameter의 시간별 color·alpha 입력

원본 Fighter Matinee 43 binding은 color35·alpha8이며 source module/property/distribution과
PSC instance를 확인했다. ClassSelection effects[].parameterTracks는 phase source time의
scalar/vector3 raw curve를 소유한다. timeMs는 소수 millisecond, value·arriveTangent·leaveTangent는
componentCount1/3 배열, interpolation은 CONSTANT/LINEAR/CUBIC이다. 원본 tangent value/sec와
구간 seconds로 Hermite를 평가한 뒤 source DistributionParticleParameter의 DPM을 적용한다.
이 곡선을 particle-relative-life의 distribution.keys에 복사하지 않는다.

Effect_Distribution CPP/H와 기존 source codec에 WORLD_SAMPLE binding과 explicit
parameterMapping(modes/minInput/maxInput/minOutput/maxOutput)을 추가한다. 실제 필요한
DIRECT/NORMAL만 허용하며 이름은 ASCII case-insensitive, 입력은 typed scalar/vector3다.
기존 NONE/ACTION_CUE와 default 상수 계약은 유지한다. mapping·typed 값·이름 중복과 누락을
검증하고 실패는 output 및 이전 재생 상태를 보존한다.

Effect_Playback의 기존 fixed-step sample에 per-instance ParticleParameters를 포함한다.
과거 샘플 전체를 먼저 검증한 뒤 Step에 적용하고, 현재 표시 parameter는 별도로 전달한다.
intro/loop/held-age의 과거 phase 조회는 root와 같은 helper를 사용한다. held-age 중 color는
spawn base와 실제 update module 순서를 보존한 최종 frame projection으로 바꾸어 반복 곱셈이나
shared Effect definition 변이를 피한다. reconstructed admission이 binding을 상수화하지 않는지
실제 대상 module부터 확인한다.

ClassSelectionPresentation은 curve parse와 raw 보간·history sample을, Effect_PresentationService와
Effect_Object는 final current-frame typed input의 사전 검증 및 기존 owner 전달을 담당한다.
새 런타임이나 C++ 파일은 만들지 않으므로 project/filter 등록 추가가 없다. 실패 원자성,
정지 age 변화, 원본 cubic→DPM 순서와 실제 particle Color 최종 소비자를 검증한다.
정규 Debug Product 및 Guardian 회귀와 신규 source fixture를 실행하고 UI/GPU draw는 별도 판정한다.


실제 movie importer는 format13 direct-authored Effect를 만든다. `bSourceContract`는 일반 source
recipe 유무가 아니라 native-v14 증거 계약 여부다. format13 source ParticleParameter에서
명시적 worldSample/name/mapping만 읽기·문서 validation·저장에 연결하며 referenceId 등의
native-v14 증거 필드는 계속 거부한다. 전체 문서의 Load→Serialize→Parse와 실제 EffectObject
frame까지 검사하여 개별 distribution helper 검사로 이 경계를 대신하지 않는다.
