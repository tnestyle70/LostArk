# 발탄 Composition 저작 흐름 정리 구현 계획서

## G30. 2026-09-28 추가 — 같은 frame의 Effect trim과 Detail 저장 일치

현재 Sequencer는 frame 시작의 immutable Pattern을 소비한다. Effect trim이 typed Balance draft를 바꾸고 Detail identity를 비운 뒤 같은 frame의 `Render_Details`가 이전 Pattern 값으로 Detail draft를 다시 채운다. 다음 Save는 최신 Balance cue와 다른 이 값을 미확정 Detail 편집으로 오인해 trim 이전 수치로 되돌릴 수 있다. `Save 19 is running`은 비동기 저장 job 상태이며 이 stale overwrite와 같은 원인으로 단정하지 않는다.

`Client/Private/ValtanActionWorkbench.cpp`의 `Render_Details` 진입에서 frame이 활성 상태이고 그 frame의 draft generation과 현재 Balance generation이 다른 경우에만 최신 `Get_ValtanPatternDraft`를 함수 안의 local snapshot으로 읽는다. 현재 selected stage를 그 snapshot에서 찾고 Detail의 초기화·검증·Apply는 모두 같은 최신 값으로 수행한다. 다른 pane이 참조 중인 shared immutable frame/cache를 교체하거나 캐시 포인터를 무효화하지 않는다. 세대가 같은 idle frame에는 재조회·문서 복사를 추가하지 않는다. 조회 실패는 상태 메시지와 기존 Detail draft를 보존하며 오래된 값을 재초기화하지 않는다.

Animation 삭제는 기존 Sound cascade만 호출하고 V2 clip reference를 검사하지 않아 orphan을 만들 수 있었다. 현재 V2 catalog에는 stable ID와 baseline/dirty/revision을 함께 원복하는 transaction API가 없으며 Append는 새 ID를 할당한다. 이번에는 삭제 전에 complete authoring V2 snapshot의 exact Pattern/Stage/action/clip reference를 검사해 연결이 남으면 binding ID와 선행 삭제 안내를 표시하고 아무 owner도 바꾸지 않는다. snapshot을 확인할 수 없을 때도 삭제를 진행하지 않는다. 이미 존재하는 별도 V2 삭제와 한 번의 Source Save를 사용하며 자동 다중 owner cascade API는 추가하지 않는다.

기존 dirty bytes/hash와 인코딩을 보존한다. 실제 production 함수·조건을 추출한 비UI regression에서 trim→같은 frame Detail→Save의 전후, left/move/group owner 갱신, 세대가 같은 사용자 Detail 값 보존, 최신 조회 실패 보존 및 V2 참조 exact-match/무관한 참조/no-snapshot의 변경0을 확인한다. 해당 CPP 최소 컴파일과 diff check를 수행한다. Source freshness/CAS, 비동기 job 상태, publisher와 사용자 파일은 이 수정에서 변경하지 않는다. public H 계약과 프로젝트 등록은 추가하지 않는다.

## G29. 2026-09-28 추가 — Effect 양끝 편집·위치 저장·Shift 선택

현재 SECOND_SMASH의 500ms Animation에 붙인 natural Effect를 오른쪽에서 줄이면
종료가 Animation 끝으로 clamp되어 긴 박스가 약401ms로 축소된다. ONCE Effect의 종료는
자기 수명으로 검증하고 시작 호출만 정확한 Animation/Stage에 연결한다. 왼쪽 grip은
호출 시각과 실제 Effect resource age를 함께 옮겨 잘린 앞부분을 다시 재생하지 않는다.
optional `playbackOffsetMs`는 resource age의 명시적 시작값이다. 누락은 기존 Full Restore
원점을 유지하고 명시0은0초를 뜻한다. Source/typed tree/Save/Product parser/preview/runtime
전체에서 값과 존재 여부를 보존한다. 기존 source 파일을 일괄 변환하지 않는다.

Effect Detail의 transform은 유효한 편집이 끝나면 typed Balance draft에 반영하고 같은
cursor의 preview를 갱신한다. Source Save가 실제 draft를 직렬화하고 실패는 기존 상태를
보존한다. Shift 클릭은 stable box identity별 선택을 누적·해제하며 기존 단일 선택과
clipboard/선택 명령을 연결한다. 작은 박스의 좌우 grip이 겹쳐도 클릭 위치로 구분한다.

검증은 실제500ms clip과 긴Effect의 소폭 right trim, 앞trim 후 resource age, 저장/재읽기,
transform의 typed draft/runtime 전달, Shift 선택과 실패 보존을 기존 focused probe로 한다.
Sound도 optional `playbackOffsetMs`와 `playbackDurationMs`로 실제 음원 구간을 저장한다.
Animation cue 시각은 호출 위치만 정하며 음원 수명은 clip 재생률로 줄이지 않는다. SPIN에서
WINDUP 또는 다른 Stage로 이동할 때 target Stage/action/clip을 원자적으로 교체하고 기존
binding/occurrence ID는 유지한다. 복제는 새 ID에 현재 음원 구간을 보존한다. 기존 Sound
manager의 age/stop 경로를 확장하고 Preview·제품 소비를 같은 계약으로 연결한다.

3회 땅 치기 후 돌진 검색에서 누락된 exact V1 N/S `par_*_rpbf_dash_01_1` 원본은
본문을 복제하지 않고 기존 library leaf에 한국어 패턴 검색 alias를 연결한다. 실제 runtime
cue 연결과 편집용 검색 분류를 구분한다. 확장 원형 예고(native2614)와 추적 도끼 원본02
(native2599)는 기존 쿠크 GroundEffect receiver 필터를 재사용해 actor GBuffer 수신을
제외한다. projection 높이·Bloom·색·크기·사용자 저장본은 바꾸지 않는다.

사용자 Data/Bloom과 실행 중 draft는 교체하지 않는다. 변경 TU 컴파일 및 가능한 정상 증분
Product build 후 실제 UI 확인은 사용자에게 남긴다. 새 C++ 파일은 계획하지 않는다.

## G28. 2026-09-28 추가 — 컷씬 Effect를 All Effects에 노출하고 Append 대상 연결

`ValtanCinematicEffectLibrary.h/.cpp`에서 기존 WorldSequence document loader를 사용해 실제
source-preview instance/template의 effectTracks를 Sequence별로 읽는다. 진입·버러지·2페이즈·
피자·사망의 실제 연결과 stable resource ID를 보존하고 asset ID 목록을 하드코딩하지 않는다.
새 두 파일은 Client 프로젝트와 filters의 물리 소스 위치에 등록한다. 신규 UTF-8 TU는
기존 CP949 PCH와 섞이지 않게 해당 항목에 `PrecompiledHeader=NotUsing`을 명시한다.
MainApp의 객체 섹션 한도는 해당 TU의 `/bigobj`로 처리한다. All Effects와 Composition
Resources는 같은 library를 사용하며 source 문서와 게시 runtime의 역할을 구분한다. 목록은
초기화/명시적 Refresh 때만 갱신하며 매 프레임 JSON을 읽지 않는다.

Effect Tool의 기존 resource Open Editor/Preview/Copy 명령을 연결한다. 반복 occurrence는
Sequence별 실제 start/duration/track ID로 식별하고 다른 asset으로 대체하지 않는다. 재사용
Effect 본체를 추가하는 명령과 컷씬의 배치·시각을 복사하는 명령을 혼동하지 않는다.

`ValtanActionWorkbench`의 Append 대상은 선택한 Animation/Effect의 owner clip, 현재 cursor의
Animation, Stage의 첫 Animation 순서로 유효한 대상을 제안하고 선택값을 화면에 표시한다.
사용자가 dropdown에서 지정한 대상은 유지한다. 여러 clip이 있다는 이유만으로 빈 대상 상태에
머무르지 않으며, WAIT/Animation 없음·stale admission은 계속 거부한다. 추가는 기존 typed
Balance draft 경로로 수행하며 이미 있는 cue를 덮어쓰지 않는다.

사용자가 방금 저장한 Bloom 값과 모든 Data 원본은 수정하지 않는다. 실제 sequence inventory와
catalog join, 새 소스 등록, 최소 native/컴파일 검증 및 최종 Product 빌드로 확인한다. UI 조작과
최종 화면 판정은 사용자가 수행한다.

## G25. 2026-09-28 추가 — 연속 스크럽·Effect 끝점·Sound 수명

`ValtanActionWorkbench.cpp`의 NATURAL/ONCE 오른쪽 grip을 활성화한다. 끝을 끌면 기존
`sourceEndMs`와 `cue_end`를 함께 stage하고 stable cue/occurrence와 다른 필드를 보존한다.
반복 clip의 ONCE는 시작 시점은 Stage 안에서 검증하되 종료는 누적 source clock으로 표현해
Stage 뒤의 자연 tail도 자를 수 있게 한다. 명시적 종료까지 이어지는 해당 발탄 tail만
`EFFECT_SPAWN_DESC`의 기본 false인 `bPreserveBossActionTail`로 전달한다. 실제 duration expiry와
owner reset은 유지한다. 새로운 저장 schema나 별도 Effect runtime은 추가하지 않는다.

`Animation_Tool_ValtanPlayback.cpp`의 reset seek는 앞 Stage의 아직 살아 있는 표현을 복원한다.
UI Render에서 생성한 V1 pending을 다음 프레임 Late_Update 전에 해당 preview owner에 한해
commit/sample하여 연속 스크럽이 새 객체를 표시 전에 제거하는 순서를 해소한다.
`Effect_PresentationService`는 기존 spawn/age/follow 경로를 사용한다.

Sound box는 Preview와 동일한 occurrence/loop hash로 WAV variant를 선택하고 기존
`Get_SoundDurationMs`로 끝을 구한다. native clip clock은 시작/반복 시점에만 쓰고 WAV 길이를
clip playRate로 나누지 않는다. EACH_LOOP 및 Stage 뒤 tail도 표시 범위에 포함한다.
신규 C++ 파일은 없으며 기존 프로젝트 등록을 유지한다. 실제 소스 함수/설치 리소스 기반
비UI seek·종료·저장 경계 검증과 정상 Debug Product Build를 수행한다. 화면 판정은 사용자 담당이다.

## G26. 2026-09-28 추가 — Source Save 이후 재개 지연

현재 SourceOnly writer 뒤의 manifest 중복 실행과 Workbench Product reopen 비용을 측정한다.
`Reload_Canonical`, `Reload_AfterPendingSave`, `Mark_SourceCommitted` 및 실제 source save job
소비에서 저장 이후 불필요한 Product 재검사를 제거한다. Source 저장의 typed validation,
writer admission, 최신 저장본 CAS, atomic replace/rollback은 유지하며 Product 준비와
Server 적용은 명시적 Publish에 남긴다. 저장 완료 문구와 source-only 상태를 정확히 갱신하고,
실패 시 draft와 이전 snapshot을 보존한다. 데이터는 사용자가 Save할 때만 변경된다.


## G27. 2026-09-28 추가 — 벽 preset과 컷씬 저속 조사

