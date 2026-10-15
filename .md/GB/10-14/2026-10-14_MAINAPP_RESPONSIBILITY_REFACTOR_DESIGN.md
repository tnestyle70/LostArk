# MainApp 전체 리팩터링 설계와 수정 순서

> G05~G16의 설계를 현재 코드와 대조한 전체 구현은 [통합 구현 PLAN](../10-15/2026-10-15_MAINAPP_G05_G16_INTEGRATED_IMPLEMENTATION_PLAN.md)을 따른다. 추가·교체 위치, 단계별 파일 전문, 생성·차용·종료 관계와 프로젝트 등록을 한 파일에서 읽는다. 제품 소스는 G04까지 반영된 상태이며 G05~G16은 문법 검증한 미적용 후보다.

작성일: 2026-10-14. 범위는 `CMainApp`의 모든 구현 파일과 직접 연결된 UI·도구·Sound·Input 소비자다.
LostArk 전체 알고리즘을 재작성하는 계획이 아니다. 현재 브랜치의 작업본을 조사했으며 다른 세션의 변경은 보존한다.

## G01~G04 현재 반영과 이 문서의 역할

사용자가 작성하던 `Update_RuntimeWindowShortcuts`와 `Sync_RuntimeWindowShortcutKeys`를 완성하고
Update의 실제 호출에 연결한다. G01의 적용 후 H/CPP 전문은
[창 단축키 PLAN](2026-10-14_MAINAPP_WINDOW_SHORTCUTS_PLAN.md), 실행한 검증과 남은 화면 확인은
[창 단축키 RESULT](2026-10-14_MAINAPP_WINDOW_SHORTCUTS_RESULT.md)에 기록한다.

이 문서는 전체 책임의 목적지·수정 파일·함수·상태·의존 순서·완료 기준을 정한 설계 정본이다.
G02는 2026-10-15 00시대에 소스 반영과 Debug x64 Product 빌드·링크를 확인했다.
실행·수동 청취는 미검증이며 [G02 RESULT](2026-10-14_MAINAPP_G02_UI_SOUND_RESULT.md)에 남은 경계를 기록한다.
G03은 사용자 구현을 검토·보정했으며 Debug Product 빌드·링크와 실제 Sound Manager의 반환값 검사 9건을 통과했다.
청취·UI 조작은 미검증이며 [G03 RESULT](../10-15/2026-10-15_MAINAPP_G03_SOUND_PREPARATION_RESULT.md)에 구분했다.
G04 손 무기 소켓 편집은 기존 BossTool로 제품 소스 반영을 마쳤다. 검증은 [G04 RESULT](../10-15/2026-10-15_MAINAPP_G04_VALTAN_AXE_RESULT.md)에 기록한다. G05~G16은 통합 PLAN에 전체 미적용 후보를 작성했으며 구 G06-a는 통합 G06에 포함됐다.
각 G의 H/CPP 전문과 project/filter 등록을 구현 PLAN으로 확정한다.
이 문서의 제안 클래스명은 현재 존재하는 클래스와 구분한다.
아래 부록은 현 `MainApp*.cpp` 멤버 정의를 전수 분류한다. 비멤버 helper·lambda는 별도 수량이며,
`Initialize`, `Update`, `Render`, `Free`, `EnsureDebugTool`, `Update_CombatHUD`의 내부 혼합은 별도로 분해한다.

## 최종 MainApp의 책임

- 장치와 Client 상위 소유자를 준비하고 실패한 초기화를 정리한다.
- Level 전환을 기존 안전한 프레임 경계에서 적용한다. 실제 `Change_Level` 호출자는 계속 MainApp이다.
- 한 프레임의 UI 입력 중재·수신 dispatch·Engine 갱신·표현 준비·렌더 제출 순서를 읽을 수 있게 유지한다.
- 종료 시 비동기 작업·차용 callback·도구·표현·Engine의 정리 순서를 확정한다.

도끼 소켓 JSON, 용 속도 draft, 재련 결과 표현, UI 음원 파일명, 도구 검색 목록은 각 기능 소유자로 이동한다.
기능별 CPP의 이름만 바꾸거나 필터만 옮겨 `CMainApp::` 정의를 계속 남기는 것을 책임 분리 완료로 보지 않는다.

## 현재 구조에서 그대로 활용할 것

| 기존 경계 | 계속 맡는 책임 |
|---|---|
| `CGameInstance`와 Engine manager | 장치·입력·사운드·객체·Level·렌더러·Profiler의 범용 관리 |
| `CValtan` → `CPart_Equipment` | 실제 도끼 모델·소켓 변환·그리기 |
| `CCharacter` → `CPart_Vehicle` | 실제 탈것 모델·애니메이션·탑승 표현 |
| `CPlayerController` / `IPlayerCommandSink` / 기존 Network service | 명령 제출과 Server 결과 수신 |
| `CUIInputRouter` / `CUITextOcclusion` / 기존 View | 클릭 점유·문자 입력·텍스트 가림·각 화면 표현 |
| `CCombatHUDViewModel` | Server가 복제한 UI 입력 데이터. 새 HUD 소유자에 HP/인벤토리 정본을 복제하지 않음 |
| `CLevelTransitionService`, `CLobbyCommandService`, `CCharacterRoster` | 전환 요청·승인과 process-session 캐릭터 상태 |
| 기존 `CSoundCueCatalog`, WorldSequence/보스 재생기 | 캐릭터·보스 cue 선택과 각 재생 시계·sound handle 수명 |
| 기존 Map/WorldScene 편집 세션·publisher | stable placement ID, draft, freshness, 저장·게시 |

## 프레임과 수명의 고정 조건

1. 전 프레임 Level 요청은 새 Level의 첫 Update/Late_Update와 Render보다 먼저 반영한다.
2. UI가 gameplay 입력 차단을 결정한 뒤 gameplay가 입력을 소비한다.
3. Network dispatch가 새 snapshot을 전달한 뒤 Engine/Level이 갱신된다.
4. 객체 갱신 뒤 Effect spawn commit·follow anchor·공용 group 시계를 기존 순서로 진행한다. 중복 tick하지 않는다.
5. 최종 카메라를 사용하는 HUD 투영, world 렌더, 텍스트/ImGui 제출 순서를 유지한다.
6. `CUIInputRouter::End_Frame`은 Render 끝이다. 함수 추출을 이유로 Update 끝으로 옮기지 않는다.
7. Profiler의 컷신 Begin/End는 실제 world draw를 감싼다. 창 표시와 capture 수집은 다른 조건이다.
8. 숨겨진 도구도 진행 중 Save/Publish·승인 대기·필수 결과 큐 처리를 계속한다.
9. 창 생성 순서는 STATIC sprite의 draw order와 연결돼 있다. 소유자 추출 중 생성 순서를 바꾸지 않는다.
10. 도구를 받는 callback에 `CMainApp*` 전체를 다시 넘기지 않는다. 입력 값·현재 controller 차용·typed 요청으로 의존을 좁힌다.
11. 새 소유자의 종료는 callback 해제와 background job 정리까지 포함한다. 기존 UI를 무조건 Engine 전/후로 몰아 해제하지 않고 실제 Layer/Component 소비를 확인한다.
12. 팀장 렌더링 옵션·sound volume·Server 권위·게시 데이터 값은 구조 변경의 부수 효과로 바꾸지 않는다.

## G02 UI 사운드 선택과 MainApp 역참조 제거

반영 후 실제 24개 H/CPP 전문과 등록 위치는 [G02 UI 사운드 PLAN](2026-10-14_MAINAPP_G02_UI_SOUND_PLAN.md)에 있다.
구 static 선언·정의·호출은 소스에 0건이며 클릭 107곳·팝업 5곳·네 창 4곳이 새 함수에 연결됐다.
Debug x64 Product 컴파일·Client 링크는 통과했고 실제 청취는 미검증이다.
근거는 [G02 RESULT](2026-10-14_MAINAPP_G02_UI_SOUND_RESULT.md), 전체 그림은 [G00 구조 지도](2026-10-14_MAINAPP_G00_ARCHITECTURE_MAP.md)를 따른다.

반영된 목적지는 Client `UISoundPlayer.h/.cpp`의 작은 cue 경계다. 첫 범위는 `BUTTON_CLICK`,
`WINDOW_OPEN`, `WINDOW_CLOSE`, `POPUP_REQUEST` 네 의미다. singleton이나 FMOD 객체를 추가하지 않는다.

