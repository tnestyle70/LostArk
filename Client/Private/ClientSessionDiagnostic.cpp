#include "ClientSessionDiagnostic.h"

#include <Windows.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iterator>
#include <limits>
#include <system_error>
#include <utility>

namespace
{
	constexpr std::size_t MAX_DIAGNOSTIC_DETAIL_BYTES = 1024u;
}

void Client::CClientSessionDiagnostic::Begin_Attempt(
	const std::string_view host,
	const std::uint16_t port,
	const std::uint16_t protocolVersion)
{
	std::scoped_lock lock{ m_Mutex };
	const std::uint64_t nextGeneration =
		m_Snapshot.iConnectionGeneration ==
			(std::numeric_limits<std::uint64_t>::max)() ?
			1u : m_Snapshot.iConnectionGeneration + 1u;
	CLIENT_SESSION_DIAGNOSTIC_SNAPSHOT next{};
	next.iConnectionGeneration = nextGeneration;
	next.strEndpoint = std::string{ host } + ":" + std::to_string(port);
	next.iProtocolVersion = protocolVersion;
	next.iConnectStartedUnixMs = Get_UnixMilliseconds();
	m_Snapshot = std::move(next);
	Ensure_CapturePathLocked();
	Append_EventLocked(
		"connect.begin",
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON::NONE,
		{});
}

void Client::CClientSessionDiagnostic::Record_Event(
	const std::string_view eventName,
	const std::string_view detail)
{
	std::scoped_lock lock{ m_Mutex };
	Ensure_CapturePathLocked();
	Append_EventLocked(
		eventName,
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON::NONE,
		detail);
}

void Client::CClientSessionDiagnostic::Record_Recovery(
	const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
	const std::string_view source,
	const std::string_view detail)
{
	if (!LostArk::Shared::Is_Known_SessionDiagnosticReason(reason) ||
		source.empty())
	{
		return;
	}

	std::scoped_lock lock{ m_Mutex };
	Ensure_CapturePathLocked();
	Append_EventLocked("recovery.reported", reason, detail, source);
}

void Client::CClientSessionDiagnostic::Record_LocalEndpoint(
	const std::string_view localEndpoint)
{
	std::scoped_lock lock{ m_Mutex };
	if (m_Snapshot.isTerminal)
		return;
	m_Snapshot.strLocalEndpoint = localEndpoint.empty() ?
		"unavailable" : Clamp_Detail(localEndpoint);
}

void Client::CClientSessionDiagnostic::Record_EnterSent(
	const LostArk::Shared::WORLD_ID worldId)
{
	std::scoped_lock lock{ m_Mutex };
	// Send completion can precede a worker-observed FIN even when this main-thread
	// bookkeeping runs after the terminal latch. Enrich causal context without
	// changing the first terminal reason, time, packet, or detail.
	m_Snapshot.eWorldId = worldId;
	Append_EventLocked(
		"entry.sent",
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON::NONE,
		{});
}

void Client::CClientSessionDiagnostic::Record_EnterAccepted(
	const LostArk::Shared::WORLD_ID worldId,
	const LostArk::Shared::PLAYER_ID playerId,
	const LostArk::Shared::NET_ENTITY_ID netEntityId)
{
	std::scoped_lock lock{ m_Mutex };
	// The frame was already received and queued before a later FIN could latch.
	// Preserve its decoded identity for Lobby/Server correlation.
	m_Snapshot.eWorldId = worldId;
	m_Snapshot.iPlayerId = playerId;
	m_Snapshot.iNetEntityId = netEntityId;
	Append_EventLocked(
		"entry.accepted",
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON::NONE,
		{});
}

void Client::CClientSessionDiagnostic::Record_EnterRejected(
	const LostArk::Shared::WORLD_ID worldId)
{
	std::scoped_lock lock{ m_Mutex };
	// Rejection decode may trail the worker's FIN observation. The validated
	// world remains useful semantic context while the transport terminal stays
	// immutable.
	m_Snapshot.eWorldId = worldId;
	Append_EventLocked(
		"entry.rejected",
		LostArk::Shared::SESSION_DIAGNOSTIC_REASON::NONE,
		{});
}

void Client::CClientSessionDiagnostic::Record_InboundFrame(
	const LostArk::Shared::PACKET_TYPE packetType,
	const std::size_t rawQueueDepth)
{
	std::scoped_lock lock{ m_Mutex };
	if (m_Snapshot.isTerminal)
	{
		m_Snapshot.iRawQueueDepth = rawQueueDepth;
		m_Snapshot.iRawQueueHighWatermark = (std::max)(
			m_Snapshot.iRawQueueHighWatermark, rawQueueDepth);
		return;
	}
	m_Snapshot.iLastReceiveUnixMs = Get_UnixMilliseconds();
	m_Snapshot.eTriggeringPacket = packetType;
	m_Snapshot.iRawQueueDepth = rawQueueDepth;
	m_Snapshot.iRawQueueHighWatermark = (std::max)(
		m_Snapshot.iRawQueueHighWatermark, rawQueueDepth);
}

