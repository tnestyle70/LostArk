# 발탄 패턴 툴 페이지 시작 결과

## 완료 상태

발탄 아레나 Debug 패턴 툴에 다음 네 버튼을 추가했다.

| 버튼 | 실제 시작 경계 | 시작 환경 | 시작 phase |
|---|---:|---|---:|
| `1페이지 시작 (160줄)` | `VALTAN_ENTRANCE_WHIRLWIND` | FRESH | 1 |
| `2페이지 시작 (109줄)` | `VALTAN_ARENA_BREAK_109` | ORDINARY_WALLS_GONE | 1, 109 IMPACT에서 2로 전환 |
| `3페이지 시작 (62줄)` | `VALTAN_CENTER_GRAB_COUNTER_64` | FLOOR84_GONE | 2 |
| `망령화 시작 (14줄)` | `VALTAN_GHOST_TRANSITION_15` | FLOOR84_AND_30_GONE | 2 |

이 버튼은 기존의 한 줄 timeline audition과 다르다. 선택한 페이지의 첫 기믹을 정상
Server Brain이 발동한 뒤, 이후 체력 기믹과 일반 rotation을 중단하지 않고 계속 실행한다.

## 구현 내용

- Shared protocol에 `START_FIGHT_PAGE` operation을 기존 ordinal 뒤에 추가했다.
- payload는 임의 페이지 번호나 vector index가 아니라 timeline row의 stable command ID다.
- Server는 위 네 mechanic row만 page boundary로 허용한다.
- 선택한 줄보다 앞선 health-bar mechanic은 완료 occurrence로 설치하고, 선택한 첫 mechanic은
  미완료로 남겨 다음 Brain tick에서 실제 crossing 경로로 시작한다.
- row가 요구하는 벽·바닥 파괴 상태를 기존 world-destruction transaction으로 구성한다.
- 필요한 그룹이 `DESPAWNED`가 될 때까지만 Brain을 보류하고, 준비가 끝나면 scripted 상태를
  해제해 정상 전투를 이어간다.
- 연결된 파티원 전원을 부활·HP/자원 복구·저작 bait 위치 배치 후 즉시 combat-ready로 만든다.
- 페이지 재선택은 이전 단일 row audition을 안전하게 대체하지만, 임의의 일반 timeline row는
  page start로 사용할 수 없도록 거부한다.
- chronological 목록의 각 row를 한 번 클릭하면 선택만 바꾸는 데서 끝나지 않고 기존
  `PLAY_TIMELINE_ROW`를 즉시 제출한다. 한 row에 pattern이 여러 개이거나 repeat가 있으면 그
  저작 묶음 전체가 순서대로 실행된다.
- 기존 아래쪽 재생 버튼은 선택 row를 다시 확인할 수 있는 `Replay Selected Fight Row`로
  유지했다.
- 페이지 버튼 한글은 locale 의존 source literal 대신 명시적 UTF-8 byte sequence로 바꿔
  ImGui에서 물음표로 깨지지 않게 했다.

## 자동 검증

- Shared x64 Debug 빌드: PASS
- NetworkProtocolHarness x64 Debug 빌드 및 실행: PASS, failures 0
- Server x64 Debug 빌드: PASS
- 네 페이지 시작 계약: PASS
  - 정확한 health bar와 초기 phase
  - 이전 health mechanic 완료 ledger
  - 각 페이지 첫 mechanic 발동
  - 환경 준비 뒤 normal Brain release
  - isolated timeline audition 비활성
- 일반 130줄 timeline row를 page start로 요청하는 실패 계약: PASS
- Client x64 Debug 빌드 및 링크: PASS
- chronological row one-click 실행 연결 후 Client x64 Debug 재빌드 및 링크: PASS
- Valtan gameplay/world-destruction publisher: Server build 중 PASS
- `git diff --check`: PASS

전체 Server contract test에는 이번 변경과 무관하게 기존
`Load Valtan's two authored armour plates from the gameplay bootstrap` 한 건이 계속 실패한다.
새 page-start 계약과 protocol 계약은 모두 통과했다.

## 사용자 화면 확인

에이전트는 Client를 실행하거나 화면 결과를 대신 판정하지 않았다. Visual Studio의
`Server + Client` 프로필을 `Ctrl+F5`로 실행하고, 로비에서 발탄 입장 후 F1 Debug Developer
Tools의 `Valtan Pattern Audition > Fight page start`에서 네 버튼을 각각 눌러 확인한다.

확인 기준은 버튼의 첫 기믹이 즉시 시작되고, 그 뒤 발탄이 IDLE이나 단일-row hold에
고정되지 않은 채 다음 일반 패턴과 이후 체력 기믹을 계속 실행하는 것이다.