`CValtanBossTool::Set_ServerArenaPreset`의 Pattern canonical gate만 분리하고 기존 publisher
barrier, active Valtan arena 및 typed Server preset 검증을 유지한다. 미게시 패턴 편집이
환경 상태 변경을 막는 원인과 실제 Server 실패보존 경계를 대조한다.

사용자가 저장한 `2페이즈컷씬캡쳐_20260928_042931_170_frame141_70404_1.json`과 직전
캡처의 CPU scope·draw/cull counter를 읽어 Composition Build, DebugTools Update와
Deploy draw 비용을 분리한다. 컬링 누락이나 동일 cue 중복은 실제 소비 경로와 수치로 확인하고
카메라·Bloom·렌더링 옵션을 임의로 바꾸지 않는다. 실제 성능 개선량은 수정 후 사용자 캡처로 확인한다.
카메라 문서는 Stage별 action/occurrence가 아닌 actor preview 세션에서 재사용하고 Stop에서
해제한다. 성공 Save/Reload는 snapshot을 갱신한다. 기존 CPU scope에 camera/document 및
cinematic/destruction 구간을 추가해 남은 프레임 지연을 분해한다.

## G23. 2026-09-28 추가 — Full Restore Preview와 실제 수명 박스

사용자는 6방향 후 전멸의 Full Restore를 Append·Save한 뒤 Preview에서 보이지 않는 현상과
Effect 박스가 점으로 표시되는 현상을 수정하도록 요청했다. V2 6방향의 재생은 사용자가 확인했고,
후속으로 V2 박스도 드래그가 막힌다고 제보했다. 현재 5개 V1 cue는 WINDUP 반복 clip에
342/950/1475/2067/3358ms로 저장되어 있다. Native clip은833ms, Stage는1800ms이며
runtime의 source window 검사와 저작 시계 해석이 불일치한다.

`ValtanActionWorkbench.cpp`의 Build_Timeline은 NATURAL을 end=start로 만드는 분기를 제거한다.
`CEffectPlayback::Calculate_ElementEndSeconds`와 기존 ModelCue·OwnerControl 종료 규칙으로
resource 수명을 구하고, NATURAL tail은 Stage 끝에서 자르지 않는다. V2도 Snapshot의 실제 leaf와
group stop·particle/trail tail을 따르며 최소 픽셀 너비를 재생 수명으로 대신하지 않는다.
`ValtanActionWorkbench.h`의 timeline cache는 V1 catalog revision도 검사해 Effect Save 뒤 갱신한다.

Preview는 저장·미저장 draft의 기존 cue 전달 경로를 유지하고, 독립 Append된 Full Restore의
trigger와 effect 내부 source 시계를 분리한다. 저장만 한 V1은 기존 priority preparation queue로
준비하고, 해당 generation이 준비되면 같은 draft와 요청 cursor에서 자동 시작한다. 준비 실패나
selection·draft 변경은 이전 preview를 보존하며 취소한다. 애니메이션 마지막 pose를 유지하는
기존 preview tail 경로를 사용해 Effect 종료까지 재생하고 Server Stage 길이는 변경하지 않는다.

V1과 V2 드래그는 전체 Pattern의 박스 시작 시각을 목적 Stage/clip 시계로 변환한다. 기존 Stage
끝으로 clamp하지 않는다. V1은 기존 typed transaction의 Remove+Add, V2는 scope 이동 전용
typed API로 asset·anchor·stop·repeat를 보존한다. 이전 원본 cue의 source trim은 유지한다.
잘못 저장된 V1 두 건은 전체 시각2067/3358ms를 보존한 최신 source patch로 교정한다.
원래 유효한 세 건과 무관한 사용자 편집은 보존하며 자동 Reload하지 않는다.

새 C++ 파일은 추가하지 않는다. 기존 native probe로 실제 자산 수명·tail·박스 구간·dispatch·
Stage 이동과 실패 보존을 검사하고 변경 TU 컴파일과 제품 링크를 수행한다. Client/UI와 최종
화면 판정은 사용자가 수행한다.

## G24. 2026-09-28 추가 — F1 벽 상태와 저장 버튼 배치

`MainApp.cpp/h`의 기존 Server Arena Active > Valtan에 있던 다섯 지형 preset 본문을 공통
helper로 옮겨 F1의 Valtan Arena 아래에 직접 노출한다. 기존 typed Server 요청·응답과 pending
잠금을 재사용하며 Client의 맵 상태를 로컬로 우회 변경하지 않는다. Debug의 기존 탭도 같은
helper를 호출한다. Debug/Release 각 TU 컴파일로 공통 F1 노출을 확인한다.

Composition Sequencer의 왼쪽 저장 버튼 표시는 Save로 바꾸고, Save & Publish는 같은 줄의
가장 오른쪽으로 이동한다. Save와 Publish의 기존 기능·검증·상태 및 Ctrl+S 의미는 유지한다.

## G16. 2026-09-28 추가 — All Effects 목록 동기화와 4방향 sector 분리

사용자는 `VALTAN_SEQUENCE_FOUR`의 부채꼴 내부 채움이 멈춘 현상과 Resources의
Product/Full Restore 누락, V1/V2 미분류, Preview/Append 비활성을 함께 수정하도록 요청했다.
후속 결정에 따라 원본 Full Restore는 보존하고 sector를 제외한 본체 복원본과 독립 sector를
분리한다. 기존 네 개 V2 impact occurrence는 유지하며 sector의 시각·방향은 독립적으로 편집한다.

실측 대상은 `effect.valtan.action.420624.stage007.full.restore`, 원본 clip
`mesh_att_battle_19_01`이다. 248개 element 중 native2615 GroundEffect 네 개의 `inner=0.5`는
고정값이며 시간 track이 없다. 쿠크 원형 장판이 사용하는 기존 material parameter track을
재사용한다. 현재 V2 Group은 LEAF/GROUP만 지원하고 쿠크 Composition의 V1_ELEMENT와는 별개다.
따라서 원본 native2615 재질을 유지하는 독립 V1 sector cue를 현재 V2 타격과 병행하는 후보로
준비한다. 전체 projector scale이나 모든 재질의 회전을 바꾸지 않는다.

`ValtanActionWorkbench`의 Resources는 All Effects가 소비하는 공용 inventory와 실제 owner를
기준으로 Product/Full Restore/V2 leaf/group을 빠짐없이 표시한다. V1/V2를 명시적으로 분류하고,
Preview는 기존 Effect Tool·authoring sequencer를 MainApp 요청 소비자에 연결한다. Append는 기존
V1 clip occurrence 및 V2 stage binding writer를 사용한다. 단일 animation clip은 자동 선택할 수
있지만 여러 clip의 목적지를 임의로 정하지 않는다. Server Sound 준비 상태는 로컬 리소스
Preview/Append의 선행조건으로 사용하지 않는다. Save/CAS 및 Product Publish의 검증은 유지한다.

sector 분리 생성기는 원본 element·재질·resource를 보존하고 새 후보만 준비한다. 최신 저장본 기준
승인 후 stable ID와 변경 field로 병합하며 교체 직전 freshness·backup·원자 교체·실패 rollback을
유지한다. 새 Data JSON은 Client `96.DataFiles` None 항목에 등록한다. 새 C++ 파일은 계획하지 않는다.

검증은 실제 목록의 ID 집합 비교, 새 V1/V2 codec·resource closure, sector 시간별 inner·alpha·방향
수치, 변경 C++ 최소 컴파일, JSON/XML parse와 diff 검사다. 현재 실행 중인 Client의 미저장 draft를
자동 Reload하지 않는다. 실행 파일 링크가 실제 점유 때문에 막히면 그 단계만 사용자 종료 후
정상 Product Build로 완료한다. Client/UI 실행과 최종 아레나 화면 판정은 사용자 전용이다.

### Full Restore 분리본의 원본 clip 계약

`ValtanPatternTree::Load_FullRestoreSourceClips`와 기존 Python metadata writer에 optional
`variantId`를 연결한다. 값이 없으면 기존 action/stage ID가 그대로 유효하며, 있으면 검증한
action/stage 뒤의 단일 variant token과 `.full.restore`로 ID를 구성한다. `body`와 `sectors`
분리본은 원본 `mesh_att_battle_19_01`의 시간과 clip을 공유한다. 원본 mapping을 교체하지 않으며
잘못된 variant, ID 불일치, 중복이 있으면 기존 읽기 snapshot을 보존한다.

## G17. 2026-09-28 추가 — 추적 도끼 원본 전수 대조

사용자가 추가로 요청한 `점프 후 플레이어 추적 도끼`의 원본 action420610 전체46 Stage를
조사한다. PlayParticleEffect, PlayDecalEffect, Trails와 현재 Product/Full Restore/V2 연결을
clip·notify별로 대조한다. 큰 원형8회, 작은 원형4회, 도넛6회는 서로 다른 크기·시계·형상이며
이름만으로 같은 프리셋을 적용하지 않는다. 큰 원형의 native2614 재사용 후보를 준비하고
source 크기·fade와 프로젝트가 추가한 inner 보간을 분리해서 기록한다.

## G18. 2026-09-28 추가 — Source Save와 게시 상태, 실제 V2 타임라인

사용자가 4방향 V1을 삭제한 현재 저작 저장본은 `effectCues=[]`이며 그 삭제를 보존한다.
기존 게시본에는 삭제 전 cue가 남아 있다. Save와 Publish를 명확히 구분하는 버튼·상태를
기존 writer에 연결하고 Source 저장만으로 Server 반영 완료를 표시하지 않는다.
`VALTAN_SIX_PIZZA_106`의 V2 Stage05/07/11 바인딩과 실제 Sequencer projection을 대조해
서버에서 사용하는 연결이 저작 타임라인에서 누락되는 원인을 수정한다. 연결 자체를 없애거나
이전 게시본으로 저작본을 덮어쓰는 방식은 사용하지 않는다.

## G19. 2026-09-28 추가 — 구르기 후 돌진 복원과 반복 클립 저장

사용자가 `VALTAN_DASH_CHARGE` 편집 저장에서 제공한 실제 실패 patch는 원본
`400424/0`의 `mesh_att_battle_4_01`을 두 번 붙인 편집이다. 원본 Sequence 선언을
한 번만 사용할 수 있는 provenance coverage와 기존 clip의 시간 편집까지 새 source로
계산하는 부분을 조사한다. 원본 tuple·clip 순서·역할 검증을 유지하면서 유효한 반복과
기존 occurrence 시간 편집이 저장되도록 수정하고 실제 실패 patch 및 잘못된 source 거부를
검증한다. 미저장 UI draft를 Reload하거나 디스크 저장본으로 대체하지 않는다.

동일 패턴의 원본 action·clip·Full Restore·Product 연결과 화면의 cyan ring을 대조한다.
다른 정상 effect에 전역 회전·크기 보정을 전파하지 않고, 확인된 carrier 원인에만 복구 후보를
준비한다. 데이터 후보의 최종 반영은 기존 편집 중 데이터 반영 절차로 수행한다.

## G20. 2026-09-28 추가 — 6방향 후 전멸 원본 확인

