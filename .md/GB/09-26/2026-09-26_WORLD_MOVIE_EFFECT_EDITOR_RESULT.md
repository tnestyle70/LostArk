# World Movie의 V1 Element 편집과 Camera live 편집 결과

## G01. 구현 상태

`All Effects → World → Character Selection Movies`에서 `Open Editor`가 선택 Movie의 Effect를
V1 Current Effect에 연다. Model View의 Intro/Loop Effect 목록에서 원본 asset ID를 선택하고
Element와 Detail을 편집한다. `View in Movie`는 선택 Effect의 첫 occurrence 시각으로 이동한다.
`Play All`, Pause/Resume, Stop과 시간 탐색은 기존 Level Movie owner를 사용한다.

V1에서 Movie 배우를 별도 Character preview로 대체하지 않는다. 기존 WORLD actor animation과
Effect·camera clock을 유지하며 Solo/Play Group도 Movie 안의 해당 Effect element를 격리한다.
Play All은 격리를 해제한다. Timeline / Camera와 World Effect 박스의 Open Effect Editor는 같은
세션을 왕복한다. 첫 V1 생성 시 callback을 연결하고 World의 playback 소유권을 전달한다.

Element draft는 catalog와 파일을 바꾸지 않는 불변 prepared target으로 준비한다. 기존 level-owned
active/pending handle을 현재 transform history와 source clock에서 검증한 뒤 교체한다. 실패한
후보는 정리하고 기존 instance와 Movie 상태를 유지한다. 편집본은 Play/Stop/Seek·Intro/Loop에서
유지되며 End Movie Editing과 Level 종료에서 정리한다. V1 Save는 기존 Effect 원본 저장과
prepared target 갱신을 사용한다. Load Saved/Discard는 같은 Movie에도 저장본을 반영한다.

문서 전환 코드에는 `A Solo → B 선택 → Play All`에서 A의 격리가 남지 않도록 전체 A draft를
먼저 복원하는 경로를 추가했다. 새 B의 검증이 끝난 뒤 복원하며 실패하면 isolation과 기존 문서를
유지한다. 같은 asset의 Load Saved/Discard도 전체 저장본을 적용하여 `A Solo → Load Saved A →
B 선택`에 격리가 남지 않게 한다. New/Save As는 End Movie Editing을 먼저 요구하며,
End는 임시 target 복원 후 isolation ID와 filter를 초기화한다. 이 단락은 소스 반영 범위이며
실행 검증 결과는 G03, 실제 클릭과 표시 확인은 G04와 구분한다.

## G02. 09-26 초기 카메라와 저장

World Camera Box Detail의 선택 key에 Eye position, Look at, Up direction과 축별 FOV 입력을
추가했다. 기본 `Apply camera when edit ends`는 입력을 놓을 때 반영하며 명시적
`Apply camera live`도 제공한다. key ID와 원본 source/movie 시간 변환을 유지한다.

동일 box 시작·길이 안의 key 변경은 현재 camera만 다시 평가한다. 재생 token, 시각, pause와
배우·Effect를 유지한다. 미래 key 수정은 현재 시점을 그 key로 강제 이동시키지 않는다.
camera box 시작·길이 변경과 다른 row는 기존 전체 admission·Stop 후 Apply를 사용한다.
`Save movie`는 기존 stable ID/field 병합·freshness·원자 저장을 사용하고 Effect 내부 파일은
V1에서 따로 저장한다. 실제 저작 원본과 rendering 옵션은 작업 중 직접 교체하지 않았다.

## G03. 자동 검증

정상 Debug Product Build는 PASS다. 명령은
`powershell -NoProfile -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug`다.
첫 전체 변경 빌드는 `out/BuildPipeline/runs/20260926T024700420Z-debug-product.json`으로
94.6초, Client OBJ68개와 EXE를 갱신했다. 후속 보완 증분은
`20260926T024942343Z-debug-product.json`과 `20260926T030209814Z-debug-product.json`이다.
최종 prepared identity 수정 빌드는 `out/BuildPipeline/runs/20260926T030655739Z-debug-product.json`으로
11.7초, Client OBJ1개와 EXE를 갱신했다. 네 실행 모두 PASS, CSO/PCH 변경0이며 기존
C4819·외부 라이브러리 PDB 경고는 남는다. 설치 실행 파일은 `Client/Bin/Debug/Client.exe`다.

`python out/WorldMovieEffectEditor20260926/build_probe.py --run`의 compile/link/run 모두 exit0이다.
현재 Product Client OBJ와 Engine DLL/import library, 별도 복사한 현재 CSO로 연결한
창 없는 D3D11 WARP 검사다. 이전 Client 구현을 복사·재컴파일한 대체 런타임은 사용하지 않았다.
실제 Loader의 primary+5 Movie stage, 460 World occurrence, 60 Movie Effect resource를 준비했다.
최종 기록은 `out/WorldMovieEffectEditor20260926/probe-run.log`, 빌드 입력과 해시는
같은 폴더의 `product-inputs.json`, `shader-inputs.json`에 있다.

| 검사 | 실제 결과 |
|---|---|
| Effect pending/active 교체 | PASS. bloom·Element transform을 바꾼 draft 교체와 bloom 값 소비를 확인하고 배우 instance·camera·token·clock·pause와 지연 layer commit을 보존 |
| 실패 rollback | PASS. 잘못된 bloom 및 두 번째 replacement의 잘못된 handle을 거절하고 원래 object·handle·preview를 보존 |
| Movie 재생 지속 | PASS. Stop 중 draft 준비는 재생을 시작하지 않으며 Play/Seek·Intro/Loop에서도 같은 draft 유지, Clear는 최신 저장본으로 복원 |
| catalog 격리 | PASS. 임시 draft가 Product catalog의 원본 문서와 identity를 변경하지 않음 |
| Camera live | PASS. Eye/FOV 즉시 sample, paused/running 상태 보존, 잘못된 Eye=LookAt 거절, 미래 key identity와 source/movie 시간 변환 보존 |
| Camera Save/Reload | PASS. sandbox에 camera-only 저장, World publish 없이 Reload 후 Eye 유지와 재적용 |
| V1 실제 명령 | PASS. public Open/Play가 첫 호출부터 동일 Movie callback·timeline 사용, Solo A→B 전환 시 A 전체 복원, 미저장 전환 거절, End 정리 |

초기 native 실행은 prepared resource와 Document identity 불일치로 실패했다.
`Build_ResourceSignature`가 주소+asset ID를 사용하므로, prepare 뒤 문서 복사본을 만들던 순서를
불변 shared Document 선생성 → 동일 객체 prepare/attach로 고쳤다. 검증을 제거하지 않았다.
실패 기록은 같은 폴더의 `probe-run-first-failure.log`, `probe-run-staging-failure.log`,
`probe-run-resource-identity-failure.log`에 보존했다. 최종 재실행은 위 모든 항목을 통과했다.

실제 `Data/Camera/ClassSelection.cinematics.json`과 SL00 World 원본의 전후 SHA-256은 동일하다.
Save는 sandbox만 사용했고 V1 검사는 실제 원본을 읽기만 했다. 변경한 C++ 파일 20개는 원래 인코딩을
유지했으며 `git diff --check`를 통과했다. 소스 JSON/XML과 프로젝트 등록 변경은 없다.

## G04. 사용자 확인 경계

Client/UI를 실행·조작하거나 화면을 캡처하지 않았다. 화면·소리와 실제 ImGui 클릭은 사용자
확인이 남아 있다. 새 Debug Client에서 Character Select 입장 후 다음 경로로 확인한다.

1. F1 → Effect Tool V1 → All Effects → World → Character Selection Movies → Open Editor.
2. Model View에서 Intro/Loop의 Effect 선택 → Current Effect의 Element 선택 → Detail 수정.
3. Play All로 배우 애니메이션과 Effect를 함께 재생하고 Pause/View in Movie/시간 탐색을 확인.
4. Timeline / Camera → Camera box 더블 클릭 → Sequence Camera Tool에서 key 선택·탐색·Eye/LookAt/FOV 수정.
5. Effect는 V1 Save, Movie 카메라는 Save movie, ALT V 카메라는 Save camera source로 저장.
6. World/Object source 변경은 별도 Publish, ALT V는 Publish saved cameras 후 다음 재생으로 확인.

이번 완료 범위는 구현·빌드·자동 계약 검증이다. 실제 화면의 시각적 일치 판정은 포함하지 않는다.


## G05. 09-27 재생·전수 조사 추가 결과

아래는 09-26 G01~G04 이후의 보완이다. 이전 native 검사에서 내부 target 격리만 확인한 항목을
실제 V1 Element 버튼과 동일한 선택 경로의 검증으로 확대했다. 개수나 helper 성공을 실제
Element Solo 표시 성공으로 대신하지 않는다.

- Element Solo, marked/manual/anchor/family Group은 필요한 부모 simulation을 유지하면서
  선택 stable Element ID만 draw한다. PSC source age를 Movie 시간으로 역변환하여 Solo는
  선택 구간 끝에서 pause하고 Group repeat는 같은 구간만 반복한다.
- V1와 World의 Play All은 전체 draft를 복원한다. 속도 0.05~2배는 원본 slomo와 같은 Movie
  clock에 곱해진다. Server simulation 속도를 변경하지 않는다.
- preview draft 교체, 선택 전환, 같은 phase seek, 숨김·삭제 ID 정리, Stop/Restart는 기존
  Movie owner의 actor/camera/token/clock/pause를 보존하는 경로로 보완했다.
- `out/WorldMovieEditing20260927/probe-run-full-pass.log`는 당시 실제 Product OBJ와 WARP의
  전체 검증 성공 기록이며 135개 camera 표본과 실제 설치 WModel의 머리 본 663개도 포함한다.
  후속 UI·저장 계약 변경의 최종 검증은 G09에 별도로 기록한다.

전수 집합은 5 class의 고유 60 Effect / 248 Elements / 230 World objects다. Intro/Loop별
460 occurrence와 고유 World object 수를 혼동하지 않는다. 네 대상 class만 합치면
56 Effect / 223 Elements / 208 World objects다. 참조 Resources 975개 및 기존 delivery
4,062개는 파일 누락 0이고, 실제 source system 대비 active emitter 집합 누락도 0이다.
이 결과는 GPU 출력과 원본 장면의 완전한 시각적 동등성을 보장하지 않는다.

## G06. 회색·미표시의 확인 원인과 복구 경계

| 확인 결함 | 반영 범위 |
|---|---|
| masked LocalVF의 cb0[0] RGB만 채우고 alpha=0 | native4644/5033/5100의 primitive RGBA 입력 복구 |
| 중간 register에서 곱하는 World primitive opacity 누락 | native1500/1501/1502/1506/1516/1520/1524, World 39개 |
| 원본 world varying에 clip 위치 전달 | 1518/1523 Base·Light 네 경로의 실제 world-cm 위치 연결 |
| 1518 PS의 camera 위치 prefix 누락 | 기존 source camera 입력 연결, Warlord World 5개 |

