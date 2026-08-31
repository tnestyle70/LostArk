# Encounter Sequencer Graph v2 구현 결과

## 결론

기존 29개 발탄 Boss Tool Flow occurrence의 stable identity와 순서를 그대로 보존하면서,
선형 `slots[]` 문서를 `entryNodeId + nodes[] + edges[]` graph v2로 승격했다. 이 변경은
Encounter Sequencer의 저장 기반만 닫는다. Product/Server의 graph cursor와 실제 node editor는
다음 G02/G03 변경에서 연결한다.

현재 Product runtime은 기존 선형 재생을 유지한다. v2 graph가 선형·watchdog 없음·동일 pursuit
조건을 만족할 때만 compatibility projection을 만들며, finite repeat, node watchdog, per-edge
pursuit를 사용하면 `RUNTIME_PROJECTION_UNSUPPORTED`로 명시적으로 publish를 막는다. 따라서
새 저작 기능이 아직 없는 Server에서 조용히 다른 순서로 재생되지 않는다.

## 실제 반영 범위

### 1. Strict graph v2 문서

- root schema와 formatVersion 2를 strict parse한다.
- Flow는 `entryNodeId`, stable node/edge ID, default pursuit, transition watchdog을 소유한다.
- node는 `patternId`와 optional watchdog을, edge는 `COMPLETED`, target, pursuit와 optional
  `maxTraversals`를 소유한다.
- dangling reference, duplicate/non-deterministic edge, unreachable node, unbounded cycle,
  forward-edge cap, premature transition watchdog을 거부한다.
- cycle은 정확히 cycle-closing back-edge 하나만 finite cap을 가질 수 있고, cap 소진 시 source
  node에서 terminal hold하는 의미로 고정했다.

### 2. v1 identity 보존

- 이전 29개 `(slotId, patternId)` pair와 배열 순서를 exact fixture로 고정했다.
- legacy `.slot.000001`과 신규 `.node.000001`이 ordinal을 재사용하지 못한다.
- 현재 `nextNodeOrdinal=41`, `nextEdgeOrdinal=29`, 기본 pursuit 1000 ms를 보존한다.
- 기존 Boss Tool의 Add/Move/Remove는 선형 compatibility graph에서만 stage → validate → commit한다.

### 3. Save CAS와 공용 writer admission

- raw source bytes의 SHA-256 revision을 baseline으로 사용한다.
- Save는 기존 Create/Project와 같은
  `out/ValtanPatternTransactions/create-pattern.lock` byte 0을 배타적으로 잡는다.
- lock은 최초 disk revision read부터 temporary round-trip, atomic replace, post-verify와 rollback까지
  유지한다. 다른 writer가 active면 source bytes를 바꾸지 않고 dirty draft를 보존한다.
- durable temp, recovery backup, pre-commit CAS와 post-replace reparse 검증을 유지한다.

### 4. Python/C++ admission parity와 비용 제한

- 양쪽 모두 node/edge 최대 255, JSON 256 KiB, depth 8, transition 최대 4096을 사용한다.
- Python은 recursive JSON decoder 전에 string/escape-aware depth scan을 수행하고
  `RecursionError`를 typed `PipelineError`로 정규화한다.
- node watchdog이 켜진 graph는 C++에서도 legacy Slots projection을 노출하지 않는다.
- 프레임 update/render 경로에 filesystem scan이나 JSON parse를 추가하지 않았다.

## 자동 검증 결과

- focused Flow contract: 30/30 PASS
- Valtan Pattern Master V2: 74/74 PASS, 358.684초
- 최신 추가 경계 targeted:
  - valid finite graph의 `RUNTIME_PROJECTION_UNSUPPORTED`: PASS
  - depth 8 reject/invalid saved Flow: PASS
  - 29개 exact migration identity: PASS
- `Project-ValtanPatternMaster.ps1 -Mode ValidateV2`: PASS
  - managed 33, legacy 26, projected artifacts 9
- `ValtanPatternAuditionServiceHarness`: PASS
  - Flow service 13/13
  - tuning command 14/14
  - canonical graph 5/5
  - Action Composition graph 9/9
  - Effect authoring 9/9
  - presentation generation admission PASS
- incremental Debug native harness build: error 0
- `git diff --check`: PASS

독립 비평의 최종 판정은 P0 없음, P1 없음, P2 코드 결함 없음이다. 초기 P1이었던 concurrent
writer lost-update/rollback overwrite와 JSON depth parity, runtime projection error typing,
migration exact identity, watchdog projection parity를 같은 변경에서 수정하고 재검토했다.

## 남은 경계

- Product와 Debug primary Valtan이 같은 catalog graph cursor를 실행하는 G02는 아직 미구현이다.
- node Add/Duplicate/Connect/Set Entry/Delete/finite Repeat UI는 G03에서 구현한다.
- finite repeat의 실제 Server trace와 terminal occurrence oracle은 G02 harness가 소유한다.
- temp/backup의 모든 OS failure point를 주입하는 native harness와 C++/Python validator golden
  differential corpus는 후속 테스트 부채다.

이 결과만으로 화면상 Encounter Sequencer가 완성된 것은 아니다. 다음 변경은 사용자 우선순위에
따라 Arena Sequence Preview, 도끼/Warp 실튜닝, `Save & Apply`, graph runtime/UI 순으로 진행한다.
