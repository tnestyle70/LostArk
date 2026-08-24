# 발탄 전투 순서 선택 재생 계획

## 1. 목표와 종료 증거

기존 `Ordered 1-67` 전체 자동 재생을 제품 Debug UI에서 제거한다. 사용자가 확정한 발탄 전투 흐름을 체력 패턴과 일반 패턴이 섞인 하나의 시간순 목록으로 저작하고, stable row ID로 한 행을 선택한 뒤 `Play Selected`를 누르면 Server가 그 시점의 아레나·비석·체력 상태를 준비한 후 해당 occurrence 하나만 실제 `CValtanBrain` 경로로 실행한다.

완료 증거는 다음과 같다.

- 160 → 일반 패턴 → 130 → 일반 패턴 → 109 → 100 → 84 → 73 → 62 → 30/29/28 → 14 순서가 하나의 스크롤 목록에 표시된다.
- 같은 일반 패턴의 반복 occurrence도 서로 다른 stable row ID로 선택할 수 있다.
- 109 직전에는 일반 벽만 사라지고 외곽 30개는 남으며, 109 이후에는 모든 벽이 사라진다.
- 84 이후에는 첫 바닥 절반, 30 이후에는 나머지 바닥 절반까지 사라진 상태로 선택 패턴이 시작한다.
- 붉은 검기 occurrence는 네 비석이 먼저 올라온 상태에서 시작해 제품 prop-break 단계가 두 개씩 파괴한다.
- repeat 2 occurrence는 한 번의 선택으로 같은 제품 패턴을 정확히 두 번 실행하고 그 뒤 hold한다.
- malformed/duplicate/unknown row, 잘못된 arena/prop precondition, protocol zero command ID를 validator와 harness가 거부한다.

## 2. 데이터 정본

`Data/Encounters/Valtan/ValtanDebugAudition.json`의 기존 67-slot 영상 ledger를 `lostark.valtan-pattern-timeline` 문서로 교체한다. 문서는 다음 필드를 소유한다.

- `timelineId`: 전투 순서 정본 ID
- `rowId`: 저장·ImGui 선택에 쓰는 stable row ID
- `commandId`: `rowId`의 UTF-8 FNV-1a32 값이며 wire에서 쓰는 non-zero stable ID
- `ordinal`: 화면 순서
- `sectionHealthBar`: 해당 occurrence가 속한 전투 줄수
- `entryType`: `MECHANIC` 또는 `NORMAL`
- `patterns`: 실제 제품 `patternId`와 선택 한 번에 실행할 `repeat`의 배열
- `arenaState`: `FRESH`, `ORDINARY_WALLS_GONE`, `ALL_WALLS_GONE`, `FLOOR84_GONE`, `FLOOR84_AND_30_GONE`
- `propState`: `HIDDEN` 또는 `FOUR_PILLARS_INTACT`
- `displayLabel`: 사용자가 확인할 한국어 occurrence 설명

Publisher는 Encounter pattern ID와 줄수 범위, ordinal 연속성, health trajectory, arena state의 단조 전이, prop state, stable row ID와 command ID 재계산·충돌을 검증한 뒤 Server bootstrap에 label을 제외한 실행 계약을 publish한다. Client Debug UI는 같은 JSON을 strict parse하여 label을 표시하고, Server에는 배열 위치가 아니라 stable command ID를 보낸다.

## 3. Shared 계약

기존 wire 위치를 유지하면서 `PLAY_ORDERED_1_67`/`STOP_ORDERED_1_67` 의미를 `PLAY_TIMELINE_ROW`/`STOP_TIMELINE_ROW`로 교체한다. `PLAY_TIMELINE_ROW`는 기존 request의 non-zero payload를 stable command ID로 사용하며 결과가 같은 값을 echo한다. `STOP_TIMELINE_ROW`는 zero payload만 허용한다. NetworkProtocolHarness는 정상 round-trip, zero command ID, stop-with-ID, unknown operation과 untouched-on-failure를 검증한다.

## 4. Server 권위 실행

`CGameplayCatalog`은 publisher bootstrap을 parse → validate → stage → commit하여 timeline row를 encounter별로 소유한다.

`CGameRoom`은 선택 요청을 받으면 다음 순서로 한 transaction을 준비한다.

1. session, Valtan world, live boss와 selected row를 검증한다.
2. 기존 active timeline audition을 중지하고 boss/world/props를 reset한다.
3. row의 `arenaState`에 맞는 ordinary wall, 109 outer wall, 84 floor, 30 floor 선행 mutation을 하나의 staged world transaction으로 만든다.
4. `FOUR_PILLARS_INTACT`면 새 prop occurrence를 stage한다.
5. row의 `sectionHealthBar`로 HP와 last evaluated bar를 맞추고 player를 authored bait에 둔다.
6. `PendingPatternIds`에 stable `patternId`를 넣고 scripted playback으로 random/rotation을 막는다.
7. 실제 Brain start/finish edge를 관찰해 `repeat`만큼 실행한 뒤 completed hold로 전이한다.

중간 준비가 실패하면 reset 이후 부분 상태를 commit하지 않고 요청을 거부한다. 실제 pattern stage가 시작된 뒤에는 기존 world destruction, cinematic camera, sky cue, encounter prop action을 그대로 소비한다.

## 5. Client UI

`CLevel_ValtanArena` 상단의 기존 1~67 자동 재생 버튼을 제거한다. 같은 위치에 높이가 고정된 child list를 두고, 각 행은 `rowId`를 ImGui ID로 사용한다. 체력 패턴은 줄수와 함께 강조하고 일반 패턴은 같은 section 안에 그대로 이어 붙인다.

사용 흐름은 다음 하나다.

`행 선택 → Play Selected Fight Row → request pending → Server verdict → 실제 replicated pattern 관찰`

목록 선택은 pending 중에도 허용하지만 Play만 비활성화한다. JSON load가 실패하면 기존 목록을 유지하지 않고 빈 staged 결과와 구체적인 오류를 표시하며 socket을 직접 호출하지 않는다. 실제 제출은 기존 `CNetworkManager::Send_ValtanAudition` typed 경계를 사용한다.

## 6. 수정 파일과 검증

- Data: `Data/Encounters/Valtan/ValtanDebugAudition.json`
- Publisher: `Tools/GameplayPipeline/Publish-GameplayBalance.ps1`
- Generated runtime: `Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap`
- Server catalog: `Server/Public/GameplayCatalog.h`, `Server/Private/GameplayCatalog.cpp`
- Shared wire: `Shared/Public/Network/PacketMessages.h`, `Shared/Private/Network/PacketMessages.cpp`
- Server authority: `Server/Public/GameRoom.h`, `Server/Private/GameRoom.cpp`
- Client consumer: `Client/Public/Level_ValtanArena.h`, `Client/Private/Level_ValtanArena.cpp`
- Harness: `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp`, `Server/Private/ServerGameplayContractTests.cpp`
- 결과: `.md/GB/08-24/2026-08-24_VALTAN_CHRONOLOGICAL_AUDITION_RESULT.md`

검증 순서는 Gameplay Balance Validate/Publish, Shared + NetworkProtocolHarness Debug/Release, Server Debug/Release + contract-test, Client Debug/Release, JSON parse, focused source identity 검사, `git diff --check`다. Client는 에이전트가 실행하지 않으며 사용자가 `Server + Client → Lobby → Valtan → F1`에서 최종 화면과 패턴 충실도를 판정한다.
