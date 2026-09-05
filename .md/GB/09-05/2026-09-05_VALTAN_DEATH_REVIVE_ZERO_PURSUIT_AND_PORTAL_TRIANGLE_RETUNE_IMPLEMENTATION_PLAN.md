# 2026-09-05 발탄 사망→부활 0ms edge와 삼각 포탈 재조정 구현 계획서

브랜치 `GB/KoukuSaydon-DataFormat`, 기준 HEAD `e61123d4`. working tree에는 다른 세션의 KoukuSaydon workbench
작업(`GameRoom.cpp`, `GameplayCatalog.cpp`, `ServerGameplayContractTests.cpp`, `PacketMessages.h` 등)이 미커밋
상태로 함께 있다. 이 계획의 모든 기준점은 현재 디스크 실측이며 다른 세션의 변경 위에 anchor 기반으로만 반영한다.
사용자 결정으로 Core/FullDiagnostic는 완료 조건에서 제외하고, domain validator·하네스·contract-test·focused
Python까지만 실행한다.

---

## 0. 목표와 종료 증거

| G | 목표 | 종료 증거 |
|---|---|---|
| G01 | 사망→부활 두 edge(STRUGGLING→GHOST_DEATH, GHOST_DEATH→GHOST_RESPAWN)의 inter-step pursuit를 **0ms**로 저작할 수 있게 하고 실제로 0으로 둔다. 데이터·Tool·publisher·Server·Client 어디서도 0을 거부하지 않는다 | `Publish-GameplayBalance -Mode Publish` PASS, bootstrap `PATTERNSEQUENCESTEP` 14/15/49 = 0, `Server.exe --contract-test` failures 0, Valtan 하네스 PASS, typed-patch/master v2/flow Python PASS |
| G02 | 삼각 포탈을 외접반지름 **9.0 m**, 포탈 소멸 뒤 빈 간격 **6000 ms**(발생 주기 7900 ms)로 바꾼다. runner/proxy 이동 시간 1.3 s와 수명 1900 ms는 유지한다 | `Project-ValtanPatternMaster -Mode PublishV2` PASS, Product/bootstrap/receipt 갱신, `GameplayCatalog` exact check 통과(Server 기동), phase3 contract Python PASS, Debug Product build PASS |

사용자 화면 판정(추격 없음, 포탈 간격·크기)은 별도 종료 증거다.

---

## 1. 실측 요약

### 1.1 사망→부활 (scripted sequence step 14~17)

| edge | 현재 `transitionPursuitMs` | 현재 동작 |
|---|---|---|
| 14 STRUGGLING → 15 GHOST_DEATH_AUDITION | 1000 | 망령화 clip 뒤 1 s `CHASE` (사용자가 본 추격) |
| 15 GHOST_DEATH_AUDITION → 16 GHOST_RESPAWN_AUDITION | 1000 | EXIT `SUPPRESS_INTER_STEP_PURSUIT`가 tick을 0으로 덮음 |
| 49 STRUGGLING → 50 GHOST_RESPAWN_AUDITION | 1000 | ghost loop 활성화 뒤 소비되지 않는 tail |

`FinishPattern`이 `PursuitTicksRemaining`을 채우고([ValtanBrain.cpp:1978](../../../Server/Private/ValtanBrain.cpp)),
남은 tick 동안 `SelectPattern`이 nullptr을 돌려 brain이 `CHASE`로 전이한다(2965행). 0이면 다음 tick에 바로 다음
step을 고른다. Server tick 변환 `(ms*30+999)/1000`은 0을 0으로 만든다.

0을 막는 하한 100은 다음 여섯 곳에 있다: pipeline `_validate_scripted_sequence`(3636행), publisher 4172행,
`GameplayCatalog.cpp` 3285행, `ValtanPatternTree.cpp` 4615/6873행, `BalanceTool.cpp` 2949행, Boss Flow edge
(`ValtanPatternFlowDocument.cpp` 713/900/1587/1673, 슬라이더 `ValtanBossTool.cpp` 3501). 전역 `interStepPursuitMs`
하한 100은 유지한다.

