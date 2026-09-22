# 고대의 바다 SPACE Element 삭제 결과

## G01. 반영 코드

`Client/Private/Effect_Tool_Editing.cpp`의 `Try_CommitDocument`는 preview가 시작되지 않은 문서 편집을 CPU 검증 후 commit한다. `Try_LoadDocumentPathStaged`의 기존 지연 준비 계약과 맞추어, 단순 Delete가 선택되지 않은 모델의 source bone 또는 GPU 준비를 요구하지 않게 했다. 활성 Sequencer 또는 표시 요청된 world preview는 기존 stage 성공 뒤에만 commit하며 실패하면 문서를 보존한다.

`Try_DeleteSelectedElement`에서 마지막 Solo Family 요소를 삭제하면 COMPLETE로 전환한다. 검증 또는 preview stage 실패 시 family/filter/선택/marks를 복원한다. 미적용 Detail guard와 모든 Element 삭제 방지는 유지했다.

고대바다 SPACE 실제 파일은 Trail 3개, Particle 4개다. Trail 3개 삭제 후보에도 Follow Particle 3개가 남으며, 요소 간 transform master/provider 참조는 없다. 따라서 CPU-only Open 후 Delete가 preview bone 입장을 강제하는 기존 경로와 마지막 Family의 빈 preview 경로를 수정했다. 사용자가 본 정확한 오류 문구와 실제 화면 조작 순서는 확인하지 못했으므로 두 재현 경로가 사용자의 유일한 원인이었다고 단정하지 않는다.

## G02. 검증

- `out/EffectElementDelete20260922/run_probe.py`는 실제 `Try_DeleteSelectedElement`, `Try_CommitDocument` 함수 본문을 추출하여 실제 Sea JSON의 stable ID 7개를 fixture로 사용했다. MSVC C++20 단일 TU로 이전 HEAD와 수정본을 각각 컴파일했다. 모델/GPU stage와 codec 호출은 성공/실패를 주입하는 경계 대역이며 실제 Client/GPU 실행 검사가 아니다.
- 15조건 중 이전 HEAD는 6실패, 수정본은 15/15 통과했다. 비활성 preview 모델 부재 삭제, 활성 preview 갱신 1회, 마지막 Family 삭제, 남은 Family 유지, preview 실패 시 document/selection/family rollback, unapplied Detail와 codec 오류·전체 삭제 거부를 검사했다. 결과는 같은 폴더의 `before-result.log`, `after-result.log`다.
- 고대바다 실제 JSON을 읽기 전용 parse하여 Trail 3개, 삭제 뒤 Particle 4개, 그중 Follow 3개, master/provider 참조 없음 확인. 정본 SHA256은 `f9853dfbabd3730e56e7e1ba49e37e37cfc80ca8e4b14aaa76316a4a66580363`다. 파일은 수정하지 않았다.
- 기존 `test_effect_tool_lazy_metadata_contract`: 9/9 통과.
- 기존 `test_effect_tool_saved_element_clone`: 19조건 중 16통과, 2FAIL/1ERROR. HEAD source를 읽는 동일 재실행에서도 같은 결과다. 오래된 UI 문구 assertion, 전환 함수의 selection 문자열 위치, `Hash_RuntimeRandomIdentity` 호출 수 3/4 불일치다. 이번 diff와 관계없는 검사를 수정하거나 전체 통과로 기록하지 않았다.
- `git diff --check` 통과. 기존 C++ UTF-8(BOM 없음)/CRLF 유지. 새 C++ 파일 또는 프로젝트/filter 변경 없음.

## G03. 남은 확인

root 세션의 Product compile/link가 필요하다. Client와 Effect Tool은 실행·조작하지 않았다. 사용자 확인 경로는 고대의 바다 → SPACE → saved Effect Open → Trail/Ribbon 3개 mark → Delete → 남은 4개 확인이며, 저장을 원하면 Save Changes를 직접 선택한다. 저작 Effect JSON, 런타임 DataFiles, 사용자의 쿠크 Composition은 이 하위 작업에서 수정하거나 publish하지 않았다.
