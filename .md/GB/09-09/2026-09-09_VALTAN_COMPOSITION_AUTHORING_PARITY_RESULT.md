# 발탄 Composition 저작 흐름 조사·계획 결과

작성일: 2026-09-09. 갱신: 2026-09-28. 현재 구현 상태는 아래 재개 결과를 따른다.

## 2026-09-28 G30: trim 뒤 stale Detail 재저장과 V2 clip orphan 방지

Timeline 오른쪽 trim은 정상적으로 Balance draft를 변경했지만, 뒤이어 같은 frame의 Detail이 이전 immutable Pattern에서 cue를 다시 가져왔다. 이후 Save의 pending Detail 적용이 이를 사용자 수정으로 오인해 이전 종료 시각으로 되돌릴 수 있었다. `Save 19 is running` 자체는 기존 비동기 job 상태이며 이 버그를 그 문구의 직접 발생 원인으로 기록하지 않는다.

`Render_Details`는 mutation이 허용된 frame에서 실제 Balance generation이 바뀐 경우에만 local Pattern snapshot을 다시 읽고 현재 selected stage를 resolve한다. 이 호출의 Detail 초기화와 Apply가 같은 최신 cue를 소비하며 다른 pane의 immutable cache는 유지한다. idle frame은 query가0이고, readonly/stale frame의 기존 조회 화면도 보존한다. 최신 조회 실패는 Detail 값을 다시 채우지 않고 메시지를 표시한다. stage가 삭제됐으면 기존 Pattern Root 분기로 돌아간다. Source pin/freshness와 Save writer/CAS, 비동기 job 처리는 수정하지 않았다.

Animation 삭제에는 Sound cascade만 있었고 V2의 clip reference 검사가 없었다. 현재 V2 API는 삭제 실패 뒤 baseline/dirty/revision과 기존 stable ID를 함께 복원하지 못하므로 안전한 자동 cascade를 추가하지 않았다. exact Pattern/Stage/action/CLIP_OCCURRENCE/clip ID 참조가 남으면 V2 binding ID와 먼저 지울 box 안내를 표시하고 Animation·Sound·V2 모두 변경하지 않는다. complete snapshot이 없을 때도 삭제를 거절한다. 기존 V2 box 삭제 뒤 Animation을 지우고 한 번 Save하는 경로를 사용한다. 이 변경은 저장 파일의 기존 orphan을 자동 삭제하지 않는다.

실제 production `Apply_EffectOccurrenceTiming`, pending 판정·Detail 적용 함수, Detail 진입/초기화 및 Save pending 블록을 추출한 native regression52조건을 통과했다. 원래 코드는2500→1400ms trim 뒤 같은 frame Detail→Save에서2500ms로 복귀하며 typed update가2회였다. 수정본은1400ms를 유지하고 update는1회였다. 연속 trim, left-trim의 offset·다른 Stage 이동, 사용자 pending position 보존, idle/readonly 무조회, 조회·validation·typed update 실패 보존, 삭제된 Stage, V2 exact/무관한 참조와 미준비 snapshot을 검사했다. fixture의 Balance owner·dependency validator·UI는 주입했으며 실제 Save job이나 파일 writer, Client/UI를 실행한 검사는 아니다.

`ValtanActionWorkbench.cpp` 실제 Debug `/Zs` 최소 컴파일, 독립 code review와 변경 diff check를 통과했다. 근거는 `out/ValtanDetailConsistency20260928/{check.py,result.log,source-tu-compile.log,actual-delta.diff}`다. dirty baseline은 같은 폴더의 `ValtanActionWorkbench.cpp.before`에 있으며 SHA256은 `9b61ee0f7ab4b26a8cab73101124fac2d6ea66d9446b7489c4125815bba56e41`다. UTF-8/CRLF와 다른 미커밋 변경을 유지했고 이 G에서 Data·Resources를 쓰지 않았다. 제품 링크·게시·사용자 Save/Reopen 화면은 최종 통합 검증과 구분한다.

## 2026-09-28 G29: 양끝 편집·Detail 반영·Sound 이동·원본 검색·장판 수신

현재 상태: 소스 수정과 아래 개별 검증 완료. 사용자가 편집 중 교체 보류를 요청한 뒤
“방금 저장본 기준으로 전부 다 반영해줘 exe 종료했어”라고 승인했다. Client/Server 종료와
최신 저작634파일 hash를 `out/ValtanBoxEdits20260928/pre-build-data.json`에 확인했다.
새 Engine Sound API를 포함한 정규 Debug Product compile/deploy를 완료했다.
최종 receipt는 `out/BuildPipeline/runs/20260927T210444128Z-debug-product.json`(PASS),
로그는 `out/ValtanBoxEdits20260928/product-build-final.log`다. Client.exe/Engine.dll/SDK와
수정 decal shader를 설치했고634개 저작파일 hash는 모두 동일하다. 실제 Arena 화면 확인은
사용자에게 남아 있다. 아래 G25~G28의 설치 기록을
이번 G29의 설치 증거로 사용하지 않는다. 사용자 Data/Bloom·미저장 메모리 draft는 교체하지
않았고 Client/Server 자동 실행·종료·UI 조작·Reload·Publish도 하지 않았다.

### Effect와 Sound의 호출 시각·재생 구간

500ms SECOND_SMASH Animation의 cue99ms에 긴 Effect가 붙은 경우, 기존 오른쪽 trim은
종료를 Animation 끝으로 clamp해 약401ms만 남겼다. ONCE Effect는 시작만 실제 clip/Stage
안에서 검증하고 종료는600000ms 범위의 자기 재생 시계로 저장한다. 왼쪽 trim은 호출 위치와
resource `playbackOffsetMs`를 함께 바꾸고 기존 Out을 유지한다. optional offset 부재는 기존
Full Restore의 source 원점을 유지하고 명시0은 리소스0초다. typed tree·Save·projection·
Product parser·runtime·preview에서 값과 존재 여부를 보존하며 Data 일괄 변환은 없다.

Detail Position/Rotation/Scale은 입력 완료 시 typed draft에 반영하고 현재 cursor/paused
상태로 Preview를 다시 준비한다. UI 임시값만 바꾼 뒤 Save에서 누락되던 경로를 연결했다.
유효하지 않은 입력은 이전 draft를 보존하며 사용자 body asset을 수정하지 않는다.
Save 버튼은 선택된 exact V1 Detail의 미확정 변경도 dirty로 표시한다. Save는 모든 pane의
입력 처리가 끝난 뒤 시작하며 마지막 입력이 남아 있으면 revision freshness 검증 후 typed
Apply부터 수행한다. 유효하지 않은 입력은 Save 자체를 취소해 이전 값이 저장 완료로 보이지
않게 한다. 현재 선택과 다른 stale Detail 값은 적용하지 않는다.

Sound의 `playbackOffsetMs`와 `playbackDurationMs`는 실제 음원 시작·길이다. 양끝 trim,
Duplicate, Preview seek, runtime, 기존 Sound Manager의 종료까지 같은 값이 전달된다.
다른 Stage로 이동하면 actual Stage/action/clip을 함께 바꾸고 생성 당시 stable ID는 유지한다.
ID 문자열에 남은 이전 clip을 현재 owner로 해석하던 검증을 수정하되 실제 join 검증은 유지한다.
Animation rate는 호출 시각에만 쓰며 음원 길이를 나누지 않는다. Engine의 기존 Sound cue API에
optional 재생 종료 위치를 추가했으므로 Engine→SDK→Client의 정상 빌드가 필요하다.

Shift 선택은 stable box ID를 사용한다. V1 Effect+Sound 혼합 그룹의 이동·복제·삭제를
원자적으로 적용하며 뒤 항목이 실패하면 두 owner 모두 rollback한다. 단일 복제·삭제도 현재
Preview를 갱신한다. V2/Animation/Logic 등은 선택은 가능하지만 그룹 변경은 전체 거부하고
기존 단일 편집을 유지한다. 그룹 Copy와 EACH_LOOP Sound 양끝 trim은 구현 범위가 아니다.

### 원본 V1 두 개와 장판

기존 N/S `effect.valtan.source.fx_mn_rpbf_00_{n,s}.par_{n,s}_rpbf_dash_01_1` 두 본문은
HEAD/디스크에 남아 있었고 각각6요소다. 한국어 `3회 땅 치기 후 돌진` 검색 alias와
`Patterns/<현재 패턴 이름>/Source Library` 분류만 복구했다. 현재01/02 Product/Full Restore와
별개인 기존 library 원본이며 이 검색 분류가 실제 DASH_CHARGE gameplay cue 연결을 뜻하지
않는다. ID·내용을 대체하거나 같은 asset의 복사본을 생성하지 않았다.

확장 원형 예고 `effect.valtan.tracking-axe.large-circle.warning`은 native2614,
추적 도끼 원본02 `effect.valtan.action.420610.stage021.full.restore`는 native2599다.
원본 projection 높이 안의 발탄 표면이 upward-normal 검사만 통과했다. 기존 쿠크의
`Reject_KoukuGroundWarningReceiver`에 이 두 native family만 연결해 GBuffer marker0/5의
skinned bit8과 source skin/equipment를 제외한다. 정적 Map·다른 profile·Near/Far·Bloom·
색·크기·시간은 유지한다. 기존 근거는09-12 KOUKU_SHOWTIME_WARNING_GROUPS RESULT G03/G04다.

### 현재 검증 증거와 남은 확인

첫 Product 링크는9월24일 `ValtanActionWorkbench_Blueprint.obj`가4인자 Select_Stage ABI를
참조해 실패했다. 해당 CL.read block은 source만 기록하고 변경 header를 추적하지 않았다.
변경된 다섯 typed header의 실제 include closure에서 이전 시점의 소비자는 Blueprint와
RenderingBenchmark 두 OBJ뿐임을 확인하고 `out/ValtanBoxEdits20260928/stale-objects`로
보관했다. 두 객체만 정상 증분 재컴파일한 다음 Product가 통과했다. 새 CL.read는 각각284/322
dependency를 기록한다. 전체 Clean/Rebuild나 source/사용자 Data 정리는 하지 않았다.


- `out/ValtanBoxEdits20260928/result.json`: 실제 DASH_CHARGE source fixture와 production
  typed Add/Update 소비자의23사례 PASS. 500ms clip에서 긴 명시적 종료·offset·position·
  explicit0·invalid range 실패 보존을 확인했다. 임시 probe만 실행했고 source writes는 없다.
- `out/ValtanBoxEdits20260928/compile.json`: 최신 PatternTree/CueAuthoring/BalanceTool
  native syntax compile PASS, 입력 hash 안정. Workbench 최종 compile은
  `out/ValtanTimelineSelection20260928/compile.json`에서 PASS다.
- `out/ValtanSoundTrim20260928/result.json`: production Sound 저장/ID19, 실제 FMOD 무음
  WAV 구간14, Effect runtime source-clock12, 실제 typed Sound placement16, 총61사례 PASS.
  실제 WHIRLWIND SPIN→WINDUP 및 THREE STEP_03 복제→STEP_02의 ID·trim 보존과
  잘못된 위치·뒤 항목 실패·예외 rollback을 확인했다. placement의 Balance admission/
  native clip-window 경계는 주입했고 Effect 자연수명은4초 fixture이므로 실제 시청각 판정은 아니다.
  관련7개 TU syntax compile도 PASS이며 최신 source hash를 기록했다.
- `out/ValtanSaveLatency20260928/source-save-publish-tests.observed.receipt.json`: 기존 실행
  출력에서 SourceSave/Publish 관련8개 테스트의 마지막 PASS를 모은 기록이다. Effect offset/
  finite tail, Sound trim/복제 후 Stage 이동, 무관한 값 보존, 잘못된 입력의 실패 보존을 포함한다.
  한 번에8개를 실행한 suite가 아니며 fixture 수정 전 실패와 재실행도 구분해 기록했다.
- `out/ValtanSaveLatency20260928/sound-optional-alignment-validation.json`: 공식 Validate가
  호출하는 hit-presentation 정렬 검사에서도 두 optional Sound field를 허용했다. 기존717cue
  기반18조건 PASS, legacy/명시0/유효구간은 정렬 결과가 같고 잘못된 값과 unknown field는 거부한다.
