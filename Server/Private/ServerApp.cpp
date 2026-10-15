// 프로세스 소유 세션·룸·데이터 트랜잭션을 구현하는 선언이다.
#include "ServerApp.h"

// 수락한 소켓을 세션으로 옮기고, 송신 큐·종료 진단·수신 콜백을 연결한다.
#include "ClientSession.h"

// 공유 메시지의 Read_Message/Write_Message와 프로토콜 enum을 사용한다.
#include "Network/PacketMessages.h"
// 수신 payload를 읽고 남은 바이트까지 검사해 패킷별 입력 계약을 확인한다.
#include "Network/PacketReader.h"
// 데이터 확정·수치 조정·진단 응답을 세션의 송신 프레임으로 직렬화한다.
#include "Network/PacketWriter.h"

// SOCKET의 실패 값·WSA 오류 코드와 수락 실패 소켓의 closesocket을 사용한다.
#include <WinSock2.h>
// CNG SHA-256으로 bootstrap·manifest의 실제 바이트 식별자를 계산한다.
#include <bcrypt.h>

// stdin을 가져와 콘솔 입력을 사용할 수 있는 프로세스인지 검사한다.
#include <cstdio>
// _fileno와 _isatty로 리디렉션 입력을 판별해 headless 대기를 선택한다.
#include <io.h>

// 30 Hz 룸 주기, 트랜잭션 제한 시간, Unix 진단 시각을 서로 맞는 시계로 계산한다.
#include <chrono>
// 멤버·참여자 검색, 정규 JSON 키 정렬, 만료 응답 기록 제거에 사용한다.
#include <algorithm>
// 모듈 경로를 받는 고정 크기 진단용 버퍼를 둔다.
#include <array>
// locale 영향 없이 JSON 정수와 bootstrap 버전·행 수를 파싱한다.
#include <charconv>
// 프로토콜 식별자와 해시 입력의 고정 폭 정수 표현을 사용한다.
#include <cstdint>
// 후보 경로·파일 크기·심볼릭 링크·정규 경로와 영속화 파일을 검사한다.
#include <filesystem>
// 후보 파일을 읽고 세션·룸 진단 로그를 기록한다.
#include <fstream>
// 프로세스 시작·종료·실패를 콘솔에 알리고 대화형 종료 입력을 기다린다.
#include <iostream>
// JSON 정수와 Win32 청크 길이의 자료형 상한을 검사한다.
#include <limits>
// 후보 artifact 경로 중복과 presentation lane의 정확한 집합을 검사한다.
#include <set>
// 연결 종료 시 수집한 값들을 한 JSON 진단 문자열로 조립한다.
#include <sstream>
// 소유권을 옮기지 않고 메모리 바이트 범위를 SHA-256 함수에 전달한다.
#include <span>
// 경로 식별자, 파싱 문자열, 실패 이유와 송신 응답 텍스트를 소유한다.
#include <string>
// 준비된 세션·룸·명령·트랜잭션 값을 다음 소유 컨테이너로 이동한다.
#include <utility>
// JSON 트리, 파일 바이트, 참여 룸과 일괄 종료 세션의 수명을 보관한다.
#include <vector>

// 아래 BCrypt 호출이 사용하는 Windows CNG import library를 링크한다.
#pragma comment(lib, "bcrypt.lib")

namespace
{
	using LostArk::Shared::GameplayDataRevision;

	// 세션 종료와 스케줄러 로그를 외부 기록과 대조하도록 Unix epoch 밀리초를 만든다.
	// 경과 시간 제한에는 이 벽시계 대신 steady_clock을 사용한다.
	std::uint64_t Current_UnixMilliseconds()
	{
		return static_cast<std::uint64_t>(
		    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
		        .count());
	}

	// 종료 이유·주소처럼 런타임에서 받은 문자열을 JSON 문자열 내부에 넣을 수 있게 이스케이프한다.
	// 따옴표 자체는 호출자가 붙이며, 원본 입력을 소유하거나 변경하지 않는다.
	std::string Escape_JsonString(const std::string_view value)
	{
		std::string escaped;
		escaped.reserve(value.size());
		constexpr char HEX[] = "0123456789abcdef";
		for (const unsigned char character : value)
		{
			switch (character)
			{
			case '"':
				escaped += "\\\"";
				break;
			case '\\':
				escaped += "\\\\";
				break;
			case '\b':
				escaped += "\\b";
				break;
			case '\f':
				escaped += "\\f";
				break;
			case '\n':
				escaped += "\\n";
				break;
			case '\r':
				escaped += "\\r";
				break;
			case '\t':
				escaped += "\\t";
				break;
			default:
				if (character < 0x20u)
				{
					escaped += "\\u00";
					escaped.push_back(HEX[(character >> 4u) & 0x0fu]);
					escaped.push_back(HEX[character & 0x0fu]);
				}
				else
					escaped.push_back(static_cast<char>(character));
				break;
			}
		}
		return escaped;
	}

	// 실행 파일 옆 Diagnostics 폴더에 프로세스별 JSONL 경로를 만든다.
	// 모듈 경로나 폴더 생성이 실패하면 빈 경로를 반환해 호출자가 기록 실패를 알린다.
	std::filesystem::path Resolve_ServerSessionDiagnosticPath()
	{
		std::array<wchar_t, 32768u> modulePath{};
		const DWORD length = ::GetModuleFileNameW(nullptr, modulePath.data(), static_cast<DWORD>(modulePath.size()));
		if (0u == length || length >= modulePath.size())
			return {};

		std::error_code error;
		const std::filesystem::path diagnosticDirectory =
		    std::filesystem::path{modulePath.data()}.parent_path() / L"Diagnostics";
		std::filesystem::create_directories(diagnosticDirectory, error);
		if (error)
			return {};
		return diagnosticDirectory / (L"server-session-" + std::to_wstring(::GetCurrentProcessId()) + L".jsonl");
	}

	// 프로세스별 룸 성능 로그는 현재 파일과 이전 파일을 각각 최대 4 MiB로 유지한다.
	bool Append_BoundedRoomPerformanceDiagnostic(const std::filesystem::path& path,
	                                             const std::string_view line) noexcept
	{
		constexpr std::uintmax_t MAX_FILE_BYTES = 4u * 1024u * 1024u;
		try
		{
			if (path.empty() || line.size() >= MAX_FILE_BYTES)
				return false;
			std::error_code error;
			const bool exists = std::filesystem::exists(path, error);
			if (error)
				return false;
			const auto currentBytes = exists ? std::filesystem::file_size(path, error) : 0u;
			if (error)
				return false;
			if (currentBytes > MAX_FILE_BYTES - (line.size() + 1u))
			{
				auto previousPath = path;
				previousPath.replace_extension(L".previous.log");
				std::filesystem::remove(previousPath, error);
				if (error)
					return false;
				std::filesystem::rename(path, previousPath, error);
				if (error)
					return false;
			}
			std::ofstream log{path, std::ios::binary | std::ios::app};
			if (!log)
				return false;
			log << line << '\n';
			log.flush();
			return static_cast<bool>(log);
		}
		catch (...)
		{
			return false; // 진단 I/O 실패는 시뮬레이션 오류로 전파하지 않는다.
		}
	}

	// On_SessionClosed가 잠금별로 복사한 종료 원인·바인딩·송신 통계를 한 행으로 직렬화한다.
	// 입력은 값 스냅샷이며 이 함수가 세션이나 룸 상태를 다시 조회하지 않는다.
	std::string Build_ServerSessionDiagnosticJson(const LostArk::Server::SESSION_ID sessionId,
	                                              const LostArk::Server::CLIENT_SESSION_PEER_ENDPOINT& peer,
	                                              const LostArk::Server::CLIENT_SESSION_CLOSE_DIAGNOSTIC& diagnostic,
	                                              const LostArk::Shared::WORLD_ID worldId,
	                                              const LostArk::Shared::PLAYER_ID playerId, const bool wasBound,
	                                              const bool leaveEnqueued,
	                                              const LostArk::Server::CLIENT_SESSION_OUTBOUND_METRICS& metrics)
	{
		const std::uint64_t occurredAt = 0u == diagnostic.iOccurredUnixMilliseconds
		                                     ? Current_UnixMilliseconds()
		                                     : diagnostic.iOccurredUnixMilliseconds;
		std::ostringstream json;
		json << "{\"schema\":\"lostark.server-session-diagnostic\""
		     << ",\"formatVersion\":1"
		     << ",\"event\":\"connection.closed\""
		     << ",\"processId\":" << ::GetCurrentProcessId()
#ifdef _DEBUG
		     << ",\"buildConfig\":\"Debug\""
#else
		     << ",\"buildConfig\":\"Release\""
#endif
		     << ",\"networkProtocolVersion\":" << static_cast<std::uint16_t>(LostArk::Shared::NETWORK_PROTOCOL_VERSION)
		     << ",\"occurredUnixMs\":" << occurredAt << ",\"sessionId\":" << sessionId << ",\"peerAddress\":\""
		     << Escape_JsonString(peer.strAddress) << "\",\"peerPort\":" << peer.iPort << ",\"reason\":\""
		     << LostArk::Shared::To_SessionDiagnosticReasonName(diagnostic.eReason) << '"'
		     << ",\"nativeErrorCode\":" << diagnostic.iNativeErrorCode << ",\"context\":\""
		     << Escape_JsonString(diagnostic.strContext) << '"'
		     << ",\"lastInboundPacket\":" << static_cast<std::uint16_t>(diagnostic.eLastInboundPacket)
		     << ",\"lastInboundUnixMs\":" << diagnostic.iLastInboundUnixMilliseconds << ",\"lastInboundAgeMs\":"
		     << (0u != diagnostic.iLastInboundUnixMilliseconds && occurredAt >= diagnostic.iLastInboundUnixMilliseconds
		             ? occurredAt - diagnostic.iLastInboundUnixMilliseconds
		             : 0u)
		     << ",\"worldId\":" << static_cast<std::uint16_t>(worldId) << ",\"playerId\":" << playerId
		     << ",\"wasBound\":" << (wasBound ? "true" : "false")
		     << ",\"leaveEnqueued\":" << (leaveEnqueued ? "true" : "false") << ",\"outbound\":{"
		     << "\"queuedFramesAtClose\":" << diagnostic.iQueuedFrameCountAtClose
		     << ",\"queuedBytesAtClose\":" << diagnostic.iQueuedByteCountAtClose
		     << ",\"queuedFramesAfterClose\":" << metrics.iCurrentQueuedFrameCount
		     << ",\"queuedBytesAfterClose\":" << metrics.iCurrentQueuedByteCount
		     << ",\"frameHighWatermark\":" << metrics.iQueuedFrameHighWatermark
		     << ",\"byteHighWatermark\":" << metrics.iQueuedByteHighWatermark
		     << ",\"reliableEnqueued\":" << metrics.iReliableEnqueuedFrameCount
		     << ",\"reliableRejected\":" << metrics.iReliableRejectedFrameCount
		     << ",\"snapshotEnqueued\":" << metrics.iSnapshotEnqueuedFrameCount
		     << ",\"snapshotCoalesced\":" << metrics.iSnapshotCoalescedFrameCount
		     << ",\"snapshotDropped\":" << metrics.iSnapshotDroppedFrameCount
		     << ",\"sentFrames\":" << metrics.iSentFrameCount << ",\"sentBytes\":" << metrics.iSentByteCount
		     << ",\"sendFailures\":" << metrics.iSendFailureCount
		     << ",\"lastSendUs\":" << metrics.iLastFrameSendMicroseconds
		     << ",\"maxSendUs\":" << metrics.iMaximumFrameSendMicroseconds << "}}";
		return json.str();
	}

	// 후보 manifest와 영속화 포인터에 필요한 JSON 종류만 표현한다.
	// 부동소수점은 revision 정규 표현에 쓰지 않으므로 파서에서 거부한다.
	enum class JSON_KIND : std::uint8_t
	{
		OBJECT,
		ARRAY,
		STRING,
		INTEGER,
		BOOLEAN,
		NULL_VALUE
	};

	// 파싱한 문자열과 자식을 값으로 소유한다. 객체 이름과 값은 같은 인덱스가 한 쌍이다.
	// 정수는 uint64 크기와 부호를 따로 보관해 부호 변환에 따른 손실을 피한다.
	struct JSON_VALUE final
	{
		JSON_KIND eKind = JSON_KIND::NULL_VALUE;
		std::string String;
		std::uint64_t Integer = 0u;
		bool isNegative = false;
		bool Boolean = false;
		std::vector<std::string> MemberNames;
		std::vector<JSON_VALUE> MemberValues;
		std::vector<JSON_VALUE> Elements;
	};

	// JSON 문자열·bootstrap 입력의 UTF-8을 검증한다. 잘린 연속 바이트, 과잉 인코딩,
	// surrogate와 Unicode 상한 밖 값을 거부하며 입력 버퍼를 수정하지 않는다.
	bool Is_ValidUtf8(const std::string_view value)
	{
		for (std::size_t index = 0u; index < value.size();)
		{
			const unsigned char lead = static_cast<unsigned char>(value[index]);
			if (lead < 0x80u)
			{
				++index;
				continue;
			}
			std::size_t count = 0u;
			std::uint32_t codePoint = 0u;
			if (0xc2u <= lead && lead <= 0xdfu)
			{
				count = 2u;
				codePoint = lead & 0x1fu;
			}
			else if (0xe0u <= lead && lead <= 0xefu)
			{
				count = 3u;
				codePoint = lead & 0x0fu;
			}
			else if (0xf0u <= lead && lead <= 0xf4u)
			{
				count = 4u;
				codePoint = lead & 0x07u;
			}
			else
				return false;
			if (index + count > value.size())
				return false;
			for (std::size_t offset = 1u; offset < count; ++offset)
			{
				const unsigned char next = static_cast<unsigned char>(value[index + offset]);
				if (0x80u != (next & 0xc0u))
					return false;
				codePoint = (codePoint << 6u) | (next & 0x3fu);
			}
			if ((2u == count && codePoint < 0x80u) || (3u == count && codePoint < 0x800u) ||
			    (4u == count && codePoint < 0x10000u) || (0xd800u <= codePoint && codePoint <= 0xdfffu) ||
			    codePoint > 0x10ffffu)
				return false;
			index += count;
		}
		return true;
	}

	// Parse_String이 검증하고 결합한 Unicode code point를 1~4바이트 UTF-8로 덧붙인다.
	// 유효성 검사는 호출자 몫이며, 이 함수는 출력 문자열 끝에 인코딩만 추가한다.
	void Append_Utf8(const std::uint32_t codePoint, std::string& value)
	{
		if (codePoint <= 0x7fu)
			value.push_back(static_cast<char>(codePoint));
		else if (codePoint <= 0x7ffu)
		{
			value.push_back(static_cast<char>(0xc0u | (codePoint >> 6u)));
			value.push_back(static_cast<char>(0x80u | (codePoint & 0x3fu)));
		}
		else if (codePoint <= 0xffffu)
		{
			value.push_back(static_cast<char>(0xe0u | (codePoint >> 12u)));
			value.push_back(static_cast<char>(0x80u | ((codePoint >> 6u) & 0x3fu)));
			value.push_back(static_cast<char>(0x80u | (codePoint & 0x3fu)));
		}
		else
		{
			value.push_back(static_cast<char>(0xf0u | (codePoint >> 18u)));
			value.push_back(static_cast<char>(0x80u | ((codePoint >> 12u) & 0x3fu)));
			value.push_back(static_cast<char>(0x80u | ((codePoint >> 6u) & 0x3fu)));
			value.push_back(static_cast<char>(0x80u | (codePoint & 0x3fu)));
		}
	}

	// 이 파일의 manifest·영속화 JSON을 제한된 트리로 읽는다. 입력 string_view는 차용하므로
	// Parse가 끝날 때까지 원본 바이트가 살아 있어야 하며 파서는 한 번의 파싱에 사용한다.
	class CBoundedJsonParser final
	{
	public:
		// 입력 소유권을 가져오지 않고 커서와 노드 수를 0에서 시작한다.
		explicit CBoundedJsonParser(const std::string_view source) : m_Source(source)
		{
		}

		// BOM·남는 바이트를 거부해 하나의 완전한 JSON 값만 받는다.
		// 실패 시 value에는 부분 파싱 결과가 남을 수 있어 호출자는 지역 후보를 폐기한다.
		bool Parse(JSON_VALUE& value, std::string& status)
		{
			if (m_Source.size() >= 3u && static_cast<unsigned char>(m_Source[0]) == 0xefu &&
			    static_cast<unsigned char>(m_Source[1]) == 0xbbu && static_cast<unsigned char>(m_Source[2]) == 0xbfu)
			{
				status = "UTF-8 BOM is forbidden in candidate JSON";
				return false;
			}
			Skip_Whitespace();
			if (!Parse_Value(value, 0u, status))
				return false;
			Skip_Whitespace();
			if (m_iCursor != m_Source.size())
			{
				status = "Candidate JSON has trailing bytes";
				return false;
			}
			return true;
		}

	private:
		// Unicode escape의 한 자리를 0~15로 바꾸며, 16진수가 아니면 -1로 거부하게 한다.
		static int Hex_Nibble(const char value)
		{
			if ('0' <= value && value <= '9')
				return value - '0';
			if ('a' <= value && value <= 'f')
				return value - 'a' + 10;
			if ('A' <= value && value <= 'F')
				return value - 'A' + 10;
			return -1;
		}

		// JSON이 허용하는 네 종류 공백만 건너뛰고 다음 토큰 위치에 커서를 둔다.
		void Skip_Whitespace()
		{
			while (m_iCursor < m_Source.size())
			{
				const char value = m_Source[m_iCursor];
				if (' ' != value && '\t' != value && '\r' != value && '\n' != value)
					break;
				++m_iCursor;
			}
		}

		// 중첩 깊이 64와 누적 노드 200000을 넘기기 전에 중단한 뒤 첫 바이트로 분기한다.
		// 자식 파싱이 실패하면 커서와 부분 결과를 되돌리지 않고 상위 호출까지 실패를 전달한다.
		bool Parse_Value(JSON_VALUE& value, const std::size_t depth, std::string& status)
		{
			if (depth > 64u || ++m_iNodeCount > 200000u || m_iCursor >= m_Source.size())
			{
				status = "Candidate JSON exceeds structural limits";
				return false;
			}
			const char lead = m_Source[m_iCursor];
			if ('{' == lead)
				return Parse_Object(value, depth, status);
			if ('[' == lead)
				return Parse_Array(value, depth, status);
			if ('\"' == lead)
			{
				value.eKind = JSON_KIND::STRING;
				return Parse_String(value.String, status);
			}
			if ('-' == lead || ('0' <= lead && lead <= '9'))
				return Parse_Integer(value, status);
			if (m_Source.substr(m_iCursor, 4u) == "true")
			{
				m_iCursor += 4u;
				value.eKind = JSON_KIND::BOOLEAN;
				value.Boolean = true;
				return true;
			}
			if (m_Source.substr(m_iCursor, 5u) == "false")
			{
				m_iCursor += 5u;
				value.eKind = JSON_KIND::BOOLEAN;
				value.Boolean = false;
				return true;
			}
			if (m_Source.substr(m_iCursor, 4u) == "null")
			{
				m_iCursor += 4u;
				value.eKind = JSON_KIND::NULL_VALUE;
				return true;
			}
			status = "Candidate JSON token is invalid";
			return false;
		}

		// 중복 키와 잘못된 구분자를 거부한다. 이름과 값은 자식 하나를 완전히 읽은 뒤
		// 각 vector의 같은 인덱스에 넣어 이후 정확한 필드 집합 검사와 정규 직렬화에 넘긴다.
		bool Parse_Object(JSON_VALUE& value, const std::size_t depth, std::string& status)
		{
			value.eKind = JSON_KIND::OBJECT;
			++m_iCursor;
			Skip_Whitespace();
			if (m_iCursor < m_Source.size() && '}' == m_Source[m_iCursor])
			{
				++m_iCursor;
				return true;
			}
			while (m_iCursor < m_Source.size())
			{
				std::string name;
				if (!Parse_String(name, status))
					return false;
				if (std::find(value.MemberNames.begin(), value.MemberNames.end(), name) != value.MemberNames.end())
				{
					status = "Candidate JSON contains a duplicate object member";
					return false;
				}
				Skip_Whitespace();
				if (m_iCursor >= m_Source.size() || ':' != m_Source[m_iCursor++])
				{
					status = "Candidate JSON object separator is invalid";
					return false;
				}
				Skip_Whitespace();
				JSON_VALUE member;
				if (!Parse_Value(member, depth + 1u, status))
					return false;
				value.MemberNames.push_back(std::move(name));
				value.MemberValues.push_back(std::move(member));
				Skip_Whitespace();
				if (m_iCursor >= m_Source.size())
					break;
				if ('}' == m_Source[m_iCursor])
				{
					++m_iCursor;
					return true;
				}
				if (',' != m_Source[m_iCursor++])
					break;
				Skip_Whitespace();
			}
			status = "Candidate JSON object is unterminated";
			return false;
		}

		// 배열 원소의 순서를 유지해 소유한다. 쉼표·닫는 괄호를 검사하며 자식 실패를 전파한다.
		bool Parse_Array(JSON_VALUE& value, const std::size_t depth, std::string& status)
		{
			value.eKind = JSON_KIND::ARRAY;
			++m_iCursor;
			Skip_Whitespace();
			if (m_iCursor < m_Source.size() && ']' == m_Source[m_iCursor])
			{
				++m_iCursor;
				return true;
			}
			while (m_iCursor < m_Source.size())
			{
				JSON_VALUE element;
				if (!Parse_Value(element, depth + 1u, status))
					return false;
				value.Elements.push_back(std::move(element));
				Skip_Whitespace();
				if (m_iCursor >= m_Source.size())
					break;
				if (']' == m_Source[m_iCursor])
				{
					++m_iCursor;
					return true;
				}
				if (',' != m_Source[m_iCursor++])
					break;
				Skip_Whitespace();
			}
			status = "Candidate JSON array is unterminated";
			return false;
		}

		// 따옴표 내부의 escape를 풀고 surrogate 쌍을 하나의 code point로 결합한다.
		// 닫는 따옴표에서 1 MiB 길이와 UTF-8을 확인하며, 실패 중에도 출력은 변경될 수 있다.
		bool Parse_String(std::string& value, std::string& status)
		{
			if (m_iCursor >= m_Source.size() || '\"' != m_Source[m_iCursor++])
			{
				status = "Candidate JSON string is invalid";
				return false;
			}
			value.clear();
			while (m_iCursor < m_Source.size())
			{
				const unsigned char raw = static_cast<unsigned char>(m_Source[m_iCursor++]);
				if ('\"' == raw)
				{
					if (value.size() > 1024u * 1024u || !Is_ValidUtf8(value))
					{
						status = "Candidate JSON string exceeds UTF-8 limits";
						return false;
					}
					return true;
				}

				// 일반 byte는 제어 문자만 즉시 거부하고 그대로 모은다. 다중 byte UTF-8의 완결성은 닫는 따옴표를 만났을 때 검사한다.
				if ('\\' != raw)
				{
					if (raw < 0x20u)
					{
						status = "Candidate JSON string contains a control byte";
						return false;
					}
					value.push_back(static_cast<char>(raw));
					continue;
				}
				if (m_iCursor >= m_Source.size())
					break;
				const char escape = m_Source[m_iCursor++];
				switch (escape)
				{
				case '\"':
					value.push_back('\"');
					break;
				case '\\':
					value.push_back('\\');
					break;
				case '/':
					value.push_back('/');
					break;
				case 'b':
					value.push_back('\b');
					break;
				case 'f':
					value.push_back('\f');
					break;
				case 'n':
					value.push_back('\n');
					break;
				case 'r':
					value.push_back('\r');
					break;
				case 't':
					value.push_back('\t');
					break;

				// 유니코드 escape의 16비트 값을 읽고 상위 surrogate이면 뒤의 하위 surrogate까지 소비해 하나의 UTF-8 문자로 만든다.
				case 'u':
				{
					if (m_iCursor + 4u > m_Source.size())
					{
						status = "Candidate JSON unicode escape is incomplete";
						return false;
					}
					std::uint32_t codePoint = 0u;
					for (std::size_t index = 0u; index < 4u; ++index)
					{
						const int nibble = Hex_Nibble(m_Source[m_iCursor + index]);
						if (nibble < 0)
						{
							status = "Candidate JSON unicode escape is invalid";
							return false;
						}
						codePoint = (codePoint << 4u) | static_cast<std::uint32_t>(nibble);
					}
					m_iCursor += 4u;
					if (0xd800u <= codePoint && codePoint <= 0xdbffu)
					{
						if (m_iCursor + 6u > m_Source.size() || '\\' != m_Source[m_iCursor] ||
						    'u' != m_Source[m_iCursor + 1u])
						{
							status = "Candidate JSON surrogate pair is incomplete";
							return false;
						}
						std::uint32_t low = 0u;
						for (std::size_t index = 0u; index < 4u; ++index)
						{
							const int nibble = Hex_Nibble(m_Source[m_iCursor + 2u + index]);
							if (nibble < 0)
							{
								status = "Candidate JSON surrogate escape is invalid";
								return false;
							}
							low = (low << 4u) | static_cast<std::uint32_t>(nibble);
						}
						if (low < 0xdc00u || low > 0xdfffu)
						{
							status = "Candidate JSON low surrogate is invalid";
							return false;
						}
						m_iCursor += 6u;
						codePoint = 0x10000u + ((codePoint - 0xd800u) << 10u) + (low - 0xdc00u);
					}
					else if (0xdc00u <= codePoint && codePoint <= 0xdfffu)
					{
						status = "Candidate JSON has an unpaired low surrogate";
						return false;
					}
					Append_Utf8(codePoint, value);
					break;
				}
				default:
					status = "Candidate JSON escape is invalid";
					return false;
				}
			}
			status = "Candidate JSON string is unterminated";
			return false;
		}

		// 부호와 크기를 분리해 읽되 선행 0, -0, 소수·지수 표기와 uint64 범위 초과를 거부한다.
		// 같은 의미의 수가 서로 다른 revision 바이트를 만드는 표현을 이 단계에서 제한한다.
		bool Parse_Integer(JSON_VALUE& value, std::string& status)
		{
			value.isNegative = '-' == m_Source[m_iCursor];
			if (value.isNegative)
				++m_iCursor;
			const std::size_t digits = m_iCursor;
			if (m_iCursor >= m_Source.size() || m_Source[m_iCursor] < '0' || m_Source[m_iCursor] > '9')
			{
				status = "Candidate JSON integer is invalid";
				return false;
			}
			if ('0' == m_Source[m_iCursor] && m_iCursor + 1u < m_Source.size() && '0' <= m_Source[m_iCursor + 1u] &&
			    m_Source[m_iCursor + 1u] <= '9')
			{
				status = "Candidate JSON integer has a leading zero";
				return false;
			}
			while (m_iCursor < m_Source.size() && '0' <= m_Source[m_iCursor] && m_Source[m_iCursor] <= '9')
				++m_iCursor;
			if (m_iCursor < m_Source.size() &&
			    ('.' == m_Source[m_iCursor] || 'e' == m_Source[m_iCursor] || 'E' == m_Source[m_iCursor]))
			{
				status = "Candidate manifest forbids floating-point numbers";
				return false;
			}
			const char* first = m_Source.data() + digits;
			const char* last = m_Source.data() + m_iCursor;
			const auto parsed = std::from_chars(first, last, value.Integer);
			if (parsed.ec != std::errc{} || parsed.ptr != last || (value.isNegative && 0u == value.Integer))
			{
				status = "Candidate JSON integer is out of range";
				return false;
			}
			value.eKind = JSON_KIND::INTEGER;
			return true;
		}

		// 입력은 외부 소유다. 커서·노드 수는 이 파서만 변경하므로 인스턴스를 동시 사용하지 않는다.
		std::string_view m_Source;
		std::size_t m_iCursor = 0u;
		std::size_t m_iNodeCount = 0u;
	};

	// 파싱된 객체에서 이름에 대응하는 값을 차용한다. 반환 포인터는 원본 트리의
	// 수명과 vector 재할당에 종속되며, 객체가 아니거나 이름이 없으면 nullptr이다.
	const JSON_VALUE* Find_Member(const JSON_VALUE& object, const std::string_view name)
	{
		if (JSON_KIND::OBJECT != object.eKind)
			return nullptr;
		for (std::size_t index = 0u; index < object.MemberNames.size(); ++index)
			if (object.MemberNames[index] == name)
				return &object.MemberValues[index];
		return nullptr;
	}

	// manifest 복사본의 revisionId를 비울 때 사용하는 수정 가능한 조회 경로다.
	// 원본 트리의 요소를 가리키므로 호출자가 그 트리를 유지해야 한다.
	JSON_VALUE* Find_Member(JSON_VALUE& object, const std::string_view name)
	{
		return const_cast<JSON_VALUE*>(Find_Member(static_cast<const JSON_VALUE&>(object), name));
	}

	// manifest 버전별 허용 필드와 개수가 모두 일치하는지 검사해 누락·추가 필드를 거부한다.
	bool Has_ExactMembers(const JSON_VALUE& object, const std::initializer_list<std::string_view> expected)
	{
		if (JSON_KIND::OBJECT != object.eKind || object.MemberNames.size() != expected.size())
			return false;
		return std::all_of(expected.begin(), expected.end(),
		                   [&object](const std::string_view name) { return nullptr != Find_Member(object, name); });
	}

	// 필수 문자열의 존재·종류·정확한 값을 함께 검사해 schema와 모드를 판정한다.
	bool Is_String(const JSON_VALUE* value, const std::string_view expected)
	{
		return nullptr != value && JSON_KIND::STRING == value->eKind && value->String == expected;
	}

	// 음수가 아닌 정수인지와 닫힌 구간 [minimum, maximum] 안인지 함께 검사한다.
	bool Is_Integer(const JSON_VALUE* value, const std::uint64_t minimum, const std::uint64_t maximum)
	{
		return nullptr != value && JSON_KIND::INTEGER == value->eKind && !value->isNegative &&
		       minimum <= value->Integer && value->Integer <= maximum;
	}

	// manifest 식별자가 소문자 16진수 64자리인지 검사한다. 실제 파일 내용의 해시는 별도로 대조한다.
	bool Is_LowerSha256(const JSON_VALUE* value)
	{
		return nullptr != value && JSON_KIND::STRING == value->eKind &&
		       value->String.size() == LostArk::Shared::GAMEPLAY_DATA_REVISION_HEX_BYTES &&
		       std::all_of(
		           value->String.begin(), value->String.end(), [](const char character)
		           { return ('0' <= character && character <= '9') || ('a' <= character && character <= 'f'); });
	}

	// manifest 복사본과 identity의 구조를 비교한다. 객체의 키 나열 순서는 무시하고
	// 배열 순서·정수 부호·문자열 값은 유지하므로 단순 텍스트 비교와 구별된다.
	bool Json_Equals(const JSON_VALUE& left, const JSON_VALUE& right)
	{
		if (left.eKind != right.eKind)
			return false;
		switch (left.eKind)
		{
		case JSON_KIND::STRING:
			return left.String == right.String;
		case JSON_KIND::INTEGER:
			return left.Integer == right.Integer && left.isNegative == right.isNegative;
		case JSON_KIND::BOOLEAN:
			return left.Boolean == right.Boolean;
		case JSON_KIND::NULL_VALUE:
			return true;
		case JSON_KIND::ARRAY:
			if (left.Elements.size() != right.Elements.size())
				return false;
			for (std::size_t index = 0u; index < left.Elements.size(); ++index)
				if (!Json_Equals(left.Elements[index], right.Elements[index]))
					return false;
			return true;
		case JSON_KIND::OBJECT:
			if (left.MemberNames.size() != right.MemberNames.size())
				return false;
			for (std::size_t index = 0u; index < left.MemberNames.size(); ++index)
			{
				const JSON_VALUE* other = Find_Member(right, left.MemberNames[index]);
				if (nullptr == other || !Json_Equals(left.MemberValues[index], *other))
					return false;
			}
			return true;
		}
		return false;
	}

