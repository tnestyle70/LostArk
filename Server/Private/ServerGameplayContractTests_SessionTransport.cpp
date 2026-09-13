#include "ServerGameplayContractTests_Runner.h"
#include "ServerGameplayContractTests.h"
#include "ClientSession.h"
#include "GameRoom.h"
#include "Network/PacketWriter.h"
#include "ServerApp.h"
#include "WinSockContext.h"
#include "WorldDestructionBootstrapContractTests.h"
#include <Windows.h>
#include <process.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <span>
#include <sstream>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>


using namespace LostArk::Server;
using namespace LostArk::Shared;

void LostArk::Server::CServerGameplayContractRunner::Run_SessionTransport(TESTS& tests)
{

	{
		CClientSession session{ 90000u, INVALID_SOCKET, {}, {} };
		session.Record_InboundPacket(PACKET_TYPE::C2S_MOVE);
		session.Request_Close(
			SESSION_DIAGNOSTIC_REASON::SERVER_RECEIVE_ERROR,
			WSAECONNRESET,
			"first terminal cause");
		session.Request_Close(
			SESSION_DIAGNOSTIC_REASON::SERVER_APPLICATION_CLOSE,
			WSAEINVAL,
			"later cleanup must not overwrite");
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC diagnostic =
			session.Get_CloseDiagnostic();
		tests.Require(
			SESSION_DIAGNOSTIC_REASON::SERVER_RECEIVE_ERROR ==
				diagnostic.eReason &&
			PACKET_TYPE::C2S_MOVE == diagnostic.eLastInboundPacket &&
			WSAECONNRESET == diagnostic.iNativeErrorCode &&
			"first terminal cause" == diagnostic.strContext &&
			0u != diagnostic.iLastInboundUnixMilliseconds &&
			diagnostic.iLastInboundUnixMilliseconds ==
				session.Get_LastInboundUnixMilliseconds() &&
			0u != diagnostic.iOccurredUnixMilliseconds,
			"Preserve the first typed session terminal cause across later cleanup");
	}
	{
		auto app = std::make_unique<CServerApp>();
		auto protocolSession = std::make_shared<CClientSession>(
			89995u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		auto malformedSession = std::make_shared<CClientSession>(
			89996u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		auto unknownSession = std::make_shared<CClientSession>(
			89997u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		app->m_Sessions.emplace(89995u, protocolSession);
		app->m_Sessions.emplace(89996u, malformedSession);
		app->m_Sessions.emplace(89997u, unknownSession);

		PACKET_FRAME protocolFrame{};
		protocolFrame.ePacketType = PACKET_TYPE::C2S_ENTER_WORLD;
		protocolFrame.Payload = { 0xffu, 0xffu, 1u, 0u };
		protocolSession->Record_InboundPacket(protocolFrame.ePacketType);
		app->On_SessionFrame(89995u, protocolFrame);

		PACKET_FRAME malformedFrame{};
		malformedFrame.ePacketType = PACKET_TYPE::C2S_MOVE;
		malformedSession->Record_InboundPacket(malformedFrame.ePacketType);
		app->On_SessionFrame(89996u, malformedFrame);

		PACKET_FRAME unknownFrame{};
		unknownFrame.ePacketType = static_cast<PACKET_TYPE>(0xffffu);
		unknownSession->Record_InboundPacket(unknownFrame.ePacketType);
		app->On_SessionFrame(89997u, unknownFrame);

		const CLIENT_SESSION_CLOSE_DIAGNOSTIC protocolDiagnostic =
			protocolSession->Get_CloseDiagnostic();
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC malformedDiagnostic =
			malformedSession->Get_CloseDiagnostic();
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC unknownDiagnostic =
			unknownSession->Get_CloseDiagnostic();
		tests.Require(
			SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_MESSAGE_DECODE_FAILED ==
				protocolDiagnostic.eReason &&
			WSAEPROTONOSUPPORT == protocolDiagnostic.iNativeErrorCode &&
			std::string::npos !=
				protocolDiagnostic.strContext.find("protocol mismatch") &&
			SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_MESSAGE_DECODE_FAILED ==
				malformedDiagnostic.eReason &&
			WSAEINVAL == malformedDiagnostic.iNativeErrorCode &&
			std::string::npos != malformedDiagnostic.strContext.find("C2S_MOVE") &&
			SESSION_DIAGNOSTIC_REASON::SERVER_UNKNOWN_PACKET ==
				unknownDiagnostic.eReason &&
			WSAEPROTONOSUPPORT == unknownDiagnostic.iNativeErrorCode,
			"Classify protocol mismatch, malformed message, and unknown packet separately");
	}
	{
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::TRAINING_GROUND);
		CGameRoom& room = *roomStorage;
		auto missingSkillBindingSession = std::make_shared<CClientSession>(
			89979u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		auto missingBindingSession = std::make_shared<CClientSession>(
			89980u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		auto staleSession = std::make_shared<CClientSession>(
			89981u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		auto nonFiniteSession = std::make_shared<CClientSession>(
			89982u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		auto outOfRangeSession = std::make_shared<CClientSession>(
			89983u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		room.m_Sessions.emplace(89979u, missingSkillBindingSession);
		room.m_Sessions.emplace(89980u, missingBindingSession);
		room.m_Sessions.emplace(89981u, staleSession);
		room.m_Sessions.emplace(89982u, nonFiniteSession);
		room.m_Sessions.emplace(89983u, outOfRangeSession);
		const auto addPlayer = [&room](
			const SESSION_ID sessionId,
			const PLAYER_ID playerId,
			const std::uint32_t lastMoveSequence)
			{
				SERVER_PLAYER player{};
				player.iSessionId = sessionId;
				player.iPlayerId = playerId;
				player.iNetEntityId = playerId;
				player.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
				player.iLastMoveSequence = lastMoveSequence;
				player.iCurrentHp = 100u;
				player.iMaximumHp = 100u;
				room.m_Players.emplace(playerId, player);
				room.m_PlayerIdBySessionId.emplace(sessionId, playerId);
				room.m_PlayerIdByEntityId.emplace(playerId, playerId);
			};
		addPlayer(89981u, 89981u, 10u);
		addPlayer(89982u, 89982u, 0u);
		addPlayer(89983u, 89983u, 0u);

		C2S_USE_SKILL missingSkillBinding{};
		missingSkillBinding.iClientSequence = 1u;
		missingSkillBinding.iSkillId = 34010u;
		missingSkillBindingSession->Record_InboundPacket(
			PACKET_TYPE::C2S_USE_SKILL);
		room.Handle_UseSkill(89979u, missingSkillBinding);
		C2S_MOVE missingBindingMove{};
		missingBindingMove.iClientSequence = 1u;
		missingBindingSession->Record_InboundPacket(PACKET_TYPE::C2S_MOVE);
		room.Handle_Move(89980u, missingBindingMove);
		C2S_MOVE staleMove{};
		staleMove.iClientSequence = 10u;
		staleSession->Record_InboundPacket(PACKET_TYPE::C2S_MOVE);
		room.Handle_Move(89981u, staleMove);
		C2S_MOVE nonFiniteMove{};
		nonFiniteMove.iClientSequence = 1u;
		nonFiniteMove.fGoalX =
			(std::numeric_limits<float>::quiet_NaN)();
		nonFiniteSession->Record_InboundPacket(PACKET_TYPE::C2S_MOVE);
		room.Handle_Move(89982u, nonFiniteMove);
		C2S_MOVE outOfRangeMove{};
		outOfRangeMove.iClientSequence = 1u;
		outOfRangeMove.fGoalX = 10001.f;
		outOfRangeSession->Record_InboundPacket(PACKET_TYPE::C2S_MOVE);
		room.Handle_Move(89983u, outOfRangeMove);

		const CLIENT_SESSION_CLOSE_DIAGNOSTIC missingSkillBindingDiagnostic =
			missingSkillBindingSession->Get_CloseDiagnostic();
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC missingBindingDiagnostic =
			missingBindingSession->Get_CloseDiagnostic();
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC staleDiagnostic =
			staleSession->Get_CloseDiagnostic();
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC nonFiniteDiagnostic =
			nonFiniteSession->Get_CloseDiagnostic();
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC outOfRangeDiagnostic =
			outOfRangeSession->Get_CloseDiagnostic();
		tests.Require(
			SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED ==
				missingSkillBindingDiagnostic.eReason &&
			std::string::npos != missingSkillBindingDiagnostic.strContext.find(
				"packet=C2S_USE_SKILL validation=missing-player-binding") &&
			SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED ==
				missingBindingDiagnostic.eReason &&
			WSAENOTCONN == missingBindingDiagnostic.iNativeErrorCode &&
			std::string::npos != missingBindingDiagnostic.strContext.find(
				"validation=missing-player-binding") &&
			SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_COMMAND_VALIDATION_FAILED ==
				staleDiagnostic.eReason &&
			WSAEINVAL == staleDiagnostic.iNativeErrorCode &&
			std::string::npos != staleDiagnostic.strContext.find(
				"validation=stale-sequence") &&
			std::string::npos != staleDiagnostic.strContext.find(
				"receivedSequence=10 lastSequence=10") &&
			SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_COMMAND_VALIDATION_FAILED ==
				nonFiniteDiagnostic.eReason &&
			std::string::npos != nonFiniteDiagnostic.strContext.find(
				"validation=non-finite-goal") &&
			SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_COMMAND_VALIDATION_FAILED ==
				outOfRangeDiagnostic.eReason &&
			std::string::npos != outOfRangeDiagnostic.strContext.find(
				"validation=out-of-range-goal") &&
			PACKET_TYPE::C2S_MOVE == staleDiagnostic.eLastInboundPacket,
			"Classify decoded C2S_MOVE binding and command validation failures exactly");
	}
	{
		CServerApp app;
		auto simulation = std::make_shared<CGameRoom>(
			WORLD_ID::TRAINING_GROUND);
		app.m_SharedGameRooms.emplace(WORLD_ID::TRAINING_GROUND, simulation);
		app.m_pActiveGameplayGeneration =
			simulation->Get_ActiveGameplayGeneration();
		constexpr SESSION_ID cleanupFirstSessionId = 89984u;
		auto cleanupFirstSession = std::make_shared<CClientSession>(
			cleanupFirstSessionId, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		app.m_Sessions.emplace(cleanupFirstSessionId, cleanupFirstSession);
		ROOM_COMMAND cleanupFirst{};
		cleanupFirst.eType = ROOM_COMMAND_TYPE::LEAVE;
		cleanupFirst.iSessionId = cleanupFirstSessionId;
		cleanupFirst.eLeaveReason = PLAYER_DESPAWN_REASON::DISCONNECTED;
		const bool cleanupQueued =
			simulation->Enqueue(std::move(cleanupFirst));
		C2S_ENTER_WORLD enterWorld{};
		enterWorld.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		enterWorld.eWorldId = WORLD_ID::TRAINING_GROUND;
		enterWorld.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		enterWorld.strNickName = "CleanupFirstFixture";
		SESSION_DIAGNOSTIC_REASON cleanupFirstReason =
			SESSION_DIAGNOSTIC_REASON::NONE;
		int cleanupFirstNativeError = 0;
		std::string cleanupFirstContext;
		const bool entryRejectedBehindCleanup =
			!app.Bind_AndEnqueueEntry(
				cleanupFirstSessionId,
				WORLD_ID::TRAINING_GROUND,
				simulation,
				enterWorld,
				cleanupFirstReason,
				cleanupFirstNativeError,
				cleanupFirstContext);
		const bool noEntryCommittedBehindCleanup =
			!app.m_GameplayBindingBySessionId.contains(cleanupFirstSessionId) &&
			simulation->m_InboundCommands.empty() &&
			1u == simulation->m_CleanupCommands.size();
		simulation->Tick(1.f / 30.f);

		constexpr SESSION_ID alreadyClosingSessionId = 89985u;
		auto alreadyClosingSession = std::make_shared<CClientSession>(
			alreadyClosingSessionId, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		alreadyClosingSession->Request_Close(
			SESSION_DIAGNOSTIC_REASON::SERVER_PEER_CLOSED,
			0,
			"entry race fixture closed first");
		app.m_Sessions.emplace(alreadyClosingSessionId, alreadyClosingSession);
		enterWorld.strNickName = "AlreadyClosingFixture";
		SESSION_DIAGNOSTIC_REASON alreadyClosingReason =
			SESSION_DIAGNOSTIC_REASON::NONE;
		int alreadyClosingNativeError = 0;
		std::string alreadyClosingContext;
		const bool terminalRecheckRejectedEntry =
			!app.Bind_AndEnqueueEntry(
				alreadyClosingSessionId,
				WORLD_ID::TRAINING_GROUND,
				simulation,
				enterWorld,
				alreadyClosingReason,
				alreadyClosingNativeError,
				alreadyClosingContext);
		tests.Require(
			cleanupQueued && entryRejectedBehindCleanup &&
			noEntryCommittedBehindCleanup &&
			SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED ==
				cleanupFirstReason &&
			WSAESHUTDOWN == cleanupFirstNativeError &&
			std::string::npos != cleanupFirstContext.find(
				"stage=register-ingress") &&
			std::string::npos != cleanupFirstContext.find(
				"REJECTED_PENDING_CLEANUP") &&
			simulation->m_CleanupCommands.empty() &&
			simulation->m_QueuedCleanupSessionIds.empty() &&
			terminalRecheckRejectedEntry &&
			SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED ==
				alreadyClosingReason &&
			WSAESHUTDOWN == alreadyClosingNativeError &&
			std::string::npos != alreadyClosingContext.find(
				"stage=terminal-recheck") &&
			!app.m_GameplayBindingBySessionId.contains(
				alreadyClosingSessionId) &&
			SESSION_DIAGNOSTIC_REASON::SERVER_PEER_CLOSED ==
				alreadyClosingSession->Get_CloseDiagnostic().eReason,
			"Reject REGISTER and ENTER behind queued or terminal session cleanup");
	}
	{
		auto failedRoom = std::make_shared<CGameRoom>(
			WORLD_ID::TRAINING_GROUND);
		failedRoom->m_iServerTick = 77u;
		failedRoom->m_strStatus =
			"fixture stage transition transaction failed";
		failedRoom->Mark_RuntimeFailure(
			"world-update.pattern-stage-transition");
		failedRoom->m_strStatus = "later status must not replace first failure";
		failedRoom->Mark_RuntimeFailure("later-runtime-failure");

		ROOM_COMMAND rejectedMove{};
		rejectedMove.eType = ROOM_COMMAND_TYPE::MOVE;
		rejectedMove.iSessionId = 89986u;
		rejectedMove.Move.iClientSequence = 1u;
		const ROOM_COMMAND_ENQUEUE_RESULT rejectedMoveResult =
			failedRoom->Enqueue_Detailed(std::move(rejectedMove));
		const std::string rejectedMoveContext =
			failedRoom->Describe_EnqueueResult(rejectedMoveResult);
		SERVER_ROOM_RUNTIME_FAILURE firstFailure{};
		const bool retainedFirstFailure =
			failedRoom->Try_GetRuntimeFailure(firstFailure) &&
			77u == firstFailure.iServerTick &&
			"world-update.pattern-stage-transition" == firstFailure.strSource &&
			"fixture stage transition transaction failed" ==
				firstFailure.strDetail;

		CServerApp assignedApp;
		auto assignedSession = std::make_shared<CClientSession>(
			89986u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		assignedApp.m_Sessions.emplace(89986u, assignedSession);
		CServerApp::SESSION_GAMEPLAY_BINDING assignedBinding{};
		assignedBinding.eWorldId = WORLD_ID::TRAINING_GROUND;
		assignedBinding.pSimulation = failedRoom;
		assignedApp.m_GameplayBindingBySessionId.emplace(
			89986u, assignedBinding);
		C2S_MOVE move{};
		move.iClientSequence = 2u;
		move.fGoalX = 1.f;
		move.fGoalZ = 2.f;
		CPacketWriter moveWriter;
		PACKET_FRAME moveFrame{};
		moveFrame.ePacketType = PACKET_TYPE::C2S_MOVE;
		const bool builtMoveFrame = Write_Message(moveWriter, move);
		moveFrame.Payload = moveWriter.Get_Buffer();
		assignedApp.On_SessionFrame(89986u, moveFrame);
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC assignedDiagnostic =
			assignedSession->Get_CloseDiagnostic();

		CServerApp entryApp;
		auto entrySession = std::make_shared<CClientSession>(
			89987u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		entryApp.m_Sessions.emplace(89987u, entrySession);
		entryApp.m_SharedGameRooms.emplace(
			WORLD_ID::TRAINING_GROUND, failedRoom);
		entryApp.m_pActiveGameplayGeneration =
			failedRoom->Get_ActiveGameplayGeneration();
		C2S_ENTER_WORLD enterWorld{};
		enterWorld.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
		enterWorld.eWorldId = WORLD_ID::TRAINING_GROUND;
		enterWorld.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		enterWorld.strNickName = "RuntimeFailureEntryFixture";
		SESSION_DIAGNOSTIC_REASON entryReason =
			SESSION_DIAGNOSTIC_REASON::NONE;
		int entryNativeError = 0;
		std::string entryContext;
		const bool entryRejected = !entryApp.Bind_AndEnqueueEntry(
			89987u, WORLD_ID::TRAINING_GROUND, failedRoom, enterWorld,
			entryReason, entryNativeError, entryContext);

		tests.Require(
			ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_NOT_READY ==
				rejectedMoveResult && retainedFirstFailure && builtMoveFrame &&
			std::string::npos != rejectedMoveContext.find(
				"firstFailureTick=77") &&
			std::string::npos != rejectedMoveContext.find(
				"world-update.pattern-stage-transition") &&
			SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_RUNTIME_FAILED ==
				assignedDiagnostic.eReason &&
			WSAESHUTDOWN == assignedDiagnostic.iNativeErrorCode &&
			std::string::npos != assignedDiagnostic.strContext.find(
				"REJECTED_ROOM_NOT_READY") &&
			std::string::npos != assignedDiagnostic.strContext.find(
				"fixture stage transition transaction failed") &&
			entryRejected &&
			SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_RUNTIME_FAILED == entryReason &&
			WSAESHUTDOWN == entryNativeError &&
			std::string::npos != entryContext.find(
				"stage=register-ingress") &&
			std::string::npos != entryContext.find(
				"fixture stage transition transaction failed") &&
			!entryApp.m_GameplayBindingBySessionId.contains(89987u),
			"Classify a stopped room separately from ingress capacity and retain its first runtime failure for active-command and re-entry diagnostics");
	}
	{
		std::array<wchar_t, 32768u> modulePath{};
		const DWORD moduleLength = ::GetModuleFileNameW(
			nullptr, modulePath.data(), static_cast<DWORD>(modulePath.size()));
		const std::filesystem::path diagnosticPath =
			0u == moduleLength || moduleLength >= modulePath.size() ?
			std::filesystem::path{} :
			std::filesystem::path{ modulePath.data() }.parent_path() /
				L"Diagnostics" /
				(L"server-session-" +
					std::to_wstring(::GetCurrentProcessId()) + L".jsonl");
		std::error_code removeError;
		if (!diagnosticPath.empty())
			std::filesystem::remove(diagnosticPath, removeError);

		auto app = std::make_unique<CServerApp>();
		auto session = std::make_shared<CClientSession>(
			89994u, INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{}, CClientSession::CLOSED_HANDLER{});
		constexpr PLAYER_ID diagnosticPlayerId = 90094u;
		constexpr NET_ENTITY_ID diagnosticEntityId = 90194u;
		session->Bind_PlayerId(diagnosticPlayerId);
		session->Record_InboundPacket(PACKET_TYPE::C2S_MOVE);
		session->Request_Close(
			SESSION_DIAGNOSTIC_REASON::SERVER_RECEIVE_ERROR,
			WSAECONNRESET,
			"JSON schema contract fixture");
		auto simulation = std::make_shared<CGameRoom>(
			WORLD_ID::TRAINING_GROUND);
		SERVER_PLAYER diagnosticPlayer{};
		diagnosticPlayer.iSessionId = 89994u;
		diagnosticPlayer.iPlayerId = diagnosticPlayerId;
		diagnosticPlayer.iNetEntityId = diagnosticEntityId;
		diagnosticPlayer.eCharacterClass = CHARACTER_CLASS_ID::ARTIST;
		diagnosticPlayer.iCurrentHp = 100u;
		diagnosticPlayer.iMaximumHp = 100u;
		simulation->m_Sessions.emplace(89994u, session);
		simulation->m_Players.emplace(diagnosticPlayerId, diagnosticPlayer);
		simulation->m_PlayerIdBySessionId.emplace(
			89994u, diagnosticPlayerId);
		simulation->m_PlayerIdByEntityId.emplace(
			diagnosticEntityId, diagnosticPlayerId);
		CServerApp::SESSION_GAMEPLAY_BINDING binding{};
		binding.eWorldId = WORLD_ID::TRAINING_GROUND;
		binding.pSimulation = simulation;
		app->m_Sessions.emplace(89994u, session);
		app->m_GameplayBindingBySessionId.emplace(89994u, binding);
		app->On_SessionClosed(89994u);
		const bool cleanupGuaranteedForBoundClose =
			!app->m_GameplayBindingBySessionId.contains(89994u) &&
			1u == simulation->m_CleanupCommands.size() &&
			1u == simulation->m_QueuedCleanupSessionIds.size();

		std::string jsonLine;
		if (!diagnosticPath.empty())
		{
			std::ifstream log{ diagnosticPath, std::ios::binary };
			(void)std::getline(log, jsonLine);
		}
		std::string jsonStatus;
		const bool parsed = CServerApp::Validate_ServerSessionDiagnosticJson(
			jsonLine, jsonStatus);
		const bool schemaExact = std::string::npos != jsonLine.find(
			"\"schema\":\"lostark.server-session-diagnostic\"") &&
			std::string::npos != jsonLine.find("\"formatVersion\":1") &&
			std::string::npos == jsonLine.find("\"schemaVersion\"") &&
			std::string::npos != jsonLine.find("\"wasBound\":true") &&
			std::string::npos != jsonLine.find("\"leaveEnqueued\":true");
		simulation->Tick(1.f / 30.f);
		const bool cleanupReleasedBoundPlayer =
			simulation->m_CleanupCommands.empty() &&
			simulation->m_QueuedCleanupSessionIds.empty() &&
			!simulation->m_Sessions.contains(89994u) &&
			!simulation->m_Players.contains(diagnosticPlayerId) &&
			!simulation->m_PlayerIdBySessionId.contains(89994u) &&
			!simulation->m_PlayerIdByEntityId.contains(diagnosticEntityId) &&
			INVALID_PLAYER_ID == session->Get_PlayerId();
		if (!diagnosticPath.empty())
			std::filesystem::remove(diagnosticPath, removeError);
		tests.Require(
			!diagnosticPath.empty() && parsed && schemaExact &&
			jsonStatus.empty() && cleanupGuaranteedForBoundClose &&
			cleanupReleasedBoundPlayer,
			"Write the versioned diagnostic and guarantee cleanup for a bound close");
	}
	{
		CWinSockContext winSock;
		SOCKET sessionSocket = INVALID_SOCKET;
		SOCKET peerSocket = INVALID_SOCKET;
		const bool socketReady = winSock.Initialize() &&
			Create_LoopbackSocketPair(sessionSocket, peerSocket);
		std::atomic_uint32_t closedCount{ 0u };
		std::unique_ptr<CClientSession> session;
		if (socketReady)
		{
			session = std::make_unique<CClientSession>(
				89998u,
				sessionSocket,
				CClientSession::FRAME_HANDLER{},
				[&closedCount](const SESSION_ID) { ++closedCount; });
			sessionSocket = INVALID_SOCKET;
		}
		const bool started = socketReady && session->Start();
		CLIENT_SESSION_PEER_ENDPOINT peerEndpoint{};
		if (started)
		{
			peerEndpoint = session->Get_PeerEndpoint();
			Close_TestSocket(peerSocket);
			(void)Wait_Until(
				std::chrono::milliseconds(1500),
				[&closedCount]() { return 1u == closedCount.load(); });
		}
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC diagnostic =
			nullptr == session ? CLIENT_SESSION_CLOSE_DIAGNOSTIC{} :
			session->Get_CloseDiagnostic();
		if (session)
			session->Stop();
		session.reset();
		Close_TestSocket(sessionSocket);
		Close_TestSocket(peerSocket);
		tests.Require(
			started && 1u == closedCount.load() &&
			"127.0.0.1" == peerEndpoint.strAddress &&
			0u != peerEndpoint.iPort &&
			SESSION_DIAGNOSTIC_REASON::SERVER_PEER_CLOSED ==
				diagnostic.eReason &&
			0 == diagnostic.iNativeErrorCode,
			"Capture immutable peer endpoint and an orderly peer-close reason");
	}
	{
		CWinSockContext winSock;
		SOCKET sessionSocket = INVALID_SOCKET;
		SOCKET peerSocket = INVALID_SOCKET;
		const bool socketReady = winSock.Initialize() &&
			Create_LoopbackSocketPair(sessionSocket, peerSocket);
		std::atomic_uint32_t closedCount{ 0u };
		std::unique_ptr<CClientSession> session;
		if (socketReady)
		{
			session = std::make_unique<CClientSession>(
				89999u,
				sessionSocket,
				CClientSession::FRAME_HANDLER{},
				[&closedCount](const SESSION_ID) { ++closedCount; });
			sessionSocket = INVALID_SOCKET;
		}
		const bool started = socketReady && session->Start();
		bool invalidFrameSent = false;
		if (started)
		{
			const std::array<std::uint8_t, PACKET_HEADER_BYTES> invalidFrame
			{
				5u, 0u, 0u, 0u,
				static_cast<std::uint8_t>(PACKET_TYPE::C2S_ENTER_WORLD), 0u
			};
			invalidFrameSent = static_cast<int>(invalidFrame.size()) == ::send(
				peerSocket,
				reinterpret_cast<const char*>(invalidFrame.data()),
				static_cast<int>(invalidFrame.size()),
				0);
			(void)Wait_Until(
				std::chrono::milliseconds(1500),
				[&closedCount]() { return 1u == closedCount.load(); });
		}
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC diagnostic =
			nullptr == session ? CLIENT_SESSION_CLOSE_DIAGNOSTIC{} :
			session->Get_CloseDiagnostic();
		if (session)
			session->Stop();
		session.reset();
		Close_TestSocket(sessionSocket);
		Close_TestSocket(peerSocket);
		tests.Require(
			started && invalidFrameSent && 1u == closedCount.load() &&
			SESSION_DIAGNOSTIC_REASON::SERVER_INVALID_FRAME ==
				diagnostic.eReason &&
			WSAEPROTONOSUPPORT == diagnostic.iNativeErrorCode &&
			PACKET_TYPE::INVALID == diagnostic.eLastInboundPacket,
			"Classify an invalid TCP frame before closing only its session");
	}
	{
		CClientSession session{ 90001u, INVALID_SOCKET, {}, {} };
		session.m_isSendRunning.store(true);
		const auto firstSnapshot = session.Queue_OutboundFrame(
			PACKET_TYPE::S2C_WORLD_SNAPSHOT, { 1u });
		const auto firstReliable = session.Queue_OutboundFrame(
			PACKET_TYPE::S2C_CHAT, { 10u });
		const auto secondSnapshot = session.Queue_OutboundFrame(
			PACKET_TYPE::S2C_WORLD_SNAPSHOT, { 2u });
		const auto secondReliable = session.Queue_OutboundFrame(
			PACKET_TYPE::S2C_PLAYER_DESPAWNED, { 11u });
		const auto thirdSnapshot = session.Queue_OutboundFrame(
			PACKET_TYPE::S2C_WORLD_SNAPSHOT, { 3u });
		const CLIENT_SESSION_OUTBOUND_METRICS metrics =
			session.Get_OutboundMetrics();
		const bool latestWinsAndReliableOrder =
			CClientSession::OUTBOUND_ENQUEUE_RESULT::QUEUED == firstSnapshot &&
			CClientSession::OUTBOUND_ENQUEUE_RESULT::QUEUED == firstReliable &&
			CClientSession::OUTBOUND_ENQUEUE_RESULT::COALESCED == secondSnapshot &&
			CClientSession::OUTBOUND_ENQUEUE_RESULT::QUEUED == secondReliable &&
			CClientSession::OUTBOUND_ENQUEUE_RESULT::COALESCED == thirdSnapshot &&
			3u == session.m_OutboundFrames.size() &&
			PACKET_TYPE::S2C_CHAT ==
				session.m_OutboundFrames[0u].ePacketType &&
			10u == session.m_OutboundFrames[0u].Bytes.front() &&
			PACKET_TYPE::S2C_PLAYER_DESPAWNED ==
				session.m_OutboundFrames[1u].ePacketType &&
			11u == session.m_OutboundFrames[1u].Bytes.front() &&
			PACKET_TYPE::S2C_WORLD_SNAPSHOT ==
				session.m_OutboundFrames[2u].ePacketType &&
			3u == session.m_OutboundFrames[2u].Bytes.front() &&
			3u == metrics.iSnapshotEnqueuedFrameCount &&
			2u == metrics.iSnapshotCoalescedFrameCount &&
			2u == metrics.iReliableEnqueuedFrameCount;
		session.Request_Close();
		tests.Require(
			latestWinsAndReliableOrder,
			"Coalesce queued snapshots to latest-wins without reordering reliable frames");
	}
	{
		CClientSession session{ 90002u, INVALID_SOCKET, {}, {} };
		session.m_isSendRunning.store(true);
		constexpr std::size_t SNAPSHOT_FRAME_LIMIT =
			CClientSession::MAX_OUTBOUND_FRAME_COUNT -
			CClientSession::RELIABLE_FRAME_RESERVE;
		bool admittedSnapshotBand = true;
		for (std::size_t index = 0u; index < SNAPSHOT_FRAME_LIMIT; ++index)
		{
			admittedSnapshotBand = admittedSnapshotBand &&
				CClientSession::OUTBOUND_ENQUEUE_RESULT::QUEUED ==
				session.Queue_OutboundFrame(
					PACKET_TYPE::S2C_CHAT,
					{ static_cast<std::uint8_t>(index) });
		}
		const auto droppedSnapshot = session.Queue_OutboundFrame(
			PACKET_TYPE::S2C_WORLD_SNAPSHOT, { 1u });
		bool admittedReliableReserve = true;
		while (session.m_OutboundFrames.size() <
			CClientSession::MAX_OUTBOUND_FRAME_COUNT)
		{
			admittedReliableReserve = admittedReliableReserve &&
				CClientSession::OUTBOUND_ENQUEUE_RESULT::QUEUED ==
				session.Queue_OutboundFrame(
					PACKET_TYPE::S2C_CHAT, { 2u });
		}
		const auto overflow = session.Queue_OutboundFrame(
			PACKET_TYPE::S2C_CHAT, { 3u });
		const bool queuePreservedOnOverflow =
			CClientSession::MAX_OUTBOUND_FRAME_COUNT ==
				session.m_OutboundFrames.size();
		const std::array<std::uint8_t, 1u> payload{ 4u };
		const bool publicOverflowFailedClosed =
			!session.Send_Frame(PACKET_TYPE::S2C_CHAT, payload) &&
			!session.m_isSendRunning.load() &&
			session.m_OutboundFrames.empty();
		const CLIENT_SESSION_CLOSE_DIAGNOSTIC closeDiagnostic =
			session.Get_CloseDiagnostic();
		const CLIENT_SESSION_OUTBOUND_METRICS metrics =
			session.Get_OutboundMetrics();
		tests.Require(
			admittedSnapshotBand && admittedReliableReserve &&
			CClientSession::OUTBOUND_ENQUEUE_RESULT::DROPPED_SNAPSHOT ==
				droppedSnapshot &&
			CClientSession::OUTBOUND_ENQUEUE_RESULT::RELIABLE_OVERFLOW ==
				overflow &&
			queuePreservedOnOverflow && publicOverflowFailedClosed &&
			SESSION_DIAGNOSTIC_REASON::SERVER_RELIABLE_OUTBOUND_OVERFLOW ==
				closeDiagnostic.eReason &&
			WSAENOBUFS == closeDiagnostic.iNativeErrorCode &&
			CClientSession::MAX_OUTBOUND_FRAME_COUNT ==
				closeDiagnostic.iQueuedFrameCountAtClose &&
			1u == metrics.iSnapshotDroppedFrameCount &&
			2u == metrics.iReliableRejectedFrameCount &&
			CClientSession::MAX_OUTBOUND_FRAME_COUNT ==
				metrics.iQueuedFrameHighWatermark,
			"Reserve outbound capacity for reliable FIFO and fail-close only the overflowing session");
	}
	{
		CWinSockContext winSock;
		const bool initialized = winSock.Initialize();
		SOCKET slowSessionSocket = INVALID_SOCKET;
		SOCKET slowPeerSocket = INVALID_SOCKET;
		SOCKET fastSessionSocket = INVALID_SOCKET;
		SOCKET fastPeerSocket = INVALID_SOCKET;
		bool socketsReady = initialized &&
			Create_LoopbackSocketPair(slowSessionSocket, slowPeerSocket) &&
			Create_LoopbackSocketPair(fastSessionSocket, fastPeerSocket);
		if (socketsReady)
		{
			const int smallBufferBytes = 1024;
			socketsReady =
				SOCKET_ERROR != ::setsockopt(
					slowSessionSocket,
					SOL_SOCKET,
					SO_SNDBUF,
					reinterpret_cast<const char*>(&smallBufferBytes),
					static_cast<int>(sizeof(smallBufferBytes))) &&
				SOCKET_ERROR != ::setsockopt(
					slowPeerSocket,
					SOL_SOCKET,
					SO_RCVBUF,
					reinterpret_cast<const char*>(&smallBufferBytes),
					static_cast<int>(sizeof(smallBufferBytes)));
		}

		std::atomic_uint32_t slowClosedCount{ 0u };
		std::atomic_uint32_t fastClosedCount{ 0u };
		std::unique_ptr<CClientSession> slowSession;
		std::unique_ptr<CClientSession> fastSession;
		if (socketsReady)
		{
			slowSession = std::make_unique<CClientSession>(
				90003u,
				slowSessionSocket,
				CClientSession::FRAME_HANDLER{},
				[&slowClosedCount](const SESSION_ID)
				{
					++slowClosedCount;
				});
			slowSessionSocket = INVALID_SOCKET;
			fastSession = std::make_unique<CClientSession>(
				90004u,
				fastSessionSocket,
				CClientSession::FRAME_HANDLER{},
				[&fastClosedCount](const SESSION_ID)
				{
					++fastClosedCount;
				});
			fastSessionSocket = INVALID_SOCKET;
		}

		const bool sessionsStarted = socketsReady &&
			slowSession->Start() && fastSession->Start();
		bool slowEnqueueStayedBounded = false;
		bool slowCoalescedSnapshots = false;
		bool fastReliableFifo = false;
		bool slowClosureWasIsolated = false;
		bool cleanShutdownWasBounded = false;
		if (sessionsStarted)
		{
			std::vector<std::uint8_t> snapshotPayload(
				static_cast<std::size_t>(MAX_PACKET_BYTES) -
					PACKET_HEADER_BYTES,
				0xA5u);
			const auto enqueueStart = std::chrono::steady_clock::now();
			std::size_t acceptedSnapshotCount = 0u;
			for (std::size_t index = 0u; index < 256u; ++index)
			{
				snapshotPayload.front() = static_cast<std::uint8_t>(index);
				if (!slowSession->Send_Frame(
					PACKET_TYPE::S2C_WORLD_SNAPSHOT,
					snapshotPayload))
				{
					break;
				}
				++acceptedSnapshotCount;
			}
			const auto enqueueElapsed = std::chrono::steady_clock::now() -
				enqueueStart;
			const CLIENT_SESSION_OUTBOUND_METRICS slowMetrics =
				slowSession->Get_OutboundMetrics();
			slowEnqueueStayedBounded = acceptedSnapshotCount > 1u &&
				enqueueElapsed < std::chrono::milliseconds(1500);
			slowCoalescedSnapshots =
				0u < slowMetrics.iSnapshotCoalescedFrameCount &&
				1u >= slowMetrics.iCurrentQueuedFrameCount;

			const std::array<std::uint8_t, 1u> firstPayload{ 21u };
			const std::array<std::uint8_t, 1u> secondPayload{ 22u };
			const std::array<std::uint8_t, 1u> thirdPayload{ 23u };
			const bool queuedFastFrames =
				fastSession->Send_Frame(PACKET_TYPE::S2C_CHAT, firstPayload) &&
				fastSession->Send_Frame(
					PACKET_TYPE::S2C_ENTER_REJECTED, secondPayload) &&
				fastSession->Send_Frame(
					PACKET_TYPE::S2C_PLAYER_DESPAWNED, thirdPayload);
			RECEIVED_TEST_FRAME receivedFirst{};
			RECEIVED_TEST_FRAME receivedSecond{};
			RECEIVED_TEST_FRAME receivedThird{};
			fastReliableFifo = queuedFastFrames &&
				Receive_TestFrame(fastPeerSocket, receivedFirst) &&
				Receive_TestFrame(fastPeerSocket, receivedSecond) &&
				Receive_TestFrame(fastPeerSocket, receivedThird) &&
				PACKET_TYPE::S2C_CHAT == receivedFirst.packetType &&
				1u == receivedFirst.payload.size() &&
				firstPayload.front() == receivedFirst.payload.front() &&
				PACKET_TYPE::S2C_ENTER_REJECTED == receivedSecond.packetType &&
				1u == receivedSecond.payload.size() &&
				secondPayload.front() == receivedSecond.payload.front() &&
				PACKET_TYPE::S2C_PLAYER_DESPAWNED == receivedThird.packetType &&
				1u == receivedThird.payload.size() &&
				thirdPayload.front() == receivedThird.payload.front();

			Close_TestSocket(slowPeerSocket);
			const bool slowSessionClosed = Wait_Until(
				std::chrono::milliseconds(1500),
				[&slowClosedCount]()
				{
					return 0u < slowClosedCount.load();
				});
			const std::array<std::uint8_t, 1u> survivorPayload{ 24u };
			RECEIVED_TEST_FRAME survivorFrame{};
			slowClosureWasIsolated = slowSessionClosed &&
				fastSession->Send_Frame(PACKET_TYPE::S2C_CHAT, survivorPayload) &&
				Receive_TestFrame(fastPeerSocket, survivorFrame) &&
				PACKET_TYPE::S2C_CHAT == survivorFrame.packetType &&
				1u == survivorFrame.payload.size() &&
				survivorPayload.front() == survivorFrame.payload.front();

			const auto shutdownStart = std::chrono::steady_clock::now();
			slowSession->Stop();
			fastSession->Stop();
			const auto shutdownElapsed = std::chrono::steady_clock::now() -
				shutdownStart;
			cleanShutdownWasBounded =
				shutdownElapsed < std::chrono::milliseconds(3000) &&
				1u == slowClosedCount.load() &&
				1u == fastClosedCount.load();
		}
		else
		{
			if (slowSession)
				slowSession->Stop();
			if (fastSession)
				fastSession->Stop();
		}

		slowSession.reset();
		fastSession.reset();
		Close_TestSocket(slowSessionSocket);
		Close_TestSocket(slowPeerSocket);
		Close_TestSocket(fastSessionSocket);
		Close_TestSocket(fastPeerSocket);
		tests.Require(
			initialized && socketsReady && sessionsStarted &&
			slowEnqueueStayedBounded && slowCoalescedSnapshots &&
			fastReliableFifo && slowClosureWasIsolated &&
			cleanShutdownWasBounded,
			"Isolate a slow reader while preserving another session FIFO and bounded shutdown");
	}
	{
		CWinSockContext winSock;
		SOCKET sessionSocket = INVALID_SOCKET;
		SOCKET peerSocket = INVALID_SOCKET;
		bool socketReady = winSock.Initialize() &&
			Create_LoopbackSocketPair(sessionSocket, peerSocket);
		if (socketReady)
		{
			const int smallBufferBytes = 1024;
			socketReady = SOCKET_ERROR != ::setsockopt(
				sessionSocket, SOL_SOCKET, SO_SNDBUF,
				reinterpret_cast<const char*>(&smallBufferBytes),
				static_cast<int>(sizeof(smallBufferBytes))) &&
				SOCKET_ERROR != ::setsockopt(
					peerSocket, SOL_SOCKET, SO_RCVBUF,
					reinterpret_cast<const char*>(&smallBufferBytes),
					static_cast<int>(sizeof(smallBufferBytes)));
		}
		std::unique_ptr<CClientSession> session;
		bool sendTimeoutConfigured = false;
		if (socketReady)
		{
			session = std::make_unique<CClientSession>(
				90008u,
				sessionSocket,
				CClientSession::FRAME_HANDLER{},
				CClientSession::CLOSED_HANDLER{});
			sessionSocket = INVALID_SOCKET;
			sendTimeoutConfigured = session->Configure_TransportOptions();
		}

		bool slowReaderTimedOut = false;
		bool shutdownWasBounded = false;
		if (sendTimeoutConfigured)
		{
			session->m_isSendRunning.store(true);
			std::vector<std::uint8_t> bytes(
				static_cast<std::size_t>(MAX_PACKET_BYTES), 0x5au);
			bool sendFailed = false;
			for (std::uint32_t index = 0u; index < 64u; ++index)
			{
				bytes.front() = static_cast<std::uint8_t>(index);
				if (!session->Send_All(bytes))
				{
					sendFailed = true;
					break;
				}
			}
			const CLIENT_SESSION_CLOSE_DIAGNOSTIC diagnostic =
				session->Get_CloseDiagnostic();
			slowReaderTimedOut = sendFailed &&
				SESSION_DIAGNOSTIC_REASON::SERVER_SEND_ERROR_OR_TIMEOUT ==
					diagnostic.eReason &&
				0 != diagnostic.iNativeErrorCode;
			const auto shutdownStart = std::chrono::steady_clock::now();
			session->Stop();
			shutdownWasBounded =
				std::chrono::steady_clock::now() - shutdownStart <
				std::chrono::milliseconds(3000);
		}
		else if (session)
		{
			session->Stop();
		}

		session.reset();
		Close_TestSocket(sessionSocket);
		Close_TestSocket(peerSocket);
		tests.Require(
			socketReady && sendTimeoutConfigured && slowReaderTimedOut &&
			shutdownWasBounded,
			"Classify a non-draining peer as an isolated bounded send timeout");
	}
	{
		CWinSockContext winSock;
		SOCKET sessionSocket = INVALID_SOCKET;
		SOCKET peerSocket = INVALID_SOCKET;
		const bool socketReady = winSock.Initialize() &&
			Create_LoopbackSocketPair(sessionSocket, peerSocket);
		std::atomic_uint32_t closedCount{ 0u };
		std::unique_ptr<CClientSession> session;
		if (socketReady)
		{
			session = std::make_unique<CClientSession>(
				90006u,
				sessionSocket,
				CClientSession::FRAME_HANDLER{},
				[&closedCount](const SESSION_ID)
				{
					++closedCount;
				});
			sessionSocket = INVALID_SOCKET;
		}

		const bool started = socketReady && session->Start();
		bool reliableFlushedBeforeClose = false;
		bool rejectedLateEnqueue = false;
		bool shutdownWasBounded = false;
		if (started)
		{
			const std::array<std::uint8_t, 1u> payload{ 0x46u };
			const bool queued = session->Send_Frame(
				PACKET_TYPE::S2C_ENTER_REJECTED, payload);
			session->Request_Close_After_Flush();
			RECEIVED_TEST_FRAME received{};
			reliableFlushedBeforeClose = queued &&
				Receive_TestFrame(peerSocket, received) &&
				PACKET_TYPE::S2C_ENTER_REJECTED == received.packetType &&
				1u == received.payload.size() &&
				payload.front() == received.payload.front() &&
				Wait_Until(
					std::chrono::milliseconds(1500),
					[&session, &closedCount]()
					{
						return 1u == closedCount.load() &&
							!session->m_isSendRunning.load();
					});
			rejectedLateEnqueue = !session->Send_Frame(
				PACKET_TYPE::S2C_CHAT, payload);
			const auto shutdownStart = std::chrono::steady_clock::now();
			session->Stop();
			shutdownWasBounded =
				std::chrono::steady_clock::now() - shutdownStart <
					std::chrono::milliseconds(3000);
		}
		else if (session)
		{
			session->Stop();
		}

		session.reset();
		Close_TestSocket(sessionSocket);
		Close_TestSocket(peerSocket);
		tests.Require(
			started && reliableFlushedBeforeClose && rejectedLateEnqueue &&
			shutdownWasBounded && 1u == closedCount.load(),
			"Flush a terminal reliable frame before sender-owned graceful close");
	}
	{
		std::atomic_uint32_t closedCount{ 0u };
		CClientSession session{
			90007u,
			INVALID_SOCKET,
			CClientSession::FRAME_HANDLER{},
			[&closedCount](const SESSION_ID)
			{
				++closedCount;
			} };
		session.m_isSendRunning.store(true);
		session.Request_Close_After_Flush();
		session.Request_Close();
		session.Sender_Loop();
		session.Notify_Closed();
		tests.Require(
			1u == closedCount.load() &&
			!session.m_isSendRunning.load() &&
			session.m_hasSenderExited,
			"Notify close exactly once when a hard stop overtakes graceful flush");
	}
	{
		CWinSockContext winSock;
		SOCKET sessionSocket = INVALID_SOCKET;
		SOCKET peerSocket = INVALID_SOCKET;
		const bool socketReady = winSock.Initialize() &&
			Create_LoopbackSocketPair(sessionSocket, peerSocket);
		std::atomic_uint32_t closedCount{ 0u };
		std::unique_ptr<CClientSession> session;
		bool senderStarted = false;
		if (socketReady)
		{
			session = std::make_unique<CClientSession>(
				90005u,
				sessionSocket,
				CClientSession::FRAME_HANDLER{},
				[&closedCount](const SESSION_ID)
				{
					++closedCount;
				});
			sessionSocket = INVALID_SOCKET;
			senderStarted = session->Configure_TransportOptions();
			if (senderStarted)
			{
				{
					std::scoped_lock lock{ session->m_OutboundMutex };
					session->m_hasSenderExited = false;
				}
				session->m_isSendRunning.store(true);
				try
				{
					session->m_SendThread = std::thread(
						&CClientSession::Sender_Loop,
						session.get());
				}
				catch (...)
				{
					senderStarted = false;
				}
			}
		}

		bool sendFailureWasIsolated = false;
		bool sendFailureWasClassified = false;
		bool shutdownWasBounded = false;
		if (senderStarted)
		{
			Abort_TestSocket(peerSocket);
			const std::array<std::uint8_t, 1u> payload{ 31u };
			const bool queued = session->Send_Frame(
				PACKET_TYPE::S2C_CHAT, payload);
			sendFailureWasIsolated = queued && Wait_Until(
				std::chrono::milliseconds(1500),
				[&session, &closedCount]()
				{
					return 0u < session->Get_OutboundMetrics().iSendFailureCount &&
					1u == closedCount.load() &&
					!session->m_isSendRunning.load();
				});
			const CLIENT_SESSION_CLOSE_DIAGNOSTIC diagnostic =
				session->Get_CloseDiagnostic();
			sendFailureWasClassified =
				SESSION_DIAGNOSTIC_REASON::SERVER_SEND_ERROR_OR_TIMEOUT ==
					diagnostic.eReason &&
				0 != diagnostic.iNativeErrorCode &&
				!diagnostic.strContext.empty();
			const auto shutdownStart = std::chrono::steady_clock::now();
			session->Stop();
			shutdownWasBounded =
				std::chrono::steady_clock::now() - shutdownStart <
					std::chrono::milliseconds(3000);
		}
		else if (session)
		{
			session->Stop();
		}

		session.reset();
		Close_TestSocket(sessionSocket);
		Close_TestSocket(peerSocket);
		tests.Require(
			socketReady && senderStarted && sendFailureWasIsolated &&
			sendFailureWasClassified &&
			shutdownWasBounded && 1u == closedCount.load(),
			"Isolate a sender socket failure to its session and notify close exactly once");
	}
	{
		/* Heap-allocated: the contract frame already sits near the 1 MiB production stack. */
		auto roomStorage = std::make_unique<CGameRoom>(WORLD_ID::TRAINING_GROUND);
		CGameRoom& room = *roomStorage;
		auto moveCommand = [](const SESSION_ID sessionId,
			const std::uint32_t sequence)
			{
				ROOM_COMMAND command{};
				command.eType = ROOM_COMMAND_TYPE::MOVE;
				command.iSessionId = sessionId;
				command.Move.iClientSequence = sequence;
				command.Move.fGoalX = static_cast<float>(sequence);
				return command;
			};
		auto skillCommand = [](const SESSION_ID sessionId,
			const std::uint32_t sequence)
			{
				ROOM_COMMAND command{};
				command.eType = ROOM_COMMAND_TYPE::USE_SKILL;
				command.iSessionId = sessionId;
				command.UseSkill.iClientSequence = sequence;
				command.UseSkill.iSkillId = 34010u;
				return command;
			};
		auto aimCommand = [](const SESSION_ID sessionId,
			const std::uint32_t sequence,
			const SKILL_ID skillId)
			{
				ROOM_COMMAND command{};
				command.eType = ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM;
				command.iSessionId = sessionId;
				command.UpdateSkillAim.iClientSequence = sequence;
				command.UpdateSkillAim.iSkillId = skillId;
				return command;
			};

		const bool enqueued = room.Is_Ready() &&
			room.Enqueue(moveCommand(1u, 1u)) &&
			room.Enqueue(skillCommand(2u, 10u)) &&
			room.Enqueue(moveCommand(1u, 2u)) &&
			room.Enqueue(skillCommand(1u, 20u)) &&
			room.Enqueue(moveCommand(1u, 3u)) &&
			room.Enqueue(aimCommand(1u, 30u, 34590u)) &&
			room.Enqueue(aimCommand(1u, 31u, 34590u)) &&
			room.Enqueue(aimCommand(1u, 32u, 34580u));
		const SERVER_ROOM_PERFORMANCE_METRICS metrics =
			room.Get_PerformanceMetrics();
		const bool preservedBarriers =
			6u == room.m_InboundCommands.size() &&
			ROOM_COMMAND_TYPE::USE_SKILL ==
				room.m_InboundCommands[0u].eType &&
			2u == room.m_InboundCommands[0u].iSessionId &&
			ROOM_COMMAND_TYPE::MOVE ==
				room.m_InboundCommands[1u].eType &&
			2u == room.m_InboundCommands[1u].Move.iClientSequence &&
			ROOM_COMMAND_TYPE::USE_SKILL ==
				room.m_InboundCommands[2u].eType &&
			ROOM_COMMAND_TYPE::MOVE ==
				room.m_InboundCommands[3u].eType &&
			3u == room.m_InboundCommands[3u].Move.iClientSequence &&
			ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM ==
				room.m_InboundCommands[4u].eType &&
			31u == room.m_InboundCommands[4u].UpdateSkillAim.iClientSequence &&
			ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM ==
				room.m_InboundCommands[5u].eType &&
			34580u == room.m_InboundCommands[5u].UpdateSkillAim.iSkillId;
		tests.Require(
			enqueued && preservedBarriers &&
			1u == metrics.iCoalescedMoveCommandCount &&
			1u == metrics.iCoalescedAimCommandCount,
			"Coalesce only same-stream movement and aim without crossing a same-session reliable barrier");
	}
}

