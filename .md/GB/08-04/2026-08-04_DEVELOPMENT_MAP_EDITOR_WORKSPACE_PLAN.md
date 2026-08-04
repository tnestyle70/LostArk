# Development Map Editor Workspace PLAN

작성일: 2026-08-04

상태: 구현 반영 완료. 실제 검증은 대응
`2026-08-04_DEVELOPMENT_MAP_EDITOR_WORKSPACE_RESULT.md`를 따른다. 비평은 1회만 수행했고
P0/P1을 아래 계약에 반영했다.

## 1. 결론

새 `LEVEL::MAP_EDITOR`나 Lobby 명령을 추가하지 않는다. Debug Lobby의 `Test`가 기존
`TRAINING_GROUND` 서버 승인을 받은 뒤 editor intent를 세우고 `LEVEL::DEVELOPMENT`를
**Map Editor Workspace 모드**로 연다. F1은 어느 Level에서나 Developer Tools 표시만
토글하며 Level 전환을 요청하지 않는다.

```text
Lobby -> Test -> S2C_ENTER_ACCEPTED
  -> LEVEL::DEVELOPMENT (Debug editor shell, socket/gameplay 없음)
  -> F1 -> Map Tool
  -> Area selector
     -> Character Select: LV_LOBBY_CLASSSELECT_SL00
     -> Bern:             LV_BER_BERNCASTLE
     -> Valtan:           LV_LUT_HEARTRB_ED
     -> Training Map:     LV_SHS_RCARENA_D
```

여기서 Training Map은 원본 302 assets / 7,856 placements인
`LV_SHS_RCARENA_D`다. 제품 Test용으로 정리된 `LV_DEV_TRAINING_GROUND`와 섞지 않는다.

## 2. 고정 경계

- 제품 `CLevelRegistry`는 제품 Level/MapLoadScope만 소유한다.
- 편집 대상과 authoring 정책은 제품 registry와 분리된
  `CMapTool::EDITOR_AREA_DESCRIPTOR`가 소유한다.
- 편집기는 `Data/Maps/MapCatalog.json`의 `sourceCatalog`, `sourcePlacements`를 정확히
  읽는다. `Client/Bin/DataFiles` fallback은 금지한다.
- 저장은 `Data/Maps/Authoring`, `Data/Navigation`, `Data/Worlds`만 변경한다.
- Client/Server runtime 산출물은 편집기가 직접 생성하지 않는다. 각 publisher만 교체한다.
- Valtan DeployProp은 이번 편집기 전환에서 명시적으로 제외한다. source/stage 계약을
  별도 수직 슬라이스로 닫기 전까지 패널과 로드를 비활성화한다.
- gameplay는 기존 schema의 `playerSpawn / npc / boss`만 다룬다. monster/wave/trigger를
  새로 만들지 않는다.

## 3. Area별 정책

| 표시 | AreaId | Navigation authoring | Gameplay authoring |
|---|---|---|---|
| Character Select | `LV_LOBBY_CLASSSELECT_SL00` | source/paint, 없으면 Nav Bounds bake로 bootstrap | NONE |
| Bern | `LV_BER_BERNCASTLE` | NONE | exact `gameplayDocument`, 파일 필수 |
| Valtan | `LV_LUT_HEARTRB_ED` | source/paint/blockers, 파일 필수 | exact `gameplayDocument`, 파일 필수 |
| Training Map | `LV_SHS_RCARENA_D` | NONE | NONE |

Character Select navigation 경로는 `MapCatalog.json`에 Data 상대 경로로 명시한다.
Bern과 Training Map은 navigation panel을 비활성화하며 문서를 추측해 만들지 않는다.
Character Select와 Training Map은 gameplay panel을 비활성화하며 빈
`Gameplay.world.json`도 만들지 않는다.

## 4. 진입과 제품 Test 격리

