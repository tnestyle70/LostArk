# Boss·Sequence·World Sequencer Undo/Redo 결과

## 구현 상태

기존 Boss Saydon과 Sequence는 이미 공용 `CEditorUndoHistory` 및 toolbar/단축키를 사용했다.
World Movie에는 이 연결이 없었다. `origin/main`의 `0164be5a9`를 현재 빙고 작업 위에 병합한
`codex/sequencer-undo-redo`에서 World 연결을 추가했다. main의 Movie lane 통일과 기존 빙고를 보존한다.

World의 재생 toolbar와 Sequence Camera Tool에 Undo/Redo, Ctrl+Z, Ctrl+Y/Ctrl+Shift+Z를 연결했다.
기존 `CClassSelectionPresentation`가 row·timing move/trim·camera cut·Repeat movie·World 제외/복원
이력을 소유한다. 별도 player나 저장 경로를 추가하지 않았다. 연속 camera 입력은 한 단계로 묶고,
class·phase·stable box/key·Movie cursor를 함께 복원한다. World 모델 선택은 기존 inspector SELECT와
동기화하며 키 추가/삭제는 편집 전·후 key 선택을 나눠 기록한다.

Undo/Redo는 재생을 정지한 뒤 메모리 draft를 복원한다. 일반 Save의 최신 baseline·World revision은
되돌리지 않으며 기존 stable-ID 3-way Merge로 이력의 변경 필드만 적용한다. 외부의 무관한 변경은
보존하고 같은 필드 충돌은 거부한다. Validate/Prepare/Commit 실패 시 현재 draft와 stack을 유지한다.
명시 Reload 성공·Level 종료가 이력 경계다. 미적용 row·텍스트 입력·진행 중 gesture·publish는 보존한다.
Effect element 내부 편집은 기존 Effect Tool 이력을 사용한다.

## 자동 검증

- Debug Product Build PASS: `out/BuildPipeline/runs/20261009T011847055Z-debug-product.json`.
  Engine/Shared/Server/Client를 정상 runner로 빌드·배포했다. Client 단계118454ms,
  전체 OBJ232/CSO0/PCH0, tracking identity 변경 없음이다. 기존 인코딩 warning은 남아 있다.
- 마지막 Reload/선택 전환의 카메라 선택 buffer 정리도 Debug Product로 다시 컴파일·링크했다.
  최종 보고서는 `out/BuildPipeline/runs/20261009T011940909Z-debug-product.json`이며
  Client 단계6464ms, OBJ1/CSO0/PCH0/EXE1이다. 로그는 `out/SequencerUndoRedo20261009/product-debug-final.log`다.
- `python out/SequencerUndoRedo20261009/build_probe.py --run`:68 checks/0 failures.
  실제 production history·Apply·timing·Repeat·exclude·Merge·Save·Reload 함수 본문을 추출하여
  입력·Undo/Redo·Save 이후 baseline/revision·외부 변경·동일 필드 충돌·실패 후 재시도를 확인했다.
  Save는 `out/SequencerUndoRedo20261009/sandbox`에만 실제 원자 교체를 수행했다.
  runtime Parse/Validate/Prepare/Commit·resource lookup·camera sampling은 실패 주입용 stub이다.
  따라서 이 검사는 실제 모델 admission·GPU·화면 검증이 아니다.
- 실제 원본 두 JSON13,338,429 bytes를 읽어17편집 후16단계 제한과 cached snapshot 재사용을 확인했다.
  idle Finish10,000회에서 새 snapshot 생성이 없었다. 두 원본의 전후 SHA256은 동일하다.
  근거는 `run.log`, `memory.json`, `provenance.json`, `source-before.json`, `source-after.json`이다.
- Client/Server project/filter XML4개 parse, C++6개 UTF-8 noBOM/CRLF, `git diff --check` PASS.
  새 C++ 파일이나 project 등록, 저작 JSON·게시 데이터·Resources 변경은 없다.
- 독립 WIP 코드 검토에서 발견한 World inspector 선택 불일치와 Camera Add key 이전 선택 소실을
  실제 경로로 대조·수정했다. 마지막 검토 범위에서 추가 P1/P2는 없었다.

## 남은 화면 확인과 비용

Client/UI 실행·자동 조작·캡처는 하지 않았다. Debug·Release 통합 빌드는 아래 main 병합 검증에서 완료했다.
사용자는 새 Debug Client의 F1 → Action Workbench에서 Boss Saydon, Sequence, World를 각각 열어
box 편집 후 Undo/Redo를 확인한다. World는 Character Select에 입장한 뒤 Intro/Loop의 camera key와
box timing, 모델 제외/복원, Save 이후 Undo, Reload를 확인한다. 미적용 row가 있으면 먼저 Apply/Revert한다.

World 이력은16단계 compact JSON 문자열이며 인접 상태는 shared snapshot을 재사용한다.
대형 문서의 실제 변경 순간에는 직렬화 비용이 있다. 최적화 없는 독립 fixture에서17회 Capture/Finish는
12.315초, private memory 증가는286.2MB였다. 이는 실행 중 Client 프레임 수치가 아니며
사용자의 실제 편집 반응·최종 영상 판정은 남아 있다. Build 산출물과 out 진단은 소스 commit에서 제외한다.

## 최신 main 병합 검증

사용자의 전체 작업 main 병합 요청에 따라 최신 `origin/main`의 `033e5621b`(PR538)를
통합 브랜치에 충돌 없이 병합했다. 검증한 코드 commit은 `13ae933b3`이다. PR537의 Bingo
Parent·Box Detail 작업과 PR539의 World Undo/Redo를 함께 검증했으며, main의 Release F7 profiler,
Bern 전용 culling·입장 카메라 FOV, camera key 목록 높이와 설명 주석을 보존했다.
작업 시작 당시 미커밋 변경은 없었다. protocol133과 기존 렌더링 옵션도 유지했다.

- Debug Product PASS: `out/BuildPipeline/runs/20261009T022349090Z-debug-product.json`.
  전체167872ms, Engine/Shared/Server/Client OBJ는 각각34/3/94/263개다.
- Release Product PASS: `out/BuildPipeline/runs/20261009T022649180Z-release-product.json`.
  전체179821ms, Engine/Shared/Server/Client OBJ는 각각35/3/94/271개다.
- 두 구성 모두 PCH0/CSO0, 각 project의 tracking identity 변경 없음이며 정상 증분 Build로
  컴파일·링크·SDK/shader/DLL 배포를 완료했다. 기존 인코딩/PDB warning은 남아 있다.
  기본 runner의 runtime 입력 누락·오류는 모두0개이며 domain publish나 Client 실행은 하지 않았다.
- shader/project 입력671개의 병합 전후 SHA256과 mtime가 동일했다.
  근거는 `out/MainMerge20261009/shader-inputs-before.json`과 제품 빌드 로그다.
- 바뀐 JSON3개, Engine/Client/Server project/filter XML6개 parse와 `git diff --check` PASS.
  `python -m unittest Tools.ValtanPipeline.test_bern_entrance_camera_contract`:4 tests PASS.
- 독립 병합 검토에서 최신 main과 두 기능의 실제 연결을 대조했고 추가 충돌이나 P0/P1 회귀
  근거는 발견하지 않았다. 빌드 PASS를 Client 화면·영상 또는 전체 gameplay 검증으로 확대하지 않는다.