원본 MIC 색·곡선·텍스처, 정상 DimensionMaster A, 팀장 rendering option은 변경하지 않았다.
Generator와 Engine/Client 설치 shader를 같이 수정했다. 19개 prefix WARP case, 196개
program/stage 입력 대조(의도한 4개만 변경, 나머지192개 동일), camera subtraction 4개 case가
성공했다. 독립 FXC 검증과 source SHA는 `out/WorldMovieMaterials20260927/RESULT.md` 및
같은 폴더 receipt가 소유한다. 통합 Product build는 G09와 구분한다.

**미복원: 1509/1517/1523을 쓰는 Warlord 8개·Artist 1개 plane.** 현재 near/far 입력 0이
Base alpha를 0으로 만든다. 원본 actor/component/CDO 6개를 조사하여 OpacityRate=1은
확인했지만 non-Decal StaticMesh의 native draw-binding 계약은 확보하지 못했다.
별도 DecalComponent FarPlane=300은 이 상속에 해당하지 않는다. `5*far/far`의 단순 약분은
far=0에서 원본 NaN/saturate0과 달라 적용하지 않았다. 9개가 복원 완료됐다고 기록하지 않는다.
근거는 `decal-engine-defaults.json`, `remaining-decal-gpu-receipt.json`,
`remaining-decal-algebra.json`이다.

DimensionMaster 정상 A는 그대로다. Movie의 cube4921과 slash4924/4926/4927은 실제 설치
EffectObject/CSO WARP draw에서 각각 PS invocation을 확인했다. 전체14 emitter CPU45개
particle이 finite이고 GPU는14개 제출·suppressed0이다. slash4925는 원본 one-sided 검은
메시의 backface이며 강제 two-sided로 바꾸지 않았다. 원본 PSC clock은6.844285초 이후
slomo=0에서 age0.261440초로 정지한다. 후반 closeup에서 cube가 frustum 밖인 경우도 있다.
이 수치 증거는 사용자의 실제 화면 표시를 완료로 대체하지 않는다. 상세는
`out/DimensionMasterAVisibility20260927/MOVIE_CUBE_SLASH_RESULT.md`에 있다.

## G07. 원본 카메라와 Sequence Camera Tool

5 class의45컷·13,096키는 원본 카메라와 parent 변환을 대조해 일치했다. 독립225개
FRotator 계산의 Eye/LookAt 오차는9.1e-13m 이하, Up은3.1e-15 이하이다.
Guardian Loop의1/3컷 얼굴이 화면 밖인 구도도 원본 데이터와 같았다. 따라서 원본에
일괄 높이 보정을 적용하지 않고 사용자가 구도를 수정·저장하는 경로를 구현했다.

World 카메라 box 더블 클릭 또는 Box Detail의 Open Sequence Camera Tool로 전용 창을 연다.
컷 목록, stable key 목록, ms/frame 시간, Eye/LookAt/Up/FOV, 보간·cut, 키 추가·삭제·시간 변경,
선택 구간 Eye+LookAt offset을 같은 editor에서 편집한다. Live preview는 현재 재생 시각을
보존한다. 저장되지 않은 행을 다른 컷으로 바꾸어 버리지 않으며 Save는 pending key도 적용한다.

ALT V/Character Action도 같은 editor를 사용한다. 기존 sequence arrangement 저장과 실제
Product camera 입력을 구분하고, `Data/Effects/Sequences`의 canonical camera source를
stable row/key ID의 3-way merge로 저장한다. 순서/미관계 필드/외부 수정은 보존하고 같은 필드
충돌은 거절한다. 원자 교체·backup freshness와 rollback을 유지한다. arrangement에는 optional
source provenance를 보존하여 다시 열어도 연결되고, 모호한 legacy row는 임의 매칭하지 않는다.
Publish saved cameras는 검증된 Product camera cache만 교체하며 다른 Effect GPU resource를
재생성하지 않는다. `SequenceCameraEditor`와 `RecoveryCameraAuthoringSession`의 H/CPP 네 파일을
Client vcxproj와 filters에 등록했다.

## G08. 행 정리·편집과 Save / Publish 계약

공통 CompositionTimeline allocator를 Saydon/Effect/Object/World가 사용한다. 겹치지 않는
box는 같은 표시 행을 재사용하고 겹치는 구간만 추가 행을 사용한다. 저장 ID를 표시 행 index로
바꾸지 않는다. 배우/slot별 animation은 분리하고, 지속 World Model·Material·Light는 기본
접기와 활성 개수·구간 요약, 검색/현재 시간 활성 항목 필터를 제공한다.

World Camera body는 컷 순서를 옮기고 edge는 이웃과 공유 경계를 이동하여 phase 전체를
빈틈없이 유지한다. 긴 첫 컷도 마지막 위치로 옮길 수 있다. Effect/Sound body 이동·edge trim,
finite native animation의 Source In/Out trim은 기존 owner의 validate→stage→commit을 사용한다.
Object는 기존 Animation owner와 stable-ID Effect/Collider gesture를 사용하고 preview clock과
접기 상태를 유지한다. 첫 animation도 기존 owner가 지원하는 지연 시작을 편집한다.
loop clip의 부적합한 trim이나 native 범위를 벗어난 편집은 거절하고 이유를 표시한다. stale gesture와 실패 후보는 기존 draft를 보존한다.

| 도메인 | Save 정본 | Publish 소비자 |
|---|---|---|
| World Movie 카메라/시계/Effect occurrence | Data/Camera/ClassSelection.cinematics.json | Client가 저장 원본을 직접 읽음 |
| ALT V canonical camera | Data/Effects/Sequences | Client Product camera cache 재검증·교체 |
| World/Object 배치·animation·sound | Data/Maps/Authoring의 Area worldsequences | 기존 WorldSequences publisher → Client/Bin/DataFiles/Map |
| Object의 연결된 Pattern/Collider/Logic | 기존 해당 저작 문서 | World 게시 후 필요한 기존 Pattern 게시 연계 |
| V1 Element 내부 재질·곡선 | 기존 Data/Effects 저작 문서 | 기존 Effect catalog/prepared 소비 경로 |

World와 Object UI의 Save는 source 저장, Publish는 저장본 검증/실행 데이터 반영으로 분리했다.
Object Save는 이후 필요한 linked Pattern publish flag를 보존한다. 서버용 gameplay source는
해당 publisher가 Server/Bin/DataFiles로 생성한다. 모든 Publish가 Server 데이터를 만드는 것은
아니고, 로컬 파일 게시를 LAN 전송이나 실행 중 Server 메모리 적용으로 설명하지 않는다.
기존 Kouku 새 Complete Play의 revision admission 등 각 domain의 소비 시점을 따른다.

## G09. 최종 검증 진행 상태

Debug Product 통합 빌드는 PASS다. 기존 정식 명령을 그대로 사용했다.

- `20260927T010546310Z-debug-product.json`: Engine/Client shader 입력 변경을 포함한 전체 증분
  PASS, 총1,413,014ms. Client 단계는 OBJ66/CSO87/EXE1 갱신이다.
- `20260927T010720569Z-debug-product.json`: 저장 검토 보완과 camera invalid-row 복구의
  후속 증분 PASS, Client OBJ2/EXE1 갱신이다.
- `20260927T010843321Z-debug-product.json`: sub-0.1ms World Effect grip의 clamp 범위 보완
  PASS, Client OBJ1/EXE1 갱신이다.
- `20260927T011833206Z-debug-product.json`: 첫 animation 지연 시작을 실제 owner와 맞춘 UI,
  phase 끝의 fractional camera 삽입점 보완, 잘못된 first-start 주석 정정 후 PASS다.
  공통 header 의존성 때문에 Client OBJ182/EXE1을 갱신했고 CSO 추가 변경은0이다.
  기존 FXC 및 C4819/PDB 경고와 신규 실패를 구분했다.

ALT V native 계약은 해당 소스의 Product OBJ12개/TLOG 의존 freshness를 확인한 뒤31개 전부 PASS다.
canonical Save→runtime parse→재열기, source provenance 왕복/legacy 모호성 거절, 키 추가·삭제·시간·offset,
외부의 다른 field/key 보존 및 같은 field 충돌 거절, invalid Up 거절, no-op clean과 dirty 전환 보호를
검사했다. `out/DimensionMasterAVisibility20260927/camera-contract-run.log`와
`camera-contract-object-audit.json`이 근거다. 실제 active Product cache 교체는 소비 코드·빌드로
확인한 범위이고, 이31개를 해당 live GPU 표시까지의 실측으로 세지 않는다.

공통 표시 행 allocator MSVC standalone 검사는 PASS다. Client 프로젝트 XML parse 및 신규 네 파일의
vcxproj/filters 유일 등록도 PASS다. 저장·테스트는 out의 sandbox만 사용하며 이번 작업에서 실제
Data/Resources를 교체하지 않았다. 다른 세션의 Data/Resources·의상·눈 튜닝 수정은 그대로 유지했다.

최종 `python out/WorldMovieEditing20260927/build_probe.py --run --v1-only --probe-only`는
archive/compile/link/run 모두 exit0이다. 이름의 v1-only는 기존135 camera full audit의 재실행만
생략하며 신규 MovieTiming/EffectRetime/PublishFreshness/Object 검사는 모두 실행했다.
현재 Product Client OBJ·Engine DLL·실제 설치 리소스·CSO를 사용한 창 없는 WARP 검사다.

| 최종 검사 | 실제 확인 |
|---|---|
| Movie 시간/표시 ID | 5 class Intro/Loop의 source↔Movie 변환, 원본 slomo와 저장 box ID 보존 |
| World Camera | key 추가·삭제·offset, 현재 camera sample/재생 token 보존, Save/Reload 후 pose, 이웃 컷 경계·순서·stable ID·phase 끝 fractional 삽입 |
| Effect 이동 | 4번 이동×129개 sample의 age/root quaternion/scalar·vector Hermite 보존, phase endpoint와4096 key 상한 실패 rollback |
| Save/Publish freshness | camera-only Save에서 외부 World 수정 병합 및 pending 갱신, no-op Save 재동기화, stale World/manifest와 cached false의 잘못된 Publish 거절 |
| Object | 실제 release handler의 Effect/Collider move·trim, native animation source 범위·첫 clip 지연 이동, 시계·접기·stale/invalid draft 보존, Save(false)와 재열기 |
| 재시작 후 Object Publish | 저장된 실제 Sequence 연결로 pending 복구, 미저장 연결 editor 보호, Sequence-only를 Pattern 게시로 오인하지 않음 |
| V1 재생/편집 | Element Solo/Group/Play All, 속도/seek/phase 전환, draft 교체, 선택 변경·실패·Stop/Restart·Save와 owner 상태 보존 |

로그에는 PASS assertion1,632회(반복 ID/표본 검사 포함), RESOURCE PASS460회, FAIL/EXCEPTION0이
있다. 이를1,632개의 독립 기능 검증으로 해석하지 않는다. 원본 Camera135표본/머리663개 전체
감사의 이전 성공은 G05 근거를 유지한다. 최종 로그는 `out/WorldMovieEditing20260927/probe-run.log`,
Product/fixture/CSO 입력은 `product-inputs.json`, `shader-inputs.json`이며 최종 source/OBJ 대응은
`final-source-product-checkpoint.json`에 기록했다. 최종 보존본은 `probe-run-timeline-pass.log`,
`product-inputs-timeline-pass.json`, `shader-inputs-timeline-pass.json`이며 실행 exit0과 SHA를
`timeline-pass-receipt.json`에 함께 저장했다.

