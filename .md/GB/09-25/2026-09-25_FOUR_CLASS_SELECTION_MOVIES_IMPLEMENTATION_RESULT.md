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


## G09. Native 재질 frame registry 제한 수정과 검증 범위 정정

사용자가 보고한 GuardianKnight 시작 즉시 중단과 DimensionMaster 약4.67초 중단의 오류는
`World Object render failed: material binding (mesh 0)`였다. 대상은 각각
`world.object.classselect.guardianknight.a12265.p0`의 얼굴 native200과
`world.object.classselect.dimensionmaster.a740.p0`의 등장 무기 native902다.
실제 설치 CModel과 shader로 두 재질을 단독 바인딩하면 diffuse·native·전체 deferred 입력이
모두 성공했다. base texture mask는 각각2047과63으로, 이 검사는 누락 texture를 원인으로
판정한 결과가 아니다.

`Engine/Private/Material.cpp`의 기존 `CMaterial::Bind_SourceCharacter`는 한 frame에 고유
재질256개를 등록하면 다음 재질을 `E_BOUNDS(0x8000000B)`로 거절했다. 실제 Guardian 재질의
prototype1개와 고유 clone255개까지 성공하고, 다음 clone인 전체257번째에서 이 실패를
재현했다. `before-cap.log`의 `material=256`은 prototype을 제외한 clone 순번이다.

수정은 같은 frame registry를 유지하면서 pointer→row `unordered_map`으로 반복 선형 탐색을
제거한다. 재질 입력 바인딩이 성공한 뒤에만 row를 등록하고 frame reset에서 vector와 map을
함께 비운다. row는 `Target_Depth`의 `R32G32B32A32_FLOAT` z성분으로 전달되므로256이
저장 형식의 상한은 아니다. 새 상한은 float32가 연속 정수를 정확히 표현하는2^24이며,
조명 단계는 기존 `Bind_SourceCharacterLight`에서 같은1-based row를 사용한다.
`Client/Private/WorldSequenceObject.cpp`는 실패한 HRESULT·native program·재질명을 함께
남긴다. 재질·조명·후처리 옵션과 기존 피격·호버 변경은 이 수정에서 바꾸지 않았다.

격리 검증의 공통 경로는 `out/ClassMovieMaterialFix20260926/`이다. 이 경로명의 날짜는
실제 산출물 이름이며, 다른 검증 디렉터리로 치환하지 않는다.

- `before-probe.cpp`, `before-cap.log`: 설치 Debug Engine.dll에서 전체257번째 등록 실패 재현.
- `probe.cpp`, `build.py`, `after-cap.log`: 현재 `Material.cpp`를 격리 probe에 컴파일하여
  실제 Guardian200·Dimension902 재질에서 각각512개 고유 clone을 등록했다. 반복 조회 뒤
  count512 유지, base/light의 row1·256·257·512 `GetRawValue` 일치, 설치 Deferred shader의
  해당 cohort pass19 `Begin` 성공, 없는513번째 light row 거절, reset 뒤 row1 재시작을
  확인했다. 최종 실행 exit0이다. 변경 `Material.cpp`와 `WorldSequenceObject.cpp`의 개별
  C++ 컴파일 및 해당 파일 `git diff --check`도 성공했다.
- `result.json`: 위 범위·mask·결과와 검증 시점 두 제품 소스의 SHA256을 기록했다.

이512개 검사는 실제 설치 재질을 복제한 registry·shader 바인딩 검사이며 GPU draw나
실제 배경의 고유 재질 수·성능 측정이 아니다. pass `Begin` 성공을 픽셀 출력이나 최종 조명
누적 성공으로 기록하지 않는다. row별 조명 제출 비용과 같은 payload 공유 여부는 별도
판정 범위다.

G05와G08의 Movie lifecycle/sample PASS는 실제 `CWorldSequenceObject::Render`를 호출한
증거가 아니었다. 당시 fixture의 마지막 로그도 `no window, UI, draw or visual claim`이다.
따라서 그 결과를 배경과 배우의 재질 바인딩·전체 draw 성공으로 해석하면 안 된다. 당시
Product build와 sample/Seek/Pause 검증 자체는 유지하되 이번 draw 실패 검출과 구분한다.

