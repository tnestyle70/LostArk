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
