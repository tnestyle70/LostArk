# G01부터 따라가는 서버 비교 구현 안내

이 문서는 diff의 +/− 기호를 해석하지 않고 파일을 열어 따라가기 위한 안내다. **첫 작성 파일은 Server/Public/IocpService.h다. ClientSession.cpp부터 수정하지 않는다.** 전체 코드 정본은 [PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md)에 있다. 2026-10-09 현재 사용자가 제품 IocpService.h 선언을 작성 중이며 CPP와 세션 연결은 미반영이다. 최신 수동 보완 위치와 한국어 설명은 [학습 입구](C:/Users/tnest/Desktop/LostArk/Study/CodeWalkthrough/README.md)를 먼저 읽는다.

## G00. 지금 Visual Studio에서 열 것

[CodeWalkthrough.sln](C:/Users/tnest/Desktop/LostArk/Study/CodeWalkthrough/CodeWalkthrough.sln)을 연다. 이전 out/ServerConcurrency20261009 학습 폴더는 현재 디스크에 없어 그 경로를 새 안내로 사용하지 않는다.

`00.Start`의 README에서 현재 헤더의 세 군데 보완을 먼저 확인한다. `01.IOCP`는 한국어 주석이 있는 헤더·CPP이고 나머지 필터는 서버·프로파일러·셰이더 설명용 사본이다. 이 솔루션은 IOCP 서비스의 선언·정의를 정적 라이브러리로 컴파일하며 실행 EXE나 성능 하네스는 제공하지 않는다. 제품 서버를 IOCP로 바꾸거나 과거 벤치마크를 재현한 것으로 해석하지 않는다.

기호의 정의는 F12, 사용하는 위치는 Shift+F12로 확인한다. 원본 제품과 학습 사본을 탭의 절대 경로로 구분한다.

## G01. IocpService.h — 다른 파일이 사용할 이름부터 선언

원본 반영 위치: `C:/Users/tnest/Desktop/LostArk/Server/Public/IocpService.h` (파일 생성·프로젝트 등록 완료, 선언 작성 중). 읽을 완성본: [완성본 Server/Public/IocpService.h](C:/Users/tnest/Desktop/LostArk/Study/CodeWalkthrough/Server/Public/IocpService.h). 기존 Server의 01.Network 필터에 속한다.

이 파일의 역할은 **여러 연결이 함께 쓸 IOCP 서비스의 약속을 선언하는 것**이다. 헤더를 추가하는 것만으로 스레드가 생기거나 IOCP가 실행되지는 않는다. 실제 동작은 다음 단계 CPP에 둔다. 다음 순서로 읽는다.

1. SESSION_TRANSPORT_BACKEND: 이 세션이 기존 SELECT_THREADS로 동작할지 IOCP로 동작할지 정한다.
2. IOCP_OPERATION_KIND: 완료된 작업이 RECEIVE인지 SEND인지 알려준다.
3. IIocpParticipant: 서비스가 세션에게 “작업이 끝났다”와 “주기 점검하라”를 전달하는 함수 계약이다.
4. IOCP_SERVICE_METRICS: 제출 수, 완료 수, 남은 수, 송수신 byte를 기록한다.
5. CIocpService: completion port와 고정 개수의 worker를 소유한다. CServerApp이 이 객체를 생성하고 마지막에 종료한다.

WinSock2.h는 SOCKET/WSABUF, Windows.h는 HANDLE/OVERLAPPED를 제공하며 이 순서로 include한다. shared_ptr는 완료까지 객체를 보존하고 weak_ptr는 관찰 목록이 세션을 영구 소유하지 않게 한다. atomic은 완료·실행 상태, mutex/condition_variable은 종료 대기와 목록 보호, span/vector는 byte 범위와 보관, thread는 worker, chrono는 종료 제한시간에 사용한다.

Start/Stop은 서비스 전체의 생성과 종료, Associate는 socket 연결, Post_Receive/Post_Send는 작업 제출이다. m_Port와 m_Workers는 서비스가 소유한다. m_Pending은 아직 완료 처리가 끝나지 않은 수이고 Stop은 이것이0이 되기를 기다린다. m_Participants는 주기 점검할 세션의 약한 참조 목록이다.

아래가 첫 파일의 완성본이다. +/−, @@ 표시를 복사하는 작업은 없다.

