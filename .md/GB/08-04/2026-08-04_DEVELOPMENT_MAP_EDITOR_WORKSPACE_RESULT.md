# Development Map Editor Workspace RESULT

작성일: 2026-08-04

상태: 코드·데이터 정책·Release 빌드 반영 완료. Debug 수동 UI smoke는 미실행.

## 1. 완료 결과

Debug Lobby의 `Test`는 기존 `TRAINING_GROUND` 서버 승인을 받은 뒤
`CMapEditorWorkspaceService`에 editor intent를 세우고 `LEVEL::DEVELOPMENT`를 격리된 Map
Editor Workspace shell로 연다. F1과 `Map Tool` 버튼은 Level 전환을 하지 않는다. editor
shell의 Loader와 Level은 제품 training map, playable character, socket replication,
player controller를 생성·갱신하지 않는다. Release Test는 기존 제품 training 경로를 유지한다.

MapTool selector에는 다음 네 Area만 노출한다.

| 표시 | AreaId | visual | navigation | gameplay |
|---|---|---|---|---|
| Character Select | `LV_LOBBY_CLASSSELECT_SL00` | 803 placements | source/paint bootstrap | disabled |
| Bern | `LV_BER_BERNCASTLE` | 50,017 placements | disabled | required |
| Valtan | `LV_LUT_HEARTRB_ED` | 13,192 placements | source/paint/blockers | required |
| Training Map | `LV_SHS_RCARENA_D` | 7,856 placements | disabled | disabled |

Training Map은 제품 Test용 `LV_DEV_TRAINING_GROUND`가 아니라 원본 302 assets
`LV_SHS_RCARENA_D`로 교정했다.

## 2. 구현 경계

- Area descriptor는 제품 `CLevelRegistry`와 분리했다.
- source path는 `Data/Maps/MapCatalog.json`의 exact `sourceCatalog`와
  `sourcePlacements`만 읽는다.
- `CMapAssetCatalog::Load_Source`가 single catalog와 Bern shard-set을 같은 catalog
  validation 경로로 읽는다.
- `Load_Source`가 editor runtime prototype tag에 Area namespace를 붙인다. Character Select와
  Valtan처럼 serialized tag는 같지만 area별 `.wmodel`이 다른 경우에도 한 Development Level의
  prototype fingerprint가 충돌하지 않는다. 제품 `Load_Area`와 저장 문서는 그대로 유지한다.
- published `Client/Bin/DataFiles/Map` fallback을 제거했다.
- staged catalog/placement/world/navigation이 모두 성공한 뒤 기존 editor-owned runtime
  object를 exact pointer 목록으로 제거하고 새 상태를 commit한다.
- prototype tag 재사용 시 normalized model path fingerprint가 같아야 한다.
- Area selector pending 값과 active save descriptor를 분리했다.
- visual/gameplay/nav paint/nav blocker dirty는 Area switch와 editor exit에서
  `Save and Continue / Discard and Continue / Cancel` gate를 통과한다.
- commit 뒤 placement bounds로 free camera를 frame한다.
- MapTool navigation save의 `.Export_Runtime` 호출을 제거했다. 저장은 Data paint/blocker만
  수행하며 runtime blocker 등록도 editor shell에서 거부한다.
- Valtan DeployProp authoring은 source/stage 계약이 없으므로 이번 UI와 load에서 제외했다.
- `ACTIVE.maparea`는 selector 변경으로 갱신하지 않는다.

## 3. 맵 담당자 사용 절차

1. Lobby에서 `Test`를 누르고 서버 승인을 기다린다.
2. 창 제목이 `LostArk Map Editor Workspace`가 될 때까지 기다린다.
3. F1을 눌러 공통 Developer Tools를 열고 `Map Tool`을 선택한다.
4. 기본 Character Select Area가 commit될
   때까지 기다린다.
5. 상단 Area selector로 Character Select, Bern, Valtan, Training Map을 전환한다.
6. `Map Assets`, 정책이 허용된 `World Gameplay`, `Navigation` 탭에서 편집한다.
7. Area 전환 또는 `Exit to Lobby` 때 dirty gate를 처리한다.
8. visual 저장 뒤 `Publish-MapAuthoring.ps1`, world 저장 뒤
   `Publish-WorldGameplay.ps1`, Server nav가 필요하면 `Publish-ServerNavigation.ps1`을 별도로
   실행한다.

Bern은 50,017 placements를 동기 stage한다. 선택 직후 창이 오래 응답하지 않을 수 있으므로
status가 새 active Area로 바뀔 때까지 기다리고 중복 선택하지 않는다.

