# 쿠크 Complete Play 준비 현황과 Workbench 초기 숨김

## G00. 실제 상태와 범위

현재 `codex/kouku-timeline-local-preview`의 기존 미커밋 변경을 보존한다. 사용자 화면은
PREPARING 중이며, 이후 사용자가 네 Client의 정상 재생을 확인했다. 실제 Server 로그도
같은 runEpoch에서 readyMask15/participants4와 CINEMATIC 전환을 기록했다. 재생 권위와
준비 장벽은 변경하지 않는다.

## G01. MainApp의 준비 진단

`Client/Public/MainApp.h`는 run epoch별 로컬 준비 단계·상세 결과·통지 상태·관찰 시간을
보존한다. `Client/Private/MainApp.cpp`의 UpdateKoukuGateCompletePlay는 준비 대기,
READY/FAILED 제출과 제출 실패를 구분해 저장한다. 매 프레임 일반 문구로 상세 실패를
덮어쓰지 않으며 world 교체 시 진단을 초기화한다.

같은 CPP의 Complete Play 패널에 Server raid phase, 고정 START roster 기준 준비 x/N,
참가자별 READY 수신 여부, 로컬 준비 상세, 실제 endpoint와 마지막 패킷 나이, Server
사유를 표시한다. 다른 PC의 세부 리소스 단계는 기존 packet에 없으므로 준비 통지 미수신과
해당 PC의 패널 확인을 정확히 안내한다. 개별 패턴 IDLE을 전체 Raid 상태로 표시하지 않는다.

## G02. Workbench 가시성

Action Workbench의 자동 준비와 사용자의 Open을 구분하여 최초 F1 열기에서는 숨김을
유지한다. 사용자가 명시적으로 연 창의 가시성과 입력 owner는 보존한다.

## G03. 검증과 전달

새 C++ 파일·프로젝트 등록·Shared packet·저작 JSON 변경은 없다. Debug/Release의 변경
translation unit 컴파일, 실제 diff와 CRLF/UTF-8 보존 및 diff check를 확인한다. 실행 중인
네 Client를 종료하거나 조작하지 않는다. 점유 EXE의 최종 링크와 새 창 화면 확인은 분리해
기록한다. 기존 네 Client 로그는 기존 실행의 정상 준비 증거이며 새 진단 UI의 화면 PASS가 아니다.
