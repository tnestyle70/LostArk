# 발탄 Map Tool 컷신과 Boss 패턴 연결 검증 결과

## G18 — 2026-09-22 22:22 원본 배우 생성 등록 누락 수정

이전 아래 결론은 연결 자료 존재와 코드 구조 확인이며, 화면/음성 성공 증거가 아니다.
추가 조사로 발탄 fresh-entry 경로의 실제 생성 결함을 확인했다.

- 원본 배우를 생성하는 `CWorldSequenceObject` Prototype이 발탄 Level에는 등록되지
  않았다. MapTool과 쿠크만 등록했다. Engine은 정확히 요청한 Level에서만 Prototype을
  찾으므로 발탄 `Ready_SourceCinematics -> Prewarm_ObjectInstances`는 MapTool을
  열기 전 Clone에 실패한다. pending=false로 남아 나중에 MapTool을 열어도 복구되지
  않는다. 배우 생성과 함께 시작되는 원본 Sound/Subtitle도 이 단계에서 막힌다.
- Loader의 발탄 로드에서 기존 Prototype을 등록하도록 수정했다. 실패는 Level 자원
  rollback을 따른다. MapTool은 발탄에서도 이미 등록된 Level 소유 Prototype을 쓴다.
- 원본 연출 연결 실패 시 로컬 Camera만 계속 나오지 않도록 차단했다. 별도의 원본
  연출 상태 문구를 Workbench Local Preview와 Composition Sequencer에 표시한다.
- Local Timeline pause를 원본 Sound까지 전달한다. Workbench는 Effect commit 뒤
  샘플링하므로 MapTool과 동일하게 자신이 만든 world-root만 같은 프레임에 commit한다.
  이 Effect 차이는 첫 프레임 지연이며 영구 Sound/Subtitle 누락 원인으로 주장하지 않는다.
- 기존 70개 Flow, gameplay 시간, Camera 키, 원본 world sequence/sound/subtitle는
  이번 G18에서 변경하지 않았다. 사운드를 별도 cue로 중복 추가하지 않았다.

검증 완료:

- 독립 코드 리뷰로 Level별 Prototype 조회/등록/Clone 실패 경로 교차 확인.
- `test_valtan_maptool_cutscene_pattern_links.py`: 5개 연결/클립 일치, 기존 포효
  카메라 1ms 수치 검사, Prototype 등록 순서, MapTool 재사용, pause, 실패 Camera 차단,
  다섯 연출의 instance/template/objectResource 저작본-런타임 일치 검사 통과.
- 정본 Debug Product 빌드 통과. Engine/Shared/Server/Client 컴파일·배포 완료.
  `out/BuildPipeline/runs/20260922T132254393Z-debug-product.json`.
  `Client/Bin/Debug/Client.exe` 수정 시각 2026-09-22 22:22:52.
  기존 C4819/C4828 인코딩 경고는 남아 있으며 컴파일 오류는 없었다.
- `git diff --check` 통과. 수정 C++ 기존 UTF-8/BOM와 CRLF 유지.
- 이번 G18은 코드 수정이며 데이터 publisher를 실행하지 않았다. 이전 Flow의 Server
  게시 상태를 이 빌드 성공으로 대신하지 않는다. 커밋/푸시 없음.

미검증: Client 화면/음성은 실행하지 않았다. fresh process로 발탄 진입 후 MapTool을
열지 않은 채 F1 → Action Workbench → Composition Actions → Boss → 발탄 등장 컷신을
재생해야 한다. 원본 배우의 방향/타이밍, 음성(원본 1800ms 시작), 자막 4개 구간,
Pause/Resume/Seek/Stop을 사용자 확인한다. 실패하면 같은 Workbench의 원본 연출 상태
문구가 진단 입력이다. F1 Valtan Arena 상태는 발탄 Level에서만 나오므로 Loading
화면에서 해당 메뉴를 찾도록 안내하지 않는다.

## 결론