이 항목 기록 시점에는 실행 중인 Debug Client가 있어 수정 후 Product 빌드·설치를 하지
않았다. 배경을 포함하는 전체 Movie draw fixture는 진행 중이며 아직 결과를 판정하지
않았다. 편집기 후속 구현과 전체 draw 최종 결과는 후속 항목에 기록한다. Client/UI 실행,
스크린샷 생성, 사용자 화면·소리 판정은 수행하지 않았다.


## G10. All Effects에서 같은 World 무비 편집기로 연결

`All Effects → World → Character Selection Movies`는 현재 Character Select Level이 준비한
class 목록을 사용한다. `Open Editor`는 typed class ID를 현재 Level에서 재확인하고 기존
World Sequencer의 같은 class를 선택한다. 별도 movie player나 별도 Effect 저장 문서를
만들지 않았다. Character Select 바깥에서는 진입 안내를 표시한다.

Intro/Loop의 World Model, Animation, Camera, Effect, Material, Light, Sound, Time Control
박스를 선택하여 기존 행과 키를 편집한다. 숫자·벡터·시간·clip과 key duplicate/delete는
Box Detail에서 처리하며 source ms를 원본 clock으로 movie ms에 투영하는 기존 경로를 유지한다.
Effect 내부 요소는 해당 박스의 `Open Effect Editor`로 V1 원본 문서를 열어 저장한 뒤
다시 Play한다. F1 Movie와 Sequencer는 같은 Level owner를 소비한다.

`Apply row`는 실제 모델·clip/source offset과 문서를 검증한 뒤 재생을 중지하고 초안에 적용한다.
처음 편집기를 열거나 박스를 선택하는 조회는 현재 재생을 중지하지 않는다.
`Save movie`는 Camera manifest와 SL00 WorldSequences를 baseline/draft/latest로 병합하며
stable ID가 있는 배열의 외부 재정렬과 무관한 필드 변경을 보존한다. 같은 필드 충돌과
stable ID 없는 key 배열의 동시 변경은 기존 파일·초안을 보존하고 거절한다. 저장은 실제
직렬화 바이트 재검증, 교체 직전 최신 바이트 재확인, 백업과 원자 파일 교체 및 자기 변경
rollback을 사용한다. World 변경은 기존 Area `WorldSequences` publisher로 비동기 게시하고
완료/실패를 따로 표시한다. Camera manifest는 기존 `CProjectDataRoot` 경로를 직접 소비한다.
`Reload saved movie`는 미저장 초안 확인 후 검증된 저장본만 적용한다.

새 `Client/Private/ClassSelectionPresentation_Authoring.cpp`를 Client 프로젝트와 filters에
각각 한 번 등록했다. 두 XML의 parse 및 등록 개수 확인은 통과했다. All Effects 진입부의
`Effect_Tool.cpp`, `Effect_Tool_ResourceBrowser.cpp`, `MainApp.cpp` 격리 컴파일은 exit0이며
`out/ClassMovieRenderRepair20260925/`의 `client-compile.json`, `main-compile.json`과 개별
compile log에 기록했다. 제품 Data/Camera 및 SL00 저작 원본은 이번 코드 구현 중 직접
교체하지 않았다. UI 조작/화면 판정과 제품 빌드 완료를 이 컴파일 결과로 대신하지 않는다.


### G10 제품 빌드 시도와 현재 실행 파일

`powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1
-Configuration Debug`를 실행했으나 `ProductOutputGuard.psm1`이 컴파일 전에 차단했다.
`out/BuildPipeline/runs/20260925T085410448Z-debug-product.json`이 이 실행 기록이다.
점유자는 Debug Client PID28640·53208과 Debug Server PID276이었다. 이 실패는 C++ 컴파일
오류가 아니며 프로세스를 에이전트가 종료하지 않았다. 따라서 G08의 이전 제품 빌드 이력과
별개로 G09/G10의 이번 소스는 현재 제품 EXE/DLL에 반영되지 않았다. 사용자 저장·종료 후
같은 정상 증분 Product Build가 필요하다. 격리 컴파일/수치 검사 결과와 화면 판정은 구분한다.


## G11. World 무비 편집 계약 최종 검증

