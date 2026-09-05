# 2026-09-05 발탄 사망→부활 0ms edge와 삼각 포탈 재조정 RESULT

브랜치 `GB/KoukuSaydon-DataFormat`, 기준 HEAD `e61123d4`. 대응 계획은
`2026-09-05_VALTAN_DEATH_REVIVE_ZERO_PURSUIT_AND_PORTAL_TRIANGLE_RETUNE_IMPLEMENTATION_PLAN.md`가 소유한다.
이 문서는 현재 working tree의 실제 구현과 이 세션이 직접 실행한 검증만 기록한다. 사용자 결정으로 Core/FullDiagnostic는
실행하지 않았다.

현재 상태는 **G01/G02 구현 반영, Product 투영·bootstrap publish PASS, Server contract-test failures 0, Valtan 하네스
PASS, focused Python PASS, Engine/Shared/Server 빌드 PASS, Client 최종 link는 실행 중인 Client.exe 잠금으로 미완료,
사용자 화면 판정 대기**다.

## 1. 범위와 완료 상태

| 범위 | 구현 | 자동 검증 | 사용자 판정 |
|---|---|---|---|
| G01 STRUGGLING→GHOST_DEATH, GHOST_DEATH→GHOST_RESPAWN edge 0ms | 완료 | bootstrap row 14/15/49 = 0, Server contract 0 failures, 하네스 Flow 13/13 | NOT RUN |
| G01 per-edge 하한 100 → 0 (pipeline, publisher, Server parser, Client tree/Balance Tool/Flow document/Boss Tool slider) | 완료 | typed patch 24/24, master v2 70/70(수정 후 단건 재실행 PASS), flow contract 기존 drift 2건 제외 PASS | 해당 없음 |
| G02 포탈 빈 간격 6000ms(주기 7900ms), 외접반지름 9.0m, 변 15.5884572681m, 속도 11.9911209755m/s | 완료 | PublishV2, Publish, GameplayCatalog exact check(Server 기동), contract-test phase-three cadence 237 tick PASS, phase3 Python 10/10 | NOT RUN |
| Debug Product 빌드 | Engine/Shared/Server PASS, Client compile PASS | Client 최종 link `LNK1104`(실행 중 Client.exe 잠금) | 해당 없음 |

## 2. 실제 구현

### 2.1 G01 사망→부활 edge

- `Data/Valtan/Valtan.gameplay.json` `transitionPursuitMs[14]`, `[15]`, `[49]` = 0. Server가 두 edge에서 `CHASE`로
  전이하지 않고 다음 tick에 바로 다음 step을 선택한다. 기존 GHOST_DEATH EXIT `SUPPRESS_INTER_STEP_PURSUIT`는 그대로 두었고
  더 이상 유일한 억제 경로가 아니다.
- 하한 100 → 0: `valtan_tuning_pipeline.py` `_validate_scripted_sequence` per-edge `integer(..., 0, 10000)`,
  `Publish-GameplayBalance.ps1` `Assert-JsonInteger ... 0 10000`, `GameplayCatalog.cpp` step row parse(`> 10000u`만 검사),
  `ValtanPatternTree.cpp` split/Product transition parse 2곳, `BalanceTool.cpp` draft lambda,
  `ValtanPatternFlowDocument.h` `MIN_EDGE_PURSUIT_MS = 0u` + `.cpp` edge 검사 4곳과 문구 2곳, `ValtanBossTool.cpp`
  edge 슬라이더 하한 0. 전역 `interStepPursuitMs`와 Flow 기본값 하한 100은 유지한다.
- 테스트 pin: `test_valtan_canonical_typed_patch_transaction.py` 거부 case `[0]` → `[10001]`,
  `test_valtan_pattern_master_v2.py` 기대 배열 14/15/49 = 0과 거부 case `[100, 0]` → `[100, 10001]`,
  `test_valtan_boss_tool_pattern_flow_contract.py` helper edge 하한 0.

### 2.2 G02 삼각 포탈

- 데이터: `Valtan.gameplay.json` volley `radiusM 9.0`, `Valtan.combatobjects.json` `speedMps 11.9911209755`,
  `maximumDistanceM 15.5884572681`, `Valtan.presentation.json` independent effect displayName `외접반지름 9m`.
- Server: `GameRoom.cpp` `PORTAL_OCCURRENCE_INTERVAL_MS 7900u`, `TRIANGLE_CIRCUMRADIUS_M 9.f`,
  `TRIANGLE_EDGE_LENGTH_M 15.5884572681f`, `PORTAL_RUNNER_SPEED_MPS 11.9911209755f`; `GameplayCatalog.cpp` exact check 3개.
- Client: `EncounterPatternReference.cpp` `PORTAL_TRIANGLE_RADIUS_M 9.0`.
- pin: `ServerGameplayContractTests.cpp` 리터럴 12곳 + cadence `237u`, harness canonical marker `"radiusM": 9.0`,
  pipeline `GHOST_PORTAL_CIRCUMRADIUS_M 9.0`과 displayName 상수, `author_valtan_phase_two_mechanics.py` displayName,
  `test_valtan_phase3_primary_ghost_loop_contract.py` 7.5 → 9.0과 C++ 문자열 4개.
