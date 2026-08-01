#pragma once

//winsock 객체가 하는 역할이 정확하게 뭘까?
//초기화해서 객체 생성하고, winsock이 제공하는 packet server client 통신 기능을 사용할 수 있게 한다.
//정확히 어떤 기능이지? serverapp에서 initialize를 통해서 객체를 생성하고, 
//해당 객체를 serverapp에서 사용을 한다.

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
		bool Initialize();
		void Shutdown();
		//반환값을 무시하면 컴파일러가 경고하도록 요청한다.
		//해당 검사는 보안과 파싱 흐름에서 중요하므로 실수로 결과를 버리지 말라는 의미이다.
		[[nodiscard]]
		bool Is_Initialized() const
		{
			return m_isInitialized;
		}

	private:
		bool m_isInitialized = false;
	};
}