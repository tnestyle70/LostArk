# 쿠크 입장 시퀀스 실패의 전투 전환 차단

## G00. 원인과 소비자

actorProfile이 있는 입장은 MainApp의 단일 Pattern→Bundle preview adapter를 사용한다.
현재 Sample_BundlePreview의 WORLD Play/Seek 실패는 status만 바꾸고 clock을 계속
진행시키므로 실패한 연출도 completed event를 통해 Gate/Pattern Flow를 시작할 수 있다.
MainApp의 단일 WORLD 실패 검사에서는 bundle을 제외한다.

## G01. 구현

PresentationPlayer는 실패 ID·원인을 one-shot event로 보존하고 세션을 즉시 Stop한다.
성공 completion과 실패 event는 서로 배타적으로 소비한다. 명시 Stop과 새 Begin은
이전 event를 폐기한다. Begin_BundlePreview의 최초 sample 실패는 admission 실패다.
MainApp은 queued UI transport 뒤 실패를 먼저 소비해 Complete run을 cancel하고
Root의 CancelKoukuGateCompletePlay를 통해 입력/HUD/암전을 해제한다. 단일 WORLD
실패도 같은 소비자를 사용한다. Preparation/Gate timeout의 상태 자체는 Root 소유다.

## G02. 검증

기존 source/out focused compile과 CPU state 검사를 재사용한다. 실패→완료 금지,
성공 completion, 명시 Stop, 실패 메시지 보존을 검사한다. Client/UI/GPU draw를 하지 않는다.
새 제품 C++ 파일이나 project 등록은 추가하지 않는다.

## G03. Flow 제출 직전 revision 고정

MainApp의 완료 run이 저장한 source revision을 BossTool의 Play_PatternFlow까지 전달한다.
해당 함수는 실제 Prepare/Reload가 끝난 직후 기대 revision을 비교하고, 달라지면 entries
생성 및 Server 제출 전에 거부한다. 기존 일반 F1 caller는 기대값0을 기본값으로 사용해
현재 저장본을 실행하는 동작을 유지한다. 검증 이후 다시 로드된 값도 제출 직전에 확인한다.


## G04. 공유 Sequencer Stop과 전투 대기 취소

Resources 등 공유 shell의 Stop은 활성 benchmark Complete run이면 MainApp의 기존
CancelKoukuGateCompletePlay에 전달한다. 입장 preview owner가 이미 해제된 spawn/teleport
대기 중에도 해당 Stop만 허용한다. 일반 preview와 Pause/Resume의 owner 검사는 유지한다.
이로써 Workbench Complete state, Level pending gate, 입력/HUD/암전 상태를 같은 취소 경로로
폐기하며 공유 Stop 뒤 입장 state가 남아 무기한 입력을 막지 않게 한다.
