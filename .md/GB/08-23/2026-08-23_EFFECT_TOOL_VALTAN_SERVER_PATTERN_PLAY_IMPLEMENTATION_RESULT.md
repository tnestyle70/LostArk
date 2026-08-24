# 2026-08-23 Effect Tool Valtan Server Pattern Play 구현 결과

## 완료 상태

Effect Tool `All Effects -> Valtan`의 각 Pattern 행에 `Play Server Pattern`을 추가했다. 이 버튼은
하위 Stage/Product 로컬 미리보기와 분리되어 실제 Server room의 복제 Valtan에 stable pattern ID를
queue하고, `CValtanBrain` fixed tick을 통해 전체 패턴을 실행한다.

```text
Effect Tool Pattern 행
-> PLAY_PATTERN_ID(placementId, patternId)
-> Server room exact boss resolve + boss-only reset
-> PendingPatternIds
-> CValtanBrain fixed tick
-> replicated action / motion / Product cue / Effect
```

## 구현 내용

- Shared protocol version을 32로 올리고 `PLAY_PATTERN_ID`에만 두 bounded stable ID를 조건부
  직렬화했다. 기존 audition operation의 payload 형식은 유지한다.
- NetworkManager는 stable-ID 결과를 기존 Valtan Arena health-bar audition과 다른 deque로
  전달한다. Effect Tool과 Level consumer가 같은 verdict를 경쟁하지 않는다.
- Effect Tool은 request sequence, placement/pattern identity, world inbound generation을 소유한다.
  exact verdict, disconnect, world reset/transfer 중 하나로 pending transaction을 종료한다.
- Character Select는 이미 `Spawn Selected -> Valtan`으로 생성된 private-room Server boss만 사용한다.
  새 local boss나 Model View boss를 만들지 않는다.
- Valtan Arena는 `boss.valtan.center`, Character Select는
  `boss.valtan.character-select.lazy`를 exact resolve한다.
- Character Select boss-only reset은 같은 NetEntityId와 monotonic pattern sequence/BossCombat
  revision을 보존하고 boss source combat object만 취소한다. 플레이어 combat object와 arena
  wall/floor/pillar/navigation/destruction state는 건드리지 않는다.
- Character Select에서 arena 환경을 소유하는 6개 패턴은 fail closed한다.
- room-level stable-ID audition in-flight 소유권으로 동시 요청이 서로 reset/overwrite되지 않게 한다.
- 기존 Stage `Replay Sequence`, `[PRODUCT] Play Saved Effect`, Effect 저작 흐름은 변경하지 않았다.

## 자동 검증

- `Shared/Default/Shared.vcxproj` Debug Build: PASS, warnings 0, errors 0.
- `NetworkProtocolHarness` Debug build/run: PASS, failures 0.
  - 기존 request/result golden payload 9/14 bytes 보존
  - 두 stable ID 각각의 truncated read와 destination atomicity
  - oversize/noncanonical stable ID reject
- `Server/Default/Server.vcxproj` Debug Build: PASS, warnings 0, errors 0.
- Server `--contract-test`: PASS, failures 0.
  - Character Select stable-ID Dash Charge queue/start
  - arena 환경 패턴 reject와 boss state 보존
  - 독립 request sequence ledger
  - 동일 NetEntityId와 monotonic sequence/revision
  - 공유 room 동일 tick 두 session 요청 중 첫 패턴만 PENDING/ACTIVE 유지
- `Client/Default/Client.vcxproj` Debug `ClCompile`: PASS, errors 0. 기존 인코딩 계열
  C4819/C4805 경고는 유지됐으며 새 오류는 없었다.
- 관련 파일 `git diff --check`: PASS.

## 브랜치와 적용 경계

`origin/main@026cc334`에서 분기한 `codex/valtan-server-pattern-play` clean worktree에 A 기능만
구현했다. Dash Charge 3650 ms 테스트 hunk, Effect authoring/trail/source-history 변경과 build가
재생성한 world destruction 문서는 포함하지 않았다. 이 결과 시점에는 commit만 만들며 push/merge와
정본 폴더 반영은 수행하지 않는다. protocol version 32이므로 실제 적용할 때 Server와 Client를 같은
commit으로 함께 빌드해야 한다.

Client 또는 UI는 자동 실행하지 않았고 시각 결과도 자동 PASS로 판정하지 않았다.

## 사용자 육안 검증

자동 visual PASS는 기록하지 않는다. 새 Server + Client 실행 후 다음 순서로 사용자가 확인한다.

1. Character Select에서 `Server arena spawn -> Valtan -> Spawn Selected`.
2. 살아 있는 캐릭터를 발탄 engage 범위 안에 둔다.
3. `F1 -> Effect Tool -> All Effects -> Valtan`에서 원하는 Pattern 행의
   `Play Server Pattern`을 누른다.
4. `VALTAN_DASH_CHARGE`의 WINDUP, CHARGE, RECOVERY와 연결 Product Effect가 실제 월드 발탄을
   따라가는지 확인한다.
5. 하위 WINDUP/CHARGE Stage와 `[PRODUCT]` 버튼은 기존 로컬 Model View 개별 검증에 사용한다.

Valtan Arena의 벽·바닥·기둥 의존 패턴은 Character Select가 아니라 실제 Valtan Arena에서 같은
Pattern 행 버튼으로 확인한다.
