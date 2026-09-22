# 고대의 바다 SPACE Element 삭제 구현 계획

## G01. 현재 삭제 경로

기준은 `GB/collider-pattern-bug-fix`의 `d05a56c1d`다. 사용자 수정 중인 쿠크 Composition과 Effect JSON은 보존한다. 고대의 바다 SPACE `effect.vehicle.vehicle_ancient_sea.98520.0.full.restore`는 Trail 3개와 Particle 4개이며, 요소 간 provider/master 의존성이 없다. Trail 삭제 후에도 본을 따라가는 Particle이 남는다.

`Try_LoadDocumentPathStaged`는 GPU와 모델 준비를 명시적 Play까지 지연한다. 반면 `Try_DeleteSelectedElement -> Try_CommitDocument`는 재생 여부와 관계없이 `Stage_WorldPreview`를 호출하여 아직 선택하지 않은 본/preview model 때문에 유효한 삭제를 거절할 수 있다. 마지막 Family 요소를 삭제해도 `SOLO_AUTHORING_FAMILY` 필터가 남는 경로도 있다.

## G02. Effect_Tool_Editing.cpp

`Try_CommitDocument`는 기존 codec 검증을 먼저 수행한다. 활성 Sequencer 또는 표시 요청된 world preview가 있을 때만 기존 stage/rollback 계약을 사용한다. 재생하지 않는 문서 편집은 CPU 검증 뒤 commit하고 GPU 준비는 Play에 남긴다. source/runtime read-only 및 미적용 Detail guard는 유지한다.

`Try_DeleteSelectedElement`는 삭제 후 선택 Family가 비면 COMPLETE로 전환한다. stage 실패 시 filter/family/선택/marks/문서를 모두 이전 상태로 유지한다. 실제 Effect 파일 삭제나 Save/Publish를 수행하지 않는다.

## G03. 검증

현재 C++ 함수 본문을 사용하는 작은 native probe에서 preview 미시작 삭제, 활성 preview 갱신, 실패 rollback, 마지막 Family 삭제와 family 복원을 확인한다. 고대바다 실제 JSON은 읽기 전용으로 검사한다. 기존 관련 focused Python 검증과 `git diff --check`를 실행한다. 새 제품 C++ 파일이 없으므로 프로젝트/filter 등록은 필요 없다. Product 컴파일은 root 세션에서 통합한다. Client 실행과 화면 확인은 사용자에게 남긴다.
