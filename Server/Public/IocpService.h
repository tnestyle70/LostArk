#pragma once

// 연결할 소켓과 수신 제출 인자의 SOCKET 형식을 선언한다.
#include <WinSock2.h>
// completion port HANDLE과 유지보수 시각 저장용 ULONGLONG 형식을 선언한다.
#include <Windows.h>

//mutex 방지를 위한 header, race condition dead lock 방지를
//위한 header들이다. 이게 있어야 이제 race condition 같은
//버그를 막을 수 있다.
// include는 동기화 타입을 사용할 수 있게 할 뿐 race/deadlock을 자동으로 막지 않는다.
// 실제 공유 상태의 접근·잠금 순서는 구현 코드에서 지켜야 하며 이 서비스의 CPP는 현재 비어 있다.
// 실행 플래그·작업 수·바이트 통계의 원자 값 멤버를 선언한다.
#include <atomic>
// Stop의 대기 제한 인자를 milliseconds 단위로 표현한다.
#include <chrono>
// 남은 작업 소진 대기를 알릴 condition_variable 멤버의 정의가 필요하다.
#include <condition_variable>
// worker 수처럼 컨테이너 크기에 대응하는 std::size_t를 사용한다.
#include <cstddef>
// 완료 콜백의 byte/error 값과 서비스의 64비트 계수 형식을 사용한다.
#include <cstdint>
// 참여자 shared/weak 참조와 제출 작업 unique_ptr 인자를 선언한다.
#include <memory>
// 작업 소진 대기와 참여자 목록을 위한 별도 mutex 멤버를 선언한다.
#include <mutex>
// 수신 완료 콜백에 복사 없이 바이트 범위를 전달하는 타입을 사용한다.
#include <span>
// 서비스가 관리할 worker thread 객체의 vector를 선언한다.
#include <thread>
// worker 소유 목록과 참여자 weak 참조 목록을 보관한다.
#include <vector>
// 서버 내부에서만 쓰는 전송 서비스 선언이다.
namespace LostArk::Server
{
	// 현재 IocpService.cpp는 비어 있고 CIocpService/IIocpParticipant의 런타임 소비자도 없다.
	// 아래는 준비 중인 선언이며 작업 제출·완료 처리·취소·join 동작이 구현됐다는 뜻이 아니다.
	// 실제 ClientSession은 기존 select/수신·송신 thread 경로를 사용하고 이 backend enum을 선택하지 않는다.

	//어떤 구조를 사용할 것인지에 대한 선택을 할 수 있도록 한다.
	// backend를 나타내는 두 값만 선언한 상태다. 실행 옵션이나 분기 코드에는 아직 연결되어 있지 않다.
	enum class SESSION_TRANSPORT_BACKEND
	{
		SELECT_THREADS,
		IOCP
	};

	// 완료 콜백이 구분할 작업 종류다. SEND 값은 있지만 이 헤더에 Post_Send 진입점은 선언되어 있지 않다.
	enum class IOCP_OPERATION_KIND
	{
		RECEIVE,
		SEND
	};
	//participant가 의미하는 게 뭐지?
	// 완료 결과와 유지보수 호출을 받을 객체의 추상 경계다. 현재는 상속한 구현체가 없어 호출 흐름은 미연결이다.
	class IIocpParticipant
	{
	public:
		//소멸자 가상함수로 선언. iocp 이거를 그냥 가상함수로 선언해야 한다?
		// 기반 포인터로 파생 객체를 파괴할 때 파생 소멸자까지 호출하도록 하는 virtual 소멸자다.
		virtual ~IIocpParticipant() = default;
		//iocp가 완료된다? 무슨 뜻일까?
		// 완료된 작업 종류·수신 바이트 범위·완료 길이·오류 코드를 받도록 선언한 콜백이다.
		// received는 비소유 span이므로 구현 시 원본 buffer 수명과 호출 스레드 정책을 정해야 한다.
		// noexcept는 구현체가 예외를 밖으로 전달하지 않는 계약이며 이 선언 자체가 오류를 처리하지는 않는다.
		virtual void On_IocpCompleted(IOCP_OPERATION_KIND kind,
			std::span<const std::uint8_t> received, std::uint32_t bytes,
			std::uint32_t error) noexcept = 0;
		// 주기적 유지보수용 콜백 선언이다. 이름의 기존 철자는 API 변경 없이 유지하며 실제 호출 주기는 미구현이다.
		virtual void On_IocpMaintanance() noexcept = 0;
	};
	//왜 이런 식으로 구조체가 설계되어야 할까?
	//원리가 어떻게 되는 걸까?
	//이거 왜 32가 아니라 64로 선언디
	// worker 수는 컨테이너 크기형 size_t, 작업·바이트 수는 더 큰 누적값을 표현할 수 있는 uint64_t다.
	// 이 struct는 조회 결과의 값 묶음이며 자체 동기화가 없다. 집계 시점·reset 정책은 CPP 구현에 달려 있다.
	struct IOCP_SERVICE_METRICS final
	{
		// worker 수와 작업 제출/완료/잔량을 같은 조회 결과로 전달하도록 선언한다.
		std::size_t iWorkerCount = 0;
		std::uint64_t iPostedOperations = 0;
		std::uint64_t iCompletedOperations = 0;
		std::uint64_t iPendingOperations = 0;
		std::uint64_t iPeakPendingOperations = 0;
		// 완료 종류와 부분 송신을 구별할 통계 필드다. 대응 증가 코드가 현재 존재하지는 않는다.
		std::uint64_t iReceiveCompletions = 0;
		std::uint64_t iSendCompletions = 0;
		std::uint64_t iPartialSendCompletions = 0;
		// 작업 횟수와 전송량은 다른 단위이므로 완료 계수와 별도로 바이트 합계를 표현한다.
		std::uint64_t iReceivedBytes = 0;
		std::uint64_t iSentBytes = 0;
	};
	//IocpService 이 객체가 어떤 역할을 가지고 있는 거지?
	// completion port와 worker 목록을 소유할 서비스 선언이다. Start/Stop을 비롯한 본문이 없어 현재 생성해 사용할 수 없다.
	class CIocpService final
	{
	public:
		// 기본 멤버 값만 초기화한다. completion port나 worker를 시작하는 생성자는 아니다.
		CIocpService() = default;
		// 서비스 정리 진입점으로 선언됐지만 본문은 아직 없다. 소멸 시 Stop을 호출한다고 가정할 수 없다.
		~CIocpService();
		// HANDLE과 thread 소유 목록이 값복사되어 이중 관리되지 않게 복사 생성·대입을 금지한다.
		CIocpService(const CIocpService&) = delete;
		CIocpService& operator=(const CIocpService&) = delete;

