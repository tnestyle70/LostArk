// 한국어 설명을 붙인 IOCP 서비스 구현 완성본.
// 적용 대상은 Server/Private/IocpService.cpp이며 이 사본은 제품 프로젝트에 자동 연결되지 않는다.
// 읽는 순서: OPERATION → Start → Associate → Post_Receive/Post_Send → Submit → Worker_Loop → Stop.
// 실제 게임의 패킷 조립과 방 명령 처리는 이후 CClientSession 연결 단계의 책임이다.
#include "IocpService.h"

#include <algorithm>
#include <array>
#include <limits>
#include <exception> // std::terminate의 직접 선언. 프로세스 종료 호출이 반환할 때의 마지막 안전장치.

namespace
{
    // 각 워커가 자신이 속한 서비스를 기록한다. 전역 하나를 공유하면 다른 워커와 덮어쓴다.
    // Is_WorkerThread가 이를 비교하여 자기 스레드를 join하는 종료 교착을 막는다.
    thread_local const LostArk::Server::CIocpService* ActiveService = nullptr;
    // 완료를 회수하지 못한 상태에서는 Windows가 OVERLAPPED/버퍼에 계속 접근할 수 있다.
    // 메모리를 버리고 서버만 계속 실행하는 대신 프로세스 전체를 실패로 종료한다.
    // 워커 하나를 TerminateThread로 강제 종료하는 함수가 아니다.
    [[noreturn]] void Fail_DrainTimeout() noexcept
    {
        ::TerminateProcess(::GetCurrentProcess(), ERROR_TIMEOUT);
        std::terminate();
    }
}

// 비동기 작업 한 건의 수명 단위. 함수의 지역 버퍼는 함수 반환과 함께 사라지므로 사용하지 않는다.
// Windows에는 OVERLAPPED 기반 포인터를 넘기고, 완료 때 원래 OPERATION으로 되돌린다.
// 소유권은 제출 전 unique_ptr → 접수 중 완료 큐가 추적하는 raw 주소 → 워커 unique_ptr 순서다.
// Owner가 세션을, SendBytes가 송신 데이터를 살린다. 수신 메모리는 작업 객체 자체가 소유한다.
// 이 단순 구현은 송신 작업에도 4096-byte ReceiveBytes 공간을 가진다. 풀링/작업별 분리 비용은
// 이후 측정할 최적화 후보이며 이 코드가 할당·메모리 비용 없는 구현이라는 뜻은 아니다.
struct LostArk::Server::CIocpService::OPERATION final : OVERLAPPED
{
    // Internal/Offset/hEvent를 0으로 초기화한다. 완료 패킷을 생략하는 특수 모드는 사용하지 않는다.
    OPERATION() : OVERLAPPED{} {}
    IOCP_OPERATION_KIND Kind = IOCP_OPERATION_KIND::RECEIVE;
    std::shared_ptr<IIocpParticipant> Owner;
    std::array<std::uint8_t, 4096> ReceiveBytes{};
    std::shared_ptr<const std::vector<std::uint8_t>> SendBytes;
    // WSABUF는 메모리를 소유하지 않는다. 위 수신 배열 또는 SendBytes 내부를 가리키는 포인터/길이다.
    WSABUF Buffer{};
};

// 서비스 소유자가 파괴할 때 남은 워커/핸들이 없도록 한다. 세션 먼저 종료하는 순서는 여전히 필요하다.
LostArk::Server::CIocpService::~CIocpService() { Stop(); }

