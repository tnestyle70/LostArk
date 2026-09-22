# 발탄 Map Tool 컷신과 Boss 패턴 연결 검증 결과

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

## 사용자 확인

새 Client를 빌드하고 Debug Server와 함께 발탄에 들어간 뒤 F1 → Action Workbench → Composition Actions → Boss에서 패턴을 고른다. 패턴 순서와 실제 동작 검증은 Preview 최상단의 `Play (Server Exact)`를 누른다. 발탄이 `boss.valtan.center`에서 시작하고 Server Stage 전체가 실행되어야 한다. 저장 전 타임라인 편집·스크럽만 필요할 때 `Play Local Timeline`을 사용하며 이 경우에도 복제본의 시작 위치와 방향은 중앙 정본과 같아야 한다. 실제 화면 판정은 사용자 확인 전까지 미확인이다.
