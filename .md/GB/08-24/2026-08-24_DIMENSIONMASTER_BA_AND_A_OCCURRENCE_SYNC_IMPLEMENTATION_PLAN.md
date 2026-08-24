# 차원술사 BA 3동작 자동 연계와 LMB/T 확정 입력 수정 구현 계획

## 1. 목표와 기준선

- 기준선은 `origin/main@1eb50f0bf9e33bd38267df41c336c50463b9e940`이다.
- 차원술사 기본 공격 `2050010`은 LMB 한 번으로 `_01 -> _03 -> _04` 세 제품 동작을 자동 재생한다.
- 첫 동작은 현재 main의 BA2 stage가 사용하던 `_01 source 0..3000ms / playRate 2.0`을 그대로 사용한다. `_01` 안의 두 attack occurrence를 다시 둘로 자르지 않는다.
- Product Effect stable ID는 사용자 판정 순서대로 `_01 -> ba2`, `_03 -> ba3`, `_04 -> ba1`만 연결한다. Effect 문서 내용은 사용자가 저작하므로 이번 변경에서 손대지 않는다.
- 물리 LMB 한 번은 정확히 한 BA command만 제출한다. 입력 차단 중 시작된 press는 차단 해제 뒤 늦게 발사되지 않고, command sink의 일시 실패는 같은 press를 영구 소비하지 않는다.
- T `2050500`은 최신 snapshot상 즉시 시작 가능하거나 실행 중 COMBO 뒤에 예약 가능한 경우 preview를 열고 유지하며, 성공한 confirm LMB는 release 전 BA로 새지 않는다.
- 자동 검증과 Debug/Release compile/link를 완료하되 Client 실행과 visual fidelity 판정은 사용자가 직접 한다.

## 2. 원본과 현재 main의 실측

원본 read-only reference인 `DimensionMaster.clipseq/.clipmap/.animnotify`에는 `_01/_02/_03/_04` 네 clip이 있다. 따라서 원본 파일이 세 clip이라고 기록하지 않는다. 다만 `_01` 하나에는 다음 두 attack occurrence가 이미 함께 들어 있다.

```text
SOUND  29ms / 303ms
EFFECT 200ms / 400ms
HIT    100..602ms
```

이번 제품 계약은 원본 네 clip을 그대로 복제하는 것이 아니라, 사용자가 확인한 `두 번 찌르기 -> 휘두르기 -> 마무리 베기`를 세 Server stage로 표현하는 project-tuned 구성이다.

현재 main의 회귀 상태는 다음과 같다.

```text
stage 1  _01 1400ms / 2x
stage 2  _01 3000ms / 2x
stage 3  _03
stage 4  _01 3400ms / 2x

_01 -> ba2
_03 -> ba3
```

같은 `_01`을 시작점부터 세 번 재생하고 `_01@0`의 BA2 cue도 재사용하므로, 두 타 모션과 natural Effect tail이 중첩된다. 과거 `_01`을 300ms 경계에서 잘랐던 방식도 같은 clip 내부 occurrence와 Server stage를 혼동했다. 이 둘이 반복·절단 animation의 원인이다. LMB가 늦거나 씹히는 문제는 별도의 input/capture/Server window 결함으로 분리해 수정한다.

## G00. 3-stage animation·Server timing·root motion

### 3. 제품 계약

| 제품 stage | 의미 | clip | wall duration | Server hit | advance/window |
|---|---|---|---:|---:|---|
| 1 | 두 번 찌르기 | `_01`, `playMs=3000`, `playRate=2.0` | `1500ms` | `50ms` | automatic `1500ms`, `0/0` |
| 2 | 휘두르기 | `_03` natural | `1067ms` | `28ms` | automatic `1067ms`, `0/0` |
| 3 | 마무리 베기 | `_04` natural | `1700ms` | `335ms` | terminal `1700ms`, `0/0` |

총 animation wall time은 `4267ms`다. non-final COMBO의 `inputOpenMs/inputCloseMs == 0/0`은 `comboAdvanceMs == actionDurationMs`인 automatic stage로 정의한다. 기존 non-zero window를 가진 다른 직업 COMBO는 계속 실제 추가 입력을 요구한다.

수정 경로는 다음 하나다.

- `DimensionMaster.skillbindings.json`: 세 group `_01(3000/2x)`, `_03`, `_04`.
- `PlayerSkills.json`: `2050010.comboStages`를 `1500/1067/1700ms` 세 행으로 축소하고 automatic `0/0`을 저장한다.
- `DimensionMaster.rootmotion.json`: source `_01 0..3000ms` curve를 2배속 wall clock으로 변환한 stage 0, 기존 `_03/_04` curve를 stage 1/2로 재번호화한다.
- gameplay balance receipt는 publisher 동기화 도구가 실제 새 배열 길이와 값을 기록하게 한다.
- `CPlayerSkillSystem`은 automatic non-final stage를 buffered press 없이 전환한다. pending MOVE/SKILL은 automatic chain 전체를 보존해 마지막 motion 종료 뒤 commit하고, manual COMBO는 현재 full motion 경계를 유지한다.
- catalog/Balance Tool/publisher는 manual window와 automatic `0/0`을 구분해 fail-closed 검증한다.
- Server contract는 단 한 번의 `Try_Start`가 stage `1 -> 2 -> 3 -> NONE`을 만들고, 다른 직업 manual COMBO는 자동화되지 않음을 검증한다.

