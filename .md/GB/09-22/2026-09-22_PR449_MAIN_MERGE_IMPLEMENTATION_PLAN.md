# PR 449 main 충돌 해결 구현 계획

## G00. 기준점과 병합 범위

작업 브랜치 `GB/collider-pattern-bug-fix` HEAD는 `82938564111e434210cf0dbe9f35b8c8f4e62a5e`, main은 `beb1bcb3dd2ac10e100c4a74389a12a4efaa6026`, 공통 조상은 `bcea95d886be4130207a2fc49092b144753e8c78`다. 시작 tracked/untracked 상태는 clean이며 main을 no-commit merge해 충돌10개를 재현했다. 기존 PNG WIC 수정과 양쪽 기능을 보존하고 PR 브랜치만 갱신한다.

## G01. 등록과 미리보기

Client project/filter는 stable Include를 기준으로 추가 항목을 합치되 branch가 제거한 오래된 Engine shader 복사 항목은 되살리지 않는다. EffectCatalog는 effectAssetId별3-way 비교로12개 branch 추가와5개 main 추가를 유지한다. CharacterPreviewPanel은 main의 Valtan yaw/축별 scale과 branch의 Character recenter/Bind_Preview를 연결한다.

## G02. Native material ID

동시에 발급된15개 Guardian program4567..4571/4576..4585가 main Esther program과 충돌한다. main ID는 유지하고 Guardian만4633..4647로 이동한다. Tables, 4544/4608 HLSLI 함수·dispatch·selected switch 및 해당 Guardian authored 문서4개의 runtimeShaderProfileId를 함께 바꾼다. 재질식·텍스처·source identity·TRS는 유지하며 ShaderFamily의 기존4671 상한과4608 wrapper를 소비한다. Resources는 변경하지 않는다.

## G03. 네트워크

양쪽 protocol104는 서로 다른 wire를 뜻한다. 통합은105를 사용하며 flight move/snapshot과 debug Esther command를 모두 보존한다. protocol harness에 reader/writer·부정 입력 검증을 연결하고 현행 팀 사용서의 버전을 일치시킨다. 과거 PLAN/RESULT의 당시 버전은 수정하지 않는다.

## G04. 검증과 PR 반영

JSON/XML parse, catalog/project3-way 비교, material ID·profile 참조 일치, 관련 shader compile, protocol harness 및 Product Debug Build를 실행한다. 미해결 index와 conflict marker가0인지 확인하고 merge commit을 기존 PR 브랜치에 정상 push한다. GitHub mergeable 상태를 다시 조회한다. Client/UI 실행과 실제 main PR merge는 수행 범위에 포함하지 않는다.
