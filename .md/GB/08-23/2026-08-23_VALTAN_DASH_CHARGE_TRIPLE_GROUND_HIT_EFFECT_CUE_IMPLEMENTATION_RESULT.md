# 2026-08-23 Valtan Dash Charge Product Effect·Cue 연결 결과

branch: `codex/valtan-pattern-effect-sync`

base: `origin/main@8231bb99`

plan: `2026-08-23_VALTAN_DASH_CHARGE_TRIPLE_GROUND_HIT_EFFECT_CUE_IMPLEMENTATION_PLAN.md`

## 0. 2026-08-24 후속 authoring 상태

아래의 빈 Product 설명은 최초 cue shell을 만들던 시점의 이력이다. 현재 authored 정본은 사용자가
Effect Tool에서 저장한 drawable Element를 각 문서에 하나씩 가진다.

```text
WINDUP  dash-charge-red-floor  | Particle | diffuse.dds
CHARGE  dash-charge-front-aura | Particle | fx_m_trail_001_cl.dds
modelCues: []
```

`Publish-Effects.ps1 -Mode Publish`가 두 source 문서를 다시 seal했고 runtime catalog는 각각
`...active-shield.aebc1ed4....effect.json`, `...windup-telegraph.f9fba265....effect.json`을
참조한다. 최초 blank hash와 중간 저장 hash는 catalog 미참조 orphan이므로 커밋 대상이 아니다.

## 1. 결론

Dash 화면의 `[REFERENCE]` 4개를 Product Effect처럼 나열하던 문제를 정리했다.
`VALTAN_DASH_CHARGE`의 active `Saved Unified Effects`에는 이제 실제 cue가 소유한
`[PRODUCT]` 2개만 투영된다.

```text
[PRODUCT] effect.valtan.project-tuned.dash-charge.windup-telegraph
[PRODUCT] effect.valtan.project-tuned.dash-charge.active-shield
```

두 문서는 최초 연결 시점에는 내부를 의도적으로 비웠다.

```text
modelCues: []
elements: []
```

이 shell을 연 뒤 사용자가 빨간 바닥과 정면 aura를 각각 한 Element씩 저장했으며, 현재 source와
runtime catalog는 위 후속 상태로 동기화됐다. 색·크기·위치의 최종 visual fidelity 판정은 계속
사용자 화면 검증 범위다.

## 2. `REFERENCE`의 뜻과 처리

기존 `[REFERENCE]`는 animation cue가 아니었다. pattern stage 이름과 authored Effect ID의
과거 naming rule이 맞아서 Tool이 참고 문서로 발견했다는 뜻이었다. 스크린샷의
`Stage reference provenance`도 실제 Product 연결이 아니라 그 탐색 근거다.

reference-only 파일을 물리 삭제하지는 않았다. 예를 들어 `part-break` 문서는 실제 element를
가진 자료라 일괄 삭제하면 다른 작업 입력까지 파괴할 수 있다. 대신 active All Effects 투영에서
`ProductSources`와 `CombatObjectStages`가 모두 없는 행을 제외했다.

- 실제 animation cue 소유: `[PRODUCT]`
- world-root visual 소유: `[WORLD]`
- reference-only: Saved Unified Effects에서 숨김

따라서 사용자가 본 Dash `Saved 4` reference 묶음은 사라지고, 새 Product 2개만 남는다.

## 3. 연결한 두 Product Effect

### 3.1 WINDUP 빨간 바닥용

```text
effectAssetId:
  effect.valtan.project-tuned.dash-charge.windup-telegraph

displayName:
  VALTAN_DASH_CHARGE / WINDUP / Red Telegraph

cue:
  WINDUP
  valtan.attack.dash-charge.windup.clip.01
  source 559–2364 ms
  root / snapshot / cue_end / once
```

사용자는 이 문서를 열고 `Sprite Particle`을 만든다. `snapshot`은 cue 시작 시 발탄 정면을
잡아 두므로, 준비 방향 바닥이 돌진 중 root를 따라 이동하지 않는다.

### 3.2 CHARGE 정면 방패용

```text
effectAssetId:
  effect.valtan.project-tuned.dash-charge.active-shield

displayName:
  VALTAN_DASH_CHARGE / CHARGE / Front Shield

cue:
  CHARGE
  valtan.attack.dash-charge.active.clip.01
  source 2450–3350 ms
  root / follow / cue_end / once
```

사용자는 이 문서를 열고 WModel seed를 바인드한 `Mesh Particle`을 만든다. `follow`는 돌진 중
발탄 root를 따라가므로 정면 aura를 Element 내부 attachment로 다시 연결할 필요가 없다.

## 4. `New Effect`가 실제로 하는 일