| 제안 cue | 기존 Resources-relative asset ID | 기존 volume |
|---|---|---|
| `BUTTON_CLICK` | `Sound/UI/Select/ui_default_button_click2__59426200.wav` | 1.f |
| `WINDOW_OPEN` | `Sound/UI/Select/ui_inventory_show1__669750910.wav` | 1.f |
| `WINDOW_CLOSE` | `Sound/UI/Select/ui_inventory_hide1__7273537.wav` | 1.f |
| `POPUP_REQUEST` | `Sound/UI/System/sys_party_request1__654410772.wav` | 1.f |

- `MainApp::Play_UIButtonClickSound`, `Play_PopupRequestSound`와 I/P/N/M의 동일 open/close WAV 선택을 이 경계로 옮긴다.
- 네 cue의 Resources-relative asset ID와 기존 volume을 한곳에서 관리한다. View는 음원 파일명을 받지 않는다.
- `CRuntimeAssetRoot::Resolve → CGameInstance::Play_Sound → CSound_Manager`의 기존 경로를 사용한다.
- UI helper 참조는 조사 시 MainApp 포함 21개 CPP에 있다. 검색으로 모든 호출자를 교체하고 MainApp을 다른 이유로 쓰는 파일은 include를 보존한다.
- `PartyInteractionView`의 중복 팝업 WAV도 같은 cue로 연결한다.
- 현재 CharacterSoundCatalog에는 해당 UI cue가 없다. 첫 분리에 새 JSON·publisher·별도 asset cache를 도입하지 않는다.
- 호출 횟수·선택 WAV·volume을 유지한다. 음원 실패는 해당 효과음만 생략하고 버튼/창 동작은 유지한다.

직접 대상: `Client/Private/MainApp.cpp`, `Client/Public/MainApp.h`, UI helper의 전수 호출자,
`PartyInteractionView.cpp`, 새 UI sound H/CPP, Client project/filter.
완료 증거: 외부 `CMainApp::Play_UIButtonClickSound/Play_PopupRequestSound` 호출 0,
각 cue의 매핑·기존 동작 비교, 변경 TU compile 및 Client link. 실제 청취는 사용자 확인이다.

## G03 UI 음원 준비와 Engine 재생 계약

G03 소스는 반영됐다. 현재 7개 H/CPP 전문과 반영 위치는
[G03 음원 준비 PLAN](../10-15/2026-10-15_MAINAPP_G03_SOUND_PREPARATION_PLAN.md)에 있다.

현재 `CSound_Manager::Initialize`는 FMOD System·channel group·focus mute를 준비한다.
WAV는 `Play_Sound → Find_Or_LoadSound`의 cache miss에서 읽으며 `(path, loop)` cache를 재사용한다.
따라서 매번 파일을 새로 읽는 구조는 아니지만, 첫 클릭에서 최초 적재가 일어날 수 있다.

현재도 `Loader::Ready_EstherSummonPresentation → CEstherActionSoundCueDocument::Preload_Sounds →
Get_SoundDurationMs`가 Sound cache를 미리 채운다. 길이 조회가 적재까지 수행하는 사실과
범용 공개 Preload/Register API가 없다는 사실을 구분한다.

선택한 방향:

- 공통 UI 네 cue는 모든 Level에서 쓰므로 Client 시작의 UI 준비 단계에서 한 번 준비한다.
- Engine의 기존 Sound Manager에 `Prepare_Sound(path, loop)`라는 명시적인 준비 진입점을 추가하고
  기존 `Find_Or_LoadSound`로 위임한다. 같은 자원 cache를 사용한다. Client가 목록·실패 정책을 정한다.
- 준비는 장치 준비 뒤, UI 입력이 시작되기 전의 직렬 구간에서 수행한다. FMOD 제어를 별도 worker로 옮기지 않는다.
- 소리 하나가 누락되면 그 cue의 오류를 기록하고 기존 UI 동작은 유지한다. 전체 UI 실패로 확대하지 않는다.
- `Sound/UI/` 문자열로 group을 판정하는 기존 Engine 결합은 명시적 category를 받는 기존 재생 API 확장으로 줄인다.
  기존 호출자의 기본 동작은 보존하고 네 UI cue부터 `INTERFACE`를 전달한다.
- 음원 전체 선적재·새 cache·eviction/streaming은 이 G의 범위가 아니다. 첫 클릭 지연이나 메모리 개선은 아직 측정하지 않았다.

직접 대상: `Engine/Public/Sound/Sound_Manager.h`, 대응 CPP, `Engine/Public/GameInstance.h`, 대응 CPP,
Client UISoundPlayer와 시작 준비. Engine/Client compile·SDK 갱신·소비자 link를 함께 확인한다.
검증은 준비 실패 격리, 동일 cache 재사용, category volume/focus mute 보존이다.
Resources 원본 WAV는 이동·재추출하지 않는다.
G03은 Engine API까지 다루는 독립 개선이다. G02의 cue 분리 뒤 별도 검증하며 G04 이후 책임 이동의 선행 조건으로 삼지 않는다.

준비 위치는 자원 수명으로 정한다. FMOD 장치·그룹은 Engine 초기화, 모든 화면의 공통 UI cue는
Client UI 시작 준비, 특정 Level/캐릭터의 cue는 기존 Loader·catalog 준비 흐름에 둔다.
Sound_Manager::Initialize에 LostArk의 모든 WAV 목록을 넣거나 UI 네 cue를 매 Level마다 다시 등록하지 않는다.

## G04 ValtanAxe를 발탄 편집 도구로 이동

G04는 제품 소스에 반영했다. 변경 위치·전체 반영 코드는 [G04 PLAN](../10-15/2026-10-15_MAINAPP_G04_VALTAN_AXE_PLAN.md), 실제 검증과 수동 확인 경계는 [G04 RESULT](../10-15/2026-10-15_MAINAPP_G04_VALTAN_AXE_RESULT.md)에 있다.

기존 `MainApp_ValtanAxe.cpp`가 맡던 것은 손 무기 소켓 편집·정본 Save·Publish UI다. 서버가 생성하는 낙하 도끼 Pattern과는 별개다.
기존 `CValtanBossTool`은 이미 `CBalanceTool*`를 사용하므로 그 안의 소켓 편집 기능으로 흡수한다.
구현 TU는 `ValtanBossTool_Axe.cpp`로 나누고, 편집 상태는 BossTool이 명시적으로 소유한다.

- `RenderValtanAxeEditor`, 본체/유령 `AXE_DRAFT[2]`, 선택 인덱스, Load/Save·serialize·writer lock을 함께 옮긴다.
- UI draft·선택의 함수-local static은 멤버로 옮긴다. 임시파일 이름의 PID와 함께 쓰는 `static atomic serial`은 프로세스 전체 고유 순번이므로 편집 객체마다 0으로 초기화하지 않는다.
- 현재 `RenderValtanArenaControls → RenderValtanArenaStateControls`가 BalanceTool과 BossTool을 먼저 lazy 생성한 뒤 도끼 편집 함수를 호출한다. 새 호출은 준비된 BossTool의 editor로 연결하며 별도 창 Open이나 graph 로드를 추가하지 않는다.
- BalanceTool은 MainApp의 unique_ptr 소유이며 BossTool의 raw pointer는 차용이다. 기존 Save 안의 make_unique 대입을 그대로 옮기지 않는다. host가 먼저 준비·전달하고 editor는 null이면 게시 시작 실패를 표시한다.
- preview는 기존 `CActorCatalog::Set_ValtanWeaponSocketTransform → CValtan → CPart_Equipment`를 사용한다.
- 같은 필드 충돌, 최신 디스크 재읽기, 무관한 축 보존, 임시 파일 검증·백업·원자적 교체를 유지한다.
- 저장 성공, 게시 시작/완료, 실행 중 Server 적용은 서로 다른 상태다. 저장 성공·게시 시작 실패의 baseline과 메시지를 보존한다. 기존 BalanceTool job Update는 창 표시와 무관하게 수행된다.
- 현재 Free는 BalanceTool을 BossTool보다 먼저 해제한다. 새 BossTool/editor 종료가 차용 publisher를 참조한다면 BossTool을 먼저 정리하도록 순서를 함께 고친다. 기존에 없던 preview 자동 복원은 추가하지 않는다.

직접 대상: `MainApp.h/.cpp`, `MainApp_ValtanAxe.cpp`, `ValtanBossTool.h/.cpp`, 새 분리 CPP와 Client project/filter.
완료 시 MainApp의 도끼 선언·정의·draft가 사라지고 옛 TU 등록을 제거한다.
도끼 부착·모델·판정 코드와 BossCatalog 값은 이 구조 변경에서 수정하지 않는다.
현재 도끼 패널에는 _DEBUG 제한이 없고 Release F1에도 연결돼 있다. 기존 팀 문서의 Release 저작 범위와
현재 소스의 차이는 별도 판정 항목이며 G02/G03 또는 G04 구조 추출을 이유로 임의로 노출 조건을 바꾸지 않는다.
옛 TU를 새 `ValtanBossTool_Axe.cpp`로 등록할 때 현재 항목의 `/utf-8`, PCH `NotUsing`,
`ForcedIncludeFiles` 비움 metadata를 보존한다. 이는 G02 신규 TU의 기본 PCH/charset과 다른 기존 파일 계약이다.