표시명 `6방향 후 전멸 패턴`은 `VALTAN_FLOOR_WIPE_130` / action420630이다.
`VALTAN_SIX_PIZZA_106`의 중앙 이동 후6방향·피자와 구분한다. 원본 animation notify의
부채꼴·원형과 현재 V1/V2/Full Restore의 실제 연결을 조사한다. 사용자 요청은 이 항목의
원본 확인이므로 새 리소스나 자동 패턴 연결을 추가하지 않는다.

## G21. 2026-09-28 추가 — 선택 검격의 독립 그룹과 복제

사용자가 선택한420609/stage008의 첫 검격9요소를 기존 source animation·particle 흐름을
유지한 채 독립 그룹으로 만들 수 있게 한다. group by anchor의35요소 전체를 변경하지 않고,
선택 stable element ID만 manual group 및 고유 runtime anchor slot으로 분리한다. 기존 원본
source slot과 실제 bone은 유지한다. 그룹 Duplicate는 기존 element 복제 codec과 내부 참조
remap을 재사용하며 복제본끼리 anchor 편집이 전파되지 않게 한다.

source transform track이 있는 요소의 Detail Transform 제한을 제거하지 않는다. 기존 Anchor
Position/Rotation 경로로 source particle의 이동 방향까지 그룹과 함께 변환하며, 검격별 시작
시각 조정은 기존 source clock offset 계약을 따른다. 실제 선택9요소의 복제·독립 anchor·시간
이동과 실제 Playback 수치를 검증한다. 사용자 현재 Effect 문서와 미저장 선택을 자동 교체하지
않고 새 UI 기능으로 사용자가 그룹을 생성·복제하도록 연결한다. 새 C++ 파일은 추가하지 않는다.

## G22. 2026-09-28 추가 — 발탄 한글 리소스 이름과 에테르의 구슬

발탄 All Effects와 Composition Resources에서 기술적인 asset/clip 이름 대신 패턴 이름과
원본 동작 순서로 리소스를 찾게 한다. 기존 ValtanPatternTree의 공용 표시 helper를 두 목록이
소비하며 stable ID, 저장 경로, clip과 cue 연결은 바꾸지 않는다. 정확한 동작 의미가 검증되지
않은 항목은 원본 이펙트와 순번으로 표시하고 원본 clip/ID는 상세와 검색에서 유지한다.
Product, Full Restore, V1/V2 및 공유 리소스의 현재 분류·Preview·Append 소비자를 재사용한다.

잡아채기 후 불어날리기의 현재 Full Restore는21_01과21_03 두 개다. 마지막21_04의 원본
visual을 확인해 누락된 복원본 한 개를 별도 후보로 추가하고 기존 두 복원본과 Product를
보존한다. 에테르의 구슬은 사용자가 제공한 파란 구체를 원본/기존 리소스에서 식별해
한글 표시명의 독립 이펙트로 분리한다. 추측한 새 shader나 임의의 대체 구체는 만들지 않는다.
새 Data 문서는 catalog/tree/96.DataFiles 및 필요할 경우 원본 clip metadata에 함께 등록한다.

사용자가 EXE를 종료하고 다시 실행하겠다고 요청했으므로 최종 제품 Build를 수행한다.
최신 디스크 반영 승인, backup/CAS/atomic 교체와 사용자 직접 화면 확인 경계는 유지한다.

## G14. 2026-09-27 추가 — 안내 네 줄 제거와 타임라인 글자 높이 확보

`ValtanActionWorkbench.cpp`의 타임라인에서 드래그 안내, 전체 길이 요약, 길이 계산 설명과
Stage Gap 편집 불가 안내를 제거해 canvas가 사용하는 세로 공간을 늘린다. 선택 label과
Duplicate/Delete 버튼, 가능한 Stage Gap 및 Animation timing 입력은 유지한다.
텍스트만 출력하던 `Render_PatternDurationControl`은 CPP 정의·호출과 H 선언을 함께 제거한다.
새 파일·project/filter 등록과 데이터 변경은 없다. 변경 TU 컴파일과 diff 검사를 수행하며,
제품 실행 파일은 사용자가 나중에 직접 정규 Debug 빌드로 반영한다.

추가 제공 화면은 폰트 높이가 고정 22px box보다 커서 Stage/Animation/Camera 글자 하단이
잘린다. `CompositionTimeline.h`에 현재 폰트·style padding을 반영하는 높이 helper를 추가하고
발탄 `Render_Timeline`의 ruler·lane·box·hit-test가 매 frame 같은 높이를 소비하도록 한다.
box 최소 32px, row 최소 38px이며 더 큰 폰트에서는 필요한 만큼 증가한다. 공통 기존 상수와
다른 도구의 높이, 시간에 대응하는 가로폭, 저장 duration과 row 수는 변경하지 않는다.

## G15. 2026-09-27 추가 — 가로 스크롤과 Preview 이펙트의 같은 시계

사용자는 줌 배율 차이가 자신의 DPI 변경 때문이라고 정정했다. DPI·줌 범위·배율·Fit은
변경하지 않는다. 하단 가로 scrollbar에서 전체 sequence를 좌우 탐색할 수 있도록 canvas의
실제 content extent와 scrollbar 영역을 연결한다. 저장 시간·box 배치와 row 수는 유지한다.

`Animation_Tool`의 Pause는 master clock을 멈추지만 현재 발탄 V1 cue와 V2 Stage authoring
spawn은 최초 age를 받은 뒤 각 object의 frame clock으로 진행한다. 이미 외부 시계를 쓰는
combat object와 마찬가지로 기존 Effect presentation/V2 경로의 external sampling을 사용한다.
`Valtan`의 local authoring preview만 전체 timeline age로 V1/V2 occurrence age를 sample하고,
Pause·Resume·seek·Stop과 Stage를 넘는 natural tail의 시계를 연결한다. Server 제품 재생의
기존 clock은 유지하며 두 번째 runtime은 만들지 않는다.

사용자가 제품 빌드는 나중에 직접 하겠다고 확정했다. 이번에는 변경 소비자 최소 컴파일과
필요한 clock 검증을 수행하고 새 EXE 링크·Client/UI 실행·최종 화면 확인은 수행하지 않는다.

## G13. 2026-09-27 추가 — 세이튼과 같은 재생·저작 표면

사용자 확정: Preview는 자유로운 Pause/Seek를 유지하는 로컬 표현 재생이며 Server와 같은
Stage 시간·선택 Logic 결과·오브젝트 수명을 재현한다. Play Pattern은 실제 Server 권위 실행이다.
발탄용 두 번째 Composition runtime이나 쿠크 문서 복사본을 만들지 않는다.

`CompositionTimeline.h`에 세이튼의 기존 palette/box 높이를 공용 상수로 두고 두 Boss에서
소비한다. Stage와 Animation clip의 별도 row·stable identity를 유지하며 색·폰트·여백·선택 표시를
같이 쓴다. `ValtanActionWorkbench.cpp`의 toolbar는 Save/Play Preview/Pause/Reset/Play Pattern과
현재 실행 상태를 분명히 표시하고 기존 source Save와 Server audition 경계를 유지한다.

Resources는 실제 Pattern 연결과 Full Restore의 exact source action/clip join으로 `Patterns`를
만든다. 여러 Pattern이 쓰거나 독립 combat object가 소유한 자원은 `Common`, 미연결 저장 자원은
`Library`에서 찾는다. V1/V2 저장 codec은 그대로 두고 같은 목록에서 각각의 owner로 dispatch한다.
Full Restore metadata join은 `ValtanPatternTree.h/.cpp`의 공용 helper를 Tool/Workbench가 공유한다.
새 `effect.valtan.six-pizza.sectors`는 원본 composite의 네 요소를 복사한 독립 resource이며
기존 Pattern 연결을 자동 교체하지 않는다. 원본 요소의11초 지연을 제거하고 모든 입자 수명은 보존한다.
EffectCatalog 및 Client 프로젝트/필터의96.DataFiles None 항목으로 등록한다.

`Valtan.cpp`의 local combat-object preview는 현재 action이 바뀔 때 이전 돌을 종료하는 결함이
있다. 전체 Pattern clock에서 spawn을 재구성하고 각각의 lifetime/terminal event까지 유지하도록
기존 preview 경로를 고친다. 이동·엄폐·피해는 기존 Server가 계속 소유한다. Sound/Logic은 실제
기존 transport 소비자를 조사해 Pause/Seek/Stop과 같은 시계를 소비하게 연결하며 결과는
source/data-only 검사, 실제 컴파일, 사용자 화면 확인을 구분한다.

## G12. 2026-09-27 재개 — 복원본 검색과 발탄 Sequencer 표시

사용자는 휠윈드·피자 Full Restore를 Effect Editor에서 Play All/Timeline/Solo로 편집하고,
Action Workbench에서 기존 sector·돌·collider와 함께 조립하려 한다. 현재 Full Restore의
원본 clip 재생과 element 편집은 이미 연결돼 있다. Product는 현재 패턴 invocation이며
Full Restore 목록에 있다는 사실만으로 제품 패턴에 연결되지는 않는다.

`Client/Private/ValtanActionWorkbench.cpp`의 Effect Resources는 현재 모든 direct-authored
ID를 분해해 나열하고 ID만 검색한다. 기존 saved Effect organization/inventory의 이름과
category를 함께 사용해 복원본과 기존 resource를 검색·선택할 수 있게 한다. 기존 typed
Append/Save/Publish와 stable ID를 유지하며 목록을 그리기 위해 대형 Effect 본문을 decode하지 않는다.
필요한 검색용 session metadata는 `Client/Public/ValtanActionWorkbench.h`가 소유한다.

같은 CPP의 `Render_Timeline`은 공통 row 높이24/label 폭180을 이미 사용하지만 Fit에서
추가360px를 빼고 있다. Saydon의24px 여백과 box 안쪽 padding1px로 맞추며, canvas 끝은
실제 box의 끝으로 계산해 point 클릭 영역을 보존한다.

`Client/Private/Effect_Tool_Valtan.cpp`는 Product/Full Restore의 역할과 실제 source clip을
명시하고 기존 Open/Play/Save 경로를 유지한다. 새 renderer·runtime·정본 복사본은 만들지 않는다.
피자 sector와 돌은 현재 Server spawn/판정 owner를 먼저 실측하고 resource 종류를 구분한다.
사용자 저작 데이터의 최종 교체가 필요하면 후보 검증을 끝낸 뒤 최신 저장본 기준 승인 절차를 따른다.

검증은 변경 TU 최소 컴파일, 관련 metadata/ID join 검사와 `git diff --check`다. 새 C++ 파일이나
project/filter 등록은 계획하지 않는다. 다른 실행 중 빌드와 IntDir를 공유하지 않는다.
Client/UI 실행과 최종 화면 판정은 사용자가 수행한다.

작성일: 2026-09-09

상태: 2026-09-18 구현 재개. 아래 기존 G의 미구현 범위와 실제 구현 범위는 RESULT에서 구분한다.

## 2026-09-18 재개 범위