### 1.2 삼각 포탈

| 값 | 현재 | 변경 |
|---|---|---|
| 발생 주기 `PORTAL_OCCURRENCE_INTERVAL_MS` | 4900 (1900 + 3000) | **7900** (1900 + 6000) |
| 외접반지름 | 7.5 | **9.0** |
| 변 길이 = R·√3 | 12.9903810568 | **15.5884572681** |
| runner/proxy 속도 = 변/1.3 s | 9.9926008129 | **11.9911209755** |
| 출발 지연 / 수명 | 300 / 1900 | 유지 |

pin 위치: `GameRoom.cpp` scheduler 상수, `GameplayCatalog.cpp` exact check(4944~4959), `EncounterPatternReference.cpp`
587행, `ServerGameplayContractTests.cpp` 3320/3322/5766/5774/5782/16411/16467/16521/16529/16530/16556/16557,
harness `ValtanCanonicalGraphContractTests.cpp` 1411, pipeline `GHOST_PORTAL_CIRCUMRADIUS_M`,
`test_valtan_phase3_primary_ghost_loop_contract.py` 205/224/229/365~368, 데이터 `Valtan.gameplay.json` 2448행,
`Valtan.combatobjects.json` 349/350행. Product(`Data/Encounters/Valtan/*.json`)와 receipt는
`Project-ValtanPatternMaster.ps1 -Mode PublishV2`가 재생성한다. bootstrap은 정수값 실수를 `9`로 쓰므로 contract-test
fixture row도 `\t9\t`다.

---

## 2. 변경 파일

| G | 파일 | 작업 |
|---|---|---|
| G01 | `Data/Valtan/Valtan.gameplay.json` | `transitionPursuitMs[14]`, `[15]`, `[49]` = 0 |
| G01 | `Tools/ValtanPipeline/valtan_tuning_pipeline.py` | per-edge `integer(..., 100, 10000)` → `0, 10000` |
| G01 | `Tools/GameplayPipeline/Publish-GameplayBalance.ps1` | `Assert-JsonInteger ... 100 10000` → `0 10000` |
| G01 | `Server/Private/GameplayCatalog.cpp` | step row parse의 `pursuitAfterMs < 100u ||` 제거 |
| G01 | `Client/Private/ValtanPatternTree.cpp` | split/Product per-edge `< 100.0 ||` 2곳 제거 |
| G01 | `Client/Private/BalanceTool.cpp` | draft per-edge `>= 100u &&` 제거 |
| G01 | `Client/Public/ValtanPatternFlowDocument.h`, `Client/Private/ValtanPatternFlowDocument.cpp` | `MIN_EDGE_PURSUIT_MS = 0u` 추가, edge 검사 4곳과 문구 2곳 교체 |
| G01 | `Client/Private/ValtanBossTool.cpp` | edge 슬라이더 하한 100 → 0 |
| G01 | `Tools/ValtanPipeline/test_valtan_canonical_typed_patch_transaction.py` | `transition-range` 거부 case `[0]` → `[10001]` |
| G01 | `Tools/ValtanPipeline/test_valtan_pattern_master_v2.py` | `EXPECTED_SCRIPTED_SEQUENCE.transitionPursuitMs` 14/15/49 = 0 |
| G02 | `Data/Valtan/Valtan.gameplay.json`, `Data/Valtan/Valtan.combatobjects.json` | `radiusM 9.0`, `speedMps 11.9911209755`, `maximumDistanceM 15.5884572681` |
| G02 | `Tools/ValtanPipeline/valtan_tuning_pipeline.py` | `GHOST_PORTAL_CIRCUMRADIUS_M = 9.0` |
| G02 | `Server/Private/GameRoom.cpp` | scheduler 상수 4개 |
| G02 | `Server/Private/GameplayCatalog.cpp` | exact check 상수 3개 |
| G02 | `Client/Private/EncounterPatternReference.cpp` | `PORTAL_TRIANGLE_RADIUS_M = 9.0` |
| G02 | `Server/Private/ServerGameplayContractTests.cpp` | fixture/기대값 12곳 |
| G02 | `Tools/ValtanPatternAuditionServiceHarness/Private/ValtanCanonicalGraphContractTests.cpp` | mutation marker `"radiusM": 9.0` |
| G02 | `Tools/ValtanPipeline/test_valtan_phase3_primary_ghost_loop_contract.py` | 7.5 → 9.0, C++ 문자열 pin 4개 |
| G01/G02 | `.md/TEAM/발탄인수인계서.md` | 11.10 포탈 문장, edge 0ms 계약 문장 |