근거: [호출·lazy 준비](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:12959),
[BalanceTool 차용](C:/Users/tnest/Desktop/LostArk/Client/Public/ValtanBossTool.h:268),
[현재 Free](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:15405),
[손 무기 편집 TU](C:/Users/tnest/Desktop/LostArk/Client/Private/ValtanBossTool_Axe.cpp).

## G05 Dragon·자유 카메라 개발 패널 분리

정확한 변경 위치·코드와 적용 후 전문은 [G05 PLAN](../10-15/2026-10-15_MAINAPP_G05_VEHICLE_DEBUG_PANEL_PLAN.md)에 있다. 제품에는 미적용이다.

- 새 `CVehicleDebugPanel`이 Dragon 속도 draft, baseline, publisher process HANDLE, 상태와 Poll을 소유한다.
- 탑승은 현재 `CPlayerController::Request_VehicleRiding` 경로로 제출한다. 현재 controller/camera는 프레임 동안 차용한다.
- 실제 `CPart_Vehicle`, Server 이동, `CCamera_Free`의 계산은 기존 경로에 둔다.
- 자유 카메라 속도 UI는 작은 `CameraDebugControls`로 분리한다. 기존 `CCameraTool`의 보스 cinematic 저작 전체를 Release에 열지 않는다.
- 현재 Poll은 `RenderDragonControls` 안에서만 실행된다. Dragon tree 접기에는 영향을 받지 않지만 F1 숨김·ImGui Begin false에서는 호출되지 않는다. 표시와 무관한 Update로 Poll을 옮기는 것은 의도한 수명 개선이며 기존 동작을 그대로 보존하는 이동과 구분해 검증한다.
- 창 숨김은 패널 객체를 파괴하지 않아 draft·진행 중 작업을 유지한다. HANDLE 소유 패널은 복사를 금지한다. 종료는 관찰 handle의 CloseHandle이며 외부 publisher 취소·종료와 다르다.
- Dragon 속도 Save는 baseline 충돌 검사와 기존 publisher를 유지한다.

직접 대상: `MainApp_Dragon.cpp`, MainApp 선언·F1 호출·정리, 새 패널/controls H·CPP,
Client project/filter. MainApp 전체 포인터를 패널의 생성 인자로 전달하지 않는다.
검증: Debug Dragon와 Debug/Release 자유 카메라 조건, follow 제한·presentation override, Save 실패 보존·중복 실행 차단,
숨김 중 완료 회수, 종료 시 handle 정리와 외부 작업의 계속 실행을 구분한다.
기존 Dragon CPP는 CP949이며 새 H/CPP는 UTF-8 BOM 없음 규칙을 적용한다.
근거: [현재 render 내부 Poll](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_Dragon.cpp:217),
[HANDLE 소멸 처리](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_Dragon.cpp:60).

## G06 개발 도구 공통 소유자

먼저 상태 없는 Debug live overlay만 기존 CProfilerTool로 옮기는
[G06-a PLAN](../10-15/2026-10-15_MAINAPP_G06A_PROFILER_OVERLAY_PLAN.md)을 준비했다.
이 후보의 Debug/Release CPP 3개씩 문법 검사는 통과했으며 제품 미반영이다. 아래 공통 소유자 전체를 완료했다는 뜻은 아니다.

새 `CClientDeveloperTools`에 도구 unique_ptr, 표시/focus/input owner, lazy 준비,
공유 preview 점유, 자원 검색·열기 요청을 묶는다. 각 도구의 내부 문서와 알고리즘은 기존 클래스에 둔다.

- `ReadyDebugTools`, `EnsureDebugTool`, `CloseAllDebugTools`, `UpdateDebugToolShortcut`,
  `RenderDeveloperTools`, `RefreshDebugResourceFiles`, `OpenDebugResourceFile`을 중심으로 이동한다.
- `EnsureDebugTool`의 `[this]` callback과 공용 preview backend 차용을 새 소유자의 수명에 맞춰 연결/해제한다.
- G04/G05를 이 공통 소유자에 연결한다. 먼저 분리한 패널의 상태를 다시 MainApp 멤버에 복제하지 않는다.
- UI focus/viewport input owner는 단일 소유자로 유지한다. 도구마다 독립 전역 flag를 만들지 않는다.
- 기존 Release F1/F7 허용 기능과 Debug 전용 저작 범위를 보존한다.

Profiler 측정 데이터는 Engine `CProfiler`, 캡처/저장 상태는 기존 `CProfilerTool`에 둔다.
MainApp의 `_Profiler` 표시/설정/F7 연결을 공통 host로 흡수하되 world draw의 계측 위치는 유지한다.
노란 FPS 표시와 frame limiter는 F7 창 표시 조건에 묶지 않는다.

## G07 렌더링 저작 도구와 제품 환경 분리

새 `CRenderingWorkbench`는 `MainApp_RenderingLighting.cpp`의 저작 draft·선택·filter·preview 시계,
`CAreaLightAuthoringSession`, 이전 preview profile과 복원 상태를 소유한다.
`RenderRenderingWorkbench`, `RenderLightingWorkbench`, `RenderLightDetail`,
`StageMapLightPreview`, `StartLightingPreview`, `StopLightingPreview`, `RenderLightSequencer`를 옮긴다.

`CRenderingProfileService`와 `CLightResourceCatalog`는 Level 전환·제품 환경·Kouku presentation도 사용한다.
따라서 도구의 독점 소유로 바꾸지 않고 제품 수명을 유지하며 도구가 차용한다.
창 닫기·Level 변경·취소 시 현재도 자신이 소유하는 override만 복원한다.
기존 benchmark 측정기는 재사용하며 팀장의 FXAA/fog/조명 저장값은 바꾸지 않는다.

## G08 WorldLevel·WorldScene·Map 편집 연결

- `MainApp_WorldLevel.cpp`의 표시·검사·pick 상태는 기존 `CWorldLevelTool`/`CWorldSceneTool`로 옮긴다.
- `Debug_PrepareMapToolAuthoring`, `Debug_PrepareWorldPlacementEditing`, `Debug_OpenWorldObjectDetails`의
  도구 간 양도는 작은 `CWorldAuthoringHost`로 묶고 기존 typed request와 편집 세션을 사용한다.
- `MapTool.cpp`와 `WorldSceneTool.cpp`의 `CMainApp::Get_Active()` 역호출도 같은 변경에서 교체한다.
- Level-owned map/Deploy, placement stable ID, 부분 로드 원본 보존, Save freshness와 publisher를 재사용한다.
- pick은 선택과 상세 정보만 갱신한다. 명시적인 편집·재생에서 기존 preview 경로를 사용하며 Server 파괴 상태는 변경하지 않는다.

검증: miss·취소·Level 세대 변경·UI 클릭 점유, 편집 양도 실패 시 기존 draft 보존,
Save/Publish 동작과 미로드 행 보존. 새 map runtime이나 별도 저장 세션을 만들지 않는다.

## G09 SequenceViewer와 Raid audition 조율

`MainApp_SequenceViewer.cpp`에는 목록 UI뿐 아니라 Kouku `ROOM_PLAYER_ARRIVAL` cue 전송이 섞여 있다.
Viewer와 arrival은 현재 같은 request 번호를 쓰고 `UpdateSequenceViewer`가 두 arena 응답 큐를 비우며
arrival 응답을 먼저 분배한다. 두 객체가 같은 큐를 각각 Consume하게 만들면 응답을 잃는다.

먼저 하나의 `CSequencePlaybackCoordinator`에 request ID 발급·world generation·결과 dispatch·timeout을 둔다.
Viewer 목록/검색/선택은 UI로, arrival cue 진행은 해당 composition preview 세션으로 이동한다.
기존 MapTool/BossTool/typed sink와 재생기를 사용한다.

그다음 MainApp의 CompletePlay 선택·자원 준비·Server 승인 대기·run epoch·취소 상태를
기존 audition service를 조율하는 Client 세션 객체로 옮긴다. 공유 선택을 사용하는 Animation/Camera/Effect/
ValtanWorkbench/Boss/Map/HUDLayout 소비자도 좁은 요청 경계로 교체한다.
Release 제품 응답·리소스 준비는 Debug 창의 표시 수명에 묶지 않는다.