- `out/ValtanSaveLatency20260928/valtan-receiver-validation.receipt.json`: 실제 decal
  FXC fx_5_0/O1 컴파일 PASS(CSO431780bytes). 추출한 기존/수정 receiver 함수의 WARP
  RGBA32F Load50,176조합에서 실패0, 차이1,082조합은 정확한 두 profile의 actor뿐이다.
  이 분류 검증을 실제 Arena 화면 판정으로 대신하지 않는다.

`out/ValtanTimelineSelection20260928/result.json`과 `flow-run.log`의 최신 Product 연결
비UI WARP 실행도 PASS다. Detail Position으로 실제 Effect RootWorld가 이동하고 paused
1000ms를 유지했다. typed Source Save patch에 위치가 들어가며 Effect In600ms는 실제
resource age1.6초로 sample되고 원본 Out5000ms를 유지한다. exact/stale/invalid pending
Detail 판정과 Shift 토글/단일선택, V1+Sound 혼합 Duplicate/50ms Move/Delete, 원본 보존,
뒤 Effect 실패 시 앞 Sound 변경까지 두 owner rollback을 확인했다. 그룹 미지원 owner는
변경 전에 거부한다. 저장은 실제 writer용 patch까지 확인했으며 원본 Data 쓰기는 없다.

첫 headless 실행의 Sound Add 거부는 제품 결함이 아니라 MainApp startup의 실제
`CSoundCueCatalog::Load`를 probe가 빠뜨린 준비 문제였다. Preview의 class snapshot과
Add의 startup catalog 소비가 달라 발생했으며 같은 initialization을 추가한 뒤 통과했다.
제품소스 변경·재빌드·실행 파일 추가 교체는 없었다. 최종 전체 `git diff --check` PASS,
저장본634JSON parse PASS, Sequencer 정적 계약14개 PASS다. 실제 화면/청음은 사용자 담당이다.

컷씬5FPS와 Bern 렌더 비용은
이전 G27의 추가 profiler 범위를 유지하며, 이번 장판 수신 수정으로 성능까지 해결됐다고
기록하지 않는다.

## 2026-09-28 G25~G28 최종 Product 설치

아래 G25~G28의 소스 후보는 정규 Debug Product compile/deploy를 완료해
`Client/Bin/Debug/Client.exe`에 반영됐다. 사용자 Client/Server가 모두 종료된 상태를 확인하고
빌드했으며 자동 실행·UI 조작·Reload·데이터 Publish는 하지 않았다. 사용자 저장 이후 체크한
cinematic body 전체52파일과 source Sequence의 hash는 빌드 뒤에도 같다. 그중 현재 실제
다섯 컷씬에 연결되는 목록은51 body/87 occurrence이고 나머지1개는 미사용 actor64 추출본이다.

최종 receipt는 `out/BuildPipeline/runs/20260927T200401607Z-debug-product.json`(PASS),
로그는 `out/ValtanTimeline20260928/product-build-final.log`, 최종 source hash와 보존 검사는
`out/ValtanTimeline20260928/final-verification.json`이다. Engine/Shared/Server/Client 단계를
통과했고 normal deployment를 완료했다. 새 library 검증은
`out/ValtanCinematicLibrary20260928/result.json`(5 groups/87 occurrences/51 unique Effects,
원본 join·시간·flag·UTF-8 label·reload 실패 보존 PASS)이다. 전체 `git diff --check`와 새 프로젝트
XML parse, 최신 Sequencer 계약14개 검사도 PASS다.

실제 빌드에서 확인한 구성/중간 산출물 문제도 해결했다. MainApp의 C1128은 해당 TU에만
기존 프로젝트 방식의 `/bigobj`를 적용했다. 신규 UTF-8 TU의 C2855는 다른 UTF-8 TU처럼
`PrecompiledHeader=NotUsing`을 명시해 CP949 PCH와 분리했다. 손상된 Client link PDB와
이전3인자 ABI를 참조하는 9월24일 Timeline OBJ는 각각 경로를 검증해 작업 out 폴더로 보관한 뒤
그 산출물만 재생성했다. 전체 Clean/Rebuild, shader cache 삭제, 사용자 Data 정리는 하지 않았다.
최종 빌드에는 기존 codepage 및 DirectXTK PDB 경고가 있으나 compile/link 오류는 없다.

남은 확인은 사용자의 실제 화면/소리와 새 profiler capture다. native seek·Append·trim·Sound
clock·Source Save·목록·camera load 검증은 통과했지만 이를 실제 FPS나 모든 GPU 표시의
성공으로 대신 기록하지 않는다. 컷씬의 매프레임126~128ms 지연 직접 원인과 Bern의 렌더 비용은
추가한 profiler scope로 사용자 재캡처를 받아 분리해야 한다.

## 2026-09-28 G28: All Effects 컷씬 목록·Append 대상

`ValtanCinematicEffectLibrary.h/.cpp`는 Arena preview와 같은 다섯 route를 공유하고 기존
WorldSequence loader가 검증한 source instance/template의 effectTracks를 읽는다. 새 H/CPP는
Client 프로젝트와 filters에 등록했다. 원본 Data를 재작성하거나 Effect를 복제하지 않는다.

All Effects → Valtan → `CINEMATIC EFFECTS`에 진입/버러지/2페이즈/피자/사망 그룹을 추가했다.
각 그룹의 공유 Effect 행에 `Open Editor`, `Preview`, `Copy Resource`가 있으며 실제 track별
시작·끝·loop/fit과 원본 Sequence 이름을 표시한다. 검색은 이름·asset·track·Sequence·Pattern ID를
사용한다. 기존 typed resource Open/Preview와 dirty guard를 재사용한다. Copy Resource는
재사용 본체를 Composition에 붙이는 명령이며 컷씬의 actor·placement·timing을 복제하지 않는다.

Composition Resources → V1 → `Cinematics`도 같은 목록을 사용한다. 목록 재로드는 첫 접근과
명시적 Refresh에만 수행하고 Source Save 뒤에는 받아 둔 cinematic snapshot을 재사용한다.
실패 시 이전 목록을 유지한다. 사용자가 저장한 Bloom과 본체 데이터는 변경하지 않는다.

실측 inventory는 진입14/버러지8/phase2 10/피자16/사망39, 총87 occurrence와 고유 V1 본체
51개다. 모든 본체의 카탈로그 등록·JSON 존재·내부 ID를 대조했다. source/runtime WorldSequence와
CameraShots는 JSON 값이 같고 파일 공백 포맷만 다르다. 실제 소비자가 포함하지 않는 별도
actor64 진입 추출 track 두 개를 현재 컷씬에 끼워 넣지 않았다. 증거는
`out/ValtanSaveLatency20260928/cinematic-effect-inventory.json`이다.

Append가 잠기던 화면은 clip 여러 개가 있는 Stage에서 대상이 비어 있던 경우였다.
`ResolveEffectAppendClip`은 유효한 dropdown 지정, 선택 Animation 또는 선택 Effect의 owner,
현재 cursor Animation, 첫 Animation 순서로 대상을 제안한다. 화면에 clip 이름과 occurrence ID를
같이 표시하며 사용자가 변경할 수 있다. Animation 없는 Stage/WAIT와 stale admission은 계속
거부한다. 추가 성공은 기존 typed draft에 새 cue를 만들고 같은 시각·pause 상태로 Preview를
갱신한다. 기존 cue는 보존한다.

`out/ValtanAppend20260928/result.json`의 native16개 사례가 PASS다. 현재
`VALTAN_DASH_CHARGE/WINDUP`의 실제3개 clip을 사용해 selected Effect가 세 번째 clip을
고르는 경우, stale 대상, cursor 경계, 빈 Stage를 검사했다. Balance Add가 위임하는 동일 typed
`CValtanPatternEffectCueAuthoring::Add`로 두 번째 clip에 cue 추가·기존 cue 보존·실패 rollback을
확인했다. catalog admission은 실제 fixture ID 하나로 주입했으며 source Data 쓰기는 없다.
Workbench 최신 TU `/Zs` 컴파일도 PASS다. 새 library와 All Effects 컴파일·전체 Product 설치는
위 최종 설치 기록을 따른다.

## 2026-09-28 G25~G27: Effect 스크럽·끝점 편집·Sound 수명·Save 재개

현재 상태: 아래 코드와 비UI 검증 및 위 최종 Product 빌드·교체를 완료했다.
사용자의 실제 화면과 새 profiler 확인은 아직 수행하지 않았다.
이번 수정은 사용자가 편집 중인 Data 원본이나 Bloom·노출·FXAA 값을 변경하지 않는다.

### G25. 스크럽과 Effect·Sound 박스

`ValtanActionWorkbench.cpp`에서 V1 NATURAL/ONCE 오른쪽 끝을 끌면 `sourceEndMs`와
`cue_end`를 함께 stage한다. 움직임 없는 grip 클릭은 NATURAL을 유지한다. loop clip의 ONCE는
시작을 Stage 안에서 검증하고 종료만 누적 source clock으로 표현하므로 Stage 뒤 tail도 자른다.
`Valtan.cpp`의 실제 cue admission도 같은 끝점 해석을 사용한다. Stage 뒤 명시적 끝이 있는
해당 cue만 `bPreserveBossActionTail`을 사용하며 종료 시각·owner Reset은 계속 강제한다.

UI Render에서 seek할 때 새 V1은 pending 상태인데, 다음 seek가 Late_Update 전에 이를
제거해 보였다 사라지던 순서를 확인했다. `Commit_LocalBossPreviewSpawns`는 해당 local
preview owner만 기존 spawn 경로로 즉시 commit하고 현재 절대 시각·anchor를 적용한다.
reset seek는 이전 Stage의 살아 있는 NATURAL/명시적 tail도 복원한다. source cinematic 이후
뒤로 seek할 때는 먼저 기존 cinematic을 정리해 boss world와 suppression을 복구한다.

Sound는 Preview와 같은 occurrence/loop hash로 WAV variation을 고르고 실제 WAV duration을
더해 박스를 그린다. animation rate는 시작/반복 시점에만 적용하며 WAV 길이를 나누지 않는다.
EACH_LOOP에서 앞 variation의 긴 tail이 마지막 variation보다 늦게 끝나는 경우도 포함한다.
정확한 loop 시각을 위해 기존 native clip duration 조회의 선택적 초 단위 출력을 재사용한다.

검증 증거:

- `out/ValtanTimelineSeek20260928/seek-result.json`, `flow-run.log`: 실제 설치 WModel·effect와
  WARP 비UI probe에서 13개 정방향/역방향 seek, 다음 Late_Update 전 age/object 존재,
  앞 Stage tail, pause 정지, Stop 정리를 확인했다. 5000ms trim은 4999ms까지 존재하고
  5000/5001/5500ms에는 없으며 3000ms 역방향 seek에서 복구된다. suppression 상태의
  boss를 reset seek하면 같은 프레임 cue가 다시 생성된다. cinematic 전체 재생/GPU 검증은 아니다.
- `out/ValtanSoundTimeline20260928/result.json`: 실제 production helper와 공용 clock 함수의
  native 14개 사례 PASS. 설치된 floor-wipe WAV 5개, rate, each-loop variation, Stage 경계,
  긴 앞 tail, 잘못된 duration·반복 budget을 검사했다.
- `out/ValtanTimeline20260928/source-save-tests.log`: 실제 SourceOnly writer 6개 테스트 PASS.
  임시 Data 복사본에서 floor-wipe 5000ms 끝점 저장·재로드, 무관한 field 보존과 잘못된
  끝점 거절을 확인했다. 저장소의 사용자 원본에는 쓰지 않았다.
- `out/ValtanTimeline20260928/sequencer-contract.log`: 수정된 UI/clock 계약 14개 정적 검사 PASS.
  정적 검사를 native 실행이나 사용자 화면 판정으로 대신하지 않는다.

### G26. Save 이후 재개 지연

실제 source save job `job-70404-4`는 staging부터 완료까지 약 1.50초였다. 그 뒤 UI가 이미
받은 source를 다시 읽고 strict Product를 재검사하며 V2와 Effect Tool을 중복 갱신했다.
`Reload_Canonical(true)`는 writer가 commit한 source snapshot을 revision 확인 후 재사용한다.
`Reload_SemanticValtanEffects(false)`는 같은 V2 snapshot에서 표시 metadata만 갱신한다.
source-only save에는 Boss Product reload와 Effect Tool product refresh를 하지 않으며,
후자는 명시적인 Publish 완료로 옮겼다. `BalanceTool`의 즉시 덮어쓰는 Product load도 제거했다.
writer 검증·freshness/CAS·원자 교체·실패 시 기존 상태 보존은 유지한다.

