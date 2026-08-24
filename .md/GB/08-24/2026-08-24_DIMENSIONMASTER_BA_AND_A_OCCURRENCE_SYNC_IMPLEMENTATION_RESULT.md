# 차원술사 BA 3동작 자동 연계와 LMB/T 확정 입력 수정 구현 결과

## 1. 결론과 현재 판정

기준선은 `origin/main@1eb50f0bf9e33bd38267df41c336c50463b9e940`이다. 원본 read-only
reference에는 `_01/_02/_03/_04` 네 physical clip이 존재한다. 이번 제품 계약은 원본 파일 수를
세 개라고 바꾼 것이 아니라, `_01` 안에 이미 함께 들어 있는 두 번의 찌르기를 하나의 motion으로
유지하고 사용자가 확인한 동작 순서에 맞춰 다음 세 Server stage로 project-tune한 결과다.

```text
_01 source 0..3000ms / playRate 2.0 = 1500ms
-> _03 natural = 1067ms
-> _04 natural = 1700ms
```

LMB 한 번은 이 세 motion을 Server 권위로 자동 연계한다. Product Effect cue는
`ba2 -> ba3 -> ba1` 순서로 stable ID만 연결했고 authored Effect payload는 수정하지 않았다.
LMB 입력은 command 제출 성공 뒤에만 press를 소비하는 transactional edge gate로 바꿨다. T는 최신
snapshot에서 즉시 시작 가능하거나 실행 중 COMBO 뒤에 예약 가능한 경우 preview를 시작·유지하고,
성공한 confirm LMB가 BA로 새지 않도록 release까지 suppression한다.

| 구분 | 현재 판정 |
|---|---|
| BA 3-motion animation·Server timing·root motion | 구현 완료 |
| Product Effect cue `ba2 -> ba3 -> ba1` | 구현 완료 |
| LMB transactional press edge | 구현 완료 |
| T availability preflight·confirm suppression | 구현 완료 |
| focused Python 26 tests | PASS |
| gameplay balance Validate/Publish | PASS |
| balance runtime set Validate/Publish | PASS |
| Effect Validate, canonical ResourceRoot | PASS, `162/171/5` |
| `Framework.sln` x64 Debug/Release build | PASS |
| Server x64 Debug/Release build | PASS |
| Server contract | `BASELINE_BLOCKED`, origin/main과 동일한 Valtan 3 failures |
| canonical regression | `BASELINE_BLOCKED`, origin/main과 동일한 Effect project-registration stale |
| 사용자 BA/T visual fidelity | `PENDING_USER_VISUAL_GATE` |

## 2. 원본 4 clips와 제품 3 motions

원본 `DimensionMaster.clipseq/.clipmap/.animnotify`에는 네 clip이 존재하므로 원본을 세 clip
구조라고 기록하지 않는다. 다만 `pc_sp_m_00_sk_att_battle_1_01` 하나에는 다음 두 attack
occurrence가 함께 존재한다.

```text
SOUND  29ms / 303ms
EFFECT 200ms / 400ms
HIT    100..602ms
```

따라서 `_01`을 중간에서 두 Server stage로 잘라 처음부터 다시 재생하면 두 찌르기가 잘리거나
반복된다. 이번 제품 구조는 현재 BA2-linked motion이었던 `_01 source 0..3000ms` 전체를 첫
motion으로 사용하고, 휘두르기 `_03`과 마무리 베기 `_04`를 뒤에 연결한다. 원본 `_02` 파일을
삭제하거나 원본 자료를 변조하지 않고 제품 binding에서만 사용하지 않는다.

| 제품 stage | 의미 | binding | wall duration | Server hit | combo advance | input window |
|---|---|---|---:|---:|---:|---|
| 1 | 두 번 찌르기 | `_01`, `playMs=3000`, `playRate=2.0` | `1500ms` | `50ms` | `1500ms` | automatic `0/0` |
| 2 | 휘두르기 | `_03`, natural | `1067ms` | `28ms` | `1067ms` | automatic `0/0` |
| 3 | 마무리 베기 | `_04`, natural | `1700ms` | `335ms` | `1700ms` | terminal `0/0` |

총 presentation wall time은 `4267ms`다. `PlayerSkills.json`의 top-level
`actionDurationMs/hitTimeMs` mirror는 첫 stage와 같은 `1500/50ms`다. non-final COMBO의
`inputOpenMs/inputCloseMs == 0/0`과
`comboAdvanceMs == actionDurationMs` 조합만 automatic stage로 인정한다. non-zero input
window를 가진 다른 직업 COMBO는 계속 추가 입력을 요구한다.