검증: 요청번호 유일성, 늦은 응답/이전 world 거부, 한 cue 한 번 제출, 취소·owner 변경·Level 전환 정리,
Server 승인 전에 제품 상태를 commit하지 않는 조건. Viewer UI 분리와 응답 소유자 변경은 별도 작은 커밋으로 진행한다.

## G10 제품 창 관리와 명령 연결

새 `CClientUIManager`는 Inventory/Repair/Shop/CharacterInfo/Avatar/Vehicle/Honor/WorldMap/SystemOption/
Chat/Party 창의 생성·열기·닫기·Escape 순서·Loading 정리·입력 점유·텍스트 가림을 맡는다.
G01의 키 상태와 두 helper도 이 소유자로 옮긴다.

먼저 `Update_CombatHUD`에 섞인 창 Update와 수리/구매/장착/탑승/칭호 요청을 분리한다.
이 함수를 그대로 HUD 소유자로 이동하지 않는다. 실제 `m_iNextUseItemSequence`는 여러 요청이 공유하므로
각 클래스가 같은 초기값의 sequence를 독립 발급하지 않게 한 명령 제출 소유자를 유지한다.
기존 controller/service와 Server snapshot을 사용하고 새 네트워크 연결은 만들지 않는다.

Bern NPC/menu의 `CMainApp::Get_Active()` 호출은 typed UI 열기 요청으로 바꾼다.
이행 중 기존 MainApp public 함수는 새 소유자에 위임한 후 모든 소비자를 전환하고 제거한다.
생성 순서와 겹침 순서, 창을 닫을 때 drag/capture 취소, 실패 시 기존 화면 유지까지 검증한다.

`Sync_CinematicUI`에는 Arena 캐릭터 표시 동기화도 섞여 있으므로 통째로 UIManager에 넣지 않는다.
장면 표시 동기화는 G15의 표현 소유자로, 공유 화면 정책 판단은 작은 읽기 결과로,
입력 차단·gesture 취소는 UI 소유자로 나눈다. 기존 호출 순서를 보존하며 미니게임 HUD 숨김과
boss bar 숨김을 하나의 bool로 합치지 않는다. 관련 `screen_policy` 네 함수는 이 경계로 함께 정리한다.

## G11 재련 UI 상태와 사운드 수명

새 `CItemUpgradeController`로 `Open/Hide/Update_ItemUpgrade`, 선택/성장/재련 버튼,
Server 결과 대기·timeout·확인 클릭·결과 표시·게이지와 텍스트를 함께 옮긴다.
실제 강화 단계·재화는 Server 결과가 정본이며 local 성공 판정을 만들지 않는다.

WAITING 반복음, 성공/실패 variant 선택과 종료는 이 표현 상태의 owner가 맡는다.
닫기·timeout·전환·종료에서는 이 기능이 시작한 WAITING 반복음을 정리한다.
성장·성공·실패 one-shot은 현재 추적 handle 없이 재생되므로 기존 자연 종료를 보존한다.
창을 닫을 때 one-shot까지 중단하려면 handle 계약과 동작을 바꾸는 별도 변경으로 다룬다.
현재 일반 반복음은 Engine의 한 슬롯을 사용한다. 독립 반복음 여러 개의 요구 없이 새 channel manager를 만들지 않는다.
향후 중첩 요구가 생기면 기존 Sound Manager의 owner handle 경로를 확장한다.
정상 제출/거부/timeout/닫기/결과 재입력에서 UI 상태와 반복음이 남지 않는지 확인한다.

## G12 HUD 표시와 퀵슬롯 입력

새 `CCombatHUDPresenter`에는 skill/gauge/cooldown/buff/health/damage/Esther/minimap의 표시 상태를 모은다.
입력은 `CCombatHUDViewModel`의 read-only view이며 flash·이전 stance·보간 시간 같은 표현 이력만 소유한다.
HP·inventory·cooldown 권위를 다시 저장하는 두 번째 gameplay 모델을 만들지 않는다.
실제 `m_PlayerNameplateView`는 현재 각 Level이 소유하므로 그대로 유지한다. MainApp의 투영 helper 사용을
근거로 nameplate의 수명 전체를 HUD 소유자로 옮기지 않는다.

퀵슬롯 drag·item/vehicle 단축키·조준 요청은 G10의 UI 명령 경계로 남긴다.
최종 카메라 뒤 world bar projection과 UI 합성 뒤 텍스트 제출을 단계별로 호출한다.
던전 timer의 함수-static tick/deadline/age는 presenter의 명시 상태로 바꾸되 reset 조건을 확인한다.
일반 layout 저작은 기존 HUDLayoutTool의 저장 경로를 사용한다.
MainApp의 `Save/Reload_HealthBarPositions`는 별도의 `KoukuHudModes.json` 필드 병합 경로이므로
관련 HP/기믹 위치·배율 상태, Saved baseline, `.madness-position.lock`, freshness·최신 저장본 병합을 함께 옮긴다.
두 저장 계약을 새 공통 Save로 합치지 않는다.

## G13 Lobby·캐릭터 선택·세션 복귀

Lobby 화면 조율은 새 `CLobbyPresenter`, 캐릭터 복귀 capture 대기는 별도의 작은 flow 상태로 분리한다.
배경·서버 목록 선택·로고 시간·CharacterSelect view는 화면 수명에,
pending capture sequence/world generation·timeout·복귀 후 열기 예약은 전환 수명에 둔다.
`CCharacterRoster`, `CCharacterSelectionState`, `CLobbyCommandService`, `CLevelTransitionService`는 재사용한다.
생성 identity commit, 서버 상태 capture/restore, 실패 시 기존 캐릭터 보존, 최초 빈 Lobby의 lazy 준비를 유지한다.
실제 Level 교체는 MainApp의 안전한 시점에서 계속 수행한다.

## G14 입력 샘플과 UI/gameplay 소비 정리

입력 관리자는 이미 `CInput_Device`로 존재하며 GameInstance가 소유한다.
현재 UI는 Engine Update보다 먼저 처리되고, `CInput_Device::Update`는 `Update_Engine` 내부에서 수행된다.
지금 `GetAsyncKeyState`를 단순히 `Get_DIKeyPressed`로 바꾸면 이전 프레임 snapshot 또는 UI 차단 결과를 읽을 수 있다.

이 단계는 기계적 API 치환과 구분해 진행한다.

- 기존 입력 샘플을 프레임 시작에 한 번 확보하는 진입점으로 분리하고 Engine Update의 중복 샘플을 없앤다.
- 키보드 UI는 raw snapshot을, gameplay는 UI 소비/차단이 적용된 동일 프레임 키보드 입력을 사용한다.
- 마우스는 별도 다음 단위로 다룬다. 현재 `Get_DIMouseStateRaw`가 호출마다 `GetAsyncKeyState`를 읽으므로
  Update 이동만으로 버튼 snapshot이 고정되지 않는다. 버튼을 한 번 샘플하고 UIInputRouter와 PlayerController의
  소비를 함께 연결한다. 커서의 `GetCursorPos`, 휠·문자의 WndProc 큐는 각각의 좌표·누적·소비 계약을 유지한다.
- virtual-key와 DIK scan code를 같은 숫자로 취급하지 않고 명시 매핑한다.
- 현재 MainApp의 같은 process 보조 창 허용과 Input_Device의 메인 HWND focus 조건을 소비자별로 유지한다.
  하나의 raw API로 합치기 위해 gameplay를 보조 창에서도 활성화하지 않는다.
- focus 재획득 시 눌러 둔 키 release gate, text input, ImGui 보조 viewport, cinematic 억제를 확인한다.

입력 정리는 Engine/SDK/Client와 사용자의 키·마우스 재현을 함께 검증하는 별도 G다.
G01에서는 현재 입력 정책을 변경하지 않는다. G14는 다른 책임 이동을 막는 선행 조건이 아니다.

## G15 제품 장면·보스·Effect 표현

관문/생성 화면 profile·캐릭터 shadow focus·subtitles·보스 presentation 연결은
새 `CScenePresentationController` 또는 해당 기존 Level/표현 service의 적합한 소유자로 옮긴다.
저작 RenderingWorkbench의 draft와 제품 환경 적용을 분리한다.

- `m_RenderingProfiles`, light resources의 제품 수명을 유지한다.
- `CKoukuSaydonPresentationPlayer`, 기존 Effect presentation/runtime을 재사용한다.
- Level 진입/이탈 시 차용 resolver와 effect authoring player 연결을 정리한다.
- Effect spawn commit/follow anchor/free-group 시계를 중복 실행하지 않는다.
- MainApp에 남은 environment 선택을 읽기 입력으로 만들고 적용 알고리즘은 기존 service를 사용한다.

