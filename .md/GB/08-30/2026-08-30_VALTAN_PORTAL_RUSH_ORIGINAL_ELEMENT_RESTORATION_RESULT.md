# 발탄 Portal Rush 원작 Element 복원 결과

## 완료 상태

`VALTAN_PORTAL_RUSH / PORTAL` Product의 삭제된 원작 Element 13개를 복원했다.
현재 남아 있던 원작 mesh Element 1개와 합쳐 정상본과 같은 총 14개가 됐다.

### 변경 파일

```text
Data/Effects/Authored/effect.valtan.carrier-v1.attack.portal-rush.portal.clip-01.effect.json
Tools/EffectPipeline/test_valtan_portal_rush_original_elements.py
.md/GB/08-30/2026-08-30_VALTAN_PORTAL_RUSH_ORIGINAL_ELEMENT_RESTORATION_IMPLEMENTATION_PLAN.md
.md/GB/08-30/2026-08-30_VALTAN_PORTAL_RUSH_ORIGINAL_ELEMENT_RESTORATION_RESULT.md
```

EffectCatalog, pattern cue, animation binding, C++, Server gameplay와 별도 WARP Portal authored 문서는
수정하지 않았다.

## 복원 근거와 결과

- 복원 정본: commit `3dd2a3e0`의 exact authored carrier
- 정본/작업 파일 Git blob: `50260f88b1bfc5da5cf485dd1830ea06fccf0ffe`
- Effect asset ID: `effect.valtan.carrier-v1.attack.portal-rush.portal.clip-01`
- 원작 source group:
  - `fx_mn_rpbf_00_n.par_n_rpbf_potal_02_01`: 9개
  - `fx_mn_rpbf_00_n.par_n_rpbf_potal_02_02`: 5개
- renderer: sprite 11개, mesh 3개
- 모든 Element: `visible=true`, `kind=particle`, `sourceRecipe.enabled=true`
- 고유 Resources-relative DDS/WModel: 12개, 510,868 bytes, 누락/빈 파일 0개

현재 catalog의 `DIRECT_AUTHORED_DOCUMENT` row와
`Valtan.patterneffectcues.json`의 `VALTAN_PORTAL_RUSH / PORTAL` cue,
`Valtan.patternbindings.json`의 `mesh_att_battle_18_01` occurrence를 그대로 소비한다.

## Anim Bench 세션과의 경계

공유 worktree의 Anim Bench/Sequencer 변경을 읽기 전용으로 검토했다. 해당 작업은 Pattern restart,
target anchor, sequencer와 `VALTAN_WARP`의 typed Portal rush motion을 수정 중이며, 이번 carrier authored
JSON은 작업 시작 시 clean이었다. 새 WARP motion test도 별도
`effect.valtan.project-tuned.sequence.warp.portal` cue를 조회하므로 이번 14-Element 원작 carrier와
소유 파일이 겹치지 않는다.

다른 세션에는 메시지를 보내지 않았고, 그 세션의 dirty 파일을 수정·정리·stage하지 않았다.

## 자동 검증

### 재현

복원 전 focused test는 catalog/cue/binding은 통과하고 Element 계약에서
`expected 14 / actual 1`로 실패했다.

### 복원 후

```text
python -B Tools/EffectPipeline/test_valtan_portal_rush_original_elements.py
5 tests / OK

python -B Tools/EffectPipeline/test_effect_tool_valtan_all_effects_contract.py
31 tests / OK

python -B Tools/EffectPipeline/test_effect_tool_valtan_saved_rows.py
35 tests / OK / skipped 7

Validate-EffectSources.ps1
directSourceCount=171
unboundReferenceCount=0
resourceFileCount=991
resourceBytes=76099728
generatedArtifactCount=0
```

focused resource 실측은 Element 14개, 고유 resource 12개, 510,868 bytes,
빈 resource 0개로 통과했다.

## 사용자 수동 확인 대기

에이전트는 Client/UI를 실행하지 않았으며 visual PASS를 기록하지 않았다.

```text
F1
→ Effect Tool
→ All Effects
→ Character / Boss: Valtan
→ VALTAN_PORTAL_RUSH
→ PORTAL
→ [PRODUCT] effect.valtan.carrier-v1.attack.portal-rush.portal.clip-01
→ Open Editor
→ Elements 14개 확인
→ Play Effect + Animation
```

원작과 같은 pixel fidelity, 위치와 방향은 사용자가 위 경로에서 직접 판정한다.

## Git 인계 상태

공유 정본 폴더에는 Anim Bench 세션의 대규모 미커밋 변경이 함께 존재한다. 충돌 방지를 위해 이번
변경은 stage, commit, push하지 않고 위 네 파일만 작업 상태로 남긴다.
