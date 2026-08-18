# 2026-08-16 발탄 아레나 바닥 붕괴 G1 결과

계획 정본은 같은 폴더의 `2026-08-16_VALTAN_ARENA_FLOOR_COLLAPSE_PLAN.md`다.
이 문서는 실제로 반영한 것, 계획과 달랐던 것, 실행한 검증과 실행하지 않은 검증을 분리해 적는다.

## 0. 결론

발탄 아레나 바닥을 SAFE_CORE / FLOOR_STAGE_A_84 / FLOOR_STAGE_B_30 세 계층으로 분리하고,
84줄과 30줄의 authored impact edge에서 해당 sector만 사라지며 그 자리가 Server nav에서
NON-WALKABLE이 되는 계약을 G1 범위로 닫았다.

**낙사(FALLING)는 이번 범위가 아니다.** G1에서 무너진 바닥은 통행 불가일 뿐이며 떨어져 죽지 않는다.

새로 cook한 아트는 0개다. 기존 placement 11개의 소유 관계를 실측해 그대로 사용했다.

## 1. 확정한 sector

| stage | 대상 | placement | 반경(m) | nav cell |
|---|---|---|---|---|
| FLOOR_STAGE_A_84 | 바깥 rail | `7000000000000000001`, `7000000000000000005` | 12.63~16.29 | 347 + 325 = 672 |
| FLOOR_STAGE_B_30 | 벽돌 링 | `7000000000000000002`, `7000000000000000003`, `7000000000000000006`, `7000000000000000007` | 7.12~14.18 | 365+427+360+425 = 1577 |
| SAFE_CORE | wedge 4 + 중앙 캡 1 | SL00 export 1271/1299/1304/1337, `15561800956777256508` | 0.41~8.28 | 없음(영구 보존) |

합계 2,249 cell, Stage A/B 교집합 0, 전부 base-walkable.
아레나 반경은 붕괴에 따라 **15.2m → 약 14.2m → 약 8.3m**로 좁아진다.

cell 소유 규칙은 "그 cell을 마지막까지 받쳐 주는 층이 소유자"이고, 규칙만으로 남는 이음매 cell은
반경 8.60~16.6m 구간에서 안전 코어와 아레나 외부 어느 쪽으로도 나가지 못하는 경우에만 이웃
region으로 흡수한다. 이 봉합으로 82 cell이 흡수됐다.

## 2. 계획서와 달랐던 것

구현하면서 실측으로 드러난 차이다. 계획서에 없던 항목이 6개 있었다.

1. **fractured 강제 지점이 3곳이 아니라 5곳이었다.** 계획서는 `CDeployPropCatalog`와
   `CDeployPropObject` 두 파일만 봤지만, `CLoader::Ready_DeployProps`(`Loader.cpp:755`)와
   `CMapTool`(`MapTool.cpp:2906`)도 STATIC이면 fractured 모델을 무조건 만들었다. 두 곳을 놓쳤다면
   Valtan Level 로드 자체가 실패했다.
2. **publisher의 admission 게이트가 극성 하나가 아니었다.** enabled binding은 109 스케줄과 정확히
   일치해야 했고(`hasExpectedSchedule`), 모든 group이 authored collision box를 가져야 했으며
   (`Destruction group has no authored source collision boxes`), simulation 문서가 모든 group을
   덮어야 했고, projection도 group마다 debris profile을 요구했다. 네 곳 전부 floor 분기를 추가했다.
3. **Server bootstrap은 빈 collision 필드를 거부한다.** `WorldDestructionBootstrap.cpp:305`가
   `-` 마커 또는 stable ID만 허용한다. publisher가 빈 문자열을 내보내자 bootstrap 전체가 로드
   실패해 contract test가 **54건** 무더기로 깨졌다. publisher가 `-`를 쓰도록 고쳐 3건으로 줄었다.
4. **`SHOW_FINAL_ARENA`는 테스트 상수가 아니라 동작을 바꿔야 했다.** `GameRoom.cpp`의
   `stagedPairs.size() != graph.Groups.size()`가 모든 group을 덮으라고 요구한다. 최종 아레나에는
   무너진 바닥도 포함되는 것이 맞으므로 84/30 두 floor stage를 함께 staging하도록 확장했다.
5. **Debug 패널의 `interiorReactedCount`가 바닥을 오탐한다.** 바닥이 무너지면 "109에 잘못 반응한
   내부 벽"으로 집계됐다. 바닥을 분리해 Stage A/B 카운터로 따로 표시한다.
6. **84 패턴의 `sourceActionIds`는 비워 둘 수 없다.** `Publish-GameplayBalance.ps1:811`이 빈 배열을
   거부한다. 기존 `VALTAN_ARENA_BREAK_33`(30줄 바닥)도 109와 같은 `420629` 아레나 붕괴 action을
   재사용하고 있어 같은 근거를 따랐다. `$patternSourceIds`는 패턴 내부 집합이라 재사용이 허용된다.

