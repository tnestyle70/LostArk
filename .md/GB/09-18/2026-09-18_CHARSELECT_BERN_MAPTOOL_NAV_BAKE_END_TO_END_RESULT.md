# 2026-09-18 Character Select·Bern Map Tool 네비 bake 전 구간 연결 결과

## 목표

Character Select(`LV_LOBBY_CLASSSELECT_SL00`)와 Bern(`LV_BER_BERNCASTLE`) 레벨에 들어간 상태에서
F1 → Map Tool → Navigation으로 네비를 bake하고 Save한 뒤 서버에 게시해 실제로 쓴다.
막히는 지점을 하나씩 고치는 대신, runtime attach부터 publish까지 전 구간을 정적으로 추적해 전부 고쳤다.

## 사용자 화면 관측 (Character Select)

- `Editing the current level's runtime map.` 표시 → runtime 바인딩 코드는 동작.
- `Workspace status: Spawn anchor box staging failed: LV_LOBBY_CLASSSELECT_SL00`
- `Sublevel jump: 1. DEBUG_REFERENCE_VALTAN_PHASE_SPACEHOLE 2. EDITOR 3. LV_LUT_HEARTRB_E...` (발탄 목록)
- Navigation 탭: `Navigation authoring is disabled for this Area.`

## attach → publish 전 구간 단계표

| # | 단계 | 위치 | 캐릭셀렉 수정 전 | 베른 수정 전 | 수정 후 |
|---|---|---|---|---|---|
| 1 | runtime 대상 획득 | `MapTool_Area.cpp:468-526` | 통과 | 통과 | 통과 |
| 2 | authoring 잠금·복구 | `MapTool_Area.cpp:820-827` | 통과 | 통과 | 통과 |
| 3 | source catalog/placement 존재 | `:829-837` | 통과 | 통과 | 통과 |
| 4 | catalog Load_Source + Bind_RuntimePrototypes | `:839-853`, `MapAssetCatalog.cpp:309-329` | 통과(게시본 재게시 후) | 통과(19152=19152) | 통과 |
| 5 | runtime/source placement ID·asset 대조 | `:866-881` | 통과(805=805) | 통과(50017=50017, 아래 근거) | 통과 |
| 6 | Gameplay + SpawnGroups 로드 | `:887-911` | 통과(anchor 2, group 0) | 통과(SpawnGroups 없음 → 빈 문서) | 통과 |
| 7 | navsource/navpaint/blockers 로드 | `:913-945` | 통과(62×62 0.5m) | 통과(50×347 0.5m) | 통과 |
| 8 | destruction/simulation | `:947-1022` | 해당 없음 | 해당 없음 | 해당 없음 |
| 9 | map lights 로드(읽기만) | `:1023-1034` | 통과 | 통과 | 통과 |
| 10 | WorldSequence panel Load_Area | `:1069-1092` | 통과 | 통과 | 통과 |
| 11 | trigger/collision box staging | `:1108-1116` | 통과(0개) | 통과(trigger 1, collision 1, 프로토타입 있음) | 통과 |
| 12 | **spawn anchor box staging** | `:1128-1138`, `MapTool_WorldGameplay.cpp:416-462` | **실패** | 통과(anchor 0) | **통과** |
| 13 | commit + sublevel jump 재구성 | `:1163-1302` | 미도달 | 도달 | 도달 |
| 14 | Navigation 패널 활성 | `MapTool_NavigationPanels.cpp:106-115` | Area 없음 → disabled | 활성 | 활성 |
| 15 | Place Nav Bounds 피킹 | `MapTool.cpp:198-201, 265-281`, `MapTool_Navigation.cpp:929-955` | — | 통과(엔진 전역 `Picking`, 레벨 무관) | 통과 |
| 16 | Bake Preview 지오메트리 수집 | `MapTool_NavigationBake.cpp:27-164` | — | 통과(runtime 배치 + 레벨에 등록된 맵 모델 프로토타입, 베이커는 파일 경로로 로드) | 통과 |
| 17 | Apply Bake → navsource 쓰기 → 재로드 | `MapTool_NavigationBake.cpp:252-427` → `Load_NavigationDocument` | — | **위험**: 비워크스페이스 분기로 재로드, live Navigation에 blocker 등록 시도 | 워크스페이스와 같은 descriptor 분기 |
| 18 | **Save Navigation** | `MapTool_Navigation.cpp:639-711` | — | **실패**: `Navigation save is only available in the Map Editor workspace` | **통과** |
| 19 | 서버 게시 Validate | `Tools/NavigationPipeline/Publish-ServerNavigation.ps1` | 통과 | 통과 | 통과 |
| 20 | Client runtime navgrid 반영 | 같은 publisher가 `Client/Bin/DataFiles/Navigation/*.navgrid/.navpolicy/.navblockers`도 씀(`:1059-1068`) | — | — | 레벨 재진입 시 Loader가 읽음 |

