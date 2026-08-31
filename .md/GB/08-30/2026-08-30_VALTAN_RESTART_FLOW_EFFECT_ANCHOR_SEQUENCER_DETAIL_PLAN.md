# 발탄 Tool·Pattern 정본 교정 상세 계획서

## 0. 문서 목적

이 문서는 기능 목록을 늘리는 계획이 아니라, 2026-08-31 감사에서 확인된 정본·배포·rollback
위반을 실제 소비자 기준으로 교정하는 실행 순서를 고정한다. 버튼, ImGui row, source-token test가
존재한다는 사실은 완료 증거가 아니다.

완료 증거는 항상 다음 순서를 만족해야 한다.

```text
authoring owner 저장
→ validator/projector가 generated Product 생성
→ actual Product loader가 canonical graph를 admit
→ 같은 source fingerprint의 표준 Client/Server 산출물
→ 실패/경합에서 기존 owner와 Product byte identity 보존
→ 사용자가 표준 Client 첫 화면과 실제 재생을 직접 관찰
```

## 1. 시작 스냅샷과 보존 경계

- 브랜치: `codex/valtan-restart-effect-sequencer`
- 시작 HEAD: `780fd0bdc21b7f78cb3c0bd0af84396516117827`
- 비교 정본: `origin/main` `21e0b85810c73c8feda6cffbd0ce796581b07cd5`
- 2026-08-31 확인 시 표준 `Client.exe`/`Server.exe` 실행 프로세스 없음
- dirty tree 전체를 한 commit으로 만들거나 범주가 다른 변경을 같이 stage하지 않는다.
- `Data/Effects/Authored/effect.valtan.carrier-v1.attack.portal-rush.portal.clip-01.effect.json`
  16k+ line 변경과 대응 Portal PLAN/RESULT/test는 별도 기능 단위다. 이 교정 작업에서 되돌리거나
  Restart/Sequencer commit에 포함하지 않는다.

### 변경 분류

| 범주 | 대표 파일 | Gate 판정 |
|---|---|---|
| F1/Workbench shell | `MainApp.*`, `Animation_Tool.*`, `CharacterPreviewPanel.cpp` | resize/data-only 방향은 보존하되 표준 Client와 사용자 확인 전 미완료 |
| Source owner/projector/runtime | `BalanceTool.*`, `ValtanPatternTree.*`, `EncounterPatternReference.*`, split/Product JSON | generated Product 직접 Save와 source/Product 이중 runtime부터 교정 |
| Create Pattern | `promote_valtan_animation_chains.py`, Create UI/tests | cross-process generation transaction 전 적용 금지 |
| Restart/Flow/Next | `BossTool.*`, `ValtanPatternAuditionService.*`, `GameRoom.cpp`, Server tests | wire predecessor CAS 전 Restart 완료 주장 금지 |
| Target Effect anchor | `ClientReplication.cpp`, `Valtan.*`, cue data/tests | facing 의미와 stale target failure를 native matrix oracle로 고정할 때까지 미완료 |
| Portal motion | gameplay/Encounter, `GameplayCatalog.*`, `ValtanBrain.cpp` | Data→publisher→Server path는 회수 가치가 있으나 canonical/standard build 뒤 재판정 |
| Portal visual carrier | 거대 authored Effect JSON, 별도 Portal PLAN/RESULT/test | 완전히 별도 검증/commit 단위 |
| Kakul/Level navigation | `MainApp.*`, `Level_CharacterSelect.*` | typed transition과 timeout unlock은 표준 Client에서 사용자가 확인해야 함 |

## 2. 정본 소유권

### 2.1 Authoring owner

```text
Gameplay:     Data/Valtan/Valtan.gameplay.json
Presentation: Data/Valtan/Valtan.presentation.json
Sound:        Data/Animation/Authored/Valtan/Valtan.patternsoundcues.json
Flow:         Data/Encounters/Valtan/ValtanBossAuditionFlows.json
```

### 2.2 Generated Product