Map Tool Camera의 대상 5개 연출은 모두 기존 Action Workbench Boss 패턴의 정확한 Stage Camera Cue와 World Sequence에 연결돼 있다. 그러나 최초 검증은 JSON과 Server 재생 경로만 확인했고 Action Workbench 로컬 `Play`의 카메라 소비자 생명주기를 확인하지 않아, 실제 UI에서는 카메라가 나오지 않는 결함을 놓쳤다.

원인은 두 단계였다. 첫째, `MainApp`이 독립 Camera Tool 객체가 이미 생성된 경우에만 `Sample_CompositionPreview`를 호출해 Action Workbench만 연 상태에서는 카메라가 생략됐다. 둘째, 카메라가 재생돼도 로컬 발탄 복제본은 일반 패턴 규칙대로 플레이어 옆에 있었지만 원본 카메라는 아레나 중앙 절대 좌표를 사용했다. 따라서 데이터 연결만으로는 원본 구도를 볼 수 없었다.

활성 Valtan 로컬 Composition 재생을 감지하면 비가시 Camera Tool 샘플러를 지연 생성한다. 이어 다섯 컷신 패턴에 한해 제품용 player와 분리된 Action Workbench World Sequence player가 원본 컷신 모델·animation/effect track·부속 배우를 같은 패턴 clock으로 재생한다. 로컬 발탄 복제본은 원본 `actor` 위치·회전·scale을 따라가 카메라 추적 root만 제공하고 화면에서는 숨기므로 중복 모델이 생기지 않는다.

추가 실행 경로 감사에서 이 설명과 실제 코드가 한 군데 어긋난 것을 확인했다. Valtan Level 초기화가 Loader의 단일 prepared Area를 제품용 source player에 이미 소비한 뒤, Action Workbench player가 같은 `Load_PreparedArea`를 다시 호출했다. 두 번째 소비는 성공할 수 없으므로 원본 배우·FX·WAV가 실제로 시작되지 않았고 일반 Pattern 모델만 원본 카메라 아래 남았다. Workbench player는 이제 이미 검증된 제품 source 문서를 `Set_Document`로 별도 admit한다. 발탄 등장에서는 Map Tool row에 없는 Actor64 body/weapon 세 instance도 로컬 타임라인, Complete Play 사전 준비, Server Exact 제품 source 재생에서 모두 제거해 Map Tool과 동일한 main + colorless 두 instance만 재생한다.

원본 사운드 데이터는 누락된 것이 아니었다. 다섯 source sequence에 각각 WAV track이 하나씩 있고 실제 `Client/Bin/Resources/Sound/Valtan/Scenes` 파일도 존재한다. player admit 실패를 제거해 이 track이 Workbench clock에서 재생된다. 자막은 entrance 4개, trash 2개, finale 2개가 이미 있었지만 MainApp 자막 renderer가 Workbench player의 sample을 수집하지 않았다. 이제 제품 source player와 Workbench player 양쪽 subtitle sample을 같은 renderer에 제출한다.

이펙트 누락도 같은 실패 경로로 설명된다. World Sequence에는 entrance 14개, phase2 10개, roar 16개, trash 8개, finale 39개 effect track이 있지만, source player가 시작되지 않아 로컬 Pattern cue만 남았다. 로컬 Pattern cue는 같은 다섯 패턴에 각각 0, 1, 1, 0, 0개뿐이다. Effect 전용 데이터나 런타임은 이번 변경에서 수정하지 않았다.

사용자 화면에서 일반 패턴이 플레이어 옆에 생성되는 것을 확인한 뒤 Preview 계약을 추가 교정했다. 기존 `Play`는 Server 실행이 아니라 로컬 animation/effect preview였고, 배치 함수도 플레이어 camera-right 3.25m를 명시적으로 사용했다. 이제 Preview 최상단의 `Play (Server Exact)`가 기존 Server Complete Play/`PLAY_PATTERN_ID`를 호출한다. 이 경로는 Server가 `Gameplay.world.json`의 `boss.valtan.center`에서 보스를 다시 만들고 실제 Stage, 이동, combat object, animation, Effect, Sound, camera, world event를 실행하므로 패턴 순서 검증에 사용한다. pause/seek가 필요한 저장 전 편집 경로는 `Play Local Timeline`으로 이름을 분리했으며, 그 복제본도 `boss.valtan.center` 위치 `[156.029999, 22.9975109, -122.059998]`와 yaw `225`를 읽어 시작한다. 로컬 타임라인은 Server 검증 결과가 아니다.