// 헤더의 Start 선언에 대응한다. 서비스 소유자가 한 번 호출하며 세션 접수 전에 끝낸다.
// 포트/워커 생성에 실패하면 이미 만든 자원을 Stop으로 회수하고 false를 반환한다.
bool LostArk::Server::CIocpService::Start(std::size_t workerCount)
{
    if (m_Port || m_Running.load()) return false;
    // 0은 자동 선택 요청이다. CPU 개수는 시작값일 뿐, 접속 수/패킷 크기에 따른 최적값은 실측한다.
    if (!workerCount) workerCount = (std::max)(1u, std::thread::hardware_concurrency());
    workerCount = (std::clamp)(workerCount, std::size_t{1}, std::size_t{16});
    // INVALID_HANDLE_VALUE로 우선 소켓과 연결되지 않은 공용 완료 포트만 만든다.
    // 마지막 인자는 동시에 실행 가능한 포트 작업 스레드 수이며 워커를 생성하는 호출은 아니다.
    m_Port = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0,
        static_cast<DWORD>(workerCount));
    if (!m_Port) return false;
    m_Running.store(true);
    try
    {
        // 실제 스레드는 여기서 만든다. 생성 직후 각 워커는 Worker_Loop에서 완료를 기다린다.
        m_Workers.reserve(workerCount);
        for (std::size_t i = 0; i < workerCount; ++i)
            m_Workers.emplace_back(&CIocpService::Worker_Loop, this);
    }
    catch (...) { Stop(); return false; }
    return true;
}

// 종료 호출 순서: 외부에서 세션의 새 제출 중단/소켓 닫기 → 이 Stop → 서비스 파괴.
// 이 함수가 모든 세션의 소켓을 대신 닫지는 않는다. 취소 완료도 워커가 회수한 뒤 끝낸다.
// worker 안에서 호출하면 자신을 join할 수 없으므로 잘못된 수명 순서로 처리한다.
void LostArk::Server::CIocpService::Stop(const std::chrono::milliseconds timeout)
{
    if (!m_Port) return;
    if (Is_WorkerThread()) Fail_DrainTimeout();
    // 새 Submit의 진입을 거부한다. 이미 진행 중인 Submit을 기다리는 잠금은 아니므로
    // 소유자가 제출자들을 먼저 정리했다는 수명 계약이 반드시 필요하다.
    m_Running.store(false);
    {
        // I/O 완료 콜백이 반환할 때까지 기다린다. pending은 maintenance 실행 여부는 세지 않는다.
        // 작업 객체의 파괴와 maintenance 종료까지는 뒤의 worker join으로 확정한다.
        // 조건변수 대기는 잠금을 풀고 쉰다. Complete_Pending이 마지막 작업에서 깨운다.
        std::unique_lock lock{m_DrainMutex};
        if (!m_Drained.wait_for(lock, timeout, [this] { return m_Pending.load() == 0; }))
            Fail_DrainTimeout();
    }
    // pending이 0이면 실제 작업 대신 overlapped=null인 종료 신호를 워커 수만큼 넣는다.
    // 각 워커가 하나를 받으면 반복문에서 빠져나오므로 다른 워커도 종료 신호를 받을 수 있다.
    for (std::size_t i = 0; i < m_Workers.size(); ++i)
        if (!::PostQueuedCompletionStatus(m_Port, 0, 0, nullptr)) Fail_DrainTimeout();
    // drain 뒤 join 단계에 별도 제한시간을 둔다. 전체 Stop이 timeout 한 번 안에 끝난다는 뜻은 아니다.
    // 기다림 완료를 먼저 확인한 뒤 std::thread::join으로 C++ 스레드 자원까지 정리한다.
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    for (auto& worker : m_Workers)
    {
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
            deadline - std::chrono::steady_clock::now()).count();
        if (::WaitForSingleObject(worker.native_handle(),
            static_cast<DWORD>((std::max)(std::int64_t{0}, static_cast<std::int64_t>(remaining)))) != WAIT_OBJECT_0)
            Fail_DrainTimeout();
        worker.join();
    }
    m_Workers.clear();
    // 워커가 더 이상 포트에 접근하지 않을 때 핸들을 닫는다. 역순이면 대기 중인 워커와 충돌한다.
    ::CloseHandle(m_Port);
    m_Port = nullptr;
    std::scoped_lock lock{m_ParticipantsMutex};
    m_Participants.clear();
}

// accept로 얻은 소켓을 공용 포트에 묶는다. 이후 그 소켓의 overlapped 완료가 이 포트에 들어온다.
// 연결 식별용 completion key는 0을 쓰며 실제 세션은 각 작업의 Owner로 찾는다.
bool LostArk::Server::CIocpService::Associate(const SOCKET socket)
{
    return m_Running.load() && m_Port &&
        ::CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), m_Port, 0, 0) == m_Port;
}

