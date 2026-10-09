# G01. 이동 패킷 하나로 읽는 현재 서버

이 문서는 2026-10-09 현재 제품 소스를 읽는 학습 자료다. 새 코드를 반영하라는 지시가 아니다. 네 명이 같은 월드에서 플레이하고, 그중 한 명이 바닥을 우클릭해 이동 목표를 보내는 상황을 따라간다.

현재 연결 경로는 `TCP → 세션 수신 스레드 → 방 명령 큐 → room 스레드 → 세션 송신 큐 → 세션 송신 스레드 → TCP`다. 중요한 구분은 **바이트를 주고받는 일**과 **플레이어의 실제 상태를 바꾸는 일**이다. 전자는 세션이, 후자는 room 스레드가 담당한다.

조사 중 [IocpService.h](C:/Users/tnest/Desktop/LostArk/Server/Public/IocpService.h:1)에 IOCP 서비스 선언이 저장됐지만 `IocpService.cpp`는 아직 비어 있다. 현재 `CClientSession`과 `CServerApp`에는 IOCP 연결이 없다. 별도 후보와 비교 결과는 [기존 PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md), [RESULT](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_RESULT.md)에 있으며, 그 후보의 구현을 현재 제품 기능으로 읽으면 안 된다. 헤더를 작성한 것과 그 API를 실제 세션이 호출하는 것은 별개의 반영 단계다.

여기서 네 명은 설명할 접속 상황이다. [MAX_PARTY_MEMBERS](C:/Users/tnest/Desktop/LostArk/Shared/Public/Network/PacketType.h:653)는 4이지만 바로 아래 `MAX_VALTAN_RAID_PLAYERS`는 8이다. 서버 전체의 최대 접속 수가 4라는 뜻은 아니다.

## G02. ServerApp이 연결을 받고 ClientSession을 만든다

먼저 [Main.cpp의 main](C:/Users/tnest/Desktop/LostArk/Server/Private/Main.cpp:15)을 연다. 인자를 읽은 뒤 206행에서 `serverApp.Run(...)`을 호출한다. [ServerApp.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/ServerApp.cpp:2613)는 WinSock을 초기화하고 listener를 연 뒤 `m_RoomThread`, `m_AcceptThread`를 만든다.

[Accept_Loop](C:/Users/tnest/Desktop/LostArk/Server/Private/ServerApp.cpp:2659)의 책임은 새 TCP 연결에 서버 내부 `SESSION_ID`와 `CClientSession`을 붙이는 것이다. 내부 [CTcpListener::Accept](C:/Users/tnest/Desktop/LostArk/Server/Private/TcpListener.cpp:84)가 Winsock `accept`를 호출한다. 받아 온 `SOCKET`은 그 연결에서 바이트를 읽고 쓰는 핸들이다. `SESSION_ID`는 서버 코드가 이 연결을 구분하는 번호다.

2680행에서 만드는 `shared_ptr<CClientSession>`은 `m_Sessions`에 보관된다. 이 컨테이너가 살아 있는 세션을 강하게 소유한다. 생성자에 넘기는 두 람다는 프레임 수신 시 `On_SessionFrame`, 종료 시 `On_SessionClosed`를 호출한다. **람다는 새 스레드를 만드는 문법이 아니다. 호출한 스레드에서 함수가 실행된다.**

이어 [CClientSession::Start](C:/Users/tnest/Desktop/LostArk/Server/Private/ClientSession.cpp:182)가 연결당 송신 스레드 하나와 수신 스레드 하나를 만든다. 평상시 이 경로의 명시적 스레드는 main 1개, accept 1개, room 1개, 연결당 2개다. 네 연결이면 11개다. 다만 콜로세움 준비와 밸런스 작업에는 별도 `std::async`도 있으므로 이것을 OS에서 관측되는 전체 스레드 수라고 말하지 않는다.

