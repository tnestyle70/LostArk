# 차원술사 BA 전체 모션·stage별 Effect 및 T ground-target 연결 수정 구현 결과

## 1. 결론과 현재 상태

`origin/main@625471363811a20e296aab77c3ee4117479f4335`을 기준으로 차원술사 기본 공격
`2050010`의 네 BA stage를 `_01/_02/_03/_04` 독립 clip으로 복구했다. 단발 또는 명시
MOVE/SKILL 대기 시 소비하는 `actionDurationMs`와 buffered LMB가 다음 BA로 넘어가는
`comboAdvanceMs`를 분리했고, 기존 BA1~BA4 Product Effect stable ID를 각 stage clip의
source `0ms`에 1:1로 연결했다.

차원술사 T `2050500`은 replicated Character 생성 시
`pNavigationPrototypeTag = nullptr`였기 때문에 local Character가 ground target을 valid로
sample하지 못했다. 네 product Level의 명시적 `strMapAreaId`를
`CMapNavigationContract::Resolve_Area`로 해석하고, resolve한 navigation prototype을
locally controlled Character에만 연결해 valid LMB confirm 경로를 복구했다.

이번 변경은 V0 또는 과거 Effect payload 복원이 아니다. 네 authored Effect 문서의 Element,
delay, texture, material, source recipe와 runtime payload는 수정하지 않았다. Debug focused
검증과 Debug 전체 compile/link, T 수정 뒤 Debug/Release Client compile/link는 아래 기록한
범위에서 통과했다. Debug와 Release 전체 regression은 같은 기존 Effect project registration
stale gate에서 중단됐으므로 둘 중 어느 쪽도 full regression PASS로 판정하지 않는다.

| 구분 | 현재 판정 |
|---|---|
| BA animation·Server timing·root motion | 구현 완료 |
| BA1~BA4 Product cue 연결 | 구현 완료 |
| T local ground-target navigation wiring | 구현 완료 |
| V0/과거 Effect payload 복원 | 범위 제외, 변경 없음 |
| Debug focused 검증 | PASS, 5절의 실행 증거에 한정 |
| Debug compile/link와 Server contract | PASS |
| 전체 Debug regression | `BASELINE_BLOCKED`, `Sync-EffectDataProject.ps1 -Check` stale registration |
| T 수정 뒤 Client Debug/Release | compile/link PASS |
| 전체 Release regression | `BASELINE_BLOCKED`, 같은 stale registration에서 중단 |
| 사용자 visual fidelity | `PENDING_USER_VISUAL_GATE` |

## 2. BA1~BA4 구현 결과

### 2.1 stage clip과 두 Server clock

`DimensionMaster.skillbindings.json`은 BA1~BA4를 다음 네 clip group으로 저장한다.

| BA | stage clip | source 재생 | presentation wall | Server hit | combo advance | 입력 window |
|---|---|---|---:|---:|---:|---|
| BA1 | `pc_sp_m_00_sk_att_battle_1_01` | `0..1400ms`, `playRate=2.0` | `700ms` | `50ms` | `276ms` | `92..276ms` |
| BA2 | `pc_sp_m_00_sk_att_battle_1_02` | natural full clip, `1.0` | `1500ms` | `43ms` | `269ms` | `179..269ms` |
| BA3 | `pc_sp_m_00_sk_att_battle_1_03` | natural full clip, `1.0` | `1067ms` | `28ms` | `494ms` | `93..494ms` |
| BA4 | `pc_sp_m_00_sk_att_battle_1_04` | natural full clip, `1.0` | `1700ms` | `335ms` | `1700ms` | 닫힘 |

`PlayerSkills.json`의 top-level `actionDurationMs/hitTimeMs`는 BA1 mirror인 `700/50ms`다.
네 stage의 `actionDurationMs`는 `700/1500/1067/1700ms`, `comboAdvanceMs`는
`276/269/494/1700ms`다. 마지막 BA4는 terminal stage이므로 advance와 duration이 같고
입력 window가 닫혀 있다.