`New Effect`는 입력한 Effect Name으로 메모리의 새 빈 문서를 만드는 버튼이다.

- 즉시 authored JSON을 쓰지 않는다.
- EffectCatalog에 row를 추가하지 않는다.
- animation clip이나 cue를 선택하지 않는다.
- WINDUP/CHARGE에 자동 연결하지 않는다.
- `Save Changes` 또는 `Save As` 전에는 파일이 아니다.

따라서 이번 Dash에서는 `New Effect`를 누르면 안 된다. 같은 두 stable ID는 이미 존재해 overwrite도
거부된다. cue까지 연결된 기존 빈 Product를 `Open Saved Effect`로 여는 것이 정확하다.

## 5. Effect Tool 사용 경로

```text
F1
-> Effect Tool
-> All Effects
-> Character / Boss = Valtan
-> VALTAN_DASH_CHARGE 검색
-> Saved Unified Effects (Saved 2)
```

빨간 바닥:

```text
[PRODUCT] ...windup-telegraph
-> Open Saved Effect
-> Element Authoring
-> Element Type = Sprite Particle
-> Layer Name 입력
-> Base DDS seed/binding
-> Create Element
-> Effect Details 손튜닝
-> Apply to Current Effect (Unsaved)
-> Save Changes
```

정면 방패:

```text
[PRODUCT] ...active-shield
-> Open Saved Effect
-> Element Authoring
-> Element Type = Mesh Particle
-> Optional Element Seed Resources
-> WModel 선택 + Bind Selected
-> Layer Name 입력
-> Create Element
-> Base/Noise/Mask/Dissolve 등 바인딩
-> Effect Details 손튜닝
-> Apply to Current Effect (Unsaved)
-> Save Changes
```

최초 빈 문서는 `Open Saved Effect`가 가능하고 `Play Saved Effect`는 잠겼다. 현재처럼 drawable
Element를 저장하면 해당 Valtan All Effects cache를 강제로 다시 parse하므로 수동 Refresh 없이
Play gate가 새 상태를 반영한다. 구조가 잘못된 문서는 Open과 Play를 모두 잠근다.

Tool 세션에서는 저장 뒤 Product hot reload를 시도한다. 다음 Client 실행에서도 같은 모양을 쓰려면
최종 authored 문서를 `Publish-Effects.ps1 -Mode Publish`로 runtime catalog에 다시 publish해야 한다.

## 6. 변경 파일

- `Client/Private/Effect_Tool.cpp`
- `Data/Effects/Authored/effect.valtan.project-tuned.dash-charge.windup-telegraph.effect.json`
- `Data/Effects/Authored/effect.valtan.project-tuned.dash-charge.active-shield.effect.json`
- `Data/Effects/EffectCatalog.json`
- `Data/Animation/Authored/Valtan/Valtan.patterneffectcues.json`
- `Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json`
- `Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py`

## 7. 자동 검증

- `Publish-Effects.ps1 -Mode Validate`: PASS, 163 Effect catalog entries / 171 material-program bindings
- `python -B -m unittest Tools.EffectPipeline.test_effect_tool_valtan_saved_rows`: PASS, 21 tests
- `python -B -m unittest Tools.ValtanActionExtractor.test_build_valtan_rootmotion`: PASS, 3 tests
- `build_valtan_rootmotion.py --check`: PASS
- `Publish-GameplayBalance.ps1 -Mode Validate`: PASS, 6 profiles / 231 skills / 34 boss patterns /
  131 stages / 67 Valtan audition occurrences
- Server x64 Debug build: PASS
- `Server.exe --contract-test`: PASS, failures 0
- Client x64 Debug build: PASS, `Client/Bin/Debug/Client.exe` link 완료
- JSON parse 13 files / XML parse 2 files: PASS
- `git diff --check`: PASS; 기존 LF→CRLF 경고만 출력

## 8. 수동 화면 확인 대기

에이전트는 Client를 실행하거나 visual PASS를 기록하지 않았다. 사용자가 직접 확인할 것은 다음이다.

1. Dash 아래에 `[REFERENCE]` 없이 `[PRODUCT]` 두 개만 보이는가.
2. 두 Product 문서가 `Open Saved Effect`로 열리는가.
3. WINDUP의 빨간 바닥 Element와 CHARGE의 정면 aura Element가 각각 한 개 보이는가.
4. 두 Product의 Play가 활성화되는가.
5. WINDUP Play에서 빨간 바닥이 준비 방향에 고정되는가.
6. CHARGE Play에서 방패가 발탄 정면을 따라가는가.

5–6의 실제 색, 크기, 위치, 방향은 아직 사용자가 작성할 Element에 달려 있으므로 미검증이다.
