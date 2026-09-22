# 발탄 Flow 컷신 추가 수정 계획

## 목표

Boss Tool의 Pattern Route Editor에서 `발탄 등장 컷신`을 선택했을 때 거부되지 않고, 기존 Flow를 보존한 채 유일한 진입 노드로 추가한다. 다른 컷신과 일반 패턴은 기존처럼 선택 노드 뒤에 추가한다.

## 확인된 원인

- 그래프 편집기는 모든 후보를 `Insert_Node_After()`로 전달한다.
- `Insert_Node_After()`는 `VALTAN_ENTRANCE_CINEMATIC`만 조건 없이 실패시켰다.
- 구형 선형 편집기의 `Add_Slot()`은 같은 패턴을 Flow 맨 앞에 추가할 수 있어 두 편집 경로의 계약이 달랐다.
- 저장 검증은 등장 컷신을 최대 1개, 반드시 `entryNodeId`로만 허용한다. 따라서 일반 노드처럼 선택 행 뒤에 넣는 것이 아니라 새 entry로 승격해야 한다.

## 수정 계약

1. 등장 컷신이 없으면 stable node/edge ID를 각각 하나 발급한다.
2. 새 등장 노드를 `entryNodeId`로 지정하고 `등장 → 기존 entry` COMPLETED edge를 만든다.
3. 기존 노드·edge·반복 구조는 변경하지 않는다.
4. 전체 문서를 검증한 뒤에만 draft를 교체한다.
5. 이미 등장 컷신이 있으면 중복 추가를 거부하고 기존 draft를 그대로 유지한다.
6. C++ 실행형 하네스로 추가, 기존 entry 연결, 중복 실패 rollback을 검사한다.

사용자 화면 확인 전에는 UI 동작을 PASS로 기록하지 않는다.