	// 정규 JSON 문자열의 따옴표와 escape를 output 뒤에 쓴다.
	// 파싱으로 검증된 문자열을 받아 원래 UTF-8 바이트는 그대로 유지한다.
	void Append_CanonicalString(const std::string& value, std::string& output)
	{
		static constexpr char HEX[] = "0123456789abcdef";
		output.push_back('\"');
		for (const unsigned char character : value)
		{
			switch (character)
			{
			case '\"':
				output += "\\\"";
				break;
			case '\\':
				output += "\\\\";
				break;
			case '\b':
				output += "\\b";
				break;
			case '\f':
				output += "\\f";
				break;
			case '\n':
				output += "\\n";
				break;
			case '\r':
				output += "\\r";
				break;
			case '\t':
				output += "\\t";
				break;
			default:
				if (character < 0x20u)
				{
					output += "\\u00";
					output.push_back(HEX[character >> 4u]);
					output.push_back(HEX[character & 0x0fu]);
				}
				else
					output.push_back(static_cast<char>(character));
				break;
			}
		}
		output.push_back('\"');
	}

	// 객체 키는 이름순, 배열은 원래 순서로 공백 없는 JSON을 덧붙인다.
	// identity 원문이 이 정규 표현과 같은지 확인하고 artifact 집합 해시를 계산할 때 사용한다.
	void Append_CanonicalJson(const JSON_VALUE& value, std::string& output)
	{
		switch (value.eKind)
		{
		case JSON_KIND::STRING:
			Append_CanonicalString(value.String, output);
			break;
		case JSON_KIND::INTEGER:
			if (value.isNegative)
				output.push_back('-');
			output += std::to_string(value.Integer);
			break;
		case JSON_KIND::BOOLEAN:
			output += value.Boolean ? "true" : "false";
			break;
		case JSON_KIND::NULL_VALUE:
			output += "null";
			break;
		case JSON_KIND::ARRAY:
			output.push_back('[');
			for (std::size_t index = 0u; index < value.Elements.size(); ++index)
			{
				if (0u != index)
					output.push_back(',');
				Append_CanonicalJson(value.Elements[index], output);
			}
			output.push_back(']');
			break;
		case JSON_KIND::OBJECT:
		{
			std::vector<std::size_t> order(value.MemberNames.size());
			for (std::size_t index = 0u; index < order.size(); ++index)
				order[index] = index;
			std::sort(order.begin(), order.end(), [&value](const std::size_t left, const std::size_t right)
			          { return value.MemberNames[left] < value.MemberNames[right]; });
			output.push_back('{');
			for (std::size_t ordinal = 0u; ordinal < order.size(); ++ordinal)
			{
				if (0u != ordinal)
					output.push_back(',');
				const std::size_t index = order[ordinal];
				Append_CanonicalString(value.MemberNames[index], output);
				output.push_back(':');
				Append_CanonicalJson(value.MemberValues[index], output);
			}
			output.push_back('}');
			break;
		}
		}
	}

	// 차용한 메모리 바이트를 CNG SHA-256으로 계산한다. CNG 호출 실패도 handle 정리로 모으며
	// 완성한 해시만 revision에 넘기므로 false를 반환하는 경로는 기존 revision을 유지한다.
	bool Hash_BytesSha256(const std::span<const std::uint8_t> bytes, GameplayDataRevision& revision)
	{
		BCRYPT_ALG_HANDLE algorithm = nullptr;
		BCRYPT_HASH_HANDLE hash = nullptr;
		DWORD objectBytes = 0u;
		DWORD resultBytes = 0u;
		std::vector<std::uint8_t> object;
		GameplayDataRevision staged{};
		bool succeeded = false;
		if (0 <= BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0u) &&
		    0 <= BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&objectBytes),
		                           sizeof(objectBytes), &resultBytes, 0u))
		{
			object.resize(objectBytes);
			if (0 <= BCryptCreateHash(algorithm, &hash, object.data(), objectBytes, nullptr, 0u, 0u))
			{
				std::size_t offset = 0u;
				succeeded = true;
				while (offset < bytes.size())
				{
					const ULONG chunk =
					    static_cast<ULONG>((std::min)(bytes.size() - offset,
					                                  static_cast<std::size_t>((std::numeric_limits<ULONG>::max)())));
					if (0 > BCryptHashData(hash, const_cast<PUCHAR>(bytes.data() + offset), chunk, 0u))
					{
						succeeded = false;
						break;
					}
					offset += chunk;
				}
				if (succeeded &&
				    0 > BCryptFinishHash(hash, staged.Bytes.data(), static_cast<ULONG>(staged.Bytes.size()), 0u))
					succeeded = false;
			}
		}
		if (nullptr != hash)
			BCryptDestroyHash(hash);
		if (nullptr != algorithm)
			BCryptCloseAlgorithmProvider(algorithm, 0u);
		if (!succeeded || !staged.Is_Valid())
			return false;
		revision = staged;
		return true;
	}

	// 선언한 상한 이하인 파일 내용을 bytes에 읽는다. 크기 확인 뒤 resize하므로
	// 읽기 실패 시 bytes가 변경될 수 있으며, 호출자는 실패한 후보를 사용하지 않는다.
	bool Read_BoundedFile(const std::filesystem::path& path, const std::uintmax_t maximumBytes,
	                      std::vector<std::uint8_t>& bytes, std::string& status)
	{
		std::error_code error;
		const std::uintmax_t size = std::filesystem::file_size(path, error);
		if (error || size > maximumBytes ||
		    size > static_cast<std::uintmax_t>((std::numeric_limits<std::size_t>::max)()))
		{
			status = "Candidate artifact exceeds its bounded file size";
			return false;
		}
		std::ifstream stream(path, std::ios::binary);
		if (!stream)
		{
			status = "Candidate artifact could not be opened";
			return false;
		}
		bytes.resize(static_cast<std::size_t>(size));
		if (!bytes.empty())
			stream.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
		if (!stream || stream.peek() != std::ifstream::traits_type::eof())
		{
			status = "Candidate artifact read was incomplete";
			return false;
		}
		return true;
	}

	// 후보 artifact를 청크로 읽어 실제 파일 SHA-256을 계산한다. 읽기·CNG 실패는
	// handle 정리 뒤 false로 모으며 완성한 해시만 revision에 기록한다.
	bool Hash_FileSha256(const std::filesystem::path& path, GameplayDataRevision& revision, std::string& status)
	{
		std::ifstream stream(path, std::ios::binary);
		if (!stream)
		{
			status = "Candidate artifact could not be opened for hashing";
			return false;
		}
		BCRYPT_ALG_HANDLE algorithm = nullptr;
		BCRYPT_HASH_HANDLE hash = nullptr;
		DWORD objectBytes = 0u;
		DWORD resultBytes = 0u;
		std::vector<std::uint8_t> object;
		GameplayDataRevision staged{};
		bool succeeded = false;
		if (0 <= BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0u) &&
		    0 <= BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&objectBytes),
		                           sizeof(objectBytes), &resultBytes, 0u))
		{
			object.resize(objectBytes);
			succeeded = 0 <= BCryptCreateHash(algorithm, &hash, object.data(), objectBytes, nullptr, 0u, 0u);
		}
		// Server.exe의 기본 스택 예약은 1 MiB다. 같은 크기의 지역 배열은 함수 진입 시
		// __chkstk에서 실패할 수 있으므로 1 MiB 청크 크기는 유지하면서 저장소는 heap에 둔다.
		std::vector<std::uint8_t> buffer(1024u * 1024u);
		while (succeeded && stream)
		{
			stream.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
			const std::streamsize count = stream.gcount();
			if (count > 0 && 0 > BCryptHashData(hash, buffer.data(), static_cast<ULONG>(count), 0u))
				succeeded = false;
		}
		if (!stream.eof())
			succeeded = false;
		if (succeeded && 0 > BCryptFinishHash(hash, staged.Bytes.data(), static_cast<ULONG>(staged.Bytes.size()), 0u))
			succeeded = false;
		if (nullptr != hash)
			BCryptDestroyHash(hash);
		if (nullptr != algorithm)
			BCryptCloseAlgorithmProvider(algorithm, 0u);
		if (!succeeded || !staged.Is_Valid())
		{
			status = "Candidate artifact SHA-256 failed";
			return false;
		}
		revision = staged;
		return true;
	}

	// candidate 내부 상대 경로 계약을 검사한다. 절대·drive 경로, 역슬래시와
	// 빈 경로 요소, . 또는 ..를 거부하며 실제 파일 존재 검사는 다음 단계에서 수행한다.
	bool Is_SafeRelativePath(const std::string& relative)
	{
		if (relative.empty() || '/' == relative.front() || relative.find('\\') != std::string::npos ||
		    relative.find(':') != std::string::npos)
			return false;
		std::size_t cursor = 0u;
		while (cursor <= relative.size())
		{
			const std::size_t slash = relative.find('/', cursor);
			const std::string_view segment(relative.data() + cursor,
			                               (std::string::npos == slash ? relative.size() : slash) - cursor);
			if (segment.empty() || "." == segment || ".." == segment)
				return false;
			if (std::string::npos == slash)
				break;
			cursor = slash + 1u;
		}
		return true;
	}

	// 허용된 상대 경로를 후보 폴더 아래의 정규 일반 파일로 확정한다. 링크·경로 이탈을
	// 거부하고 성공한 경로만 path에 넘겨 manifest 밖 파일을 읽지 않게 한다.
	bool Resolve_ExactRegularFile(const std::filesystem::path& candidateDirectory, const std::string& relative,
	                              std::filesystem::path& path, std::string& status)
	{
		if (!Is_SafeRelativePath(relative))
		{
			status = "Candidate artifact relative path is invalid";
			return false;
		}
		std::error_code error;
		const std::filesystem::path relativePath{relative};
		if (relativePath.is_absolute() || relativePath.has_root_name() || relativePath.has_root_directory())
		{
			status = "Candidate artifact path is drive-qualified or absolute";
			return false;
		}
		const std::filesystem::path expected = (candidateDirectory / relativePath).lexically_normal();
		const std::filesystem::file_status linkStatus = std::filesystem::symlink_status(expected, error);
		if (error || !std::filesystem::is_regular_file(linkStatus) || std::filesystem::is_symlink(linkStatus))
		{
			status = "Candidate artifact is not a regular non-symlink file";
			return false;
		}
		const std::filesystem::path canonical = std::filesystem::canonical(expected, error);
		const std::filesystem::path relativeCanonical = canonical.lexically_relative(candidateDirectory);
		if (error || canonical != expected || relativeCanonical.empty() || relativeCanonical.is_absolute() ||
		    relativeCanonical.has_root_name() || relativeCanonical.has_root_directory() ||
		    relativeCanonical.begin() == relativeCanonical.end() || *relativeCanonical.begin() == L"..")
		{
			status = "Candidate artifact escaped its canonical revision directory";
			return false;
		}
		path = canonical;
		return true;
	}

	// 환경 변수, EXE 위치, 현재 디렉터리를 시작점으로 각 10단계 이내 부모를 찾는다.
	// Data와 Tools/ValtanPipeline이 함께 있는 위치를 후보 저장소 루트로 사용한다.
	std::filesystem::path Resolve_RepositoryRoot()
	{
		std::vector<std::filesystem::path> starts;
		std::vector<wchar_t> pathBuffer(32768u);
		const DWORD configuredLength = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", pathBuffer.data(),
		                                                       static_cast<DWORD>(pathBuffer.size()));
		if (0u != configuredLength && configuredLength < pathBuffer.size())
			starts.emplace_back(pathBuffer.data());
		const DWORD moduleLength =
		    GetModuleFileNameW(nullptr, pathBuffer.data(), static_cast<DWORD>(pathBuffer.size()));
		if (0u != moduleLength && moduleLength < pathBuffer.size())
			starts.emplace_back(std::filesystem::path(pathBuffer.data()).parent_path());
		std::error_code error;
		starts.push_back(std::filesystem::current_path(error));
		for (std::filesystem::path start : starts)
		{
			for (std::size_t depth = 0u; depth < 10u && !start.empty(); ++depth)
			{
				error.clear();
				if (std::filesystem::is_directory(start / L"Data", error) &&
				    std::filesystem::is_directory(start / L"Tools" / L"ValtanPipeline", error))
					return std::filesystem::canonical(start, error);
				const std::filesystem::path parent = start.parent_path();
				if (parent == start)
					break;
				start = parent;
			}
		}
		return {};
	}

	// LOSTARK_SERVER_DATA_ROOT를 우선하고 없으면 EXE 상위의 DataFiles를 사용한다.
	// Gameplay.bootstrap이 정확한 정규 일반 파일일 때만 경로를 돌려준다.
	std::filesystem::path Resolve_RuntimeGameplayBootstrap()
	{
		namespace fs = std::filesystem;
		std::vector<wchar_t> pathBuffer(32768u);
		fs::path dataRoot;
		const DWORD configuredLength = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", pathBuffer.data(),
		                                                       static_cast<DWORD>(pathBuffer.size()));
		if (0u != configuredLength && configuredLength < pathBuffer.size())
		{
			dataRoot = fs::path(pathBuffer.data()).lexically_normal();
		}
		else
		{
			const DWORD moduleLength =
			    GetModuleFileNameW(nullptr, pathBuffer.data(), static_cast<DWORD>(pathBuffer.size()));
			if (0u == moduleLength || moduleLength >= pathBuffer.size())
				return {};
			dataRoot = fs::path(pathBuffer.data()).parent_path().parent_path() / L"DataFiles";
		}
		const fs::path expected = (dataRoot / L"Gameplay" / L"Gameplay.bootstrap").lexically_normal();
		std::error_code error;
		const fs::file_status linkStatus = fs::symlink_status(expected, error);
		const fs::path canonical = fs::canonical(expected, error);
		if (error || !fs::is_regular_file(linkStatus) || fs::is_symlink(linkStatus) || canonical != expected)
		{
			return {};
		}
		return canonical;
	}

	// bootstrap 한 행을 복사 없이 필드로 나눈다. 반환 view들은 line의 원본 버퍼를
	// 차용하므로 이 행의 검사가 끝날 때까지 원본 내용을 유지해야 한다.
	std::vector<std::string_view> Split_Tabs(const std::string_view line)
	{
		std::vector<std::string_view> fields;
		std::size_t start = 0u;
		while (true)
		{
			const std::size_t tab = line.find('\t', start);
			fields.push_back(line.substr(start, std::string_view::npos == tab ? tab : tab - start));
			if (std::string_view::npos == tab)
				break;
			start = tab + 1u;
		}
		return fields;
	}

	// 발탄 전용 boss·damage·encounter 행만 식별해 비발탄 해시에서 제외한다.
	// 종류와 owner를 함께 검사하므로 다른 encounter의 같은 종류 행은 그대로 비교한다.
	bool Is_ValtanOwnedGameplayRow(const std::vector<std::string_view>& fields)
	{
		if (fields.size() < 2u)
			return false;
		const std::string_view kind = fields[0];
		const std::string_view owner = fields[1];
		if (("BOSS" == kind || "BOSSARMOR" == kind || "BOSSPART" == kind) && "BOSS_VALTAN" == owner)
		{
			return true;
		}
		if ("DAMAGE" == kind)
			return owner.starts_with("damage.valtan.");
		const bool encounterOwned = "ENCOUNTERINTRO" == kind || "BOSSCOMBATOBJECT" == kind ||
		                            "BOSSCOMBATOBJECTHIT" == kind || kind.starts_with("PATTERN") ||
		                            kind.starts_with("VALTANTIMELINE");
		return encounterOwned && "ENCOUNTER_VALTAN" == owner;
	}

	// bootstrap 버전·전체 행 수를 검증하고 발탄 전용 행을 뺀 내용을 LF로 정규화해 해시한다.
	// 발탄 조정 후보가 다른 게임플레이를 바꾸지 않았는지 비교하는 값이며 시작 시 revision을 비운다.
	bool Build_NonValtanGameplayRevisionBytes(const std::string_view content, GameplayDataRevision& revision,
	                                          std::string& status)
	{
		revision = {};

		if (!Is_ValidUtf8(content) || std::string_view::npos != content.find('\0'))
		{
			status = "Gameplay bootstrap is not bounded valid UTF-8 text";
			return false;
		}
		std::string normalized;
		normalized.reserve(content.size());
		std::uint64_t declaredRowCount = 0u;
		std::uint64_t actualRowCount = 0u;
		bool hasHeader = false;
		std::size_t start = 0u;
		while (start < content.size())
		{
			const std::size_t newline = content.find('\n', start);
			std::string_view line =
			    content.substr(start, std::string_view::npos == newline ? newline : newline - start);
			if (!line.empty() && '\r' == line.back())
				line.remove_suffix(1u);
			if (line.empty())
			{
				status = "Gameplay bootstrap contains an empty row";
				return false;
			}
			const std::vector<std::string_view> fields = Split_Tabs(line);
			if (!hasHeader)
			{
				std::uint32_t version = 0u;
				const auto parsedVersion =
				    fields.size() < 2u
				        ? std::from_chars(line.data(), line.data(), version)
				        : std::from_chars(fields[1].data(), fields[1].data() + fields[1].size(), version);
				const auto parsedCount =
				    fields.size() < 3u
				        ? std::from_chars(line.data(), line.data(), declaredRowCount)
				        : std::from_chars(fields[2].data(), fields[2].data() + fields[2].size(), declaredRowCount);
				if (3u != fields.size() || "LOSTARK_GAMEPLAY_BOOTSTRAP" != fields[0] ||
				    std::errc{} != parsedVersion.ec || parsedVersion.ptr != fields[1].data() + fields[1].size() ||
				    LostArk::Server::GAMEPLAY_BOOTSTRAP_VERSION != version || std::errc{} != parsedCount.ec ||
				    parsedCount.ptr != fields[2].data() + fields[2].size() || !declaredRowCount ||
				    declaredRowCount > LostArk::Shared::GAMEPLAY_BOOTSTRAP_MAX_ROWS)
				{
					status = "Gameplay bootstrap header is invalid for domain hashing";
					return false;
				}
				normalized.append("LOSTARK_NON_VALTAN_GAMEPLAY\t");
				normalized.append(fields[1]);
				normalized.push_back('\n');
				hasHeader = true;
			}
			else
			{
				++actualRowCount;
				if (!Is_ValtanOwnedGameplayRow(fields))
				{
					normalized.append(line);
					normalized.push_back('\n');
				}
			}
			if (std::string_view::npos == newline)
				break;
			start = newline + 1u;
		}
		if (!hasHeader || actualRowCount != declaredRowCount ||
		    !Hash_BytesSha256(std::span<const std::uint8_t>(reinterpret_cast<const std::uint8_t*>(normalized.data()),
		                                                    normalized.size()),
		                      revision))
		{
			status = "Gameplay bootstrap row count or non-Valtan hash is invalid";
			return false;
		}
		status.clear();
		return true;
	}

	// 파일 크기를 제한해 읽은 후 동일한 비발탄 행 해시 경로로 넘긴다.
	// 파일 읽기 실패는 바이트 파싱 전에 반환하고 status에 원인을 남긴다.
	bool Build_NonValtanGameplayRevision(const std::filesystem::path& path, GameplayDataRevision& revision,
	                                     std::string& status)
	{
		std::vector<std::uint8_t> bytes;
		if (!Read_BoundedFile(path, LostArk::Shared::GAMEPLAY_BOOTSTRAP_MAX_BYTES, bytes, status))
			return false;
		return Build_NonValtanGameplayRevisionBytes(
		    std::string_view(reinterpret_cast<const char*>(bytes.data()), bytes.size()), revision, status);
	}

	// 호출 중 살아 있는 bytes를 파서에 차용해 JSON 트리로 읽는다.
	// 반환된 트리는 문자열을 자체 소유하므로 성공 후 원본 bytes와 별개로 사용할 수 있다.
	bool Parse_JsonBytes(const std::vector<std::uint8_t>& bytes, JSON_VALUE& value, std::string& status)
	{
		return CBoundedJsonParser(std::string_view(reinterpret_cast<const char*>(bytes.data()), bytes.size()))
		    .Parse(value, status);
	}

	// presentation lane처럼 순서까지 계약인 문자열 배열을 예상 개수·값과 대조한다.
	bool Validate_ExactStringArray(const JSON_VALUE* value, const std::initializer_list<std::string_view> expected)
	{
		if (nullptr == value || JSON_KIND::ARRAY != value->eKind || value->Elements.size() != expected.size())
			return false;
		std::size_t index = 0u;
		for (const std::string_view item : expected)
			if (!Is_String(&value->Elements[index++], item))
				return false;
		return true;
	}

	// 수신 요청 또는 시작 복구가 지정한 발탄 후보를 디스크에서 검증해 불변 catalog로 준비한다.
	// manifest·artifact 해시·presentation·비발탄 동일성을 모두 확인하며 런타임 활성 상태는 바꾸지 않는다.
	// candidate와 두 revision 출력을 먼저 비우고, catalog는 끝까지 검증한 뒤에만 넘긴다.
	bool Admit_ValtanCandidateGeneration(const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& request,
	                                     const GameplayDataRevision& activeNonValtanGameplayRevision,
	                                     std::shared_ptr<const LostArk::Server::CGameplayCatalog>& candidate,
	                                     GameplayDataRevision& candidateBootstrapContentRevision,
	                                     GameplayDataRevision& candidateNonValtanGameplayRevision, std::string& status)
	{
		using namespace LostArk::Shared;
		namespace fs = std::filesystem;
		candidate.reset();
		candidateBootstrapContentRevision = {};
		candidateNonValtanGameplayRevision = {};
		if (request.iRequiredPresentationLaneMask != GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK ||
		    !activeNonValtanGameplayRevision.Is_Valid())
		{
			status = "Candidate request or active non-Valtan identity is invalid";
			return false;
		}

		// 요청의 논리 revision을 고정 후보 폴더 이름으로 해석한다.
		// 사용자가 전달한 임의 경로를 admission의 시작점으로 사용하지 않는다.
		const std::string revisionHex = Format_GameplayDataRevision(request.CandidateRevision);
		const fs::path repositoryRoot = Resolve_RepositoryRoot();
		if (repositoryRoot.empty() || revisionHex.empty())
		{
			status = "Repository or candidate revision identity is unavailable";
			return false;
		}
		const fs::path expectedDirectory =
		    (repositoryRoot / L"Intermediate" / L"ValtanTuningCandidates" / L"revisions" / fs::path(revisionHex))
		        .lexically_normal();
		std::error_code error;
		const fs::file_status directoryStatus = fs::symlink_status(expectedDirectory, error);
		const fs::path candidateDirectory = fs::canonical(expectedDirectory, error);
		if (error || !fs::is_directory(directoryStatus) || fs::is_symlink(directoryStatus) ||
		    candidateDirectory != expectedDirectory)
		{
			status = "Fixed candidate revision directory is unavailable or non-canonical";
			return false;
		}

		// manifest와 identity를 제한된 크기로 읽고 identity 원문 해시를 요청 revision과 대조한다.
		fs::path manifestPath;
		fs::path identityPath;
		std::vector<std::uint8_t> manifestBytes;
		std::vector<std::uint8_t> identityBytes;
		if (!Resolve_ExactRegularFile(candidateDirectory, "revision-manifest.json", manifestPath, status) ||
		    !Resolve_ExactRegularFile(candidateDirectory, "revision-identity.json", identityPath, status) ||
		    !Read_BoundedFile(manifestPath, 8u * 1024u * 1024u, manifestBytes, status) ||
		    !Read_BoundedFile(identityPath, 8u * 1024u * 1024u, identityBytes, status))
			return false;
		GameplayDataRevision identityRevision{};
		if (!Hash_BytesSha256(identityBytes, identityRevision) || identityRevision != request.CandidateRevision)
		{
			status = "Raw revision identity SHA-256 does not match the request";
			return false;
		}

		// 파싱 구조와 정규 바이트를 모두 확인한다. manifest의 revisionId만 비운 복사본이
		// identity와 구조적으로 같아야 자기 참조 hash 필드와 내용 사이의 계약이 맞는다.
		JSON_VALUE manifest;
		JSON_VALUE identity;
		if (!Parse_JsonBytes(manifestBytes, manifest, status) || !Parse_JsonBytes(identityBytes, identity, status))
			return false;
		std::string canonicalIdentity;
		canonicalIdentity.reserve(identityBytes.size());
		Append_CanonicalJson(identity, canonicalIdentity);
		if (canonicalIdentity.size() != identityBytes.size() ||
		    !std::equal(canonicalIdentity.begin(), canonicalIdentity.end(), identityBytes.begin()))
		{
			status = "Revision identity bytes are not canonical JSON";
			return false;
		}
		if (!Has_ExactMembers(manifest,
		                      {"schema", "formatVersion", "revisionId", "revisionIdentity", "sourceManifestId",
		                       "authoringBaseRevision", "artifactSetId", "draftPatchOperationCount", "allowedDomains",
		                       "requiredPresentationLanes", "clientPresentationCompatibility",
		                       "serverGameplayBootstrap", "applyClass", "runtimeActivation", "serverSubmanifestSha256",
		                       "clientSubmanifestSha256", "authoringSubmanifestSha256", "artifacts"}))
		{
			status = "Candidate revision manifest fields are not exact";
			return false;
		}
		JSON_VALUE manifestIdentity = manifest;
		JSON_VALUE* blankRevision = Find_Member(manifestIdentity, "revisionId");
		if (nullptr == blankRevision || JSON_KIND::STRING != blankRevision->eKind)
		{
			status = "Candidate revisionId is not a string";
			return false;
		}
		blankRevision->String.clear();
		if (!Json_Equals(manifestIdentity, identity))
		{
			status = "Manifest and canonical parent identity differ";
			return false;
		}
		if (!Is_String(Find_Member(manifest, "schema"), "lostark.valtan-tuning-revision-manifest") ||
		    !Is_Integer(Find_Member(manifest, "formatVersion"), 1u, 1u) ||
		    !Is_String(Find_Member(manifest, "revisionId"), revisionHex) ||
		    !Is_String(Find_Member(manifest, "applyClass"), "HOT_RELOAD") ||
		    !Is_String(Find_Member(manifest, "runtimeActivation"), "SERVER_2PC_TICK_BOUNDARY") ||
		    !Validate_ExactStringArray(Find_Member(manifest, "allowedDomains"), {"VALTAN_BOSS"}) ||
		    !Validate_ExactStringArray(Find_Member(manifest, "requiredPresentationLanes"),
		                               {"ANIMATION", "EFFECT", "COMBAT_VISUAL", "CAMERA", "WORLD_EVENT_SET"}) ||
		    !Is_Integer(Find_Member(manifest, "draftPatchOperationCount"), 0u, 100000u))
		{
			status = "Candidate HOT_RELOAD activation contract is invalid";
			return false;
		}
		for (const std::string_view field :
		     {"revisionId", "sourceManifestId", "authoringBaseRevision", "artifactSetId", "serverSubmanifestSha256",
		      "clientSubmanifestSha256", "authoringSubmanifestSha256"})
		{
			if (!Is_LowerSha256(Find_Member(manifest, field)))
			{
				status = "Candidate manifest contains a malformed SHA-256 field";
				return false;
			}
		}

		const JSON_VALUE* revisionIdentity = Find_Member(manifest, "revisionIdentity");
		if (nullptr == revisionIdentity ||
		    !Has_ExactMembers(*revisionIdentity,
		                      {"kind", "algorithm", "identityPayloadPath", "serverBootstrapContentRevision"}) ||
		    !Is_String(Find_Member(*revisionIdentity, "kind"), "PARENT_MANIFEST") ||
		    !Is_String(Find_Member(*revisionIdentity, "algorithm"), "SHA256_CANONICAL_JSON_WITH_EMPTY_REVISION_ID") ||
		    !Is_String(Find_Member(*revisionIdentity, "identityPayloadPath"), "revision-identity.json") ||
		    !Is_LowerSha256(Find_Member(*revisionIdentity, "serverBootstrapContentRevision")))
		{
			status = "Candidate parent revision identity contract is invalid";
			return false;
		}

		// 클라이언트 presentation은 허용한 다섯 lane의 byte-identical alias만 받아들인다.
		// 서버 gameplay 변경과 별개로 표현 세대 식별자와 lane 전체 포함 여부를 검증한다.
		const JSON_VALUE* compatibility = Find_Member(manifest, "clientPresentationCompatibility");
		if (nullptr == compatibility ||
		    !Has_ExactMembers(*compatibility, {"mode", "presentationGenerationId", "requiredLanes", "artifacts"}) ||
		    !Is_String(Find_Member(*compatibility, "mode"), "BYTE_IDENTICAL_TO_ACTIVE") ||
		    !Is_LowerSha256(Find_Member(*compatibility, "presentationGenerationId")) ||
		    !Validate_ExactStringArray(Find_Member(*compatibility, "requiredLanes"),
		                               {"ANIMATION", "EFFECT", "COMBAT_VISUAL", "CAMERA", "WORLD_EVENT_SET"}))
		{
			status = "Candidate presentation alias contract is invalid";
			return false;
		}
		const JSON_VALUE* aliasArtifacts = Find_Member(*compatibility, "artifacts");
		const std::string presentationGenerationId = Find_Member(*compatibility, "presentationGenerationId")->String;
		std::set<std::string> coveredLanes;
		if (nullptr == aliasArtifacts || JSON_KIND::ARRAY != aliasArtifacts->eKind)
		{
			status = "Candidate presentation alias artifact list is invalid";
			return false;
		}
		for (const JSON_VALUE& row : aliasArtifacts->Elements)
		{
			if (!Has_ExactMembers(row, {"lane", "path", "sha256", "bytes", "repositorySourceSha256"}))
			{
				status = "Candidate presentation alias row fields are invalid";
				return false;
			}
			const JSON_VALUE* lane = Find_Member(row, "lane");
			const JSON_VALUE* path = Find_Member(row, "path");
			const JSON_VALUE* hash = Find_Member(row, "sha256");
			const JSON_VALUE* sourceHash = Find_Member(row, "repositorySourceSha256");
			if (nullptr == lane || JSON_KIND::STRING != lane->eKind || nullptr == path ||
			    JSON_KIND::STRING != path->eKind || !Is_SafeRelativePath(path->String) || !Is_LowerSha256(hash) ||
			    !Is_LowerSha256(sourceHash) || hash->String != sourceHash->String ||
			    !Is_Integer(Find_Member(row, "bytes"), 0u, (std::numeric_limits<std::uint64_t>::max)()))
			{
				status = "Candidate presentation alias identity is invalid";
				return false;
			}
			if (lane->String != "ANIMATION" && lane->String != "EFFECT" && lane->String != "COMBAT_VISUAL" &&
			    lane->String != "CAMERA" && lane->String != "WORLD_EVENT_SET")
			{
				status = "Candidate presentation alias lane is unknown";
				return false;
			}
			coveredLanes.insert(lane->String);
		}
		if (coveredLanes != std::set<std::string>{"ANIMATION", "EFFECT", "COMBAT_VISUAL", "CAMERA", "WORLD_EVENT_SET"})
		{
			status = "Candidate presentation alias lane coverage is incomplete";
			return false;
		}

		// 서버 bootstrap의 버전·행 수·원문 해시가 parent identity와 같은 값을 가리키는지 확인한다.
		const JSON_VALUE* bootstrap = Find_Member(manifest, "serverGameplayBootstrap");
		if (nullptr == bootstrap ||
		    !Has_ExactMembers(*bootstrap,
		                      {"path", "formatVersion", "baselineRowCount", "candidateRowCount", "removedValtanRows",
		                       "addedValtanRows", "baselineSha256", "candidateSha256"}) ||
		    !Is_String(Find_Member(*bootstrap, "path"), "Runtime/Gameplay/Gameplay.bootstrap") ||
		    !Is_Integer(Find_Member(*bootstrap, "formatVersion"), LostArk::Server::GAMEPLAY_BOOTSTRAP_VERSION,
		                LostArk::Server::GAMEPLAY_BOOTSTRAP_VERSION) ||
		    !Is_LowerSha256(Find_Member(*bootstrap, "baselineSha256")) ||
		    !Is_LowerSha256(Find_Member(*bootstrap, "candidateSha256")) ||
		    Find_Member(*bootstrap, "candidateSha256")->String !=
		        Find_Member(*revisionIdentity, "serverBootstrapContentRevision")->String)
		{
			status = "Candidate Server gameplay bootstrap contract is invalid";
			return false;
		}
		for (const std::string_view field :
		     {"baselineRowCount", "candidateRowCount", "removedValtanRows", "addedValtanRows"})
		{
			if (!Is_Integer(Find_Member(*bootstrap, field), 0u, 4096u))
			{
				status = "Candidate Server gameplay row count is invalid";
				return false;
			}
		}
		GameplayDataRevision manifestBaselineRevision{};
		if (!Try_Parse_GameplayDataRevision(Find_Member(*bootstrap, "baselineSha256")->String,
		                                    manifestBaselineRevision))
		{
			status = "Candidate bootstrap baseline identity is malformed";
			return false;
		}

		// artifact 목록의 중복 경로를 거부하고 각각의 실제 크기·해시를 읽어 확인한다.
		// 목록 전체의 정규 JSON 해시와 세 종류 submanifest 해시도 이어서 대조한다.
		const JSON_VALUE* artifacts = Find_Member(manifest, "artifacts");
		if (nullptr == artifacts || JSON_KIND::ARRAY != artifacts->eKind || artifacts->Elements.empty())
		{
			status = "Candidate artifact manifest is empty";
			return false;
		}
		std::set<std::string> artifactPaths;
		const std::string presentationGenerationManifestRelative =
		    "Runtime/Gameplay/ValtanPresentationGenerations/" + presentationGenerationId + ".json";
		bool hasPresentationGenerationManifest = false;
		for (const JSON_VALUE& row : artifacts->Elements)
		{
			if (!Has_ExactMembers(row, {"path", "sha256", "bytes"}))
			{
				status = "Candidate artifact row fields are invalid";
				return false;
			}
			const JSON_VALUE* relative = Find_Member(row, "path");
			const JSON_VALUE* hash = Find_Member(row, "sha256");
			const JSON_VALUE* bytes = Find_Member(row, "bytes");
			if (nullptr == relative || JSON_KIND::STRING != relative->eKind || !Is_SafeRelativePath(relative->String) ||
			    relative->String == "revision-manifest.json" || relative->String == "revision-identity.json" ||
			    !artifactPaths.insert(relative->String).second || !Is_LowerSha256(hash) ||
			    !Is_Integer(bytes, 0u, (std::numeric_limits<std::uint64_t>::max)()))
			{
				status = "Candidate artifact identity is invalid";
				return false;
			}
			if (relative->String == presentationGenerationManifestRelative)
			{
				if (hash->String != presentationGenerationId)
				{
					status = "Candidate presentation generation manifest identity is invalid";
					return false;
				}
				hasPresentationGenerationManifest = true;
			}
			fs::path artifactPath;
			GameplayDataRevision artifactRevision{};
			error.clear();
			if (!Resolve_ExactRegularFile(candidateDirectory, relative->String, artifactPath, status) ||
			    fs::file_size(artifactPath, error) != bytes->Integer || error ||
			    !Hash_FileSha256(artifactPath, artifactRevision, status) ||
			    Format_GameplayDataRevision(artifactRevision) != hash->String)
			{
				if (status.empty())
					status = "Candidate artifact hash or byte count is invalid";
				return false;
			}
		}
		if (!hasPresentationGenerationManifest)
		{
			status = "Candidate artifact set omits its presentation generation manifest";
			return false;
		}
		std::string canonicalArtifacts;
		Append_CanonicalJson(*artifacts, canonicalArtifacts);
		GameplayDataRevision artifactSetRevision{};
		if (!Hash_BytesSha256(
		        std::span<const std::uint8_t>(reinterpret_cast<const std::uint8_t*>(canonicalArtifacts.data()),
		                                      canonicalArtifacts.size()),
		        artifactSetRevision) ||
		    Format_GameplayDataRevision(artifactSetRevision) != Find_Member(manifest, "artifactSetId")->String)
		{
			status = "Candidate artifactSetId is invalid";
			return false;
		}
		for (const auto& pair :
		     {std::pair<std::string_view, std::string_view>{"serverSubmanifestSha256", "_manifest/server.json"},
		      std::pair<std::string_view, std::string_view>{"clientSubmanifestSha256", "_manifest/client.json"},
		      std::pair<std::string_view, std::string_view>{"authoringSubmanifestSha256", "_manifest/authoring.json"}})
		{
			fs::path submanifestPath;
			GameplayDataRevision submanifestRevision{};
			if (!Resolve_ExactRegularFile(candidateDirectory, std::string(pair.second), submanifestPath, status) ||
			    !Hash_FileSha256(submanifestPath, submanifestRevision, status) ||
			    Format_GameplayDataRevision(submanifestRevision) != Find_Member(manifest, pair.first)->String)
			{
				status = "Candidate submanifest SHA-256 is invalid";
				return false;
			}
		}

		const std::string bootstrapRelative = Find_Member(*bootstrap, "path")->String;
		fs::path bootstrapPath;
		GameplayDataRevision bootstrapRevision{};
		error.clear();
		if (!Resolve_ExactRegularFile(candidateDirectory, bootstrapRelative, bootstrapPath, status) ||
		    fs::file_size(bootstrapPath, error) > LostArk::Shared::GAMEPLAY_BOOTSTRAP_MAX_BYTES || error ||
		    !Hash_FileSha256(bootstrapPath, bootstrapRevision, status) ||
		    Format_GameplayDataRevision(bootstrapRevision) != Find_Member(*bootstrap, "candidateSha256")->String)
		{
			status = "Candidate gameplay bootstrap raw SHA-256 is invalid";
			return false;
		}

		// 발탄 밖의 행이 활성 세대와 같은지 확인한 후에만 독립 catalog를 로드한다.
		// catalog가 해석한 presentation 세대까지 일치해야 최종 후보를 반환한다.
		if (!Build_NonValtanGameplayRevision(bootstrapPath, candidateNonValtanGameplayRevision, status) ||
		    candidateNonValtanGameplayRevision != activeNonValtanGameplayRevision)
		{
			if (status.empty())
				status = "Candidate changes gameplay outside allowedDomains VALTAN_BOSS";
			return false;
		}
		auto staged = std::make_shared<LostArk::Server::CGameplayCatalog>();
		if (nullptr == staged ||
		    !staged->Load_FromBootstrap(bootstrapPath, bootstrapRevision, request.CandidateRevision))
		{
			status = nullptr == staged ? "Candidate gameplay catalog allocation failed" : staged->Get_Status();
			return false;
		}
		GameplayDataRevision presentationGenerationRevision{};
		if (!Try_Parse_GameplayDataRevision(presentationGenerationId, presentationGenerationRevision) ||
		    staged->Get_ValtanPresentationGenerationId() != presentationGenerationRevision)
		{
			status = "Candidate gameplay bootstrap and presentation generation disagree";
			return false;
		}
		candidate = std::move(staged);
		candidateBootstrapContentRevision = bootstrapRevision;
		status.clear();
		return true;
	}

	// 디스크 포인터 교체 도중 중단됐을 때 정확히 어느 이전/다음 세대인지 판정할 기록이다.
	// 메모리 포인터나 catalog를 저장하지 않고 재검증 가능한 식별자만 값으로 소유한다.
	struct RUNTIME_GAMEPLAY_ACTIVATION_JOURNAL final
	{
		std::uint32_t iTransactionSequence = 0u;
		LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION Base;
		LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION Candidate;
	};

	// 활성 세대 포인터와 미완료 교체 저널의 고정 파일명이다. 둘은 같은 검증된 루트에 둔다.
	constexpr std::wstring_view RUNTIME_ACTIVE_POINTER_NAME = L"active-generation.json";
	constexpr std::wstring_view RUNTIME_ACTIVATION_JOURNAL_NAME = L"active-generation.journal.json";

	// 논리 revision뿐 아니라 출처·bootstrap 원문·비발탄 내용까지 같아야 같은 영속 세대로 본다.
	bool RuntimeGenerationEquals(const LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION& left,
	                             const LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION& right)
	{
		return left.eSource == right.eSource && left.Revision == right.Revision &&
		       left.BootstrapContentRevision == right.BootstrapContentRevision &&
		       left.NonValtanGameplayRevision == right.NonValtanGameplayRevision;
	}

	// 영속 JSON의 sourceKind를 계약 문자열로 표현한다. 허용 enum 검증은 레코드의 Is_Valid가 맡는다.
	// 현재 Is_Valid는 세 revision만 검사하며 eSource를 검사하지 않는다. 이 함수는 CANDIDATE 외의 값을 PACKAGED_BASELINE으로 표현한다.
	// 디스크에서 읽은 출처 문자열의 허용 여부는 ParseRuntimeGenerationRecord가 확인한다.
	std::string RuntimeSourceName(const LostArk::Server::RUNTIME_GAMEPLAY_GENERATION_SOURCE source)
	{
		using SOURCE = LostArk::Server::RUNTIME_GAMEPLAY_GENERATION_SOURCE;
		return SOURCE::CANDIDATE == source ? "CANDIDATE" : "PACKAGED_BASELINE";
	}

	// 재시작 복구에 필요한 네 식별자를 정해진 키 순서로 output에 추가한다.
	// 식별자의 유효성은 쓰기 전 호출자가 확인한다.
	void AppendRuntimeGenerationRecord(const LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION& record,
	                                   std::string& output)
	{
		output += "{\"nonValtanGameplayRevision\":";
		Append_CanonicalString(Format_GameplayDataRevision(record.NonValtanGameplayRevision), output);
		output += ",\"revisionId\":";
		Append_CanonicalString(Format_GameplayDataRevision(record.Revision), output);
		output += ",\"serverBootstrapContentRevision\":";
		Append_CanonicalString(Format_GameplayDataRevision(record.BootstrapContentRevision), output);
		output += ",\"sourceKind\":";
		Append_CanonicalString(RuntimeSourceName(record.eSource), output);
		output.push_back('}');
	}

	// 활성 포인터의 schema·version과 세대 레코드를 정규 JSON 바이트로 만든다.
	std::string BuildRuntimeActivePointerBytes(const LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION& record)
	{
		std::string output = "{\"formatVersion\":1,\"generation\":";
		AppendRuntimeGenerationRecord(record, output);
		output += ",\"schema\":\"lostark.server-runtime-active-gameplay-generation\"}";
		return output;
	}

	// 저널에 이전·후보 세대와 transaction sequence를 기록해 디스크 중단 상태를 구별한다.
	std::string BuildRuntimeActivationJournalBytes(const RUNTIME_GAMEPLAY_ACTIVATION_JOURNAL& journal)
	{
		std::string output = "{\"base\":";
		AppendRuntimeGenerationRecord(journal.Base, output);
		output += ",\"candidate\":";
		AppendRuntimeGenerationRecord(journal.Candidate, output);
		output +=
		    ",\"formatVersion\":1,\"schema\":\"lostark.server-runtime-gameplay-activation-journal\",\"transactionSequence\":";
		output += std::to_string(journal.iTransactionSequence);
		output.push_back('}');
		return output;
	}

	// 정확한 필드·출처·SHA 표현을 검사해 복구 레코드를 채운다.
	// 단계별로 record를 쓰므로 실패 시 부분 값이 남을 수 있으며 호출자는 지역 후보를 사용한다.
	bool ParseRuntimeGenerationRecord(const JSON_VALUE* value,
	                                  LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION& record)
	{
		using SOURCE = LostArk::Server::RUNTIME_GAMEPLAY_GENERATION_SOURCE;
		if (nullptr == value ||
		    !Has_ExactMembers(
		        *value, {"nonValtanGameplayRevision", "revisionId", "serverBootstrapContentRevision", "sourceKind"}) ||
		    !Is_LowerSha256(Find_Member(*value, "nonValtanGameplayRevision")) ||
		    !Is_LowerSha256(Find_Member(*value, "revisionId")) ||
		    !Is_LowerSha256(Find_Member(*value, "serverBootstrapContentRevision")))
		{
			return false;
		}
		const JSON_VALUE* source = Find_Member(*value, "sourceKind");
		if (Is_String(source, "PACKAGED_BASELINE"))
			record.eSource = SOURCE::PACKAGED_BASELINE;
		else if (Is_String(source, "CANDIDATE"))
			record.eSource = SOURCE::CANDIDATE;
		else
			return false;
		return Try_Parse_GameplayDataRevision(Find_Member(*value, "revisionId")->String, record.Revision) &&
		       Try_Parse_GameplayDataRevision(Find_Member(*value, "serverBootstrapContentRevision")->String,
		                                      record.BootstrapContentRevision) &&
		       Try_Parse_GameplayDataRevision(Find_Member(*value, "nonValtanGameplayRevision")->String,
		                                      record.NonValtanGameplayRevision) &&
		       record.Is_Valid();
	}

	// 영속 JSON을 읽은 뒤 다시 직렬화해 원문과 비교한다. 공백·키 순서까지 일치해야
	// 디스크 레코드의 단일 표현으로 인정하며 실패 시 파싱된 value는 사용하지 않는다.
	bool ParseCanonicalRuntimeJson(const std::vector<std::uint8_t>& bytes, JSON_VALUE& value, std::string& status)
	{
		if (!Parse_JsonBytes(bytes, value, status))
			return false;
		std::string canonical;
		canonical.reserve(bytes.size());
		Append_CanonicalJson(value, canonical);
		if (canonical.size() != bytes.size() || !std::equal(canonical.begin(), canonical.end(), bytes.begin()))
		{
			status = "Runtime gameplay activation JSON is not canonical";
			return false;
		}
		return true;
	}

	// 활성 포인터의 schema/version과 세대를 검사한다. 성공 시 status를 비우고,
	// 실패 시 호출자가 전달한 record의 부분 변경을 확정하지 않도록 false를 돌려준다.
	bool ParseRuntimeActivePointer(const std::vector<std::uint8_t>& bytes,
	                               LostArk::Server::RUNTIME_ACTIVE_GAMEPLAY_GENERATION& record, std::string& status)
	{
		JSON_VALUE root;
		if (!ParseCanonicalRuntimeJson(bytes, root, status) ||
		    !Has_ExactMembers(root, {"formatVersion", "generation", "schema"}) ||
		    !Is_Integer(Find_Member(root, "formatVersion"), 1u, 1u) ||
		    !Is_String(Find_Member(root, "schema"), "lostark.server-runtime-active-gameplay-generation") ||
		    !ParseRuntimeGenerationRecord(Find_Member(root, "generation"), record))
		{
			if (status.empty())
				status = "Runtime active gameplay pointer is invalid";
			return false;
		}
		status.clear();
		return true;
	}

	// 이전·후보 세대가 서로 다르고 후보 출처와 양의 transaction sequence가 맞는지 검사한다.
	// 복구 시 이 레코드의 정확한 이전 또는 다음 세대만 선택할 수 있다.
	bool ParseRuntimeActivationJournal(const std::vector<std::uint8_t>& bytes,
	                                   RUNTIME_GAMEPLAY_ACTIVATION_JOURNAL& journal, std::string& status)
	{
		JSON_VALUE root;
		if (!ParseCanonicalRuntimeJson(bytes, root, status) ||
		    !Has_ExactMembers(root, {"base", "candidate", "formatVersion", "schema", "transactionSequence"}) ||
		    !Is_Integer(Find_Member(root, "formatVersion"), 1u, 1u) ||
		    !Is_Integer(Find_Member(root, "transactionSequence"), 1u, (std::numeric_limits<std::uint32_t>::max)()) ||
		    !Is_String(Find_Member(root, "schema"), "lostark.server-runtime-gameplay-activation-journal") ||
		    !ParseRuntimeGenerationRecord(Find_Member(root, "base"), journal.Base) ||
		    !ParseRuntimeGenerationRecord(Find_Member(root, "candidate"), journal.Candidate) ||
		    LostArk::Server::RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE != journal.Candidate.eSource ||
		    RuntimeGenerationEquals(journal.Base, journal.Candidate))
		{
			if (status.empty())
				status = "Runtime gameplay activation journal is invalid";
			return false;
		}
		journal.iTransactionSequence = static_cast<std::uint32_t>(Find_Member(root, "transactionSequence")->Integer);
		status.clear();
		return true;
	}

	// 영속화 루트의 절대·정규 경로와 재분석 지점 여부를 검사한다. create가 참이면
	// 폴더를 만들 수 있으나 성공한 root 출력만 확정한다.
	bool PrepareRuntimeGameplayRoot(const std::filesystem::path& requested, const bool create,
	                                std::filesystem::path& root, std::string& status)
	{
		namespace fs = std::filesystem;
		if (requested.empty() || !requested.is_absolute())
		{
			status = "Runtime gameplay activation root is not absolute";
			return false;
		}
		std::error_code error;
		const fs::path expected = requested.lexically_normal();
		if (create && !fs::create_directories(expected, error) && error)
		{
			status = "Runtime gameplay activation root cannot be created";
			return false;
		}
		if (!fs::exists(expected, error) || error)
		{
			status = create ? "Runtime gameplay activation root is unavailable"
			                : "Runtime gameplay activation root is absent";
			return false;
		}
		const fs::file_status linkStatus = fs::symlink_status(expected, error);
		const fs::path canonical = fs::canonical(expected, error);
		const DWORD attributes = ::GetFileAttributesW(expected.c_str());
		if (error || !fs::is_directory(linkStatus) || fs::is_symlink(linkStatus) ||
		    INVALID_FILE_ATTRIBUTES == attributes || 0u != (attributes & FILE_ATTRIBUTE_REPARSE_POINT) ||
		    canonical != expected)
		{
			status = "Runtime gameplay activation root is non-canonical or reparsed";
			return false;
		}
		root = canonical;
		status.clear();
		return true;
	}

	// 루트 바로 아래 파일인지와 일반 파일·재분석 지점 조건을 확인한다.
	// 파일 부재는 오류와 구분해 exists=false로 반환하며 path는 검사 과정에서 먼저 설정된다.
	bool InspectRuntimeGameplayFile(const std::filesystem::path& root, const std::wstring_view name,
	                                std::filesystem::path& path, bool& exists, std::string& status)
	{
		namespace fs = std::filesystem;
		path = (root / fs::path(name)).lexically_normal();
		if (path.parent_path() != root)
		{
			status = "Runtime gameplay activation file escaped its root";
			return false;
		}
		std::error_code error;
		const fs::file_status linkStatus = fs::symlink_status(path, error);
		if (error == std::errc::no_such_file_or_directory)
		{
			exists = false;
			status.clear();
			return true;
		}
		if (error)
		{
			status = "Runtime gameplay activation file status failed";
			return false;
		}
		exists = fs::exists(linkStatus);
		if (!exists)
		{
			status.clear();
			return true;
		}
		const DWORD attributes = ::GetFileAttributesW(path.c_str());
		if (!fs::is_regular_file(linkStatus) || fs::is_symlink(linkStatus) || INVALID_FILE_ATTRIBUTES == attributes ||
		    0u != (attributes & FILE_ATTRIBUTE_REPARSE_POINT))
		{
			status = "Runtime gameplay activation file is not an exact regular file";
			return false;
		}
		status.clear();
		return true;
	}

	// 포인터·저널의 부재를 정상 상태로 구분하고, 존재하면 64 KiB 이내로 읽는다.
	// 존재 검사가 성공하면 bytes를 먼저 비우므로 실패 시 이전 내용 보존을 보장하지 않는다.
	bool ReadOptionalRuntimeGameplayFile(const std::filesystem::path& root, const std::wstring_view name, bool& exists,
	                                     std::vector<std::uint8_t>& bytes, std::string& status)
	{
		std::filesystem::path path;
		if (!InspectRuntimeGameplayFile(root, name, path, exists, status))
			return false;
		bytes.clear();
		return !exists || Read_BoundedFile(path, 64u * 1024u, bytes, status);
	}

	// 같은 폴더의 고유 stage 파일을 CREATE_NEW로 열어 전체 쓰기·flush·close 후
	// MoveFileEx로 최종 파일을 교체한다. 실패하면 자기 stage 삭제를 시도하고 이유를 돌려준다.
	// 이 함수는 한 파일을 교체하며 포인터와 저널의 순서는 상위 트랜잭션이 관리한다.
	bool AtomicWriteRuntimeGameplayFile(const std::filesystem::path& root, const std::wstring_view name,
	                                    const std::string_view bytes, std::string& status)
	{
		std::filesystem::path finalPath;
		bool finalExists = false;
		if (!InspectRuntimeGameplayFile(root, name, finalPath, finalExists, status))
			return false;
		const std::filesystem::path stagePath = finalPath.wstring() + L".stage." +
		                                        std::to_wstring(::GetCurrentProcessId()) + L"." +
		                                        std::to_wstring(::GetTickCount64());
		HANDLE file = ::CreateFileW(stagePath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
		                            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, nullptr);
		if (INVALID_HANDLE_VALUE == file)
		{
			status = "Runtime gameplay activation stage file cannot be created";
			return false;
		}
		bool succeeded = true;
		std::size_t written = 0u;
		while (written < bytes.size())
		{
			const DWORD requestBytes = static_cast<DWORD>(
			    (std::min)(bytes.size() - written, static_cast<std::size_t>((std::numeric_limits<DWORD>::max)())));
			DWORD chunkBytes = 0u;
			if (!::WriteFile(file, bytes.data() + written, requestBytes, &chunkBytes, nullptr) ||
			    chunkBytes != requestBytes)
			{
				succeeded = false;
				break;
			}
			written += chunkBytes;
		}
		if (succeeded)
			succeeded = FALSE != ::FlushFileBuffers(file);
		if (!::CloseHandle(file))
			succeeded = false;
		if (succeeded)
		{
			succeeded = FALSE != ::MoveFileExW(stagePath.c_str(), finalPath.c_str(),
			                                   MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		}
		if (!succeeded)
		{
			(void)::DeleteFileW(stagePath.c_str());
			status = "Runtime gameplay activation atomic write failed";
			return false;
		}
		status.clear();
		return true;
	}

	// 활성화가 끝난 뒤 저널 정리를 시도하는 경로다. 삭제 실패를 결과로 보고하지 않으므로
	// 오프라인 reset처럼 부재 확정이 필요한 경로는 아래 Exact 함수를 사용한다.
	void DeleteRuntimeGameplayJournal(const std::filesystem::path& root) noexcept
	{
		std::filesystem::path path;
		bool exists = false;
		std::string ignored;
		if (InspectRuntimeGameplayFile(root, RUNTIME_ACTIVATION_JOURNAL_NAME, path, exists, ignored) && exists)
		{
			(void)::DeleteFileW(path.c_str());
		}
	}

	// 오프라인 reset이 포인터를 다시 바꾸기 전에 저널을 지우고 부재를 재검사한다.
	// 삭제 실패·재등장을 성공으로 취급하지 않고 status로 돌려준다.
	bool DeleteRuntimeGameplayJournalExact(const std::filesystem::path& root, std::string& status)
	{
		std::filesystem::path path;
		bool exists = false;
		if (!InspectRuntimeGameplayFile(root, RUNTIME_ACTIVATION_JOURNAL_NAME, path, exists, status))
			return false;
		if (exists && !::DeleteFileW(path.c_str()))
		{
			status = "Runtime gameplay activation journal cannot be removed";
			return false;
		}
		if (!InspectRuntimeGameplayFile(root, RUNTIME_ACTIVATION_JOURNAL_NAME, path, exists, status) || exists)
		{
			if (status.empty())
				status = "Runtime gameplay activation journal removal was not durable";
			return false;
		}
		status.clear();
		return true;
	}
}