// 주기 점검 대상 등록. 세션 수명은 소유자/작업이 관리하고 목록은 weak_ptr로만 관찰한다.
// 목록 변경과 워커의 목록 복사를 같은 mutex로 보호한다.
void LostArk::Server::CIocpService::Register(const std::shared_ptr<IIocpParticipant>& participant)
{
    std::scoped_lock lock{m_ParticipantsMutex};
    m_Participants.emplace_back(participant);
}

// 세션이 다음 수신을 준비할 때 호출한다. socket/유효한 owner를 입력받고 접수 결과와 error를 반환한다.
// 4096 bytes는 이번 읽기 버퍼 크기이며 게임 패킷의 최대 길이가 아니다.
// TCP 조각을 완전한 게임 패킷으로 합치는 parser는 완료를 받은 세션이 담당한다.
bool LostArk::Server::CIocpService::Post_Receive(const SOCKET socket,
    const std::shared_ptr<IIocpParticipant>& owner, int& error)
{
    auto operation = std::make_unique<OPERATION>();
    operation->Owner = owner;
    operation->Buffer.buf = reinterpret_cast<char*>(operation->ReceiveBytes.data());
    operation->Buffer.len = static_cast<ULONG>(operation->ReceiveBytes.size());
    return Submit(socket, std::move(operation), error);
}

// 세션의 송신 큐가 선택한 프레임을 보낸다. offset 앞은 이미 완료되어 다시 보내면 안 되는 구간이다.
// const shared_ptr 저장소는 큐에서 프레임을 꺼내더라도 I/O 완료 전 메모리가 없어지지 않게 한다.
// 부분 완료에서 남은 구간을 다시 제출하는 정책은 세션이 담당하고 서비스는 한 번의 전송만 맡는다.
bool LostArk::Server::CIocpService::Post_Send(const SOCKET socket,
    const std::shared_ptr<IIocpParticipant>& owner,
    std::shared_ptr<const std::vector<std::uint8_t>> bytes, const std::size_t offset, int& error)
{
    // bytes가 null, 범위 밖 offset, Windows 길이 타입에 담을 수 없는 요청은 제출 전에 거부한다.
    // owner가 null이 아닌지는 호출자가 보장한다. 아래 검사가 owner까지 검증하는 것은 아니다.
    if (!bytes || offset >= bytes->size() ||
        bytes->size() - offset > (std::numeric_limits<ULONG>::max)())
    { error = WSAEINVAL; return false; }
    auto operation = std::make_unique<OPERATION>();
    operation->Owner = owner;
    operation->Kind = IOCP_OPERATION_KIND::SEND;
    operation->SendBytes = std::move(bytes);
    // Winsock API의 포인터 타입에 맞추기 위한 변환이다. 송신 경로가 원본 bytes 내용을 수정하지 않는다.
    operation->Buffer.buf = reinterpret_cast<char*>(
        const_cast<std::uint8_t*>(operation->SendBytes->data() + offset));
    operation->Buffer.len = static_cast<ULONG>(operation->SendBytes->size() - offset);
    return Submit(socket, std::move(operation), error);
}

