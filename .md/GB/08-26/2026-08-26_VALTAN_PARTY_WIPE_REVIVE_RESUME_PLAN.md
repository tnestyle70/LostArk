# 발탄 전멸 후 부활 시 패턴 재개 계획

## 현재 실제 반영 상태와 이번 작업 경계

정상 `VALTAN_ARENA`는 한 `CGameRoom`과 한 `SERVER_WORLD_ENTITY`로 160줄부터
마지막 패턴까지 진행한다. 현재 부활 처리는 HP와 액션을 복구한 뒤
`SERVER_PLAYER::isCombatReady`를 `false`로 남긴다. 발탄 Brain은 이 값이 `true`인
살아 있는 플레이어만 타깃으로 인정하며, 전멸로 `NO_VALID_TARGET` 실패가 기록된
체력줄 메커닉 장부도 그런 플레이어가 나타날 때만 해제한다.

이번 변경은 정상 레이드의 서버 권위 부활 승인 자체를 전투 재참가 의사로 취급한다.
F1의 `PLAY_TIMELINE_ROW`가 한 행을 재생한 뒤 `COMPLETED_HOLD`에 머무는 Debug 계약은
변경하지 않는다.

## 수정 파일

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | `C:/Users/USER/source/졸업팀폴/LostArk/Server/Private/GameRoom.cpp` | 부활 성공 commit에서 플레이어를 즉시 전투 타깃 집합에 복귀 |
| 수정 | `C:/Users/USER/source/졸업팀폴/LostArk/Server/Private/ServerGameplayContractTests.cpp` | 실제 `Handle_RevivePlayer -> CGameRoom::Tick -> CValtanBrain` 회귀 검증 |
| 추가 | `C:/Users/USER/source/졸업팀폴/LostArk/.md/GB/08-26/2026-08-26_VALTAN_PARTY_WIPE_REVIVE_RESUME_RESULT.md` | 구현·자동 검증·사용자 수동 검증 경계 기록 |

## GameRoom.cpp 교체 블록

적용 위치: `CGameRoom::Handle_RevivePlayer`의 마지막 부활 상태 commit.

```cpp
	player.iKnockdownEndTick = 0u;
	player.iHitReactionGraceEndTick = 0u;
	// A successful authoritative revive immediately makes the player a valid
	// combat participant again so party-wipe recovery can resume the encounter.
	player.isCombatReady = true;
	m_ServerTriggerSystem.Remove_Player(player.iPlayerId);
```

## ServerGameplayContractTests.cpp 추가 검증

실제 Valtan 방과 실제 부활 handler를 사용해 다음 상태 전이를 검증한다.

```text
활성 발탄 + 전멸로 실패한 NO_VALID_TARGET occurrence + 죽은 유일 플레이어
-> C2S_REVIVE_PLAYER 처리
-> 부활 즉시 isCombatReady == true
-> 실제 room Tick
-> occurrence COMPLETED, latch false
-> 발탄이 IDLE 고정에서 벗어나 다음 패턴 선택
```

다른 실패 원인인 `STAGE_TRANSITION_PREFLIGHT`, `STAGE_TRANSITION_COMMIT`,
`MISSING_PATTERN_DEFINITION`, `BOSS_DIED`는 기존처럼 자동 해제하지 않는다.

## 적용 및 검증

1. 테스트를 먼저 추가해 현재 코드에서 실패하는 것을 확인한다.
2. `Handle_RevivePlayer`의 성공 commit을 수정한다.
3. Server x64 Debug 빌드와 `Server.exe --contract-test`를 실행한다.
4. `git diff --check`를 실행한다.
5. 사용자가 로비에서 발탄에 진입해 전멸, 부활, 다음 패턴 재개를 직접 확인한다.