// Run과 오프라인 reset이 빌드 구성별 named mutex를 공유해 같은 활성 포인터의 동시 변경을 막는다.
// 성공 시 handle 소유권을 호출자에게 넘기며 같은 소유 스레드가 Release를 호출해야 한다.
bool LostArk::Server::CServerApp::Acquire_RuntimeGameplayProcessMutex(void*& handle, std::string& status)
{
#ifdef _DEBUG
	constexpr const wchar_t* MUTEX_NAME = L"Local\\LostArk.Server.ValtanRuntimeActivation.Debug";
#else
	constexpr const wchar_t* MUTEX_NAME = L"Local\\LostArk.Server.ValtanRuntimeActivation.Release";
#endif
	return Acquire_NamedRuntimeGameplayProcessMutex(MUTEX_NAME, handle, status);
}

// 실제 named mutex 생성·즉시 획득 경로이며 검증 코드도 별도 이름으로 사용할 수 있다.
// 다른 프로세스 소유면 대기하지 않고 실패한다. abandoned 획득도 허용하고 디스크 복구가 상태를 검증한다.
bool LostArk::Server::CServerApp::Acquire_NamedRuntimeGameplayProcessMutex(const wchar_t* name, void*& handle,
                                                                           std::string& status)
{
	handle = nullptr;
	if (nullptr == name || L'\0' == name[0])
	{
		status = "Runtime gameplay process mutex name is missing";
		return false;
	}
	HANDLE mutex = ::CreateMutexW(nullptr, FALSE, name);
	if (nullptr == mutex)
	{
		status = "Runtime gameplay process mutex cannot be created";
		return false;
	}
	const DWORD wait = ::WaitForSingleObject(mutex, 0u);
	if (WAIT_OBJECT_0 != wait && WAIT_ABANDONED != wait)
	{
		::CloseHandle(mutex);
		status = WAIT_TIMEOUT == wait ? "Another Server process owns the runtime gameplay activation lock"
		                              : "Runtime gameplay process mutex wait failed";
		return false;
	}
	handle = mutex;
	status.clear();
	return true;
}

// 획득한 mutex 소유권과 handle을 정리하고 nullptr로 만든다.
// 이미 비어 있으면 아무 작업 없이 끝나며 Win32 정리 오류를 반환하지 않는다.
void LostArk::Server::CServerApp::Release_RuntimeGameplayProcessMutex(void*& handle) noexcept
{
	if (nullptr == handle)
		return;
	HANDLE mutex = static_cast<HANDLE>(handle);
	(void)::ReleaseMutex(mutex);
	(void)::CloseHandle(mutex);
	handle = nullptr;
}

// 룸 세대 확정 전에 디스크의 기존 포인터가 base와 같은지 검사하고 저널 → 후보 포인터 순으로 쓴다.
// 포인터 쓰기 실패 시 base 복구를 시도하며, 복구도 실패하면 저널을 남기고 실패 이유를 합친다.
// 메모리 룸은 이 함수에서 바꾸지 않는다. 호출자는 프로세스/트랜잭션 직렬화를 유지해야 한다.
bool LostArk::Server::CServerApp::Persist_RuntimeGameplayActivation(const std::filesystem::path& runtimeRoot,
                                                                    const std::uint32_t transactionSequence,
                                                                    const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& base,
                                                                    const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& candidate,
                                                                    std::string& status)
{
	if (0u == transactionSequence || !base.Is_Valid() || !candidate.Is_Valid() ||
	    RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE != candidate.eSource || RuntimeGenerationEquals(base, candidate))
	{
		status = "Runtime gameplay activation identity is invalid";
		return false;
	}
	std::filesystem::path root;
	if (!PrepareRuntimeGameplayRoot(runtimeRoot, true, root, status))
		return false;
	bool journalExists = false;
	std::vector<std::uint8_t> journalBytes;
	if (!ReadOptionalRuntimeGameplayFile(root, RUNTIME_ACTIVATION_JOURNAL_NAME, journalExists, journalBytes, status) ||
	    journalExists)
	{
		if (status.empty())
			status = "An unfinished runtime gameplay activation journal exists";
		return false;
	}
	bool pointerExists = false;
	std::vector<std::uint8_t> pointerBytes;
	if (!ReadOptionalRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME, pointerExists, pointerBytes, status))
		return false;
	if (pointerExists)
	{
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION diskBase{};
		if (!ParseRuntimeActivePointer(pointerBytes, diskBase, status) || !RuntimeGenerationEquals(diskBase, base))
		{
			if (status.empty())
				status = "Runtime active pointer does not match process active generation";
			return false;
		}
	}
	else if (RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE == base.eSource)
	{
		status = "Candidate process active generation has no durable pointer";
		return false;
	}

	// 저널이 먼저 기록되어야 포인터 교체 중 프로세스가 멈춰도 이전/다음 세대의 의미를 복구할 수 있다.
	RUNTIME_GAMEPLAY_ACTIVATION_JOURNAL journal{};
	journal.iTransactionSequence = transactionSequence;
	journal.Base = base;
	journal.Candidate = candidate;
	if (!AtomicWriteRuntimeGameplayFile(root, RUNTIME_ACTIVATION_JOURNAL_NAME,
	                                    BuildRuntimeActivationJournalBytes(journal), status))
		return false;
	if (!AtomicWriteRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME, BuildRuntimeActivePointerBytes(candidate),
	                                    status))
	{
		std::string rollbackStatus;
		if (!AtomicWriteRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME, BuildRuntimeActivePointerBytes(base),
		                                    rollbackStatus))
		{
			status += "; durable base-pointer rollback also failed: " + rollbackStatus;
			return false;
		}
		DeleteRuntimeGameplayJournal(root);
		return false;
	}
	status.clear();
	return true;
}

// 디스크 후보를 쓴 뒤 참여자 재검사가 실패했을 때 base 포인터로 되돌린다.
// 포인터 교체가 성공하면 저널 삭제를 시도한다. 이 복구 자체가 실패하면 호출자가 실행을 중단한다.
bool LostArk::Server::CServerApp::Rollback_RuntimeGameplayActivation(const std::filesystem::path& runtimeRoot,
                                                                     const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& base,
                                                                     std::string& status)
{
	if (!base.Is_Valid())
	{
		status = "Runtime gameplay rollback base is invalid";
		return false;
	}
	std::filesystem::path root;
	if (!PrepareRuntimeGameplayRoot(runtimeRoot, false, root, status) ||
	    !AtomicWriteRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME, BuildRuntimeActivePointerBytes(base),
	                                    status))
		return false;
	DeleteRuntimeGameplayJournal(root);
	status.clear();
	return true;
}

// 시작 복구 또는 메모리 확정이 끝난 뒤 저널 정리를 시도한다.
// 활성 포인터는 보존하며 삭제 성공을 반환하는 API는 아니다.
void LostArk::Server::CServerApp::Complete_RuntimeGameplayActivation(const std::filesystem::path& runtimeRoot) noexcept
{
	std::filesystem::path root;
	std::string ignored;
	if (PrepareRuntimeGameplayRoot(runtimeRoot, false, root, ignored))
		DeleteRuntimeGameplayJournal(root);
}