void Client::CClientSessionDiagnostic::Record_EventQueueDepth(
	const std::size_t eventQueueDepth)
{
	std::scoped_lock lock{ m_Mutex };
	m_Snapshot.iEventQueueDepth = eventQueueDepth;
	m_Snapshot.iEventQueueHighWatermark = (std::max)(
		m_Snapshot.iEventQueueHighWatermark, eventQueueDepth);
}

void Client::CClientSessionDiagnostic::Record_RawQueueDepth(
	const std::size_t rawQueueDepth)
{
	std::scoped_lock lock{ m_Mutex };
	m_Snapshot.iRawQueueDepth = rawQueueDepth;
	m_Snapshot.iRawQueueHighWatermark = (std::max)(
		m_Snapshot.iRawQueueHighWatermark, rawQueueDepth);
}

void Client::CClientSessionDiagnostic::Record_ServerTick(
	const std::uint32_t serverTick)
{
	std::scoped_lock lock{ m_Mutex };
	// Only frames already accepted by the raw queue reach this main-thread call;
	// a later terminal latch must not discard their final observed Server tick.
	m_Snapshot.iLastServerTick = serverTick;
}

bool Client::CClientSessionDiagnostic::Record_Terminal(
	const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
	const int wsaError,
	const LostArk::Shared::PACKET_TYPE triggeringPacket,
	const std::string_view detail)
{
	if (!LostArk::Shared::Is_Known_SessionDiagnosticReason(reason))
		return false;

	std::scoped_lock lock{ m_Mutex };
	if (m_Snapshot.isTerminal)
		return false;

	m_Snapshot.isTerminal = true;
	m_Snapshot.eReason = reason;
	m_Snapshot.iWsaError = wsaError;
	if (LostArk::Shared::PACKET_TYPE::INVALID != triggeringPacket)
		m_Snapshot.eTriggeringPacket = triggeringPacket;
	m_Snapshot.iTerminalUnixMs = Get_UnixMilliseconds();
	m_Snapshot.strDetail = Clamp_Detail(detail);
	Ensure_CapturePathLocked();
	Append_EventLocked("terminal", reason, m_Snapshot.strDetail);
	return true;
}

Client::CLIENT_SESSION_DIAGNOSTIC_SNAPSHOT
Client::CClientSessionDiagnostic::Get_Snapshot() const
{
	std::scoped_lock lock{ m_Mutex };
	return m_Snapshot;
}

std::uint64_t Client::CClientSessionDiagnostic::Get_UnixMilliseconds() noexcept
{
	using namespace std::chrono;
	return static_cast<std::uint64_t>(duration_cast<milliseconds>(
		system_clock::now().time_since_epoch()).count());
}

std::string Client::CClientSessionDiagnostic::Clamp_Detail(
	const std::string_view detail)
{
	return std::string{
		detail.substr(0u, (std::min)(detail.size(), MAX_DIAGNOSTIC_DETAIL_BYTES)) };
}

std::string Client::CClientSessionDiagnostic::Escape_Json(
	const std::string_view value)
{
	std::string escaped;
	escaped.reserve(value.size());
	for (const unsigned char character : value)
	{
		switch (character)
		{
		case '\\': escaped += "\\\\"; break;
		case '"': escaped += "\\\""; break;
		case '\n': escaped += "\\n"; break;
		case '\r': escaped += "\\r"; break;
		case '\t': escaped += "\\t"; break;
		default:
			if (character >= 0x20u)
				escaped.push_back(static_cast<char>(character));
			break;
		}
	}
	return escaped;
}

void Client::CClientSessionDiagnostic::Ensure_CapturePathLocked()
{
	if (m_hasAttemptedCapturePath)
	{
		m_Snapshot.strCapturePath = m_CapturePath.string();
		m_Snapshot.strCaptureIoStatus = m_CaptureIoStatus;
		return;
	}
	m_hasAttemptedCapturePath = true;

	wchar_t modulePath[32768]{};
	const DWORD length = GetModuleFileNameW(
		nullptr, modulePath, static_cast<DWORD>(std::size(modulePath)));
	if (0u == length || length >= std::size(modulePath))
	{
		m_CaptureIoStatus =
			"GetModuleFileNameW failed; JSONL capture is unavailable.";
		m_Snapshot.strCaptureIoStatus = m_CaptureIoStatus;
		return;
	}

	std::error_code error;
	const std::filesystem::path directory =
		std::filesystem::path(modulePath).parent_path() / L"Diagnostics";
	std::filesystem::create_directories(directory, error);
	if (error)
	{
		m_CaptureIoStatus =
			"Diagnostics directory creation failed: " + error.message();
		m_Snapshot.strCaptureIoStatus = m_CaptureIoStatus;
		return;
	}

	m_CapturePath = directory /
		(L"client-session-" + std::to_wstring(GetCurrentProcessId()) + L".jsonl");
	m_Snapshot.strCapturePath = m_CapturePath.string();
	m_CaptureIoStatus = "ready";
	m_Snapshot.strCaptureIoStatus = m_CaptureIoStatus;
}

