# Effect V2 기본 UV 경로 정리 — JS 미커밋 작업 보존

작성일: 2026-09-11. 기준 HEAD: `79c7cd76`.
브랜치: `codex/js-effect-checkpoint-0911`.
JS가 Claude Code에서 이미 작성한 변경을 검토·검증·커밋했다. 구현 전 PLAN을 소급 작성하지 않는다.
사용자 지정에 따라 JS 작업 기록은 `.md/JS`에 둔다. `.md/GB`는 팀장 작업 폴더다.

## 보존한 구현

- `EffectV2_Object.h`의 `UV_MODE`와 `PARAMS.eUVMode` 제거.
- `EffectV2_Document.cpp`, `Effect_Tool_V2.cpp`, `EffectV2_Object.cpp`의 enum 저장·편집·shader 바인딩 제거.
- `Shader_EffectV2_Common.hlsli`의 극좌표 변환을 제거하고 기존 UV에 tile/start/speed 적용.
- `effect_v2_binding_pipeline.py`의 `uvMode` enum 검사 제거.
- 기존 쿠크·니나브 leaf 46개의 `uvMode: Planar` 필드 제거. 다른 저작 값은 보존.
- 새 C++ 파일과 project/filter 변경 없음. 이번 세션에서 기존 코드·데이터의 bytes와 인코딩은 추가 수정하지 않음.

## 실행한 검증

- `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product` 성공.
  기존 산출물을 이용한 증분 Build·배포이며 전체 Rebuild는 실행하지 않았다.
  로그: `out/js-effect-checkpoint-build.log`.
  receipt: `out/BuildPipeline/runs/20260911T073804834Z-debug-product.json`.
- 전체 보존 작업의 변경·신규 JSON 56개 parse 성공(기존 reader의 중복 key·숫자 검사 포함).
- 변경·신규 leaf 53개는 기존 Effect V2 validator로 구조와 실제 Resources 참조 검사 성공.
- `python -m unittest Tools.EffectToolV2.test_effect_v2_binding_pipeline Tools.EffectToolV2.test_validate_effect_v2`:
  53개 중 52개 성공, 1개 실패. `test_repository_group_migration_is_dry_run_and_resolves_natural_tails`의
  고정 전체 group 목록에 현재 laser 및 기존 bingo 등 추가 그룹이 없어 비교 실패했다.
  전체 PASS로 기록하지 않으며 보존 작업에서 테스트 기대 목록을 임의 변경하지 않았다.
  로그: `out/js-effect-checkpoint-tests.log`.
- `git diff --check` 성공. 변경 XML 없음.

## 미검증

Client/UI 실행·조작·캡처와 새 시각 판정은 수행하지 않았다. 이전 Claude Code 세션의 검증은
확인되지 않아 이번 완료 증거로 추정하지 않는다. Resources와 빌드 산출물은 Git에 추가하지 않는다.