| Map Tool 컷신 | 연결된 Boss 패턴 | 연결 Stage |
|---|---|---|
| 발탄 등장 | `VALTAN_ENTRANCE_CINEMATIC` | `ESTABLISH`, `ARENA_REVEAL`, `HERO_HANDOFF` |
| 발탄 벽·기둥 파괴 | `VALTAN_ARENA_BREAK_109` | `IMPACT_HOLD` 600ms 이후, `WIDE_REVEAL`, `RECOVERY` |
| 발탄 포효 | `VALTAN_SIX_PIZZA_106` | `STEP_04`, `STEP_05` |
| 발탄 스킬 연출(버러지) | `VALTAN_TRASH` | `STEP_05`, `STEP_06` |
| 발탄 최후 | `VALTAN_GHOST_DEATH_AUDITION` | `STEP_01` |

신규 Event_02 카메라는 104키, 0~5500ms이며 500+2300+2700ms 구간으로 기존 2페이즈 패턴에 연결돼 있다.

## 추가한 회귀 계약

`Tools/ValtanPipeline/test_valtan_maptool_cutscene_pattern_links.py`가 다음을 한 번에 검사한다.

- CameraShots 저작본과 런타임 게시본의 컷신 일치
- 5개 Pattern/Stage와 길이
- Stage Camera Invocation과 실제 Camera Cue ID/길이
- 컷신과 World Sequence instance
- Valtan Arena의 패턴별 source cinematic 선택
- 다섯 source-preview instance의 실제 `actor` transform track 존재
- 격리된 Action Workbench player의 source instance Play/Seek 및 로컬 복제본 중복 표시 차단
- Event_02 104키와 5500ms 범위
- Action Workbench 로컬 재생의 Camera Tool 사전 실행 의존성 제거
- 다섯 패턴의 source 시작 Stage/offset 계약
- 원본 actor transform 적용이 environment/camera 샘플보다 먼저 실행되는 순서
- Loader prepared Area의 이중 소비 금지와 이미 admit된 source 문서 복제
- 로컬 타임라인과 Server Exact의 발탄 등장 instance 집합이 모두 Map Tool과 같은 main + colorless인지 검사
- 다섯 source WAV 파일 존재와 Workbench subtitle sample 수집 경로

## 실행 결과

- focused link contract: PASS. 로컬 타임라인뿐 아니라 Complete Play 준비와 제품 source 재생에도 Actor64 instance가 남아 있지 않음을 검사한다.
- Map Authoring `CameraShots` Check: PASS
- Map Authoring `WorldSequences` Check: PASS
- Map Authoring 전체 Area Validate: PASS (`PlacementCount 13184`, `FileCount 24`)
- Map Authoring 전체 Area Check: 기존 `LV_LUT_HEARTRB_ED.deployassets` 불일치로 실패. 이번 변경 범위인 CameraShots와 WorldSequences의 scope Check는 각각 통과했으며 deployassets는 자동 게시하거나 교정하지 않았다.
- `git diff --check`: PASS
- 수정 `Level_ValtanArena.cpp` MSVC C++20 `/Zs`: 오류 0, 종료 코드 0. 기존 CP949 경고만 발생.
- Server-exact 버튼·canonical center/yaw 연결 구조 검사: PASS
- Client C++ build: 이후 Flow 진입 컷신 수정과 함께 `Client/Default/Client.vcxproj` Debug x64 빌드를 실행해 PASS했다. 현재 Debug `Client.exe`에는 이 변경도 함께 반영됐다.
- 전체 Gameplay Validate: 발탄 연결 전에 실행되는 쿠크 정본 검사에서 `KoukuSaydonEncounter.json` stale로 중단. 이번 발탄 연결과 무관한 기존 저장소 상태이므로 자동 교정하거나 게시하지 않았다.

