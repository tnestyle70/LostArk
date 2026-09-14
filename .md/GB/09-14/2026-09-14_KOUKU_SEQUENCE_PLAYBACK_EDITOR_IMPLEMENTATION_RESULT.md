# 쿠크 Sequence 재생과 Object 편집 구현 결과

## G00. 반영 범위와 상태

소스·데이터 반영 및 아래 수치 검증을 완료했다. Client/UI 실행·조작·캡처는 하지 않았다.
최종 화면·사용자 입력 판정은 사용자 확인 경계다. 기존 MainApp/Composition/Map 사용자
변경을 보존했고 Server Mario 변경과 같은 파일에서는 Preview 함수만 별도로 수정했다.
신규 C++ 파일은 없다. 전체 Product 빌드는 통합 작업에서 진행한다.

## G01. 작은 오망성 및 19,389ms 재생 중단

실제 작은 오망성 정본은 `effect.kouku.gate3.mario.boss.pentagram.full.restore`다.
Resource ID `kakulsaydon.effect.7e765b32937e215a2fef`와 마리오 Pattern 34가 같은 정본을
소비하며 `pantagram` 철자의 다른 asset/alias는 확인되지 않았다.

MainApp은 새 Resource Preview를 시작한 뒤 이전 Animation Preview를 정리하면서 정상
정리 메시지 `No KoukuSaydon composition preview is playing`을 공용 상태에 덮어썼다.
정리용 상태를 별도 지역 문자열로 분리했다. 실제 V1 실패는 준비 큐의 failure receipt를
`Get_ProductCuePreparationFailure`로 읽고 occurrence의 `failureStatus`에 처음 이유를
보존한다. 뒤의 성공/Queued 메시지가 앞의 실패 근거를 가리지 않는다.

19,389ms capture boundary에서는 이전 popup source-material-carriers의 커튼 준비 실패가
남아 있었다. `bg_rad_koukusaton_curtain01c_sm.wmodel`은 정상 decode됐지만 내장 material의
세 texture가 새 Effect/Meshes 상대 위치에 없었다. 원본 Map의 동일 hash 파일을 설치했다.
재질 override가 있어도 CModel은 내장 material 의존성을 먼저 준비하므로 mesh만 복사하면
실패한다. `Tools/KoukuSaydonPipeline/install_popup_curtain_materials.py --install`이 정확한
mesh hash와 세 원본 texture hash를 검증한 뒤 누락 파일만 설치한다. 다른 기존 파일은
덮어쓰지 않는다. `--install` 없이 설치 상태를 검사할 수 있다.

capture hold는 모든 과거 failed row 때문에 전체 preview를 중단하지 않고 실제 capture
occurrence의 실패만 처리한다. actor Pattern을 가진 bundle 경로도 동일한 boundary/한 프레임
대기를 소비하도록 연결했다. Preview의 V1 targets를 먼저 priority queue에 넣고 준비 시간이
짧은 Effect window를 소진하지 않도록 준비가 settled될 때까지 clock을 유지한다.

현재 설치 상태에서 작은 오망성의 Product 준비 실패 자체는 재현되지 않았다. 실제 오류
메시지 보존, cold preparation clock, 설치 누락, capture의 과도한 실패 전파를 각각 교정했다.

## G02. Object timeline 선택·복제

Motion timeline에 Stage/Transform/Animation/Effect 이름을 표시하며 Animation은 slot별 한
행에 배치한다. 개별 animation/effect box의 hit target이 Detail 선택과 연결된다. Animation
Detail의 Duplicate Clip, Effect Detail의 Duplicate Selected Effect, timeline의 Duplicate
Selected Box가 같은 `CWorldSequenceDocument::Duplicate_TimelineBox`를 호출한다.

문서가 candidate를 검증한 뒤 대상 template만 교체한다. Animation duplicate는 현재 clip의
window 뒤에 같은 clip을 삽입하고 뒤의 같은 slot clip을 이동한다. Effect duplicate는 새 stable
ID와 TIME 시작점을 만든다. 필요한 motion 끝 key만 hold로 연장하며 기존 template 주소와
원본 key/Effect 값을 보존한다. 용량·600초 제한·잘못된 선택은 문서와 선택 결과를 보존한다.

실제 작은 괴기스러운 인형 flame template의 11,334ms 값을 사용한 native probe는 Effect와
Animation을 두 번씩 복제해 34,002ms 세 window를 만들고 Save→Load equivalent, template
주소 보존, ID 분리, 실패 rollback까지 13 checks를 통과했다. fixture는 기존 source의 단일
motion과 model/effect 연결을 복사하고 다른 motion 참조만 제거한 입력이며 UI 조작 증거는 아니다.

## G03. 팝업북과 실제 세이튼 Pattern

원본 boss Pattern 36 `세이튼_1관문연출`은 13 clip/40,652ms다. 명목 animation blend Logic 52에
triggerKind가 없어 실제 blend가 없었다. 다른 Pattern의 Logic 52는 유지하고 P36 전용
`kakulsaydon.g1.logic.62` ANIMATION_BLEND에 기존 다섯 window를 연결했다.

Sequencer revision 43에 P36의 실제 clip/source range/play rate를 보존한 Pattern 8
`1관문_연출`을 추가했다. P1과 P4의 Pattern row가 이를 참조한다. 기존 `world.13`의
DEPLOY_BOSS_MN_RPCT_00 puppet 행은 제거하고 그 WORLD에 붙던 spotlight는 실제 BOSS
anchor로 옮겼다. actual actor는 기존 BossCatalog의 MN_RPCT_05 CModel/weapon clone을
사용한다. 걷기·나머지 clip과 다섯 Logic blend가 기존 native animation/root-motion sampler로
이어진다. 확장된 child animation들은 clip마다 행을 늘리지 않고 한 Animation 행에 표시한다.

전체 13 clip이 끝나도록 P1은 37,800→40,652ms, P4는 58,810→61,662ms로 연장했다.
P4 child 시작 21,010ms 및 기존 portal/capture/Effect의 모든 시작점은 유지했다. 마지막
camera/scene profile의 끝 window만 함께 연장해 끝 두 clip 중 시점이 먼저 돌아가지 않게 했다.
원본 WORLD/Effect의 동작 속도와 lifetime은 유지한다.

단일 actor Pattern은 기존 Level WORLD preview owner에 world/lighting 관리를 맡긴다.
그 결과 popup map-light 준비, 구형 book 숨김, 펼친 맵/기본 아레나의 배타적 visibility 계약을
유지하면서 actor animation만 bundle sampler가 담당한다. WORLD가 먼저 sample된 뒤 해당
actor Effect/light가 같은 clock을 소비한다.

## G04. 실행한 검증과 남은 사용자 확인

- `out/KoukuSequenceFix20260914/compile.ps1`: WorldSequenceDocument, WorldObjectTool,
  KoukuSaydonActionWorkbench, KoukuSaydonPresentationPlayer, Effect_PresentationService,
  MainApp의 Debug isolated CL 6 TUs PASS.
- `native_probe.cpp`: pentagram, portal-collapse, portal-context, popup-light, popup-carrier의
  실제 Product CPU stage/Playback Seek PASS(5 documents).
- `gpu_probe.cpp`: UI/window 없는 D3D11 WARP에서 같은 5개 실제 document의
  Stage_Document 및 Stage_VisualProgramTarget PASS. capture render/visual 판정은 하지 않았다.
- `duplicate_probe.cpp`: native duplicate/Save/Load/failure-preserve 13 checks PASS.
- `intro_probe.cpp`: Sequencer parse 및 serialize roundtrip PASS. P1 expansion
  40,652ms/13 clips/5 blends, P4 61,662ms/14 clips(첫 idle 포함)/5 blends PASS.
  설치 MN_RPCT_05 WModel을 실제 decode하고 모든 expanded clip 이름과 마지막 source sample
  범위를 native duration으로 검사해 각각 13/14 endpoints PASS.
- `curtain-texture-closure.json`: exact mesh와 embedded texture 3개 SHA-256 일치.
- 수정 source의 `git diff --check` PASS. 소스 C++의 기존 UTF-8/CRLF를 유지했다.

사용자는 새 Product 빌드 후 F1에서 작은 오망성 Play Effect, 마리오1 preview,
Sequencer `1관문_통합_시퀀스`의 19,389ms 통과 및 61,662ms 완료를 확인한다. 팝업북 마지막
두 clip과 카메라·조명의 화면 결과도 확인한다. Object에서 작은/큰 괴기스러운 인형의 화염
motion을 열어 Animation/Effect box를 각각 선택→Duplicate→Save→다시 열기 순서로 확인한다.
ScreenPost/재질 복원의 최종 시각 PASS는 이 문서에 기록하지 않는다.

## G05. Client focus 입력·사운드와 백색 Map Light 삭제

사용자는 외부 앱에 프롬프트를 입력하는 동안 Client 스킬이 실행된다고 보고했고,
Winters와 같은 foreground 입력·사운드 정책 및 여러 EXE의 중복 사운드 방지를 요청했다.
`CInput_Device`의 키보드·마우스가 모두 `DISCL_BACKGROUND`로 수집하고 읽기 실패를
무시하던 것을 확인했다. `DISCL_FOREGROUND`, raw getter의 창 검사, focus/read 실패 시
상태 초기화와 복귀 시 held 입력 release 대기를 반영했다. `CPlayerController`는
foreground가 아니면 새 gameplay 명령을 막고 기존 조준·hold 취소 경로를 사용한다.
MainApp의 직접 Escape 폴링에도 같은 프로세스 focus 검사를 추가했다.

`C:/Users/user/Desktop/Winters/Engine/Private/Sound/Sound_Manager.cpp`의
`UpdateApplicationFocusMute`를 대조했다. 기존 LostArk `CSound_Manager`의 Initialize,
Update와 실제 sound 재생 경로에 foreground PID 기준 FMOD master mute를 연결했다.
같은 프로세스의 분리 도구 창은 소리를 유지하고 다른 Client EXE·외부 앱은 현재 Client의
소리를 mute한다. 채널별 볼륨·명시 pause·seek·재생 시각은 유지한다.

삭제 대상은 사용자가 `월드_3관문_백색_스포트라이트`라고 확정한
`light.LV_LUT_MIDNIGHTC_ED.7`이다. 초기 디스크에는 enabled=false로 남아 있었다.
사용자가 Save한 RenderingProfiles는 revision 38→40과 정렬·서식 변경뿐이며 profile
내용은 동일했다. Map Delete 후 기본 방향광을 선택하면서 다음 Save가 RenderingProfiles를
저장하던 UI 연결을 확인했다.