현재 요청은 발탄 Composition의 모델 Play/Pause/Seek와 Effect Tool Full Restore의 Solo/Play Group을 연결하고, Saydon과 같은 Resources 분류와 트랙 편집 흐름으로 맞추는 것이다. 같은 `ICompositionWorkbenchSession` shell을 유지하며 발탄의 split gameplay/presentation과 Server 권위는 그대로 소비한다.

- `EffectAuthoringSequencer`와 Effect Tool은 Full Restore가 우회하던 model sampling/anchor clock을 하나로 연결한다.
- `MainApp`, `Animation_Tool_ValtanComposition`과 `ValtanActionWorkbench`는 선택된 발탄 세션의 transport와 실제 model clock을 연결한다. UI의 선택 상태가 다른 보스의 transport로 제출되지 않도록 한다.
- `ValtanActionWorkbench`는 Resources와 timeline domain 순서, 선택/추가/Box Detail/drag 입력을 공통 형식으로 맞춘다. 각 box는 기존 typed owner의 clock으로 변경하고 임시 표시값만 저장하지 않는다.
- `ValtanPatternTree`, `BalanceTool`, 기존 Valtan writer는 source authoring read/save와 strict Product Publish를 분리한다. 변경 owner의 JSON 구조·stable ID·CAS·atomic rollback은 유지한다. 마지막 정상 Product는 source 저장 실패 또는 Publish 실패로 교체하지 않는다.
- `VALTAN_FOUR_SLASH`의 SLASHES/SPIN은 기존 cue identity와 명시적 사용자 scale을 보존하고 stage008/009 Full Restore로 교체한다. 기존 carrier를 중복 추가하지 않는다.
- Summon/World/Scene Profile/Camera/Light는 실제 기존 저장 필드와 소비자를 조사한 뒤 연결한다. 탭이나 빈 lane 이름만 추가한 상태를 구현 완료로 기록하지 않는다.

검증은 변경 C++ 최소 컴파일과 정상 증분 Product Build, source 저장/reopen/Publish 실패 보존, 변경 JSON parse 및 domain publisher, `git diff --check`로 한다. Client 실행과 최종 화면 확인은 사용자가 직접 한다.

기준: `591012db`와 2026-09-09 현재 작업 트리. 쿠크와 이펙트 관련 동시 변경은 보존한다.

이 계획의 목표는 발탄에서 `Composition Patterns → Resources Append → row/box 편집 → Box Detail → local Play → Save → 명시적 Publish → Server Play`를 각각의 책임에 맞게 연결하는 것이다. **Save는 미완성 저작 원본을 안전하게 저장하고, Publish는 실행 가능한 발탄 Product를 만드는 경계로 분리한다.** 기존 발탄 Server 전투와 V1/V2 재생기를 확장하며, 별도 발탄 런타임이나 새로운 manifest 체계는 만들지 않는다.

문서는 개인 계획서 규칙의 구현 계획서 형식을 따른다. G별 파일, 상태, 호출 흐름, 종료 검증을 정하며 H/CPP 전문은 포함하지 않는다. 아래에서 `추가`라고 명시한 함수·필드만 구현 예정 계약이다. 나머지 함수명은 현재 파일에서 확인한 기준점이다.

연결 문서:

- [기존 발탄 V2 binding 구현 계획](C:/Users/user/Desktop/LostArk/.md/GB/09-03/2026-09-03_VALTAN_COMPOSITION_EFFECT_BINDING_IMPLEMENTATION_PLAN.md): V2 저작 연결의 선행 범위. 이 문서는 Save, Draft Pattern, row, Product 소비까지 확장한 별도 범위를 소유한다.
- [발탄 Save pipeline 조사 결과](C:/Users/user/Desktop/LostArk/.md/GB/09-03/2026-09-03_VALTAN_LARGE_DONUT_AND_SAVE_PIPELINE_RESULT.md), [발탄 V2 timeline/Save 결과](C:/Users/user/Desktop/LostArk/.md/GB/09-04/2026-09-04_VALTAN_WARP_PORTAL_V2_TIMELINE_AND_SAVE_PIPELINE_RESULT.md): 과거 상태와 현재 코드의 차이를 확인하는 근거다.
- [쿠크 Composition Workbench 결과](C:/Users/user/Desktop/LostArk/.md/GB/09-05/2026-09-05_KOUKU_SAYDON_ACTION_COMPOSITION_WORKBENCH_AND_BOSS_TOOL_IMPLEMENTATION_RESULT.md), [쿠크 append 결과](C:/Users/user/Desktop/LostArk/.md/GB/09-05/2026-09-05_KOUKU_MODEL_PATTERN_APPEND_SEQUENCE_RESULT.md), [쿠크 pattern/bundle 결과](C:/Users/user/Desktop/LostArk/.md/GB/09-07/2026-09-07_KOUKU_GATE_PATTERN_BUNDLE_RESULT.md): 사용자 흐름과 기존 저장·readiness 처리의 재사용 기준이다.
- [Effect Composition 계획](C:/Users/user/Desktop/LostArk/.md/GB/09-07/2026-09-07_EFFECT_COMPOSITION_WORKBENCH_AND_PATTERN_CAMERA_IMPLEMENTATION_PLAN.md), [결과](C:/Users/user/Desktop/LostArk/.md/GB/09-07/2026-09-07_EFFECT_COMPOSITION_WORKBENCH_AND_PATTERN_CAMERA_RESULT.md): 공통 Composition shell, Resources와 leaf owner 경계를 재사용한다.
- [Effect Tool 북극성 가이드](C:/Users/user/Desktop/LostArk/.md/GB/08-05/2026-08-05_EFFECT_TOOL_G06_SHADER_REMAINING_GUIDE.md), [렌더링·이펙트 복원 계약](C:/Users/user/Desktop/LostArk/.md/GB/렌더링이펙트복원V2.md), [팀 인터페이스](C:/Users/user/Desktop/LostArk/.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md).

## G00. 현재 저장 경로와 변경 경계 고정

### 목표와 종료 증거

현재 Save가 실패하는 정확한 위치를 보존하면서, 저작 저장에 Product 전체 완성을 요구하는 연결을 제거할 준비를 한다. “70개 검증 때문에 현재 모든 Save가 불가능하다”를 전제로 코드를 바꾸지 않는다.

2026-09-09 읽기 전용 검증에서 `valtan_tuning_pipeline.py validate`는 PASS였다. managed pattern 42개, legacy pattern 25개, combat object 9개, world member 97개, projected artifact 9개였다. 이어 실제 Save가 호출하는 `validate_and_project`, Pattern Sound candidate dependency 검사, V2 candidate binding 검사를 현재 데이터로 실행해 모두 PASS를 확인했다. V2 binding 문서는 formatVersion 2, binding 102개다. 이는 현재 source 검증 결과이며, Workbench 버튼을 누른 Save나 Client 화면·재생을 검증한 결과가 아니다.

현재 Save의 조건부 Product 산출물은 8개다. 기본 Source·descriptor 5개와 합하면 13개 target 후보이며, dirty Sound/V2 owner가 함께 있으면 최대 15개다. 실제 쓰기는 바이트가 바뀐 target만 수행한다. 이 숫자를 “70개 저장 대상”이나 “70개 필수 하네스”로 설명하지 않는다.

### 수정 파일과 현재 함수

| 파일·현재 기준점 | 현재 책임과 이번 변경에서 유지할 것 |
|---|---|
| `Client/Private/ValtanActionWorkbench.cpp:4831` `Save_Reload` | dirty Pattern/Sound/V2 owner를 모아 비동기 Save job을 요청한다. UI 프레임에서 파일을 직접 쓰지 않는 구조를 유지한다. |
| `Client/Private/BalanceTool.cpp:5390` `Begin_ValtanCompositionSave`, `:5413` `Begin_ValtanSaveJob`, `:5518` `Launch_ValtanSaveCommand` | Save job 상태와 staging 파일을 소유한다. `-CommitOnly`도 이미 존재하지만 현재는 source만 쓰는 의미가 아니다. |
| `Tools/ValtanPipeline/Run-ValtanAuthoringSaveJob.ps1` | canonical commit 뒤 SourceManifest를 다시 생성한다. 선택적 Publish와 durable commit 완료를 별도 단계로 기록하는 기존 receipt를 유지한다. |
| `Tools/ValtanPipeline/promote_valtan_animation_chains.py:3320` `commit_typed_authoring_patch` | writer lock, baseline/CAS, journal, atomic commit을 소유한다. `:3480` 전체 `validate_and_project`가 V2-only Save에도 실행된다. |
| 같은 파일 `:1796` `validate_and_project`, `:2997` `_atomic_commit_locked` | 전자는 Product 준비로 이동할 검사 묶음이고, 후자는 Source Save에서도 유지할 저장 안전장치다. |
| `Tools/ValtanPipeline/valtan_tuning_pipeline.py:7591` `source_manifest` | 14개 Source·관련 문서를 하나의 revision으로 묶어 strict join한다. 이 전체 revision은 editor의 열기·일반 Save 허용 조건에서 분리한다. |
| `Client/Private/ValtanActionWorkbench.cpp:1801` `Reload_Canonical`, `:5107` `Reload_AfterPendingSave`, `:5178` `Update_SaveState` | 전체 Product reopen 실패가 저작 화면과 Save 성공 표시까지 결합되는 부분을 바꾼다. |

`Save_Reload`는 이미 dirty owner별 local 검사와 job receipt를 사용한다. `Animation_Tool.cpp:2021`의 `Validate_ValtanCompositionAnimationGraphMutations`도 변경되지 않은 animation signature는 건너뛴다. 이 개선을 유지하고 writer와 reopen에 남은 전체 결합을 해소한다.

### 상태와 호출 흐름

현재 흐름은 `Save_Reload → Begin_ValtanCompositionSave → Save job → commit_typed_authoring_patch → 전체 projection 및 Sound/V2 join → atomic commit → SourceManifest strict join → Workbench/Product reopen`이다. `Publish after Save`를 끄더라도 projection과 strict join은 앞 단계에 남는다.

현재 `VALTAN_BIND_SLOT`의 동일 `boss.valtan.shout` 세 binding은 서로 다른 `bindingId`와 `CLIP_OCCURRENCE`의 `clip.02/03/04`를 가진다. 예상 stage 위치는 각각 **1400 / 2300 / 3200 ms**다. 그러나 `BuildEffectV2BindingStableId`(`ValtanActionWorkbench.cpp:262`)는 typed `bindingId` 대신 resource/start/anchor 조합을 만들고, `Build_Timeline`(`:4257`)은 convenience `strStage`가 있으면 clock basis를 보지 않고 stage-local 0 ms로 투영한다. 세 상자가 0 ms와 같은 선택 ID로 충돌할 수 있다. 저장 전체 검증과 별도로 고쳐야 하는 저작 모델 결함이다.

### G00 종료 검증

- 현재 Source의 읽기 전용 validate 결과와 V2 세 occurrence 값을 구현 RESULT에 baseline으로 보존한다. 과거 41개 패턴 집계나 2026-09-04의 `missing V2 read-set` 실패를 현재 실패로 재사용하지 않는다.
- Save의 source commit, local reopen, publish, Server 적용 상태를 분리한 뒤 테스트할 실패 지점을 현재 job receipt에 대응시킨다.
- 이 G는 조사·기준 고정이다. Client 실행·화면 PASS는 부여하지 않는다.