## G17 재조사·수정 — 2026-09-22

이전의 ID/문자열 검사는 실제 연결 조건과 카메라 궤적 일치까지 증명하지 못했다.
그 검사를 근거로 다섯 연출의 정상 재생을 단정한 설명을 정정한다.

### 현재 프로젝트의 애니메이션 역대조

`sync_valtan_roar_pattern_camera.py`는 Map Tool World Sequence의 actor animationTracks를
읽고 전체 split presentation의 연속 clip chain과 대조한다. 다섯 개 모두 후보가 하나였다.

| Map Tool 연출 | 일치 클립 | 현재 Boss 패턴 | 원본 WAV/자막/FX 트랙 |
|---|---|---|---|
| 등장 | `valtan.cinematic.entrance` | 발탄 등장 컷신 / `VALTAN_ENTRANCE_CINEMATIC` | 1 / 4 / 14 |
| 벽·기둥 파괴 | `valtan.cinematic.phase2` | 중앙 이동 후 2페이즈 컷씬 / `VALTAN_ARENA_BREAK_109` | 1 / 0 / 10 |
| 포효 | `mesh_att_battle_12_04 -> mesh_att_battle_12_05` | 중앙이동 후 6방향 공격 후 피자 패턴 / `VALTAN_SIX_PIZZA_106` | 1 / 0 / 16 |
| 버러지 | `valtan.cinematic.trash` | 버러지 패턴 / `VALTAN_TRASH` | 1 / 2 / 8 |
| 최후 | `valtan.cinematic.finale` | 3페이즈 발탄 사망 / `VALTAN_GHOST_DEATH_AUDITION` | 1 / 2 / 39 |

이 표는 현재 프로젝트의 모델·클립 일치다. 원작 전투에서 같은 HP/페이즈에 발생한다는
별도 증거로 사용하지 않는다. 포효 Stage의 기본 clip sourceStart(0)와 Map Tool trim(2644/78)은
다르며, 연출 구간에는 기존 World Sequence player가 Map Tool의 trim과 actor/FX track을
그대로 재생한다. gameplay Stage 길이·패턴 순서·피격 판정은 이번에 변경하지 않았다.

### 확인한 결함과 수정

1. Server input의 `strStageId`를 채우지 않아 phase2/roar 선택 조건이 성립하지 않았다.
   정본 pattern과 stage index에서 Stage ID를 resolve한다.
2. 포효 배우는 Map Tool 연출인데 product 카메라는 첫 eye부터 다른 6724ms 궤적이었다.
   Map Tool 세 컷을 기존 두 cue에 0~2800 / 2800~5000ms로 투영했다.
   기존 stable cue ID는 유지했다. 이름에 남은 `wall-break` 문자열로 의미를 추측하지 않는다.
3. 포효/벽파괴 카메라에 BOSS_XZ가 남아 절대월드 키에 배우 이동이 추가됐다.
   해당 두 연출의 다섯 cue를 WORLD로 복귀하고 Event_02의 stage blend도 제거했다.
4. 카메라 종료 시 source actor/FX/WAV도 중단됐다. source 업데이트를 카메라 성공 분기 밖으로
   옮겨 포효의 5000~7003ms 구간을 계속 샘플링한다. 활성 카메라 age에는 invocation offset을
   더하고, 카메라 종료 후 동일 Server action tick age를 사용한다. 같은 occurrence의 시계 역행은
   player의 기존 elapsed로 제한해 WAV 재탐색을 막는다. Local Timeline 스크럽은 계속 역행 가능하다.
5. Local Timeline은 source 마지막 pose를 후속 Stage까지 무한 유지했다.
   source 시작 전/종료 후 player를 정리하고 원래 preview actor 표시와 transform을 복귀한다.