### 5번 근거 (Bern)

- `LevelRegistry.cpp`에서 Bern은 `MakeBernMapScope()` = `MakeFullMapScope()`(범위 ±FLT_MAX, 제외 그룹 없음) + frustum culling 설정.
- `MapPlacementRuntime.cpp:146-170` `Apply_LoadScope`는 제외 그룹이 없고 `Contains`가 항상 참이라 레코드를 지우지 않는다. frustum culling은 렌더 시점 정책이다.
- `MapPlacementRuntime.cpp:566` 적재 루프는 `outPlacements.size() == records.size()`를 요구한다.

## 확정 원인

### A. `Spawn anchor box staging failed` (캐릭셀렉)

- `Stage_SpawnAnchorBoxes`(`MapTool_WorldGameplay.cpp:431-437`)는 anchor마다 `Add_GameObject_to_Layer(m_iAuthoringLevelIndex, "Prototype_GameObject_TriggerBox", ...)`를 호출한다.
- 그 프로토타입은 `CLoader::Ready_MapAuthoringCore`(`Loader.cpp:1143-1153`, 수정 전)에서 `DEVELOPMENT, BERN, VALTAN_ARENA, KAKULSAYDON_ARENA`만 등록했다. **CHARACTER_SELECT가 빠져 있었다.**
- `Data/Worlds/LV_LOBBY_CLASSSELECT_SL00/SpawnGroups.world.json`은 anchor 2개를 가진다 → 첫 anchor의 clone이 실패.
- 같은 레벨의 trigger/collision box 단계(11번)는 Gameplay 문서에 해당 kind가 0개(`playerSpawn 4, boss 1`)라 우연히 통과했다.
- Bern은 TriggerBox가 등록돼 있고 SpawnGroups 파일이 없어(`CSpawnGroupDocument::Load`는 없는 파일을 빈 문서로 받음) 이 단계를 통과한다.
- 상세 사유(`m_WorldGameplayStatus`)가 `Switch_EditorArea`에서 `"Spawn anchor box staging failed: <AreaId>"`로 덮여 화면에 안 보였다.

### B. 캐릭셀렉에 발탄 서브레벨 목록이 남음

- `Handle_LevelTransition`(`MapTool_Area.cpp:1305-1411`)은 레벨이 바뀔 때 `m_EditorAreas`, catalog, placement 등을 비우지만 `m_EditorSublevelJumps`는 비우지 않았다.
- 발탄에서 만든 목록이 Lobby → Character Select로 넘어오고, Character Select attach가 12번에서 실패해 `Rebuild_EditorSublevelJumps`(`:1298`)에 도달하지 못해 그대로 표시됐다.

### C. runtime attach에서 Navigation Save가 불가능 (캐릭셀렉·베른 공통, 이번에 추가로 찾음)

