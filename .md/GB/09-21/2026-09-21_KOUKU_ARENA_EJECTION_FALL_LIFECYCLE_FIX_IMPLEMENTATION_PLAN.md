# 쿠크 아레나 외곽 이탈 낙사 수명 수정 구현 계획

## G00. 현재 실측

기존 `CGameRoom::Advance_PlayerKnockback`은 외곽 이탈을 감지하면 `Begin_PlayerFall`로
전환한다. 그러나 `Begin_PlayerFall`이 `bArenaEjectionActive`와 `bKnockbackBallistic`을
초기화하지 않아, 다음 `Update_PlayerFall` 호출이 공중 이동 중이라는 guard에서 반환된다.
따라서 FALLING snapshot은 남지만 dead-zone의 `iFallDeathTick`에 도달해도 사망하지 않는다.

## G01. 변경 계약

`Server/Private/GameRoom_PlayerSimulation.cpp`의 `Begin_PlayerFall`에서 공중/외곽 이동
상태와 owner ID를 FALLING 전환 시 한 번에 지운다. 이후 낙하는 기존 `fFallVelocityY`,
`iFallDeathTick`, `Update_PlayerFall` 경로만 소비한다. 포물선의 gap 전환은 호출 직전에
속도를 `fFallVelocityY`로 넘긴 뒤 같은 정리 경계를 통과하므로, 하강 속도는 보존되고
ejection guard만 제거된다.

## G02. 검증

기존 Server Kouku overlap contract가 외곽 이탈 → `FALLING` → deadline 사망을 검사한다.
변경 후 Server Debug contract build에서 해당 검증을 실행하고 `git diff --check`를 확인한다.
Client 실행과 화면 판정은 부모 작업의 경계다.