계획서가 맞았던 가장 중요한 판단은 **nav 극성에 Server C++ 변경이 필요 없다**는 것이다.
`CServerNavigation::bActivateWhenConditionTrue`와 `GameRoom.cpp:3070-3079`의
`bValue = (FRACTURED || DESPAWNED)` 조합이 이미 양방향을 지원한다. 바닥은 navblockers region의
`activateWhenTrue=1`만으로 표현된다.

## 3. 실제 변경 파일

### 추가

- `Tools/LevelPlacementExtractor/build_valtan_floor_collapse.py`
- `.md/GB/08-16/2026-08-16_VALTAN_ARENA_FLOOR_COLLAPSE_PLAN.md`
- 이 문서

### 수정 — Client

- `Client/Private/DeployPropCatalog.cpp` — STATIC의 fractured 쌍을 optional로. ANIM은 계속 거부하고 한쪽만 있는 행도 계속 거부한다.
- `Client/Private/DeployPropObject.cpp` — `Initialize` 검증과 `Ready_Components`에서 빈 fractured tag 허용. 렌더는 기존 intact 폴백을 그대로 쓴다.
- `Client/Private/Loader.cpp` — fractured prototype이 없는 STATIC deploy asset을 건너뛴다.
- `Client/Private/MapTool.cpp` — 같은 조건으로 fractured admission을 건너뛴다.
- `Client/Private/Level_ValtanArena.cpp` — `Reset + Play 84`, `Reset + Play 30` 버튼과 Stage A/B 상태 진단. 바닥을 `interiorReactedCount`에서 제외.

### 수정 — Server

- `Server/Private/GameRoom.cpp` — 최종 아레나 Debug staging에 floor stage A/B 추가, `<array>` include.
- `Server/Private/WorldDestructionBootstrapContractTests.cpp` — 105 group / 117 binding / 113 member로 갱신하고 바닥 전용 검사 4종 추가.
- `Server/Private/ServerGameplayContractTests.cpp` — 최종 아레나 group 수 99 → 105.

### 수정 — Tools

- `Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1` — floor 상수, `BLOCK_WHILE_FRACTURED` 허용,
  극성별 `activateWhenTrue` 대조, floor 스케줄 분기, collision box 면제, simulation/projection 면제,
  navblockers cell 보관, `Test-BaseWalkableCell`, floor 서브그래프 검증, `-` 마커, ContractTest 기대값.

### 수정 — Data (authoring 원본)

- `Data/Maps/Imported/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.deployassets` — 9 → 12행
- `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.deployplacements` — 115 → 121행
- `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.mapplacements` — 13192 → 13186행
- `Data/Encounters/Valtan/ValtanWorldEvents.json` — group/mutation/binding 각 6개 추가
- `Data/Navigation/LV_LUT_HEARTRB_ED.navblockers` — region 97 → 103
- `Data/Encounters/Valtan/ValtanEncounter.json` — `VALTAN_ARENA_BREAK_84` 추가
- `Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json` — `Update-BalanceProvenanceReceipt.ps1`이 71 field를 `PROJECT_TUNED`로 동기화

### 생성물 (직접 편집하지 않음)

전부 publisher가 만들었다.

- `Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.{mapplacements,deployplacements,deployassets}`
- `Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestruction{,presentation}.json`
- `Client/Bin/DataFiles/Navigation/LV_LUT_HEARTRB_ED.{navgrid,navblockers}`
- `Server/Bin/DataFiles/Navigation/LV_LUT_HEARTRB_ED.{navgrid,navblockers}`
- `Server/Bin/DataFiles/World/VALTAN_ARENA.worlddestructionbootstrap`

`Client/Bin/Resources`에 새로 필요한 asset은 **없다.** 바닥은 이미 배포된
`Map/BG_RAD_VALTAN_A/BG_RAD_VALTAN_FLOOR01{,A,B}_SM/*.wmodel`을 deploy asset으로 다시 가리킨다.

## 4. 실행한 검증

| 검증 | 결과 |
|---|---|
| `build_valtan_floor_collapse.py --mode Validate` | PASS, 2249 cell / stage A 672 / stage B 1577 / overlap 0 |
| `build_valtan_floor_collapse.py --mode Apply` | PASS, 5개 문서 한 트랜잭션 교체 |
| `Publish-ValtanWorldDestruction.ps1 -Mode Validate` | PASS, groups=105 bindings=117 outer109=30/30/360 impact159=10 contacts=69 |
| `Publish-ValtanWorldDestruction.ps1 -Mode ContractTest` | PASS, revision `6346a764...e3f` |
| `Publish-ValtanWorldDestruction.ps1 -Mode Publish` | PASS |
| `Publish-ServerNavigation.ps1 -Mode Validate / ContractTest / Publish` | PASS, 392x312 walkable 21381 |
| `Publish-MapAuthoring.ps1 -AreaId LV_LUT_HEARTRB_ED` | PASS, 13186 placements |
| `Publish-GameplayBalance.ps1 -Mode Validate` | PASS, 33 boss patterns / 124 stages |
| `Publish-WorldGameplay.ps1 -Mode Validate` | PASS, VALTAN_ARENA 124 placements / 3 spawn groups |
| 변경 JSON parse | PASS 6/6 |
| `git diff --check` | PASS |
| PowerShell parser로 publisher 구문 검사 | PASS |