buffered LMB는 BA1~BA3의 빠른 `276/269/494ms` 경계에서 다음 stage로 전환한다. 다음 BA가
buffer되지 않았거나 MOVE/SKILL이 대기 중이면 현재 stage를 각각
`700/1500/1067ms`의 전체 action duration까지 유지한다. 공용 `CPlayerSkillSystem` 알고리즘은
수정하지 않고 기존 public 의미를 데이터와 contract test로 다시 분리했다.

### 2.2 root motion

`DimensionMaster.rootmotion.json`의 `2050010` stage curve는 각 presentation wall duration과
같은 `700/1500/1067/1700ms`에서 끝난다. sample 수는 `43/46/33/52`, 마지막 forward 값은
`0.8418/0.5131/0.2802/1.0404`다. 따라서 current main의 BA1/BA2 짧은 source slice가 아니라
복구된 `_01/_02` motion curve를 Server stage가 소비한다.

### 2.3 BA Product Effect cue

Product cue는 다음처럼 stage clip 시작에 연결했다.

| BA | clip source cue | Product Effect stable ID |
|---|---:|---|
| BA1 | `_01 @ 0ms` | `effect.dimensionmaster.skill.2050010.ba1.unified` |
| BA2 | `_02 @ 0ms` | `effect.dimensionmaster.skill.2050010.ba2.unified` |
| BA3 | `_03 @ 0ms` | `effect.dimensionmaster.skill.2050010.ba3.unified` |
| BA4 | `_04 @ 0ms` | `effect.dimensionmaster.skill.2050010.ba4.unified` |

네 cue는 모두 `root / follow / action_facing / natural`을 유지한다. 실제 animevents 변경은
BA2 cue를 `_01 @ 300ms`에서 `_02 @ 0ms`로 옮긴 한 행이며 BA1, BA3, BA4 stable ID는
그대로다. `stop=natural`이므로 다음 BA로 전환돼도 각 Effect의 tail은 현재 authored lifetime을
따른다.

## 3. T ground-target navigation 연결 결과

### 3.1 root cause

`CClientReplication::Create_Character`는 replicated Character의
`CHARACTER_DESC::pNavigationPrototypeTag`에 항상 `nullptr`를 전달했다. 이 때문에 로컬
차원술사도 navigation component 없이 생성됐고, `CCharacter::Try_SampleTargetGround`가
실패해 preview가 떠 있어도 valid confirm 상태가 될 수 없었다. 결과적으로 LMB
`GROUND_POINT` command, Server 승인 action과 기존
`pc_sp_m_00_sk_sk_dimensionprison` presentation이 모두 시작되지 않았다.

### 3.2 explicit Area navigation wiring

해결 경로는 다음 하나다.

```text
LevelRegistry의 stable map area ID
-> CClientReplication::DESC.strMapAreaId
-> CMapNavigationContract::Resolve_Area
-> runtime grid/prototype tag 검증 및 보관
-> locally controlled Character navigation component
-> valid LMB GROUND_POINT confirm
```

`CClientReplication::Initialize`는 비어 있지 않은 `strMapAreaId`를 요구하고
`Resolve_Area` 결과의 `runtimeGridAvailable`과 `prototypeTag`를 검증한다. 성공한 tag는
`m_strLocalPlayerNavigationPrototypeTag`에 보관하고, `Create_Character`는
`isLocallyControlled`인 Character에만 이를 전달한다. remote Character는 계속
`pNavigationPrototypeTag = nullptr`다. Client navigation은 preview valid/invalid sample만
담당하고 최종 finite/range/navigation admission과 승인 target XYZ는 계속 Server 권위다.

다음 네 Level은 enum이나 prototype tag를 추측하지 않고 각 `CLevelRegistry` descriptor의
`pMapAreaId`를 replication에 명시한다.

- Character Select
- Bern
- Development
- Valtan Arena

## 4. 변경 범위와 명시적 제외

이번 구현 단위에서 변경한 제품 계약은 다음과 같다.

