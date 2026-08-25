# 차원술사 BA Effect occurrence 손튜닝 분리 구현 결과

## 1. 결론

차원술사 LMB `2050010`의 authored Effect 손튜닝을 animation·Server 변경 PR #212와 분리했다.
첫 motion의 BA2 문서는 visible Element 두 개만 가지며 source start는 `0.55s`, `1.26s`다.
두 번째 motion의 BA3 문서는 visible Element 세 개로 구성된 하나의 composite cue다.

| stable Effect ID | 결과 | runtime seal SHA-256 |
|---|---:|---|
| `effect.dimensionmaster.skill.2050010.ba2.unified` | visible Element 2개 | `4030c9a27ef1af729aebf24d28f0eb17096667a233e2e7fd47c0e3cf59f8e4f0` |
| `effect.dimensionmaster.skill.2050010.ba3.unified` | visible Element 3개 | `c6e6f46edeb0758519dbc5efe8f6d280f4da67f03a65d55b71bd7304d425406c` |

`Data/Effects/EffectCatalog.json`의 stable binding은 바뀌지 않았다. 선택 publisher로 두 문서만
compact·seal하고 runtime catalog의 해당 두 행만 새 seal 경로로 교체했다. 다른 Effect row와
material program, component 계약은 semantic JSON 비교에서 동일했다.

## 2. 변경 범위

- BA2/BA3 schema v13 authored source document
- 위 두 source의 content-addressed runtime seal
- `EffectCatalog.runtime.json`의 위 두 stable ID 행
- 대응 구현 계획과 결과 문서

Artist, Lance Master, Warlord, Valtan과 다른 차원술사 스킬의 dirty Effect 변경은 포함하지 않았다.
과거 seal도 삭제하지 않았다.

## 3. 자동 검증

| 검증 | 결과 |
|---|---|
| selected direct-authored Validate | PASS, BA2/BA3 hash 일치 |
| runtime catalog semantic isolation | PASS, 목표 두 행만 변경 |
| source/runtime/catalog JSON parse | PASS, 5개 문서 |
| visible Element contract | PASS, BA2 2개 / BA3 3개 |
| PR #212와 합성한 `test_dimensionmaster_2050010_stage_split.py` | 5/5 PASS |
| PR #212와 합성한 `test_effect_tool_buffered_combo_audition.py` | 11/11 PASS |
| `git diff --check` | PASS |

이 변경은 data/runtime payload만 바꾸므로 C++ build를 새로 요구하지 않는다. Server stage,
animation binding, root motion과 balance timing build·contract는 PR #212의 Ready 전 검증 범위다.

## 4. 수동 검증과 완료 경계

Client/UI는 에이전트가 실행하지 않았다. 사용자가 실제 Client에서 다음을 확인하기 전까지 visual
판정은 `PENDING_USER_VISUAL_GATE`다.

1. 첫 `_01` motion에서 BA2 occurrence가 두 번 보이는지 확인한다.
2. 이어지는 `_03` motion에서 BA3 composite cue가 한 번 시작하는지 확인한다.
3. `_04 + ba1`은 PR #212 적용 뒤 더 이상 제품 chain에서 시작하지 않는지 확인한다.