사운드·자막·이펙트 원본 트랙을 삭제하거나 새로 지어내지 않았다. Stage 선택 실패 때문에
도달하지 못하던 기존 트랙 재생 경로를 연결했다. 포효/벽파괴에는 자막 트랙 자체가 없다.

### 검증 상태

- 다섯 clip chain의 유일 후보와 설치 WAV 존재 확인: 통과.
- 포효 카메라 1ms 대조: 464키/200키, eye/lookAt 최대 0.001593m 이내, FOV 0.002도 이내.
  WORLD tracking·0ms blend도 검사한다. 픽셀/화면 판정을 뜻하지 않는다.
- focused `test_valtan_maptool_cutscene_pattern_links.py`: 통과.
- `valtan_tuning_pipeline.py validate`: errors=[] / ok=true.
- `Project-ValtanPatternMaster.ps1 -Mode PublishV2`: 성공, 9개 산출물 중 2개 교체.
- 정본 Debug Product 빌드: Engine/Shared/Server/Client 성공. 수정 cpp 컴파일 및 Client.exe 링크 완료.
  로그: `out/BuildPipeline/runs/20260922T115536538Z-debug-product.json`. 기존 C4819 경고가 있다.
- 전체 Gameplay 게시: 쿠크 생성 `KoukuSaydonEncounter.json` stale 검사에서 중단.
  발탄 전용 생성 성공과 전체 bootstrap 게시 성공을 구분한다. 쿠크 원본은 수정하지 않았다.
- 기존 `test_valtan_camera_tool_contract.py`: HEAD에도 없는
  `VALTAN_GAMEPLAY_FOLLOW_LOOK_HEIGHT` 문자열 기대에서 실패. 이 기대값을 완화하지 않았다.
- 실행 중 Client/Server는 에이전트가 종료하지 않았다. 종료된 것을 확인한 뒤 빌드만 수행했다.
  자동 UI 실행·화면 캡처·visual PASS는 하지 않았다.

원본 백업·수치 보고: `out/ValtanRoarPatternCamera20260922/`.
리소스 바이너리 변경/Drive 배포는 없다. 현재 변경은 commit/push하지 않았다.

## 사용자 확인

새 Client를 빌드하고 Debug Server와 함께 발탄에 들어간 뒤 F1 → Action Workbench → Composition Actions → Boss에서 패턴을 고른다. 패턴 순서와 실제 동작 검증은 Preview 최상단의 `Play (Server Exact)`를 누른다. 발탄이 `boss.valtan.center`에서 시작하고 Server Stage 전체가 실행되어야 한다. 저장 전 타임라인 편집·스크럽만 필요할 때 `Play Local Timeline`을 사용하며 이 경우에도 복제본의 시작 위치와 방향은 중앙 정본과 같아야 한다. 실제 화면 판정은 사용자 확인 전까지 미확인이다.
# G19. 일반 Play의 벽 파괴 연결 (2026-09-22 22:58 빌드)

## G20. 전투 게시 실패 수정 및 70개 순서 게시 완료

이 절은 아래 23:48 게시 실패 기록의 후속 해결 상태다. 원인은 게시기의
고정 지점 도약(LEAP_TO_ANCHOR) 시선 예외가 BOSS_XZ에만 적용된 것이었다.
Client WORLD 소비는 절대좌표를 그대로 사용하므로 원본 Matinee lookAt을
보스 착지 XZ에 고정할 이유가 없다. 동일 고정 도약의 WORLD에도 기존 예외를
확장했다. LEAP_TO_TARGET의 착지점 시선, 105번 tracking, origin, key 구조,
finite/FOV/시간 검사와 실패 시 게시본 보존 경로는 유지했다.