비UI 측정에서 source load는 약 166ms, strict Product load는 364~391ms, V2 reload는 약
664ms였다. semantic reopen은 1231→572ms였다. 이는 개별 경로 측정이며 실제 UI Save의
수정 후 총 지연으로 합산해 주장하지 않는다. 증거는 `out/ValtanSaveLatency20260928`의
compile/read-only 실행 로그다. 새 입력으로 사용자 Save 후의 체감·총 시간 확인이 남아 있다.

### G27. 벽 preset과 실제 phase2 이펙트의 위치

`CValtanBossTool::Set_ServerArenaPreset`에서 환경 preset과 무관한 Pattern canonical graph
gate를 제거했다. publish barrier·활성 Valtan Arena·기존 typed command·Server의 world/session,
destruction/collision/navigation 검증은 그대로다. source 저장과 Product의 차이 때문에 Pattern이
read-only여도 벽 복원·외곽 제거·3시/9시/전체 붕괴 요청을 제출할 수 있다. 관련 TU 컴파일 PASS,
실행 중 Server에 명령 제출은 사용자 확인 전이므로 성공으로 기록하지 않는다.

사용자 스크린샷은 wall clock 4.233초, source cinematic `phase2` 1633ms다. 선택된
`effect.valtan.carrier-v1.mechanic.arena-break-109.takeoff.clip-01`과 화면의 컷씬 이펙트는
다른 연결이다. 실제 정본은 `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/`의
`LV_LUT_HEARTRB_ED.worldsequences.json`, `sequence.LV_LUT_HEARTRB_ED.valtan.source-preview.phase2`,
표시명 `Valtan original phase2 / Event_02`다. 10개 effect track에 고유 V1 asset 7개가 연결된다.
중앙의 두 본체는 아래와 같다.

- `effect.valtan.source.fx_mn_rpbf_00_o.par_o_rpbf_atk_01_07`: 1451ms 시작, 원 둘레 청록색
  기둥/입자를 포함한 13 elements. 원본 반경650cm와 occurrence scale 1.53846으로 반경10m다.
- `effect.valtan.source.fx_q_w_01.fx_par_10.par_q_rpbf_shout_01`: 1317ms 시작, 중앙 충격파/입자
  14 elements. 주변에는 별도 폭발·먼지와 actor75/76 occurrence가 함께 재생된다.

현재 UI 진입은 F1 → `Open Effect Tool V1` → `Data Files` → `Authoring Category: Valtan`
→ `Search skill name or Effect ID`에서 위 짧은 asset 이름 검색 → `Unassigned / Test Effects`
→ 해당 행 더블클릭 또는 `Load Saved Effect for Editing`이다. WorldSequence panel 자체에는
Effect track Open Editor가 없으며 선택된 takeoff 박스로 이 source cinematic 본체를 열 수 없다.

사용자 profiler 41프레임의 평균은197.43ms(약5.07FPS)였다. Composition Build 1089.86ms
스파이크와 이후 DebugTools Update 214~493ms가 확인됐고, Deploy draw도 별도 비용이다.
컬링 counter는 후보4175/표시2914, 후보923/표시153 등으로 변하므로 컬링 전체 미작동으로
단정하지 않는다. `CameraTool::Sample_CompositionPreview`는 모든 Stage가 공유하는 검증된
카메라 문서를 actor preview 세션마다 한 번만 읽는다. Stage별 action/occurrence는 cache key가
아니며 명시 Stop은 해제하고 성공 Save/Reload는 snapshot을 갱신한다. 실제6개 Stage의 서로 다른
action과 Stage-local clock으로479샘플을 실행해 원본6회 load/848.728ms에서1회/124.083ms로
감소했다. pause60회, Stop/새세션/새actor 재로드, camera 없는 구간 해제도 확인했다. 근거는
`out/CutsceneCapture20260928/camera_stage_result.json`이다. 앞선 action 고정477샘플 fixture는
Stage 계약이 부족해 이 결과로 대체했으며 최종 CameraTool `/Zs` 컴파일도 PASS다.

Camera sample/document load, Workbench cinematic/destruction sample, Character Workbench
Update에 profiler scope를 추가했다. 기존 캡처에는 이 분해가 없어 매프레임126~128ms가
발생한 정확한 트리거는 아직 미확정이며 실제 FPS 개선량은 새 실행 파일로 사용자 재캡처가
필요하다. Bern의 NonBlend 렌더 비용과 같은 원인으로 단정하거나 rendering 옵션을 낮추지 않았다.

## 2026-09-28 G23~G24: Full Restore Preview·수명·이동 및 F1/저장 버튼

현재 상태: 사용자의 저장·Client/Server 종료 확인 뒤 최신 저장본을 재검사하고, 두 cue의
Stage 연결 보정과 정규 Debug Product 빌드·Client EXE 교체를 완료했다. 아래 이전 설치·게시와
이번 source-only 보정을 구분한다. 이번 사용자의 추가 이펙트는 서버 runtime에 게시하지 않았다.

### 원인과 반영 코드

- Source Save로 새로 추가한 Full Restore는 기존 Product 선행 준비 목록에 없어서 natural
  duration 조회에서 dispatch를 건너뛰었다. Workbench가 선택한 Stage 경로의 실제 V1 cue를
  기존 preparation queue에 넣고 준비 완료 후 요청한 위치·pause 상태로 재생한다. 준비 실패는
  표시하고 선택·draft·catalog 변경 및 Reset·도구 비활성화 시 예약 재생을 취소한다.
- loop인 WINDUP의 ONCE source clock 950/1475ms를 native clip 833.333ms 바깥으로 잘못
  판정했다. Stage 전체에 유효한 누적 source clock을 허용하되 Stage 밖의 2067/3358ms는 계속
  거부한다. Composition에 독립 Append한 Full Restore는 cue 시각에 document age 0으로 시작한다.
  기존 원본 clip 정렬용 Full Restore의 source seek는 보존한다.
- V1 box는 Playback의 요소·model cue·owner control 및 native particle tail로 끝을 계산한다.
  V2는 leaf/group duration, particle/trail tail, playRate, NATURAL/STAGE_END/CLIP stop을 적용한다.
  무한 반복은 유한 자연 수명으로 표시하지 않는다. Stage 뒤 NATURAL tail을 위해 local Preview
  표현 clock만 연장하며 Server Stage·판정 시간은 바꾸지 않는다.
- V1/V2 drag는 기존 Stage 내부 clamp 대신 drop 위치의 Stage와 clip occurrence를 선택한다.
  sourceStart/playRate 변환, 각 repeat policy와 anchor 등 무관한 필드를 보존하며 유효하지 않은
  위치나 stale draft에서는 기존 값을 유지한다. V2는 narrow typed move API로 binding ID를 유지한다.
- F1 → `Valtan Arena`에 Debug/Release 공통 벽·지형 상태 버튼을 노출한다. 기존 typed Server
  preset 요청과 replication을 그대로 사용한다. 전체 복원·외곽 벽 제거·3시·9시·양쪽 붕괴의
  다섯 상태이며 현재 요청이 진행 중이면 중복 제출을 막는다.
- Composition Sequencer 맨 왼쪽은 `Save`, 같은 행 가장 오른쪽은 `Save & Publish`다.
  Ctrl+S와 Save는 source 저장, Save & Publish는 저장 후 게시라는 동작 구분을 유지한다.

### 검증한 범위

실제 설치 WModel의 833.333ms loop와 CBalanceTool/EffectCatalog/Playback을 사용했다.
원본 다섯 개의 box 범위는 342~10342, 950~11780, 1475~5475, 2067~12067,
3358~9620ms이며 마지막 asset의 실제 수명은 6.261773109초다. source-only target 6개의
기존 준비 큐가 완료되고 다섯 effect가 각각 350/950/1483/2083/3367ms(60Hz 한 프레임 오차)에
생성되어 document age 0~16ms로 시작했다. 7500ms에도 preview가 유지되고 전체 표현 시간은
12067ms다. 이는 native 수치·객체 검증이며 GPU 화면 확인은 사용자가 수행한다.

V2 실제 `boss.valtan.six.sonic.after` binding은 WINDUP 1799ms에서 오른쪽 500ms 이동 시
FIRST_SMASH 499ms로 바뀐다. source clock·rate·경계·EACH_LOOP·잘못된 target·stale 거절 및
무관한 payload 보존을 native 검증했다. Library의 `boss.valtan.six.flash_1`은 기존 그룹
`boss.valtan.six.sonic`에 속하고 기존 VALTAN_STAGGER_SLOT/FINAL_ATTACK 1000ms에 연결되어
있다. Library 표시만으로 미연결이라고 판단하거나 같은 연결을 새로 추가하지 않았다.

증거는 `out/ValtanFloorWipePreview20260928/flow-run.log`, `compile.json`, `compile.log`,
`v2-duration-run.log`, `arena-REPORT.md`, `arena-compile-results.json`에 있다. MainApp은
Debug/Release 각각 1TU, 변경한 Valtan/Workbench/Animation Composition/Catalog는 실제
제품 CP949 설정으로 컴파일했다. Client/UI 자동 실행·조작·Reload는 수행하지 않았다.

### 최종 설치와 실행 파일

최신 저장본 revision `7b957808ba8ba17d4f2d5c21b9cec3cd79cc38ce945810470c44859a0e4b7d68`을
다시 확인한 뒤 기존 typed Source Save transaction으로 REMOVE+ADD 4개 operation을 적용했다.
전체 시각 2067/3358ms를 유지하면서 WINDUP의 cue 두 개를 각각 FIRST_SMASH의 267ms,
INTERVAL 두 번째 clip의 source 258ms로 옮겼다. typed writer는 같은 transaction의 중복 target을
허용하지 않으므로 새 Stage에 유일한 cue/occurrence ID를 부여하고 나머지 cue payload를 보존했다.
변경 파일은 `Data/Valtan/Valtan.presentation.json` 하나이며 해당 Pattern의 effectCues 세 배열만
달라졌다. 다른 Pattern, Animation과 gameplay는 구조·원본 byte 검사로 보존을 확인했다.
최종 source revision은 `667f8962418abfcdbf285ba329e51f91ac5a4acf7f74d92822c198946a9f47e5`다.
백업과 receipt는 `out/ValtanFloorWipePreview20260928/rehome/applied-20260928-040419-216517`,
`rehome/installation-result.json`, `rehome/changed-paths.json`에 있다. runtime publish는 수행하지
않았고 서버 재생에 반영하려면 사용자가 오른쪽 Save & Publish를 사용한다.

최종 정규 Product compile/deploy PASS는
`out/BuildPipeline/runs/20260927T190418006Z-debug-product.json`이며 로그는
`out/ValtanFloorWipePreview20260928/product-build-final.log`다. 최종 Valtan.cpp 변경에 대한
OBJ 1개와 Client EXE 재링크를 완료했다. 직전 병렬 native TU 검사와 제품 PDB 공유로 발생한
C1041은 probe의 `/Fd`를 전용 out 경로로 분리하고 compile 종료 후 단독 제품 빌드로 해결했다.
컴파일 옵션이나 검증 조건을 완화하지 않았다. 앞선 첫 제품 빌드 receipt의 시각보다 나중인
최종 보완 소스가 실제 EXE에 포함되도록 이 마지막 incremental 빌드를 별도로 확인했다.

마지막 포즈는 실제 WModel의 loop clip track 15→15, 실제 `b_wp_r_01` matrix delta 0으로
고정되고 같은 Effect age는 6.458→7.058초로 증가했다. 기존 V2 Stage retirement 경로를 사용해
최종 Stage 이후 추가 EACH_LOOP를 멈추며 NATURAL tail clock은 유지한다. 별도의 최종 V2
EACH_LOOP/STAGE_END fixture와 ImGui pending-frame 자동 재시작, GPU 화면 검사는 미실행이다.
최종 native compile/link/run과 source hash 일치는 `flow-verified.json`, `WORKBENCH_REPORT.md`,
V2 drag 검증은 `out/ValtanV2Drag20260928/verified-native.json`을 따른다.

## 2026-09-28 최종 설치·게시

사용자의 “전부 다 반영할 거 반영해” 승인에 따라 최신 디스크 저장본을 다시 읽고 실제 설치와
게시를 완료했다. 아래 G16~G22의 후보·승인 대기는 각 검증 당시 상태이며 이 최종 기록으로 갱신한다.

