# 2026-08-18 발탄 아레나 바닥 붕괴 파편 결과

계획 정본은 같은 폴더의 `2026-08-18_VALTAN_FLOOR_COLLAPSE_DEBRIS_PLAN.md`다.
이 문서는 실제로 반영한 것, 계획과 달랐던 것, 실행한 검증과 실행하지 않은 검증을 분리해 적는다.

## 0. 결론

바닥 sector 6개가 파편 없이 사라지던 문제를 닫았다. 파편 presentation profile이 99개에서
105개가 되어 projection group 105개를 전부 덮는다.

새로 cook한 아트는 **0개**다. 이미 배포된 Valtan 잡석 4종을 재사용했다.
`Client/Bin/Resources`는 팀장 관리 물리 폴더이므로 파일을 넣지 않았다.

**아직 화면으로 확인되지 않았다.** 파편 recipe 테이블이 C++이라 Client를 다시 빌드해야
바닥 cue가 실제로 재생된다. 재빌드 전에는 recipe 조회가 12개를 못 채워 fail-closed로
로그만 남기고 지나간다.

## 1. 실제 변경 파일

### 수정 — Client

- `Client/Private/DestructionSimulationRuntime.cpp` — `Get_ProjectAuthoredDebrisModelSpecs()`에
  바닥 exact debris spec 36개 추가(+187/-0). asset 3종 × 조각 12개이며 prototype tag는
  `Prototype_Component_Model_DestructionFloor_{Rail,BrickA,BrickB}_Chunk00..11`이다.

### 수정 — Data (authoring 원본)

- `Data/Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.destructionsimulation.json` —
  profile 99 → 105(+228/-0). 바닥 group마다 element 1개.

### 수정 — Tools

- `Tools/WorldPipeline/Publish-ValtanWorldDestruction.ps1` — 바닥 면제 규칙 3곳 제거와
  계약 테스트 기대값 1곳 교체. 작업 전 대비 순수 변경은 이 네 곳뿐이다.

### 생성물 (직접 편집하지 않음)

- `Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestructionpresentation.json` — +157/-1
- `Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestruction.json` — publisher가 다시 썼으나
  작업 전 백업본과 **바이트 단위로 동일**하다. projection은 이번 G에서 바뀌지 않는다.

`Server/Bin/DataFiles/World/VALTAN_ARENA.worlddestructionbootstrap`은 바뀌지 않았다.
`combatRuntimeRevision`이 `6346a764...e3f`로 그대로이므로 **Server 재빌드는 필요 없다.**

## 2. 계획과 달랐던 것

구현하면서 실측으로 드러난 차이가 두 개 있다.

1. **바닥 면제가 3곳이 아니라 4곳이었다.** 계획 단계에서 `Compile-ValtanDebrisProfiles`의
   coverage, projection의 profile 유무, projection의 member 커버리지 세 곳을 찾았지만,
   `Invoke-ContractTests`(`:1469` 부근)가 `presentation은 projection의 비-바닥 부분집합`을
   따로 강제하고 있었다. `-Mode ContractTest`가
   `Canonical debris presentation must be the exact non-floor subset of the projection.`으로
   멈춰서 발견했다. 기대값을 `projection 전체를 덮되 그중 정확히 6개가 바닥`으로 바꿨다.

2. **`.deployassets`의 fractured 두 칸은 건드릴 필요가 없었다.** 제품 파편은
   `CDestructionSimulationRuntime`의 컴파일된 spec 테이블에서 `sourceDeployAssetId`로 조회한다.
   deploy asset의 fractured prototype은 `FRACTURED` 상태를 그리는 렌더 모델이고, 바닥의 authored
   mutation은 `DESPAWNED`로 끝나 그 상태를 거치지 않는다. 그래서 데이터 3행을 그대로 두었다.

계획이 맞았던 판단은 **바닥 placement가 이미 `destructible=1`**이라는 것이다.
`CDeployPropObject::Is_Destructible()`은 placement 플래그만 보므로 fractured 모델이 없어도
파편 emitter의 source가 될 수 있다.

## 3. 파편 위치를 어떻게 정했는가

