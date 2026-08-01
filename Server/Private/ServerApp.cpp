#include "ServerApp.h"
#include "ClientSession.h"

#include "Network/PacketMessages.h"
#include "Network/PacketWriter.h"
#include "Network/PacketReader.h"
#include "Network/PacketType.h"

#include <cstdint>
#include <iostream>

int LostArk::Server::CServerApp::Run()
{
	using namespace LostArk::Shared;

	static_assert(Is_Known_Packet_Type(
		PACKET_TYPE::C2S_ENTER_WORLD));
	if (!m_WinSockContext.Initialize())
	{
		std::cerr << "Failed to initialize winsock 2.2.\n";
		return 1;
	}

	constexpr std::uint16_t SERVER_PORT = 7777;

	if (!m_TcpListener.Open(SERVER_PORT))
	{
		std::cerr
			<< "Failed to open TCP listener. Error: "
			<< m_TcpListener.Get_LastErrorCode()
			<< '\n';
		return 1;
	}

	std::cout
		<< "Listening on 127.0.0.1:"
		<< SERVER_PORT
		<< '\n'
		<< "Waiting for one client...\n";

	SOCKET clientSocket = m_TcpListener.Accept();
	if (INVALID_SOCKET == clientSocket)
	{
		std::cerr
			<< "Failed to accept client. Error: "
			<< m_TcpListener.Get_LastErrorCode()
			<< '\n';
		return 1;
	}

	std::cout << "Client connected.\n";

	CClientSession clientSession{ clientSocket };
	PACKET_FRAME frame{};

	const SESSION_RECEIVE_RESULT receiveResult =
		clientSession.Receive_Frame(frame);

	if (SESSION_RECEIVE_RESULT::FRAME_READY != receiveResult)
	{
		std::cerr
			<< "Failed to receive frame. Result: "
			<< static_cast<int>(receiveResult)
			<< ", Socket Error: "
			<< clientSession.Get_LastErrorCode()
			<< '\n';
		return 1;
	}

	if (PACKET_TYPE::C2S_ENTER_WORLD != frame.ePacketType)
	{
		std::cerr << "Expected C2S_ENTER_WORLD.\n";
		return 1;
	}

	CPacketReader reader{ frame.Payload };

	C2S_ENTER_WORLD enterWorld{};

	if (!Read_Message(reader, enterWorld))
	{
		std::cerr << "Invalid C2S_ENTER_WORLD payload.\n";
		return 1;
	}

	if (0 != reader.Get_RemainingSize())
	{
		std::cerr << "Unexpected trailing payload bytes.\n";
		return 1;
	}

	std::cout
		<< "C2S_ENTER_WORLD received.\n"
		<< "Character Class ID: "
		<< static_cast<unsigned int>(enterWorld.eCharacterClass)
		<< '\n'
		<< "Nickname: "
		<< enterWorld.strNickName
		<< '\n'
		<< "Press Enter to stop the server.\n";

	S2C_ENTER_ACCEPTED accepted{};

	accepted.iPlayerId = 1;
	accepted.iNetEntityId = 100;

	CPacketWriter acceptedWriter;

	if (!Write_Message(
		acceptedWriter,
		accepted))
	{
		std::cerr
			<< "Failed to encode "
			<< "S2C_ENTER_ACCEPTED.\n";

		return 1;
	}

	if (!clientSession.Send_Frame(
		PACKET_TYPE::S2C_ENTER_ACCEPTED,
		acceptedWriter.Get_Buffer()))
	{
		std::cerr
			<< "Failed to send "
			<< "S2C_ENTER_ACCEPTED. Error: "
			<< clientSession.Get_LastErrorCode()
			<< '\n';

		return 1;
	}

	std::cout
		<< "S2C_ENTER_ACCEPTED sent.\n"
		<< "Player ID: "
		<< accepted.iPlayerId
		<< '\n'
		<< "Net Entity ID: "
		<< accepted.iNetEntityId
		<< '\n';

	std::cin.get();
	return 0;
}
