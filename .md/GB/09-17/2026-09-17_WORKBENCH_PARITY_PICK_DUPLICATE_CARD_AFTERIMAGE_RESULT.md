# Workbench 동등화·World Level Tool 피킹/복제·카드 잔상/분출 창·팔각별 제거 결과

## G00. 반영 범위와 실행 경계

데이터 축 5건은 라이브 저장본에 반영·검증했다. 코드 축은 Object 툴, Character 동등화(+콜라이더/사운드 소유자 API),
World Level Tool 배치 편집 세션이며 touched TU 격리 컴파일까지 마쳤다. Client.exe·Server.exe·VS가 실행 중이라
제품 EXE는 교체하지 않았다. 화면 판정은 사용자가 한다.
증거 폴더: `out/StaggerBallValtan20260917/{cards,octagram,spotlight,hoop}`, `out/WorldObjectToolWf2`, `out/CharacterWorkbench20260917`,
`out/CombatSoundOwner20260917`, `out/WorldLevelEdit20260917`, `out/MapToolRevert20260918`, `out/Review_{A,B}`.

## G01. 데이터 축

| 항목 | 반영 | 검증 |
| --- | --- | --- |
| 빙글빙글 카드 잔상 | `effect.kouku.common.spinning.card.throw` sk_13_1 emitter_0 24요소 `visible` true (CRLF 보존) | codec 4/4, playback 19.99s failures=0 |
| 카드 발사 창 2배 | Composition P48 logic.1 durationMs 4100→8200(3533~11733), STAGE_11 800→4900 `LOOP_TO_WINDOW`, lifetime 15833. 사용자 저장 rev 1395까지 유지 | **KoukuSaydon publish 미완료**(사용자 저장 중 CAS 중단). 편집이 끝나면 Workbench `Publish All Patterns` 1회 |
| 캐릭터 선택 팔각별 | editor 배치 `editor:LV_LOBBY_CLASSSELECT_SL00:1` 행·placementLighting·FloorSwap hidden id 제거, MapCatalog count 804 | `Publish-MapAuthoring -Scope Area` Validate/Publish/Check exit 0, 런타임 804행 |
| 쿠크_피자 스포트라이트 수명 | `…par_u_rpcz_spotlight_01_loc_int` 4요소 수명 12.679s | codec 4/4, playback failures=0 |
| 쿠크_훌라후프 수명 | P84 presentation.3(17333ms) 대상 `…par_g_rpcz_throwhoop_01_loc_int` 7요소 창 17.333s. 후프 메시 3개(emitter_5 본체, emitter_10/12 링) 입자 수명 17.333s. 본체 alphaoverlife는 원래 수명 전체 1→0 페이드라 `lookupTableTimeScale` 17.333·`lookupTableStartTime` 0.9423066으로 마지막 1초에만 원본 페이드. 시작 플래시(emitter_7) 입자 수명 유지, 반짝임 스프라이트는 창 전체 방출 | 41 토큰만 변경, CRLF 12655 유지, codec 4/4, duration probe 34.666s failures=0, 라이브 SHA 신선도 검사 후 원자 교체(`hoop/install-receipt.json`) |

훌라후프 문서는 publish 대상이 아니며 authored 파일을 직접 읽는다. 실행 중 Client는 이전 문서를 캐시하므로 Client 재시작 또는 Effect Tool에서 다시 열어야 반영된다.

## G02. Object 툴 (`WorldObjectTool.cpp`, 리뷰 A 반영)

- Transform 박스 강조식에 kind 0 조건, Effect 강조식에 kind 2 조건을 넣어 Effect 박스가 단독 선택된다.
- Box Detail은 kind 2일 때 `Effect Box` 섹션과 Effect Rows 편집기만 그린다.
- Map Position 드래그는 Mark_Dirty + Seek로 preview를 즉시 재시작한다. Preview at Character 체크박스는 별도 줄이다.
- Save 비활성 시 사유 표시. Resources 창은 Effect/Physical/Animation 탭.
- 이전에 넣었던 Map 카테고리 런처(Map Tool로 요청 전달)는 요청 위치가 틀려 제거했다.

## G03. Character 동등화 (`CharacterActionWorkbench`, 리뷰 B, 소유자 API)