- 출발 지연 300ms, 수명 1900ms, V2 포탈 leaf lifetime 1.9s는 그대로다. 이동 시간은 1.3s를 유지한다.

### 2.3 Product·bootstrap

- `Project-ValtanPatternMaster.ps1 -Mode PublishV2`: `changed=4 artifacts=7`. `ValtanEncounter.json`,
  `ValtanCombatObjects.json`, `ValtanPatternRotations.json`, provenance receipt(`speedMps`/`maximumDistanceM` 등
  `PROJECT_TUNED`) 갱신.
- `Publish-GameplayBalance.ps1 -Mode Publish`: PASS. `Gameplay.bootstrap`에
  `PATTERNSEQUENCESTEP ... 14 VALTAN_STRUGGLING 0`, `15 VALTAN_GHOST_DEATH_AUDITION 0`, `49 VALTAN_STRUGGLING 0`,
  `BOSSCOMBATOBJECT ... portal-charge ... 11.9911209755 15.5884572681 300 0 0 1900`,
  `PATTERNSTAGEVOLLEY ... RADIAL 9 30 120`.

## 3. 자동 검증

| 검증 | 결과 |
|---|---|
| `Server.exe --contract-test` (재빌드 후) | 1145 PASS, `failures : 0`. 1회차는 cadence pin 147 tick으로 1건 실패했고 237 tick으로 교정 |
| `ValtanPatternAuditionServiceHarness.exe` | 전 suite PASS(30/30, Flow 13/13, Tuning 11/11, canonical 7/7, EncounterPatternReference 87 case, grip 4/4). 1회차 canonical 4건 FAIL은 동시에 돌던 Python projection transaction의 파일 잠금(Win32 33)이었고 재실행 PASS |
| phase3 primary ghost loop contract | 10/10 PASS |
| canonical typed patch transaction | 24/24 PASS |
| pattern master v2 | 70 tests 중 1건(0 거부 case)이 계약 변경으로 실패 → fixture 교정 후 단건 재실행 PASS |
| boss tool pattern flow contract | 38 중 36 PASS. 남은 2건은 HEAD에도 존재하는 기존 drift(`"Valtan Boss Tool Restart ..."`, `"Stop a running Valtan Boss Tool flow ..."` 문자열이 Server 소스와 불일치)로 이번 변경과 무관 |
| animation chain promotion 17/17, world entity spawn revision 7/7, effect tool saved rows 35/35(skip 7), model view composition 16/16 | PASS |
| `git diff --check` | PASS |

## 4. 빌드

`Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product` 최종 실행: Engine/Shared/Server 빌드 PASS,
`Server.exe` 13:24:11. Client는 전 translation unit compile PASS 뒤 최종 link에서
`LNK1104 '..\Bin\Debug\Client.exe' 파일을 열 수 없습니다`로 실패했다. 같은 시각 사용자가 Visual Studio로 빌드해 띄운
`Client.exe`(13:24:32)가 실행 중이었기 때문이며 compile 실패가 아니다. 사용자가 Client를 종료한 뒤 Client 최종 link만 한 번
다시 수행하면 된다. 그 전 1회차 Product는 다른 세션의 `ValtanActionWorkbench.h` 리팩터링 중간 상태(`COMPOSITION_RESOURCE_TREE_NODE`
재정의)로 Client compile이 실패했고, 그쪽 정리 뒤 재실행에서 compile PASS로 바뀌었다.

## 5. 사용자 수동 검증

`Server + Client` profile로 다음을 확인한다.

1. Valtan Arena에서 STRUGGLING 망령화 clip이 끝난 직후 발탄이 플레이어 쪽으로 달려오지 않고 바로 dead clip(3.7초)으로
   들어가는지, dead clip 뒤에도 추격 없이 respawn clip으로 이어지는지 확인한다.
2. phase 3 유령 loop에서 삼각 포탈이 이전보다 넓은 정삼각형(외접반지름 9m, 변 15.6m)으로 생기고, 포탈이 사라진 뒤 6초 동안
   비어 있다가 다음 세 포탈이 동시에 생기는지 확인한다. runner가 1.3초에 다음 꼭짓점에 도착하는지도 확인한다.
3. F1 Boss Tool Flow에서 edge "Wait before next (ms)" 슬라이더가 0까지 내려가고 저장/재로드가 통과하는지 확인한다.

## 6. 남은 경계

- 포탈 값은 여전히 데이터 3곳 + C++ 4곳 + Python/PS pin에 흩어져 있다. 이번엔 같은 숫자로 일괄 교체했고, 상수를 bootstrap에서
  읽어 pin을 없애는 구조 변경은 별도 범위다.
- flow contract의 기존 문자열 drift 2건은 KoukuSaydon/Valtan 이름 분리 작업 쪽에서 맞춰야 한다.
- Client 최종 link 1회와 Core/FullDiagnostic는 미실행이다.
- 구현은 공유 dirty working tree에 있으며 stage/commit/push하지 않았다.