- `Save_Navigation`(`MapTool_Navigation.cpp:639-711`, 수정 전)은 `CMapEditorWorkspaceService::Is_Active()`가 아니면 무조건 거부했다. runtime attach는 워크스페이스가 아니다.
- 같은 이유로 `Load_NavigationDocument`(`:237-476`)는 runtime attach에서 비워크스페이스 분기를 탔다. 이 분기는 옛 AssetTest용으로, 끝에 `Register_RuntimeBlockers()`로 live 플레이어 Navigation(`Layer_Player`/`Com_Navigation`)을 바꾼다. Apply Bake 직후 이 재로드가 실패하면 Bake가 `Bake validation failed; previous navigation was restored`로 롤백된다.
- attach를 뚫어도 Bake 후 칠하기를 저장할 수 없어 목표(bake → Save)가 막혀 있었다.
- workspace bar 문구(`Save changes authoring files, not Server collision or gameplay.`)가 이미 runtime attach의 계약을 "Data 저작 파일 저장"으로 정의하고 있다.

## 고친 내용

| 파일 | 줄 | 변경 |
|---|---|---|
| `Client/Private/Loader.cpp` | 1143-1147 | `Ready_MapAuthoringCore`의 `_DEBUG` TriggerBox 등록 레벨에 `LEVEL::CHARACTER_SELECT` 추가, 목록의 의미를 주석으로 명시 |
| `Client/Private/MapTool_Area.cpp` | 1114-1115, 1137-1138 | trigger/spawn anchor staging 실패 문구에 `m_WorldGameplayStatus` 상세 사유를 붙임 |
| `Client/Private/MapTool_Area.cpp` | 1378 | `Handle_LevelTransition`에서 `m_EditorSublevelJumps.clear()` |
| `Client/Private/MapTool_WorldGameplay.cpp` | 445-448 | anchor clone 실패 사유에 `Prototype_GameObject_TriggerBox`와 Level 인덱스를 명시 |
| `Client/Private/MapTool_WorldGameplay.cpp` | 455-457 | clone 타입 불일치 시 앞서 만든 anchor 박스를 정리하고 사유를 남김(수정 전에는 사유 없이 반환) |
| `Client/Private/MapTool_Navigation.cpp` | 239-242 | `Load_NavigationDocument`: `Is_Active() \|\| m_bRuntimeAuthoring`이면 descriptor 분기 사용 |
| `Client/Private/MapTool_Navigation.cpp` | 644 | `Save_Navigation`: 같은 조건으로 Data 저장 허용. 경로 대조(`HasSameNavigationPath`)와 destruction 검증은 그대로 |
| `Client/Private/MapTool_Navigation.cpp` | 712 | 거부 문구를 새 조건에 맞춤 |
| `Client/Private/MapTool_Navigation.cpp` | 725 | `Resolve_SelectedNavigationContract`: 같은 조건으로 active descriptor를 Area 권위로 사용 |
| `Client/Private/MapTool_NavigationPanels.cpp` | 33 | Region 콤보의 Area ID도 같은 조건 |

검사를 없애거나 완화한 곳은 없다. `Set_NavigationCondition`과 `Register_RuntimeBlockers`는 바꾸지 않았다.

### 영향 범위 (쿠크·발탄)

C 수정은 runtime attach 전체에 적용되므로 쿠크·발탄 아레나의 runtime attach도 이제 Navigation Save가 가능하고, Bake 후 재로드가 live 플레이어 Navigation에 blocker를 등록하지 않는다. 발탄(`SOURCE_PAINT_BLOCKERS`)은 Save 시 Test 워크스페이스와 같이 `navblockers`도 저장한다. 서버 권위와 runtime publish 경계는 그대로다.

## 검증

- 앵커 11개 전부 정확히 1회 일치 확인 후 바이트 치환(드라이런 → 적용).
- 5개 파일 UTF-8(BOM 없음)·CRLF 유지, lone LF 0, `MapTool_Navigation.cpp`의 기존 비ASCII 61바이트 불변.
- `git diff --check` 경고 없음.
- 격리 구문 검사 `out/IsolatedCompile20260918e/`(`cl /Zs`, `_UNICODE`/`UNICODE`, 출력 파일 없음): `Loader`, `MapTool_Area`, `MapTool_WorldGameplay`, `MapTool_Navigation`, `MapTool_NavigationPanels` 모두 `EXITCODE=0`. 경고는 기존 C4819 55건뿐. 코드 생성·링크는 하지 않았다.
- `Publish-ServerNavigation.ps1 -Mode Validate`(현재 데이터, 게시 안 함)
  - `LV_LOBBY_CLASSSELECT_SL00 62x62, cellSize=0.5, walkable=2368, maxStep=0.474`
  - `LV_BER_BERNCASTLE 50x347, cellSize=0.5, walkable=7674, maxStep=17.03`