- `DimensionMaster.skillbindings.json`: 네 BA stage clip 1:1 복구
- `DimensionMaster.animevents`: BA2 Product cue를 `_02 @ 0ms`로 이동
- `PlayerSkills.json`과 balance provenance receipt: full duration, hit, combo advance 반영
- `DimensionMaster.rootmotion.json`: 네 stage 전체 motion curve 복구
- `ServerGameplayContractTests.cpp`: full duration과 buffered combo boundary를 별도 검증
- `test_dimensionmaster_2050010_stage_split.py`: full-stage binding, Server timing, cue, catalog identity,
  root motion을 검증하는 5개 focused test로 교체
- character effect restoration inventory: 변경된 binding/animevents identity를 다시 기록
- `ClientReplication.h/.cpp`: explicit `strMapAreaId` resolve와 local-only navigation tag 연결
- `Level_CharacterSelect.cpp`, `Level_Bern.cpp`, `Level_Development.cpp`,
  `Level_ValtanArena.cpp`: 각 Level의 stable map area ID 전달
- `test_ground_target_preview_prototype_scope.py`: 네 Level 전달, navigation resolve와
  local-only Character 연결을 고정하는 focused contract 추가

다음 항목은 이번 구현에서 바꾸지 않았다.

- V0 또는 과거 BA Effect payload
- `effect.dimensionmaster.skill.2050010.ba1~ba4.unified.effect.json` 네 authored 문서의 내용
- `Data/Effects/EffectCatalog.json`, runtime catalog와 content-addressed Effect payload
- playable `sourceStartMs` parser/runtime와 Action Presentation 공용 clock
- 차원술사 A `2050210`의 기존 main 상태
- `PlayerSkillTargeting.json`, T `2050500` balance·skillbinding·Server timing/damage
- T Effect authored source와 runtime payload

`Publish-Effects.ps1`은 source 또는 payload publish 없이 `Validate`만 실행했다. 따라서 이 결과는
현재 stable ID가 cue와 catalog에서 resolve된다는 증거이며 Effect 모양을 복원하거나 변경했다는
증거가 아니다.

## 5. Debug 자동 검증 결과

아래 항목은 실제 실행 결과가 확인된 범위만 PASS로 기록한다.

| 검증 | 결과 |
|---|---|
| `test_dimensionmaster_2050010_stage_split.py` | 5 tests PASS |
| `test_ground_target_preview_prototype_scope.py` | 3 tests PASS |
| `test_build_character_effect_restoration_inventory.py` | 23 tests PASS |
| direct-authored runtime focused test | 6 tests PASS |
| `Publish-GameplayBalance.ps1 -Mode Validate` | PASS |
| `Publish-GameplayBalance.ps1 -Mode Publish` | PASS |
| `Publish-BalanceRuntimeSet.ps1 -Mode Validate` | PASS |
| `Publish-BalanceRuntimeSet.ps1 -Mode Publish` | PASS |
| `Publish-Effects.ps1 -Mode Validate` | PASS, 163 entries / 171 bindings |
| `ActionPresentationTimelineHarness` Debug | PASS |
| Engine Debug build + `UpdateLib.bat Debug` | compile/link PASS |
| Shared Debug build | compile/link PASS |
| NetworkProtocolHarness Debug build | compile/link PASS |
| Server Debug build | compile/link PASS |
| Debug `Server.exe --contract-test` | PASS, failures 0 |
| Client Debug build, T wiring 반영 뒤 재실행 | compile/link PASS |

Effect Validate의 `163 entries / 171 bindings`는 admission 검증 수치다. authored Effect source와
runtime payload를 새로 publish하지 않았다.

## 6. 기존 baseline failure와 이번 판정

전체 regression의 실패를 BA 복구 실패와 섞지 않았다.

- 전체 `Test-EffectPipeline`은 184 tests 중 `9 failures / 18 errors`다. current main에 남아 있는
  Valtan/catalog frozen expectation 문제이며, BA focused 5 tests와 direct-authored runtime focused
  6 tests는 별도로 통과했다. 이번 BA 변경에서 이 광역 baseline을 고치거나 성공으로 기록하지 않았다.