```cpp
#pragma once

// 이 파일은 IOCP 전송 서비스를 학습하기 위한 선언 완성본이다.
// 적용 위치: Server/Public/IocpService.h. 대응 구현: Server/Private/IocpService.cpp.
// 현재 제품 연결 여부는 별개다. 이 파일을 추가하는 것만으로 기존 서버가 IOCP로 바뀌지는 않는다.

// WinSock2가 SOCKET/WSABUF를, Windows가 HANDLE/OVERLAPPED를 제공한다.
// Windows.h가 구형 Winsock 선언을 먼저 포함하지 않도록 이 순서를 유지한다.
#include <WinSock2.h>
#include <Windows.h>

// 헤더를 포함하는 것만으로 경쟁 상태/데드락이 없어지지는 않는다.
// atomic은 한 변수의 연산, mutex는 함께 지켜야 하는 상태 묶음을 실제 코드에서 보호할 때 효과가 있다.
#include <atomic>             // 여러 워커가 공유하는 실행 상태와 완료 횟수.
#include <chrono>             // 종료 대기 제한시간. 시간 단위를 타입으로 구분한다.
#include <condition_variable> // 미처리 I/O가 0이 될 때 종료 담당 스레드를 깨운다.
#include <cstddef>            // 버퍼 길이와 인덱스에 사용하는 size_t.
#include <cstdint>            // 바이트와 누적 카운터의 고정 폭 정수.
#include <memory>             // 작업의 단독 소유, 세션의 강한 참조, 관찰용 약한 참조.
#include <mutex>              // 종료 대기 조건과 참여자 목록 보호.
#include <span>               // 완료 콜백 동안만 빌려 주는 수신 바이트 범위.
#include <thread>             // 서비스가 생성하고 종료 시 join하는 고정 워커 목록.
#include <vector>             // 워커·참여자 목록 및 송신 프레임 저장소.

namespace LostArk::Server
{
    // 이후 ClientSession이 선택할 전송 경로. 게임 규칙이나 패킷 형식을 바꾸는 값이 아니다.
    // SELECT_THREADS는 기존 세션별 수신/송신 스레드, IOCP는 공용 완료 포트 워커를 뜻한다.
    enum class SESSION_TRANSPORT_BACKEND { SELECT_THREADS, IOCP };

    // 같은 완료 포트로 들어온 알림이 수신인지 송신인지 구분한다.
    // OVERLAPPED만으로는 우리 서비스의 작업 종류를 알 수 없으므로 작업 객체에 함께 저장한다.
    enum class IOCP_OPERATION_KIND { RECEIVE, SEND };

    // 서비스가 CClientSession의 게임/큐 구현을 직접 알지 않아도 되도록 만든 콜백 계약이다.
    // 연결 단계에서 CClientSession이 구현한다. 서비스는 바이트 완료를 알리고,
    // 세션은 패킷 조립·송신 offset·연결 종료를 처리한다. GameRoom 상태는 여기서 바꾸지 않는다.
    class IIocpParticipant
    {
    public:
        // Participant는 "완료 통지를 받을 참여자"라는 뜻이다. 여기서는 이후의 CClientSession이다.
        // 순수 가상 함수(=0)는 인터페이스만 정하고 실제 세션 처리는 파생 클래스에 맡긴다.
        // 가상 소멸자는 기반 타입을 통해 파괴할 때도 파생 클래스 정리가 실행되게 하는 계약이다.
        virtual ~IIocpParticipant() = default;

        // 호출자: CIocpService::Worker_Loop의 IOCP 워커.
        // kind는 수신/송신, bytes는 이번 완료의 바이트 수, error는 Windows 오류 코드다.
        // received는 수신 때만 유효하며 소유권이 없는 span이다. 콜백 반환 후 보관하면 안 된다.
        // 작업 객체가 Owner의 shared_ptr를 잡아 취소 완료를 포함한 콜백 전체 동안 세션을 살린다.
        // 서비스의 목록/종료 mutex를 잡지 않은 상태로 호출한다. 세션 내부 공유 상태는
        // 세션이 동기화해야 하며 서로 다른 작업·maintenance 콜백의 직렬 실행을 보장하지 않는다.
        virtual void On_IocpCompleted(IOCP_OPERATION_KIND kind,
            std::span<const std::uint8_t> received, std::uint32_t bytes,
            std::uint32_t error) noexcept = 0;

        // 호출자: 워커의 Run_Maintenance. 조용한 연결도 시간 초과 등을 확인할 기회를 준다.
        // 약 100ms마다 점검을 시도하지만 실시간 주기 보장은 아니다. 게임의 30Hz Tick과 다르다.
        virtual void On_IocpMaintenance() noexcept = 0;
    };

    // Get_Metrics가 반환하는 값 복사본. atomic 자체를 외부에 공개하지 않는다.
    // 여러 카운터를 차례로 읽으므로 실행 중 완전히 같은 순간의 스냅샷은 아니다.
    // I/O 완료 횟수는 게임 패킷 개수와 다르다. TCP에서는 한 패킷이 여러 수신으로 쪼개질 수 있다.
    // 한 번의 bytes는 Windows DWORD에 맞는 uint32_t지만 누적 합계는 훨씬 커져 uint64_t로 둔다.
    // 예를 들어 uint32_t 누적 바이트는 약 4GiB에서 넘친다. 64비트는 장시간 누적의 여유를 준다.
    struct IOCP_SERVICE_METRICS final
    {
        std::size_t iWorkerCount = 0;                  // 서비스가 보유한 워커 수.
        std::uint64_t iPostedOperations = 0;          // 즉시 성공 또는 PENDING으로 접수된 작업 수.
        std::uint64_t iCompletedOperations = 0;       // 성공/실패 콜백을 처리한 완료 수.
        std::uint64_t iPendingOperations = 0;         // 제출 예약 후 아직 정리되지 않은 작업 수.
        std::uint64_t iPeakPendingOperations = 0;     // 위 미처리 수의 최대값.
        std::uint64_t iReceiveCompletions = 0;        // 오류를 포함한 수신 완료 알림 수.
        std::uint64_t iSendCompletions = 0;           // 오류를 포함한 송신 완료 알림 수.
        std::uint64_t iPartialSendCompletions = 0;    // 성공했지만 요청 길이보다 짧은 양의 송신 완료.
        std::uint64_t iReceivedBytes = 0;             // 완료 알림이 보고한 수신 바이트 합계.
        std::uint64_t iSentBytes = 0;                 // 완료 알림이 보고한 송신 바이트 합계.
    };

    // CServerApp 또는 독립 실험기가 서비스 하나를 소유하고 여러 세션이 공유하는 구조다.
    // 세션마다 기다리는 스레드를 만들지 않고 완료된 작업을 고정 개수의 워커가 처리한다.
    // 그 대신 작업/버퍼 수명, 부분 송신, 취소 완료와 종료 순서를 명시적으로 관리해야 한다.
    //
    // 수명 계약: 모든 세션의 새 제출을 막고 소켓을 닫아 취소를 시작한 뒤 이 서비스를 Stop한다.
    // Stop과 새 Submit을 임의로 동시에 호출해도 안전한 범용 큐가 아니다.
    // m_Running의 atomic 검사 한 번은 제출자 전체를 멈추는 장벽이 될 수 없다.
    // 서비스는 마지막 세션과 완료 콜백보다 오래 살아 있어야 한다.
    class CIocpService final
    {
    public:
        CIocpService() = default;
        ~CIocpService(); // 소유자가 종료를 누락해도 Stop 경로로 정리한다.

        // HANDLE과 실행 중 워커를 복사하면 이중 종료가 생기므로 복사를 금지한다.
        CIocpService(const CIocpService&) = delete;
        CIocpService& operator=(const CIocpService&) = delete;

        // 호출자: 이후 ServerApp의 시작 단계. 포트 생성 후 워커를 시작한다.
        // 0은 CPU 동시 실행 수를 참고하며 이 구현은 1~16개로 제한한다. 최적값 보장은 아니다.
        bool Start(std::size_t workerCount = 0);

        // 호출자: 워커가 아닌 소유자 스레드. 세션 종료 후 완료를 모두 회수하고 워커를 join한다.
        // 5000은 5초 후 종료를 예약한다는 뜻이 아니다. 지금 종료를 시작하고 기다릴 상한을 지정한다.
        // 미완료 버퍼를 해제한 채 계속 실행하지 않도록 제한시간 초과 시 프로세스를 종료한다.
        // timeout은 drain 단계와 워커 join 단계에 각각 사용되며 전체 5초 보장이 아니다.
        void Stop(std::chrono::milliseconds timeout = std::chrono::milliseconds{5000});

        // accept된 overlapped 소켓을 이 포트에 연결한다. accept/listen/WSAStartup은 외부 책임이다.
        bool Associate(SOCKET socket);

        // 주기 점검 대상에 약한 참조를 넣는다. 세션 소유자가 사라지면 목록 때문에 살아남지 않는다.
        // 한 세션은 한 번 등록한다. 이 함수는 중복 등록을 제거하지 않는다.
        void Register(const std::shared_ptr<IIocpParticipant>& participant);

        // 호출자: 세션 시작 또는 직전 수신 완료. 4096-byte 버퍼와 owner를 보존해 수신을 맡긴다.
        // owner는 null이 아니어야 한다. true는 접수 성공이며 데이터 도착을 뜻하지 않는다.
        bool Post_Receive(SOCKET socket, const std::shared_ptr<IIocpParticipant>& owner,
            int& error);

        // 호출자: 세션의 송신 시작/부분 완료 처리. bytes 전체 중 offset 이후 구간을 보낸다.
        // 변경 불가능한 프레임을 공유 소유하여 비동기 완료까지 메모리가 유지되게 한다.
        // owner는 null이 아니어야 한다. bytes도 완료 전까지 다른 mutable 참조로 수정하거나
        // 재할당하지 않아야 한다. shared_ptr<const> 하나가 다른 참조의 쓰기까지 금지하지는 않는다.
        // 여기서는 offset을 누적하지 않는다. 실제 완료 bytes를 보고 세션이 다음 offset을 정한다.
        bool Post_Send(SOCKET socket, const std::shared_ptr<IIocpParticipant>& owner,
            std::shared_ptr<const std::vector<std::uint8_t>> bytes,
            std::size_t offset, int& error);

        // [[nodiscard]]는 반환값을 버리는 호출에 컴파일러가 진단하도록 권하는 표시다.
        // 값을 버리지 못하게 런타임에서 강제하거나 스레드 안전성을 추가하는 기능은 아니다.
        // 자기 자신인 워커를 join하려는 잘못된 종료 호출을 감지한다.
        [[nodiscard]] bool Is_WorkerThread() const noexcept;

        // Start/Stop이 m_Workers를 바꾸는 동안 동시에 호출하지 않는다.
        [[nodiscard]] std::size_t Get_WorkerCount() const noexcept;
        [[nodiscard]] IOCP_SERVICE_METRICS Get_Metrics() const noexcept;

    private:
        // 외부가 포트·pending·작업 목록을 직접 바꾸면 "접수 한 번/회수 한 번" 순서를 깨뜨린다.
        // public 함수로 가능한 동작만 허용하고 내부 표현과 정리 순서는 private에서 유지한다.
        // OVERLAPPED와 Owner/버퍼의 실제 배치는 CPP에 숨긴다. 외부에는 제출 함수만 공개한다.
        // 이것이 중첩 타입의 전방 선언이다. 이 시점에는 크기를 몰라도 함수 선언에 사용할 수 있다.
        struct OPERATION;

        // Post_Receive/Post_Send 공통 경로. pending 예약과 Windows 호출·접수 실패 정리를 소유한다.
        bool Submit(SOCKET socket, std::unique_ptr<OPERATION> operation, int& error);
        void Worker_Loop() noexcept;       // 완료 포트 대기 → 작업 회수 → 세션 콜백 → pending 감소.
        void Run_Maintenance() noexcept;   // 유효한 세션 목록 복사 → 잠금 해제 → 주기 점검 콜백.
        void Complete_Pending() noexcept;  // 접수 실패/완료 처리에서 작업 하나를 정리하고 0이면 알림.

        HANDLE m_Port = nullptr;                // 서비스 소유의 완료 포트. worker join 뒤 CloseHandle.
        std::vector<std::thread> m_Workers;     // Start/Stop 소유자 스레드가 생성·정리하는 워커.
        std::atomic_bool m_Running{false};      // 새 제출 허용 상태. 이미 접수한 작업의 완료는 계속 회수.
        // atomic의 fetch_add는 다른 스레드와 겹쳐도 증가 한 건을 잃지 않는다.
        // 주변의 다른 변수까지 잠그거나 여러 atomic을 하나의 transaction으로 만들지는 않는다.
        // atomic 자체가 모든 플랫폼에서 lock-free라는 보장도 없다.
        std::atomic<std::uint64_t> m_Pending{0}, m_PeakPending{0}, m_Posted{0}, m_Completed{0};
        // 아래 두 줄로 나눈 것은 각각 완료 횟수와 바이트 합계라는 의미를 묶은 것이다.
        // 줄을 나눈다고 동기화 범위가 달라지지는 않는다. 각 atomic은 서로 독립된 변수다.
        std::atomic<std::uint64_t> m_ReceiveCompletions{0}, m_SendCompletions{0}, m_PartialSends{0};
        std::atomic<std::uint64_t> m_ReceivedBytes{0}, m_SentBytes{0};

        // wait_for의 조건 검사와 pending=0 알림을 함께 보호해 종료 대기의 깨움 누락을 막는다.
        // 단순 spin으로 계속 CPU를 쓰지 않게 condition_variable을 사용한다.
        std::mutex m_DrainMutex;
        std::condition_variable m_Drained;

        // 목록 변경/복사만 잠근다. 세션 콜백까지 잠그면 세션 잠금과 역순 경합이 생길 수 있다.
        std::mutex m_ParticipantsMutex;
        std::vector<std::weak_ptr<IIocpParticipant>> m_Participants;

        // GetTickCount64 기준 다음 점검 시각(ms). CAS에 성공한 워커가 그 차례의 점검을 맡는다.
        // 콜백이 100ms보다 오래 걸리면 다음 차례와 겹칠 수 있으므로 세션 동기화가 필요하다.
        std::atomic<ULONGLONG> m_NextMaintenance{0};
    };
}
```