`MainApp_RenderingLighting.cpp`는 삭제 후 Map 저장 도메인을 유지하고, `Light Resources`
목록 위에 선택 없이 전체 Map 변경을 저장하는 `Save Map Lights`와 `Publish Map Lights`를
제공한다. Map 상세에도 같은 이름을 사용하며, 삭제된 행이 없어도 저장할 수 있다.
`CAreaLightAuthoringSession::Publish_Runtime`은 기존 publisher의 새 `-Scope Lights`로
maplights 한 파일만 게시한다. 기존 parser·참조 보호·freshness·파일 교체 트랜잭션을 유지한다.

확정된 `.7` 한 행을 authoring에서 삭제하고 공식 publisher로 runtime에 게시했다.
양쪽 122→121행, `nextLightOrdinal=8`, 청색 `.6` 및 나머지 행의 모든 값을 유지했다.
다른 JSON의 `.7` 참조는 없었다. Lights Validate→Publish→Check가 성공했고 게시 파일은
1개다. 사용자 RenderingProfiles 및 다른 Map runtime 파일은 baseline SHA256과 동일하다.
증거는 `out/KoukuSpotlightRemoval20260914/verification.json`, `publish.log`, `check.log`다.

입력·사운드의 실제 함수 본문과 inline getter를 추출하고 Win32/DirectInput/FMOD만 대체한
focused C++ 검사는 44 checks, failures 0이다. 비활성·복귀 held 입력·실패한 partial read·
제한된 재획득·raw UI 차단·foreground PID 변화·mute 실패 재시도와 기존 sound 설정 보존을
확인했다. `out/FocusInputSound20260914/result.json` 및 source-inputs에 근거를 기록했다.
실제 Client/UI 조작이나 여러 EXE의 청취 판정은 수행하지 않았다.

## G06. 19,819ms capture의 저FPS 반복 재생성 수정

`CKoukuSaydonPresentationPlayer::Sample`은 sample 간격이 150ms를 넘을 때
`Stop_Session`으로 V1 handle을 파괴했다. renderer가 그 handle의 고정 HDR/bloom을
소유하므로 다음 `Resolve_PreviewCaptureClock`은 미캡처로 판단해 19,819ms로 돌아간다.
즉 GPU capture 완료가 성공해도 느린 다음 프레임이 그 상태를 잃는 재발 경로였다.

단일·bundle Preview는 같은 occurrence의 V1/V2 외부 clock으로 계속 샘플하도록 변경했다.
실제 박스 종료·Stop·완료·실패·새 Preview의 정리는 그대로이며 Server 제품 session의 기존
discontinuity 처리도 유지한다. 저작된 시작 시각·카메라·수축 시간·shader는 변경하지 않았다.

현재 source의 실제 resolver와 Sample의 session 정리 분기를 함께 추출한 전후 검사는
단일·bundle 각각 100/200/500ms 프레임으로 실행했다. HEAD의 200ms 경우 capture 45회,
500ms 경우 18회 재생성을 재현했고, 수정 후 여섯 경우 모두 capture 1회를 유지하며
37,630ms 박스 끝을 통과했다. product discontinuity 정리도 유지됐다. render 대상은 이미
존재하던 handle만 다음 프레임에 받는 mock이며 실제 WORLD/Client 화면 재생은 아니다.
증거는 `out/PortalCaptureRecurrenceClock20260914/before.result.txt`, `after.result.txt`,
실제 전후 branch 추출이 포함된 `build_probe.py`다.

별도로 실제 portal Product load→GPU Stage/Commit→Prepared Playback/renderer→Seek→
Build_NativeScreenPost→Material Bind를 같은 renderer 수명으로 연결했다. 4.259초는
ScreenPost 0개, 4.260초는 1개였다. 첫 priming Bind는 capture pending, 다음 허용 Bind는
capture ready이며 후속 zero-delta update와 4.261초에서도 유지됐다. 창 없는 WARP의
4×4 synthetic HDR/bloom 입력만 사용했고 게임 장면·이미지를 생성하지 않았다.
현재 Playback/Renderer_Rendering은 다시 컴파일했고 다른 준비된 OBJ는 재사용했다.
포탈 전체 history seek 1회의 실측은 약49~57ms다. 증거는
`out/PortalCaptureRecurrence20260914/receipt.json`, `result.log`다.

## G07. 현재 컴파일·반영 상태와 사용자 실행 대기

변경 Client 5 TU(PresentationPlayer, MainApp, RenderingLighting, AreaLightAuthoringSession,
PlayerController)와 Engine 2 TU(Input_Device, Sound_Manager)의 Debug 격리 컴파일을
통과했다. 기존 한글 code page 경고는 남아 있다. 최초 Client 시도 중 다른 진행 작업이 추가한
Mario return 인터페이스와 Shared 선언의 변경 시점이 달라 미완성 선언 오류가 발생했으나,
현재 선언을 다시 읽은 후 재컴파일은 성공했다. 별도 Engine 컴파일 명령의 FMOD include 누락도
공식 include 경로를 맞춰 재검증했다. 다른 작업의 Mario 소스는 수정하지 않았다.
명령과 최종 로그는 `out/SequenceCaptureFocus20260914/compile.ps1`,
`client-compile.log`, `engine-compile.log`에 있다.

변경 maplights 두 파일과 기존 사용자 RenderingProfiles의 JSON parse, 공식 Lights publisher의
Validate/Publish/Check, `git diff --check`를 통과했다. 신규 C++/project/filter 변경은 없다.
기존 C++ 파일 인코딩·CRLF를 보존했다. 다른 작업의 Shared/Server/Mario/네트워크 변경과
사용자 RenderingProfiles diff를 함께 stage하거나 되돌리지 않았다.

사용자는 실행 중 EXE를 사용한 뒤 종료 사실을 알려주겠다고 답했다. 현재 Client와 Server를
에이전트가 종료하거나 새로 실행하지 않았다. **최종 Product Build와 Engine SDK·DLL/Client
실행 파일 교체는 EXE 종료 후 진행 대기**다. 격리 컴파일을 runtime 적용 완료로 기록하지 않는다.

사용자 확인 경로는 다음과 같다.

1. 새 Product Build 후 `F1 → Rendering Workbench → Light Resources`,
   `Level category: KoukuSaydon`에서 백색 `.7`이 없는지 확인한다. Map 조명 변경 후
   `Save Map Lights → Publish Map Lights`로 전체 저장·반영한다. 이번 `.7`은 이미 게시됐다.
2. Sequencer `1관문_통합_시퀀스` Preview/Complete Play에서 19,819ms를 통과하여
   고정 장면 수축·검은 배경·이동과 후속 재생이 이어지는지 확인한다.
3. Client focus를 벗어나 프롬프트를 입력할 때 스킬·마우스 동작·소리가 없는지 확인한다.
   복귀 시 이미 누른 키·버튼은 놓고 다시 누른다. 여러 EXE 중 전경 Client만 소리 나는지 확인한다.

실제 입력·화면·청취의 최종 판정은 사용자 확인으로 남아 있다.

## G08. Despawn Fire Object와 반경 편집 후속 작업

사용자가 요청한 `Despawn Fire Object`를 F1 Gate Controls의 `Despawn Arena Bosses`
바로 아래에 추가했다. `Debug_DespawnFireObjects`는 실제 gateIndex=2인 기본 외곽불
owner만 기존 release 경로로 정리한다. Boss despawn이 active gate 표시를 비워도 동작한다.
Gate/player 이동 대기 중에는 UI와 Level에서 모두 막으며 실패 상태를 전달한다.
정상 제거 뒤 owner를 폐기해 Update나 다른 Gate 전환 실패 rollback에서 다시 생성되지 않는다.
다음 명시적인 Gate 3 activation은 기존 준비·commit 경로로 불을 만든다.

사용자는 뒤의 ‘방금 수정한 정보 복원’이 버튼 추가 취소가 아니라 방금 조절한 외곽불
반경/배치 복원이라고 확정했다. 여섯 기본 외곽불 저장값은 원래대로이며 57m 저장값은 없다.
현재 실행 중 툴의 미저장 draft를 에이전트가 초기화했다고 기록하지 않는다.
`WorldObjectTool`의 실제 반경·저장값 대비 증감량·휠 즉시 Preview와 기본불 중복 방지의
구현 및 수치 검증은 09-13 `WORLD_OBJECT_TRAVEL_PARENT_LOOP_IMPLEMENTATION_RESULT`
G08을 따른다.

MainApp, WorldObjectTool, Level_KakulSaydonArena_WorldObjects의 현재 Debug 격리 컴파일은
3 TU 모두 통과했다. 로그는 `out/SequenceCaptureFocus20260914/fire-radial-compile.log`,
명령은 같은 폴더의 `compile-fire-radial.ps1`이다. header BOM과 각 파일의 UTF-8/CRLF를
유지했다. 현재 Client PID11224/Server PID53512는 계속 실행 중이며 종료하지 않았다.
이 버튼과 반경 변경도 앞의 capture·focus·sound 변경과 함께 최종 Product Build 대기다.

## G09. Parent 진단과 Effect 목록 통합

빈 Parent는 폴더 소속만으로 실행되지 않는다는 안내와 실제 child box 수를 표시한다.
실행 행이 전혀 없는 Parent의 Complete Play만 거부하고 공통 Effect/Logic/명시적 idle은 보존했다.
Kouku All Effects와 Composition Resources의 저장 Effect 목록을 같은 helper/cache에 연결했다.
1관문·2관문·3관문·공통 네 분류, 표시명·검색·정렬을 공유하며 BINGO는 3관문에 포함한다.
실제 catalog/metadata union 652개(256/201/180/15)를 모두 보존했고 실제 helper 추출 probe의
5,510 checks가 통과했다. Effect 트리 3 TU 컴파일도 통과했다.
증거는 `out/KoukuParentEffectTree20260914/effect-tree-probe-receipt.json`과
`out/EffectTreeParity20260914/compile-receipt.json`이다. UI/GPU를 실행한 결과는 아니다.

## G10. 전방 Summon과 세 방향 분신 연결