최종 검증은 `out/ClassMovies20260925/authoring/`의 실제 Client C++를 연결한 무창 native
fixture에서 수행했다. `authoring-contract-result.json`과 `probe-run.log`는 exit0,
299 assertions 통과를 기록한다. 실제 World instance 460개와 Product Effect 60개의 리소스를
준비하고, 다섯 class의 Intro/Loop에서 존재하는 행 종류별 첫 박스 74건을 원본 행으로
resolve했다. 74건은 모든 박스의 모든 필드 조합을 편집했다는 뜻이 아니다. 실제 변경과
Apply/Save/Reload/Play 검사는 Camera FOV key, World animation playbackRate, movie clock으로
수행했으며 새 재생기를 만들지 않고 기존 Level의 `CClassSelectionPresentation`을 사용했다.

G10의 조회·재생 계약은 다음과 같이 최종 보완했다. `Begin_Authoring`은 원본을 읽어 검증한
초안만 준비하며 현재 재생 token과 generation을 유지한다. 명시적 `Play`는 그때 이미 읽은
초안 generation을 실제 모델·clip으로 준비한 뒤 같은 World owner에 적용한다. Play에서 외부
파일을 다시 읽거나 미저장 편집을 버리지 않는다. 외부 저장본 재읽기는 Save/Reload 경계다.
원본 clock의 마지막 wall time을 sandbox에서 100ms 늘린 뒤 Begin의 기존 duration 유지와
Play의 새 duration 소비를 확인했다. 실제로 없는 clip을 지정한 Apply 및 최초 편집기 조회
후의 명시적 Play는 준비에 실패했고, 기존 활성 재생과 token을 보존했다.

`clockKeys`가 생략된 Guardian Loop의 Time Control 행도 편집한다. 읽을 때는 런타임과 같은
identity clock의 시작·끝 두 key를 초안으로 합성하고, 조회만으로 원본에 필드를 추가하지
않는다. 마지막 wall time을 25ms 변경한 뒤 명시적 Apply에서만 해당 필드를 생성하고 기존
parser와 실제 모델 검증을 통과하는 것을 확인했다. `CWorldSequenceDocument::Load_Text`의
16MiB 상한 거절도 실제 메모리 입력 경로에서 검사했다.

저장·충돌·게시 검증도 sandbox의 실제 파일과 기존 함수를 사용했다.

- Camera key를 적용한 뒤 다른 scene의 외부 저장을 합성했다. Save는 그 무관한 변경을
  보존했고, 저장한 FOV는 Reload 후에도 유지됐다. 같은 Camera key의 동시 수정은 Save가
  거절하여 편집 중 초안과 최신 디스크 바이트를 모두 보존했다.
- 실제 `Merge` 구현으로 optional 필드 추가/삭제와 무관한 필드 변경의 병합, 같은 필드
  충돌, stable ID 없는 key 배열의 외부 재정렬 충돌을 검사했다. vector index를 저장용
  안정 ID로 취급하지 않는다. 직렬화한 실제 World 바이트도 같은 runtime parser에 통과했다.
- World animation을 적용한 뒤 sandbox의 publisher 파일을 잠시 다른 이름으로 옮겼다.
  Save는 원본 저장 완료와 게시 불가를 구분했고, Reload 후에도 게시 재시도 상태를 유지했다.
  파일 복원 후 수정 없는 Save로 기존 `Publish-MapAuthoring.ps1 -Scope WorldSequences`
  게시를 재시도하여 실제 exit0으로 완료했다. sandbox runtime의 revision과 저장된 World
  revision이 일치했으며 같은 movie owner에서 다시 Play했다. 이는 publisher 부재와 복구
  재시도 검사이며, 실행된 publisher의 임의 내부 오류를 모두 주입한 검사는 아니다.
- 마지막 Play 실패 검사의 재입장 준비는 기존 Level 소유 World prototype 정리와
  `Prepare_AreaLoad`의 one-shot Loader staging을 fixture에서 재현했다. 준비 전제 누락으로
  실패했던 중간 로그와 최종 exit0 로그를 구분하며 제품 코드를 우회하는 재생기를 쓰지 않았다.