// 수신과 송신의 공통 접수 함수. 중요한 순서는 pending 증가 → 소유권 이전 → Windows 호출이다.
// 다른 워커가 매우 빨리 완료를 꺼내더라도 카운터와 작업 수명이 먼저 준비되어 있어야 한다.
bool LostArk::Server::CIocpService::Submit(const SOCKET socket,
    std::unique_ptr<OPERATION> operation, int& error)
{
    error = 0;
    if (!m_Running.load()) { error = WSAESHUTDOWN; return false; }
    // 즉시 실패하면 아래에서 감소시킨다. 정상 접수되면 Worker_Loop가 콜백 이후 감소시킨다.
    // peak 갱신은 CAS 재시도로 다른 워커가 저장한 더 큰 최대값을 덮어쓰지 않게 한다.
    const auto pending = m_Pending.fetch_add(1) + 1;
    auto peak = m_PeakPending.load();
    while (peak < pending && !m_PeakPending.compare_exchange_weak(peak, pending)) {}
    // 이 서비스는 완료 알림 생략 모드를 사용하지 않으므로 즉시 성공도 완료 포트에서 회수한다.
    // Windows 호출 직후 다른 워커가 객체를 파괴할 수 있어 먼저 unique_ptr의 소유권을 놓는다.
    // 접수 성공 후 raw를 다시 읽으면 이미 해제된 객체일 수 있다.
    OPERATION* raw = operation.release();
    DWORD transferred = 0, flags = 0;
    const int result = raw->Kind == IOCP_OPERATION_KIND::RECEIVE ?
        ::WSARecv(socket, &raw->Buffer, 1, &transferred, &flags, raw, nullptr) :
        ::WSASend(socket, &raw->Buffer, 1, &transferred, 0, raw, nullptr);
    if (SOCKET_ERROR == result)
    {
        error = ::WSAGetLastError();
        // WSA_IO_PENDING은 실패가 아니라 정상 비동기 접수다. 이때는 워커가 나중에 정리한다.
        // 그 밖의 오류만 완료 알림이 오지 않는 접수 실패이므로 이 함수가 소유권을 회수한다.
        if (WSA_IO_PENDING != error)
        {
            // 완료 큐로 넘어가지 못했으므로 원래 unique_ptr로 회수하고 pending 감소 전에 파괴한다.
            operation.reset(raw);
            operation.reset();
            Complete_Pending();
            return false;
        }
    }
    error = 0;
    // 작업 완료가 이 증가보다 먼저 관찰될 수도 있다. 실행 중 Posted/Completed의 일시적인
    // 차이만으로 유실을 판정하지 말고 제출 중단과 완료 회수 뒤 최종 값을 비교한다.
    m_Posted.fetch_add(1);
    return true;
}

// 접수 실패 또는 완료 콜백 처리에서 정확히 한 번 호출한다.
// mutex를 잡고 0 전이와 알림을 수행해 Stop의 조건 검사와 실제 대기 사이의 깨움 누락을 막는다.
void LostArk::Server::CIocpService::Complete_Pending() noexcept
{
    std::scoped_lock lock{m_DrainMutex};
    if (m_Pending.fetch_sub(1) == 1) m_Drained.notify_all();
}

// 워커 하나가 실행하는 루프다. 공용 포트에서 완료 한 건을 받아 실제 소유 세션으로 전달한다.
// GQCS의 false만 보고 모두 timeout으로 처리하면 실패한 I/O의 완료 객체를 유실한다.
// overlapped가 있으면 성공 여부와 관계없이 반드시 작업을 회수한다.
void LostArk::Server::CIocpService::Worker_Loop() noexcept
{
    ActiveService = this;
    for (;;)
    {
        DWORD bytes = 0;
        ULONG_PTR key = 0;
        OVERLAPPED* overlapped = nullptr;
        // 최대 100ms 기다린다. 수신이 없어도 깨어나 연결 시간 초과 등의 maintenance를 시도한다.
        const BOOL completed = ::GetQueuedCompletionStatus(m_Port, &bytes, &key, &overlapped, 100);
        const DWORD error = completed ? 0 : ::GetLastError();
        if (overlapped)
        {
            // 제출 때 놓은 소유권을 여기서 단 한 번 회수한다. 블록 종료 때 버퍼/Owner도 해제된다.
            auto operation = std::unique_ptr<OPERATION>(static_cast<OPERATION*>(overlapped));
            // 완료 수는 오류도 포함한다. 따라서 이것을 정상 처리한 게임 패킷 수로 해석하면 안 된다.
            if (operation->Kind == IOCP_OPERATION_KIND::RECEIVE)
            {
                m_ReceiveCompletions.fetch_add(1);
                m_ReceivedBytes.fetch_add(bytes);
            }
            else
            {
                m_SendCompletions.fetch_add(1);
                m_SentBytes.fetch_add(bytes);
                if (!error && bytes && bytes < operation->Buffer.len) m_PartialSends.fetch_add(1);
            }
            // 수신 span은 현재 작업 메모리를 빌린다. 안전하게 버퍼 길이 안으로 제한하며
            // 송신 콜백에는 빈 span과 실제 완료 bytes만 전달한다.
            const auto received = operation->Kind == IOCP_OPERATION_KIND::RECEIVE ?
                std::span<const std::uint8_t>{operation->ReceiveBytes.data(),
                    (std::min)(static_cast<std::size_t>(bytes), operation->ReceiveBytes.size())} :
                std::span<const std::uint8_t>{};
            // 서비스 mutex 없이 호출한다. 세션은 수신 조립 또는 송신 offset 갱신을 수행한다.
            // 콜백에서 다음 수신을 제출할 수 있으므로 그 다음 작업의 pending이 먼저 증가할 수 있다.
            // 콜백이 끝나기 전에 현재 pending을 줄이면 Stop이 너무 일찍 끝날 수 있다.
            operation->Owner->On_IocpCompleted(operation->Kind, received, bytes, error);
            m_Completed.fetch_add(1);
            Complete_Pending();
        }
        // Stop이 넣은 성공/null 패킷이면 종료한다. false/null/WAIT_TIMEOUT은 단순 대기 만료다.
        // false/non-null 오류 완료는 위에서 이미 회수했고, 그 외 포트 오류는 안전하게 종료한다.
        else if (completed && !m_Running.load()) break;
        else if (!completed && error != WAIT_TIMEOUT) Fail_DrainTimeout();
        Run_Maintenance();
    }
    ActiveService = nullptr;
}