// Run의 세대 로드와 오프라인 reset이 디스크 포인터·저널을 읽어 정확한 이전/후보 세대를 고른다.
// 파일을 쓰지 않으며 active를 packaged로 초기화한 뒤 결과를 채운다. 실패 시 출력은 확정값이 아니다.
// packaged 식별자 차이 허용은 명시적 reset 경로에만 전달된다.
bool LostArk::Server::CServerApp::Recover_RuntimeActiveGameplayPointer(
    const std::filesystem::path& runtimeRoot, const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& packaged,
    RUNTIME_ACTIVE_GAMEPLAY_GENERATION& active, bool& hasPersistedPointer, std::string& status,
    const bool allowPackagedIdentityDrift)
{
	active = packaged;
	hasPersistedPointer = false;
	if (!packaged.Is_Valid() || RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE != packaged.eSource)
	{
		status = "Packaged runtime gameplay identity is invalid";
		return false;
	}
	std::error_code existsError;
	if (!std::filesystem::exists(runtimeRoot, existsError))
	{
		if (existsError)
		{
			status = "Runtime gameplay activation root status failed";
			return false;
		}
		status.clear();
		return true;
	}
	std::filesystem::path root;
	if (!PrepareRuntimeGameplayRoot(runtimeRoot, false, root, status))
		return false;
	bool pointerExists = false;
	bool journalExists = false;
	std::vector<std::uint8_t> pointerBytes;
	std::vector<std::uint8_t> journalBytes;
	if (!ReadOptionalRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME, pointerExists, pointerBytes, status) ||
	    !ReadOptionalRuntimeGameplayFile(root, RUNTIME_ACTIVATION_JOURNAL_NAME, journalExists, journalBytes, status))
		return false;

	RUNTIME_ACTIVE_GAMEPLAY_GENERATION pointer{};
	if (pointerExists && !ParseRuntimeActivePointer(pointerBytes, pointer, status))
		return false;
	if (!journalExists)
	{
		if (pointerExists)
			active = pointer;
		hasPersistedPointer = pointerExists;
		if (RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE == active.eSource && !allowPackagedIdentityDrift &&
		    !RuntimeGenerationEquals(active, packaged))
		{
			status = "Durable packaged pointer does not match packaged gameplay";
			return false;
		}
		status.clear();
		return true;
	}

	// 저널이 있으면 포인터가 가리킨 값이 정확히 base 또는 candidate여야 한다.
	// 제3의 식별자나 설명할 수 없는 부재는 추측으로 복구하지 않는다.
	RUNTIME_GAMEPLAY_ACTIVATION_JOURNAL journal{};
	if (!ParseRuntimeActivationJournal(journalBytes, journal, status))
		return false;
	if (pointerExists && RuntimeGenerationEquals(pointer, journal.Candidate))
		active = journal.Candidate;
	else if (pointerExists && RuntimeGenerationEquals(pointer, journal.Base))
		active = journal.Base;
	else if (!pointerExists && RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE == journal.Base.eSource &&
	         (allowPackagedIdentityDrift || RuntimeGenerationEquals(journal.Base, packaged)))
		active = journal.Base;
	else
	{
		status = "Activation journal cannot recover to an exact old or new generation";
		return false;
	}
	hasPersistedPointer = pointerExists;
	if (RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE == active.eSource && !allowPackagedIdentityDrift &&
	    !RuntimeGenerationEquals(active, packaged))
	{
		status = "Recovered packaged generation does not match packaged gameplay";
		return false;
	}
	status.clear();
	return true;
}

// 오프라인 reset이 검증한 packaged 세대로 디스크 포인터를 교체한다.
// 먼저 기존 저널의 정확한 세대를 확정하고 저널 부재를 확인하므로 중단 후 모호한 세대를 남기지 않는다.
bool LostArk::Server::CServerApp::Reset_RuntimeGameplayActivationToPackaged(
    const std::filesystem::path& runtimeRoot, const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& packaged, std::string& status)
{
	if (!packaged.Is_Valid() || RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE != packaged.eSource)
	{
		status = "Packaged runtime gameplay identity is invalid for reset";
		return false;
	}
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION recovered{};
	bool hadPointer = false;
	if (!Recover_RuntimeActiveGameplayPointer(runtimeRoot, packaged, recovered, hadPointer, status, true))
		return false;
	(void)hadPointer;

	// 오프라인 schema 갱신용 reset 경로다. 앞선 복구는 디스크 포인터/저널의 구조와
	// 정확한 이전/다음 식별자를 검증했다. 버릴 후보 artifact는 없거나 손상됐거나 폐기한
	// bootstrap schema일 수 있으므로 다시 admission하지 않는다. 아래에서는 새로 검증한
	// packaged 세대만 선택한다. 잘못되거나 모호한 포인터/저널은 쓰기 전에 거부한다.

	std::filesystem::path root;
	if (!PrepareRuntimeGameplayRoot(runtimeRoot, true, root, status))
		return false;
	// 먼저 저널의 정확한 이전/다음 세대를 포인터에 확정한다. 이 쓰기 전후에 중단돼도
	// 완전한 세대 하나를 가리키게 한 뒤, packaged 포인터를 한 번의 원자적 교체로 선택한다.
	if (!AtomicWriteRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME, BuildRuntimeActivePointerBytes(recovered),
	                                    status) ||
	    !DeleteRuntimeGameplayJournalExact(root, status))
		return false;
	if (!RuntimeGenerationEquals(recovered, packaged) &&
	    !AtomicWriteRuntimeGameplayFile(root, RUNTIME_ACTIVE_POINTER_NAME, BuildRuntimeActivePointerBytes(packaged),
	                                    status))
		return false;
	status.clear();
	return true;
}

// Run 시작 때 복구한 포인터가 packaged이면 전달된 catalog를 공유하고, 후보이면 artifact를 다시 검증한다.
// 후보 raw/비발탄 해시와 HOT_RELOAD base profile까지 일치해야 activeGeneration을 넘긴다.
// 실패하면 activeGeneration은 비어 있고 아직 listener나 룸 worker를 시작하지 않는다.
bool LostArk::Server::CServerApp::Load_RuntimeActiveGameplayGeneration(
    const std::filesystem::path& runtimeRoot, const std::shared_ptr<const CGameplayCatalog>& packagedGeneration,
    const RUNTIME_ACTIVE_GAMEPLAY_GENERATION& packaged, std::shared_ptr<const CGameplayCatalog>& activeGeneration,
    RUNTIME_ACTIVE_GAMEPLAY_GENERATION& active, bool& isCandidate, std::string& status)
{
	activeGeneration.reset();
	isCandidate = false;
	bool hasPersistedPointer = false;
	if (nullptr == packagedGeneration ||
	    !Recover_RuntimeActiveGameplayPointer(runtimeRoot, packaged, active, hasPersistedPointer, status))
		return false;
	if (RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE == active.eSource)
	{
		activeGeneration = packagedGeneration;
		Complete_RuntimeGameplayActivation(runtimeRoot);
		status.clear();
		return true;
	}

	LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST request{};
	request.iTransactionSequence = 1u;
	request.BaseRevision = packaged.Revision;
	request.CandidateRevision = active.Revision;
	request.iRequiredPresentationLaneMask = LostArk::Shared::GAMEPLAY_PRESENTATION_KNOWN_LANE_MASK;
	std::shared_ptr<const CGameplayCatalog> candidateGeneration;
	LostArk::Shared::GameplayDataRevision bootstrapRevision{};
	LostArk::Shared::GameplayDataRevision nonValtanRevision{};
	if (!Admit_ValtanCandidateGeneration(request, packaged.NonValtanGameplayRevision, candidateGeneration,
	                                     bootstrapRevision, nonValtanRevision, status) ||
	    bootstrapRevision != active.BootstrapContentRevision || nonValtanRevision != active.NonValtanGameplayRevision ||
	    !Validate_ValtanHotReloadBaseProfile(
	        packagedGeneration->Find_Boss("BOSS_VALTAN"),
	        nullptr == candidateGeneration ? nullptr : candidateGeneration->Find_Boss("BOSS_VALTAN"), status))
	{
		if (status.empty())
			status = "Durable runtime candidate failed startup admission";
		return false;
	}
	activeGeneration = std::move(candidateGeneration);
	isCandidate = true;
	Complete_RuntimeGameplayActivation(runtimeRoot);
	status.clear();
	return true;
}

// 검증용 override가 있으면 우선하고, 아니면 저장소의 빌드 구성별 runtime 폴더를 정한다.
// 경로 계산만 하며 폴더 생성·재분석 지점 검사는 영속화 helper가 수행한다.
std::filesystem::path LostArk::Server::CServerApp::Resolve_RuntimeActiveGameplayRoot() const
{
	if (!m_RuntimeActiveGameplayRootOverride.empty())
		return m_RuntimeActiveGameplayRootOverride;
	const std::filesystem::path repositoryRoot = Resolve_RepositoryRoot();
	if (repositoryRoot.empty())
		return {};
#ifdef _DEBUG
	constexpr std::wstring_view configuration = L"Debug";
#else
	constexpr std::wstring_view configuration = L"Release";
#endif
	return (repositoryRoot / L"Intermediate" / L"ValtanTuningRuntime" / L"Server" / configuration).lexically_normal();
}

// Main의 명시적 reset 옵션이 사용하는 Debug 전용 오프라인 작업이다.
// 프로세스 mutex를 획득하고 packaged bootstrap을 검증한 뒤 포인터만 교체하며 네트워크를 시작하지 않는다.
// 각 반환 경로는 finish로 mutex를 정리하고 실패 종류에 맞는 종료 코드를 돌려준다.
int LostArk::Server::CServerApp::Reset_ValtanRuntimeToPackaged()
{
#ifndef _DEBUG
	std::cerr << "Valtan runtime reset is unavailable in Release builds.\n";
	return 2;
#else
	void* processMutex = nullptr;
	std::string status;
	if (!Acquire_RuntimeGameplayProcessMutex(processMutex, status))
	{
		std::cerr << "Valtan runtime reset refused. Status=" << status << '\n';
		return 3;
	}
	const auto finish = [&processMutex](const int code)
	{
		Release_RuntimeGameplayProcessMutex(processMutex);
		return code;
	};
	auto packagedCatalog = std::make_shared<CGameplayCatalog>();
	if (nullptr == packagedCatalog || !packagedCatalog->Load())
	{
		std::cerr << "Packaged gameplay failed to load for Valtan reset. Status="
		          << (nullptr == packagedCatalog ? "allocation failed" : packagedCatalog->Get_Status()) << '\n';
		return finish(1);
	}
	const std::filesystem::path bootstrapPath = Resolve_RuntimeGameplayBootstrap();
	GameplayDataRevision nonValtanRevision{};
	if (bootstrapPath.empty() || !Build_NonValtanGameplayRevision(bootstrapPath, nonValtanRevision, status))
	{
		std::cerr << "Packaged gameplay identity failed for Valtan reset. Status=" << status << '\n';
		return finish(1);
	}
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION packaged{};
	packaged.eSource = RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE;
	packaged.Revision = packagedCatalog->Get_ActiveRevision();
	packaged.BootstrapContentRevision = packaged.Revision;
	packaged.NonValtanGameplayRevision = nonValtanRevision;
	const std::filesystem::path repositoryRoot = Resolve_RepositoryRoot();
	const std::filesystem::path runtimeRoot =
	    repositoryRoot.empty()
	        ? std::filesystem::path{}
	        : (repositoryRoot / L"Intermediate" / L"ValtanTuningRuntime" / L"Server" / L"Debug").lexically_normal();
	if (runtimeRoot.empty() || !Reset_RuntimeGameplayActivationToPackaged(runtimeRoot, packaged, status))
	{
		std::cerr << "Valtan runtime reset failed closed. Status="
		          << (status.empty() ? "runtime root unavailable" : status) << '\n';
		return finish(1);
	}
	std::cout << "Valtan runtime reset selected packaged gameplay revision "
	          << LostArk::Shared::Format_GameplayDataRevision(packaged.Revision) << ". Restart Server normally.\n";
	return finish(0);
#endif
}

// On_SessionClosed가 만든 JSON의 기본 schema/version/protocol/reason/outbound를 검사한다.
// 모든 outbound 수치 필드까지 검증하는 전체 schema validator는 아니며 실패 이유를 status로 알린다.
bool LostArk::Server::CServerApp::Validate_ServerSessionDiagnosticJson(const std::string_view json, std::string& status)
{
	JSON_VALUE root{};
	CBoundedJsonParser parser{json};
	if (!parser.Parse(root, status) || JSON_KIND::OBJECT != root.eKind)
	{
		if (status.empty())
			status = "Session diagnostic root must be a JSON object";
		return false;
	}
	const auto findMember = [&root](const std::string_view name) -> const JSON_VALUE*
	{
		for (std::size_t index = 0u; index < root.MemberNames.size(); ++index)
		{
			if (root.MemberNames[index] == name)
				return &root.MemberValues[index];
		}
		return nullptr;
	};
	const JSON_VALUE* schema = findMember("schema");
	const JSON_VALUE* formatVersion = findMember("formatVersion");
	const JSON_VALUE* protocolVersion = findMember("networkProtocolVersion");
	const JSON_VALUE* reason = findMember("reason");
	const JSON_VALUE* outbound = findMember("outbound");
	if (nullptr == schema || JSON_KIND::STRING != schema->eKind ||
	    "lostark.server-session-diagnostic" != schema->String || nullptr == formatVersion ||
	    JSON_KIND::INTEGER != formatVersion->eKind || formatVersion->isNegative || 1u != formatVersion->Integer ||
	    nullptr == protocolVersion || JSON_KIND::INTEGER != protocolVersion->eKind || protocolVersion->isNegative ||
	    LostArk::Shared::NETWORK_PROTOCOL_VERSION != protocolVersion->Integer || nullptr == reason ||
	    JSON_KIND::STRING != reason->eKind || reason->String.empty() || nullptr == outbound ||
	    JSON_KIND::OBJECT != outbound->eKind)
	{
		status = "Session diagnostic schema/format/protocol/reason/outbound contract invalid";
		return false;
	}
	status.clear();
	return true;
}

// Run의 조기 실패 경로도 같은 정리 순서를 거치도록 소멸 시 Shutdown을 다시 호출한다.
LostArk::Server::CServerApp::~CServerApp()
{
	Shutdown();
}

// Main이 서버 수명을 시작하는 동기 진입점이다. catalog·룸을 먼저 준비하고 listener를 연 뒤
// 룸/accept worker를 시작한다. 자동 종료 시간은 밀리초이며 0이면 콘솔 또는 headless 대기를 사용한다.
// 초기화 실패는 1, 정상 대기 종료와 Shutdown 완료는 0을 반환한다. 조기 실패 정리는 소멸자가 맡는다.
int LostArk::Server::CServerApp::Run(const std::uint32_t automaticShutdownMilliseconds,
                                     const std::string_view bindAddress, const std::uint16_t port, const bool headless)
{
	using LostArk::Shared::WORLD_ID;
#ifdef _DEBUG
	std::string processMutexStatus;
	if (!Acquire_RuntimeGameplayProcessMutex(m_hRuntimeGameplayProcessMutex, processMutexStatus))
	{
		std::cerr << "Server runtime gameplay lock acquisition failed. Status=" << processMutexStatus << '\n';
		return 1;
	}
#endif

	// 먼저 설치된 gameplay와 실제 bootstrap 내용의 식별자를 준비한다.
	// 네트워크를 열기 전이므로 실패한 초기 후보를 세션이 관찰하지 않는다.
	auto packagedGameplayCatalog = std::make_shared<CGameplayCatalog>();
	if (nullptr == packagedGameplayCatalog || !packagedGameplayCatalog->Load())
	{
		std::cerr << "Process gameplay generation failed to initialize. Status="
		          << (nullptr == packagedGameplayCatalog ? "Gameplay catalog allocation failed"
		                                                 : packagedGameplayCatalog->Get_Status())
		          << '\n';
		return 1;
	}
	std::shared_ptr<const CGameplayCatalog> packagedGameplayGeneration = std::move(packagedGameplayCatalog);
	std::shared_ptr<const CGameplayCatalog> initialGameplayGeneration = packagedGameplayGeneration;
	GameplayDataRevision initialNonValtanGameplayRevision{};
	std::string nonValtanStatus;
	const std::filesystem::path runtimeGameplayBootstrap = Resolve_RuntimeGameplayBootstrap();
	if (runtimeGameplayBootstrap.empty() ||
	    !Build_NonValtanGameplayRevision(runtimeGameplayBootstrap, initialNonValtanGameplayRevision, nonValtanStatus))
	{
		std::cerr << "Process non-Valtan gameplay identity failed to initialize. "
		             "Status="
		          << (nonValtanStatus.empty() ? "Runtime gameplay bootstrap is unavailable" : nonValtanStatus) << '\n';
		return 1;
	}
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION packagedRuntimeGeneration{};
	packagedRuntimeGeneration.eSource = RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE;
	packagedRuntimeGeneration.Revision = packagedGameplayGeneration->Get_ActiveRevision();
	if (!Hash_FileSha256(runtimeGameplayBootstrap, packagedRuntimeGeneration.BootstrapContentRevision, nonValtanStatus))
	{
		std::cerr << "Process gameplay content identity failed: " << nonValtanStatus << '\n';
		return 1;
	}
	packagedRuntimeGeneration.NonValtanGameplayRevision = initialNonValtanGameplayRevision;
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION initialRuntimeGeneration = packagedRuntimeGeneration;
	bool initialGenerationIsCandidate = false;
#ifdef _DEBUG
	std::string runtimeRecoveryStatus;
	if (!Load_RuntimeActiveGameplayGeneration(
	        Resolve_RuntimeActiveGameplayRoot(), packagedGameplayGeneration, packagedRuntimeGeneration,
	        initialGameplayGeneration, initialRuntimeGeneration, initialGenerationIsCandidate, runtimeRecoveryStatus))
	{
		std::cerr << "Runtime active gameplay recovery failed closed. Status=" << runtimeRecoveryStatus << '\n';
		return 1;
	}
#endif

	// 공유 월드는 지역 맵에서 모두 준비한 뒤 프로세스 맵으로 넘긴다.
	// 중간 룸 초기화 실패는 지역 shared_ptr들이 정리하며 부분 초기화 맵을 공개하지 않는다.
	std::map<WORLD_ID, std::shared_ptr<CGameRoom>> stagedSharedSimulations;
	const auto stageSharedSimulation = [&stagedSharedSimulations, &initialGameplayGeneration](const WORLD_ID worldId)
	{
		auto simulation = std::make_shared<CGameRoom>(worldId, initialGameplayGeneration);
		if (nullptr == simulation || !simulation->Is_Ready())
		{
			std::cerr << "World simulation failed to initialize. World=" << static_cast<unsigned>(worldId)
			          << ", Status="
			          << (nullptr == simulation ? "Simulation allocation failed" : simulation->Get_Status()) << '\n';
			return false;
		}
		return stagedSharedSimulations.emplace(worldId, std::move(simulation)).second;
	};

	if (!stageSharedSimulation(WORLD_ID::BERN) || !stageSharedSimulation(WORLD_ID::VALTAN_ARENA) ||
	    !stageSharedSimulation(WORLD_ID::TRAINING_GROUND) || !stageSharedSimulation(WORLD_ID::KOUKUSAYDON_ARENA) ||
	    !stageSharedSimulation(WORLD_ID::MAHARAKA) || !stageSharedSimulation(WORLD_ID::COLOSSEUM))
	{
		return 1;
	}

	// Character Select도 다른 월드와 같은 서버 게임플레이 런타임을 사용하지만
	// 입장 세션마다 별도의 시뮬레이션을 소유한다.
	// 시작 시 임시 인스턴스 하나를 준비해 실제 입장 전에 초기화 실패를 확인한다.
	{
		const auto validationSimulation =
		    std::make_shared<CGameRoom>(WORLD_ID::CHARACTER_SELECT_ARENA, initialGameplayGeneration);
		if (nullptr == validationSimulation || !validationSimulation->Is_Ready())
		{
			std::cerr << "Character Select simulation failed to initialize. Status="
			          << (nullptr == validationSimulation ? "Simulation allocation failed"
			                                              : validationSimulation->Get_Status())
			          << '\n';
			return 1;
		}
	}

	{
		std::scoped_lock lock{m_SessionsMutex};
		m_SharedGameRooms = std::move(stagedSharedSimulations);
		m_pActiveGameplayGeneration = std::move(initialGameplayGeneration);
		m_ActiveGameplayBootstrapContentRevision = initialRuntimeGeneration.BootstrapContentRevision;
		m_ActiveNonValtanGameplayRevision = initialRuntimeGeneration.NonValtanGameplayRevision;
		m_isActiveGameplayGenerationFromCandidate = initialGenerationIsCandidate;
#ifdef _DEBUG
		m_isRuntimeActivePersistenceEnabled = true;
#else
		m_isRuntimeActivePersistenceEnabled = false;
#endif
		m_CharacterSelectArenas.clear();
		m_ColosseumMatches.clear();
		m_GameplayBindingBySessionId.clear();
	}

	// 활성 세대에 대응하는 수치 조회/저장 계층을 준비한 뒤 WinSock과 listener를 연다.
	std::string numericStatus;
	if (!m_NumericBalanceStore.Initialize(m_pActiveGameplayGeneration, numericStatus))
	{
		std::cerr << "Numeric balance initialization failed: " << numericStatus << '\n';
		return 1;
	}

	if (!m_WinSockContext.Initialize())
	{
		std::cerr << "Failed to initialize WinSock 2.2\n";
		return 1;
	}
	if (0u == port || !m_TcpListener.Open(bindAddress, port))
	{
		std::cerr << "Failed to open TCP listener. Address=" << bindAddress << ':' << port
		          << ", Error=" << m_TcpListener.Get_LastErrorCode() << '\n';
		return 1;
	}

	// 이 시점부터 worker가 프로세스 상태를 사용한다. 콘솔·headless·시간 제한은
	// 대기 방식만 결정하며 같은 Shutdown 경로로 자원을 정리한다.
	m_isRunning.store(true);
	m_RoomThread = std::thread(&CServerApp::Room_Loop, this);
	m_AcceptThread = std::thread(&CServerApp::Accept_Loop, this);
	const bool useHeadlessMode = headless || 0 == ::_isatty(::_fileno(stdin));
	std::cout << "Listening on " << bindAddress << ':' << port << " with shared BERN, VALTAN_ARENA, TRAINING_GROUND, "
	          << "KOUKUSAYDON_ARENA, MAHARAKA, COLOSSEUM and "
	          << "session-private CHARACTER_SELECT_ARENA simulations.";
	if (0u == automaticShutdownMilliseconds && useHeadlessMode)
	{
		std::cout << " Headless mode; terminate the process to stop.\n";
		while (m_isRunning.load())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
		}
	}
	else if (0u == automaticShutdownMilliseconds)
	{
		std::cout << " Press Enter to stop.\n";
		std::cin.get();
	}
	else
	{
		std::cout << " Smoke timeout=" << automaticShutdownMilliseconds << "ms.\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(automaticShutdownMilliseconds));
	}
	Shutdown();
	return 0;
}

// accept worker가 수락한 소켓을 CClientSession에 넘기고 프로세스 세션 맵에 보관한다.
// 맵 등록 후 Start해야 수신 콜백이 세션을 찾을 수 있다. 시작 실패도 같은 종료 통지·회수 경로에 넣는다.
void LostArk::Server::CServerApp::Accept_Loop()
{
	while (m_isRunning.load())
	{
		const SOCKET clientSocket = m_TcpListener.Accept();
		if (INVALID_SOCKET == clientSocket)
		{
			if (m_isRunning.load())
			{
				std::cerr << "Accept failed. Error=" << m_TcpListener.Get_LastErrorCode() << '\n';
			}
			break;
		}

		const SESSION_ID sessionId = m_iNextSessionId.fetch_add(1);
		if (sessionId == INVALID_SESSION_ID)
		{
			::closesocket(clientSocket);
			continue;
		}
		auto session = std::make_shared<CClientSession>(
		    sessionId, clientSocket, [this](const SESSION_ID id, const LostArk::Shared::PACKET_FRAME& frame)
		    { On_SessionFrame(id, frame); }, [this](const SESSION_ID id) { On_SessionClosed(id); });
		{
			std::scoped_lock lock{m_SessionsMutex};
			m_Sessions.emplace(sessionId, session);
		}
		if (!session->Start())
		{
			session->Request_Close();
			On_SessionClosed(sessionId);
			continue;
		}
		std::cout << "Client connected. SessionId=" << sessionId << '\n';
	}
}

// 룸 worker만 30 Hz 고정 delta로 게임플레이와 제어 트랜잭션을 진행한다.
// 한 주기보다 더 늦으면 기준 시각을 현재로 옮겨 무한 추격을 피하고, 종료 시 남은 정리 명령을 한 번 더 처리한다.
void LostArk::Server::CServerApp::Room_Loop()
{
	using namespace std::chrono;
	constexpr duration<double> FIXED_STEP_SECONDS{1.0 / 30.0};
	constexpr float FIXED_DELTA_SECONDS = static_cast<float>(FIXED_STEP_SECONDS.count());
	const steady_clock::duration fixedStep = duration_cast<steady_clock::duration>(FIXED_STEP_SECONDS);
	steady_clock::time_point nextTick = steady_clock::now();
	SERVER_ROOM_SCHEDULER_METRICS schedulerMetrics;

	while (m_isRunning.load())
	{
		nextTick += fixedStep;
		Tick_GameplaySimulations(FIXED_DELTA_SECONDS, schedulerMetrics);
		Reap_ClosedSessions();
		std::this_thread::sleep_until(nextTick);
		const auto completedAt = steady_clock::now();
		schedulerMetrics.iSampleUnixMilliseconds = Current_UnixMilliseconds();
		schedulerMetrics.iPreviousLoopLatenessMicroseconds =
		    completedAt > nextTick
		        ? static_cast<std::uint64_t>(duration_cast<microseconds>(completedAt - nextTick).count())
		        : 0u;
		schedulerMetrics.iMaximumLoopLatenessMicroseconds =
		    (std::max)(schedulerMetrics.iMaximumLoopLatenessMicroseconds,
		               schedulerMetrics.iPreviousLoopLatenessMicroseconds);
		if (completedAt > nextTick + fixedStep)
		{
			nextTick = steady_clock::now();
			++schedulerMetrics.iScheduleResetCount;
		}
	}
	Tick_GameplaySimulations(FIXED_DELTA_SECONDS, schedulerMetrics);
	Reap_ClosedSessions();
}