`client-compile.json`과 네 개 `*.authoring.compile.log`는 `ClassSelectionPresentation.cpp`,
`ClassSelectionPresentation_Authoring.cpp`, `SequencerTool.cpp`, `WorldSequenceDocument.cpp`의
최종 격리 컴파일 exit0을 기록한다. `probe-compile.log`와 `probe-link.log`도 exit0이다.
Client 프로젝트/XML filters parse와 해당 변경의 `git diff --check`를 통과했다. 최종 구현과
fixture 입력의 SHA256은 `implementation-source-hashes.json`, `probe-source-hashes.json`에 있다.
이 격리 검사는 정상 Product build 완료 여부와 별개다.

`source-hashes-before.json`과 `source-hashes-after.json`에서 실제 원본의 SHA256이 일치한다.
Save와 publisher가 쓴 경로는 `out/ClassMovies20260925/authoring/sandbox/` 아래다.

| 변경하지 않은 원본 | 작업 전후 동일 SHA256 |
|---|---|
| `Data/Camera/ClassSelection.cinematics.json` | `b157eb86c8ae8f8c44d8f16747eafb963b5c89d740bfb296e851bbe1beee96ba` |
| `Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/LV_LOBBY_CLASSSELECT_SL00.worldsequences.json` | `319761aa3fd3c3be81809c8c3824287a965d1e0da7e92f4e63ccf77c20e533bd` |

이 fixture는 최신 Debug C++와 완료된 동일 Release CSO 세트의 물리 복사본 230개를 사용한
무창 D3D11 WARP 편집 계약 검사다. `consistent-shader-fixture.json`에 원본 경로와 hash를
기록했고 out 사본은 hardlink를 끊은 독립 파일임을 확인했다. 실행 중 제품 빌드가 shader
출력을 갱신하더라도 시험 입력이 바뀌지 않도록 분리했으며 제품 CSO를 덮어쓰지 않았다.
Client.exe/UI 조작, 실제 화면·소리 판정, shader 간 시각적 동등성이나 Movie 전체 GPU draw의
성공을 이 결과로 주장하지 않는다. 해당 제품 빌드·GPU 결과는 별도 후속 검증으로 구분한다.


## G12. 최종 Debug 제품 빌드와 실제 GPU draw 검증

사용자가 Client/Server 종료를 알린 뒤 정상 Product Build가 완료됐다.
`out/BuildPipeline/runs/20260925T092422866Z-debug-product.json`은 전체 PASS, 약 26분 59초이며
model CSO 48개를 갱신했다. 최종 증분도
`out/BuildPipeline/runs/20260925T092644071Z-debug-product.json`에서 40.025초 PASS다.
최종 Debug Client.exe 수정 시각은 2026-09-25 18:26:43 KST이며 Engine.dll도 배포했다.
G09/G10의 미빌드·파일 점유 설명은 당시 이력이고 현재 설치 상태는 이 항목으로 갱신한다.
Release는 이번 후속 수정의 최종 제품 빌드 대상이 아니다. 기존 컴파일 경고는 남는다.

draw 근거는 `out/CombatHitHover20260925/movie-render-probe/`에 있다. 실제 Engine link 입력에
현재 Material/Model/Mesh와 같은 DLL 내부 registry 진단 bridge를 연결한 격리 Engine.dll을
사용했다. `engine-inputs.json`과 실행 receipt의 loaded path/hash가 그 경계를 기록한다.
완성된 Product의 CSO 230개를 독립 파일로 복사했고, 복사 전후 설치 source와 복사본 hash를
`shader-inputs-complete-product.json`으로 대조했다. 후속 증분은 CSO/Engine을 변경하지 않았다.
기존 movie runtime와 일치하는 baseline header/OBJ의 render fixture를 사용했으며 최신
편집기 코드의 메모리 layout·Apply/Save 검사는 G11의 별도 일치 fixture로 검증했다.

