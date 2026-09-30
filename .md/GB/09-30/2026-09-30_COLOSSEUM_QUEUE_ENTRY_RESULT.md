# 2026-09-30 콜로세움(증명의 전장) 입장 대기열 RESULT

베른 콜로세움 NPC의 입장 흐름을 "수락/거절 창(15초) → 대기창(ESC 취소) → 서버 대기열 → 랜덤 팀 배정 → 매치 로딩 → 콜로세움"으로 바꾼 서버 권위 수직 슬라이스다. 소스 적용과 구문 확인(`cl /Zs`)까지만 했다. 빌드, Client 실행, 화면 확인, 하네스 실행은 하지 않았다.

## 1. 사용자가 확정한 사양

- NPC에 말을 걸면 "증명의 전장 입장 대기" 창이 뜬다. 문구는 "섬멸전 전투가 준비되었습니다! 참여하시겠습니까?", 노란 바가 15초 동안 줄어들고 그 아래 "N초 남았습니다.", 하단에 수락(녹색 체크)/거절(빨간 X).
- 15초 안에 누르지 않으면 거절. 거절은 창만 닫히고 서버에는 아무것도 보내지 않는다.
- 수락하면 같은 제목에 "투기장 바닥을 청소 중.."과 계속 도는 파란 원이 있는 대기창으로 바뀐다. 시간 제한 없이 기다리고, 대기창에서 ESC를 누르면 그 플레이어만 대기열에서 빠진다.
- 서버가 수락자를 세고, Release는 4명, Debug는 1명이 되면 랜덤으로 팀을 나눠 매치 로딩 → PvP 월드로 보낸다.

## 2. 구현 범위와 바뀐 파일

Shared (protocol 126 → 127, 패킷은 끝에 append)
- `PacketType.h`: `C2S_COLOSSEUM_QUEUE_JOIN`, `C2S_COLOSSEUM_QUEUE_LEAVE`, `S2C_COLOSSEUM_QUEUE_STATE`, `S2C_COLOSSEUM_MATCH_FOUND`, 버전 127, known switch.
- `PacketMessages.h/.cpp`: 위 네 메시지의 struct와 Write/Read. `COLOSSEUM_QUEUE_STATE`(WAITING/LEFT/REJECTED), `COLOSSEUM_MATCH_PARTICIPANT`(닉네임·직업·팀), `MAX_COLOSSEUM_MATCH_PLAYERS = 4`. MATCH_FOUND는 참가자 1~4명, 팀 0/1, 닉네임·직업 유효성, 수신자 본인 행(`iLocalIndex`)이 범위 안일 때만 통과한다.

Server
- `RoomCommand.h`, `ServerApp.cpp`, `GameRoom.cpp`: 새 명령 두 개의 수신·디스패치.
- `GameRoom.h`, `GameRoom_Inventory.cpp`: `Handle_ColosseumQueueJoin/Leave`, `Send_ColosseumQueueState`, `Try_FormColosseumMatch`, `m_ColosseumQueue`.
- `GameRoom_Admission.cpp`(`Leave`): 접속 종료·이동한 세션을 대기열에서 제거.
- `GameRoom_Inventory.cpp`(`Handle_ConfirmNpcEntry`): 콜로세움 NPC 행을 표에서 제거했다. 예전 직접 입장 경로가 대기열을 우회하지 못한다. 이 경로에만 있던 콜로세움 예외(파티 거절, 귀환 NPC 생략)도 원래 코드로 되돌렸다.

Client
- `LevelTransitionService.h/.cpp`: 서버가 준 팀 명단 저장소(`Set/Try_Get/Clear_ColosseumMatch`).
- `NetworkManager.h/.cpp`: JOIN/LEAVE 송신, QUEUE_STATE 큐, MATCH_FOUND는 `LevelTransitionService`에 바로 저장한다. ENTER_ACCEPTED가 타입별 큐를 비우기 때문에 replication 큐를 쓰지 않았다.
- `PlayerCommandSink.h`, `NetworkPlayerCommandSink.h/.cpp`: `Request_ColosseumQueueJoin/Leave`, `Consume_ColosseumQueueState`.
- `RaidEntryPreviewView.h/.cpp`: 오퍼(15초 바 + 수락/거절)와 대기창(도는 원, ESC 취소). 기존 모달이므로 입력 차단과 ESC 소유(`Is_EscapeOwnedElsewhere`)가 그대로 적용된다. 텍스트 위치는 레이아웃의 숨김 마커 슬롯에서 읽는다.
- `Level_Bern.h/.cpp`: NPC가 `Open_ColosseumOffer()`를 열고, 수락/ESC 의도를 JOIN/LEAVE로 제출하며, 서버의 REJECTED/LEFT가 오면 대기창을 닫는다. 이전 `SIMPLE_ACCEPT → Request_ConfirmNpcEntry` 분기는 제거했다.
- `Level_Loading.h/.cpp`: 매치 로딩이 서버 명단으로 팀별 카드(이름·직업·인원 수)를 채운다. 접속한 플레이어의 팀이 왼쪽, 본인이 그 팀의 key 카드다. 오른쪽 팀 첫 사람의 직업이 오른쪽 일러스트가 된다. 서버 이름 줄은 이전 요청대로 없다. 서버 이름·샘플 이름·샘플 직업 필드는 더 이상 쓰지 않아 제거했다.