- 복원 이펙트8개, 신규 DDS2개, Catalog/Resource Tree/Full Restore metadata 및 프로젝트 등록을 설치했다.
  Full Restore metadata는46개다. 독립 에테르 구슬과 추적 도끼 큰 원형은 선택·Preview·Append용 리소스다.
- 4방향 독립 sector cue4건을 추가하고 돌진 CHARGE의 기존 cue asset ID1건만 복원본으로 교체했다.
  삭제했던 기존 4방향 V1과 사용자의 애니메이션·무관한 편집은 보존했다. 기존 V2 impact도 유지했다.
- `Project-ValtanPatternMaster.ps1 -Mode PublishV2`는9개 산출물 중5개 변경으로 성공했고,
  `Publish-GameplayBalance.ps1 -Mode Publish`도 최종 성공했다. 원본 source revision은
  `4c348a1cc62a53077681bdfb0f88e8e11d66803ec3528ffab69a3bd341cc8f68`이다.
- 현재 Client presentation151개 산출물의 generation과 게시된 Gameplay.bootstrap 값이 모두
  `40ade52b6e24ffff26d22c6b366aaf62ccc8aded9369c4f71334902988954617`로 일치한다.

첫 gameplay 게시에서 저장된 돌진 애니메이션과 오래된 rootmotion의 duration 불일치를 검출했다.
Save Source는 원본 저장이며 현재 PublishV2의9개 투영 목록에는 rootmotion이 포함되지 않는다.
기존 `build_valtan_rootmotion.py`로 최신 Encounter/Bindings를 투영해 WINDUP3650→7350ms,
GROGGY6833→6897ms 두 행만 교체했다. 다른118행·20m authored 돌진·portal8개는 보존했다.
검증 조건을 완화하지 않고 재게시를 통과했다. 일반 typed transaction의 rootmotion 생성 경로는
그대로이며 Source Save 자체를 서버 활성화로 설명하지 않는다.

설치15파일의 JSON/XML과8개 asset/2개 texture hash, metadata46개, cue4개 및 저장본 보존을
확인했다. 백업은 `out/ValtanFourSector20260928/installation/applied-20260928-030356-755778`이다.
증거는 같은 작업 폴더의 `installation/installation-result.json`, `installed-source-verification.json`,
`rootmotion-installation.json`, `publish-products.log`, `publish-gameplay.log`다. 설치 receipt의
`runtimePublished=false`는 설치 직후의 역사적 상태이며 뒤의 성공한 게시와 구분한다.
Client/UI 자동 실행·Reload·GPU 화면 검증은 수행하지 않았다. 재시작한 Server의 메모리 활성화와
사용자의 최종 아레나 화면 확인은 파일 설치·게시 성공과 별개다.

설치 후 실제 All Effects strict view와 Workbench source view를 대조해362개 중62개 이름이
달랐음을 추가로 발견했다. strict tree의 legacy owner가 표시 선택에 개입한 원인이었다.
All Effects의 선택·재생 tree는 유지하고 표시 map만 같은 admission의 authoring view에서
만들도록 수정했다. 실제 두 소비자의 V1 275개/V2 87개 총362개 이름 일치·충돌0·검색·제목,
Full Restore46개와 신규8개 등록을 native 검사로 확인했다. 추가 read는 admission의 shared
lock 안에서 시작/종료 freshness를 검사하며 실패하면 이전 tree/map을 보존한다.
최종 Product 빌드·EXE 재링크도 성공했다. 최종 receipt는
`out/BuildPipeline/runs/20260927T181157965Z-debug-product.json`, 로그는
`out/ValtanFourSector20260928/product-build-final.log`다. 설치 후 이름 검증 증거는
`out/ValtanFriendlyNames20260928/installed-workbench-native.log`다.

## 2026-09-28 코드 빌드

정규 `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product` 빌드·배포를 완료했다.
Engine/Shared/Server/Client가 통과했고 `Client/Bin/Debug/Client.exe` 링크를 확인했다.
에테르 shader group에 필요한 CSO5개는 첫 실행에서 컴파일됐으며 최종 실행은 변경 없는 CSO를
재사용했다. 최종 receipt는 `out/BuildPipeline/runs/20260927T172312700Z-debug-product.json`,
로그는 `out/ValtanFourSector20260928/product-build.log`다. runtime 필수 파일·Navigation·
Items/Valtan reward 읽기 검사는 통과했고 이 Product runner는 데이터를 게시하지 않았다.

첫 제품 빌드는 신규 한글 literal을 기존 CP949 source charset으로 해석해 실패했다.
개별 검증의 `/utf-8`가 이 차이를 숨겼음을 확인하고, 신규65개 문자열만 UTF-8 byte escape로
교정했다. 기존 파일 인코딩과 프로젝트 옵션은 유지했다. 실제 CP949 TU object로 native 검사를
다시 연결해 이전354개 표시명과 byte-for-byte 일치 및 충돌0/검색·제목 일치를 확인했다.
증거: `out/ValtanFriendlyNames20260928/product-codepage-receipt.json` 및 해당 REPORT.

이 빌드 당시 코드·EXE·CSO를 반영했고 복원 데이터8개/신규 DDS2개/typed cue5건은 후보로
검증했다. 이후 승인에 따른 실제 설치·runtime publish 결과는 위 최종 설치·게시 기록을 따른다.
자동 Client 실행·Reload·GPU 화면 확인은 수행하지 않았다.

## 2026-09-28 G22: 한글 리소스 표시와 두 추가 복원 후보

All Effects, Composition Resources와 Effect Editor 제목은 같은 한글 표시 map을 사용한다.
refresh에서 V1/V2 header inventory와 현재 Pattern/원본 clip 순서를 결합하고 화면 row에서는
map을 조회한다. 저장된 displayName·asset ID·경로·cue는 표시 때문에 변경하지 않는다.
같은 이름이 되는 서로 다른 리소스만 안정적인 순번으로 구분한다. 원본 clip과 ID 검색은 유지한다.
실제 native inventory 검사에서 설치 전 V1 267개/V2 87개 총354개 모두 한글 표시,
서로 다른 ID 간 이름 충돌0, picker/editor 제목 일치와 한글·원본 clip·ID 검색을 확인했다.
Full Restore42개 및 다른 class의 이름 보존도 통과했다. 최종6개 CPP의 실제 `/c` 컴파일과
해당 변경 `git diff --check`도 통과했다. 확장자 없는 중간 cl 호출은 무작업 warning이었고
컴파일 성공으로 세지 않았으며, 이후 모든 `.cpp`를 명시한 실제 컴파일 결과만 채택했다.
증거: `out/ValtanFriendlyNames20260928/compile.log`, `workbench-native.log`.

잡아채서 불어 날리기는 기존420623.stage001/21_01의9요소, stage003/21_03의4요소 외에
stage004/21_04의 원본5 notify/26요소가 누락되어 있었다. 기존 두 문서·Product는 보존하고
마지막 복원본만 후보로 생성했다.2초/loop false인 원본 clip metadata와 기존 Resources42개를
대조했다. 실제 codec 및5시각517개 finite particle 검증 통과, 신규 Resources 없음.
목록 순서는01 잡기 준비,02 사자후 준비,03 불어 날리기로 native helper에서 확인했다.
증거: `out/ValtanCatchBreath20260928/manifest.json`, `REPORT.md`.

`effect.valtan.ether.orb`의 표시명은 `에테르의 구슬`이다. 원본 ETHER_MP LookInfo와
FX_BS_02.Item.Par_C_Ether_MP_001의5 sprite/1 ribbon을 기존 native 변환 경로로 복구했다.
원본 전용 material5개에5169~5173을 등록했고 기존64단위 shader group5120의 table·dispatch·
selected group·native 함수4파일만 확장했다. 새 C++/FX wrapper나 두 번째 runtime은 없다.
새 DDS2개와 기존 동일 DDS2개를 대조했으며 최종 설치 후보에는 새2개만 포함한다.
원본 loopCount0과 particle 분포는 보존하되 독립 저작의 방출 창을10초로 한정한다. 이는
원작 드롭 아이템 gameplay 수명을 복원한 값이 아니다. 기존 Playback에서2초 tail 뒤
12/12.1/15초 particle/trail0 및 Is_Finished=true를 확인해 NATURAL 재생도 끝난다.
native codec6요소/packet6개와 시간별 finite 재생, compile/link/run 모두 통과했다.
증거: `out/ValtanEtherOrb20260928/manifest.json`, `verification.json`, `REPORT.md`.

두 복원본과6개 이전 복원본을 합친 최종 후보8개, source cue 추가4건·asset 교체1건,
신규 DDS2개를 `out/ValtanFourSector20260928/installation/manifest.json`에 고정했다.
최신 저장본을 다시 읽어 무관한 편집 보존을 검증했고, cue/metadata 입력과 resource payload의
hash도 교체 직전에 비교한다. installer의 실패 복구는 동시 편집 파일을 보존하면서 나머지
자기 변경을 계속 복구한다. 실제 파일 실패·동시 수정의 focused2개 포함 Python16검사 통과.
이 후보 검증 뒤 사용자 승인에 따라 정본 Data/Resources 설치와 게시를 완료했다.
제품 빌드, 실제 설치·게시 및 사용자 화면 확인의 경계는 위 최종 기록을 따른다.

## 2026-09-28 G21: 검격 선택 그룹·복제·독립 변환

Effect Tool에 `Create Group from Marked`, manual group별 `Duplicate Group`, `Start (s)`를
추가했다. 실제420609/stage008의 선택9요소만 새 manual group·runtime anchor로 분리하고
원본 source slot·b_wp_r_01·source transform track은 보존한다. 복제는 기존 codec의 stable ID
및 내부 참조 remap을 사용하고 복제본마다 anchor를 독립화한다. source track이 있는 요소의
Detail Transform 제한은 유지하며 기존 Anchor Position/Rotation으로 particle 이동 방향도
함께 바꾼다. 시작 이동은 delay와 source origin을 반대로 변경해 source phase를 유지한다.

| 실제 실행한 검사 | 결과 |
|---|---|
| 실제 문서62요소의9선택·2회복제 | 원본 보존, 선택9만 그룹 변경,9×3그룹/고유anchor3개, 총80요소 |
| 그룹 편집·저장 | 그룹2의TRS 독립, 그룹3 시작+0.7초와 원본 track phase 일치, codec 왕복 통과 |
| 실패 보존 | carrier를 가진 Trail06 불완전 선택 거부 및 원본 보존 |
| 실제 fixed-step Playback |77개 particle packet의90도 회전·평행 이동, 최대 위치 오차2.67e-7m/속도 오차2.38e-7m/s |
| 실제 설치 모델·본 | MN_RPBF_01 body/AnimSet의 mesh_att_battle_10_01/b_wp_r_01, preScale0.0001·basis 정규화·owner1.4,4시각에서27요소/anchor3개의 Playback 변환 일치 |
| 컴파일 | Helpers/Detail/Editing/ResourceBrowser4개 TU MSVC `/c /Y-` exit0, 입력 hash 고정 |

실제본 검사는 no-window WARP로 리소스를 읽었으며 Client/UI 실행이나 GPU draw는 하지 않았다.
검증용80요소 문서는 out에만 썼고 사용자의 현재62요소 저장본은 교체하지 않았다.
증거: `out/ValtanSlashGroups20260928/group-native.log`, `group-probe-candidate.effect.json`,
`out/ValtanFourSector20260928/compile-group.json`.

## 2026-09-28 G19·G20: 돌진 원본 후보와 6방향 후 전멸 원본 확인

돌진은 현재 CHARGE가 참조하는400424/stage0/mesh_att_battle_4_01의 notify009를 복구했다.
기존 수동 단일 반구를 원본과 같은 것으로 간주하지 않고 원본 sprite1개·반구2개의 별도
Full Restore와0초 시작 독립 후보를 만들었다. raw FRotator의 yaw16384=90도,
TypeData pitch−90도, 실제 FX_Att_01→b_effectroot socket basis를 따로 검증했다.
원본 emit0.7초와 particle tail을 유지하고 실제 모델·본·애니메이션을 사용하는 native probe에서
12시각·56개 행렬 및108정점의 범위·전방·크기를 확인했다. 독립 asset ID와 정확한 socket만
기존 import-scale 정규화 helper에 추가했고 서비스 TU 컴파일도 통과했다.