		// worker 수를 받아 시작 결과를 bool로 돌려주도록 선언한다. 0의 자동 선택 규칙과 실패 rollback은 미구현이다.
		bool Start(std::size_t workerCount = 0);
		//5초 뒤에 멈춘다 이런 뜻인 건가?
		// 기본 인자가 5000밀리초인 대기 제한 형식이다. 5초 후 자동 종료 예약을 선언한 것은 아니다.
		// 실제 취소·drain·join과 timeout 처리 방식은 비어 있는 CPP에서 아직 확인할 수 없다.
		void Stop(std::chrono::milliseconds timeout =
			std::chrono::milliseconds{ 5000 });
		//socket을 할당한다? 이거의 api가 어디서부터 호출되고 어디서 어떤 흐름으로 돌아서
		//전체적인 서버 구조와 흐름을 만들게 되는 걸까?
		// 이미 만든 socket을 completion port와 연결하기 위한 선언이다. 실제 호출자는 아직 없다.
		// socket의 소유권 이전·실패 시 닫기 여부도 본문이 없으므로 구현된 계약으로 단정하지 않는다.
		bool Associate(SOCKET socket);
		//iocp participant를 shared_ptr로 선언을 한다.
		//iocp participant를 왜 shared_ptr로 선언을 하는 거지?
		//어떤 흐름과 이유 때문에?
		// 인자는 호출 동안 유효한 참여자 공유 참조를 받는다. 저장 목록은 아래 weak_ptr 형식이므로
		// 목록 자체가 참여자를 영구 소유하지 않게 설계한 선언이다. 실제 등록/제거 코드는 아직 없다.
		void Register(const std::shared_ptr<IIocpParticipant>& participant);
		//이 함수의 역할은 뭐지?
		// socket의 비동기 수신 제출 결과와 오류를 반환하려는 선언이다. owner의 작업 완료까지 수명 유지,
		// 수신 buffer 소유와 OS 제출 실패 정리는 OPERATION/Submit 구현이 있어야 확인할 수 있다.
		bool Post_Receive(SOCKET socket, const std::shared_ptr<IIocpParticipant>& owner,
			int& error);
		//nodiscard 이거 뜻이 뭐였지? 이거 알았는데 까먹었다.
		// [[nodiscard]]는 반환값 무시를 진단하도록 권하는 속성이다. 호출 성공이나 thread 안전성을 보장하지 않는다.
		// 현재 호출 스레드가 이 서비스 worker인지 조회하는 선언이다. 판정 구현과 사용 지점은 아직 없다.
		[[nodiscard]] bool Is_WorkerThread() const noexcept;
		// worker 목록의 수를 조회하려는 선언이다. 동시 Start/Stop 중 조회 정책은 본문에서 정해야 한다.
		[[nodiscard]] std::size_t Get_WorkerCount() const noexcept;
		// 통계를 값으로 반환하려는 선언이다. 여러 atomic을 한 시점으로 캡처하는지는 아직 구현되지 않았다.
		[[nodiscard]] IOCP_SERVICE_METRICS Get_Metrics() const noexcept;
		//변수 핵심이 되는 변수이다. 이 변수들도 왜 private으로 선언을 해야 하는지에 대한
		//모든 근거를 들 수가 있어야 하는 것이다. 모든 파일과 header cpp 파일들 기준으로
		//선언되어있는 모든 것들에 대한 이유와 근거를 들어서 설명을 할 수 있도록 해야 한다.
		// 내부 자원과 잠금은 외부가 직접 바꾸지 못하도록 private에 둔다. 실제 동시 접근 규칙은 구현과 함께 검증해야 한다.
	private:
		//이거는 전방선언?
		// 맞다. 제출 작업을 가리키는 unique_ptr 인자만 선언하므로 여기서는 OPERATION의 크기가 필요하지 않다.
		// 해당 타입의 정의도 현재 CPP에는 없다.
		struct OPERATION;
		//여기 안에서 사용할 함수들의 경우 private으로 선언하는 그림인 건가?
		// Submit은 공개 Post_Receive와 구별한 내부 제출 진입점이다. operation 인자는 독점 소유권을
		// 받는 형식이지만 OS 제출 뒤 소유권 인계·실패 정리의 실제 시점은 미구현이다.
		bool Submit(SOCKET socket, std::unique_ptr<OPERATION> operation, int& error);
		//Iocp worker가 loop를 어떻게 도는지에 대한 이해?
		// 완료 큐를 처리할 worker 본체의 선언이다. 실제 wait/wakeup/종료 조건은 아직 없다.
		void Worker_Loop() noexcept;
		// 참여자 유지보수 호출을 모을 내부 진입점 선언이다. 목록 잠금과 콜백 실행 순서는 미구현이다.
		void Run_Maintenance() noexcept;
		// 완료 후 pending 계수와 drain 알림을 처리할 내부 진입점 선언이다. 감소·통지 코드는 아직 없다.
		void Complete_Pending() noexcept;