기존 Debug `Server.exe`(2026-08-16 13:22 빌드)로 **데이터만** 회귀 검사했다. G1은 데이터 변경이
대부분이라 이 검사가 유효하다. 결과는 실패 54건 → `-` 마커 수정 후 **3건**이다.

남은 3건은 전부 이번에 고친 Server 소스에 대응하는 것이므로 **재빌드해야 사라진다.**

```text
[FAILURE] Stage every independent wall as one Server-authoritative final-arena Debug transaction
[FAILURE] Commit all ninety-nine wall units to the disappeared final-arena state
[FAILURE] Load ninety-nine independent Valtan source-wall destruction units
```

## 5. 실행하지 않은 검증

| 항목 | 상태 |
|---|---|
| Engine / Client / Server Debug·Release 빌드 | 미실행 — 사용자가 수행 |
| 재빌드 후 `Server.exe --contract-test` | 미실행 — 위 3건이 0이 되어야 한다 |
| `NetworkProtocolHarness` / `ClientFrontendHarness` | 미실행 |
| `Invoke-BuildAndRegression.ps1` | 미실행 |
| Client 화면 확인 | 미실행 — 에이전트는 Client를 실행·조작하지 않는다 |

Shared protocol은 바꾸지 않았으므로 이번 G1에서 protocol 버전업은 없다. 다만 Server 소스를
바꿨으므로 Server는 반드시 다시 빌드해야 한다.

## 6. 사용자 확인 절차

빌드 순서는 `Engine`(변경 없음, 생략 가능) → `Shared` → `Server` → `Client`다.
재빌드 뒤 `Server.exe --contract-test`가 `failures : 0`인지 먼저 본다.

그다음 화면 확인이다.

```text
1. Sync-TeamLanEndpoint.ps1 출력 확인, Server listening 확인
2. Lobby -> Valtan 진입
3. F1 -> Valtan Pattern Audition
4. 시작 화면에서 바닥에 구멍 / z-fighting / 사라진 조각이 없는지
5. Reset + Play 109 Only            -> 외벽 30개만 사라지고 바닥은 전부 남는가
6. Reset + Play 84 (Floor Stage A)  -> 바깥 테두리 링만 사라지는가
7. 6번 자리로 우클릭 이동           -> 경로가 거부되는가
8. Reset + Play 30 (Floor Stage B)  -> 벽돌 링이 사라지고 중앙 코어만 남는가
9. Reset + Remove All Walls         -> 벽 99개와 바닥 6개가 모두 사라지는가
10. Lobby로 나갔다 재진입           -> 현재 구멍 상태가 그대로 보이고 파편/카메라 재생이 없는가
11. room empty 후 새 전투           -> 바닥이 전부 복구되는가
```

패널 판정표는 다음과 같다.

| 항목 | 기대값 |
|---|---|
| `Floor Stage A (84)` | 시작 `INTACT 2`, 84 재생 후 `GONE 2` |
| `Floor Stage B (30)` | 시작 `INTACT 4`, 30 재생 후 `GONE 4` |
| `Outer walls` | 109 재생 후 `GONE 30` |
| interior 경고 | 바닥이 무너져도 표시되지 않아야 한다 |

7번에서 "구멍으로 걸어가면 낙사"는 **G1 범위가 아니다.** 이동이 거부되기만 하면 PASS다.

## 7. 남은 불확실성

1. **84 패턴의 애니메이션과 타이밍은 `PROJECT_TUNED` 잠정값이다.** stage 900/500/1200 ms와
   `valtan.mechanic.arena-floor-84.*` actionId는 원작 클립 근거가 없다. `sourceActionIds`는 109/30과
   같은 `420629`를 재사용했고 이는 "아레나 붕괴 계열"이라는 근거일 뿐 84 전용 클립 근거가 아니다.
2. **30줄 collapse edge를 `LANDING` STAGE_ENTER로 잡았다.** camera cue와 Server action start tick을
   실제 재생으로 대조하지 않았다. 어긋나면 `stageId`를 같은 변경 단위에서 교정한다.
3. **파편이 없다.** sector는 250 ms BREAKING 뒤 그냥 사라진다. 기존 generic stone 4종을 재사용한
   파편은 별도 단계다.
4. **`Data/Maps/Imported`에 project 저작 deploy asset 3행을 넣었다.** `BG_RAD_VALTAN_*`가 이미
   Imported `.mapassets`에 overlay로 들어가 있는 선례를 따랐다. Authoring deploy catalog 레이어를
   새로 만드는 편이 옳다면 MapCatalog schema와 publisher를 함께 바꾸는 별도 작업이다.
5. **낙사 전체(G2).** 계획서 §5의 7단계가 그대로 남아 있다.
6. **`build_valtan_floor_collapse.py`는 Apply를 한 번 수행한 뒤 다시 실행하면 실패한다.** 중복
   등록을 거부하도록 만들었기 때문이며 의도한 동작이다. 재생성이 필요하면 대상 문서를 먼저
   되돌린다.