- `Publish-GameplayBalance.ps1` 조건 한 곳과 설명 수정. 카메라 키·motion·사용자 순서 수정 없음.
- 실제 PowerShell AST의 검사 loop를 실행하는 10개 정상/거부 사례 통과.
- 기존 컷신 5종 연결·곡선·payload/벽 파괴 focused 검사 통과.
- `Publish-GameplayBalance.ps1 -Mode Publish -SkipValtanSplitProjection`: exit 0, 게시 성공.
- 생성된 Gameplay.bootstrap: `PATTERNSEQUENCE ENCOUNTER_VALTAN sequence.valtan.server-authored.v1 ORDERED_ONCE_THEN_IDLE 1000 70`.
- 70개 pattern ID의 순서와 69개 transition wait를 source scriptedSequence와 전수 대조, 불일치 0. 처음 VALTAN_ENTRANCE_CINEMATIC, 마지막 VALTAN_GHOST_DEATH_AUDITION. 마지막 step wait는 0.
- 게시 로그의 `52 Valtan audition timeline rows`는 별도 audition 행 수이며 scriptedSequence 개수가 아니다.
- `test_valtan_presentation_generation.py`: 13 tests OK. `git diff --check` 통과.
- C++ 변경/재빌드 없음. 서버와 클라이언트를 자동 실행하지 않았다. 새 Server 실행 후 Stage_Boss 진입의 실제 재생 확인은 사용자 몫이다.
- 쿠크 DJ PNG 두 개의 설치/전체 Effect 검증은 여전히 별도 미완료이며 사용자가 리소스를 확보한다. 전체 pipeline PASS를 의미하지 않는다. commit/push 없음.

## 후속 전체 게시 시도 (2026-09-22 23:48)

- 사용자 요청으로 `Run-FullPipeline.ps1 -Configuration Debug -DataOnly` 실행. 발탄 Product 투영/검증, 쿠크 Product, Composition, 쿠크 Map 게시 통과.
- `effect.v2`는 설치 리소스 `UI/KoukuSaydon/GameNote/dj_kouku_cardrain.png` 누락으로 실패. 동일 authored slot 전체 점검에서 `dj_kouku_delivery.png`도 누락 확인. 문서에 기록된 `out/CardRain20260922/Resources/` 후보도 현재 PC에서 찾지 못했다. 대체 이미지를 만들거나 검증을 완화하지 않았다.
- 독립적인 정본 `Invoke-BuildDomainOwner.ps1 -Owner Server` 실행. world.gameplay, navigation(Bern/Bern2/Bern3 포함), world.destruction 게시 통과. gameplay.balance에서 중단.
- 중단 원인은 `Publish-GameplayBalance.ps1`의 landing-anchor lookAt 검사다. WORLD 원본 카메라 `camera.valtan.arena-break-109.impact-hold`를 `anchor.valtan.arena-break-109.landing`의 XZ와 0.05m 이내로 항상 일치시키는 조건에 걸렸다. 카메라 원본을 착지점 고정으로 덮어쓰거나 검사를 삭제하지 않았다.
- 나머지 독립 domain items.catalog, vehicles.profiles, honortitles.catalog, valtan.rewards는 같은 `Invoke-BuildDomain` 정본 실행기로 게시 통과.
- 실행 종료 후 재확인: 저작 scriptedSequence는 70개지만 `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`은 여전히 52개다. Stage_Boss의 70개 순서 반영은 **미완료**다. source revision은 `45f7515e7f12543c227f2080f1d550aa463f0e07d43fd27f02d91003fe738c84`.
- 다음 작업은 원본 시네마틱 검증과 착지점 추적 카메라 검증의 계약 분리 및 회귀 검사, 누락 PNG 두 개의 정식 리소스 확보 후 재게시다. 전체 게시 PASS나 서버 70개 적용 완료로 보고하지 않는다.
- 이번에는 C++ 재빌드, Server/Client 시작, UI 조작을 하지 않았다. `git diff --check` 통과. commit/push 없음. 상세 domain receipt는 `out/BuildPipeline/receipts/`에 있다.

## G19 구현 기록