교체 후보는 CHARGE의 기존 stable cue/occurrence의 effectAssetId 한 필드다. 사용자가 저장한
WINDUP의 mesh4 클립3개, 반복 수, CHARGE 시간·재생률과 기존 shared shield 문서는 보존한다.
사용자는 Source Save 성공을 직접 확인했다. 후보 파일 설치·게시 완료는 아래 최종 반영 기록으로
구분하며, 이 수치 검증은 GPU draw나 사용자 화면 확인을 의미하지 않는다.
증거: `out/ValtanDashRestore20260928/REPORT.md`, `manifest.json`, `verification.json`.

사용자가 지칭한6방향 후 전멸은420630/VALTAN_FLOOR_WIPE_130이다. 앞서 조사한
SIX_PIZZA_106과 구분했다. 원본27 Stage의272 payload SHA를 확인했고 직접 PlayDecal3건은
모두15_02의 전체 원형이다. 현재6방향은 V1 Product의 sector 텍스처 요소6개이며
0/−60/−120/−180/−240/−300도로 배치되어 있다. 별도 sprite_particle_5의 sector 텍스처
분사도 있지만 사용자 화면의 다른 효과와 동일하다고 단정하지 않았다.
Source·게시 binding·unified graph에서 이 패턴의 V2 연결은0이며 Full Restore5개는 미연결이다.
SkillEffect의35도/12방향 정의와 원본6방향 visual 호출 연결은 미확정으로 남겼다.
조사 요청에 따라 해당 패턴 데이터는 변경하지 않았다.
증거: `out/ValtanFloorWipeSource20260928/REPORT.md`, `source-and-current.json`.

## 2026-09-28 G19: 반복 클립과 시간 편집의 Source Save

실제 실패 로그 `Intermediate/Logs/ValtanRuntimePublish/LostArk.ValtanBalancePipeline.42804.192317968.log`와
`Intermediate/Logs/ValtanAuthoringSave/job-42804-4-192317921/draft-patch.json`의 5개 operation을 대조했다.
`VALTAN_DASH_CHARGE` WINDUP에 원본 `400424/0`의 `mesh_att_battle_4_01`을 두 occurrence로 더 붙였지만,
출처 검증은 Sequence 선언 하나당 source slice 하나만 선택해 두 번째 복제를 덮지 못했다.
수정 전 코드로 `changed=['mesh_att_battle_4_01', 'mesh_att_battle_4_01']`와
`SOURCE_PROVENANCE_MISMATCH`를 동일하게 재현했다.

`valtan_tuning_pipeline.py`의 기존 source-slice matcher는 같은 ordered slice의 유한 반복을 허용한다.
서로 다른 clip을 set으로 합치거나 source 순서를 바꾸는 경우는 계속 거부한다. 신규 source coverage는
새 stable occurrence ID, clip 교체와 재정렬을 검사하며, 기존 ID/clip의 시간·재생률 편집은 기존 typed
Animation 값·범위 검증을 따른다. manual pattern의 기존 REFERENCE 삭제 정리도 source identity를
보존한 시간 편집을 허용하도록 맞췄다. 원본 action/sequence tuple 조회, deterministic role, stable ID,
Source revision, 저장 전 baseline 비교 및 원자 교체는 유지했다. C++ 저장 우회는 추가하지 않았다.

| 검증 | 실제 결과 |
|---|---|
| 실제 실패 patch 재평가 | 수정 전 실패 재현, 수정 후 5 operation 통과, 입력 master 불변 |
| 임시 저장소 Source writer | 실제 `commit_source_authoring_patch` 성공, 임시 gameplay/presentation 두 파일만 변경, runtime `NOT_ACTIVATED` |
| focused 회귀 | 29개 통과: 동일 source 복제, 기존 시간 편집, manual REFERENCE 시간 편집, partial/HOLD/삭제 보존, 다른 clip·잘못된 tuple·role·순서·사용하지 않는 출처 거부 |
| 문법·diff | 변경 Python 2개 `py_compile`, `git diff --check` 통과 |

회귀 도중 사용자가 저장한 최신 디스크에 `400424/0` 선언이 생겼다. 이 저장본을 원복하지 않고 실제
실패 job이 남긴 읽기 전용 `source-baselines`를 회귀 입력으로 사용했다. 신규 테스트의 복제 전 owner도
메모리 복사본에서만 구성한다. 실제 저장소의 Data, 게시 데이터와 실행 중 UI draft는 이 검증에서 쓰지 않았다.

manual stage 광역 묶음의 `test_manual_stage_sequence_gap_kind_and_counter_project_together`는 기존
`sequence.charge` Sound cue의 `startMs`가 후보 clip 구간 밖이라는 오류로 실패했다. 수정 전 백업 모듈로도
같은 실패를 재현했으며 Source Save 수정의 통과 항목에 포함하지 않는다. 이는 read-only snapshot을
소비하는 기존 Sound 검증 실패이며 검증을 통과시키려고 Sound 저장본을 변경하지 않았다.

증거는 `out/ValtanAnimationSave20260928/`의 `before-replay.json`, `after-replay.json`,
`temp-source-save.json`, `focused-tests.log`, `verification.json`, `unrelated-baseline-failure.log`와
이번 변경만 분리한 `session-only.patch`다. 기존 Python의 ownerHitChain 등 다른 dirty 변경은 보존했다.
Client/UI 실행과 화면 성공을 이 결과로 주장하지 않는다. 구르기 후 돌진 원본 이펙트 후보는 별도 복원 결과다.

## 2026-09-28 G16·G18: Resources와 실제 Pattern 연결, 저장 상태

Composition Resources는 All Effects의 물리 V1 inventory와 V2 inventory를 함께 읽는다.
V1 / V2 Groups / V2 Leaves를 나누고, 여러 Pattern이 사용하는 Full Restore는 Common과
각 Pattern에 같은 stable ID로 표시한다. 실제 Product cue로 쓰는 Full Restore는 Product와
Full Restore 양쪽에서도 찾을 수 있다. 목록의 존재와 개별 Preview·Append admission은 구분한다.
Preview 요청은 MainApp을 통해 기존 Effect Tool 소비자로 전달하고, V1 Append는 해당 Stage의
clip이 하나일 때 자동 선택하며 여러 개일 때는 사용자가 선택한 정확한 occurrence를 요구한다.

원본 Sequence 선택은 서버 재생 owner만 별도로 resolve하고 기존 Sequencer 선택은 그대로였다.
`Open Owning Pattern in Sequencer`는 그 원본의 실제 소유 Pattern을 stable ID로 예약하고,
frame 종료 뒤 기존 선택 경로로 연다. 다른 Pattern의 미저장 draft는 기존 보호 절차를 유지한다.
native probe에서 source420612/1 → VALTAN_SIX_PIZZA_106 이동 후 실제 Build_Timeline의
V2 세 항목을 확인했다: STEP_05 shout7500ms, STEP_07 impact19450~19650ms,
STEP_11 twohand28750ms, 전체33000ms. 임시 실행 파일의 잘못된 Resources root로 발생한
첫 probe의0개는 실제 Client 결함으로 기록하지 않는다. 실제 설치 Resources root를 지정한
최종 probe에서100개 binding을 정상 admit했다.

`Save Source`와 `Save & Publish`를 분리하고 이미 저장된 Source도 후자로 게시할 수 있게 했다.
상단은 Source 저장, Publish, Server 연결을 각각 표시한다. 서버 revision을 단순 관찰한 결과로
현재 Source가 활성화됐다고 단정하지 않으며, 실제 Play 명령의 exact revision admission을
유지한다. 사용자가 삭제한4방향 V1은 실제 Source의 빈 effectCues로 저장돼 있었고 이전 게시본의
잔존 cue와 달랐다. 그 삭제를 복원하거나 Source를 이전 게시본으로 덮어쓰지 않았다.

Full Restore metadata의 optional variantId는 action/stage 뒤의 검증한 단일 token에 대응한다.
기존 metadata42개는 그대로 읽히며 분리본도 원본 clip·시간을 소비한다. Python14회귀와 실제
C++ metadata probe의 정상 variant·잘못된 variant 거부·실패 시 이전 index 보존을 확인했다.
변경 C++와 WB header의 직접 소비자6개 TU를 MSVC `/c /Y-`로 컴파일했고 최종 Workbench 수정도
별도 재컴파일했다. 실제 Product 실행 파일 링크와 Client/UI 실행은 하지 않았다.

증거: `out/ValtanFourDirectionWorkbench20260928/workbench-native.log`,
`out/ValtanFourSector20260928/compile-six-tus.json`, `compile.json`, `metadata-native.log`.
자원 후보·새 cue4개의 typed preflight는 원본 문서·기존 V2·무관한 Pattern 보존 및 삭제한 V1을
되살리지 않는 것을 확인했다. 이 섹션 작성 시점에는 데이터 설치·게시 전이며, 후보의 최종 반영은
편집 중 데이터 반영 절차와 이후 설치 결과를 따른다.

추적 도끼 전체 조사는46 Stage/460 notify payload를 검증했다. Particle94회(8종),
직접 PlayDecal18회, 별도 Trail0회, AK54회(6종)이다. 독립 Particle8문서의70요소와
참조 Resources72개는 존재하지만, 현재 Full Restore019/021의 명시 sourceNode로 연결된
Particle 발생은10/94이고 직접 PlayDecal은0/18이다. 미연결 발생을 모두 복원했다고 기록하지
않는다. 상세 표와 closure는 `out/ValtanTrackingAxe20260928/REPORT.md`와
`occurrence-resource-closure.json`을 따른다.

## 2026-09-28 G16: 4방향 부채꼴 분리와 추적 도끼 큰 원형 후보

이번 기록은 원본 조사, 후보 생성과 CPU 소비자 검증까지의 결과다. 이 후보 검증 시점에는 authoring 문서·cue·catalog·Resources·게시 runtime을 교체하지 않았다. Client 실행, GPU draw, 사용자의 최종 화면 확인도 미실시다. Workbench 입력·저장과 실제 설치 결과는 별도 검증 기록으로 구분한다.

### 원본과 생성기 변경

`build_valtan_portal_ground_restore.py`에 원본을 읽어 `out`에만 쓰는 `--split-four-candidates`와 `--tracking-axe-circle-candidate`를 추가했다. 쿠크의 `animate_radial_fill`은 기존 원형·도넛과 함께 발탄 native2614/2615를 허용하며, 기존 `SourceTransformTrack.materialParameterTracks` 소비자를 재사용한다. Client C++·shader의 두 번째 재생 경로는 추가하지 않았다.

4방향의 원본은 action420624/stage007, `mesh_att_battle_19_01`, `effect.valtan.action.420624.stage007.full.restore`의248요소다. 네 sector의 `inner=0.5`에는 시간 트랙이 없었다. 원본248은 보존하고, 원본 비sector244요소를 그대로 가진 `.body.full.restore`, 성장하는4요소의 `.sectors.full.restore`, 독립1요소 `effect.valtan.sequence.four.sector`를 만들었다. 원본4요소는 `materialParameterTracks`만 변경했고 독립1요소는 delay와 source origin을0으로 맞췄다. 원본 lifetime1.1초, fade-in0.2초, fade-out0.5초, 반경10m를 유지하며 inner0→1은 fade-out 시작인0.6초에 완료한다. 이는 요청한 채움 동작이며 회수된 원본 엔진 시간 곡선이라는 뜻은 아니다.

원본 sector의 detail yaw는−90/90/180/0도이고 `snapshotRootSourceBasisYawDegrees=-90` 및 decal의 projector 전방을 함께 적용하면 실제 전방은+Z/−Z/−X/+X다. 독립1요소는 이 기존 basis를 유지해+Z를 향한다. 전체 asset이나 공통 shader에 회전을 강제하지 않았다. 현재 V2의 `boss.valtan.impact`4개는 시작1233/2233/3233/4200ms, yaw0/175/270/90도를 그대로 보존한다. Valtan V2 group의 소비자는 LEAF/GROUP만 지원하므로 Kouku 전용 V1_ELEMENT를 연결하지 않고, 기존 V1 cue4개를 V2 impact와 병행하는 후보를 준비했다. cue 시작은 원본100/1100/2100/3100ms, yaw는 각 현재 impact와 같으며, 고정 GAMEPLAY_FOOTPRINT1.5와 localScale2/3의 합성 배율1.0으로 원본 반경을 보존한다. metadata 후보는 원본 source action/stage/clip과 variantId body/sectors를 포함한다.