Data / Tools
- `Data/UI/Colosseum/QueueDialog_Layout.json`(신규), `Tools/LpkPipeline/build_colosseum_queue_dialog_ui.py`(신규, 이 JSON과 PNG를 만든다).
- `Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp`: protocol 기대값 126 → 127(최신 두 곳)과 `Test_ColosseumQueueProtocol` 추가(`--colosseum-queue-only`).
- `CLAUDE.md`: 서버 권위 월드 절의 protocol 표기를 v125 → v127로 고쳤다. 문서 전체에는 콜로세움 서술이 원래 없어 그 이상은 건드리지 않았다.

리소스(Git 비추적, `Client/Bin/Resources`와 `CY_Resources`의 같은 상대 경로에 둠): `UI/Colosseum/QueueDialog/remain_bar.png`(211×9), `ring_blue.png`, `ring_white.png`(144×144). 복사 스크립트나 목록은 만들지 않았다.

## 3. 서버 규칙

- 대기열은 BERN 방 안의 수락 순서 목록이다. 수락자는 살아 있고, 행동 상태가 NONE이고, 콜로세움 NPC 3m 안이며(서버가 다시 검사), 다른 전송에 묶이지 않아야 한다. 아니면 REJECTED를 답해 대기창이 닫힌다.
- 이미 대기 중인 세션의 JOIN은 무시(WAITING만 다시 답한다).
- 파티에 속한 플레이어(가이드 동행 포함)는 REJECTED. 대기열은 한 명씩 옮기기 때문에 파티를 갈라놓지 않으려는 기존 규칙과 같은 판단이다.
- 인원 조건: `_DEBUG` 서버는 1명, Release 서버는 4명. 채워지면 앞에서 최대 4명을 꺼낸다.
- 팀: 자리를 섞고 앞의 `(n+1)/2`명이 팀 0(왼쪽), 나머지가 팀 1. 1명이면 1:0, 2명 1:1, 3명 2:1, 4명 2:2.
- 그다음 각 세션에 MATCH_FOUND를 보내고, 기존 solo 월드 전송(`WORLD_ID::COLOSSEUM`, 귀환 NPC 없음)을 stage한다. ENTER_ACCEPTED 이후는 기존 경로 그대로다.
- 대기 중 접속이 끊기거나(`Leave`) LEAVE를 보낸 세션은 목록에서 빠지고 나머지는 계속 기다린다. 형성 시점에 죽었거나 다른 전송에 묶인 사람은 빠지고 LEFT가 간다.
- 콜로세움 안에 있는 사람이나 다른 월드의 사람은 BERN 방이 아니므로 대기열에 들어갈 수 없다.

## 4. 원본과 측정 구분

원본(lpk 무비) 데이터
- 금색 남은 시간 바: `EFUI_DIALOG`의 `dialog_i1` 아틀라스(무비 `dialog`에 `DialogRemainProgress`, `AnimatedButton_renew_confirm/cancel`, `DialogPositiveIcon`, `DialogNagativeIcon` 존재) 안의 바를 그대로 잘랐다(`remain_bar.png`).
- 도는 원: `EFUI_COMMONOBJECT`의 `globalobject_i4` 안 파란 혜성 호와 흰 혜성 호를 잘랐다. 각 호의 원 중심이 이미지 중심이 되도록 정사각 캔버스에 놓았다(엔진은 이미지 중심으로 회전한다).
- 패널·버튼·체크/X 아이콘은 기존 `UI/Common` 원작 아트를 재사용했다(같은 원작 다이얼로그).
- 문구는 원작 문자열과 일치한다(`sys.colosseum.loading_1`, `matching_completion`, `cooperation_giveup_dialog_remain_time`).

측정 값(원본에 수치가 없어 스크린샷 두 장에서 픽셀로 잰 값. 오퍼 405×204, 대기 409×261을 다이얼로그 폭 571로 환산)
- 모든 슬롯 위치와 크기: 제목·본문·초 텍스트 중심, 바 x 59~346·y 110~115, 버튼 x 94~316·y 158~194, 체크/X 아이콘, 원의 중심(207.7, 166.9)과 반지름 40.1.
- 두 다이얼로그의 높이가 다르다(오퍼 약 288, 대기 약 364 기준 px). 두 캡처의 실제 크기 차이를 그대로 따랐다.
- 파란 호의 원 중심 (390, 18)은 ridge/skeleton 적합, 흰 호 중심 (903.5, 17.7)은 skeleton 적합(잔차 0.2px).