- Resources 탭 7개, 시퀀서 lane 7개(Stage/Animation/Logic/Effect/Collider/Sound/Camera), Post-delay GAP 박스, Stage/Timing Box Detail.
- 콜라이더: `CCharacterActionCombatDocument::Insert_CasterHit`/`Remove_CasterHit` 추가, Save는 행 수 비교 대신 id 기반 검사. `Add Collider`(playhead, 선택 콜라이더 shape 복제)·`Remove Collider` 활성. 투사체 hit, stage의 마지막 hit, flat→staged 변환은 사유와 함께 거절.
- 사운드: `CAnimation_Tool::Apply/Add/Remove_CharacterActionSoundEvent`가 `.animevents` 원자 저장을 재사용한다. SOUND 박스 드래그 이동, Box Detail 이벤트 변경·삭제, Sound 탭 `Add Sound at playhead`.
- 격리 컴파일 17 TU exit 0(`out/CombatSoundOwner20260917/compile-workbench.log`).
- Save Combat 뒤 `Publish-GameplayBalance.ps1 -Mode Publish`와 Server 재시작이 필요하다. stage 내부 시작 오프셋 저작은 skillbindings 형식 상향이 필요해 표시만 한다.

## G04. World Level Tool 배치 편집 (`WorldLevelTool`, `MapPlacementEditSession`)

- F1 `World Level Tool` 창의 Map 패널: MapCatalog Area 선택, 배치 행 검색. 현재 Level이 그 Area를 소유할 때(Character Select, Kouku 아레나)만 편집 세션이 붙고, 아니면 사유와 함께 읽기 전용.
- `Pick World Object`: MainApp이 한 번의 viewport 클릭을 독점(ImGui/UI router 검사, Esc·우클릭 취소, Move Player 피커 취소) → 1픽셀 picking → 가장 작은 포함 bounds의 배치 선택. 초록 CTrigger_Box 외곽선이 선택·변형 때 갱신되고 선택 해제·Level 전환·창 닫기에서 제거된다.
- `Duplicate`: 충돌 검사한 `editor:<Area>:<id>` 복제본 생성·선택, 세션에서 만든 복제본만 삭제 가능.
- Position/Euler Rotation/Scale 드래그가 `CMapPlacementRuntime::Apply_PlacementTransform`으로 즉시 반영(배치 batch·mirror 이전 포함). 저장 draft는 live 샘플과 분리.
- `Save`: Bind 시점 bytes와 신선도 비교 → rollback 사본 → `CMapPlacementDocument::Write` → `Publish-MapAuthoring -Scope Area` 비동기. publish 실패 시 이전 bytes 복원, 성공 시 rollback 사본 삭제와 LFS 두 `.mapplacements` 커밋 안내.
- 공용 부품: `MapPlacementRuntime` static(피킹·bounds·transform), `IMapAuthoringHost`(Character Select·Kouku Level 구현), `CMapPublishRunner`.
- Map Tool에 들어갔던 Pick/외곽선/Duplicate/Publish Area UI와 P2 후속 수정은 요청 위치가 틀려 HEAD로 원복했다. `CMapAreaInventory`도 소비자가 없어져 삭제했다.
- 격리 컴파일: `out/WorldLevelEdit20260917` groupC=0, includers=0. 원복 후 재컴파일: `out/MapToolRevert20260918`.
- 알려진 한계: Level 왕복 뒤 재바인딩은 저장하지 않은 draft를 버리고 사유를 표시한다. 같은 Level에서 Map Tool과 동시 편집은 막지 않으며 두 번째 Save가 신선도 검사로 거절된다.

## G05. 실행한 검증

- 격리 컴파일: WorldObjectTool, CharacterActionWorkbench·CombatDocument·Animation_Tool 12 TU, WorldLevelTool, MapPlacementEditSession, MainApp, MainApp_WorldLevel, MapTool 계열(원복 후), Level 계열. 모두 exit 0(기존 C4828만).
- 데이터: codec/duration/playback probe, publisher exit code, 런타임 row count, `git diff --check`.
- 미실행: Product Build, Client/UI 실행, 화면 판정, Kouku publish(사용자 저장 대기).

## G06. 남은 단계

1. Client 종료 후 Product Debug Build.
2. 편집이 끝나면 Workbench `Publish All Patterns`(P48 발사 창, P84 훌라후프 박스 반영).
3. 확인 경로: Lobby→Character Select(별 제거) → F1 World Level Tool → Map → `LV_LOBBY_CLASSSELECT_SL00` → Pick World Object/Duplicate/드래그/Save → exe 재실행.
   F1 Action Workbench → Object(공_튀기기 Effect 박스·Map Position) / Character(탭·lane·콜라이더 추가·삭제·사운드 이동).
   KoukuSaydon P48 Complete Play(잔상·8.2초 발사), P84 훌라후프 17.3초 유지.
4. `git add` 시 authoring·runtime `.mapplacements`(LFS) 포함.
