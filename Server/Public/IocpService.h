#pragma once

#include <WinSock2.h>
#include <Windows.h>

//mutex 방지를 위한 header, race condition dead lock 방지를
//위한 header들이다. 이게 있어야 이제 race condition 같은
//버그를 막을 수 있다.
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <thread>
#include <vector>
//name space lostark server
namespace LostArk::Server
{
	//어떤 구조를 사용할 것인지에 대한 선택을 할 수 있도록 한다.
	enum class SESSION_TRANSPORT_BACKEND {SELECT_THREADS, IOCP};
	enum class IOCP_OPERATION_KIND {RECEIVE, SEND};
	//participant가 의미하는 게 뭐지?
	class IIocpParticipant
	{
	public:
		//소멸자 가상함수로 선언. iocp 이거를 그냥 가상함수로 선언해야 한다?
		virtual ~IIocpParticipant() = default;
		//iocp가 완료된다? 무슨 뜻일까?
		virtual void On_IocpCompleted(IOCP_OPERATION_KIND kind,
			std::span<const std::uint8_t> received, std::uint32_t bytes,
			std::uint32_t error) noexcept = 0;
		virtual void On_IocpMaintanance() noexcept = 0;
	};
	//왜 이런 식으로 구조체가 설계되어야 할까?
	//원리가 어떻게 되는 걸까?
	//이거 왜 32가 아니라 64로 선언디
	struct IOCP_SERVICE_METRICS final
	{
		std::size_t iWorkerCount = 0;
		std::uint64_t iPostedOperations = 0;
		std::uint64_t iCompletedOperations = 0;
		std::uint64_t iPendingOperations = 0;
		std::uint64_t iPeakPendingOperations = 0;
		std::uint64_t iReceiveCompletions = 0;
		std::uint64_t iSendCompletions = 0;
		std::uint64_t iPartialSendCompletions = 0;
		std::uint64_t iReceivedBytes = 0;
		std::uint64_t iSentBytes = 0;
	};
	//IocpService 이 객체가 어떤 역할을 가지고 있는 거지?
	class CIocpService final
	{
	public:
		CIocpService() = default;
		~CIocpService();
		CIocpService(const CIocpService&) = delete;
		CIocpService& operator=(const CIocpService&) = delete;

		bool Start(std::size_t workerCount = 0);
		//5초 뒤에 멈춘다 이런 뜻인 건가?
		void Stop(std::chrono::milliseconds timeout =
			std::chrono::milliseconds{ 5000 });
		//socket을 할당한다? 이거의 api가 어디서부터 호출되고 어디서 어떤 흐름으로 돌아서
		//전체적인 서버 구조와 흐름을 만들게 되는 걸까?
		bool Associate(SOCKET socket);
		//iocp participant를 shared_ptr로 선언을 한다.
		//iocp participant를 왜 shared_ptr로 선언을 하는 거지?
		//어떤 흐름과 이유 때문에?
		void Register(const std::shared_ptr<IIocpParticipant>& participant);
		//이 함수의 역할은 뭐지?
		bool Post_Receive(SOCKET socket, const std::shared_ptr<IIocpParticipant>& owner,
			int& error);
		//nodiscard 이거 뜻이 뭐였지? 이거 알았는데 까먹었다.
		[[nodiscard]] bool Is_WorkerThread() const noexcept;
		[[nodiscard]] std::size_t Get_WorkerCount() const noexcept;
		[[nodiscard]] IOCP_SERVICE_METRICS Get_Metrics() const noexcept;
		//변수 핵심이 되는 변수이다. 이 변수들도 왜 private으로 선언을 해야 하는지에 대한
		//모든 근거를 들 수가 있어야 하는 것이다. 모든 파일과 header cpp 파일들 기준으로
		//선언되어있는 모든 것들에 대한 이유와 근거를 들어서 설명을 할 수 있도록 해야 한다.
	private:
		//이거는 전방선언?
		struct OPERATION;
		//여기 안에서 사용할 함수들의 경우 private으로 선언하는 그림인 건가?
		bool Submit(SOCKET socket, std::unique_ptr<OPERATION> operation, int& error);
		//Iocp worker가 loop를 어떻게 도는지에 대한 이해?
		void Worker_Loop() noexcept;
		void Run_Maintenance() noexcept;
		void Complete_Pending() noexcept;

		HANDLE m_Port = nullptr;
		//iocp worker를 보관하기 위한 vector
		std::vector<std::thread> m_Workers;
		std::atomic_bool m_Running{ false };
		//왜 이렇게 atomic을 이렇게 구성한 거지?
		//atomic의 의미가 나눌 수 없는, mutex가 해당 객체에 대한 쓰기 작업을 할 때,
		//다른 worker가 해당 정보를 쓰지 못하도록 하는 것이 핵심인 거잖아
		//그래서 멀티 쓰레드에서 해당 개념이 등장을 해서 접근해서 서로 다른 값으로 사용하지 못하게,
		//뮤텍스, 세마포어 이런 기법들을 사용해서 race condition을 막는 거고.
		std::atomic<std::uint64_t> m_Pending{ 0 }, m_PeakPending{ 0 },
			m_Posted{ 0 }, m_Completed{ 0 };
		//이거는 왜 또 여기에서 분리를 해서 작업을 하는 거지?
		std::atomic<std::uint64_t> m_ReceivedBytes{ 0 }, m_SentBytes{ 0 };
		std::mutex m_DrainMutex;
		std::condition_variable m_Drained;
		std::mutex m_ParticipantsMutex;
		std::vector<std::weak_ptr<IIocpParticipant>> m_Participants;
		std::atomic<ULONGLONG> m_NextMaintenance{ 0 };
	};
}