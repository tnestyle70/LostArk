# 발탄 패턴 툴 페이지 시작 계획

## 목표

발탄 아레나 Debug 패턴 툴에 `1페이지`, `2페이지`, `3페이지`, `망령화` 버튼을
추가한다. 버튼은 한 패턴만 재생하는 기존 timeline row audition과 달리 해당 전투
구간의 첫 기믹부터 정상 Server Brain을 계속 실행한다.

## 페이지 기준

| 버튼 | 시작 timeline row | 시작 줄 | 시작 환경 | 시작 gameplay phase |
|---|---|---:|---|---:|
| 1페이지 | `valtan.timeline.160-entrance-whirlwind` | 160 | FRESH | 1 |
| 2페이지 | `valtan.timeline.109-arena-break` | 109 | ORDINARY_WALLS_GONE | 1, 109 IMPACT에서 2로 전환 |
| 3페이지 | `valtan.timeline.62-center-grab-counter` | 62 | FLOOR84_GONE | 2 |
| 망령화 | `valtan.timeline.14-ghost-transition` | 14 | FLOOR84_AND_30_GONE | 2 |

## 구현 계약

1. Shared audition operation에 `START_FIGHT_PAGE`를 추가한다. payload는 vector index나
   임의 page 번호가 아니라 기존 timeline row의 stable command ID다.
2. Server는 위 네 stable row만 허용하고 row가 가진 health bar와 arena state를 사용한다.
3. page보다 위에 있는 health-bar mechanic은 이미 완료된 것으로 표시하고 page 첫 mechanic은
   아직 발동하지 않은 상태로 둔다.
4. 3페이지와 망령화는 필요한 wall/floor destruction을 Server transaction으로 구성하고,
   DESPAWNED commit이 끝날 때까지 Brain을 보류한다.
5. 환경이 준비되면 scripted audition을 해제하고 정상 Brain rotation을 계속 실행한다.
6. 플레이어는 부활·자원 복구·bait 배치 후 즉시 전투 참여 상태가 된다.
7. 기존 선택 row의 `COMPLETED_HOLD`, 단일 패턴 audition과 정상 제품 레이드는 변경하지 않는다.
8. chronological fight row는 선택과 재생을 분리하지 않는다. 사용자가 row를 한 번 클릭하면
   해당 stable command ID의 `PLAY_TIMELINE_ROW`를 즉시 제출하고, row에 저작된 pattern/repeat
   묶음 전체를 기존 Server timeline audition으로 실행한다.
9. C++ source execution code page와 무관하게 페이지 버튼 한글이 ImGui UTF-8로 표시되도록
   버튼 label은 명시적 UTF-8 byte sequence를 사용한다.

## 변경 파일

- `Shared/Public/Network/PacketMessages.h`
- `Shared/Private/Network/PacketMessages.cpp`
- `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp`
- `Server/Public/GameRoom.h`
- `Server/Private/GameRoom.cpp`
- `Server/Private/ServerGameplayContractTests.cpp`
- `Client/Public/Level_ValtanArena.h`
- `Client/Private/Level_ValtanArena.cpp`
- 대응 RESULT 문서

## 검증

1. Shared/NetworkProtocolHarness x64 Debug 빌드와 실행
2. Server x64 Debug 빌드와 page start 회귀 계약 실행
3. Client x64 Debug 빌드
4. `git diff --check`
5. 사용자가 `Server + Client`로 각 버튼의 첫 기믹·아레나 상태·이후 연속 패턴을 직접 확인