- 최종 무창 D3D11 HARDWARE 검사 `receipt-reported-hardware-lifecycle.json`은 42.346초,
  exit 0이다. GuardianKnight와 DimensionMaster의 전체 intro 및 두 loop 주기를 250ms 간격으로
  진행하고 Dimension 4500/4600/4650/4666.667/4670/4700/4750/4800ms도 직접 검사했다.
  총 646 sample frame, WORLD Render_Group 29,446회, 고유 instance 141개, render 실패 0이다.
  pause·seek·0.25/2배속·phase end·Stop·replay와 camera/effect/particle finite 값도 검사했다.
  GPU completion event 및 GetDeviceRemovedReason 성공을 확인했다. predicate는 사용하지 않았다.
- 이 전체 구간 검사는 WORLD 배우 draw이며 배경 draw는 0, 최대 native row는 27이다.
  배경을 포함한 별도 실제 WARP draw에서 Guardian 시작은 고유 row 305개까지 도달했고
  문제의 a12265.p0/p1은 row 289, p2는 290에서 성공했다. 기존 256 제한을 실제 장면이 넘는다.
  `run-guardian-first-start-only.log`와 해당 후속 검증 로그가 근거다.
- Dimension 4670ms(원본 clock 4665.55ms)의 배경 동반 검사는
  `receipt-dimensionmaster-background-focus.json`에서 exit 0이다. WORLD 72회와 배경 queue
  2,851회, 최대 native row 222개에서 a740.p0 draw도 성공했다. 이 표본은 256을 넘지 않으므로
  Dimension의 사용자 오류까지 동일 cap 하나가 확정 원인이라고 주장하지 않는다. 독립 재질
  입력·같은 소비자·오류 구간의 현재 성공과 더 자세한 실패 진단이 확보된 상태다.

첫 hardware 검사도 모든 draw/GPU event는 통과했으나 진단 fixture의 ExitProcess가 FMOD
종료를 기다렸다. 제품 수정 없이 out fixture의 조기 ExitProcess를 제거하고 재생 resource와
FMOD를 정상 정리한 뒤 같은 검사를 한 번 재실행해 exit 0을 확인했다.
`receipt-hardware-before-teardown.json`/`run-reported-hardware-before-teardown.log`가 최초
경계를 보존하며 최종 log에는 explicit FMOD cleanup PASS가 있다. 강제 종료를 정상 종료로
기록하지 않았다. 이전 WARP 전체구간 지연/중단 기록도 최종 성공으로 치환하지 않는다.

새 Server의 SkillStages/KoukuProduct 검사는 모두 exit 0, failures 0이고 설치 shader cohort
검사도 통과했다. 세부 증거는 `2026-09-25_COMBAT_HIT_HOVER_RESULT.md` G07/G08을 따른다.
두 프로젝트 XML parse와 전체 `git diff --check`도 통과했다. 최종 EXE/DLL hash는
`out/CombatHitHover20260925/final-product-artifacts.json`에 기록했다.

Client/UI 실행·스크린샷·실제 소리/색/프레임레이트 판정은 하지 않았다. 전체 게임 렌더 합성의
매 프레임을 검증한 결과와도 구분한다. 사용자는 새 Debug Client로 Character Select에 진입해
`F1 → All Effects → World → Character Selection Movies → Open Editor`에서 Intro/Loop와
Box Detail을 열고 `Apply row → Save movie → Play`로 확인할 수 있다.

## G13. 2026-09-29 창술사 지면 잔디 후보 복구

원인은 일반 StaticMeshComponent만 소비하던 추출 범위다. SL03의 기존 catalog 137개와
placement 509개는 그대로 있고, 원본의 InstancedStaticMeshComponent 24개에 담긴 longgrass02
300개 instance만 누락됐다. source visibility는 300개 모두 true이며 원본 RNM도 존재한다.
무비에서 흩날리는 잎 Particle을 추가하거나 조명·후처리 설정을 바꾸는 문제로 처리하지 않았다.

`extract_ue3_foliage_placements.py`는 기존 Bern matrix decoder, component RNM decoder와
source visibility resolver를 재사용한다. v868의 LOD/RNM/footer/80-byte stride·전체 byte span,
행렬·stable ID 충돌과 원본 package hash를 검사한다. 비identity component transform 등 아직
구현하지 않은 layout은 조용히 무시하지 않고 거절한다. 출력은 증거 JSON이며 정본을 교체하지 않는다.