사용자가 저장한 revision 553을 보존하고 요청한 다섯 Pattern만 갱신해 revision 554로 저장했다.
P52 Parent에 P50 전방을 0~1600ms child box로 연결했다. P50의 기존 Summon.2 박스에는
P53 후방·P54 왼쪽·P55 오른쪽을 각각 독립 spawnId로 연결했다. P53~55를 MECHANIC으로
바꾸고 P55의 왼쪽 복제 클립을 원본 P44의 오른쪽 4219923/rpct00_att_battle_34_03으로 교정했다.
기존 P44 Summon과 다른 Pattern·root 값은 보존했다. 변경 전 원문은
`out/SummonPatternSpawns20260914/composition-before-summon.json`에 보존했다.

Summon occurrence의 optional patternSpawns를 Client codec/UI → Python projection →
Gameplay publisher → Server trigger/clone admission → Client dependent spawn/snapshot에 연결했다.
최대 네 개의 같은 actor/Gate animation-only Pattern을 지원하고 별도 개체에서 원본 root motion을
재생한다. 본체 현재 위치·yaw에 저작 offset을 적용하며 이번 세 분신은 모두 offset 0이다.
임의 반경을 더하지 않았다. 기존 owner ID·sequence·endTick 정리와 primary AI 제외를 재사용한다.
세 개체와 spawn payload를 모두 준비한 뒤 commit하며 실패 시 기존 본체·개체를 유지한다.
G3 dependent CNpc를 허용하고 분신 despawn이 본체 HUD를 지우지 않도록 했다.

실제 codec 62 checks, Python projection/negative 19 checks(14 invalid cases), 변경 Client와
Server의 최소 TU 컴파일을 통과했다. Composition publish는 46 Product Patterns/322 stages와
8 Bundles를 생성했고 P50·P52 각각 1개 trigger/3개 spawn의 실제 bootstrap 생성을 확인했다.
증거는 `out/SummonPatternSpawns20260914/receipt.json`, `projection-test-receipt.json`,
`data-connection-receipt.json`, `out/KoukuSummonPatterns20260914/server/compile.log` 및
`out/KoukuParentEffectTree20260914/client-replication-verification.json`이다.

전방 본체의 추가 시퀀스·이펙트는 사용자 저작으로 남겼다. 분신 자식의 재귀 Summon·Logic·
World·Scene·presentation 실행은 지원하지 않고 거부한다. 실제 세 방향 위치·표시는 사용자가
새 빌드에서 직접 확인한다. EXE 종료 후 최종 Product 빌드 결과는 아래에 별도로 기록한다.

정식 `Publish-GameplayBalance.ps1 -Mode Publish -SkipValtanSplitProjection`도 통과했고
`Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`까지 게시했다. source revision은 554다.
최종 5 TU 재컴파일과 diff 검사를 통과한 뒤 Product 빌드를 시도했으나, 그 사이 사용자가
새 EXE를 실행하여 출력물 잠금 검사에서 빌드 시작 전에 중단했다. 에이전트가 EXE를 종료하지 않았다.
확인한 Client PID3236 / Server PID50676은 11:54:11 실행이며 Server.exe는 11:53:05 생성,
최종 Server 소스 가드는 11:55:10 수정이라 마지막 수정이 현재 EXE에 포함되지 않았다.
Client·Server 종료 후 솔루션 Build와 Server/Client 재시작이 필요하다. 최종 Product 링크 성공이나
실제 세 분신 화면·사운드·입력 검증을 에이전트가 완료했다고 기록하지 않는다.

## G11. 1관문 통합 시퀀스의 WORLD 카메라 검증 회귀

사용자가 첨부한 화면의 `World Object anchor requires a worldId` 오류는 Sequence 정본
revision 43의 P4.presentation.23에서 발생했다. 이 행은 CAMERA `카메라_통합_피날레`다.
최근 추가한 공용 authoring 검사가 kind에 관계없이 WORLD의 빈 worldId를 거부해 기존
카메라를 재생 준비에서 차단했다. Sample_CompositionCamera는 shot asset 좌표를 사용하므로
이 카메라에는 World Object 참조가 필요하지 않다. 저장 데이터 손상이나 이번 Summon 변경이
원인이 아니다. Gameplay publish·컴파일·Summon codec 검사에 별도 Sequence 정본의 실제
Preview 입력을 포함하지 못한 것이 앞선 검증의 누락이다.

실제 Sequence의 WORLD/빈 worldId 45행은 모두 Camera이며 P3=5, P4=7, P5=18, P6=7,
P7=8행이다. 원본 생성기도 같은 Camera 표기를 사용한다. Sequence와 Gameplay JSON을
변경하거나 카메라를 MAP으로 바꾸지 않았다. Client Validate의 일반/PRODUCT 검사와
Product Read_Occurrence, Python validator에서 Object 참조 필수 범위를 Effect/Light/Collider로
일치시켰다. Camera/Sound는 기존 전역 재생을 유지하며 누락된 Effect Object 참조는 계속 거절한다.

실제 C++ codec 57 checks/0 failures로 Sequence 8개 Parse/Validate/Expand, Gameplay의
P50/P52 Summon 확장, 두 정본 Serialize/Parse 동일성과 P4 Camera 7행의 모든 값 보존을
확인했다. 최종 Document TU 컴파일도 통과했다. `out/SequenceCameraAnchor20260914/receipt.json`에
최종 source hash와 두 JSON 변경 없음이 기록돼 있다. Python 집중 회귀 4개도 통과했다.
재생 준비 검증 실패를 CPU/GPU 병목으로 해석하지 않으며 실제 렌더 비용은 이번에 측정하지 않았다.
gotchas, 렌더링 복원 원리 문서와 팀 사용서에 kind별 앵커와 두 정본 검사 경계를 기록했다.

실제 Read_Occurrence와 JSON helpers를 사용한 CPU reader 검증도 55/55 통과했다.
이전 reader는 기존 Camera 45행과 전역 Sound 1행을 거부했으며 수정 후 원래 값으로 읽었다.
WORLD Effect/Light/Collider의 누락 참조 거부와 정상 Object 참조·MAP Effect 조건은 보존했다.
최종 PresentationPlayer TU 컴파일과 diff 검사도 통과했다.
증거는 `out/SequenceCameraAnchorReader20260914/after-result.log`와 `compile.log`다.

현재 Client PID3236/Server PID50676은 실행 중이다. 새 Client 링크와 실제 통합 시퀀스 화면은
EXE 종료 뒤 Build·재실행 및 사용자 확인으로 남는다. 실행 중 EXE를 종료하거나 UI를 조작하지 않았다.

## G12. 19,819ms 라이브 대기 확인과 임시 제외 — G13에서 원복

현재 Client PID45256/12:10:34 실행본을 matching PDB와 VM_READ/QUERY 권한으로 조사했다.
Client 실행·UI 조작·화면 캡처·attach·suspend·메모리 쓰기는 하지 않았다. 처음 읽은
stopped/queued 상태에는 사용자의 명시 Reset 이력이 있었으므로 최초 멈춤 원인으로 쓰지 않는다.
사용자가 다시 재생한 monitor.txt, held-detail.txt/held-detail2.txt, monitor2.txt에서
P4가 한 member Bundle backend로 19,819ms를 대기하는 실제 경로를 확보했다.

held/sampled/allowed=1, active portal handle 존재, pending spawn=0, capture map의
hLastResult=S_FALSE와 color/bloom 없음이 확인됐다. WORLD 오류는 없었다. 대기는
120초가 아니라 120 preview updates이며 이번 실행은 약400ms/update였다. 이후 capture
timeout이 Player·SequenceWorkbench에 기록되고 owner 해제/Preview 종료로 이어졌다.
그 전에 표시된 Queued admitted Effect 문구는 capture 진행 상태가 아닌 과거 spawn 요청 결과다.
전체 history seek를 경계에서 매번 반복하는 비용도 있어 120회 대기가 약48초까지 길어졌다.

float 경계와 실제 Product load → CEffectObject → Submit_FrameProviders → synthetic
material Bind 검증은 정상이나, 라이브 Render에서 pair 생성이 누락된 정확한 지점은
확정하지 못했다. 이 검증을 실제 Bundle+Render 완료 또는 캡처 엔진 수정 완료로 기록하지 않는다.
읽기 전용 메모리 여러 field는 같은 원자적 snapshot이 아니므로 seek 진행 중 읽힌 Frame 시간의
일시적 불일치를 원인으로 단정하지 않는다.

사용자는 캡처 연출을 제외하고 팝업북으로 진행하는 대안을 승인했다. 전용
effect.kouku.gate1.intro.portal-suction.scene-collapse의 authored.scene-image-collapse만
visible=false로 숨겨 resolver와 playback에서 제외했다. captureShrinkSeconds를 보존하려면
ScreenPost.enabled 메타데이터가 필요하므로 enabled와 shrink 설정은 원본대로 유지했다. 나머지30 particle과 다른 Effect,
Sequence revision43의 전체 bytes, Camera/WORLD/팝업북 시작21,010ms는 보존했다.
별도 원본 fade는 약20.514초 암전 후23.131초에 풀리는 기존 값 그대로다.

수정 전 원본은 out/PortalActualCapture20260914/portal-before-disable.effect.json에 보존했다.
수정은 정확히 visible bool1개이며 변경 후 SHA256은 d50c74ea0467dfe16a3846dbc025c1f132958377d8b479350952c0aef8ad4385다.
disabled-data-result.json에서 실제 Sequence P4의 V1 occurrence5개를 조사해 활성 capture
후보0개를 확인했다. 실제 Effect catalog는 ProjectDataRoot의 Authored JSON을 읽으므로
publisher/EXE/CSO 변경은 없다. 현재 Client는 이미 읽은 문서를 보유하므로 사용자 편집 Save 후
Client 재시작이 필요하다. Server는 이번 데이터 변경의 소비자가 아니다. 화면 재생 판정은 사용자에게 남긴다.

## G13. 기존 암전의 BC1 알파 검증으로 취소되던 캡처 수정

사용자는 연출을 무조건 제외하라는 뜻이 아니라 구조적으로 불가능할 때만 제외하고,
가능하면 수정하여 유지하라고 정정했다. G12의 임시 제외는 취소했다. Portal JSON은
원본 backup과 byte가 완전히 같은 visible=true 상태로 복원했다. Sequence revision43도
원래 SHA256 8eb1429d6f97acbc1f21dd2c5cfa6b32cc342260e477303c8afe794e8dfbef9c 그대로다.
캡처·검은 배경·수축, 나머지 particle30개와21,010ms 팝업북 timing을 유지한다.