## G01. Source 저작 세션과 immutable Product 소비 분리

### 목표와 종료 증거

저작 원본이 미완성 또는 일부 오류 상태여도 정상 패턴을 열고 수정할 수 있게 한다. 동시에 Source Save가 진행 중인 Server Play의 V2 표현을 바꾸지 못하게 한다. **G02의 source-only Save를 사용자에게 활성화하기 전에 이 runtime 분리를 먼저 닫는다.**

### 수정 파일과 H 계약

| 기존 파일 | 변경할 상태와 역할 |
|---|---|
| `Client/Public/BalanceTool.h`, `Client/Private/BalanceTool.cpp` | `PATTERN_EDIT`, `PATTERN_STAGE_EDIT`, `VALTAN_SOURCE_JOIN_STATUS`에서 원본 존재·파싱 상태와 Product 준비 상태를 분리한다. Source session은 baseline bytes/revision, editable row, row별 오류, dirty owner를 소유한다. |
| `Client/Public/ValtanPatternTree.h`, `Client/Private/ValtanPatternTree.cpp` | 기존 strict `Load_FromAuthoringPaths`/`Load_WhileAdmitted`를 Product 경계에 남긴다. 추가 `Read_AuthoringInventory`는 문서의 row 목록과 오류를 보존하고, 추가 `Build_AuthoringPatternView`는 선택한 정상 row의 편집 view만 만든다. |
| `Client/Public/ValtanActionWorkbench.h`, `Client/Private/ValtanActionWorkbench.cpp` | source inventory/selection과 마지막 admitted Product view를 별도 멤버로 소유한다. Source error를 Product admission enum 하나로 덮지 않는다. |
| `Client/Public/EffectV2_Document.h`, `Client/Private/EffectV2_Document.cpp`, `Client/Public/EffectV2_Catalog.h`, `Client/Private/EffectV2_Catalog.cpp` | strict runtime parser는 유지한다. 추가 `Parse_BindingsForAuthoring`는 유효 JSON의 각 binding row와 오류를 보존한다. authoring snapshot과 admitted runtime snapshot을 목적이 드러나는 호출로 구분한다. |
| `Client/Public/EffectV2_Runtime.h`, `Client/Private/EffectV2_Runtime.cpp` | 기존 evaluator와 local-preview snapshot overload를 유지한다. live BOSS_VALTAN binding cache가 authoring catalog revision을 따라가던 `Ensure_Bindings`를 admitted Product snapshot 소비로 바꾼다. |
| `Client/Public/ValtanPresentationGenerationAdmission.h`, `Client/Private/ValtanPresentationGenerationAdmission.cpp` | 이미 있는 generation artifact receipt를 통해 Product V2 binding·leaf/group snapshot을 획득한다. Server gameplay revision과 exact network PREPARE 검사는 유지한다. |

Source row에는 stable ID, parsed value, 오류 이유, 보존용 JSON, 수정 여부를 둔다. 문서 전체 syntax 오류와 개별 row의 의미 오류를 구분한다. duplicate ID는 해당 row들을 오류 항목으로 보존하고 정상 row처럼 lookup하지 않는다. 표시용 임시 key는 저장 ID로 쓰지 않는다. `CBalanceTool`의 현재 runtime형 필드에 값 0이나 빈 action을 채워 “완성된 pattern”을 만들지 않는다.

`DATA_JSON_VALUE`는 원본 byte span을 제공하지 않는다. 따라서 UI의 보존 JSON은 오류 표시와 세션 유지에 사용하고, 디스크에서 수정하지 않은 row의 byte 보존은 G02의 Python writer가 baseline/current 원문으로 수행한다. `KoukuSaydonCompositionDocument.cpp`의 `Parse_Text`/`strLoadError`/`strPreservedJson` 방식은 오류 항목 격리 원리로 재사용하며 byte-exact 보존을 이미 보장하는 것으로 오인하지 않는다.

### 함수와 실제 소비 흐름

`Reload_Canonical`을 Source 로드와 Product 로드의 두 결과를 받는 흐름으로 바꾼다. Source 로드 성공이면 inventory와 편집 가능한 row를 stage하고 한 번에 교체한다. Product 로드 실패면 마지막 admitted Product view와 이유를 유지한다. 이 실패가 Source inventory를 비우거나 정상 row의 Save를 막지 않는다. Source 문서 전체가 파싱 불가능하면 해당 owner의 기존 세션과 bytes를 유지하고 그 owner만 쓰기 금지한다.

`Tools/GameplayPipeline/valtan_presentation_generation.py`의 `build_presentation_generation`과 `Tools/ValtanPipeline/valtan_tuning_pipeline.py`의 `_stage_presentation_generation_closure`는 이미 V2 bindings 및 그 leaf/group 문서를 generation artifact로 묶는다. live runtime은 이 **기존 generation의 bytes**에서 만들어진 immutable snapshot을 pin한다. `EffectV2_Runtime.cpp:171`의 `Ensure_Bindings`가 `Get_BossValtanSnapshot()`의 최신 authoring 상태를 자동 반영하지 않게 한다. local preview는 현재 draft로 만든 명시적 snapshot을 기존 `Notify_Stage`/local preview overload에 전달한다. snapshot 선택만 분리하고 renderer·clock evaluator를 복제하지 않는다.

Pattern Sound는 기존 presentation generation의 M lane에 강제로 넣지 않는다. 현재 독립 S receipt와 playback admission을 유지한다. 이미 저장된 Product와 새 source의 sound/effect를 암묵적으로 섞지 않는다.

### G01 종료 검증

- valid JSON 안의 잘못된 pattern/binding 한 row가 있어도 다른 정상 row는 inventory에 남고 선택·편집된다. 오류 row는 이유와 원문을 보존한다.
- source 전체 parse 실패 때 이전 session/preview가 유지된다. 해당 owner가 실패해도 다른 문서를 조용히 지우지 않는다.
- Product generation A 재생 중 authoring V2를 B로 stage/Save해도 A의 binding/leaf bytes가 바뀌지 않는다. local preview에만 명시적으로 B를 넘기면 B를 소비한다.
- 수정한 `BalanceTool`, `ValtanPatternTree`, `ValtanActionWorkbench`, V2와 admission H/CPP만 최소 컴파일한다. 기존 V2 binding/Valtan audition harness에 snapshot 고정·실패 보존 사례를 추가하여 해당 사례만 실행한다.

## G02. Source-only Save와 저장 완료 상태 닫기

### 목표와 종료 증거

패턴, V1 cue, V2 binding, row layout, Sound의 dirty source를 안전하게 저장한다. Save의 성공은 디스크 Source commit과 Source reopen으로 판정하고, local Preview 준비와 Product Publish 결과는 별도로 표시한다.

### 수정 파일과 H 계약

`ValtanActionWorkbench.h`의 post-save 상태와 `BalanceTool.h`의 job result에 `sourceCommitted`, `sourceReopened`, `previewReady`, `publishAttempted`, `publishSucceeded`의 의미를 분리한다. 기존 job/receipt를 확장하며 새 영속 manifest를 만들지 않는다. source commit 후 Preview가 준비되지 않아도 dirty 표시를 정확히 해제하고 `저장됨 / 미리보기 준비 안 됨: 해당 이유`를 표현한다. 실패한 save를 성공으로 표시하지 않는다.

`Tools/ValtanPipeline/promote_valtan_animation_chains.py`에 **추가 `commit_source_authoring_patch`**를 둔다. 기존 `commit_typed_authoring_patch`의 writer lock, read-set staging, `_atomic_commit`과 journal 코드를 재사용하고 아래 Source 저장 경계만 분리한다. `valtan_tuning_pipeline.py`의 기존 `commit-canonical` dispatch와 Save job에는 명시적 source 저장 mode를 연결한다. 옵션 이름만 `CommitOnly`로 바꾸고 내부 projection을 남기는 구현은 종료로 인정하지 않는다.

### 저장 schema와 parse 경계

| Source 문서 | 저장 가능 조건 | Save에서 요구하지 않는 조건 |
|---|---|---|
| `Data/Valtan/Valtan.gameplay.json`, `Valtan.presentation.json` | 유효 JSON, 지원하는 source version, 변경 row의 stable ID/field type/수치 범위, paired pattern/stage identity, source syntax 보존 | 전체 42개 pattern의 runtime 완결성, 모든 animation native window, 모든 dependency의 Product join |
| `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json` | binding row의 typed 구조, ID 유일성, path 안전성, 변경 필드의 단위/범위 | 선택하지 않은 resource의 물리 준비, 모든 binding과 현재 Product stage의 exact join |
| 기존 Pattern Sound owner 문서 | 기존 row ID와 source 구조 및 변경 수치 범위 | 전체 발탄 Product와의 join 및 unrelated Sound/Effect 완성 |

실제 V2 owner 경로는 현재 catalog가 resolve하는 `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`을 사용한다. 입력 파일 경로를 UI나 새 workbench 상수에서 추측하지 않는다.

미완성 Source를 저장하려면 strict runtime parser를 느슨하게 만드는 것만으로 끝낼 수 없다. split source formatVersion 2를 도입하고 gameplay pattern의 `authoringStatus`를 `DRAFT | PRODUCT`로 저장한다. `DRAFT`는 최소 `patternId`, `displayName`, `authoringStatus`, `stages`를 가진다. runtime에 필요한 entry/action/eligibility/hit 등이 아직 없으면 **field 부재**로 보존한다. 무피해 action이나 가짜 정상 값으로 채우지 않는다. `PRODUCT` 표시는 저자의 publish 의도를 나타내며 준비 완료는 publisher가 계산한다. 기존 formatVersion 1 row는 메모리에서 기존 Product 의도를 유지하고, 명시적 첫 source Save 때 version 2로 transactionally 이행한다. 열기만으로 파일을 바꾸지 않는다.

presentation의 paired row는 같은 `patternId`, `stages`를 가진다. 비어 있는 stage/animation/effect 목록은 Draft 저장에서 허용한다. source projection 전에 runtime이 요구하는 완전한 구조를 별도로 만든다. 기존 version 1 strict Product loader와 generated Product format은 그대로 유지하고, publisher의 source adapter가 version 2의 준비된 부분만 현행 strict projection에 전달한다.

unknown field는 원본 row에서 보존한다. 수정한 row도 원본 object에 typed field patch를 적용해 만든 뒤 직렬화하며, 편집 view가 아는 field만으로 row 전체를 다시 만들지 않는다. 지원하지 않는 field를 정상 동작하는 편집 필드로 보여 주지는 않는다. 누락 resource나 미해결 reference는 row별 준비 안 됨 사유다. ID 또는 version을 판별할 수 없는 row는 수정하지 않고 보존하며, 명시적 복구·삭제 명령만 허용한다. 새 ID는 정상 항목과 격리된 오류 항목 모두에 충돌하지 않아야 한다. 이미 존재하던 duplicate ID 오류를 정상화하지 않은 채 보존하는 것은 허용하지만, mutation으로 duplicate를 추가하는 것은 거부한다.

