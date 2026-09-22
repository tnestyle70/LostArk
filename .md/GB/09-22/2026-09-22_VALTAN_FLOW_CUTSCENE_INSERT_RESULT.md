# 발탄 Flow 컷신 추가 수정 결과

## 원인과 수정

`Add From Pattern Slot...`은 후보를 정상적으로 표시했지만 그래프 mutation 함수가 `VALTAN_ENTRANCE_CINEMATIC`을 무조건 거부했다. 데이터 누락이나 카메라 연결 문제가 아니라 선형 편집기와 그래프 편집기의 진입 컷신 추가 규칙이 서로 달랐던 문제다.

이제 발탄 등장 컷신을 선택하면 선택 행 뒤에 반복 패턴으로 넣지 않고 Flow 맨 앞의 유일한 entry로 추가한다. 새 entry에서 기존 entry로 COMPLETED edge를 만들기 때문에 기존 60개 패턴의 순서와 연결은 그대로 유지된다. 이미 등장 컷신이 있으면 두 번째 추가는 실패하며 draft는 바뀌지 않는다. 다른 컷신과 일반 패턴의 삽입 방식은 변경하지 않았다.

## 변경 파일

- `Client/Private/ValtanPatternFlowDocument.cpp`
- `Tools/ValtanPatternAuditionServiceHarness/Private/ValtanCanonicalGraphContractTests.cpp`
- `Tools/ValtanPipeline/test_valtan_boss_tool_pattern_flow_contract.py`

## 검증

- `test_valtan_boss_tool_pattern_flow_contract.py`의 진입 컷신 삽입 focused test: PASS (`1 test`).
- `ValtanPatternAuditionServiceHarness` Debug x64 빌드: PASS.
- `Client/Default/Client.vcxproj` Debug x64 빌드: PASS. 새 `Client/Bin/Debug/Client.exe`에 수정이 반영됐다.
- 네이티브 실행 결과 중 `ValtanPatternFlowDocumentContractTests`: PASS. 진입 컷신을 새 entry로 추가하고 기존 entry를 잇는 edge, 중복 추가 rollback을 실제 C++ 문서 구현으로 검사했다.
- 전체 하네스 프로세스는 실행 중인 제품 데이터의 writer lock을 열지 못한 `Win32 error 5`와 기존 Presentation admission 항목 때문에 종료 코드 1이었다. 이번 Flow 문서 계약 테스트 실패는 아니다.
- 전체 Python 파일에는 이번 수정과 무관한 기존 Server 활성 시퀀스 문구 계약 실패를 포함해 2개 실패가 남아 있어 전체 PASS로 기록하지 않는다.
- 변경 대상 파일의 `git diff --check`: PASS.
- 실제 UI 동작은 사용자 화면 검증 전까지 미확인이다.

## 사용자 확인 경로

새로 빌드된 Debug Client를 실행한 뒤 Boss Tool의 Pattern Route Editor에서 `Add From Pattern Slot...`을 열고 `발탄 등장 컷신`을 선택한다. 해당 컷신은 선택 행 뒤가 아니라 `01`번 `[First]`로 들어가고, 기존 `01`번은 `02`번으로 밀려야 한다. 같은 컷신을 다시 추가하면 목록이 바뀌지 않아야 한다. 다른 컷신과 일반 패턴은 기존처럼 선택한 패턴 뒤에 추가된다.

이 수정은 편집기 C++ 로직만 바꾸므로 publisher 대상 데이터는 없다. 실제 화면에는 새 Client 빌드가 필요하며, 저장은 기존 `Save Flow` 경로를 그대로 사용한다.
