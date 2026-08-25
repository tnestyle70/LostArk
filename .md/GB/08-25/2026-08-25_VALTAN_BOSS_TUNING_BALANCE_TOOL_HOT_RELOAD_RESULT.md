# Valtan Boss Tuning Balance Tool Hot Reload 결과

## 1. 구현 상태

`origin/main` `b8653bfd`의 PR #210 authoring data 위에 runtime core를 분리 조립했다.
authoring JSON은 main의 정본을 그대로 소비하며 이 변경에서 다시 수정하지 않았다.

구현된 계약은 다음과 같다.

- Shared typed gameplay revision, 2PC packet, world active/pinned revision, decision trace,
  audition lifecycle 직렬화
- Client world-entry presentation baseline과 byte-identical alias stage/commit/abort/isolation
- Server immutable catalog generation, all-room stage/commit/abort, occurrence revision pin
- Server candidate path/hash/non-Valtan domain 검증과 durable active pointer/journal recovery
- Balance Tool의 strict candidate apply class, revision 상태, decision trace, typed audition
- Balance Tool/Effect Tool 공용 `CValtanPatternAuditionService`
- Valtan v19 managed selection window, typed gameplay phase action, per-player combat-object volley 소비

## 2. 범위 분리

다음 dirty 작업은 이 branch에 포함하지 않았다.

- Server performance/reaper/navigation/replan
- Effect recovery와 sky-axe visual asset 조정
- DimensionMaster BA 변경
- profiler 및 ActionPresentationTimelineHarness

## 3. 자동 검증

| 검증 | 결과 |
|---|---|
| `test_valtan_pattern_tree_contract.py` | PASS 14/14 |
| `test_valtan_balance_tool_contract.py` | PASS 21/21 |
| `Publish-GameplayBalance.ps1 -Mode Validate` | PASS |
| `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate` | PASS |
| `test_valtan_pattern_master_v2.py` | 실행 중; 최종 commit 전 exact 결과 갱신 |
| `Test-ValtanTuningRuntimeSet.ps1` | 실행 중; 최종 commit 전 exact 결과 갱신 |
| `git diff --check` | PASS |

## 4. 남은 수동 경계

Client/UI는 에이전트가 실행하거나 조작하지 않았다. 사용자는 Debug Server와 Client를 직접 시작한 뒤
`Valtan Arena -> F1 -> Balance Tool -> Bosses -> Valtan`에서 다음을 확인해야 한다.

1. `Validate Draft -> Save Authoring -> Publish Candidate`
2. `Apply class: HOT_RELOAD`인 candidate만 `Apply Hot Reload`
3. active/pending/pinned revision과 decision trace가 Server 상태와 일치
4. `Play Server Pattern` lifecycle이 queued/active/completed 또는 typed failure로 닫힘

animation/Effect visual fidelity와 occurrence 승인 여부는 사용자 서면 판정 전에는 PASS가 아니다.
