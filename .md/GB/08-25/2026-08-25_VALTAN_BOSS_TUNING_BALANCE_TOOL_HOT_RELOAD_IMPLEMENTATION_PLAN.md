# Valtan Boss Tuning Balance Tool Hot Reload 구현 계획

## 1. 목표

PR #210으로 `origin/main`에 합쳐진 Valtan gameplay/presentation split authoring을 입력으로 사용해,
Debug F1 Balance Tool에서 만든 gameplay-only candidate를 Server 권위의 immutable revision으로
검증하고 모든 영향 room/Client에 원자 적용한다.

이 변경은 다음 수직 슬라이스만 소유한다.

- Shared: typed revision identity, PREPARE/READY-or-NACK/COMMIT-or-ABORT, active/pinned revision wire 계약
- Server: immutable `CGameplayCatalog` generation, process-wide room cohort, room tick boundary commit,
  진행 중 occurrence pin, durable active pointer/journal, restart recovery
- Client: world-entry presentation baseline, byte-identical alias admission, revision isolation,
  typed Valtan decision trace와 audition lifecycle
- Tool: Validate Draft -> Save Authoring -> Publish Candidate -> Apply Hot Reload -> Play Server Pattern
- 검증: Valtan 32/14/21 focused gate, gameplay/Valtan publisher validation, protocol/Server contract

## 2. 정본과 데이터 흐름

`Data/Valtan/Valtan.gameplay.json`과 `Valtan.presentation.json`은 PR #210의 joined authoring
정본이다. 이 브랜치는 해당 JSON을 다시 저작하거나 복제하지 않는다.

```text
joined authoring
-> immutable authoring revision
-> immutable candidate artifact set
-> Server candidate catalog stage
-> affected Client presentation PREPARE
-> every room stage
-> one fixed-tick COMMIT or global ABORT
```

candidate manifest revision, raw `Gameplay.bootstrap` content revision, non-Valtan gameplay domain
revision은 서로 다른 identity다. Valtan-only candidate가 더 최신인 non-Valtan row를 되돌릴 수 없도록
세 identity를 함께 검증한다.

## 3. transaction 불변식

1. Server active catalog는 immutable shared generation이다.
2. PREPARE 전 parse/hash/path/domain validation을 끝낸다.
3. shared room과 session-private Character Select room을 같은 cohort로 stage한다.
4. join/disconnect/room 생성은 transaction membership 검증을 통과한다.
5. 모든 영향 Client가 required presentation lane을 READY하기 전에는 commit하지 않는다.
6. NACK, timeout, disconnect, stale base, hash mismatch는 전 room에서 old generation을 유지한다.
7. running pattern, queued mechanic, combat object, timeline/audition은 시작 revision을 끝까지 pin한다.
8. 다음 occurrence만 active generation을 소비한다.
9. durable runtime pointer 승격이 실패하면 memory commit도 수행하지 않는다.
10. restart는 exact candidate를 재검증하며 손상 상태를 packaged baseline으로 조용히 대체하지 않는다.

## 4. Client admission

현재 non-byte-identical presentation generation loader는 없다. 따라서 Client는 world entry 때 캡처한
immutable baseline과 candidate의 모든 required presentation artifact가 byte-identical일 때만 alias를
stage하고 READY한다. Release Client, missing lane, repository drift, overlapping/stale transaction은 NACK
또는 isolation으로 닫는다.

## 5. Valtan observability와 audition

Server selector는 선택 source/result, 후보별 exclusion, authored/effective weight, RNG interval,
definition revision을 bounded trace로 보낸다. Balance Tool은 이 typed trace만 표시한다.

`PLAY_PATTERN_ID` 결과와 lifecycle queue는 `CValtanPatternAuditionService` 한 곳이 frame마다
소비한다. Balance Tool과 Effect Tool은 stable consumer ID로 같은 서비스를 사용하며 packet queue를
각자 drain하지 않는다.

## 6. 명시적 제외

다음은 이 변경에 포함하지 않는다.

- Server room performance 계측, session reaper, navigation/path replan 조정
- Effect recovery, sky-axe visual inventory/fidelity 조정
- DimensionMaster 2050010 BA stage/duration 변경
- Client profiler와 ActionPresentationTimelineHarness 확장
- non-byte-identical presentation live generation과 controlled encounter reset

## 7. 완료 조건

- `test_valtan_pattern_master_v2.py`: 32/32
- `test_valtan_pattern_tree_contract.py`: 14/14
- `test_valtan_balance_tool_contract.py`: 21/21
- `Publish-GameplayBalance.ps1 -Mode Validate`
- `Publish-ValtanTuningRuntimeSet.ps1 -Mode Validate`
- `Test-ValtanTuningRuntimeSet.ps1`
- `git diff --check`
- 가능한 범위의 Shared/protocol/Server build와 contract test

Client visual fidelity와 실제 F1 수동 Apply/Play 판정은 사용자가 직접 수행하며 자동 PASS로 기록하지 않는다.
