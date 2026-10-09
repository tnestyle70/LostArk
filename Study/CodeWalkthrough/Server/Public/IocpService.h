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