// 세션 수신 콜백이 프레임을 typed 입력으로 검증해 배정 룸 또는 서버 제어 큐로 넘긴다.
// 저장 중 미룬 입장은 룸 worker도 이 경로로 재처리한다. command의 sessionId는 payload 대신 인자로 정한다.
// 게임플레이 변경은 룸 worker가 맡고 잘못된 payload·남는 바이트·허용되지 않은 패킷은 세션 종료를 요청한다.
void LostArk::Server::CServerApp::On_SessionFrame(const SESSION_ID sessionId,
                                                  const LostArk::Shared::PACKET_FRAME& frame)
{
	using namespace LostArk::Shared;

	CPacketReader reader{frame.Payload};
	const auto closeMalformedPayload = [this, sessionId](const std::string_view packetName)
	{
		const std::string context = "Malformed payload or trailing bytes for " + std::string{packetName};
		Request_SessionClose(sessionId, SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_MESSAGE_DECODE_FAILED, WSAEINVAL,
		                     context);
	};

	// 수치 조회/저장은 게임플레이 명령과 별도인 제어 이벤트다.
	// 수신에서는 메시지만 해석하고 store 접근과 worker 시작은 룸 경계에 맡긴다.
	if (frame.ePacketType == PACKET_TYPE::C2S_BALANCE_QUERY || frame.ePacketType == PACKET_TYPE::C2S_BALANCE_PATCH)
	{
		SERVER_CONTROL_EVENT event;
		event.iSessionId = sessionId;
		const bool query = frame.ePacketType == PACKET_TYPE::C2S_BALANCE_QUERY;
		event.eKind = query ? SERVER_CONTROL_EVENT_KIND::BALANCE_QUERY : SERVER_CONTROL_EVENT_KIND::BALANCE_PATCH;
		if (!(query ? Read_Message(reader, event.BalanceQuery) : Read_Message(reader, event.BalancePatch)) ||
		    reader.Get_RemainingSize())
		{
			closeMalformedPayload(query ? "C2S_BALANCE_QUERY" : "C2S_BALANCE_PATCH");
			return;
		}
		if (!Queue_ServerControlEvent(std::move(event)))
			Request_SessionClose(sessionId);
		return;
	}

	// 입장은 protocol/world prefix를 먼저 검사해 잘못된 접속 이유를 구체적으로 남긴다.
	// 전체 메시지와 남는 바이트 검사를 마친 뒤 목적 룸과 세션 바인딩을 준비한다.
	if (frame.ePacketType == PACKET_TYPE::C2S_ENTER_WORLD)
	{
		CPacketReader entryPrefixReader{frame.Payload};
		std::uint16_t receivedProtocolVersion = 0u;
		std::uint16_t receivedWorldId = 0u;
		if (!entryPrefixReader.Read_U16(receivedProtocolVersion) || !entryPrefixReader.Read_U16(receivedWorldId))
		{
			Request_SessionClose(sessionId, SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_MESSAGE_DECODE_FAILED, WSAEMSGSIZE,
			                     "C2S_ENTER_WORLD is truncated before protocol/world prefix");
			return;
		}
		if (receivedProtocolVersion != NETWORK_PROTOCOL_VERSION)
		{
			const std::string context =
			    "C2S_ENTER_WORLD protocol mismatch: expected=" + std::to_string(NETWORK_PROTOCOL_VERSION) +
			    ", received=" + std::to_string(receivedProtocolVersion);
			Request_SessionClose(sessionId, SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_MESSAGE_DECODE_FAILED,
			                     WSAEPROTONOSUPPORT, context);
			return;
		}
		if (!Is_Known_World_Id(static_cast<WORLD_ID>(receivedWorldId)))
		{
			const std::string context = "C2S_ENTER_WORLD contains unknown world id=" + std::to_string(receivedWorldId);
			Request_SessionClose(sessionId, SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_MESSAGE_DECODE_FAILED, WSAEINVAL,
			                     context);
			return;
		}

		C2S_ENTER_WORLD enterWorld{};
		if (!Read_Message(reader, enterWorld) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId, SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_MESSAGE_DECODE_FAILED,
			                     WSAEPROTONOSUPPORT, "C2S_ENTER_WORLD decode or trailing-byte validation failed");
			return;
		}

		// 수치 저장의 디스크 단계에서는 신규 입장 원본 프레임을 소유하는 제어 이벤트로 미룬다.
		// 해제 뒤 같은 수신 검증 경로로 다시 처리해 이전 세대로 입장하는 것을 막는다.
		std::unique_lock numericAdmissionLock{m_DataRevisionAdmissionMutex};
		if (m_NumericAdmissionPaused)
		{
			SERVER_CONTROL_EVENT event;
			event.eKind = SERVER_CONTROL_EVENT_KIND::BALANCE_DEFERRED_ENTRY;
			event.iSessionId = sessionId;
			event.DeferredEntry = frame;
			if (!Queue_ServerControlEvent(std::move(event)))
				Request_SessionClose(sessionId);
			return;
		}
		const std::shared_ptr<CGameRoom> targetSimulation = Acquire_EntrySimulation(sessionId, enterWorld.eWorldId);
		if (nullptr == targetSimulation)
		{
			Request_SessionClose(sessionId, SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED, 0,
			                     "entry simulation acquisition failed");
			return;
		}
		SESSION_DIAGNOSTIC_REASON failureReason = SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED;
		int nativeErrorCode = 0;
		std::string failureContext;
		if (!Bind_AndEnqueueEntry(sessionId, enterWorld.eWorldId, targetSimulation, std::move(enterWorld),
		                          failureReason, nativeErrorCode, failureContext))
		{
			Request_SessionClose(sessionId, failureReason, nativeErrorCode, failureContext);
		}
		return;
	}

	// 일반 입력은 payload 값과 신뢰할 수 있는 연결 sessionId를 ROOM_COMMAND로 묶는다.
	// 아래 각 분기는 해석·잔여 바이트 검사만 수행하고 권한·현재 상태 판정은 배정 룸이 맡는다.
	ROOM_COMMAND command{};
	command.iSessionId = sessionId;
	if (frame.ePacketType == PACKET_TYPE::C2S_MOVE)
	{
		C2S_MOVE move{};
		if (!Read_Message(reader, move) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_MOVE");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::MOVE;
		command.Move = move;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_USE_SKILL)
	{
		C2S_USE_SKILL useSkill{};
		if (!Read_Message(reader, useSkill) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_USE_SKILL");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::USE_SKILL;
		command.UseSkill = useSkill;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_RELEASE_SKILL)
	{
		C2S_RELEASE_SKILL releaseSkill{};
		if (!Read_Message(reader, releaseSkill) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_RELEASE_SKILL");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::RELEASE_SKILL;
		command.ReleaseSkill = releaseSkill;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_UPDATE_SKILL_AIM)
	{
		C2S_UPDATE_SKILL_AIM updateSkillAim{};
		if (!Read_Message(reader, updateSkillAim) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_UPDATE_SKILL_AIM");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::UPDATE_SKILL_AIM;
		command.UpdateSkillAim = updateSkillAim;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_USE_ESTHER_SKILL)
	{
		C2S_USE_ESTHER_SKILL useEstherSkill{};
		if (!Read_Message(reader, useEstherSkill) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_USE_ESTHER_SKILL");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::USE_ESTHER_SKILL;
		command.UseEstherSkill = useEstherSkill;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_USE_SQUAREHOLE)
	{
		C2S_USE_SQUAREHOLE useSquareHole{};
		if (!Read_Message(reader, useSquareHole) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_USE_SQUAREHOLE");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::USE_SQUAREHOLE;
		command.UseSquareHole = useSquareHole;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_REVIVE_PLAYER)
	{
		C2S_REVIVE_PLAYER revivePlayer{};
		if (!Read_Message(reader, revivePlayer) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_REVIVE_PLAYER");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::REVIVE_PLAYER;
		command.RevivePlayer = revivePlayer;
	}

	// 전투 디버그 입력도 동일한 typed 명령 큐를 거치므로 수신 worker가 엔티티를 직접 수정하지 않는다.
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_KILL_GATE_BOSSES)
	{
		C2S_DEBUG_KILL_GATE_BOSSES request{};
		if (!Read_Message(reader, request) || reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_KILL_GATE_BOSSES");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_KILL_GATE_BOSSES;
		command.DebugKillGateBosses = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_SET_COOLDOWN_MODE)
	{
		C2S_SET_COOLDOWN_MODE request{};
		if (!Read_Message(reader, request) || reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_SET_COOLDOWN_MODE");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::SET_COOLDOWN_MODE;
		command.SetCooldownMode = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_KILL_SELF)
	{
		C2S_DEBUG_KILL_SELF debugKillSelf{};
		if (!Read_Message(reader, debugKillSelf) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_KILL_SELF");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_KILL_SELF;
		command.DebugKillSelf = debugKillSelf;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_ENTER_KOUKUSAYDON_ARENA)
	{
		C2S_DEBUG_ENTER_KOUKUSAYDON_ARENA request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_ENTER_KOUKUSAYDON_ARENA");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_ENTER_KOUKUSAYDON_ARENA;
		command.DebugEnterKoukuSaydonArena = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_TELEPORT_TO_PLACEMENT)
	{
		C2S_DEBUG_TELEPORT_TO_PLACEMENT request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_TELEPORT_TO_PLACEMENT");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_TELEPORT_TO_PLACEMENT;
		command.DebugTeleportToPlacement = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_TELEPORT_TO_POSITION)
	{
		C2S_DEBUG_TELEPORT_TO_POSITION request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_TELEPORT_TO_POSITION");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_TELEPORT_TO_POSITION;
		command.DebugTeleportToPosition = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_MARIO_JUMP)
	{
		C2S_DEBUG_MARIO_JUMP request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_MARIO_JUMP");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_MARIO_JUMP;
		command.DebugMarioJump = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_MARIO_RETURN)
	{
		C2S_MARIO_RETURN request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_MARIO_RETURN");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::MARIO_RETURN;
		command.MarioReturn = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_MARIO_MOVE)
	{
		C2S_MARIO_MOVE request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_MARIO_MOVE");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::MARIO_MOVE;
		command.MarioMove = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_CHANGE_CHARACTER_CLASS)
	{
		C2S_CHANGE_CHARACTER_CLASS request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_CHANGE_CHARACTER_CLASS");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::CHANGE_CHARACTER_CLASS;
		command.ChangeCharacterClass = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_BINGO_FILL)
	{
		C2S_DEBUG_BINGO_FILL request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_BINGO_FILL");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_BINGO_FILL;
		command.DebugBingoFill = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_BINGO_BOMB)
	{
		C2S_DEBUG_BINGO_BOMB request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_BINGO_BOMB");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_BINGO_BOMB;
		command.DebugBingoBomb = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_BINGO_HAMMER)
	{
		C2S_DEBUG_BINGO_HAMMER request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_BINGO_HAMMER");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_BINGO_HAMMER;
		command.DebugBingoHammer = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_RESUMMON_WAVE_MONSTERS)
	{
		C2S_DEBUG_RESUMMON_WAVE_MONSTERS request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_RESUMMON_WAVE_MONSTERS");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_RESUMMON_WAVE_MONSTERS;
		command.DebugResummonWaveMonsters = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_USE_ESTHER)
	{
		C2S_DEBUG_USE_ESTHER request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_USE_ESTHER");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_USE_ESTHER;
		command.DebugUseEsther = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_SET_MADNESS_FORM)
	{
		C2S_DEBUG_SET_MADNESS_FORM request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_SET_MADNESS_FORM");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_SET_MADNESS_FORM;
		command.DebugSetMadnessForm = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_SET_VEHICLE_RIDING)
	{
		C2S_SET_VEHICLE_RIDING request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_SET_VEHICLE_RIDING");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::SET_VEHICLE_RIDING;
		command.SetVehicleRiding = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_SET_HONOR_TITLE)
	{
		C2S_SET_HONOR_TITLE request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_SET_HONOR_TITLE");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::SET_HONOR_TITLE;
		command.SetHonorTitle = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_INTERACTION_SLOT)
	{
		C2S_INTERACTION_SLOT request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_INTERACTION_SLOT");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::INTERACTION_SLOT;
		command.InteractionSlot = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_SET_KOUKU_HUD_MODE)
	{
		C2S_DEBUG_SET_KOUKU_HUD_MODE request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_SET_KOUKU_HUD_MODE");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_SET_KOUKU_HUD_MODE;
		command.DebugSetKoukuHudMode = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_SPAWN_WORLD_ENTITY)
	{
		C2S_SPAWN_WORLD_ENTITY request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_SPAWN_WORLD_ENTITY");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::SPAWN_WORLD_ENTITY;
		command.SpawnWorldEntity = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_VALTAN_AUDITION_REQUEST)
	{
		C2S_VALTAN_AUDITION_REQUEST request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_VALTAN_AUDITION_REQUEST");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::VALTAN_AUDITION;
		command.ValtanAudition = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST)
	{
		C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST request{};
		if (!Read_Message(reader, request) || reader.Get_RemainingSize() != 0u)
		{
			closeMalformedPayload("C2S_DEBUG_KOUKUSAYDON_RAID_REQUEST");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::KOUKUSAYDON_RAID;
		command.KoukuSaydonRaid = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK)
	{
		C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK chunk;
		if (!Read_Message(reader, chunk) || reader.Get_RemainingSize() != 0u)
		{
			closeMalformedPayload("C2S_DEBUG_KOUKUSAYDON_DRAFT_CHUNK");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::KOUKUSAYDON_DRAFT_CHUNK;
		command.KoukuSaydonDraftChunk = std::move(chunk);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST)
	{
		C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_KOUKUSAYDON_PATTERN_AUDITION_REQUEST");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::KOUKUSAYDON_PATTERN_AUDITION;
		command.KoukuSaydonPatternAudition = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_VALTAN_PATTERN_FLOW_START)
	{
		C2S_DEBUG_VALTAN_PATTERN_FLOW_START request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_VALTAN_PATTERN_FLOW_START");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::VALTAN_PATTERN_FLOW_START;
		command.ValtanPatternFlowStart = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT)
	{
		C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::VALTAN_PATTERN_FLOW_STOP_AFTER_CURRENT;
		command.ValtanPatternFlowStopAfterCurrent = std::move(request);
	}

	// 발탄 후보 요청은 디스크를 읽는 admission과 tick에서 수행하는 활성 확정을 분리한다.
	// 현재 세대를 잠금 안에서 복사하고 후보 검증 뒤에는 불변 catalog와 해시를 제어 큐로 넘긴다.
	else if (frame.ePacketType == PACKET_TYPE::C2S_DATA_REVISION_PREPARE_REQUEST)
	{
		C2S_DATA_REVISION_PREPARE_REQUEST request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DATA_REVISION_PREPARE_REQUEST");
			return;
		}

		std::shared_ptr<CClientSession> session;
		std::shared_ptr<CGameRoom> simulation;
		std::shared_ptr<const CGameplayCatalog> activeGeneration;
		GameplayDataRevision activeBootstrapContentRevision{};
		GameplayDataRevision activeNonValtanGameplayRevision{};
		{
			std::scoped_lock lock{m_SessionsMutex};
			const auto sessionIter = m_Sessions.find(sessionId);
			const auto bindingIter = m_GameplayBindingBySessionId.find(sessionId);
			if (m_Sessions.end() != sessionIter)
				session = sessionIter->second;
			if (m_GameplayBindingBySessionId.end() != bindingIter)
				simulation = bindingIter->second.pSimulation;
			activeGeneration = m_pActiveGameplayGeneration;
			activeBootstrapContentRevision = m_ActiveGameplayBootstrapContentRevision;
			activeNonValtanGameplayRevision = m_ActiveNonValtanGameplayRevision;
		}
		if (nullptr == session || nullptr == simulation || nullptr == activeGeneration)
		{
			Request_SessionClose(sessionId);
			return;
		}
		if (request.CandidateRevision == activeGeneration->Get_ActiveRevision())
		{
			// 기존 2단계 확정이 끝난 뒤 재시도 요청의 base는 이전 값일 수 있다.
			// 후보가 이미 활성 세대이면 프로토콜상 ABORTED를 보낼 수 없으므로
			// 동일 요청의 최종 결과인 COMMITTED를 다시 응답한다.
			if (!Send_DataRevisionResult(session, request, DATA_REVISION_RESULT::COMMITTED,
			                             activeGeneration->Get_ActiveRevision(), {}))
				Request_SessionClose(sessionId);
			return;
		}
#ifndef _DEBUG
		if (!Send_DataRevisionResult(session, request, DATA_REVISION_RESULT::ABORTED,
		                             activeGeneration->Get_ActiveRevision(),
		                             "Release Server rejects gameplay data revision activation"))
		{
			Request_SessionClose(sessionId);
		}
		return;
#else
		if (WORLD_ID::VALTAN_ARENA != simulation->Get_WorldId() ||
		    request.BaseRevision != activeGeneration->Get_ActiveRevision())
		{
			if (!Send_DataRevisionResult(session, request, DATA_REVISION_RESULT::ABORTED,
			                             activeGeneration->Get_ActiveRevision(),
			                             WORLD_ID::VALTAN_ARENA != simulation->Get_WorldId()
			                                 ? "Data revision requester is not bound to Valtan Arena"
			                                 : "Data revision base does not match process active generation"))
				Request_SessionClose(sessionId);
			return;
		}
		std::shared_ptr<const CGameplayCatalog> candidateGeneration;
		GameplayDataRevision candidateBootstrapContentRevision{};
		GameplayDataRevision candidateNonValtanGameplayRevision{};
		std::string admissionStatus;
		{
			std::scoped_lock admissionLock{m_DataRevisionAdmissionMutex};
			if (!Admit_ValtanCandidateGeneration(request, activeNonValtanGameplayRevision, candidateGeneration,
			                                     candidateBootstrapContentRevision, candidateNonValtanGameplayRevision,
			                                     admissionStatus) ||
			    !Validate_ValtanHotReloadBaseProfile(
			        activeGeneration->Find_Boss("BOSS_VALTAN"),
			        nullptr == candidateGeneration ? nullptr : candidateGeneration->Find_Boss("BOSS_VALTAN"),
			        admissionStatus))
			{
				candidateGeneration.reset();
			}
		}
		if (nullptr == candidateGeneration)
		{
			if (!Send_DataRevisionResult(
			        session, request, DATA_REVISION_RESULT::ABORTED, activeGeneration->Get_ActiveRevision(),
			        admissionStatus.empty() ? "Candidate admission failed closed" : admissionStatus))
				Request_SessionClose(sessionId);
			return;
		}
		SERVER_CONTROL_EVENT event{};
		event.eKind = SERVER_CONTROL_EVENT_KIND::DATA_REVISION_REQUEST;
		event.iSessionId = sessionId;
		event.RevisionRequest = request;
		event.pCandidateGeneration = std::move(candidateGeneration);
		event.BaseBootstrapContentRevision = activeBootstrapContentRevision;
		event.CandidateBootstrapContentRevision = candidateBootstrapContentRevision;
		event.BaseNonValtanGameplayRevision = activeNonValtanGameplayRevision;
		event.CandidateNonValtanGameplayRevision = candidateNonValtanGameplayRevision;
		if (!Queue_ServerControlEvent(std::move(event)))
		{
			if (!Send_DataRevisionResult(session, request, DATA_REVISION_RESULT::ABORTED,
			                             activeGeneration->Get_ActiveRevision(), "Server control ingress is full"))
				Request_SessionClose(sessionId);
		}
		return;
#endif
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DATA_REVISION_PREPARE_RESPONSE)
	{
		C2S_DATA_REVISION_PREPARE_RESPONSE response{};
		if (!Read_Message(reader, response) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DATA_REVISION_PREPARE_RESPONSE");
			return;
		}
#ifdef _DEBUG
		SERVER_CONTROL_EVENT event{};
		event.eKind = SERVER_CONTROL_EVENT_KIND::DATA_REVISION_RESPONSE;
		event.iSessionId = sessionId;
		event.RevisionResponse = std::move(response);
		if (!Queue_ServerControlEvent(std::move(event)))
			Request_SessionClose(sessionId);
#else
		Request_SessionClose(sessionId);
#endif
		return;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_VALTAN_DECISION_TRACE_QUERY)
	{
		C2S_VALTAN_DECISION_TRACE_QUERY request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_VALTAN_DECISION_TRACE_QUERY");
			return;
		}
#ifdef _DEBUG
		SERVER_CONTROL_EVENT event{};
		event.eKind = SERVER_CONTROL_EVENT_KIND::VALTAN_DECISION_TRACE_QUERY;
		event.iSessionId = sessionId;
		event.DecisionTraceQuery = std::move(request);
		if (!Queue_ServerControlEvent(std::move(event)))
			Request_SessionClose(sessionId);
#else
		std::shared_ptr<CClientSession> session;
		{
			std::scoped_lock lock{m_SessionsMutex};
			const auto iter = m_Sessions.find(sessionId);
			if (m_Sessions.end() != iter)
				session = iter->second;
		}
		S2C_VALTAN_DECISION_TRACE_RESPONSE response{};
		response.iRequestSequence = request.iRequestSequence;
		response.strBossPlacementId = request.strBossPlacementId;
		response.eResult = VALTAN_DECISION_TRACE_QUERY_RESULT::REJECTED_RELEASE_BUILD;
		CPacketWriter writer;
		if (nullptr == session || !Write_Message(writer, response) ||
		    !session->Send_Frame(PACKET_TYPE::S2C_VALTAN_DECISION_TRACE_RESPONSE, writer.Get_Buffer()))
			Request_SessionClose(sessionId);
#endif
		return;
	}

	// 인벤토리·장비·캐릭터 복원 입력은 각 메시지의 필드 계약을 해석해 룸 명령으로 전달한다.
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_GIVE_ITEM)
	{
		C2S_DEBUG_GIVE_ITEM request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_GIVE_ITEM");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_GIVE_ITEM;
		command.DebugGiveItem = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_USE_ITEM)
	{
		C2S_USE_ITEM request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_USE_ITEM");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::USE_ITEM;
		command.UseItem = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_SET_EQUIPMENT)
	{
		C2S_SET_EQUIPMENT request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_SET_EQUIPMENT");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::SET_EQUIPMENT;
		command.SetEquipment = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_REPAIR_EQUIPMENT)
	{
		C2S_REPAIR_EQUIPMENT request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_REPAIR_EQUIPMENT");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::REPAIR_EQUIPMENT;
		command.RepairEquipment = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_BUY_ITEMS)
	{
		C2S_BUY_ITEMS request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_BUY_ITEMS");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::BUY_ITEMS;
		command.BuyItems = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_RESTORE_CHARACTER)
	{
		C2S_RESTORE_CHARACTER request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_RESTORE_CHARACTER");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::RESTORE_CHARACTER;
		command.RestoreCharacter = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_UPGRADE_EQUIPMENT)
	{
		C2S_UPGRADE_EQUIPMENT request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_UPGRADE_EQUIPMENT");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::UPGRADE_EQUIPMENT;
		command.UpgradeEquipment = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_CAPTURE_CHARACTER)
	{
		C2S_CAPTURE_CHARACTER request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_CAPTURE_CHARACTER");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::CAPTURE_CHARACTER;
		command.CaptureCharacter = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DESPAWN_ALL_WORLD_ENTITIES)
	{
		C2S_DESPAWN_ALL_WORLD_ENTITIES request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DESPAWN_ALL_WORLD_ENTITIES");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DESPAWN_ALL_WORLD_ENTITIES;
		command.DespawnAllWorldEntities = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_CONFIRM_NPC_ENTRY)
	{
		C2S_CONFIRM_NPC_ENTRY request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_CONFIRM_NPC_ENTRY");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::CONFIRM_NPC_ENTRY;
		command.ConfirmNpcEntry = request;
	}

	// 매칭 대기·모집·로드 준비는 연결 바인딩의 룸이 처리한다.
	// 실제 목적 경기 준비와 월드 전환은 tick 뒤 전환 요청 경로에서 진행한다.
	else if (frame.ePacketType == PACKET_TYPE::C2S_COLOSSEUM_QUEUE_JOIN)
	{
		C2S_COLOSSEUM_QUEUE_JOIN request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_COLOSSEUM_QUEUE_JOIN");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::COLOSSEUM_QUEUE_JOIN;
		command.ColosseumQueueJoin = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_COLOSSEUM_RECRUIT)
	{
		C2S_COLOSSEUM_RECRUIT request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_COLOSSEUM_RECRUIT");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::COLOSSEUM_RECRUIT;
		command.ColosseumRecruit = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_COLOSSEUM_QUEUE_LEAVE)
	{
		C2S_COLOSSEUM_QUEUE_LEAVE request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_COLOSSEUM_QUEUE_LEAVE");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::COLOSSEUM_QUEUE_LEAVE;
		command.ColosseumQueueLeave = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_COLOSSEUM_LOAD_READY)
	{
		if (!Read_Message(reader, command.ColosseumLoadReady) || reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_COLOSSEUM_LOAD_READY");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::COLOSSEUM_LOAD_READY;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_COLOSSEUM_RETURN)
	{
		if (!Read_Message(reader, command.ColosseumReturn) || reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_COLOSSEUM_RETURN");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::COLOSSEUM_RETURN;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_INTERACT_TRIGGER)
	{
		C2S_INTERACT_TRIGGER request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_INTERACT_TRIGGER");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::INTERACT_TRIGGER;
		command.InteractTrigger = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_MAHARAKA_AI_TUNING)
	{
		C2S_MAHARAKA_AI_TUNING request{};
		if (!Read_Message(reader, request) || reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_MAHARAKA_AI_TUNING");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::MAHARAKA_AI_TUNING;
		command.MaharakaAITuning = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_DEBUG_WORLD_PLAYBACK)
	{
		C2S_DEBUG_WORLD_PLAYBACK request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_DEBUG_WORLD_PLAYBACK");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::DEBUG_WORLD_PLAYBACK;
		command.DebugWorldPlayback = std::move(request);
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_RETURN_TO_BERN)
	{
		C2S_RETURN_TO_BERN request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			closeMalformedPayload("C2S_RETURN_TO_BERN");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::RETURN_TO_BERN;
		command.ReturnToBern = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_GUIDE_CONTROL)
	{
		C2S_GUIDE_CONTROL request{};
		if (!Read_Message(reader, request) || reader.Get_RemainingSize() != 0u)
		{
			closeMalformedPayload("C2S_GUIDE_CONTROL");
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::GUIDE_CONTROL;
		command.GuideControl = request;
	}

	// 파티·레이드·관문 동의와 ping/chat도 같은 연결 순서로 룸에 전달한다.
	// payload가 다른 플레이어를 지정하더라도 명령 발신자는 위에서 설정한 sessionId다.
	else if (frame.ePacketType == PACKET_TYPE::C2S_PARTY_INVITE)
	{
		C2S_PARTY_INVITE request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::PARTY_INVITE;
		command.PartyInvite = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_PARTY_INVITE_RESPOND)
	{
		C2S_PARTY_INVITE_RESPOND request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::PARTY_INVITE_RESPOND;
		command.PartyInviteRespond = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_RAID_ENTRY_PROPOSE)
	{
		C2S_RAID_ENTRY_PROPOSE request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::RAID_ENTRY_PROPOSE;
		command.RaidEntryPropose = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_RAID_ENTRY_RESPOND)
	{
		C2S_RAID_ENTRY_RESPOND request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::RAID_ENTRY_RESPOND;
		command.RaidEntryRespond = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_GATE_PROGRESS_PROPOSE)
	{
		C2S_GATE_PROGRESS_PROPOSE request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::GATE_PROGRESS_PROPOSE;
		command.GateProgressPropose = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_GATE_PROGRESS_RESPOND)
	{
		C2S_GATE_PROGRESS_RESPOND request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::GATE_PROGRESS_RESPOND;
		command.GateProgressRespond = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_ROOM_PING)
	{
		C2S_ROOM_PING request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::ROOM_PING;
		command.RoomPing = request;
	}
	else if (frame.ePacketType == PACKET_TYPE::C2S_CHAT)
	{
		C2S_CHAT request{};
		if (!Read_Message(reader, request) || 0u != reader.Get_RemainingSize())
		{
			Request_SessionClose(sessionId);
			return;
		}
		command.eType = ROOM_COMMAND_TYPE::CHAT;
		command.Chat = request;
	}
	else
	{
		Request_SessionClose(sessionId, SESSION_DIAGNOSTIC_REASON::SERVER_UNKNOWN_PACKET, WSAEPROTONOSUPPORT,
		                     "packet type is not a supported Client command");
		return;
	}

	std::string enqueueContext;

	// 룸 큐의 거부 원인을 세션 진단으로 분류한다. 신뢰성 입력 용량 초과·룸 실패·
	// 바인딩 누락을 같은 오류로 뭉개지 않아 종료 로그에서 실제 경계를 찾을 수 있다.
	const ROOM_COMMAND_ENQUEUE_RESULT enqueueResult =
	    Enqueue_AssignedCommand(sessionId, std::move(command), enqueueContext);
	if (Is_AcceptedRoomCommandEnqueueResult(enqueueResult) ||
	    ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_PENDING_CLEANUP == enqueueResult)
	{
		return;
	}

	if (ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY == enqueueResult)
	{
		Request_SessionClose(sessionId, SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_INGRESS_OVERFLOW, WSAENOBUFS,
		                     "assigned room reliable ingress reached its hard bound; " + enqueueContext);
		return;
	}
	if (ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_NOT_READY == enqueueResult)
	{
		Request_SessionClose(sessionId, SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_RUNTIME_FAILED, WSAESHUTDOWN,
		                     "assigned room stopped after a runtime failure; " + enqueueContext);
		return;
	}
	Request_SessionClose(sessionId,
	                     ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_INVALID_COMMAND == enqueueResult
	                         ? SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_COMMAND_VALIDATION_FAILED
	                         : SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED,
	                     ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_BINDING_MISSING == enqueueResult ? WSAENOTCONN
	                                                                                            : WSAESHUTDOWN,
	                     "assigned room command admission failed; " + enqueueContext);
}

// 세션 종료 콜백에서 배정 룸에 LEAVE를 넣고 바인딩을 제거한 뒤 진단 값을 기록한다.
// 현재 세션 worker에서 호출될 수 있으므로 여기서 join하지 않고 닫힌 ID를 룸 worker의 회수 큐로 넘긴다.
// 게임플레이 퇴장 처리와 세션 thread 수명 정리는 서로 다른 큐에서 마무리된다.
void LostArk::Server::CServerApp::On_SessionClosed(const SESSION_ID sessionId)
{
	std::shared_ptr<CClientSession> session;
	LostArk::Shared::WORLD_ID worldId = LostArk::Shared::WORLD_ID::END;
	bool wasBound = false;
	bool leaveEnqueued = false;
	ROOM_COMMAND_ENQUEUE_RESULT leaveEnqueueResult = ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_BINDING_MISSING;
	std::string leaveEnqueueContext;
	{
		std::scoped_lock lock{m_SessionsMutex};
		const auto sessionIter = m_Sessions.find(sessionId);
		if (sessionIter != m_Sessions.end())
			session = sessionIter->second;
		const auto bindingIter = m_GameplayBindingBySessionId.find(sessionId);
		if (bindingIter != m_GameplayBindingBySessionId.end())
		{
			wasBound = true;
			worldId = bindingIter->second.eWorldId;
			if (nullptr != bindingIter->second.pSimulation)
			{
				ROOM_COMMAND command{};
				command.eType = ROOM_COMMAND_TYPE::LEAVE;
				command.iSessionId = sessionId;
				command.eLeaveReason = LostArk::Shared::PLAYER_DESPAWN_REASON::DISCONNECTED;
				leaveEnqueueResult = bindingIter->second.pSimulation->Enqueue_Detailed(std::move(command));
				leaveEnqueued = Is_AcceptedRoomCommandEnqueueResult(leaveEnqueueResult);
				leaveEnqueueContext = bindingIter->second.pSimulation->Describe_EnqueueResult(leaveEnqueueResult);
			}
			m_GameplayBindingBySessionId.erase(bindingIter);
		}
	}

	// 바인딩 잠금을 풀고 세션의 종료 원인과 큐 통계를 값으로 가져온다.
	// 진단 출력만 별도 mutex로 직렬화해 여러 연결의 JSON 한 행이 섞이지 않게 한다.
	if (nullptr != session)
	{
		CLIENT_SESSION_CLOSE_DIAGNOSTIC diagnostic = session->Get_CloseDiagnostic();
		if (LostArk::Shared::SESSION_DIAGNOSTIC_REASON::NONE == diagnostic.eReason)
		{
			session->Request_Close(LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_APPLICATION_CLOSE, 0,
			                       "close callback arrived without a terminal classification");
			diagnostic = session->Get_CloseDiagnostic();
		}
		const CLIENT_SESSION_OUTBOUND_METRICS metrics = session->Get_OutboundMetrics();
		const std::string json =
		    Build_ServerSessionDiagnosticJson(sessionId, session->Get_PeerEndpoint(), diagnostic, worldId,
		                                      session->Get_PlayerId(), wasBound, leaveEnqueued, metrics);
		std::string jsonValidationStatus;
		const bool isJsonValid = Validate_ServerSessionDiagnosticJson(json, jsonValidationStatus);
		std::scoped_lock diagnosticLock{m_SessionDiagnosticLogMutex};
		if (!isJsonValid)
		{
			std::cerr << "[SessionDiagnostic][INVALID_JSON] SessionId=" << sessionId
			          << ", Status=" << jsonValidationStatus << '\n';
		}
		std::cout << "[SessionDiagnostic] " << json << '\n';
		if (wasBound && !leaveEnqueued && ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_NOT_READY != leaveEnqueueResult)
		{
			std::cerr << "[SessionDiagnostic][LEAVE_ENQUEUE_INVARIANT_FAILED] SessionId=" << sessionId
			          << ", World=" << static_cast<std::uint16_t>(worldId)
			          << ". A live binding unexpectedly rejected its dedicated cleanup "
			             "queue; inspect room readiness/retirement invariants. "
			          << leaveEnqueueContext << '\n';
		}
		const std::filesystem::path logPath = Resolve_ServerSessionDiagnosticPath();
		if (!logPath.empty())
		{
			std::ofstream log{logPath, std::ios::binary | std::ios::app};
			if (log)
				log << json << '\n';
			else
				std::cerr << "Session diagnostic log open failed. Path=" << logPath.string() << '\n';
		}
		else
		{
			std::cerr << "Session diagnostic log path resolution failed.\n";
		}
	}

	// private Character Select 룸은 큐에 넣은 LEAVE를 계속 소유한다.
	// 룸 worker가 이를 소비하고 빈 큐를 봉인한 뒤 arena를 회수한다.
	{
		std::scoped_lock lock{m_ClosedSessionMutex};
		m_ClosedSessionIds.push_back(sessionId);
	}
	SERVER_CONTROL_EVENT controlEvent{};
	controlEvent.eKind = SERVER_CONTROL_EVENT_KIND::SESSION_DISCONNECTED;
	controlEvent.iSessionId = sessionId;
	(void)Queue_ServerControlEvent(std::move(controlEvent));
}

// 세션 맵 잠금 안에서 shared_ptr만 확보하고 잠금 밖에서 종료를 요청한다.
// 수신·송신 종료와 콜백은 세션 책임이며 여기서는 worker를 기다리거나 맵에서 제거하지 않는다.
void LostArk::Server::CServerApp::Request_SessionClose(const SESSION_ID sessionId,
                                                       const LostArk::Shared::SESSION_DIAGNOSTIC_REASON reason,
                                                       const int nativeErrorCode, const std::string_view context)
{
	std::shared_ptr<CClientSession> session;
	{
		std::scoped_lock lock{m_SessionsMutex};
		const auto iter = m_Sessions.find(sessionId);
		if (iter != m_Sessions.end())
			session = iter->second;
	}
	if (nullptr != session)
		session->Request_Close(reason, nativeErrorCode, context);
}

// 룸 worker가 종료 ID 큐를 가져와 맵의 세션 소유권을 지역 shared_ptr로 옮긴다.
// 세션 맵 잠금을 해제한 뒤 Stop으로 worker를 합류시켜 종료 콜백의 자기 join과 잠금 교착을 피한다.
void LostArk::Server::CServerApp::Reap_ClosedSessions()
{
	std::deque<SESSION_ID> closedIds;
	{
		std::scoped_lock lock{m_ClosedSessionMutex};
		closedIds.swap(m_ClosedSessionIds);
	}
	for (const SESSION_ID sessionId : closedIds)
	{
		std::shared_ptr<CClientSession> session;
		{
			std::scoped_lock lock{m_SessionsMutex};
			const auto iter = m_Sessions.find(sessionId);
			if (iter == m_Sessions.end())
				continue;
			session = std::move(iter->second);
			m_Sessions.erase(iter);
		}
		if (nullptr != session)
			session->Stop();
	}
}

// 입장 요청의 목적 룸을 가져온다. 일반 월드는 공유 룸을 사용하고 Character Select는 세션별 룸을 준비한다.
// 기존 private 바인딩은 재사용하며 새 룸 생성은 세션 맵 잠금 밖에서 수행한다. 실제 등록은 Bind 단계가 맡는다.
std::shared_ptr<LostArk::Server::CGameRoom> LostArk::Server::CServerApp::Acquire_EntrySimulation(
    const SESSION_ID sessionId, const LostArk::Shared::WORLD_ID worldId)
{
	using LostArk::Shared::WORLD_ID;

	if (!LostArk::Shared::Is_Known_World_Id(worldId))
		return nullptr;
	if (WORLD_ID::CHARACTER_SELECT_ARENA != worldId)
		return Find_SharedSimulation(worldId);

	std::shared_ptr<const CGameplayCatalog> activeGameplayGeneration;
	{
		std::scoped_lock lock{m_SessionsMutex};
		if (!m_Sessions.contains(sessionId))
			return nullptr;

		const auto bindingIter = m_GameplayBindingBySessionId.find(sessionId);
		if (bindingIter != m_GameplayBindingBySessionId.end())
		{
			const SESSION_GAMEPLAY_BINDING& binding = bindingIter->second;
			return binding.eWorldId == worldId && binding.iPrivateArenaOwnerSessionId == sessionId ? binding.pSimulation
			                                                                                       : nullptr;
		}
		activeGameplayGeneration = m_pActiveGameplayGeneration;
	}

	if (nullptr == activeGameplayGeneration)
		return nullptr;
	auto simulation = std::make_shared<CGameRoom>(worldId, activeGameplayGeneration);
	if (nullptr == simulation || !simulation->Is_Ready())
	{
		std::cerr << "Private Character Select simulation failed. SessionId=" << sessionId
		          << ", Status=" << (nullptr == simulation ? "Simulation allocation failed" : simulation->Get_Status())
		          << '\n';
		return nullptr;
	}
	return simulation;
}

// 세션 맵과 같은 잠금 아래 공유 룸을 찾고 shared_ptr를 복사해 호출 동안 수명을 보장한다.
// 없는 world는 nullptr이며 여기서 새 룸을 만들지는 않는다.
std::shared_ptr<LostArk::Server::CGameRoom> LostArk::Server::CServerApp::Find_SharedSimulation(
    const LostArk::Shared::WORLD_ID worldId)
{
	std::scoped_lock lock{m_SessionsMutex};
	const auto iter = m_SharedGameRooms.find(worldId);
	return iter == m_SharedGameRooms.end() ? nullptr : iter->second;
}

// 초기 입장에서 세션·룸 세대·기존 바인딩을 재확인하고 REGISTER_SESSION 다음 ENTER_WORLD를 큐에 넣는다.
// 실패 시 새 바인딩을 되돌리거나 LEAVE 정리를 예약하고 진단 이유를 출력한다.
// 이미 큐에 들어간 private 룸은 소비·retirement가 끝날 때까지 소유 컨테이너에 남을 수 있다.
bool LostArk::Server::CServerApp::Bind_AndEnqueueEntry(const SESSION_ID sessionId,
                                                       const LostArk::Shared::WORLD_ID worldId,
                                                       const std::shared_ptr<CGameRoom>& simulation,
                                                       LostArk::Shared::C2S_ENTER_WORLD enterWorld,
                                                       LostArk::Shared::SESSION_DIAGNOSTIC_REASON& outFailureReason,
                                                       int& outNativeErrorCode, std::string& outFailureContext)
{
	using namespace LostArk::Shared;
	outFailureReason = SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED;
	outNativeErrorCode = WSAEINVAL;
	outFailureContext = "entry stage=bind-validation packet=C2S_ENTER_WORLD";

	if (nullptr == simulation || !Is_Known_World_Id(worldId) || simulation->Get_WorldId() != worldId ||
	    enterWorld.eWorldId != worldId)
	{
		return false;
	}

	// 바인딩, REGISTER/ENTER 큐 등록, 종료 상태 재검사는 On_SessionClosed와
	// 같은 트랜잭션 경계에 둔다. 잠금 순서는 세션 맵 → 룸 명령 큐로 유지한다.
	std::scoped_lock lock{m_SessionsMutex};
	const auto sessionIter = m_Sessions.find(sessionId);
	if (sessionIter == m_Sessions.end() || nullptr == sessionIter->second)
	{
		outNativeErrorCode = WSAENOTCONN;
		outFailureContext = "entry stage=session-lookup packet=C2S_ENTER_WORLD";
		return false;
	}
	if (sessionIter->second->Is_Closing())
	{
		outNativeErrorCode = WSAESHUTDOWN;
		outFailureContext = "entry stage=terminal-recheck packet=C2S_ENTER_WORLD "
		                    "sessionAlreadyClosing=true";
		return false;
	}
	if (nullptr == m_pActiveGameplayGeneration || nullptr == simulation->Get_ActiveGameplayGeneration() ||
	    simulation->Get_ActiveGameplayGeneration()->Get_ActiveRevision() !=
	        m_pActiveGameplayGeneration->Get_ActiveRevision())
	{
		outFailureContext = "entry stage=gameplay-revision-validation "
		                    "packet=C2S_ENTER_WORLD";
		return false;
	}

	const SESSION_ID privateOwnerSessionId =
	    WORLD_ID::CHARACTER_SELECT_ARENA == worldId ? sessionId : INVALID_SESSION_ID;
	const auto existingBinding = m_GameplayBindingBySessionId.find(sessionId);
	if (existingBinding != m_GameplayBindingBySessionId.end())
	{
		outFailureReason = SESSION_DIAGNOSTIC_REASON::SERVER_CLIENT_COMMAND_VALIDATION_FAILED;
		outNativeErrorCode = WSAEALREADY;
		outFailureContext = "entry stage=duplicate-binding packet=C2S_ENTER_WORLD";
		return false;
	}

	// 개인 선택창은 owner ID로 룸의 강한 참조를 먼저 보관하고, 공유 월드는 이미 등록된 룸과 같은 포인터인지 확인한다.
	// 아직 REGISTER가 큐에 들어가기 전이므로 다음 바인딩/등록 실패 때는 이번에 추가한 개인 룸 항목도 제거할 수 있다.
	bool insertedPrivateArena = false;
	if (WORLD_ID::CHARACTER_SELECT_ARENA == worldId)
	{
		const auto [arenaIter, inserted] = m_CharacterSelectArenas.emplace(sessionId, simulation);
		if (!inserted && arenaIter->second != simulation)
		{
			outFailureContext = "entry stage=private-arena-binding packet=C2S_ENTER_WORLD";
			return false;
		}
		insertedPrivateArena = inserted;
	}
	else
	{
		const auto sharedIter = m_SharedGameRooms.find(worldId);
		if (sharedIter == m_SharedGameRooms.end() || sharedIter->second != simulation)
		{
			outFailureContext = "entry stage=shared-room-binding packet=C2S_ENTER_WORLD";
			return false;
		}
	}

	SESSION_GAMEPLAY_BINDING binding{};
	binding.eWorldId = worldId;
	binding.iPrivateArenaOwnerSessionId = privateOwnerSessionId;
	binding.pSimulation = simulation;
	const auto [bindingIter, insertedBinding] = m_GameplayBindingBySessionId.emplace(sessionId, std::move(binding));
	(void)bindingIter;
	if (!insertedBinding)
	{
		if (insertedPrivateArena)
			m_CharacterSelectArenas.erase(sessionId);
		outFailureContext = "entry stage=binding-commit packet=C2S_ENTER_WORLD";
		return false;
	}

	ROOM_COMMAND registerCommand{};
	registerCommand.eType = ROOM_COMMAND_TYPE::REGISTER_SESSION;
	registerCommand.iSessionId = sessionId;
	registerCommand.pSession = sessionIter->second;
	const ROOM_COMMAND_ENQUEUE_RESULT registerResult = simulation->Enqueue_Detailed(std::move(registerCommand));
	if (ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED != registerResult)
	{
		m_GameplayBindingBySessionId.erase(sessionId);
		if (insertedPrivateArena)
			m_CharacterSelectArenas.erase(sessionId);
		outFailureReason = ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY == registerResult
		                       ? SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_INGRESS_OVERFLOW
		                   : ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_NOT_READY == registerResult
		                       ? SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_RUNTIME_FAILED
		                       : SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED;
		outNativeErrorCode =
		    ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY == registerResult ? WSAENOBUFS : WSAESHUTDOWN;
		outFailureContext =
		    "entry stage=register-ingress packet=C2S_ENTER_WORLD " + simulation->Describe_EnqueueResult(registerResult);
		return false;
	}

	// REGISTER가 승인된 뒤 ENTER를 이어 넣는다. ENTER만 거부되면 LEAVE 보상 예약을 시도하고 성공 여부를 진단에 남긴다.
	// 이때 binding은 제거하지만 개인 룸은 유지하며, 실제 정리 소비와 retirement 조건에 따라 후속 회수한다.
	ROOM_COMMAND enterCommand{};
	enterCommand.eType = ROOM_COMMAND_TYPE::ENTER_WORLD;
	enterCommand.iSessionId = sessionId;
	enterCommand.EnterWorld = std::move(enterWorld);
	const ROOM_COMMAND_ENQUEUE_RESULT enterResult = simulation->Enqueue_Detailed(std::move(enterCommand));
	if (ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED != enterResult)
	{
		ROOM_COMMAND rollbackCommand{};
		rollbackCommand.eType = ROOM_COMMAND_TYPE::LEAVE;
		rollbackCommand.iSessionId = sessionId;
		rollbackCommand.eLeaveReason = PLAYER_DESPAWN_REASON::DISCONNECTED;
		const bool rollbackEnqueued = simulation->Enqueue(std::move(rollbackCommand));
		m_GameplayBindingBySessionId.erase(sessionId);
		outFailureReason = ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY == enterResult
		                       ? SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_INGRESS_OVERFLOW
		                   : ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_NOT_READY == enterResult
		                       ? SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_RUNTIME_FAILED
		                       : SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED;
		outNativeErrorCode =
		    ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY == enterResult ? WSAENOBUFS : WSAESHUTDOWN;
		outFailureContext = "entry stage=enter-ingress packet=C2S_ENTER_WORLD " +
		                    simulation->Describe_EnqueueResult(enterResult) +
		                    " "
		                    "rollbackCleanupEnqueued=" +
		                    std::string{rollbackEnqueued ? "true" : "false"};
		return false;
	}
	return true;
}

// 일반 수신 명령을 현재 바인딩의 룸 큐에 넣고 구체적 거부 결과를 돌려준다.
// 세션 맵 조회와 Enqueue를 같은 잠금 범위에 두어 월드 전환·종료와 명령 순서를 일치시킨다.
LostArk::Server::ROOM_COMMAND_ENQUEUE_RESULT LostArk::Server::CServerApp::Enqueue_AssignedCommand(
    const SESSION_ID sessionId, ROOM_COMMAND command, std::string& outContext)
{
	outContext.clear();
	if (command.iSessionId != sessionId)
	{
		outContext = "enqueueResult=REJECTED_INVALID_COMMAND identityMismatch=true";
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_INVALID_COMMAND;
	}

	// 조회와 Enqueue를 Transfer_SessionWorld에 대해 하나의 잠금 구간에 둔다.
	// 두 경로 모두 m_SessionsMutex → CGameRoom::m_CommandMutex 순서로 잠근다.
	std::scoped_lock lock{m_SessionsMutex};
	const auto iter = m_GameplayBindingBySessionId.find(sessionId);
	if (iter == m_GameplayBindingBySessionId.end() || nullptr == iter->second.pSimulation)
	{
		outContext = "enqueueResult=REJECTED_BINDING_MISSING";
		return ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_BINDING_MISSING;
	}
	const ROOM_COMMAND_ENQUEUE_RESULT result = iter->second.pSimulation->Enqueue_Detailed(std::move(command));
	outContext = iter->second.pSimulation->Describe_EnqueueResult(result);
	return result;
}

// 수신·종료 경로와 지연 입장 재처리가 만든 제어 이벤트의 소유권을 bounded 큐로 옮긴다.
// 큐 잠금은 삽입에만 사용하고 트랜잭션 검증·확정은 룸 worker가 다음 경계에서 진행한다.
bool LostArk::Server::CServerApp::Queue_ServerControlEvent(SERVER_CONTROL_EVENT&& event)
{
	std::scoped_lock lock{m_ServerControlMutex};
	if (m_ServerControlEvents.size() >= MAX_SERVER_CONTROL_EVENTS)
		return false;
	m_ServerControlEvents.push_back(std::move(event));
	return true;
}

// 검증 코드도 실제 후보 경로 admission과 같은 정규 파일 검사를 사용하도록 공개한 얇은 진입점이다.
// 선언은 private이며 friend인 CServerGameplayContractRunner가 접근한다. 제품 외부에 노출한 public API는 아니다.
bool LostArk::Server::CServerApp::Resolve_CandidateArtifactForAdmission(const std::filesystem::path& candidateDirectory,
                                                                        const std::string& relativePath,
                                                                        std::filesystem::path& resolvedPath,
                                                                        std::string& status)
{
	return Resolve_ExactRegularFile(candidateDirectory, relativePath, resolvedPath, status);
}

// 후보 검증과 계약 검사에서 동일한 비발탄 bootstrap 행 해시를 계산한다. 파일 내용은 변경하지 않는다.
bool LostArk::Server::CServerApp::Build_NonValtanGameplayRevisionForAdmission(
    const std::filesystem::path& bootstrapPath, LostArk::Shared::GameplayDataRevision& revision, std::string& status)
{
	return Build_NonValtanGameplayRevision(bootstrapPath, revision, status);
}

// 계약 검사가 실제 artifact와 같은 SHA-256 계산을 사용하도록 연결한다. 성공 시에만 revision을 갱신한다.
bool LostArk::Server::CServerApp::Hash_GameplayFileForAdmission(const std::filesystem::path& path,
                                                                LostArk::Shared::GameplayDataRevision& revision,
                                                                std::string& status)
{
	return Hash_FileSha256(path, revision, status);
}

// 후보 admission과 시작 복구가 현재 엔티티 상태에 복사된 발탄 기본값 변경을 차단한다.
// 허용되지 않는 값 차이는 status로 ENCOUNTER_RESET 필요를 알리고 profile을 수정하지 않는다.
bool LostArk::Server::CServerApp::Validate_ValtanHotReloadBaseProfile(const BOSS_RUNTIME_PROFILE* activeProfile,
                                                                      const BOSS_RUNTIME_PROFILE* candidateProfile,
                                                                      std::string& status)
{
	// 이 값들은 spawn 때 엔티티의 체력·전투·이동 상태에 복사된다.
	// 불변 정의만 교체해서는 이미 생성된 엔티티에 적용되지 않으므로 변경을 거부한다.
	if (nullptr == activeProfile || nullptr == candidateProfile)
	{
		status = "Valtan HOT_RELOAD base profile is unavailable; "
		         "ENCOUNTER_RESET required";
		return false;
	}
	if (activeProfile->iMaximumHp != candidateProfile->iMaximumHp ||
	    activeProfile->iDamageReferenceHp != candidateProfile->iDamageReferenceHp ||
	    activeProfile->iMaximumHealthBars != candidateProfile->iMaximumHealthBars ||
	    activeProfile->iAttackPower != candidateProfile->iAttackPower ||
	    activeProfile->fCollisionRadius != candidateProfile->fCollisionRadius ||
	    activeProfile->fEngageDistance != candidateProfile->fEngageDistance ||
	    activeProfile->fMoveSpeed != candidateProfile->fMoveSpeed)
	{
		status = "Candidate changes live Valtan base fields; "
		         "ENCOUNTER_RESET required";
		return false;
	}
	status.clear();
	return true;
}

// 룸 worker가 참여 세션에 transaction/base/candidate/lane 식별자를 reliable 송신 큐로 넣는다.
// true는 직렬화·큐 등록 성공이며 클라이언트의 준비 완료나 실제 송신 완료가 아니다.
bool LostArk::Server::CServerApp::Send_DataRevisionPrepare(
    const std::shared_ptr<CClientSession>& session, const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& request)
{
	using namespace LostArk::Shared;
	if (nullptr == session)
		return false;
	S2C_DATA_REVISION_PREPARE prepare{};
	prepare.iTransactionSequence = request.iTransactionSequence;
	prepare.BaseRevision = request.BaseRevision;
	prepare.CandidateRevision = request.CandidateRevision;
	prepare.iRequiredPresentationLaneMask = request.iRequiredPresentationLaneMask;
	CPacketWriter writer;
	return Write_Message(writer, prepare) &&
	       session->Send_Frame(PACKET_TYPE::S2C_DATA_REVISION_PREPARE, writer.Get_Buffer());
}

// 확정 결과에 후보와 현재 활성 revision을 함께 실어 재시도 응답을 판별하게 한다.
// COMMITTED 이유는 비우고 거부 이유는 프로토콜 바이트 상한에 맞춰 직렬화한다. 송신 실패는 호출자가 처리한다.
bool LostArk::Server::CServerApp::Send_DataRevisionResult(
    const std::shared_ptr<CClientSession>& session, const LostArk::Shared::C2S_DATA_REVISION_PREPARE_REQUEST& request,
    const LostArk::Shared::DATA_REVISION_RESULT result, const LostArk::Shared::GameplayDataRevision& activeRevision,
    std::string reason)
{
	using namespace LostArk::Shared;
	if (nullptr == session)
		return false;
	if (DATA_REVISION_RESULT::COMMITTED == result)
		reason.clear();
	else
	{
		if (reason.empty())
			reason = "Data revision transaction aborted";
		if (reason.size() > MAX_DATA_REVISION_REASON_BYTES)
			reason.resize(MAX_DATA_REVISION_REASON_BYTES);
	}
	S2C_DATA_REVISION_RESULT message{};
	message.iTransactionSequence = request.iTransactionSequence;
	message.CandidateRevision = request.CandidateRevision;
	message.ActiveRevision = activeRevision;
	message.eResult = result;
	message.strReason = std::move(reason);
	CPacketWriter writer;
	return Write_Message(writer, message) &&
	       session->Send_Frame(PACKET_TYPE::S2C_DATA_REVISION_RESULT, writer.Get_Buffer());
}

// 룸 worker가 현재 바인딩 룸의 발탄 의사결정 진단을 읽고 해당 세션에만 응답한다.
// 바인딩이 없으면 typed 거절, 응답 구성·송신 실패면 세션 종료 요청으로 처리한다.
void LostArk::Server::CServerApp::Process_ValtanDecisionTraceQuery(
    const SESSION_ID sessionId, const LostArk::Shared::C2S_VALTAN_DECISION_TRACE_QUERY& request)
{
	using namespace LostArk::Shared;
	std::shared_ptr<CClientSession> session;
	std::shared_ptr<CGameRoom> simulation;
	{
		std::scoped_lock lock{m_SessionsMutex};
		const auto sessionIter = m_Sessions.find(sessionId);
		const auto bindingIter = m_GameplayBindingBySessionId.find(sessionId);
		if (m_Sessions.end() != sessionIter)
			session = sessionIter->second;
		if (m_GameplayBindingBySessionId.end() != bindingIter)
			simulation = bindingIter->second.pSimulation;
	}
	if (nullptr == session)
		return;
	S2C_VALTAN_DECISION_TRACE_RESPONSE response{};
	response.iRequestSequence = request.iRequestSequence;
	response.strBossPlacementId = request.strBossPlacementId;
	std::string status;
	if (nullptr == simulation)
		response.eResult = VALTAN_DECISION_TRACE_QUERY_RESULT::REJECTED_WRONG_WORLD;
	else if (!simulation->Build_ValtanDecisionTraceResponse(request, response, status))
	{
		std::cerr << "Valtan decision trace query failed closed. SessionId=" << sessionId << ", Reason=" << status
		          << '\n';
		Request_SessionClose(sessionId);
		return;
	}
	CPacketWriter writer;
	if (!Write_Message(writer, response) ||
	    !session->Send_Frame(PACKET_TYPE::S2C_VALTAN_DECISION_TRACE_RESPONSE, writer.Get_Buffer()))
		Request_SessionClose(sessionId);
}

// 준비 시작 때 캡처한 참여 세션·바인딩·룸 집합과 현재 세대가 계속 같은지 검사한다.
// 룸 worker의 트랜잭션 상태를 읽으며 세션 맵은 잠금으로 보호한다. 차이가 있으면 확정하지 않고 이유를 돌려준다.
bool LostArk::Server::CServerApp::Validate_DataRevisionTransactionMembership(std::string& status)
{
	if (!m_DataRevisionTransaction.Is_Active())
	{
		status = "Data revision transaction is inactive";
		return false;
	}
	std::scoped_lock lock{m_SessionsMutex};
	if (nullptr == m_pActiveGameplayGeneration ||
	    m_pActiveGameplayGeneration->Get_ActiveRevision() != m_DataRevisionTransaction.Request.BaseRevision ||
	    m_ActiveGameplayBootstrapContentRevision != m_DataRevisionTransaction.BaseBootstrapContentRevision ||
	    m_ActiveNonValtanGameplayRevision != m_DataRevisionTransaction.BaseNonValtanGameplayRevision ||
	    m_ActiveNonValtanGameplayRevision != m_DataRevisionTransaction.CandidateNonValtanGameplayRevision)
	{
		status = "Process active gameplay generation changed during prepare";
		return false;
	}
	std::size_t boundParticipantCount = 0u;
	for (const auto& [sessionId, binding] : m_GameplayBindingBySessionId)
	{
		if (m_Sessions.contains(sessionId) && nullptr != binding.pSimulation)
			++boundParticipantCount;
	}
	if (boundParticipantCount != m_DataRevisionTransaction.Participants.size())
	{
		status = "Bound data revision participant set changed during prepare";
		return false;
	}
	for (const DATA_REVISION_PARTICIPANT& participant : m_DataRevisionTransaction.Participants)
	{
		const auto sessionIter = m_Sessions.find(participant.iSessionId);
		const auto bindingIter = m_GameplayBindingBySessionId.find(participant.iSessionId);
		if (m_Sessions.end() == sessionIter || sessionIter->second != participant.pSession ||
		    m_GameplayBindingBySessionId.end() == bindingIter || bindingIter->second.eWorldId != participant.eWorldId ||
		    bindingIter->second.pSimulation != participant.pSimulation)
		{
			status = "Data revision participant disconnected or changed rooms";
			return false;
		}
	}

	// 참여 세션 수만 같아도 private arena나 경기 룸이 교체될 수 있다.
	// 실제 프로세스 소유 룸 포인터 집합도 준비 당시와 대조한다.
	std::set<const CGameRoom*> currentRooms;
	for (const auto& [worldId, room] : m_SharedGameRooms)
	{
		(void)worldId;
		if (nullptr != room)
			currentRooms.insert(room.get());
	}
	for (const auto& [sessionId, room] : m_CharacterSelectArenas)
	{
		(void)sessionId;
		if (nullptr != room)
			currentRooms.insert(room.get());
	}
	for (const auto& [matchId, room] : m_ColosseumMatches)
	{
		(void)matchId;
		if (nullptr != room)
			currentRooms.insert(room.get());
	}
	if (currentRooms.size() != m_DataRevisionTransaction.Simulations.size())
	{
		status = "Process gameplay room set changed during prepare";
		return false;
	}
	for (const auto& room : m_DataRevisionTransaction.Simulations)
	{
		if (nullptr == room || !currentRooms.contains(room.get()))
		{
			status = "Prepared gameplay room is no longer process-owned";
			return false;
		}
	}
	status.clear();
	return true;
}

// 활성 트랜잭션을 먼저 지역 값으로 옮겨 비활성화하고 준비된 룸을 취소한다.
// 아직 응답하지 않은 참여자는 10초 tombstone으로 기록해 이미 전송된 응답 하나를 안전하게 소모한다.
// 세션 알림에는 현재 활성 revision을 사용하고 송신 실패는 해당 세션 종료로 정리한다.
void LostArk::Server::CServerApp::Abort_DataRevisionTransaction(std::string reason)
{
	using namespace LostArk::Shared;
	if (!m_DataRevisionTransaction.Is_Active())
		return;
	DATA_REVISION_TRANSACTION transaction = std::move(m_DataRevisionTransaction);
	m_DataRevisionTransaction = {};

	// 취소 직전에 클라이언트가 이미 보낸 응답은 네트워크에서 늦게 도착할 수 있다.
	// 응답을 기다리던 참여자만 제한된 수·수명의 기록으로 남긴다.
	const auto tombstoneExpiry = std::chrono::steady_clock::now() + std::chrono::seconds(10);
	for (const DATA_REVISION_PARTICIPANT& participant : transaction.Participants)
	{
		if (participant.hasResponded)
			continue;
		const auto existing =
		    std::find_if(m_DataRevisionResponseTombstones.begin(), m_DataRevisionResponseTombstones.end(),
		                 [&transaction, &participant](const DATA_REVISION_RESPONSE_TOMBSTONE& tombstone)
		                 {
			                 return tombstone.iSessionId == participant.iSessionId &&
			                        tombstone.iTransactionSequence == transaction.Request.iTransactionSequence &&
			                        tombstone.CandidateRevision == transaction.Request.CandidateRevision;
		                 });
		if (m_DataRevisionResponseTombstones.end() != existing)
		{
			existing->ExpiresAt = tombstoneExpiry;
			continue;
		}
		if (m_DataRevisionResponseTombstones.size() >= MAX_DATA_REVISION_RESPONSE_TOMBSTONES)
		{
			m_DataRevisionResponseTombstones.pop_front();
		}
		m_DataRevisionResponseTombstones.push_back(
		    {participant.iSessionId, transaction.Request.iTransactionSequence, transaction.Request.CandidateRevision,
		     transaction.Request.iRequiredPresentationLaneMask, tombstoneExpiry});
	}
	for (const auto& simulation : transaction.Simulations)
		if (nullptr != simulation)
			simulation->Abort_GameplayGeneration(transaction.Request.iTransactionSequence);
	GameplayDataRevision activeRevision = transaction.Request.BaseRevision;
	{
		std::scoped_lock lock{m_SessionsMutex};
		if (nullptr != m_pActiveGameplayGeneration)
			activeRevision = m_pActiveGameplayGeneration->Get_ActiveRevision();
	}
	if (reason.empty())
		reason = "Data revision transaction aborted";
	for (const DATA_REVISION_PARTICIPANT& participant : transaction.Participants)
	{
		if (!Send_DataRevisionResult(participant.pSession, transaction.Request, DATA_REVISION_RESULT::ABORTED,
		                             activeRevision, reason))
			Request_SessionClose(participant.iSessionId);
	}
}

// 룸 worker의 tick 경계에서 전 룸 사전검사 → 디스크 후보 포인터 → 참여 집합 재검사 → 전 룸 교체를 수행한다.
// 디스크 I/O 동안 세션 맵 잠금은 풀며, 재검사 실패면 디스크 base 복구 후 취소한다.
// 일부 룸이 교체된 뒤의 실패나 재검사 후 base 복구 실패는 정상 ABORT로 숨길 수 없어 프로세스를 종료한다.
bool LostArk::Server::CServerApp::Commit_DataRevisionTransaction()
{
	using namespace LostArk::Shared;
	if (!m_DataRevisionTransaction.Is_Active())
		return false;
	std::string status;
	if (!Validate_DataRevisionTransactionMembership(status))
	{
		Abort_DataRevisionTransaction(std::move(status));
		return false;
	}

	RUNTIME_ACTIVE_GAMEPLAY_GENERATION baseRuntime{};
	baseRuntime.eSource = m_isActiveGameplayGenerationFromCandidate
	                          ? RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE
	                          : RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE;
	baseRuntime.Revision = m_DataRevisionTransaction.Request.BaseRevision;
	baseRuntime.BootstrapContentRevision = m_DataRevisionTransaction.BaseBootstrapContentRevision;
	baseRuntime.NonValtanGameplayRevision = m_DataRevisionTransaction.BaseNonValtanGameplayRevision;
	RUNTIME_ACTIVE_GAMEPLAY_GENERATION candidateRuntime{};
	candidateRuntime.eSource = RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE;
	candidateRuntime.Revision = m_DataRevisionTransaction.Request.CandidateRevision;
	candidateRuntime.BootstrapContentRevision = m_DataRevisionTransaction.CandidateBootstrapContentRevision;
	candidateRuntime.NonValtanGameplayRevision = m_DataRevisionTransaction.CandidateNonValtanGameplayRevision;

	bool stageSucceeded = true;
	{
		// 전 룸의 확정 사전검사 동안 등록을 막는다.
		// 영속 포인터 I/O는 이 잠금을 해제한 뒤 수행한다.
		std::scoped_lock lock{m_SessionsMutex};
		if (nullptr == m_pActiveGameplayGeneration ||
		    m_pActiveGameplayGeneration->Get_ActiveRevision() != m_DataRevisionTransaction.Request.BaseRevision)
		{
			status = "Process active generation changed before room commit";
			stageSucceeded = false;
		}
		for (const auto& simulation : m_DataRevisionTransaction.Simulations)
		{
			if (!stageSucceeded)
				break;
			std::string roomStatus;
			if (nullptr == simulation ||
			    !simulation->Stage_GameplayGeneration(m_DataRevisionTransaction.Request.iTransactionSequence,
			                                          m_DataRevisionTransaction.Request.BaseRevision,
			                                          m_DataRevisionTransaction.pCandidateGeneration, roomStatus))
			{
				status = roomStatus.empty() ? "A process gameplay room rejected generation stage" : roomStatus;
				stageSucceeded = false;
			}
		}
		if (!stageSucceeded)
			for (const auto& simulation : m_DataRevisionTransaction.Simulations)
				if (nullptr != simulation)
					simulation->Abort_GameplayGeneration(m_DataRevisionTransaction.Request.iTransactionSequence);
	}
	if (!stageSucceeded)
	{
		Abort_DataRevisionTransaction(std::move(status));
		return false;
	}

	const std::filesystem::path runtimeRoot = Resolve_RuntimeActiveGameplayRoot();

	// 세션 맵 잠금 밖에서 디스크를 쓴다. 완료 뒤에는 메모리 참여 집합을 다시 확인해야 한다.
	bool durablePointerPromoted = false;
	if (m_isRuntimeActivePersistenceEnabled)
	{
		if (!Persist_RuntimeGameplayActivation(runtimeRoot, m_DataRevisionTransaction.Request.iTransactionSequence,
		                                       baseRuntime, candidateRuntime, status))
		{
			for (const auto& simulation : m_DataRevisionTransaction.Simulations)
				if (nullptr != simulation)
					simulation->Abort_GameplayGeneration(m_DataRevisionTransaction.Request.iTransactionSequence);
			Abort_DataRevisionTransaction(status.empty() ? "Runtime active pointer promotion failed"
			                                             : std::move(status));
			return false;
		}
		durablePointerPromoted = true;
	}

	bool cohortStillExact = true;
	{
		// 프로세스 전체 룸의 tick 경계에서 세대를 공개한다. 디스크 I/O 뒤 등록을 막고
		// 참여 집합을 재검사한 다음 준비한 포인터를 교체한다. 파일·소켓 I/O는 잠금 밖에서 처리한다.
		std::scoped_lock lock{m_SessionsMutex};
		if (nullptr == m_pActiveGameplayGeneration ||
		    m_pActiveGameplayGeneration->Get_ActiveRevision() != m_DataRevisionTransaction.Request.BaseRevision ||
		    m_ActiveGameplayBootstrapContentRevision != m_DataRevisionTransaction.BaseBootstrapContentRevision ||
		    m_ActiveNonValtanGameplayRevision != m_DataRevisionTransaction.BaseNonValtanGameplayRevision)
		{
			cohortStillExact = false;
			status = "Process active generation changed during durable promotion";
		}
		std::size_t boundCount = 0u;
		for (const auto& [sessionId, binding] : m_GameplayBindingBySessionId)
			if (m_Sessions.contains(sessionId) && nullptr != binding.pSimulation)
				++boundCount;
		if (boundCount != m_DataRevisionTransaction.Participants.size())
		{
			cohortStillExact = false;
			status = "Bound participant set changed during durable promotion";
		}
		for (const DATA_REVISION_PARTICIPANT& participant : m_DataRevisionTransaction.Participants)
		{
			const auto session = m_Sessions.find(participant.iSessionId);
			const auto binding = m_GameplayBindingBySessionId.find(participant.iSessionId);
			if (m_Sessions.end() == session || session->second != participant.pSession ||
			    m_GameplayBindingBySessionId.end() == binding || binding->second.eWorldId != participant.eWorldId ||
			    binding->second.pSimulation != participant.pSimulation)
			{
				cohortStillExact = false;
				status = "Prepared participant changed during durable promotion";
				break;
			}
		}
		std::set<const CGameRoom*> currentRooms;
		for (const auto& [worldId, room] : m_SharedGameRooms)
		{
			(void)worldId;
			if (nullptr != room)
				currentRooms.insert(room.get());
		}
		for (const auto& [sessionId, room] : m_CharacterSelectArenas)
		{
			(void)sessionId;
			if (nullptr != room)
				currentRooms.insert(room.get());
		}
		for (const auto& [matchId, room] : m_ColosseumMatches)
		{
			(void)matchId;
			if (nullptr != room)
				currentRooms.insert(room.get());
		}
		if (currentRooms.size() != m_DataRevisionTransaction.Simulations.size())
		{
			cohortStillExact = false;
			status = "Process room set changed during durable promotion";
		}
		for (const auto& simulation : m_DataRevisionTransaction.Simulations)
			if (nullptr == simulation || !currentRooms.contains(simulation.get()))
			{
				cohortStillExact = false;
				status = "Prepared room left during durable promotion";
				break;
			}

		if (!cohortStillExact)
		{
			for (const auto& simulation : m_DataRevisionTransaction.Simulations)
				if (nullptr != simulation)
					simulation->Abort_GameplayGeneration(m_DataRevisionTransaction.Request.iTransactionSequence);
		}
		else
		{
			for (const auto& simulation : m_DataRevisionTransaction.Simulations)
			{
				if (!simulation->Commit_GameplayGeneration(m_DataRevisionTransaction.Request.iTransactionSequence))
				{
					// 영속화 사용 시 디스크는 이미 새 세대를 가리킨다. 룸별 메모리 세대가 갈린
					// 상태를 ABORT로 숨기지 않고 중단해 재시작 복구가 디스크 세대를 선택하게 한다.
					std::cerr << "FATAL: partial gameplay generation commit" << '\n';
					std::terminate();
				}
			}
			m_pActiveGameplayGeneration = m_DataRevisionTransaction.pCandidateGeneration;
			m_ActiveGameplayBootstrapContentRevision = m_DataRevisionTransaction.CandidateBootstrapContentRevision;
			m_ActiveNonValtanGameplayRevision = m_DataRevisionTransaction.CandidateNonValtanGameplayRevision;
			m_isActiveGameplayGenerationFromCandidate = true;
		}
	}

	// 디스크 쓰기 중 연결이나 룸 구성이 바뀌면 새 메모리 세대를 공개하지 않는다.
	// 이미 교체한 포인터는 base로 보상하고, 보상 실패는 프로세스를 중단해 숨기지 않는다.
	if (!cohortStillExact)
	{
		if (durablePointerPromoted)
		{
			std::string rollbackStatus;
			if (!Rollback_RuntimeGameplayActivation(runtimeRoot, baseRuntime, rollbackStatus))
			{
				std::cerr << "FATAL: runtime gameplay pointer rollback failed: " << rollbackStatus << '\n';
				std::terminate();
			}
		}
		Abort_DataRevisionTransaction(std::move(status));
		return false;
	}
	if (durablePointerPromoted)
		Complete_RuntimeGameplayActivation(runtimeRoot);

	DATA_REVISION_TRANSACTION transaction = std::move(m_DataRevisionTransaction);
	m_DataRevisionTransaction = {};
	const GameplayDataRevision committedRevision = transaction.pCandidateGeneration->Get_ActiveRevision();
	for (const DATA_REVISION_PARTICIPANT& participant : transaction.Participants)
	{
		if (!Send_DataRevisionResult(participant.pSession, transaction.Request, DATA_REVISION_RESULT::COMMITTED,
		                             committedRevision, {}))
			Request_SessionClose(participant.iSessionId);
	}
	return true;
}

// 매 fixed step 앞에서 수신 제어 큐를 지역 큐로 옮겨 처리하고 numeric worker 완료를 반영한다.
// 발탄 준비 응답은 캡처된 참여자·세대·lane과 대조하며, 전원 준비·동일 멤버십·기한 조건을 만족해야 확정한다.
// 수신 worker가 게임플레이 세대를 직접 교체하지 않게 하는 단일 진행 지점이다.
void LostArk::Server::CServerApp::Advance_ServerControlTransactions()
{
	using namespace LostArk::Shared;
	Advance_NumericBalanceTransaction();

	// 공유 큐를 빠르게 비운 뒤 잠금 밖에서 처리한다. 처리 중 도착한 이벤트는 다음 fixed step으로 남는다.
	std::deque<SERVER_CONTROL_EVENT> events;
	{
		std::scoped_lock lock{m_ServerControlMutex};
		events.swap(m_ServerControlEvents);
	}
	const auto now = std::chrono::steady_clock::now();
	m_DataRevisionResponseTombstones.erase(
	    std::remove_if(m_DataRevisionResponseTombstones.begin(), m_DataRevisionResponseTombstones.end(),
	                   [now](const DATA_REVISION_RESPONSE_TOMBSTONE& tombstone) { return tombstone.ExpiresAt <= now; }),
	    m_DataRevisionResponseTombstones.end());
	const auto abortCurrent = [this](std::string reason) { Abort_DataRevisionTransaction(std::move(reason)); };
	for (SERVER_CONTROL_EVENT& event : events)
	{
		// 저장 중 미룬 입장은 아직 pause 상태면 다시 큐에 남기고, 해제되면 원래 프레임 경로로 검증한다.
		if (event.eKind == SERVER_CONTROL_EVENT_KIND::BALANCE_DEFERRED_ENTRY)
		{
			On_SessionFrame(event.iSessionId, event.DeferredEntry);
			continue;
		}
		if (event.eKind == SERVER_CONTROL_EVENT_KIND::BALANCE_QUERY ||
		    event.eKind == SERVER_CONTROL_EVENT_KIND::BALANCE_PATCH)
		{
			Process_NumericBalanceEvent(event);
			continue;
		}
		if (SERVER_CONTROL_EVENT_KIND::VALTAN_DECISION_TRACE_QUERY == event.eKind)
		{
			Process_ValtanDecisionTraceQuery(event.iSessionId, event.DecisionTraceQuery);
			continue;
		}
		if (SERVER_CONTROL_EVENT_KIND::SESSION_DISCONNECTED == event.eKind)
		{
			if (m_DataRevisionTransaction.Is_Active() &&
			    std::any_of(m_DataRevisionTransaction.Participants.begin(),
			                m_DataRevisionTransaction.Participants.end(),
			                [&event](const DATA_REVISION_PARTICIPANT& participant)
			                { return participant.iSessionId == event.iSessionId; }))
			{
				abortCurrent("A prepared client disconnected before commit");
			}
			continue;
		}

		// 진행 중인 세대 승인의 응답은 캡처한 참여자와 sequence·revision·lane 전체를 대조한다.
		// 활성 승인이 없을 때는 중단된 승인에 남긴 tombstone과 일치하는 늦은 응답 한 번만 허용한다.
		if (SERVER_CONTROL_EVENT_KIND::DATA_REVISION_RESPONSE == event.eKind)
		{
			const C2S_DATA_REVISION_PREPARE_RESPONSE& response = event.RevisionResponse;
			if (!m_DataRevisionTransaction.Is_Active())
			{
				const auto answeredAbortedTransaction = std::find_if(
				    m_DataRevisionResponseTombstones.begin(), m_DataRevisionResponseTombstones.end(),
				    [&event, &response](const DATA_REVISION_RESPONSE_TOMBSTONE& identity)
				    {
					    const bool validReady =
					        DATA_REVISION_PREPARE_STATUS::READY == response.eStatus &&
					        (response.iPreparedPresentationLaneMask & identity.iRequiredPresentationLaneMask) ==
					            identity.iRequiredPresentationLaneMask &&
					        0u == (response.iFailedPresentationLaneMask & identity.iRequiredPresentationLaneMask);
					    const bool validNack = DATA_REVISION_PREPARE_STATUS::NACK == response.eStatus;
					    return (validReady || validNack) && identity.iSessionId == event.iSessionId &&
					           identity.iTransactionSequence == response.iTransactionSequence &&
					           identity.CandidateRevision == response.CandidateRevision &&
					           identity.iRequiredPresentationLaneMask == response.iRequiredPresentationLaneMask;
				    });
				if (m_DataRevisionResponseTombstones.end() == answeredAbortedTransaction)
				{
					Request_SessionClose(event.iSessionId);
				}
				else
				{
					// 전송 당시 유효했던 늦은 응답 하나만 소모한다. 이후 중복 응답은
					// 요청하지 않은 응답이므로 기존 정책대로 세션을 종료한다.
					m_DataRevisionResponseTombstones.erase(answeredAbortedTransaction);
				}
				continue;
			}
			auto participant = std::find_if(
			    m_DataRevisionTransaction.Participants.begin(), m_DataRevisionTransaction.Participants.end(),
			    [&event](const DATA_REVISION_PARTICIPANT& value) { return value.iSessionId == event.iSessionId; });
			if (m_DataRevisionTransaction.Participants.end() == participant || participant->hasResponded ||
			    response.iTransactionSequence != m_DataRevisionTransaction.Request.iTransactionSequence ||
			    response.CandidateRevision != m_DataRevisionTransaction.Request.CandidateRevision ||
			    response.iRequiredPresentationLaneMask !=
			        m_DataRevisionTransaction.Request.iRequiredPresentationLaneMask)
			{
				abortCurrent("Duplicate, stale, or mismatched client prepare response");
				Request_SessionClose(event.iSessionId);
				continue;
			}
			participant->hasResponded = true;
			if (DATA_REVISION_PREPARE_STATUS::NACK == response.eStatus)
			{
				abortCurrent(response.strReason.empty() ? "A client rejected the candidate presentation generation"
				                                        : response.strReason);
				continue;
			}
			const std::uint32_t required = m_DataRevisionTransaction.Request.iRequiredPresentationLaneMask;
			if ((response.iPreparedPresentationLaneMask & required) != required ||
			    0u != (response.iFailedPresentationLaneMask & required))
			{
				abortCurrent("Client READY did not prepare every required presentation lane");
				Request_SessionClose(event.iSessionId);
				continue;
			}
			participant->isReady = true;
			continue;
		}
		if (SERVER_CONTROL_EVENT_KIND::DATA_REVISION_REQUEST != event.eKind)
			continue;

		if (m_DataRevisionTransaction.Is_Active() || m_NumericBalanceRequester != INVALID_SESSION_ID)
		{
			const bool exactDuplicate =
			    m_DataRevisionTransaction.iRequesterSessionId == event.iSessionId &&
			    m_DataRevisionTransaction.Request.iTransactionSequence == event.RevisionRequest.iTransactionSequence &&
			    m_DataRevisionTransaction.Request.CandidateRevision == event.RevisionRequest.CandidateRevision;
			if (exactDuplicate)
				continue;
			std::shared_ptr<CClientSession> requester;
			GameplayDataRevision active{};
			{
				std::scoped_lock lock{m_SessionsMutex};
				const auto iter = m_Sessions.find(event.iSessionId);
				if (m_Sessions.end() != iter)
					requester = iter->second;
				if (nullptr != m_pActiveGameplayGeneration)
					active = m_pActiveGameplayGeneration->Get_ActiveRevision();
			}
			if (active.Is_Valid() && event.RevisionRequest.CandidateRevision != active &&
			    !Send_DataRevisionResult(requester, event.RevisionRequest, DATA_REVISION_RESULT::ABORTED, active,
			                             "Another gameplay data revision transaction is active"))
				Request_SessionClose(event.iSessionId);
			continue;
		}

		DATA_REVISION_TRANSACTION staged{};
		staged.iRequesterSessionId = event.iSessionId;
		staged.Request = event.RevisionRequest;
		staged.pCandidateGeneration = std::move(event.pCandidateGeneration);
		staged.BaseBootstrapContentRevision = event.BaseBootstrapContentRevision;
		staged.CandidateBootstrapContentRevision = event.CandidateBootstrapContentRevision;
		staged.BaseNonValtanGameplayRevision = event.BaseNonValtanGameplayRevision;
		staged.CandidateNonValtanGameplayRevision = event.CandidateNonValtanGameplayRevision;
		std::string beginStatus;
		{
			std::scoped_lock lock{m_SessionsMutex};
			const auto requesterSession = m_Sessions.find(event.iSessionId);
			const auto requesterBinding = m_GameplayBindingBySessionId.find(event.iSessionId);
			if (m_Sessions.end() == requesterSession || m_GameplayBindingBySessionId.end() == requesterBinding ||
			    WORLD_ID::VALTAN_ARENA != requesterBinding->second.eWorldId ||
			    nullptr == requesterBinding->second.pSimulation)
				beginStatus = "Data revision requester left Valtan Arena";
			else if (nullptr == m_pActiveGameplayGeneration ||
			         m_pActiveGameplayGeneration->Get_ActiveRevision() != staged.Request.BaseRevision)
				beginStatus = "Data revision base is no longer process active";
			else if (!staged.BaseBootstrapContentRevision.Is_Valid() ||
			         !staged.CandidateBootstrapContentRevision.Is_Valid() ||
			         m_ActiveGameplayBootstrapContentRevision != staged.BaseBootstrapContentRevision)
				beginStatus = "Candidate bootstrap baseline is no longer process active";
			else if (!staged.BaseNonValtanGameplayRevision.Is_Valid() ||
			         !staged.CandidateNonValtanGameplayRevision.Is_Valid() ||
			         m_ActiveNonValtanGameplayRevision != staged.BaseNonValtanGameplayRevision ||
			         m_ActiveNonValtanGameplayRevision != staged.CandidateNonValtanGameplayRevision)
				beginStatus = "Candidate changes gameplay outside allowedDomains VALTAN_BOSS";
			else if (nullptr == staged.pCandidateGeneration ||
			         staged.pCandidateGeneration->Get_ActiveRevision() != staged.Request.CandidateRevision)
				beginStatus = "Admitted candidate parent revision is inconsistent";
			else
			{
				// 세션 맵 잠금 안에서 승인 참여자와 적용할 모든 룸의 강한 참조를 고정한다.
				// 응답 대기 중 실제 binding이나 룸 집합이 바뀌면 뒤의 멤버십 재검사가 전체 확정을 거부한다.
				staged.Participants.reserve(m_GameplayBindingBySessionId.size());
				for (const auto& [participantId, binding] : m_GameplayBindingBySessionId)
				{
					const auto sessionIter = m_Sessions.find(participantId);
					if (m_Sessions.end() == sessionIter || nullptr == binding.pSimulation)
						continue;
					DATA_REVISION_PARTICIPANT participant{};
					participant.iSessionId = participantId;
					participant.eWorldId = binding.eWorldId;
					participant.pSession = sessionIter->second;
					participant.pSimulation = binding.pSimulation;
					staged.Participants.push_back(std::move(participant));
				}
				std::sort(staged.Participants.begin(), staged.Participants.end(),
				          [](const DATA_REVISION_PARTICIPANT& left, const DATA_REVISION_PARTICIPANT& right)
				          { return left.iSessionId < right.iSessionId; });
				for (const auto& [worldId, simulation] : m_SharedGameRooms)
				{
					(void)worldId;
					if (nullptr != simulation)
						staged.Simulations.push_back(simulation);
				}
				for (const auto& [ownerSessionId, simulation] : m_CharacterSelectArenas)
				{
					(void)ownerSessionId;
					if (nullptr != simulation)
						staged.Simulations.push_back(simulation);
				}
				for (const auto& [matchId, simulation] : m_ColosseumMatches)
				{
					(void)matchId;
					if (simulation)
						staged.Simulations.push_back(simulation);
				}
				const bool requesterParticipates = std::any_of(staged.Participants.begin(), staged.Participants.end(),
				                                               [&event](const DATA_REVISION_PARTICIPANT& participant)
				                                               { return participant.iSessionId == event.iSessionId; });
				if (!requesterParticipates || staged.Simulations.empty())
					beginStatus = "Data revision participant or room set is empty";
			}
		}
		if (!beginStatus.empty())
		{
			std::shared_ptr<CClientSession> requester;
			GameplayDataRevision active = staged.Request.BaseRevision;
			{
				std::scoped_lock lock{m_SessionsMutex};
				const auto iter = m_Sessions.find(event.iSessionId);
				if (m_Sessions.end() != iter)
					requester = iter->second;
				if (nullptr != m_pActiveGameplayGeneration)
					active = m_pActiveGameplayGeneration->Get_ActiveRevision();
			}
			if (active == staged.Request.CandidateRevision ||
			    !Send_DataRevisionResult(requester, staged.Request, DATA_REVISION_RESULT::ABORTED, active, beginStatus))
				Request_SessionClose(event.iSessionId);
			continue;
		}

		// 준비 요청은 무기한 대기하지 않는다. 캡처한 참여자 전원의 응답과 멤버십을
		// 제한 시간 안에 만족해야만 아래의 tick 경계 확정으로 진행한다.
		staged.Deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
		m_DataRevisionTransaction = std::move(staged);
		bool prepareSent = true;
		for (const DATA_REVISION_PARTICIPANT& participant : m_DataRevisionTransaction.Participants)
		{
			if (!Send_DataRevisionPrepare(participant.pSession, m_DataRevisionTransaction.Request))
			{
				prepareSent = false;
				Request_SessionClose(participant.iSessionId);
				break;
			}
		}
		if (!prepareSent)
			abortCurrent("A participant could not receive the prepare message");
	}

	if (!m_DataRevisionTransaction.Is_Active())
		return;

	// 이번 큐의 응답을 모두 소비한 뒤 멤버십과 기한을 다시 검사한다.
	// 전원이 READY여도 기한이 지났으면 중단하며, 검사를 통과한 경우에만 다음 룸 tick 전에 확정한다.
	std::string membershipStatus;
	if (!Validate_DataRevisionTransactionMembership(membershipStatus))
	{
		abortCurrent(std::move(membershipStatus));
		return;
	}
	if (std::chrono::steady_clock::now() >= m_DataRevisionTransaction.Deadline)
	{
		abortCurrent("Data revision prepare timed out before every client was ready");
		return;
	}
	const bool allReady =
	    std::all_of(m_DataRevisionTransaction.Participants.begin(), m_DataRevisionTransaction.Participants.end(),
	                [](const DATA_REVISION_PARTICIPANT& participant) { return participant.isReady; });
	if (allReady)
		(void)Commit_DataRevisionTransaction();
}

// 제어 확정과 콜로세움 준비 완료를 먼저 반영하고 모든 현재 룸을 같은 fixed delta로 tick한다.
// 강한 참조 목록을 잠금 안에서 복사한 뒤 잠금 밖에서 룸을 실행한다. tick 이후 전환·빈 룸 회수를 처리한다.
void LostArk::Server::CServerApp::Tick_GameplaySimulations(const float fixedDeltaSeconds,
                                                           const SERVER_ROOM_SCHEDULER_METRICS& schedulerMetrics)
{
	// 어느 룸도 이번 fixed step을 소비하기 전에 제어 트랜잭션을 확정해
	// 모든 현재 룸이 같은 경계에서 세대를 바꾸게 한다.
	Advance_ServerControlTransactions();
	Advance_ColosseumPreparation();
	std::vector<std::shared_ptr<CGameRoom>> simulations;
	{
		std::scoped_lock lock{m_SessionsMutex};
		simulations.reserve(m_SharedGameRooms.size() + m_CharacterSelectArenas.size());
		for (const auto& [worldId, simulation] : m_SharedGameRooms)
		{
			(void)worldId;
			if (nullptr != simulation)
				simulations.push_back(simulation);
		}
		for (const auto& [sessionId, simulation] : m_CharacterSelectArenas)
		{
			(void)sessionId;
			if (nullptr != simulation)
				simulations.push_back(simulation);
		}
		for (const auto& [matchId, simulation] : m_ColosseumMatches)
		{
			(void)matchId;
			if (simulation)
				simulations.push_back(simulation);
		}
	}

	// 게임플레이 상태의 쓰기는 룸 worker가 맡는다. Tick 전에 세션 맵 잠금을 풀어
	// 수신·종료 worker가 룸 Tick 전체 동안 이 잠금을 기다리지 않게 한다.
	for (const std::shared_ptr<CGameRoom>& simulation : simulations)
	{
		simulation->Tick(fixedDeltaSeconds, schedulerMetrics);
		const std::string diagnostic = simulation->Take_PerformanceDiagnostic();
		if (diagnostic.empty())
			continue;
		bool written = false;
		try
		{
			auto path = Resolve_ServerSessionDiagnosticPath();
			if (!path.empty())
			{
				path.replace_filename(L"server-room-perf-" + std::to_wstring(::GetCurrentProcessId()) + L".log");
				written = Append_BoundedRoomPerformanceDiagnostic(path, diagnostic);
			}
		}
		catch (...)
		{
		} // 진단 경로 조회 실패가 게임플레이 진행을 중단하지 않게 한다.
		if (!written && !m_bRoomPerformanceLogWarningReported)
		{
			m_bRoomPerformanceLogWarningReported = true;
			std::cerr << "Room performance diagnostic file unavailable; stdout logging continues.\n";
		}
	}
	// 이번 tick에 이미 대기하던 입장 명령을 소비한 뒤 월드 전환을 받는다.
	for (const std::shared_ptr<CGameRoom>& simulation : simulations)
	{
		Handle_WorldTransfers(simulation);
	}
	Retire_QuiescentCharacterSelectArenas();
	Retire_QuiescentColosseumMatches();
}

// 소유 세션의 바인딩이 사라지고 퇴장·입력 큐 정리까지 끝난 private arena만 봉인 후 제거한다.
// 세션 종료 직후 바로 삭제하지 않아 이미 큐에 넣은 LEAVE를 룸이 처리할 수 있다.
void LostArk::Server::CServerApp::Retire_QuiescentCharacterSelectArenas()
{
	// 잠금은 ServerApp 세션 맵 → 룸 명령 큐 순서다. Try_Seal이 빈 상태 판정과
	// 수신 worker의 Enqueue를 직렬화해 회수하기로 한 룸에 새 명령이 들어가지 않게 한다.
	std::scoped_lock lock{m_SessionsMutex};
	for (auto iter = m_CharacterSelectArenas.begin(); iter != m_CharacterSelectArenas.end();)
	{
		const SESSION_ID ownerSessionId = iter->first;
		const std::shared_ptr<CGameRoom>& simulation = iter->second;
		const auto bindingIter = m_GameplayBindingBySessionId.find(ownerSessionId);
		const bool isStillBound =
		    bindingIter != m_GameplayBindingBySessionId.end() && bindingIter->second.pSimulation == simulation;

		if (isStillBound || nullptr == simulation || !simulation->Try_SealPrivateArenaForRetirement())
		{
			++iter;
			continue;
		}
		iter = m_CharacterSelectArenas.erase(iter);
	}
}

// Bern의 경기 전환 요청 하나를 보관하고 별도 worker가 불변 catalog로 목적 룸을 준비하게 한다.
// worker는 세션 맵을 수정하지 않으며 catalog·취소 atomic의 shared_ptr를 캡처해 수명을 유지한다.
// 시작 실패는 false로 알리고 원래 룸의 참가자는 아직 이동시키지 않는다.
bool LostArk::Server::CServerApp::Begin_ColosseumPreparation(const std::shared_ptr<CGameRoom>& source,
                                                             const SERVER_WORLD_TRANSFER_REQUEST& transfer)
{
	using namespace LostArk::Shared;
	if (!source || !transfer.bColosseumMatch || source->Get_WorldId() != WORLD_ID::BERN ||
	    transfer.eTargetWorldId != WORLD_ID::COLOSSEUM ||
	    (transfer.PartyBatchSessionIds.empty() || transfer.PartyBatchSessionIds.size() > MAX_COLOSSEUM_MATCH_PLAYERS) ||
	    m_ColosseumPreparationWorker.valid())
		return false;
	std::shared_ptr<const CGameplayCatalog> generation;
	{
		std::scoped_lock lock{m_SessionsMutex};
		generation = m_pActiveGameplayGeneration;
	}
	if (!generation)
		return false;
	try
	{
		auto cancelled = std::make_shared<std::atomic_bool>(false);
		m_ColosseumPreparationTransfer = transfer;
		m_ColosseumPreparationSource = source;
		m_ColosseumPreparationCancelled = cancelled;
		m_ColosseumPreparationWorker = std::async(
		    std::launch::async,
		    [generation = std::move(generation), cancelled]()
		    {
			    COLOSSEUM_PREPARATION_RESULT result;
			    const auto start = std::chrono::steady_clock::now();
			    try
			    {
				    if (!cancelled->load(std::memory_order_relaxed))
					    result.Room = std::make_shared<CGameRoom>(WORLD_ID::COLOSSEUM, generation, cancelled.get());
				    if (cancelled->load(std::memory_order_relaxed) || !result.Room || !result.Room->Is_Ready())
				    {
					    result.Status = result.Room ? result.Room->Get_Status() : "Colosseum preparation cancelled";
					    result.Room.reset(); // 실패한 준비 객체는 생성 worker에서 정리한다.
				    }
			    }
			    catch (const std::exception& error)
			    {
				    result.Status = error.what();
				    result.Room.reset();
			    }
			    catch (...)
			    {
				    result.Status = "Colosseum preparation failed";
				    result.Room.reset();
			    }
			    result.ElapsedMilliseconds =
			        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
			    return result;
		    });
		return true;
	}
	catch (...)
	{
		m_ColosseumPreparationSource.reset();
		m_ColosseumPreparationCancelled.reset();
		return false;
	}
}

// 룸 worker가 준비 중 참가자의 원래 바인딩을 재확인하고 이탈하면 협력 취소를 요청한다.
// future가 준비되었고 세대 확정이 입장을 막지 않을 때만 결과를 가져와 실제 전환을 시도한다.
// 성공·실패를 source에 통지한 뒤 준비 참조를 정리한다.
void LostArk::Server::CServerApp::Advance_ColosseumPreparation()
{
	if (!m_ColosseumPreparationWorker.valid())
		return;
	bool sourceBound = true;
	{
		std::scoped_lock lock{m_SessionsMutex};
		for (const auto sessionId : m_ColosseumPreparationTransfer.PartyBatchSessionIds)
		{
			const auto session = m_Sessions.find(sessionId);
			const auto binding = m_GameplayBindingBySessionId.find(sessionId);
			if (session == m_Sessions.end() || !session->second || session->second->Is_Closing() ||
			    binding == m_GameplayBindingBySessionId.end() ||
			    binding->second.pSimulation != m_ColosseumPreparationSource)
			{
				sourceBound = false;
				break;
			}
		}
	}
	if (!sourceBound && m_ColosseumPreparationCancelled)
		m_ColosseumPreparationCancelled->store(true, std::memory_order_relaxed);
	if (m_ColosseumPreparationWorker.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
		return;
	// 진행 중인 세대 확정이 입장을 관리한다. 이미 완료한 불변 결과는 보관해
	// 저장 중 같은 룸 준비를 버리고 반복 로드하지 않게 한다.
	if (sourceBound && (m_DataRevisionTransaction.Is_Active() || m_NumericAdmissionPaused))
		return;
	COLOSSEUM_PREPARATION_RESULT result;
	try
	{
		result = m_ColosseumPreparationWorker.get();
	}
	catch (...)
	{
		result.Status = "Colosseum preparation future failed";
	}
	m_PreparedColosseumRoom = std::move(result.Room);
	SESSION_WORLD_TRANSFER_FAILURE failure;
	const bool cancelled =
	    m_ColosseumPreparationCancelled && m_ColosseumPreparationCancelled->load(std::memory_order_relaxed);
	const bool committed = sourceBound && !cancelled && m_PreparedColosseumRoom &&
	                       Transfer_SessionWorld(m_ColosseumPreparationSource, m_ColosseumPreparationTransfer, failure);
	if (m_ColosseumPreparationSource)
		m_ColosseumPreparationSource->Notify_ColosseumTransferResult(committed);
	std::cout << "[ColosseumPreparation] workerMs=" << result.ElapsedMilliseconds
	          << " committed=" << (committed ? "true" : "false")
	          << " status=" << (result.Status.empty() ? failure.strContext : result.Status) << '\n';
	m_ColosseumPreparationSource.reset();
	m_ColosseumPreparationCancelled.reset();
	m_PreparedColosseumRoom.reset();
	m_ColosseumPreparationTransfer = {};
}

// 어느 세션에도 배정되지 않고 큐까지 비워 봉인할 수 있는 경기 룸만 소유 맵에서 제거한다.
void LostArk::Server::CServerApp::Retire_QuiescentColosseumMatches()
{
	std::scoped_lock lock{m_SessionsMutex};
	for (auto it = m_ColosseumMatches.begin(); it != m_ColosseumMatches.end();)
	{
		const auto& room = it->second;
		const bool bound = std::any_of(m_GameplayBindingBySessionId.begin(), m_GameplayBindingBySessionId.end(),
		                               [&room](const auto& binding) { return binding.second.pSimulation == room; });
		if (!bound && room && room->Try_SealColosseumForRetirement())
			it = m_ColosseumMatches.erase(it);
		else
			++it;
	}
}

// 각 룸 tick이 만든 전환 요청을 꺼내 콜로세움 준비 또는 즉시 전환으로 연결한다.
// source를 보존하는 파티 거절은 typed 실패로 알리고, 세션을 유지할 수 없는 실패는 종료 사유로 전달한다.
void LostArk::Server::CServerApp::Handle_WorldTransfers(const std::shared_ptr<CGameRoom>& sourceSimulation)
{
	if (nullptr == sourceSimulation)
		return;

	SERVER_WORLD_TRANSFER_REQUEST transfer{};
	while (sourceSimulation->Try_DequeueWorldTransfer(transfer))
	{
		if (transfer.bColosseumMatch)
		{
			if (!Begin_ColosseumPreparation(sourceSimulation, transfer))
				sourceSimulation->Notify_ColosseumTransferResult(false);
			continue;
		}
		SESSION_WORLD_TRANSFER_FAILURE failure{};
		const bool committed = Transfer_SessionWorld(sourceSimulation, transfer, failure);
		if (transfer.bColosseumMatch)
			sourceSimulation->Notify_ColosseumTransferResult(committed);
		if (!committed)
		{
			if (!transfer.PartyBatchSessionIds.empty() || failure.bSourcePreservedOnRejection)
			{
				std::cerr << "Party transfer rejected without source departure: " << failure.strContext << '\n';
				sourceSimulation->Notify_PartyTransferFailure(transfer.iSessionId, transfer.iPartyRequestSequence,
				                                              transfer.eTargetWorldId, failure.ePartyResult);
				continue;
			}
			Request_SessionClose(transfer.iSessionId, failure.eReason, failure.iNativeErrorCode, failure.strContext);
		}
	}
}

// 룸 worker가 source 바인딩과 목적 룸을 검증해 전환을 수행한다.
// 파티·가이드 owner 단독·콜로세움은 전원 admission과 초기 송신 FIFO를 준비한 뒤 source에서 나간다.
// 일반 단독 전환은 REGISTER/ENTER 큐 등록 뒤 source 퇴장·바인딩 변경을 진행하며 실패 종류를 outFailure로 돌려준다.
bool LostArk::Server::CServerApp::Transfer_SessionWorld(const std::shared_ptr<CGameRoom>& sourceSimulation,
                                                        const SERVER_WORLD_TRANSFER_REQUEST& transfer,
                                                        SESSION_WORLD_TRANSFER_FAILURE& outFailure)
{
	using namespace LostArk::Shared;
	outFailure = {};
	const auto setFailure =
	    [&outFailure, &sourceSimulation, &transfer](const SESSION_DIAGNOSTIC_REASON reason, const int nativeErrorCode,
	                                                const std::string_view stage, const bool rollbackRequired = false,
	                                                const bool rollbackEnqueued = false)
	{
		const WORLD_ID sourceWorldId = nullptr == sourceSimulation ? WORLD_ID::END : sourceSimulation->Get_WorldId();
		outFailure.eReason = reason;
		outFailure.iNativeErrorCode = nativeErrorCode;
		outFailure.bRollbackCleanupRequired = rollbackRequired;
		outFailure.bRollbackCleanupEnqueued = rollbackEnqueued;
		outFailure.strContext = "world transfer stage=" + std::string{stage} +
		                        " sourceWorld=" + std::to_string(static_cast<std::uint16_t>(sourceWorldId)) +
		                        " targetWorld=" + std::to_string(static_cast<std::uint16_t>(transfer.eTargetWorldId)) +
		                        " rollbackCleanupRequired=" + (rollbackRequired ? "true" : "false") +
		                        " rollbackCleanupEnqueued=" + (rollbackEnqueued ? "true" : "false");
	};

	if (nullptr == sourceSimulation || CHARACTER_CLASS_ID::END == transfer.eCharacterClass ||
	    transfer.strNickName.empty())
	{
		setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED, WSAEINVAL, "request-validation");
		return false;
	}

	const WORLD_ID sourceWorldId = sourceSimulation->Get_WorldId();
	if (transfer.bColosseumMatch)
	{
		std::scoped_lock lock{m_SessionsMutex};
		if (sourceWorldId != WORLD_ID::BERN || transfer.eTargetWorldId != WORLD_ID::COLOSSEUM ||
		    (transfer.PartyBatchSessionIds.empty() ||
		     transfer.PartyBatchSessionIds.size() > MAX_COLOSSEUM_MATCH_PLAYERS) ||
		    m_iNextColosseumMatchId == 0u || !m_pActiveGameplayGeneration || m_DataRevisionTransaction.Is_Active() ||
		    m_NumericAdmissionPaused)
		{
			setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_PREFLIGHT_FAILED, WSAEINVAL,
			           "Colosseum batch validation");
			return false;
		}
		for (const auto sessionId : transfer.PartyBatchSessionIds)
		{
			const auto session = m_Sessions.find(sessionId);
			const auto binding = m_GameplayBindingBySessionId.find(sessionId);
			if (session == m_Sessions.end() || !session->second || session->second->Is_Closing() ||
			    binding == m_GameplayBindingBySessionId.end() || binding->second.pSimulation != sourceSimulation)
			{
				setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_PREFLIGHT_FAILED, WSAENOTCONN,
				           "Colosseum source binding");
				return false;
			}
		}
		const auto target = m_PreparedColosseumRoom;
		if (!target || !target->Is_Ready() || target->Get_WorldId() != WORLD_ID::COLOSSEUM ||
		    target->Get_ActiveGameplayGeneration() != m_pActiveGameplayGeneration)
		{
			setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_PREFLIGHT_FAILED, WSAEINVAL,
			           "Colosseum prepared generation is unavailable or stale");
			return false;
		}

		// 이 경기의 고유 ID와 소유 맵을 먼저 준비하되 참가자 전환 실패면 추가한 맵 항목을 되돌린다.
		const auto matchId = m_iNextColosseumMatchId;
		const auto [entry, inserted] = m_ColosseumMatches.emplace(matchId, target);
		if (!inserted)
		{
			setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_PREFLIGHT_FAILED, WSAEINVAL, "Colosseum match identity");
			return false;
		}
		std::string status;
		if (!sourceSimulation->Transfer_ColosseumMatchTo(*target, transfer.PartyBatchSessionIds, matchId, status))
		{
			m_ColosseumMatches.erase(entry);
			setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_PREFLIGHT_FAILED, WSAEINVAL, status);
			return false;
		}
		for (const auto sessionId : transfer.PartyBatchSessionIds)
		{
			auto& binding = m_GameplayBindingBySessionId.at(sessionId);
			binding.eWorldId = WORLD_ID::COLOSSEUM;
			binding.iPrivateArenaOwnerSessionId = INVALID_SESSION_ID;
			binding.pSimulation = target;
		}
		++m_iNextColosseumMatchId;
		return true;
	}
	const std::shared_ptr<CGameRoom> targetSimulation = Find_SharedSimulation(transfer.eTargetWorldId);
	if (nullptr == targetSimulation || targetSimulation == sourceSimulation || sourceWorldId == transfer.eTargetWorldId)
	{
		setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED, WSAEINVAL, "target-resolution");
		return false;
	}

	// Enqueue_AssignedCommand와 On_SessionClosed도 같은 mutex를 사용한다.
	// 따라서 경로 조회·큐 등록·바인딩 확정이 하나의 순서로 진행된다.
	std::scoped_lock lock{m_SessionsMutex};
	const auto sessionIter = m_Sessions.find(transfer.iSessionId);
	const auto bindingIter = m_GameplayBindingBySessionId.find(transfer.iSessionId);
	if (sessionIter == m_Sessions.end() || nullptr == sessionIter->second ||
	    bindingIter == m_GameplayBindingBySessionId.end() || bindingIter->second.eWorldId != sourceWorldId ||
	    bindingIter->second.pSimulation != sourceSimulation ||
	    (WORLD_ID::CHARACTER_SELECT_ARENA == sourceWorldId &&
	     bindingIter->second.iPrivateArenaOwnerSessionId != transfer.iSessionId))
	{
		setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED, WSAENOTCONN, "source-binding-validation");
		return false;
	}
	if (sessionIter->second->Is_Closing())
	{
		setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED, WSAESHUTDOWN,
		           "source-session-terminal-recheck");
		return false;
	}
	// 가이드 owner의 단독 레이드 이동도 파티와 같은 admission/FIFO 트랜잭션을 사용한다.
	// 참여자는 인간 한 명 그대로이며 이 경로가 새 파티를 만들지는 않는다.
	auto transactionSessions = transfer.PartyBatchSessionIds;
	const bool bernRaidPair =
	    (sourceWorldId == WORLD_ID::BERN && (transfer.eTargetWorldId == WORLD_ID::VALTAN_ARENA ||
	                                         transfer.eTargetWorldId == WORLD_ID::KOUKUSAYDON_ARENA)) ||
	    (transfer.eTargetWorldId == WORLD_ID::BERN &&
	     (sourceWorldId == WORLD_ID::VALTAN_ARENA || sourceWorldId == WORLD_ID::KOUKUSAYDON_ARENA));
	if (transactionSessions.empty() && bernRaidPair &&
	    (sourceSimulation->Has_PersonalGuideOwner(sessionIter->second) ||
	     targetSimulation->Has_PersonalGuideOwner(sessionIter->second)))
	{
		transactionSessions.push_back(transfer.iSessionId);
		outFailure.bSourcePreservedOnRejection = true;
	}

	if (!transactionSessions.empty())
	{
		if (transactionSessions.front() != transfer.iSessionId)
		{
			setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_VALIDATION_FAILED, WSAEINVAL, "party-leader-identity");
			return false;
		}
		for (const SESSION_ID memberId : transactionSessions)
		{
			const auto member = m_Sessions.find(memberId);
			const auto binding = m_GameplayBindingBySessionId.find(memberId);
			if (member == m_Sessions.end() || nullptr == member->second || member->second->Is_Closing() ||
			    binding == m_GameplayBindingBySessionId.end() || binding->second.eWorldId != sourceWorldId ||
			    binding->second.pSimulation != sourceSimulation)
			{
				setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED, WSAENOTCONN, "party-member-binding");
				return false;
			}
		}
		std::string status;
		if (!sourceSimulation->Transfer_PartyTo(*targetSimulation, transactionSessions, outFailure.ePartyResult, status,
		                                        transfer.strRaidReturnNpcPlacementId,
		                                        transfer.strSpawnPlacementOverrideId))
		{
			setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_JOIN_PREFLIGHT_FAILED, WSAEINVAL, status);
			return false;
		}
		for (const SESSION_ID memberId : transactionSessions)
		{
			auto& binding = m_GameplayBindingBySessionId.at(memberId);
			binding.eWorldId = transfer.eTargetWorldId;
			binding.iPrivateArenaOwnerSessionId = INVALID_SESSION_ID;
			binding.pSimulation = targetSimulation;
		}
		return true;
	}

	// 일반 단독 전환은 목적 룸에 등록·입장 명령을 순서대로 넣는다.
	// 입장 등록 실패는 목적 룸 LEAVE를 예약해 이미 큐에 넣은 등록 상태를 정리한다.
	ROOM_COMMAND registerCommand{};
	registerCommand.eType = ROOM_COMMAND_TYPE::REGISTER_SESSION;
	registerCommand.iSessionId = transfer.iSessionId;
	registerCommand.pSession = sessionIter->second;
	const ROOM_COMMAND_ENQUEUE_RESULT targetRegisterResult =
	    targetSimulation->Enqueue_Detailed(std::move(registerCommand));
	if (ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED != targetRegisterResult)
	{
		setFailure(ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY == targetRegisterResult
		               ? SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_INGRESS_OVERFLOW
		           : ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_NOT_READY == targetRegisterResult
		               ? SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_RUNTIME_FAILED
		               : SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED,
		           ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY == targetRegisterResult ? WSAENOBUFS
		                                                                                           : WSAESHUTDOWN,
		           "target-register-ingress " + targetSimulation->Describe_EnqueueResult(targetRegisterResult));
		return false;
	}

	C2S_ENTER_WORLD enterWorld{};
	enterWorld.iProtocolVersion = NETWORK_PROTOCOL_VERSION;
	enterWorld.eWorldId = transfer.eTargetWorldId;
	enterWorld.eCharacterClass = transfer.eCharacterClass;
	enterWorld.strNickName = transfer.strNickName;
	enterWorld.iVoiceType = transfer.iVoiceType;
	enterWorld.strAppearanceJson = transfer.strAppearanceJson;
	ROOM_COMMAND enterCommand{};
	enterCommand.eType = ROOM_COMMAND_TYPE::ENTER_WORLD;
	enterCommand.iSessionId = transfer.iSessionId;
	enterCommand.EnterWorld = std::move(enterWorld);
	enterCommand.strSpawnPlacementOverrideId = transfer.strSpawnPlacementOverrideId;
	enterCommand.strRaidReturnNpcPlacementId = transfer.strRaidReturnNpcPlacementId;
	enterCommand.eEntrySourceWorldId = sourceWorldId;
	enterCommand.CarriedInventory = transfer.CarriedInventory;
	enterCommand.bHasCarriedCharacterState = true;
	enterCommand.CarriedPurse = transfer.CarriedPurse;
	enterCommand.iCarriedHonorTitleId = transfer.iHonorTitleId;
	const ROOM_COMMAND_ENQUEUE_RESULT targetEnterResult = targetSimulation->Enqueue_Detailed(std::move(enterCommand));
	if (ROOM_COMMAND_ENQUEUE_RESULT::ACCEPTED != targetEnterResult)
	{
		ROOM_COMMAND targetRollback{};
		targetRollback.eType = ROOM_COMMAND_TYPE::LEAVE;
		targetRollback.iSessionId = transfer.iSessionId;
		targetRollback.eLeaveReason = PLAYER_DESPAWN_REASON::LEVEL_CHANGED;
		const bool rollbackEnqueued = targetSimulation->Enqueue(std::move(targetRollback));
		setFailure(ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY == targetEnterResult
		               ? SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_INGRESS_OVERFLOW
		           : ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_ROOM_NOT_READY == targetEnterResult
		               ? SESSION_DIAGNOSTIC_REASON::SERVER_ROOM_RUNTIME_FAILED
		               : SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED,
		           ROOM_COMMAND_ENQUEUE_RESULT::REJECTED_RELIABLE_CAPACITY == targetEnterResult ? WSAENOBUFS
		                                                                                        : WSAESHUTDOWN,
		           "target-enter-ingress " + targetSimulation->Describe_EnqueueResult(targetEnterResult), true,
		           rollbackEnqueued);
		return false;
	}

	// 목적 룸 큐 등록을 마친 뒤 source 퇴장을 확정한다. 이 단계 실패도 목적 룸에
	// 정리 명령을 넣고 바인딩은 바꾸지 않는다. source 성공 후에만 아래 바인딩을 교체한다.
	if (!sourceSimulation->Commit_WorldTransferDeparture(transfer.iSessionId))
	{
		ROOM_COMMAND targetRollback{};
		targetRollback.eType = ROOM_COMMAND_TYPE::LEAVE;
		targetRollback.iSessionId = transfer.iSessionId;
		targetRollback.eLeaveReason = PLAYER_DESPAWN_REASON::LEVEL_CHANGED;
		const bool rollbackEnqueued = targetSimulation->Enqueue(std::move(targetRollback));
		setFailure(SESSION_DIAGNOSTIC_REASON::SERVER_SESSION_BIND_FAILED, WSAEINVAL, "source-departure-commit", true,
		           rollbackEnqueued);
		return false;
	}

	bindingIter->second.eWorldId = transfer.eTargetWorldId;
	bindingIter->second.iPrivateArenaOwnerSessionId = INVALID_SESSION_ID;
	bindingIter->second.pSimulation = targetSimulation;
	return true;
}

// 새 연결을 막고 accept·룸 worker를 합류한 뒤 준비/저장 worker와 세션을 정리한다.
// 콜로세움 준비에는 협력 취소를 전달하고 저장 worker는 완료를 기다려 이미 영속화된 수치를 확정한다.
// 그 뒤 모든 세션 worker를 중지하고 룸 소유 컨테이너·WinSock·프로세스 mutex를 순서대로 해제한다.
void LostArk::Server::CServerApp::Shutdown()
{
	// listener를 닫아 accept 대기를 깨우고 룸 worker까지 합류해 주 진행자를 멈춘다.
	m_isRunning.store(false);
	m_TcpListener.Close();
	if (m_AcceptThread.joinable())
		m_AcceptThread.join();
	if (m_RoomThread.joinable())
		m_RoomThread.join();

	// 별도 준비 worker에는 취소를 요청하고 최대 30초 완료를 기다린다.
	// 기한 초과 시 worker만 버리고 계속 실행하지 않고 ERROR_TIMEOUT으로 프로세스를 끝낸다.
	if (m_ColosseumPreparationWorker.valid())
	{
		if (m_ColosseumPreparationCancelled)
			m_ColosseumPreparationCancelled->store(true, std::memory_order_relaxed);
		if (m_ColosseumPreparationWorker.wait_for(std::chrono::seconds(30)) != std::future_status::ready)
		{
			std::cerr << "Colosseum preparation did not stop within the shutdown deadline.\n";
			::ExitProcess(ERROR_TIMEOUT);
		}
		try
		{
			(void)m_ColosseumPreparationWorker.get();
		}
		catch (...)
		{
		}
	}
	m_ColosseumPreparationSource.reset();
	m_PreparedColosseumRoom.reset();
	m_ColosseumPreparationCancelled.reset();

	// 수치 저장 worker도 합류한 다음 처리 결과를 소비한다. 준비만 하던 후보는 취소하고
	// 디스크 저장 중이던 작업은 실제 성공 여부에 따라 메모리 확정·취소를 마무리한다.
	if (m_NumericBalanceWorker.valid())
	{
		if (m_NumericBalanceWorker.wait_for(std::chrono::seconds(30)) != std::future_status::ready)
		{
			std::cerr << "Numeric save worker did not stop within the shutdown deadline.\n";
			::ExitProcess(ERROR_TIMEOUT);
		}
		// 디스크 저장 단계까지 진행한 작업은 프로세스 메모리를 버리기 전에 결과를 확정한다.
		if (m_NumericBalancePersisting)
			Advance_NumericBalanceTransaction();
		else
		{
			(void)m_NumericBalanceWorker.get();
			Finish_NumericBalanceTransaction(false, "Server stopped before saving");
		}
	}
	Abort_DataRevisionTransaction("Server shutdown aborted data revision prepare");

	// 세션들의 강한 참조를 먼저 모으고 맵 잠금 밖에서 모두 close 요청 → Stop 순으로 처리한다.
	// 각 종료 콜백이 세션 맵을 다시 조회할 수 있으므로 join 중 맵 잠금을 잡지 않는다.
	std::vector<std::shared_ptr<CClientSession>> sessions;
	{
		std::scoped_lock lock{m_SessionsMutex};
		for (auto& [sessionId, session] : m_Sessions)
		{
			(void)sessionId;
			sessions.push_back(session);
		}
	}
	for (const auto& session : sessions)
	{
		if (nullptr != session)
		{
			session->Request_Close(LostArk::Shared::SESSION_DIAGNOSTIC_REASON::SERVER_SHUTDOWN, 0,
			                       "Server process shutdown");
		}
	}
	for (const auto& session : sessions)
	{
		if (nullptr != session)
			session->Stop();
	}
	{
		std::scoped_lock lock{m_SessionsMutex};
		m_Sessions.clear();
		m_GameplayBindingBySessionId.clear();
	}

	// 수신 콜백과 룸 worker를 먼저 멈춘 뒤 시뮬레이션을 제거한다.
	// 소유자를 잃은 룸에 생산자가 명령을 넣지 않도록 수명 종료 순서를 고정한다.
	{
		std::scoped_lock lock{m_SessionsMutex};
		m_CharacterSelectArenas.clear();
		m_ColosseumMatches.clear();
		m_SharedGameRooms.clear();
		m_pActiveGameplayGeneration.reset();
		m_ActiveGameplayBootstrapContentRevision = {};
		m_ActiveNonValtanGameplayRevision = {};
		m_isActiveGameplayGenerationFromCandidate = false;
		m_isRuntimeActivePersistenceEnabled = false;
	}
	m_DataRevisionResponseTombstones.clear();
	{
		std::scoped_lock lock{m_ClosedSessionMutex};
		m_ClosedSessionIds.clear();
	}
	{
		std::scoped_lock lock{m_ServerControlMutex};
		m_ServerControlEvents.clear();
	}
	m_WinSockContext.Shutdown();
	Release_RuntimeGameplayProcessMutex(m_hRuntimeGameplayProcessMutex);
}

// 룸 worker 또는 worker 합류 뒤 Shutdown이 요청 결과와 현재 numeric revision을 해당 세션에 보낸다.
// 없는 세션·종료 중 세션은 건너뛰고 직렬화·송신 큐 실패만 종료 요청으로 연결한다.
void LostArk::Server::CServerApp::Send_NumericBalanceResult(const SESSION_ID sessionId, const std::uint32_t sequence,
                                                            const LostArk::Shared::BALANCE_APPLY_RESULT result,
                                                            std::string reason)
{
	using namespace LostArk::Shared;
	std::shared_ptr<CClientSession> session;
	{
		std::scoped_lock lock{m_SessionsMutex};
		const auto found = m_Sessions.find(sessionId);
		if (found != m_Sessions.end())
			session = found->second;
	}
	if (!session || session->Is_Closing())
		return;
	if (reason.size() > MAX_BALANCE_REASON_BYTES)
		reason.resize(MAX_BALANCE_REASON_BYTES);
	const S2C_BALANCE_RESULT response{sequence, result, m_NumericBalanceStore.Get_NumericRevision(), std::move(reason)};
	CPacketWriter writer;
	if (!Write_Message(writer, response) || !session->Send_Frame(PACKET_TYPE::S2C_BALANCE_RESULT, writer.Get_Buffer()))
		Request_SessionClose(sessionId);
}

// 룸 worker가 입장한 세션의 수치 조회·저장 요청을 처리한다. 조회는 현재 store의 페이지를 직렬화한다.
// 저장은 base revision과 동시 트랜잭션을 검사한 뒤 store 복사본을 worker로 보내 후보를 준비한다.
// worker는 프로세스 활성 store를 바꾸지 않고 future에 준비 결과·해시·오류를 반환한다.
void LostArk::Server::CServerApp::Process_NumericBalanceEvent(const SERVER_CONTROL_EVENT& event)
{
	using namespace LostArk::Shared;
	const bool query = event.eKind == SERVER_CONTROL_EVENT_KIND::BALANCE_QUERY;
	const auto sequence = query ? event.BalanceQuery.iRequestSequence : event.BalancePatch.iRequestSequence;
	std::shared_ptr<CClientSession> session;
	std::shared_ptr<const CGameplayCatalog> active;
	{
		std::scoped_lock lock{m_SessionsMutex};
		const auto found = m_Sessions.find(event.iSessionId);
		const auto binding = m_GameplayBindingBySessionId.find(event.iSessionId);
		if (found != m_Sessions.end() && binding != m_GameplayBindingBySessionId.end() && binding->second.pSimulation)
			session = found->second;
		active = m_pActiveGameplayGeneration;
	}
	if (!session || session->Is_Closing())
	{
		Send_NumericBalanceResult(event.iSessionId, sequence, BALANCE_APPLY_RESULT::INVALID_SESSION,
		                          "Enter a Server world before editing balance");
		return;
	}
	std::string status;
	// 직전에 완료한 패턴 저작 트랜잭션이 활성 catalog를 교체했을 수 있다.
	if (m_NumericBalanceStore.Get_ActiveGeneration() != active && !m_NumericBalanceStore.Initialize(active, status))
	{
		Send_NumericBalanceResult(event.iSessionId, sequence, BALANCE_APPLY_RESULT::INVALID_CHANGE, status);
		return;
	}
	if (query)
	{
		S2C_BALANCE_SNAPSHOT snapshot;
		if (!m_NumericBalanceStore.BuildSnapshot(sequence, event.BalanceQuery.iPageIndex, snapshot, status))
		{
			Send_NumericBalanceResult(event.iSessionId, sequence, BALANCE_APPLY_RESULT::INVALID_CHANGE, status);
			return;
		}
		CPacketWriter writer;
		if (!Write_Message(writer, snapshot) ||
		    !session->Send_Frame(PACKET_TYPE::S2C_BALANCE_SNAPSHOT, writer.Get_Buffer()))
			Request_SessionClose(event.iSessionId);
		return;
	}
	if (m_DataRevisionTransaction.Is_Active() || m_NumericBalanceRequester != INVALID_SESSION_ID)
	{
		Send_NumericBalanceResult(
		    event.iSessionId, sequence, BALANCE_APPLY_RESULT::BUSY,
		    "Another balance or authored-pattern save is being applied; this draft was not changed");
		return;
	}

	// 편집 화면이 읽었던 numeric revision과 현재 값이 같아야 준비를 시작한다.
	// 다른 저장이 먼저 끝났으면 사용자의 이전 draft를 적용하지 않고 stale 결과를 돌려준다.
	if (event.BalancePatch.BaseNumericRevision != m_NumericBalanceStore.Get_NumericRevision())
	{
		Send_NumericBalanceResult(
		    event.iSessionId, sequence, BALANCE_APPLY_RESULT::STALE_REVISION,
		    "Server numbers changed since this draft was read; reload the current values before saving");
		return;
	}
	m_NumericBalanceRequester = event.iSessionId;
	m_NumericBalanceRequestSequence = sequence;
	if (++m_NumericBalanceTransactionSequence == 0u)
		++m_NumericBalanceTransactionSequence;
	m_NumericBalancePersisting = false;
	try
	{
		const auto runtimeRoot =
		    m_isRuntimeActivePersistenceEnabled ? Resolve_RuntimeActiveGameplayRoot() : std::filesystem::path{};
		RUNTIME_ACTIVE_GAMEPLAY_GENERATION runtimeBase;
		runtimeBase.eSource = m_isActiveGameplayGenerationFromCandidate
		                          ? RUNTIME_GAMEPLAY_GENERATION_SOURCE::CANDIDATE
		                          : RUNTIME_GAMEPLAY_GENERATION_SOURCE::PACKAGED_BASELINE;
		runtimeBase.Revision = active->Get_ActiveRevision();
		runtimeBase.BootstrapContentRevision = m_ActiveGameplayBootstrapContentRevision;
		runtimeBase.NonValtanGameplayRevision = m_ActiveNonValtanGameplayRevision;

		// store와 요청은 값으로 복사하고 runtime 식별자도 캡처한다. worker가 준비하는 catalog·
		// 문서 후보는 공유 활성 상태와 분리되며 결과 shared_ptr는 future를 통해 룸 worker로 돌아온다.
		m_NumericBalanceWorker = std::async(
		    std::launch::async,
		    [store = m_NumericBalanceStore, request = event.BalancePatch, runtimeRoot, runtimeBase]()
		    {
			    BALANCE_WORK_RESULT result;
			    try
			    {
				    result.Prepared = std::make_shared<SERVER_BALANCE_PREPARED>();
				    result.Succeeded = store.PreparePatch(request, *result.Prepared, result.Failure, result.Status);
				    if (result.Succeeded)
				    {
					    const auto& bytes = result.Prepared->BootstrapBytes;
					    result.Succeeded =
					        Hash_BytesSha256(std::span<const std::uint8_t>(
					                             reinterpret_cast<const std::uint8_t*>(bytes.data()), bytes.size()),
					                         result.BootstrapRevision) &&
					        Build_NonValtanGameplayRevisionBytes(bytes, result.NonValtanRevision, result.Status);
					    if (result.Succeeded && !runtimeRoot.empty() && std::filesystem::exists(runtimeRoot))
					    {
						    bool exists = false;
						    std::vector<std::uint8_t> pointerBytes;
						    if (!ReadOptionalRuntimeGameplayFile(runtimeRoot, RUNTIME_ACTIVE_POINTER_NAME, exists,
						                                         pointerBytes, result.Status))
							    result.Succeeded = false;
						    else if (exists)
						    {
							    // 영속 포인터가 있으면 준비 시작 때 캡처한 base와 정확히 같은지 확인한다.
							    // 다음 raw·비발탄 해시를 반영한 바이트만 후보에 보관하고 여기서는 파일을 쓰지 않는다.
							    RUNTIME_ACTIVE_GAMEPLAY_GENERATION disk;
							    if (!ParseRuntimeActivePointer(pointerBytes, disk, result.Status) ||
							        !RuntimeGenerationEquals(disk, runtimeBase))
							    {
								    result.Succeeded = false;
								    result.Status = "Durable gameplay pointer changed before numeric save";
							    }
							    else
							    {
								    auto next = runtimeBase;
								    next.BootstrapContentRevision = result.BootstrapRevision;
								    next.NonValtanGameplayRevision = result.NonValtanRevision;
								    result.Prepared->RuntimeActivePointerPath =
								        runtimeRoot / RUNTIME_ACTIVE_POINTER_NAME;
								    result.Prepared->RuntimeActivePointerBaseBytes.assign(
								        reinterpret_cast<const char*>(pointerBytes.data()), pointerBytes.size());
								    result.Prepared->RuntimeActivePointerBytes = BuildRuntimeActivePointerBytes(next);
							    }
						    }
					    }
				    }
			    }
			    catch (const std::exception& error)
			    {
				    result.Succeeded = false;
				    result.Status = error.what();
			    }
			    catch (...)
			    {
				    result.Succeeded = false;
				    result.Status = "Numeric candidate validation failed";
			    }
			    return result;
		    });
	}
	catch (const std::exception& error)
	{
		Finish_NumericBalanceTransaction(false, error.what());
	}
}

// numeric future를 기다리지 않고 완료 여부만 확인한다. 검증 완료면 전 룸에 후보를 stage하고
// 신규 입장을 잠시 막은 뒤 디스크 저장 worker를 시작한다. 저장 완료면 Finish로 확정/취소를 넘긴다.
// 이 과정의 실패도 Finish로 모아 이미 stage한 룸과 admission 상태를 정리한다.
// 정상 실행에서는 룸 worker가 호출하고 Shutdown도 룸 worker 합류 및 저장 future 완료 대기 뒤 이 경로를 사용한다.
void LostArk::Server::CServerApp::Advance_NumericBalanceTransaction()
{
	using namespace LostArk::Shared;
	if (!m_NumericBalanceWorker.valid() ||
	    m_NumericBalanceWorker.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
		return;
	BALANCE_WORK_RESULT completed;
	try
	{
		completed = m_NumericBalanceWorker.get();
	}
	catch (const std::exception& error)
	{
		Finish_NumericBalanceTransaction(false, error.what());
		return;
	}
	if (m_NumericBalancePersisting)
	{
		m_NumericBalancePrepared.Failure = BALANCE_APPLY_RESULT::SAVE_FAILED;
		Finish_NumericBalanceTransaction(completed.Succeeded, std::move(completed.Status));
		return;
	}
	m_NumericBalancePrepared = std::move(completed);
	if (!m_NumericBalancePrepared.Succeeded)
	{
		Finish_NumericBalanceTransaction(false, m_NumericBalancePrepared.Status);
		return;
	}
	std::string status;
	try
	{
		// 준비 동안 세대가 바뀌지 않았는지 확인하고 현재 공유·private·경기 룸 모두에 stage한다.
		// 모든 stage가 성공한 뒤 admission pause를 켜야 디스크 저장 중 새 룸이 이전 숫자로 들어오지 않는다.
		std::unique_lock admissionLock{m_DataRevisionAdmissionMutex};
		{
			std::scoped_lock lock{m_SessionsMutex};
			const auto& prepared = *m_NumericBalancePrepared.Prepared;
			if (prepared.BaseNumericRevision != m_NumericBalanceStore.Get_NumericRevision() ||
			    m_DataRevisionTransaction.Is_Active())
				status = "Active balance changed during validation";
			else
			{
				m_NumericBalanceSimulations.reserve(m_SharedGameRooms.size() + m_CharacterSelectArenas.size());
				for (const auto& [world, room] : m_SharedGameRooms)
					if (room)
						m_NumericBalanceSimulations.push_back(room);
				for (const auto& [owner, room] : m_CharacterSelectArenas)
					if (room)
						m_NumericBalanceSimulations.push_back(room);
				for (const auto& [matchId, room] : m_ColosseumMatches)
					if (room)
						m_NumericBalanceSimulations.push_back(room);
				for (const auto& room : m_NumericBalanceSimulations)
					if (!room->Stage_NumericBalance(m_NumericBalanceTransactionSequence, prepared.Generation, status))
						break;
				if (status.empty())
					m_NumericAdmissionPaused = true;
			}
		}
		admissionLock.unlock();
		if (!status.empty())
		{
			Finish_NumericBalanceTransaction(false, status);
			return;
		}
		m_NumericBalancePrepared.Failure = BALANCE_APPLY_RESULT::SAVE_FAILED;
		m_NumericBalancePersisting = true;

		// 영속화 worker는 준비된 shared_ptr와 store 복사본만 사용한다.
		// 룸 실행을 기다리게 하지 않고 다음 Advance에서 완료 결과를 확인한다.
		m_NumericBalanceWorker =
		    std::async(std::launch::async,
		               [store = m_NumericBalanceStore, prepared = m_NumericBalancePrepared.Prepared]()
		               {
			               BALANCE_WORK_RESULT result;
			               result.Failure = BALANCE_APPLY_RESULT::SAVE_FAILED;
			               try
			               {
				               result.Succeeded = store.PersistPrepared(*prepared, result.Status);
			               }
			               catch (const std::exception& error)
			               {
				               result.Status = error.what();
			               }
			               catch (...)
			               {
				               result.Status = "Numeric persistence failed";
			               }
			               return result;
		               });
	}
	catch (const std::exception& error)
	{
		Finish_NumericBalanceTransaction(false, error.what());
	}
}

// 룸 worker 또는 worker 합류 후 Shutdown이 numeric 트랜잭션을 마무리한다.
// 저장이 성공한 경우 전 룸 → store → 활성 해시 순으로 확정하고 실패면 각 룸의 stage를 취소한다.
// 디스크 성공 뒤 룸 확정 실패는 복구 가능한 거절로 처리하지 않고 재시작을 위해 프로세스를 종료한다.
// 잠금을 해제하고 요청 상태를 비운 다음 요청자 결과와 다른 입장 세션의 갱신 통지를 보낸다.
void LostArk::Server::CServerApp::Finish_NumericBalanceTransaction(const bool commit, std::string status)
{
	using namespace LostArk::Shared;
	const auto requester = m_NumericBalanceRequester;
	const auto sequence = m_NumericBalanceRequestSequence;
	const auto failure = m_NumericBalancePrepared.Failure == BALANCE_APPLY_RESULT::APPLIED
	                         ? BALANCE_APPLY_RESULT::INVALID_CHANGE
	                         : m_NumericBalancePrepared.Failure;
	std::vector<SESSION_ID> notify;
	{
		std::scoped_lock admissionLock{m_DataRevisionAdmissionMutex};
		std::scoped_lock lock{m_SessionsMutex};
		if (commit)
		{
			// 어느 룸도 다음 fixed step을 실행하기 전에 모든 룸의 numeric 후보를 확정한다.
			for (const auto& room : m_NumericBalanceSimulations)
				if (!room->Commit_NumericBalance(m_NumericBalanceTransactionSequence))
				{
					std::cerr << "Durable numeric balance could not activate in a staged room; restart is required.\n";
					::ExitProcess(ERROR_INVALID_STATE);
				}
			m_NumericBalanceStore.CommitPrepared(*m_NumericBalancePrepared.Prepared);
			m_pActiveGameplayGeneration = m_NumericBalancePrepared.Prepared->Generation;
			m_ActiveGameplayBootstrapContentRevision = m_NumericBalancePrepared.BootstrapRevision;
			m_ActiveNonValtanGameplayRevision = m_NumericBalancePrepared.NonValtanRevision;
			for (const auto& [id, binding] : m_GameplayBindingBySessionId)
				if (id != requester && binding.pSimulation)
					notify.push_back(id);
		}
		else
			for (const auto& room : m_NumericBalanceSimulations)
				room->Abort_NumericBalance(m_NumericBalanceTransactionSequence);
		m_NumericAdmissionPaused = false;
	}

	// 성공·실패 공통으로 임시 수명과 요청 identity를 비운다. 응답은 아래에서 현재 revision으로 보낸다.
	m_NumericBalanceSimulations.clear();
	m_NumericBalancePrepared = {};
	m_NumericBalanceRequester = INVALID_SESSION_ID;
	m_NumericBalanceRequestSequence = 0u;
	m_NumericBalancePersisting = false;
	if (requester != INVALID_SESSION_ID)
		Send_NumericBalanceResult(requester, sequence, commit ? BALANCE_APPLY_RESULT::APPLIED : failure,
		                          std::move(status));
	if (commit)
		for (const auto id : notify)
			Send_NumericBalanceResult(id, 0u, BALANCE_APPLY_RESULT::APPLIED, {});
}
