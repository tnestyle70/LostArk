# 쿠크 Sequencer 전체 lane 선택·구간 복제

## G00. 현재 경로와 목표

Render_Timeline의 marquee hit box와 Duplicate_TimelineSelection은 Stage/Animation만 처리한다.
다른 lane은 개별 Detail 선택만 있고, Stage 삽입 뒤 Logic/Collider 등의 absolute startMs는 그대로다.
모든 표시 lane을 드래그·Ctrl+click으로 선택하고 한 타격 구간의 배치와 연결을 함께 복제한다.
저장된 사용자 패턴에 타격 횟수를 자동 추가하지 않고 저작 명령을 구현한다.

## G01. 선택과 원자 복제

기존 Stage ID 목록과 occurrence ID 목록을 사용한다. occurrence는 Animation/Logic/Summon/World/
SceneProfile/Effect/Collider/Sound/Camera/Light 전체의 stable ID다. UI hit box·highlight·Detail focus와
Normalize_Selection을 같은 집합에 연결한다. Duplicate와 Ctrl+D, Delete는 같은 batch 명령을 쓰고,
Stage reorder는 Stage/Animation만 선택했을 때 허용한다. 새로운 Sound 정의를 생성하지 않는다.
마우스 범위는 각 박스의 시간폭 전체를 감싸야 선택한다. 짧은 타격을 드래그할 때 장시간 유지되는
검색 master와 카드 WORLD까지 같이 선택되지 않게 하며, 필요한 박스는 Ctrl+click으로 추가한다.

Stage/Animation만 선택한 기존 복제의 순서·trim은 유지한다. 다른 lane과 함께 선택하면 전체 선택의
시작~끝 차이를 보존해 뒤 Stage 경계에 삽입한다. 선택하지 않은 사이 구간은 빈 Stage 시간으로
유지한다. World tail이 Pattern 끝을 넘으면 복제 구간에도 해당 tail 시간을 둔다. lane만 선택하면
선택 끝에 같은 상대 간격으로 복제하고 필요한 만큼 마지막 Stage 수명을 늘린다.

Stage 삽입 이후의 원본 lane start는 삽입 길이만큼 밀고, 삽입점을 가로지르는 master Logic/World
등은 duration을 늘린다. source animation 시간·재생률·fade/blend와 공유 resource는 유지한다.
BossMotion 중간 삽입은 두 점 경로로 정지 구간을 표현할 수 없어 명시 거절한다.

Collider↔Logic(연결된 전체 Collider), World↔companion Effect 소유권은 먼저 중복 없이 모은다. 새 occurrence/Stage/action/
region ID를 할당하고 복제본끼리 참조를 바꾼다. 공유 Logic 정의와 결과가 복제된 World/Logic/region을
참조할 때만 정의를 별도로 복사하여 typed 참조를 바꾸고, 기존 카드·전체 검색 master·외부 motion/
후속 Pattern 참조는 유지한다. ID 한계·시간 한계·참조 오류는 기존 Commit_Candidate 이전에 거절해
source/draft/counter/선택 상태를 보존한다. 복제는 DRAFT이며 Save 및 명시 PRODUCT 승격을 따른다.

Delete도 전체 lane ID를 한 번 검증하고 한 candidate로 처리한다. Stage 시간 삭제로 사라지는
미선택 lane이나 남은 참조가 있으면 실패 이유를 표시하며 원본을 보존한다.

## G02. 파일과 검증

기존 Workbench H/CPP와 BossCompositionDocumentContractTests.cpp를 수정한다. 새 C++/schema/
project/filter 항목은 없다. 기존 focused native 검사에 혼합 lane·의존성·COW·stable ID·시각 삽입·
master 연장·lane-only·Save/Reload·invalid/overflow 보존을 추가한다. 관련 최소 컴파일과 최종 Debug
Product 빌드, source JSON/XML parse, scoped diff check를 확인한다. 사용자의 실제 파일은 수정하지
않으며 Client/UI 조작·화면 검증은 사용자가 한다.

## G03. 실행 중 PRODUCT revision 거절 진단

사용자 screenshot의 `expected Product source revision is not active`는 저장 좌표가 아니라 Server
활성 catalog revision 검사다. 최신 source/product166과 실행 중 Server의 활성 revision을 확인한다.
동일 publisher·Server 권위를 유지하고 미지원 runtime hot reload로 검사를 우회하지 않는다.
거절 메시지에 기대/활성 번호와 Server 재시작을 안내할 필요가 있으면 해당 실제 Server 경계에서
수정하고 최신 사용자 저장 데이터는 보존한다.
