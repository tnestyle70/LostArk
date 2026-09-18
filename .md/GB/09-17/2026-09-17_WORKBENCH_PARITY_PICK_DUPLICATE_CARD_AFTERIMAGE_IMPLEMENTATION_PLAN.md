# Workbench 동등화·World 피킹/복제·카드 잔상/분출 창·팔각별 제거 구현 계획서

## G00. 목표와 종료 증거

한 요청 묶음을 데이터 축과 코드 축으로 나눈다.

데이터 축(즉시 반영):
1. 빙글빙글돌며카드던지기(P48)의 카드별 알파 잔상 24요소 복구.
2. 같은 패턴의 카드 발사 창(Logic `세이튼_회전카드_원작12발`) 종료 시점을 두 배(4100→8200ms)로 늘리고 회전 stage를 그 창까지 연장.
3. 캐릭터 선택 맵 시작 위치의 별 문양 배치(`editor:LV_LOBBY_CLASSSELECT_SL00:1`) 제거·publish.

코드 축(Client 재빌드 필요):
4. Object 시퀀서에서 Effect 박스 단독 선택·Box Detail 편집, Map position 실시간 preview, Resources 탭.
5. Character 대상의 Composition Resources 탭·Boss형 lane 행(Stage/Animation/Logic·Timing/Effect/Collider/Sound/Camera), 공백(후딜레이)·action clip 표시, 콜라이더 박스 편집·저장, 사운드 행, Source Out 표시.
6. MapTool 호스트 일반화(캐릭터 선택 Level 런타임 부착), 뷰포트 피킹→선택, 초록 외곽선, 복제, Euler 회전, Save+Publish(Area), World tool `Map` 카테고리 런처, 예외 처리.

종료 증거는 데이터 축의 codec/playback probe·publisher exit code·runtime 파일 비교, 코드 축의 touched TU 격리 컴파일과 독립 리뷰다. 화면 판정은 사용자가 한다.

## G01. 현재 실측

- 잔상: `effect.kouku.common.spinning.card.throw` 224요소 중 sk_13_1 emitter_0(native 2999 보라 알파 sprite) 24개가 09-17 G26에서 `visible=false`로 억제됨. 복구 함수 `restore_spinning_card_afterimages`는 준비돼 있었으나 미설치.
- 발사 창: P48 Logic `kakulsaydon.g1.logic.74`(PURSUIT_PROJECTILES, spawnIntervalMs 300, countPerWave 6)의 occurrence startMs 3533 durationMs 4100. Server는 `iEndTick`까지 300ms마다 wave를 쏘므로 durationMs가 곧 발사 종료 시점이다(wave 수 필드 없음). 회전 stage(4_02 800ms×6)가 7633ms에 끝난다.
- 팔각별: 09-15에 프로젝트가 추가한 editor 배치 1(ARCH01A_FEIYI 변형, 스폰에서 0.54m), mapmaterials placementLighting과 CharacterSelectFloorSwap hiddenSourcePlacementIds가 참조.
- Object 시퀀서: `m_SelectedBoxKind`(0 Transform/1 Animation/2 Effect/3 Collider/4 Stage)와 family별 index를 쓰지만 Transform 박스 강조식에 kind 조건이 빠져 첫 Transform 박스가 항상 강조되고 Box Detail이 kind로 분기하지 않는다. Map position은 Mark_Dirty만 하고 preview는 다음 Seek에서 재시작된다(radius 편집기는 즉시 Seek).
- Character Workbench: `CCharacterActionWorkbench`가 Animation/Effect/Collider 유사 탭과 연속 clip 행만 그리고 Stage/공백/Logic/Sound lane과 Kouku식 Box Detail이 없다. 콜라이더 정본은 `Data/Animation/HitShapes/<Asset>.hitshapes.json`(v4 Workbench 소유), 사운드는 Authored `.animevents` SOUND 행(Animation Tool 소유), 후딜레이는 `PlayerSkills.json actionDurationMs`(Balance 소유).
- MapTool: 배치 뷰포트 피킹 없음(리스트 선택만), 외곽선 없음(CTrigger_Box 재사용 가능), 복제 없음, 캐릭터 선택 Level은 runtime-attach 대상이 아님. 배치 이동은 batched instance에도 `Update_Instance`로 가능.