Server stage 수를 4에서 3으로 줄이면 현재 fallback damage 발생도 네 번에서 세 번으로 바뀐다. 이번 요청은 animation/input/Effect 연결 범위이므로 별도 damage-profile 증폭이나 새 hit shape를 추측해 넣지 않고 결과 문서에 이 경계를 명시한다.

## G01. Product Effect stable cue

### 4. 연결만 수정

`DimensionMaster.animevents`의 Product cue는 다음 세 행만 둔다.

```text
_01 @ 0ms -> effect.dimensionmaster.skill.2050010.ba2.unified
_03 @ 0ms -> effect.dimensionmaster.skill.2050010.ba3.unified
_04 @ 0ms -> effect.dimensionmaster.skill.2050010.ba1.unified
```

세 cue는 모두 `root / follow / action_facing / natural`이다. `ba1~ba4.unified` authored 문서, Element, delay, mesh, DDS, material, runtime payload는 수정하지 않는다. 사용자의 로컬 BA2 손튜닝도 stable ID가 같으면 첫 동작에서 한 번 재생된다. 사용하지 않는 `_02`와 `ba4` 파일도 삭제하지 않는다.

## G02. LMB edge와 T confirm

### 5. LMB command transaction

현재 `CBASIC_ATTACK_PRESS_EDGE_GATE`는 `Poll_BasicAttack`에서 command sink 호출 전에 press를 제출 완료로 표시한다. 이후 aim/transform/send가 실패하면 같은 물리 press가 영구히 소비된다. 반대로 ImGui 또는 gameplay input 차단 중 시작된 press는 차단이 풀릴 때까지 미제출로 남아 늦은 BA가 될 수 있다.

수정 계약은 다음과 같다.

1. raw up은 gate를 rearm한다.
2. raw down이지만 gameplay command가 차단된 press는 그 release까지 consume한다.
3. eligible raw down은 candidate만 만든다.
4. `Request_UseSkill` 성공 뒤에만 gate를 commit한다.
5. 일시적인 local sink 실패는 commit하지 않아 같은 held press가 다음 frame 재시도된다.
6. 성공 뒤 held 상태에서는 중복 command를 제출하지 않는다.

Client가 animation을 선행 예측하지는 않는다. 첫 화면 animation은 계속 Server 승인 snapshot edge에서 시작하므로 정상 network tick 지연과 입력 유실을 구분한다.

### 6. T ground target

- `PlayerSkillCatalog`은 `identityCost`를 exact `uint32`로 검증·보존하고, HUD는 snapshot의 현재 skill ID를 보존한다.
- action이 `NONE`이면 class, ACTIVE/GROUND_POINT kind, HP, stance, cooldown, resource, identity를 검사해 즉시 시작 가능한 경우에만 preview를 연다.
- 다른 COMBO가 실행 중이어도 현재 snapshot의 cost/cooldown/stance를 만족하면 Server pending 계약과 같이 T preview/confirm을 허용한다. automatic BA에서는 3동작 전체 종료 뒤 Server가 다시 검증하고 실행한다.
- Valtan의 `isCombatReady=false`는 유효한 첫 이동/스킬이 전투를 시작하는 상태이므로 T를 선제 차단하지 않는다.
- `Request_UseGroundTargetSkill` 성공 직후 confirm LMB를 `Suppress_UntilRelease()`하고 나서 preview를 닫는다.
- invalid ground와 RMB cancel은 action을 시작하지 않는다.
- ImGui mouse capture를 전역 우회하지 않는다.

## G03. 검증과 인계

### 7. 자동 검증

- `test_dimensionmaster_2050010_stage_split.py`: 세 clip group, 3-stage timing, root motion, 세 stable cue.
- `test_player_skill_catalog_combo_timings.py`: automatic stage parse/rollback과 DM만의 automatic non-final 행.
- `test_effect_tool_buffered_combo_audition.py`: `_01 -> _03 -> _04`, BA3 누적 offset `1500ms`, final tail.
- `PlayerController.h` compile-time contract +
  `test_ground_target_preview_prototype_scope.py`: production gate/fake-confirm callback의
  blocked/retry/commit/release 상태 전이와 T availability/runtime join.
- gameplay balance Validate/Publish 및 runtime-set Validate/Publish.
- Effect publisher Validate.
- Server Debug/Release build와 `Server.exe --contract-test`.
- Client Debug/Release compile/link.
- JSON parse와 `git diff --check`.

### 8. 사용자 수동 확인

1. LMB 한 번으로 `_01(약 1.5초) -> _03(약 1.067초) -> _04(약 1.7초)`가 자동으로 이어지는지 확인한다.
2. 첫 `_01`에서 현재 손튜닝한 `ba2`, `_03`에서 `ba3`, 마지막 `_04`에서 `ba1` stable Effect가 각각 한 번 시작하는지 확인한다.
3. LMB를 누르고 있어도 두 번째 BA command가 자동 제출되지 않고, 추가 클릭 없이 Server 자동 stage만 진행하는지 확인한다.
4. F1/ImGui가 press를 잡은 상태에서 시작한 LMB가 캡처 해제 뒤 늦게 BA로 발사되지 않는지 확인한다.
5. 평시 또는 BA 진행 중 T preview 뒤 valid 위치 LMB가 `dimensionprison` animation을 즉시 또는 BA 종료 뒤 재생하고, 같은 click이 BA로 새지 않는지 확인한다.
6. invalid 위치와 RMB cancel은 T와 BA를 모두 시작하지 않는지 확인한다.

사용자의 서면 관찰 전 visual 상태는 `PENDING_USER_VISUAL_GATE`로 유지한다.
