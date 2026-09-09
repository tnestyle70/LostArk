# Action Workbench 선택 복구와 연출 Sequencer 구현 계획

작성일: 2026-09-10. 사용자는 첨부 화면에서 쿠크 Logic/Collider가 사라진 원인을 복구하고,
Action Workbench 아래 별도의 Open Sequencer Benchmark를 요청했다. 현재 dirty 변경은 보존한다.

## G00. Action Workbench 보스 세션 선택 복구

실측 원본은 revision229, Pattern24/Logic35/Collider Resource16개이며 조커·거미·잡기 연결이 남아 있다.
첨부 화면은 Valtan owner의 VALTAN_ARENA_BREAK_109와 source265개다.
MainApp의 첫 CSequencerTool 생성은 Level을 검사하지만 재열기는 Open()만 호출한다.
Lobby/Valtan에서 먼저 만든 shell이 쿠크 입장 뒤에도 Valtan session을 선택할 수 있다.
또한 Open(boss)는 session의 Select_WorkbenchBoss를 호출하지 않아 shell과 실제 Gate 필터가 다를 수 있다.

CSequencerTool의 명시 Open과 Boss selector를 하나의 선택 함수로 연결한다. Open은 숨겨진 toolbar,
Resources/Patterns/Detail을 다시 표시한다. MainApp은 현재 아레나와 선택한 boss family가 다르면
해당 아레나 family를 명시 선택한다. 같은 쿠크 family 안의 Gate2/3 선택은 보존한다.
기존 source, resource, pattern ID와 UI 창 ID는 변경하지 않는다. 자료를 예전 파일로 덮어쓰지 않는다.

기존 선택/저장 검사에 재열기와 명시 Gate 전환 검증을 추가하고 해당 Client 파일을 최소 컴파일한다.
입력과 실제 화면 확인은 사용자가 수행한다. Client/UI 자율 실행이나 캡처는 하지 않는다.

## G01. 독립 연출 편집 세션과 저장 경계

사용자는 파티 캐릭터 등장은 후속 Summon/Logic 작업으로 미루고, 현재 핵심을 Action Workbench와
동일한 ImGui를 재사용하는 sequence 전용 편집 도구라고 확정했다. 이번에는 새 Server 시퀀스나
플레이어 생성·이동·잠금을 구현하지 않는다.

CKoukuSaydonActionWorkbench의 두 번째 인스턴스를 sequenceWorkspace 모드로 만든다.
동일 Render_WorkbenchPane이 Timeline/Resources/Box Detail/Create/Rename/Delete/Duplicate와
Save/Reload를 제공하므로 별도 임시 위젯 복제품을 만들지 않는다. 별도 CSequencerTool shell은
자기 window ID, 선택·검색·dirty 상태를 소유한다. 새 최상위 버튼은 Open Sequencer Benchmark이며
Action Workbench 바로 아래에 놓는다. Action의 기존 pane ID는 유지하고 새 shell에는 별도 ID를 쓴다.

저장은 Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json,
compositionId는 boss.composition.kakulsaydon.sequencer다. 기존 KoukuSaydonCompositionDocument의
구조와 atomic writer를 재사용하고 path/identity 조합을 검사해 Action 원본과 교차 저장하지 않는다.
Sequence 모드의 Server Play와 Publish API는 거부하고 버튼도 숨긴다. 실제 제품 패턴 publisher의
정본은 계속 Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json이다.

새 Sequence 원본에는 기존 저작 Resource 정의를 독립 편집의 시작값으로 가져오며 Action 원본은
변경하지 않는다. 기존 World Sequence와 Camera stable 참조를 같은 시간축에 배치한 연출을 준비한다.
이는 기존 World/Camera 리소스의 위치·클립 원본을 재작성하는 작업이 아니다.

## G02. 기존 Preview와 Resource 공급 재사용

MainApp은 두 Workbench의 resource refresh와 typed preview 요청을 각각 소비한다. 기존 Animation
Tool, KoukuSaydonPresentationPlayer, CWorldSequencePlayer가 실제 animation/World/Effect/Scene/Camera를
재생하며 동시에 한 편집 세션만 미리보기 시간을 소유한다. 재생을 요청한 session만 playhead/state를
받는다. 다른 창을 열거나 닫는 일은 반대편 draft와 선택을 변경하지 않는다.

Animation 없는 시간 Stage도 기존 Begin_Preview와 Begin_KoukuWorldPreview로 재생하므로
카메라·맵 연출을 넣기 위해 임의 보스 animation을 만들지 않는다. Logic/Trigger/Summon 저작 탭은
같은 UI로 유지하지만 Sequence 미리보기에서 Server gameplay를 실행하지 않는다.
플레이어 순차 등장은 이번 완료 범위가 아니다.

새 C++ 파일은 필요하지 않다. 새 Data JSON은 Client vcxproj의 96.DataFiles None과 filters에 등록한다.
기존 C++ 파일 인코딩을 보존하고 사용자 실행 중 Client는 종료하지 않는다.

## G03. 검증과 결과

저장본 revision229와 기존 패턴/Logic/Collider 보존, 두 storage path/identity의 왕복과 교차 저장 거부,
재열기 보스 선택 및 독립 shell IDs, 새 source JSON과 project XML을 검사한다. 변경 Client 최소
컴파일을 수행하고 실행 파일 교체가 잠겨 있으면 컴파일과 최종 링크 상태를 분리해 보고한다.
사용자의 실제 Action/Sequence 창 재열기·Save·Preview 화면 검증은 사용자 단계로 남긴다.

## G04. F1 저장 높이와 Server 게시 배치 일치

추가 실측에서 F1 원본 Gameplay.world.json의 G2 Big Saydon Y는 10.0이나 Server worldbootstrap은
8.63000011로 확인됐다. Source 저장일은 09-09, worldbootstrap은 09-08이다. 기존 Publish All은
Composition과 Gameplay balance만 게시하므로 F1 보스 배치를 반영하지 않았다.

같은 Publish All의 Kouku owner transaction에 기존 world.gameplay publisher를 연결한다.
Publish-WorldGameplay.ps1의 KAKULSAYDON_ARENA 범위 옵션으로 쿠크 월드만 생성하고, 동일 writer와
codec을 사용한다. Product·world·balance 출력 및 receipt를 함께 보존하여 뒤 단계 실패는 모두 복구한다.
다른 월드 게시 파일과 원본 배치는 변경하지 않는다. 저장 Y10을 정상 publisher로 배포하고 검증한다.
Server의 실행 중 world 교체는 기존 계약에 없어 여전히 Server 재시작이 필요하다.