// 여러 워커 중 CAS에 성공한 하나가 그 100ms 차례의 점검을 맡는다.
// 목록을 잠근 채 콜백을 부르지 않는다. 약한 참조를 강한 참조 목록으로 바꾼 뒤 잠금을 푼다.
// 이것은 이번 호출 동안만 세션을 보존하며 등록 목록이 세션을 영구 소유하게 만들지 않는다.
// 긴 콜백은 다음 점검과 겹칠 수 있고 완료 콜백과도 병렬일 수 있다. 세션에서 동기화해야 한다.
void LostArk::Server::CIocpService::Run_Maintenance() noexcept
{
    const auto now = ::GetTickCount64();
    auto next = m_NextMaintenance.load();
    if (now < next || !m_NextMaintenance.compare_exchange_strong(next, now + 100)) return;
    try
    {
        std::vector<std::shared_ptr<IIocpParticipant>> participants;
        {
            std::scoped_lock lock{m_ParticipantsMutex};
            std::erase_if(m_Participants, [](const auto& p) { return p.expired(); });
            for (const auto& weak : m_Participants)
                if (auto participant = weak.lock()) participants.push_back(std::move(participant));
        }
        for (const auto& participant : participants) participant->On_IocpMaintenance();
    }
    catch (...) { Fail_DrainTimeout(); }
}

// 아래 조회는 게임 로직을 바꾸지 않는다. Get_WorkerCount/Get_Metrics는 Start/Stop과 동시 호출하지 않는다.
// 원자 카운터 각각의 읽기는 안전하지만 서로 다른 시각의 값이 섞일 수 있는 근사 관측이다.
// 카운터는 Start 때 초기화하지 않으므로 같은 객체를 재시작하면 누적되고 새 실험은 새 객체를 사용한다.
bool LostArk::Server::CIocpService::Is_WorkerThread() const noexcept { return ActiveService == this; }
std::size_t LostArk::Server::CIocpService::Get_WorkerCount() const noexcept { return m_Workers.size(); }
LostArk::Server::IOCP_SERVICE_METRICS LostArk::Server::CIocpService::Get_Metrics() const noexcept
{
    return { m_Workers.size(), m_Posted.load(), m_Completed.load(), m_Pending.load(), m_PeakPending.load(),
        m_ReceiveCompletions.load(), m_SendCompletions.load(), m_PartialSends.load(),
        m_ReceivedBytes.load(), m_SentBytes.load() };
}
