#pragma once

// listening socket handle과 Winsock 연결 API의 선언이다.
#include <WinSock2.h>
// Open 구현이 IPv4 주소 문자열을 바이너리 주소로 변환할 때 쓰는 선언이다.
#include <WS2tcpip.h>
// TCP 포트의 16비트 표현을 고정한다.
#include <cstdint>
// Open 호출 동안 bind 주소를 차용하며 구현에서 널 종료 문자열로 복사한다.
#include <string_view>
//server 플레이어 동기화 
// AcceptThread의 handle 읽기와 ServerApp 종료 경로의 Close를 교차시킨다.
#include <atomic>

//Client가 서버에 접속할 수 있도록 특정 주소와 포트에서 "입구를 열고 기다리는 소켓"을 
//소유하는 클래스이다. 
//Shared에서 만든 6바이트짜리 헤더는 TCP 헤더가 아니라, 우리 게임 전용으로 client<->server 간의 
//통신을 하는데 사용하는 전용 앱 헤더이다.

//TCP : 메시지가 아니라 연속적인 바이트 스트림을 제공한다. server의 recv가 반드시 두 번에 나눠서
//15바이트를 쪼개서 보내는 것이 아니기 때문에,packetstreamparser를 사용해서, tcp가 30바이트씩
//순서를 지킨 상태로 보낸 패킷을 header를 파싱해서 정보를 읽어야 하는 것이다
//TCP가 보장하는 것은 보낸 바이트의 순서와 내용, 보장하지 않는 것은 우리 게임 페킷 하나의 시작과 끝이다.

//packet이 게임 패킷 경계를 찾기 위해서 totalSize : uint32 : 4byte, packetType : uint16 2byte
//그 뒤에 payload가 붙는다. C2S_ENTER_WORLD payload - classId : uint8, nicknameSize - uint16
//nickname : UTF-8 byte
//[EtherNet Header]
//[IP Header]
//[TCP Header] - windows가 생성
//[게임 프레임 헤더]
//[게임 Payload]

//ListeningSocket - client A B C 접속 -> connected socket A B C 연결
//TCP 역할은 3가지, socket->bind->listen

//역할 3가지 socket -> bind -> listen
//socket -> AF_INET SOCK_STREAM IPPROTO_TCP - windows에 TCP 소켓 핸들을 요청한다.
//bind - 소켓에 서버 주소와 포트를 연결한다.
//이 프로그램이 현재 컴퓨터의 7777번 포트를 사용하겠다고 운영체제에 등록하는 단계
//listen - 해당 소켓을 client 연결을 기다리는 listenening socket으로 변경한다.
//실제 접속은 accept()에서 수행

//CServerApp - server 전체의 수명 관리 -> TCPListener 0.0.0.0:7777 팀 LAN 입구 개방 -> accept()->
//ClientSession Client 한 명의 연결 -> CPacketStreamParser TCP 바이트에서 프레임 분리 ->
//Shared Read Message 구조체 복원 -> GameRoom 게임 상태 처리

namespace LostArk::Server
{
	class CTcpListener final
	{
	public:
		//CTcpListener도 하나만 존재해야 한다! 복제 방지하기!
		CTcpListener() = default;
		~CTcpListener();
		//복사 생성 금지
		CTcpListener(const CTcpListener&) = delete;
		//대입 연산 금지
		CTcpListener& operator=(const CTcpListener&) = delete;

	public:
		//listen socket open
		// ServerApp::Run이 호출한다. socket → 주소 해석 → bind → listen까지 성공한 뒤에만 handle을 소유한다.
		// 중간 실패는 임시 socket을 닫고 Get_LastErrorCode로 원인을 남긴다. 이미 열렸으면 주소를 바꾸지 않고 true다.
		bool Open(
			std::string_view bindAddress,
			std::uint16_t port);

		// 주소를 생략하는 호출자는 모든 로컬 IPv4 인터페이스에서 지정 포트를 받는다.
		bool Open(std::uint16_t port)
		{
			return Open("0.0.0.0", port);
		}
		
		//실제 client 받아오기
		// AcceptThread가 대기하며 성공한 연결 socket을 반환한다. 반환 뒤 소유권은 호출자가 ClientSession으로 넘긴다.
		// 실패 시 INVALID_SOCKET이며 listener 자체의 소유권은 유지한다.
		[[nodiscard]]
		SOCKET Accept();

		//socket close
		// 종료 경로가 handle을 INVALID_SOCKET으로 먼저 교환한다. 중복 Close 중 한 호출만 실제 handle을 닫는다.
		void Close();
		//NoDiscard반환값이 반드시 있어야 한다.
		// [[nodiscard]]는 반환값 존재를 강제하는 문법이 아니라 호출자가 결과를 버리면 경고해 달라는 표시다.

		[[nodiscard]]
		bool Is_Open() const
		{
			//atomic은 load()를 사용해야 함?
			// load는 다른 스레드가 Close에서 교환한 handle 값을 안전하게 읽는다. 이후 소켓이 계속 열려 있음을 보장하지는 않는다.
			return INVALID_SOCKET != m_hListenSocket.load();
		}

		[[nodiscard]]
		// 가장 최근 기록된 Winsock 오류다. 여러 동작이 동시에 일어나면 특정 호출만의 오류 저장소는 아니다.
		int Get_LastErrorCode() const
		{
			return m_iLastErrorCode.load();
		}

	private:
		//SOCKET m_hListenSocket = INVALID_SOCKET;
		//int m_iLastErrorCode = 0;
		//기존 소켓 멤버를 교체한다
		// handle 접근과 오류 값의 데이터 경쟁을 막는다. socket API 자체의 수명 판단은 각 호출 결과로 다시 확인한다.
		std::atomic<SOCKET> m_hListenSocket{ INVALID_SOCKET };
		std::atomic<int> m_iLastErrorCode{ 0 };
	};
}
