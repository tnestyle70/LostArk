# 2026-08-23 Effect Tool Valtan Server Pattern Play 구현 계획

## 목표

Effect Tool `All Effects -> Valtan`의 각 Pattern 행에서 전체 패턴을 실제 Server fixed tick 경로로
재생한다. Pattern 아래의 Stage `Replay Sequence`와 `[PRODUCT] Play Saved Effect`는 기존처럼
Model View에서 애니메이션 단계와 개별 Effect를 저작·확인하는 로컬 미리보기로 유지한다.

```text
Pattern 행 Play Server Pattern
-> stable boss placementId + stable patternId
-> Debug typed request
-> private/shared CGameRoom의 실제 BOSS_VALTAN
-> boss-only reset
-> CValtanBrain PendingPatternIds
-> Server snapshot
-> CValtan animation + Product cue + Effect presentation
```

Character Select의 `Spawn Selected -> Valtan`으로 생성한 보스는 이미 Server `CGameRoom`,
`CValtanBrain`, `ENCOUNTER_VALTAN`, Product cue를 소비하는 실제 복제 보스이므로 별도 로컬
`CValtan` 또는 Model View 보스를 만들지 않는다.

## 현재 실측

- 기존 `PLAY_PATTERN`은 `iTargetHealthBar`를 encounter 문서의 1-based vector index로 재사용한다.
- `CValtanPatternTree`는 화면 표시를 Gimmick/Rotation으로 분리·재정렬하므로 화면 index를 wire로
  보내면 정본 패턴과 어긋날 수 있다.
- 기존 Valtan audition 결과 deque는 `CLevel_ValtanArena`가 단일 소비한다. Effect Tool이 같은
  deque를 소비하면 verdict consumer race가 생긴다.
- world enter/reset은 연결을 유지한 채 inbound 결과 deque를 비우므로 Effect Tool pending이
  world generation을 함께 소유하지 않으면 버튼이 영구 대기 상태로 남을 수 있다.
- 공유 Valtan Arena에서 같은 fixed-tick command batch에 둘 이상의 stable-ID 요청이 들어오면
  후속 reset이 선행 queue를 덮을 수 있으므로 room-level in-flight 소유권이 필요하다.
- Server audition은 `VALTAN_ARENA`, `boss.valtan.center`, initialized destruction runtime에
  고정돼 있어 Character Select의 `boss.valtan.character-select.lazy`를 재생할 수 없다.

## G01. stable ID 기반 요청과 결과 분리

수정 파일:

- `Shared/Public/Network/PacketMessages.h`
- `Shared/Private/Network/PacketMessages.cpp`
- `Client/Public/NetworkManager.h`
- `Client/Private/NetworkManager.cpp`

`PLAY_PATTERN_ID`는 `bossPlacementId`와 `patternId`를 bounded stable ID로 전달한다. 기존 operation의
wire 형식은 보존하고 새 operation에서만 두 문자열을 추가한다. NetworkManager는 이 operation의
결과를 전용 deque로 분리해 `CLevel_ValtanArena`의 기존 transaction consumer와 경합하지 않는다.

## G02. Character Select boss-only Server reset과 queue

수정 파일:

- `Server/Public/GameRoom.h`
- `Server/Private/GameRoom.cpp`
- `Server/Private/ServerGameplayContractTests.cpp`

Server는 요청 session이 소유한 room에서 stable placement와 `BOSS_VALTAN/ENCOUNTER_VALTAN`을
exact resolve한다. Character Select에서는 이미 Spawn된 보스만 대상으로 하고 자동 로컬 생성이나
arena encounter activation을 하지 않는다.

boss-only reset은 동일 `NetEntityId`, monotonic pattern sequence와 BossCombat revision을 유지하면서
보스 action/pattern/combat state와 그 보스의 active combat object를 초기화한다. Valtan Arena의 벽,
바닥, pillar, navigation blocker, destruction full sync는 Character Select에서 변경하지 않는다.

Character Select에서는 환경 의존 패턴을 fail closed한다.

- `VALTAN_ARMOR_BREAK_OPENING`
- `VALTAN_ENTRANCE_WHIRLWIND`
- `VALTAN_ARENA_BREAK_109`
- `VALTAN_ARENA_BREAK_84`
- `VALTAN_ARENA_BREAK_33`
- `VALTAN_FOUR_PILLARS_105`

`VALTAN_DASH_CHARGE`를 포함한 일반 패턴은 engaged player와 live boss가 있을 때 실제
`PendingPatternIds`에 queue한다.

room은 queue된 stable-ID 패턴이 pending 또는 active인 동안 해당 audition을 in-flight로
소유한다. 후속 요청은 별도 성공으로 보고하지 않고 fail closed하며, 선택 패턴이 완료·유실되거나
대상 보스가 교체된 뒤에만 다음 요청을 받는다.

## G03. Effect Tool Pattern 행 Server Play

수정 파일:

- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_Tool.cpp`

Pattern 행의 `Play Server Pattern` 버튼은 `VALTAN_PATTERN_VIEW::strPatternId`만 command 입력으로
사용한다. Effect Tool 내부 controller가 request sequence, pending identity, 전용 result 소비,
사용자 상태 문자열을 소유한다. 요청 당시 NetworkManager world inbound generation도 함께
기록해 world transfer/reset이 verdict queue를 지우면 pending을 명시적으로 취소한다.

버튼은 다음 조건에서만 활성화한다.

- Server connection이 살아 있음
- 현재 Level이 Character Select 또는 Valtan Arena
- 이전 Server Pattern request가 pending이 아님
- Character Select에서 선택 pattern이 환경 의존 blocklist에 속하지 않음

서버 보스가 없으면 Server의 `REJECTED_NO_BOSS`를 받아 `Spawn Valtan first`로 표시한다. 하위
Stage/Product 버튼은 수정하지 않는다.

## 검증

```powershell
MSBuild.exe Shared/Default/Shared.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m /nologo
MSBuild.exe Tools/NetworkProtocolHarness/Default/NetworkProtocolHarness.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m /nologo
Tools/NetworkProtocolHarness/Bin/Debug/NetworkProtocolHarness.exe
MSBuild.exe Server/Default/Server.vcxproj /t:Build /p:Configuration=Debug /p:Platform=x64 /m /nologo
Server/Bin/Debug/Server.exe --contract-test
MSBuild.exe Client/Default/Client.vcxproj /t:ClCompile /p:Configuration=Debug /p:Platform=x64 /m /nologo
git diff --check
```

에이전트는 Client를 실행·조작하거나 visual PASS를 대신 판정하지 않는다. 사용자는 Server + Client를
실행한 뒤 Character Select에서 Valtan을 Spawn하고 Effect Tool Pattern 행의 Server Play로 실제
animation, 이동, cue, Effect follow를 육안 확인한다.
