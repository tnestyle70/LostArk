# 발탄 Composition 저작 흐름 조사·계획 결과

작성일: 2026-09-09. 상태: **현재 코드·데이터 조사와 구현 계획 작성 완료 / 발탄 구현 전**.

후속 실행 준비에서도 새 저작 기능은 미구현이다. Source Save Python 초안은 UI 소비자와 Product snapshot 분리가
완료되지 않아 `out/ValtanCompositionParity20260909/unfinished_source_save/`에 파일·patch·보존 기록으로 남겼다.
초안 3개만 작업 전 상태로 되돌려 기존 Save를 유지했다. 후속 Product 빌드 PASS는 이 계획의 구현 완료가 아니다.

대응 [구현 계획서](C:/Users/user/Desktop/LostArk/.md/GB/09-09/2026-09-09_VALTAN_COMPOSITION_AUTHORING_PARITY_IMPLEMENTATION_PLAN.md)에
Source Save, V1/V2 append·box/detail, row와 Draft Pattern, local preview 및 Product Publish의 변경 범위를 정리했다.
쿠크의 좋은 저작 흐름을 기준으로 하되 현재 발탄의 source, Server authority와 재생기를 그대로 확장하는 계획이다.

## 현재 실측

기준 HEAD는 `591012dbebf7eeab0b660baec42852b9396e77d4`, 브랜치는 `codex/dimensionmaster-tool-round3`다.
조사 당시 HEAD와 origin/main은 같았고, 쿠크·이펙트·캐릭터 관련 다른 세션의 미커밋 변경이 있었다.
그 변경을 유지한 현재 working copy의 코드와 Source를 읽었다.

읽기 전용으로 다음을 확인했다.

- `python Tools/ValtanPipeline/valtan_tuning_pipeline.py --repository-root . validate`: PASS.
  managed pattern 42, legacy pattern 25, combat object 9, world member 97, projected artifact 9.
- 실제 Save writer가 호출하는 `validate_and_project`, Sound candidate dependency 검사,
  V2 binding candidate 검사도 현재 데이터로 PASS였다. 파일 commit은 실행하지 않았다.
- BOSS_VALTAN V2 source는 format 2, binding 102개이며 NATURAL 100개, STAGE_END 2개다.
- Save는 기본 Source·descriptor 5개와 조건부 Product 8개, dirty Sound/V2를 포함하면 최대 15개 target 후보를
  구성한다. 실제 쓰기는 바뀐 bytes만 수행한다. “현재 70개 검증이 항상 실패해 모든 Save가 불가능”이라는
  설명은 이 실측과 맞지 않는다.

## 실제 구조적 결합

현재 Save에는 dirty owner/CAS/atomic writer가 있으나 전체 `validate_and_project`, Sound/V2 join,
전체 source manifest와 Product reopen이 일반 Source 저장에도 연결돼 있다.
준비되지 않은 패턴 하나가 전체 editor reopen/저장 허용 상태에 영향을 줄 수 있는 구조다.
과거 2026-09-04 `missing V2 read-set` 오류는 현재 구현에서 고쳐진 이력이므로 현재 실패로 재사용하지 않았다.

V2에는 Save와 별개의 정확한 UI 결함도 있다. `VALTAN_BIND_SLOT`의 shout 세 binding은 고유 binding ID와
`clip.02/03/04`를 가지지만 현재 UI는 convenience stage 필드와 합성 ID를 소비한다.
runtime source-window 식의 올바른 시작은 1400/2300/3200ms인데 0ms의 같은 선택 ID로 충돌할 수 있다.
typed bindingId와 clock basis를 모든 선택·편집·저장·재생에서 동일하게 쓰는 변경을 계획했다.

현재 V2 append는 STAGE/NATURAL 등의 고정 기본값을 주고 detail은 start 중심이며,
V1은 기존 source draft·cue 편집 경로가 존재한다. V1을 저장본 재생만 가능한 기능으로 판정하지 않았다.
표시용 subrow packing은 저자가 저장한 row ID가 아니며, 빈 Draft Pattern 생성도 현재 Product intake와 결합돼 있다.

Source-only Save를 먼저 열기 전에 runtime snapshot 분리가 필요하다.
live `CEffectV2Runtime::Ensure_Bindings`가 authoring catalog revision을 따라가는 경로를 확인했다.
기존 presentation generation receipt의 immutable binding/leaf/group bytes를 제품이 소비하고,
local preview에는 명시 draft snapshot을 전달하도록 계획했다. 새 runtime/manifest는 만들지 않는다.

## 계획으로 정한 구현 단위

| G | 계획한 결과 |
|---|---|
| G01 | 정상 Source inventory와 오류 row 보존, Product snapshot 고정 |
| G02 | Source-only Save, owner별 CAS/atomic rollback, 미완성 Draft schema와 정확한 저장 상태 |
| G03 | V2 bindingId·typed clock·source-window·실제 finite end의 UI/runtime/publisher 일치 |
| G04 | Resources typed append와 V1/V2 Box Detail의 실제 owner mutation 연결 |
| G05 | 빈 Draft Pattern, 지속되는 row ID와 row assignment |
| G06 | 선택 draft local preview, 기존 encounter 전체 atomic Publish 및 Server revision 보존 |
| G07 | 기능별 최소 컴파일·focused 입력/저장 검사와 사용자 아레나 재생 인계 |

발탄 raid의 scripted/cross-pattern 참조는 기존 encounter 전체 Product 단위로 검증·배포한다.
실패한 일부를 이전 Product와 섞어 새 revision 전체가 적용됐다고 하지 않는다.
Product에 연결되지 않은 새 Draft는 정상 저작 Save를 막지 않게 한다.

## 수행 상태

계획서와 이 조사 RESULT만 추가했다. 발탄 C++/Python/Source schema는 이번 요청에서 수정하지 않았다.
새 schema·source-only Save·V2 identity 수정·row/Draft 기능은 모두 미구현이다.
현재 source의 읽기 전용 구조 검증과 문서 검토를 수행했으며 Client compile/link는 실행하지 않았다.
Workbench 실제 버튼 Save/Reopen, local/Server Play, Client 시각 결과는 이번 조사에서 실행하지 않았다.
그 결과는 사용자의 직접 조작과 후속 구현 검증 후에만 기록한다.

PLAN/RESULT의 UTF-8, 모든 링크 대상 존재, 신규 파일을 포함한 공백 검사를 확인했다.
문서 확인 근거는 [documents_verified.json](C:/Users/user/Desktop/LostArk/out/DimensionMasterScale20260909/documents_verified.json)에 있다.
