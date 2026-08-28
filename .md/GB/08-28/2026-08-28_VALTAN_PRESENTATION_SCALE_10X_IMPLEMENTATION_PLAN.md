# Valtan Scale 복원 및 HP 10x 핫픽스 구현 계획

## 목표

PR에서 일반 `BOSS_VALTAN`의 HP 10배 요청이 Client 시각 배율 `10.0`으로 잘못 반영된 부분을 바로잡는다.
Client 시각 배율은 `1.0`으로 복원하고 Server 권위 본체 HP는 `60000`에서 `600000`으로 변경한다.
유령 발탄 HP는 `60000`, health bar 수는 `160`, Server gameplay 충돌 반경은 `1.4m`를 유지한다.

## 변경

- `Data/Actors/BossCatalog.json`의 일반 발탄 `presentationScale`을 `1.0`으로 복원한다.
- `Data/Balance/BossProfiles.json`의 일반 발탄 `maximumHp`만 `600000`으로 변경한다.
- balance provenance receipt는 공식 수치를 주장하지 않고 `PROJECT_TUNED` `600000`으로 동기화한다.
- model composition, floor destruction, balance 및 Server 계약 테스트가 scale/HP/body radius를 각각 고정한다.
- Release Server bootstrap을 다시 publish하고 Client 원클릭 패키지를 새 바이너리로 재생성한다.

## 검증

```powershell
python -B Tools/EffectPipeline/test_valtan_model_view_composition.py
python -B Tools/WorldPipeline/test_valtan_floor_destruction_transition_contract.py
python -B Tools/GameplayPipeline/test_valtan_fast_combat_tuning_contract.py
powershell -NoProfile -ExecutionPolicy Bypass -File `
  Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
```

그 뒤 UI 변경과 함께 Debug/Release 정본 build/regression을 통과시킨다. 사용자가 병합본 Client에서
실제 visual scale을 판정하기 전에는 수동 visual PASS로 기록하지 않는다.
