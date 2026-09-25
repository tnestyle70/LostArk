# World Object Parent와 범위 선택 결과

## G00. 구현과 저장 계약

Object Resources에서 Ctrl 클릭은 개별 항목의 추가·해제, Shift 클릭은 현재 검색 결과와
펼침 상태에 따른 Object/Parent 행 범위 선택이다. Ctrl+Shift는 기존 선택에 범위를 더한다.
이미 선택한 항목을 우클릭하면 다중 선택을 유지하고 `Create Parent`, 그 아래
`Move to Parent`를 표시한다. 같은 두 명령을 목록 상단 버튼으로도 제공한다.

Create Parent는 입력한 이름으로 폴더를 만들고 선택한 가지를 포함한다. Move to Parent는
기존 폴더·Object 또는 category root를 선택한다. 사용자가 요청한 새 갈고리 두 개를 기존
`월드오브젝트_갈고리` 아래로 옮길 수 있다. 상위와 자식을 함께 선택하면 최상위 선택만 옮겨
내부 소속을 보존한다. 부모 하나를 선택한 Create Object는 그 아래에 생성하고 해당 category를 따른다.
변경한 가지는 검색을 해제하고 부모를 펼쳐 확인할 수 있게 한다. Parent 이름도 Detail에서 편집한다.

정리 정보는 WorldSequence v3 optional `objectFolders`와 Object `parentId`에 저장한다.
폴더/Object의 ID 중복, 없는 부모, category 불일치, 순환 및 64단계 초과는 기존 문서를
보존하며 거부한다. 모델 지정 전 draft는 계층만 검사하고 Save는 기존 전체 검증을 적용한다.
기존 모델·위치·개수·Motion ID와 binding은 그대로이며 `motionInstanceIds` 합성 재생과 분리한다.
계층에 연결된 Object의 Anchor Type 변경은 먼저 연결을 root로 풀도록 안내한다.

기존 Save의 stage → native Load → Is_Equivalent → CAS/rollback → Map publisher를 사용한다.
이번 작업에서 사용자의 저작 JSON을 직접 옮기거나 게시하지 않았다.

## G01. 실행한 검증

- WorldObjectTool와 WorldSequenceDocument의 out 전용 x64 Debug 컴파일 성공.
- 기존 Python 계약 검사 3개 통과: 새 hierarchy publisher, 기존 object publisher,
  합성 Motion 편집기 회귀. 새 검사 내부의 20개 사례는 기존 문서·정상 계층의 반환 값 보존,
  잘못된 참조·순환·ID·anchor·깊이 제한을 검증했다.
- 실제 WorldSequenceDocument/DataJson codec와 실제 도구 함수 본문을 사용한 native 검사
  39개, 실패 0. 정·역방향/추가/필터 범위 선택, Parent 생성, 기존 Object 아래 이동,
  상하위 동시 선택, Motion/TRS 보존, Save/Load/equality, 실패 Load·Save의 기존 값 보존,
  Reset 및 선택한 부모 아래 Create Object를 확인했다. preview/profiler는 stub이며 GPU/UI 검사가 아니다.
- PowerShell parse, 관련 diff 및 전체 `git diff --check` 성공. C++ UTF-8 BOM 없음·CRLF 유지.
- 사용자가 저장·종료했다고 확인한 후 정상 Debug Product Build 성공. Engine→Shared→Server→Client,
  Client OBJ 176개·EXE 1개 갱신, PCH/CSO 쓰기 0. compiler/link 오류 없음.
  기존 C4819/C4828 코드페이지 경고는 별도이며 소스 인코딩을 일괄 변환하지 않았다.
- 제품 결과: `out/BuildPipeline/runs/20260924T105553839Z-debug-product.json`.
  실행 파일: `Client/Bin/Debug/Client.exe`. 데이터 publish는 실행하지 않았다.

검증 파일은 `out/WorldObjectParent20260924/`의 Product/최소 compile 로그,
`review/verification.json`, `review/run.log`, native fixture와 source hashes다.

## G02. 사용자 확인과 전달 범위

F1 → World Object Tool → Map에서 첫 행 클릭 후 열 번째 행 Shift 클릭,
우클릭 Create Parent와 이름 입력을 확인한다. 새 갈고리 두 개는 Ctrl 클릭으로 고른 뒤
우클릭 Move to Parent → `월드오브젝트_갈고리` → Move를 누른다. Save/Reload 후 소속 유지와
기존 개수·위치·Motion 재생은 사용자가 직접 확인한다. Client/UI 실행·조작·캡처를 하지 않았다.

변경은 기존 WorldObjectTool H/CPP, WorldSequenceDocument H/CPP, Map publisher와 기존 검사,
Area 가이드, gotchas의 추가 항목 및 이 PLAN/RESULT다. 새 C++ 파일·프로젝트 등록·Resources
변경은 없다. 공유 worktree의 무관한 미커밋 변경은 보존했고 자동 commit/push는 하지 않았다.

후속 카드미로 피킹 수정까지 포함한 최종 Debug Product Build도 PASS다.
최종 결과는 `out/BuildPipeline/runs/20260924T111810242Z-debug-product.json`이며
실행 파일과 Engine.dll은 `Client/Bin/Debug`에 반영했다.

## G03. 후속 갈고리 복제 가능 여부 조사

사용자는 새 갈고리 Object 두 개에 `원본_갈고리`의 잡기·끌기 동작을 복사하고 위치와 방향을
수정할 수 있는지 확인을 요청했다. 아래는 공용 Sequencer 확장 전 조사 시점의 상태다.
이후 구현한 독립 복사·붙여넣기는
[공용 Sequencer 결과](2026-09-24_SHARED_SEQUENCER_RESOURCE_TRANSFER_RESULT.md)를 따른다.

현재 원본 Motion `sequence.kouku.hook.original_preview`는 emission 15개, hook animation
4개와 `HOOK_CAPTURE` collider를 가진다. collider의 `attachmentBone=b_hook_01` 및 grip을
publisher가 실제 emission/occurrence transform과 함께 계산해 Server의 포획·끌기로 게시한다.
특정 표시 이름이나 원본 Object ID에만 동작을 묶은 구조는 아니다.

- 같은 Motion 안에서 Authored Emissions의 Dup 후 Offset XYZ/Yaw/Delay를 바꾸는 기능은
  이미 있다. 각 emission에 기존 collider가 적용된다.
- Composition의 `원본_갈고리` WORLD 박스를 Ctrl+D 또는 Duplicate한 뒤 박스의 MAP 위치·회전을
  바꾸면 동일 묶음을 별도 장소에 놓을 수 있다. 같은 Motion을 참조하므로 개수·동작 편집은 공유한다.
- Create Object/Create Motion은 빈 resource와 기본 2-key Motion을 만든다. 이번 Parent 이동은
  조직 정보만 바꾸며 model·Motion·Collider를 상속하거나 복사하지 않는다.
- 조사 당시에는 전체 Object/Motion 복제 명령이 없었다. 후속 구현은 새 ID로
  Object/Template/Instance 및 animation·collider를 함께 복제해 개수·동작을 독립 편집한다.
- 실제 포획 확인은 저장한 Composition에 Motion을 연결해 게시하고 Saved Pattern의
  Play (with Collisions)를 사용한다. 일반 Play는 로컬 시각 미리보기다.

현재 디스크의 WorldSequence source/runtime에서 새 두 이름의 갈고리 resource는 확인되지 않았다.
이를 사용자 메모리 draft의 부재로 간주하거나 저작 데이터를 임의로 생성·교체하지 않았다.
