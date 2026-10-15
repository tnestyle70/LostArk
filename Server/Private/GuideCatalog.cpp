// 독립 Guide 문서를 게시 정의의 소유 값으로 구성하고 조회한다.
#include "GuideCatalog.h"
// 가이드 displayName을 실제 player nickname의 UTF-8·길이·제어문자 계약으로 검사한다.
#include "Network/PacketMessages.h"
// 환경변수/EXE 경로를 읽고 대사 UTF-8을 Win32 변환 함수로 검증한다.
#include <Windows.h>
// prompt/combo 선형 조회와 HELP_COMMAND의 command 참조 유무를 확인한다.
#include <algorithm>
// JSON 숫자를 locale에 의존하지 않는 double로 변환한다.
#include <charconv>
// 유한성·정수성·거리/가중치 범위의 절댓값을 검사한다.
#include <cmath>
// 환경변수 또는 EXE 기준 Guide.runtime.json 경로를 구성한다.
#include <filesystem>
// 게시 문서의 binary 입력과 파일 크기 조회를 사용한다.
#include <fstream>
// istreambuf_iterator로 JSON 원문을 소유 string에 읽는다.
#include <iterator>
// 파싱·계약 위반을 예외로 모아 Load의 진단 경계로 전달한다.
#include <stdexcept>
// category/trigger/command ID와 활성 명령 별칭의 중복을 검출한다.
#include <set>
// Parser가 Load 지역 source를 복사하지 않고 읽는 동안 차용한다.
#include <string_view>
// JSON null/bool/number/string/array/object 중 실제 종류를 보관한다.
#include <variant>

