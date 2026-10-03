# Object 목록의 Arena 진입 후 로드 복구 결과

## G00. 원인과 수정

사용자가 첨부한 화면은 쿠크 아레나에 진입했지만 Object 목록에 진입 전 오류가 남아 있었다.
기존 Open은 실패에도 m_Open=true를 남겼고 Begin_WorkbenchFrame은 열린 창의 source를
다시 로드하지 않았다. Action Workbench 도킹 탭이 가려져 수동 Reload도 목록에서 보이지 않았다.
현재 WorldSequence source에는 Object470개·instance345개·template287개가 있다.
이번 화면의 안내문은 source 파일 읽기 이전의 Arena 검사에서 발생한다.

WorldObjectTool H/CPP에 최초 로드 지연 상태를 추가했다. CurrentLevelID와 Arena 포인터가
모두 준비된 frame에서 최초 로드를 한 번 재시도한다. 생성자에서 포인터만 설정된 Loading은
계속 기다리며, 실제 파일 읽기를 시작하면 지연 상태를 해제해 데이터 오류를 반복하지 않는다.
이미 로드한 document와 dirty draft는 자동 재로드하지 않는다. 미로드 Object 목록에는
기존 Render_Toolbar를 재사용해 Reload Source와 기존 dirty 확인 동작을 노출했다.

새 C++/project/filter 등록과 저작·게시 데이터 변경은 없다. 기존 네트워크 변경과 사용자 파일은
보존했다. 관련 팀 인계 문서와 gotchas에 창 열림과 source 준비 상태의 구분을 기록했다.

## G01. 실행한 검증

- 실제 Open/Begin_WorkbenchFrame 본문과 Load_Source 활성 guard를 추출한 독립 CPU probe:
  20 checks, failures0. 파일 읽기 이후는 stub이며 GPU/UI·source codec 검사가 아니다.
  Lobby·Loading·실제 활성·실패 후 반복·수동 재시도·ready/dirty 보존을 확인했다.
  변경 전 HEAD의 실제 본문에서는 진입 후 로드 누락과 Loading 중 조기 로드를 재현했다.
- WorldObjectTool.cpp를 프로젝트의 실제 Debug CL tlog 옵션에서 out 전용 OBJ/PDB로
  분리하여 컴파일, exit0. 공유 PCH를 사용하지 않고 표준 forced include는 유지했다.
  초기 VS 기본14.51 include/14.44 compiler 불일치 및 공유 PCH/PDB 검증 실패는
  `-vcvars_ver=14.44`, `/Y-`, `/Z7`로 격리 명령을 교정했다. 제품 설정은 바꾸지 않았다.
- 기존 `test_linked_save_uses_area_lock_and_stale_source_cas`: PASS.
- 초기 검증의 `test_combined_object_selection_retains_full_motion_editor`: FAIL.
  Render_GroupSequence 내부에서 track.keys를 직접 찾는 오래된 문자열 검사이며 실제 구현은
  Render_TimelineRows로 위임한다. 변경 전 HEAD에서도 같은 실패를 재현했다.
  이후 사용자가 요청한 타임라인 양식 통일에서 이 테스트를 실제 delegate 경로에 맞춰 수정했고
  key/animation/effect 편집 내용과 caller 연결 검사가 PASS했다.
- C++ UTF-8 BOM 없음·CRLF 유지, `git diff --check` PASS.

컴파일 로그와 OBJ는 `out/WorldObjectSourceActivation20261003/`, CPU 검증과 소스 hash는
그 아래 `review/`에 있다. 독립 agent의 dirty 사전 검토에서도 추가 결함을 발견하지 못했다.

## G02. 실행 반영과 사용자 확인

수정 당시 Debug Client PID53880와 Server PID55144가 실행 중이었다. Client의 EXE/PDB를
자동 덮어쓰거나 프로세스를 종료하지 않았으며, 사용자에게 편집 저장 후 Client 종료를 요청했다.
사용자가 전체 반영을 요청하고 두 프로세스가 종료된 뒤 정식 Debug Product 빌드로 반영했다.
`out/BuildPipeline/runs/20261003T053410131Z-debug-product.json`이 PASS, exit0이다.
이어 Release Product 빌드도 PASS/exit0이며
`out/BuildPipeline/runs/20261003T053612726Z-release-product.json`에 기록했다.
서버/프로토콜/데이터가 바뀌지 않으므로 Server 종료·publish는 필요하지 않다.

현재 실행 버전의 복구 경로는 오른쪽 위 Action Workbench 탭 → Reload Source다.
수정본에서는 Lobby에서 Object를 먼저 연 뒤 쿠크에 입장해도 목록이 로드되어야 한다.
미로드 상태에서는 Composition Actions의 Object 목록에서 직접 Reload Source를 사용할 수 있다.
실제 목록·영상 촬영 화면·GPU 결과는 사용자가 확인한다. Client/UI 실행·조작·캡처는 하지 않았다.