실제 연결은 P4.presentation.19 → kakulsaydon.g1.presentation.48 →
kouku.gate1.authored.fade.black → Effect/KoukuSaydon/Textures/FX_TEX_00/fx_a_blankwhite_01.dds다.
V2 TexturedOverlay가0~58,810ms 동안 사용하는 이 DDS는 DXT1이다. 실제 V2 loader와
같은 CreateDDSTextureFromFileEx 및 PARAMS 기본 SRGB 설정으로 생성한 SRV는
BC1_UNORM_SRGB(72), 기본 coverage는 A(3)였다. 기존 Engine이 이 형식의 A를 거부했다.
동일 입력에서 coverage만 R로 바꾸면 통과하여 이 거부 조건을 분리했다. 텍스처나 저장된
coverage를 R로 바꾸는 우회는 적용하지 않았다.

또한 기존 Submit_FrameProviders는 격리 가능한 provider 하나의 실패에도 전체 frame의
post/light/overlay를 지우고 S_FALSE를 반환했다. V2 암전 provider는 계속 재등록되고
Renderer는 S_FALSE에서 실행을 계속하므로 portal 객체는 정상인데 캡처 Bind가 실행되지
않았다. 실제 설치 DLL과 원본 DDS, 정상 portal을 함께 제출한 검사에서20프레임 연속
posts0, render S_OK, capture S_FALSE가 재현됐다. 이것이19,819ms의 완료 대기를 막는
연결된 결함이다. GPU 캡처 자체의 구조적 한계가 아니며 timeout을 늘리는 수정도 아니다.

Engine/Private/Presentation_Manager.cpp에서 BC1/BC1_SRGB의 A coverage를 허용했다.
BC1의 불투명/1-bit alpha 계약과 일치하며 다른 형식의 channel/SRGB 검사는 유지했다.
명시적인 isolated LOCAL_PROVIDER_CONTRACT 실패는 해당 provider의 contribution과
channel 통계만 rollback한다. 실패 provider는 Finalize(false) 한 번, 정상 provider는
채널 검증 후 Finalize(true)를 받는다. 격리 횟수와 S_FALSE는 유지하며 전역 실패·budget
초과·불명확한 scope는 계속 전체 rollback한다. 새 public 타입이나 별도 렌더 경로는 없다.

검증 결과는 다음과 같다.

- 실제 Presentation_Manager.cpp 독립 CPU 검사: 수정 전138조건 중37실패, 수정 후138/138 통과.
  provider 순서,3채널 부분 제출,복수 실패,후속 전역 실패,용량,통계와 Finalize1회를 확인했다.
- 새 정식 Engine DLL: 실제 BC1_SRGB/A overlay S_OK. 정상 portal+암전이 post1/overlay1로
  commit되고, priming frame 다음 allowed frame에서 captureResult=S_OK/ready1이었다.
- 별도 local failure를20프레임 반복해도 정상 post/overlay와 captureReady1이 보존됐다.
  이 실패 입력은 실제 DDS 복구 검증과 별도로 failure isolation을 검사하는 주입 조건이다.
- 정식 Debug Product Build는 Engine/Shared/Server/Client 모두 PASS, elapsed6699ms.
  Engine OBJ1개를 컴파일했고 CSO 쓰기는0개다. Engine DLL을 Client runtime에 배포했다.
  Engine_Enum.h의 기존 codepage 경고는 있었고 빌드 오류는 없었다.
- Portal 원본 복원과 Sequence의 JSON parse 및 byte/hash 보존, git diff --check를 확인했다.

검증 근거는 out/PortalActualCapture20260914의 provider-fade-bc1-audit.json,
renderer-dds-before-receipt.json, renderer-dds-before-probe-result.log,
renderer-dds-after-probe-result.log와 out/PresentationProviderIsolation20260914/result-after.log다.
Product 결과는 out/BuildPipeline/runs/20260914T041207760Z-debug-product.json이다.
Engine/Bin/Debug와 Client/Bin/Debug의 Engine.dll SHA256은 모두
507d461035c2d39005f7bc1907c5b84ef21b3ae64c5a6faee81f6e4d2321c1cc다.

GPU 검증은 synthetic4×4 WARP와 실제 DDS/캡처 Object/provider/설치 Renderer를 사용했다.
전체 Client UI나 게임 장면을 실행·조작·촬영하지 않았고 최종 화면 PASS를 대신 판정하지 않았다.
검증 당시 Client와 Server는 종료 상태였다. 사용자는 Server와 Client를 실행하고
시퀀서의1관문_통합_시퀀스 → Complete Play로19,819ms 통과와 수축/이동/팝업북 화면을
확인한다. 필요한 소스·빌드·배포는 완료됐으며 추가 Product Build 대기는 없다.


## G14. 한 번 캡처한 장면을 19,819~21,010ms에 축소

사용자는 G13 설치본에서 실제 화면 축소를 확인했다. 후속의 어두워짐·재등장은 별도 V2 display fade가 20,441ms에 약94.1%, 20,538ms에 거의100%, 21,315ms에도94.4%로 화면을 덮기 때문이었다. box 끝만 당겨도 V1 내부 shrink4.130초는 그대로여서 중간 크기에서 끊겼다. 현재 저장 자료에는 동시에 활성인 다른 V1 ScreenPost가 없고, 왜곡은 capture 전에 해결된다. 두 연출을 포기할 필요가 없었다.

사용자가 확정한 capture19,819→popup21,010ms를 구현했다. P4.presentation.36의 시작은15,559,길이는5,451ms이며 원래4.26초 delay를 유지한다. 최근 위치[65.723,2.75,-95.438]와 회전[-5.4,-8,0.15] 및 나머지 occurrence 값은 보존했다. fade.black에서 최초0~1.9971559초의16개 key와 종료58.81초 zero는 그대로 두고 두 번째 전환30개 key만 제거했다. 원본 portal particles와 ScreenPost shrink metadata는 바꾸지 않았다.

Sequence의 일반/geometry external Seek가 box duration을 전달한다. pending descriptor와 active Object에서 occurrence renderer로 end age를 연결하고 SCENE_COLLAPSE의 shrink만 가용 시간으로 제한한다. 양수 end는 external sampling 전용이며 NaN/음수는 거부한다. 0은 기존 authored 동작이고 cube/model-cue 경로는 제외한다. Clear는 새 시간을0으로 돌린다. 저장 schema와 shared prepared cache에는 새 상태를 넣지 않는다.

실제 Product Effect load/Playback/Build_NativeScreenPost 및 synthetic WARP 검사에서 다음을 확인했다. Client/UI 실행·캡처나 사용자 화면 판정은 수행하지 않았다.

- 19,819ms progress0,20,414.5ms progress0.5,21,009ms progress0.999160051346.
- 박스 길이5.451→7.451초 수정 시 진행률만0.186618611217로 바뀌고 capture state와 Color/Bloom SRV가 동일했다. 다른 live 입력으로 Bind해도 다시 캡처하지 않았다.
- 외부 end0은 authored progress0.144184961915를 유지했고 particle age는 바뀌지 않았다. Clear 뒤 end0을 확인했다. cube는 새 guard에서 제외됨을 소스 대조했으며 별도 cube 화면 검사는 하지 않았다.
- Service/Object/PresentationPlayer 3TU와 Renderer/Rendering/Staging 3TU의 격리 Debug 컴파일 및 probe 링크가 통과했다. Product 링크 결과는 후속 항목에 기록한다.
- 수정한 fade를 기존 V2 authored validator로 검증했다.19,819/20,441/20,538/21,010/21,315/23,698ms의 overlay intensity는 모두0이며 첫 fade16개 key는 exact 유지됐다.

종료 직전 matched-PDB VM_READ snapshot에서 Sequence revision47과 Gameplay558은 clean이고 LastGood 원문과 디스크가 같았다. 이후 사용자 Client/Server 종료를 확인하고 직전 bytes 비교와 백업 뒤 필요한 필드만 수정했다. 사용자 조명 원본과 RenderingProfiles는 수정하지 않았다.

근거: out/PortalClipWindow20260914/result.log,compile-probe.log; out/PortalActualCapture20260914/clip/compile-client-clip-receipt.json; g14-fade-validation/receipt.json; draft-freshness-p36-receipt.json; g14-data-backup/. 새 상태의 실제 장면 연결은 사용자의 재생 확인이 남는다.

## G15. Stage 공백 축소 시 Animation Blend 박스가 남던 오류

사용자 첨부 이미지에는 P36 첫 walk가0~4,997ms로 이동했지만 STAGE_1은 약15초까지
남아 있고, `logic.1: the box must contain exactly one transition between consecutive pose owners`
오류와 Unsaved changes가 표시된다. 실제 디스크 P36은 STAGE_1=15,274ms,
walk startOffset=10,277ms/play=4,997ms다. 화면의 walk 위치는 미저장 편집 상태이므로
저장 파일을 외부 수정하거나 사용자의 Client를 조작하지 않았다.

Workbench의 `Retime_AnimationBlendWindows`를 기존 Stage duration과 Pattern Start Offset
setter에 연결했다. 변경 전 실제 resolver가 확정한 source/target occurrence 쌍을 보존하면서
target 경계 이동량만큼 해당 blend 시작만 이동한다. 이동 후 기존 검증을 다시 실행하고,
다른 pair·음수·다중 경계·겹침이면 candidate를 거절한다. 일반 lane 시간과 source clip 범위는
그대로다. 선택 Stage 상세에 `Fit Stage to Animation`을 추가하여 현재 animation 끝으로
Stage 길이를 맞춘다. 수동 Duration 입력과 Stage edge drag도 같은 setter를 소비한다.

실제 Workbench helper, 두 setter와 Commit_Candidate 정의를 추출한 CPU 검사에 실제
Composition codec/validator를 연결했다. UI selection 및 buffer 동기화만 inert 함수로
치환했으므로 GUI 조작이나 실제 화면 검사로 기록하지 않는다. 수정 전에는 P36 Stage 축소와
Start Offset 변경 두 동작이 모두 기존 blend 경계 오류로 거절됐다. 수정 후51개 검사가 모두 통과했다.

P36의 first offset만0으로 옮긴 candidate에서 STAGE_1을4,997ms로 줄이면 Stage2는4,997ms에
시작하고 전체 길이는40,652→30,375ms가 된다. 다섯 blend의 길이와 stable pair는 유지되고
시작은 각10,277ms 앞당겨져14,767/16,850/19,648/20,984/28,310ms가 된다.
CPU Serialize/Parse/Validate 왕복도 통과했다. 음수 retime, 다중 pose 경계·중복,
다른 source pair, animation 내용을 자르는 길이, offset overflow의 거절과 기존 draft,
Dirty/generation 보존을 확인했다. 비활성 blend 박스는 원래 값으로 보존한다.