namespace
{
	// 독립 Guide 문서의 임시 값 트리다. 멤버 문자열·배열·객체를 소유하고 parser 입력 버퍼를 차용하지 않는다.
	// 종류 불일치는 std::get, 필수 키 누락은 map::at의 예외로 Load에 전달한다.
	struct Json
	{
		using Object = std::map<std::string, Json>;
		using Array = std::vector<Json>;
		std::variant<std::nullptr_t, bool, double, std::string, Array, Object> Value;
		// 필수 object 키의 값을 차용한다. object 종류가 아니거나 키가 없으면 예외다.
		const Json& at(const std::string& key) const
		{
			return std::get<Object>(Value).at(key);
		}
		// 선택 키 누락만 nullptr로 알린다. JSON 종류가 object가 아닌 경우까지 기본값으로 허용하지 않는다.
		const Json* find(const std::string& key) const
		{
			const auto& o = std::get<Object>(Value);
			auto i = o.find(key);
			return i == o.end() ? nullptr : &i->second;
		}
		// 배열의 원소를 복사하지 않고 순회한다. 반환 참조는 이 Json 값 트리의 수명에 종속된다.
		const Array& array() const
		{
			return std::get<Array>(Value);
		}
		// 문자열 값은 복사해 반환하므로 catalog가 임시 JSON 트리보다 오래 보관할 수 있다.
		std::string text() const
		{
			return std::get<std::string>(Value);
		}
		// 숫자 종류만 읽는다. 정수/구간 제한은 필드별 helper와 Load가 따로 검사한다.
		double number() const
		{
			return std::get<double>(Value);
		}
		// bool 종류만 허용하며 문자열/숫자를 암묵적으로 참거짓으로 바꾸지 않는다.
		bool boolean() const
		{
			return std::get<bool>(Value);
		}
	};
	// Load 지역 source의 byte view를 동기적으로 순회한다. 반환 Json은 값을 소유하지만 Parser 자체는 입력보다 오래 살면 안 된다.
	// 재귀 깊이와 값 node 수를 제한하고 중복 object 키·문서 뒤 잔여 입력을 거부한다. 모든 문자열의 raw UTF-8 검사를 대신하지는 않는다.
	class Parser
	{
		std::string_view Text;
		std::size_t Cursor = 0, Nodes = 0;
		// 현재 byte 커서를 포함한 구문 진단을 던진다. 부분 트리는 지역 객체 소멸로 정리된다.
		[[noreturn]] void fail() const
		{
			throw std::runtime_error("malformed Guide JSON at byte " + std::to_string(Cursor));
		}
		// JSON이 허용하는 ASCII 공백 네 종류만 건너뛴다.
		void space()
		{
			while (Cursor < Text.size() &&
			       (Text[Cursor] == ' ' || Text[Cursor] == '\r' || Text[Cursor] == '\n' || Text[Cursor] == '\t'))
				++Cursor;
		}
		// 유니코드 escape의 16진수 네 자리를 읽고 커서를 전진한다. 부족하거나 잘못된 자리는 즉시 구문 실패다.
		unsigned hex4()
		{
			unsigned x = 0;
			for (int n = 0; n < 4; ++n)
			{
				if (Cursor == Text.size())
					fail();
				char c = Text[Cursor++];
				unsigned v = c >= '0' && c <= '9'   ? c - '0'
				             : c >= 'a' && c <= 'f' ? c - 'a' + 10
				             : c >= 'A' && c <= 'F' ? c - 'A' + 10
				                                    : 16;
				if (v == 16)
					fail();
				x = (x << 4) | v;
			}
			return x;
		}
		// string이 검사·결합한 유니코드 code point를 UTF-8 byte로 덧붙인다. 이 helper가 입력 scalar의 유효성을 재검사하지는 않는다.
		static void utf8(std::string& s, unsigned c)
		{
			if (c < 0x80)
				s += char(c);
			else if (c < 0x800)
			{
				s += char(0xc0 | (c >> 6));
				s += char(0x80 | (c & 63));
			}
			else if (c < 0x10000)
			{
				s += char(0xe0 | (c >> 12));
				s += char(0x80 | ((c >> 6) & 63));
				s += char(0x80 | (c & 63));
			}
			else
			{
				s += char(0xf0 | (c >> 18));
				s += char(0x80 | ((c >> 12) & 63));
				s += char(0x80 | ((c >> 6) & 63));
				s += char(0x80 | (c & 63));
			}
		}
		// 따옴표 문자열을 소유 값으로 만든다. 제어 byte와 잘못된 escape를 거부하고 surrogate pair를 결합한다.
		// escape 없는 고위 byte는 그대로 복사하며 이름·대사 UTF-8의 추가 검사는 Load가 수행한다.
		std::string string()
		{
			if (Cursor == Text.size() || Text[Cursor++] != '"')
				fail();
			std::string out;
			while (Cursor < Text.size())
			{
				unsigned char c = Text[Cursor++];
				if (c == '"')
					return out;
				if (c < 32)
					fail();
				if (c != '\\')
				{
					out += char(c);
					continue;
				}
				if (Cursor == Text.size())
					fail();
				// 일반 escape와 유니코드 escape를 분리한다. 단독 low surrogate와 뒤 low surrogate가 없는 high surrogate는 허용하지 않는다.
				switch (Text[Cursor++])
				{
				case '"':
					out += '"';
					break;
				case '\\':
					out += '\\';
					break;
				case '/':
					out += '/';
					break;
				case 'b':
					out += '\b';
					break;
				case 'f':
					out += '\f';
					break;
				case 'n':
					out += '\n';
					break;
				case 'r':
					out += '\r';
					break;
				case 't':
					out += '\t';
					break;
				case 'u':
				{
					unsigned v = hex4();
					if (v >= 0xd800 && v <= 0xdbff)
					{
						if (Text.substr(Cursor, 2) != "\\u")
							fail();
						Cursor += 2;
						unsigned low = hex4();
						if (low < 0xdc00 || low > 0xdfff)
							fail();
						v = 0x10000 + ((v - 0xd800) << 10) + low - 0xdc00;
					}
					else if (v >= 0xdc00 && v <= 0xdfff)
						fail();
					utf8(out, v);
					break;
				}
				default:
					fail();
				}
			}
			fail();
		}
		// 현재 커서에서 JSON 값 하나를 파싱한다. 구조 구분자·숫자 문법을 먼저 확인하고 필드 의미 검증은 Load에 남긴다.
		Json value(unsigned depth)
		{
			// root depth는 0이다. 각 값 진입에서 깊이 32와 누적 node 100000 한도를 확인한다.
			if (depth > 32 || ++Nodes > 100000)
				fail();
			space();
			if (Cursor == Text.size())
				fail();
			char c = Text[Cursor];
			if (c == '"')
				return Json{string()};
			// 객체/배열은 종료 구분자와 쉼표를 확인하며 재귀 처리한다. 같은 object 키가 재등장하면 이전 값을 덮어쓰지 않고 실패한다.
			if (c == '{' || c == '[')
			{
				++Cursor;
				space();
				Json::Object object;
				Json::Array list;
				char end = c == '{' ? '}' : ']';
				if (Cursor < Text.size() && Text[Cursor] == end)
				{
					++Cursor;
					return c == '{' ? Json{object} : Json{list};
				}
				for (;;)
				{
					space();
					if (c == '{')
					{
						auto key = string();
						space();
						if (Cursor == Text.size() || Text[Cursor++] != ':')
							fail();
						if (!object.emplace(key, value(depth + 1)).second)
							fail();
					}
					else
						list.push_back(value(depth + 1));
					space();
					if (Cursor == Text.size())
						fail();
					char next = Text[Cursor++];
					if (next == end)
						break;
					if (next != ',')
						fail();
				}
				return c == '{' ? Json{object} : Json{list};
			}
			for (auto literal : {std::string_view("true"), std::string_view("false"), std::string_view("null")})
				if (Text.substr(Cursor, literal.size()) == literal)
				{
					Cursor += literal.size();
					if (literal == "null")
						return Json{nullptr};
					return Json{literal == "true"};
				}
			// 숫자 부호·정수·소수·지수를 JSON 문법으로 스캔한다. 소수점과 지수 뒤에는 최소 한 자리 숫자가 필요하다.
			std::size_t start = Cursor;
			if (Text[Cursor] == '-')
				++Cursor;
			if (Cursor == Text.size())
				fail();
			if (Text[Cursor] == '0')
				++Cursor;
			else
			{
				if (Text[Cursor] < '1' || Text[Cursor] > '9')
					fail();
				while (Cursor < Text.size() && Text[Cursor] >= '0' && Text[Cursor] <= '9')
					++Cursor;
			}
			if (Cursor < Text.size() && Text[Cursor] == '.')
			{
				++Cursor;
				auto begin = Cursor;
				while (Cursor < Text.size() && Text[Cursor] >= '0' && Text[Cursor] <= '9')
					++Cursor;
				if (begin == Cursor)
					fail();
			}
			if (Cursor < Text.size() && (Text[Cursor] == 'e' || Text[Cursor] == 'E'))
			{
				++Cursor;
				if (Cursor < Text.size() && (Text[Cursor] == '+' || Text[Cursor] == '-'))
					++Cursor;
				auto begin = Cursor;
				while (Cursor < Text.size() && Text[Cursor] >= '0' && Text[Cursor] <= '9')
					++Cursor;
				if (begin == Cursor)
					fail();
			}
			double result = 0;
			auto read = std::from_chars(Text.data() + start, Text.data() + Cursor, result);
			if (read.ec != std::errc{} || !std::isfinite(result))
				fail();
			return Json{result};
		}