사용자는 Action Workbench에서 일반 Play를 사용했다. 기존 Level의 벽 파편 발생은 Server live event만 소비했고, 일반 Play의 local animation clock에는 world-destruction consumer가 없었다. 서버 게시 누락은 아니었다.

## 반영

- 현재 draft Stage 길이와 enabled STAGE_ENTER binding을 결합하여 기존 editor destruction simulator로 샘플링한다. 바인딩, mutation, group, Client projection, debris profile의 ID와 전체 member 집합을 검증한다.
- 현행 109 데이터는 외벽 27 emitter와 내부벽 67 emitter, 합계 94 emitter / source+alias 129 placement로 결합된다. 위치·속도·중력·수명은 게시된 profile을 소비한다.
- Play/재시작은 시뮬레이션 준비, Pause는 같은 시간 유지, 전진은 필요한 고정 스텝만 진행, 역방향 Seek는 Reset 후 재생이다. Stop·패턴 전환·F1/Sequencer 닫기·Level 종료 시 preview를 복구한다. F1을 닫았다 다시 열면 Play를 재시작해야 한다.
- MapTool과 Workbench의 전역 physics preview clock 중복 소유를 거부한다. 실제 Server live debris가 있으면 local preview를 중단한다. Server Full/Delta projection 전에 editor alias를 복구하여 서버의 새 상태를 나중에 덮어쓰지 않게 한다. 복구 실패를 숨기지 않는다.
- local preview 파편은 기존 editor 시뮬레이터의 seed/분산을 사용한다. Server와 동일한 물리 궤적이라는 의미가 아니다. Server의 충돌·네비게이션·전투 상태를 조작하지 않는다.

## 타이밍과 검증 경계

파괴는 기존 IMPACT 진입 1600ms, source camera 시작은 2600ms다. 이번 수정은 이 값을 바꾸지 않았다. 카메라 첫 프레임에 발사되는 설정이 아니라 그때 이미 1초 진행한 파편을 보게 되는 설정이다. 원본 영상과의 최종 타이밍/화면 일치는 사용자 확인 전이며 자동 visual PASS가 아니다.

- `test_valtan_maptool_cutscene_pattern_links.py`: 통과. 94/129 데이터 join, 1600ms, Server projection 이전 cleanup, local 연결/되감기/독점 owner 구조 확인. 실제 PhysX 실행·화면 검사를 대체하지 않는다.
- `valtan_tuning_pipeline.py --repository-root . validate`: ok=true, errors=[], worldMembers=94.
- `Publish-ValtanWorldDestruction.ps1 -Mode Validate`: 통과, groups=102 / bindings=218 / emitters=102. 게시본을 바꾸지 않았다.
- Debug Product 빌드: Engine/Shared/Server/Client 통과. 최종 기록 `out/BuildPipeline/runs/20260922T135804684Z-debug-product.json`. Client.exe 2026-09-22 22:58:02, 67776000 bytes. 기존 C4819/C4828 경고는 남아 있다.
- 독립 코드 리뷰 후 projection mutation/member 일치 검사를 보강하고 재빌드했다.
- `git diff --check`: 통과. G19 변경 C++ 9개 파일의 전문을 PLAN에 기록했다.
- 추가 `Server.exe --contract-test`: 전체 PASS 아님. 쿠크 Debug gate boss spawn/idle/placement/audition/despawn 관련 `[FAILURE]` 7건을 확인했다. 약 10분간 진행한 뒤 이 작업에서 만든 테스트 프로세스만 PID·경로·`--contract-test` 인자를 검증하여 중단했다. 마지막까지 완료한 회귀 검사로 기록하지 않는다. 이번 변경은 Client만 수정했으며 이 실패의 근본 원인은 조사·수정하지 않았다.
- Client/UI를 실행하거나 화면을 캡처하지 않았다. 일반 Play → 1.6초 전후 → Pause/역방향 Seek/Stop → 재시작 확인은 사용자 몫이다. 이번 변경에는 새 Resources, Drive 배포, domain Publish가 필요 없다.
