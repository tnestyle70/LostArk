# Valtan All Effects Saved UI 결과

## 1. 목표

발탄 `All Effects`를 캐릭터 스킬과 같은 저작 흐름으로 정리한다.

```text
Pattern
├─ Saved Unified Effects
│  └─ Saved Effect
│     ├─ Open Saved Effect
│     └─ Play Saved Effect
└─ Animations / Semantic Stages
```

- `Open Saved Effect`는 발탄 모델과 그 Effect의 정확한 소유 애니메이션을 준비한다.
- `Play Saved Effect`는 준비된 애니메이션과 Effect를 같은 타임라인에서 시작한다.
- phase 반복, Effect 보유 stage 필터처럼 캐릭터에 없는 별도 조작은 제거한다.

## 2. 구현 결과

### 2.1 Pattern 단위 Saved Effect 정본

각 Pattern은 다음 세 출처를 합쳐 `Saved Unified Effects`를 만든다.

- Product cue
- semantic-stage Effect reference
- combat-object Effect

현재 발탄 정본은 다음과 같다.

| 항목 | 수 |
|---|---:|
| Pattern | 33 |
| 원시 Effect 연결 | 107 |
| `(patternId, effectAssetId)` 중복 병합 후 Saved Effect | 106 |

같은 Effect가 Product cue와 stage reference 양쪽에 존재하면 행을 버리지 않고
하나의 Saved Effect에 provenance를 합친다. EffectCatalog의 저작 경로가 있으면
그 경로를 우선해 과거 reference 경로가 catalog 정본을 덮지 못하게 한다.

### 2.2 Open / Play 동기화

- Product cue는 정확한 `clipOccurrenceId + occurrenceId`를 사용한다.
- stage reference는 해당 stage의 ordered clip sequence 전체를 사용한다.
- combat-object Effect도 world-root attachment는 유지하되 Tool preview에서는
  소유 stage의 발탄 애니메이션을 함께 재생한다.
- 다중 clip stage를 첫 clip 하나로 축소하지 않는다.
- 다른 Saved Effect로 전환할 때 미저장 문서가 있으면 대상 모델이나
  애니메이션을 바꾸기 전에 Save/Discard를 묻는다.
- Save/Discard 뒤에도 선택했던 ordered clip sequence와 Play 요청을 보존한다.

### 2.3 UI와 로드 경계

- Pattern 아래에 `Saved Unified Effects`를 먼저 표시한다.
- 각 행은 캐릭터와 같은 `Open Saved Effect`, `Play Saved Effect`를 제공한다.
- 아래 `Animations / Semantic Stages`는 애니메이션과 timing metadata만 표시한다.
- Refresh나 tree 확장만으로 106개 Effect JSON을 미리 decode하지 않는다.
- 실제 Open 또는 Play를 누를 때만 선택 문서를 parse한다.
- 기존 phase selector, phase별 rotation 반복, Effect stage 전용 필터와
  legacy unmapped Valtan 목록은 제거했다.

## 3. 자동 검증

### 3.1 Focused 계약

```powershell
python -m unittest `
  Tools.EffectPipeline.test_effect_tool_valtan_saved_rows `
  Tools.EffectPipeline.test_build_valtan_stage_effects `
  Tools.EffectPipeline.test_migrate_valtan_pattern_occurrences_v2
```

결과: `20 tests / PASS`

추가 회귀 계약은 다음을 검사한다.

- flat Pattern 구조와 phase 조작 제거
- 33 Pattern, raw 107, unique 106
- Product/reference/combat provenance 병합
- catalog 경로 우선과 usable path
- ordered clip sequence 전달
- 미저장 문서 guard와 pending-load replay
- stage metadata 영역의 saved-document eager decode 금지

### 3.2 Effect publisher

```powershell
powershell -NoProfile -ExecutionPolicy Bypass `
  -File Tools/EffectPipeline/Publish-Effects.ps1 `
  -Mode Validate `
  -ResourceRoot C:\Users\user\Desktop\LostArk\Client\Bin\Resources
```

결과: `145 Effect catalog entries / PASS`

별도 worktree에는 Git 비관리 runtime Resource가 없으므로 팀장이 관리하는 main
workspace의 `Client/Bin/Resources`를 읽기 전용 입력으로 사용했다.

### 3.3 Client

```powershell
MSBuild Client/Default/Client.vcxproj `
  /t:Build /p:Configuration=<Debug|Release> /p:Platform=x64 /m
```

결과: `Client Debug + Release build / PASS`, 각 `Client.exe` 생성

## 4. 사용자 육안 검증 경계

자동 검증은 모델·애니메이션·Effect 선택의 코드·데이터 배선과 빌드 가능성을
확인한다. 실제 동기 재생, first pixel과 visual fidelity는 사용자가 다음 순서로
판정한다.

1. Server + Client를 시작한다.
2. `F1 > Effect Tool > All Effects > Valtan`을 연다.
3. `VALTAN_FRONT_BACK_FRONT` 같은 Pattern을 연다.
4. `Saved Unified Effects`의 한 행에서 `Open Saved Effect`를 누른다.
5. 발탄 모델과 해당 애니메이션이 준비되는지 확인한다.
6. `Play Saved Effect`를 눌러 애니메이션과 Effect가 동시에 재생되는지 확인한다.

사용자의 서면 확인 전에는 visual PASS나 Product fidelity 완료로 승격하지 않는다.