## 4. 자동 검증

### 통과

- JSON/XML parse: `MapCatalog.json`, `Client.vcxproj`, `.filters`
- 네 authoring source catalog와 placement 실재 확인
- Valtan authoring placement header와 `MapCatalog.json` count 13,192 일치
- `Load_Source` editor runtime prototype의 Area namespace 격리 정적 계약 확인
- scoped `git diff --check`: whitespace error 없음
- direct Client x64 Debug build: 오류 0, 링크 성공
- canonical Release:
  - Engine/Shared/Server/Client build와 Client 링크 성공
  - NetworkProtocolHarness: failures 0
  - ClientFrontendHarness: failures 0
  - Server contract test: failures 0
  - gameplay balance/navigation validate 성공
- ProjectAudit의 신규 `maps.editor-workspace-policy` 포함 Map 관련 check 통과
- 정적 확인: `Client/Private/MapTool.cpp`에 `.Export_Runtime(` 호출 없음

### 전체 PASS가 아닌 기존 외부 상태

- 이번 회귀 확인의 ProjectAudit 최종 결과는 다음 두 건만 실패했다.
  `asset-lock.inventory`와 `gameplay.balance-publish-contract`는 이 Map Tool 수정 범위 밖의
  기존 외부 상태다. Map catalog와 Server navigation validation은 통과했다.
- `asset-lock.inventory`:
  현재 Resources는 10,256 files / 5,507,131,395 bytes이며, 사용자가 폐기하기로 한 이전
  immutable resource pack lock과 불일치한다. 이번 Map Editor 변경에서 lock/payload를
  다시 만들지 않았다.
- `gameplay.balance-publish-contract`: 현재 extractor 기준 balance provenance receipt가
  stale 상태다. 맵 editor runtime prototype 및 navigation 데이터와 무관하므로 이 PR에서
  receipt를 재생성하지 않았다.
- canonical Debug 첫 실행은 동시에 수정 중이던 `Animation_Tool`/
  `AnimationAuthoringBridge`의 중간 상태 때문에 Client compile에서 실패했다. 그 뒤 Release는
  같은 파일까지 포함해 전체 Client 링크에 성공했다.
- canonical Debug 재실행은 `UpdateLib.bat`의 Engine.dll copy에서 sharing violation으로
  중단됐다. 실행 중인 다른 세션/프로세스를 강제 종료하지 않았다.

## 5. 수동 검증 상태

이번 작업에서는 Client를 새로 실행해 Test/F1/Area selector를 누르는 UI smoke를 수행하지 않았다.
다른 세션의 실행·빌드를 임의로 종료하지 않기 위해 자동 빌드와 정적 계약까지만 닫았다.
따라서 다음 항목은 맵 담당자 첫 실행에서 확인해야 한다.

- Character Select 최초 stage와 camera framing
- Character Select → Valtan → Bern → Training Map 전환
- dirty gate 세 선택
- gameplay/navigation disabled policy 표시
- Lobby Test 승인 → editor shell 진입과 F1의 tool-only 동작

## 6. 의도적으로 남긴 후속

- exact floor placement admission을 저장하는 `.navbake`
- navsource provenance/digest v3
- Bern cancellable parse와 main-thread incremental stage
- Valtan DeployProp의 exact source/stage/save transaction
- 일반 monster/wave/trigger vertical slice

Character Select navigation의 floor 선택은
`2026-08-04_CHARACTER_SELECT_NAVIGATION_AUTHORING_PLAN.md` 후속 계약을 따른다. 현재 generic
baker의 visible static 전체 수집을 `Use for Navigation Bake` 완료로 간주하지 않는다.

## 7. Bern 최초 카메라 프레이밍 교정

### 원인

Bern visual placement 50,017개의 전체 bounds에는 도시 본체와 멀리 떨어진 배경·이벤트
오브젝트가 포함돼 있었다. 기존 `CMapTool::Switch_EditorArea`는 전체 min/max 중점과 span으로
카메라를 배치했기 때문에 도시가 화면에서 매우 작게 보였다. Bern gameplay의 `playerSpawn`
4개는 현재 원점 주변 placeholder라 최초 카메라 기준으로 사용할 수 없었다.

### 구현