완료 기준은 특정 보스/캐릭터의 세부 처리 변경이 MainApp의 프레임 조율 변경을 요구하지 않는 것이다.

## G16 프레임 셸·종료·프로젝트 표시 마무리

앞선 소유권 이동 뒤 Initialize/Update/Render/Free를 단계 호출 중심으로 정리한다.
처리 순서를 숨기는 임의의 모든-system Update 루프를 만들지 않는다.
유일한 Level 전환 commit, 입력 차단 확정, network dispatch, final camera, render/text/UI End_Frame은 명시한다.

- 각 소유자의 Initialize 실패·Shutdown·background job·callback 해제 순서를 확인한다.
- public `CMainApp::Get_Active` 및 남은 위임 API의 직접 소비자가 없어졌는지 전수 검색한다.
- 역할을 이관한 `MainApp_*` TU의 project 등록을 제거하고 새 파일만 적합한 기존 filter에 등록한다.
- 물리 `Public/Private` 경로와 project/filter 항목을 함께 확인한다. 무관한 필터는 재배치하지 않는다.
- include는 실제 사용자를 기준으로 줄이며 PCH가 우연히 제공하는 정의에 기대지 않는다.
- C++ 파일별 인코딩·BOM·개행을 보존한다. UTF-8 파일을 CP949로 컴파일하는 기존 경계는
  별도 소비자 include 감사와 build option 변경 단위로 다룬다. 주석만 옮겨도 전처리 결과를 확인한다.

## 파일과 등록의 적용 방식

새 파일은 기존 `Client/Public/<역할>.h`, `Client/Private/<역할>.cpp` 물리 구조를 사용한다.
새 H/CPP마다 Client.vcxproj의 ClInclude/ClCompile 및 filters의 대응 항목을 같이 추가한다.
기존 도구의 분리 CPP만 추가하는 G04는 `ValtanBossTool.h`를 확장하고 새 H를 억지로 만들지 않는다.
직접 CPP를 포함하는 source-contract/harness가 있으면 해당 owner의 등록 파일 집합을 같이 갱신한다.

| 변경 파일군 | 대응 표시 위치 |
|---|---|
| MainApp, ClientDiagnostics | 00.MainApp의 앱 진입/종료 역할 유지 |
| UISoundPlayer, 제품 창·HUD·Lobby·재련 | 기존 제품 UI 관련 filter 아래 역할별 등록 |
| ValtanBossTool_Axe | 기존 Boss Tools |
| VehicleDebugPanel, CameraDebugControls | 개발 도구/카메라 관련 filter |
| RenderingWorkbench | 기존 Rendering Authoring |
| WorldAuthoringHost | 기존 Map WorldTools |
| SequencePlaybackCoordinator | 기존 Sequencer의 연결/세션 영역 |
| 실제 도끼/용 파츠 | 기존 GameObjects 위치 유지 |

정확한 XML 항목은 각 G의 실제 파일 생성 목록과 함께 구현 PLAN에 싣는다.
GameObject로 옮길 내용은 실제 객체의 생성·갱신·표현이고, editor Save/Publish는 Tools 역할이다.

## 진행 순서와 완료 판정

책임 이동의 기본 순서는 G01 → G02 → G04 → G05 → G06이다.
G03의 Sound 준비/category 개선은 G02 이후 독립 변경으로 검증한다.
그 뒤 G07/G08/G09는 도구 경계, G10/G11/G12/G13은 제품 UI 경계로 각각 작은 단위로 진행한다.
G14는 입력 순서 변경을 단독 검증한 뒤, G15/G16에서 앱 셸과 수명을 마무리한다.
같은 파일을 동시에 고치는 단계는 병렬 적용하지 않는다.

각 G는 다음 증거로 완료를 판정한다.

- 옮긴 함수뿐 아니라 관련 지속 상태·실패 정리·직접 호출자가 목적지로 이동함.
- 이전 MainApp 구현/역참조/중복 runtime이 남지 않음.
- 변경한 TU와 실제 소비자 compile/link, 필요한 Debug/Release 조건 확인.
- 새 project/filter와 변경 JSON/XML parse, 변경 diff의 whitespace 확인.
- 입력·저장·취소·전환·종료의 관측과 사용자의 화면/청취 판정을 별도로 기록함.

파일 수나 MainApp 줄 수 감소만으로 완료를 주장하지 않는다. 컴파일 성공은 화면·소리 성공과 다르다.
G01 이후 코드는 아직 적용하지 않았으며 원래 소스와 사운드/렌더링 데이터는 보존한다.

## 조사 근거

- `Client/Public/MainApp.h`, `Client/Private/MainApp*.cpp`, Client project/filter.
- `Engine/Public/GameInstance.h`, `Engine/Private/GameInstance.cpp`의 manager 소유와 프레임 순서.
- `Engine/Public/Input_Device.h`, 대응 CPP의 snapshot·focus·release gate.
- `Engine/Public/Sound/Sound_Manager.h`, 대응 CPP의 cache·category·재생 handle.
- `Client/Private/SoundCueCatalog.cpp`, `EstherActionSoundCueDocument.cpp`, `Loader.cpp`의 기존 준비 경로.
- `Client/Private/Valtan.cpp`, `Character.cpp`, `Part_Equipment.cpp`, `Part_Vehicle.cpp`의 실제 객체 경로.
- `Client/Private/WorldSequencePlayer_Objects.cpp`의 owner별 사운드 수명.
- 기존 `09-22_RAID_AUDIO` PLAN/RESULT, `09-29_RELEASE_ENTRY_AUDIO_REPAIR_RESULT`, `10-11_CODE_READABILITY_RESULT`.

<!-- function-inventory:begin -->
## 부록: MainApp 멤버 함수 정의 목록

2026-10-15 G02 반영 후 작업본 기준으로 7개 `MainApp*.cpp` 정의와 줄 위치를 다시 대조했다.
최초 `function-groups.json`의 195개 분류에서 G02가 제거한 두 UI sound 멤버를 제외해 CPP 정의는 193개다.
생성자·소멸자를 포함하며 누락·중복·추가 이름은 0개다. 기존 분류 JSON은 G02 전 조사 기록으로 유지한다.
헤더 inline `Get_Active`는 별도 행이다. 비멤버 helper·lambda 및 함수 내부 처리 블록은 이 집계에 포함하지 않는다.
그룹은 책임 분류이며 혼합 함수 전체를 그대로 새 소유자로 옮기라는 뜻이 아니다.
새 `Play_UISound`는 CMainApp 멤버가 아니므로 이 표에 포함하지 않는다.