### writer 호출 흐름과 CAS

`Save_Reload → Begin_ValtanCompositionSave → Save job source mode → commit_source_authoring_patch → owner baseline 비교 → 변경 row storage 검사 → source bytes stage → _atomic_commit → source reopen receipt → Accept_PendingSaveOwners`로 연결한다.

- CAS 단위는 **이번 Save에서 쓰는 기존 owner 파일**이다. 현재 전체 14개 manifest hash를 editor 저장 허용 조건으로 사용하지 않는다. 같은 owner 파일에 외부 변경이 있으면 그 파일 이름과 충돌 이유를 표시하고 기존 dirty draft를 보존한다. 다른 owner 문서의 변경은 해당 Save를 막지 않는다. 자동 3-way merge framework는 이번 범위에 추가하지 않는다.
- Pattern 변경은 gameplay/presentation paired source를 같은 transaction에 넣는다. V2-only Save는 V2 owner만, Sound-only Save는 Sound owner만 쓴다. row metadata는 presentation source에 함께 저장한다. 여러 owner를 한 번에 저장할 때는 기존 다중 파일 atomic commit과 rollback을 유지한다.
- 기존 `_array_span`, `replace_or_append_rows`, `replace_append_or_remove_rows`, `_assert_unmanaged_raw_rows_preserved`를 재사용해 수정하지 않은 오류 row와 unknown field의 원문을 보존한다. paired source 전체를 runtime view로 재직렬화해 유효하지 않은 row를 탈락시키지 않는다.
- Source 저장은 `validate_and_project`, 전체 native audit, provenance receipt 재생성, global Pattern Sound/V2 Product join, strict `source_manifest`를 호출하지 않는다. 기존 effect resource read-set은 resource 내용 교체를 실제로 소비하는 준비·Publish 경계에서 유지하며, source reference 문자열 저장 때문에 전체 Resources 상태를 요구하지 않는다.
- `Data/Encounters/Valtan/*`, generated animation/V1 cue, rootmotion, balance provenance receipt, `Valtan.bosscomposition.json`은 Source Save target에서 빠진다. 마지막 파일은 현재 SHADOW descriptor이므로 별도 정본으로 승격하지 않고 Publish 때 갱신한다.
- commit 후 Source reopen 실패는 디스크 저장 여부를 receipt 그대로 표시하고 draft를 복구 가능하게 남긴다. 반복 Save로 이미 commit한 source를 덮어써 문제를 숨기지 않는다.

### G02 종료 검증

- 미완성 Draft, 누락 resource를 참조하는 typed row, 정상 row 옆의 오류 row가 저장 후 reopen에서 동일한 상태로 남는다. 수정하지 않은 오류 row의 bytes가 보존된다.
- V2-only/Sound-only/row-only Save의 실제 write set이 해당 source owner로 제한된다. generated Product, descriptor, receipt와 Server revision의 bytes가 변하지 않는다.
- 같은 owner 외부 수정은 CAS 실패하고 disk/current draft가 보존된다. unrelated owner 수정은 Save를 막지 않는다. multi-owner 두 번째 rename 실패는 전체 rollback된다.
- 기존 `test_action_composition_atomic_save_contract.py`, `test_action_composition_dirty_owner_save_contract.py`, `test_valtan_canonical_typed_patch_transaction.py`에 이 경계의 사례를 보강하여 해당 모듈을 실행한다. 단순 소스 문자열 기대값만 바꿔 통과시키지 않는다.
- C++ job/receipt 소비 변경을 최소 컴파일하고 `git diff --check`를 실행한다. 일반 Save 실행에 전체 publisher나 70개 suite를 선행하지 않는다.

## G03. bindingId와 typed clock으로 모든 V2 box 연결

### 목표와 종료 증거

Append한 V2 box의 선택, 이동, 세부 수정, Duplicate/Delete, Save/Reopen, local preview와 Product 재생이 같은 `bindingId`와 clock을 사용하게 한다. `VALTAN_BIND_SLOT` 세 shout box의 정확한 위치와 독립 편집이 이 G의 회귀 기준이다.

### 수정 파일과 H 계약

`EffectV2_Document.h`의 `EFFECT_V2_BINDING::strBindingId`, `eClockBasis`, `strClipOccurrenceId`, `iStartMs`, `eRepeatPolicy`, `strAnchorSlotId`, `eFollowPolicy`, `eRotationBasis`, `LocalTransform`, `eStopPolicy`가 저작 계약이다. convenience `strStage/strClip/strBone` 등은 호환 소비용이며 저장 key나 clock 판정에 쓰지 않는다.

`ValtanActionWorkbench.h`의 `TIMELINE_ITEM`에 V2 원본 `bindingId`와 표시 occurrence identity를 구분한다. EACH_LOOP 표시 box가 여러 개여도 underlying owner는 한 binding이다. 선택 state와 ImGui ID에는 `bindingId + displayed occurrence`를 쓰고, mutation은 typed `bindingId` 하나로 보낸다. 복제만 새 bindingId를 발급한다.

`EffectV2_Catalog.h`의 기존 `EFFECT_V2_STAGE_BINDING_KEY`와 `Stage_AppendBossValtanStageBinding`, `Stage_RemoveBossValtanStageBinding`, `Stage_DuplicateBossValtanStageBinding`, `Stage_UpdateBossValtanStageBindingStart`를 확장한다. **추가 `Stage_ReplaceBossValtanBinding`**은 old bindingId를 대상으로 완전한 typed replacement를 검증하고 한 번에 stage한다. Box Detail에서 anchor/transform/clock/stop을 바꾸는 모든 입력이 이 경계를 호출한다. `UPDATE_START`만 구현한 상태에서 나머지 UI field를 저장 가능하게 보이지 않는다.

### clock 투영과 편집 흐름

`BuildEffectV2BindingStableId`의 합성 key 소비를 제거하고, `Resolve`/`Build_Timeline`/selection/delete/duplicate/detail/preview가 typed binding ID로 찾게 한다.

- STAGE: `stage offset + clock.startMs`가 pattern상의 시작이다.
- CLIP_OCCURRENCE: pattern/stage/action과 정확한 occurrence ID를 resolve하고, 기존 `Resolve_ClipSourceToStageMs`의 source window·playRate 변환으로 `clock.startMs`를 stage 위치에 투영한다. sourceStart 이전·window 바깥·missing occurrence를 0으로 clamp해 정상 box로 만들지 않는다. 오류 표시와 원래 typed 값을 유지한다.
- drag 입력은 화면의 stage ms를 현재 basis의 ms로 역변환한 candidate를 stage한다. basis 변경은 기존 화면상 시작을 유지할 수 있는 정확한 변환에 성공할 때만 commit한다. occurrence 재배치·playRate 변경 후 binding의 occurrence ID와 원본 clock 값은 유지되고 화면 위치만 다시 계산한다.
- STAGE는 현재 계약대로 occurrence null/ONCE만 허용한다. EACH_LOOP는 명시된 occurrence의 반복을 evaluator와 같은 식으로 표시한다. occurrence 삭제는 참조하는 V1/V2/Sound 항목을 보여 주고 기존 typed cascade 명령으로 함께 제거하거나 삭제를 거부한다. 무관한 occurrence로 자동 재연결하지 않는다.

현재 `Resolve_ClipSourceToStageMs`는 missing occurrence에서 0을 반환하고 sourceStart 이전을 clamp한다. 이 함수를 성공 여부와 오류를 반환하는 변환으로 바꾸고 V1/V2 호출자가 실패를 소비하게 한다. runtime의 `Resolve_StageSpawnClock`(`EffectV2_Runtime.cpp:799` 부근)이 사용하는 식은 `occurrence wall start + loop epoch × loop wall duration + (binding source start − occurrence source start) / playRate`다. Python `effect_v2_binding_pipeline.py:1318` 부근의 raw startMs→playMs/stageDuration 직접 비교도 바꾼다. source window `[sourceStart, sourceStart + sourceDuration)`와 투영된 wall time을 각각 검사하여 sourceStart가 0이 아닌 clip과 playRate를 UI·publisher·runtime이 동일하게 취급하게 한다.

끝점은 실제 런타임 의미를 가져야 한다. 현재 V2 `EXPLICIT`은 finite duration field가 없으므로 임의 resize를 그대로 활성화하지 않는다. 이 G에서 **BOSS_VALTAN binding formatVersion 3**를 추가하고 `clock.durationMs`를 nullable 양의 정수로 정의한다. `EXPLICIT`이면 양수 필수이며 같은 clock basis에서 `startMs + durationMs`가 끝이다. 다른 stop policy이면 null이다. version 1/2 읽기는 유지하고 version 2의 기존 EXPLICIT 의미를 임의로 유한 길이로 바꾸지 않는다. version 2 EXPLICIT row는 migration 시 명시 길이가 없다는 사유를 표시하며 저자가 NATURAL/경계 종료 또는 유한 길이를 선택한다. 현재 102개 source binding의 stopPolicy는 NATURAL 100개, STAGE_END 2개이고 EXPLICIT은 0개다. 현재 데이터는 기존 종료 의미를 유지한 채 version 3로 이행할 수 있다.

`EffectV2_Document.cpp`의 Parse/Serialize/Equals, `EffectV2_Catalog.cpp` staging, `EffectV2_Runtime.cpp` pending/group lifetime, Python V2 binding parser/publisher를 같은 G에서 갱신한다. group child의 종료는 자신의 유한 끝과 binding 끝 중 빠른 쪽이며 자연 수명·stage/occurrence 종료·Stop/Seek에서도 같은 규칙을 쓴다. V1 resize는 기존 `cue_end`/source duration 계약을 사용한다. 시각적 box 폭만 바뀌고 runtime은 자연 종료하는 상태를 허용하지 않는다.

### G03 종료 검증

- `VALTAN_BIND_SLOT` shout 3개의 box가 1400/2300/3200 ms이고 bindingId가 모두 다르다. 가운데 box만 이동/수정/삭제해도 다른 둘은 바뀌지 않는다. Save/Reopen에서 ID와 typed clock이 유지된다.
- STAGE, CLIP_OCCURRENCE, sourceStart가 0이 아닌 clip, playRate 변경, EACH_LOOP, missing occurrence, 중복 ID를 각각 검증한다.
- `durationMs`의 Parse→Serialize→Parse, local seek/loop/stop, group 종료와 Product snapshot 소비가 일치한다. 지원 version 외 값·0 duration·overflow는 오류를 보존하며 거부한다.
- 기존 `test_action_composition_effect_v2_clip_projection_contract.py`, `test_action_composition_sequence_identity_contract.py`, `Tools/EffectToolV2/test_effect_v2_binding_pipeline.py` 및 해당 V2 runtime harness 사례를 실행한다. Python 모듈의 실제 파일 경로는 기존 test module을 사용하며 새로운 광역 harness를 만들지 않는다.

## G04. Composition Resources append와 Box Detail의 typed owner 연결

### 목표와 종료 증거

