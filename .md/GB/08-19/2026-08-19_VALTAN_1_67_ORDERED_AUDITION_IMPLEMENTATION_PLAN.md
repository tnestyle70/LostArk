# 2026-08-19 발탄 1~67 Server ordered audition 구현 계획

branch: `codex/valtan-arena-destruction-and-pattern-preview`

로비의 기존 `Valtan` 입장 명령은 유지하고, Debug Client가 발탄 레벨의
local character replication을 확인한 첫 frame에 Server에 1~67 ordered audition을 한 번
요청한다. Server는 발탄을 자동 생성하고 arena를 한 번만 reset한 뒤, 각
occurrence의 stable product `patternId`를 `CValtanBrain` 큐에 하나씩 넘긴다.

## G00. 신뢰 경계와 완료 표현

- `Valtan.patternpreview.json`/`Valtan.clipseq`는 계속 Animation Tool 검토용이다. Server가
  clip 이름, `sourceActionId`, `sequenceIndex`를 runtime ID로 읽지 않는다.
- 새 `ValtanDebugAudition.json`은 정확히 67행의 occurrence 순서와 stable
  product `patternId`, 반복, HP bar, bounded pause만 소유한다.
- 현재 자료로 원본 일치가 확정되지 않은 행은 `PRODUCT_CANDIDATE` 또는
  `PRODUCT_PARTIAL`로 남긴다. 애니메이션이 빈 `NO_ANIMATION`/`UNRESOLVED`
  행은 다른 공격으로 채우지 않고 bounded idle step으로 순서만 보존한다.
- 자동화는 순서, Server pattern 실행, 종료, 누적 world state를 검증하지만
  원본 영상 frame, camera, sky, 파편 fidelity를 PASS로 승격하지 않는다.
- Release Server는 해당 요청을 기존처럼 명시적으로 거부하고 Release Client는
  자동 요청하지 않는다.

## G01. Data -> publisher -> runtime catalog

- `Data/Encounters/Valtan/ValtanDebugAudition.json`
  - root exact fields: `schema`, `formatVersion`, `authority`, `encounterId`,
    `sequenceId`, `sourcePreview`, `steps`
  - step exact fields: `occurrenceId`, `ordinal`, `mapping`, `patternId`, `repeat`,
    `targetHealthBar`, `pauseAfterMs`
  - `ordinal` 1~67 contiguous, `occurrenceId` unique, mapping enum strict validation
  - executable mapping의 `patternId`는 `ValtanEncounter.json`과 strict join
  - marker/unresolved는 empty pattern + repeat 0, 실행 행은 repeat 1~4
- `Publish-GameplayBalance.ps1`가 위 문서와 source preview의 1~67 번호를 교차
  검증하고 `VALTANDEBUGSEQUENCE` 1행과 `VALTANDEBUGSTEP` 67행을 publish한다.
- `Gameplay.bootstrap` format을 9 -> 10으로 올린다.
- `CGameplayCatalog` load는 audition map을 기존 parse -> validate -> stage -> commit rollback에
  포함하고 encounter ID로 read-only 67-step view를 제공한다.
- Git 관리 `Data` 원본은 `Client.vcxproj`/`.filters`의 `96.DataFiles`에 `None`으로만
  등록한다.

## G02. Shared Debug command

- 기존 payload를 바꾸지 않고 `VALTAN_AUDITION_OPERATION`에 append-only
  `PLAY_ORDERED_1_67`, `STOP_ORDERED_1_67`을 추가한다.
- 두 operation은 `iTargetHealthBar == 0`인 barless command로만 encode/decode한다.
- 중복 start는 running sequence를 reset하지 않고 `REJECTED_PATTERN_UNAVAILABLE`로 답한다.
- wire payload byte 형식은 그대로지만 기존 protocol 23의 `END=8` 자리에 새 operation을
  추가하면 구 Server가 같은 version handshake 뒤 packet을 거부한다. 따라서 protocol을
  `23 -> 24`로 올리고 enum boundary와 bar policy를 NetworkProtocolHarness로 고정한다.