추적 도끼는 action420610의18개 PlayDecal 중 큰 원8개(stage006/011/014/015/033/038/041/042)를 대상으로 했다. SkillDecal2002 → `GR_Mon_Circle_behit_02` → SkillEffect42061011/12의 원본 범위는 반경875cm다.8개 모두 notify3.5초, lifetime1.4초, fade-in/out0.2초이며, 원본 material은 native2614의 `fx_o_de_behitcircle_02_01_tr`이다.193-byte raw notify의 optionalName `Decal`과 뒤의 `notify` 문자열 길이·전체 SHA·실제 field offset을 검증했다. 기존187-byte notify의 뒤쪽 offset을 그대로 적용하지 않았다.

독립 후보 `effect.valtan.tracking-axe.large-circle.warning`은 원본17.5m 지름,6m projector 깊이, 색·수명·fade를 유지한다. 독립 시작0초와 inner0→1의1.2초 채움은 편집용 정책이며 원본 notify3.5초는 provenance에 남겼다. 작은 원4개와 donut6개의 설치·변경은 이 후보 범위에 없다. tracking circle의 product cue는 생성하거나 추가하지 않았다.

### 실제 실행한 검증

| 대상 | 실행 결과 |
|---|---|
| 4방향 codec | 실제 `CEffectDocumentCodec::Load`로244/4/1요소의3문서 통과 |
| 4방향 native packet |25시각 샘플에서 inner0/0.5/1/1/0.5, 다른3175lane 불변, NaN 거부 후 packet 보존 |
| 4방향 playback | 실제 Stage_Document/Seek로310샘플 projector 행렬 고정·finite, 전방+Z/−Z/−X/+X,20m 지름, 채움 끝 alpha1.5, 수명 종료 뒤 제거 |
| 큰 원형 codec/packet |1요소 통과,5시각 샘플에서 inner0/0.5/1/1/0.5, 다른635lane 불변, NaN 실패 보존 |
| 큰 원형 playback |80샘플의17.5m 지름·6m 깊이 고정,1.2초 채움 끝 alpha1.5, 수명 종료 뒤 제거 |
| 생성·리소스 |4방향 texture3개와 큰 원형 texture2개 존재, 후보 재생성 바이트 일치, Python AST 및 변경 범위 `git diff --check` 통과 |

검증은 현재 `Effect_Playback.cpp`를 MSVC로 컴파일하고 기존 codec·native material 소비자와 연결한 CPU probe로 수행했다. GPU device나 draw를 생성하지 않았으므로 위 수치가 실제 화면의 색·방향·가림 결과까지 증명하지 않는다. source-clock 트랙은 `startDelaySeconds + sourceTimeOriginSeconds`로 key를 만들고 기존 runtime이 source time으로 한 번 평가하도록 했다. native2614의 inner는 row0/lane1, native2615는 row0/lane2이며 다른 lane을 함께 변경하지 않았다.

증거는 `out/ValtanFourDirectionWorkbench20260928/sector/`의 `four-sector-projection.json`, `verification.json`, `sector-cue-additions.json`, `animation-metadata-additions.json`과 `out/ValtanTrackingAxe20260928/circle-inner/`의 `tracking-circle-projection.json`, `verification.json`, 각 compile/link/run 로그다.4방향 원본 SHA256은 `6e80879f7be57be1957edf9388b278d8341670d7be2c7cea71c06073594b24ff`로 후보 검증 전후 동일했다. 큰 원형 후보 SHA256은 `4244e4d8ccf26cb1752f2d89833b9ff7084939deca07793598a810112b580739`다. 후보3문서는 manifest에 명시된 ID만 사용하며, 이전 이름으로 남은 임시 산출물은 설치 목록에 포함하지 않는다.

## 2026-09-27 G14: 안내 네 줄 제거와 글자 높이 확보

- `Selected Box` 아래 드래그 안내, Pattern Total Duration, 길이 계산 설명과 편집 불가
  Stage Gap 안내를 제거했다. 선택 label·Duplicate/Delete와 가능한 timing 입력은 유지한다.
- `CompositionTimeline::GetBoxHeight/GetLaneHeight`로 발탄의 고정 22px box·24px row를
  최소 32px box·38px row로 바꿨다. 현재 폰트와 style padding이 더 크면 함께 증가한다.
  예를 들어 28px 글꼴, 기본 padding이면 box 36px·row 42px다.
- ruler·lane·subrow·box·hit-test가 같은 frame의 높이를 소비한다. row 수, 가로 시간폭,
  authoring 데이터와 다른 도구의 기존 높이는 바꾸지 않았다.
- `ValtanActionWorkbench.cpp` 및 `_Timeline.cpp`의 MSVC 14.44 `/Zs /Y-` 컴파일 PASS,
  변경 파일 `git diff --check` PASS. 로그는 `out/ValtanTimelineReadability20260927/compile.log`다.
- 사용자가 제품 빌드는 나중에 직접 하겠다고 확정해 새 실행 파일로 링크하지 않았다.
  소스·최소 컴파일과 제품 EXE 반영을 구분한다. Client/UI 조작과 실제 화면 검증은 수행하지 않았다.

## 2026-09-27 G15: 타임라인 탐색과 이펙트 시계

사용자는 줌 차이가 자신의 DPI 변경 때문이라고 정정했다. 줌·Fit·DPI 코드는 변경하지 않았다.
`Render_Timeline`은 전체 canvas 폭을 child content로 명시하고 하단 가로 scrollbar를 항상
표시한다. 모든 행·box·playhead는 기존 스크롤된 CanvasOrigin을 공유한다. 화면보다 긴
sequence는 막대로 좌우 탐색하고, 전체가 들어오면 scrollbar의 이동 범위는 0이다.
해당 변경 후 Workbench 두 TU 최소 컴파일과 diff 검사를 다시 통과했다.

V1은 `EFFECT_SPAWN_DESC`에 local boss preview의 source-clock offset을 유지하고,
`Sample_LocalBossPreview`가 pending/active cue를 전체 timeline에서 sample한다. local authoring
boss만 external clock으로 생성할 수 있으며 일반 Server spawn의 시계는 유지한다.
V2 `Sync_StageAuthoring`은 전체 timeline 시간을 받아 child의 절대 birth/stop과 natural tail을
sample한다. 객체의 layer Update는 paused 상태여서 별도 frame delta로 진행하지 않는다.

현재 cursor의 Pause/Resume는 기존 handle을 보존하며 target generation/owner를 확인한다.
새 Stage에서 local age가 0이 되는 상황을 역 seek로 판단해 V1 owner 전체를 지우던 조건도
수정했다. 실제 seek/reset은 기존 재구성 경로를 사용한다.

`EffectV2_Runtime.cpp`, `Effect_PresentationService.cpp`, `Valtan.cpp`,
`Animation_Tool_ValtanComposition.cpp` MSVC 14.44 `/Zs /Y-` PASS.
관련 헤더 변경 후 Workbench 두 TU도 다시 PASS였다. 로그는
`out/ValtanTimelineReadability20260927/effects-compile.log`, `ui-compile.log`다.
V1 forward sample은 직전 committed age와의 차이만 진행하고 최초/역방향만 Seek해
follow 입자의 기존 이동 이력을 보존한다. 이 보완 후 service TU도 다시 컴파일 PASS였으며
`effects-service-final-compile.log`에 기록했다.
실제 설치된 `effect.valtan.pattern.420633.active`를 준비하고 실제 휠윈드 Play를 실행한
무창 native probe도 통과했다. 이펙트 생성 뒤 Pause에서 service/object 60회 Update 동안
동일 객체·age를 유지했고, Resume 0.1초는 source playRate 0.4441667에 따라 약 0.04442초
진행했다. 효과가 존재하는 SPIN 시각으로 역 seek 후 30회 Update에서도 age가 고정됐으며
Reset은 모든 preview effect를 제거했다. 결과는
`out/ValtanPreviewCrash20260927/native/timeline-clock-pass.log`다. 변경 CPP를 out에 독립
컴파일해 연결한 진단 실행이며 제품 EXE의 새 빌드나 UI 화면 성공으로 기록하지 않는다.
실제 V2 sampler 함수 본문을 추출하고 object layer Update만 대체한 CPU probe에서
Sync 없는 정지 유지, 같은 시간 반복, 재개, Stage tail의 절대 시각, DEACTIVATE 종료/잔여 입자,
KILL, 제품 경로 분리와 birth 0을 검증했다. 결과는
`out/ValtanFloor20260927/v2-preview-clock-probe/result.log`다. 실제 GPU 화면 검증은 아니다.
사용자 요청에 따라 제품 EXE 빌드·링크와 Client/UI 실행은 하지 않았다.

## 2026-09-27 G12~G13: 패턴별 리소스와 Saydon 편집 흐름

이번 변경은 현재 Valtan source/Pattern owner와 CValtan presentation을 확장했다. 다른 보스 아래에
새 Valtan runtime을 만들지 않았다. 기존 패턴의 effect binding, collider, damage, 돌 크기는 변경하지 않았다.
작업 시작 LAN 설정은 client / 192.168.0.22:7777 / not-listening이었고 로컬 설정은 성공했다.
기존 `codex/bern-dragon-camera-performance`의 다른 기능 미커밋 변경을 보존했다.

### 리소스 검색과 Effect Tool

- `Composition Resources → Effect`를 V1/V2 leaf/group가 함께 있는 Patterns/Common/Library로 묶었다.
  이름, pattern ID/표시명, clip, category와 asset ID로 검색하고 다른 owner는 옵션으로 표시한다.
- 연결된 Pattern이 하나면 Patterns, 공유 연결/Independent/combat-object visual은 Common으로 분류한다.
  독립 저장 자원의 사용자 category는 유지하며 label로 저장 ID나 owner kind를 추론하지 않는다.
- Full Restore의 action-or-Product-cue 및 clip 이름 exact join을 `CValtanPatternTree`에서 공유한다.
  같은 clip을 공유하는 다른 원본 action을 잘못 묶지 않는다. index는 41개이고 기존 join 동작을 유지했다.
- `Open Editor`는 stable typed resource를 MainApp에서 기존 Effect Tool owner로 전달한다.
  V1 Full Restore는 cold entry에서도 source metadata를 준비하고 미저장 Save/Discard/Cancel을 유지한다.
  Product는 현재 연결 cue이고 Full Restore는 독립 복원 자원이라는 설명을 표시한다.
- 선택 Stage/Animation occurrence에 `Append Effect to Pattern Draft`로 붙이고 기존 Box Detail/Save를 쓴다.
  Effect 내부의 Play All/Solo·Element timeline은 계속 Effect Tool이 소유한다.

| 대상 | 실제 Full Restore asset |
|---|---|
| `VALTAN_WHIRLWIND` | `effect.valtan.action.420633.stage014.full.restore`, `stage015.full.restore` |
| 피자 `VALTAN_SIX_PIZZA_106` | `effect.valtan.action.420629.stage006.full.restore`, `stage008.full.restore`, `effect.valtan.action.420620.stage004.full.restore` |

휠윈드의 다른 패턴 `ATTACK_WHIRLWIND`/`SEQUENCE_WHIRLWIND`는 각각 다른 source action을 사용한다.
피자 복원본은 현재 연결 기준 3종이며 clip 이름은 `mesh_att_battle_12_01`, `_12_03`, `_12_10`이다.

### 기존 V1 피자 섹터의 재사용 자원

`Data/Effects/Authored/effect.valtan.six-pizza.sectors.effect.json`을 새 direct-authored resource로 등록했다.
EffectCatalog/EffectResourceTree와 Client project/filter의 `96.DataFiles`만 함께 등록했다.
Common/Telegraphs의 `발탄 피자 / 기존 섹터 묶음`으로 검색할 수 있다.

기존 `effect.valtan.project-tuned.sequence.six-pizza-106`의 12개 element 중 4개의 섹터/overlay를
분리했다. ID·particle/reproduction payload·재질·transform을 유지하고 최소 delay 11초만 뺐다.
새 자원의 element delay는 0/8.5/0/12초다. 기존 Pattern의 11초 위치와
`arena.center.target-follow`, follow 및 root scale 1.5를 적용하면 원래 섹터 시각을 재현한다.
원본과 신규 자원을 동시에 붙이면 이 4개가 중복되므로 기존 invocation과의 교체를 선택해야 한다.
기존 composite의 나머지 8개 이펙트는 사용자가 조정한 별도 시각에 있어 자동 삭제하지 않았다.
원본 SHA256 `ceff278cc5cf7ab4584c3198683eca1a6eb70bbac4910e41be1a62ddf6fb3e36`을 보존했다.