초기 fixture의 MSVC private-access 선언과 잘못된 MOTION_END start 전제는 fixture에서 수정했다.
첫 animation은0 고정이라는 오래된 주석과 실제 owner가 달라 UI 제한·주석을 실제 지연 시작
계약으로 바로잡았다. 실패 검증을 제거하지 않고 overflow/stale/anchor 보존 검사를 유지했다.

Save의 ReplaceFileW 경쟁 경계도 actual backup≠baseline이면 원래 baseline이 아니라 실제 latest를
복원하도록 보완했다. Windows 임시 파일 단계 검증2개가 성공했고, 이 단계 검사를 production
owner에 실시간 race를 주입한 결과로 기록하지 않는다. 근거는
`authoring-retime/backup-cas-receipt.json`이다.

Client/UI는 실행하거나 조작하지 않았다. 실제 ImGui 조작·장면 표시·소리 확인은 사용자 화면
확인 경계이며 G06의9개 plane은 미복원이다.

## G10. GBResources 전달 확인

사용자의 추가 요청에 따라 실제 설치 파일과 이전 receipt를 다시 대조했다. 이번 재질 변경은
HLSLI15개와 Python generator3개이며 Resources 신규 파일·수정 파일·새 asset 참조는0이다.
설치 receipt4,062개는 missing0이고 이번 복구 전 감사 이후 SHA 차이0이다. 기존에 달랐던
SL12 normal3개와 Lance face는 그 당시 SHA를 그대로 유지했다. 현재 Movie 참조975개도
모두 존재한다. 따라서 이번 작업으로 GBResources에 추가·교체할 파일은0개이며 폴더를 쓰지 않았다.
다른 세션의 의상/avatar180개 전달과 눈·머리카락 작업은 보존했다. 셰이더 소스와 생성기는 코드
변경으로 전달하며 CSO/EXE/중간 산출물을 GBResources에 넣지 않는다. 근거는
`out/WorldMovieMaterials20260927/gbresources-scope-audit.json`이다.

## G11. Character Select 114/114 뒤 Client 종료 원인

사용자 재현의 PID63376/53620/38364는 각각10:32:37/10:32:42/10:36:27에
Render_Shadow_Object(CMapStaticBatchObject) E_FAIL로 종료됐다. Effect 준비 후의 첫 배경 렌더
실패이며 정상 startup 로그와 PID를 대조했다. 서버 연결 오류나 Windows exception으로 기록하지 않는다.

실제 설치 SL00의 Loader→MapPlacementRuntime→CMapStaticBatchObject를 창 없는 WARP로 호출했다.
수정 전213 batch/336 mesh에서 shadow59개·일반 render204개가 실패하고 직접 material bind331개가
실패했다. wind-enabled mesh는0개다. 설치 Debug/Release MapInstance의 필수 uniform reflection 및
shadow pass Apply도 성공했으므로 처음 조사한 wind program 누락과 OBJECT/INSTANCED mode 차이를
이 종료의 원인으로 기록하지 않는다. 해당 wind 소비자 누락은 설치되지 않은 별도 후속 경계다.

핵심 원인은 CMaterial::Bind_SurfaceTexture의 SRV 복사 감소 최적화에서 사용한 `&m_SurfaceDiffuse`
등10개 ComPtr 주소 취득이다. Windows SDK WRL의 ComPtrRef::operator T*는 owner를 nullptr로 만들며
기존 참조를 Release한다. 그래서 재질 연결 시 자기 texture가 사라졌다. 같은 최적화를 유지하면서
10개 주소 취득만 std::addressof로 수정했다. MapStaticBatchObject는 실패 단계/asset/mesh/pass/HRESULT와
device-removed HRESULT를 기존 제한된 진단 로그에 추가하고 upload Map의 원래 HRESULT를 보존한다.

실제 WRL+fake IUnknown native 검사11개가 PASS다. 잘못된 & 차용은 단독 소유 resource를 파괴하고,
공유 소유일 때는 material owner만 비운다. std::addressof 10,000회는 identity/refcount 및 AddRef/Release
호출 수를 보존한다. 이는 C++ 소유권 검증이며 실제 GPU scene 검증과 분리한다.
근거는 out/MapStaticShadowExit20260927/com_ptr_borrow_run.log 및 receipt다.

수정 전 native 로그/입력은 before-receipt.json, probe-run-before-native.log,
probe-run-diagnostic.log, product-inputs-before-native.json, map-inputs.json에 보존했다.
최초 정본 Product 시도는 실행 중 Server PID64980을 ProductOutputGuard가 거절해 컴파일 전 중단했다.
실패 receipt는20260927T014813564Z-debug-product.json이다. Server를 자동 종료하지 않았다.
수정 후 전체 batch/정본 빌드 결과는 검증 후 아래에 추가한다.

정본 출력은 보존한 채 기존 Engine의 실제 CL/link tlog 명령에서 출력 경로만 out으로 바꾸고
Material.obj 하나를 수정 소스로 교체한 격리 DLL을 만들었다. compile/link 모두 exit0이며
동일한 수정 전 native EXE에서 Engine DLL만 교체한 A/B 결과는 다음과 같다.

| 실제 SL00 소비자 | 수정 전 | 수정 후 |
|---|---:|---:|
| 배경 batch / mesh | 213 / 336 | 213 / 336 |
| material Bind 실패 | 331 | 0 |
| Render_Shadow 실패 | 59 | 0 |
| 일반 Render 실패 | 204 | 0 |
| shadow / visible instance 수 | 760 / 768 | 760 / 768 |

수정 후 실행 exit0. 셰이더/리소스/배치/렌더 설정은 같은 입력이며 Engine DLL의 해당10개 차용
수정으로 오류가 사라졌다. out/MapStaticShadowExit20260927/probe-run-isolated-fixed.log와
engine-candidate/receipt.json에 source SHA, compile/link/run 결과, DLL SHA를 기록했다.
Product/EngineSDK는 아직 교체하지 않았다. 사용자에게 현재 Server 종료 후 정본 빌드 허용을
요청했고, 답변 전에는 프로세스 종료나 실제 출력 교체를 수행하지 않는다.

## G12. WORLD 항목 검사·Delete 저장·F6 자유시점 구현 (2026-09-27)

### 완료한 연결

Effect Tool Movie controls와 WORLD Action Workbench/Sequencer에 공용 `Movie world models`
검사 패널을 연결했다. 기존 ClassSelectionPresentation 하나가 재생과 검사 상태를 소유한다.
목록/타임라인/장면 triangle Pick으로 같은 stable instance/slot/object를 선택하며 WModel,
mesh·material 이름, authored/drawn 상태 및 실제 sampled XYZ를 표시한다. 검색과 현재 시각의
저작 visibility 필터가 있다. 선택 강조와 Focus selected는 현재 CModel pose를 사용한다.
배경 Map/Effect는 별도 preview 토글이며 Solo는 WORLD 모델끼리 격리한다.

Mute/Solo는 렌더 제출과 실제 draw 양쪽을 차단하는 별도 inspection gate다. WORLD authored
visibility, animation, bone pivot 계산을 유지한다. Effect도 Set_Visible(false)를 쓰지 않고
queued/active root와 preview replacement에 draw 전용 gate를 전달해 lifetime·history를 유지한다.
Play All로 시간을 재시작해도 WORLD 검사 필터는 유지하고 Clear preview filters로 해제한다.

Delete from Movie는 scene의 optional excludedWorldObjectIds에 선택 object ID를 기록한다.
현재 프레임에서 즉시 제외하며 같은 클래스 Intro/Loop에 적용한다. 공유 WModel과 WORLD
리소스를 삭제하지 않고 원본 가시성·pose·bone provider도 보존한다. Show deleted models로
항목을 찾아 Restore to Movie 할 수 있다. Save Movie는 기존 stable-ID merge, freshness,
writer lock, backup 및 원자 교체를 사용하고 최신 disk의 무관한 필드는 보존한다. Mute/Solo/
카메라 검사 상태는 저장하지 않는다. 이 변경은 소스 Movie manifest에 직접 소비되므로 이
필드만 바꾸면 WorldSequences publish가 필요 없다. 원본 사용자 문서에 실제 Delete/Save를
수행하지 않았으며 검증의 데이터 쓰기는 out sandbox 사본에서만 했다.

Play는 Movie camera로 시작하고 F6는 현재 pose/FOV를 보존해 free로 전환한다. 다시 F6는
현재 Movie time의 authored camera로 복귀한다. 재생·일시정지·Seek·Intro→Loop·Effect Solo
모두 같은 clock을 유지한다. free 상태의 Stop은 위치를 유지하고 다음 F6는 기존 플레이어
follow 경로를 사용한다. 자유시점의 실제 camera XYZ와 저작 카메라 sample 표시는 구분했다.
Inspector source 변경 뒤 clean row cache는 generation을 갱신하고, dirty row는 보존하며
명시적인 discard/reopen을 제공해 다른 창의 Delete/Save로 편집이 막히는 문제를 보완했다.

### 실행한 검증

- 변경 header의 tlog 의존 폐쇄 75 TU와 신규 2 TU, 총77 Client TU를 out에서 컴파일했다.
  exit0, 컴파일 도중 source 변경 없음. Model.cpp도 별도로 컴파일하고 새 triangle-pick
  overload를 포함한 격리 Engine.dll/lib 링크와 export 확인을 통과했다.
- 현재 구현/실제 설치 CModel·shader를 사용한 창 없는 WARP fixture compile/link/run0.
  460 WORLD instance 리소스와60 Movie Effect 준비, 5개 class admission 및255 assert PASS.
  F6와 같은 requested follow 전환, exact view/FOV handoff, pause/seek/loop/Effect Solo,
  queued/active Effect mask·preview replacement·history 유지, 실제 Warlord actor 본 부착과
  plane triangle/mesh picking, Mute/Solo/Delete 즉시 draw 격리, malformed/foreign ID 거부,
  sandbox Delete→Save→Reload→Restore→Save→Reload를 검사했다.
  로그: `out/WorldMovieInspection20260928/probe-run.log`.
- UI generation cache의 실제 source 기반13 assertions PASS. UI3개 TU를 다시 컴파일했다.
  `out/ClassMovieInspector20260927`의 generation 및 compile 로그가 증거다.
- 최종 정상 Product Debug Engine/Shared/Server/Client compile/link/deploy PASS.
  최초통합 receipt `out/BuildPipeline/runs/20260927T031256328Z-debug-product.json`,
  UI cache 보완 후 최종 receipt `out/BuildPipeline/runs/20260927T031521071Z-debug-product.json`.
  후자는7 OBJ와 Client binary를 갱신했다. 기존 encoding 및 외부 PDB 경고는 남고 오류0이다.
  build runner의 runtime file/Navigation/Item/Valtan reward 검사도 통과했으며 data publish는 없었다.
- 관련 project/filter XML과 Movie/World/Kouku UI JSON parse 및 scoped git diff --check PASS.

### 화면과 범위 경계

