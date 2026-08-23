# 2026-08-23 Valtan Whirlwind SPIN Effect 연결 계획

## 목표

사용자가 Effect Tool에서 튜닝하는
`effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01` 문서는 수정하지 않고,
그 Product cue의 재생 owner만 `VALTAN_WHIRLWIND / RECOVERY`에서 실제 회전 구간인
`VALTAN_WHIRLWIND / SPIN`으로 옮긴다.

## 현재 정본

- SPIN stage: `SPIN`, action `valtan.attack.whirlwind.active`, duration `2133 ms`
- SPIN animation occurrence: `valtan.attack.whirlwind.active.clip.01`
- SPIN animation clip: `mesh_att_battle_20_03`, loop `true`
- RECOVERY animation clip `mesh_att_battle_20_04`는 animation binding에 그대로 둔다.
- SPIN에는 기존 `effect.valtan.pattern.420633.active` cue가 이미 있다. 이번 cue를 옮긴 뒤에는
  기존 Effect와 사용자가 튜닝하는 carrier V1 Effect가 같은 SPIN clip에서 함께 재생된다.

## G01. Product cue owner 이동

수정 파일:

- `Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json`
- `Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py`

대상 cue의 stable identity와 Effect asset ID는 유지하고 다음 owner 세 필드만 교체한다.

```json
{
  "bindingId": "cue.valtan.carrier-v1.attack.whirlwind.recovery.clip-01",
  "occurrenceId": "cue.valtan.carrier-v1.attack.whirlwind.recovery.clip-01.occurrence.01",
  "patternId": "VALTAN_WHIRLWIND",
  "stageId": "SPIN",
  "actionId": "valtan.attack.whirlwind.active",
  "clipOccurrenceId": "valtan.attack.whirlwind.active.clip.01",
  "effectAssetId": "effect.valtan.carrier-v1.attack.whirlwind.recovery.clip-01"
}
```

유지 항목:

- `bindingId`, `occurrenceId`, `effectAssetId`
- `sourceStartMs: 0`
- `repeatPolicy: once`
- `stopPolicy: natural`
- authored Effect의 elements와 사용자가 저장한 튜닝 값

Carrier materializer와 역사 receipt는 실행 정본이 아니다. 이번 owner 이동을 위해
materializer `--write`를 실행하거나 authored Effect를 재생성하지 않는다.

## 검증

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File `
  Tools/EffectPipeline/Publish-Effects.ps1 -Mode Validate `
  -ResourceRoot Client/Bin/Resources

python -B -m unittest `
  Tools.EffectPipeline.test_effect_tool_valtan_saved_rows

MSBuild.exe Client/Default/Client.vcxproj /t:Build `
  /p:Configuration=Debug /p:Platform=x64 /m /nologo

git diff --check
```

Client는 cue JSON을 새 process 시작 시 다시 읽는다. 화면상의 실제 타이밍과 표현은 사용자가
Valtan → F1 → All Effects → `VALTAN_WHIRLWIND / SPIN`에서 직접 판정한다.