[Configure_TransportOptions](C:/Users/tnest/Desktop/LostArk/Server/Private/ClientSession.cpp:598)는 `TCP_NODELAY`를 설정하고 `ioctlsocket(FIONBIO)`로 nonblocking 모드를 켠다. 작은 이동·상태 메시지를 모아서 보내는 지연을 피하려는 설정이며, 처리량이 항상 증가한다는 뜻은 아니다. 소켓이 당장 읽거나 쓸 준비가 안 됐을 때는 [Wait_SocketReady](C:/Users/tnest/Desktop/LostArk/Server/Private/ClientSession.cpp:33)의 `select`로 기다린다. 현재 구현은 여러 연결을 한 `select` 루프에 모은 서버가 아니라 **세션별 스레드가 자기 소켓 하나를 기다리는 구조**다.

## G03. 이동 요청이 바이트에서 PACKET_FRAME이 된다

[C2S_MOVE](C:/Users/tnest/Desktop/LostArk/Shared/Public/Network/PacketMessages.h:402)를 먼저 읽는다. 지상 이동 요청에는 `iClientSequence`, 목표 `fGoalX/fGoalZ`, `eIntent`, `fVerticalInput`이 있다. 예를 들어 요청 번호 17, 목표 X=12/Z=8, `GROUND_GOAL`, 수직 입력 0을 보냈다고 하자. 이 좌표는 서버에 반영하라고 명령하는 현재 위치가 아니라 **이동하고 싶은 목표**다.

[Write_Message(C2S_MOVE)](C:/Users/tnest/Desktop/LostArk/Shared/Private/Network/PacketMessages.cpp:2138)는 이를 U32, F32, F32, U8, F32 순서로 쓴다. payload는 17바이트다. C++ 구조체의 `sizeof`만큼 메모리를 그대로 보내는 방식이 아니므로 구조체 padding을 패킷에 섞지 않는다.

[Build_Packet_Frame](C:/Users/tnest/Desktop/LostArk/Shared/Private/Network/PacketFrame.cpp:7)은 앞에 전체 크기 U32와 패킷 종류 U16을 쓴다. 따라서 이 예시의 한 frame은 `6바이트 header + 17바이트 payload = 23바이트`다. [PacketType.h](C:/Users/tnest/Desktop/LostArk/Shared/Public/Network/PacketType.h:493)의 상한은 header 포함 frame당 64 KiB, parser 누적 버퍼 256 KiB다.

서버의 [Receive_Frame](C:/Users/tnest/Desktop/LostArk/Server/Private/ClientSession.cpp:641)은 먼저 parser에 이미 완성된 frame이 있는지 `Try_Pop`으로 확인한다. 없다면 `recv`로 최대 4096바이트를 읽고 `Append`에 넘긴다. `recv`가 반환한 바이트 수는 게임 패킷 개수가 아니다. 위 23바이트가 5바이트와 18바이트로 나뉘어 도착해도 이상하지 않다.

[CPacketStreamParser::Try_Pop](C:/Users/tnest/Desktop/LostArk/Shared/Private/Network/PacketStreamParser.cpp:17)은 첫 5바이트만 있을 때 header 6바이트가 부족하므로 `NEED_MORE_DATA`를 반환한다. 다음 18바이트를 붙인 뒤에는 header의 전체 크기 23과 실제 남은 바이트 수를 비교한다. 다 모였을 때만 `PACKET_FRAME`을 완성한다. 여러 frame이 한꺼번에 들어왔다면 다음 `Receive_Frame`에서 남아 있는 버퍼를 먼저 소비한다.

`PACKET_HEADER`는 경계를 판정하는 정보다. `PACKET_FRAME`은 경계를 잘라 낸 뒤의 **패킷 종류 + payload 바이트**다. 아직 `C2S_MOVE`라는 게임 자료형은 아니다. `unreadBytes`와 `subspan`은 기존 버퍼의 일부를 보는 범위이고, `decoded.Payload.assign`은 완성된 payload를 frame 소유 메모리로 복사한다. 따라서 다음 `recv`에서 임시 수신 배열을 덮어써도 이 frame은 유지된다. `m_iReadOffset`은 누적 버퍼에서 이미 소비한 바이트 수다.

잘못된 header는 parser가 거부하고 세션을 종료하는 경로로 이어진다. 데이터가 덜 온 상태와 형식이 틀린 상태를 구분하기 때문에, TCP 분할 수신을 잘못된 패킷으로 처리하지 않는다.