원본 mesh는 `bg_ber_foliage_a.mesh.bg_ber_foliage_longgrass02_sm_ksy`, MIC는
`bg_ber_foliage_a.mat.bg_ber_foliage_longgrass01_mi_ksy`다. source package의 실제 MIC·terminal
foliage graph·정확한 shader static set에서 수치와 wind expression을 회수했다. diffuse brightness
1.2, specular 0.5/power60과 원본 양면 설정을 사용한다. 같은 mesh의 설치 SL01 WModel은 629개
정점/339개 삼각형으로, 새 source glTF의 위치 오차는 최대 7.63e-6cm이고 normal/tangent.xyz,
UV0/UV1과 반전 winding topology는 일치한다. 기존 tangent.w 재구성 229개는 보존했으므로
전체 geometry가 byte 단위 원본과 같다는 증거로 삼지 않는다. diffuse/specular 원본 mip0 압축
payload도 설치 texture와 대조했으며 다른 Area의 material override는 복사하지 않았다.

RNM 24쌍(48개 DDS)의 원본 전체 mip chain을 회수했고 component별 texture와 instance별
UV bias를 연결했다. 별도 catalog/material 24개와 placement 300개를 추가한 후보는 161 assets /
809 placements다. 새 RNM 위치는 `Map/LV_LOBBY_CLASSSELECT_SL03_FOLIAGE/Lightmaps/`다.
원본 shader map의 LocalVF E4FE 입력을 기존 adapter에 연결했다. 원본 InstancedVF와 GPU 결과가
동등하다는 검증은 아니며, 기존 scene no-wind 입력·player sentinel 계약을 유지한다.

publisher는 runtime이 이미 지원하는 foliageWind를 family exact-field 검사에서 잘못 거부했다.
`MapAssetCatalog.cpp`와 `MODEL_SURFACE_PARAMETERS`의 기존 E4FE/A1C6/1C39 검증을 대조해
optional field를 별도로 검증하도록 수정했다. program/family, vector·scalar 크기와 유한값,
bounds·no-wind·time/unused lane·pivot/player sentinel을 검사한다. 새 C++나 program은 없다.

검증 자료는 `out/LanceGrass20260929/`에 있다.

- `foliage.json`: 원본 24 components / 300 instances / 300 visible, stable ID 충돌 없음.
- `geometry-proof.json`, `material-map.json`, `parameters.json`, `lightmaps/`: geometry·원본
  material static set·parameter와 RNM mip 근거.
- `validation.json`: 정상 후보와 기존 A1C6/1C39 입력 통과, 잘못된 wind 8종 거부, native
  truncated/trailing/bad-stride 3종 거부. 원본 source 3개 hash와 기존 모든 row 보존 통과.
- `stage-root`의 isolated ProjectRoot에서 Validate/Publish/Check가 모두 809 placements /
  3 files로 통과했다. `publish.log`, `check.log`는 sandbox 게시 경로를 명시한다.
- Python syntax compile, 후보 JSON parse와 변경 파일 `git diff --check`를 확인했다.

이 항목 시점에는 **후보 준비 완료, live Data/Resources 교체와 제품 Area 게시 미완료**다.
`patch.json`에 최신 병합용 신규 row/Resource와 조사 baseline hash를 기록했다. 적용 직전 최신
저장본을 다시 읽어 stable ID 기준으로 병합해야 한다. 카메라·SL00 worldsequence, 기존 배치,
rendering option과 실행 중 메모리 draft는 변경하지 않았다. Client/UI 실행·화면 촬영·잔디의
GPU draw나 최종 색·밀도·wind 육안 검증은 수행하지 않았다.

### G13 후속 설치·게시와 실제 소비 범위

사용자의 전체 진행 및 GBResources 전달 요청 후 latest source3개를 stable ID로 병합했다.
809 placements/161 assets와24 material variant,300 instance RNM row 및48 DDS를 설치했다.
실제 Area Publish는809 placements/3 files로 통과했다. Placements scope만 사용하면 material
파일을 제외하므로 최종 게시는 Area scope로 수행했다. out/LanceGrass20260929의
install-receipt.json, publish-live-area.log, mirror-receipt.json과 installer-tests/tests.json이 증거다.