pivot을 원점에 두면 잡석이 전부 아레나 중심에서 나온다. 바닥은 고리 모양이라 중심은 아직
멀쩡한 자리다. 그래서 G1이 이미 저작해 둔 `Data/Navigation/LV_LUT_HEARTRB_ED.navblockers`의
바닥 collapse region에서 실측했다.

```text
grid 392x312  cell 0.5m  origin (-6, -165)   아레나 중심 (156.279007, -121.976997)
```

| region | cell | 최소 r | 중앙 r | 최대 r |
|---|---|---|---|---|
| `floor84.rail.7000000000000000001` | 347 | 13.52 | 14.67 | 16.26 |
| `floor84.rail.7000000000000000005` | 325 | 13.58 | 14.68 | 16.68 |
| `floor30.brick.7000000000000000002` | 365 | 7.81 | 11.19 | 13.83 |
| `floor30.brick.7000000000000000003` | 427 | 7.52 | 11.23 | 13.78 |
| `floor30.brick.7000000000000000006` | 360 | 7.66 | 11.19 | 13.80 |
| `floor30.brick.7000000000000000007` | 425 | 7.49 | 11.21 | 13.75 |

identity 회전 placement의 region을 각도 12등분하고 각 구간의 중앙 반경 cell을 하나씩 골라
pivot으로 썼다. y는 바닥면 바로 위인 `+0.25 m`다.

같은 asset을 쓰는 나머지 placement 3개는 `yaw180`이라 같은 local pivot이 180도 회전해 반대편에
놓인다. region 각도로 대칭을 확인했다.

```text
VALTAN_FLOOR_RAIL      identity 335.0~161.4   identity+180 155.0~341.4   yaw180 156.1~337.9
VALTAN_FLOOR_BRICK_A   identity 341.8~ 67.1   identity+180 161.8~247.1   yaw180 162.8~247.4
VALTAN_FLOOR_BRICK_B   identity  65.0~166.7   identity+180 245.0~346.7   yaw180 244.8~344.1
```

오차 0.1~3.5도는 cell 양자화 범위다. 그래서 asset당 pivot 한 벌만 저작했다.

## 4. 실행한 검증

| 검증 | 결과 |
|---|---|
| `Publish-ValtanWorldDestruction.ps1 -Mode Validate` | PASS, groups=105 bindings=117 **emitters=105**(이전 99) |
| `Publish-ValtanWorldDestruction.ps1 -Mode ContractTest` | PASS, revision `6346a764...e3f` |
| `Publish-ValtanWorldDestruction.ps1 -Mode Publish` | PASS |
| `Server.exe --contract-test` (기존 Debug 바이너리) | `failures : 0` |
| `ClientFrontendHarness.exe` (기존 Debug 바이너리) | 265 PASS / 61 FAILURE, **신규 실패 0** |
| World Destruction 계열 harness 25건 | 전부 PASS |
| 변경 JSON parse | PASS 3/3 |
| PowerShell parser로 publisher 구문 검사 | PASS |
| `git diff --check` | PASS |
| C++ 편집분 괄호 수지 | 중괄호 `+0` |
| projection 문서 vs 작업 전 백업 | 바이트 동일 |

`ClientFrontendHarness`의 실패 61건은 전부 Effect 계열이며 이번 변경과 무관하다. 작업 전
baseline 63건과 대조해 **신규 실패가 0건**이고, 사라진 2건은 Effect 쪽 순서 의존 테스트다.

기존 Debug 바이너리로 **데이터만** 회귀 검사했다. C++ spec 테이블 변경은 이 검사에 포함되지
않는다.

## 5. 실행하지 않은 검증

| 항목 | 상태 |
|---|---|
| Client Debug/Release 빌드 | 미실행 — 사용자가 수행 |
| 재빌드 후 `ClientFrontendHarness` | 미실행 |
| Client 화면 확인 | 미실행 — 에이전트는 Client를 실행·조작하지 않는다 |

Shared protocol과 Server bootstrap을 바꾸지 않았으므로 protocol 버전업과 Server 재빌드는 없다.
**Client만 다시 빌드하면 된다.**