## G02. 변경 파일

| 축 | 파일 | 변경 |
| --- | --- | --- |
| 1 | `Data/Effects/Authored/effect.kouku.common.spinning.card.throw.effect.json` | 24요소 `visible` false→true (바이트 보존) |
| 2 | `Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json` | P48 logic.1 durationMs 8200, STAGE_11 800→4900 + `LOOP_TO_WINDOW`, revision +1; KoukuSaydon publish |
| 3 | `Data/Maps/Authoring/LV_LOBBY_CLASSSELECT_SL00/*.mapplacements`, `*.mapmaterials.json`, `Data/Rendering/Authored/CharacterSelectFloorSwap.json`, `Data/Maps/MapCatalog.json` | editor:1 행·lighting·hidden id 제거, count 804; `Publish-MapAuthoring -Scope Area` |
| 4 | `Client/Private/WorldObjectTool.cpp`, `.h` | kind guard, Effect Box Detail 분기, Map position 즉시 Seek, Resources 탭 |
| 5 | `Client/Private/CharacterActionWorkbench.cpp`, `Client/Public/CharacterActionWorkbench.h` | 탭 바, ROW_KIND STAGE/GAP/TIMING, lane 배치, 콜라이더 편집·추가·삭제, Source Out, 사운드 Box Detail |
| 6 | `Client/Private/MapTool*.cpp`, `MapPlacementRuntime.cpp/.h`, `Level_CharacterSelect.cpp/.h`, `Level_KakulSaydonArena*.h/.cpp`, `Loader.cpp`, `WorldObjectTool.cpp`, `MainApp*.cpp` | IMapAuthoringHost, 피킹/외곽선/복제, Save+Publish runner, World tool Map 카테고리 |

## G03. 데이터·호출 흐름

- 잔상: 요소 span을 raw_decode로 찾아 `"visible": false` 한 줄만 교체. codec Load/Drawable/Roundtrip/Stage + 15초 playback으로 확인.
- 발사 창: 최신 저장 revision을 다시 읽고 P48 span 안에서 logic.1 durationMs·STAGE_11 durationMs/playMs/endPolicy만 교체, root revision +1. projector 규칙(startMs+durationMs ≤ pattern lifetime, END_POLICIES)을 만족한다. `Invoke-BuildDomainOwner -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision <rev>`로 Encounter/Server bootstrap을 게시한다. Workbench가 저장 중이면 CAS로 실패하므로 저장이 멈춘 뒤 재시도한다.
- 팔각별: 행 삭제 + header count, lighting 객체 삭제(dangling 검사), floor-swap hidden id 삭제, MapCatalog count 동기화, Validate→Publish→Check.
- 코드 축은 각 W 분석(`out/StaggerBallValtan20260917/…` 및 scratchpad wf2_w1/w3/w5)의 anchor를 따르며 구현 에이전트가 TU별 격리 컴파일로 검증한다.

## G04. 검증

1. 데이터: codec/playback probe, publisher exit code·runtime 파일 SHA·row count, `git diff --check`.
2. 코드: touched TU 격리 컴파일(out/<group>/compile-*.log), 독립 리뷰 P0/P1 수정, 이후 사용자 Product Build.
3. 사용자 확인 경로: Lobby→Character Select(별 제거), F1 Action Workbench→Object→공_튀기기 Effect 박스 단독 선택/Map position 드래그, →Character(탭·lane·콜라이더), →Object→Map→Pick World Object/Duplicate/Save, KoukuSaydon P48 Complete Play(잔상·8.2초 발사).
