# 발탄 아레나 World Destruction 저작 결과

대응 계획: `2026-08-07_VALTAN_WORLD_DESTRUCTION_PLAN.md`
브랜치: `feature/maptool-world-destruction-bern-nav`

## 0. 계획과 실제 반영 범위가 다르다 — 먼저 읽을 것

PLAN 2절은 이번 G1을 **읽기 전용 진단 모드**로 못박고 "저장 경로, dirty 플래그, runtime 상태
변경을 만들지 않는다"고 적었다. PLAN 4절 파일 목록도 `CEncounterPatternReference` 한 클래스와
`MapTool.h` / `MapTool.cpp` 두 파일만 든다.

실제 반영된 코드는 그 경계를 넘었다.

```text
PLAN에 없는 것   Client/Public/WorldDestructionDocument.h
                 Client/Private/WorldDestructionDocument.cpp
                 CMapTool::Save_WorldDestruction / Apply_DestructionPreview / Try_PickDeployProp
                 Client/Public/DeployPropObject.h 의 Get_WorldBounds
PLAN 문자열 검사  grep -c "WorldDestructionDocument" PLAN = 0
```

그 결과 PLAN이 G1 종료 증거로 든 두 불변식은 현재 코드에서 성립하지 않는다.

| PLAN G1 불변식 | 현재 코드 |
|---|---|
| 저장 경로 0개 | `CMapTool::Save_WorldDestruction()` 존재 (`MapTool.cpp:3514`) |
| `Has_UnsavedAuthoring()` 이 이 모드 사용 후에도 false | `m_DestructionDocument.Is_Dirty()` 를 소비 (`MapTool.cpp:2234`) |

`Save_AllAuthoring()` 도 destruction 문서를 저장 대상에 넣는다 (`MapTool.cpp:580`).

따라서 이 RESULT는 **PLAN 5절을 코드 정본으로 취급하지 않는다.** 코드가 정본이고 PLAN에서
현재도 유효한 부분은 0절 실측과 1절 G 경계표뿐이다. G 경계표 기준으로 실제 반영 범위는
G1(진단) + G2(문서와 strict parser, atomic save) + G3(그룹 저작) + G4의 미리보기 일부다.
G5 이후는 착수하지 않았다.

PLAN 본문을 실제 코드에 맞춰 다시 쓰는 일은 이번 변경 단위에 넣지 않았다. 문서를 코드에
맞추는 교정은 다음 작업에서 별도로 한다.

## 1. 구현 완료

### 1.1 새 파일

| 파일 | 줄 수 | 역할 |
|---|---|---|
| `Client/Public/EncounterPatternReference.h` | 86 | Encounter pattern/stage 읽기 전용 참조 계약 |
| `Client/Private/EncounterPatternReference.cpp` | 314 | `Data/Encounters/Valtan/ValtanEncounter.json` strict parse |
| `Client/Public/WorldDestructionDocument.h` | 207 | 파괴 그룹·mutation·binding 저작 계약 |
| `Client/Private/WorldDestructionDocument.cpp` | 1084 | 위 계약의 parse / validate / save 구현 |

네 파일 모두 UTF-8 BOM 없음이고 non-ASCII 바이트가 0이다.

`CEncounterPatternReference`는 Save 함수를 갖지 않는다. 쓰기 정본은 Balance Tool이다.

`CWorldDestructionDocument`의 저작 대상은 `Data/Encounters/Valtan/ValtanWorldEvents.json`이고
schema는 `lostark.world-destruction-events`다 (`WorldDestructionDocument.cpp:17`).
저장하는 것은 안정 ID 관계뿐이다.

```text
DESTRUCTION_GROUP     groupId, memberPlacementIds(.deployplacements 의 runtimePlacementId),
                      navigationRegionIds, nav polarity, 초기 상태
DESTRUCTION_MUTATION  mutationId, groupId, 목표 상태, iBreakingDurationMs
DESTRUCTION_BINDING   bindingId, mutationId, patternId, stageId, trigger kind,
                      iOffsetMs, receiverCollisionId, isEnabled(기본 false)
```

deploy transform은 `.deployplacements`, navigation cell은 `.navblockers`, stage timing은
encounter profile이 계속 소유한다. 이 문서는 runtime 값을 갖지 않는다.

`DESTRUCTION_NAV_POLARITY`가 그룹마다 따로 있는 이유는 publish된 Valtan navgrid가 deploy
footprint를 전부 walkable로 굽기 때문이다. 온전한 벽은 blocker를 **추가**해야 하고 아레나
바닥은 반대로 무너질 때 막아야 한다. bool 하나로는 두 방향을 표현할 수 없다.