실행 중 Client의 catalog 메모리는 파일 추가로 자동 바뀌지 않는다. 새 resource는 catalog를 다시 로드한
세션에서 보이며 Workbench Refresh는 inventory refresh다. Pattern source/runtime publish는 이번에 하지 않았다.

### Sequencer와 Preview

- Saydon과 같은 공용 palette/DrawBox를 사용한다. 행 24px, box 22px, label 180px, text inset 4px.
  Stage는 한 줄이고 Animation은 실제 clip 이름을 표시한다. 작은 box와 긴 label은 tooltip으로 보완한다.
- 현재 Saydon 실제 색상은 Stage 회색, Animation 파랑, Logic 주황, Effect/Sound/Collider 등 표현 lane
  청록 계열이다. 사용자 기억의 색을 별도로 하드코딩하지 않고 두 보스가 같은 상수를 소비하도록 했다.
- `Save / Play Preview / Pause·Resume / Reset / Play Pattern`을 같은 Sequencer 위에 배치했다.
  Play Preview는 현재 cursor에서 시작하며 dirty draft, 선택 Branch, Loop와 seek를 유지한다.
- Sound는 실제 Stage/action/clip tuple과 공용 source-time 변환으로 시작점을 계산한다. managed Sound handle로
  pause/resume, 앞뒤 seek, 반복, speed, 정지와 paused 상태의 Sound draft generation 변경을 처리한다.
  누락 사운드는 진단을 남기고 animation preview를 유지한다. `NONE` Stage는 기존 pose hold를 유지한다.
- combat object Preview는 선택 branch의 누적 Stage clock을 사용한다. 다음 Stage로 넘어가도 생성된 돌이
  사라지지 않고 TIMED/repeat hit의 terminal visual을 기존 external sampler로 재생한다.
  CONTACT를 임의 폭발로 만들지 않는다. Pause·역 seek·Stop·complete·대상 제거의 handle 정리를 연결했다.
- Server pending-next/flow-stop도 playback ownership으로 확인해 local Preview와 겹치지 않게 한다.
  Reset 후 이전 cursor가 다시 덮이는 Preview 패널 상태 갱신도 수정했다.

Preview는 선택한 Logic 결과 분기와 animation/effect/sound/collider mirror/environment를 재현한다.
실제 target 결정, counter 성공 여부, World action, 피해·엄폐 판정은 `Play Pattern`으로 확인한다.
Valtan Play Pattern은 기존 저장·게시된 서버 revision을 사용한다. Kouku의 미저장 draft 임시 Server
protocol은 추가하지 않았다. Dirty 상태에서 과거 Product를 대신 실행하지 않고 Save/Publish를 안내한다.
Save는 기존 Pattern/Sound/V2 dirty-owner atomic transaction이고 Publish after Save/Retry Publish,
Server active revision 검증은 기존 경로를 유지한다. 서버 재시작이 필요한 상태는 별도로 표시한다.

### 돌과 피자 판정의 실제 저장 상태

네 방향 돌은 Effect 4개가 아니라 Summon/combat-object 1개 호출의 count 4다.

| 패턴 | Pattern 기준 생성/폭발/Server 소멸 | 배치 반경 |
|---|---|---|
| 땅구르기 후 사자후 | 0 / 5 / 6.2초 | 6.363961m, boss-relative |
| 3페이즈 전 발악 | 5 / 10 / 11.2초 | 6.363961m, boss-relative |
| 피자 | 1 / 20.5 / 21.7초 | 10m, arena-center |

셋의 visual scale과 coverRadius 1.5m는 같다. 피자 돌이 2배 크기라는 가정은 현재 데이터와 다르다.
`VALTAN_TRASH`(버러지) 계열은 rock spawn이 없으며 STRUGGLING(발악)과 별개다.
피자의 19.45초 착지는 실제로 CIRCLE 25m hit이고, 살아 있는 돌 뒤 선분에 있으면 해당 피해·넉백이 차단된다.
visual sector와 일치하는 damage shape는 아니다. Server 소멸과 authored NATURAL visual tail도 구분한다.
이번 변경은 이 gameplay 수치와 기존 사용자가 튜닝한 타이밍을 변경하지 않았다.

### 검증과 남은 확인

- Workbench/Saydon, Valtan/Tree, Effect Tool entry, Animation Tool의 관련 CPP MSVC 14.44 `/Zs /Y-` 성공.
- 실제 Sound handler 본문과 공용 ActionPresentationTimeline을 사용하는 console test PASS:
  source offset/playRate/each_loop, variant 재현, pause/resume/역 seek, paused draft 변경과 cursor 보존,
  정지·실패 격리 및 NONE pose hold. 오디오 장치를 통한 청음은 수행하지 않았다.
- 실제 combat preview/selected path/Stage clock 본문 console probe PASS:
  Stage 간 유지, Pause 동일시각, 정/역 seek, terminal/natural tail, rollback, 독립 action,
  네 branch, overflow 및 TIMED repeat/CONTACT 구분. GPU/Server damage 실행은 수행하지 않았다.
- 기존 metadata 검사 13건과 TIMED rock carrier focused unittest PASS.
  과거 preflight source-shape assertion 1건은 HEAD에서도 동일 실패하여 이번 성공 근거에 포함하지 않았다.
- 신규 sector의 실제 C++ codec Load/Validate_Drawable 성공, texture closure 4개 확인.
  sourceModel을 전제하는 기존 전체 probe exit는 독립 sector에 적합하지 않아 전체 probe PASS로 기록하지 않는다.
- 세부 로그/수치: `out/ValtanEditor20260927/`의 sound-handler-test, combat-preview-probe,
  combat-object-audit, sector-candidate-report, sector-resource-install 및 full-restore-shared-index-check.
- 최종 Client Debug x64 정규 `Build` exit0, compile/link/runtime dependency 배포 PASS.
  `out/ValtanEditor20260927/client-debug-final.log`, 12분35.61초, 오류0/경고3,700(shader/C4819/DirectXTK PDB 등).
  생성된 `Client/Bin/Debug/Client.exe`는 2026-09-27 08:52:42, 75,662,336bytes다.
  처음 시도는 동시에 편집되던 Movie helper 미선언으로 실패했고, 담당 변경이 안정된 뒤 성공했다.
  실패 로그도 보존하며 최종 성공을 모든 무비 기능의 시각 검증으로 확대하지 않는다.
- 마지막 code snapshot 15개가 빌드 중 바뀌지 않았음을 확인했고 JSON3개/XML2개 parse,
  원본 pizza composite와 신규 sector payload hash, 작업 파일 `git diff --check` PASS.
  최종 receipt는 `out/ValtanEditor20260927/final-verification.json`이다.
- 모든 Play Pattern 진입점은 dirty draft뿐 아니라 saved source/Product 준비 상태와 Server playback
  ownership을 확인한다. 게시되지 않은 source를 과거 Product로 조용히 대신 실행하지 않는다.
- 기존 빌드 최적화 반영본을 그대로 사용했다. 같은 feature branch의 CPP/INL 분리, 셰이더 그룹 분할,
  PCH, FXC4-worker를 유지했다. 시작 당시 Release 빌드가 병행 중이어서 이번 Debug 명령에만
  `CL_MPCount=2`를 적용했고 공유 기본 최대8-worker 값은 변경하지 않았다. Clean/Rebuild는 하지 않았다.
  다른 변경의 기본 셰이더 재컴파일이 포함돼 12분35초를 최적화 전후 성능 비교로 사용하지 않는다.
  앞 작업의 최적화 Debug/Release PASS는
  [베른·발탄 최적화 결과 G08~G09](../09-27/2026-09-27_BERN_VALTAN_CAPTURE_OPTIMIZATION_RESULT.md)를 따른다.
  이번 최종 delta의 직접 Product 검증은 Debug이며 Release 전체 재빌드를 추가하지 않았다.
- Client/UI를 실행하거나 조작하지 않았다. 사용자 확인은 `Action Workbench → Boss Valtan →
  휠윈드/피자 → Composition Resources 검색 → Open Editor → Play All/Solo`, 이어서 대상 Stage/Clip 선택→
  Append→Preview/Pause/seek→Save/Publish→Play Pattern 순서다. 실제 화면·청음·Save/Reopen·Server cover 판정은 미확인이다.

---

## 2026-09-18 구현 및 실행 반영

이번 재개는 발탄의 실제 source 애니메이션 transport, 공통 Composition 편집 화면,
Source Save와 Product 분리, 4연속 공격의 Full Restore 연결을 구현했다.
아래 2026-09-09 내용은 조사 당시 기록이다. 당시 계획의 모든 G가 완료됐다는 뜻은 아니다.

### G08. Play·Pause·Seek와 Effect Solo·Group

- Valtan source sequence는 CModel을 pause 상태로 두고 하나의 커서로 pose를 sample한다.
  Play/Pause, Reset, 역방향 seek, clip 경계와 종료 pose, Loop가 같은 clock을 소비한다.
- 공통 Resources transport, Sequencer ruler, 별도 Preview 창이 source/master 소유자를 구분한다.
  source preview는 Pattern ID가 비어 있으므로 Pattern ID 비교보다 source owner를 먼저 처리한다.
- Effect Tool의 `420609 stage008/009 Full Restore` Play All, Solo, Play Group은 실제 원본
  clip occurrence와 source anchor history를 EffectAuthoringSequencer에 전달한다.
  기존 root scale 1과 Valtan bone owner scale 1.4를 구분한다.
- root 이외 세션·창으로 이동하면 이전 preview와 Camera lease를 종료하고, 오래된 PLAYING UI 상태를 남기지 않는다.

### G09. 공통 Resources·Sequencer와 실제 편집 경로

- Saydon과 Valtan이 같은 `COMPOSITION_RESOURCE_CATEGORIES`와 `CompositionTimeline`의
  행 높이 24, 라벨 폭 180, 최소 box 폭 8을 사용한다. Resource tab은 Animation, Logic,
  Summon, World, Scene Profile, Effect, Collider, Sound, Camera, Light, Pattern이다.
- Stage 다음 Animation·Logic·Summon·World·Scene Profile과 Effect·Sound·Camera·Collider·Light
  트랙이 이어진다. Resources의 추가, Box Detail의 수정·삭제, timeline drag를 같은 typed owner에 연결했다.
- Stage body 이동은 앞 Stage 길이를 바꿔 뒤 clock을 함께 이동한다. 첫 Stage의 시작은 0이다.
  Animation은 기존 source clip 순서·구간을, Collider는 실제 hit schedule을 편집한다.
- Camera·Scene Profile·Light는 시간 이동·양끝 trim과 Stage 간 이동을 지원한다.
  실패한 Stage 간 변경은 draft와 dirty generation을 함께 rollback한다.
- Summon은 기존 combat-object resource, spawn count/wave/interval과 공유 archetype lifetime을 편집한다.
  공유 lifetime 변경은 같은 resource의 모든 occurrence에 적용됨을 UI에 표시한다.
- Logic과 World는 Server의 ENTER/EXIT 계약을 유지해 가까운 Stage 경계로 snap한다.
  Counter/Groggy topology와 phase 전환 등 전용 계약은 해당 typed 편집기를 사용한다.
  기존 World set은 단일 invocation 계약 때문에 Resource에서 `Move to Stage`로 이동한다.
- Sound는 시점 이벤트다. 참조·자동 파생 행과 길이가 없는 이벤트에 임의의 duration을 저장하지 않는다.
  새 Scene/Light/Camera 끝과 Summon 마지막 spawn을 넘겨 Stage를 줄이면 변경 전 거부한다.

### G10. Source Save와 Product 소비

- Save는 owner별 최신 baseline/CAS, 구조 parse, atomic writer·rollback을 유지하면서
  전체 Product projection 검증을 일반 Source 저장과 분리한다. Product reopen 실패를
  이미 성공한 Source 저장 실패로 보고하지 않는다.
- Workbench와 Balance가 source inventory를 직접 읽고, 명시적 draft preview는 해당 draft를 소비한다.
  제품은 strict Publish로 만든 snapshot을 계속 사용한다.
