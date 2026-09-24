# 공용 Balance Test와 현재 관문 보스 처치 구현 계획

## G00. 현재 상태와 목표

현재 F1 Balance Tool은 일반 수치를 편집할 수 있지만 일반 Save가 차단되어 있고, Valtan의 joined source와 게시 revision에 묶여 있다. Animation/Effect/Valtan Boss Tool은 같은 CBalanceTool의 typed draft를 소비하므로 backend를 삭제하지 않는다. F1의 공용 진입점은 별도 일반 수치 panel을 열고, Valtan 전용 호출은 기존 authoring backend를 계속 사용한다.

현재 ALT_V 즉사 테스트는 damage/HP 값에 의존한다. 새 Server 명령은 현재 관문에 실제 존재하는 primary boss만 사망 상태로 만들고 기존 death/clear/Encore/reward 처리로 넘긴다. 관문 번호, HP, 대상 entity ID를 Client가 지정하지 않는다. Release Server는 debug 요청을 거부한다.

## G01. 공용 수치 draft와 저장

`Client/Public/BalanceTestPanel.h`, `Client/Private/BalanceTestPanel.cpp`는 공용 숫자 편집 draft, 선택 행, 비동기 작업 상태를 소유한다. PlayerProfiles/PlayerSkills/DamageProfiles/BossProfiles의 stable ID별 허용 수치만 편집한다. 원문 전체를 다시 만드는 기존 serializer를 사용하지 않는다.

`Tools/GameplayPipeline/Save-BalanceTestDraft.ps1`은 변경 field의 이전 값과 최신 저장본을 비교해 병합한다. 충돌은 기존 파일을 보존한다. candidate overlay에서 provenance 동기화와 gameplay validation을 먼저 수행하고, 최신 bytes를 재확인한 뒤 원자 replace와 자기 변경 rollback으로 저장한다. 게시 버튼은 기존 Publish-BalanceRuntimeSet을 사용하며 Server 재시작 필요 상태를 분리한다. 다른 도구의 메모리 draft는 자동 Reload하지 않는다.

새 C++ 파일 두 개는 Client.vcxproj/.filters의 기존 BalanceTool 인접 물리 분류에 등록한다. 공용 panel은 Valtan source 로드를 요구하지 않는다. 기존 backend는 Open_Valtan 및 typed 호출을 유지한다.

## G02. Server 권위 현재 관문 보스 처치

Shared request/result는 request sequence, world, 화면에서 마지막으로 본 boss archetype, Server 결과와 처치 수를 전달한다. archetype은 오래된 화면 요청을 거부하는 조건이며 대상 선택 권한이 아니다. IPlayerCommandSink, NetworkPlayerCommandSink, NetworkManager, ServerApp, RoomCommand, GameRoom queue를 수직 연결한다. GameRoom_GateProgress의 현재 관문 resolver와 stable placement를 재사용한다. G2의 두 actor를 함께 대상으로 잡고, 다른 관문과 종속 소환체는 제외한다. 동일 request sequence 재시도는 새 관문에 적용하지 않는다.

Kill 처리 후 기존 Update_WorldEntities가 사망 event, attachment/combat object 정리, gate clear, Valtan 보상 및 G3 Encore를 처리한다. 최종 raid clear를 직접 설정하거나 despawn 명령을 재사용하지 않는다. F1 Valtan Arena/KoukuSaydon Arena 및 공용 Balance Test는 동일 typed 요청을 사용한다.

## G03. 검증과 경계

작성 에이전트는 source/diff/JSON/XML/PowerShell parse와 focused contract를 검증한다. build, publisher 실행, commit은 root 에이전트가 통합 후 수행한다. Shared packet round-trip, Server wrong-world/Release/replay/다른 관문 보존 및 G3 progression을 검증한다. Client/UI 실행과 최종 4인 화면 확인은 사용자 경계로 남긴다.

#454는 이번 구현에 병합하지 않는다. Retail의 수치 범위와 Client 표시/publisher 전달/monster 게시/버프 stun 연결 문제는 대응 RESULT의 별도 리뷰 항목으로 남긴다.

## G04. 2026-09-24 F1 Balance Test 진입 위치 조정

사용자는 F1에서 Balance Test 버튼을 눌러 기존 별도 창을 여는 방식을 유지하고, 그 버튼을 기존 Player Follow Camera가 있던 위치에 배치하도록 확정했다. F1에 수치 editor를 직접 삽입하지 않는다. MainApp의 버튼 위치와 기존 별도 창 Render/Ensure 연결은 통합 담당자가 수정한다. 숫자 editor의 Reload/Save/Publish/쿨타임/Kill Boss와 기존 창 크기·본문은 유지한다.

`CBalanceTool::Update_EmbeddedPanel`과 panel의 `Update`를 제공해 MainApp이 F1 표시 여부와 Debug/Release에 관계없이 진행 중 저장·게시 결과를 수거한다. 닫기는 실행 중 writer를 취소하지 않는다. 새 파일과 project/filter 등록은 없으며 기존 인코딩·개행을 유지한다. 검증은 해당 diff와 호출 경계 점검 후 통합 담당자의 Debug/Release 컴파일로 구분하고, Client 화면은 사용자가 확인한다.