## G04. ServerApp이 해석한 명령을 방 큐에 넣는다

[Receive_Loop](C:/Users/tnest/Desktop/LostArk/Server/Private/ClientSession.cpp:415)은 완성한 frame을 `m_OnFrame(m_iSessionId, frame)`으로 전달한다. 이 순간 [On_SessionFrame](C:/Users/tnest/Desktop/LostArk/Server/Private/ServerApp.cpp:2739)도 **그 세션의 수신 스레드**에서 실행된다.

ServerApp은 payload로 `CPacketReader`를 만들고 [C2S_MOVE 분기](C:/Users/tnest/Desktop/LostArk/Server/Private/ServerApp.cpp:2862)에서 `Read_Message`를 호출한다. [Read_Message(C2S_MOVE)](C:/Users/tnest/Desktop/LostArk/Shared/Private/Network/PacketMessages.cpp:2155)는 임시 `decoded`에 읽고 값 검증까지 성공한 뒤 출력 인자에 대입한다. 호출자는 payload 뒤에 설명되지 않은 바이트가 남았는지도 검사한다. `Read_Message`의 출력 인자가 `const`가 아닌 이유는 해석한 결과를 그 인자에 써야 하기 때문이다.

성공하면 `ROOM_COMMAND`의 종류를 `MOVE`, 세션 번호를 방금 수신한 세션의 번호, 내용을 해석한 `move`로 채운다. 클라이언트가 다른 플레이어 번호를 넣어 누구를 이동시킬지 지정하는 경로가 아니다. **명령의 발신자 identity는 연결에서 결정된다.**

[Enqueue_AssignedCommand](C:/Users/tnest/Desktop/LostArk/Server/Private/ServerApp.cpp:4130)는 `m_GameplayBindingBySessionId`에서 이 세션이 속한 simulation을 찾아 [CGameRoom::Enqueue_Detailed](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom.cpp:246)를 호출한다. 이때 잠금 순서는 `m_SessionsMutex → m_CommandMutex`다. 방을 조회한 직후 다른 방으로 이동되어 명령이 이전 방에 들어가는 상황을 막기 위해 조회와 enqueue를 같은 세션 잠금 범위에 둔다.

여기까지는 플레이어의 위치를 바꾸지 않는다. `m_InboundCommands`에 작업을 보관했을 뿐이다. 여러 수신 스레드가 같은 방에 들어오더라도 큐 삽입은 `m_CommandMutex`로 보호된다. `std::deque`가 들어 있다고 해서 Chase–Lev deque나 lock-free 큐인 것은 아니다.

이동 목표를 빠르게 다시 보내면 [Try_RemoveCoalescedCommand](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_Helpers.cpp:317)가 같은 세션의 이전 이동을 최신 이동으로 합칠 수 있다. 다만 같은 세션의 스킬·해제 같은 다른 명령이 중간에 있으면 그 경계를 넘지 않는다. `이동 A → 스킬 → 이동 B`를 임의로 `스킬 → 이동 B`로 바꾸지 않으려는 규칙이다.

입력 큐는 무한히 증가하지 않는다. [GameRoom.h](C:/Users/tnest/Desktop/LostArk/Server/Public/GameRoom.h:1785)의 현재 상한은 일반 best-effort 입력 768개, reliable 입력을 포함한 상한 960개, tick당 일반 명령 처리 최대 256개다. 이동·조준은 혼잡할 때 생략할 수 있지만 reliable 명령의 공간 부족은 별도 실패로 처리한다. `LEAVE`는 별도 cleanup 큐에 넣어 이동 입력이 많아도 퇴장 정리가 밀려나지 않게 한다.

## G05. room 스레드가 이동을 검증하고 실제 위치를 바꾼다

[Room_Loop](C:/Users/tnest/Desktop/LostArk/Server/Private/ServerApp.cpp:2705)는 고정 간격 `1/30초`로 [Tick_GameplaySimulations](C:/Users/tnest/Desktop/LostArk/Server/Private/ServerApp.cpp:5017)를 호출한다. 이 함수는 shared world, 세션 전용 Character Select, 콜로세움 match의 `shared_ptr<CGameRoom>` 목록을 확보한 뒤 세션 잠금을 풀고 각 방을 **차례대로** Tick한다. 현재는 방마다 전용 스레드를 두거나 JobSystem으로 방 Tick을 병렬 실행하는 구조가 아니다.