빌드 시 Client/Server가 실행 중이지 않음을 확인했으며 에이전트가 종료·실행·조작하지 않았다.
최신 Debug EXE/DLL 설치는 완료됐지만 실제 사용자 화면 판정은 남는다. picking은 posed
triangle 기준이며 texture alpha texel 판정은 아니다. 회색 plane 원인을 특정했다고 주장하거나
임의 항목을 자동 삭제하지 않았다. 실제 제거 대상은 새 목록과 Mute/Solo로 비교해 선택한다.
새 헤어/Guardian Movie 복원 요청은 해당 실측·변경 결과와 별도로 기록한다.

## G13. Sequencer의 Mute·Delete와 Element 숨김 노출 (2026-09-29)

### 확인한 원인과 반영 코드

기존 G12 WORLD Mute/Solo/Delete는 구현돼 있었지만 Action Workbench session의
`PANE::PREVIEW`에만 공용 검사 패널을 그렸다. 사용자가 첨부한 Composition Sequencer에는
모델 막대 선택만 있어 그 창에서 숨김을 조작할 수 없었다. V1 Element Mute/Visible도 별도
Effect 편집기와 Detail 안에 있었다.

`SequencerTool.cpp`의 Movie 타임라인 상단에 `Movie visibility / models`, 배경/Effect preview
토글, 선택 모델 `Mute model / Unmute model`, Solo, `Delete from Movie / Restore to Movie`,
`Save movie`를 연결했다. WORLD 막대는 muted/solo/excluded 상태를 표시한다. Delete 키는
해당 Sequencer focus와 실제 선택 WORLD stable ID를 요구하고 입력창·active widget·popup·
drag·미적용 row·publish·Effect/Background 선택 중에는 작동하지 않는다. 삭제는 frame 끝에
기존 EXCLUDE 명령으로 전달해 같은 class Intro/Loop의 draw를 제외한다. Movie 저장은
excludedWorldObjectIds에 남기며 WModel/Resources 자체를 지우지 않는다. Restore할 수 있다.

공용 `ClassMovieInspector.cpp`의 각 모델 행에도 M/S와 In Movie checkbox를 표시했다.
선택 Effect의 `Edit Elements / Mute / Hide`는 기존 V1 Movie 편집기를 연다. Movie controls에서
Element 선택, Mute/Unmute, Solo, 저장되는 Visible 초안 및 Save Changes를 직접 조작한다.
Save Changes는 `Try_ApplyDraftAndSave`를 사용해 미적용 Visible 변경까지 저장한다.
선택 Element는 preview 명령 전에 값을 보관하여 Document 교체 후 포인터를 재사용하지 않는다.

기존 Mute는 남은 visible ID가 0개이면 audition interval을 만들 수 없어 마지막 Element를
숨기지 못했다. typed `previewVisibility` callback을 MainApp에서 기존 Movie owner로 연결하고
`Preview_EffectDocument`의 optional submission ID mask로 준비/교체한다. 비어 있는 mask도
허용하며 full Document의 visible·simulation·시간 의미는 유지한다. Mute target은 누적하며
Unmute는 선택 ID만 복원해 같은 Movie 시각에서 비교한다. 실패하면 기존 mask와 target을
보존한다. Play All은 완전한 draft로 복원한다. Valtan cinematic의 기존 audition 경로는 유지한다.

저장 Visible OFF도 마지막 항목에서 실패하던 부분을 codec에서 보완했다. schema-valid하고
비어 있지 않은 문서의 모든 Element/ModelCue가 명시적으로 visible=false일 때만 no-draw로
인정한다. empty 문서, 잘못된 stable ID와 visible 상태의 미지원 carrier 거부는 유지한다.
새 C++ 파일은 없으며 두 기존 header와 여섯 TU만 수정했다.

### 실행한 검증

- 정상 Product Debug의 실제 compile command를 사용해 변경 여섯 TU를 격리 컴파일했다.
  이후 Delete·Unmute·누적 Mute 후속 수정은 해당 TU를 다시 컴파일했다. source 변경 없는
  compile exit0를 확인했으며 로그는 `out/MovieVisibilityBalance20260929/*.compile.log`다.
  이는 최소 컴파일 검증이며 설치 Client EXE의 재링크/교체와는 구분한다.
- 실제 Product Codec OBJ와 변경 RuntimeValidation OBJ를 연결한 console probe compile/link/run0,
  18 assertions PASS. 실제 창술사 `effect.classselect.lancemaster.bfx_low_02.glow.par_b_glow_param_001`
  문서의 마지막 Visible OFF, serialize/parse, sandbox CAS 저장·재로드·Restore를 확인했다.
  empty, malformed/duplicate ID와 schema-valid하지만 visible인 unsupported carrier는 거부됐다.
  stale 저장 실패 뒤 기존 hidden 디스크 내용도 유지됐다. 증거는 `codec-probe-run.log`와
  `codec-probe-receipt.json`이며 원본 Data는 변경하지 않았다.
- 실제 `Preview_EffectDocument` 함수 본문을 사용하는 native boundary fixture compile/run0,
  12 assertions PASS. empty draw mask, 같은 asset의 여러 occurrence만 교체, 잘못된 ID/foreign
  Effect/준비·교체 실패 시 이전 target 유지, Unmute/complete 복원과 clock/pause 보존을 확인했다.
  `movie-mask-run.log`와 `movie-mask-receipt.json`에 기록했다. GPU/service collaborator는
  deterministic 대역이므로 실제 GPU 제출 또는 pixel 표시 성공으로 해석하지 않는다.
- 변경 C++ UTF-8/BOM 여부와 기존 LF/CRLF를 유지했고 scoped `git diff --check`를 통과했다.
  이번 코드 변경은 JSON/XML schema나 project/filter 등록을 추가하지 않는다.

### 사용자 확인 경계

Client/UI를 실행·조작하거나 화면을 캡처하지 않았다. 사용자는 Movie 재생 중 WORLD 모델을
선택하여 Mute를 켰다 꺼 실제 회색 요소를 식별하고, 제거할 모델이면 Delete from Movie와
Save movie를 사용한다. Effect 내부 요소라면 Edit Elements / Mute / Hide로 들어가 비교 후
Visible OFF와 Save Changes를 사용한다. 어느 모델/Element가 회색 원인인지 임의로 판단해
데이터를 삭제하지 않았고, 최종 화면 판정과 설치 실행본 적용은 별도다.

### G13 후속 설치 확인

최종 소스는 기존 사용자 편집을 보존한 채 원래 Desktop/LostArk에 반영했다. 사용자가 저장 후
Client/Server를 종료한 것을 확인하고 Product Debug compile/link/deploy를 완료했다.
원래 저장소의 `out/BuildPipeline/runs/20260928T220610990Z-debug-product.json`이 PASS 근거다.
새 실행 파일에 Mute/Unmute·Delete/Restore 기능이 포함된다. 에이전트가 실행하거나 회색
항목을 임의 삭제하지 않았으며 사용자의 실제 화면에서 대상 선택과 비교가 남는다.

## G14. 창술사·워로드 사운드 동기화와 Sound source-in (2026-09-29)

### 원본과 현재 소비자 대조

| 클래스 | 원본/현재 AkEvent 시작 | Movie2.7초의 source 시각 | 조사 결과 |
|---|---:|---:|---|
| 창술사 |0ms|1807.82ms|PCM 첫 유효 sample1.293ms, 당시 실제 회전 변화90채널|
| 워로드 |300ms|1862.79ms|PCM 첫 유효 sample53.923ms, 당시 실제 회전 변화70채널|
| 도화가 |0ms|2442.46ms|사용자 정상 판정, 기존두 stem과 slomo 보존|
| 차원술사 |100ms|2700ms|사용자 정상 판정, 기존두 stem과 slomo 보존|

네 원본 Wwise Play에는 추가 DelayTime이 없다. 창술사/워로드 PCM 길이는8492.517ms/
9371.202ms로 초 단위 선두 무음은 없다. 원본 finite clip의 마지막 자세 유지도 확인했다.
창술사 slotC의 계산상 끝 source2498.745ms와 설치 torso/hand의2450~2490ms, 워로드
slotA의 끝2217.055ms와 설치60Hz sample2233.333ms가 부합한다. 따라서2.7초를 고정
보정값으로 저장하거나 animation 앞 구간을 임의 추가하지 않았다. 이는 전체 시각적
동등성 판정이 아니라 해당 누락 가설에 대한 원본/설치 수치 대조다.

### 수정한 실제 결함

Movie Update는 긴 frame을250ms까지만 진행하고 Play 직후 한 frame을 defer한다. FMOD는
wall time을 계속 진행하지만 기존 World Apply_Sounds는 살아 있는 handle의 pitch만 바꿨다.
원본 slomo가 .28/.32인 구간에서는 긴 stall의 source delta가250ms보다 작아 기존 seek
조건으로도 잡히지 않는다. 사운드와 시각 clock이 벌어진 채 유지될 수 있는 결함이다.

Engine Synchronize_SoundCue가 실제 cursor를 expected sourceStartMs+age와 비교하며100ms
초과만 seek한다. 명시적 external Movie clock만 사용한다. 소리만 먼저 끝났는데 Movie는
box 안이면 현재 위치에서 새 채널을 준비한다. 최초부터 unavailable인handle0은 반복 로드하지
않는다. 일반 World/SFX의 독립 재생, 기존 음원/시작 시각/슬로모 데이터는 보존했다.

기존 Sound 왼쪽 edge는 startMs/durationMs만 바꿨으므로 음원 앞부분 trim이 아니었다. optional
sourceStartMs를 reader/writer/validation/Map·Composition publisher/실제 WAV offset에 연결했다.
왼쪽 edge는 source-in과 시작을 함께 바꾸고 body 이동은 source-in을 보존한다. Box Detail의
Audio source in (ms)로 timeline 시작은 유지한 채 음원 앞부분을 건너뛸 수 있다. 음원 범위 밖은
기존 초안을 보존하며 거절한다. 이번 데이터에는 임의 source-in 값을 넣지 않았다.

### 검증·적용 상태

- Engine 정식 Debug Build exit0, 실제 Engine.dll/lib20:20:29 갱신. 첫ClCompile;Link 명령은
  OBJ 컴파일만 수행하여 이후 정상Build로 실링크를 확인했다. 증거는
  out/MovieSoundSync20260929/engine-product-build.log이다.
- SDK 갱신과 Client ClCompile exit0. client-final-compile.log에 기록했다. 기존 C4819 경고는
  남았으며 C++ 인코딩과 개행은 유지했다. 새 C++ 파일/project 등록은 없다.
- Composition World Sound focused tests2개 PASS. source-in 기본0/저장값 보존, 음수·소수·
  bool·초과 범위 거부를 검사했다.
- 실제 Product OBJ·FMOD 채널 fixture와 최종 Client 설치, 가디언 데이터 게시 결과는 후속
  검증 아래에 기록한다. 현재 실행 중 Client는 종료하거나 조작하지 않았다.

가디언 사운드·창술사 잔디의 원본/후보/게시 증거는 같은 작업의
09-25 FOUR_CLASS_SELECTION_MOVIES RESULT 후속 항목과 연결한다. 사용자의 실제 청감과
잔디 화면 판정은 자동 검사와 구분한다.

### G14 실제 Product 사운드 검사