## 사용자 절차

1. Client.exe 종료 → VS `Client` `Debug|x64` **Build**(증분). `Loader.cpp`가 바뀌었으므로 **Character Select는 새 빌드로 다시 들어가야** TriggerBox가 등록된다.
2. Lobby → Character Select(또는 Bern) → F1 → Map Tool.
   - 정상: `Workspace status: Active editor Area: Character Select (LV_LOBBY_CLASSSELECT_SL00) / 805 placements. Runtime publish is separate.` Area 콤보에 해당 Area, 발탄 서브레벨 목록 없음.
   - 실패 시 이제 괄호 안에 구체 사유가 붙는다(예: `... could not clone Prototype_GameObject_TriggerBox on Level 3`).
3. F6 자유 카메라로 전환(마우스 클릭이 캐릭터 평타·이동 명령으로 가지 않게).
4. Navigation 탭 → `Region`은 `<AreaId> (base)` → `Bake` 라디오.
   - 정상: `Navigation authoring ready`.
5. `Place Nav Bounds` → 바닥 클릭(`Nav Bounds placed; adjust Transform and press Bake`) → `X Min / Max`, `Z Min / Max`, `Bottom Y`로 범위 조정. Cell Size 0.5 유지. `Grid: W x H = N cells`가 1,000,000 이하인지 확인(베른 마을 전체 약 52만).
6. `Bake Preview` → 칸 수·walkable 확인 → `Apply Bake`.
   - 범위가 바뀌면 `Grid layout changed; confirm reset of paint and regions`가 뜬다 → `Confirm Reset and Rebake`. 이때 기존 칠하기(베른 5,175칸)는 새 격자와 호환되지 않아 지워진다. 원본은 Git에 있다.
   - 정상: `Baked source; Save Navigation to persist authoring paint`.
7. `Walkability`에서 필요한 칸 칠하기 → `Save Navigation`.
   - 정상: `Saved Data authoring only; publisher must build runtime navigation`.
8. `powershell -ExecutionPolicy Bypass -File Tools/NavigationPipeline/Publish-ServerNavigation.ps1 -Mode Validate -AreaId LV_LOBBY_CLASSSELECT_SL00` → 성공하면 같은 명령을 `-Mode Publish`로. Bern은 `-AreaId LV_BER_BERNCASTLE`.
9. Server 재시작 + Client에서 해당 레벨 재진입.

World Gameplay 탭의 Save는 누르지 않는다(`Gameplay.world.json` 전체 재작성).

## 실행해야만 알 수 있어 확인 못 한 것

- Character Select·Bern에서 실제 attach 성공 화면, 피킹, Bake Preview 결과(칸 수·walkable), Save 완료.
- 베른 마을 전체 bounds Bake의 소요 시간과 메모리(수천 개 메시를 베이커가 파일에서 로드).
- 새 격자 크기(예: 52만 칸)에서 `Publish-ServerNavigation.ps1`의 처리 시간. 09-06 계획서는 칸 수가 커지면 PowerShell 행 파싱이 느려진다고 기록했다.
- 베른 현재 `.navpolicy` maxStep 17.03m가 의도된 값인지(이번 범위 밖, 관측만).

## 실수

1. 격리 컴파일 배치를 heredoc 안 Python 문자열로 만들다 백슬래시가 깨져 한 번 실패했다. 세션 gotcha에 있던 항목이다. 기존 배치 파일을 바이트 복제해 목록만 바꾸는 방식으로 해결했다.