App의 컨테이너는 방과 세션을 소유한다. [GameRoom의 m_Sessions](C:/Users/tnest/Desktop/LostArk/Server/Public/GameRoom.h:1835)는 `weak_ptr<CClientSession>`이라 세션의 영구 수명을 소유하지 않는다. 송신할 때 `Find_Session`이 잠깐 `shared_ptr`를 얻는다. 실제 플레이어 상태는 바로 다음 `m_Players`가 소유한다. `shared_ptr`는 수명을 관리하는 장치이며, 그 객체 내부의 동시 변경을 자동으로 안전하게 만들어 주지는 않는다.

[CGameRoom::Tick](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom.cpp:840)은 명령 큐 잠금 안에서 이번에 처리할 명령만 지역 deque로 옮긴다. 잠금을 푼 다음 cleanup을 먼저 처리하고 `MOVE`이면 918행의 `Handle_Move`로 보낸다. 긴 이동·전투 계산 동안 큐 잠금을 잡지 않으므로 수신 스레드는 다음 입력을 계속 넣을 수 있다.

[Handle_Move](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_PlayerCommands.cpp:32)는 `sessionId → playerId → SERVER_PLAYER`를 찾아 [Execute_PlayerMove](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_PlayerCommands.cpp:57)를 호출한다. 이 함수는 이전보다 새로운 요청 번호인지, 좌표가 유한하고 범위 안인지, 현재 행동 상태가 이동을 허용하는지 검사한다. 정상적인 지상 이동은 [Commit_MoveGoal](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_PlayerCommands.cpp:204)로 연결되어 navigation의 직선 이동 가능 여부나 경로를 확인한다. 새 경로를 만들지 못하면 진행 중인 기존 경로를 먼저 지우지 않는다.

목표를 저장했다고 곧바로 그 위치로 순간이동하지 않는다. Tick 안의 [Update_Players](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom.cpp:1168)가 [속도 × fixedDeltaSeconds](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_PlayerSimulation.cpp:1183)만큼 이번 이동 거리를 구한다. 이후 [충돌 해결과 최종 navigation 검사](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_PlayerSimulation.cpp:1223)를 통과한 좌표를 1271행에서 `player.fPositionX/Y/Z`에 반영한다.

**이 한 스레드가 실제 gameplay 상태를 쓰는 주체라는 점이 설계의 중심이다.** 네 명의 이동과 스킬 패킷이 동시에 도착해도 네 수신 스레드가 HP·위치를 직접 동시에 수정하지 않는다. 코드에서 실행 순서를 따라가기 쉽고, 게임 상태 곳곳에 잠금을 추가할 필요가 줄어든다. 대신 한 방의 계산이 오래 걸리면 같은 room 루프의 다른 방과 snapshot도 늦어질 수 있다. 30 Hz는 목표 주기이며 모든 부하에서 지켜진다는 보장이 아니다.

참고로 [RoomCommand.h](C:/Users/tnest/Desktop/LostArk/Server/Public/RoomCommand.h:15)의 “현재 베른 하나이므로 GameRoom 하나”라는 기존 주석은 현재 App의 여러 simulation 경로와 다르다. 이 학습 자료는 실제 호출 코드를 기준으로 설명한다.

## G06. 확정된 상태를 snapshot으로 만들어 네 세션에 배포한다

Tick은 [m_iServerTick 갱신](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom.cpp:1304) 뒤 인간 플레이어가 있으면 `Broadcast_WorldSnapshot()`을 호출한다. [Broadcast_WorldSnapshot](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_Replication.cpp:494)은 방의 상태를 `S2C_WORLD_SNAPSHOT`에 담는다. 예시 플레이어의 확정 좌표와 마지막 처리 이동 번호 17도 519~523행에서 기록한다.