		// 서비스가 관리할 completion port handle 자리다. 실제 생성·닫기 경로는 CPP가 구현되어야 한다.
		HANDLE m_Port = nullptr;
		//iocp worker를 보관하기 위한 vector
		// thread 객체를 값으로 소유하므로 구현 시 join 가능한 객체를 파괴하기 전에 합류 또는 분리해야 한다.
		std::vector<std::thread> m_Workers;
		// 실행 상태를 여러 worker와 공유할 단일 원자값이다. thread 생성·종료 순서까지 대신 동기화하지는 않는다.
		std::atomic_bool m_Running{ false };
		//왜 이렇게 atomic을 이렇게 구성한 거지?
		//atomic의 의미가 나눌 수 없는, mutex가 해당 객체에 대한 쓰기 작업을 할 때,
		//다른 worker가 해당 정보를 쓰지 못하도록 하는 것이 핵심인 거잖아
		//그래서 멀티 쓰레드에서 해당 개념이 등장을 해서 접근해서 서로 다른 값으로 사용하지 못하게,
		//뮤텍스, 세마포어 이런 기법들을 사용해서 race condition을 막는 거고.
		// atomic은 해당 값의 원자적 접근을 제공하며 mutex와 같은 객체 잠금은 아니다.
		// 여러 계수를 묶은 불변식·자원 수명은 별도 접근 순서와 동기화가 필요하다.
		std::atomic<std::uint64_t> m_Pending{ 0 }, m_PeakPending{ 0 },
			m_Posted{ 0 }, m_Completed{ 0 };
		//이거는 왜 또 여기에서 분리를 해서 작업을 하는 거지?
		// 작업 개수와 byte 수가 서로 다른 통계이므로 저장 필드를 구분한다.
		std::atomic<std::uint64_t> m_ReceivedBytes{ 0 }, m_SentBytes{ 0 };
		// 남은 작업이 없다는 조건을 기다릴 mutex/condition_variable 쌍이다. predicate와 알림 구현은 아직 없다.
		std::mutex m_DrainMutex;
		std::condition_variable m_Drained;
		// 참여자 목록을 보호할 별도 잠금이다. weak_ptr의 만료 처리는 실제 유지보수 구현이 담당해야 한다.
		std::mutex m_ParticipantsMutex;
		std::vector<std::weak_ptr<IIocpParticipant>> m_Participants;
		// 다음 유지보수 시점을 공유할 원자값 자리다. 시계·단위·갱신 주기는 현재 코드에서 정해지지 않았다.
		std::atomic<ULONGLONG> m_NextMaintenance{ 0 };
	};
}