## G03. Server fixed-tick ordered driver

- `Evaluate_ValtanAudition`의 ordered start는 세션/world/catalog 67행을 먼저 검증한다.
  검증 전에는 boss/world/player를 변경하지 않는다.
- boss가 없으면 기존 disabled placement `boss.valtan.center`를 Server가 활성화한다.
- start commit은 `Reset_ValtanAuditionState()`를 한 번만 호출하고 owner player를
  authored audition bait에 배치한다. 기존 낙사 작업의 `FALLING` cleanup을 보존한다.
- brain 직전 driver가 idle step을 소비하거나 stable pattern ID 하나를
  `PendingPatternIds`에 넣는다. pattern start sequence를 실제로 관찰한 후 idle로
  돌아와야만 repeat/다음 occurrence로 전진한다.
- marker/pause/completed hold 중에는 `CValtanBrain::Update()`를 호출하지 않아 random
  weighted pattern이 사이에 끼어들지 못하게 한다.
- target health bar를 적용할 때 HP와 `iLastEvaluatedHealthBar`를 같이 stage해
  중간 threshold가 자동 queue되지 않게 한다. 14줄 후 40줄 회복은 명시적
  occurrence bar이므로 허용한다.
- run 중 owner의 HP/사망/낙사를 Debug driver가 복구하고 boss HP를 step staged
  값으로 유지해 중간 사망으로 sequence가 끝나지 않게 한다. damage event는 기존
  Server path가 생성하며 시각 타격 표현을 막지 않는다.
- owner leave와 room-empty reset은 driver를 cancel한다. 완료 후에는 최종 arena를
  보기 위해 idle hold하고 explicit Restart/Stop 전에 random AI를 재개하지 않는다.

## G04. Client auto-start와 사용자 검증

- `_DEBUG` `CLevel_ValtanArena::Update()`에서 replication update와 local character 생성을
  확인한 후 ordered start를 한 번만 제출한다.
- Debug panel에 `Restart 1-67 Ordered Audition`, `Stop Ordered Audition`을 추가한다.
- Client는 pending 요청을 sequence·operation·bar 전체 tuple로 보관하고, 같은 tuple만
  완료 응답으로 소비한다. 응답 유실은 bounded retry하며 Restart는 STOP 응답 뒤 PLAY를
  직렬 제출한다.
- Client는 pattern/clip을 스스로 선택하지 않고 기존 authoritative snapshot의
  `patternId/actionId/stageIndex/patternSequence`를 표현한다.
- 사용자 실행 경로는 `Server Debug` 실행 -> `Client Debug` 실행 -> Lobby의
  `Valtan` -> 레벨 로드 후 자동 시작이다. Client/UI 조작과 visual PASS는 사용자가
  직접 판정한다.

## G05. 검증

- JSON exact property/count/order/enum/pattern join/invalid repeat/bar/pause validation
- gameplay publisher `Validate`/`Publish`, generated bootstrap header 10 and 67 rows
- catalog normal load + malformed/duplicate/missing pattern rollback contract
- NetworkProtocolHarness Debug/Release ordered start/stop round-trip and barred request rejection
- Server contract:
  - fresh room boss auto activation
  - reset once, 67 occurrence·61 pattern start·8 idle marker의 exact order,
    #40/#43 repeat, random pattern exclusion
  - health staging and existing wall/floor/prop/camera stage path consumption
  - duplicate start rejection, stop, owner leave, completed hold
  - Release request rejection without mutation
- Client Debug/Release build; Debug auto request is one-shot and Release path is absent
- scoped `git diff --check`, JSON/XML parse, existing fall-death dirty hunks preservation audit
- Client 화면의 1~67 원본 영상 일치와 후보 행 승격은 사용자가 직접 수동 검증