여기서 snapshot은 “이 tick에 서버가 확정한 상태 묶음”이다. 클라이언트에 이동 요청을 그대로 되돌리는 것이 아니다. 다른 플레이어·월드 객체·전투 상태도 같은 방의 결과에 포함된다.

880행부터 payload를 `Write_Message`로 **한 번 직렬화**한다. 903행부터 대상 세션을 순회하면서 `Send_Frame(S2C_WORLD_SNAPSHOT, writer.Get_Buffer())`를 호출한다. 따라서 네 명이라고 payload의 복잡한 직렬화를 네 번 하는 구조는 아니다. 다만 각 세션의 `Send_Frame`은 별도의 frame 바이트를 구성해 자기 큐에 넣으므로 완전한 무복사 공유 전송도 아니다.

[CClientSession::Send_Frame](C:/Users/tnest/Desktop/LostArk/Server/Private/ClientSession.cpp:327)의 이름만 보고 즉시 socket `send`까지 완료됐다고 읽으면 안 된다. 실제 책임은 **frame 생성과 송신 큐 접수**다. 성공 반환은 상대 게임이 처리했다는 ACK가 아니다. snapshot은 큐 정책에 따라 합쳐지거나 버려져도 이 호출이 성공으로 반환할 수 있다.

## G07. 송신 큐가 느린 접속자의 영향을 제한한다

[Queue_OutboundFrame](C:/Users/tnest/Desktop/LostArk/Server/Private/ClientSession.cpp:520)은 `m_OutboundMutex`를 잡아 세션의 `m_OutboundFrames`를 다룬다. `S2C_WORLD_SNAPSHOT`이면 아직 큐에 남은 이전 snapshot을 제거하고 최신 것을 넣는다. 이미 송신 스레드가 꺼내 보낸 frame까지 취소하는 것은 아니다.

느린 클라이언트에게 오래된 위치 snapshot을 계속 쌓으면, 연결은 유지되어도 화면은 과거 상태를 뒤늦게 따라가게 된다. 최신 상태를 남기는 정책은 이 누적을 줄인다. 반면 입장 승인·생성·인벤토리 응답 같은 다른 packet type은 이 큐에서 reliable로 취급되어 서로의 FIFO 순서를 유지한다. [큐 상한](C:/Users/tnest/Desktop/LostArk/Server/Public/ClientSession.h:183)은 4096 frame/8 MiB이고, snapshot에는 reliable 여유분 16 frame/128 KiB를 남긴다. reliable 상한을 넘으면 조용히 성공 처리하지 않고 연결 종료 사유를 기록한다.

두 종류 모두 실제 전송은 TCP다. 여기서 best-effort는 UDP라는 뜻이 아니라 **애플리케이션 큐에 넣기 전후에 오래된 정보를 생략할 수 있다는 정책**이다. 또한 현재 snapshot에는 [DamageEvents와 BossCombatEvents](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_Replication.cpp:858)도 들어간다. 따라서 snapshot에 넣은 일회성 이벤트까지 항상 전달된다고 설명하면 안 된다. 반드시 관측해야 하는 사건은 별도 reliable 전달 또는 다음 snapshot에도 유지되는 상태 계약이 필요한지 구분해야 한다.

[Sender_Loop](C:/Users/tnest/Desktop/LostArk/Server/Private/ClientSession.cpp:437)은 condition variable로 큐를 기다린다. 깨어나면 잠금 안에서 첫 frame을 꺼내고 **잠금을 해제한 뒤** [Send_All](C:/Users/tnest/Desktop/LostArk/Server/Private/ClientSession.cpp:730)을 호출한다. 네 명 중 한 명의 소켓이 막혀도 그 대기 때문에 room 스레드가 직접 `send`를 기다리지 않는 이유다. 물론 queue lock·메모리 복사·CPU 부하까지 완전히 사라지는 것은 아니다.

`send`가 frame 전체보다 적은 바이트만 받으면 `sentByteCount`를 늘리고 나머지 구간을 보낸다. `WSAEWOULDBLOCK`이면 `select`로 쓰기 가능 상태를 기다린다. 250 ms의 진행 정지는 진단 대상으로 기록하지만 그 자체로 정상 연결을 바로 닫지는 않는다. 종료 응답을 비우는 별도 terminal drain에는 2초 제한이 있다.