실제 변경 Client OBJ와 새 Engine/Bin/Debug/Engine.dll을 연결한 headless FMOD fixture에서
51 assertions PASS, exit0을 확인했다. 실제 DLL 경로도 검사했으며 채널은 master mute/paused로
검증했다. source-in 저장 왕복과 malformed rollback,2700ms ahead 동기화,50/100ms 보존과
101ms seek, 뒤처진 cursor·종료된 채널 복구, pause/single voice, 일반 World 비개입, loop/end,
Clear ownership reset을 실행했다. 증거는 out/MovieSoundSync20260929/run.log 및
product-inputs.json이다. 실제 청취·GPU 장면을 검증한 것은 아니다.

Guardian 후보2 WAV와 soundTracks를 최신 revision3에서4로 병합하고 WorldSequences
Publish를 완료했다. 나머지 JSON 바이트/카메라 값은 보존했으며 Runtime SHA256은
ed4e013958dac79c90418b227c72af50228ad31ba29153461e7b931c740eac21이다.
원본2stem과400ms fade, byte patch/atomic install 및 GBResources 동일 복사 증거는
out/ClassMovieSoundRestore20260929/guardian의 candidate/validation.json, install-receipt.json,
publish.log에 기록했다. 후속 Artist 교정 게시에 따라 문서 revision/hash는 추가 변경될 수 있다.

## G15. 자유 시점에서 카메라 키 포즈 가져오기

Sequence Camera Tool의 Use free cam pos가 선택key의 Eye/LookAt/Up만 현재 자유 카메라의
View 역행렬에서 가져온다. MainApp typed callback은 현재 CharacterSelect, free/follow 및
presentation override 상태를 확인한다. 저장된 Movie sample을 대신 쓰지 않는다. model-relative
row는 world pose를 잘못 넣지 않도록 버튼을 비활성화한다. time/ID/FOV/cut/easing/보간은 보존한다.

키 개수를 표시하며 기존 Delete key로 중간 키를 줄일 수 있다. 첫 키 보호와 Revert는 유지하고
사용자의1003개 키를 자동 삭제하지 않았다. 키를 이동하거나 삭제하면 인접 구간의 경로가
달라지지만 기존 sampler와 보간 설정은 그대로 사용한다. ALT V shared editor 호출도 유지했다.

실제 sampler CPU fixture41개 PASS: 정상 pose교체,8종 비정상basis의 atomic rejection,
missing key/up 개수 거부,시간/FOV축/Linear·Catmull/easing/cut/endpoint보존,중간키삭제와첫키보호.
변경3TU 및ALT V호출부를 격리 ClCompile exit0으로 검사했다. out/CameraKeyCapture20260929의
validation.json, test.log, ui-compile.log가 증거이며 제품 UI를 실행한 검증은 아니다.

## G16. 다섯 클래스의 PCM clipping과 원본 mix bus 복구

도화가의 두 음원은 원본 Layer의 음성과 효과음이며 중복 재생 버그가 아니다. 원본 WEM을
float로 다시 decode하면 peak1.2301/1.1431인데 기존 PCM16 변환에서 각각5654/2270샘플이
잘려 있었다. 두 원본의 합은2.156918이다. 음원을 하나 지우거나 기존 clipped WAV를 float로
바꾸는 것으로는 해결되지 않는다.

원본 INIT에는 bus1635194334→393239870(-2dB)→3803692087(-2dB)와 활성 Peak Limiter가
있다. threshold-5dB,ratio10,lookahead15ms,release100ms,output+3dB,stereo-link를 확인했다.
다섯 클래스 모두 같은 bus를 사용하고 추가 local gain/pitch/delay/RTPC/state는 없으며 Guardian
Play의 fade400ms만 적용한다. Tools/SoundPipeline/restore_layered_movie_audio.py와 recipe5개가
원본 HIRC·INIT·WEM 해시를 고정하고 원본 float에서 다시 처리한다.

동시 stem 합으로 계산한 공통 envelope를 각 stem에 분배하거나 기존 단일 mix로 저장했다.
첫 샘플 위치와 frame 수, Sound stable ID·개수·start/duration/source-in/volume을 보존했다.
Lance는 기존 파일 끝의 무음1frame(0.0227ms)만 명시적으로 유지했다. 상대 gain/time이 바뀐
저장본은 자동 덮어쓰지 않는다. 새 asset ID8개를 설치하고 원래 WAV는 보존했다.

| 클래스 | 유지한 Sound 박스 | 복원 후 합산 peak | 잘린 샘플 |
|---|---:|---:|---:|
| Artist |2|0.867728|0|
| GuardianKnight |2|0.861227|0|
| DimensionMaster |2|0.821700|0|
| LanceMaster |1|0.864067|0|
| Warlord |1|0.841235|0|

이 처리는 원본 파라미터에 맞춘 offline 근사다. 실제 Wwise DSP의 bit-identical 결과, 다른
게임 소리까지 포함한 master mix, 동적 상태를 복원했다고 주장하지 않는다. source-time envelope는
Movie slowmo와 함께 늘어나므로 live wall-clock limiter와 다르다. 이후 두 박스의 상대 시간이나
gain을 변경하면 envelope를 다시 검토해야 한다. 별도 runtime 오디오 경로와 전역 볼륨 변경은 없다.

Python focused6개, 모든 sample의 유한성·공통 envelope·lookahead/release·끝 경계·박스 보존,
프로젝트/GBResources8개 SHA 일치를 확인했다. FMOD NOSOUND로8개 모두 stereo32-bit PCMFloat와
frames×8bytes를 읽었다. Product 사운드 fixture는 Artist float 채널의 admission/2700ms paused
seek/.24pitch/두 voice까지 추가하여 총56 PASS다. 원본과 수치 증거는
out/MovieSoundSync20260929/{artist,guardian,dimensionmaster,lancemaster,warlord}-restored 및
all-class-installed-validation.json, run.log에 있다. 실제 청감 판정은 사용자가 한다.

최종 WorldSequences Publish는 source/runtime SHA256
6b8cac11b11face85bb07ab8f4b2876f7b9d761c965317f6846f0c51bb689bf4로 완료했다.
통합 게시 로그는 out/MovieRaidFinal20260929/world-movie-final-publish.log다.


## G17. Movie 1회 재생 후 마지막 장면 유지 (2026-09-29)

SCENE.repeatMovie(optional bool, 생략=true)를 실제 parser와 저작 owner에 연결했다. 현재 Guardian,
Artist, DimensionMaster, LanceMaster, Warlord의 repeatMovie=false만 최신 카메라 문서에 추가했다.
원자 교체와 백업/hash 검증, Intro/Loop 내부의 키·시각·clock·Effect 불변은
out/MovieRepeatHold20260929/policy-install-receipt.json에 기록했다. 기존 반복 데이터는 보존했다.

Update는 마지막 유효 source sample을 유지하고 completedHold에 진입한다. exact duration은
World actor를 wrap/hide할 수 있으므로 기존 float nextafter 경계를 사용한다. 모델/카메라/Effect
owner를 해제하지 않으며 F6 inspection은 계속 처리한다. 완료 뒤 Resume는 차단하고 새 Play/Seek와
Stop에서 완료 상태를 초기화한다. Effect selection 시작 실패 후 rollback에서도 기존 완료 상태와
사운드 종료를 복원한다. Save movie의 기존 Stop 동작은 유지하여 저장 뒤 Play All로 확인한다.

WorldSequencePlayer.Finish_Sounds는 해당 owner의 active/retired 채널만 종료하며 연속 샘플의
재생 부활을 막는다. 별도 Product C++/FMOD NOSOUND 검사21개가 PASS다. 무관한 채널 보존,
실패/빈 핸들, tail-only, 반복 완료 호출, 같은 sample/+10ms 연속seek와 명시적 불연속seek를 확인했다.
검증은 out/MovieFinishSound20260929/{validation.json,test.log}를 따른다.

실제 Product parser와 camera sampler로5클래스×Intro/Loop10phase의 마지막 wall/source 경계,
마지막 cut 유효성,120회 고정 위치·시선·FOV·Up 재샘플, legacy repeat=true 및 잘못된 bool 거부와
실패 시 기존 scenes 보존을 확인했다. probe-validation.json에는 사용한 실제OBJ/소스/data SHA가
있다. 이 CPU 검사는 실제 Client의 매frame Tick나 GPU 화면 검증을 대신하지 않는다.

처음엔 실행 중 Debug EXE를 보존하기 위해 별도 OutDir를 사용했다. 변경하지 않은 shader 재컴파일은
해당 작업의 compiler만 중단했으며, 별도 compile/link에서 C++ compile은 성공하고 Shared.lib의
OutDir 전파 경로 때문에 링크가 실패했다. 사용자 EXE 종료 후 정상 Product 빌드로 전환했다.
Debug Product compile/link/deploy는20260929T123342593Z-debug-product.json PASS이며 SkipBuild=false다.
검사 당시 추가 Resources221개는 Desktop/GBResources와 bytes/hash가 모두 일치했고 이번 hold수정은
새 리소스를 추가하지 않았다. resource-mirror-final.json을 따른다.

최종 Release Product compile/link/deploy도20260929T123546398Z-release-product.json PASS이며
SkipBuild=false다. 두 구성 모두 Engine/Shared/Server/Client를 표준 Product 경로로 확인했다.
기존 컴파일 경고와 DirectXTK PDB 부재 경고는 남아 있으나 최종 빌드 오류는 없다.
사용자가 마지막 저장한 SL00 WorldSequences를 다시 게시했으며 source/runtime SHA256은
29d4b78ec53533c0e6601eb5ce5bf0f4e2aed0c6e841a066a480a29714387b8b로 일치한다.
추가 잔디 catalog의 잘못된 UV24행도 별도 Area publisher로 교정·게시했고 실제 Client CPU loader의
source/runtime161개 admission을 확인했다. 세부 증거는09-25 FOUR_CLASS RESULT G13과
out/LanceGrassCatalogFix20260929/install-receipt.json이다. Client 화면은 자율 실행하지 않았다.

## G18. 사용자 재검증 후 첫 주요 연출의 종료 경계 교정

G17 완료 보고 후 사용자는 뒤쪽 Camera box로 여전히 이동하며 잔디도 보이지 않는다고 확인했다.
현재 프로세스는21:33:39 빌드된 Debug Client를21:43:09 시작했고, EXE 내부에 G17 완료 처리와
저장본의 다섯 repeatMovie=false가 있었다. 단순 EXE 미배포가 아니었다. G17은 전체 Intro의 끝을
종료로 잘못 해석했다. 사용자가 요청한 경계는 첫 주요 연출 Camera box 끝이다.

SCENE.holdAfterCameraId가 첫 Intro camera stable ID를 참조하고 실제 Update가 해당 box의 종료를
source clock에서 Movie clock으로 역변환한다. 종료 직전 float source sample을 먼저 선택하여
double epsilon의 float 반올림으로 다음 cut에 진입하는 것도 막는다. paused Seek는 자동 완료를
하지 않아 뒤쪽 box를 계속 편집할 수 있다. Repeat ON과 명시적 Loop, 필드 없는 legacy는 기존
종료 정책을 사용한다. 소스 구현과 최종 EXE 설치·검증 결과는 아래 완료 기록에서 구분한다.

