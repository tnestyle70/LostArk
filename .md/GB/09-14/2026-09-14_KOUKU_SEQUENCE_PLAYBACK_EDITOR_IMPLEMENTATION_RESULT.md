# 쿠크 Sequence 재생과 Object 편집 구현 결과


## G44. PR #393 Composition·Sequence 병합 검증 — 2026-09-16

우리560741ac와 이전 작업자의 #392가 반영된 main af056661을 공통 기준c65b2cf4로 병합했다. Gate1 Composition은 우리72패턴과392직접 애니메이션 및 기존 WORLD40/Presentation186/Logic72/Summon7/Scene2/folder19/bundle11/flow3을 구조적으로 그대로 보존했다. 상대 신규 컷신5개는 P63~67→P73~77, 충돌 WORLD23→27, Presentation64/65→67/68로 옮겼다. 내부 action/occurrence·리소스 참조를 함께 재발급해 총77패턴/58WORLD/234Presentation, revision1052가 됐다.

실제 추가 이름은 `2관문_진입컷씬`, `2관문클리어_3관문진입`, `빙고_최종엔딩씬`, `쇼타임_연출`, `카드미로연출`이다. 처음 두 패턴은 각각 WORLD31/3과 연결된 animationTracks11/3, CAMERA7/18을 가진다. 카드미로는 WORLD1의 애니메이션과 CAMERA7을 가진다. 빙고엔딩과 쇼타임은 상대 원문 그대로 camera-only이며 새 배우 애니메이션을 추가했다고 기록하지 않는다. 신규5패턴과 WORLD18/Presentation48의 내용은 지정 ID치환 외 상대 원문과 동일하다. DRAFT/Product 상태와 기존 Flow는 바꾸지 않았다.

Sequence는 이전 작업자의 카메라·컷신을 기준으로 rev63/9패턴/37Logic/150Presentation으로 병합했다. 상대 P3 수정·P9 빙고엔딩을 그대로 유지하고 겹치지 않는 우리 플레이어 도착 트리거4개도 보존했다. WorldSequences는 상대 HandBook·촛대1/2 원문, 우리 바주카·즉사톱날 및 나머지 개선을 모두 유지한 rev2032/449objectResources/253templates/309instances다. 기존 숫자 원문을 행 단위로 보존했으며 모든 양쪽 stable ID와 수정 행을 대조했다.

검증 완료: 변경 C++4개(MainApp, KoukuSaydonActionWorkbench, Level_KakulSaydonArena, Level_KakulSaydonArena_WorldObjects)를 병합된 Client/Shared 헤더와 기존 EngineSDK로 격리 Debug `/c` 컴파일해 모두 exit0/OBJ생성을 확인했다. 기존 헤더의 C4828 경고는 남는다. C++/H6파일의 자동 병합 호출 흐름·서명도 대조했고 변경 JSON8개/XML2개 parse와 diff-check를 통과했다. WorldSequences 정식 Publish/Check는 exit0이며 source/runtime SHA256이 `5111f45e6262e3fc8031f8833a9c7662f02360d0997edb7d9de2d5c2c655c8e7`로 같다. 새 패턴 WORLD→instance→template→animation/model, CAMERA→source114샷 참조 누락은0개다.

구분할 기존 검사 한계: generic `composition_pipeline.py validate`는 병합 전 우리560741ac에 이미 있던 colliderTracks를 unknown field로 거부한다. 실제 Workbench는 두 Composition 정본을 직접 읽고 Product는 patternbindings, World runtime은 현재 codec/Map publisher를 사용한다. generic Boss/Sequence 출력은 현재 호출자가 없는 REFERENCE_ONLY/SHADOW이므로 이번 병합에서는 이 별도 검사기의 스키마를 확장하지 않았다. 초기 Kouku Product validate의 stale1051 출력은 revision1052 공식 publisher로 갱신할 대상이지 누락을 허용하는 근거가 아니다.

증거는 `out/Pr393Merge20260916/{gate1-preservation.json,sequence-merge-evidence.json,parse-validation.json,world-sequences-publish.log,world-sequences-check.log,compile/result.json}`에 있다. 기존10:34 Product 전체빌드 성공과 이번4TU 컴파일 성공은 별개다. 사용자가 Client/Server를 계속 사용 중이므로 이 병합 검증에서 제품 최종 링크·바이너리 교체나 UI 실행·캡처·화면 판정은 수행하지 않았다.

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

## G26. 일반 재생과 전투 연결 분리, 네 플레이어 도착 Logic

2026-09-15 사용자 요청을 `GB/Rendering-Restore`의 기존 변경 위에 적용했다. 일반 `Play Sequence`가 입장 metadata를 보고 Complete 실행으로 승격되던 분기를 제거했다. 이제 현재 cursor에서 연출을 재생하고 정상 종료 시 실제 replicated local player를 대상으로 저장된 맵 follow camera와 gameplay 입력을 복귀한다. `Complete Play`와 `Complete Play - Sequences + Pattern Flow`만 관문의 입장을 0ms부터 재생하고, Server gate 승인 뒤 시작 때와 같은 게시 revision의 Saved Pattern Flow를 실행한다. 실행 중 Complete에서 일반 Play로 바꾸면 이전 자동 전투 대기를 먼저 취소한다.

### 저장한 네 박스와 원본 근거

`Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json`을 revision 59→60으로 저장했다. P4 `1관문_통합_시퀀스`의 Logic lane에 `kakulsaydon.g1.logic.37`, 이름 `1관문_연출_플레이어 생성`, kind `TRIGGER / ROOM_PLAYER_ARRIVAL`을 등록하고 네 occurrence를 추가했다. definition은 동작 종류만, 각 occurrence의 optional `roomPlayerArrival`은 `playerSlot`과 `position`을 소유한다. 시간은 기존 occurrence의 `startMs`를 사용한다. Workbench Box Detail에서 Player slot, Arrival position, 기존 시간 항목을 편집하고 같은 Composition Save 경로로 저장한다.

| UI 슬롯 | 시작 시간 | Server 이동 좌표 X/Y/Z (m) | 대응 원본 emitter |
|---|---:|---|---|
| 1 | 42.253초 | -3.913588 / 1.317626 / 739.883125 | SCENE03A emitter_0 |
| 2 | 42.985초 | -3.290587 / 1.317626 / 742.070391 | SCENE03A emitter_2 |
| 3 | 43.331초 | -5.324063 / 1.317626 / 738.528281 | SCENE03A emitter_1 |
| 4 | 43.516초 | -1.156042 / 1.317626 / 742.512031 | SCENE03A emitter_3 |

시간과 XZ는 P4의 `presentation.41`이 소비하는 `effect.kouku.gate1.authored.portal-arrival.context.p4-late`의 실제 원본 네 그룹(총 48요소), World transform과 현재 박스 offset으로 계산했다. 이펙트 중심 Y 약 2.47~2.50m는 공중 불꽃 위치이므로 플레이어 높이에 복사하지 않았다. 실제 Server navigation의 같은 XZ 지면 높이를 샘플해 위 Y를 저장했다. 사용자가 세부 연출을 조정할 초기 배치이며 최종 화면 승인이 아니다. 추가 Logic/박스/revision 외 JSON 값은 기존 저장본과 deep-equal을 확인했다. 설치 CAS와 원본 ID·전체 정밀도는 `out/KoukuArrival20260915/data-install.receipt.json`에 있다.

### 실제 소비자와 실패 처리

- 재생 시작 시 immutable expanded Sequence에서 enabled arrival 박스를 읽는다. pause/scrub는 이동을 제출하지 않으며, 재생 시작 cursor보다 앞선 박스는 소급 실행하지 않는다. 시간 경계를 지난 박스만 실행당 한 번 제출한다.
- `CPlayerController::Request_KoukuRoomPlayerArrival → IPlayerCommandSink::Request_DebugWorldPlayback → Shared PLACE_ROOM_PLAYER → Server GameRoom`의 기존 typed 경계를 사용한다. Client Transform을 직접 변경하지 않는다.
- Server는 첫 도착 요청에서 현재 방의 연결된 PlayerId 순서 최대 네 명을 고정한다. 퇴장한 사람을 뒤의 새 플레이어로 대체하지 않는다. 없는 슬롯은 `SKIPPED_PLAYER`, 중복 occurrence는 `ALREADY_USED`, 이전 run epoch는 `STALE_REQUEST`다. 다른 방·잘못된 슬롯·위치·실패 상태는 기존 위치를 보존하며 거절한다.
- 기존 debug teleport의 navigation, 지면 오차, collision 및 player 상태 검사를 공통 함수로 추출해 재사용했다. 승인된 위치는 기존 Server reset/snapshot 경로로만 반영한다. 이 명령은 Sequence 저작용 Debug 경로이고 Release Server는 `DISABLED`를 반환한다. Gameplay publisher는 이 Sequence 전용 Logic을 거절한다.
- 도착 응답은 Sequence Viewer의 다른 request ID 필터보다 먼저 소비한다. 5초 응답 제한과 world generation 검사를 유지한다. 마지막 프레임에 응답이 남으면 완료 Pattern ID를 보존하고, 응답 완료 후 같은 ID로 한 번만 연출을 마친다. 실패·Stop·방 변경은 자동 전투를 중단한다.
- Complete에서 승인된 도착 위치가 있으면 gate 전환의 기본 플레이어 teleport를 생략해 네 위치를 보존한다. 보스 생성 승인과 실패 처리는 그대로 기다린다. arrival 없는 기존 gate 전환은 기본 teleport 승인도 계속 요구한다.
- Shared protocol을 86으로 올렸다. 대응 Server와 Client를 함께 빌드해 사용해야 한다. 새 production CPP 파일이나 프로젝트 등록은 추가하지 않았다.

### 실행한 검증

| 검사 | 실제 결과와 범위 |
|---|---|
| 실제 Server room/teleport 및 packet 검사 | 6,694 assertions, 0 실패. 1~4명과 다섯 번째 제외, 고정 roster, 이탈/빈 슬롯, 중복·epoch, 잘못된 world/높이, 실제 네 지점 navigation, 기존 collision/teleport 포함. 반복 Mario navigation 검사가 전체 개수에 포함된다. |
| 실제 revision 60 Composition codec | 25 checks PASS. Parse/Validate/immutable Expand/Serialize 왕복, out 사본 Save_Atomic/reopen, 한 박스 편집 시 다른 데이터 보존, 잘못된 save의 파일/LastGood 보존. |
| 실제 MainApp arrival 메서드 추출 검사 | 29 checks PASS. clock 경계·pause/scrub·느린 프레임·중복 방지, request/response, timeout, 새 run, stale 응답, world 변경, 잘못된 슬롯, ID 고갈, send 실패, 취소. 네트워크·아레나 의존성은 fixture다. |
| 실제 완료·카메라·gate 본문 8개 추출 검사 | 30 checks PASS. 마지막 ACK 지연과 완료 ID 보존, 일반 Play의 카메라/입력 복귀와 전투 미실행, Complete의 도착 위치 보존, boss ACK 전 Flow 차단, 기존 teleport/실패 계약, Complete→일반 Play 전환. 카메라와 전송 의존성은 fixture다. |
| 최소 컴파일 | 변경 Shared/Client codec/PlayerController, MainApp, MainApp_SequenceViewer, Workbench, Arena Debug TU와 변경 Server 두 TU의 Release 컴파일 PASS. MainApp_SequenceViewer는 프로젝트의 `/utf-8` 옵션으로 확인했다. |

기존 Python `test_typed_logic_definitions_follow_their_kind`는 현재 데이터 fixture가 kind를 제거하면서 judgement 값을 남기는 문제로 실패했고 HEAD projector에서도 동일 재현했다. 이번 Sequence 전용 publisher 거절 검사는 통과했다. 해당 무관한 fixture나 gameplay 데이터를 테스트 통과 목적으로 바꾸지 않았다.

근거는 `out/KoukuRoomArrival20260915/server-logic-result.md`, `server_result.log`, `codec_result.log`, `out/KoukuArrival20260915/dispatch_test.result.log`, 각 TU compile log, `out/KoukuSequenceCompletion20260915/validation.receipt.json`이다. 제품 전체 링크/실행과 Client/UI 조작·화면 캡처를 수행하지 않았다. 실제 다인 화면, 불꽃과 이동의 최종 타이밍, 카메라와 전투 전환의 육안 확인은 사용자 검증으로 남긴다.

### 제품 빌드 확인과 publish의 남은 경계

사용자가 실행한 Debug 빌드의 `Client/Default/x64/Debug/Client.log`(2026-09-15 14:11:08 갱신)는 오류 없이 Client.exe 출력과 shader/DLL 배포로 끝났다. 현재 소스에서 `Level_Loading`의 잘못된 lambda 내부 배치도 이미 수정되어 있었다. 이 파일의 해당 수정은 이번 root가 별도로 작성한 것이 아니다. 이후 Debug Client와 Server 프로세스가 실행 중임을 읽기 전용으로 확인했다. 에이전트가 Client를 실행하거나 UI를 조작하지 않았다.

Complete Flow의 source/product revision 정합은 별도다. 공식 KoukuSaydon owner publish를 시도했으나 작업 도중 다른 source closure가 바뀌어 rollback되었다. Gameplay 719 시도에서 네 domain의 실제 작업은 실행됐지만 마지막 gameplay.balance closure 일치 검사가 거절했고, 이전 제품과 Server 산출물이 복구됐다. 마지막 확인한 patternbindings는 revision 707이었다. 소스 저장이나 빌드 성공만으로 최신 Flow publish 완료라고 기록하지 않는다. 재시도는 현재 Composition revision과 모든 입력의 안정 상태를 다시 확인한 뒤 수행해야 한다.

중간 publish에서 기존 Client `ActorCatalog::ReadDefaultParticles`가 이미 지원하는 BossCatalog optional `defaultParticles`를 balance publisher만 거절하는 계약 불일치도 발견했다. `Publish-GameplayBalance.ps1`에 같은 6필드·개수·ID·bone UTF-8 길이·finite vector·범위·소비자 검사를 연결했다. 기존 필드와 unknown 필드 거절은 유지하고 Server bootstrap에 presentation payload를 새로 넣지 않는다. 실제 PowerShell validator를 AST로 추출한 `test_boss_default_particles_contract.py`의 정상/오류 20조건이 통과했다. 이 수정 후 해당 검증은 통과했지만 위의 동시 source 변경에 따른 publish 실패와 구분한다.

로그는 `out/KoukuArrival20260915/publish709-final.log`, `publish719.log`, `publish-recovery.json`에 있다. 카메라 파일 rollback의 공유 잠금 메시지도 확인했고 target과 백업 SHA256이 일치해 기존 내용 보존을 별도로 검사했다. publish 검사나 freshness를 해제하지 않았다.


## G27. ActionWorkbench Effect 그룹 중심 회전

기존 `Render_EffectGroupDetails`의 Group Center Offset 바로 다음에 `Group rotation (deg)` XYZ 입력을 추가했다. 여러 Effect occurrence의 평균 pivot을 기준으로 중심 상대 위치와 각 occurrence 방향에 같은 delta quaternion을 후곱한다. 회전 입력의 이전 값과 새 값을 quaternion으로 비교하므로 X/Y/Z를 차례로 바꿔도 Euler 성분 덧셈으로 상대 방향을 왜곡하지 않는다. 선택한 occurrence ID 집합이 바뀌면 회전 입력의 상대 기준을 0으로 시작한다. 개별 Effect V1 문서 내부의 그룹 편집창과 구분되는 ActionWorkbench occurrence 그룹 기능이다.

기존 그룹 확장·anchor 좌표계 검사와 geometry 범위를 통과한 전체 그룹만 `m_StagedPresentationGeometry`에 반영한다. 첫 preview 요청 전에 모든 멤버를 stage하고, preview가 불가해도 기존 Save 가능한 대기 상태를 유지한다. anchor, timing, stable ID, 비균일 Scale을 바꾸지 않으며 중심만 이동할 때 authored Euler 값도 그대로 둔다. 기존 Collider 편집과 Save/Commit_Candidate 구현은 수정하지 않았다. 새 production 파일이나 프로젝트 등록은 없다.