클라이언트가 수신한 상태의 적용 시작점까지 보고 싶다면 [CClientReplication::Apply_WorldSnapshot](C:/Users/tnest/Desktop/LostArk/Client/Private/ClientReplication.cpp:4076)을 이어 읽는다. 이번 문서는 서버 호출 경로를 설명하며, 실제 게임 화면에서의 보간·표시 결과를 실행 검증한 자료는 아니다.

## G08. 네 명이 함께 이동할 때 reliable transaction이 필요한 이유

파티가 함께 월드를 옮기는 작업은 한 명의 snapshot보다 더 강한 준비 절차가 필요하다. 한 명의 큐에는 입장 응답을 넣었는데 다른 사람의 큐가 꽉 찼다면, gameplay만 먼저 옮겼을 때 일부 사람에게만 결과가 보일 수 있기 때문이다.

[RELIABLE_BATCH_TRANSACTION::Prepare](C:/Users/tnest/Desktop/LostArk/Server/Private/ClientSession.cpp:60)는 대상 세션을 세션 번호순으로 정렬하고 `DiagnosticMutex → OutboundMutex` 순서로 잠근다. 각 큐의 상태·용량·frame 인코딩을 검사하여 임시 큐를 준비한다. 여기서는 socket I/O를 하지 않는다. 모든 준비가 성공한 뒤 [Commit](C:/Users/tnest/Desktop/LostArk/Server/Private/ClientSession.cpp:138)이 큐를 교체하고 잠금을 풀어 송신 스레드를 깨운다. Commit 전에 실패하면 준비분을 실제 큐에 게시하지 않는다.

이 장치는 **서버의 gameplay 변경과 송신 준비를 일관되게 묶기 위한 것**이다. 이미 끊어진 네트워크까지 복구하거나 네 클라이언트가 같은 물리 시각에 결과를 수신하게 하는 분산 transaction은 아니다. 잠금 순서를 맞추는 것도 교착 가능성을 줄이는 설계 규칙이지, 모든 실행에서 교착이 없다는 증명은 아니다.

## G09. 면접에서는 구현 범위와 개선 후보를 나눠 설명한다

현재 코드에 대한 답변은 다음처럼 할 수 있다.

> 네 명이 플레이하는 상황을 기준으로, TCP 수신과 gameplay 상태 변경을 큐로 분리했습니다. 세션별 수신 스레드는 프레임 복원과 메시지 검증까지 하고, 이동·스킬은 방의 명령 큐에 넣습니다. 실제 위치·전투 상태는 하나의 30 Hz room 스레드에서 순서대로 변경합니다. 결과 snapshot은 방에서 한 번 직렬화한 후 각 세션 송신 큐에 넣고, 별도 송신 스레드가 전송합니다. 입력과 출력 큐에 상한을 두고 이동·snapshot은 조건부로 최신 값에 합치며, reliable 메시지는 순서와 실패를 별도로 관리합니다.

“왜 멀티스레드인데 gameplay는 한 스레드인가?”에는 수신·송신 대기와 gameplay 계산을 분리하여 다른 연결의 대기 영향을 줄이고, 게임 상태 변경의 실행 순서를 명확하게 유지하려는 선택이라고 답하면 된다. 단일 room 루프가 병목이 되면 동시 방 수·tick 시간·snapshot 비용을 먼저 측정한 뒤 분리 범위를 결정해야 한다.

현재의 [tick 시간 계측](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom.cpp:845), [snapshot 직렬화·enqueue 시간 계측](C:/Users/tnest/Desktop/LostArk/Server/Private/GameRoom_Replication.cpp:880), [송신 정지 진단](C:/Users/tnest/Desktop/LostArk/Server/Private/ClientSession.cpp:807)은 구간 비용과 혼잡을 보는 장치다. 이것만으로 “deadlock profiling을 구현했다”고 답하지 않는다. 잠금 획득 대기 시간·보유 관계·순환 탐지를 별도로 수집하는 기능은 이 경로에서 구현되어 있지 않다.