실제 Workbench TU 격리 Debug 컴파일과 `git diff --check`가 통과했다. 변경 파일은
KoukuSaydonActionWorkbench.cpp 한 개이며 UTF-8 BOM 없음/CRLF를 유지했다. 기존 다른 기능의
미커밋 변경을 보존했다. 검증 자료는 `out/StageBlendRetime20260914/result-before.log`,
`result-after.log`, `compile-workbench.log`, `run.ps1`이다. Product 링크/Client 화면 및
사용자 draft의 실제 Save는 이번 하위 작업에서 수행하지 않았다.



## G16. 저장한 Parent 연결 복구와23,698ms 패턴 배치

최종 저장 상태는 Gameplay revision558와 Sequence revision47이며, root의 화면 캡처 시간
수정이 먼저 Sequence revision48에 반영됐다. 두 Workbench의 Dirty=false와 저장 기준본/디스크
일치가 VM_READ 진단에서 확인되고 Client/Server가 종료된 뒤 최신 두 문서를 읽었다.

Sequence의 folder.2→P1, folder.3→P4에는 timelinePatternId가 있지만 P1/P4의 folderId는
없었다. 이 값은 이번 사용자 저장 전에 HEAD98d99eec/revision43부터 존재한다. 직전
160b7a2a/revision41에는 두 Parent 링크가 없었다. codec은 누락된 관계를 숨기지 않고 Parent를
quarantine하므로 Append에 `Parent timeline requires a same-folder, same-Gate Pattern`을 표시한다.
현재 Stage_ParentTimeline은 새 timeline의 folderId와 Parent의 timelinePatternId를 함께 설정하고,
Serialize3739행과 reader3141행은 이를 기록/복원한다. 신규 Parent fixture와 기존 두 Parent의
actual Serialize/Parse 왕복이 동일했으므로 검증을 완화하거나 새 복원 fallback을 넣지 않았다.
해당 두 owner의 누락된 folderId만 저장 문서에 복구했다.

Sequence P8 `1관문_연출`은 Gameplay P36 `세이튼_1관문연출`의 stable ID 변환 사본이다.
두 Pattern의 첫 Stage를15,274→4,997ms, 첫 walk offset을10,277→0ms로 수정하고 각5개
Animation Blend 시작을10,277ms 당겼다. 기존13개 clip, source 범위·속도·endPolicy와 다른
Stage 길이는 보존했다. Sequence P8에 사용자가 새로 저장한 presentation.1 Effect도 보존했다.

P4의 nextPatternOccurrenceOrdinal=2를 사용하여 `KAKULSAYDON_G1_PATTERN_4.pattern.2`를
추가했다. source는P8, start23,698ms, duration30,375ms, repeat=false이며54,073ms에 끝난다.
P4 전체61,662ms와 기존 Camera/Effect/Light/World 배치를 유지한다. 독립 P36과 실제 재생
owner인 Parent P4의 resetBossToSpawn을true로 설정했다. Child P8은false를 유지했다.
현재 child expansion은 child spawn reset을 명시적으로 거부하므로 새 임시 runtime을 만들지 않았다.
통합 Preview는 Parent 시작 시 `MN_RPCT_05 / boss.kakulsaydon.g1.saydon`의 저장 spawn을 적용하고,
23,698ms에 child walk를 시작한다. child 시작 순간의 별도 teleport는 추가하지 않았다.
World placement의 요청 좌표(6.43,1.3,730) 수정/게시와 Product Build는 root/focus가 담당한다.

검증된 candidate는 모든 변경 leaf 경로를 기록한 뒤 최신 source SHA를 두 차례 비교하는
CAS와 atomic replace로 적용했다. Gameplay558→559, Sequence48→49다. JSON 전체를 재직렬화하지
않고 대상 field의 text span만 바꿔 다른 사용자 변경을 보존했다. 실제 codec30개 검사 모두 통과:
두 Parent 오류 해소, 두 문서 Serialize/Parse 동일성, P36/P8 각5 blend, P4 실제 Expand의
actor/placement·첫 walk23,698ms·전체61,662ms, Parent presentation과 새 child Effect 보존을 확인했다.
`git diff --check`도 통과했다. 근거는 `saved-layout-receipt.json`, `saved-probe-result.log`,
`gameplay.saved-before.json`, `sequence.saved-before.json`이며 모두 `out/StageBlendRetime20260914`에 있다.

## G17. World 이펙트와 Camera Shot의 게시 검증 복구

최종 Composition 게시가 `source.templates[66].effectTracks[0]`의 `bone`, `followObject`를
unknown field로 거부했다. 실제 WorldSequenceDocument의 reader/serializer/validator와
Map publisher, WorldSequencePlayer는 이미 두 필드를 지원한다. Composition Python 검증만
이전 계약이어서, 이 검증에 strict boolean과 최대 256-byte UTF-8 bone 규칙 및 기존
`V1_EFFECT` resource kind를 반영했다. 유효하지 않은 UTF-8, 제어 문자, 잘못된 타입은 거부한다.

이어 Camera 문서는 실제 110개 Shot을 포함하지만 Composition 검증의 64개 제한에 걸렸다.
Arena의 `CAMERA_SHOT_MAX_COUNT`와 Map publisher는 이미 128개를 허용하므로 Python 상수를
128로 동기화했다. Camera keyframe 수나 다른 pose/timing 범위는 변경하지 않았다.
두 수정은 Python 검증만 바꾸며 사용자 World/Camera JSON과 C++ 파일은 수정하지 않았다.

실행한 자동 검증은 다음과 같다.

- WorldSequenceEffectContractTests 7개 및 CameraShotOptionalContractTests 4개, 총 11/11 PASS.
- 실제 World Sequence 문서 전체와 Camera Shot 110개 전체 검증 PASS; 입력 deep equality 유지.
- Camera 128개 허용, 129개 거부; 기존 optional field 부재, boolean 타입, bone UTF-8 byte
  경계 및 잘못된 값 거부 PASS.
- 두 Python 파일의 `py_compile`, 해당 파일에 대한 `git diff --check` PASS.

이 변경 이후 root의 최종 Composition publish도 PASS했다. 생성된 sourceManifestId는
`5b64db44e9a484f4e4021802c1613ed9416ebf723d8244c7b81a91ab0ac47da7`이다.
이 G에서 Client/UI 실행이나 화면 검증을 수행하지 않았다.

## G18. 최종 게시·빌드와 사용자 확인 경계

G14~G17 반영 후 Gameplay revision559와 Sequence revision49를 확인했다. KoukuSaydon owner의 product/map/world/gameplay.balance 게시가 모두 통과했다. Composition 게시도 최종 통과했으며 sourceManifestId는 5b64db44e9a484f4e4021802c1613ed9416ebf723d8244c7b81a91ab0ac47da7다. 초기 게시 실패는 G17의 뒤처진 validator 계약으로 구분하고 재시도 성공 로그를 남겼다. 후속 Python/data 수정으로 C++를 다시 빌드하지 않았다.

사용자가 지정한 saydon placement position [6.43,1.3,730]은 World 정본과 게시된 viewer world에 동일하게 들어갔다. 원래 yaw237과 나머지 World bytes는 보존했다. 사용자 Map lights122개는 정본과 게시본이 완전히 같으며 Map lights/RenderingProfiles 원본 SHA도 수정 전과 동일하다. P8에 새로 저장한 Effect와 다른 Parent 배치도 유지했다. 통합 재생은 Parent 시작 시 지정 spawn을 적용하고23,698ms에 P8 walk를 시작한다. 별도 child teleport를 추가하지 않았다.

정식 Debug Product Build는 Engine/Shared/Server/Client 모두 PASS, 총71,049ms였다. OBJ61/PCH0/CSO0/binary1을 기록했다. Client.exe는2026-09-14 14:01:40 KST 설치본이다. Engine.dll은 G13의13:12:02 설치본을 계속 사용하고 Server.exe는12:10:31 기존 최신 소스 빌드 결과를 유지한다. 기존 C4819 경고 외 오류는 없었다. Product 결과는 out/BuildPipeline/runs/20260914T050141062Z-debug-product.json, 상세 로그는 out/PortalActualCapture20260914/g14-product-build.log다.

기능 검증은 renderer의 실제 frozen SRV/시간 검사, Stage51조건, 저장·확장30조건, publisher 계약11검사, 변경 JSON parse, user 저장 보존 및 git diff --check를 통과했다. out/PortalActualCapture20260914/g14-final-data-receipt.json과 두 publish 로그에 실제 반영 상태를 기록했다. XML/프로젝트/HLSL은 이번 G14~G17에서 수정하지 않았다. 무관한 미커밋 변경과 빌드 산출물을 stage/commit하지 않았다.

현재 Client/Server는 종료 상태다. 실행 대상은 Server/Bin/Debug/Server.exe와 Client/Bin/Debug/Client.exe이며 Client 작업 디렉터리는 Client/Default다. 사용자가 Server를 먼저 실행한 뒤 Client의 시퀀서 →1관문_통합_시퀀스 →Complete Play에서19,819~21,010ms 축소 연결과23,698ms 세이튼 패턴을 확인한다. Client/UI 자동 실행·조작·촬영과 최종 visual PASS는 수행하지 않았다. 추가 게시나 Product 빌드 대기는 없다.

## G19. 통합 연출의 Parent 접근 확인

사용자는 Sequences by Gate의 통합 연출이 없어졌다고 보고했다가, 첨부한 화면에서 `1관문_통합_시퀀스 [Parent]`를 확인하고 "아 쏘리 확인했어"라고 정정했다. 첨부 이미지를 열람했으며 같은 Parent 행이 보이고 현재 선택된 하위 leaf는 `1관문_연출 [Saydon]`이다.

읽기 전용 진단의 현재 Client PID14620은14:01 Product와 matching PDB를 사용한다. Sequence rev49는 clean이며 P4/folder3가 정상 로드되고 load error는 비어 있다. GATE1이 선택돼 있다. G16의 folderId 복구 때문에 기존 root leaf는 제외됐고 동일한 owner는 Parent 행 클릭으로 계속 접근된다. 이 경로에는 발탄 패턴 검증 호출이 없다.