Server의 단순 fallback damage 경로는 stage마다 한 번의 damage event를 만들기 때문에 stage 수가
4에서 3으로 줄면서 fallback event 수도 4에서 3으로 줄어든다. 이번 범위는 animation, input,
Effect occurrence 연결이며 damage profile 증폭, hit-shape 추가 또는 별도 damage rebalance는
포함하지 않았다.

## 3. Product Effect cue와 authored payload 보호

`DimensionMaster.animevents`에는 다음 세 Product cue를 둔다.

| motion clip | Product Effect stable ID |
|---|---|
| `_01 @ 0ms` | `effect.dimensionmaster.skill.2050010.ba2.unified` |
| `_03 @ 0ms` | `effect.dimensionmaster.skill.2050010.ba3.unified` |
| `_04 @ 0ms` | `effect.dimensionmaster.skill.2050010.ba1.unified` |

세 cue는 `root / follow / action_facing / natural`을 유지한다. 사용자가 Effect Tool에서
손튜닝하는 stable authored 문서를 그대로 소비하도록 연결만 바꿨다. 다음 항목은 수정하거나
삭제하지 않았다.

- `effect.dimensionmaster.skill.2050010.ba1~ba4.unified.effect.json`의 Element와 resource binding
- delay, lifetime, mesh, DDS, material, source recipe와 runtime payload
- 현재 제품 sequence에서 사용하지 않는 `ba4` authored 문서
- V0 또는 과거 Effect payload

작업 시작 시 존재하던 다른 작성자와 사용자의 dirty worktree는 그대로 보호했다. 범위 밖 파일을
reset, checkout, clean하거나 기존 변경을 되돌리지 않았고, Effect authored payload를 정리한다는
이유로 덮어쓰지 않았다.

## 4. Root Motion 결과

`DimensionMaster.rootmotion.json`의 `2050010`은 세 제품 stage와 같은 wall clock을 사용한다.

| stage | source | duration | samples | 마지막 forward |
|---|---|---:|---:|---:|
| 0 | 기존 `_01` source `0..3000ms`를 2배속 재타이밍 | `1500ms` | 91 | `0.9491` |
| 1 | 기존 `_03` curve, old stage 2에서 재번호화 | `1067ms` | 33 | `0.2802` |
| 2 | 기존 `_04` curve, old stage 3에서 재번호화 | `1700ms` | 52 | `1.0404` |

stage 0은 source sample의 `0..3000ms` timestamp만 정확히 절반 wall time으로 변환하고
forward/lateral/up 값은 보존했다. 기존 `_02`용 old stage 1 curve는 제품 sequence에서 제거했다.

## 5. LMB와 T 입력 수정

### 5.1 LMB transactional edge gate

기존 gate는 command sink가 command를 수락하기 전에 현재 물리 press를 제출 완료로 표시했다.
aim, transform 또는 send가 일시 실패하면 같은 LMB press가 영구히 소비됐고, 반대로 입력 차단 중
시작된 press가 차단 해제 뒤 늦은 BA로 나갈 수 있었다.

수정된 `CBASIC_ATTACK_PRESS_EDGE_GATE` 계약은 다음과 같다.

1. raw physical up만 다음 press를 rearm한다.
2. gameplay input이 차단된 상태에서 시작된 press는 그 release까지 blocker가 소비한다.
3. eligible down은 command candidate만 만들고 아직 소비하지 않는다.
4. `Request_UseSkill` 성공 뒤에만 `Commit_Submission()`으로 press를 소비한다.
5. local command sink의 일시 실패는 commit하지 않아 같은 held press가 다음 frame 재시도된다.
6. 성공 뒤 held 상태에서는 같은 press의 두 번째 BA command를 제출하지 않는다.

두 production gate와 ground-target confirm transaction은 `constexpr` state machine으로 유지하고 같은
구현을 `static_assert` 계약에서 직접 실행한다. 이 compile-time harness는 capture 중 시작된 press,
sink reject 재시도, sink accept 뒤 held 억제, raw release 재무장과 T confirm reject/accept를 fake
request callback으로 검증한다. 따라서 소스 문자열 위치만 확인하는 테스트에 의존하지 않는다.

Client가 animation을 local 선행 재생하지는 않는다. 첫 animation은 계속 Server 승인 snapshot의
action edge에서 시작하므로 정상 network tick 지연을 입력 유실로 숨기지 않는다.

### 5.2 Server automatic combo

Server catalog와 publisher는 non-final `0/0` window를 automatic stage로 읽되 advance와 duration이
같지 않으면 거부한다. `CPlayerSkillSystem`은 automatic stage의 full-motion boundary에서 buffered
LMB 없이 다음 stage로 전환한다. 명시적인 MOVE/SKILL pending command도 automatic chain을 끊지 않고
마지막 motion 종료 뒤 commit한다. manual window를 가진 다른 COMBO는 현재 full-motion 경계에서
pending을 commit하는 기존 buffered-input 계약을 유지한다.

