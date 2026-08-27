# 2026-08-28 Valtan Product Effect unlink transaction 구현 계획

## 목표

Effect Tool `All Effects`의 `[PRODUCT]` 행에서 `Delete Effect`를 확인하면 Effect asset을 삭제하지 않고,
선택한 Pattern이 소유한 exact Product cue 연결만 해제한다.

```text
patternId + effectAssetId + exact cueId set
-> split Valtan.presentation.json candidate
-> atomic PublishV2
-> ValidateV2
-> Effect Tool pattern tree/DataFiles reparse
```

`EffectCatalog.json`, authored `.effect.json`, 같은 Effect를 사용하는 다른 Pattern, 선택 Pattern의 다른
Effect cue는 보존한다.

## 현재 실측

- 기존 unlink는 source를 먼저 바꾼 뒤 이전 Product를 대상으로 `ValidateV2`를 실행해 정상 drift도 실패했다.
- managed cue scale-policy 검사가 live cue 집합을 고정해 의도적인 cue 제거를 거부했다.
- sealed legacy cue가 전역 배열 ordinal로 검증되어 앞쪽 managed cue 하나를 제거하면 무관한 legacy cue까지
  drift로 판정됐다.
- 실패 rollback 과정에서 split source는 복구됐지만 generated cue Product만 candidate 상태로 남을 수 있었다.

## 구현

1. unlink transaction을 `source CAS -> PublishV2 -> ValidateV2`로 바꾸고, 어느 단계든 실패하면 source를
   baseline으로 CAS rollback한 뒤 baseline Product를 다시 publish한다.
2. 현재 managed cue는 scale-policy migration ledger의 부분집합을 허용하되, 남아 있는 cue의 typed policy는
   계속 exact 검증한다.
3. sealed legacy Effect cue는 `cueOrdinal` 위치가 아니라 stable `bindingId`로 조회해 payload를 비교한다.
4. V1 migration fixture는 current split과 Product에서 의도적으로 사라진 cue를 재생성하지 않는다.
5. FOUR_SLASH의 `RECOVERY` cue 하나만 split 정본에서 제거하고 PublishV2 생성물을 함께 갱신한다.

## 검증

```powershell
python -m unittest Tools.EffectPipeline.test_valtan_pattern_effect_unlink
python -m unittest Tools.EffectPipeline.test_effect_tool_valtan_all_effects_contract
python -m unittest Tools.EffectPipeline.test_effect_tool_valtan_saved_rows
python -m unittest Tools.ValtanPipeline.test_valtan_pattern_master_v2
python -m unittest Tools.ValtanPipeline.test_valtan_pattern_tree_contract
powershell -File Tools/EffectPipeline/Validate-EffectSources.ps1
powershell -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode PublishV2
powershell -File Tools/ValtanPipeline/Project-ValtanPatternMaster.ps1 -Mode ValidateV2
git diff --check
```

Client/UI는 에이전트가 실행하지 않는다. 실행 중 Client에는 `Refresh`, Server Product 판정에는 Server 재시작과
Valtan Arena 재진입이 필요하다.
