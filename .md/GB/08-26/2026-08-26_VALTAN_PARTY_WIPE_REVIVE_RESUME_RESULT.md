# 발탄 전멸 후 부활 시 패턴 재개 결과

## 결과

정상 `VALTAN_ARENA`에서 전원이 사망해 발탄이 `IDLE`에 들어간 뒤, 플레이어가
서버 승인을 받아 부활하면 같은 방의 패턴 진행을 다시 이어가도록 수정했다.

원인은 `CGameRoom::Handle_RevivePlayer`가 HP와 액션은 복구하면서도
`SERVER_PLAYER::isCombatReady`를 `false`로 남긴 것이었다. 발탄 Brain은 이 상태의
플레이어를 살아 있는 타깃으로 인정하지 않으므로, 전멸 때 기록된
`NO_VALID_TARGET` 메커닉 실패와 `bMechanicLedgerRequiresReset` 잠금이 풀리지 않았다.

부활 성공 commit에서 `isCombatReady = true`로 복구하도록 바꿨다. 이에 따라 별도의
이동·스킬 패킷을 기다리지 않고 다음 방 tick에서 전멸 실패 장부가 정리되며 발탄이
다음 패턴을 선택한다. `NO_VALID_TARGET` 이외의 데이터·스테이지 commit 실패는 기존처럼
자동 해제하지 않는다. F1 개별 패턴 재생의 `COMPLETED_HOLD` 계약도 변경하지 않았다.

## 변경 파일

| 파일 | 변경 |
|---|---|
| `Server/Private/GameRoom.cpp` | 권위 부활 성공 시 플레이어를 즉시 전투 참여 가능 상태로 복귀 |
| `Server/Private/ServerGameplayContractTests.cpp` | 실제 `Handle_RevivePlayer -> CGameRoom::Tick -> CValtanBrain` 회귀 테스트 추가 |

## 검증

수정 전 새 회귀 테스트가 다음 항목으로 실패하는 것을 먼저 확인했다.

```text
[FAILURE] Resume the product Valtan rotation on the authoritative revive tick after a party wipe
```

수정 후 결과는 다음과 같다.

```text
Server x64 Debug build: PASS (warnings 0, errors 0)
Gameplay/World/Navigation publisher invoked by Server pre-build: PASS
Party-wipe revive/resume contract test: PASS
git diff --check: PASS
```

전체 `Server.exe --contract-test`에는 이번 변경과 무관하게 기존 갑옷 데이터 항목 한 건이
남아 있어 프로세스 종료 코드는 1이다.

```text
[FAILURE] Load Valtan's two authored armour plates from the gameplay bootstrap
failures : 1
```

이번 부활 회귀 항목은 같은 실행에서 명시적으로 PASS했다.

## 사용자 수동 확인

자동 화면 조작은 수행하지 않았다. `Server + Client`를 실행한 뒤 로비에서 발탄에 들어가
정상 레이드를 처음부터 진행하고, 모든 플레이어가 사망한 상태에서 한 명을 부활시킨다.
이동이나 스킬 입력을 추가로 하지 않아도 발탄이 다음 server tick에서 `IDLE`을 벗어나
다음 패턴을 시작하는지 사용자가 직접 확인한다.