## 6. 사용자 확인 절차

Client Debug를 다시 빌드한 뒤 다음을 본다.

```text
1. Lobby -> Valtan 진입
2. F1 -> Valtan Pattern Audition
3. Reset + Play 84 (Floor Stage A / Outer Rail)
4. Reset + Play 30 (Floor Stage B / Brick Ring)
5. Reset + Play 109 Only
```

| 항목 | 기대값 |
|---|---|
| 84 재생 | 바깥 테두리 링이 사라지면서 그 자리 반경 13~16 m 위에 잡석 12개씩 두 벌이 아래로 떨어진다 |
| 30 재생 | 벽돌 링이 사라지면서 반경 7~14 m 위에 잡석 12개씩 네 벌이 아래로 떨어진다 |
| 109 재생 | 기존과 같이 외벽 30개만 부서지고 바닥과 바닥 파편은 전혀 나오지 않는다 |
| 잡석 위치 | 아레나 중심이 아니라 **무너지는 고리 위**에 흩어져야 한다 |
| 수명 | 4초 뒤 사라진다 |

Debug 출력에 `[Level_ValtanArena][DestructionDebris]`가 찍히면 recipe 조회가 실패한 것이므로
Client 재빌드가 반영되지 않은 상태다.

## 7. 남은 불확실성

1. **잡석 4종은 벽 파편과 같은 메시다.** 바닥 전용 조각이 아니라 이미 배포된 Valtan 잡석을
   재사용했다. 크기(`3.5f`)와 밀도가 바닥 붕괴에 맞는지는 사용자 육안 판정이 필요하다.
2. **`direction`, `speed`, `gravity`, `lifetime`은 `PROJECT_AUTHORED` 값이다.** 원작 근거가 없다.
   `[0,-1,0]` / 6 / 2 / 4는 런타임이 y에 무조건 더하는 `UPWARD_SPEED_METERS_PER_SECOND`(3.25)를
   감안해 초기 y 속도가 약 -2.75 m/s가 되도록 고른 값이다.
3. **sector당 조각이 12개로 고정이다.** `ACTORS_PER_EMITTER`가 컴파일 상수라 밀도를 바꾸려면
   emitter를 늘리거나 그 상수 계약을 함께 바꿔야 한다.
4. **파편은 무너진 자리 아래로 그냥 떨어져 사라진다.** 낙사(G2)는 여전히 이번 범위가 아니다.
5. **pivot은 nav cell 중앙값이다.** 실제 메시 형상이 아니라 그 sector가 덮는 통행 가능 cell의
   기하학적 분포에서 뽑았다. 바닥 메시의 시각적 경계와 미세하게 다를 수 있다.

## 8. 후속 반영 — 누적 audition (2026-08-19)

바닥 파편을 실제로 확인하려다 드러난 문제를 같은 작업 단위에서 닫았다.

### 8.1 문제

`Reset + Play`는 매번 `Reset_ValtanAuditionState`를 호출해 모든 group을 INTACT로 되돌린다.
그래서 84나 30을 재생하면 **벽이 전부 되살아난 상태**에서 바닥만 무너진다.

실제 전투는 그렇지 않다. 일반 벽 67개의 109 STAGE 바인딩은 `ValtanWorldEvents.json`에서
의도적으로 `enabled:false`이고, 이 벽들은 오직 보스 몸통·무기 접촉(`COLLIDER_CONTACT` 67개)과
159 돌진(`COLLISION_IMPACT` 10개)으로만 부서진다. 즉 109줄에 도달할 무렵이면 안쪽 벽은 이미
대부분 사라져 있는 것이 정상이다.

`SHOW_FINAL_ARENA`는 벽과 바닥을 **한꺼번에** 없애 버리므로, 벽만 없는 상태에서 바닥이
무너지는 장면을 볼 수 없었다.

### 8.2 반영

누적 재생 경로 자체는 이미 있었다. `Level_ValtanArena.cpp:454`가
`Only the first step resets. Every later chapter is an ARM/CROSS pair`라고 적어 둔 대로
`ARM_HEALTH_BAR` + `CROSS_HEALTH_BAR` 쌍은 리셋하지 않는다. 다만 UI에 노출돼 있지 않았다.

