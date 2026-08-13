# 2026-08-13 HOLD 스킬 차지 중 조준 회전 RESULT

작성자: JS · 2026-08-13 · branch `feature/lancemaster-charge-aim`

PLAN: `2026-08-13_HOLD_SKILL_CHARGE_AIM_PLAN.md`

## 1. 구현 완료

PLAN 2장의 변경 파일 표 그대로 구현했다. 요점:

- `C2S_UPDATE_SKILL_AIM { iClientSequence, iSkillId, fAimX, fAimZ }` 신설,
  `NETWORK_PROTOCOL_VERSION` 16 → 17. 배선은 `C2S_RELEASE_SKILL`과 동일 경로
  (ServerApp dispatch → RoomCommand → GameRoom → PlayerSkillSystem).
- `CPlayerSkillSystem::Update_Aim` — 실행 중인 스킬이 해당 HOLD 스킬이고
  `iComboStage ∈ {1,2}`이며 `hasReleasedHold`가 아닐 때만 `fSkillAimDirection`·
  `fYawDegrees` 갱신. stage 3(발사)·release 후·비HOLD·다른 스킬·액션 없음은 무시.
  `ResolveAimDirection`은 메시지 타입 대신 좌표 인자로 일반화.
- `CPlayerController` — HOLD 키 유지 중 50ms 간격, 직전 전송 지점에서 0.1유닛 이상
  움직인 마우스 지면 좌표를 `Request_SkillAim`으로 재전송. 시전 성공 시점에 전송
  기준점을 초기화하고, 캐릭터 재바인딩 시 상태 리셋.
- 특정 스킬 하드코딩 없음 — skillKind == HOLD 전체에 적용된다. 현재 대상은
  적룡포(34590, 창술사 짧은창 S)와 풀배럴 캐넌(17240, 워로드 T).
  데이터 스키마·provenance receipt 미변경.

새 파일 없음. Engine 미변경(UpdateLib 불필요).

## 2. 자동 검증 (실행함)

- Shared → NetworkProtocolHarness 빌드·실행 failures 0
  (신규: 왕복, payload 16바이트, INVALID_SKILL_ID·seq 0·non-finite 거부, truncate 무변이).
- Server 빌드 → `Server.exe --contract-test` failures 0. 신규 7건:
  차지 중 회전 허용, 미실행 스킬 지정 무시, release 후 잠금, 발사 단계 잠금,
  비HOLD 스킬 무시 등.
- Client 빌드 오류 0. `git diff --check` 통과.

## 3. 수동 검증 (사용자 확인 완료, 2026-08-13)

- 로컬 루프백에서 적룡포·풀배럴 캐넌 모두: 차지 중 마우스 방향으로 회전, 키 release
  또는 차지 완료 시 마지막 방향으로 발사, 발사 모션 중 회전 잠금 확인.

## 4. 남은 경계

- 프로토콜 버전 17 — 팀원 전원이 이 브랜치 머지 후 Server/Client를 함께 재빌드해야
  접속된다(버전 불일치는 입장 거부).
- 차지 중 root motion이 있는 HOLD 스킬이 추가되면 회전이 이동 방향도 돌린다(현재
  두 스킬 모두 차지 구간 이동 없음). 문제되면 그때 stage별 회전 허용 플래그를 검토.
