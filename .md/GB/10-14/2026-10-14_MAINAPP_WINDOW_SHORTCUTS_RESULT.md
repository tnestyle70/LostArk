# G01 MainApp 창 단축키 분리 반영 결과

작성일: 2026-10-14. 사용자가 일부 작성한 helper를 현재 작업본에 이어서 완성했다.
[PLAN](2026-10-14_MAINAPP_WINDOW_SHORTCUTS_PLAN.md)의 전문은 마지막 컴파일 확인본과 같다.
전체 책임 분리의 미구현 단계는 [리팩터링 설계](2026-10-14_MAINAPP_RESPONSIBILITY_REFACTOR_DESIGN.md)에 있다.

## 실제 완료

- `MainApp.h`에 이미 있던 두 helper 선언을 유지하고 해당 구간의 공백만 정리했다.
- `Update_RuntimeWindowShortcuts`의 작성된 Inventory 코드에 기존 P/N/M 처리 블록을 옮겼다.
- `Sync_RuntimeWindowShortcutKeys(bool_t bFocused)`에 네 키의 현재 상태 기록을 채웠다.
- Update의 UI 표시 분기에는 처리 helper 한 번, 숨김 분기에는 상태 기록 helper 한 번을 연결했다.
- Update의 기존 네 창 블록과 숨김 분기의 중복 네 대입은 제거했다.
- 작성 중인 사운드·입력 질문 주석과 무관한 작업본 변경은 보존했다.
- `/*I 키*/` 한 곳은 `/* I 키 */`로 고쳤다. 현재 UTF-8 소스를 CP949로 읽는 컴파일 설정에서
  원래 주석은 종료 기호를 잃어 다음 P 블록의 선언을 삼켰다. 전처리 출력으로 원인을 확인했다.
- UTF-8 BOM 없음과 CRLF를 유지했다. 새 include·멤버·CPP·project/filter·JSON/XML 변경은 없다.

이 단계는 동일 CMainApp 내부의 함수 분리다. UI·사운드·도구 소유권을 별도 객체로 옮긴 상태는 아니다.

## 실행한 검증

| 검증 | 결과와 범위 |
|---|---|
| 선언·정의·호출 수 | 두 helper 각각 헤더 선언 1개, CPP 정의 1개, Update 호출 1개 |
| 이동 동작 비교 | 원래 Update 네 블록과 최종 helper의 주석/공백 제외 실행 토큰이 일치 |
| 숨김 처리 | 네 키 상태만 기록하며 Toggle/Play_Sound 없음 |
| 사용자 시작본 대비 diff | 이번에 추가한 행의 trailing whitespace 0; 변경 구간 밖 내용 보존 |
| 소스 인코딩·개행 | 두 파일 UTF-8 BOM 없음 / CRLF 유지 |
| Debug x64 CPP 컴파일 | MainApp.cpp 단독 object compile PASS, exit 0 |
| 컴파일 중 변경 감지 | CPP의 컴파일 전후 SHA-256 일치 |
| `git diff --check` 두 소스 | 기존 작업본의 MainApp.cpp:11005 주석 끝 공백 경고 1건 유지. 전체 PASS로 기록하지 않음 |

컴파일은 Client 프로젝트에서 평가한 Debug include·define·C++20 설정과 MSVC 14.44.35207을 사용했다.
`out/MainAppRefactor/isolated-debug`에 별도 object를 만들고 `/Y-` 및 강제 PCH 헤더 include로
본 CPP를 확인했다. 제품 출력·SDK·기존 빌드 중간 파일을 교체하지 않았다.
기존 C4819 문자 집합 경고는 남아 있다. 전역 인코딩 정리는 별도 단계다.

다른 작업의 Release 빌드와 실행 중인 Server를 확인했으므로 제품 빌드/링크를 중복 시작하거나
프로세스를 종료하지 않았다. 이번 PASS는 Debug 단일 CPP 컴파일이며 Product 전체 빌드나 Release 검증이 아니다.

## 검증 자료

- `out/MainAppRefactor/window-shortcuts/MainApp.cpp.before`, `MainApp.h.before`: 사용자 시작본.
- 같은 폴더의 `*.delta.patch`, `checks.json`, `manifest.json`: 실제 변경과 정적 비교·hash.
- `out/MainAppRefactor/isolated-debug/compile.log`, `result.json`: 최종 컴파일 결과.
- `inline-comment-encoding-failure.log`, `MainApp.i`: 주석 종료 손상 때의 실패와 전처리 근거.
- `initial-include-command.log`: 첫 격리 컴파일 명령의 include quoting 실패. 명령을 고친 뒤 재검증했다.

최종 CPP SHA-256: `0e35d26fc658229f56753a497895b9ed26c77b8a0bd5961acf06af0a699d76f5`.
최종 H SHA-256: `6707f5dd01b8c72eee0ffd02d53e29fe2dba34d4c0770fde56666b486dc7cbf6`.
scratch와 object/log는 소스 커밋 대상이 아니다.

## 미완료와 사용자 확인

- Product 전체 링크·Release 컴파일은 이번 변경으로 수행하지 않았다.
- Client 실행·UI 조작·화면 및 청취 판정은 수행하지 않았다.
- 사용자는 I/P/N/M을 각각 눌렀을 때 한 번 토글, 누른 채 반복 없음, 문자 입력 중 열림 없음,
  시네마틱 중 열림 없음, 종료 후 놓았다 다시 누른 입력을 확인한다.
- 사운드 cue 분리, 선적재 API, ValtanAxe/Dragon/Profiler와 다른 MainApp 책임 이동은 아직 구현하지 않았다.
- 대규모 기존 미커밋 변경이 있는 작업본이며 이번 작업으로 commit/push하지 않았다.
