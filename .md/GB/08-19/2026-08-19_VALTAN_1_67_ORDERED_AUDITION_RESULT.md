# 2026-08-19 발탄 1~67 Server ordered audition 결과

branch: `codex/valtan-arena-destruction-and-pattern-preview`

Debug Server와 Debug Client를 실행하고 Lobby의 기존 `Valtan` 명령으로 입장하면,
local character replication이 완료된 뒤 발탄을 자동 생성하고 1~67 ordered audition을
한 번 시작하도록 연결했다. Server가 실제 `CValtanBrain`과 기존 stage·hit·motion·wall·floor·prop
경로를 구동하며, Client는 authoritative snapshot을 표현하기만 한다.

## G00. 구현 결과

- `Data/Encounters/Valtan/ValtanDebugAudition.json`
  - 정확히 67 occurrence
  - `PRODUCT_DIRECT` 23, `PRODUCT_CANDIDATE` 18, `PRODUCT_PARTIAL` 18,
    `MARKER` 3, `UNRESOLVED` 5
  - 실행 행 59개, #40/#43 repeat를 펼친 실제 pattern start 61회, idle 행 8개
- `Publish-GameplayBalance.ps1`
  - source preview의 1~67 번호·evidence와 strict join
  - product `patternId`, mapping, repeat, pause, HP bar trajectory를 case-sensitive 검증
  - gameplay bootstrap v10에 `VALTANDEBUGSEQUENCE` 1행과
    `VALTANDEBUGSTEP` 67행 publish
- `CGameplayCatalog`
  - audition sequence를 기존 parse -> validate -> stage -> commit/rollback에 포함
  - 정확히 67행, owner encounter·boss·pattern join, occurrence ID와 14->40 회복을 검증
- Shared/Server
  - `PLAY_ORDERED_1_67`, `STOP_ORDERED_1_67` typed operation 추가
  - enum acceptance가 바뀌므로 `NETWORK_PROTOCOL_VERSION 23 -> 24`
  - Debug Server가 fresh boss를 preflight한 뒤 arena를 한 번 reset하고 stable product
    pattern을 한 개씩 queue
  - 실행 중 marker는 bounded idle로 보존하고 weighted random pattern은 차단
  - 다른 player action·skill·projectile과 trigger 진입을 동결해 driver를 격리
  - 84/30 floor, 109 outer wall, repeatable pillar와 기존 destruction/prop hook은 누적 유지
  - duplicate/rejected request sequence도 소비하고 wrap을 modular 비교
  - Stop, owner leave, room empty, failure에서 ordered/pillar 예약을 함께 정리
- Client
  - Debug Valtan Level에서 local character가 생긴 뒤 자동 PLAY를 한 번 제출
  - pending request를 sequence·operation·bar tuple로 소유하고 exact echo만 완료 처리
  - 750ms 간격 최대 3회 bounded retry, zero-skip request sequence wrap
  - Restart는 `STOP 응답 -> PLAY`로 직렬화하고 focused audition과 ordered run의 충돌 차단
  - Release Client에는 auto start가 없고 Release Server는 audition을 명시 거부

`Valtan.patternpreview.json`과 `Valtan.clipseq`는 계속 Animation Tool 검토 자료다.
Server는 clip 이름, `sourceActionId`, `sequenceIndex`를 runtime 실행 ID로 읽지 않는다.

## G01. 자동 검증

```text
Gameplay balance Validate/Publish
  PASS: 6 player profiles / 132 skills / 108 damage profiles
        1 boss / 33 boss patterns / 124 stages / 67 audition occurrences

Generated Gameplay.bootstrap
  PASS: version 10 / rows 674
        VALTANDEBUGSEQUENCE 1 / VALTANDEBUGSTEP 67

NetworkProtocolHarness Debug/Release
  PASS: protocol 24 / failures: 0

Shared + Client Debug/Release build
  PASS

Server Debug/Release build
  PASS

Server Debug ordered contract
  PASS: fresh auto activation, preflight rollback, request/retry/wrap,
        active-operation isolation, repeat/pause, pillar deadline wrap
  PASS: exact 67 rows -> 61 stable starts + 8 idle rows
  PASS: #40/#43 each x2, #55 14 bars -> #56 40 bars
  PASS: pillar x3 INTACT -> BREAKING -> HIDDEN
  PASS: floor84 rails 2 -> floor30 bricks 4
  PASS: COMPLETED_HOLD, 30 further idle ticks, owner-leave reset

Server Release contract
  PASS: reject every Valtan audition without moving the boss

ValtanDebugAudition JSON / Client project XML / git diff --check
  PASS
```

전체 `Server.exe --contract-test`의 process exit는 기존 별도 낙사 작업 때문에 아직 1이다.
이번 ordered assertion 20개는 모두 PASS했고, 남은 6 failure는 기존 dirty slice의
낙사 region 4개와 strict first-slice world-destruction fixture 2개다. 이 작업에서 해당
미커밋 변경을 되돌리거나 섞어 고치지 않았다.

## G02. 사용자 실행 경로

1. Visual Studio에서 x64 `Debug`의 `Server + Client` profile을 실행한다.
2. Lobby에서 기존 `Valtan` 버튼을 누른다.
3. 발탄 맵에 local character가 생성되면 1번부터 자동으로 시작한다.
4. Debug Developer Tools는 `F1`로 열고, 필요하면
   `Restart 1-67 Ordered Audition` 또는 `Stop Ordered Audition`을 누른다.

자동 contract 기준 전체 실행은 약 7,987 Server tick, 즉 약 4분 26초다. 원본 영상의
긴 대기 구간을 복제한 시간이 아니라 현재 product stage duration과 occurrence pause를
사용한 검토용 실행 시간이다.

Client/UI는 에이전트가 실행하지 않았다. 실제 화면에서 animation, camera, 파편, 벽 접촉과
원본 영상 일치 여부는 사용자가 직접 판정한다.

## G03. 남은 정확도 경계

- `PRODUCT_CANDIDATE` 18개와 `PRODUCT_PARTIAL` 18개는 원본 영상과의 exact mapping이 아니다.
- #1 첫 회오리와 #2/#5/#10/#18/#24의 손 3회+단발 dash는 특히 사용자 영상 대조가 필요하다.
- `MARKER` 3개와 `UNRESOLVED` 5개는 다른 공격으로 추측 대체하지 않고 idle로 보존했다.
- 자동 검증은 순서·repeat·Server pattern·누적 환경 상태를 보장하지만 frame, camera, sky,
  debris fidelity를 보장하지 않는다.
- 기존 dirty worktree에 다른 낙사 작업과 다수 미추적 파일이 있어 stage/commit/push는 하지 않았다.