	  public:
		// 입력은 parse 호출이 끝날 때까지 호출자가 유지한다. Cursor/Nodes는 새 parser마다 0에서 시작한다.
		explicit Parser(std::string_view text) : Text(text)
		{
		}
		// root 값 뒤에 공백만 남아야 성공이다. object 구조와 schema는 반환 뒤 Load에서 확인한다.
		Json parse()
		{
			Json j = value(0);
			space();
			if (Cursor != Text.size())
				fail();
			return j;
		}
	};
	// 선택 문자열은 키가 없을 때만 fallback을 복사한다. 존재하는 값의 종류가 틀리면 예외로 거부한다.
	std::string text(const Json& j, const char* key, std::string fallback = {})
	{
		auto v = j.find(key);
		return v ? v->text() : fallback;
	}
	// 선택 숫자의 누락 기본값을 적용한다. 수치 범위나 정수 여부는 이 helper가 결정하지 않는다.
	double number(const Json& j, const char* key, double fallback)
	{
		auto v = j.find(key);
		return v ? v->number() : fallback;
	}
	// 선택 bool의 누락 기본값만 허용하며 다른 JSON 종류를 bool로 변환하지 않는다.
	bool jsonBoolean(const Json& j, const char* key, bool fallback)
	{
		auto v = j.find(key);
		return v ? v->boolean() : fallback;
	}
	// ms·버전·priority 등 비음수 정수를 읽는다. 호출별 maximum 이내의 정수만 uint32로 옮긴다.
	std::uint32_t uint(const Json& j, const char* key, std::uint32_t fallback, std::uint32_t maximum = 3600000)
	{
		auto v = number(j, key, fallback);
		if (v < 0 || v > maximum || std::floor(v) != v)
			throw std::runtime_error(std::string("invalid integer: ") + key);
		return static_cast<std::uint32_t>(v);
	}
	// world 위치 또는 상자 반범위의 세 성분을 읽고 각 절댓값을 100000 이내로 제한한다. 반범위 양수 조건은 trigger 로드에서 따로 검사한다.
	std::array<float, 3> position(const Json& j)
	{
		const auto& a = j.array();
		if (a.size() != 3)
			throw std::runtime_error("Guide vector must have 3 values");
		std::array<float, 3> p{};
		for (int i = 0; i < 3; ++i)
		{
			auto n = a[i].number();
			if (std::abs(n) > 100000)
				throw std::runtime_error("Guide vector outside world bounds");
			p[i] = static_cast<float>(n);
		}
		return p;
	}
	// 유효 길이의 LOSTARK_SERVER_DATA_ROOT를 우선한다. 없거나 버퍼보다 길면 EXE 부모의 부모 아래 DataFiles를 사용한다.
	// EXE 경로 조회 실패는 빈 경로로 전달되어 Load의 파일 열기 진단이 된다.
	std::filesystem::path path()
	{
		wchar_t buffer[32768]{};
		auto size = GetEnvironmentVariableW(L"LOSTARK_SERVER_DATA_ROOT", buffer, 32768);
		if (size && size < 32768)
			return std::filesystem::path(buffer) / L"Guide" / L"Guide.runtime.json";
		size = GetModuleFileNameW(nullptr, buffer, 32768);
		if (!size || size >= 32768)
			return {};
		return std::filesystem::path(buffer).parent_path().parent_path() / L"DataFiles" / L"Guide" /
		       L"Guide.runtime.json";
	}
	// Guide category가 지원하는 Bern/Valtan/Kouku 이름만 변환한다. 복귀 사건의 Maharaka/Colosseum 이름은 별도 문자열 필드다.
	LostArk::Shared::WORLD_ID world(std::string name)
	{
		using LostArk::Shared::WORLD_ID;
		if (name == "BERN")
			return WORLD_ID::BERN;
		if (name == "VALTAN_ARENA")
			return WORLD_ID::VALTAN_ARENA;
		if (name == "KOUKUSAYDON_ARENA")
			return WORLD_ID::KOUKUSAYDON_ARENA;
		throw std::runtime_error("unknown Guide world: " + name);
	}
}
// 룸 초기화가 publisher의 단일 Guide.runtime.json을 읽는다. 공개 상태 대신 지역 candidate에 파싱/검증을 끝낸 뒤 한 번 이동 대입한다.
// commit 전 예외는 기존 catalog를 유지한다. commit 뒤 status 생성 예외는 이미 교체한 catalog를 복원하지 않는다는 경계가 있다.
bool LostArk::Server::CGuideCatalog::Load(std::string& status)
{
	try
	{
		// EXE/환경변수 기준 게시 파일을 열고 조회된 크기가 4MiB를 넘으면 거부한다. UTF-8 BOM은 파서 진입 전에 제거한다.
		std::ifstream file(path(), std::ios::binary);
		if (!file)
			throw std::runtime_error("Guide.runtime.json is missing; run Guide Publish");
		file.seekg(0, std::ios::end);
		if (file.tellg() > 4 * 1024 * 1024)
			throw std::runtime_error("Guide runtime exceeds 4 MiB");
		file.seekg(0);
		std::string source((std::istreambuf_iterator<char>(file)), {});
		if (source.starts_with("\xef\xbb\xbf"))
			source.erase(0, 3);
		const auto root = Parser(source).parse();
		if (text(root, "schema") != "lostark.guide-runtime" || uint(root, "formatVersion", 0) != 1)
			throw std::runtime_error("unsupported Guide schema");
		// 필수 schema/version 이후 지역 정의를 만든다. revision은 double에서 정수로 정확히 표현 가능한 1~2^53-1 범위다.
		CGuideCatalog candidate;
		const double revision = root.at("revision").number();
		if (revision < 1 || revision > 9007199254740991.0 || std::floor(revision) != revision)
			throw std::runtime_error("invalid Guide revision");
		candidate.Revision = static_cast<std::uint64_t>(revision);
		candidate.Id = root.at("guideId").text();
		candidate.Name = root.at("displayName").text();
		if (!candidate.Revision || candidate.Id.empty() || !LostArk::Shared::Is_Valid_PlayerNickname(candidate.Name) ||
		    text(root, "characterClass") != "DIMENSIONMASTER")
			throw std::runtime_error("invalid Guide identity");
		// category ID와 world의 일대일 등록을 검사한다. placement의 category 참조 검사는 모든 정의를 읽은 뒤 수행한다.
		std::set<std::string> categoryIds;
		for (const auto& c : root.at("categories").array())
		{
			const auto id = c.at("categoryId").text();
			if (id.empty() || id.size() > 128 || !categoryIds.insert(id).second ||
			    !candidate.Categories.emplace(world(c.at("worldId").text()), id).second)
				throw std::runtime_error("duplicate or invalid Guide category");
		}
		// 안내 actor의 시작 world 위치와 방향을 읽는다. AnchorPolicy 문자열은 보관/검증하지만 현재 owner 선택은 룸의 session binding을 따른다.
		const auto& p = root.at("placement");
		candidate.PlacementId = p.at("placementId").text();
		candidate.Position = position(p.at("position"));
		candidate.Yaw = static_cast<float>(p.at("yawDegrees").number());
		candidate.AnchorPolicy = text(p, "anchorPolicy", "INVITER_THEN_LEADER");
		// 대사는 선언 순서대로 segment를 보관한다. 여기서 개별 길이·시간과 중복 ID를 검사하고 전체 UTF-8/합계 한도는 아래에서 확인한다.
		for (const auto& pmt : root.at("prompts").array())
		{
			GUIDE_PROMPT prompt;
			prompt.Id = pmt.at("promptId").text();
			for (const auto& s : pmt.at("segments").array())
			{
				auto value = s.at("text").text();
				auto duration = uint(s, "durationMs", 5000, 120000);
				if (value.empty() || value.size() > 512 || duration < 1000 || duration > 20000)
					throw std::runtime_error("invalid Guide prompt segment");
				prompt.Segments.push_back({value, duration});
			}
			if (prompt.Id.empty() || prompt.Segments.empty() || candidate.Find_Prompt(prompt.Id))
				throw std::runtime_error("duplicate or empty Guide prompt");
			candidate.Prompts.push_back(std::move(prompt));
		}
		// event 종류별 참조를 정리한다. prompt는 이미 읽은 목록에서 찾고 combo/command는 나중에 구성된 목록과 다시 대조한다.
		for (const auto& t : root.at("triggers").array())
		{
			GUIDE_TRIGGER trigger;
			trigger.Id = t.at("triggerId").text();
			trigger.Category = t.at("categoryId").text();
			trigger.Enabled = jsonBoolean(t, "enabled", true);
			trigger.PromptId = text(t, "promptId");
			trigger.ComboId = text(t, "comboId");
			trigger.CooldownMs = uint(t, "cooldownMs", 10000);
			trigger.Priority = static_cast<int>(uint(t, "priority", 0, 255));
			const auto& e = t.at("event");
			trigger.Type = e.at("type").text();
			trigger.PatternId = text(e, "patternId", text(e, "commandId"));
			if (trigger.Type == "RAID_RETURNED")
				trigger.PatternId = text(e, "raidWorldId");
			if (trigger.Type == "WORLD_RETURNED")
				trigger.PatternId = text(e, "sourceWorldId");
			// 시작/복귀 안내는 Bern category에 묶는다. raid와 일반 world 복귀의 source 이름 허용 집합은 서로 다르다.
			if (trigger.Type == "GUIDE_STARTED" || trigger.Type == "RAID_RETURNED" || trigger.Type == "WORLD_RETURNED")
			{
				auto bern = candidate.Categories.find(LostArk::Shared::WORLD_ID::BERN);
				if (bern == candidate.Categories.end() || trigger.Category != bern->second)
					throw std::runtime_error("Guide start/return requires Bern");
			}
			if (trigger.Type == "RAID_RETURNED" && trigger.PatternId != "VALTAN_ARENA" &&
			    trigger.PatternId != "KOUKUSAYDON_ARENA")
				throw std::runtime_error("unsupported Guide source raid");
			if (trigger.Type == "WORLD_RETURNED" && trigger.PatternId != "MAHARAKA" && trigger.PatternId != "COLOSSEUM")
				throw std::runtime_error("unsupported Guide source world");
			// 공간 진입만 기존 placement 상자로 옮긴다. 원본 publisher가 검증한 world 좌표(m)·반범위(m)·yaw(degree)를 룸 포함 판정에 전달한다.
			if (const auto* box = trigger.Type == "SPACE_ENTER" ? &e : nullptr)
			{
				auto pos = position(box->at("position"));
				auto ext = position(box->at("halfExtents"));
				if (ext[0] <= 0 || ext[1] <= 0 || ext[2] <= 0)
					throw std::runtime_error("Guide box extents must be positive");
				trigger.Box.fPositionX = pos[0];
				trigger.Box.fPositionY = pos[1];
				trigger.Box.fPositionZ = pos[2];
				trigger.Box.fHalfExtentX = ext[0];
				trigger.Box.fHalfExtentY = ext[1];
				trigger.Box.fHalfExtentZ = ext[2];
				trigger.Box.fYawDegrees = static_cast<float>(number(*box, "yawDegrees", 0));
			}
			if (trigger.Type != "PARTY_JOINED" && trigger.Type != "SPACE_ENTER" &&
			    trigger.Type != "BOSS_PATTERN_STARTED" && trigger.Type != "HELP_COMMAND" &&
			    trigger.Type != "GUIDE_STARTED" && trigger.Type != "RAID_RETURNED" && trigger.Type != "WORLD_RETURNED")
				throw std::runtime_error("unknown Guide trigger type");
			if (trigger.Type != "HELP_COMMAND" && !trigger.ComboId.empty())
				throw std::runtime_error("only help triggers can start a combo");
			if (!trigger.PromptId.empty() && !candidate.Find_Prompt(trigger.PromptId))
				throw std::runtime_error("Guide trigger has an unknown prompt");
			candidate.Triggers.push_back(std::move(trigger));
		}
		// 추적 거리(m)와 시간(ms→초)을 읽고 최소≤목표≤최대≤재개≤복구 순서를 우선 확인한다. 최종 엄격 부등식/범위 검사는 아래에서 수행한다.
		const auto& combat = root.at("combat");
		const auto& follow = combat.at("follow");
		candidate.DesiredDistance = static_cast<float>(number(follow, "desiredDistanceM", 3));
		candidate.MinimumDistance = static_cast<float>(number(follow, "minimumDistanceM", 2));
		candidate.MaximumDistance = static_cast<float>(number(follow, "maximumDistanceM", 4));
		candidate.ResumeDistance = static_cast<float>(number(follow, "resumeDistanceM", 6));
		candidate.RecoverDistance = static_cast<float>(number(follow, "recoverDistanceM", 30));
		candidate.RecoverDelay = uint(follow, "recoverDelayMs", 3000) / 1000.f;
		if (candidate.MinimumDistance < 0 || candidate.MinimumDistance > candidate.DesiredDistance ||
		    candidate.DesiredDistance > candidate.MaximumDistance ||
		    candidate.MaximumDistance > candidate.ResumeDistance ||
		    candidate.ResumeDistance > candidate.RecoverDistance)
			throw std::runtime_error("invalid Guide follow distance ordering");
		// UI 저작값의 ms를 simulation이 누적하는 초로 바꾼다. 점수 차와 HP 비율은 단위 없는 값으로 유지한다.
		const auto& decision = combat.at("decision");
		candidate.ThinkSeconds = uint(decision, "thinkIntervalMs", 100, 10000) / 1000.f;
		candidate.HorizonSeconds = uint(decision, "horizonMs", 1000, 10000) / 1000.f;
		candidate.SwitchMargin = static_cast<float>(number(decision, "switchMargin", .15));
		candidate.MinimumHoldSeconds = uint(decision, "minimumHoldMs", 250, 10000) / 1000.f;
		candidate.LethalHpFraction = static_cast<float>(number(decision, "lethalHpFraction", .25));
		// 두 mode의 가중치를 읽는다. 음수 검사를 먼저 하고 전체 설정이 준비된 뒤 유한성·0~1·합 1을 함께 확인한다.
		auto weights = [](const Json& row)
		{
			GUIDE_WEIGHTS w;
			w.Attack = static_cast<float>(number(row, "attack", 1));
			w.Avoid = static_cast<float>(number(row, "avoid", 3));
			w.Follow = static_cast<float>(number(row, "follow", 5));
			if (w.Attack < 0 || w.Avoid < 0 || w.Follow < 0)
				throw std::runtime_error("negative Guide weight");
			return w;
		};
		candidate.FollowWeights = weights(combat.at("weights").at("FOLLOW"));
		candidate.AssistWeights = weights(combat.at("weights").at("ASSIST"));
		// publisher가 해석한 input slot/skill ID 순서를 값으로 보관한다. catalog는 ID 범위·개수만 검사하고 설치된 class/slot binding은 룸 초기화가 확인한다.
		for (const auto& c : combat.at("combos").array())
		{
			GUIDE_COMBO combo;
			combo.Id = c.at("comboId").text();
			combo.TimeoutMs = uint(c, "timeoutMs", 45000);
			combo.StepWaitMs = uint(c, "stepWaitMs", 5000);
			combo.Repeat = jsonBoolean(c, "repeat", false);
			for (const auto& s : c.at("inputSlots").array())
				combo.Slots.push_back(s.text());
			if (const auto* ids = c.find("resolvedSkillIds"))
				for (const auto& id : ids->array())
				{
					const auto n = id.number();
					if (n < 1 || n > 4294967295.0 || std::floor(n) != n)
						throw std::runtime_error("invalid resolved skill id");
					combo.Skills.push_back(static_cast<std::uint32_t>(n));
				}
			if (combo.Id.empty() || combo.Slots.empty() || combo.Slots.size() > 64 || candidate.Find_Combo(combo.Id))
				throw std::runtime_error("invalid Guide combo");
			candidate.Combos.push_back(std::move(combo));
		}
		// 명령 별칭과 stop/실행 모드를 읽고 stop이 아닌 명령의 combo 참조를 검사한다. ID/별칭 중복과 최종 제한은 뒤에서 검사한다.
		for (const auto& c : combat.at("commands").array())
		{
			GUIDE_COMMAND command;
			command.Id = c.at("commandId").text();
			command.ComboId = text(c, "comboId");
			command.Stop = jsonBoolean(c, "stop", false);
			command.Enabled = jsonBoolean(c, "enabled", true);
			command.CooldownMs = uint(c, "cooldownMs", 3000);
			for (const auto& a : c.at("aliases").array())
				command.Aliases.push_back(a.text());
			if (!command.Stop && !candidate.Find_Combo(command.ComboId))
				throw std::runtime_error("Guide command has unknown combo");
			candidate.Commands.push_back(std::move(command));
		}
		// 모든 combo/command가 준비된 뒤 trigger 참조를 완성한다. 비활성 명령도 ID가 존재하면 이 참조 검사는 통과한다.
		for (const auto& trigger : candidate.Triggers)
		{
			if (!trigger.ComboId.empty() && !candidate.Find_Combo(trigger.ComboId))
				throw std::runtime_error("Guide trigger has unknown combo");
			if (trigger.Type == "HELP_COMMAND" &&
			    std::none_of(candidate.Commands.begin(), candidate.Commands.end(),
			                 [&](const auto& command) { return command.Id == trigger.PatternId; }))
				throw std::runtime_error("Guide trigger has unknown command");
		}
		// 공개 상태 교체 전 전체 수치·ID·개수 제한을 확인한다. publisher의 모든 저작 schema 검증을 이 runtime loader가 반복하는 것은 아니다.
		auto bounded = [](float value, float low, float high)
		{ return std::isfinite(value) && value >= low && value <= high; };
		if (candidate.Id != "guide.dimensionmaster" || candidate.PlacementId.empty() ||
		    candidate.PlacementId.size() > 128 || !categoryIds.contains(p.at("categoryId").text()) ||
		    !bounded(candidate.Yaw, -36000, 36000) ||
		    (candidate.AnchorPolicy != "INVITER_THEN_LEADER" && candidate.AnchorPolicy != "PARTY_LEADER"))
			throw std::runtime_error("invalid Guide placement contract");
		if (candidate.Prompts.size() > 512 || candidate.Combos.size() > 128 ||
		    !bounded(candidate.MinimumDistance, .1f, 200) || !bounded(candidate.RecoverDistance, .1f, 200) ||
		    candidate.MaximumDistance >= candidate.ResumeDistance ||
		    candidate.ResumeDistance >= candidate.RecoverDistance || !bounded(candidate.RecoverDelay, 1, 60) ||
		    !bounded(candidate.ThinkSeconds, .033f, 1) || !bounded(candidate.HorizonSeconds, .1f, 5) ||
		    !bounded(candidate.SwitchMargin, 0, 1) || !bounded(candidate.MinimumHoldSeconds, 0, 3) ||
		    !bounded(candidate.LethalHpFraction, .1f, 1))
			throw std::runtime_error("invalid Guide decision bounds");
		for (const auto& weights : {candidate.FollowWeights, candidate.AssistWeights})
			if (!bounded(weights.Attack, 0, 1) || !bounded(weights.Avoid, 0, 1) || !bounded(weights.Follow, 0, 1) ||
			    std::abs(weights.Attack + weights.Avoid + weights.Follow - 1) > .001f)
				throw std::runtime_error("invalid normalized Guide weights");
		if (candidate.FollowWeights.Attack != 0)
			throw std::runtime_error("follow mode cannot attack");
		// segment별 UTF-8 유효성과 대사 묶음의 개수·총 byte·총 표시 시간을 검사한다. byte 수는 글자 수와 구분한다.
		for (const auto& prompt : candidate.Prompts)
		{
			std::size_t bytes = 0;
			std::uint32_t duration = 0;
			for (const auto& segment : prompt.Segments)
			{
				bytes += segment.Text.size();
				duration += segment.DurationMs;
				if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, segment.Text.data(),
				                         static_cast<int>(segment.Text.size()), nullptr, 0))
					throw std::runtime_error("invalid prompt UTF-8");
			}
			if (prompt.Id.size() > 128 || prompt.Segments.size() > 16 || bytes > 4096 || duration > 60000)
				throw std::runtime_error("Guide prompt exceeds published limits");
		}
		// trigger/command ID는 각각 중복을 거부한다. 별칭 중복 검사는 Enabled 명령끼리만 적용해 비활성 정의의 별칭 재사용을 허용한다.
		std::set<std::string> triggerIds, commandIds, aliases;
		for (const auto& trigger : candidate.Triggers)
			if (trigger.Id.empty() || trigger.Id.size() > 128 || !triggerIds.insert(trigger.Id).second ||
			    !categoryIds.contains(trigger.Category) ||
			    (trigger.Type == "BOSS_PATTERN_STARTED" && trigger.PatternId.empty()))
				throw std::runtime_error("invalid Guide trigger identity/category");
		for (const auto& combo : candidate.Combos)
			if (combo.Id.size() > 128 || combo.Slots.size() > 32 || combo.Skills.size() != combo.Slots.size() ||
			    combo.TimeoutMs < 1000 || combo.TimeoutMs > 120000 || combo.StepWaitMs > 30000)
				throw std::runtime_error("invalid Guide combo bounds");
		for (const auto& command : candidate.Commands)
		{
			if (command.Id.empty() || command.Id.size() > 128 || !commandIds.insert(command.Id).second ||
			    command.Aliases.empty() || command.Aliases.size() > 32 || command.CooldownMs > 60000 ||
			    (command.Stop && !command.ComboId.empty()))
				throw std::runtime_error("invalid Guide command");
			for (const auto& alias : command.Aliases)
				if (alias.empty() || alias.size() > 128 || (command.Enabled && !aliases.insert(alias).second))
					throw std::runtime_error("invalid or duplicate Guide alias");
		}
		// 모든 파싱·검증을 마친 후보를 공개한다. 이후 성공 status 구성은 이미 교체된 Revision을 사용하며 별도의 rollback은 없다.
		candidate.Loaded = true;
		*this = std::move(candidate);
		status = "Guide revision " + std::to_string(Revision) + " loaded";
		return true;
	}
	// 일반 파싱·검증·할당 예외를 진단 문자열로 돌려준다. 진단 문자열 대입 자체의 실패나 비표준 예외까지 삼키는 noexcept 함수는 아니다.
	catch (const std::exception& error)
	{
		status = error.what();
		return false;
	}
}
// Load의 중복/참조 검사와 룸의 대사 송신이 ID로 목록 원소를 차용한다. 없으면 nullptr이며 복사나 수명 연장은 하지 않는다.
const LostArk::Server::GUIDE_PROMPT* LostArk::Server::CGuideCatalog::Find_Prompt(const std::string& id) const
{
	auto i = std::find_if(Prompts.begin(), Prompts.end(), [&](const auto& p) { return p.Id == id; });
	return i == Prompts.end() ? nullptr : &*i;
}
// Load의 중복/참조 검사와 룸의 현재 combo 실행이 같은 목록을 조회한다. 공개 vector 변경 또는 성공한 재로드 뒤 기존 포인터는 사용하지 않는다.
const LostArk::Server::GUIDE_COMBO* LostArk::Server::CGuideCatalog::Find_Combo(const std::string& id) const
{
	auto i = std::find_if(Combos.begin(), Combos.end(), [&](const auto& p) { return p.Id == id; });
	return i == Combos.end() ? nullptr : &*i;
}
