# Release 전환 리소스 준비 재사용과 Server 송신 큐 확대

## G00. 현재 실측과 범위

Release의 Level_Loading은 전체 레이드 V1/V2를 준비하고 Level_KakulSaydonArena는 enabled WORLD를 준비한다. 그러나 1관문 트리거의 Server PREPARING을 받은 MainApp은 새 Complete Play 준비를 시작한다. 이 준비는 WORLD를 다시 Load하여 모델·clone pool을 비우고 actor/V2/WORLD를 프레임당 하나씩 다시 준비한다. 이미 로딩한 상태에서도 이 반복이 트리거 지연을 만든다.

기준 HEAD는 d6a9cc2240257779c285110bac9b73714349291a이며 작업 시작 시 추적·미추적 변경은 없었다. 기능 브랜치는 codex/kouku-release-sequence-ready다. 현재 Visual Studio의 Debug 셰이더 빌드는 별도 사용자 작업으로 보존한다.

## G01. Level의 기존 준비 경로 재사용

Client/Public/Level_KakulSaydonArena.h와 Client/Private/Level_KakulSaydonArena.cpp에서 실제 Complete Play 준비 본문을 private 공통 함수로 연결한다. 기존 Debug 호출은 idle WORLD reload와 프레임별 진행을 유지한다. Release Initialize만 Loader가 검증한 WORLD를 그대로 사용해 동일 준비를 끝내며, 초기 Release WORLD 순회는 이 공통 준비로 대체한다.

초기 준비는 게시 inventory의 Get_PlayAllPatternIds와 source revision, wholeRaid=true를 사용한다. V1은 이미 끝난 Loading barrier의 실제 prepared/current/failure 상태를 검사한다. 초기화에서 worker를 기다리는 무한 루프를 만들지 않고 actor/V2/WORLD 및 catalog actor 수로 제한한 호출 안에서 준비를 끝낸다. binding 준비는 기존 실제 reader와 rollback을 유지하고 Server Product admission은 미리 수행하지 않는다.

완료된 m_CompletePlayPreparation은 Level 수명 동안 유지된다. 1관문 숨김 책·환경도 기존 Prepare_ServerRaidGatePresentation으로 stage한다. 트리거가 같은 선택/revision을 요청하면 이미 완료한 준비와 gate stage를 재사용한다. MainApp의 saved/published/Server revision 검사와 모든 참가자 READY, Server 시작 tick은 유지한다. Debug 저작·명시 reload·실패·취소 경계를 우회하지 않는다.

새 C++ 파일과 project/filter 등록, JSON/publisher 변경은 필요 없다. 현재 rendering option은 수정하지 않는다.

## G02. 서버 입장 실패 로그 조사

사용자가 추가로 보고한 Release 카드 회전 패턴 중 두 명의 Server entry failed를 최근 Client/Server session 로그로 추적한다. server-session-36460의 session3/4는 같은 시각 queuedFrames=128에서 SERVER_RELIABLE_OUTBOUND_OVERFLOW로 종료됐다. 사용자 승인에 따라 Server/Public/ClientSession.h의 per-session 상한을 4096 frame/8 MiB로 확대한다. 기존 reliable FIFO, snapshot coalescing, 초과 시 종료와 terminal drain은 유지한다. 실제 큐를 사용하는 기존 ServerGameplayContractTests_SessionTransport.cpp에 burst/FIFO와 개수·바이트 한계 사례를 넣는다. 다른 PC의 Client 로그와 패킷 처리 화면은 미확보 상태다.

## G03. Release 전환 준비 범위 확대

사용자는 캐릭터 교체·쿠크 시퀀스·발탄의 실행 직전 준비 대기도 제거하도록 범위를 확장했다. 필요한 리소스 로드는 최초 Level Loading에서 수행하고 실제 요청은 검증된 준비 결과를 재사용한다. Character Select는 현재 catalog의 7개 직업 모델·장비·애니메이션·스킬 Effect와 선택 시퀀스를 이미 선로딩하므로 별도 경로를 추가하지 않는다. 다른 gameplay map의 원격 class lazy-load 정책은 이번 캐릭터 선택 교체 변경 대상이 아니다.

발탄은 Loader와 Initialize에서 이미 전체 레이드 Effect 및 시퀀스 WORLD를 준비한다. Complete Play의 V2/WORLD 진행만 Release에서 유한 반복으로 처리하여 리소스별 추가 프레임 대기를 제거하고 Debug의 분할 준비를 유지한다. 서버 승인·데이터 정합성 검사를 삭제하거나 누락 자산을 준비 완료로 처리하지 않는다.

쿠크의 명시적인 WORLD reload가 성공하면 아직 사용하지 않은 입장 G1 준비 객체와 조명을 함께 폐기한다. 실패한 로드, 현재 재생으로 지연된 reload와 활성 cinematic이 빌린 상태는 유지한다.

## G04. 검증과 완료 경계

실제 초기화·준비·트리거·정리 소비자를 읽고 독립 검토한다. 정상 Release Product Build와 git diff --check를 수행한다. 변경 JSON/XML이 없다면 별도 데이터 게시를 하지 않는다. 기존 Client/UI는 자율 실행하지 않는다. 사용자 확인 경로는 Release Server/Client → 쿠크 입장 → 1관문 트리거이며, 준비 지연·시퀀스 시작·중단/재입장과 파티 플레이는 사용자 화면 판정을 별도 RESULT로 남긴다.