실제 Product Update→Sample_Frame→Sample_Camera CPU fixture711개를 통과했다. 기존 다섯 클래스의
camera/clock을 사용하고 GPU 생성이 필요한 world/material/light/effect만 분리했다. 첫 cut 이전과
교차 직후의 camera ID, 완료 후120회 Update의 동일 clock/pose, 뒤쪽 paused Seek 보존, Resume,
Repeat ON, legacy 전체 종료, 명시 Loop 종료와 다른 stable marker를 검사했다. 잘못된 참조6종의
transactional 거부도 확인했다. test 전용 DLL PATH에 PhysX를 누락한 최초 loader 종료는 경로를
교정한 뒤 재실행해 해결했으며 제품 오류와 구분한다. 최종 probe-validation.json은 exit0이다.

1배속 Movie clock의 첫 연출 종료는 창술사7695.059ms, 워로드8163.213ms, 도화가8596.199ms,
가디언8733.171ms, 차원술사9994.457ms다. camera sampler는 정수ms를 소비하므로 해당 source 끝의
1ms 전 샘플을 유지하고 World/Effect에는 직전 float source sample이 전달된다.
최신 저장본에 다섯 holdAfterCameraId만 hash 재확인·백업·원자 교체로 설치했다. 나머지 JSON 필드는
동일하며 설치 SHA256은 e36691f9bfa2800c711bf93051faa7e1a77daf881aa65eaa38cb43bc28a8c053이다.
검증·설치 기록은 out/MovieFirstCutHold20260929/{probe-validation,install-receipt,runtime-path-audit}.json이다.

최종 Release Product build/deploy는20260929T131955762Z-release-product.json PASS(SkipBuild=false)다.
Client.exe는22:19:53 생성됐고 변경된 C++ OBJ10개·CSO30개·EXE1개가 갱신됐다. 설치된 Release
Binary/MapInstance 및 Binary28개 변형을 실제 CShader로 생성해 wind 입력9개, Clone pass와 reset을
검사했다. 이전 E_FAIL은 두 경로 모두 S_OK로 바뀌었다. 로그는 installed-release-shader-check.log다.

최종 Debug Product build/deploy도20260929T132038835Z-debug-product.json PASS(SkipBuild=false)다.
C++는 앞선 Debug compile 단계에서 갱신했고 Product는CSO30개와EXE를 갱신했다. 설치된 Debug
FX30개도 같은 실제 CShader 검사에서 wind9입력·Binary28변형·Clone pass·reset을 통과했다.
프로젝트의 Debug/Release 최종 EXE에서 holdAfterCameraId 문자열을 확인했다. 실제 Client UI와
최종 잔디 표시 판정은 수행하지 않았다. 검증기는 Client 실행 없이 WARP device를 사용했다.

최종 대조에서 사용자가21:56에 저장한 Lance sound 앞1574ms trim(sourceStartMs1574,
duration6919, World revision13)이 이전 게시본과 달랐다. 이 최신 편집을 보존해 WorldSequences를
게시했으며 source/runtime SHA256은 d6e74a0c1b3e38f599650887307434b5a7893a15d3091475f78b38b1e51cee27로
일치한다. 데이터 변경이므로 추가 C++ 빌드는 필요하지 않았다. final-delivery.json은 두 구성 EXE의
신규 marker 코드, 구성별30CSO, 실제 바인딩 검사, Camera 후보와 설치본 일치, World 일치와
GBResources221개의 SHA 동일성을 확인했다. Debug EXE22:20:36, Release EXE22:19:53 생성이다.


## G19. 카메라 감속과 독립 Movie 사운드 시계

2026-09-29 사용자가 감속 구간의 음향 왜곡을 보고하여 Sound cursor/pitch/drift/trim을
카메라 source clock에서 분리했다. source→Movie 매핑은 cue 시작에만 쓰고 WAV 길이·
재생 위치는 Movie 시간, pitch는 수동 배속을 사용한다. 기존 phase·카메라·사용자 Sound
저장본은 보존했다. Release 실제 Product OBJ+FMOD NOSOUND488검사 PASS이며 실제 청감은
사용자 확인 범위다. 상세 파일·검증·배포 근거는09-29 RELEASE_ENTRY_AUDIO_REPAIR_RESULT,
out/MovieAudioClock20260929/probe-validation.json을 따른다.


## G20. 창술사·워로드 원본 Sound 앞부분 복구 (2026-09-30)

사용자가 G19 시간 분리의 정상 청감을 확인했다. 남은 잘림은 WAV 손상이 아니라 이전 지연을
피하려고 저장한 source-in이었다. 창술사1574/워로드850ms를 건너뛰면서 두 박스 startMs는
이미0이라 Sequencer의 왼쪽 drag는 Movie0 경계에 막혔다. 이 동작을 임의 음수 배치로 풀지 않았다.

SL00 WorldSequences의 stable Sound 두 개를 sourceStartMs0으로 복구했다. 창술사는
6919→8493ms, 워로드는8522→9372ms다. 실제44100Hz float WAV의374520/413270frame
길이를 올림한 원본 전체 구간이며, 기존 startMs0·volume1·bus-restored WAV 경로를 유지했다.
카메라·다른 track·instance·리소스 실물과 G19 C++는 변경하지 않았다.

정본 revision13→14, 기존 바이트를 out/MovieSoundUntrim20260930에 백업한 뒤 최신 hash
재확인·동일 폴더 임시 파일·원자 교체로 저장했다. 정본을 field 단위로 비교해 두 Sound의
sourceStartMs/durationMs와 root revision 외에는 바뀌지 않았음을 확인했다.
Publish-MapAuthoring.ps1 -AreaId LV_LOBBY_CLASSSELECT_SL00 -Scope WorldSequences -Mode Publish
exit0이며 source/runtime 전체 JSON 의미가 같다. WAV의 SHA256도 전후 동일하다.
근거는 out/MovieSoundUntrim20260930/restore-receipt.json, validation.json, publish.log다.

현재 실행 중 Debug Client는 종료·UI 조작·Reload하지 않았다. 사용자 편집창의
Reload saved movie는 복구된 source를 다시 읽으며, 다음 Level 진입은 게시 runtime을 사용한다.
코드 변경이 없어 추가 컴파일이나 전체 하네스 재실행은 하지 않았다.

G20의 새 데이터까지 같은 Desktop ZIP에 반영 완료했다. 최종stage는
`out/ReleasePackaging/20260930-movie-full-audio`, ZIP크기는166876247bytes,
SHA256은`a8a515153db60fc732e1c3def0ceb7e4207e93a5183c6236fdb831c78beb27fb`다. CRC/전체manifest/preflight와 source/runtime 두파일의
ZIP내 해시 대조PASS다. 영수증은out/MovieSoundUntrim20260930/package-receipt.json.
Resources·EXE변경은없고 기존ZIP은백업됐다.

## G21. 차원술사 첫 구도 기준 후속 카메라 보정과 반복 복구 (2026-09-30)

사용자가 모든 편집을 저장하고 반영을 승인한 최신 `ClassSelection.cinematics.json`에서
차원술사 Intro Camera2·3·4·5의 첫0ms 키를 기준으로 후속 포즈를 보정했다. 네 첫 키의
Eye/LookAt/Up과 나머지 필드를 정확히 보존했다. 수정 전 기준은 직전 movie 백업이며 해당
Intro/Loop camera 배열이 HEAD와 같음을 확인했다. 다른 class, Intro Camera1과 사용자가
추가한 excludedWorldObjectIds를 포함한 World·Effect·Sound·material·light 필드는 그대로다.

각 컷의 이전·새 첫 camera basis 사이 회전으로 상대 Eye 위치, 시선 벡터와 Up을 변환했다.
Use free cam pos의 약10길이 LookAt 벡터와 원본의 약1길이 차이는 컷마다 일정한 시선 길이
비율로 처리했다. 카메라 경로의 거리·시간·FOV는 확대하지 않았다. 대응하는 Loop Camera1~4도
자기 원본 첫 키·곡선·FOV를 사용해 Intro2~5의 새 기준 포즈로 각각 보정했다. 총8컷에서
사용자 첫 키4개를 유지하고1,163개 키의 포즈를 바꿨다. 수정한 Loop source는 PROJECT_TUNED로
표시했고 차원술사 repeatMovie만true로 바꿨다. holdAfterCameraId는 보존되며 Repeat ON에서는
정지 경계로 소비되지 않는다. 전체 키의 stable ID, timeMs, cut, 보간, easing, FOV와 source clock은 보존했다.

실제 `CClassSelectionPresentation::Parse`와 `CEffectRecoveryCamera::Sample`을 사용하는 별도
CPU 콘솔 검사에서8컷의 전체41,668ms를1ms 간격으로 샘플링했다. 부동소수점 런타임 기준
예상 회전·평행이동과의 최대 Eye 오차는0.4573mm, 시선 방향은0.0197도, Up은0.00374도다.
키 사이 Eye 거리의 double 계산 오차는4.4e-13m 미만이며 경로 배율은1이다. 첫0→1ms의
최대 Eye 이동은0.0229mm, 시선 변화는0.00888도여서 첫 키 직후 예전 구도로 복귀하는
불연속이 사라졌음을 수치로 확인했다. FOV·키 시간과 네 기준 포즈의 정확한 보존도 검사했다.
이는8컷의 반복 표본 검사이며 별도 기능 수나 화면 합격 수로 집계하지 않는다.

검토된 후보를2026-09-30 21:42 KST에 최신 저장본 hash 재확인과 Windows ReplaceFileW로
원자 반영했다. 실제 교체된 파일을 백업하고 그 bytes가 직전 저장본과 일치함을 확인했다.
설치본을 다시 읽어 같은 실제 parser/sampler 검사를 통과했다. 반영 전 SHA256은
`d85aef471bbd5a738e637b25452860acbc65a16009525ba72b7f4abf58bc5d41`, 설치본은
`dd61e689964c861d4758f547aa4852ae8c5f9e0451be0260402f67cf6bde5b30`이다.
근거는 `out/DimensionMovieRebase20260930`의 `candidate-validation.json`,
`stable-field-patches.json`, `focused-proof-summary.json`, `install-receipt.json`과
`installed-20260930T124247586321Z/installed-parser.log`다. 같은 폴더의 실제 백업은 보존했다.

기존 컷 전환 시각과 즉시 전환 방식은 유지했다. 사용자가 각 컷을 독립적으로 다시 잡았으므로
Loop 마지막→처음 컷의 Eye 간격은6.5678m, 시선 방향 차이는50.4607도다. 원본도 해당 경계에서
즉시 컷을 전환하며, 이번에는 컷 사이 블렌딩이나 무봉합 연결을 추가하지 않았다. 화면 구도와
전환의 자연스러움은 사용자 확인 대기다. 추가 Product 빌드·Client/Server 실행·종료·UI 조작·
Reload·GPU 검사는 하지 않았다. 실행 중 메모리 초안과 Server는 자동 갱신하지 않았으며,
사용자는 기존 WORLD 편집기의 `Reload saved movie` 후 차원술사 `Play All`로 설치본을 확인한다.


## G22. 차원술사 정적 배경의 소유자와 Map Tool 편집 연결 (2026-09-30)

첨부 화면의 하늘은 Movie WORLD Object가 아니라 `LV_LOBBY_CLASSSELECT_SL12`의 정적 배치다.
기존 `Pick in scene`은 WorldSequence sampled object만 검사했으며 MapTool도 primary SL00만
빌렸으므로 사용자가 보던 배경에 도달할 수 없었다. Show background는 SL12 전체의 임시 필터다.