Parent 안에 중복 owner 선택 항목을 넣는14줄 임시 변경을 준비했으나 사용자 확인에 따라 그 변경만 되돌렸다. 추가 데이터 편집·publish·Product Build·UI 실행은 하지 않았고 기존14:01 설치본을 유지한다. 데이터 검증 성공과 사용자가 익숙한 목록 접근 위치를 구분해 보고해야 한다. 근거는 out/PortalActualCapture20260914/tree-live-14620-receipt.json과 사용자가 첨부한 화면이다. 이는 목록 접근 확인이며 capture/popup의 최종 화면 판정은 아니다.

## G20. 중앙 걷기 candidate와 미저장 편집 보존

최신 저장52/561 기준 candidate는 실제 codec/Parent 확장/이동 sampler66검사를 통과했다. P8/P36은0~4997ms에9.947406697m,1.990675745m/s로 이동하고 P4에서는23698~28695ms다. ServerNavigation의21표본도 통과했다. 뒤이어 Gameplay의 새 dirty 편집이 확인되어 방금 CAS 적용한53/562만 정확한 이전52/561 bytes로 원복했다. 현재 이동 값은 검증 candidate에 보관 중이며 원본에 적용했다고 기록하지 않는다. 사용자 Save 이후 새 기준으로 G21과 함께 검증·적용해야 한다.

## G21. 캡처 경계 CPU와 정확한 sample 수정 — 데이터 적용 대기

PresentationPlayer는 캡처 대기를 이유로 모든 V1 history를 무조건 재생하지 않는다. Service의 capture commit과 일반 external Update는 하나의 기존 seek/incremental 선택 함수를 공유한다. 최초 sample, 명시적인 배치 변경, 되감기,0.5초 초과 이동은 전체 seek를 유지한다. Playback은 고정 간격 사이의 정확한 마지막 표시 시각과 root/source anchor를 검증·반영한다. 캡처 대기의 첫 pass-through render, 다음 render의 capture permission, capture-ready 및120업데이트 제한은 유지한다.

실제31개 요소 포탈을 현재 codec과 수정 Playback으로 실행한 CPU157검사가 통과했다. 최초4.26초 seek는256 provider호출/50.3924ms였다. 같은 시각20회 old capture commit은5120호출/987.5846ms, 새 commit은20호출/0.1148ms였다.255개 fixed steps와22개 입자는 그대로다. 최초 seek 비용이 제거된 것은 아니며 실화면 전체 hitch가 없다는 증거로 확대하지 않는다.

4.250→4.260초 incremental frame/clock은 정확하고 capture ScreenPost1개가 나타났다. 전체 Seek와 particle signature가 같았고, 동일시각 root/source anchor 변경과 provider실패 시 이전 상태 보존도 통과했다. 실제 Service 함수 본문은 최소 forwarding object에 추출해 production Playback을 호출한 검사이며 전체 Service Update·실제 CEffectObject renderer·GPU 화면 검증은 아니다. 기존 MAP/BOSS anchor 회귀11검사도 수정 Playback/current codec으로 다시 링크하여 통과했다.

최종140ms 설계의 실제 capture resolver 추출 검사는 single/bundle294검사가 통과했다. fullportal은15559~19819ms, selected capture는15559~19959ms로 서로 다른 소유 수명을 사용한다. 첫 render·ready·동일handle·역방향 seek·종료·선택 element·실패 제한을 검사했으며 서비스 capture 상태는 fixture다.

P4 전용440ms fade candidate를 실제 V2 codec과 opacity evaluator로 검사한22개 항목은 통과했다.19819ms의투명→19959ms의완전검정→20259ms의투명을 순방향/역방향/저장왕복에서 확인했다. Engine의 실제 합성 순서는 ScreenPosts→Bloom→Final→DisplayOverlays→UI이므로 display-space fade가 캡처와 Bloom의 최종 출력에 함께 적용된다. 추가 shader 수정은 없다.

세 변경 CPP는 각각 최소 컴파일을 통과했다. 현재 Client/Server는 실행 중이며 신규 Product link, 원본 G20/G21 데이터 적용과 게시, 최종 사용자 화면 판정은 아직 수행하지 않았다. 신규 재질·책/맵 조명 요청은 별도 원본 연결 감사 중이다.

증거: out/PortalActualCapture20260914/history/final-history-receipt.json, history-result.json, anchor-regression-result.json; out/PortalSelectedCaptureClock20260914/result.log; out/PortalCaptureSmooth20260914/compile-player-receipt.json, fade-probe.run.log; out/PortalCaptureSplit20260914/candidate-140ms-transition/manifest.json. 뒤의 candidate는 sourceWritten=false다.

### G21-2. 최종 전용 capture140ms 후보

최종 capture asset은 `effect.kouku.gate1.authored.p4.scene-image-collapse`이며 원본에서 사용자 capture 요소 하나만 복사했다. 내부 delay0 외의 원본 lifetime17.811초, shrink4.130111694초, transform과 화면 설정을 보존했다. occurrence가19819~19959ms의140ms를 소유한다. 원본31개 포탈을 선택 표시만 하면 다른 입자도 계속 계산되므로 최종안은 전용1개 요소를 사용한다. 원본 포탈·전반context는19819ms에 끝나며,48개 후반요소는 원래 절대 시각으로 재생한다.

새 전용 capture와 late48 variant의 actual Product admission, GPU resource stage/commit, CPU frame 및140ms capture progress를 검사했다. 이는 화면 render/시각 판정이 아니다. 마지막 resolver 검사는 실제 후보에서 asset별 fixture를 만들어300검사를 통과했다. 새 Effect/후반 variant/V2 fade는 기존 catalog/tree 및96.DataFiles에 필요한 항목만 추가하는 후보로 준비했다. Source/mesh/texture의 원본 물리 자산은 변경하지 않는다.

visible=false인 원본 runtimeCarrier4개를 남기는 첫 latevariant는 기존 codec에 의해 거부됐다. 최종안은 전반72개 요소를 해당 전용 variant에서 제외하고 후반48개 요소 원문/ID/clock과 cross-reference closure를 보존하여 admission을 통과했다. 거부된 후보는 설치하지 않았다.

검사한 peak P4 예산은 기존 hard limit 안에 있다. 선택 표시 방식의 중복 포탈 예산도 초과하지 않았으나, 최종 전용 capture는 추가 particle/mesh 시뮬레이션과 예약을 없앤다. 기존 예산 제한을 완화하지 않았다.

최종 candidate 위치: `out\PortalCaptureSplit20260914\candidate-final576`. 준비 기준 Sequence revision52이며, 실제 fresh-save receipt의 Gameplay revision을 함께 보존했다. `sourceWritten=false` 상태이므로 아직 설치·게시·신규 Product 빌드 완료가 아니다.

## G22. 재질·조명 감사 결과

### 팝업북·맵 재질의 현재 연결 범위 재확인

사용자가 전달한 책·맵 이미지는 원작 참고 입력으로 열람했다. 이 이미지를 현재 Client의 결함 화면이나 visual PASS로 분류하지 않았다. 기존 09-12 G06-04/G06-05와 G10-3, 09-11 전체 맵 재질 결과, 렌더링 V2·TEAM ABI·Area material binding 계약을 먼저 대조했다. 원본을 이미 연결한 표면을 일괄 밝히거나 moving object에 F1 static RNM을 복사하는 수정은 하지 않았다.

`out/PopupMaterialAudit20260914/audit.py`가 World Sequence revision 1845의 original_8T6_00~04 바인딩을 실제 map placement/catalog와 조인하고, 설치 WModel의 **실제 submesh가 참조하는 material index**를 읽었다. material table의 사용되지 않는 빈 항목은 draw 슬롯으로 세지 않는다. 136 placement, 37 WModel, 실제 사용 40 model/slot, 41 asset/material 행의 원본 descriptor와 설치 runtime descriptor가 일치했다. 책의 별도 3개 slot binding까지 합친 93 texture role, 46개 실제 파일에 누락이 없었다. 이 검사는 현재 파일·바인딩 검사이며 새 GPU 또는 화면 검증은 아니다.

책은 `world.object.kouku.popup.book`의 `DEPLOY_CINE_KOUKU_BOOK.wmodel`을 사용한다. 설치 material section은 WMA2이고 실제 슬롯은 floor18, floor09c, floor17 세 개다. mesh/material/skeleton section은 기존 `Map/KakulSaydon/SourceSequences/kouku.gate1.full/Book/Book.wmodel`과 byte-identical이다. 세 mapMaterialBindings의 target slot과 source slot은 모두 일치한다. `WorldSequencePlayer_Objects.cpp`가 source의 native BG descriptor를 target slot 이름으로 복사하고 CModel 생성에 전달하며, baked/static-shadow 입력만 제거한다. diffuse override와 unlit은 이 책에 설정되지 않았다.

#### 실제 프로그램 소비 경계

이 BG 모델 경로는 Effect의 CVariant 상태를 사용하지 않는다. `MapAssetCatalog.cpp`의 엄격한 family parsing과 `MODEL_SURFACE_FAMILY`, `MapAssetRenderUtils::Bind_Material`의 `g_SurfaceProgram` 선택을 사용한다. 알 수 없는 family는 로드를 거절한다.

| 현재 입력 | 프로그램·소비자 | 확인 범위 |
|---|---|---|
| popup 39 asset/material 행, 책 3행 | SOURCE_BG_OPAQUE_MASKED=8 | Catalog가 flags/수치/선택 texture를 보존하고 CMaterial이 선택된 SRV를 필수 로드한다. Bind_Material이 D/N/S·reflection·UV·bump·specular·emissive 조건부 입력을 shader에 바인딩한다. |
| 승인된 floor08, floor08A 2행 | SPECULAR_TEXTURE_REFLECTION=1, DIFFUSE_SPECULAR_REFLECTION=2 | 기존 사용자 승인 F1 표면을 보존한다. 단순 LEGACY=0이 아니며 shader의 별도 source surface 계산을 사용한다. |
| animated book | WorldSequenceObject → MapAssetRenderUtils → AnimMesh pass 6 | `Shader_VtxAnimMeshBinary.hlsl`이 program8에서 `EvaluateMapSourceBGSurface`를 호출하고 diffuse/normal/specular/marker8을 Deferred로 전달한다. |
| static floor·curtain | MeshBinary/MapInstance source surface 분기 | 같은 MapMaterialSurface 계산과 source specular light 분기로 이어진다. |