```text
Data/Encounters/Valtan/ValtanEncounter.json
Data/Animation/Authored/Valtan/Valtan.patternbindings.json
Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json
```

`patternbindings`와 `patterneffectcues`는 Workbench가 직접 교체하지 않는다. Animation/Effect row를
편집할 때는 stable `patternId/stageId/actionId/occurrenceId`를 이용해 split presentation draft를
수정하고, Balance와 같은 immutable revision pipeline으로 validate/save/project/publish한다.

제품 Arena는 generated Product만 읽는다. split gameplay/presentation loader는 authoring tool과
projector 검증에만 사용한다. Product runtime이 arena-center, binding, cue를 얻기 위해 split source를
다시 읽는 두 번째 해석 경로를 두지 않는다.

## 3. Gate 0 — 회수와 분류

1. 신규 패턴 확장 subtask를 중단한다.
2. 각 dirty hunk를 위 표 범주에 배정한다.
3. generated Product 직접 Save, source/Product 혼용, unsafe transaction은 `교정`으로 분류한다.
4. resize/data-only shell, semantic source, exact authored Effect list, canonical native oracle은 `회수`로
   분류한다.
5. 거대 Portal carrier는 `별도 기능`으로 격리한다.
6. 실제 소비자 없는 placeholder/두 번째 정본은 `폐기`한다.

Gate 0 완료 조건은 분류 문서와 물리 diff가 일치하고, 신규 기능 agent가 추가 변경을 남기지 않는 것이다.

## 4. Gate 1 — 저장·runtime·intake 데이터 보존

### G1-A. Presentation typed authoring

- Workbench의 Animation slot Save는 `CValtanPatternAnimationBindingDocument::Save_Atomic`을 호출하지 않는다.
- generated Effect cue 문서에도 Add/Edit/Remove direct Save를 추가하지 않는다.
- split presentation draft는 current source bytes/SHA와 generation을 소유한다.
- save는 `parse → validate → stage → cross-owner CAS → authoring commit → project → Product admission`
  순서다.
- projection이나 active revision 적용이 실패하면 source와 이전 Product를 byte-identical하게 복원한다.
- `STALE_PRESERVED`는 표시만 허용하고 모든 Save/Create Apply/Complete Play/Restart/서버 mutation을 막는다.

### G1-B. Product-only runtime

- `CValtan`의 binding/effect reload는 projected Product loader만 사용한다.
- arena-center/anchor metadata도 Product 문서 또는 Server snapshot의 stable field에서 얻는다.
- `CValtanPatternTree::Load_FromAuthoringPaths`는 authoring/projector/native parity test 이외 Product Arena에서
  호출하지 않는다.

### G1-C. Create/Intake transaction

- malformed chain/occurrence/unknown property는 row `continue`가 아니라 전체 문서 reject다.
- destination을 먼저 삭제하고 rename하는 fallback을 제거한다.
- repository-wide cross-process lock과 immutable generation manifest를 잡는다.
- commit 전에 모든 baseline을 다시 검사한다.
- stage 디렉터리/새 generation을 완성한 뒤 하나의 pointer 또는 동등한 admission commit만 교체한다.
- lock contention, stale generation, N번째 replace 실패, projector 실패, reload 실패에 기존 bytes가 유지되는
  실제 concurrent-process/injected-failure test를 둔다.

## 5. Gate 2 — EXE/Data skew 방지

`Invoke-BuildAndRegression.ps1`은 선택 Configuration과 `-SkipBuild` 여부와 무관하게 가장 먼저 다음 네
표준 출력의 실행 프로세스를 exact resolved path로 검사한다.

```text
Client/Bin/Debug/Client.exe
Client/Bin/Release/Client.exe
Server/Bin/Debug/Server.exe
Server/Bin/Release/Server.exe
```

검사는 Python gate, domain publisher, shared Data mutation보다 앞선다. 같은 이름이지만 다른 경로인
프로세스는 막지 않는다. 실제 실행 가능한 파일을 임시 fake repository의 표준 경로에서 시작한 fixture로
PID/path reject를 확인하고, fixture 프로세스만 `finally`에서 종료한다.