### 5.3 T availability와 confirm suppression

`CPlayerSkillCatalog`은 기존 resource cost와 함께 `identityCost`를 exact `uint32`로 검증·보존한다.
`CCombatHUDViewModel`은 snapshot의 현재 skill ID도 보존해 Server pending 가능 여부를 판단한다. T
preview 시작과 유지는 다음 두 admission을 구분한다.

- action이 `NONE`인 즉시 cast는 alive local player, class, `ACTIVE/GROUND_POINT`, cooldown,
  resource, identity와 required stance를 모두 검사한다.
- 다른 COMBO가 실행 중인 pending cast도 현재 snapshot의 cooldown, resource, identity와 stance를
  만족해야 preview와 confirm을 허용한다. automatic BA에서는 3동작 전체 종료 뒤 Server가 다시
  검증한다.
- Valtan 입장 직후 `isCombatReady=false`는 첫 유효 이동/스킬이 true로 전환하는 상태이므로 T를
  선제 차단하지 않는다.

조건이 사라지면 active preview를 취소한다. valid ground에서
`Request_UseGroundTargetSkill`이 성공한 뒤에는 `Suppress_UntilRelease()`를 먼저 적용하고
preview를 닫는다. 따라서 confirm에 사용한 같은 LMB가 다음 input pass에서 BA로 재해석되지 않는다.
invalid ground와 RMB cancel은 T action 또는 BA를 시작하지 않는다.

## 6. 변경 범위

실제 수정 범위는 다음 vertical slice다.

- animation/balance data: `DimensionMaster.skillbindings.json`,
  `DimensionMaster.animevents`, `DimensionMaster.rootmotion.json`, `PlayerSkills.json`,
  balance provenance receipt
- Client input/catalog: `PlayerController.h/.cpp`, `PlayerSkillCatalog.h/.cpp`,
  `CombatHUDViewModel.h/.cpp`
- Server validation/runtime: `GameplayCatalog.cpp`, `PlayerSkillSystem.cpp`,
  `PlayerSkillSystem.h`, `ServerGameplayContractTests.cpp`
- authoring/validation: `BalanceTool.cpp`, `Publish-GameplayBalance.ps1`
- focused tests: BA stage split, catalog combo timing, Effect Tool automatic audition,
  ground-target availability·LMB gate
- public handoff/known issue: Animation Tool·Balance Tool owner handoff, gameplay interface handbook,
  GB gotchas와 대응 PLAN/RESULT

Effect authored 문서, Effect runtime payload, T animation binding과 T Effect payload는 이 변경
범위에 포함하지 않았다.

## 7. 자동 검증 결과

### 7.1 Focused Python

다음 네 실행형 focused test의 합계 26 tests가 PASS했다.

| test | 결과 |
|---|---:|
| `test_dimensionmaster_2050010_stage_split.py` | 5 PASS |
| `test_player_skill_catalog_combo_timings.py` | 4 PASS |
| `test_effect_tool_buffered_combo_audition.py` | 11 PASS |
| `test_ground_target_preview_prototype_scope.py` | 6 PASS |
| 합계 | **26 PASS** |

검증 범위는 세 clip group, 3-stage timing과 root motion, `ba2/ba3/ba1` stable cue,
automatic parse/rollback, 다른 직업 manual COMBO 보존, Animation/Effect Tool 누적 timeline,
production `static_assert`에서 실행되는 blocked/retry/commit/release LMB gate와 fake-confirm
transaction, T availability와 confirm suppression이다.

### 7.2 Publisher

| 명령 영역 | 결과 |
|---|---|
| `Publish-GameplayBalance.ps1 -Mode Validate` | PASS |
| `Publish-GameplayBalance.ps1 -Mode Publish` | PASS |
| `Publish-BalanceRuntimeSet.ps1 -Mode Validate` | PASS |
| `Publish-BalanceRuntimeSet.ps1 -Mode Publish` | PASS |
| `Publish-Effects.ps1 -Mode Validate` | PASS |

Effect Validate는 임시 또는 추측 경로가 아니라 저장소의 canonical
`Client/Bin/Resources`를 `-ResourceRoot`로 사용했다. 결과 수치는
`162 Effect catalog entries / 171 material-program bindings / 5 registry-bound audition effects`다.
이는 stable cue와 runtime admission 검증이며 Effect authored payload를 publish하거나 시각 모양을
변경했다는 증거가 아니다.

### 7.3 Build와 contract

