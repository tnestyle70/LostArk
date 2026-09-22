# Clown·MAZE 이펙트와 Ctrl 핑 구현 계획

## G00. 현재 기준과 적용 경계

기준은 GB/collider-pattern-bug-fix의 d05a56c1d다. 시작 시 Composition의 사용자 변경과 실행 중인 Client를 확인했다. 저작 데이터는 후보로 준비하고, 최종 저장본 반영 시에만 저장·반영 의사를 확인한다. 코드 변경과 구조 검증은 먼저 수행한다.

Clown POLYMORPH에는 아직 effectAssetId가 없고, 일곱 클래스 MAZE Q/LMB는 빈 공유 Effect를 참조한다. interactionbinding의 기존 Effect 및 timeline 소비자를 사용한다. 각 세부 후보의 source·asset 연결은 대응 RESULT에 기록한다.

## G01. PlayerController의 Ctrl 대기와 클릭 소비

PlayerController.h의 입력 gate 옆에 CPING_INPUT_GATE를 둔다. Ctrl을 새로 누르면 대기하고, Ctrl을 누른 동안 다음 입력이 물리 좌클릭이면 한 번 소비한다. 다른 키/우클릭, Ctrl 해제, 입력 차단은 대기를 취소한다. 소비한 클릭은 버튼을 놓기 전까지 공격·이동·MAZE/ground targeting으로 전달하지 않는다. Ctrl+Z/X/C의 기존 Esther 입력은 유지한다.

PlayerController.cpp는 raw edge를 early return 전에 관찰한다. 지면 위치는 기존 Try_PickGroundPlane과 Character의 navigation 높이 sample을 사용한다. 핑은 로컬 표시이며 move/skill command를 제출하지 않는다.

## G02. ClickMoveEffect의 표식 수명

기존 CClickMoveEffect에 지면 핑 handle과 Ctrl 대기 머리 위 과녁 handle을 추가한다. 과녁은 실제 Character의 기존 head anchor를 따라가고, 핑은 navigation 높이 바로 위에 3초 유지한다. Level 변경·사망·입력 잠금·owner 소멸 시 정리한다. 기존 EffectPresentationService의 준비·spawn·update·stop 경로를 재사용한다.

원본 Resources에 과녁 texture fx_l_symbol_07_cl과 핑 atlas fx_l_symbol_21_cl이 이미 설치돼 있다. 기존 native 2813/2800 descriptor와 sprite carrier를 재사용한 독립 저작 Effect 후보를 만들고, 원본 texture나 shader 수식은 변경하지 않는다. 핑 atlas의 3×3 중 가운데 셀을 사용한다. 흰 과녁과 유지시간·크기·정지 위치는 요청에 맞춘 저작값으로 구분한다.

## G03. 병렬 변경과 검증

Clown/MAZE 연결, Effect Tool 삭제 transaction, 화염파동 14개 편집 그룹은 별도 PLAN/RESULT와 후보로 준비한다. 공유 catalog·resource tree·프로젝트 None 등록은 통합 시 갱신한다. 새 제품 C++ 파일은 없다.

검증은 입력 gate의 실제 C++ 동작, 대상 Effect codec/source 검증 및 JSON/XML parse, 필요한 Product Build와 git diff --check다. Client 자동 실행·조작은 하지 않는다. 데이터 저장·도구 Reload·사용자 화면 판정을 파일 반영 및 빌드 성공과 구분한다.