| 구분 | 경로 | 변경 |
|---|---|---|
| 수정 | `Shared/Public/Network/PacketMessages.h` | `VALTAN_AUDITION_OPERATION::BREAK_EVERY_WALL` 추가 |
| 수정 | `Shared/Private/Network/PacketMessages.cpp` | barless 검증 목록에 추가 |
| 수정 | `Server/Private/GameRoom.cpp` | `isOpenArenaView`/`isArenaStagingView` 분기, 바닥 staging 조건화, coverage 기대값 분기 |
| 수정 | `Client/Private/Level_ValtanArena.cpp` | `Reset + Break Every Wall (Keep Floor)`와 `Play Selected (Keep Broken)` 버튼, `Submit_Audition` barless 목록 |
| 수정 | `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp` | barless round-trip 2종 → 3종 |
| 수정 | `Server/Private/ServerGameplayContractTests.cpp` | open-arena 계약 1건 추가 |

`BREAK_EVERY_WALL`은 `SHOW_FINAL_ARENA`와 같은 트랜잭션을 만들되 바닥 두 stage를
**준비만 하고 append하지 않는다.** 준비하는 이유는 바닥이 몇 group을 소유하는지 알아야
coverage를 검사할 수 있기 때문이며, group ID 문자열 접두사 대신 실제 transition 수를 쓴다.

```text
final arena : stagedPairs == graph.Groups.size()                    (105)
open arena  : stagedPairs == graph.Groups.size() - floorGroupCount   (99)
```

뺄셈 앞에 `0 == floorGroupCount || floorGroupCount >= graph.Groups.size()` 가드를 둬서
unsigned wrap이 통과값을 만들 수 없게 했다.

### 8.3 protocol version을 올리지 않은 이유

`C2S_VALTAN_AUDITION_REQUEST`의 wire는 `uint32 sequence / uint8 operation / uint32 bar`이고
enum 값 추가는 **layout을 바꾸지 않는다.** `AGENTS.md`가 버전을 올리라고 정한 경우는
`PLAYER_SNAPSHOT` 같은 layout 변경이다. 구버전 Server는 `Is_Valid_AuditionRequest`의
`rawOperation >= END`에서 깨끗이 거부하므로 실패 모드도 fail-closed다. 따라서
`NETWORK_PROTOCOL_VERSION`은 21 그대로다.

### 8.4 실행한 검증

| 검증 | 결과 |
|---|---|
| 편집한 6개 C++ 파일 괄호 수지 | 전부 중괄호 `+0` 소괄호 `+0` |
| `BREAK_EVERY_WALL` 참조 일관성 | Shared/Server/Client/harness/계약 테스트 6개 파일 전부 |
| audition operation을 switch 하는 코드 | 없음 (새 case 누락 위험 없음) |
| `git diff --check` | PASS |
| `NETWORK_PROTOCOL_VERSION` | 21 유지 확인 |

### 8.5 실행하지 않은 검증

Shared·Server·Client를 모두 바꿨으므로 **세 프로젝트를 다시 빌드해야** 한다.
빌드 후 `NetworkProtocolHarness`와 `Server.exe --contract-test`를 실행하기 전에는
이번 8절의 코드가 동작한다고 기록하지 않는다.

### 8.6 사용자 확인 절차

```text
1. Reset + Break Every Wall (Keep Floor)   -> 벽 99개만 사라지고 바닥 6개는 그대로
2. 체력바 84 선택 -> Play Selected (Keep Broken)  -> 바닥 바깥 링만 무너지고 벽은 사라진 채 유지
3. 체력바 30 선택 -> Play Selected (Keep Broken)  -> 벽돌 링이 무너지고 1~2의 결과가 유지
```

패널의 `Floor Stage A (84)`가 1단계에서 `INTACT 2`, 2단계 뒤 `GONE 2`,
`Floor Stage B (30)`이 2단계까지 `INTACT 4`, 3단계 뒤 `GONE 4`가 되어야 한다.