- Bern Area commit 직후 finite placement의 X/Y/Z를 정렬한다.
- 축별 중앙값을 최초 look-at center로 사용한다.
- X/Z 5%~95% span 중 큰 값으로 radius를 계산해 먼 outlier를 제외한다.
- 계산 실패 시 기존 full-bounds frame을 유지한다.
- Valtan의 기존 playable-center override와 모든 저장 문서는 변경하지 않았다.

현재 정본 50,017개에서 계산된 값은 다음과 같다.

```text
center = (145.568486, 41.5955225, -69.5091992)
central span X = 196.4528422
central span Z = 431.206426
radius = 150.9222491
```

### 검증

- `Client/Default/Client.vcxproj` x64 Debug build: 성공
- Engine/Shared/Client 증분 링크: 성공
- `Client/Bin/Debug/Client.exe` 생성: 성공
- CP949, BOM 없음, CRLF 유지: 성공
- placement, catalog, gameplay, navigation 데이터 변경: 없음
- `git diff --check`: 성공
- `ProjectAudit`: Map/navigation 검증과 gameplay balance validate/publish는 성공했으나 전체 결과는
  기존 asset pack lock 불일치, Effect G1 문서 경계, DimensionMaster asset/runtime-animation
  항목 4건으로 실패했다. 이번 카메라 변경 파일과 직접 관련된 실패는 없다.
- Bern 수동 진입 육안 검증: 사용자 실행 확인 대기

## 8. Map Editor와 제품 Bern/Valtan 표시 데이터 연결

### 원인 확인

- Map Editor는 `Data/Maps/Imported` catalog와 `Data/Maps/Authoring` placement를 읽는다.
- 제품 Bern/Valtan은 publisher가 만든 `Client/Bin/DataFiles/Map`만 읽는다.
- Bern source와 runtime의 mapset 및 13개 shard catalog/placement는 byte hash까지 일치했다.
- Valtan은 catalog가 byte hash까지 일치했고 placement 13,192개도 줄바꿈만 정규화하면 내용이
  일치했다.
- 제품 화면이 잘려 보인 직접 원인은 데이터 불일치가 아니라 `CLevelRegistry`의 좁은
  `MAP_LOAD_SCOPE`였다. Loader와 `CMapPlacementRuntime`이 범위 밖 placement를 제거하고 있었다.

### 구현

- `MAP_LOAD_SCOPE`에 optional `excludedAssetGroupId`를 추가했다.
- scope cache 비교에도 제외 group ID를 포함해 서로 다른 scope의 staged record를 재사용하지 않는다.
- Bern 제품 Level은 published map 전체를 읽되 `groupId == "landscape"`인 42개 placement만
  제외한다. 이는 Map Editor의 기본 `Show Bern Landscape = false` 화면과 같은 정책이다.
- Valtan 제품 Level은 published placement 13,192개 전체를 읽는다.
- Character Select와 Development descriptor의 기존 scope는 변경하지 않았다.
- Loader와 runtime placement가 동일한 descriptor scope를 계속 소비하며, 제품 Level이 Authoring
  문서를 직접 읽는 우회 경로는 추가하지 않았다.

### 기대 배치 수

```text
Bern source total       = 50,017
Bern landscape excluded = 42
Bern product visual     = 49,975
Valtan product visual   = 13,192
```

### 검증

- `Client/Default/Client.vcxproj` x64 Debug build: 경고 0, 오류 0
- `Client/Bin/Debug/Client.exe` 링크: 성공
- `maps.product-editor-visual-scope` ProjectAudit check: 성공
- `git diff --check`: 성공
- ProjectAudit 전체 결과: 기존 무관 항목 4건만 실패
  (`asset-lock.inventory`, `effect.g1-document-boundary`, DimensionMaster asset/runtime-animation 2건)
- Lobby에서 Bern/Valtan을 각각 진입해 Map Editor 화면과 육안 비교하는 수동 smoke: 사용자 실행 확인 대기

이후 Map Editor에서 저장한 변경을 제품 Level에 반영할 때는 제품이 Authoring 문서를 직접 읽게
바꾸지 않는다. 반드시 Area별 `Publish-MapAuthoring.ps1`을 거쳐 runtime 문서를 갱신한다.

## 9. 제품 Bern 진입 위치 연결

### 추가 원인 확인

- Bern Authoring의 단일 placement 50,017개와 runtime 13개 shard를 합친 placement 50,017개는
  행 단위 집합까지 일치했다. 따라서 제품 Bern이 Map Editor와 다르게 보인 원인은 시각 맵 데이터
  누락이 아니었다.
