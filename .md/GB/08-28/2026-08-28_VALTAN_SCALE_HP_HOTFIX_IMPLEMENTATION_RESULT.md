# Valtan Scale 복원 및 HP 10x 핫픽스 구현 결과

## 완료 상태

- 일반 `BOSS_VALTAN.presentationScale`을 `10.0`에서 `1.0`으로 복원했다.
- 일반 `BOSS_VALTAN.maximumHp`를 `60000`에서 `600000`으로 변경했다.
- 유령 발탄 HP `60000`, health bar `160`, Server collision radius `1.4m`는 유지했다.
- HP provenance를 `PROJECT_TUNED` authored override로 동기화했다.

## 자동 검증

- Gameplay balance publisher Validate: PASS
- Valtan fast combat tuning contract: 3 PASS
- Valtan floor destruction transition contract: 8 PASS
- Valtan model view composition contract: 16 PASS
- JSON parse 및 `git diff --check`: PASS

전체 Debug/Release 빌드와 사용자 육안 검증은 이 핫픽스 PR에서 수행하지 않았다.