실제 설치 WModel을 읽은 결과 sky_multi_sm 후보1876/1949는 각각8정점·4삼각형의 유한 사각형이며
deferred opaque 경로다. source native1112/1157/1115/1116 shader 함수에는 clip/discard가 없고
native dispatch는 generic diffuse alpha discard 전에 반환한다. 새 Intro2~5 첫 구도를 기준으로
16:9 CPU ray81개씩 검사했을 때1876은34/10/4/12개 hit,1949는모두0개였다.1876의 아래 외곽이
화면에서 사선을 만들어 새로운 카메라가 평면의 범위 밖을 보게 되는 근거를 확인했다. 정확한
첨부 당시 자유카메라와 다른 물체의 occlusion은 재현하지 않았으므로 최종 GPU 화면 판정은 별도다.
배치의 stable ID는 `LV_LOBBY_CLASSSELECT_SL12:export:1876`, asset은
`SL12_82D2D1E4E4F51A1AF5968E66`, 모델은
`Map/LV_LOBBY_CLASSSELECT_SL12/Models/f95dbb4bacb9f3b9/f95dbb4bacb9f3b9.wmodel`이다.

Level_CharacterSelect는 이미 로드한 배경을 stable Area로 조회하는 Debug getter만 제공한다.
ClassMovieInspector는 배경 Area와 source 경로를 표시하고 `Edit background in Map Tool`을
MainApp의 typed callback으로 연결한다. 현재 class·Area·runtime을 재확인한 뒤 기존 MapTool을
열고 현재 구도를 자유 카메라로 넘긴다. 성공 시 기존 WORLD pick만 취소하고 MAP 입력과 포커스를
선택하며 Movie 시계와 Sequencer 선택은 유지한다. 버튼은 WORLD picking의 범위를 명시한다.

MapTool의 `Character Select target`은 primary와 현재 Movie 배경을 구분한다. 고정 Area ID를
stage/commit하므로 class를 변경해도 저장 대상이 다른 배경으로 자동 전환되지 않는다. dirty
초안의 target 변경을 거부하고 배경이 재생 중이 아니거나 전체 preview가 꺼지면 편집을 막는다.
Hierarchy에 이름·asset ID·sourcePlacementId 검색을 추가했고 기존 Position·Rotation quaternion·
Signed scale·Visible 편집을 재사용한다. 검색이 비어 있으면 기존 clipper의 직접 index 경로를
유지한다. 배경 mode는 placement 편집으로 한정해 다른 Area의 gameplay/sky-phase를 적용하지 않는다.

Save placements는 기존 `Data/Maps/Authoring/<Area>/<Area>.mapplacements` linked transaction을
사용한다. 존재하지 않고 수정되지 않은 optional WorldSequences는 새로 쓰지 않지만 기존
freshness·backup·verification·rollback 경계는 유지한다. 원래 존재하거나 편집한 sequence는 기존
저장을 따른다. 저장한 배경의 실행 데이터는 기존 `Publish-MapAuthoring.ps1 -AreaId
LV_LOBBY_CLASSSELECT_SL12 -Scope Placements -Mode Publish` 대상이다. 이번에는 편집 연결만
추가했으며 하늘 위치·크기·가시성·재질·렌더링 옵션과 source/runtime map 데이터는 바꾸지 않았다.
새 runtime이나 저장 schema, C++ 파일과 project/filter 항목은 추가하지 않았다.

검증 증거는 `out/ClassMovieBackground20260930`에 있다. actual CMapAssetCatalog::Load_Source와
CMapPlacementDocument::Read/Write로1461배치를 읽어1876만 위치(+.5,-200,+.25)·visible=false로
out 파일에 저장·재로드했다. 나머지1460배치의 모든 필드와 대상의 비수정 필드가 정확히 같았고,
원래 값으로 되돌린 out 저장본도 원본의 parsed record와 일치했다. 실제 Data 원본과 게시 파일의
hash는 전후 동일하다. `placement-codec-verification.json`, `placement-source-hashes.json`,
`sky-geometry-audit.json`으로 입출력·geometry 근거를 구분한다. 이는 제품 codec 검사이며 실행 중
MapTool 입력 조작이나 GPU 표시 검사가 아니다.

Level getter와 linked save TU는 격리 Debug/Release 컴파일을 통과했다. MainApp·Inspector의
격리 Debug 컴파일 근거는 `out/DimensionMovieRebase20260930/inspector-route/compile-validation.json`이다.
기존 include의 C4819 경고는 남아 있다. MapTool 최종 컴파일·집중 전환 검사·Product 빌드 결과는
아래 후속 검증에 기록한다. 현재 Product guard는 Debug Client61832와 Server84960 점유를
확인했으며, 사용자에게 저장·종료를 요청했다. 프로세스 종료와 UI 조작은 자동 수행하지 않았다.

G22 후속 검증: MapTool.cpp/MapTool_Area.cpp의 격리 Debug·Release 컴파일4개가 통과했다.
실제 Open_ClassMovieBackground/Can_ChangeRuntimeStructure 본문을 추출하고 Level/runtime/stage
경계만 대체한 CPU 검사15개에서 dirty 전환 거절, camera draft 거절, 현재 class·runtime 오류,
같은 dirty target 유지, class 변경·Stop·Show background OFF의 편집 차단과 명시적인 clean
primary 복귀를 검사했다. 전체 Switch 실행이나 GPU를 검사한 것으로 집계하지 않는다.
`maptool-validation.json`, `maptool-target-probe.cpp/.log`에 근거를 기록했다. 실제 source/runtime
SL12 문서는1461개의 ID·asset 매핑·TRS·visible 의미값이 모두 같고, 서로 다른 파일 해시는
직렬화 차이임을 `placement-source-runtime-comparison.json`으로 확인했다.

독립 코드 검토에서 optional Load_CameraShots가 Switch의 commit 전에 기존 camera 상태를
바꾸는 경계를 발견했다. 최종 target commit 이후로 이동해 다른 source 검증 실패 시 기존
camera owner도 보존하도록 보완하고 해당 TU의 Debug/Release 컴파일을 다시 통과했다.
사용자 종료 답변 뒤 Client/Server process가 없고 G21 카메라·리벤지 스피어 설치 hash가
유지됨을 재확인했다. `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`가
Engine·Shared·Server·Client 컴파일/링크/배포 및 포함된 runtime 입력 검사를 통과했다.
receipt는 `out/BuildPipeline/runs/20260930T125746734Z-debug-product.json`이며 result=PASS,
elapsedMs=56862다. 기존 C4819/C4828과 DirectXTK PDB LNK4099 경고가 남았고 오류는 없었다.
이 Product 작업은 data publisher를 실행하지 않았다. 빌드 후 G21 카메라·리벤지 스피어의
설치 hash와 배경 source/runtime의 기존 hash를 `final-installed-hashes.json`으로 재확인했다.
현재 Debug 실행 파일까지 반영 완료이며, 새 Release EXE 빌드와 Client/GPU 화면 판정은 수행하지 않았다.


## G23. 저장된 다섯 클래스의 후속 Camera·Loop 보정 반영 (2026-09-30)

사용자의 추가 저장본 SHA7fca8ba0af04ddc24708768e17d654465895c22a573d8cc71019006c80257e36을
G21 설치본과 비교했다. 차원술사 Intro2, 가디언나이트·도화가·워로드 Intro2~5, 창술사 Intro2~4의
첫0ms pose16개만 변경되었다. 차원술사 Intro3는 이전 설치본과 같아 중복 보정하지 않았다.
워로드의 새 excludedWorldObjectIds(a1936 포함)와 모든 사용자 기준 키를 유지했다.

실제 source export와 컷 순서를 확인해 Intro/Loop32컷의 후속 pose8,557개를 보정했다.
사용자 anchor16개는 exact 보존하며 대상 총8,573키다. Loop 자체의 키·FOV·시각·clock을 사용하고,
창술사·워로드의 편집하지 않은 Intro1/Loop1은 그대로다. 다섯 클래스 모두 repeatMovie=true로
설정했으며 hold marker는 보존했다. pose25,671/source15/repeat4의 총25,690필드만 변경했다.

실제 제품 parser와 CEffectRecoveryCamera/ValtanCinematicCameraController sampler의
557,179표본에서1,679,442수치 검사를 통과했다. 최대Eye오차0.000620896m, 시선0.033575도,
Up0.004132도이며 첫1ms 이동은 최대0.000020385m였다. 실제 구현을 재사용한 CPU검사이며
UI/GPU 화면 합격을 뜻하지 않는다. 설치 스크립트의 stable-field CAS와 입력 의존 guard는
무관 제외목록 병합을 허용하고 기준키·새로 편집한 비대상컷·뒤 pose·repeat 충돌을 거절했다.

22:46 KST에 최신 저장본을 재확인하고 ReplaceFileW/실제교체본백업/ownedrollback 경계로
반영했다. 설치SHA는2f7efcb35f86572c4c17b12e9b01c0c45c6f1abade1bb3cdbf51dfae9121281a이며
설치본의 actual parser/sampler도PASS다. 근거는 out/ClassMovieCameraBatch20260930의
candidate-validation.json, loop-mapping.json, focused-proof-summary.json,
install-guard-validation.json, install-receipt.json과 installed-20260930T134613893969Z다.

기존 hard cut을 유지했으므로 서로 독립적으로 잡은 구도 사이의 경계는 연속이 아니다.
워로드 Intro3→4는7.25862m/143.12895도, 차원 Loop wrap은12.13485m/48.95903도다.
카메라 구도·컷 전환의 사용자 화면 판정은 대기이며 자동 블렌딩은 추가하지 않았다.
디스크 반영과 열려 있는 Movie draft는 구분한다. 에이전트는 Reload·Client/Server 종료·실행을
하지 않았으며 사용자는 Reload saved movie→Play All로 새 반복과 구도를 확인한다.

## G24. Character Select F1 상단의 자유 카메라 속도 (2026-09-30)

기존 speed UI는 F1 아래쪽의 접힌 Camera에 있었으므로 Character Select에서 찾기 어려웠다.
같은 RenderCameraSpeedControls를 현재Level 표시 바로 아래에서 한 번 호출하고
Character Select Camera 헤더를 기본으로 펼쳤다. 다른 Level의 위치는 유지했다.
CurrentCamera는 이 Level의 실제 소유 camera를 Debug/Release 공통 getter로 읽는다.
기존0.1~400m/s slider·Reset20·Shift배율30과 speed getter/setter를 그대로 사용한다.
Movie clock/재생 배속은 바뀌지 않고 자유카메라 이동 속도에만 적용된다.

MainApp.cpp/MainApp_Dragon.cpp/Level_CharacterSelect.h의 기존인코딩을 보존했다.
새 파일·runtime·project/filter 등록은 없다. 두CPP의 Debug/Release 격리컴파일4개PASS,
실제 helper와 CCamera_Free setter 본문을 사용한 Debug9/Release9 CPU검사PASS다.
UI입력/Level 경계는stub이므로 실제 F1 조작이나 화면검증으로 기록하지 않는다.
근거는 out/ClassSelectCameraSpeed20260930/camera-speed-validation.json과
camera-speed-only.diff다. 사용자가 현재 Client/Server 사용을 계속하며 빌드를 나중으로 미뤘다.
새 Product EXE 링크는 미실시이고 기존 실행 파일은 유지한다. F1 UI 변경은 다음 빌드부터 적용된다.