- `Invoke-BuildAndRegression.ps1 -Configuration Debug`는 Engine부터 Client까지 Debug compile/link와
  Server contract를 통과한 뒤 기존 `Sync-EffectDataProject.ps1 -Check` stale registration에서
  중단됐다. 따라서 전체 Debug regression을 PASS로 기록하지 않는다.
- `Invoke-BuildAndRegression.ps1 -Configuration Release`는 Engine, Shared,
  NetworkProtocolHarness, Server, Client compile/link와 Effect validate를 완료한 뒤 같은
  `Sync-EffectDataProject.ps1 -Check` stale registration에서 중단됐다. 따라서 전체 Release
  regression도 PASS가 아니다.
- 회귀 실행이 만든 범위 밖 WorldDestruction runtime 출력은 이번 BA/T 구현 또는 완료 증거에 포함하지
  않는다.

## 7. Release 검증 상태

T wiring 반영 뒤 Client x64 Release compile/link는 PASS다. 이어 실행한 Release 전체
`Invoke-BuildAndRegression.ps1`도 Engine, Shared, NetworkProtocolHarness, Server, Client를
compile/link했고 Effect validate를 통과했다. 그러나 그 뒤 기존
`Sync-EffectDataProject.ps1 -Check` stale registration에서 중단됐으므로 전체 명령의 최종
판정은 PASS가 아니다.

```text
Client x64 Release post-fix: PASS
NetworkProtocolHarness Release: PASS, failures 0
Server Release --contract-test: PASS, failures 0
Server Release --dimensionmaster-ground-target-contract: PASS, 11/11
ActionPresentationTimelineHarness Release: PASS
Release full Invoke-BuildAndRegression: BASELINE_BLOCKED
```

compile/link와 Effect validate 성공은 실행한 하위 단계의 증거로만 기록하고, baseline registration
gate를 해소해 전체 명령이 exit 0이 되기 전에는 full Release regression PASS로 승격하지 않는다.

## 8. 사용자 visual 검증

에이전트는 Client/UI를 실행하거나 화면 결과를 대신 판정하지 않았다. 사용자는 전체 검증 가능한
실행 산출물이 준비된 뒤 다음을 직접 확인한다.

1. 차원술사 LMB 한 번에서 BA1 `_01` motion이 약 `700ms` 재생되는지 확인한다.
2. 다음 LMB가 buffer되면 BA1은 `276ms`에 BA2로 전환하고 `_02/_03/_04` 순서가 유지되는지 확인한다.
3. BA1~BA4의 현재 main authored Effect가 각 stage 시작에 한 번씩 재생되는지 확인한다.
4. 이동·회전 중 네 Effect가 `root`를 follow하고 action facing과 natural tail을 유지하는지 확인한다.
5. 이번 결과가 V0 payload 복원이 아니라 현재 authored payload의 stage 연결 수정임을 구분해 판정한다.
6. T로 ground-target preview를 연 뒤 valid 위치에서 LMB를 누르면 preview가 닫히고
   `pc_sp_m_00_sk_sk_dimensionprison` animation이 시작되는지 확인한다.
7. navigation 밖 또는 그 밖의 invalid 위치에서는 red preview를 유지하고 LMB가 action을 시작하지
   않는지 확인한다.
8. preview 중 RMB를 누르면 cancel되고 T action이나 animation이 시작되지 않는지 확인한다.

BA Effect와 T ground-target의 사용자의 서면 관찰 전 최종 상태는 다음과 같다.

```text
PENDING_USER_VISUAL_GATE
```

## 9. 완료 판정 경계

BA와 T 구현, focused contract, post-fix Debug/Release Client compile/link는 완료된 범위다.
Debug/Release 전체 regression의 existing stale registration과 사용자의 BA/T visual 판정은 완료
항목으로 승격하지 않는다. V0 또는 과거 Effect payload 복원도 계속 이 구현 단위 밖이다.
