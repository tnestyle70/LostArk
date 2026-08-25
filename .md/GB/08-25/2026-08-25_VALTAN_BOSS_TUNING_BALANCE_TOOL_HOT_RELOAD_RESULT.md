# Valtan Boss Tuning Balance Tool Hot Reload 결과

## 1. 구현 상태

`origin/main`의 PR #210 split authoring data 위에 runtime core를 분리 조립하고,
rotation product v3와 남아 있던 legacy v2 projector/validator 사이의 회귀를 닫았다.
`Data/Valtan/Valtan.pattern.json`은 이제 덮어쓰지 않는 v1 migration 입력이다. 활성 저작 정본은
`Data/Valtan/Valtan.gameplay.json`, `Valtan.presentation.json`과 companion 문서이고,
명시적 `PublishV2`가 7개 split runtime product를 같은 revision으로 원자적으로 투영한다.
Client/Server build는 투영 결과를 수정하지 않고 drift를 fail-closed 검증한다.

구현된 계약은 다음과 같다.

- Shared typed gameplay revision, 2PC packet, world active/pinned revision, decision trace,
  audition lifecycle 직렬화
- Client world-entry presentation baseline과 byte-identical alias stage/commit/abort/isolation
- Server immutable catalog generation, all-room stage/commit/abort, occurrence revision pin
- Server candidate path/hash/non-Valtan domain 검증과 durable active pointer/journal recovery
- Balance Tool의 strict candidate apply class, revision 상태, decision trace, typed audition
- Balance Tool/Effect Tool 공용 `CValtanPatternAuditionService`
- Valtan v19 managed selection window, typed gameplay phase action, per-player combat-object volley 소비
- `Project-ValtanPatternMaster.ps1 -Mode Validate/ValidateV2`의 v3 split validator 통합
- compare-and-swap와 rollback을 포함한 7개 산출물 atomic `PublishV2`
- legacy v2 `Publish`의 명시적 거부, explicit `PublishV2`, build 전 자동 `ValidateV2`
- charge 판정을 sidecar가 아니라 `WALL_CONTACT -> immediate GROGGY`와 같은 pattern의
  단일 `PART_BREAK` sequence에서 파생
- cue의 source window/animation occurrence strict join과 finite cue end boundary 통일
- Valtan 실제 model clip 이름을 사용하는 idle/run/presentation fallback
- Valtan Arena debug rotation의 v3 `candidates[].patternId/enabled` 소비
- high-jump volley의 target axe -> `effect.valtan.sky-axe.active` occurrence 검증
- main PR #220의 first-pulse target tracking, per-target radial spacing, v19 strict admission 보존
- `mappingBasis`를 합의된 6개 vocabulary로 제한하고 occurrence/cue 오타를 fail-closed 거부
- source manifest 입력을 LF로 고정하고 manifest 계산 시 CRLF/CR만 LF로 정규화해
  기존 Windows checkout과 새 LF checkout이 같은 source revision을 사용

## 2. 범위 분리

다음 dirty 작업은 이 branch에 포함하지 않았다.

- Server performance/reaper/navigation/replan
- Effect visual asset 자체의 재저작
- DimensionMaster BA 변경
- profiler

## 3. 자동 검증

| 검증 | 결과 |
|---|---|
| `test_valtan_pattern_tree_contract.py` | PASS 15/15 |
| `test_valtan_balance_tool_contract.py` | PASS 22/22 |
| `test_effect_tool_valtan_saved_rows.py` | PASS 28/28 |
| `test_animation_tool_valtan_pattern_master.py` | PASS 7/7 |
| `test_valtan_pattern_master_v2.py` | PASS 34/34; LF/CRLF source revision 동일성 포함 |
| `Publish-GameplayBalance.ps1 -Mode Validate` | PASS |
| `Project-ValtanPatternMaster.ps1 -Mode PublishV2` 2회 | PASS; 첫 투영 뒤 `changed=0`, idempotent |
| `Project-ValtanPatternMaster.ps1 -Mode Validate` | PASS |
| legacy `Project-ValtanPatternMaster.ps1 -Mode Publish` | PASS; 의도한 retirement 오류로 거부 |
| `Test-ValtanTuningRuntimeSet.ps1` | PASS 32/32 |
| `ActionPresentationTimelineHarness` Release | BUILD PASS / RUN PASS |
| Client x64 Debug | BUILD PASS; main 결합 뒤 Client effective source 동일 |
| Server x64 Debug | BUILD PASS; main PR #220 conflict resolution 포함 |
| `--dimensionmaster-ground-target-contract` | PASS, `failures : 0` |
| `--contract-test` | stack overflow 해소 및 끝까지 실행; 별도 실행 중인 Debug Server가 process mutex를 점유해 single-owner 항목 1건만 환경 실패 |
| `git diff --check` | PASS |

## 4. 남은 수동 경계

Client/UI는 에이전트가 실행하거나 조작하지 않았다. 사용자는 Debug Server와 Client를 직접 시작한 뒤
`Valtan Arena -> F1 -> Balance Tool -> Bosses -> Valtan`에서 다음을 확인해야 한다.

1. `Validate Draft -> Save Authoring -> Publish Candidate`
2. `Apply class: HOT_RELOAD`인 candidate만 `Apply Hot Reload`
3. active/pending/pinned revision과 decision trace가 Server 상태와 일치
4. `Play Server Pattern` lifecycle이 queued/active/completed 또는 typed failure로 닫힘

animation/Effect visual fidelity와 occurrence 승인 여부는 사용자 서면 판정 전에는 PASS가 아니다.
