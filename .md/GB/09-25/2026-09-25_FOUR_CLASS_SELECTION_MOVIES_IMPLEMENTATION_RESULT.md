# 다섯 클래스 선택 무비와 Workbench 결과

## G00. 현재 설치 상태와 완료 경계

가디언 나이트·도화가·워로드·차원술사·창술사를 `Data/Camera/ClassSelection.cinematics.json`에
등록했다. F1과 Action Workbench WORLD는 같은 Level의 category 선택·Play·Stop·Seek를 사용한다.
category 목록은 요청한 다섯 개만 표시한다. SL00에 배치된 다른 원본 바닥은 선택 목록과 별개다.

네 신규 클래스의 배우·소품·재질·애니메이션·사운드·Effect를 설치했고, SL01/03/08/12 배경과
SL00 WorldSequences를 공식 Map publisher로 게시했다. Debug/Release Product 빌드와 실제 다섯 클래스의
공통 Play 경로 검증을 통과했다. Client/UI 자동 실행이나 사용자 화면 판정은 하지 않았다.

## G01. 일곱 번째 Guardian Play 실패의 원인

`CLIENT_LEVEL_DESCRIPTOR::LOAD_FUNCTION`이 전방 선언된 `CLoader`의 멤버 함수 포인터였고,
MSVC의 include 순서에 따라 크기가 24/8 bytes로 달라졌다. background 필드 offset도152/136으로
달라져 Loader가 SL10 prototype 준비를 건너뛰었고, runtime placement1349의 첫 모델 clone이
실패했다. 리소스 재추출이나 GK 오타의 문제가 아니다.

`HRESULT (*)(CLoader&)` 일반 함수 포인터와 일곱 captureless callback으로 바꿨다. 실제 Loader와
Registry 번역 단위의 layout 대조는 `completion/loader/layout_before.log`, `layout_after.log`에 있다.
기존 Guardian 단독 probe는 실제 SL00 804배치/213batch, SL10 1356배치/400batch를 만든 뒤
Initialize→intro/loop→Stop→재Play를 통과했다. 최종 다섯 클래스 검증은 별도로 아래에 기록한다.

## G02. 원본과 설치된 네 클래스

정본은 SCENE01 lossless extraction12698exports/1197imports, parseErrors0이다. 원본 tagged
Matinee·AnimTree·actor/component·MIC·PSC closure를 사용하며 플레이 캐릭터 외형으로 대체하지 않는다.

| 클래스 | 원본 intro/loop export0 | 배경 | 배우 | World Object resources |
|---|---|---|---:|---:|
| 도화가 |699/692|SL08|32|58|
| 차원술사 |700/698|SL12|33|56|
| 창술사 |690/691|SL03|22|27|
| 워로드 |689/688|SL01|53|67|

네 클래스 합계208resources/416instances,76Effect 문서를 연결했다. 창술사의 원본 양손 소품
748/749는 실제 hand bone과 reference palette로 복구했고, 차원술사의 primary transparent
palette3개를 actor 번호만으로 숨기던 후보 가정을 제거했다. 원본 reference pose, 부모 bone basis,
corner normal/tangent handedness/UV1/UV2와 Material slot을 보존한다.

배경은 SL01 436배치/166assets, SL03 509/137, SL08 374/141, SL12 1461/407이다. native background
1100..1166/1400..1413, native prop1500..1525, 해당 actor program과 forward/deferred/RNM 입력을
기존 CModel→CMaterial 경로에 연결했다. SL03 양손 소품은 정확히 같은 원본 PS·CPU packing인
기존 program14를 재사용한다. source NULL override의 texture fallback은 원본 cached texture
expression의 유일한 parameterName 대응으로만 해석한다.

원본 LookInfo mask는 선택한 native packer가 실제 소비하는 parameter에만 적용한다. 소비하지
않는 cutting_mask를 모든 slot에 강제로 넣던 후보 데이터를 교정했다. header의 한글 parameter
이름을 C++ octal UTF-8로 저장할 때 publisher도 이를 decode한 실제 이름으로 검사한다.

도화가 reflection과 워로드 plane의 음수 scaleMultiplier는 원본대로 유지한다. 각 축은 nonzero이고
track 전체에서 부호가 같아야 한다. World Object의 음수 determinant는 기존 rasterizer의 front
winding만 뒤집고 draw 뒤 복원한다. two-sided 상태는 유지한다. MAP/DEPLOY binding은 계속 양수
multiplier만 허용하며 기존 object scale·preScale의 양수 계약도 유지한다.

## G03. 리소스 전달과 게시 증거

추가 네 클래스가 실제 참조하는 리소스4062개,1805173244bytes를 runtime Resources와
`C:/Users/user/Desktop/GBResources`에 설치했다. 원본 UPK/LPK와 추출 캐시는 전달하지 않았다.
이전에 전달한 자기 후보 파일92개는 이전 영수증 SHA 일치를 확인한 뒤 백업·CAS 교체했다.
사용자나 다른 작업의 내용이 다른 파일은 이 절차로 덮어쓰지 않는다.

증거의 공통 루트는 `out/ClassMovies20260925/completion/`이다.

