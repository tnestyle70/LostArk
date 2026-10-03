# Object 폴더 정리 정보 복구 조사

## G00. 현재 저장 상태

사용자는 bg_rad_koukusaydon 계열 Object를 이전에 폴더로 나눴지만 Reload 후 평면 목록으로
보인다고 보고했다. 현재 정본은 `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/`
`LV_LUT_MIDNIGHTC_ED.worldsequences.json`, revision2290이다.
조사 기준 SHA256은 `f8acb58c0d7550822d72116c384e0ba48cc7a7c27d4c48704ca8fae52704d286`이다.

Object470개가 있고 objectFolders 필드는 없으며 parentId를 가진 Object도 0개다.
따라서 단순한 ImGui 펼침 상태 문제가 아니라 현재 디스크에 정리 계층이 없는 상태다.
현재 source는 HEAD의 blob `d9985b566d71cd9683b26260ca98a610f2ef304c`와 같다.
이전 Object 최초 로드 수정에서는 이 데이터 파일을 변경하지 않았다.

## G01. 복구 후보 조사

- 저장소 BackupData, 기존 Codex worktree, Desktop 배포 폴더 및 out의 같은 Area 후보40개:
  JSON38개 모두 폴더/부모 연결0. 나머지2개는 과거 merge conflict 원문이며 계층 필드 token0.
- Release 및 Desktop ZIP5개의 source/runtime 문서8개: 계층0.
- Git 전체 refs·reflog·merge parent를 포함한 9/24 이후 관련 commit38개에서
  source/runtime 양쪽 objectFolders/parentId 변경 이력0.
- 그중 source 변경10개 상태를 JSON으로 읽었고 모두 계층0.
- 9/27~10/3 안전 stash6개의 source도 계층0이며 untracked stash에는 해당 복구 후보가 없다.

파일 후보와 ZIP 조사 목록은 `out/ObjectFolderRecovery20261003/`에 있다.
복구 가능한 실제 사용자 폴더 이름·소속을 발견하지 못했으므로 분류를 추측해서 만들거나
전체 과거 파일로 되돌리지 않았다. 저장본·게시본·실행 중 draft를 수정하지 않았다.

## G02. 원인 판정과 사용자 작업

Create Parent와 Move to Parent는 도구 메모리의 문서와 dirty만 변경한다. 현재 구현은
Save_Source의 stage/Load/equality/CAS 경로로 objectFolders와 parentId를 저장한다.
폴더 생성·이동 자체는 자동 저장이 아니며 화면에도 Save to keep the hierarchy를 표시한다.

이번 자료만으로 과거 정리가 Save 전 메모리 상태였는지, 이후 다른 로컬 저장본으로 교체됐는지는
확정할 수 없다. 확인한 이력에는 저장된 분류가 없으므로 이 PC에서 해당 분류를 복구할 근거가 없다.
사용자가 다시 Parent를 만들고 정리한 뒤 Object의 Save를 완료하고 Reload Source로
유지 여부를 확인한다. 분류 저장만을 위해 runtime Publish를 실행할 필요는 없다.
사용자 Client/UI를 조작하거나 자동 Reload·종료하지 않았다.