| 검증 | 결과 |
|---|---|
| `Framework.sln` x64 Debug compile/link | PASS |
| `Framework.sln` x64 Release compile/link | PASS |
| Server x64 Debug/Release compile/link | PASS |
| Debug `Server.exe --contract-test` | `BASELINE_BLOCKED` |
| Release `Server.exe --contract-test` | `BASELINE_BLOCKED` |
| canonical Debug regression `-SkipBuild` | `BASELINE_BLOCKED` |

Server contract는 BA automatic sequence 관련 변경 여부와 별개로 Valtan baseline 3 failures를
반환했다. 같은 기준선의 깨끗한 `origin/main` contract도 동일한 Valtan 3 failures를 반환하므로
이번 변경의 신규 failure로 분류하지 않는다. 그러나 전체 contract command가 failures 0이 아니므로
PASS로 기록하지 않고 `BASELINE_BLOCKED`로 유지한다.

canonical regression은 focused test, gameplay/balance/nav/Effect validation을 통과한 뒤
`Sync-EffectDataProject.ps1 -Check`에서 `Client.vcxproj/.filters` 등록 불일치로 중단됐다. 같은
`origin/main@1eb50f0b`의 깨끗한 worktree에서도 동일 명령이 동일하게 실패하므로 이번 변경의 신규
failure가 아니다. 자동 수정은 이번 BA/T 수직 슬라이스와 무관한 Effect catalog 프로젝트 등록을
대량 변경하므로 범위에 섞지 않고 `BASELINE_BLOCKED`로 기록한다.

### 7.4 깨끗한 origin/main compile 진단

`origin/main@1eb50f0b` detached clean worktree에는 Git 제외 대상인 `EngineSDK`가 없었다. 이 상태에서
`Framework.sln /m`을 바로 실행하면 `EffectRenderContractHarness`가 첫 오류로
`C1083: Engine_Defines.h`를 찾지 못한다. 이는 repository C++ source 오류가 아니라 clean
clone/pull 뒤 초기 SDK publish 순서를 생략한 재현 가능한 로컬 세팅 오류다.

같은 worktree에서 정본 순서를 적용한 결과는 다음과 같다.

| clean main 단계 | 결과 |
|---|---|
| Engine x64 Debug | PASS |
| `UpdateLib.bat Debug` | PASS |
| `Framework.sln` x64 Debug `/nodeReuse:false` | PASS |
| `EffectRenderContractHarness.exe`, `Client.exe` link | PASS |

따라서 현재 기준선 source에는 확인된 compiler/linker regression이 없다. 다른 PC는 `git lfs pull`을
완료하고 구성별로 `Engine -> UpdateLib.bat <Debug|Release> -> Framework.sln` 순서를 지켜야 한다.
특히 clean clone에서 Client부터 빌드하거나 `/m` solution build로 SDK 생성까지 대신하려 하면 같은
missing-header/link 오류가 날 수 있다.

## 8. 사용자 수동 visual 검증

에이전트는 Client/UI를 자율 실행·조작하거나 화면 fidelity를 대신 판정하지 않았다. 사용자는
`Framework.sln` 빌드가 완료된 실행 파일로 다음을 직접 확인한다.

1. LMB 한 번으로 `_01(약 1.5초) -> _03(약 1.067초) -> _04(약 1.7초)`가 추가 클릭 없이 이어지는지 확인한다.
2. 첫 `_01`에서 현재 손튜닝한 `ba2`, `_03`에서 `ba3`, 마지막 `_04`에서 `ba1`이 각각 한 번 시작하는지 확인한다.
3. LMB를 누르고 있어도 두 번째 BA command가 제출되지 않고 Server automatic stage만 진행하는지 확인한다.
4. F1/ImGui가 잡은 상태에서 시작한 LMB가 capture 해제 뒤 늦게 BA로 발사되지 않는지 확인한다.
5. 평시 또는 BA 진행 중 T preview 뒤 valid 위치 confirm LMB가 T animation을 즉시 또는 BA 종료 뒤 시작하고 같은 click이 BA로 새지 않는지 확인한다.
6. T 사용 불가 상태, invalid ground와 RMB cancel에서 T와 BA가 시작되지 않는지 확인한다.

사용자의 서면 관찰 전 최종 시각 상태는 다음과 같다.

```text
PENDING_USER_VISUAL_GATE
```

## 9. 완료 경계

BA 3-motion data, Server automatic stage, LMB transactional input, T availability/confirm suppression과
focused publisher/test 범위는 구현 완료다. 다음 항목은 완료로 승격하지 않는다.

- origin/main에도 동일하게 존재하는 Valtan contract 3 failures: `BASELINE_BLOCKED`
- origin/main에도 동일하게 존재하는 Effect data project-registration stale: `BASELINE_BLOCKED`
- 사용자의 BA/T animation·Effect visual 판정
- 4→3 stage에 따른 별도 damage rebalance
- V0 또는 Effect authored payload 복원