첫 단계에서 확인할 것은 “backend enum과 service 타입을 다른 헤더가 사용할 수 있다”는 점이다. 함수 정의 CPP와 이후 연결까지 완료하기 전에는 전체 서버 빌드 완료 단계가 아니다. 현재 학습 솔루션은 이 서비스 H/CPP만 컴파일한다. 세션·ServerApp 연결은 별도 다음 단계이며 PLAN의 해당 전문과 현재 제품 코드를 대조한다.

## G02. IocpService.cpp — Windows에 작업을 맡기고 완료를 받음

원본 반영 위치: `C:/Users/tnest/Desktop/LostArk/Server/Private/IocpService.cpp` 신규. [완성본 Server/Private/IocpService.cpp](C:/Users/tnest/Desktop/LostArk/Study/CodeWalkthrough/Server/Private/IocpService.cpp)에서 Start → Submit → Worker_Loop → Stop 순서로 읽는다.

Start는 completion port와 worker를 만든다. Submit은 WSARecv/WSASend 전에 pending을 늘리고 operation의 소유권을 넘긴다. operation은 session의 shared_ptr와 송수신 메모리를 들고 있어 완료 전에 메모리가 사라지지 않게 한다. Worker_Loop는 GetQueuedCompletionStatus에서 완료를 받고 session의 On_IocpCompleted를 부른다. Stop은 모든 세션이 제출을 멈춘 뒤 남은 완료를 회수하고 worker를 종료한다. 이 클래스는 gameplay 상태를 수정하지 않는다.