Lance scene의 backgroundAreaId는SL03이며 Loader와 Level_CharacterSelect가 같은 full-map
scope를 사용한다. 신규24 prototype을 같은 placement asset ID에서 준비하고 visibility/suppression
및 frustum을 통과하는300 instance를 기존 signed-scale batch로 렌더한다. 잔디 전용 거리 cutoff는
없으며 source camera1003키의 가장 가까운 잔디는0.65~2.40m다. 기존 LocalVF wind와 instance
RNM UV 소비를 scope-audit.json에 기록했다. 실제 GPU 표시·밀도·색 판정은 사용자가 한다.

### G13 Client catalog 거부 정정

후속 사용자 Play All에서 `Catalog validation failed for SL03_FOLIAGE_0003`이 발생했다.
최초 잔디 생성 스크립트가 카탈로그 token 13을 0으로 덮어썼는데, 이 필드는 shadow flag가
아닌 `renderProfile.uvScale.x`다. 신규 24개 행 모두 UV 배율이 `(0, 1)`이 되었고,
Client `CMapAssetCatalog::Load`의 양축 양수 검사에서 첫 행을 거부했다. 최초 publisher는
source catalog의 ID와 경로만 검사해 이 잘못된 render profile을 통과시켰다. 앞선 게시·scope
검증만으로 Client admission까지 확인된 것으로 해석하면 안 된다.

사용자의 현재 저장본 반영 승인 후 24행의 `uvScale.x`만 0에서 1로 교정했다. 교체 직전 hash,
백업, 원자 교체 및 실패 rollback을 유지했으며 source와 runtime의 mapplacements와
mapmaterials hash는 변경되지 않았다. castsShadow와 렌더링 옵션도 바꾸지 않았다.
생성 스크립트의 해당 override를 제거하고 donor UV 양수 검사를 넣었다.
publisher는 imported/published 두 경로에서 카탈로그 숫자 범위와 형식을 Client와 대조한다.

검증 증거는 `out/LanceGrassCatalogFix20260929/`에 있다.

- 제품 C++ `MapAssetCatalog`, `RuntimeAssetRoot`, `ProjectDataRoot`, `DataJson`,
  `SourceCharacterMaterialParameters`를 별도 CPU probe로 빌드했다. Client나 GPU는 실행하지 않았다.
- `cpu-before.log`: 기존 source에서 같은 `SL03_FOLIAGE_0003` 오류 재현.
- `cpu-candidate.log`: 교정 source의 실제 `Load_Source`와 전체 material admission 통과, 161 assets.
- `cpu-runtime-after.log`: 게시 파일의 실제 `Load_Area` 통과, v5 / 161 assets.
- `prepare-receipt.json`: 정상 후보 및 UV 0/음수/underflow, 비유한·overflow speed,
  opacity/emissive/specular/tint/opacityPower 오류 10종의 publisher 거부 확인.
- `install-receipt.json`, `live-validate.log`, `live-publish.log`, `live-check.log`: 원자 설치와
  Area Validate/Publish/Check 809 placements / 3 files 완료, 무관한 source/runtime 보존.
- 기존 48 DDS와 Desktop/GBResources의 SHA-256 일치. 새 리소스는 없다.

검증 도중 runtime 경로를 authoring용 `Load_Source`에 전달해 canonical authoring path 거부가
발생했다. 이는 probe 호출 방식 오류이며 제품의 추가 로딩 결함이 아니다. source rollback 후
실제 `Load_Area` 모드로 교정하고 재설치·게시·Check를 완료해 source/runtime 정합성을 회복했다.
잔디 최종 GPU 표시와 사용자 화면 판정은 별도다.

### G13 잔디 전체 미표시의 실제 draw 바인딩 결함

UV 교정 후 Play All은 동작했지만 사용자가 지면 잔디 전체 미표시를 보고했다.
이전 source/runtime catalog와 scope 검사는 draw 성공을 확인하지 못했다.
`MapAssetRenderUtils::BindSourceFoliageWind`는 enabled일 때
`g_SourceFoliageWindProgram`을 필수로 묶지만 공용 HLSL과 설치된 Debug/Release
Binary/MapInstance CSO에 해당 global이 없었다. 제품 `CShader::Bind_RawValue`는
`E_FAIL(0x80004005)`을 반환하고, `CMapStaticBatchObject::Render`가 material bind 실패에서
실제 Begin/Render_Instanced 전에 반환한다. wind가 활성화된 신규 24개 잔디 그룹 전체에
적용되는 결함이다. 배치 누락이나 카메라 거리·알파 cutout 문제로 우회하지 않았다.

