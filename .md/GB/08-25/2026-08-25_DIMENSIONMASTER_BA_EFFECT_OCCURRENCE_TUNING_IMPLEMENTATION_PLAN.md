# 차원술사 BA Effect occurrence 손튜닝 분리 구현 계획

## 1. 목표와 기준선

- 기준선은 `origin/main@b8653bfd96113ac8614f16c681bccba1b5fae5a4`다.
- 차원술사 LMB `2050010`의 제품 animation·Server stage 변경과 authored Effect payload 변경을 서로 독립적인 PR로 분리한다.
- 첫 motion이 소비하는 `effect.dimensionmaster.skill.2050010.ba2.unified`는 사용자가 확정한 두 visible occurrence만 보존한다.
- 두 번째 motion이 한 번 시작하는 `effect.dimensionmaster.skill.2050010.ba3.unified`는 현재 손튜닝한 composite payload를 보존한다.
- stable Effect ID와 `Data/Effects/EffectCatalog.json`의 authoring binding은 바꾸지 않는다.
- 선택한 두 문서만 content-addressed runtime seal로 publish하고 runtime catalog의 두 행만 교체한다.

## G00. authored Effect payload

### 2. 변경 데이터

| Effect stable ID | main | 손튜닝 결과 | 제품 의미 |
|---|---:|---:|---|
| `effect.dimensionmaster.skill.2050010.ba2.unified` | visible Element 8개 | visible Element 2개 | 첫 motion 안의 두 occurrence |
| `effect.dimensionmaster.skill.2050010.ba3.unified` | visible Element 4개 | visible Element 3개 | 두 번째 motion에서 한 번 시작하는 composite cue |

두 authored 문서는 Effect Tool이 저장한 schema v13 정본이다. 다른 차원술사 스킬, Artist,
Lance Master, Warlord, Valtan authored 문서는 이 변경에 포함하지 않는다.

## G01. 선택 publish와 runtime identity

### 3. runtime 출력

선택 publish는 두 source document를 compact한 뒤 SHA-256이 포함된 새 seal을 만들고
`Client/Bin/DataFiles/Effect/EffectCatalog.runtime.json`의 동일 stable ID 행만 새 seal로 바꾼다.
과거 seal은 이 변경에서 삭제하지 않는다. catalog 전체를 다시 생성해 범위 밖 dirty Effect를
흡수하지 않는다.

검증은 다음을 포함한다.

- authored JSON schema/version/stable ID와 중복 Element ID
- Resources-relative `.wmodel`/`.dds` 존재
- compact runtime payload hash와 catalog seal 경로 일치
- 기존 runtime catalog에서 목표 두 행 외 semantic JSON 불변
- 차원술사 BA animation PR과 합성한 `test_dimensionmaster_2050010_stage_split.py` 5건
- JSON parse와 `git diff --check`

## 4. 완료 경계

이 변경은 Effect payload와 runtime identity만 소유한다. LMB 입력, Server stage,
animation binding, root motion과 balance timing은 PR #212가 소유한다. Client visual fidelity는
사용자가 직접 확인하기 전까지 `PENDING_USER_VISUAL_GATE`다.