void Client::CClientSessionDiagnostic::Append_EventLocked(
	const std::string_view eventName,
	const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
	const std::string_view detail,
	const std::string_view source)
{
	if (m_CapturePath.empty())
		return;

	std::ofstream output{
		m_CapturePath, std::ios::binary | std::ios::out | std::ios::app };
	if (!output)
	{
		m_CaptureIoStatus =
			"JSONL capture open failed; gameplay continues without file capture.";
		m_Snapshot.strCaptureIoStatus = m_CaptureIoStatus;
		return;
	}

	const std::string reasonName{
		LostArk::Shared::To_SessionDiagnosticReasonName(reason) };
	const std::uint64_t eventUnixMs = Get_UnixMilliseconds();
	const std::uint64_t elapsedEnd = 0u != m_Snapshot.iTerminalUnixMs ?
		m_Snapshot.iTerminalUnixMs : eventUnixMs;
	const std::uint64_t elapsedMs =
		elapsedEnd >= m_Snapshot.iConnectStartedUnixMs ?
			elapsedEnd - m_Snapshot.iConnectStartedUnixMs : 0u;
	const std::uint64_t lastReceiveAgeMs =
		0u != m_Snapshot.iLastReceiveUnixMs &&
		elapsedEnd >= m_Snapshot.iLastReceiveUnixMs ?
			elapsedEnd - m_Snapshot.iLastReceiveUnixMs : 0u;
	output << "{\"schema\":\"" << CLIENT_SESSION_DIAGNOSTIC_SCHEMA << "\""
		<< ",\"formatVersion\":" << CLIENT_SESSION_DIAGNOSTIC_FORMAT_VERSION
		<< ",\"unixMs\":" << eventUnixMs
		<< ",\"processId\":" << GetCurrentProcessId()
		<< ",\"generation\":" << m_Snapshot.iConnectionGeneration
		<< ",\"event\":\"" << Escape_Json(eventName) << "\""
		<< ",\"source\":\"" << Escape_Json(Clamp_Detail(source)) << "\""
		<< ",\"remoteEndpoint\":\""
		<< Escape_Json(m_Snapshot.strEndpoint) << "\""
		<< ",\"localEndpoint\":\""
		<< Escape_Json(m_Snapshot.strLocalEndpoint) << "\""
		<< ",\"protocolVersion\":" << m_Snapshot.iProtocolVersion
		<< ",\"reason\":\"" << Escape_Json(reasonName) << "\""
		<< ",\"worldId\":" << static_cast<std::uint16_t>(m_Snapshot.eWorldId)
		<< ",\"playerId\":" << m_Snapshot.iPlayerId
		<< ",\"netEntityId\":" << m_Snapshot.iNetEntityId
		<< ",\"packetType\":"
		<< static_cast<std::uint16_t>(m_Snapshot.eTriggeringPacket)
		<< ",\"wsaError\":" << m_Snapshot.iWsaError
		<< ",\"connectStartedUnixMs\":"
		<< m_Snapshot.iConnectStartedUnixMs
		<< ",\"lastReceiveUnixMs\":" << m_Snapshot.iLastReceiveUnixMs
		<< ",\"terminalUnixMs\":" << m_Snapshot.iTerminalUnixMs
		<< ",\"elapsedMs\":" << elapsedMs
		<< ",\"lastReceiveAgeMs\":" << lastReceiveAgeMs
		<< ",\"lastServerTick\":" << m_Snapshot.iLastServerTick
		<< ",\"rawQueueDepth\":" << m_Snapshot.iRawQueueDepth
		<< ",\"rawQueueHighWatermark\":"
		<< m_Snapshot.iRawQueueHighWatermark
		<< ",\"eventQueueDepth\":" << m_Snapshot.iEventQueueDepth
		<< ",\"eventQueueHighWatermark\":"
		<< m_Snapshot.iEventQueueHighWatermark
		<< ",\"detail\":\"" << Escape_Json(Clamp_Detail(detail)) << "\"}\n";
	output.flush();
	if (!output)
	{
		m_CaptureIoStatus =
			"JSONL capture write failed; gameplay continues without file capture.";
		m_Snapshot.strCaptureIoStatus = m_CaptureIoStatus;
	}
}