`Shader_SourceFoliageWind.hlsli`에 program 1 선택 변수를 선언하고 기존 E4FE 함수의
입장 조건에 연결했다. 기존 E4FE 산술, source scalar, 지면 배치, material/Resources와
모든 rendering option은 바꾸지 않았다. CPU가 인정하는 A1C6/1C39 입력 계약과 실제 HLSL
산술 지원 범위를 구별하며, 이번 수정은 기존 E4FE의 바인딩 복구만 완료 범위로 삼는다.

독립 설치 CSO dump는 `out/SL03FoliageRenderAudit20260929/installed-shader-audit.json`,
제품 Engine의 실제 CShader 검증은 `out/LanceGrassRenderFix20260929/`에 있다.

- `bind-before.log`: 설치된 기존 두 CSO 및 Binary 28 variant를 out으로 복사해 실제
  CShader 생성 후 Program bind가 두 경로 모두 `0x80004005`임을 재현했다.
- `compile-mapinstance.log`, `bind-map-after.log`: 수정 MapInstance의 fx_5_0/O1 컴파일,
  Program/Enabled/center/bounds/actor/wind/player/scalars[4]/time의 9개 필수 입력,
  surface 9, Clone의 cull-none pass 2 Begin 및 후속 no-wind reset/pass 0 Begin 통과.
- Binary base의 모든 constant는 28 SourceGroup variant에도 같은 형식으로 있어야 한다.
  공용 HLSL 변경의 최종 제품 배포는 Binary/MapInstance와 Binary SourceGroup 28개를
  함께 재컴파일하는 통합 Product 빌드에서 수행한다. 중복 CPU 작업을 줄이기 위해 별도
  out variant 전체 컴파일은 중단했고, 해당 미완료 산출물을 제품에 설치하지 않았다.

이 항목의 현재 검증은 실제 제품 바인딩과 pass 적용이며 최종 게임 장면의 draw pixels나
사용자 화면 확인을 대신하지 않는다. Client 실행·조작·캡처는 수행하지 않았다.

## G14. 가디언 나이트 Intro 사운드 복구
원본 SCENE01 Intro Matinee export702/ref703의0ms AkEvent pc_common_create.pc_create_dragonknight1를
확인했다. Loop Matinee에는 소리 이벤트가 없다. 원본 단일 Play는Layer의 음성303593929와
공통효과107503277을 함께 재생하며 TransitionTime400ms는 지연이 아닌 선형fade다.

48kHz stereo412000frames(8583.333ms) 두 WAV와 stable Sound row2개를 기존 intro carrier에
추가했다. source revision3→4 병합과 WorldSequences Publish를 완료했고 기존 카메라·숫자표기와
다른 carrier 바이트를 보존했다. prepare_guardian_selection_sound.py가 원본HIRC/source hash를
검사한다. out/ClassMovieSoundRestore20260929/guardian의 source/candidate/prepare-validation,
install-receipt/publish.log에 추출·구조·설치/GBResources·게시 근거가 있다.

최초 복원 PCM16의 clipping은 후속 전체 Movie 음질 감사에서 발견되어 float/bus 복원 대상으로
추가됐다. 최종 음원 경로와 음질 수치는09-26 WORLD_MOVIE_EFFECT_EDITOR_RESULT G16을 따른다.
최초 PCM16 후보를 최종 음질 완료로 기록하지 않는다.

최종 잔디 수정의 Release 설치본 후속 검증: Product build/deploy
20260929T131955762Z-release-product.json PASS 뒤 installed Release30개 CSO를 hash 확인해
out/LanceGrassRenderFix20260929/installed-Release로 복사했다. 실제 CShader의 Binary28개 변형
admission, MapInstance/Binary의 wind9개 입력·Clone pass·reset 모두 통과했다. probe.log가 증거다.
수정 전 두 경로 Program bind의 HRESULT0x80004005는 모두0x00000000으로 바뀌었다.