### 실행한 확인과 남은 경계

- 현재 소스의 실제 `Transform_SelectedEffects`, `Render_EffectGroupDetails`, geometry 검사 및 placement copy 함수 본문을 out fixture에 그대로 추출해 149 checks / 0 failures를 확인했다. 정사각형 90도 회전 방향, 고정 중심·모든 점 쌍의 거리, 비균일 Scale을 포함한 방향 basis, 복합 XYZ 입력·역회전, 이동·회전 동시 입력, 잘못된 입력의 전체 보존, preview 불가 시 저장 대기, 선택 변경 시 입력 기준 초기화를 포함한다. ImGui 입력, 변경하지 않은 선택/anchor collector 및 preview endpoint는 fixture이며 실제 Client/UI 실행 결과가 아니다.
- runtime과 같은 DirectX float quaternion 및 기존 SimpleMath Euler 변환을 사용한다. 일반 basis 비교 허용치는 2e-5, pitch ±90도 근처의 비균일 scale3 포함 basis 허용치는 1e-4다. 이 근처에서 측정한 최대 성분 오차는 4.0323e-5이며 정확한 Euler 숫자 일치를 주장하지 않는다. 첫 임시 2e-5 기준의 gimbal 비교 4개는 이 float 역변환 한계로 실패했고, 실제 basis를 비교하는 위 명시 범위로 확인했다.
- 실제 `KoukuSaydonActionWorkbench.cpp`의 현재 Debug TU를 out로 분리 컴파일했다. 오류 0이며, 기존 `Engine/Public/Level.h` 등의 인코딩 경고는 남아 있다. CPP/header의 UTF-8·CRLF와 변경 범위 `git diff --check`를 확인했다. 제품 링크·EXE 교체는 수행하지 않았다.

근거는 `out/KoukuEffectGroupRotation20260915/{source_extraction.json,probe.log,compile.log,source_status.json}`과 before/after scoped diff다. 실행 중인 Client, 사용자의 미저장 duration, Composition/Resources와 게시 산출물을 수정하지 않았다. 기존 포털 2문서는 여전히 미설치이며 Valtan 신규 full restore 제품 cue 0 정책을 유지한다. 이 검사 단계에서는 제품 빌드를 수행하지 않았으며, 실제 화면 회전의 최종 확인은 사용자에게 남긴다.


## G28-P. SHOWTIME_PLAYER_TARGETS 투영·게시 연결

Python projector가 새 DURATION 정의의 고정 그룹/추적 occurrence 참조를 같은 Pattern 안에서 resolve하고, template 원본 행만 Product static playback에서 제외한다. 저작 source 행과 사용자 배치를 삭제하지 않는다. 기존 resource/occurrence serializer를 공용으로 사용하며 resourceDurationMs, 상대 시간, 회전·Scale, fade/dissolve를 보존한다. XZ는 가장 이른 고정 표식 또는 추적 자신의 위치를 기준으로 빼고 Y는 원래 지면 기준 offset을 유지한다. 고정 marker의 -0.2m를 다른 요소에서 빼지 않는다.

정규화한 template 내부 ID와 내용 SHA256으로 `kouku.showtime.fixed.<hash>` / `kouku.showtime.tracking.<hash>`를 만들고 patternbindings root의 targetedCombatVisuals에서 중복을 제거한다. fixed는 loop=false, 원본 그룹의 마지막 종료까지 재생하며 tracking은 자기 원본 duration을 loop=true로 반복한다. Parent 반복은 참조된 fixed group과 tracking occurrence만 scope remap하고 원본 template를 잘라야 하는 child slot은 게시 전에 거절한다. 참조 미완료 Pattern은 저장 inventory에 이유와 함께 남고 정상 Pattern의 게시를 막지 않는다.

Gameplay publisher는 같은 revision Client template의 ID/archetype/duration/loop를 exact-join하고 11-field PATTERNSHOWTIMETARGETS를 생성한다. fixed/tracking 중 하나 또는 둘을 사용할 수 있으며 둘 다 없으면 거절한다. JSON의 빈 ID는 TSV에서 `-`, fixed가 없으면 lifetime은0이다. interval은1~600000ms, speed는0.01~10 범위다.

### 실제818 연결과 검증

- 사용자 저장817 전체62 Pattern을 바탕으로 logic64만 연결한 준비 검사에서 P35가 Product로 admit됐다. 이름만 있던 TRIGGER logic63/.logic6는 기존 정책대로 저작에 유지되고 runtime trigger로 투영되지 않으며 P35 admission을 막지 않는다. source 바이트는 이 검사에서 변경하지 않았다.
- root가 합친 candidate818 전체 검사도 Product51 Pattern, output2개로 통과했다. P35.logic.7은 start37685ms / duration16050ms, fixed group .636의 .632~.635 네 요소는4384ms, tracking .454는14740ms, spawn interval2000ms와 follow speed0.5다. 두 template hash와 전체 정밀도는 `out/KoukuShowtimeTargets20260915/P35.projection.receipt.json`에 있다.
- 신규 Python/실제 PowerShell 계약 검사8개가 통과했다. 원본 TRS/높이/시간 보존, static 소유 분리, content hash/dedup, 미완료 저작, Parent remap, 잘린 template 거절, 범위·참조·revision 오류와 11fields, World optional lane/unknown key 거절을 포함한다.
- 관련 기존 검사27개 중20개가 통과했고7개는 현재 저장 데이터의 초과시간 또는 dynamic FOLLOWUP_PATTERN child로 실패했다. HEAD projector를 별도 메모리 로드해 같은7개가 같은 이유로 실패하는 것을 확인했다. 해당 사용자 데이터나 기존 검사를 이번 기능 통과 목적으로 고치지 않았다. `regression.log`, `regression-head-baseline.log`가 근거다.
- 현재818의 새 SHOWTIME을 제외한61 Pattern을 HEAD와 비교해60개 presentation 출력이 바이트 단위로 같고1개 미완성 Parent의 거절 이유가 같았다. `ordinary-projection-parity.json`에 기록했다. Python/PowerShell parse와 변경 범위 diff-check를 통과했다.

### Owner 게시의 누락 소비자와 최종 데이터

첫 Owner 게시에서는 World publisher의 strict Pattern allowed-key가 showtimeTargets를 거절해 runtime 전체가 rollback됐다. `Publish-WorldGameplay.ps1::Get-EncounterProfiles`에 Kouku만 허용하는 optional 배열<=64를 추가했다. 상세 값·Client template 검증은 Gameplay publisher에 유지한다. 실제 Get-EncounterProfiles 전체 본문으로 Valtan/BossProfiles와 candidate818 Kouku 입력을 읽어 통과했고, 원래 필드가 없는 legacy 및 잘못된 신규 필드의 거절도 검사했다.

