# Valtan Presentation Scale 10x 구현 계획

## 목표

일반 `BOSS_VALTAN`의 Client 시각 배율을 `10.0`으로 변경한다. 유령 발탄은 `1.0`, Server gameplay
충돌 반경은 `1.4m`, HP/공격력과 model admission pre-scale은 기존 값을 유지한다.

## 변경

- `Data/Actors/BossCatalog.json`의 일반 발탄 `presentationScale`만 `10.0`으로 변경한다.
- model composition 계약은 Client 시각 배율과 Server collision을 독립 검증한다.
- floor destruction 계약과 현재 팀 정본 문서의 scale 설명을 `10.0`으로 동기화한다.
- `Data/Actors/BossCatalog.json`을 최종 실행 패키지의 Data closure에 포함한다.

## 검증

```powershell
python -B Tools/EffectPipeline/test_valtan_model_view_composition.py
python -B Tools/WorldPipeline/test_valtan_floor_destruction_transition_contract.py
powershell -NoProfile -ExecutionPolicy Bypass -File `
  Tools/GameplayPipeline/Publish-GameplayBalance.ps1 -Mode Validate
```

그 뒤 UI 변경과 함께 Debug/Release 정본 build/regression을 통과시킨다. 사용자가 병합본 Client에서
실제 visual scale을 판정하기 전에는 수동 visual PASS로 기록하지 않는다.
