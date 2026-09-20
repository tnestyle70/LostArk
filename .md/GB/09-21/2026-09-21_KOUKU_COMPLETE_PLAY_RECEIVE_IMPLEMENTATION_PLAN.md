# Kouku Complete Play 응답과 수신 배치 수정

## G00. 현재 실측과 작업 경계

사용자는 2관문 Sequences + Pattern Flow의 응답 timeout과 대형 세이튼 세 번 내려치기의 disconnected session을 보고하고 수정을 요청했다. `client-session-72676.jsonl` 444행은 2026-09-21 03:02:27에 lifecycle queue 64개 한도로 연결을 종료했으며 바로 다음 행이 사용자가 본 replication 오류다. raw queue high watermark는 1,305, 기록된 main pump 최대 간격은 14,953ms다. 이 수치만으로 특정 불뿜기 패턴이 원인이라고 단정하지 않는다.

현재 브랜치는 `pattern-bug-fix-bingo`다. 앞서 적용한 DataJson·F1 카메라·크기 Save 변경과 다른 작업의 미커밋 파일, 사용자가 편집한 Sequence JSON을 보존한다. Client/UI 실행과 화면 판정은 사용자가 한다.

## G01. NetworkManager 수신 배치

현재 `CNetworkManager::Update`는 raw queue 전체를 로컬 deque로 옮겨 전부 `Handle_Frame`한 뒤 반환한다. Kouku lifecycle 소비자는 이후 `CKoukuSaydonPatternAuditionService::Update`에서 실행되므로 한 배치가 64개를 넘으면 정상 소비 기회 전에 종료된다.

`Client/Public/NetworkManager.h`와 `Client/Private/NetworkManager.cpp`의 실제 queue 계약에 맞춰 한 번의 dispatch 양을 제한하고 다음 Update까지 나머지 reliable 입력의 FIFO를 보존한다. 연결 generation 교체·terminal 정리·snapshot coalescing·원래 overflow 방어를 유지한다. 실제로 소비하지 않은 입력을 버리거나 reliable lifecycle을 snapshot처럼 합치지 않는다. 기존 하네스에서 실제 정책과 소비자 경계를 사용해 64개를 넘는 lifecycle burst, mixed packet 순서, terminal/session reset을 확인한다.

## G02. Complete Play의 늦은 응답

`MainApp.cpp`의 raid 5초 timeout은 pending request ID를 지워 늦게 도착한 epoch 0 거절 사유를 무시한다. 완료가 확인되지 않은 요청의 식별자를 보존해 동일 요청의 늦은 성공·거절을 처리한다. timeout 안내를 반복해서 덮어쓰지 않고, 사용자 취소·level/session 변경·새 요청과의 관계를 실제 호출자에 맞춰 정리한다. 서버 상태를 Client에서 성공으로 추정하거나 자동 재시도하지 않는다.

`ClientReplication`은 같은 Update에서 여러 raid packet을 소비하므로 단일 latest reply를 일반 상태 broadcast가 덮을 수 있다. 실제 pending request와 local owner가 일치하는 최종 응답은 별도로 보존하고, raid presentation state는 기존 순서대로 계속 갱신한다. `MainApp -> Level_KakulSaydonArena -> ClientReplication`의 typed 경계를 통해 요청 추적을 연결하며 다른 플레이어 요청과 같은 숫자의 request sequence를 혼동하지 않는다. 두 플레이어의 START·거절·후속 READY broadcast가 같은 배치에 있는 경우도 검증한다.

## G03. 현재 저장본과 서버 게시본 일치

추가 실측에서 Action/Encounter/patternbindings revision은 1918로 일치하지만 Sequence 저작본은 154, Server Gameplay.bootstrap의 모든 RAIDGATE는 120이었다. 현재 Complete Play는 Sequence 154를 요청하므로 기존 게시본으로 승인할 수 없다. 사용자 Client/Server가 종료된 상태에서 현재 저장본과 출력 baseline hash를 보존하고, 정식 `Invoke-BuildDomainOwner.ps1 -Owner KoukuSaydon -ExpectedKoukuSaydonSourceRevision 1918`의 검증·트랜잭션을 통해 생성물을 갱신한다. Sequence 저작 문서와 불뿜기 클립/이펙트는 편집하지 않는다. 저장본이 바뀌면 기존 snapshot으로 덮어쓰지 않고 원인을 다시 확인한다.

실제 Server GameRoom의 격리 검증은 게시 전 154 요청의 정확한 실패와 게시 후 Gate2 START→READY→CINEMATIC, 대형 세이튼 세 번 내려치기 admission을 비교한다. 제품 Server listener나 Client/UI를 자동 실행하지 않는다.

## G04. 검증과 반영

변경한 C++ 인코딩과 기존 project/filter 등록을 보존한다. 새 제품 파일이 필요하면 해당 project/filter에 필요한 항목만 등록한다. 실제 queue 정책 회귀, 늦은 거절·성공·취소 검증 및 Debug/Release 최소 컴파일을 수행하고 최종 Product 빌드 가능 여부를 확인한다. 사용자 실행/빌드와 경합하지 않으며 실행 중 EXE 점유 해제가 필요한 마지막 시점에만 종료를 안내한다. 변경 source의 `git diff --check`와 변경 XML/JSON parse를 확인하고 실행한 증거만 RESULT에 기록한다.

사용자는 불뿜기 중복 클립 의심은 보류하고 Complete Play만 먼저 수정하도록 범위를 확정했다. 불뿜기 패턴 자체와 기존 sound/sequence 편집값을 이 작업에서 교체하지 않는다.