쿠크의 resource 선택→candidate→commit 흐름을 발탄의 기존 owner에 연결한다. 리소스 목록은 준비되지 않은 항목까지 사유와 함께 유지하고, 사용할 수 있는 리소스는 선택한 row/stage에 append할 수 있게 한다.

### 수정 파일과 H 계약

`ValtanActionWorkbench.cpp`의 `Render_ResourcesPane` 영역(`:11900` 부근), `Can_AppendCompositionAnimationResource`, `Append_CompositionAnimationResource`, `Apply_CompositionResourceAppend`, detail 렌더링과 `PENDING_RESOURCE_APPEND`를 확장한다. `ICompositionWorkbenchSession`과 공통 shell의 frame 종료 후 deferred command 처리를 재사용한다.

resource identity는 `ANIMATION_CLIP`, `ANIMATION_SEQUENCE`, `V1_EFFECT`, `V2_EFFECT`, `V2_GROUP`, 기존 Sound/Camera 등 **현재 owner별 typed 종류와 stable resource ID**다. label, filename prefix, pointer, vector index로 종류를 역추론하지 않는다. `IsBossValtanEffectV2Resource`의 `boss.valtan.` prefix만으로 append 지원 여부를 판단하지 않고 catalog의 typed resource 및 선택 대상의 capability로 결정한다. 목록 탐색 때문에 모든 leaf resource를 미리 로드하지 않으며 `Read_Inventory → 선택 resource Load_ResourceSnapshot`을 사용한다.

| 입력 종류 | 실제 append owner와 저장 | Box Detail와 runtime 소비 |
|---|---|---|
| Animation clip/sequence | `CBalanceTool`의 stage/animation slot draft. 새 occurrence ID를 발급하고 순서를 저장한다. | source window, playRate, repeat 및 stage time을 기존 animation graph/local preview에 반영한다. |
| V1 effect | 기존 `Add_ValtanStageEffectCue`로 cue ID와 exact occurrence를 가진 `Valtan.presentation.json` product cue draft를 만든다. | cue timing/transform/follow/stop은 기존 cue 편집 경계에 연결한다. Publish가 patterneffectcues를 만들고 기존 V1 playback이 소비한다. leaf 자체 수정은 Effect Tool owner가 저장한다. |
| V2 effect/group | 기존 V2 Catalog의 typed binding append/replacement. 선택 row의 stage/action/clock을 명시한다. | G03 typed clock/anchor/detail과 같은 immutable V2 snapshot을 local preview·Product runtime이 각각 소비한다. leaf/group 문서 자체 수정은 Effect Tool V2 owner가 저장한다. |
| Sound/Camera | 기존 sound/camera owner의 typed 항목 append 명령을 호출한다. | 각 기존 소비자의 단위와 receipt를 유지한다. Effect용 timing 구조로 강제 변환하지 않는다. |

V1은 현재 source occurrence 기반 계약을 사용하므로 선택 row에 유효 occurrence가 없으면 정확한 사유를 표시한다. 새로운 임의 stage clock을 V1에 조용히 추가하지 않는다. V2 STAGE binding은 animation occurrence 없이 만들 수 있다. append가 지원되지 않는 대상 조합은 행 선택만으로 성공한 것처럼 보이지 않고, 필요한 stage/occurrence를 선택하도록 상태를 표시한다.

### 함수와 commit 흐름

`Resources 선택 → 선택 resource의 저장된 snapshot 확보 → 대상 pattern/row/stage 및 typed capability 확인 → candidate 작성 → owner stage → frame 종료 시 draft commit → Build_Timeline → 선택한 새 box 표시 → 기존 local preview snapshot 갱신` 순서다. 실패하면 기존 document와 preview 및 selection을 유지한다.

Box Detail의 Apply는 session input buffer를 owner mutation으로 바꾸는 순간이다. 작업 중 수치 입력을 원본에 매 프레임 쓰지 않는다. V1/V2 Deep Link는 정확한 저장 resource ID와 source binding/cue ID를 Effect Tool에 전달한다. 현재 Product에 없다는 이유만으로 Draft의 resource 편집 진입을 차단하지 않는다. Workbench Save는 invocation과 배치만 저장하며 leaf 내부 변경을 몰래 함께 저장하지 않는다.

### G04 종료 검증

- Animation clip/sequence, V1 leaf, V2 leaf/group의 append→detail Apply→Duplicate/Delete→Save/Reopen 경로를 각각 확인한다. append 실패 때 문서/선택/preview가 유지된다.
- 저장되지 않은 leaf edit와 저장된 resource snapshot의 차이를 구분한다. source leaf 저장 후 명시적 refresh가 성공하면 새 preview snapshot을 만들며 실패 시 이전 것을 유지한다.
- 기존 `test_action_composition_effect_invocation_contract.py`, `test_action_composition_resource_categories.py`와 수정한 owner의 focused round-trip 검증을 실행한다.
- 사용자 확인 경로는 `F1 → Action Composition Workbench → Valtan → Composition Resources → 대상 row 선택 → Append → Box Detail → Apply → local Play → Save → Reopen`이다. 에이전트는 Client를 대신 실행·조작하지 않는다.

## G05. 지속되는 row와 빈 Draft Pattern 만들기

### 목표와 종료 증거

Composition Patterns에서 원본 animation intake의 Product 승격 transaction을 거치지 않고 빈 Draft Pattern을 만들 수 있게 한다. 사용자가 추가한 빈 row와 box의 row 배치가 Save/Reopen 후 유지된다.

### 수정 파일과 H 계약

`ValtanActionWorkbench.h/.cpp`는 row 선택·Create input buffer·deferred command를 소유하고, `BalanceTool.h/.cpp`는 source draft mutation을 소유한다. 기존 `TIMELINE_ITEM::iSubrow`는 충돌 없는 표시용 packing 값이다. 이를 저장 ID로 사용하지 않고 presentation pattern에 아래 metadata를 추가한다.

| 추가 source field | 책임과 유효 조건 |
|---|---|
| gameplay root `nextPatternOrdinal` | 문서의 단조 증가 pattern ID 발급 counter. 새 ID는 `VALTAN_AUTHORED_000001` 형식으로 전체 source와 retired ID에 충돌하지 않는 다음 값을 원자적으로 사용한다. 삭제된 ID를 재사용하지 않는다. |
| presentation pattern `nextRowOrdinal` | 해당 pattern의 row ID 발급 counter. 삭제나 정렬로 감소하지 않는다. |
| presentation pattern `rows[]` | `rowId`, `lane`, `displayName`, `order`를 가진 명시적 저작 row. 빈 row도 보존한다. row ID는 pattern 범위에서 안정적이다. |
| presentation pattern `rowAssignments[]` | `rowId`, `ownerKind`, `ownerId`로 기존 stage/occurrence/cue/binding/sound/camera 항목의 row를 지정한다. gameplay clock과 resource 내용은 소유하지 않는다. |

metadata는 별도 JSON, `.bosscomposition` descriptor, leaf 문서에 복제하지 않는다. 기존 formatVersion 1은 메모리에서 deterministic 기본 row를 만들고 첫 명시 Save에만 metadata를 기록한다. 원래 row가 없는 기존 파일을 여는 동작은 source dirty를 만들지 않는다. 관리하지 않는 오류 항목의 row assignment도 함께 보존한다.

새 패턴의 최소 paired source는 G02의 Draft schema를 사용한다. **추가 `Create_ValtanDraftPattern`**, `Add_ValtanCompositionRow`, `Move_ValtanCompositionItemToRow`, `Remove_ValtanCompositionRow`는 `CBalanceTool`의 source candidate를 검증·교체하는 mutation이다. 필수 입력은 displayName이며 stable ID는 owner가 발급한다. 생성 직후 stages가 비어 있어도 Save가 된다.

기존 `CAnimation_Tool::Stage_ValtanCompositionIntakeSequence`와 Python `create_pattern_from_request`/`prepare_create_pattern_transaction`은 source reference intake·승격 용도로 남긴다. Composition의 `New Draft Pattern`은 이 전체 promotion pipeline을 호출하지 않는다. Draft에 sequence를 append할 때는 G04의 기존 animation append를 사용하고 출처 reference는 알려진 값만 기록한다.

### row와 stage의 호출 흐름

`New Draft Pattern → paired source candidate → local commit → pattern 선택 → Add Stage/Append Sequence → Add Row → Append/Move box → Source Save`다. 빈 Pattern의 local Play는 “재생 가능한 stage 없음”으로 실패하지만 저장은 된다.

row는 배치 layout이며 stage 순서·duration·action branch를 바꾸지 않는다. 같은 lane의 다른 row로 이동은 metadata만 바꾼다. 다른 stage로 transfer할 때는 기존 animation/effect/sound dependency cascade와 typed clock 변환이 성공해야 한다. row 삭제는 비어 있을 때 즉시 candidate에서 제거하고, 내용이 있으면 항목 이동 또는 명시적 함께 삭제 명령을 실행한다. 숨겨진 항목을 유실시키지 않는다.

새 stage는 owner가 `stageId`와 `actionId`를 안정적으로 발급하고 명시적으로 입력한 양의 duration을 저장한다. animation/hit가 없어도 Draft로 유지한다. category/eligibility/damage/entry 및 graph edge가 준비되지 않았다고 다른 정상 패턴이나 전체 Save를 막지 않는다. 새 pattern을 normal rotation 또는 scripted sequence에 자동 가입시키지 않는다.

### G05 종료 검증

- 빈 Draft 생성→Save→Reopen에서 동일 pattern ID와 빈 stages를 확인한다. 이후 sequence/V1/V2 append와 재저장이 이어진다.
- 빈 row 추가, 이름 변경, 순서 변경, item 이동이 round-trip에서 유지된다. row-only Save는 animation/effect/gameplay timing을 변경하지 않는다.
- duplicate/delete/undo 성격의 세션 되돌리기에서 ID를 재활용하지 않는다. dangling assignment는 오류 사유를 남기고 다른 row를 지우지 않는다.
- 기존 `test_action_composition_manual_stage_topology_contract.py`, sequence identity와 atomic save 테스트에 신규 Draft/row 사례를 추가한다. Product-ready fixture만 만들어 빈 Draft가 저장되는 핵심 경계를 우회하지 않는다.

## G06. local Play와 Product Publish의 준비 검사 연결

### 목표와 종료 증거

저장한 Source의 준비된 부분은 local preview로 즉시 확인하고, 발탄 전투에 적용할 때만 기존 strict Product·Server revision 검사를 통과하게 한다. publish 실패는 기존 해당 Product 단위를 통째로 보존한다.

### 수정 파일과 현재 함수

`ValtanActionWorkbench.cpp`의 `Play_EffectivePreview`, `Seek_EffectivePreview`, `Refresh_PatternLocalPreviewAfterMutation`, V2 preview refresh와 `Animation_Tool.cpp`의 `Play_ValtanCompositionDraftPattern`/`Seek_ValtanCompositionPattern`는 선택한 source draft로 immutable preview를 만드는 경계다. build 실패 시 마지막 성공 preview를 유지하며 “현재 draft가 재생됨”으로 오인되지 않도록 source/preview revision과 실패 이유를 표시한다.