### 1.2 수정 파일

| 파일 | 증감 | 내용 |
|---|---|---|
| `Client/Public/MapTool.h` | +51 | `TOOL_MODE::WORLD_DESTRUCTION`, descriptor의 `encounterReference` / `worldEventsDocument`, 패널 9개와 저작 함수 7개 선언, 저작 상태 멤버 |
| `Client/Private/MapTool.cpp` | +1206 / -1 | 모드 라디오, 패널 switch, 패널 구현, Valtan descriptor 확장, 저장·미리보기·피킹 |
| `Client/Public/DeployPropObject.h` | +8 | `Get_WorldBounds()` 선언 |
| `Client/Private/DeployPropObject.cpp` | +61 | 현재 렌더 중인 model의 local bounds를 배치 transform으로 회전·스케일해 world AABB로 환산 |
| `Client/Default/Client.vcxproj` | +4 | 새 `.h` 2개, `.cpp` 2개 등록 |
| `Client/Default/Client.vcxproj.filters` | +12 | 위 4개를 `03. Tools\00. Map` 필터에 등록 |

`Get_WorldBounds()`는 저작 도구가 prototype tag로 다시 clone하지 않고 prop을 hit-test하고
외곽선을 그리기 위한 경계다. baked local bounds가 없으면 `false`를 돌려 호출자가 0 크기
상자를 쓰지 않고 해당 prop을 건너뛰게 한다.

### 1.3 MapTool에 추가된 함수

```text
Load_EncounterReference        MapTool.cpp:1449
Load_WorldDestruction          MapTool.cpp:3480
Save_WorldDestruction          MapTool.cpp:3514
Try_PickDeployProp             MapTool.cpp:3534
Refresh_DestructionHighlight   MapTool.cpp:3576
Apply_DestructionPreview       MapTool.cpp:3646
Render_DestructionGroupEditor  MapTool.cpp:3675
Render_DestructionBindingEditor MapTool.cpp:3891
Render_DestructionTimeline     MapTool.cpp:4074
Render_WorldDestructionPanel   MapTool.cpp:4178
Render_DestructionEncounterSource MapTool.cpp:4212
Render_DestructionDeployList   MapTool.cpp:4340
Render_DestructionWorldRows    MapTool.cpp:4418
Render_DestructionNavigationRegions MapTool.cpp:4461
Render_DestructionDiagnostics  MapTool.cpp:4511
```

`worldEventsDocument`는 Valtan descriptor에만 설정된다 (`MapTool.cpp:2178`). 다른 Area는
경로가 비어 있어 `Get_WorldDestructionPath()`가 빈 경로를 돌려주고 패널이 저작 없음으로
표시한다.

## 2. 자동 검증 (실행함)

| 항목 | 결과 |
|---|---|
| Client x64 Debug 빌드 | 성공. MSBuild exit 0, `Client\Bin\Debug\Client.exe` 링크 |
| 새 파일 컴파일 | `EncounterPatternReference.cpp`, `WorldDestructionDocument.cpp` 둘 다 컴파일 로그에 나옴 |
| 신규 경고 | 없음. `C4819`(CP949 코드페이지)와 `LNK4099`(DirectXTK PDB)는 이번 변경 이전부터 있던 것 |
| 새 파일 인코딩 | 4개 모두 UTF-8 BOM 없음, non-ASCII 0바이트 |
| `git diff --check` | clean |
| `Publish-WorldGameplay.ps1 -Mode Validate` | 통과. BERN 7 / VALTAN_ARENA 12 / TRAINING_GROUND 4 / CHARACTER_SELECT_ARENA 5 placements, VALTAN_ARENA 3 spawn groups |
| `Publish-ServerNavigation.ps1 -Mode Validate` | 통과 |
| ProjectAudit `projects.data-source-visibility` | expected=545 project=545 filters=545 |
| ProjectAudit 전체 | **실패 1건.** 아래 2.1 |

### 2.1 남아 있는 ProjectAudit 실패는 사전 존재

```text
Project audit failed (1): effect.g09-authoring-world-runtime-boundary:
  paths=0 authoredUnexpected=0 intake=2 shaders=1 symbols=0 project=0
  entry=False detailPreview=True
```

Effect Tool 저작/런타임 경계 검사이며 이번 변경은 Effect 파일을 하나도 건드리지 않았다.
같은 실패가 `2026-08-07_MAPTOOL_NAVIGATION_BOOTSTRAP_GATE_RESULT.md` 4.1절에도 baseline
실패로 기록돼 있다. 별도 담당 작업으로 남긴다.