- `delivery/resource-closure.json`: Resources-relative 경로·원본 경로·bytes·SHA256.
- `delivery/reconciliation.json`, `delivery/reconciliation-backups/`: 자기 후보92개 교체 증거.
- `delivery/Installation/receipt.json`: 최초 설치5592개/동일2624개. 후속 parameter 수정은
  `material-parameter-reconciliation.json`, `material-parameter-backups/`에서 별도 보존한다.
- `publish-LV_LOBBY_CLASSSELECT_SL01.log`, `SL03.log`, `SL08.log`, `SL12.log`에 각 Area 게시 결과.
- `publish-worldsequences.log`: SL00 WorldSequences 게시 결과.
- `movie-publisher-tests.log`: 실제 publisher의 한글 native parameter 이름과 signed scale의
  nonzero/axis-sign 검증 회귀 통과. `program-cohort-tests.log`: sparse RNM helper 삽입 회귀4개 통과.

## G04. Movie 시간과 Action Workbench

새 Engine timer를 추가하지 않는다. `CClassSelectionPresentation`의 movie clock이 진행량을
소유하고, 원본 slomo를 포함한 phase clock mapping으로 source track time을 얻는다.
사용자 배속0.05..2.0은 movie clock의 진행량에 곱한다. Server/GameRoom 시간은 변경하지 않는다.

`ClassSelectionTimeline.h`는 이미 로드·검증된 document의 읽기 전용 투영이다. stable row/box ID와
movie/source 시작·끝·key 시간을 가지며 phase별 immutable cache로 소유한다. World Model,
Animation, Camera, Effect, Material, Light, Sound, Time Control을 공통 CompositionTimeline의
눈금·box로 표시한다. 필터·줌·접기·선택·구간 탐색과 intro/loop 전환을 제공한다.

카메라 Box Detail은 저장 key의 eye/look·원본 horizontal/vertical FOV·local/source/movie 시간을
보여준다. Applied sample은 실제 camera submission 성공 후에만 보관한 보간 eye/look/up,
viewport aspect와 변환된 vertical FOV다. 선택 row와 실제 적용 row가 다르면 그 사실을 표시한다.
단위와 source→movie mapping 차이를 설명하지 않고 두 값을 동일한 숫자로 덮어쓰지 않는다.

pause/seek/restart/배속은 F1과 Workbench가 같은 presentation owner를 사용한다. source와 movie
배속을 FMOD cue pitch에도 전달한다. 현재 row/카메라 값은 조회·탐색용이며 원본 key를 편집하고
저장하는 authoring 기능으로 설명하지 않는다.

## G05. 빌드·런타임 검증

최종 Debug Product 빌드·설치는 `out/BuildPipeline/runs/20260925T025852791Z-debug-product.json`으로 통과했다.
`completion/loader/build.py --refresh-archive --run`은 최신 실제 Client OBJ와 Engine DLL을 연결하고
창 없이 WARP에서 실제 LevelRegistry/Loader/CModel/CEffectObject/CClassSelectionPresentation을 사용했다.
mock movie player나 별도 모델 런타임을 사용하지 않았다.

- 실제 배경 여섯 Area의 prototype·placement 준비와 World Object 460개 생성: 실패0.
- 실제 이펙트60개 준비 및 각 EffectObject 인스턴스 초기화: 실패0.
- 실제 CLevel_CharacterSelect의 다섯 category 선택→Play_ClassCinematic: 다섯 클래스 모두 성공.
- 클래스마다 intro 전체와 loop 두 주기 이상, Stop→재Play, intro/loop/end Seek, Pause,
  0.25/2배속, 잘못된 배속 거부 및 기존 값 보존, camera override·Effect·audio handle 정리 성공.
- row/box stable ID, phase별 cache, camera key source↔movie 시간 round-trip, 실제 camera submission
  sample의 clock/FOV, model/particle transform의 finite 상태를 검사했다.

| 클래스 | 모델 sample | Effect occurrence sample | particle sample |
|---|---:|---:|---:|
| 가디언 나이트 |6741|1214|107173|
| 도화가 |18084|2744|49964|
| 차원술사 |12874|1166|333538|
| 창술사 |13149|9200|347267|
| 워로드 |41644|5309|809787|

`completion/loader/run.log`가 위 재생 증거다. UI 클릭, swapchain draw, 원작 화면 동등성이나
실제 소리 청취의 증거는 아니다. native fixture는 필요한 offscreen scene/bloom target을 소유하고
FMOD master를 음소거했다. F1/Workbench가 호출하는 같은 Level Play 함수를 직접 사용했다.

Character Select 입장 승인 자체는 별도 실제 Server/network Core fixture로 검사했다.
`character-admission-run.log`: private Arena 승인·세션 격리·재입장·Bern 전송, failures0.
movie fixture의 SERVER_ARENA 상태 설정만으로 socket 승인을 검사했다고 설명하지 않는다.