책의 floor18/floor09c/floor17 flags는 각각 1157/141/133이다. 세 행 모두 normal·specular를 사용하고 normal strength는 1이다. floor09c는 별도 S texture를 사용하며 나머지 두 행은 원본 family의 D 기반 specular 분기를 사용한다. floor18의 D U/V mirror도 실제 sampler 분기에 연결된다. 세 행의 bump branch, reflection branch와 emissive는 현재 저작값에서 비활성이다. 해당 값이 0이라는 이유로 임의 bump, metalness, 반사나 Fresnel을 추가할 근거는 확보되지 않았다. shader는 BG의 선택된 bump/parallax, D/N/S, signed 2D reflection, subspecular/rim와 emissive flicker를 구현하며 Deferred가 선택된 specular cap과 rim light를 소비한다. 이 확인은 현재 저작 descriptor에 대한 소비 연결 확인이고, 원작 전체 shader의 모든 미선택 변형을 새로 전수 검증했다는 뜻은 아니다.

현재 여섯 F1 surface placement override(43/44, 45/46, 114/115)와 7 movable lights·6 material carriers도 기존 복원분으로 존재한다. material audit의 과거 공통 행은 변경되지 않았다. 이동 popup의 baked/RNM 행은 0이다. 이 범위에서는 새로 누락된 재질 입력을 특정하지 못했으며, .33/.34 및 popup WORLD/profile의 연출 시간 연결 변경과 별개로 재질 데이터를 다시 만들지 않았다.

`MATERIAL_RENDER_SETTINGS::bUseSourceMaterials`의 기본값은 true지만 F1의 `Recovered map materials (B)`가 false이면 Bind_Material이 명시적으로 program0을 선택한다. 별도 담당자의 matched-PDB VM_READ에서 Client PID14620의 `bUseSourceMaterials=true`, `eDebugView=0(FINAL)`, mapLightMultiplier=1이 확인되어 해당 스냅샷의 toggle OFF 가설은 배제했다(`out/PortalActualCapture20260914/material-settings-live-14620.json`). 실제 GPU draw의 program은 이번 파일·소스 검사에서 읽지 않았다. 또한 skinned BG의 COLOR0 입력은 현재 shader에서 1이며, 이 책의 원본 per-vertex alpha 재추출·동등성을 새로 증명하지 않았다. 따라서 결론은 **현재 필요한 slot/texture/선택된 shader 소비 연결에서 누락 미발견**이며, 전체 재질 복원 완료나 원작 화면 동등성 판정이 아니다.

검사 결과는 `material-audit.json`에 있다. Client/UI 실행·조작·캡처, 새 WModel/texture 생성, 원본·runtime 데이터 변경은 모두 0회다.


### 조명과 실제 실행 설정

저작/runtime MapLight122개 파일은 byte 동일하다. 기존121개 광원의 전체 필드는 HEAD와 같고, 사용자가 삭제한 .7만 빠지고 비활성 .8이 추가됐다. 원본 SCENE03A의5POINT/1SPOT/1DIRECTIONAL 움직이는7개 조명도 기존 검증된 복원 candidate와 byte 동일하다.32개 F1사본과14개 popup 제외 ID는 모두 존재한다. 신규 광원이 하드코딩 목록에 없다는 이유로 문서에서 버려지지 않는다.

현재 비활성 `월드_1관문스포트라이트` .1은 Gate1 provider의 기존 정책에서 사본만 강제로 켜지지만, HOLD popup provider는 원문 enabled를 보존한다. 이 기존 정책을 다른 provider에 확장할 원본 복원 근거가 없으므로 이번에 .1/.8을 강제 활성하지 않았다. 해당 정책의 도입 이력과 범위는 popup-light-coverage-receipt.json에 남겼다.

.33의 원래12258+8752=21010ms 연결은 후보11207+8752=19959ms로 옮긴다. .34재질 carrier와 book WORLD/camera/scene profile도 같은 시작을 소비한다. 기존 책3개slot, 맵40slot과7개light를 중복 재생하지 않는다.

matched-PDB VM_READ에서 Recovered map materials=true, FINAL debug view, MapLight multiplier1을 확인했다. 해당 시점의 rendering status는G3 dark였고 P4를 재생 중인 화면으로 판정하지 않았다. 이 값만으로 원작 화면 동등성을 승인하지 않는다. 실제 fresh-save snapshot은 Sequence52/Gameplay576 모두clean, 기준본문과디스크일치, read전후generation/hash안정이었다.

증거: out/PopupMaterialAudit20260914/material-audit.json, out/PortalActualCapture20260914/popup-light-coverage-receipt.json, material-settings-live-14620.json, combined-after-save-receipt.json. 새 재질·광원 원본 값 변경은 없으며 시간 연결 후보만 준비했다. 최종 사용자 화면 확인과 실행 파일 교체는 남아 있다.


## G23. UV 잔류와 상시 그네의 실제 원인

이 항목의 검사 시점에는 이전 G21 authoring 후보를 아직 적용하지 않았다. 새 Client PID36424는15:17:03 실행 파일을15:23:14에 시작했고, 실제 저장본은Sequence54/Gameplay578에서 추가Save579로 진행했다. EXE가 새로 빌드됐다는 사실과 Effect/Composition 후보가 실제 반영됐다는 사실은 별개다.

### UV 왜곡의 소유자와 종료

사용자의20067ms 잔류는 종료된 capture SRV만으로 설명하지 않았다. Sequence54에서 P4.presentation.36의 full portal은15559~20080ms이며, 별도P4.presentation.20 full context는0~58810ms다. 후자의 native2461 메시 `kouku.action.b12102906388704d1f78f823`, `kouku.action.0459601aef34749d73a8d4d7`가 실제 왜곡을 소유한다. shader2432 dispatch도2461의 Distortion 함수를 실행한다.

실제 Product admission/Playback에서20067/20081ms에두메시162+37개,20500ms110+25개,21010ms43+13개를 확인했다.21913ms에야0+0이다. 따라서 portal 박스가끝난 뒤에도 살아 있는context가 매프레임 다시왜곡하는것이 직접원인이다. Engine의 frame ScreenPosts/provider 목록은매프레임clear되고, owning row종료는Stop_WorldRoot로pending/active를제거한다. SRV잔류를막겠다고공용render상태를추가clear하거나Scene Profile암전으로가리지않았다.

G21 partition의 early owner는19819ms에종료되고 late48실제Frame에는동일시각의두메시가모두0개다. 전용capture19819~19959ms, 동시fadeout140ms, popup19959ms시작/fadein300ms후보로기존20080~21010ms의930ms공백을제거한다. 후반요소의원래source clock은유지한다. 검사로그는out/PopupRedPlane20260914/late-flash/result.log이며811지연검증까지183/183 PASS다. GPU resource stage와CPUframe검사이며실제Client draw/사용자화면PASS는아니다.

### 그네 가시성

숨겨진그네2개는SL04 static placement16604805168055844864/16988743889141117816의DECO02B다. MapPlacementRuntime::Sample_SelfMotions가batch를저작record visible=true로재구성해초기/Sequence owner의숨김을되돌렸다. 현재runtime visibility를sampled.visible에보존하는4줄을수정했다. 같은실제함수검사는이전130중33실패에서수정후130/130 PASS이며해당TU컴파일도통과했다. 원본배치/모션삭제는없다.

중앙의세이튼형상은첨부시점의owner를확정하지못했다. 후속VM_READ에서Deploy5/7은DESPAWNED, Saydon replicatedentity는없었으나그전에사용자가Debug boss Despawn을수행한로그가있었다. 이후다른read에서는사용자가선택한G3LIGHT resourcepreview의정상actor가생겼다. 따라서후속snapshot의부재를첨부시점의부재로일반화하거나정상Pattern/Resourcepreview배우를삭제하지않았다. popup중심±35m의실제static614배치에서shown은그네2개뿐이었다. 현재UI와process memory를수정하지않았다.

### 붉은평면과스포트라이트

붉은면은curtain848이아니라actor811 plan02/scene_a.mat.white_t/native3617의후반섬광평면이다. op0.3과붉은첫color키가pre-roll에서도평가됐다. 원본flash이동시점까지811만활성을늦추는후보가실제codec/frame/materialpacket검사를통과했다. 나머지5개carrier와후반flash를보존한다.

별도P4스포트라이트프로필후보는기존spider-blackout을보존하고새id scene.kakulsaydon.g1.spotlight-stage.v1을추가한다. 기존exposure0.1과fog밀도8은조명까지어둡게하므로신규profile에서노출배수1/fog disabled로바꾼다. 방향광/ambient0과맵배경조명배수0은유지한다. 저작LIGHT lane은Collect_FrameLights의별도provider이므로map배경배수와무관하게원래15밝기값을제출한다. 정상Rendering publisher Validate PASS, source40→candidate41이다. 현재authoring에는미적용이다.

### 책 재질 원본 재조사

이전G22의파일/바인딩누락미발견은전체복원완료판정이아니다. 원본SCENE03Acomponent2290과실제book mesh/MIC staticparameter를다시해독했다. floor18/floor09c/floor17은원본책의실제슬롯이며21개선택분기가현재값과일치한다. bump OFF는원본명시override다. sourceDynamicLightEnvironment51도disabled여서해당component SH누락으로책차이를단정하지않는다.

애니메이션COLOR0미전달은현재runtime지원공백이지만원본book nativecolorbuffer를완전히해독하지못해이책에실제손실이있는지는미확인이다. 기존사용자승인6개F1표면치환과SCENE03A원문차이를기록했고자동되돌리지않았다. 원본SH/contrast/shadowedSH등장면환경전체는여전히동등성복원완료가아니다. SourceMaterials존재만으로원작과같아야한다고설명하지않는다. 새밝기/반사/가짜vertexcolor는추가하지않았다. 증거는out/PopupMaterialSourceReaudit20260914/book-material-reaudit-receipt.json이다.

최종저작적용·게시·Product빌드와사용자화면확인은남아있다. 현재실행중Client/Server의추가편집Save와종료를요청했으며그동안검증된최신데이터후보를준비한다.

### G23 최종 통합 후보 상태

`out/PortalCaptureSplit20260914/candidate-seq54-game579-residual/manifest.json`은 G20 이동, G21 capture/context/fade, 811 전반 노출, P4 전용 spotlight profile을 합친 검토 후보다. 실제 codec/Product/resource stage/CPU 검사 110개, 등록 및 기존 저장 보존 64개를 통과했다. UV/811의 별도 183개 검사와 payload 일치도 `independent-evidence.json`에서 확인했다. authoring은 아직 쓰지 않았고 readyFromFreshSave=false다.

