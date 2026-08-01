#pragma once

#include "TcpListener.h"
#include "WinSockContext.h"

//server app이 winsock context 객체를 소유해서,
//serverapp 이 한 파일이 하는 역할이 뭘까?
//level lobby는 캐릭터 4개 존재하고, UI 클릭을 통해서 이름을 입력 받고,
//확인 누르면 해당 string 이름이 캐릭터 아래 UI 창에 반영되도록 character info struct를
//vector로 들고 있잖아 그거를 바탕으로 입장 버튼을 클릭하면, 실제 client 베른성에 입장을 하면서
//server와 연동이 되어서, nickname과 class에 대한 정보를 바탕으로 client에 띄워줘야 하는 거지. 
//그렇다면 serverapp의 역할은 winsockcontext 객체를 통해서 client와 server 같의 
//shared의 packet을 통한 통신이 가능하게 해야 하는 것일까?

//Client와 직접 게임 패킷을 주고 받는 객체라기보다 서버 전체의 시작 실행 종료를 조율하는 
//최상위 객체이다

namespace LostArk::Server
{
	class CServerApp final
	{
	public:
		int Run();

	private:
		//멤버 순서 중요! 생성 : WinSockContext -> TcpListener
		//소멸 : TcpListener -> WinSockContext 소멸은 역순
		CWinSockContext m_WinSockContext;
		CTcpListener m_TcpListener;
	};
}