`particles/run.log`: 원본 창술사의 atlas tile 3/2/1 실제 평가, scalar SubUVSelect 거부,
실제 PhysX 접촉 후 FreezeRotation의 sprite/mesh 회전 정지와 이동·수명 유지, 원본 도화가의
수신자 없는 event generator 보존·초기화, 연결된 self-cycle 거부를 통과했다.
`movie-admission-tests-final.log`: phase clock 상한·보존과 잘못된 DDS container 거부3개 통과.
`structure-validation-final.log`: 변경 JSON/XML120개 parse 및 git diff --check exit0.

쿠크는 `kouku/run.log`에서 실제 Complete Play 준비 함수의 revision2348 전체115패턴,
5boss/157V1/34V2/340WORLD 준비를 통과했다. `kouku-raid-final.log`와 `kouku-bingo-final.log`는
실제 Server의 관문 진행·빙고 자동 입장·독립 prefix/반복 tail·세 번째/여섯 번째 표식 interrupt와
특수 parent 복귀, 12초 폭탄, 가로/세로3줄 판정, 30초 무적, 성공13줄 피해·실패 전멸을 통과했다.
Client 리소스 준비와 Server 권위 진행은 별도 검증이며 GUI에서 연속 플레이한 화면 기록은 아니다.

## G06. 원작 화면과 비교하는 범위

추출·설치·재생 계약과 원작의 pixel 단위 화면 동등성은 구분한다. 별도 의상/머리카락 물리,
원작 환경 조명과 후처리(DOF/bloom/camera shake 포함)는 원본 기능별 잔여 경계다. 원본에 없는
임의 clip이나 다른 class 모델로 대체하지 않았다. 실제 Client 화면은 사용자가 확인한다.


## G07. 최종 리소스와 추가 admission 수정

최종 전달 범위는 다섯 클래스와 쿠크 요청 리소스를 합친8142개,4823898439bytes다.
`requested-resource-delivery.json`에 Resources-relative 경로·SHA256을 기록했고
`resource-verification-final.log`의 재검사에서 missing0/conflicts0/errors0을 확인했다.
기존4062개 영수증은 첫 네 클래스 설치 이력이며 최종 전달 범위는 이 항목이 우선한다.
GBResources는 같은 상대 경로를 유지한다. EXE/DLL과 Data/DataFiles는 이 리소스 묶음의 대체물이 아니다.

- SL12의 TGA bytes/.dds 이름 불일치3개를 실제 픽셀 일치 DDS로 변환했다. installer도 magic을
  검사한다. `texture-container-repair.json`과 backup이 원본·설치·GB 복사본의 증거다.
- 이미 두 clip을 포함하는 모델을 자신의 animationSet으로 다시 연결하던60개 인스턴스만
  실제 WANM/clip 대조 후 자기 donor 참조를 제거했다. SL00 WorldSequences를 재게시했다.
  generator도 명시적 내장 clips가 확인된 같은 asset 참조만 생략한다.
- 창술사 FT 눈의150vertices는 원본UV1과 기존 ocular UV clamp 규칙을 대조해 UV2를 구성했다.
  다른 section·basis·skin은 보존했다. `lance-eye-uv-repair.json`에 검증·교체를 기록했다.
- CModel은 원본 constant-only native1510/1512의 mask0을 exact program 범위로 허용한다.
- 원본 MeshMaterial의 단일 flower 재질을 sourceMaterialSlots로 전달하고 Required의 빈 재질을
  실제 원본대로 유지했다. WORLD_SAMPLE의 비활성 lookup metadata도 정규화했다.
- SubUVSelect와 FreezeRotation은 기존 portable execution 경로에 연결했다. 도화가의 수신자 없는
  원본 이벤트는 bounded no-op으로 보존하고 연결된 이벤트의 순환 거부·queue 상한은 유지했다.
  codec와 runtime은 mesh sourceMaterialSlots까지 같은 element 실행 판정을 사용한다.

## G08. Release 최종 실행 검증

Release Product compile/deploy는 `out/BuildPipeline/runs/20260925T033407355Z-release-product.json`으로
통과했다. Client EXE/DLL과89개 셰이더 산출물을 갱신했다. 최종 Client.exe는
`Client/Bin/Release/Client.exe`이며 Debug도 `Client/Bin/Debug/Client.exe`에 갱신돼 있다.

`completion/release-loader/build.py --link-only --refresh-archive --run`의 Release OBJ/DLL/CSO를
연결한 native fixture에 실제 게시 `Client/Bin/DataFiles`를 DataFiles junction으로 연결하고
`release-loader/run_only.py`로 실행했다. Debug의 개발 경로 fallback에 기대지 않는다.
`completion/release-loader/run.log`에서 다섯 클래스 모두 실제 Level의 category/Play, intro와
두 loop 주기, Seek/Pause/0.25·2배속, Stop과 재Play를 통과했다. 클래스별 sample 수는 G05와 같다.

새 Release Server도 `--kouku-raid-contract-test`, `--bingo-contract-test`,
`--kouku-object-overlap-contract-test`를 모두 exit0으로 통과했다. 각각
`kouku-raid-release-final.log`, `kouku-bingo-release-final.log`, `kouku-overlap-release-final.log`에 있다.
Client/UI 클릭·실제 화면과 소리 판정은 수행하지 않았으며 G06의 화면 확인 경계를 유지한다.