1. F1과 Map Tool 클릭은 Level 전환을 요청하지 않는다.
2. Lobby Test는 기존 `TRAINING_GROUND` 서버 승인을 먼저 받는다.
3. Debug 승인 소비 경로가 editor intent를 세우고 `CLevelTransitionService`로 Development
   load를 요청하며, editor가 소유하지 않는 승인 socket은 닫는다.
4. Loader는 editor intent일 때 공용 map shader/object/camera prototype만 준비한다.
5. `CLevel_Development`는 editor shell에서 제품 training runtime, character preload,
   replication, player controller를 만들거나 갱신하지 않는다.
6. editor shell 종료 시 active/intent를 정리한다.
7. Release의 Test는 Debug editor intent가 없으므로 기존 제품 training 경로를 유지한다.

## 5. Area 전환 transaction

selector의 pending Area와 실제 save 대상인 active descriptor를 분리한다.

```text
request
  -> dirty gate (Save and Switch / Discard and Switch / Cancel)
  -> exact descriptor와 Data source 경로 검증
  -> catalog + full placements + optional world/nav parse
  -> model prototype fingerprint 검증
  -> 새 placement runtime stage
  -> 성공 시 기존 editor-owned object exact-pointer 제거
  -> catalog/documents/runtime/camera focus를 한 번에 commit
```

실패·취소 시 active descriptor, 현재 객체, dirty 상태를 유지한다. 새 객체는 전용
generation layer 또는 exact pointer 목록으로 rollback한다. Prototype을 기존 tag로 재사용할
때는 같은 normalized model path인지 검증해 tag 충돌을 성공으로 오인하지 않는다.

dirty 집합은 visual placement, gameplay, navigation paint, navigation blocker다. F1 창을
숨기거나 다른 도구로 전환하는 것은 세션을 유지한다. Area switch/reload/editor exit만
Save/Discard/Cancel gate를 통과한다. `ACTIVE.maparea`는 선택 때 자동 저장하지 않는다.

## 6. 저장과 publish

- visual: active descriptor의 exact `sourcePlacements`
- gameplay: policy가 REQUIRED일 때 exact `gameplayDocument`
- navigation: policy가 허용한 exact Data source/paint/blocker 경로
- navigation save는 `.navgrid` export나 runtime blocker 등록을 하지 않는다.
- product visual: `Publish-MapAuthoring.ps1`
- product gameplay: `Publish-WorldGameplay.ps1`
- server navigation: `Publish-ServerNavigation.ps1`

Bern 50,017 placements는 동기 전환 시 긴 정지가 생길 수 있다. 이번 변경에서는 명시적인
loading 상태와 중복 요청 차단을 제공하고, cancellable parse + main-thread incremental stage는
후속 성능 수직 슬라이스로 남긴다. 완료 문서에서 수동 smoke 여부를 과장하지 않는다.

## 7. Navigation 후속 경계

이번 변경은 Area별 source/paint/blocker load/save와 기존 generic Nav Bounds bake를 editor
workspace에 연결한다. 현재 baker가 visible static mesh 전체를 admission하는 동작을
`Use for Navigation Bake` 완료로 간주하지 않는다.

후속 계약은 stable numeric placement ID와 expected asset ID를 가진 `.navbake` 문서로
정확한 floor placement만 admission하고, `CUL_BOX`는 경계 힌트로만 사용하도록 닫는다.

## 8. 문서와 검증

- `.md/TEAM/AREA_DATA_LAYER_GUIDE.md`: Map Editor 진입, Area 정책, 저장/publish 소유권
- `.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md`: 제품 Level과 editor shell 경계
- 대응 RESULT: 실제 자동 검증, 수동 검증, 미완료 floor admission 분리

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
powershell -ExecutionPolicy Bypass -File Tools/ProjectAudit/Invoke-ProjectAudit.ps1
git diff --check
```

Debug smoke는 Lobby Test 승인 후 editor 진입, F1만으로 전환되지 않음, 네 Area의
load/unload, dirty gate 세 동작, policy별 disabled panel을 확인한다. Release에서는 Debug
Map Editor를 PASS로 기록하지 않고 제품 Lobby/Test/Bern/Valtan 회귀만 기록한다.