- `Data/Worlds/LV_BER_BERNCASTLE/Gameplay.world.json`의 `playerSpawn` 4개가 모두 원점 부근
  임시 좌표를 사용하고 있었다.
- 제품 Bern 카메라는 Server가 승인·생성한 플레이어를 따라가므로, Map Editor처럼 배치 bounds의
  중앙을 자동으로 바라보지 않는다. 이 차이 때문에 동일한 맵을 로드하고도 멀리 떨어진 화면으로
  시작했다.

### 구현

- SL00의 실제 바닥 `bg_ber_berncastle_floor02d_sm_ksr` 배치가 존재하는
  `(144.818887, 42.56, -70.3215674)` 부근을 제품 진입 기준점으로 선택했다.
- Bern gameplay authoring revision을 2에서 3으로 올리고 네 spawn slot을 다음처럼 배치했다.

```text
entry   = (144.8, 42.7, -70.3)
party02 = (145.8, 42.7, -70.3)
party03 = (143.8, 42.7, -70.3)
party04 = (144.8, 42.7, -71.3)
```

- Client의 카메라나 Bern Level에 좌표를 하드코딩하지 않았다. Server authority 정본인
  `Gameplay.world.json`만 수정하고 `Publish-WorldGameplay.ps1`로
  `Server/Bin/DataFiles/World/BERN.worldbootstrap`을 다시 생성했다.

### 검증

- `Publish-WorldGameplay.ps1 -Mode Validate`: 성공
- `Publish-WorldGameplay.ps1 -Mode Publish`: 성공
- 생성된 `BERN.worldbootstrap`: revision 3 및 네 spawn 좌표 일치
- `Server.exe --contract-test`: failures 0
- ProjectAudit의 world/map 관련 검사: 성공
- ProjectAudit 전체 결과: 기존 무관 항목 4건만 실패
  (`asset-lock.inventory`, `effect.g1-document-boundary`, DimensionMaster asset/runtime-animation 2건)
- Server와 Client를 완전히 재시작한 뒤 Lobby의 Bern 명령으로 진입하는 수동 smoke: 사용자 실행 확인 대기
## 8. 2026-08-05 Valtan Area 전환 회귀 수정

### 원인과 데이터 보존 확인

- `dc6ed57`에서 네 Area를 하나의 Development Level에서 stage하도록 바뀌었지만 source catalog의
  prototype tag는 Area 범위로 격리되지 않았다.
- 기본 Character Select와 Valtan은 동일 prototype tag 3개를 서로 다른 model path로 사용한다.
  Character Select prototype이 남은 상태에서 Valtan을 선택하면 fingerprint 검증이 올바르게 충돌을
  감지했지만, editor 전환에 필요한 Area-local tag가 없어 전체 stage가 rollback됐다.
- Valtan navsource v2는 `392x312`, cell size `0.5`, origin `(-6, -165)`이며 navpaint와
  navblockers도 같은 identity다. source 122,304셀은 중복·범위 오류가 없고 25,593셀에 surface가 있다.
- 이전 Valtan placement LFS object의 13,115행과 현재 13,192행을 stable sourcePlacementId로 비교했다.
  삭제 0, 변경 0이며 navigation bake용 `editor` placement 77행만 추가됐다. 원본 visual map을
  복구하거나 nav v1으로 되돌릴 필요가 없다.

### 반영

- `CMapAssetCatalog::Load_Source`가 parse/validate 성공 후 runtime-only prototype tag에
  `MapEditorArea:<AreaId>:` namespace를 붙인다.
- 제품 `Load_Area`와 serialized catalog는 바꾸지 않았다.
- `MapCatalog.json`의 Valtan `placementCount`를 실제 authoring header와 같은 13,192로 교정했다.

### 검증

- 네 editor Area source 검증 통과:
  - Character Select: 55 assets / 803 placements
  - Bern: 1,003 unique assets / 50,017 placements
  - Valtan: 275 assets / 13,192 placements
  - Training Map: 302 assets / 7,856 placements
- 모든 placement의 asset 참조, placement ID/source ID 중복 검증 통과
- Area-scoped prototype tag의 교차 Area 충돌 0개
- Client x64 Debug build 성공, `MapAssetCatalog.cpp` 컴파일과 Client 링크 성공
- ProjectAudit의 gameplay balance와 세 navigation validate 성공
- 전체 ProjectAudit은 이번 변경과 무관한 기존 `asset-lock.inventory`, 미완성 Effect G4 경계 2건으로 실패
- 실제 Debug UI의 Character Select → Valtan selector smoke는 미실행
