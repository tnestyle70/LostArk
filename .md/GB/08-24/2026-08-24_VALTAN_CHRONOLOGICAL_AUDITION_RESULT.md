# 발탄 전투 순서 선택 재생 결과

## 1. 구현 상태

기존 `Ordered 1-67` 전체 자동 재생 계약을 제거하고, 체력 기믹과 그 사이의 일반 패턴을 함께 보여 주는 52행 전투 순서 선택 재생으로 교체했다.

- 순서: 160 → 130 → 109 → 100 → 84 → 73 → 62 → 30 → 29 → 28 → 14
- 각 일반 패턴은 해당 체력 구간 안에 실제 진행 순서대로 배치했다.
- 반복은 한 행의 `repeat`, 연속 기믹은 한 행의 `patterns` 배열로 저작했다.
- Client는 한국어 `displayLabel`을 표시하고 Server에는 순번이 아니라 `rowId`에서 계산·검증한 stable `commandId`를 보낸다.
- Server는 선택 행마다 벽·바닥·비석 상태를 먼저 준비하고 실제 `CValtanBrain` 제품 패턴을 실행한다.

## 2. 데이터와 통신 계약

`Data/Encounters/Valtan/ValtanDebugAudition.json`은 `lostark.valtan-pattern-timeline` formatVersion 1 정본이다. Publisher는 exact 52행, 연속 ordinal, 의미 기반 stable row ID, UTF-8 FNV-1a32 command ID 재계산·중복, 내림차순 체력 구간, 단조 arena state, encounter pattern join, action repeat를 검증한다.

생성 bootstrap 실측:

- `VALTANTIMELINE`: 1행
- `VALTANTIMELINEROW`: 52행
- `VALTANTIMELINEPATTERN`: 56행
- 기존 `VALTANDEBUGSEQUENCE` / `VALTANDEBUGSTEP`: 0행

기존 wire ordinal 8/9는 각각 `PLAY_TIMELINE_ROW`와 `STOP_TIMELINE_ROW`로 이름과 의미를 교체했다. PLAY는 non-zero stable command ID, STOP은 zero payload만 허용한다. `PLAY_PATTERN`의 wire ordinal 10은 유지했다.

## 3. Client 화면

`Valtan Pattern Audition` 상단의 1~67 자동 재생 버튼을 제거했다. 같은 위치에 체력 구간별 구분선, `MECHANIC` 강조, 일반 패턴 행, 현재 선택 행, `Play Selected Fight Row`, `Stop Fight Row`를 배치했다. 목록은 패널의 전체 가용 폭을 사용하고 긴 한국어 설명을 행 안에서 줄바꿈한다.

Client loader는 JSON을 parse → exact validate → stage → commit한다. 실패하면 요청을 보내지 않고 패널에 오류를 표시한다. ImGui 선택 ID는 vector index가 아니라 `rowId`다.

## 4. Server 권위 실행

Server catalog는 publisher bootstrap의 timeline header/row/action을 입력 순서와 무관하게 staging하고 command ID·row ID·action join을 검증한다. `CGameRoom`은 command ID로 선택 row를 resolve하고 preflight한 뒤 boss, destruction, collision/navigation, encounter props, player bait를 준비하며, 패턴 start/finish edge를 관찰해 action과 repeat를 순서대로 진행한다.

선행 환경은 다음과 같다.

- `FRESH`: 초기 아레나
- `ORDINARY_WALLS_GONE`: 109 직전 일반 벽만 제거, 외곽 109 벽 유지
- `ALL_WALLS_GONE`: 109 이후 모든 벽 제거
- `FLOOR84_GONE`: 84 붕괴까지 반영
- `FLOOR84_AND_30_GONE`: 30 붕괴까지 반영
- `FOUR_PILLARS_INTACT`: 선택 패턴 전에 비석 4개 생성

## 5. 자동 검증

- Gameplay Balance `Validate`: PASS, 52 timeline rows
- Gameplay Balance `Publish`: PASS, 52 timeline rows
- Shared Debug/Release build: PASS
- NetworkProtocolHarness Debug/Release build: PASS
- 새 timeline wire 검증 6건: PASS
- Client x64 Debug build: PASS
- Server 및 전체 contract 결과: 작업 종료 시 최종 기록
- `git diff --check`: 작업 종료 시 최종 기록

전체 NetworkProtocolHarness에는 이번 변경과 무관하게 `NETWORK_PROTOCOL_VERSION`이 32인데 기존 world-destruction 테스트가 31 literal을 요구하는 선행 실패 1건이 남아 있다. 새 timeline wire 검증 자체는 Debug/Release 모두 통과했다.

## 6. 수동 검증 경계

에이전트는 Client를 실행하거나 화면을 대신 판정하지 않았다. 사용자가 `Server + Client`를 시작하고 `Lobby → Valtan → Valtan Pattern Audition`에서 목록 표시와 각 선택 행의 시각·패턴 충실도를 확인해야 한다.
