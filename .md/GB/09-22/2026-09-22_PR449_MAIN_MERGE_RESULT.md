# PR 449 main 충돌 해결 결과

## 통합 기준과 실제 수정

`GB/collider-pattern-bug-fix`의 `82938564111e434210cf0dbe9f35b8c8f4e62a5e`에 main `beb1bcb3dd2ac10e100c4a74389a12a4efaa6026`를 병합했다. 시작 worktree는 clean이며10개 충돌 파일을 재현했다. V2 PNG WIC 수정은 기존 PR에 들어 있으며 병합에서도 유지한다. Resources·사용자 draft·Client UI는 변경하지 않았다.

프로젝트와 filter는 양쪽 추가를 유지하고 branch에서 삭제한 기존 EngineMeshLodShaderSource copy 항목은 복원하지 않았다. 부모/공통 조상의 Include3-way 집합이 정확히 일치한다. EffectCatalog는 branch12개와 main5개 추가를 포함한1328개 stable ID의3-way 결과와 일치하며 중복이 없다. CharacterPreviewPanel은 main의 Valtan yaw/축별 scale과 branch의 Character recenter/Bind_Preview를 함께 유지한다.

## Native material 충돌

양쪽 program4567..4571/4576..4585는 같은 숫자에 다른 material/PS/layout을 배정했다. main Esther ID와 재질은 유지하고 branch Guardian15개만4633..4647로 옮겼다. 네 authored 문서의 runtimeShaderProfileId20곳, C++ texture/parameter/switch/descriptor, HLSLI 함수·dispatch·selected lookup과 distortion companion4개를 같은 mapping으로 갱신했다. 실제 source material/parent/profile identity, 수식·texture·TRS는 보존했다.

4544/4608 그룹이 병합 대상이다. 기존 main4608 Mesh/Particle wrapper와 upper bound4671을 사용한다. main descriptor2120개 보존, 통합2135개 ID 유일성, 두 group의 함수·case·lookup agreement를 확인했다. tracked generator에 구 ID를 고정한 추가 consumer는 없었다. 상세 mapping은 `out/PR449Merge20260922/shader/remap.json`, byte·구조 비교는 `semantic_audit.json`과 `material-binding-validation.json`에 있다.

## Protocol 통합

두 parent는 각각 다른 wire를 protocol104로 사용했다. branch의 vehicle flight move/snapshot과 main의 Debug Esther request를 모두 포함하는105로 올렸다. 기존97개 enum entry(INVALID 포함)의 숫자는 보존하고 과거104 handshake를 거부한다. reader/writer·Client typed command·Server decode/dispatch를 대조했으며 현재 팀 사용서와 CLAUDE 실행 버전도105로 맞췄다. 과거 RESULT의 당시 버전은 보존한다.

## 실행한 검증

- JSON/XML23개 parse와 변경 source의 conflict marker0.
- project/filter Include3-way 집합과 catalog stable-ID3-way 비교 PASS.
- Guardian20개 참조의 sourceMaterialPath/parentMaterialPath/profileId가 remapped table과 일치하며 main Ninave authored payload는 원문과 같다. inverse remap은 네 문서의 수정 전 bytes를 정확히 복구한다.
- Mesh/Particle4544·4608 네 FXC /T fx_5_0 /O1 compile PASS. 출력은 `out/PR449Merge20260922/shader`에만 생성했다.
- isolated NetworkProtocolHarness:1257 PASS, failures0, exit0. flight codec과 Debug Esther roundtrip/invalid rollback/104 handshake rejection을 포함한다. `out/PR449Protocol/protocol-result.log`.
- 발탄 MapTool 컷신 연결5종 검사 PASS. 위치·camera source clock·Server Exact·canonical placement 연결을 확인했다.
- `git diff --check` PASS. Product Debug Build와 PR 반영 최종 상태는 아래에서 기록한다.

## 제품 실행과 남은 화면 확인

Client/UI를 실행하지 않았다. 실제 아레나/Complete Play와 Guardian·Esther의 시각 판정은 사용자 확인 대상이다. protocol105 Client/Server는 함께 재시작해야 한다. source 병합·자동 검증과 사용자 화면 결과를 구분한다.

## 충돌 해결 커밋 시점의 Product 상태

사용자가 우선 PR을 갱신한 뒤 main으로 동기화하는 순서를 요청했다. 공식 Product Debug는 Engine·Shared·Server compile/link PASS이며 Client의 FXC 단계가 진행 중이다. 발견된 컴파일 오류는 없지만 전체 Product PASS로 기록하지 않는다. 전체 로그는 `out/PR449Merge20260922/product-build.log`이며 완료 결과는 후속 기록으로 분리한다. 이 변경은 기존 PR449 브랜치에 push하여 같은 PR을 갱신하고 GitHub의 mergeable 상태를 확인한다.