root의 최종 Owner 재게시가 exit0으로 끝난 뒤 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`1874행에서 P35.logic.7의 위 시간·두 hash ID·4384/2000/0.5가 실제 설치된 것을 읽기 전용으로 확인했다. 이는 schema34의 bootstrap이며1874는 행 번호다. source/생성물818, template2개와 Owner 설치는 root가 수행했고 이 Tools 담당 작업은 canonical JSON·EXE를 직접 교체하거나 추가 C++ 빌드를 하지 않았다. Client/UI 조작과 최종 시각 판정도 수행하지 않았다. 최종 실행 안내와 전체 Client/Server 확인은 root의 설치 결과를 따른다.

준비·게시 근거는 `out/KoukuShowtimeTargets20260915/{full-source-readiness.receipt.json,candidate818-readiness.receipt.json,projected818,candidate818.showtime.bootstrap.tsv,validate_actual818.ps1,validate_world818.ps1,validation_receipt.json}`이다.


## G29. Stage 축소 시 이펙트와 비애니메이션 시간 보존

사용자가 “이펙트 시간 그대로”를 명확히 지시했다. 이전 G29의 모든 lane splice·압축은 요구를 잘못 해석한 구현이므로 현재 정책에서 폐기했다. 이전 `out/KoukuStageResize20260915` 검사는 당시 구현의 동작 기록이며 현재 요구 충족 증거로 사용하지 않는다. 이미 저장된 사용자 시각의 복구·선택은 root가 최신 source를 기준으로 담당하며 이 하위 작업은 정본 JSON을 쓰지 않았다.

`Set_StageDuration`은 선택 Stage 길이 및 해당 animation의 startOffset/play/blendIn만 새 Stage 안으로 제한한다. sourceStart/sourceEnd/playRate는 유지한다. `Retime_StageLanes`를 제거하고 `Extend_PatternLifetimeForAuthoredLanes`로 Stage 합·기존 explicit duration·모든 저작 lane의 끝·BossMotion 끝의 최댓값만 Pattern 수명에 반영한다. Presentation/Logic/Summon/World/SceneProfile/Pattern occurrence 및 BossMotion의 시간·fade·TRS·재질·그룹·연결 ID는 변경하지 않는다. fixed ANIMATION_BLEND가 새 pose 경계와 충돌하면 기존 Validate가 candidate 전체를 거절하며 해당 Logic을 이동하지 않는다. UI Stage Duration과 Fit Stage 설명도 이 정책으로 교체했다.

현재 revision947은21 Stages,392 Presentation 행이다. 실제 setter 본문으로 Stage57의1618ms를448/1/2318ms로 변경하고 C++ codec/Preview expansion/Save 경로를 검사했다. 축소448/1에서는 source Stage 합57216/56769ms와 별개로 마지막 저작 행까지 lifetime58386ms를 사용한다. 확장2318에서는 합59086ms로 모든 행이 포함되어 새 explicit duration이 필요 없다. 모든 비애니메이션 구조 전체가 동일하며 다른 Stage와 animation source window도 동일하다.

실제 setter+codec probe는183 checks/0 failures다. P36 ANIMATION_BLEND 충돌,600000ms 상한과 실패 시 draft 보존, 원문 roundtrip, 실제 Save→Reload, 외부 파일이 바뀐 stale Save 거절을 포함한다. 새 fitEffectToDuration은947의 sector5개(.99/.648/.700/.716/.755)를 out 사본에서true로 만들고 Stage 편집·roundtrip·Preview·Save에서 보존했다. 숫자타입은 Pattern load error로 격리하고 Camera true는 Validate가 거절했다. source JSON은 변경하지 않았다.

Workbench와 CompositionDocument의 fit flag까지 반영된 TU를 out로 격리 컴파일해 오류0이다. 기존 Engine header C4828 경고는 유지한다. 이후 독립 담당의 BOSS_TRACK_TARGET kind 추가 최종 compile은 해당 담당 결과를 따른다. 새 production 파일·project 등록은 없다. 근거는 `out/KoukuStageTimingPreserve20260915/{probe.cpp,probe.log,compile.log,document-compile.log,stage57-448ms-preserved.candidate.json}`이다. Product 빌드·설치·Client/UI 실행과 최종 화면 판정은 수행하지 않았다.

## G30-P. 빈 마지막 Stage의 실제 pose hold 게시

`KoukuSaydonPresentationPlayer::Sample_BundlePreview`와 source-bone sampler는 다음 animation이 없는 동안 마지막 animation의 age를 playMs에서 clamp한다. 제품도 `PresentationAssetService::holdAtWindowEnd → ClientReplication → CNpc::Set_NetworkAnimationWindow/Try_SampleNetworkAnimationTicks`의 같은 기존 source sampler를 사용한다. 이 계약을 Python publisher에 연결했다.

`_trailing_pose_hold_source`는 explicit Parent가 아닌 leaf의 마지막 빈 Stage, 직전 단일 animation, 같은 stageKind, tail retarget 없음, 합치기 전후 같은30Hz Stage tick 합일 때만 admit한다. 원본 구조/ID/범위 검증은 유지한다. `_coalesce_trailing_pose_holds`는 파생 사본에서만 직전 Stage duration에 tail을 더하고 기존 단일 clip의 holdAtWindowEnd를 사용한다. 저작 Stage 삭제·새 clip·새 occurrence·JSON 계약을 추가하지 않았다. 내부 빈 Stage, 전부 빈 Pattern, 다른 Stage kind나 retarget, tick 합이 바뀌는 경우와 Parent의 기존 idle 정책은 변경하지 않았다.

실제844 쇼타임은 Stage12의3863ms와 빈 Stage52의2580ms를 파생 Stage12의6443ms로 합친다. action/animation ID와 `rpct00_att_battle_28_11`, sourceStart0, play2400, rate1, EXACT를 그대로 유지한다. holdAtWindowEnd=true이며 native root motion49 samples의2400ms와6443ms XYZ가 정확히 같고 그 사이 새로운 이동이 없다. 모든 Logic/Effect의 절대 시간과 template 원본 행은 유지한다.

- 새 tail 지원/거절 검사2개와 기존 ShowTime8개·기존 초기/끝 pose hold2개, 총12 tests가 통과했다. source 비변경, stage ID/animation 보존, root curve의 끝 위치 유지, 같은 targeted template, retarget/kind/내부공백/전부공백/다중clip/Parent/tick 변화 거절을 포함한다.
- Stage57 448ms 사본 전체 prepare_publication/project는51 Product, P35 admitted=true, output2로 통과했다. 고정4384ms/추적14740ms template2개의 hash와5개 controlled ID가 변하지 않았고 SHOWTIME Logic은 같은 Stage delta3361ms만 이동했다.
- root의 최종 합친 candidate845도 같은 전체 검사에 통과했다. 순간이동 `.logic.6`은54054/1000ms와[2.57,1.3,952.27], SHOWTIME `.logic.7`은61563/16050ms, interval2000·speed0.5, 고정/추적 hash2개가 실제 생성물에 들어갔다. `.presentation.638`의 사용자5290ms와 resource5645ms, 새 blue `.639`의9547/3301ms도 생성물에서 확인했다.

근거는 `out/KoukuStageResize20260915/{projector-tests.log,stage57-448.publication.receipt.json,combined845.publication.receipt.json,tail-consumer.receipt.json,projected-combined845}`다. 이 하위 작업은 Source/Resources/제품 생성물의 정본 설치와 제품 빌드·EXE 실행을 수행하지 않았다. 실제 설치·새 EXE 및 사용자 화면 확인 상태는 root의 최종 통합 기록을 따른다.

G30-P 검증 후 root가14개 준비 파일을 CAS 설치하고 Kouku Owner4-domain 게시를 완료했다. `out/KoukuShowtimePolish20260915/installed-verification.json`의 revision845, Product51/P35 ready, teleport25필드와showtime11필드, drop5290/blue9547·3301, sourceStage52/57 보존을 읽기 전용으로 확인했다. 제품 빌드·Client/UI 실행·새 육안 판정은 수행하지 않은 상태다.


### 현재 Stage 보존 정책의 explicit tail 검증

leaf의 explicit duration이 Stage 합보다 길면 C++ `Try_ExpandPatternDocument`의 Preview 사본에서 마지막 Stage만 연장한다. Python leaf expansion도 같은 source clip·그룹·lane을 보존한다. 마지막 빈 tail을 합치는 기존 조건은 계속 적용하되 explicit duration은 기존 fixedTimeline의 절대 경계를 소비하므로 implicit Stage별 ceil tick 합 검사를 요구하지 않는다. Parent의 기존 idle 정책은 유지한다.

current947은 자체적으로 새 name-only DURATION logic65가 있어 처음 Product50/P35 unavailable이었다. 담당자가 제공한 `BOSS_TRACK_TARGET` judgementKind만448ms 보존 후보에 결합해 실제 전체 prepare_publication/project를 재검사했다. Product51/P35 admitted와 outputs2를 확인했고 어떤 사용자 occurrence도 삭제하지 않았다.392 Presentation 행의 전체 source 구조와 target template가 동일하며 runtime387개와 controlled5개로 기존 분리된다. source Stage 합57216ms, Pattern lifetime58386ms, 마지막 Stage12의 source play2400ms를 유지하면서 파생 Stage만3570ms로 확장한다. 실제 native root49 samples의2400~3570ms XYZ는 정확히 동일하다.

빈 tail 지원/거절·explicit tail·Parent 기존 동작 focused7 tests PASS다. 근거는 `out/KoukuStageTimingPreserve20260915/{check_tail.py,tail-consumer.receipt.json,stage57-448ms-preserved-typed.candidate.json}`이며 이들은 out-only 검증 후보다. 정본 JSON·Products·EXE에 설치한 결과로 기록하지 않는다.


## G31. Preparing preview Effects 무한 대기와 준비 완료 문구


`Advance_LoadingProductCuePreparation`의 structural failure에서 owner를 먼저 해제하면 다음 priority enqueue가 실패 target A를 B 뒤로 이동시킬 수 있었다. worker 종료 뒤 strict front-only 실패 receipt가 거절되어 같은 revision의 모든 준비가 중단됐다. 실제 queue와 이전 consumer로 pending2/failed0/fatal latch를 재현했다. 현재 사용자 프로세스가 이 순서를 거쳤는지는 live 로그 증거가 없으므로 동일 증상과 확정 구현 결함을 구분한다.

runtime worker의 known target은 owner가 front를 고정한 상태에서 failure receipt를 먼저 commit하고 owner를 해제한다. start/Submit 실패도 같은 순서다. 중복 worker 종료는 첫 receipt를 유지하며 다른 target은 계속 준비된다. queue identity가 실제로 손상된 경우 strict 거절과 revision latch는 유지하고 원인 문자열을 별도로 보존한다. `EFFECT_PRODUCT_PREWARM_TARGET_PROBE::strBlockingFailure`는 같은 revision의 요청 중 pending이 남을 때만 전달한다. `Prepare_PreviewEffects`는 이를 기존 `Fail_Preview → Stop_Preview → Consume_FailedPreview → MainApp::rejectPreview`로 소비하므로 문서 변경 없이 프리뷰를 종료하고 원인을 표시한다.

기존 ready 분기가 이전 “Preparing preview Effects; timeline is held” 문구를 남기는 별도 결함도 수정했다. 준비 중 settled/전체 및 failed/unavailable 수를 표시하고 완료 시 held 접두 문구만 prepared 상태로 바꾼다. 이후 occurrence 오류는 덮지 않는다. 개별 failed/unavailable의 terminal 격리, 빈 target, Loading owner, catalog revision rebase, 취소와 pacing yield는 기존 계약을 유지한다. timeout이나 validation 우회는 없다.

검증은 `out/EffectPreviewPreparation20260915`에 이번 baseline과 함수 원문을 분리했다. 실제 queue/job 및 추출된 현재 failure/probe/Preview consumer의37검사 PASS: 이전 오류 재현, 실패 A 정산 후 B 준비, cancellation, 중복 receipt, prepared 보존, stale revision, pending clock 보존, 완료 문구 교체, 후속 오류 보존, scope별 failed/unavailable와 fatal 차단을 확인했다. Preview Fail 호출의 unit seam은 실제 GPU/UI 실행을 대신하지 않는다. Service/PresentationPlayer 두 TU의 isolated Debug 컴파일은 exit0이며 기존 include 인코딩 경고만 남았다. 세 C++ 파일 UTF-8/CRLF와 scoped diff 검사를 확인했다. 새 production 파일·project 등록은 없다.

이번 snapshot은 source855이며 과거 source708의13-target 감사와 혼용하지 않는다. 사용자 저작·JSON·Resources·실행 중 EXE는 이 작업에서 변경하지 않았으며 Product 재링크/배포와 실제 화면 확인은 미수행이다. 이미 실행된 구버전 EXE의 메모리 latch가 소스 편집만으로 해제되지는 않는다.


## G32-S. SHOWTIME 지정 타겟 회전을 Duration 동안 보간


`GameRoom_BossSimulation.cpp::Update_KoukuPlayerTargets`의 매 tick 즉시 yaw 대입을 현재 yaw에서 목표 yaw까지 shortest angular arc 보간으로 변경했다. 기존 server-owned target을 유지하고 target이 사망·이탈하면 기존 선택 경로를 사용한다. body yaw만 변경하며 플레이어별 장판/추적 visual의 MAP축과 이동속도, 큰 Saydon의-90도 모델 basis는 유지했다. C++ header, Logic schema, JSON, packet을 추가하지 않았다.

보간 비율은 `경과 tick / 남은 duration tick`이다. 첫 활성 tick은 한 tick을 소비하고 마지막 유효 tick은 비율1로 현재 목표에 도달한다. 정지 타겟은 전체 저작 duration에 걸쳐 일정 각속도로 향하며 움직이는 타겟은 현재 방향에서 남은 시간에 맞춰 계속 따라간다. 사용자가 원하는 duration 보간이므로 별도 임의 deg/sec 상수는 없다. 기존 fixed30Hz와 reserved-zero elapsed helper를 사용하고 같은 tick 및 역행 tick은 재소비하지 않는다. duration 종료에서는 기존 exclusive-end 경계가 회전을 중지한다.

기존 실제 Server room/CombatObject 소비자를 쓰는 SupportSurface 검사에12개 회전 사례를 추가했다. 전체 79검사/실패0: 첫3도→500ms45도→마지막90도, duration 두 배의 초기1.5도, ±180도 shortest arc, 개별/묶음 tick 동일, 동일·과거tick 보존, 움직이는 지정 타겟과 BigSaydon basis, duration 밖 보존을 확인했다. 기존 고정/추적 lifecycle, 사망 재선택, XZ teleport와 catalog roundtrip 검증도 유지했다. 근거는 `out/KoukuShowtimeYaw20260915/result.log` 및 `validation.receipt.json`이다.

변경된 BossSimulation/기존 test 두 TU의 isolated Debug 컴파일·격리 executable 링크·실행은 모두 exit0이다. 나머지 unchanged room 객체는 직전 SupportSurface 검증 산출물을 재사용했고 ABI 변경은 없다. 두 파일 UTF-8/CRLF와 scoped diff 검사를 확인했다. Product EXE 재빌드·재시작·배포·UI 조작, 사용자 source947 및 이후 저작 변경은 하지 않았다. 실행 중인 제품에 적용됐다는 주장과 최종 화면 판정은 하지 않는다.


## G33. 사용자 logic65 추적회전의 저장·게시·Server 연결


최신 사용자947의 `쿠크세이튼_쇼타임_추적회전`은 name-only DURATION logic65이며, 기존 logic64 SHOWTIME_PLAYER_TARGETS와 다른 정의였다. G32의 보간 구현은 최종적으로 새 `BOSS_TRACK_TARGET`에만 연결했다. 기존 logic64 장판 추적은 직전 즉시 facing/visual 동작으로 유지한다. G32 결과만 읽고 logic64가 사용자 새 회전을 대체한다고 해석하지 않는다.

Client 공용 judgement 목록에 새 kind를 추가하고 같은 목록을 쓰는 Workbench에서 선택할 수 있게 했다. 추가 field는 없으며 공용 outcome/collider helper가 두 항목을 받지 않는다. 기존 Composition codec의 kind/value 검증·serialize와 Save, Apply_Draft는 이 정의를 그대로 소비한다. 기존 Workbench Apply에서 연결된 결과를 정리하는 typed 전환 범위에 회전 전용도 포함했고 Stage/fit 코드는 수정하지 않았다. DURATION 설명은 지정 타겟·최단 회전·Duration을 명시한다.

Python projector는 회전 전용을 judgement 결과 창에서 제외하고 기존 `mechanicTriggers` 배열에 원래 start/duration으로 투영한다. PowerShell은 `PATTERNMECHANICTRIGGER`의 기존25필드로 새 kind를 쓰고 unrelated teleport/HUD/visual/clone/spawn 값은 거절한다. World publisher는 기존 mechanicTriggers top-level 계약을 소비하므로 새키나 우회가 없다. Server Catalog/Brain이 typed kind를 검증하고 LogicRuntime이 기존 PlayerTargetWindows duration clock을 만든다. GameRoom은 같은 Server target 선택을 재사용한 뒤 G32 남은duration shortest arc 보간을 적용하며 visual 생성 전에 끝낸다. 별도 모델·Effect template·CombatObject·packet은 없다.

source947 out 사본의 logic65 judgementKind 한 field만 바꿨고 전체 나머지 데이터가 같음을 비교했다. 실제 전체 prepare/project에서 Product51, P35 ready, 기존 targeted visual2개 유지, 회전4행을 확인했다: `.logic.11`6121/1079ms, `.logic.12`11256/1306ms, `.logic.14`25597/1306ms, `.logic.16`31306/1306ms. 이 값은 사용자 편집값이며 agent가 재계산하지 않았다. root CAS 자료는 `out/KoukuTrackTarget20260915/logic65.patch.json`, typed candidate, `rotation.bootstrap.tsv`이다.

검증: 실제947 Parse/Validate/Serialize/Save/Expand14검사 PASS(전체 pattern 원문 보존, kind UI목록1개, 결과·visual값 거절), Python4검사 PASS(새회전2+기존targeted2), 실제 PowerShell publisher 본문4행·5거절 PASS, 기존 실제 Server room/collision/catalog 검사 포함82검사 PASS. 회전 전용의 visual0, 지정 타겟·단계별 yaw·±180도 shortest arc·frame 묶음·end와 기존 logic64 보존을 확인했다. Client Codec/Workbench2TU와 Server Catalog/Brain/LogicRuntime/BossSimulation/test5TU를 out에서 컴파일했고 링크·실행도 exit0이다. 수정 C++의 UTF-8/CRLF를 유지했다. 새 프로젝트 파일은 없다.

이 subtask는 canonical JSON·runtime 게시·EXE 재빌드/실행·UI 조작을 하지 않았다. out Save fixture만948로 증가시켰으며 실제 사용자947 및 이후 저장은 root의 freshness CAS 소유다. 코드 준비와 실제 제품 적용, 사용자 화면 판정은 구분한다. 근거: `out/KoukuTrackTarget20260915/validation.receipt.json`, `publication.receipt.json`, `codec.result.log`, `result.log`, `publisher.log`.


## G32-B. V1 박스 수명 맞춤 및 편집 줌 보존

Box Detail의 `Fit Effect lifetime to box`를 V1_EFFECT/V1_ELEMENT occurrence에 연결했다. optional `fitEffectToDuration`은 기본 false로 기존 문서/재생을 보존한다. true면 resource duration / occurrence duration 비율로 원본 clock을 한 번 재생하며 원본 문서·입자 생성 횟수·모든 TRS를 바꾸지 않는다. owner/bone history는 역시간변환해 실제 박스 clock을 조회하고 Screen Post capture boundary도 같은 변환을 사용한다. Source parser/typed validation/두 serializer, Preview/Product parser와 publisher가 동일 필드를 소비한다. 미지원 kind와 잘못된 bool은 거절한다.

Duplicate/Delete 후 자동 Fit을 요청하지 않고 같은 Pattern 재선택도 기존 줌을 보존한다. 사용자의 명시 Fit과 다른 Pattern 선택은 유지한다. 원래 타임라인의 시작/길이를 이 기능이 변경하지 않는다.

actual helper/transform-provider 본문 추출 82 checks/0 failures, PresentationPlayer 격리 Debug TU compile PASS, 최신947 실제 Codec/Save/Expand에 포함한 sector5개 bool 보존 및 잘못된 type/Camera/stale Save 검사를 포함해 Stage 담당 183 checks PASS다. 이 CPU/저장 검사는 GPU 화면 확인을 대신하지 않는다. 근거는 `out/KoukuBoxLifetime20260915/validation.receipt.json` 및 `out/KoukuStageTimingPreserve20260915/probe.log`다.

### 사용자 정정과 보존

사용자가 폭탄 두 개는 복제 뒤 위치를 옮기지 않아 겹친 것이며 기존 폭탄 동작과 대형 파란 폭발 크기가 맞다고 확인했다. 따라서 bomb 4개 fit, 본체 duration/end-aligned 활성화, 폭발 일반형 교체 및 시작시간 정렬은 최종 적용 후보에서 제외했다. 두 Effect 정본은 byte-identical이며 모든 저장된 박스 시간/위치/크기를 보존한다. 최근 사용자가 옮겨 Save한 `.presentation.652/.653`을 포함한 source948도 보존했다.

사용자의 명시적인 등록 요청 후 source948→949와 부채꼴 Effect의 두 파일만 writer lock/CAS로 설치했고 KoukuSaydon Owner 게시가 exit0으로 완료됐다. Product51/P35 ready, sector fit5개, BOSS_TRACK_TARGET4개와 실제 Server bootstrap4행을 확인했다. 사용자392개 박스의 시작/길이/위치/회전/크기를 모두 보존했고 원래 폭탄·대형 파란 폭발 두 파일은 SHA256까지 동일하다. 설치 전의 source947 CAS는 사용자948 저장을 보호하며 거절했고, 그때 사용자948을 대상으로 생성된 Product50 결과는 이번949 전체 게시로 교체됐다. 근거는 `out/KoukuShowtimeFinal20260915/{installation-receipt.json,installed-source-verification.json,published-verification.json,owner-publish949.log}`다. 제품 EXE를 빌드하거나 Client/UI를 조작하지 않았다. 새 Client/Server 빌드·재실행 후 `Play Pattern` 서버 재생에서 회전하고, 일반 `Play`는 연출 Preview다.


### G32-B 저장 수와 동시 재생량 점검

사용자947은 Presentation392개/서로 다른 V1 asset19개지만 최대 동시26개다. 실제 `Estimate_DocumentBudget`로 문서 전체 비용을 occurrence 전체 기간 동안 보수 집계한 peak는 particles6233/16384, mesh779/4096, draw1294/6144, light1/32로 해당 Pattern 단독의 scene hard 초과는0이었다. 이는 다른 장면·플레이어 이펙트와 Server 동적 복제를 포함한 GPU 표시 성공 증거가 아니다. source resource duration과 박스 길이가 다른352개(짧은 원본9/짧은 박스343)는 각각 내부 effect 수명과 사용자가 정한 종료의 차이이며 이를 모두 버그나 삭제로 간주하지 않는다. 요청한 sector5개만 fit으로 연결했다. `out/KoukuStageTimingPreserve20260915/budget/overlap.json`과 `lifetime-boundaries.json`에 원문 계산을 보존했다.


## G34. 쇼타임 Duration 동안 500ms 랜덤 낙하

사용자가 두15행 묶음에서 한 발사 세트씩 교대하고, 랜덤 위치는 쇼타임 아레나의 이동 가능한 바닥 전체로 확정했다. Source952의 groups797/853에서5행씩 A1/B1/A2/B2/A3/B3를 stable occurrence ID로 연결했다. `randomVolleyOccurrenceSets`와500ms/16m/0.1m 설정을 logic64에 추가한 source953을 Composition writer lock/CAS로 설치했다. 사용자 모든 patterns/resource 원문, 기존 fixed636의2000ms·tracking454의0.5배 속도, logic65의 회전, 폭탄/파란 폭발과 기존 부채꼴5개 fit을 보존했다.

### G34-01. 저장과 실제 소비자

Codec의 Parse/Validate/Serialize/Save/Expand 및 parent remap, Workbench Logic의 Arena volley sequence 순서/추가/제거·간격·영역 입력을 연결했다. Projector는30행을 Product의 정적 재생에서 제외하고, Preview 저작행은 그대로 둔다. 시간은 각5행의 최초 시작에서 상대화하고 MAP 위치는 첫 MAP 예고의XZ에서만 상대화한다. 총구 BOSS follow/TRS는 원문 그대로다. A/B 내용이 같아3개의 content visual hash로 중복 제거되지만6회 순서 배열은 보존한다. 수명은4853/4853/4587/4587/3806/3806ms다. 기존 fixed/tracking template는 deep-equal이다.

Server Gameplay publisher와Catalog의 supplemental10field PATTERNSHOWTIMERANDOM을 기존11field SHOWTIME row와 exact join한다. GameRoom의 기존 CombatObject 경로가15tick마다 전역 한 세트를 생성한다. 플레이어가4명이어도 랜덤은 한 세트이며 기존 플레이어별 fixed/tracking만 인원에 비례한다. no-alive 때는 순번을 보존하고 다음 마감만 진행한다. navigation/commit 실패는 순번과 마감을 보존하며, 늦은 tick을 몰아 생성하지 않는다. Duration 종료는 새 생성을 닫고 이미 시작한 finite 세트의 수명은 보존한다.

Client targeted 정의를 MAP/BOSS 두 묶음으로 나눠 같은 Sample 함수와 서버 생성 clock을 사용한다. MAP은 복제된 CombatObject의 지면 root, 총구는 정확히 같은 source entity의 실제 CNpc transform/model을 소비한다. source 첫 관측 시0초 root history를 채워 V1 birth0 조회를 지원하고, 보스 소실 시 총구만 정리한다. 재등장은 현재 age의 활성 행만 재생한다. 이 두 경계는 독립 리뷰 지적 뒤 수정하고 실제 내부 history 소비 경로까지 재검토했다.

### G34-02. 실행한 검증과 경계

Python12검사, 실제 Client codec43검사, 변경 Composition/Workbench2TU와 PresentationPlayer1TU 격리 컴파일 PASS다. 실제 Sample/Update/Stop 본문24 CPU 검사에서 root 분리·동일clock·source 누락/재등장·수명 종료·실패 cleanup을 확인했다. Effect/모델 의존성은 해당 probe의 mock이며 실제 GPU 판정을 대신하지 않는다. Server는 Trigger ABI 변경을 반영해 out의 OBJ를 새로 컴파일·링크했고 실제 room106검사와 publisher20검사, actual 후보6행 join을 통과했다.

실제 CServerNavigation Load/Is_PointWalkableExact/Sample을 사용한0.125m 격자32768점 검사에서32개 평탄 walkable cell을 확인했다. cell 모서리 최대반경15.803586m, 첫 외부 동일높이cell 최소거리16.099499m라 기존 spawn 중심 sampler 반경16m를 설정했다. 높이는1.299005~1.317626m로 spawnY1.32에서0.1m 허용한다. 이는 navigation 중심 판정이며 원형 FLOOR08 메시의 실제 최대반경13.694941m나 각 이펙트의 전체 footprint와 동일한 경계라는 주장은 하지 않는다.

현재17.071초 Duration은35회/175개 Effect birth를 만든다. 기존 static·4인 fixed/tracking까지 보수적으로 합치면 동시61개, particles9113/16384, mesh1109/4096, draws1844/6144이며 이 Pattern만의 Scene hard 초과는0이다. 다른 플레이어 스킬·장면 효과 및 실제 GPU 표시는 이 계산에 포함하지 않았다. 마지막 이미 시작한 tail은62402ms까지 허용한다.

근거: `out/KoukuShowtimeRandomTargets20260915/{ClientCodec,arena,budget,client-validation.json,installation-receipt.json}`와 `out/KoukuRandomVolleys20260915/{server_result.log,publisher_result.log,actual-candidate-verification.json,source-verification.json}`. 설치 source는953이며 제품 owner 게시 결과는 다음 단락에 기록한다. Client/Server 제품 EXE 빌드·UI 실행·화면 캡처는 수행하지 않았다.

G34 제품 게시도 완료했다. `Invoke-BuildDomainOwner -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 953`의4개 domain이 PASS이며 Product51개에 P35가 포함됐다. Patternbindings/Encounter sourceRevision953, targeted visual5개(기존2+random내용3), 실제 Server bootstrap의6개 PATTERNSHOWTIMERANDOM 행과500ms/16m/.1m를 확인했다. 원래 폭탄·대형 파란 폭발 문서 SHA는 이전 값과 동일하다. 근거는 `owner-publish953.log`, `published-verification.json`이다. 새 Server/Client 코드의 제품 EXE 재빌드는 사용자가 수행해야 한다.

## G35. 알비온 전기·내부 채움·소멸과 백스텝 교체 후보 검증

### G35-02. 원본 호출과 실제 재생 방향

첨부한 두 이미지를 열람하고 원본 action4219903의 `par_v_rpct_line_atk_01_loc_int` 호출을 분리했다. 기존 `fourfan.impact`의 crack projectile과 다른 시스템이며 기존 crack 문서는 보존했다. Stage0은 네 호출, Stage3은 세 호출이다. 원본14 emitter를 각각56/42요소로 합쳤고 source 재질·모듈·수명·TypeData mesh 회전은 유지했다. 호출의 상대 지연은 십자0~4.257ms, 세 갈래0~0.639ms이고 원본 notify scale은0.8000000119다.

실제 설치 `fm_i_corssplane_01.wmodel`의 원본 정점 X는[-140,0]cm이며, 실제 CEffectPlayback의0.1초 matrix와 modelPreScale.01을 적용하면 끝점 X[-9.5,4.5]m로 -X 방향이다. 중앙 source FRotator32768(180도)을 적용한 +X를 배우 +Z로 변환하는 -90도 basis를 새 독립 그룹에 한 번만 적용했다. notify X=-100cm 위치도 같은 basis로 [0,0,-1]m에 옮겼다. P39의 실제 actorProfile은MN_RPCT_05이며 G2 대형 세이튼 yaw 예외를 적용하지 않는다.

Stage3 raw FRotator는41870/32768/23119다. 정확한 source degree는229.998779296875/180/126.9964599609375이며 대칭 각도로 반올림하지 않는다. 최종 CPU 정점 끝점의 배우 전방 기준 heading은+49.998806/0.000003/-53.003507도이고 각 선의 길이는11.2m다. 십자는 네 직각 heading을 확인했다. 길이는 source mesh/particle/notify scale을 합친 실제 값이며 사용자 확대값을 추가하지 않았다. 신규 두 resource의 기본 anchor는BOSS로 현재 배우 yaw를 소비하고 바닥 경고의 MAP anchor는 유지한다.

### G35-03. 기존 ID 보존과 검증 범위

`fourfan.warning.single` 및 `fourfan.warning`의 총5요소에는 기존 원·도넛 helper의 native3602 `inner` track만 추가했다. alpha·11m radius·70도 각도·TRS·source recipe·재질은 동일하다. 실제 `Build_ArtistMaterialTrackBindings/Apply_ArtistMaterialTrackSamples`가 native row0.z에0/.25/.5/.75/1을 기록하며 다른 parameter lane은 bit-identical임을 확인했다.0→1 채움은0~1.473333초에 진행하고 기존1.7초 fade를 유지한다. 이 곡선은 사용자 요청한 프로젝트 연출이며 회수되지 않은 원본 엔진의 곡선이라고 주장하지 않는다. 기존 쇼타임 fan의 static inner 결정은 변경하지 않았다.

소멸은 기존 `effect.kouku.gate3.showtime.saydon.disappear`와 resource66의 ID를 유지해 `알비온_사라지기이펙트`로 이름만 변경했다. 실제 Codec에서 이름 이외 serialize가 동일하고 Save/Reload가 동일함을 확인했다. 기존 sourceModelPreview와 본부착 재생은 그대로이며 이번 검증으로 실제 본 재생을 새로 보장하지 않는다. 사용자 추가 요청의 백스텝은 기존 `effect.kouku.gate3.backstep.electric.threeway.authored`를 새 전방 세 갈래 구성으로 교체하고 기존 tree 위치·resource ID를 유지한다. 이것은 명시적인 사용자 연출 교체이며 원래 백스텝 projectile 복원 주장이 아니다. 다른 source leaf와 손 트레일은 삭제하지 않았다.

새 생성기 `Tools/EffectPipeline/build_kouku_albion_polish.py`는6문서와 CAS registration만 out에 작성한다. 실제 최신 Effect_Playback.cpp를 out에서 새로 컴파일해 신규 전기2·경고2·백스텝1의 Load/Validate_Drawable/Save/Reload, 고정60Hz 재생·종료·반복 seek를 검사했다. 관측 요소는각56/42/1/4/42개이고 peak particles는511/377/0/0/376이다. 소멸의 이름 전용 검사를 포함해6문서/0실패이며44개 Resources 경로 누락0, source before SHA 일치를 확인했다. Python parse/compile 및 변경 파일 diff-check를 통과했다. 공유 shader와 제품 C++ 파일은 추가하지 않았다.

근거는 `out/KoukuAlbionPolish20260915/{registration.json,source-calls.json,validation.receipt.json}` 및 `CPU/playback.json`, `CPU/{leaf,three,cross}-direction.json`이다. 이 절은 후보 준비·CPU 수치 검증까지이며 정본 CAS/게시 상태는 후속 설치 절에 기록한다. 새 timeline placement나 시작시간은 만들지 않았고 Client/UI 실행·캡처·GPU visual PASS는 수행하지 않았다.

### G35 정본 설치 및 게시 완료

최신953에11개 파일을 Composition writer lock/CAS로 등록해954가 됐다. Effect6개(신규2·경고2·소멸명칭1·기존백스텝본문교체1), Catalog/Tree, Composition, Client project/filter의 새 Data None2개가 포함된다. 새 십자와 세 갈래 resource는BOSS 기준, 기존 백스텝resource502f6b0a64280f618807는ID/BOSS 기준을 유지하고 수명3851ms와 이름을 갱신했다. P39 warning.presentation3 fittrue 외 타임라인 시간·TRS·개수는 보존했다. P46은 기존 Effect box0개이므로 임의 재생 시간은 추가하지 않았고 요청한 백스텝 Effect library payload를 교체했다.

KoukuSaydon owner954의4개 domain 게시가PASS이며 Product51개, Patternbindings/Encounter sourceRevision954, P35 random6개/500ms 및 P39경고fit을 실제 결과에서 확인했다. source953의쇼타임pattern·logic 전체와 기존 폭탄·대형 파란폭발·쇼타임sector SHA는 동일하다. 변경 JSON9/XML2 및 scoped diff check를 통과했다. 새 native shader/program이나 RenderingProfiles 변경은 없어서 별도 Rendering publisher는 실행하지 않았다. `installation-receipt.json`, `installed-verification.json`, `owner-publish954.log`, `published-verification.json`이 근거다. 제품 EXE 재빌드와 Client 시각확인은 사용자에게 남긴다.

## G36. Server 시작 실패의 Showtime 행 순서 수정

사용자가 보고한 `Process gameplay generation failed to initialize`는 bootstrap의 1879~1884행 RANDOM 6개가 1885행 TARGETS 부모보다 먼저 놓여 발생했다. `New-KoukuShowtimeTargetRows`는 부모를 먼저 생성하지만 최종 정렬에서 RANDOM이 TARGETS 앞으로 이동했다. 첫 자식의 부모 조회 실패이며 발사 설정이나 Character Select 바닥 문제가 아니다. G34/G35의 게시 성공과 행 개수 확인은 실제 Server 초기화 검증을 대신하지 못했다.

`Publish-GameplayBalance.ps1`의 `Get-BootstrapRowSortKey`에 encounter/pattern/trigger별 TARGETS 우선순위를 추가했다. 기존 숫자 정렬과 실제 행 내용, 서버의 소유자·순번·반복 설정 검증은 유지한다. 새 `Test-GameplayBootstrapRowOrder.ps1`이 실제 publisher AST 함수를 실행해 다중 소유자·부모만 있는 경우·32개 발사·행 byte와 중복 수·기존 World/Logic/Stage 의존 순서를 검사한다.

검증 및 적용:

- 현재 실제 CGameplayCatalog로 기존 오류 재현, 수정 후보 전체 로드 성공, P35 logic7의 6개 발사·500ms·16m·0.1m 유지, 잘못된 순번·소유자·반복 설정 3개 후보 거절을 확인했다.
- Windows PowerShell 5.1의 정렬 회귀 320행 × 3 shuffle이 통과했다. 기존 오류 정렬의 negative control도 실패를 검출했다.
- 빌드와 같은 powershell.exe의 공식 Gameplay publisher로 후보 생성·검증 뒤 runtime 게시를 완료했다. 총 4854줄의 내용은 동일하고 1879~1885의 7줄 순서만 변경됐다. sourceRevision954와 저작 타임라인·설정은 유지했다. 초기 PowerShell 7 후보의 실수 표기 차이는 배포하지 않았다.
- 게시 SHA256은 a6d11f3c3d5a462b6a07a3c6d585ca7c4914a92b7570447380982137e133358f이며 검증 후보와 같다. 게시 경로도 실제 카탈로그 로더에서 성공했다.
- 기존 Server/Bin/Debug/Server.exe를 --headless --bind-address 127.0.0.1 --port 17777 --smoke-timeout-ms 1000으로 실행해 모든 world simulation 초기화·listen 진입 및 자동 종료 exit0을 확인했다. 격리 검사는 팀 LAN endpoint를 변경하지 않는다. 제품 EXE 재빌드와 Client/UI 실행은 하지 않았다.

이 오류 수정은 데이터 게시로 현재 EXE에 적용됐다. Server를 재실행하면 되며 다른 세션의 코드 변경에 필요한 빌드는 별도다. 증거는 out/ShowtimeBootstrapOrder20260915/의 row-comparison-windows.json, Probe/validation.receipt.json, runtime-publish.log, runtime-native.json, server-startup.log, final-validation.json에 있다.

## G37. 2026-09-16 십자 위치 고정과 세 갈래별 중심 편집

### G37-01. 반영한 데이터와 편집 코드

`effect.kouku.albion.cross.electric.impact`의 56개 요소 중 켜져 있던 `detail.particle.localSpace` 44개를 껐다. 나머지 12개는 이미 꺼져 있었다. 원본 Required 모듈 literal과 source leaf, 요소 ID·재질·Transform·시간은 그대로다. 현재 P39의 `presentation.6`도 `BOSS` anchor를 유지하면서 `followBoss=false`로 바꿨다. 시작 2033ms, 수명 3855ms와 사용자 offset[-0.05,0,0.95]·회전·크기를 유지하며 첫 생성의 보스 기준점을 계속 사용한다.

알비온 전방 세 갈래와 이를 사용하는 백스텝 문서는 각각 원본 호출의 14개 요소를 `manual.albion.frontthree.ray.1/2/3`, `manual.kouku.backstep.threeway.ray.1/2/3`으로 나눴다. 원본 stable ID 변환을 재현해 현재 요소와 연결하므로 사용자 TRS·시간·배열 순서로 갈래를 추측하지 않는다. 변경은 문서별 42개 groupId뿐이다. `build_kouku_albion_polish.py`의 `--current-group-edits`는 최신 문서에서 이 필드만 바꾼 후보를 만들며 원본 재생성 경로에도 같은 정책을 적용한다.

`Effect_Tool_Helpers.cpp`의 기존 그룹 계산은 unattached 요소의 명시적인 `manual.*` ID만 키에 추가하고 `Ray 1/2/3` 라벨을 보여 준다. 각 갈래의 `Group Center (effect-local m)`과 `Play Group`을 사용할 수 있다. 자동 source 그룹·본별 그룹·inheritance·source track 검증은 유지했다. 기존 UTF-8 무BOM, CRLF와 기존 bare LF 2개를 보존했다. 새 제품 파일·shader·프로젝트 등록은 없다.

### G37-02. 검증과 설치

- 실제 helper 원문과 실제 문서 codec의 52개 검사를 통과했다. 각 문서는 편집 가능한 3그룹 × 14요소이며 선택한 14개 위치만 이동하고 다른 요소·시간·재질·회전·크기는 동일하다. 기존 본별/자동 그룹 동작과 실패 시 문서 보존도 확인했다. 변경 Helper TU 최소 컴파일 exit0이다.
- 실제 CPU 재생 비교 12,072개 검사에서 그룹 분리 전후 World·색·clock 차이 0이다. 십자 고정 root의 전후 geometry 차이도 0이며 모든 birth 뒤 root를 움직여도 수정본의 변위는 0이었다. 원래 Local Space 대조군은 11.3863m 차이를 보여 검사 감도를 확인했다. 3문서의 codec Save/Reload·종료·seek와 Resources 36개 누락 0을 확인했다.
- 실제 PresentationPlayer의 placement 조건문·Make_Pivot·PivotSampler·V1TransformProvider를 추출한 14개 수치 검사는 추적을 끈 첫 root를 지연 생성에도 보존했다. 이 검사는 전체 Sample/renderer 실행이 아니며 history/bone 의존성은 mock이다. seek/restart로 새 row를 만드는 경우의 최초 기준점은 이번 연속 재생 검증과 별개다.
- 저장 중 writer lock과 변경 해시를 만나 정본을 쓰지 않고 최신 저장본을 다시 읽었다. 최종 사용자 저장 revision965를 백업하고 Composition writer lock/CAS로 JSON4개를 설치해966으로 올렸다. Composition은 십자 한 occurrence의 follow와 revision 이외 모든 값이 동일하다.
- 공식 `Invoke-BuildDomainOwner -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 966`의4개 domain은 PASS/REUSED다. Product51개, Patternbindings/Encounter sourceRevision966과 게시된 십자 follow=false·BOSS·시간·TRS 일치를 확인했다. 변경 JSON parse와 scoped diff-check를 통과했다.

근거는 `out/KoukuAlbionGroups20260916/`의 `installation-receipt.json`, `installed-verification.json`, `validation.receipt.json`, `Groups/validation.receipt.json`, `CPU/comparison.json`, `RootProbe/result.json`, `owner-publish966.log`다. Client 제품 EXE 재빌드는 사용자가 수행해야 새 Group Center 분리가 반영된다. Client/UI 실행·화면 캡처·최종 시각 판정은 수행하지 않았다.

## G38. 알비온 공중 등장·중앙 착지와 파란 원 채움

### G38-01. Save 충돌 복구와 최신 저작 보존

G37에서 외부로 바꾼 Composition966의 십자 follow 값 때문에 열린 편집기의965 기준 Save가 거절됐다. 자기 변경의966 SHA를 확인하고 writer lock 아래 follow와 revision만 역변경해 사용자965 기준 bytes와 일치시켰다. 이펙트 문서·그룹 코드는 유지했고 freshness 검사를 제거하거나 미저장 draft를 Reload하지 않았다. 사용자가 Save 성공을 확인했다. 이후 저장969/977을 거쳐 최종988의 파란 원7개와 전체Effect14행, 소멸 배치, 위치·시간, Stage11 길이6541ms를 다시 읽어 후보989에 보존했다. 예전977 후보는 설치하지 않았다.

원본 측정과 비교한 결과 마지막 Stage의 hold 길이만 늘었으며 모든 animation occurrence와 source/play/startOffset은 같았다. 후보는 logic53·66~70의6정의에 typed 값을 추가하고, 두 번째 SELECT와 같은7817ms에 빠져 있던 APPEAR occurrence11을 추가한다. 기존 logic 시간은 유지한다. 십자 follow=false를 다시 반영하고 사용자가 재확인한 중앙 시작에 맞춰 P39 `resetBossToSpawn=true`를 설정한다. 다른 패턴·저작 값은 바꾸지 않는다.

### G38-02. 실제 소비자 연결

Composition codec·Workbench·projector·Gameplay publisher·Server catalog에 `ALBION_AIRBORNE`의6phase와 네 필드를 연결했다. 기존25열 mechanic 부모 뒤에7열 supplemental 행을 exact join하며 unrelated kind에 값을 허용하지 않는다. 같은 시각의 SELECT는 APPEAR보다 먼저 실행한다. 선택은 살아 있는 플레이어 ID를 고정하고 실제 등장은 해당 시점 XZ를 읽는다. 플레이어 없음·목적지 navigation/body 검증 실패는 기존 actor pose와 phase를 유지한다.

원본 `_24_03`은 고정 공중 pose다. 기존 후속 하강으로 계산한13.6788133052m를 유지하면서3483ms JUMP부터200ms에 상승한다. 두 APPEAR는5432/7817ms에 `_24_04`의3.2783114316m 높이와 당시 원본 root 진행을 사용한다. DISAPPEAR6913ms는 현재XZ에서 처음 정점 높이로, CENTER9292ms는 실제3관문 spawn `[-0.0700000003,1.32000005,942.330017]`의XZ에서 같은 높이로 복귀한다. SLAM11808ms부터 `_24_05`에 남은 하강을 정규화한다. 원본 최저 위치에 닿은 뒤에는 지면에 머물며 늘어난 Stage11 tail을 천천히 낙하하는 시간으로 쓰지 않는다.

Server는 기존 `Apply_StageRootMotion`의 navigation·body sweep과 clock을 유지하며 phase Y를 계산한다. 순간이동은 stage root 원점과 지면도 갱신한다. 실제 최저 sample을 fixed tick이 건너뛰어도 prefix minimum으로 착지하고 작은 원본 반등을 다시 높이로 적용하지 않는다. Begin/Finish/abort에서 per-pattern 상태를 정리한다. 새 packet·이동 runtime·모델은 없다.

일반 Play는 기존 preview clone에서 같은 phase를 소비한다. SELECT의ID와 APPEAR 최초 시점의 복제 위치를 따로 저장해 중간에 플레이어가 이동해도 등장 시점 위치를 사용하고 seek 때는 같은 위치를 재현한다. 원본 root suppression을 유지해 mesh와 actor의 Y를 이중 적용하지 않는다. `Complete Play (Server)`는 실제 Server 권위 선택·위치와 snapshot을 사용한다. 기존 두 경로 모두 `resetBossToSpawn=true`로3관문 중앙에서 시작한다.

### G38-03. 파란 원과 수치 검증

standalone warning 및 combined runtime의 원형 요소에서 `sourceTransformTrack.materialParameterTracks`만 추가했다. native3600의row0.y inner가0초0→2초1로 커지며 combined의18개 번개가 시작하는2초와 맞는다. 기존 색·직경3.2m·alpha fade와 모든 번개TRS/시간은 보존했다. Composition 박스5680ms를 전체 source7000ms에 fit하지 않아 낙뢰 시점을 앞당기지 않는다. 최신 사용자의7개 배치는 같은 문서를 사용하며 각 배치의 start+2000ms에 채움이 끝난다.

- 실제 Effect codec/Save/Reload와 native parameter14sample을 포함한463검사 통과, 다른 native lane bit 동일, Resources35개 누락0이다.
- 최종989의 실제 Composition codec Parse/Validate/Serialize/격리Save와 설치MN_RPCT_05 실제CModel sampler, 실제 Preview consumer 함수의3,173검사/실패0이다. SELECT/APPEAR 시간 분리·사망 재선택·정방향/역방향seek·Stage11 extended hold를 포함한다. 해당4Client TU 컴파일 exit0이고7파일 UTF-8 무BOM/CRLF를 보존했다.
- 실제 source 게시 곡선과 native sampling의 SLAM 시작 차이는0.835mm로 기존3mm 허용 이내이며 원본 최저값/최종 지면은 일치한다. 추출한 Preview consumer는 snapshot과actor sink를 대체한 CPU검사이며 전체 Client/GPU 실행을 의미하지 않는다.
- 독립 코드 검토에서 발견한 Preview 착지 반등·SELECT 때의 오래된 XYZ 재사용은 수정 연결까지 확인했다. Server explicit Stop은 기존 정책대로 현재XYZ를 유지하며 새 Complete Play의 중앙 시작과 별개다.

자료는 `out/KoukuAlbionJump20260916/{save-recovery.json,composition-stage.json,projection.json}`, `BlueCircle/validation.receipt.json`, `out/KoukuAlbionGroups20260916/AirborneClient/validation.receipt.json`이다. 최종 정본 설치·Server 전체 bootstrap 로드·공식 게시 결과는 아래에 별도로 기록한다. 제품 EXE 빌드 및 Client 화면 검증은 사용자가 수행한다.

### G38-04. 정본 설치·공식 게시 완료

실제 Server 코드의 격리 컴파일·링크·실행을 완료했고126개 consumer 검사/실패0이다. 새 공중17검사와 parser3검사를 포함하며 missing/duplicate/out-of-order supplemental, 잘못된 phase/높이/시간, 선행JUMP 없음과 남은 하강 없음은 이전 Catalog generation을 유지하며 거절한다. 실제 전체 `CGameplayCatalog::Load`가 최종989의8공중 trigger·중앙 시작·Stage11 6541ms를 읽었다. Server C++의 UTF-8 무BOM/CRLF도 보존했다.

최신988의SHA를 다시 확인하고 Composition writer lock/CAS로 Effect2개와Composition1개를 설치해989로 올렸다. 사용자 파란 원7개를 포함한Effect14행, 모든 기존Stage/animation/Logic/Effect 시간·TRS가 그대로임을 비교했다. 요청된6정의 typed값·등장1행 추가·중앙시작·십자follow 외 변경은 없다.

공식 `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 989`는4개 domain PASS/REUSED 및 exit0이다. Patternbindings/Encounter sourceRevision989와 검증 후보의 전체 gameplay/presentation 값이 동일하다. 공식 publisher가 덧붙인 patternInventory도 최신 저작에서 계산한 inventory와 일치한다. 게시 Gameplay.bootstrap은 실제Catalog로 검사한 후보와 bytes가 완전히 같다. SHA256 `1ecb8f53fb4abb76d00c5d3eb52e8ea22ddbb04f0ed4e093a509d10935d5c77b`, 데이터4870행과header1행이다.

변경JSON parse·Python airborne2검사·scoped `git diff --check`를 통과했다. 근거는 `out/KoukuAlbionJump20260916/{server-verification.json,installation-receipt.json,installed-verification.json,published-bootstrap-verification.json,owner-publish989.log}`다. 실행 중인 Client48252/Server14032는 기존Debug EXE이며 이번 작업에서 재빌드·재실행하지 않았다. 사용자에게 두EXE 종료→Client/Server 빌드→재실행→알비온 패턴 `Complete Play (Server)`를 안내했다. 일반 Play는 preview이고 최종 Client 시각 판정은 미완료다.

### G38 후속 저장 호환성 처리

이전 EXE를 계속 사용하던 Object 편집 저장이 새 airborne 필드 때문에 거절되어,09-12 World Object Group RESULT G07에서 자기988→989 변경만 잠시 역변경했다. 사용자의 새 Logic 정의와 칼날8개 저장 후 최신본에 같은 G38 필드를 다시 적용했으며 현재 Composition992에 보존된다. Stage/animation/기존Effect 시간과 파란 원7개는 그대로다. 상세 CAS/게시 증거는 G07을 정본으로 따른다.


## G39. 네 방향 본체 선택·Summon Play와 그룹 위치·회전

### G39-01. Group Center와 Group Rotation

Effect Tool의 Current Effect → Group by anchor에서 Group Center를 DragFloat3로 바꾸고 Group Rotation (deg)를 추가했다. 공통 불뿜기와 세 방향 본 그룹을 중심 기준으로 움직이거나 회전할 수 있다. 첫 요소의 저장 orientation을 입력 기준으로 삼고 모든 멤버의 위치·orientation·선형 위치/속도 끝점에 같은 quaternion delta를 적용한다. per-element Transform을 Save Changes로 저장하므로 별도 저장 schema나 누적 UI 회전값은 없다. 회전 animation/revolution owner는 명시적으로 거절하고 기존 이동 편집은 유지한다. Try_CommitDocument의 Stage_WorldPreview/Refresh_Effects가 현재 정지·재생 커서에서 다시 준비하며 실패하면 기존 문서와 preview를 유지한다.

실제 common firebreath25요소·clone breath15요소·three-way breath110요소의6그룹에 대해 현재 helper와 실제 codec Validate/Save/Reload를 사용한2,098검사/실패0, 최대 matrix 차이1.41859e-6이다. 비선택 요소·source recipe·본 anchor·시간·크기 보존과 역회전·실패 보존을 확인했다. Effect_Tool_Helpers/Detail의 격리 TU 컴파일은 모두exit0이다. 이후 사용자가 수행한 VS 빌드의 Client.log와 실제 OBJ09:19:31 및 Client.exe09:19:55에서 두 변경 TU의 컴파일·링크를 확인했다. 이 기록은 에이전트가 실행한 Product runner 검사와 구분한다. Client/UI 실행·캡처·육안 판정은 수행하지 않았다.

### G39-02. 방향 선택의 저작과 실행 연결

새 DURATION CROSS_DIRECTION_CLONES는 네 stable directionPatternIds, cloneEndStageId, summonOccurrenceId를 저장한다. Summon은 같은 시작 시각으로 Logic 전체를 포함하고 독립 patternSpawns를 겹치지 않는다. Codec/Workbench와 Python projector가 같은 actor/Gate/boss의 MECHANIC leaf Animation+Effect만 허용하며 본체 action owner 중첩을 거절한다. Logic/Summon 연결의 복제와 Parent 확장에서는 stable ID를 함께 remap한다.

Server는 cutoff까지의 실제 forward/lateral root motion을 현재 본체 yaw로 변환하고 원래 boss spawn XZ와의 거리로 방향을 결정한다. 같은 거리는 저작 순서를 유지한다. 기존 실제 boss identity·Parent clock·Logic ledger를 유지한 채 선택된 child만 별도 재생 상태로 소유하고 다른 세 개는 기존 dependent entity 생성 경로로 commit한다. ownerRunsFinale admission에 typed cross trigger를 연결했으며, 만료된 duration의 지연 trigger는 새 분신을 생성하지 않는다. 분신은 cutoff에서 정리하고 본체는 child 전체 브레스를 재생한 뒤 Parent로 돌아간다.

Shared protocol87의 optional presentation pattern/action/startTick/stage를 ClientReplication이 본체 animation에 사용하고 PresentationPlayer는 Parent와 child의 Effect session을 따로 소비한다. 일반 Play는 기존 bundle preview actor 준비에서 네 배우를 만들고, 명시적 Summon patternSpawns도 같은 경로로 재생한다. 원본 Pattern/Bundle 중복-boss 검사는 해제하지 않는다. fake actor는 시작·종료 구간에서만 표시한다. child animation 선택은 Logic window로 제한해 이후 Parent의 idle/animation을 복구하며 누적 이동 위치는 유지한다.

### G39-03. 검사와 설치 상태

현재 실제 Composition codec의 Parse/Validate/Serialize/Save/reload/freshness 실패 보존84검사/실패0이다. 기존 P32 quarantine은 원본과 동일하게 보존한다. Client 변경4TU 격리 컴파일과 설치된 실제 CModel 골격·animation을 사용한332검사를 통과했다.12개 위치/yaw의 방향 선택, 네 배우 준비, fake1600ms·브레스 차단, 일반 Summon, 역방향 seek, 생성 실패 정리, exact window 종료 후 Parent pose와 최종 위치 보존을 포함한다. 이 검사는 실제 production helper와 CPU model sampling이며 Client/GPU 화면 검사가 아니다.

Server+Shared를 out에서 새로 컴파일·링크하여 기존 Kouku bundle/Logic112검사/실패0을 확인했다. 실제 Room의 네 몸체 이동·세 clone 생성·STAGE_1 종료·본체 STAGE_2·이후 Parent HUD·identity/clock 보존과 최종 정리, snapshot 왕복/잘못된 필드 거절을 포함한다. 실제 root/navigation/collision 경로로 네 종점과 본체 최종 위치도 확인했다. Python focused3검사와 PowerShell AST를 통과했다. 전체 후보 prepare_publication은51개 Product와 대상P50/P52/P53/P54/P55를 포함했으며, 현재 Gameplay publisher의 실제 validation/serialization 본문으로 out-only 후보23,329행을 생성했다. 이 전체 후보를 실제 CGameplayCatalog::Load로 읽어 Parent52·네 방향ID·STAGE_1 cutoff까지 확인했다. 정본 canonical preflight/게시와는 별개다.

데이터는 아직 out의 후보이며 실행 중 편집기를 덮어쓰지 않았다. 사용자 추가 저장의 P52 Summon15초 및 P39 변경을 최신 후보에 보존한다. 각 방향은 이동STAGE_1 1600ms 뒤 기존P60 브레스STAGE_2 5167ms와Effect3589..6589ms를 연결하여 explicit duration6767ms로 준비했다. 원본P60과 다른 Pattern은 유지한다. 최종 source CAS 설치·공식domain 게시·최신 Product 빌드와 사용자 화면 판정은 아래 완료 기록 전까지 미완료다.

근거: out/EffectGroupRotation20260916/validation.receipt.json, out/KoukuCrossClones20260916/{composition-stage.json,projection.json,publisher-probe.log,server_result.log}, Client/validation.receipt.json, compile/codec_probe.exe. 신규 제품 CPP·project/filter 항목·shader·Resources 바이너리는 추가하지 않았다. 기존 대규모 dirty 변경과 함께 있어 자동 stage/commit/push하지 않았다.

## G40. 일반 Play의 순간이동과 플레이어 추적 회전

쇼타임의 logic63 BOSS_TELEPORT_XZ는39209ms에[2.57,1.3,952.27]로 이동하도록 저작되어 있지만 일반 Play의 소비자가 없었다. 새 PreviewRootMotion 이벤트는 목적지XZ + D(t) - D(trigger)를 사용하고 Y·yaw·animation clock을 유지한다. 같은 시각 이벤트는 저작 순서를 보존하며 마지막 위치를 사용한다. BossMotion과의 기존 소유 충돌은 유지한다. 이름만 있던 중앙이동 logic47에는 같은 typed kind와 Gate3 boss spawn을 연결한 후보를 준비했다. 공유 정의이므로 P35의55869ms와 P38의1990ms 배치가 같은 중앙 목적지를 소비한다.

Preview의 BOSS_TRACK_TARGET은 대상 identity와 처음 방문한30Hz tick의 입력 위치를 기록하고 Server와 같은 남은 tick 기준 최단각 보간을 사용한다. 재생 중 움직이는 플레이어 위치를 읽고 되감기는 기록을 재사용한다. SHOWTIME_PLAYER_TARGETS의 본체 회전도 현재 목표를 따르며 반복 발사·추적/랜덤 투사체 생성은 이번 일반 Play 구현 범위에 포함하지 않았다. 실제 gameplay Transform과 판정은 기존 Server authority다.

typed 공간 Preview의 Effect root는 source clock의 pose sampler를 사용한다. BOSS follow=false의 최초 발생 pivot을 현재 커서 대신 occurrence 시작에서 계산하고, 따라가는 입자도50m 미만 순간이동을 두 프레임 사이로 보간하지 않는다. 과거 root 샘플은 실제 actor/animation을 바꾸거나 미기록 미래 플레이어를 새로 선택하지 않는다. 이 경로는 typed 공간 Preview의 BOSS root에 한정하며 현재 P35 BOSS274행은 모두 bone이 비어 있다. 일반 WEAPON/WORLD/bone history까지 바꿨다고 기록하지 않는다.

최종 header/hooks를 포함한 RootMotion/PresentationPlayer/MainApp/LogicPreview4TU 격리 컴파일과 scoped diff-check를 통과했다. 설치된 실제 모델 기반 검사는 기존 분신332개와 새 쇼타임54개, 총386개/실패0이다. 두 순간이동·Y 보존·움직이는 대상 추적·seek와 frozen/따라가는 Effect root 경계를 포함한다. float seconds→ms의 경계 반올림으로39209ms 이벤트가39208.999로 평가되는 문제도 실제 probe에서 수정 후 재검사했다. 근거는 out/KoukuCrossClones20260916/Client/{spatial-validation.receipt.json,spatial_preview_probe.run.log}이며 Product와 GPU 화면 검사가 아니다.

## G41. 플레이어 중심 파란 장판과 세 갈래 Local Space

새 P39.logic.12의 logic72는 기존 ALBION_BLUE_CIRCLE에 countPerPlayer1/radiusM0/effectLifetimeMs7000을 넣은 후보로 준비했다. 실제 BossCatalog의 combatvisual.kouku.albion.bluecircle이 원 예고·폭발 runtime Effect를 resolve한다. Collider 전용 logicOccurrenceId는 변경하지 않았고 기존 수동 MAP 장판7개·P39의 모든 배치·Stage는 보존했다. 기존 source의 이름만 있는 Trigger를 새 kind로 연결하므로 최종 정본 설치와 게시 전에는 사용 중인 편집기에 적용되지 않는다.

KoukuSaydonPresentationPlayer_LogicPreview.cpp는 기존 class SESSION/Sample을 재사용하여 Trigger가 처음 재생될 때 살아 있는 플레이어의 발생 위치를 고정한다. 사망/낙하/마리오 참가자를 제외하고 기존 ground sample을 확인한다.7초 수명, 역방향/정방향 seek, 대상 없음의 일회 소비, Stop/새 Play의 정리를 처리한다. 사전 준비도 동일 BossCatalog asset을 포함한다. arena-random/radial/randomPlayerOnly 옵션은 일반 Play의 이번 지원 범위가 아니다. 새 CPP와 Client 프로젝트/filter를 최소 등록했고 XML parse 및 UTF-8 무BOM을 확인했다. 격리 TU 컴파일과 catalog/effect sink를 대체한 CPU scheduling/pinning14검사/실패0이다. 기존 Effect renderer 자체의 화면 검사로 기록하지 않는다.

세 갈래 알비온과 파생 백스텝42요소 문서는 각각 켜져 있던33개 detail.particle.localSpace만 끈 후보를 만들었다. 원본 Required literal·그룹·TRS·시간과 음수0까지 다른 모든 bytes를 보존했다. build_kouku_albion_polish.py의 원본 생성 정책과 --current-threeway-world-space 후보 경로도 반영했다. 실제 CPU5,995검사/실패0에서 고정 root geometry 차이0, 방출 후 root 이동 변위0이며 기존 local-space 대조군은12.8004m로 감도를 확인했다. 실제 codec Save/Reload와 반복 seek2문서, 리소스36개 누락0, 원본 재생성6문서 검사도 통과했다. Composition의 BOSS follow 값은 별개로 보존했다.

G40/G41의 소스 구현과 out 후보 검증만 완료했으며, 최종 사용자 Save 뒤3개 정본 CAS 설치·공식 게시와 사용자 빌드/화면 판정은 아직 남아 있다. 근거는 out/KoukuCrossClones20260916/LogicPreview/{compile.log,probe.result.log}, candidate-verification.json과 out/KoukuPlayLogic20260916/localspace/{registration.json,validation.receipt.json}이다.


## G42. 화염링 fixed-axis Sprite 회전 — 2026-09-16

원인은 화염링_동일화염포의 Sprite Particle 02(fx_a_noise_001.dds)가 SourceRecipe orientation axis lock을 쓰지만 SourceTransformTrack은 없어, 최종 Billboard 면 구성에서 authored Transform 회전이 제거되는 것이었다. 선택적 detail.sprite.followEmitterAxisRotation(bool, 기본 false)을 codec/UI/Playback/Geometry에 연결했다. 고정 source 축에 정규화한 emitter basis를 한 번 적용한다. local-space는 현재 basis, world-space는 생성 시점 basis를 사용한다. camera/velocity 및 기존 Matinee/local 경로와 워로드 수동 roll 보정은 유지한다. 새 CPP/셰이더/프로젝트 항목은 없다.

실제 저장 SourceRecipe→Playback→최종 quad 검사 73개와 fresh codec core+DetailIo 검사 78개가 통과했다. 잘못된 bool 타입 거부·실패 시 이전 문서 보존·false 생략·save/reload를 포함한다. 기존 struct padding의 새 bool 때문에 구 OBJ의 생성자를 섞지 않고 최신 헤더로 codec core까지 다시 컴파일했다. Geometry/Playback/UI/codec TU와 Debug 제품 빌드가 통과했다. 생성기는 두 조립 문서의 해당 요소만 opt-in하며 원본 leaf나 공통 화염 Sprite는 변경하지 않는다.

근거: out/KoukuCrossClones20260916/SpriteAxisRotation/probe.result.log, out/KoukuSpriteAxisRotation20260916/authoring/stage.json, out/KoukuSpriteAxis20260916/UI/validation.receipt.json. 화면 판정은 수행하지 않았다.

## G43. Parent의 Summon 하나로 분신 공통 실행 — 2026-09-16

최종 구조는 전방을 실행 owner로 겸하지 않고 Parent의 Summon occurrence 하나를 사용하는 방식이다. Summon definition에 summonKind=CROSS_DIRECTION_CLONES, 네 directionPatternIds와 cloneEndStageId를 설정하며 별도 hidden Logic은 저장하지 않는다. Summon의 start/duration이 유일한 시계다. 기존 Logic 방식도 같은 resolved window로 처리한다. Summon Catalog/Box Detail에서 설정을 검증 후 Apply하고, MainApp ordinary Play와 서버 publisher가 실제 설정을 소비한다. No child Patterns 안내는 Summon으로 실행 가능한 Parent 상태를 반영하도록 수정했다.

실제 revision 1050 저장본을 기준으로 1051 후보를 만들고 모든 unrelated Pattern과 사용자 P50의 8-stage 애니메이션, P63/64 십자 이펙트 시간·TRS, P43의 별도 추가 동작을 보존했다. Parent52는 기존 Summon2를 쓰던 P44에 영향을 주지 않도록 새 Summon7로 분리하고 P50의 이전 독립 3분신 발생은 Parent 정책으로 옮겼다. Parent65/Summon5는 기존 십자 좌우를 보존하고 비어 있던 전방71/후방72를 추가한다. Parent66/Summon6는 67~70의 이동에 9_01→9_02→9_03 구간과 3방향 이펙트를 연결한다. 첫 가족은 사용자가 저장한 P50 8-stage를 유지하고 다른 세 방향에도 같은 후속 구간을 준비했다. 각 Parent는 Summon 하나만 가지며 네 leaf에는 애니메이션과 이펙트가 있다.

- Parent52: 50/53/54/55, STAGE_1 종료1600ms, 각11334ms, Summon15000ms.
- Parent65: 71/72/63/64, STAGE_18 종료1600ms, 각6575/6575/6575/6525ms, Summon6665ms.
- Parent66: 67/68/69/70, STAGE_1 종료1600ms, 각6267ms, Summon15000ms.

실제 설치 모델 기반 Client 검사 834개가 통과했다. legacy clone+Showtime, 2 Parent의 공유 정책, 다양한 위치/yaw, 최신 세 Parent의 네 배우·가장 가까운 본체·가짜1600ms 종료·본체의 전체 애니메이션과 이펙트·reverse seek를 포함한다. Codec 27개(typed/legacy/중복 owner 거부/JSON round trip/atomic save/freshness)와 projector 집중 8개가 통과했다. 준비된 projection은 세 Parent 및 모든 방향 Pattern을 제품에 포함한다.

Client/Server 실행 프로세스가 없고 저장본 SHA가 그대로인 것을 확인한 뒤 writer lock과 파일별 SHA 비교·원본 백업·원자 교체로 Composition+전기 LocalSpace 두 문서+화염링 옵션 두 문서, 총5개 정본을 설치했다. 기존 G39~G41의 ‘정본 적용 대기’ 상태는 이 설치로 해소했다. Source revision은1051이다.

Debug Product 빌드는 Engine/Shared/Server/Client 모두 PASS, SkipBuild=false다. 앞선 다른 빌드의 잠금 해제 뒤 최신 상태를 증분 확인했으며 코드의 임시 우회나 실행 중 EXE 교체는 하지 않았다. 근거: out/BuildPipeline/runs/20260916T013403657Z-debug-product.json 및 out/KoukuSummonParent20260916/product-build.log. 설치 근거는 installation-receipt.json, 소스 후보/보존 근거는 composition-stage.json, Client 수치 근거는 out/KoukuCrossClones20260916/Client/summon-validation.receipt.json이다. Client/UI는 실행하지 않았다.


G43 서버 검증: 후보1051을 현행 publisher로 생성한23556줄 Gameplay bootstrap을 fresh GameplayCatalog/KoukuSaydonBrain으로 로드했다. 세 Parent마다 정확히 하나의 CROSS trigger가 Summon occurrence ID와 원래 시계를 사용하며 네 child 의존성, 실제 root endpoint의 중앙 거리 선택, cutoff1600ms가 모두 통과했다. P39 단독 파란 장판 Logic도 게시 가능하다. 기존 P37의 동적 FOLLOWUP_PATTERN 미지원과 비어 있는 Pattern의 unavailable 상태는 이번 수정과 무관하게 유지했다. out/KoukuSummonParent20260916/catalog-probe.log 및 projection.json이 근거다.


G43 최종 게시 완료: `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 1051`이 exit0으로 완료했다. koukusaydon.product/world.gameplay/gameplay.balance는PASS, map.kakulsaydon은REUSED다. 공식 Server Gameplay.bootstrap은 실제 Server 카탈로그로 검사한23556줄 후보와 byte-identical이고 Client patternbindings의 sourceRevision도1051이다. 정본5개 JSON과 프로젝트 XML parse, scoped git diff --check를 통과했다. 마무리 시 Client/Server는 실행하지 않은 상태다. 사용자는 재실행한 쿠크 편집기에서 각 분신 Parent를 선택해 Play하고, 화염링 Sprite Particle02의 Element/Group 회전을 화면으로 확인하면 된다. 결과: out/KoukuSummonParent20260916/{publish.log,completion.receipt.json}.


## G45. 불뿜기 쇼 clip 추가의 explicit duration 거절 — 2026-09-16

현재 source1052의 P67은 Stage 합6267ms와 explicit duration6267ms가 같다. MN_RPCT_07 action4219940 `쿠크세이튼_불뿜기 쇼`의 첫 clip1667ms 또는 전체9668ms를 추가하면 기존 Workbench는 Stage만 늘리고 explicit duration을 유지해 `Explicit Pattern duration is shorter than its Stages`로 candidate를 거절한다. 검증기는9월12일5f0b1046부터 존재했고 P67은560741ac의 G43에서 명시 길이와 함께 추가됐다. pull 병합이 append 코드를 삭제한 것이 아니라 기존 누락이 새 데이터에서 드러난 회귀다.

`KoukuSaydonActionWorkbench.cpp`의 Add_Stage, Append_AnimationAsStage, Bind_Animation, Append_ActionAsStages, Append_ActionToStage, Append_CinematicGroup 및 Set_PatternStartOffset은 commit 전에 기존 lifetime 확장 함수를 호출한다. 기존 긴 tail과 Effect/Logic 등 비애니메이션 행의 시간·TRS·ID를 유지하고 Stage 합이 더 길 때만 전체 수명을 확장한다. implicit clock은 필요 없으면 그대로0이며 600000ms 상한과 candidate 검증·rollback을 유지한다. 정본 Composition JSON, schema, 헤더, project/filter는 수정하지 않았다.

변경 Workbench 전체 TU를 현행 헤더로 out에 격리 Debug 컴파일해 exit0을 확인했다. 기존 Engine header의 C4828 경고는 남았다. `out/KoukuClipDurationBuild20260916/compile.log`가 근거다. C++ UTF-8 BOM 없음/CRLF 유지, 두 Composition JSON parse와 scoped diff-check를 확인했다. 실행 중 Client61448/Server50504의 편집을 보존하기 위해 종료·UI 조작·정본 덮어쓰기를 하지 않았다. 이 격리 컴파일은 Product 재링크 증거와 구분하며 실제 UI 입력 확인은 사용자 전용이다. 이후 사용자 종료가 확인되어 Debug Product Build를 진행했다.

실제7개 편집 함수와 Commit_Candidate, 현행 codec으로 focused311검사/실패0을 확인했다. 원래 P67에 첫1667ms clip을 추가한7934ms, 실제 Save_Atomic→새 owner Reload와 revision 증가, 외부 변경 후 stale Save 거절이 통과했다. 기존30000ms tail 보존, implicit0 보존,600000ms 초과와 잘못된 입력에서 draft·ordinal·dirty·generation 보존을 확인했다. UI 선택·동기화만 검증용 shim을 사용했으며 Client UI는 실행하지 않았다. 근거는 `out/KoukuClipDuration20260916/{result.log,result-summary.md,source-extraction.json}`이다.

전체 Action9668ms 추가는 합15935ms이므로 원래 P66의 Summon window15000ms 검사가 계속 거절한다. 이는 이번 버그와 별도인 정상 보호 조건이다. 전체 Action 추가를 허용하려면 사용자가 Parent와 해당 Summon occurrence 길이를 먼저15935ms 이상으로 늘려야 한다. 전체 Action 및 긴 tail 검사는 out 사본의 Parent/Summon만40000ms로 늘려 실제 의존 그래프를 유지했고 정본은 변경하지 않았다.

최종 Product 명령은 Engine/Shared/Server가 통과했으나 Client에서 동시에 진행된 VS 빌드와 `Effect_Tool.obj` 출력이 겹쳐 C1083 Permission denied 및 D8040으로 실패했다. `out/BuildPipeline/runs/20260916T023249078Z-debug-product.json`과 `out/KoukuClipDurationBuild20260916/product-build.log`가 근거이며 이 Product 명령을 PASS로 기록하지 않는다. 시작 전 발견한 VS 재사용 MSBuild node를 실제 빌드 유무 확인 없이 진행한 것은 피해야 하는 중복 실행이었다. 빌드를 재시도하거나 실행 중 프로그램을 종료하지 않았다.

별도 VS 빌드의 `Client/Bin/Debug/Client.exe`는11:32:58에 갱신됐고11:32:59에 Client62548/Server63068이 실행됐다. Workbench source11:19:50, Product OBJ11:31:44이며 새 EXE에는 변경한 `Stage edit would exceed the 600000 ms Pattern lifetime.` 문자열이1개, 이전 `Stage resize` 문자열은0개라 이번 TU 반영을 확인했다. 이는 EXE 반영의 증거이며 에이전트 Product 명령의 성공이나 사용자 UI 확인을 대신하지 않는다. 사용자는 현재 Client에서 Pattern67의 `Append as Stage`를 다시 입력해 실제 편집을 확인한다.

## G46. 외부 Composition 편집과 미저장 draft의 보존 병합 — 2026-09-16

작은 오망성의 재생 길이 두 필드를 외부에서3000→5052ms로 바꾸자 실행 중 편집기의 Save가 실패했다. 설치 직후 source SHA를 writer lock 안에서 확인해 Composition만1061→1060의 정확한 bytes로 복구했고 사용자가 Reload 없이 Save 성공을 확인했다. 이후 정본은 계속 사용자 소유 편집 상태이며 추가 외부 수정을 하지 않았다. 이것은 기존 실행 파일의 저장 복구이며 아래 새 병합 코드가 실행됐다는 증거가 아니다.

원인은 Save_Atomic의 외부 변경 허용 범위가 PresentationResources의 끝에 추가된 항목으로 한정된 데 있었다. Document 내부에서 LastGood·사용자 candidate·현재 디스크의 canonical typed JSON을 비교하도록 수정했다. schema에 명시된 stable ID 배열은 행별, 객체는 필드별로 병합한다. 동일 변경과 한쪽만 바꾼 값은 보존하며 같은 필드의 다른 값·동일 ID의 다른 추가·삭제와 수정·서로 다른 재정렬·순환 insertion anchor는 경로를 알려 거절한다. 좌표와 비-ID 배열은 원자 값이며 임의 asset 참조를 행 ID로 추정하지 않는다.

정확한 root/Pattern/Bundle의 allocation counter만 최댓값으로 합친다. current revision의 증가·상한, writer lock, 후보 검증, 임시파일 durable write/검증, 마지막 bytes CAS, 원자 교체와 reopen 후 LastGood commit은 유지했다. Workbench는 기존 Save 성공 후 LastGood을 draft로 가져오는 경로를 그대로 사용하므로 해당 파일의 다른 작업을 변경하지 않았다. 기존 invalid/orphan Pattern·Folder·Bundle의 원문 보존은 유지하되 두 개의 유효 편집을 합쳐 새 invalid 행이 생기면 저장을 거절한다.

검증은 실제 Save_Atomic→디스크→Reload의 기본30건, Pattern 경계6건, 최종 Pattern/Folder/Bundle 공용 guard6건이 모두 통과했다. 동일 occurrence의 외부duration5052와 사용자Scale2/3/4 보존, 같은duration경쟁 거절, 삭제/순서/삽입, Sequence workspace, -0.0/1.0 보존, 충돌 해결 후 재시도, live writer lock과 실제 임시파일 생성 중 경쟁쓰기의 CAS 거절을 포함한다. 각각의 거절에서 디스크·LastGood·호출자 draft가 보존됐다. 각 반복의 정확한 소스/범위는 `out/CompositionThreeWaySave20260916/RESULT_NOTES.md`, `probe_result.log`, `nested_result.log`, `hierarchy_result.log`에 기록했다.

최종 Document CPP의 독립 native 컴파일과 scoped diff-check가 통과했다. 새 제품 EXE/DLL을 빌드하거나 교체하지 않았다. 현재 실행 Client에는 편집 저장 후 사용자가 새 빌드로 재실행해야 적용되며 UI에서의 새 병합 저장 확인은 사용자 전용이다. 새 H/CPP와 프로젝트 등록은 없다.


## G47. 화염링 fx_a_noise_001 양면 적용 — 2026-09-16

사용자가 한 면에서만 보인다고 한 대상은 화염링_동일화염포의 Sprite Particle02, `kouku.backstep.c03d483eb0e60fc432c2.1`이다. native2876의 기존 Additive One Sided(pass4, back cull)는 회전 옵션과 별개로 유지돼 있었다. 사용자의 후속 지시대로 ImGui 옵션은 추가하지 않았고 해당 요소에만 `detail.sprite.twoSided=true`를 설치했다. 최신 사용자 저장본 SHA315a82a7…를 다시 확인해 18bytes 필드 삽입만 적용했으며 결과SHA118a7975…다. 재질·TRS·followEmitterAxisRotation·나머지34요소와 Composition은 동일하다. 생성기도 조립ring.flame 하나의 해당 요소만 재생성하며 원본leaf와 다른5문서는 동일하다.

선택bool은 기본false/저장시false생략이다. 지원범위는 기존 Artist registry의 정상 native SourceRecipe Sprite 중 Alpha/Additive One Sided다. Material renderProfile/native ID/원본 descriptor equality를 그대로 두고 실제 sprite draw에서 양면pass1/2로 바꾼다. 지원하지 않는 carrier·compiled adapter·원래양면profile·Multiply/Opaque는 true를 거절한다. 별도native-v14 SourceContract에는 이 저작옵션을 허용하지 않는다. 이는 원본복원값이 아닌 사용자 요청의 컬링 변경이다.

현재헤더로 codec/core/material 관련35CPP를 모두 다시 컴파일했고 Renderer TU의 별도컴파일도 통과했다. native probe371검사 실패0, 실제fixture의Alpha8/Additive3지원, bool오류66건·미지원10건거절과 실패시이전문서보존, 기본false·true왕복·native ID보존을 확인했다. 실제Kouku2816과3136 FX를 메모리컴파일하고 headless WARP에서 pass Apply 후 CULL_NONE과 기존blend/depth state 동일성을 확인했다. 재수입의Detail전체보존은 현재코드로 확인했으나 native fixture에 generic reimport의Base DDS전제가없어 실제reimport 검사는 완료로 쓰지 않는다.

근거는 `out/KoukuSpriteTwoSided20260916/{source-installation.json,native_result.json,native_source_receipt.json,installed/receipt.json,authoring/focused-verification.json}`이다. 기존인코딩과줄끝을보존했고 Python AST·JSON parse·scoped diffcheck를 통과했다. UI수정은 이번작업전bytes로 정확히 회복했다. 제품EXE/DLL/CSO를 설치하거나 Client/UI를 실행하지 않았으며 사용자의 새빌드·재실행 후 해당Effect 재로드가 필요하다. GPU상태검사는 원작외형 또는 사용자화면승인이 아니다.

실제 설치한35요소 ring.flame도 별도15검사로 Load/Validate_Drawable/정확target2876의Supports·원본contract·실효AdditiveTwoSide 및 Serialize→Parse→Validate를 통과했다. canonical 전체가같아재질/source/TRS/flag가보존됐다. `target_result.json`이 근거이며 최종sourceSHA는설치receipt와같다.

## G48. 패턴 사이 Animation·Effect Ctrl+C/V — 2026-09-16

Workbench의 기존 다중 선택을 세션 내 값 snapshot으로 복사하고, 다른 Pattern 끝에 한 번의 candidate commit으로 붙여넣는다. 전체 Stage와 개별 Animation·Effect 혼합을 지원하고 Stage 자식을 중복 복사하지 않는다. 원본 source clip identity·trim·속도·끝 정책, Effect 수명·follow/fit·TRS·선택 그룹과 유효한 미저장 배치를 보존한다. 필요한 World owner와 두 선택 clip 사이의 Animation Blend도 새 occurrence ID에 연결한다. 선택한 전체 Stage 또는 의존 owner가 먼저 시작하면 그 선행 구간까지 포함하며 선택 항목 사이의 간격은 유지한다. 선택하지 않은 clip과의 외부 blend 경계는 제외하고 메시지로 알린다.

원본을 편집하거나 삭제해도 snapshot은 유지된다. 없는 공유 정의는 snapshot에서 복구하지만 같은 ID의 정의가 달라졌으면 기존 것을 덮지 않고 거절한다. 지원하지 않는 lane, gameplay Logic 연결 Effect, 다른 actor의 animation/bone Effect, 잘못된 참조, ID 소진과 600000ms 초과는 전체 실패하며 draft·선택·이전 clipboard를 보존한다. 붙여넣은 항목은 새 stable ID와 선택을 가지며 자동 Save·Publish는 하지 않는다.

Patterns와 Sequencer 창의 focus를 기록하고 frame-local 행 포인터 사용이 끝난 뒤 단축키를 한 번 처리한다. standalone에도 같은 경로를 연결했다. 텍스트 입력·활성 widget·popup/modal·마우스 드래그·marquee와 다른 도구 focus에서는 Ctrl+C/V를 소비하지 않는다. Ctrl+D/Delete는 기존 동작을 유지한다. 별도 OS clipboard나 두 편집 세션 사이 전송은 지원 범위가 아니다.

현재 실제 Copy/Paste/Commit/Validate_SourceStart 함수와 전체 Composition codec의 격리 native 검증 182개가 통과했다. 혼합·반복 Paste, 빈 Stage, Effect-only, snapshot 유지, 미저장 TRS, World remap, 실제 resolver의 내부 blend 새 ID/7400~7800ms 창, 실패 원자성과 out 사본 Save_Atomic→Reload를 포함한다. UI 입력 guard는 실제 handler에 ImGui/API stub만 연결한 25개 검사로 확인했다. 전체 Workbench CPP의 Debug 격리 컴파일은 오류 0이며 기존 Engine C4828 경고만 남았다. Client/UI를 실행하지 않았고 제품 EXE는 교체하지 않았다. 사용자가 저장 후 새 빌드로 재실행해야 실제 단축키가 적용된다. 정본 Composition은 외부 수정하지 않았다.

근거는 `out/KoukuTimelineClipboard20260916`의 native 검증 결과와 `ui/hotkey_guard_result.log`, `ui/compile_workbench.log`다. H/CPP의 기존 UTF-8/CRLF와 scoped `git diff --check`를 확인했다. 새 C++ 파일이나 프로젝트 등록은 없다.

## G49. 마리오 소환 공·인형 HP 수명과 Object Tool — 2026-09-16

### 소스 구현 및 후보

본무대 공은 NPC480713/MN_PPCC_00이고 설치 StripedBall은 원본 MN_PPCC_00_SK에서 나온 모델이다. 원본290정점/512삼각형의 위치·UV 오차는3.58e-8m, topology와 c/n/s decoded pixel은 일치한다. preScale.01·Object scale1에서 높이0.941609497m, 바닥 보정0.00146865845m다. 신규 `world.object.kouku.mario_circus_ball`을 `마리오 소환 공 (원본 크기)`로 인형 옆에 추가하는 후보를 준비했다. 기존 미니게임 공과 MN_RHCN 공의 데이터는 유지한다.

원본 action4194527 stage002 notify003의 `Par_L_PPCC_SK_02_Single`15요소를 `effect.kouku.gate3.mario.circus.ball.aura`로 구성했다. 무지개빛·문양·고리와 요소별 빨간색 전환 곡선을 유지하며 저장 주기는7500ms다. MeshMaterial 배열3개가 Required보다 우선하는 원본 계약을 적용했다. 기존14개 native 계약을 재사용하고 누락된117/118 MIC에만 native3687/3688과 DDS1개를 추가했다. 기존 shader 함수1372개의 본문은 보존했다. 외형의 사용자 승인은 아직 없다.

인형은 기존 작은/큰 크기와17314ms Motion, 사용자 large Effect 회전90도,3개 clip의trim/속도를 보존했다. 준비·끝8요소를 유지하고 본화염28요소만 공통 불뿜기25요소×두 입으로 교체한58요소 `effect.kouku.gate3.doll.flame.shared`를 준비했다. 실제 설치 CModel의 b_mouth_f/b_mouth_b와 기존 FX_Prj_01/02 socket을 사용한다. 공통+Z를 원본 socket+X로 yaw90도 연결하고 cm basis100을 한 번 보상하여 공통의 기존1.7배를 유지했다. 이는 사용자가 요청한 공통 화염 교체이며 원본 인형 불꽃과 동일하다는 주장은 아니다.

두 인형과 공은HP2000이며 실제 설치 모델 bounds를 body에 기록했다. 인형은33개 실제 애니메이션 pose/9301정점의 union bounds, 공은 실제290정점의 구형 bounds다. 구형 공만 ELLIPSOID를 사용하므로 Server 피격 반지름은0.4708047485m이며 AABB 대각선0.665816m로 커지지 않는다. Object/placement 배율은 그 뒤 한 번 적용한다.

### 재생·서버·도구 계약

선택적 combatBody→Kouku projector→PATTERNWORLDCOMBAT→WORLD_OBJECT→기존 melee/projectile HP adapter를 연결했다. WORLD 박스는 spawn 시각만 소유하고 정상 Pattern/Bundle 종료 뒤에도 살아 있는 개체가 재생된다. HP0·명시 취소·새 run·owner session 퇴장·room reset·원래 boss의 제거/사망은 정확한 STOP_CUE를 보낸다. 늦은 PLAY는 tombstone으로 막고 late join은 살아 있는 cue만 받는다. boss iteration에서는 body를 stage한 뒤 tick 경계에 commit하고 실제 packet preflight 실패는 commit하지 않는다. 일반 bootstrap에서 WORLD_OBJECT를 직접 만들지 않는다. wire는protocol88로 Client/Server를 함께 갱신해야 한다.

고정 WORLD/count1/단일 binding/LOOP만 이 생존 정책의 제품 지원 범위다. body는 기존 Object→player Collider와 별도이며 Client가 HP를 판정하지 않는다. Object Tool에서 Loop Animation + Effects를 켜면 저장된 model/Effect 창의 최대 길이로 한 단위를 반복한다. 공7500ms, 인형17314ms가 같은 model/Effect 시계다. 보수적 native tail 추정값13.5초로 공 주기를 늘리지 않는다.

Object Tool의 Zoom·Ctrl+wheel·Fit, Stage 끝 드래그·Stage Duration·Fit Stage to Animation, V1 Effect lifetime fit과 HP/body 편집을 연결했다. Stage 확장은 기존 clip/Effect/TRS 시점을 재분배하지 않으며 잘리는 축소는 원자적으로 거절한다. MOTION_END→TIME 고정은 상태에 알린다. Workbench WORLD Resource/Box Detail은 HP 수명과 박스 길이를 구분해 표시한다.

### 실행한 검증

- 실제 인형 CModel+production bone/socket helper와 Effect CPU 재생1040프레임:58요소, peak202입자,15~16초에도 두 입이 계속 방출, 실제 본 basis의100배 보상 오차1.00136e-5, backward seek 재생 통과. canonical 문서 전체 왕복 일치. UI를 실행하지 않았다.
- Object codec/Stage/loop/fit 시계94검사, publisher31case 및 기존 회귀2test, 최신 정본253template/449object/309instance 저장 왕복 통과. WorldDocument/Tool/Player/Objects4TU와 Workbench/MainApp2TU 격리 컴파일 오류0.
- Server 변경15TU 컴파일 오류0. 실제 HP/packet/lifecycle24검사, Client cue race14검사, projection16검사와 기존 collider13검사 통과. peer 보강 후 boss 제거·사망·불량 packet rollback 등을 포함한51검사와 변경2TU 컴파일 통과.
- ELLIPSOID codec27검사·publisher10검사·수치34검사, 변경2TU 컴파일 통과. MeshMaterial 기존4다중재질 element의 실제codec78검사와 변경TU 컴파일 통과.
- 공 원본 후보 CPU의0.1~7.5초에 빈 프레임0, peak134입자, 마지막 실제 입자7.5초, 이후0, seek/reset TRS·color 오차0.15/15emitter를 유지했다.

검증 근거는 `out/KoukuMarioObjectLife20260916`의 body-bounds/doll-probe/ball/world, `out/WorldObjectParity20260916`, `out/KoukuWorldObjectHealth20260916`, `out/KoukuWorldObjectHealthPeer20260916`, `out/KoukuEllipsoidCombatBody20260916`, `out/KoukuMeshMaterialOverride20260916`이다. 개별 assertion 반복 횟수는 서로 다른 테스트 수로 합산하지 않는다.

### 게시 승인 전 상태 — 이후 G49-01에서 데이터 게시 완료

현재 이 기록 시점의 새 Effect2개와 WorldSequences는 out 후보이며, 기존 Client/Server가 실행 중이라 열린 Object Tool의 저장 기준본을 외부 교체하지 않았다. native2개·DDS1개는 추가 설치했다. candidate builder는 World JSON의16MiB 공통 상한을 검사하며, 배열 숫자까지 전부 들여쓰기해21.8MB가 된 최초 후보는 폐기하고 stable entity별 compact row로 약7.98MB 후보를 다시 만들었다. 제한을 늘리거나 검증을 우회하지 않았다.

사용자 저장·종료 확인 후 최신 source SHA에서 후보를 다시 확인해 새 Effect/cat/tree/project와 WorldSequences를 설치하고 공식 Map/Kouku/Gameplay publisher를 실행해야 한다. 현재 제품 EXE/DLL/CSO는 교체하지 않았으며 격리 TU/CPU 검증은 제품 전체 링크 또는 화면 완료가 아니다. 최종 Client 화면 판정은 사용자에게 남는다.

G49 후속 후보 전체 검증도 통과했다. 후보 rev2033/SHA1b590ac2…/7,982,844bytes는 실제 Client Load→Save→Reload→IsEquivalent와 원본 WModel material slot/Area material을 포함한 전체 publisher 함수를 통과했다. 254template/450object/310instance이며 기존 stable row 삭제0, 새 공3행 외에는 인형 두 형태의 요청 필드만 바뀌었다. native3687/3688이 포함된 실제 MeshKouku3648 fx_5_0/O1 컴파일은53,822bytes로 통과했고 source 재생성도 candidate와 일치했다. `out/KoukuMarioObjectLifeValidation20260916/final-validation.json`과 ball manifest가 근거다. 사용자가 계속 사용 중이며 소스 작업만 진행하라고 답했으므로 World 정본·런타임 게시·제품 설치는 보류했다. 이후 저장·종료가 확인되면 최신 저장본에서 후보를 다시 생성한다.


## G50. 중앙 포탈·쇼타임 노이즈의 화면 중복 보호 — 2026-09-16

### 확인한 원인과 수정 범위

사용자는 중앙 오망성과 캐릭터·쿠크가 겹쳐 두 개처럼 보이며 이전 쇼타임 폭탄도 같은 noise 문제였다고 보고했다. 소스 조사에서 정본6문서의 native에는 직접 SceneColor 읽기가 없고 별도 distortion pass14개가 연결됐다. 중앙 문양2584와 폭탄 도화선2847/2805/2848에는 이 왜곡이 없다. 조합된 중앙 waterflow2587, 폭발 mesh2461 및 큰 푸른 폭발3682가 texture-dependent offset을 갖고 나머지11개 pass는 원본 상수0이다. 따라서 화면 왜곡이 이미 그려진 actor 영상을 옮길 수 있는 소비 경로를 수정했다. 이 연결 근거가 사용자 화면 결함을 단독으로 확정하는 visual 판정은 아니다.

기존 Target_Distortion의 RGBA16_FLOAT에서 RG 일반 왜곡은 유지하고 해당3개 offset만 BA로 분리했다. native 함수1374개와 해당 정본6문서의 SHA는 그대로이며 원본 색·시각·크기·재질식을 바꾸지 않았다. installer도 같은 명시적 ID만 재생성하므로 후속 native 추가로 보호 채널이 사라지지 않는다. 기존 source[0].w mesh-prefix 때문에2461의 실제 출력이0일 가능성은 별도 경계로 기록했으며 이번 수정에서 원본식·prefix를 추측 변경하지 않았다.

EffectCommon의 RT1 RGBA One+One, RingFill/LinearReveal/scale 네 채널 coverage와 material adapter fixed-function 검사를 연결했다. Engine SceneResolve는 BA에만 depth/PickPos 검사를 적용한다. 현재 actor 픽셀, 일반RG와 BA합성 샘플의 실제 bilinear footprint에 있는 actor, 깊이 불연속을 넘는 sample은 BA를 거절한다. 경사면은 NDC 평면 기울기로 비교하며 일반RG는 보존한다. HDR와 가중 Bloom은 같은 UV를 소비한다. 별도 화면 캡처나 추가 render target/pass는 없다.

Renderer는 Depth/PickPos를 명시 바인딩한다. 독립 peer가 불투명 G-buffer→SceneHDR/Blend 종료→SceneResolve 순서, 기존 PS SRV 해제, 분리된 post RTV와 DSVnull을 대조했고 신규 SRV/RTV hazard를 찾지 못했다. 실제 default/source actor writer의 marker0/5 bit8과 source skin/equipment1~24,26~29가 검사와 일치하며 다른 map marker payload는 보호 actor로 오인하지 않는다.

### 실행한 검증

- native regeneration 회귀4개 통과. 실제 MeshKouku2432/ParticleKouku2560/ParticleKouku3648의 FXC fx_5_0/O1 전체 컴파일3종 오류0. 원본 함수1374개와 대상 JSON6개 불변 확인.
- 공통 Sprite/MeshPreview/Decal3개 실제 FXC fx_5_0/O1 및 MaterialHelpers.cpp 전체TU 컴파일 오류0. 등록 adapter16개가 공유하는 실제 Effects11 pass14개의 RT1 RGBA/MRT0·Bloom 불변 상태를 확인했다. RGBA16F WARP12상황×2회=24draw/192성분 readback과 상태 검사를 포함한333assertion으로 signedRG·BA 누적, RingFill/LinearReveal50%coverage 및 시작 전discard가 통과했다. 일반 writer의 zero-init/명시float4도 확인했다. 근거는 `out/KoukuScopedDistortion20260916/common/final-validation.json`이며333을 독립 시나리오 수로 해석하지 않는다.
- Engine Deferred 전체 FX 컴파일 통과. 실제 Effects11 SceneResolve pass와 실제 named SRV를 사용한 D3D11 WARP66검사/69draw/1,130,496 RGBA pixel 수치 검증 통과, D3D11 debug 오류0. Client/UI나 화면 캡처는 사용하지 않았다.
- 일반RG3fixture는 HEAD의 실제 기존PS와 두 MRT 모든 픽셀이 bit 단위 동일했다. BA 배경 유지, actor marker와 program 범위, map payload 보존, 양수/음수/대각 bilinear 이웃, depth 단절·빈 깊이·경사면·가림 경계, mixed clamp와 HDR/Bloom UV 일치를 확인했다.
- WARP resolve 검증은 제어된 float32 texture를 사용했다. 해석식 대비 최대 오차0.00938416은 bilinear fraction 양자화 범위였고 HDR/Bloom 동일UV 불변식은0.0001 허용오차로 통과했다. 제품 전체 frame/실 GPU 시간/최종 화면 동일성은 판정하지 않았다.
- Renderer.cpp 격리TU 컴파일 오류0. 이후 같은 파일의 Target_Distortion 주석만 RG/BA 설명으로 갱신했다. 변경 JSON10개, 프로젝트 XML2개, Python11개 parse 및 해당 변경 diff-check 통과.

근거는 `out/KoukuNoiseOverlay20260916/{source_audit.json,native_compile_result.json}`, `out/ProtectedNoiseReceiver20260916/{gpu-summary.json,gpu-results.csv,source-receipt.json,RESULT_NOTES.md}`, `out/KoukuNoiseReceiver20260916/{renderer_compile.log,final-parse.json}`이다. 컴파일한 Deferred SHA는5dc80cff46470aa5373b841e83d66994d40bee32e9846983355e8eab416231d9다.

### 현재 설치와 화면 확인 경계

사용자는 계속 편집 중이며 소스 작업만 진행하라고 명시했다. 현재 Composition/WorldSequences를 외부 교체하지 않았고 Client/Server를 종료·실행·조작하지 않았다. G49 공·인형 Effect2개와 WorldSequences 추가 후보는 검증된 out에 있으며 아직 Object Tool 정본에 게시되지 않았다. 제품 EXE/DLL/CSO 전체 빌드·설치도 보류했다. Engine 정본과 Client carrier를 기존 build/deploy 경로로 함께 갱신한 뒤 사용자가 중앙 포탈·쇼타임을 재생해 최종 화면을 확인해야 한다.

보호 범위는 G-buffer actor 표식과 깊이 경계다. actor 표식이 없는 정적 prop의 같은 연속면 내부나 G-buffer를 기록하지 않는 투명 객체까지 종류별로 완전히 제외하는 수정은 아니다. 일반RG 왜곡과 원본 노이즈의 배경 표현을 유지한 범위를 기록하며 사용자 관찰 전에 화면 PASS 또는 이중상 완전 해결로 쓰지 않는다.


## G49-01. 공·인형 데이터 정본 설치와 제품 게시 완료 — 2026-09-16

사용자가 데이터 전체 게시를 명시 요청했고 Client/Server 및 빌드 프로세스가 종료된 것을 확인했다. 최신 World source revision2032는 검증 기준 SHA와 같았고, Composition은 사용자 최신 revision1140/SHA07228abb2ffd232d9851f6ea202be7638a37be2ce94f637e9418e78887a34a6a를 유지했다. 추가 확인 요청 없이 기존 검증 후보7개를 정본에 설치했다. World는 revision2033이며 Effect2개·Catalog·Tree·프로젝트 None/filter 등록을 포함한다.

Object Tool에 `마리오 소환 공 (원본 크기)`를 등록하고 작은/큰 괴기스러운 인형의 기존 항목을 갱신했다. 세 Object는 HP2000·UNTIL_DESTROYED·LOOP·loopFullPresentation=true다. 공은 기존 실측 원본 크기와7500ms aura15요소, 인형 두 크기는 기존17314ms 모션과 공통 불뿜기58요소를 사용한다. 큰 인형의 yaw90도와 기존 사용자 TRS/clip은 보존했다. 새 공을 사용자가 선택하지 않은 Pattern 위치에 임의 배치하지 않았다.

정본 전체 재직렬화를 피하도록 staging serializer를 보강했다. 원본 숫자 표기와 기존 행을 유지하고 변경 필드·새3행만 반영했다. 최종 World는13,246,294bytes로16MiB 제한 안이며 Git diff18추가/15삭제다. SHA1009328035bd7e9dbb74c40ed9c18c95f209391c08699d2f4021315f90f5c4b7이고, Map publisher의 정상 CRLF→LF 처리 후 runtime SHA는9ae637c78148bf5e7b4c06a0349ec5b231fdae1b522afb6fc2d86cee723cb199다. 줄바꿈 정규화 후 bytes와 JSON 내용이 일치했다. compact 후보의 이전 SHA는 과거 검증 기록이며 최종 설치값은 이 항목을 따른다.

공식 `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 1140`으로 product→map→쿠크 world→gameplay balance 게시가 모두 통과했다. 최종 서식 보존본의 product/map도 재게시했고 변경 없는 world/balance는 정식 fingerprint 검증 후 REUSED였다. P34.world.1의 인형은 Encounter combatBody와 Server Gameplay.bootstrap의 PATTERNWORLDCOMBAT에 HP2000·radius1.085593201691381m가 실제 생성됐다. 아직 Pattern에 Append하지 않은 새 공의 bootstrap spawn 행이 없는 것은 정상이며, Append·저장·패턴 게시 시 해당 배치가 생성된다.

설치 파일7개 JSON/XML parse, source 보존 serializer 재생성/의미 동일성, catalog/tree/payload 연결, authoring/runtime 일치와 현재 Composition SHA 보존을 확인했다. 원본 Effect·크기·재질 검증은 G49의 선행 결과와 같다. 근거는 `out/KoukuMarioObjectLife20260916/{installation-result.json,world-format-preservation.json,publish-world.log,publish-kouku.log,publish-kouku-final.log,publication-final.json}`이다. 이전 게시 보류 상태는 이 완료 기록으로 대체한다.

남은 작업은 사용자의 최신 제품 솔루션 빌드와 Server/Client 재실행·화면 확인이다. 이번에는 데이터만 게시했고 EXE/DLL/CSO 빌드나 Client/UI 실행은 하지 않았다. Server Play는 정상 Pattern 종료 뒤 HP0/명시 취소·리셋까지 유지한다. 일반 Play preview는 여전히 Pattern 종료 시 정리되며 데이터 게시가 그 경계를 바꾸지 않는다.


## G51. F1 Saved Pattern Flow·All Patterns 목록 로드 복구 — 2026-09-16

### 실제 원인과 수정

사용자가 Publish All Patterns 완료 후 두 목록이 비었다고 보고했다. 현재 revision1143 Encounter16,061,098bytes를 기존 `CKoukuSaydonBossTool::Load_ProductIndex`의8MiB 상한이 거절했다. Reload가 Flow 로드 전에 return하여 저장된 Gate1/2/3 Flow6/11/5개도 없는 것처럼 표시됐다. 게시 실패나 Composition 누락은 아니었다.

BossTool의 파일 읽기·CDataJson 허용량을64MiB, JSON value4,000,000개로 일치시켰다. depth64, 패턴·ID·참조·Stage 검증은 유지한다. 선행 측정 크기까지만 읽고 크기 변화·읽기/파싱 실패를 정확히 보고하며 기존 목록을 보존한다. Product/Flow 로드 오류를 별도 멤버로 보존해 F1 각 목록 내부에 표시하고, 아직 로드되지 않은 상태를 `No saved Pattern Flow`로 오인하지 않게 했다. projector도 같은 byte/value/depth 상한을 출력 교체 전에 검사한다. 정본 Composition·사용자 편집·기존 게시 데이터는 수정하지 않았다. 신규 C++/프로젝트 항목은 없다.

### 실행한 검증

- HEAD의 기존 실제 로더+현재 게시 파일:8MiB 거절과 Flow 미로드4checks PASS.
- 수정한 실제 로더/Reload/Flow/selection 함수와 현재 Composition·DataJson·ProjectDataRoot codec:28checks PASS. 패턴77개 중 실행 가능63개, 폴더19개, 묶음11개 중 실행 가능8개, Gate Flow6/11/5개 확인. 기존 unavailable14패턴/3묶음은 숨기지 않는다.
-20MiB로 공백 padding한 유효 JSON도 실제 parser 통과하여 기본16MiB 한도가 남지 않음을 확인했다. 손상JSON·초과크기·틀린schema 재로드 거절 시 마지막 정상 목록·revision·Flow 보존 확인.
- publisher admission 신규3테스트 PASS:현재 게시 데이터, 출력 전 byte 거절, depth/value 경계.
- Debug x64 격리TU 컴파일:KoukuSaydonBossTool/MainApp/KoukuSaydonActionWorkbench 모두 exit0. 헤더·클래스 크기 변경의 실제 소비자까지 확인했다. 기본 빌드 산출물 변경 없음.
- 변경 코드/문서 `git diff --check` PASS. 기존 C++ CRLF/UTF-8 유지.

증거는 `out/KoukuInventoryLoading20260916/{baseline.run.log,fixed.run.log,compile/compile-results.json}`에 있다. Client/UI는 실행·조작하지 않았으며 EXE 전체 링크·교체는 수행하지 않았다. 사용자 Client 저장·종료 후 Debug x64 빌드·재실행, F1 > KoukuSaydon Complete Play > Load/Reload KoukuSaydon Inventory에서 해당 Gate와 두 Category를 확인한다. 이번 소스 수정은 추가 Publish를 요구하지 않는다. 화면 확인은 사용자 대기다.