- V2와 Sound의 게시된 문서는 `Data/Valtan/Published/` 아래 생성물로 분리한다.
  Source Save만으로 실행 중 또는 재실행한 제품에 미완성 binding이 활성화되지 않는다.
  local V2 preview에는 기존 명시적 authoring snapshot 주입을 유지한다.
- Scene Profile·Light는 stage별 저장, product patternbindings v5와 실제 CValtan stage clock까지 연결했다.
  기존 공통 presentation sampler, rendering profile lease와 frame light provider를 사용한다.
- Camera occurrence도 v5 binding을 소비한다. 제품의 기존 cinematic controller와 local CameraTool이
  같은 stage offset/duration을 사용하며 새 camera renderer는 만들지 않았다.

### G11. 4연속 공격의 Full Restore

- `VALTAN_FOUR_SLASH`의 SLASHES와 SPIN에 `effect.valtan.action.420609.stage008.full.restore`,
  `effect.valtan.action.420609.stage009.full.restore`를 연결했다.
- 기존 stable occurrence ID, timing, source window와 사용자의 scale 1.5를 유지했다.
- source presentation과 generated pattern effect cues를 기존 writer lock/CAS 경로로 함께 반영했다.
  이어 Camera v5와 새 published V2/Sound snapshot을 strict Publish로 생성했다.
- 검격의 sprite 복원·empty alpha key 수정은
  [09-18 검격 복원 결과](../09-18/2026-09-18_VALTAN_FOUR_SLASH_SPRITE_RESTORE_RESULT.md)를 함께 따른다.

### 검증과 사용자 확인 경계

- Effect/Animation transport 변경 TU, Workbench/MainApp, environment/Camera와 Save owner의 최소 컴파일을 수행했다.
- 실제 C++ source transport 함수를 이용한 CPU 검사는 14건을 통과했다.
- 실제 auxiliary drag 함수를 이용한 CPU 검사는 Stage ripple, Camera/환경 Stage 간 이동,
  상태쌍·World 경계, Summon clock과 rollback을 검사했다. 최종 건수는 해당 실행 로그를 따른다.
- strict source/product projection은 42 managed / 25 legacy / 9 combat object / 97 world member로 통과했다.
- 검증 파일: `out/ValtanSequencerTransport20260918/`, `out/ValtanUnifiedSequencer20260918/`.
- Debug Product build는 14:19와 14:21 증분 빌드 모두 PASS. 마지막 증분은 native generation admission의
  게시된 V2/Sound 경로를 포함한다. 증거: `out/BuildPipeline/runs/20260918T052152203Z-debug-product.json`.
- 그 뒤 독립 검토에서 Camera의 지연을 잘못된 trigger로 저장하던 두 UI 지점을 `ENTER + startOffsetMs`로,
  Summon Add가 오래된 prototype lifetime으로 미저장 공유 값을 되돌리던 경로를 최신 typed draft 조회로 고쳤다.
  마지막 Workbench/MainApp/Kouku TU 컴파일은 exit 0이다. 이후 사용자가 직접 빌드하기로 했으므로
  에이전트의 추가 제품 빌드는 수행하지 않았다. 14:25 EXE 갱신과 Server/Client 실행은 읽기 전용으로 확인했다.
- Source 저장 4검사(CAS, 무변경/다른 owner 보존, 미해결 Sound의 Save 허용·Publish 거부,
  두 sidecar 중간 실패의 바이트 단위 rollback)와 실제 PowerShell 비동기 Save wrapper 1검사 PASS.
  Windows PowerShell 5.1의 null backup `File.Replace`가 최종 receipt 쓰기를 실패시키던 문제도
  job 소유 backup 경로를 사용해 고쳤다. 저장됐는데 UI가 실패로 남을 수 있던 실제 경로다.
- generation 검사 3건 PASS. 미완성 source V2/Sound 변경은 제품 generation을 바꾸지 않고,
  게시된 V2/Sound 변경은 generation을 바꾼다.
- auxiliary drag CPU 검사 최종 15건 PASS. native source-only inventory는 42 patterns / 194 stages /
  8 Summon occurrences / 3 World triggers이며 Product/effect reader 호출을 abort sentinel로 차단해 확인했다.
  환경 typed patch 왕복, 8개 잘못된 입력 거부, v5 parser 실패 보존과 Camera offset/duration도 PASS.
- 변경 JSON 22개와 project/filter XML 2개 parse, 새 TU 단일 등록, `git diff --check` PASS.
- 최신 쿠크 저장본 revision 1556에서 generated Encounter/PatternBindings가 stale임을 확인해 공식 projector로
  generated 두 파일만 다시 게시했다. 사용자 authoring 파일은 변경하지 않았다.
- `Publish-GameplayBalance.ps1 -Mode Publish` PASS, `Gameplay.bootstrap`은 14:30:57 갱신됐다.
  145개 presentation artifact로 재계산한 generation
  `9a858a90804166b8fd1bc959078c6d056ab65c084463428930f575867c102b04`가 실제 bootstrap 행과 일치한다.
  게시 로그는 `out/ValtanUnifiedSequencer20260918/gameplay-publish.log`, 재계산은 `generation-final.json`이다.
- 실행 중인 14:25 Server/Client는 게시보다 먼저 시작됐다. 새 제품 데이터 적용은 사용자가 두 프로그램을
  다시 시작한 후 확인한다. 에이전트가 Reload·프로세스 종료를 수행하지 않았다.
- Client/UI를 실행하거나 화면 판정을 대신하지 않았다. 사용자는 새 EXE에서 Valtan Sequencer의
  Play/Pause/Reset/seek, Full Restore Solo/Group, resource 추가·drag·Save 후 다시 열기를 확인한다.
  실행 중 도구의 미저장 draft는 외부 파일 변경으로 자동 교체하지 않는다.

### 이번 재개와 구별할 과거 계획

09-09의 빈 formatVersion 2 Draft Pattern 및 저자가 지속 저장하는 자유 row ID 도입은 이번 변경에
포함되지 않았다. 현재 source schema와 Server의 실제 Stage/액션 계약을 유지한 편집 기능을 구현했다.
Source 저장 성공, strict Publish, EXE 빌드와 사용자 화면 판정은 서로 별개의 완료 단계다.

---

## 2026-09-09 조사 당시 기록



후속 실행 준비에서도 새 저작 기능은 미구현이다. Source Save Python 초안은 UI 소비자와 Product snapshot 분리가
완료되지 않아 `out/ValtanCompositionParity20260909/unfinished_source_save/`에 파일·patch·보존 기록으로 남겼다.
초안 3개만 작업 전 상태로 되돌려 기존 Save를 유지했다. 후속 Product 빌드 PASS는 이 계획의 구현 완료가 아니다.

대응 [구현 계획서](C:/Users/user/Desktop/LostArk/.md/GB/09-09/2026-09-09_VALTAN_COMPOSITION_AUTHORING_PARITY_IMPLEMENTATION_PLAN.md)에
Source Save, V1/V2 append·box/detail, row와 Draft Pattern, local preview 및 Product Publish의 변경 범위를 정리했다.
쿠크의 좋은 저작 흐름을 기준으로 하되 현재 발탄의 source, Server authority와 재생기를 그대로 확장하는 계획이다.

## 현재 실측

기준 HEAD는 `591012dbebf7eeab0b660baec42852b9396e77d4`, 브랜치는 `codex/dimensionmaster-tool-round3`다.
조사 당시 HEAD와 origin/main은 같았고, 쿠크·이펙트·캐릭터 관련 다른 세션의 미커밋 변경이 있었다.
그 변경을 유지한 현재 working copy의 코드와 Source를 읽었다.

읽기 전용으로 다음을 확인했다.

- `python Tools/ValtanPipeline/valtan_tuning_pipeline.py --repository-root . validate`: PASS.
  managed pattern 42, legacy pattern 25, combat object 9, world member 97, projected artifact 9.
- 실제 Save writer가 호출하는 `validate_and_project`, Sound candidate dependency 검사,
  V2 binding candidate 검사도 현재 데이터로 PASS였다. 파일 commit은 실행하지 않았다.
- BOSS_VALTAN V2 source는 format 2, binding 102개이며 NATURAL 100개, STAGE_END 2개다.
- Save는 기본 Source·descriptor 5개와 조건부 Product 8개, dirty Sound/V2를 포함하면 최대 15개 target 후보를
  구성한다. 실제 쓰기는 바뀐 bytes만 수행한다. “현재 70개 검증이 항상 실패해 모든 Save가 불가능”이라는
  설명은 이 실측과 맞지 않는다.

## 실제 구조적 결합

현재 Save에는 dirty owner/CAS/atomic writer가 있으나 전체 `validate_and_project`, Sound/V2 join,
전체 source manifest와 Product reopen이 일반 Source 저장에도 연결돼 있다.
준비되지 않은 패턴 하나가 전체 editor reopen/저장 허용 상태에 영향을 줄 수 있는 구조다.
과거 2026-09-04 `missing V2 read-set` 오류는 현재 구현에서 고쳐진 이력이므로 현재 실패로 재사용하지 않았다.

V2에는 Save와 별개의 정확한 UI 결함도 있다. `VALTAN_BIND_SLOT`의 shout 세 binding은 고유 binding ID와
`clip.02/03/04`를 가지지만 현재 UI는 convenience stage 필드와 합성 ID를 소비한다.
runtime source-window 식의 올바른 시작은 1400/2300/3200ms인데 0ms의 같은 선택 ID로 충돌할 수 있다.
typed bindingId와 clock basis를 모든 선택·편집·저장·재생에서 동일하게 쓰는 변경을 계획했다.

현재 V2 append는 STAGE/NATURAL 등의 고정 기본값을 주고 detail은 start 중심이며,
V1은 기존 source draft·cue 편집 경로가 존재한다. V1을 저장본 재생만 가능한 기능으로 판정하지 않았다.
표시용 subrow packing은 저자가 저장한 row ID가 아니며, 빈 Draft Pattern 생성도 현재 Product intake와 결합돼 있다.

Source-only Save를 먼저 열기 전에 runtime snapshot 분리가 필요하다.
live `CEffectV2Runtime::Ensure_Bindings`가 authoring catalog revision을 따라가는 경로를 확인했다.
기존 presentation generation receipt의 immutable binding/leaf/group bytes를 제품이 소비하고,
local preview에는 명시 draft snapshot을 전달하도록 계획했다. 새 runtime/manifest는 만들지 않는다.

## 계획으로 정한 구현 단위

| G | 계획한 결과 |
|---|---|
| G01 | 정상 Source inventory와 오류 row 보존, Product snapshot 고정 |
| G02 | Source-only Save, owner별 CAS/atomic rollback, 미완성 Draft schema와 정확한 저장 상태 |
| G03 | V2 bindingId·typed clock·source-window·실제 finite end의 UI/runtime/publisher 일치 |
| G04 | Resources typed append와 V1/V2 Box Detail의 실제 owner mutation 연결 |
| G05 | 빈 Draft Pattern, 지속되는 row ID와 row assignment |
| G06 | 선택 draft local preview, 기존 encounter 전체 atomic Publish 및 Server revision 보존 |
| G07 | 기능별 최소 컴파일·focused 입력/저장 검사와 사용자 아레나 재생 인계 |

발탄 raid의 scripted/cross-pattern 참조는 기존 encounter 전체 Product 단위로 검증·배포한다.
실패한 일부를 이전 Product와 섞어 새 revision 전체가 적용됐다고 하지 않는다.
Product에 연결되지 않은 새 Draft는 정상 저작 Save를 막지 않게 한다.

## 수행 상태

계획서와 이 조사 RESULT만 추가했다. 발탄 C++/Python/Source schema는 이번 요청에서 수정하지 않았다.
새 schema·source-only Save·V2 identity 수정·row/Draft 기능은 모두 미구현이다.
현재 source의 읽기 전용 구조 검증과 문서 검토를 수행했으며 Client compile/link는 실행하지 않았다.
Workbench 실제 버튼 Save/Reopen, local/Server Play, Client 시각 결과는 이번 조사에서 실행하지 않았다.
그 결과는 사용자의 직접 조작과 후속 구현 검증 후에만 기록한다.

PLAN/RESULT의 UTF-8, 모든 링크 대상 존재, 신규 파일을 포함한 공백 검사를 확인했다.
문서 확인 근거는 [documents_verified.json](C:/Users/user/Desktop/LostArk/out/DimensionMasterScale20260909/documents_verified.json)에 있다.