새 C++ 파일 없음. project/filter 변경 없음.

---

## G01 0ms edge

```text
파일: Data/Valtan/Valtan.gameplay.json
작업: 교체
기준점: decisionModel.scriptedSequence.transitionPursuitMs 배열의 index 14, 15, 49 (92행부터 한 값 한 줄)
```

```json
        0,
```

```text
파일: Tools/ValtanPipeline/valtan_tuning_pipeline.py
작업: 교체
기준점: for index, pursuit_ms in enumerate(transition_pursuit_ms): 안의 integer(...) 호출 하한
```

```python
        for index, pursuit_ms in enumerate(transition_pursuit_ms):
            integer(
                pursuit_ms,
                f"scriptedSequence.transitionPursuitMs[{index}]",
                0,
                10000,
            )
```

```text
파일: Tools/GameplayPipeline/Publish-GameplayBalance.ps1
작업: 교체
기준점: Assert-JsonInteger $transitionPursuitValue 'Valtan scripted transition pursuit milliseconds' 100 10000
```

```powershell
		Assert-JsonInteger $transitionPursuitValue `
			'Valtan scripted transition pursuit milliseconds' 0 10000
```

```text
파일: Server/Private/GameplayCatalog.cpp
작업: 교체
기준점: (hasNextStep && (pursuitAfterMs < 100u || pursuitAfterMs > 10000u)) ||
```

```cpp
				 (hasNextStep && pursuitAfterMs > 10000u) ||
```

```text
파일: Client/Private/ValtanPatternTree.cpp
작업: 삭제 2곳
기준점 1: split scriptedSequence transition loop 안 PursuitMs.Get_Number() < 100.0 || 한 줄
기준점 2: Product scriptedSequence transition loop 안 PursuitMs.Get_Number() < 100.0 || 한 줄
```

```text
파일: Client/Private/BalanceTool.cpp
작업: 교체
기준점: Set_ValtanScriptedSequenceDraft 안 lambda return pursuitMs >= 100u && pursuitMs <= 10000u;
```

```cpp
				return pursuitMs <= 10000u;
```

```text
파일: Client/Public/ValtanPatternFlowDocument.h
작업: 추가
기준점: static constexpr std::uint32_t MIN_INTER_STEP_PURSUIT_MS = 100u; 바로 아래
```

```cpp
		/* A Flow edge may wait zero ticks (death -> respawn). The global default
		   keeps the 100 ms floor so an unauthored edge never becomes a same-tick
		   chain by accident. */
		static constexpr std::uint32_t MIN_EDGE_PURSUIT_MS = 0u;