이 작업 전에는 `projects.data-source-visibility`도 함께 실패했다
(`expected=545 project=543 filters=543`). 원인은 Bern navigation 저작본 두 개가
`Client.vcxproj`에 등록되지 않은 것이었고, navigation 관문 커밋에서 `96.DataFiles\Navigation`
`None` 항목으로 등록해 해소했다.

## 3. 수동 검증 (미실행 — 사용자 확인 대기)

Client는 GUI이므로 아래는 실행해서 눈으로 확인해야 한다. **PASS로 기록하지 않는다.**

PLAN 7.3~7.5의 절차 전부가 여기 해당한다.

```text
1. Server + Client 실행 -> Lobby -> Test -> LEVEL::DEVELOPMENT -> F1 -> Map Tool
2. Valtan 선택 후 status commit 대기
3. 모드 바가 Map Assets | World Gameplay | World Destruction | Navigation | Camera 다섯 개인지
4. Reload Encounter Reference 로 31 pattern, Deploy 85행이 보이는지
5. 손상된 ValtanEncounter.json 에서 표가 유지되고 상태 문자열만 실패로 바뀌는지
```

PLAN이 다루지 않는 새 경로는 검증 절차 자체가 PLAN에 없다. 최소한 다음은 별도로 확인해야 한다.

```text
그룹 생성 -> 멤버 추가 -> Save -> Area 재진입 -> Load 왕복이 동일한지
잘못된 문서에서 기존 문서가 보존되는지
Apply_DestructionPreview 의 INTACT/FRACTURED 전환이 화면에 보이는지
Try_PickDeployProp 의 선택이 Get_WorldBounds 결과와 맞는지
Save 하지 않고 Area 를 바꿀 때 Has_UnsavedAuthoring 이 물어보는지
```

## 4. 남은 경계

### 4.1 첫 Save가 ProjectAudit을 다시 깨뜨린다

`Data/Encounters/Valtan/ValtanWorldEvents.json`은 아직 존재하지 않는다. 현재
`Data/Encounters/Valtan/`에는 `ValtanEncounter.json` 하나뿐이다.

Map Tool에서 처음 Save하면 이 파일이 생기고, 그 순간 `projects.data-source-visibility`가
`expected=546 project=545`로 다시 실패한다. audit이 `git ls-files --cached --others
--exclude-standard -- Data`로 tracked와 untracked를 모두 세기 때문이다.

Save한 뒤에는 같은 변경 단위에서 다음을 함께 추가해야 한다.

```xml
<None Include="..\..\Data\Encounters\Valtan\ValtanWorldEvents.json" />
```

```xml
<None Include="..\..\Data\Encounters\Valtan\ValtanWorldEvents.json">
  <Filter>96.DataFiles\Encounters</Filter>
</None>
```

### 4.2 publisher / Server / Shared 미연결

`Publish-WorldGameplay.ps1:401-402`는 placement kind를
`playerSpawn, npc, boss, triggerBox, collisionBox`로만 허용하고 그 외를 던진다.
`destroyable`은 여전히 fail-closed다. publisher는 `ValtanWorldEvents.json`을 읽지도 않는다.

따라서 지금 저작한 내용은 Server 런타임에 아무 영향을 주지 않는다. G5(publisher 검증),
G6(Server 상태와 collision receiver), G7(Shared 복제), G8(Client `Set_State` 서버 연결)이
닫히기 전에는 "벽이 실제로 부서진다"고 판단하지 않는다.

### 4.3 착수하지 않은 항목

```text
G9   타임라인 미리보기 재생
G10  돌진 충돌 판정과 스킬 이동 sweep 보강
G11  아레나 바닥 붕괴와 낙사 hazard
G12  파괴 Effect / Camera / Audio
G13  나머지 그룹 확장
```

### 4.4 팀 문서

PLAN 8절대로 G1 단계에서는 팀 문서를 갱신하지 않는다. publisher가 `destroyable`을
admission하는 G5에서 `AREA_DATA_LAYER_GUIDE.md`,
`UNIFIED_DATA_MANAGEMENT_ARCHITECTURE.md`, `BALANCE_TOOL_OWNER_HANDOFF.md` 세 곳을 같은
변경 단위에서 교정한다.

## 5. 다음 작업에서 먼저 할 것

```text
1. 3절 수동 검증을 실행하고 결과를 이 문서에 채운다
2. PLAN 본문을 실제 반영 코드(WorldDestructionDocument 포함)에 맞춰 교정한다
3. 첫 Save 이후 4.1의 project 등록을 같은 커밋에 넣는다
4. G5 publisher 검증부터 순서대로 연다
```