추정(원본에서 확인하지 못한 것)
- 파란 호와 흰 호가 같은 원 중심을 공유하고 함께 도는 구성, 회전 속도(파랑 270°/초, 흰색 360°/초).
- 원작 캡처의 바 채움은 "15초" 시점에도 약 70%였다. 이번 구현은 15초에서 가득 차고 선형으로 줄어든다(사용자 지시 15초 기준).
- 창 전체 배경(어두운 사각형 + 얇은 테두리)은 캡처의 색을 눈으로 맞춘 근사다.

## 5. 검증 (실행한 것만)

- `cl /Zs` 통과: Shared `PacketMessages.cpp`, Server `GameRoom_Inventory.cpp`/`GameRoom.cpp`/`GameRoom_Admission.cpp`/`ServerApp.cpp`, Client `LevelTransitionService.cpp`/`NetworkManager.cpp`/`NetworkPlayerCommandSink.cpp`/`RaidEntryPreviewView.cpp`/`Level_Bern.cpp`/`Level_Loading.cpp`/`MainApp.cpp`, `NetworkProtocolHarness.cpp`. (표준 헤더 강제 include로 제품 PCH를 대신함. 빌드가 아니다.)
- `build_colosseum_queue_dialog_ui.py` 실행: JSON 23슬롯과 PNG 세 장 생성, JSON 파싱, 아틀라스 바 위치 assert 통과.
- 수정한 소스는 줄바꿈 CRLF 개수와 LF 개수가 같고 깨진 문자가 늘지 않았다(NetworkManager의 기존 U+FFFD 698/116개는 그대로). `git diff --check`는 이번에 건드리지 않은 파일의 LF 경고만 남았다.
- 실행하지 않음: 제품 빌드, Server/Client 실행, 하네스 실행, 서버 계약 테스트, 화면 확인(visual PASS 없음).
- 하네스에는 이번 작업 전부터 `NETWORK_PROTOCOL_VERSION == 125u`를 요구하는 검사가 일곱 곳 있고 현재 버전과 이미 맞지 않는다. 그대로 두었다.

## 6. 사용자가 확인할 것

1. Debug/x64로 Server와 Client를 한 번 빌드하고 함께 재시작한다(protocol 127, 이전 버전 peer와 호환되지 않는다).
2. 베른 성 안 콜로세움 NPC에 말을 건다. 오퍼 창의 문구·바·"N초 남았습니다."·버튼 위치와 15초 후 자동 거절을 본다.
3. 수락하면 Debug 서버는 1명이라 곧바로 매치 로딩으로 넘어간다. 대기창(도는 원)은 짧게 보이거나 안 보일 수 있다. 원의 모양과 회전은 Release 서버(4명) 또는 Debug에서 대기창이 잠깐 뜨는 동안 확인한다. ESC로 대기 취소도 Release 조건에서만 오래 볼 수 있다.
4. 매치 로딩에서 카드가 서버 명단대로(1명이면 왼쪽 1장, 오른쪽 숨김) 나오는지 본다. 오른쪽 팀이 있으면 일러스트가 그 팀 첫 사람의 직업인지 본다.
5. 팀원 4명이 Release 서버에 각자 수락해 2:2로 나뉘고 각자 자기 팀이 왼쪽에 나오는지 확인한다. 파티를 맺은 상태로는 수락이 거절된다.

## 7. 남은 한계와 다음 단계

- 서버가 정한 팀은 클라이언트 표시용으로만 전달한다. COLOSSEUM 방 안의 플레이어 상태(팀 소속)로는 저장하지 않는다(현재 PvP 규칙이 없다). PvP 규칙을 만들 때 전송 요청(`SERVER_WORLD_TRANSFER_REQUEST`)에 팀을 실어 방에 넘겨야 한다.
- 카드의 단수·KDA는 샘플 값 그대로다(이전과 같음). 원작의 서버·계급·KDA는 서버가 채우는 값이라 프로젝트에 없다.
- 오퍼 창에서 ESC는 거절로 처리한다(사용자 지시에 명시되지 않은 선택).
- `CRaidEntryPreviewView::Open_SimpleConfirm`과 `SIMPLE_ACCEPT` 의도는 호출하는 곳이 없어졌다. 지우지 않고 남겼다.
- 회전 속도, 배경 색, 호의 공유 중심은 화면을 보고 조정해야 한다(`RaidEntryPreviewView.cpp`의 `COLOSSEUM_RING_*`, 레이아웃 JSON의 `QueueOffer_/QueueWait_` 슬롯, 빌더 스크립트를 다시 실행).
- 대기열은 BERN 방 메모리다. Server를 재시작하면 대기열이 비고, 클라이언트 대기창은 세션이 끊기면 함께 사라진다.
