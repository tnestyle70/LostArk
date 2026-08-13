# 2026-08-13 HOLD 스킬 차지 중 조준 회전 PLAN

작성자: JS · 2026-08-13 · branch `feature/lancemaster-charge-aim`

08-06 HOLD 계약(`2026-08-06_LOSTARK_HOLD_SKILL_CONTRACT_*.md`)의 후속. 당시 "방향은 시전
시점에 확정"으로 설계했으나, 원작의 적룡포(34590, 짧은창 S)는 차지(start/loop) 중 마우스
방향으로 계속 회전하고 발사 단계에서 마지막 방향으로 공격한다. 이 동작을 서버 권위로 추가한다.

## 1. 계약

- 새 메시지 `C2S_UPDATE_SKILL_AIM { iClientSequence, iSkillId, fAimX, fAimZ }`.
  `NETWORK_PROTOCOL_VERSION` 16 → 17.
- 서버 규칙: `eAction == SKILL`이고 현재 스킬이 해당 HOLD 스킬이며 `iComboStage ∈ {1, 2}`
  (차지·루프)이고 아직 release되지 않았을 때만 `fSkillAimDirection`과 `fYawDegrees`를 갱신.
  stage 3(발사)부터는 잠금 — "차지 끝나면 마지막 방향으로 공격". HOLD가 아닌 스킬,
  다른 스킬, 액션 없음은 전부 무시(연결 종료 아님, move와 동일한 관용).
- 적용 대상은 skillKind == HOLD 전체(데이터 스키마 변경 없음 — provenance receipt 계약을
  건드리지 않는다).

## 2. 변경 파일

| 파일 | 내용 |
|---|---|
| Shared/PacketType.h | enum + Is_Known_Packet_Type + version 17 |
| Shared/PacketMessages.h/.cpp | struct + Write/Read (USE_SKILL 패턴) |
| Server/RoomCommand.h | `UPDATE_SKILL_AIM` + payload |
| Server/ServerApp.cpp | frame dispatch 분기 |
| Server/GameRoom.h/.cpp | `Handle_UpdateSkillAim` + command switch |
| Server/PlayerSkillSystem.h/.cpp | `ResolveAimDirection` 좌표 인자화, `Update_Aim` |
| Client/PlayerCommandSink.h | `Request_SkillAim` 추가 |
| Client/NetworkPlayerCommandSink.h/.cpp | 구현 → `Send_SkillAim` |
| Client/NetworkManager.h/.cpp | `Send_SkillAim` |
| Client/PlayerController.h/.cpp | HOLD 키 유지 중 50ms 간격, 이동 임계 넘은 마우스 지점을 aim으로 재전송 |
| Tools/NetworkProtocolHarness | 왕복 + invalid(seq 0, INVALID_SKILL_ID, non-finite) 거부 |
| Server/ServerGameplayContractTests.cpp | stage 1·2 회전 허용, stage 3·release 후 잠금, 비HOLD 거부 |

새 파일 없음 → vcxproj/filters 변경 없음.

## 3. 검증

1. Shared+NetworkProtocolHarness 빌드·실행 → 신규 왕복 PASS.
2. Server 빌드 → `Server.exe --contract-test` PASS.
3. Client 빌드 → 로컬 루프백에서 짧은창 스탠스 S 꾹 누른 채 마우스 회전 → 캐릭터가 차지 중
   따라 돌고, 발사가 마지막 방향으로 나가는지 육안 확인(사용자).
4. `git diff --check`.