```

`ValtanPatternFlowDocument.cpp`는 edge 검사 네 곳(`edge.iPursuitMs <`, `pursuitMs >=` transition lambda,
`Add_Edge`류의 `pursuitMs <`, `Set_EdgePursuitMs`의 `pursuitMs <`)에서 `MIN_INTER_STEP_PURSUIT_MS`를
`MIN_EDGE_PURSUIT_MS`로 바꾸고, 문구 두 곳의 `100..10000`을 `0..10000`으로 바꾼다. 전역 default(`iDefaultPursuitMs`,
`interStepPursuitMs`, `Set_DefaultPursuitMs`)는 그대로 100이다.

```text
파일: Client/Private/ValtanBossTool.cpp
작업: 교체
기준점: ImGui::SliderInt("Wait before next (ms)", &EdgePursuitMs, 100, 10000,
```

```cpp
			"Wait before next (ms)", &EdgePursuitMs, 0, 10000,
```

테스트 교체: `test_valtan_canonical_typed_patch_transaction.py`의 `("transition-range", {**valid,
"transitionPursuitMs": [0]})` → `[10001]`, `test_valtan_pattern_master_v2.py`의 기대 배열 index 14/15/49 → `0`.

## G02 포탈

데이터 세 값, pipeline 상수 하나, C++ 상수/exact check/parser/테스트 리터럴을 아래 값으로 일괄 교체한다.

| 리터럴 | 기존 | 변경 |
|---|---|---|
| `PORTAL_OCCURRENCE_INTERVAL_MS` | `4900u` | `7900u` |
| `TRIANGLE_CIRCUMRADIUS_M` / `PORTAL_CIRCUMRADIUS` / `PORTAL_TRIANGLE_RADIUS_M` | `7.5f` / `7.5` | `9.f` / `9.0` |
| `TRIANGLE_EDGE_LENGTH_M`, exact check, runner fixture | `12.9903810568f` | `15.5884572681f` |
| `PORTAL_RUNNER_SPEED_MPS`, exact check, runner fixture | `9.9926008129f` | `11.9911209755f` |
| contract-test float literal | `12.9903811f`, `9.99260081f` | `15.5884573f`, `11.9911210f` |
| contract-test bootstrap row | `RADIAL\t7.5\t30\t` | `RADIAL\t9\t30\t` |
| harness canonical marker | `"\"radiusM\": 7.5"` | `"\"radiusM\": 9.0"` |
| pipeline | `GHOST_PORTAL_CIRCUMRADIUS_M = 7.5` | `= 9.0` |
| phase3 Python | `7.5 * math.sqrt(3.0)`, `assertAlmostEqual(7.5, ...)`, C++ 문자열 4개 | `9.0 * ...`, `9.0`, 새 리터럴 |
| gameplay JSON 2448행 | `"radiusM": 7.5,` | `"radiusM": 9.0,` |
| combatobjects JSON 349/350행 | `9.9926008129`, `12.9903810568` | `11.9911209755`, `15.5884572681` |

## 검증 순서

```powershell
$env:PYTHONPATH='.'; $env:PYTHONIOENCODING='utf-8'
powershell -ExecutionPolicy Bypass -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode PublishV2
powershell -ExecutionPolicy Bypass -File Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Publish
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
Server\Bin\Debug\Server.exe --contract-test
Tools\ValtanPatternAuditionServiceHarness\Bin\Debug\ValtanPatternAuditionServiceHarness.exe   (MSBuild 후)
python -m unittest Tools.ValtanPipeline.test_valtan_phase3_primary_ghost_loop_contract Tools.ValtanPipeline.test_valtan_pattern_master_v2 Tools.ValtanPipeline.test_valtan_canonical_typed_patch_transaction Tools.ValtanPipeline.test_valtan_boss_tool_pattern_flow_contract Tools.ValtanPipeline.test_valtan_animation_chain_promotion Tools.ValtanPipeline.test_valtan_camera_tool_contract Tools.ValtanPipeline.test_world_entity_spawn_revision_contract Tools.EffectPipeline.test_effect_tool_valtan_saved_rows Tools.EffectPipeline.test_valtan_model_view_composition
git diff --check
```

기대: bootstrap `PATTERNSEQUENCESTEP ... 14 ... 0`, `BOSSCOMBATOBJECT ... 11.9911209755 15.5884572681`,
`PATTERNSTAGEVOLLEY ... RADIAL 9 30 120`, Server contract failures 0, 하네스 전 suite PASS, Python PASS.
