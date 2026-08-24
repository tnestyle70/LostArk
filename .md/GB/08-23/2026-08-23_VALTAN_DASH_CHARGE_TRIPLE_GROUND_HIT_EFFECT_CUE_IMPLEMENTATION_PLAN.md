# 2026-08-23 Valtan Dash Charge 빈 Product Effect·Cue 연결 계획

> 2026-08-24 후속 상태: 이 문서의 `elements: []`는 최초 authoring shell을 만들던 시점의 계획이다.
> 이후 사용자가 WINDUP에 `dash-charge-red-floor`, CHARGE에 `dash-charge-front-aura`를 각각
> 저장했다. 현재 결과와 검증 정본은 대응 RESULT의 후속 상태를 따른다.

## 목표

`VALTAN_DASH_CHARGE`의 시각 요소를 에이전트가 미리 완성하지 않는다. Effect Tool에서 사용자가
직접 채울 수 있도록 다음 두 Product Effect의 ID, catalog row, animation cue만 먼저 연결한다.

| Stage | Product Effect | 사용자가 만들 Element |
|---|---|---|
| `WINDUP` | `effect.valtan.project-tuned.dash-charge.windup-telegraph` | 빨간 바닥 `Sprite Particle` |
| `CHARGE` | `effect.valtan.project-tuned.dash-charge.active-shield` | 정면 청록 방패 `Mesh Particle` |

두 authored v13 문서는 `elements: []`, `modelCues: []`인 구조적으로 유효한 빈 문서로 둔다.
빈 상태에서는 runtime spawn이 no-op이며 `Play Saved Effect`도 잠겨야 한다.

이번 단계에서 도넛 패턴은 다루지 않는다. 기존 Dash animation/Server 변경은 별도 작업 상태로
보존하며, 이 계획의 완료 조건은 두 빈 Product Effect의 authoring 진입점과 cue 연결이다.

## G01. `REFERENCE` 행 정리

기존 `[REFERENCE]`는 Product cue가 아니라 stage 이름 규칙으로 발견된 과거 authored 문서였다.
따라서 animation cue와 연결됐다는 뜻이 아니며, `Saved Unified Effects`에 Product처럼 보이면
사용자가 잘못 열기 쉽다.

파일을 일괄 삭제하지 않는다. 일부 문서는 실제 element를 가진 조사 자료이거나 다른 작업의 입력일
수 있기 때문이다. 대신 아래 조건의 reference-only 행만 active All Effects 목록에서 제외한다.

```text
ProductSources.empty() && CombatObjectStages.empty()
```

표시 이름은 의미를 분리한다.

- `[PRODUCT]`: 실제 animation Product cue가 소유
- `[WORLD]`: encounter/world-root visual이 소유
- `[V1]`: 기존 optional V1 alias

## G02. 빈 authored Product 문서 2개

생성 파일:

- `Data/Effects/Authored/effect.valtan.project-tuned.dash-charge.windup-telegraph.effect.json`
- `Data/Effects/Authored/effect.valtan.project-tuned.dash-charge.active-shield.effect.json`

공통 계약:

```json
{
  "schema": "lostark.effect-authoring",
  "version": 13,
  "particleSystem": {
    "uniformScaleMultiplier": 1,
    "yawOffsetDegrees": 0,
    "directionYawDegrees": 0,
    "initialSpeedMultiplier": 1
  },
  "modelCues": [],
  "elements": []
}
```

두 ID를 `Data/Effects/EffectCatalog.json`에
`DIRECT_AUTHORED_DOCUMENT_V13`으로 등록하고 Effect runtime catalog를 publish한다.

## G03. WINDUP·CHARGE cue 연결

`Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json`에 아래 두 occurrence를 둔다.

| Effect | Stage / clip | Source window | Attachment |
|---|---|---:|---|
| windup telegraph | `WINDUP / valtan.attack.dash-charge.windup.clip.01` | 559–2364 ms | `root / snapshot / cue_end / once` |
| active shield | `CHARGE / valtan.attack.dash-charge.active.clip.01` | 2450–3350 ms | `root / follow / cue_end / once` |

`snapshot`은 준비 때 잡은 정면에 바닥을 고정하고, `follow`는 돌진하는 발탄 root를 방패가
따라가게 하는 Product attachment다. Element 내부에서 attachment를 중복 작성하지 않는다.

## G04. 빈 Product authoring UX

`New Effect`는 Effect Name으로 새 in-memory 문서만 만든다. 누르는 즉시 파일이 생성되지 않고,
EffectCatalog row나 animation cue도 자동 생성하지 않는다. `Save Changes` 또는 `Save As`가
있어야 authored 파일이 생긴다.

이번 두 ID는 catalog와 cue가 이미 준비돼 있으므로 `New Effect`를 사용하지 않는다.

```text
F1
-> Effect Tool
-> All Effects
-> Character / Boss = Valtan
-> VALTAN_DASH_CHARGE 검색
-> Saved Unified Effects (Saved 2)
-> [PRODUCT] ...windup-telegraph 또는 ...active-shield
-> Open Saved Effect
```

구조적으로 유효하지만 비어 있는 Product는 `Open Saved Effect`가 가능해야 한다.
`Play Saved Effect`는 drawable Element가 없으므로 잠근다. 구조적으로 잘못된 문서는 Open과
Play를 모두 잠근다.

사용자가 첫 Element를 만들고 저장하면 All Effects cache를 강제로 다시 parse해, 별도 Refresh 없이
drawable 상태가 Play gate에 반영되게 한다. 저장 성공 뒤 cache 재검증이 실패해도 이미 atomic
commit된 authored 파일을 rollback하지 않는다.

## 검증

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File `
  Tools/EffectPipeline/Publish-Effects.ps1 -Mode Publish `
  -ResourceRoot Client/Bin/Resources

python -B -m unittest `
  Tools.EffectPipeline.test_effect_tool_valtan_saved_rows

MSBuild.exe Client/Default/Client.vcxproj /t:Build `
  /p:Configuration=Debug /p:Platform=x64 /m /nologo

git diff --check
```

에이전트는 Client를 실행하거나 화면 fidelity를 대신 판정하지 않는다. 사용자가 두 빈 Product를
열어 각각 Sprite Particle과 Mesh Particle을 만든 뒤 직접 색, 크기, 방향, 수명을 손튜닝한다.