IOCP는 세션별 I/O 대기 스레드를 고정된 완료 처리 worker로 바꾸는 후보이고, JobSystem은 분리 가능한 CPU 작업을 worker에 배분하는 후보다. 둘을 도입해도 방의 상태를 누가 변경하는지에 관한 규칙은 따로 유지해야 한다. 현재 제품의 방 입력 큐를 Chase–Lev deque라고 부르지 않는다. 현재 packet codec도 Shared의 자체 `PacketWriter/Reader`이며, Google Protobuf나 FlatBuffers를 쓰는 코드로 설명하지 않는다.

## G10. Visual Studio에서 한 번에 읽을 분량

한국어 설명 주석이 붙은 전문 복사본도 만들었다. 이 네 파일은 현재 제품 소스를 보존한 참고 자료이며 IOCP 후보로 바꾼 코드가 아니다. 원본과 줄 번호는 다르므로 함수 이름으로 대조한다.

- [PacketFrame.cpp 복사본](C:/Users/tnest/Desktop/LostArk/Study/CodeWalkthrough/Reference/Shared/Private/Network/PacketFrame.cpp): header를 붙이고 검증하는 두 함수.
- [PacketStreamParser.cpp 복사본](C:/Users/tnest/Desktop/LostArk/Study/CodeWalkthrough/Reference/Shared/Private/Network/PacketStreamParser.cpp): TCP 바이트를 frame으로 잘라 내는 과정.
- [ClientSession.cpp 복사본](C:/Users/tnest/Desktop/LostArk/Study/CodeWalkthrough/Reference/Server/Private/ClientSession.cpp): 연결 수명, 수신·송신 스레드, 큐, reliable 준비와 종료.
- [GameRoom_Replication.cpp 복사본](C:/Users/tnest/Desktop/LostArk/Study/CodeWalkthrough/Reference/Server/Private/GameRoom_Replication.cpp): snapshot을 만들고 세션으로 넘기는 과정.

첫 번째로 `PacketMessages.h`의 `C2S_MOVE`, `PacketFrame.cpp`, `PacketStreamParser.cpp`를 읽어 **23바이트가 어떻게 한 이동 요청으로 복원되는지** 확인한다. `decoded`와 `m_iReadOffset`의 의미를 말할 수 있으면 다음으로 넘어간다.

두 번째로 `ClientSession.cpp`의 `Receive_Loop/Receive_Frame`과 `ServerApp.cpp`의 `On_SessionFrame/Enqueue_AssignedCommand`를 읽는다. 각 함수 옆에 “현재 수신 스레드”라고 적고, `Enqueue_Detailed`까지 실제 위치는 바뀌지 않는다는 점을 확인한다.

세 번째로 `Room_Loop → Tick_GameplaySimulations → GameRoom::Tick → Handle_Move → Update_Players`를 읽는다. 여기에는 “room 스레드, gameplay 상태 변경”이라고 적는다. 네 명의 패킷이 동시에 도착해도 상태 변경을 한 곳에 모으는 이유를 설명한다.

네 번째로 `Broadcast_WorldSnapshot → Send_Frame → Queue_OutboundFrame → Sender_Loop → Send_All`을 읽는다. `Send_Frame`까지는 room 스레드이고 `Sender_Loop`부터는 송신 스레드다. enqueue 성공과 상대 수신 성공이 다름을 설명하면 한 바퀴가 끝난다.

검증 범위: 위 함수 정의·호출자·큐·상수와 실제 IOCP 연결 여부를 정적으로 대조했다. 이 문서와 주석 복사본만 작성했으며 원본 C++·프로젝트·데이터는 수정하지 않았다. 복사본의 주석·공백을 제외한 C++ token 순서는 원본과 일치한다. 원본/복사본 SHA-256·인코딩·행수와 비교 결과는 [server-copy-manifest.json](C:/Users/tnest/Desktop/LostArk/Study/CodeWalkthrough/server-copy-manifest.json)에 보관했다. 서버·클라이언트·하네스 실행과 성능 재측정은 하지 않았다. 줄 번호는 조사 시점 기준이며 이후 편집으로 달라지면 같은 함수 이름으로 찾는다.