## 6. Gate 3 — 동일 revision 자동 oracle

순서는 다음과 같다.

1. JSON/XML 전부 parse
2. Valtan split/source validator
3. projection 후 generated Product diff/owner parity
4. `CValtanPatternTree` authoring parity oracle
5. `CEncounterPatternReference` + Product binding/effect loaders의 canonical native graph oracle
6. 실제 `Build_PlayablePatternInventory`, Complete Play/Next inventory, saved Flow admission
7. Debug Product → Debug Core
8. Release Product → Release Core
9. 새 표준 EXE hash/timestamp와 build receipt/source fingerprint 대조

현재 분류 oracle의 기준값은 Product split 33, Encounter/reference 57, Complete Play 33, saved Flow 29다.
이 숫자는 기능 완료가 아니라 reference-only row를 playable로 오인하지 않기 위한 회귀 기준이다.

## 7. Gate 4 — 사용자 첫 화면

표준 Client에 최소 변경만 올린 뒤 사용자가 직접 확인한다.

```text
F1 root window resize/scroll
→ Level Navigation 버튼과 timeout/reject 뒤 unlock
→ semantic Authoring Sources, raw index 기본 닫힘
→ Action Presentation Workbench data-only shell
→ Product Pattern inventory와 Complete Play 선택
→ Effect Tool exact authored Valtan 목록
```

사용자 관찰 전에는 UI `PASS`, visual `PASS`, 완료를 기록하지 않는다.

## 8. Gate 5 — Create New Pattern

`Validate`는 무변경 transaction 전체를 수행하고 exact request+source generation digest를 반환한다.
`Apply`는 동일 digest에만 허용한다. 성공은 다음 모두가 같은 generation일 때만 표시한다.

```text
split gameplay/presentation committed
→ projector generated Encounter/bindings/effect cues
→ publisher/Server activation admitted exact revision
→ Anim Product loader reload
→ Boss canonical inventory reload
→ 새 AUDITION_ONLY pattern selected
```

hit/motion/counter/groggy/prop/silence를 자동 추론하지 않는다.

## 9. Gate 6 — Restart exact occurrence CAS

Restart request는 boss/pattern ID만 보내지 않는다. wire에 최소한 predecessor를 함께 보낸다.

```text
operation = RESTART_PATTERN
roomEpoch
bossNetEntityId 또는 stable placement generation
expectedPatternSequence
expectedPatternRevision
patternId
requestId
```

Server는 exact predecessor가 active이고 같은 session/consumer owner일 때만 replacement를 stage한다.
player combat-ready/pose, boss state, pending Next, Flow, destruction/prop/collision/Nav를 preflight 전에
변경하지 않는다. 어떤 reject/중간 실패에서도 모든 상태가 byte/value-identical해야 한다.

## 10. Gate 7 — 패턴별 재개

Gate 0~6 뒤에만 패턴을 하나씩 별도 수직 슬라이스로 진행한다. 우선순위는 기존 Product closure,
reference promotion, 신규 gameplay 계약 순이다. 각 패턴은 Data→Shared→Server→Client/Effect→native harness와
별도 commit을 가진다.

Six Pizza는 target position과 facing의 의미를 먼저 확정한다. `player yaw` 또는 `Server-locked boss facing`
중 하나만 정본으로 선택하고, data→snapshot→matrix→Effect spawn의 수치 oracle을 둔다.

## 11. Commit 분리

다음 단위를 합치지 않는다.

1. F1/Workbench shell
2. source owner/projector/runtime admission
3. Create Pattern transaction
4. Restart/Flow/Next protocol
5. target Effect anchor
6. portal motion
7. portal visual carrier
8. Kakul/Level navigation

각 commit에는 해당 코드가 소비하는 schema/data, project registration, native/domain test, PLAN/RESULT 정정만
포함한다. 표준 build와 사용자 수동 검증이 남아 있으면 RESULT에 `자동 검증 완료 / 사용자 검증 대기`로
분리한다.