## G03. ClientSession.h — 기존 연결 객체에 선택값과 완료 상태 추가

원본: [Server/Public/ClientSession.h](C:/Users/tnest/Desktop/LostArk/Server/Public/ClientSession.h) / [완성본 Server/Public/ClientSession.h — PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md#file-server-public-clientsession-h). 작업은 기존 클래스 확장이다. ClientSession_Iocp.cpp라는 이름의 새 클래스를 만들지 않는다.

| 실제 기준점 | 작업 | 이유·연결 |
|---|---|---|
| #include "ServerIds.h" 바로 아래 | IocpService.h include 추가 | 방금 만든 backend/service/participant 이름 사용 |
| class CClientSession final | 후보의 상속 선언으로 교체 | 완료 callback과 shared_from_this 지원 |
| public CClientSession 생성자 선언 전체 | backend/service 인자를 붙인 선언으로 교체 | ServerApp이 선택값을 전달 |
| private void Notify_Closed(); 아래, 다음 private: 위 | 후보의 IOCP 함수 선언 블록 추가 | 새 CPP에서 이 멤버들을 정의 |
| m_iSendStallOrdinal | atomic<uint64_t>로 교체 | completion과 maintenance의 접근 보호 |
| ordinal 뒤, FRAME_HANDLER m_OnFrame 앞 | 후보의 IOCP 상태 멤버 블록 추가 | 선택·pending·offset·buffer·종료 수명 저장 |

생성자 선언의 완성된 교체 블록은 다음과 같다. 헤더에만 기본값을 쓰며 CPP 정의에는 기본값을 다시 쓰지 않는다.

```cpp
CClientSession(
    SESSION_ID sessionId,
    SOCKET clientSocket,
    FRAME_HANDLER onFrame,
    CLOSED_HANDLER onClosed,
    SESSION_TRANSPORT_BACKEND backend = SESSION_TRANSPORT_BACKEND::SELECT_THREADS,
    CIocpService* iocpService = nullptr);
```

backend는 “이 연결의 송수신 방식”이다. iocpService는 CServerApp이 가진 서비스의 주소이며 세션의 소유물이 아니다. 세션은 이 포인터를 delete하지 않는다. 기본 인자를 생략하면 기존 방식을 사용하므로 기존 호출자가 한꺼번에 IOCP로 전환되지 않는다.

새 멤버는 선택값, 진행 상태, 송신 보관의 세 묶음으로 읽는다. m_IocpMutex는 post/cancel/close 순서를 보호하고, m_iIocpOutstanding은 callback이 끝날 때까지 남은 일을 센다. m_isIocpReceivePending/m_isIocpSendPending은 같은 방향에 중복 게시를 막는다. m_IocpSendBytes는 보낼 frame을, offset은 이미 보낸 byte 수를 보관한다. 기존 송수신 스레드와 outbound queue는 보존한다.

## G04. ClientSession_Iocp.cpp — 기존 클래스의 IOCP 함수 정의

원본 반영 위치: `C:/Users/tnest/Desktop/LostArk/Server/Private/ClientSession_Iocp.cpp` 신규 / [완성본 Server/Private/ClientSession_Iocp.cpp — PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md#file-server-private-clientsession-iocp-cpp). 여기의 함수는 모두 CClientSession의 멤버다. 여러 CPP가 같은 클래스의 함수들을 나누어 정의할 수 있다.

먼저 Wake_Sender를 찾는다. 선언은 앞 단계 헤더에 들어 있고 정의는 이 새 CPP에 한 번만 둔다. 후보에서는 Post_IocpSendLocked 뒤, Kick_IocpSend 앞이다.

```cpp
void LostArk::Server::CClientSession::Wake_Sender() noexcept
{
    if (SESSION_TRANSPORT_BACKEND::IOCP == m_TransportBackend) Kick_IocpSend();
    else m_OutboundCondition.notify_one();
}
```

이 함수는 보낼 데이터가 있다는 사실을 실행 방식에 맞게 전달한다. SELECT_THREADS면 대기 중인 기존 sender를 깨운다. IOCP면 Kick_IocpSend가 큐에서 frame을 꺼내 WSASend를 게시한다. 함수 이름이 Wake지만 IOCP에서는 잠든 세션별 sender를 깨우는 구조가 아니다. producer가 비동기 게시까지 할 수 있고 송신 완료를 기다리지는 않는다.

나머지는 Start_Iocp(연결·최초 수신), Post_IocpReceive/Post_IocpSendLocked(제출), On_IocpCompleted(완료 종류 분기), Handle_IocpReceive(기존 파서·callback), Handle_IocpSend(부분 송신·다음 frame), On_IocpMaintenance(정체·종료 점검) 순으로 읽는다.

## G05. ClientSession.cpp — 사용자가 올린 diff를 실제 함수로 읽기

원본: [Server/Private/ClientSession.cpp](C:/Users/tnest/Desktop/LostArk/Server/Private/ClientSession.cpp) / [완성본 Server/Private/ClientSession.cpp — PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md#file-server-private-clientsession-cpp). 파일을 열고 Ctrl+F로 CClientSession::CClientSession(을 찾는다. 생성자 시작부터 빈 본문 {}까지 다음 완성본으로 교체하는 변경이다.

```cpp
LostArk::Server::CClientSession::CClientSession(
	SESSION_ID sessionId,
	SOCKET clientSocket,
	FRAME_HANDLER onFrame,
	CLOSED_HANDLER onClosed,
	SESSION_TRANSPORT_BACKEND backend,
	CIocpService* iocpService)
	: m_iSessionId{ sessionId }
	, m_PeerEndpoint{ Resolve_PeerEndpoint(clientSocket) }
	, m_hClientSocket{ clientSocket }
	, m_TransportBackend{ backend }
	, m_pIocpService{ iocpService }
	, m_OnFrame{ std::move(onFrame) }
	, m_OnClosed{ std::move(onClosed) }
{}
```

괄호 안 backend/iocpService는 생성자가 받는 값이다. 콜론 뒤 m_TransportBackend{backend}, m_pIocpService{iocpService}는 받은 값을 이 세션의 멤버에 저장한다. 이 생성자가 IOCP service를 새로 만드는 것은 아니다. service 생성은 ServerApp의 Run이 맡는다.

다음으로 같은 CPP에서 RELIABLE_BATCH_TRANSACTION::Commit()을 찾는다. transaction이 준비한 여러 세션의 reliable queue를 확정하는 기존 함수다. 바뀌는 의미는 마지막 sender 알림 하나다. 위쪽 queue 교체와 통계, lock 해제는 유지한다.

```cpp
void LostArk::Server::CClientSession::RELIABLE_BATCH_TRANSACTION::Commit() noexcept
{
	for (const auto& staged : m_Queues)
	{
		CClientSession& session = *staged->pSession;
		session.m_OutboundFrames.swap(staged->Frames);
		session.m_iQueuedOutboundBytes = staged->iQueuedBytes;
		auto& metrics = session.m_OutboundMetrics;
		metrics.iReliableEnqueuedFrameCount += staged->iAddedFrameCount;
		metrics.iCurrentQueuedFrameCount = session.m_OutboundFrames.size();
		metrics.iCurrentQueuedByteCount = session.m_iQueuedOutboundBytes;
		metrics.iQueuedFrameHighWatermark = (std::max)(
			metrics.iQueuedFrameHighWatermark, metrics.iCurrentQueuedFrameCount);
		metrics.iQueuedByteHighWatermark = (std::max)(
			metrics.iQueuedByteHighWatermark, metrics.iCurrentQueuedByteCount);
	}
	for (const auto& staged : m_Queues)
	{
		staged->Lock.unlock();
		staged->DiagnosticLock.unlock();
	}
	for (const auto& staged : m_Queues)
		staged->pSession->Wake_Sender();
	m_Queues.clear();
}
```

흐름은 준비 큐를 세션 큐로 확정 → 통계 갱신 → 모든 lock 해제 → 각 session의 Wake_Sender 호출이다. staged는 준비해 둔 세션 큐 기록이고 pSession은 해당 연결이다. 기존 notify_one만 호출하면 세션별 sender가 없는 IOCP 모드에서는 다음 frame을 시작할 수 없어, 두 방식을 선택하는 함수로 연결한다. Kick_IocpSend가 outbound lock을 다시 잡으므로 반드시 기존 lock을 푼 뒤 호출한다.

이 두 함수 다음에는 아래 변경을 순서대로 본다. 정확한 완성된 본문은 PLAN과 학습용 파일에 있다.

| 함수 검색어 | 추가·교체할 동작 |
|---|---|
| Start | 옵션 검증 뒤 IOCP면 Start_Iocp, 기존이면 recv/send thread 생성; 시작 실패1회 통지 |
| Request_Close | IOCP일 때 CancelIoEx, post와 shutdown의 mutex 순서 보장 |
| Request_Close_After_Flush | receive 중지 후 기존 reliable queue를 제한시간 안에 송신 |
| Stop | IOCP outstanding drain, callback worker의 자기 대기 방지 |
| Queue_OutboundFrame | 마지막 sender 알림을 Wake_Sender로 연결 |
| Configure_TransportOptions | 기존은 nonblocking, IOCP는 overlapped 경로; TCP_NODELAY 유지 |
| Close_Socket | post/cancel과 동일 mutex로 socket 수명 보호 |
| Record_SendProgressDiagnostic | atomic ordinal을 load해 출력 |

## G06. Main과 ServerApp — 어떤 객체가 서비스를 만드는가

신규 [완성본 Server/Public/ServerConcurrencyOptions.h — PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md#file-server-public-serverconcurrencyoptions-h)의 Transport/IocpWorkers/SnapshotJobs/JobWorkers는 시작 설정이다. [완성본 Server/Private/Main.cpp — PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md#file-server-private-main-cpp)에서는 bool hasPort 선언 뒤 설정변수를 추가하고 argument를 읽은 직후 네 인자를 해석하며 마지막 serverApp.Run에 전달한다.

[완성본 Server/Public/ServerApp.h — PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md#file-server-public-serverapp-h)에서는 Run의 마지막 인자에 설정을 추가하고, m_PreparedColosseumRoom과 m_isRunning 사이에 설정·IOCP owner·job pool owner를 둔다. [완성본 Server/Private/ServerApp.cpp — PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md#file-server-private-serverapp-cpp)의 Run은 필요한 서비스만 생성한다. Accept_Loop의 make_shared<CClientSession> 마지막 인자에 transport와 service.get()을 전달한다.

Shutdown에서는 세션 Stop과 room 정리 뒤, 기존 m_DataRevisionResponseTombstones.clear() 바로 위에 sessions.clear → IOCP Stop/reset → JobSystem Shutdown/reset 순서를 둔다. 세션이 쓰는 service 주소가 먼저 무효가 되지 않게 하기 위한 순서다. 공유 room, Character Select private room, 비동기 Colosseum room에도 같은 선택을 넘긴다. Room_Loop의30Hz gameplay 순서는 그대로다.

## G07. JobSystem과 Chase–Lev — 전송 계층 다음에 별도로 읽기

| 작성·읽기 순서 | 완성본 | 함수·책임 |
|---|---|---|
| 1 | [완성본 Shared/Public/Concurrency/ChaseLevDeque.h — PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md#file-shared-public-concurrency-chaselevdeque-h) | Push/Pop은 소유 worker, Steal은 다른 worker; 마지막 원소 경쟁 CAS |
| 2 | [완성본 Shared/Public/Concurrency/WorkStealingJobSystem.h — PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md#file-shared-public-concurrency-workstealingjobsystem-h) | Config, JobCounter, Submit/Wait/Shutdown, 계측 계약 |
| 3 | [완성본 Shared/Private/Concurrency/WorkStealingJobSystem.cpp — PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md#file-shared-private-concurrency-workstealingjobsystem-cpp) | worker local deque, 외부 injection, 도움 실행, 완료와 종료 |
| 4 | [완성본 Server/Public/ServerSnapshotFanout.h — PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md#file-server-public-serversnapshotfanout-h) | 고정된 recipients/payload를 실행하고 결과를 반환 |
| 5 | [완성본 Server/Private/ServerSnapshotFanout.cpp — PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md#file-server-private-serversnapshotfanout-cpp) | 동일 execute(index)를 직렬 호출 또는 Submit/Wait |
| 6 | [완성본 Server/Public/GameRoom.h — PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md#file-server-public-gameroom-h) | 생성자 인자와 m_SnapshotJobs 추가 |
| 7 | [완성본 Server/Private/GameRoom.cpp — PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md#file-server-private-gameroom-cpp) | 생성자에서 전달된 pool 보관 |
| 8 | [완성본 Server/Private/GameRoom_Replication.cpp — PLAN](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_PLAN.md#file-server-private-gameroom-replication-cpp) | Broadcast_WorldSnapshot의 encode 성공 뒤 fanout 호출 |

CGameRoom::Broadcast_WorldSnapshot에서 Write_Message 성공을 확인한 뒤, 기존 세션별 Send_Frame 반복을 recipients 수집과 Dispatch_WorldSnapshot 호출로 바꾼다. snapshot을 만드는 gameplay 로직은 worker로 옮기지 않는다. Shared pool은 게임 클래스를 알지 않고 callable만 실행한다.

중요한 구분: deque 구현이 있어도 모든 작업이 그 deque를 통과하는 것은 아니다. room thread가 제출하는 현재 fanout은 외부 injection을 사용한다. 벤치마크의 localPushes/steals=0은 정상적으로 기록된 결과이며, 이것을 Chase–Lev의 속도 측정으로 해석하지 않는다.

## G08. 비교를 재실행하고 기술소개서로 연결

현재 `Study/CodeWalkthrough`는 서비스 H/CPP만 컴파일하는 학습 솔루션이다. 이전 실험 복사본과 EXE, raw 결과가 있던 `out/ServerConcurrency20261009`는 현재 디스크에 없으므로 그 위치의 재실행 명령을 지금 사용할 수 없다. 이후 전체 비교 구현을 반영할 때 PLAN에 남은 Harness 프로젝트와 실행 스크립트를 함께 구성하고 새 빌드·정확성 검사 뒤 비교한다. 이번 학습용 컴파일을 이전 벤치마크의 재실행으로 기록하지 않는다.

ComparisonProfiles.json은 조건, Run-ComparisonProfile.ps1은 실행과 기록, SnapshotFanoutBenchmark.cpp는 같은 함수의 직렬/작업 분배, JobSystemTests.cpp는 정확성 검사를 담당한다. 숫자가 달라지면 새 환경·EXE hash와 원시 데이터를 함께 보관한다. 기존 RESULT는 이전 EXE에서 완료한 측정으로 유지한다.

기술소개서는 [TECHNICAL_NOTE](C:/Users/tnest/Desktop/LostArk/.md/GB/10-09/2026-10-09_SERVER_IOCP_JOB_COMPARISON_TECHNICAL_NOTE.md)에 작성했다. 판단은 “비교 후보에 구현·측정했으나 현재 제품 기본 경로의 목표 부하에서 일관된 채택 근거를 얻지 못해 기본 경로에 도입하지 않았다”이다. 구현하지 않았다는 뜻과 실험 후 채택하지 않았다는 뜻을 구분한다. 통계 검정을 하지 않았으므로 통계적으로 유의하지 않았다고 쓰지 않는다.

## G09. 원본에 직접 작성할 때의 파일 등록

원본에 수동 작성하는 단계에서는 물리 폴더에 파일을 저장하고 Server/Shared 프로젝트의 해당 필터에서 추가 → 기존 항목으로 등록한다. 새 C++은 UTF-8 BOM 없음으로 저장한다. Shared의 Concurrency는 새 필터, Server의 Network/Main/Room은 기존 필터를 사용한다. vcxproj는 컴파일 대상, filters는 솔루션 탐색기의 논리적 분류다. [Microsoft filters 설명](https://learn.microsoft.com/en-us/cpp/build/reference/vcxproj-filters-files?view=msvc-170)

현재 학습용 솔루션은 IOCP H/CPP 두 파일만 컴파일하도록 등록했고, 나머지 학습 사본은 None 항목으로 노출한다. 원본 제품의 IocpService.h/cpp는 사용자가 생성·등록했다. 이후 파일을 옮기는 시점에는 그때의 파일과 비교해 필요한 등록만 병합하며, 오래된 프로젝트 파일 전체를 덮어쓰지 않는다.
