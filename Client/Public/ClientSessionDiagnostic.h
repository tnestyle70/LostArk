#pragma once

#include "Network/PacketMessages.h"
#include "Network/SessionDiagnostic.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <mutex>
#include <string>
#include <string_view>

namespace Client
{
	inline constexpr std::string_view CLIENT_SESSION_DIAGNOSTIC_SCHEMA =
		"lostark.client-session-diagnostic";
	inline constexpr std::uint32_t CLIENT_SESSION_DIAGNOSTIC_FORMAT_VERSION = 1u;

	struct CLIENT_SESSION_DIAGNOSTIC_SNAPSHOT final
	{
		std::uint64_t iConnectionGeneration = 0u;
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON eReason =
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON::NONE;
		bool isTerminal = false;
		std::string strEndpoint;
		std::string strLocalEndpoint = "unavailable";
		std::uint16_t iProtocolVersion = 0u;
		LostArk::Shared::WORLD_ID eWorldId = LostArk::Shared::WORLD_ID::END;
		LostArk::Shared::PLAYER_ID iPlayerId =
			LostArk::Shared::INVALID_PLAYER_ID;
		LostArk::Shared::NET_ENTITY_ID iNetEntityId =
			LostArk::Shared::INVALID_NET_ENTITY_ID;
		LostArk::Shared::PACKET_TYPE eTriggeringPacket =
			LostArk::Shared::PACKET_TYPE::INVALID;
		int iWsaError = 0;
		std::uint64_t iConnectStartedUnixMs = 0u;
		std::uint64_t iLastReceiveUnixMs = 0u;
		std::uint64_t iTerminalUnixMs = 0u;
		std::uint32_t iLastServerTick = 0u;
		std::size_t iRawQueueDepth = 0u;
		std::size_t iRawQueueHighWatermark = 0u;
		std::size_t iEventQueueDepth = 0u;
		std::size_t iEventQueueHighWatermark = 0u;
		std::string strDetail;
		std::string strCapturePath;
		std::string strCaptureIoStatus;
	};

	/* Owns one process capture file and one connection-generation snapshot.
	   Packet payloads and nicknames are never written. Frequent snapshot frames
	   update bounded counters only; connect/admission/terminal edges append one
	   JSONL line each. */
	class CClientSessionDiagnostic final
	{
	public:
		void Begin_Attempt(
			std::string_view host,
			std::uint16_t port,
			std::uint16_t protocolVersion);
		void Record_Event(
			std::string_view eventName,
			std::string_view detail = {});
		void Record_Recovery(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
			std::string_view source,
			std::string_view detail);
		void Record_LocalEndpoint(std::string_view localEndpoint);
		void Record_EnterSent(LostArk::Shared::WORLD_ID worldId);
		void Record_EnterAccepted(
			LostArk::Shared::WORLD_ID worldId,
			LostArk::Shared::PLAYER_ID playerId,
			LostArk::Shared::NET_ENTITY_ID netEntityId);
		void Record_EnterRejected(LostArk::Shared::WORLD_ID worldId);
		void Record_InboundFrame(
			LostArk::Shared::PACKET_TYPE packetType,
			std::size_t rawQueueDepth);
		void Record_RawQueueDepth(std::size_t rawQueueDepth);
		void Record_EventQueueDepth(std::size_t eventQueueDepth);
		void Record_ServerTick(std::uint32_t serverTick);
		bool Record_Terminal(
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
			int wsaError,
			LostArk::Shared::PACKET_TYPE triggeringPacket,
			std::string_view detail);
		[[nodiscard]] CLIENT_SESSION_DIAGNOSTIC_SNAPSHOT Get_Snapshot() const;

	private:
		static std::uint64_t Get_UnixMilliseconds() noexcept;
		static std::string Clamp_Detail(std::string_view detail);
		static std::string Escape_Json(std::string_view value);
		void Ensure_CapturePathLocked();
		void Append_EventLocked(
			std::string_view eventName,
			LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
			std::string_view detail,
			std::string_view source = {});

	private:
		mutable std::mutex m_Mutex;
		CLIENT_SESSION_DIAGNOSTIC_SNAPSHOT m_Snapshot;
		std::filesystem::path m_CapturePath;
		std::string m_CaptureIoStatus;
		bool m_hasAttemptedCapturePath = false;
	};
}
