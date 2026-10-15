#pragma once

//winsock 객체가 하는 역할이 정확하게 뭘까?
//초기화해서 객체 생성하고, winsock이 제공하는 packet server client 통신 기능을 사용할 수 있게 한다.
//정확히 어떤 기능이지? serverapp에서 initialize를 통해서 객체를 생성하고, 
//해당 객체를 serverapp에서 사용을 한다.
// 이 객체는 소켓이나 연결을 만들지 않는다. WSAStartup 성공 1회에 대응하는 WSACleanup 1회를 소유한다.
// ServerApp은 listener를 열기 전에 Initialize를 호출하고 모든 session을 정리한 뒤 Shutdown한다.

namespace LostArk::Server
{
	class CWinSockContext final
	{
	public:
		CWinSockContext() = default;
		~CWinSockContext();
		//winsock 초기화 소유권을 복사하지 못하게 하기 위해서이다.
		CWinSockContext(
			const CWinSockContext&) = delete;

		CWinSockContext& operator=(const CWinSockContext&) = delete;

	public:
		// Run이 Winsock 2.2를 요청한다. 재호출은 이미 초기화됐으면 성공하며, 버전 불일치는 즉시 정리하고 false다.
		bool Initialize();

		// ServerApp의 종료 또는 이 객체의 소멸자가 호출한다. 자신이 성공시킨 초기화만 해제한다.
		void Shutdown();
		//반환값을 무시하면 컴파일러가 경고하도록 요청한다.
		//해당 검사는 보안과 파싱 흐름에서 중요하므로 실수로 결과를 버리지 말라는 의미이다.
		[[nodiscard]]
		bool Is_Initialized() const
		{
			return m_isInitialized;
		}

	private:
		// Run/Shutdown 수명 관리 경로가 사용하는 플래그다. session worker가 직접 읽거나 바꾸지 않는다.
		bool m_isInitialized = false;
	};
}