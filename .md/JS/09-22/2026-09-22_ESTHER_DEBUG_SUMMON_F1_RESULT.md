# 2026-09-22 에스더 F1 디버그 소환 · 월드별 로스터 RESULT

## 목표

- 캐릭터 셀렉트에서 Ctrl+Z/X/C가 발탄 로스터(실리안·웨이·바훈투르)만 내보내 니나브·이난나를
  테스트할 수 없던 문제를 F1 디버그 버튼으로 해결한다.
- 최종 목표인 월드별 로스터(발탄: 실리안·웨이·바훈투르, 쿠크: 니나브·웨이·이난나)를 Server에
  미리 구조화한다.

## 구현

### Shared (protocol 103 → 104)

- `PACKET_TYPE::C2S_DEBUG_USE_ESTHER` append, `Is_Known_Packet_Type` 등록.
- `ESTHER_ID { SILLIAN, WEI, BAHUNTUR, NINAV, INANNA }`, `Is_Valid_EstherId`.
- `C2S_DEBUG_USE_ESTHER { iRequestSequence, eWorldId, eEsther, fAimX, fAimZ }` Write/Read.
  결과 메시지 없음. 캐스팅·소환은 world snapshot으로 온다.

### Server

- `EstherSkillSystem.h`: `ESTHER_DEFINITIONS[5]`(ESTHER_ID → archetype, strikeMs)와 월드별
  `ESTHER_WORLD_ROSTER`(발탄 / 쿠크). `Initialize(worldId)`가 로스터를 고르고 Character Select는
  발탄 로스터를 유지한다. `Try_Consume(slot)`은 방 로스터의 슬롯을 정의 표에서 찾는다.
  `Find_EstherDefinition(ESTHER_ID)` 추가.
- strikeMs: 니나브 `npc_sk_parkunas` 145f/30fps → 4900, 이난나 `npc_sk_magicshield` 121f → 4100
  (기존 실리안 157f→5300, 바훈투르 121f→4100과 같은 올림 규칙).
- `CGameRoom`: `Handle_UseEstherSkill`을 `Find_EstherCaster` + `Begin_EstherCall`로 분리.
  `Handle_DebugUseEsther`는 `_DEBUG`에서만 동작하며 world 일치·Esther 활성 월드·idle caster만
  검사하고 게이지는 건드리지 않는다. Release는 무시한다.
- `ROOM_COMMAND_TYPE::DEBUG_USE_ESTHER`, `ServerApp` 파싱, `GameRoom.cpp` dispatch.

### Client

- `IPlayerCommandSink::Request_DebugUseEsther` (기본 false) → `CNetworkPlayerCommandSink` →
  `CNetworkManager::Send_DebugUseEsther`.
- `CPlayerController::Request_DebugUseEsther(ESTHER_ID)` (`_DEBUG`): 로컬 캐릭터 look 전방 5m를
  aim으로 보낸다.
- F1 Developer Tools의 `Esther Cutin (Debug)` 헤더를 `Esther Skill (Debug)`로 바꾸고
  `Summon Sillian / Wei / Bahuntur / Ninav / Inanna` 버튼 추가. 기존 컷인 Preview 버튼은 유지.
  `Find_ActivePlayerController()`로 Character Select / Bern / Valtan / Kouku 모두에서 동작하며
  Server가 Esther를 켠 월드(발탄·쿠크·Character Select)에서만 실제 소환된다.

### Harness

- `NetworkProtocolHarness.cpp`의 `NETWORK_PROTOCOL_VERSION == 103u` 6곳을 104u로 갱신.
  (`ServerGameplayContractTests_SpawnGroups.cpp:1285`의 `== 101u`는 이전부터 stale이며 건드리지 않음.)

## 검증

- 실행한 것: `git diff --check` 통과, 변경 파일 EOL은 HEAD와 동일(LF).
- 미실행: 컴파일·링크, harness, Server/Client 실제 재생. 빌드는 사용자가 직접 수행.

## 사용자 확인 절차

1. Shared → Server → Client 순 Debug 빌드, Server/Client 둘 다 재시작(protocol 104).
2. Lobby → Character Select 입장 → F1 → `Esther Skill (Debug)` → `Summon Ninav` / `Summon Inanna`.
   캐릭터가 캐스팅 자세를 취하고 1초 뒤 전방 2m에 니나브/이난나가 나타나야 한다.
   니나브·이난나는 `cutinMovie`가 없어 컷인은 나오지 않는다(기존 추출 결과 그대로).
3. Ctrl+Z/X/C는 Character Select·발탄에서 기존과 동일, 쿠크 아레나에서는 니나브·웨이·이난나.

## 남은 경계

- 니나브·이난나 컷인 프레임 미추출(`cutinMovie` 없음).
- 디버그 소환은 caster idle 조건과 Esther 활성 월드 조건은 그대로 두고 게이지만 우회한다.