`Tools/ValtanPipeline/valtan_tuning_pipeline.py`, `promote_valtan_animation_chains.py`, `Tools/GameplayPipeline/valtan_presentation_generation.py`가 Publish 준비와 기존 candidate/receipt를 소유한다. `Tools/KoukuSaydonPipeline/project_kouku_saydon_composition.py:1551`의 `prepare_publication`이 이미 사용하는 inventory, dependency closure, per-pattern unavailable reason, 최종 union 검증 원리를 가져온다. 쿠크와 발탄의 source parser를 억지로 합치거나 공용 publisher framework를 새로 만들지 않는다.

### dependency closure와 Product 단위

발탄의 closure에는 선택 pattern의 stage/action graph, referenced pattern, follow-up/branch, scripted sequence, normal selection entry, world event set, combat object, animation/rootmotion, V1/V2 resource, Sound/Camera의 실제 참조를 포함한다. 기존 parser가 판별하는 edge를 사용하고 문자열 prefix나 “현재 화면에 보이는 box”만으로 closure를 만들지 않는다.

**초기 구현의 발탄 raid Publish 단위는 기존 encounter 전체다.** 현재 scripted sequence와 cross-pattern 참조가 엮인 기존 Product를 pattern별 old/new overlay로 섞어 쓰지 않는다. 선택한 준비된 새 독립 Draft를 local preview로 확인할 수 있어도 Server raid에 추가할 때는 저자가 연결한 encounter closure 전체를 다시 검증하고 기존 candidate transaction으로 배포한다. Product에 연결되지 않은 새 Draft는 raid Publish를 막지 않는다. 연결된 항목이 준비되지 않으면 candidate 준비를 실패시키고 마지막 encounter Product, generation receipt, Server gameplay revision을 그대로 보존한다.

Source adapter는 `DRAFT | PRODUCT` 의도와 계산된 준비 상태를 구분한다. Product 의도가 있어도 필수 field/reference가 빠지면 unavailable 사유를 내며, reference target이 DRAFT인 경우에도 closure 내에서 필요한 준비 조건을 모두 검사한다. “READY”를 수동 flag로 저장해 strict 검사를 우회하지 않는다. 부분 성공 publication, 별도 manifest/admission registry, Resource hash pack을 이번 작업의 완료 조건으로 추가하지 않는다.

### 실제 호출 흐름과 Server 권위

`명시적 Publish → 최신 Source snapshot → 기존 edge 기반 encounter closure → strict validate_and_project 및 native/provenance/Sound/V2 dependency 검사 → 기존 candidate staging → generation artifacts/receipt 준비 → 전체 target CAS/atomic publish → 기존 Server 적용 절차`다.

Server gameplay revision과 `CValtanPresentationGenerationReadAdmission::Acquire_ExactReceipt`의 network PREPARE 일치는 유지한다. 제품 transform/action/phase/damage는 계속 Server가 결정한다. source 저장·local Play는 Client `CValtan` local AI를 제품 전투로 승격하지 않는다. protocol에 새 임의 local play action을 추가하지 않는다. 기존 command sink와 server audition 경로가 지원하는 pattern만 Server Play에 노출한다.

기존 Publish 결과가 재시작을 요구하면 `Published / Server restart required`를 유지하고 실제 Server 재시작 전 적용 완료를 표시하지 않는다. Source와 Product의 전체 exact join은 editor gate에서 제거하지만, Server가 승인한 gameplay와 Client presentation generation의 일치는 그대로 검사한다. “Source 저장됨”, “Preview 준비됨”, “Product 배포됨”, “Server 적용됨”을 하나의 성공 표시로 합치지 않는다.

### G06 종료 검증

- 선택 pattern의 누락 resource는 그 preview와 연결된 Product closure에 이유를 표시한다. unrelated Draft는 정상 패턴의 편집·Save·local preview를 막지 않는다.
- scripted sequence 중간 dependency가 실패하면 이전 encounter/generated documents/generation receipt가 모두 그대로 유지된다. old Product와 new Source 조합을 새 revision 전체 적용으로 보고하지 않는다.
- 준비된 current source는 기존 publisher validate/candidate 경로를 통과하고 V1 cue/V2 binding·finite duration/animation·Sound·Camera를 실제 consumer가 읽는다. Source metadata와 미연결 Draft는 generated runtime 문서로 새지 않는다.
- 기존 candidate atomicity, presentation generation admission, Server audition 관련 focused 검사만 실행한다. Server 관련 데이터 계약을 바꾸면 해당 Server loader/command 소비까지 컴파일·검증한다. Server revision 검사를 약화해 테스트를 통과시키지 않는다.

## G07. 기능별 검증과 사용자 재생 인계

### 구현 순서와 변경 단위

G01→G02를 먼저 닫아 source 저장을 정상화한다. G03은 stable identity/typed clock을 고정하고, G04/G05는 그 위에서 resource append/detail와 row/Draft 패턴을 연결한다. G06은 최종 Product 적용을 닫는다. 각 G에서 실제 사용하는 기존 파일만 수정하고 신규 C++ 파일은 계획하지 않는다. 현재 `.vcxproj/.filters`에 등록된 owner/runtime 파일을 재사용하므로 새 등록은 없다. 구현 중 파일 분리가 실제로 필요해지면 그 G 계획에 파일의 소비자·등록·검증을 먼저 반영하고 빈 placeholder를 만들지 않는다.

각 commit은 해당 G의 코드, source grammar/consumer, 필요한 focused test, PLAN/RESULT를 같은 기능 단위로 묶는다. 대규모 dirty worktree의 다른 Effect/쿠크/캐릭터 문서는 자동 stage·commit하지 않는다. `Client/Bin/Resources`에는 이 작업의 자동 변경이나 Git payload를 만들지 않는다.

### 자동 검증 명령과 기대 결과

다음은 구현 때 실행할 명령이다. 이 계획서를 작성한 사실을 실행 증거로 기록하지 않는다.

```powershell
python Tools/ValtanPipeline/valtan_tuning_pipeline.py --repository-root . validate
```

위 명령은 G00 baseline 및 G06 Product 준비 검증에 사용한다. G02의 Draft source Save 허용 조건으로 자동 실행하지 않는다. Draft source 도입 뒤에는 publisher의 준비된 encounter adapter를 통해 같은 Product 검증 계약을 유지한다.

```powershell
python -m unittest discover -s Tools/ValtanPipeline -p test_action_composition_atomic_save_contract.py
python -m unittest discover -s Tools/ValtanPipeline -p test_action_composition_dirty_owner_save_contract.py
python -m unittest discover -s Tools/ValtanPipeline -p test_valtan_canonical_typed_patch_transaction.py
python -m unittest discover -s Tools/ValtanPipeline -p test_action_composition_effect_v2_clip_projection_contract.py
python -m unittest discover -s Tools/ValtanPipeline -p test_action_composition_sequence_identity_contract.py
python -m unittest discover -s Tools/EffectToolV2 -p test_effect_v2_binding_pipeline.py
```

각 명령은 연결된 G에서만 실행한다. 다른 G의 미구현 기대값이나 무관한 전체 suite를 Save·컴파일의 필수 gate로 붙이지 않는다. fixture는 임시 디렉터리에서 writer를 실행하며 실제 Source·Products를 테스트 때문에 수정하지 않는다.

MSBuild는 설치된 Visual Studio의 `MSBuild.exe`를 resolve해 사용한다. 변경 CPP를 명시한 `/t:ClCompile /p:Configuration=Debug /p:Platform=x64 /p:BuildProjectReferences=false /m:1 /nr:false /v:minimal`로 최소 컴파일하고, 최종 변경 단위에서 `Client/Default/Client.vcxproj` Debug x64 Build로 link를 확인한다. Server consumer를 바꾼 G에서만 Server 프로젝트도 빌드한다. 변경 JSON은 실제 source/runtime parser로, XML을 바꿨을 때만 project/filter XML parse로 검증한다. 마지막으로 `git diff --check`를 실행한다.

### 사용자 입력·저장·재생 확인

| 확인 항목 | 사용자가 직접 수행할 입력 | 종료 관찰 |
|---|---|---|
| 기본 Source Save | 기존 Valtan pattern의 box 하나 수정→Save→Reopen | 선택/ID/수치가 유지되고 Product Publish 요구 없이 저장 완료가 표시된다. |
| V2 clock | `VALTAN_BIND_SLOT` shout 세 box 확인, 가운데만 이동/Detail Apply | 세 시작이 1400/2300/3200 ms에서 독립적으로 동작하고 해당 binding만 바뀐다. |
| V1/V2 append | Resources에서 V1 leaf/V2 leaf/group를 차례로 append | 각 box의 Detail·Play·Save가 같은 invocation을 소비한다. |
| row/Draft | 빈 Draft 생성→빈 row 추가→Save/Reopen→animation/effect append | 미완성 단계부터 저장되고 row·ID가 지속된다. |
| local Play | Play/Seek/Loop/Stop, typed clock·finite end 변경 | 이전 effect 잔류·중복 실행 없이 현재 preview revision을 소비한다. |
| 실패 보존 | 누락 resource 또는 잘못된 row가 있는 패턴과 정상 패턴을 함께 열기 | 오류 항목과 정상 항목이 모두 남고 정상 Source Save가 된다. |
| Server Product | 명시 Publish 성공 후 요구된 Server 적용 절차→기존 Valtan Server Play | gameplay/presentation revision이 맞고 기존 Server 권위 경로에서 실행된다. 실패하면 이전 Product가 유지된다. |

현재 LAN 스크립트는 이 PC를 `server-host`로 판정했고 `Server + Client` profile을 준비했다. 조사 당시 endpoint는 not-listening이었다. 실제 실행 확인 시에는 그 시점의 Server CMD·Client 상태를 다시 보고하고 사용자가 `Ctrl+F5`로 실행한다. 에이전트는 Client/UI를 자율 실행·조작·캡처하지 않는다. visual fidelity와 실제 아레나 재생 PASS는 사용자의 서면 관찰 이후에만 RESULT에 기록한다.

### 현재 문서 작성의 완료 상태

| 구분 | 2026-09-09 상태 |
|---|---|
| 구현 계획서 | 이 문서로 작성. G01~G07 구현은 아직 하지 않았다. |
| 현재 Source 구조 검증 | 읽기 전용 validate와 실제 Save 하위 projection/Sound/V2 candidate 검사 PASS. |
| 신규 schema·source-only Save·typed V2 clock·row/Draft | 계획이며 미구현이다. |
| Client 최소 컴파일·최종 link | 이 문서 변경에서는 실행하지 않았다. |
| Workbench 실제 입력·Save/Reopen·local/Server Play | 이번 조사에서 실행하지 않았다. |
| Client visual fidelity | 사용자 확인 전이다. |

구현 후에는 해당 RESULT에 실행한 검증과 미확인 화면 경계를 기록하고, 실제 public 계약이 바뀐 부분만 `CLAUDE.md`와 팀 사용서에 반영한다. 현재 계획서 작성 단계에서 공유 문서를 먼저 바꿔 구현 완료처럼 만들지 않는다.