| 그룹 | 함수 | 파일:줄 |
|---|---|---|
| app_lifecycle | `CMainApp::CMainApp` | [Client/Private/MainApp.cpp:732](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:732) |
| app_lifecycle | `CMainApp::~CMainApp` | [Client/Private/MainApp.cpp:738](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:738) |
| app_lifecycle | `CMainApp::Initialize` | [Client/Private/MainApp.cpp:1205](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:1205) |
| app_lifecycle | `CMainApp::Create` | [Client/Private/MainApp.cpp:15338](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:15338) |
| app_lifecycle | `CMainApp::Free` | [Client/Private/MainApp.cpp:15348](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:15348) |
| app_lifecycle | `CMainApp::Ready_Fonts` | [Client/Private/MainApp.cpp:10548](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:10548) |
| app_lifecycle | `CMainApp::Ready_Prototype_For_Static` | [Client/Private/MainApp.cpp:10610](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:10610) |
| app_lifecycle | `CMainApp::Ready_Prototype_For_LoadingChrome` | [Client/Private/MainApp.cpp:10656](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:10656) |
| app_lifecycle | `CMainApp::ReadyImGuiRuntime` | [Client/Private/MainApp.cpp:10971](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:10971) |
| frame_shell | `CMainApp::Update` | [Client/Private/MainApp.cpp:2148](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:2148) |
| frame_shell | `CMainApp::Render` | [Client/Private/MainApp.cpp:4166](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:4166) |
| frame_shell | `CMainApp::Limit_FrameRate` | [Client/Private/MainApp.cpp:10502](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:10502) |
| frame_shell | `CMainApp::IsWindowOwnedByCurrentProcess` | [Client/Private/MainApp.cpp:10990](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:10990) |
| session_transition | `CMainApp::Return_ToCharacterSelect` | [Client/Private/MainApp.cpp:754](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:754) |
| session_transition | `CMainApp::Pump_ReturnToCharacterSelect` | [Client/Private/MainApp.cpp:817](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:817) |
| session_transition | `CMainApp::Start_Level` | [Client/Private/MainApp.cpp:10730](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:10730) |
| session_transition | `CMainApp::Apply_LevelRequest` | [Client/Private/MainApp.cpp:10792](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:10792) |
| session_transition | `CMainApp::Render_ColosseumTransferPortrait` | [Client/Private/MainApp.cpp:4092](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:4092) |
| runtime_windows | `CMainApp::Open_SystemOptionsWindow` | [Client/Private/MainApp.cpp:746](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:746) |
| runtime_windows | `CMainApp::Open_ShipWindow` | [Client/Private/MainApp.cpp:847](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:847) |
| runtime_windows | `CMainApp::Open_RepairWindow` | [Client/Private/MainApp.cpp:854](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:854) |
| runtime_windows | `CMainApp::Open_ShopWindow` | [Client/Private/MainApp.cpp:861](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:861) |
| runtime_windows | `CMainApp::Update_RuntimeWindowShortcuts` | [Client/Private/MainApp.cpp:2004](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:2004) |
| runtime_windows | `CMainApp::Sync_RuntimeWindowShortcutKeys` | [Client/Private/MainApp.cpp:2084](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:2084) |
| runtime_windows | `CMainApp::Register_UITextOccluders` | [Client/Private/MainApp.cpp:4050](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:4050) |
| runtime_windows | `CMainApp::Close_RuntimeWindowsForLoading` | [Client/Private/MainApp.cpp:6770](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6770) |
| runtime_windows | `CMainApp::Is_AnyRuntimeWindowOpen` | [Client/Private/MainApp.cpp:6811](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6811) |
| runtime_windows | `CMainApp::Is_EscapeWindowOpen` | [Client/Private/MainApp.cpp:6831](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6831) |
| runtime_windows | `CMainApp::Sync_EscapeWindowOrder` | [Client/Private/MainApp.cpp:6857](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6857) |
| runtime_windows | `CMainApp::Close_TopEscapeWindow` | [Client/Private/MainApp.cpp:6881](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6881) |
| runtime_windows | `CMainApp::Is_EscapeOwnedElsewhere` | [Client/Private/MainApp.cpp:6923](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6923) |
| runtime_windows | `CMainApp::Update_SystemOptionWindow` | [Client/Private/MainApp.cpp:6945](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6945) |
| screen_policy | `CMainApp::Sync_CinematicUI` | [Client/Private/MainApp.cpp:2095](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:2095) |
| screen_policy | `CMainApp::Is_KoukuMinigameHUDHidden` | [Client/Private/MainApp.cpp:6743](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6743) |
| screen_policy | `CMainApp::Is_KoukuBossHealthBarHidden` | [Client/Private/MainApp.cpp:6752](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6752) |
| screen_policy | `CMainApp::Is_RuntimeUIScreenSuppressed` | [Client/Private/MainApp.cpp:6762](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6762) |
| lobby_presentation | `CMainApp::Update_LobbyButtons` | [Client/Private/MainApp.cpp:6611](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6611) |
| lobby_presentation | `CMainApp::Update_CharacterSelectWindow` | [Client/Private/MainApp.cpp:7010](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:7010) |
| lobby_presentation | `CMainApp::RenderCharacterSelectWindowText` | [Client/Private/MainApp.cpp:7071](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:7071) |
| lobby_presentation | `CMainApp::RenderLobbyButtonText` | [Client/Private/MainApp.cpp:7184](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:7184) |
| lobby_presentation | `CMainApp::Load_LobbyServers` | [Client/Private/MainApp.cpp:7307](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:7307) |
| scene_presentation | `CMainApp::Update_KoukuGateSceneProfile` | [Client/Private/MainApp.cpp:928](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:928) |
| scene_presentation | `CMainApp::Update_CustomizingSceneProfile` | [Client/Private/MainApp.cpp:979](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:979) |
| scene_presentation | `CMainApp::Update_ClassShowcaseShadowFocus` | [Client/Private/MainApp.cpp:1014](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:1014) |
| scene_presentation | `CMainApp::RenderCinematicSubtitles` | [Client/Private/MainApp.cpp:10361](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:10361) |
| hud | `CMainApp::Hide_CombatHUD` | [Client/Private/MainApp.cpp:4956](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:4956) |
| hud | `CMainApp::Update_CombatHUD` | [Client/Private/MainApp.cpp:4963](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:4963) |
| hud | `CMainApp::RenderShipHudTexts` | [Client/Private/MainApp.cpp:5537](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:5537) |
| hud | `CMainApp::RenderQuickSlotKeyLabels` | [Client/Private/MainApp.cpp:5604](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:5604) |
| hud | `CMainApp::Find_ActivePlayerController` | [Client/Private/MainApp.cpp:5789](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:5789) |
| hud | `CMainApp::Update_QuickSlotDrag` | [Client/Private/MainApp.cpp:5821](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:5821) |
| hud | `CMainApp::Update_SpecialQuickSlots` | [Client/Private/MainApp.cpp:5873](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:5873) |
| hud | `CMainApp::Update_VehicleHud` | [Client/Private/MainApp.cpp:5903](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:5903) |
| hud | `CMainApp::Update_WaterGunHud` | [Client/Private/MainApp.cpp:6028](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6028) |
| hud | `CMainApp::Resolve_HudCooldownRatio` | [Client/Private/MainApp.cpp:6070](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6070) |
| hud | `CMainApp::Update_SpecialSlot` | [Client/Private/MainApp.cpp:6092](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6092) |
| hud | `CMainApp::Update_BuffBar` | [Client/Private/MainApp.cpp:6160](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6160) |
| hud | `CMainApp::Update_SkillSlotMarks` | [Client/Private/MainApp.cpp:6279](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6279) |
| hud | `CMainApp::Load_HudQuickSlotData` | [Client/Private/MainApp.cpp:6343](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6343) |
| hud | `CMainApp::Load_KoukuHudModes` | [Client/Private/MainApp.cpp:6470](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6470) |
| hud | `CMainApp::Update_KoukuHudMode` | [Client/Private/MainApp.cpp:6537](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:6537) |
| hud | `CMainApp::Update_Minimap` | [Client/Private/MainApp.cpp:7082](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:7082) |
| hud | `CMainApp::RenderMinimapText` | [Client/Private/MainApp.cpp:7176](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:7176) |
| hud | `CMainApp::Update_ItemQuickSlots` | [Client/Private/MainApp.cpp:8166](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8166) |
| hud | `CMainApp::Update_PlayerHealthManaBar` | [Client/Private/MainApp.cpp:8229](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8229) |
| hud | `CMainApp::Update_LanceMasterIdentityGauge` | [Client/Private/MainApp.cpp:8278](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8278) |
| hud | `CMainApp::Update_GuardianKnightIdentity` | [Client/Private/MainApp.cpp:8335](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8335) |
| hud | `CMainApp::Update_SkillCooldowns` | [Client/Private/MainApp.cpp:8429](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8429) |
| hud | `CMainApp::RenderSkillCooldownText` | [Client/Private/MainApp.cpp:8486](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8486) |
| hud | `CMainApp::Hide_BossHealthBar` | [Client/Private/MainApp.cpp:8641](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8641) |
| hud | `CMainApp::Get_HealthBarPositions` | [Client/Private/MainApp.cpp:8668](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8668) |
| hud | `CMainApp::Set_HealthBarPositions` | [Client/Private/MainApp.cpp:8675](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8675) |
| hud | `CMainApp::Get_MechanicBarScale` | [Client/Private/MainApp.cpp:8694](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8694) |
| hud | `CMainApp::Set_MechanicBarScale` | [Client/Private/MainApp.cpp:8700](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8700) |
| hud | `CMainApp::Set_ValtanMechanicTuning` | [Client/Private/MainApp.cpp:8712](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8712) |
| hud | `CMainApp::Apply_MechanicBarRect` | [Client/Private/MainApp.cpp:8725](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8725) |
| hud | `CMainApp::Update_MechanicBarAnchor` | [Client/Private/MainApp.cpp:8764](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8764) |
| hud | `CMainApp::Reload_HealthBarPositions` | [Client/Private/MainApp.cpp:8822](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8822) |
| hud | `CMainApp::Save_HealthBarPositions` | [Client/Private/MainApp.cpp:8848](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8848) |
| hud | `CMainApp::Update_WorldHealthBars` | [Client/Private/MainApp.cpp:9009](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:9009) |
| hud | `CMainApp::Update_BossHealthBar` | [Client/Private/MainApp.cpp:9048](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:9048) |
| hud | `CMainApp::RenderBossHealthBarText` | [Client/Private/MainApp.cpp:9237](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:9237) |
| hud | `CMainApp::RenderDeadSceneText` | [Client/Private/MainApp.cpp:9320](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:9320) |
| hud | `CMainApp::RenderRaidClearText` | [Client/Private/MainApp.cpp:9384](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:9384) |
| hud | `CMainApp::RenderItemAnnounceText` | [Client/Private/MainApp.cpp:9430](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:9430) |
| hud | `CMainApp::Update_ChargeGauge` | [Client/Private/MainApp.cpp:9488](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:9488) |
| hud | `CMainApp::RenderChargeGaugeText` | [Client/Private/MainApp.cpp:9636](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:9636) |
| hud | `CMainApp::Hide_EstherUI` | [Client/Private/MainApp.cpp:9703](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:9703) |
| hud | `CMainApp::Update_EstherGauge` | [Client/Private/MainApp.cpp:9718](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:9718) |
| hud | `CMainApp::RenderEstherGaugeText` | [Client/Private/MainApp.cpp:9784](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:9784) |
| hud | `CMainApp::Update_SkillIcons` | [Client/Private/MainApp.cpp:9945](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:9945) |
| hud | `CMainApp::Update_QuickSlotFlash` | [Client/Private/MainApp.cpp:10006](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:10006) |
| hud | `CMainApp::RenderCombatHUDText` | [Client/Private/MainApp.cpp:10040](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:10040) |
| hud | `CMainApp::RenderDamageNumbers` | [Client/Private/MainApp.cpp:10090](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:10090) |
| debug_host | `CMainApp::ReadyDebugTools` | [Client/Private/MainApp.cpp:11000](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:11000) |
| debug_host | `CMainApp::IsDebugToolVisible` | [Client/Private/MainApp.cpp:11015](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:11015) |
| debug_host | `CMainApp::SetDebugToolVisible` | [Client/Private/MainApp.cpp:11031](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:11031) |
| debug_host | `CMainApp::ClaimCompositionPreviewOwner` | [Client/Private/MainApp.cpp:11100](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:11100) |
| debug_host | `CMainApp::StopCompositionPreview` | [Client/Private/MainApp.cpp:11131](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:11131) |
| debug_host | `CMainApp::CloseAllDebugTools` | [Client/Private/MainApp.cpp:11144](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:11144) |
| debug_host | `CMainApp::EnsureAnimationPreviewBackend` | [Client/Private/MainApp.cpp:11156](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:11156) |
| debug_host | `CMainApp::EnsureDebugTool` | [Client/Private/MainApp.cpp:11177](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:11177) |
| debug_host | `CMainApp::RequestDebugLevelNavigation` | [Client/Private/MainApp.cpp:11755](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:11755) |
| debug_host | `CMainApp::RenderArenaCameraAndPlayerControls` | [Client/Private/MainApp.cpp:11879](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:11879) |
| debug_host | `CMainApp::RenderCharacterSelectFloorSwapControls` | [Client/Private/MainApp.cpp:12046](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:12046) |
| debug_host | `CMainApp::RenderKoukuUiPreviewControls` | [Client/Private/MainApp.cpp:12152](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:12152) |
| debug_host | `CMainApp::RenderDebugLevelNavigation` | [Client/Private/MainApp.cpp:12321](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:12321) |
| debug_host | `CMainApp::RefreshDebugResourceFiles` | [Client/Private/MainApp.cpp:12418](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:12418) |
| debug_host | `CMainApp::OpenDebugResourceFile` | [Client/Private/MainApp.cpp:12627](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:12627) |
| debug_host | `CMainApp::RenderValtanArenaStateControls` | [Client/Private/MainApp.cpp:12958](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:12958) |
| debug_host | `CMainApp::RenderValtanArenaControls` | [Client/Private/MainApp.cpp:13045](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:13045) |
| debug_host | `CMainApp::RenderKoukuSaydonArenaControls` | [Client/Private/MainApp.cpp:13095](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:13095) |
| debug_host | `CMainApp::RenderServerArenaActiveControls` | [Client/Private/MainApp.cpp:14000](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:14000) |
| debug_host | `CMainApp::RenderHUDBarPositionControls` | [Client/Private/MainApp.cpp:14300](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:14300) |
| debug_host | `CMainApp::RenderBalanceTestLauncher` | [Client/Private/MainApp.cpp:14361](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:14361) |
| debug_host | `CMainApp::RenderDeveloperTools` | [Client/Private/MainApp.cpp:14380](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:14380) |
| debug_host | `CMainApp::UpdateDebugToolShortcut` | [Client/Private/MainApp.cpp:15325](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:15325) |
| debug_host | `CMainApp::RenderCameraSpeedControls` | [Client/Private/MainApp_Dragon.cpp:174](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_Dragon.cpp:174) |
| debug_host | `CMainApp::RenderDragonControls` | [Client/Private/MainApp_Dragon.cpp:217](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_Dragon.cpp:217) |
| debug_host | `CMainApp::RenderValtanAxeEditor` | [Client/Private/MainApp_ValtanAxe.cpp:375](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_ValtanAxe.cpp:375) |
| raid_audition | `CMainApp::UpdateKoukuGateCompletePlay` | [Client/Private/MainApp.cpp:1609](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:1609) |
| raid_audition | `CMainApp::PrepareKoukuGateCompletePlay` | [Client/Private/MainApp.cpp:13369](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:13369) |
| raid_audition | `CMainApp::StartKoukuGateCompletePlay` | [Client/Private/MainApp.cpp:13393](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:13393) |
| raid_audition | `CMainApp::FinishKoukuGateCompletePlay` | [Client/Private/MainApp.cpp:13418](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:13418) |
| raid_audition | `CMainApp::CancelKoukuGateCompletePlay` | [Client/Private/MainApp.cpp:13423](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:13423) |
| raid_audition | `CMainApp::RenderKoukuSaydonCompletePlayControls` | [Client/Private/MainApp.cpp:13481](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:13481) |
| raid_audition | `CMainApp::RenderKoukuRaidPreparationDiagnostics` | [Client/Private/MainApp.cpp:13677](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:13677) |
| raid_audition | `CMainApp::RefreshCompletePlayPatternOptions` | [Client/Private/MainApp.cpp:13776](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:13776) |
| raid_audition | `CMainApp::Debug_SelectCompletePlayPattern` | [Client/Private/MainApp.cpp:13817](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:13817) |
| raid_audition | `CMainApp::Debug_GetSelectedCompletePlayPatternId` | [Client/Private/MainApp.cpp:13855](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:13855) |
| raid_audition | `CMainApp::Debug_CompletePlaySelected` | [Client/Private/MainApp.cpp:13860](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:13860) |
| raid_audition | `CMainApp::Debug_OpenValtanPatternFlow` | [Client/Private/MainApp.cpp:13891](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:13891) |
| raid_audition | `CMainApp::RenderCompletePlayControls` | [Client/Private/MainApp.cpp:13904](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:13904) |
| raid_audition | `CMainApp::ClearKoukuSequenceArrivals` | [Client/Private/MainApp_SequenceViewer.cpp:438](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_SequenceViewer.cpp:438) |
| raid_audition | `CMainApp::BeginKoukuSequenceArrivals` | [Client/Private/MainApp_SequenceViewer.cpp:449](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_SequenceViewer.cpp:449) |
| raid_audition | `CMainApp::SampleKoukuSequenceArrivals` | [Client/Private/MainApp_SequenceViewer.cpp:501](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_SequenceViewer.cpp:501) |
| raid_audition | `CMainApp::KoukuSequenceArrivalsPending` | [Client/Private/MainApp_SequenceViewer.cpp:540](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_SequenceViewer.cpp:540) |
| raid_audition | `CMainApp::ConsumeKoukuSequenceArrivalResult` | [Client/Private/MainApp_SequenceViewer.cpp:548](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_SequenceViewer.cpp:548) |
| sequence_viewer | `CMainApp::RefreshSequenceViewer` | [Client/Private/MainApp_SequenceViewer.cpp:122](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_SequenceViewer.cpp:122) |
| sequence_viewer | `CMainApp::ExecuteSequenceViewerAction` | [Client/Private/MainApp_SequenceViewer.cpp:344](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_SequenceViewer.cpp:344) |
| sequence_viewer | `CMainApp::UpdateSequenceViewer` | [Client/Private/MainApp_SequenceViewer.cpp:574](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_SequenceViewer.cpp:574) |
| sequence_viewer | `CMainApp::RenderSequenceViewer` | [Client/Private/MainApp_SequenceViewer.cpp:613](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_SequenceViewer.cpp:613) |
| world_authoring | `CMainApp::CompositionPreviewWorldSource` | [Client/Private/MainApp.cpp:14107](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:14107) |
| world_authoring | `CMainApp::RefreshWorldObjectResources` | [Client/Private/MainApp.cpp:14114](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:14114) |
| world_authoring | `CMainApp::Debug_PrepareMapToolAuthoring` | [Client/Private/MainApp_WorldLevel.cpp:57](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_WorldLevel.cpp:57) |
| world_authoring | `CMainApp::Debug_PrepareWorldPlacementEditing` | [Client/Private/MainApp_WorldLevel.cpp:75](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_WorldLevel.cpp:75) |
| world_authoring | `CMainApp::Debug_OpenWorldObjectDetails` | [Client/Private/MainApp_WorldLevel.cpp:99](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_WorldLevel.cpp:99) |
| world_authoring | `CMainApp::UpdateWorldMeshInspectionInput` | [Client/Private/MainApp_WorldLevel.cpp:165](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_WorldLevel.cpp:165) |
| world_authoring | `CMainApp::RenderWorldMeshInspection` | [Client/Private/MainApp_WorldLevel.cpp:257](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_WorldLevel.cpp:257) |
| world_authoring | `CMainApp::RenderWorldSceneTool` | [Client/Private/MainApp_WorldLevel.cpp:273](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_WorldLevel.cpp:273) |
| world_authoring | `CMainApp::GetWorldLevelAreaId` | [Client/Private/MainApp_WorldLevel.cpp:366](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_WorldLevel.cpp:366) |
| world_authoring | `CMainApp::FocusWorldLevelPosition` | [Client/Private/MainApp_WorldLevel.cpp:377](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_WorldLevel.cpp:377) |
| world_authoring | `CMainApp::UpdateMapEffectPlacementInput` | [Client/Private/MainApp_WorldLevel.cpp:430](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_WorldLevel.cpp:430) |
| world_authoring | `CMainApp::UpdateWorldLevelPlacementPickInput` | [Client/Private/MainApp_WorldLevel.cpp:537](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_WorldLevel.cpp:537) |
| world_authoring | `CMainApp::RenderMapEffectPlacementMarker` | [Client/Private/MainApp_WorldLevel.cpp:587](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_WorldLevel.cpp:587) |
| world_authoring | `CMainApp::UpdateWorldLevelTool` | [Client/Private/MainApp_WorldLevel.cpp:657](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_WorldLevel.cpp:657) |
| world_authoring | `CMainApp::RenderWorldLevelTool` | [Client/Private/MainApp_WorldLevel.cpp:797](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_WorldLevel.cpp:797) |
| rendering_authoring | `CMainApp::RenderOptimizationBenchmarkPanel` | [Client/Private/MainApp.cpp:14913](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:14913) |
| rendering_authoring | `CMainApp::RenderRenderingWorkbench` | [Client/Private/MainApp.cpp:14930](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:14930) |
| rendering_authoring | `CMainApp::UpdateLightingPreview` | [Client/Private/MainApp_RenderingLighting.cpp:248](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_RenderingLighting.cpp:248) |
| rendering_authoring | `CMainApp::SelectRenderingLight` | [Client/Private/MainApp_RenderingLighting.cpp:317](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_RenderingLighting.cpp:317) |
| rendering_authoring | `CMainApp::RenderLightingWorkbench` | [Client/Private/MainApp_RenderingLighting.cpp:337](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_RenderingLighting.cpp:337) |
| rendering_authoring | `CMainApp::ReloadPublishedMapLights` | [Client/Private/MainApp_RenderingLighting.cpp:559](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_RenderingLighting.cpp:559) |
| rendering_authoring | `CMainApp::RenderLightDetail` | [Client/Private/MainApp_RenderingLighting.cpp:596](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_RenderingLighting.cpp:596) |
| rendering_authoring | `CMainApp::StageMapLightPreview` | [Client/Private/MainApp_RenderingLighting.cpp:811](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_RenderingLighting.cpp:811) |
| rendering_authoring | `CMainApp::StartLightingPreview` | [Client/Private/MainApp_RenderingLighting.cpp:839](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_RenderingLighting.cpp:839) |
| rendering_authoring | `CMainApp::StopLightingPreview` | [Client/Private/MainApp_RenderingLighting.cpp:913](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_RenderingLighting.cpp:913) |
| rendering_authoring | `CMainApp::RenderSceneProfileDetail` | [Client/Private/MainApp_RenderingLighting.cpp:935](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_RenderingLighting.cpp:935) |
| rendering_authoring | `CMainApp::RenderLightSequencer` | [Client/Private/MainApp_RenderingLighting.cpp:1060](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_RenderingLighting.cpp:1060) |
| profiler | `CMainApp::UpdateProfilerRuntime` | [Client/Private/MainApp_Profiler.cpp:27](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_Profiler.cpp:27) |
| profiler | `CMainApp::RenderProfilerOverlay` | [Client/Private/MainApp_Profiler.cpp:58](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_Profiler.cpp:58) |
| profiler | `CMainApp::RenderProfilerSettings` | [Client/Private/MainApp_Profiler.cpp:107](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp_Profiler.cpp:107) |
| profiler | `CMainApp::Update_DebugWindowTitleWithFps` | [Client/Private/MainApp.cpp:1178](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:1178) |
| profiler | `CMainApp::RenderFpsText` | [Client/Private/MainApp.cpp:10461](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:10461) |
| item_upgrade | `CMainApp::Open_ItemUpgradeWindow` | [Client/Private/MainApp.cpp:868](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:868) |
| item_upgrade | `CMainApp::Hide_ItemUpgrade` | [Client/Private/MainApp.cpp:914](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:914) |
| item_upgrade | `CMainApp::Update_ItemUpgrade` | [Client/Private/MainApp.cpp:1057](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:1057) |
| item_upgrade | `CMainApp::RenderItemUpgradeButtonText` | [Client/Private/MainApp.cpp:7364](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:7364) |
| item_upgrade | `CMainApp::RenderItemUpgradeListText` | [Client/Private/MainApp.cpp:7413](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:7413) |
| item_upgrade | `CMainApp::Update_ItemUpgradeSelection` | [Client/Private/MainApp.cpp:7506](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:7506) |
| item_upgrade | `CMainApp::Update_ItemUpgradeGrowButton` | [Client/Private/MainApp.cpp:7557](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:7557) |
| item_upgrade | `CMainApp::Update_ItemUpgradeReforgeButton` | [Client/Private/MainApp.cpp:7600](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:7600) |
| item_upgrade | `CMainApp::Update_ItemUpgradeServerResult` | [Client/Private/MainApp.cpp:7666](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:7666) |
| item_upgrade | `CMainApp::Update_ItemUpgradeResultWaitClick` | [Client/Private/MainApp.cpp:7707](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:7707) |
| item_upgrade | `CMainApp::Update_ItemUpgradeResultOkButton` | [Client/Private/MainApp.cpp:7770](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:7770) |
| item_upgrade | `CMainApp::Reset_ItemUpgradeIdleGauge` | [Client/Private/MainApp.cpp:7822](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:7822) |
| item_upgrade | `CMainApp::Set_ItemUpgradeCenterPanelVisible` | [Client/Private/MainApp.cpp:7839](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:7839) |
| item_upgrade | `CMainApp::RenderItemUpgradeLevelText` | [Client/Private/MainApp.cpp:7883](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:7883) |
| item_upgrade | `CMainApp::RenderItemUpgradeMaterialCounts` | [Client/Private/MainApp.cpp:7957](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:7957) |
| item_upgrade | `CMainApp::RenderItemUpgradeGaugePercentText` | [Client/Private/MainApp.cpp:8012](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8012) |
| item_upgrade | `CMainApp::RenderItemUpgradeResultWaitText` | [Client/Private/MainApp.cpp:8041](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8041) |
| item_upgrade | `CMainApp::RenderItemUpgradeSuccessDetailText` | [Client/Private/MainApp.cpp:8076](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8076) |
| item_upgrade | `CMainApp::RenderItemUpgradeFailDetailText` | [Client/Private/MainApp.cpp:8123](C:/Users/tnest/Desktop/LostArk/Client/Private/MainApp.cpp:8123) |

## 헤더 inline 정의

| 그룹 | 함수 | 파일:줄 |
|---|---|---|
| app_lifecycle / 이행 중 공개 연결 | `CMainApp::Get_Active` | [Client/Public/MainApp.h:157](C:/Users/tnest/Desktop/LostArk/Client/Public/MainApp.h:157) |

<!-- function-inventory:end -->