G23 독립 검토에서32컷의 별도 변환 수식과 설치값 차이는 최대4.55e-13이었다. 미대상13컷과
Intro 첫 키24개는 저장본과 같다. 차원술사의 반복 보정은 최초 원본→최신 anchor 단일 변환과
최대9.38e-13 차이로 중복 적용이 없었고, Look 길이 비율1.000009147로 다시10배 늘지 않았다.

## G25. 가디언나이트 화면의 회색 배경 후보 확인 (2026-09-30)

가디언나이트의 backgroundAreaId 생략은 Level fallback LV_LOBBY_CLASSSELECT_SL10을 사용한다.
WORLD 목록은 캐릭터·그림자·용 배우이며, 이 Area의 정적 배치는 Movie WORLD picking 범위 밖이다.
기존 G22 Edit background in Map Tool로 같은 Level 소유 runtime을 편집할 수 있다.

설치 geometry와 실제 source5367ms/movie5818ms 카메라1 pose의81개CPU ray를 비교했다.
배경의 sky_mirror_sm/export704는 구체이며 중앙까지 약548m 떨어져 있다. 화면 가까이 걸리는
구체적인 후보는 lv_anh_horizonold_planecircle02_sm 수면으로 source ID는
LV_LOBBY_CLASSSELECT_SL10:export:811, placementId17522923997188417410,
assetId SL10_3E2567A22176EFC6E268C788이다. 원본 재질은
bg_pcselect14.mat.bg_scd_yorn_water_02_mi_lnh(native237), translucent/opacity0.4/depth_bias-60이다.
실제 수면의 가장 높은Y=-44.56105721은 카메라 EyeY=-44.44331741보다0.11774m 아래다.

전체1356visible placement/81unique 설치 모델 검사에서 오른쪽 아래 ray13/81(source5367),
11/81(movie5818)가 이 수면을0.475~5.80m 앞에서 최근접으로 만났다. 중앙ray는 나무·바위로
약41~49m 앞이었다. 첨부 당시 자유카메라와 저장된 Movie pose의 동일성이 확인되지 않았으므로
회색 영역 전체가 수면이라고 확정하지 않는다. 실제 화면 판정은 사용자 확인 대기다.
Map Tool Hierarchy 검색을 기본sky에서811로 바꾸면 해당 배치를 선택할 수 있다.
Position의 가운데Y는 월드 높이이고 Visible은 해당 배치의 저장된 숨김이다. 이 조사에서는
위치·rotation·visible·재질·렌더링 옵션을 바꾸거나 mapplacements를 publish하지 않았다.
근거는 out/GuardianMovieBackground20260930의 all-map-ray-audit.json과 water-plane-811-summary.json이다.


### G24 후속 — Debug·Release 제품 빌드 완료 (2026-09-30)

사용자가 보류를 해제하고 전체 빌드를 요청했다. 실행 중 Client/Server와 경쟁 MSBuild가 없음을
확인한 뒤 정본 Invoke-BuildAndRegression.ps1 -Profile Product를 Debug, Release 순서로 실행했다.
두 구성 모두 Engine→Shared→Server→Client의 필요한 증분 컴파일·링크·SDK/CSO/runtime DLL 배포와
필수 runtime 입력 검사를 통과했다. Debug44,735ms, Release88,107ms이며 실제 receipt는 아래다.

- out/BuildPipeline/runs/20260930T140358282Z-debug-product.json
- out/BuildPipeline/runs/20260930T140536992Z-release-product.json

F1 상단 카메라 속도 변경이 두 구성의 Client.exe에 포함됐다. C4819/C4828 인코딩 경고와
Release DirectXTK.pdb 누락 LNK4099 경고는 남았지만 빌드 오류는 없다. 기존 유효한 shader tracking을
사용했으며 강제 Clean/Rebuild나 별도 광역 진단은 수행하지 않았다. 이 제품 빌드는 data publisher를
실행하지 않는다. 카메라G23 hash와 다섯class Repeat, Guardian G12 source/runtime 및 mip30개와
미러의 설치값이 유지됨을 확인했다. 실제 Engine.dll과 각 Client 배포본 hash도 같다.

근거는 out/ClassMovieFullBuild20260930/build-summary.json, debug-build.log, release-build.log,
read-only-installed-verification.json이다. 에이전트는 Client/Server/UI를 실행·조작하지 않았으며
최종 구도·Loop 전환·재질 화면 판정은 사용자 확인 단계로 유지한다.

## G26. MapTool 배경 저장 후 재진입 미반영 수정 (2026-10-04)

차원술사 SL12 source와 runtime은 모두 1461개 동일 stable ID를 가진다. source export1876의
sky_multi_sm(placement16549887706206081491, assetSL12_82D2D1E4E4F51A1AF5968E66)은 사용자
저장본의 Y=702.599976, runtime의 Y=1003.07109로 달랐다. 나머지 1460행은 float 직렬화
차이 범위였다. Save는 성공했지만 publisher가 연결되지 않아 다음 진입이 이전 위치를 읽었다.
이 배경은 Movie WORLD Object가 아닌 기존 Level 소유 map runtime이며 임의 legacy 경로는 없었다.

MapTool toolbar에 `Save Data + publish placements`를 연결하고 기존 Save는 `Save Data only`로
명명했다. 통합 버튼은 기존 linked save의 source freshness/backup/readback 검증 뒤 같은 baseline을
다시 확인하고 기존 CMapPublishRunner를 통해 Placements scope를 실행한다. runner 허용 scope에
Placements만 추가했으며 새 publisher나 runtime은 만들지 않았다. source 저장 후 publish 시작 실패와
process 실패는 `Saved ..., unpublished`로 따로 표시하고 source를 보존한다. 게시 중에는 Save·Reload·
명시적 target 변경만 잠그며 목록 선택·카메라·메모리 draft 편집은 유지한다. 완료 후 disk 변경이나
추가 draft가 있으면 별도 안내한다. 성공한 게시본은 다음 Level 진입에 반영된다.

기존 attach는 저장된 source를 inspector draft로 읽되 기존 runtime 표시를 덮어쓰지 않는다.
Movie 배경에 TRS/Visible 차이가 있으면 차이를 안내하고 `Apply saved background placements`를
제공한다. clean draft에서 사용자가 누를 때만 기존 Load_Placements의 stage/commit을 호출한다.
dirty/camera draft는 보존하며 자동 Reload는 추가하지 않았다. 없는 Publish controls와 F2 AssetTest
안내는 현재 버튼/진입 안내로 교체했다. TEAM Area guide에도 실제 저장·게시·preview 반영을 구분했다.

read-only publisher `-AreaId LV_LOBBY_CLASSSELECT_SL12 -Scope Placements -Mode Validate`는
1461 placements/2 files, exit0이었다. 같은 범위 Check는 runtime 미게시 차이로 exit1이며 원인을
정확히 SL12.mapplacements로 지목했다. `out/MapToolSavePublish20261004/source-runtime-evidence.json`
에는 실제 float32 의미 비교와 파일 hash를 기록했다. 구현 전후 source SHA256
b4b4a7e904649be5621c3099d8a256080c1a77bd6e98bd951d2471ce9993053f 및 runtime SHA256
b4c74fc146fca6aaf58b9b66a6c2b56e36b5960e55a419dc9328b03b6a05978d가 유지됐다.
신규 C++ 파일은 없으며 기존 project/filter 등록과 변경 범위 diff --check를 확인했다.

이 G의 담당 에이전트는 실제 Data publish, Product/TU 빌드, Client/UI 실행을 하지 않았다.
통합 root가 컴파일·제품 배포와 최신 저장본 게시를 조율한다. 사용자 화면 검증은 차원술사 Movie→
Edit background in Map Tool→sky/export1876 선택→필요 시 Apply saved background placements→
Save Data + publish placements 성공 확인→Level 재진입에서 같은 높이 유지다. 게시 실패 뒤
authoring 저장본 보존 및 진행 중 메모리 편집은 코드 경로 검토이며 UI 실측 완료로 기록하지 않는다.

G26 후속 실행 회귀: 실제 Save_AndPublishPlacements, Poll_PlacementPublish와 기존 runner의
Start/Poll/Area 검증 본문 5개를 추출한 headless C++ fixture가 compile exit0, run exit0,
18 checks/0 failures로 통과했다. 실패한 linked save의 spawn 차단, dirty Deploy/sequence 거부,
save 이후 freshness/readback 실패, publisher 시작/종료 실패의 source 보존, 게시 중 추가된 memory
draft 보존, 외부 저장 보존과 완료 한 번 전달, 정확한 Placements scope 및 잘못된 scope/Area 거부를
실행했다. 기존 linked 저장 결과·파일 읽기·Win32 process/file handle만 대체했다. 실제 linked save
I/O·PowerShell 실행·GPU·UI 검증으로 집계하지 않는다. 원본 추출 스크립트와 C++ 원문, compile/run log,
함수별 source/body SHA 및 결과는 `out/MapToolSavePublish20261004/production-body-fixture*`와
`run_production_body_fixture.py`에 있다. fixture의 가짜 owner 필드명과 production local 변수명의
충돌에 따른 C4458 경고 2개가 있으며 제품 소스 경고로 집계하지 않는다.

동시에 WorldLevel→WorldScene handoff를 독립 검토했다. host Area 불일치는 도구를 열기 전에
거절하고, live 범위 밖 placement는 Select 호출 전에 거절해 같은 host의 이전 선택을 유지한다.
Esc/우클릭/입력 owner 상실은 기존 world picker의 armed 상태만 취소하며 선택을 지우지 않는다.
Level/Area/runtime generation 변경 시 기존 선택을 비우는 경계는 의도된 수명 처리다. handoff는
별도 placement session을 만들지 않고 기존 WorldScene 선택·one-shot picker를 사용한다.
현재 diff에서 새 P0/P1 결함은 확인하지 못했다. 이 handoff 검토는 코드 조사이며 UI 실행은 하지 않았다.

## G27. 차원술사 배경의 현재 저장 위치 게시 (2026-10-04)

통합 root가 G26 구현과 컴파일 검증 뒤 최신 디스크 저장본의 SHA를 다시 확인했다.
기존 source/runtime 배치와 catalog를 out에 백업한 뒤 공식 Map publisher의
`-AreaId LV_LOBBY_CLASSSELECT_SL12 -Scope Placements -Mode Publish`를 실행했다.
Publish와 후속 Check 모두 1461 placements/2 files, exit0이다. export1876의 저장된
Y=702.599976이 게시되었고 source/runtime mapplacements bytes가 같아졌다.
source SHA는 b4b4a7e904649be5621c3099d8a256080c1a77bd6e98bd951d2471ce9993053f로
유지된다. catalog 및 Data/Rendering 정본4개의 SHA도 그대로다.

근거는 `out/MapToolSavePublish20261004/apply-saved-sl12/result.json`, publish.log,
check.log 및 before 백업이다. 실행 중 도구를 Reload하거나 Client/UI를 실행하지 않았다.
게시 데이터는 다음 Level 진입에서 소비되며 실제 구도 확인은 사용자 단계다.