15:57:34 KST에 Client36424/Server59268은 계속 실행 중이었다. Gameplay는 이후 Save로581이 됐으므로579 후보를 그대로 적용하면 안 된다. Sequence54와 Rendering40은 유지됐다. 사용자의 Save·종료 뒤 최신 기준을 한 번 확보하고 `READY_AFTER_SHUTDOWN.ps1.txt`의 준비·검사·CAS 적용·게시·정상 Product 순서를 진행한다. 검증된 후보를 실제 설치 완료로 기록하지 않는다.


## G24. 포탈 종료 후 왜곡 재진단과 최신 저장본 적용

조사 시작 시 G21/G23 후보는 실제 데이터에 반영되지 않았다. 기존 manifest는 `sourceWritten=false`, `readyFromFreshSave=false`였으며, 전용 capture와 late-context asset 두 파일도 `Data/Effects/Authored`에 없었다. 당시 Client EXE와 link tlog는 16:54:34였고 해당 링크에 PresentationPlayer, EffectPresentationService, EffectPlayback, EffectDocumentRenderer_Rendering 객체 파일이 포함됐다. 실행 파일 갱신과 저작 데이터 적용을 구분했다.

최신 Sequence55는 Sequence54에서 `P4.presentation.20`의 “전후 쥐·금빛 연결” 행만 삭제한 상태였다. 따라서 G23의 20,067ms 왜곡 메시 **162+37=199개**는 삭제 전 full-context 소유자의 재현값이며, 최신 저장본에서 그 행이 활성이라는 뜻은 아니다. 최신 full-portal `.36`은 여전히 15,559~20,080ms였다. 실제 Product admission/Playback에서 20,067ms에 총 16입자 중 native2461 메시 2개와 capture ScreenPost 1개를 확인했다. 20,244ms에는 이 저장 행의 활성 구간을 벗어난다. 이를 삭제 전 context의 199개와 혼동하거나, 첨부 화면의 모든 왜곡을 이 값만으로 확정하지 않았다.

현재 코드의 `Resolve_PreviewCaptureClock`, `Seek_Preview`, `Stop_Session`과 Sample의 시간 판정·만료 행 정리를 추출한 검사 **310개**가 통과했다. 단일/Bundle 재생, 20,244ms 앞으로 이동, 정확한 종료점, 뒤로 이동, pending capture 종료를 포함한다. 이어 실제 `Stop_WorldRoot → Remove_At → CLayer::Remove_GameObject → Clear_Frame` 본문을 연결한 수명 검사 **32개**도 통과했다. 만료된 핸들의 pending/active 소유자를 제거하고 Layer에서 생산자를 삭제한다. 이미 제출된 동일 프레임의 참조는 프레임 정리 때 해제된다. 이 경로에서 지속적인 capture 잔류나 종료 호출 누락은 재현하지 못했다. 객체·provider는 명시한 수명 fixture를 사용했으며 전체 아레나나 GPU 화면 검사는 아니다.

삭제한 `.20`은 저장 원문 그대로 작업 사본에 복구한 뒤 같은 변경에서 early 소유 구간을 **0~19,819ms**로 제한했다. 후반 48요소는 기존 ID·값·source clock을 유지한 별도 late-context로 연결한다. full-portal은 **15,559~19,819ms**, 전용 1요소 capture는 **19,819~19,959ms**를 소유하고 popup은 **19,959ms**부터 시작한다. capture 사본은 내부 delay만 0으로 바꾸고 기존 lifetime·shrink·transform을 보존하며, 기존 consumer가 140ms 박스 종료를 사용한다. popup fade-in은 300ms다. 이 수정은 금빛 쥐 행 전체를 제거하여 UV를 숨기는 방식으로 설치하지 않았다.

최신 저장본 기반 후보는 실제 Composition/V2 codec, Product admission, GPU 리소스 준비, CPU capture 진행률 검사 **110개**와 등록·기존 저장 보존 검사 **64개**를 통과했다. Gameplay는 제작 시점의 **609→610**이며 P36 이동 연결 이외의 화염 등 저장 변경을 보존했다. 후보 폴더명의 `game606`은 실제 revision 근거로 사용하지 않는다.

부모 작업에서 Client/Server 종료 상태와 현재 파일 SHA256을 확인한 CAS로 **13파일을 적용**했다. 적용 결과는 **Sequence56 / Gameplay610 / Rendering41 / World8795**다. 위 split/capture와 함께 811 전반 노출 지연, 전용 spotlight profile, 중앙 이동 연결 및 별도 검증된 legacy trigger 비활성화를 포함한다. 이 기록은 저작 파일 설치 완료이며, 후속 게시·Product 빌드 결과와 사용자 화면 판정은 별도로 기록한다. Client/UI 실행·조작·캡처와 visual PASS는 수행하지 않았다.

근거: `out/PortalCurrentResidual20260914/final-diagnosis.json`, `native_result.log`, `result.log`(310), `service-chain-result.log`(32), `candidate-seq55-game606/verification-receipt.json`(110), `candidate-seq55-game606/registration-result.json`(64), `installed-seq56-game610/receipt.json`(13파일 설치). `final-diagnosis.json`의 baseline CAS 불일치는 부모 설치 뒤 읽은 값이므로 설치 전 freshness 실패로 해석하지 않으며, 설치 시점 근거는 마지막 receipt를 따른다.


### G24-01. 실제 설치·게시·빌드와 입력창 assertion 수정

`install_reviewed.py`는 검증된 candidate manifest/registration/native receipt SHA를 확인한 뒤 기존 `prepare_flame_unification.apply`의 종료 process 확인·before SHA·원자 교체·rollback 계약을 재사용했다. 별도 VM_READ clean-save 결과를 만들지 않았다. 설치 뒤13개 파일 after SHA가 전부 일치했다. Rendering41 공식 Publish, KoukuSaydon owner의4개 domain(현재610), Composition Publish를 통과했고 sourceManifestId는54d8ef465fa62a7a22ccf423239eaa001055316895faecfd2c6b8d87bb2b70a7이다. Patternbindings/Encounter sourceRevision610과 실제 Rendering runtime41도 확인했다.

Product Debug 전체 빌드 `20260914T083959618Z-debug-product.json`이 통과했다. Group Center·감전빔 목적지·World Position/Yaw 네 숫자 입력에서 금지된 EnterReturnsTrue를 제거했다. 현재 bundled ImGui의InputScalar는 해당 flag를assert하므로 Open Editor에서 아무 숫자도 바꾸기 전에 종료됐다. 현재값 변경 반환으로 기존 stage/commit을 사용하고 Save Changes 저장 경계, 개별 offset, dirty Detail 보존은 유지했다. 문자열 InputText의 Enter 동작은 유지한다. 독립 코드 리뷰도 통과했다.

별도 Deploy5 팝업북 생성·파티강제이동 함수를 제거하고 unowned original_kouku PLAY/REPLAY를 Server/Client에서 정상 Pattern 중지·이동 전에 거절하도록 연결했다. 기존 WORLD 원본 맵·책 자료와 Pattern run epoch는 보존했다. 실제 Server `--world-playback-contract-test`는0실패다. 첫 `--debug-teleport-contract-test`에서는4개 신규 assertion이 재진입 가능한 Mario lane 상태를0이라고 잘못 가정했다. 이를 거절 직전 실제 stage/form과 비교하도록 교정하고 재빌드 `20260914T084407274Z-debug-product.json` 및 같은 전체 검사0실패를 확인했다. 런타임 동작을 테스트 기대에 맞추기 위해 바꾸지 않았다.

빌드·게시 로그는 `out/PortalCurrentResidual20260914/{product-build-final.log,rendering-publish.log,kouku-publish610.log,composition-publish.log,world-playback-contract.log,debug-teleport-contract-final.log}`와 `installed-validation.json`에 남겼다. Client/Server 제품을 사용자 대신 실행하지 않았으며 위 Server 실행은 즉시 종료하는 계약 검사 모드다.

### G24-02. 원본 책·맵 재질과 조명 누락 재조사

136개 배치·37개 모델에서 선택한 texture role 93개와 파일 46개를 다시 대조했다. 선택 리소스 누락과 source/runtime descriptor 차이는 없었다. 원본 책의 native GPU LOD는 1,746정점(stride 36, UV 2개)이며, 뒤의 ExtraInfluence 0·adjacency 0·bone NameMap 13개까지 원문 offset으로 해독했다. 별도 COLOR0 버퍼나 source bHasVertexColors가 없어, G23에서 남긴 일반적인 animated COLOR0 지원 공백을 이 책의 실제 손실이라고 볼 근거는 없다. 승인된 F1 재질 floor08b/floor09b의 MIC scalar 12개도 원문과 1e-6 이내로 일치했다.

확정된 표시 오류는 미적용 actor811의 전반 노출과 P4 암전 profile의 exposure 0.1·fog 8이다. 현재 actor811은 원본 섬광 시점까지 숨기고 P4만 전용 exposure 1·fog off를 사용한다. 기존 MapLight 122개, 다른 profile, 사용자가 승인한 재질 치환값은 유지했다. 원본 공간 SH/contrast·scene reflection capture와 전체 장면의 원작 동등성은 여전히 입증하지 않았으며, 텍스처 존재 검사만으로 완전 복원을 주장하지 않는다. 근거는 `out/PopupMaterialMissingInputs20260914/deep-material-validation.json`과 `installation.json`이다.


## G25. 사용자 시퀀스 확인과 세이튼 반시계90도

사용자가 시퀀스가 깔끔하게 복원됐다고 직접 확인한 뒤 세이튼만반시계90도를요청했다. 마지막Save/Client·Server종료후 Sequence57→58와Gameplay610→611에서P8/P36의bossMotion.yawDegrees만 -40.801213684→-130.801213684로바꾸고각revision을올렸다. 사용자P4의resetBossToSpawn=false를포함해다른값·이동경로·clock은동일하다. `out/KoukuSequenceYaw20260914/receipt.json`은설치true이며KoukuSaydon owner611와Composition publish가통과했다. 그뒤별도요청4이펙트추가로Gameplay612가됐지만P36회전은보존했다. 기존시퀀스완료의육안확인은사용자발언에근거하며새90도방향의시각판정까지대신PASS처리하지않았다.
