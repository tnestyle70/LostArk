// ServerApp의 숫자 조회·검증·파일 교체 단계에 필요한 구현이다.
#include "ServerBalanceNumericStore.h"

// 저작 lock, durable 쓰기, backup을 동반하는 파일 교체와 프로세스 식별자를 사용한다.
#include <Windows.h>
// bootstrap·정본·receipt를 byte 내용으로 대조하는 SHA-256을 계산한다.
#include <bcrypt.h>
// stable field 검색과 뒤쪽 offset부터 적용할 수정 목록의 정렬에 사용한다.
#include <algorithm>
// 동시 저장의 임시 파일 이름에 프로세스 내 고유 순번을 부여한다.
#include <atomic>
// locale에 영향받지 않는 숫자 token의 읽기·쓰기에 사용한다.
#include <charconv>
// 유한수, 정수 조건, Retail 무력화 배율의 나눠떨어짐을 확인한다.
#include <cmath>
// 크기를 제한해 저작·설치 파일의 현재 bytes를 읽는다.
#include <fstream>
// double을 왕복 가능한 정밀도로 JSON 숫자에 기록한다.
#include <limits>
// JSON 객체, 문서 캐시, alias별 변경 값과 receipt 파일 목록을 이름으로 찾는다.
#include <map>
// 중복 field 검출과 허용 문서·변경된 패턴 집합을 유지한다.
#include <set>
// 내부 parse·충돌·I/O 실패 문맥을 공개 bool 경계의 status까지 전달한다.
#include <stdexcept>
// 현재 파일 bytes를 복사하지 않고 JSON parser와 경로 탐색에 차용한다.
#include <string_view>
// domain·stable ID·field의 복합 키로 동일 필드 중복 변경을 거부한다.
#include <tuple>
// JSON 값의 종류를 보존해 문자열 identity와 숫자 index를 구분한다.
#include <variant>

#pragma comment(lib, "bcrypt.lib")

namespace
{
    using namespace LostArk::Shared;
    namespace fs = std::filesystem;
    constexpr std::size_t MaximumBytes = 64u * 1024u * 1024u;

    // Server Guide에서 사용한 제한된 JSON 문법이다. 각 값의 원문 범위를 기억해 숫자만 바꿀 때
    // 무관한 bytes·미지정 property·사용자의 배치를 재직렬화하지 않고 보존한다.
    struct Json
    {
        using Object = std::map<std::string, Json, std::less<>>;
        using Array = std::vector<Json>;
        std::variant<std::nullptr_t, bool, double, std::string, Array, Object> Value;
        std::size_t Begin = 0, End = 0;

        // 선택 property 조회다. 객체가 아니거나 key가 없으면 nullptr이며 값은 원문 tree가 소유한다.
        const Json* find(std::string_view key) const
        {
            const auto* object = std::get_if<Object>(&Value);
            if (!object)
                return nullptr;
            const auto it = object->find(key);
            return it == object->end() ? nullptr : &it->second;
        }

        // 필수 property 조회다. 누락은 예외로 공개 저장 함수의 status까지 전달한다.
        const Json& at(std::string_view key) const
        {
            const auto* child = find(key);
            if (!child)
                throw std::runtime_error("Missing JSON field: " + std::string(key));
            return *child;
        }

        const Array& array() const
        {
            return std::get<Array>(Value);
        }

        const std::string& text() const
        {
            return std::get<std::string>(Value);
        }

        double number() const
        {
            return std::get<double>(Value);
        }
    };

    // 파일 bytes는 호출자가 소유한다. parser는 커서와 node 수만 보관하며 깊이·크기 상한을 넘으면 거부한다.
    class Parser
    {
        std::string_view Text;
        std::size_t Cursor = 0, Nodes = 0;

        [[noreturn]] void fail() const
        {
            throw std::runtime_error("Malformed balance JSON at byte " + std::to_string(Cursor));
        }

        void space()
        {
            while (Cursor < Text.size() &&
                   (Text[Cursor] == ' ' || Text[Cursor] == '\r' || Text[Cursor] == '\n' || Text[Cursor] == '\t'))
                ++Cursor;
        }

        // JSON의 \u 뒤 네 자리 16진수를 읽는다. surrogate 결합 검사는 string이 담당한다.
        unsigned hex4()
        {
            unsigned result = 0;
            for (int i = 0; i < 4; ++i)
            {
                if (Cursor == Text.size())
                    fail();
                const char c = Text[Cursor++];
                const unsigned value = c >= '0' && c <= '9'   ? c - '0'
                                       : c >= 'a' && c <= 'f' ? c - 'a' + 10
                                       : c >= 'A' && c <= 'F' ? c - 'A' + 10
                                                              : 16;
                if (value == 16)
                    fail();
                result = (result << 4) | value;
            }
            return result;
        }

        // string이 검증·결합한 코드 포인트를 UTF-8 bytes로 붙인다. 입력 검사를 중복 수행하는 함수는 아니다.
        static void utf8(std::string& out, unsigned value)
        {
            if (value < 0x80)
                out += char(value);
            else if (value < 0x800)
            {
                out += char(0xc0 | (value >> 6));
                out += char(0x80 | (value & 63));
            }
            else if (value < 0x10000)
            {
                out += char(0xe0 | (value >> 12));
                out += char(0x80 | ((value >> 6) & 63));
                out += char(0x80 | (value & 63));
            }
            else
            {
                out += char(0xf0 | (value >> 18));
                out += char(0x80 | ((value >> 12) & 63));
                out += char(0x80 | ((value >> 6) & 63));
                out += char(0x80 | (value & 63));
            }
        }

        // 따옴표·escape와 surrogate 쌍을 검증한다. 완성 전 입력 종료와 제어 문자는 정상 문자열로 받지 않는다.
        std::string string()
        {
            if (Cursor == Text.size() || Text[Cursor++] != '"')
                fail();
            std::string out;
            while (Cursor < Text.size())
            {
                const unsigned char c = Text[Cursor++];
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
                    unsigned value = hex4();
                    if (value >= 0xd800 && value <= 0xdbff)
                    {
                        if (Text.substr(Cursor, 2) != "\\u")
                            fail();
                        Cursor += 2;
                        const unsigned low = hex4();
                        if (low < 0xdc00 || low > 0xdfff)
                            fail();
                        value = 0x10000 + ((value - 0xd800) << 10) + low - 0xdc00;
                    }
                    else if (value >= 0xdc00 && value <= 0xdfff)
                        fail();
                    utf8(out, value);
                    break;
                }
                default:
                    fail();
                }
            }
            fail();
        }

        // 값 종류별로 재귀 파싱하고 [Begin, End) 원문 byte 범위를 보관한다. 중복 객체 key와 숫자 문법 오류를 거부한다.
        Json value(unsigned depth)
        {
            if (depth > 64 || ++Nodes > 4000000)
                fail();
            space();
            if (Cursor == Text.size())
                fail();
            Json result;
            result.Begin = Cursor;
            const char c = Text[Cursor];
            if (c == '"')
                result.Value = string();
            else if (c == '{' || c == '[')
            {
                ++Cursor;
                space();
                Json::Object object;
                Json::Array array;
                const char end = c == '{' ? '}' : ']';
                if (Cursor < Text.size() && Text[Cursor] == end)
                    ++Cursor;
                else
                    for (;;)
                    {
                        space();
                        if (c == '{')
                        {
                            auto key = string();
                            space();
                            if (Cursor == Text.size() || Text[Cursor++] != ':')
                                fail();
                            if (!object.emplace(std::move(key), value(depth + 1)).second)
                                fail();
                        }
                        else
                            array.push_back(value(depth + 1));
                        space();
                        if (Cursor == Text.size())
                            fail();
                        const char separator = Text[Cursor++];
                        if (separator == end)
                            break;
                        if (separator != ',')
                            fail();
                    }
                if (c == '{')
                    result.Value = std::move(object);
                else
                    result.Value = std::move(array);
            }
            else if (Text.substr(Cursor, 4) == "true")
            {
                Cursor += 4;
                result.Value = true;
            }
            else if (Text.substr(Cursor, 5) == "false")
            {
                Cursor += 5;
                result.Value = false;
            }
            else if (Text.substr(Cursor, 4) == "null")
            {
                Cursor += 4;
                result.Value = nullptr;
            }
            else
            {
                const auto start = Cursor;
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
                    const auto begin = Cursor;
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
                    const auto begin = Cursor;
                    while (Cursor < Text.size() && Text[Cursor] >= '0' && Text[Cursor] <= '9')
                        ++Cursor;
                    if (begin == Cursor)
                        fail();
                }
                double number = 0;
                const auto read = std::from_chars(Text.data() + start, Text.data() + Cursor, number);
                if (read.ec != std::errc{} || read.ptr != Text.data() + Cursor || !std::isfinite(number))
                    fail();
                result.Value = number;
            }
            result.End = Cursor;
            return result;
        }

    public:
        // 원문 수명은 parse 완료까지 호출자가 유지한다. UTF-8 BOM이 있으면 커서만 건너뛰어 원문 offset을 보존한다.
        explicit Parser(std::string_view text) : Text(text)
        {
            if (Text.size() > MaximumBytes)
                fail();
            if (Text.substr(0, 3) == "\xef\xbb\xbf")
                Cursor = 3;
        }

        // 최상위 값 뒤에는 공백만 허용한다. 뒤에 붙은 두 번째 값이나 쓰레기 bytes도 저장 입력으로 받지 않는다.
        Json parse()
        {
            auto result = value(0);
            space();
            if (Cursor != Text.size())
                fail();
            return result;
        }
    };

    // receipt에 넣을 문자열을 JSON escape로 감싼다. 원문 document 전체 재직렬화에는 사용하지 않는다.
    std::string quote(std::string_view value)
    {
        constexpr char hex[] = "0123456789abcdef";
        std::string out = "\"";
        for (unsigned char c : value)
        {
            switch (c)
            {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\b':
                out += "\\b";
                break;
            case '\f':
                out += "\\f";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                if (c < 32)
                {
                    out += "\\u00";
                    out += hex[c >> 4];
                    out += hex[c & 15];
                }
                else
                    out += char(c);
            }
        }
        return out + '"';
    }

    // 수정할 숫자 하나를 locale 비의존 token으로 만든다. 비유한수나 변환 실패는 저장을 중단한다.
    std::string number(double value)
    {
        char buffer[64];
        const auto result = std::to_chars(buffer, buffer + sizeof(buffer), value, std::chars_format::general,
                                          std::numeric_limits<double>::max_digits10);
        if (result.ec != std::errc{} || !std::isfinite(value))
            throw std::runtime_error("Invalid numeric token");
        return {buffer, result.ptr};
    }

    // 디스크 숫자와 runtime에서 읽은 이전 값을 대조한다. 허용하는 float 축소 외의 임의 오차 범위는 없다.
    bool equalNumber(double a, double b)
    {
        // bootstrap의 float 필드는 runtime reader에서 이미 정밀도가 줄었다. float 변환 후 같아도 같은 원본 수치로 인정한다.
        return a == b || (std::isfinite(a) && std::isfinite(b) && static_cast<float>(a) == static_cast<float>(b));
    }

    // seal 비교용 객체 key 순서를 고정한다. 배열 순서는 보존하고 숫자·bool·null은 원문 token을 유지한다.
    std::string canonical(const Json& value, std::string_view source)
    {
        if (const auto* object = std::get_if<Json::Object>(&value.Value))
        {
            std::string out = "{";
            bool first = true;
            for (const auto& [key, child] : *object)
            {
                if (!first)
                    out += ',';
                first = false;
                out += quote(key) + ':' + canonical(child, source);
            }
            return out + '}';
        }
        if (const auto* array = std::get_if<Json::Array>(&value.Value))
        {
            std::string out = "[";
            bool first = true;
            for (const auto& child : *array)
            {
                if (!first)
                    out += ',';
                first = false;
                out += canonical(child, source);
            }
            return out + ']';
        }
        if (const auto* text = std::get_if<std::string>(&value.Value))
            return quote(*text);
        return std::string(source.substr(value.Begin, value.End - value.Begin));
    }

    // 파일·정규화된 seal의 bytes를 SHA-256 문자열로 만든다. 생성한 BCrypt provider는 성공 여부와 무관하게 닫는다.
    std::string sha256(std::string_view bytes)
    {
        BCRYPT_ALG_HANDLE algorithm = nullptr;
        if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0)
            throw std::runtime_error("SHA256 provider unavailable");
        GameplayDataRevision digest{};
        const auto result =
            BCryptHash(algorithm, nullptr, 0, reinterpret_cast<PUCHAR>(const_cast<char*>(bytes.data())),
                       static_cast<ULONG>(bytes.size()), digest.Bytes.data(), static_cast<ULONG>(digest.Bytes.size()));
        BCryptCloseAlgorithmProvider(algorithm, 0);
        if (result < 0)
            throw std::runtime_error("SHA256 failed");
        return Format_GameplayDataRevision(digest);
    }

    // 파일 전체를 상한 안에서 읽어 소유 문자열로 반환한다. 짧게 읽힌 파일은 정상 원본으로 인정하지 않는다.
    std::string readFile(const fs::path& path)
    {
        std::ifstream stream(path, std::ios::binary | std::ios::ate);
        if (!stream)
            throw std::runtime_error("Cannot read " + path.string());
        const auto size = stream.tellg();
        if (size < 0 || size > static_cast<std::streamoff>(MaximumBytes))
            throw std::runtime_error("File exceeds balance persistence limit: " + path.string());
        std::string out(static_cast<std::size_t>(size), '\0');
        stream.seekg(0);
        if (!out.empty() && !stream.read(out.data(), static_cast<std::streamsize>(out.size())))
            throw std::runtime_error("Incomplete read: " + path.string());
        return out;
    }

    // bootstrap 비교에서 BOM과 CRLF/CR 차이를 정규화한다. gameplay 행의 값이나 순서는 바꾸지 않는다.
    std::string normalize(std::string_view text)
    {
        if (text.substr(0, 3) == "\xef\xbb\xbf")
            text.remove_prefix(3);
        std::string out;
        out.reserve(text.size());
        for (std::size_t i = 0; i < text.size(); ++i)
        {
            if (text[i] == '\r')
            {
                out += '\n';
                if (i + 1 < text.size() && text[i + 1] == '\n')
                    ++i;
            }
            else
                out += text[i];
        }
        return out;
    }

    // 정해진 Windows 버퍼에 환경 경로를 읽는다. 미지정·버퍼 초과는 빈 경로로 반환한다.
    fs::path environment(const wchar_t* name)
    {
        wchar_t buffer[32768]{};
        const DWORD count = GetEnvironmentVariableW(name, buffer, 32768);
        return count && count < 32768 ? fs::path(buffer) : fs::path{};
    }

    // 현재 Server EXE의 위치를 설치 DataFiles 탐색 기준으로 얻는다. 실패는 임의 작업 디렉터리로 숨기지 않는다.
    fs::path moduleDirectory()
    {
        wchar_t buffer[32768]{};
        const DWORD count = GetModuleFileNameW(nullptr, buffer, 32768);
        if (!count || count >= 32768)
            throw std::runtime_error("Cannot locate Server executable");
        return fs::path(buffer).parent_path();
    }

    // 명시한 Data 환경 경로를 우선한다. 없으면 작업 위치·bootstrap·EXE의 조상에서 Retail 정본을 찾는다.
    fs::path authoringRoot(const fs::path& bootstrap)
    {
        if (auto configured = environment(L"LOSTARK_PROJECT_DATA_ROOT"); !configured.empty())
            return fs::absolute(configured).lexically_normal();
        for (auto start : {fs::current_path(), bootstrap.parent_path(), moduleDirectory()})
        {
            for (auto current = fs::absolute(start); !current.empty(); current = current.parent_path())
            {
                if (fs::exists(current / L"Data" / L"Balance" / L"Profiles" / L"Retail.balanceprofile.json"))
                    return current / L"Data";
                if (current == current.root_path())
                    break;
            }
        }
        return {};
    }

    // 한 JSON 값의 반개방 원문 byte 범위와 교체 token이다. Document의 Before를 기준으로 한 offset이다.
    struct Replacement
    {
        std::size_t Begin, End;
        std::string Text;
    };

    // 원본 bytes와 parse tree 및 수정 후보를 함께 소유한다. output 전에는 디스크를 쓰지 않는다.
    struct Document
    {
        fs::path Path;
        std::string Before;
        Json Root;
        std::vector<Replacement> Replacements;

        explicit Document(fs::path path) : Path(std::move(path)), Before(readFile(Path)), Root(Parser(Before).parse())
        {
        }

        // 같은 값의 후속 수정은 갱신하되 부분 겹침은 거부한다. 두 의미가 다른 원문 범위를 조용히 덮지 않는다.
        void replace(const Json& node, std::string text)
        {
            for (auto& current : Replacements)
            {
                // 같은 패턴의 다른 숫자를 합친 뒤 receipt 전체 값을 다시 만들 수 있다.
                // 정확히 같은 범위의 후속 수정은 최종 조합값으로 대체한다.
                if (node.Begin == current.Begin && node.End == current.End)
                {
                    current.Text = std::move(text);
                    return;
                }
                if (node.Begin < current.End && current.Begin < node.End)
                    throw std::runtime_error("Overlapping source edits");
            }
            Replacements.push_back({node.Begin, node.End, std::move(text)});
        }

        // 요청이 읽었던 이전 숫자와 최신 원본이 같을 때만 해당 token의 교체를 준비한다.
        void scalar(const Json& node, double before, double after)
        {
            if (!equalNumber(node.number(), before))
                throw std::runtime_error("CONFLICT: numeric source changed on disk: " + Path.string());
            replace(node, number(after));
        }

        // 뒤쪽 offset부터 적용해 앞쪽 주소가 밀리지 않게 한다. 조합한 문서를 다시 parse한 뒤 bytes를 반환한다.
        std::string output() const
        {
            auto edits = Replacements;
            std::sort(edits.begin(), edits.end(), [](const auto& a, const auto& b) { return a.Begin > b.Begin; });
            std::string out = Before;
            for (const auto& edit : edits)
                out.replace(edit.Begin, edit.End - edit.Begin, edit.Text);
            Parser(out).parse();
            return out;
        }
    };

    // 배열에서 stable identity를 정확히 하나 찾는다. 없음은 nullptr, 둘 이상은 모호한 저작 데이터로 거부한다.
    const Json* findRow(const Json& document, std::string_view array, std::string_view key, const std::string& identity)
    {
        const Json* result = nullptr;
        const auto* list = document.find(array);
        if (!list)
            return nullptr;
        for (const auto& row : list->array())
        {
            const auto& id = row.at(key);
            const auto current = std::holds_alternative<std::string>(id.Value) ? id.text() : number(id.number());
            if (current != identity)
                continue;
            if (result)
                throw std::runtime_error("Ambiguous stable JSON identity: " + identity);
            result = &row;
        }
        return result;
    }

    // 반드시 존재해야 하는 stable 행을 구한다. 사용자에게 전달할 identity를 오류에 포함한다.
    const Json& requiredRow(const Json& document, std::string_view array, std::string_view key,
                            const std::string& identity)
    {
        const auto* row = findRow(document, array, key, identity);
        if (!row)
            throw std::runtime_error("Missing stable source identity: " + identity);
        return *row;
    }

    // 복합 stable ID를 구분자 기준으로 해석한다. 빈 항목도 보존하므로 의미 검사는 호출자가 수행한다.
    std::vector<std::string> split(std::string_view value, char separator)
    {
        std::vector<std::string> result;
        for (;;)
        {
            const auto end = value.find(separator);
            result.emplace_back(value.substr(0, end));
            if (end == std::string_view::npos)
                return result;
            value.remove_prefix(end + 1);
        }
    }

    using Documents = std::map<std::string, std::unique_ptr<Document>>;

    // Data 상대 문서를 한 번 읽어 같은 저장 요청의 수정들이 동일한 Before와 tree를 공유하게 한다.
    Document& document(Documents& documents, const fs::path& data, const std::string& relative)
    {
        auto& result = documents[relative];
        if (!result)
            result = std::make_unique<Document>(data / fs::path(relative));
        return *result;
    }

    // 바뀐 저작 field의 provenance를 PROJECT_TUNED로 갱신한다. 공식 근거인 것처럼 기존 basis를 남기지 않는다.
    void updateReceipt(Documents& documents, const fs::path& data, const std::string& targetDocument,
                       const std::string& targetId, const std::string& field, const std::string& value,
                       bool optional = false)
    {
        auto& receipt =
            document(documents, data, "Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json");
        const Json* found = nullptr;
        for (const auto& row : receipt.Root.at("entries").array())
            if (row.at("targetDocument").text() == targetDocument && row.at("targetId").text() == targetId &&
                row.at("targetField").text() == field)
            {
                if (found)
                    throw std::runtime_error("Duplicate provenance receipt field");
                found = &row;
            }
        if (!found)
        {
            if (optional)
                return;
            throw std::runtime_error("Missing provenance receipt field: " + targetId + "." + field);
        }
        receipt.replace(found->at("basis"), quote("PROJECT_TUNED"));
        receipt.replace(found->at("source"),
                        "{\"type\":\"project-policy\",\"policyId\":\"balance-tool-authored-override-v1\"}");
        receipt.replace(found->at("sourceValue"), value);
        receipt.replace(found->at("resultValue"), value);
        receipt.replace(found->at("transform"), quote("Balance Tool authored override"));
        if (const auto* note = found->find("note"))
            receipt.replace(
                *note,
                quote("Changed through the F1 Balance Tool; re-export official sources to restore an official basis."));
    }

    // 기존 ENTER 무력화 action을 가리키는 차용 포인터다. Document의 tree를 파괴하기 전까지만 사용한다.
    struct StageAddress
    {
        const Json* Pattern;
        const Json* Stage;
        const Json* Action;
        std::size_t StageIndex;
    };

    // action ID와 ordinal을 실제 stage에 연결하고 양수 ENTER gauge인지 확인한다. 새 action을 합성하지 않는다.
    StageAddress stageAddress(const Json& pattern, const std::string& actionId, std::size_t actionIndex)
    {
        const auto& stages = pattern.at("stages").array();
        const Json* selected = nullptr;
        std::size_t selectedIndex = 0;
        for (std::size_t i = 0; i < stages.size(); ++i)
            if (stages[i].at("actionId").text() == actionId)
            {
                if (selected)
                    throw std::runtime_error("Ambiguous stage actionId");
                selected = &stages[i];
                selectedIndex = i;
            }
        if (!selected || actionIndex >= selected->at("actions").array().size())
            throw std::runtime_error("Missing stagger stage action");
        const auto& action = selected->at("actions").array()[actionIndex];
        if (action.at("kind").text() != "SET_STAGGER_GAUGE" || action.at("trigger").text() != "ENTER" ||
            action.at("value").number() <= 0)
            throw std::runtime_error("Stagger source is not an existing positive ENTER gauge");
        return {&pattern, selected, &action, selectedIndex};
    }

    // 숫자 변경을 Retail override 또는 원본 stable field에 배치한다. 발탄 무력화는 같은 원본 의미를 가진 두 문서를 함께 준비한다.
    void stageSources(Documents& documents, const fs::path& data, const std::vector<BALANCE_NUMERIC_CHANGE>& changes)
    {
        auto& retail = document(documents, data, "Balance/Profiles/Retail.balanceprofile.json");
        std::set<std::string> staggerPatterns;
        for (const auto& change : changes)
        {
            std::string array, idKey, base, prefix;
            switch (change.eDomain)
            {
            case BALANCE_DOMAIN::PLAYER:
                array = "players";
                idKey = "characterClass";
                base = "Balance/PlayerProfiles.json";
                prefix = "player:";
                break;
            case BALANCE_DOMAIN::SKILL:
                array = "skills";
                idKey = "skillId";
                base = "Balance/PlayerSkills.json";
                prefix = "skill:";
                break;
            case BALANCE_DOMAIN::DAMAGE:
                array = "damageProfiles";
                idKey = "damageProfileId";
                base = "Balance/DamageProfiles.json";
                prefix = "damage:";
                break;
            case BALANCE_DOMAIN::BOSS:
                array = "bosses";
                idKey = "archetypeId";
                base = "Balance/BossProfiles.json";
                prefix = "boss:";
                break;
            case BALANCE_DOMAIN::MADNESS:
                array = "madness";
                idKey = "policyId";
                break;
            case BALANCE_DOMAIN::PATTERN_DAMAGE:
                continue; // 아래의 게시된 source binding으로 정확한 저작 위치를 찾는다.
            case BALANCE_DOMAIN::STAGGER:
            {
                if (change.strId == "RAID_COMMON" && change.strField == "staggerGaugeMaximum")
                {
                    if (change.fValue < 1. || change.fValue > 1000000000. || std::floor(change.fValue) != change.fValue)
                        throw std::runtime_error("Raid stagger maximum must be a whole number in 1..1000000000");
                    retail.scalar(retail.Root.at("raidStaggerMaximum"), change.fBefore, change.fValue);
                    continue;
                }
                const auto id = split(change.strId, '|');
                if (id.size() != 4 || id[0] != "ENCOUNTER_VALTAN" || change.strField != "staggerGaugeMaximum")
                    throw std::runtime_error("Unsupported stagger source owner");
                std::size_t ordinal = 0;
                const auto converted = std::from_chars(id[3].data(), id[3].data() + id[3].size(), ordinal);
                if (converted.ec != std::errc{} || converted.ptr != id[3].data() + id[3].size())
                    throw std::runtime_error("Invalid stagger action ordinal");
                const double scale = retail.Root.at("staggerGaugeScale").number();
                if (scale < 1 || std::floor(scale) != scale || std::fmod(change.fValue, scale) != 0 ||
                    std::fmod(change.fBefore, scale) != 0)
                    throw std::runtime_error("Stagger gauge must be a multiple of Retail staggerGaugeScale (" +
                                             number(scale) + ")");
                auto& legacy = document(documents, data, "Valtan/Valtan.legacy-compatibility.json");
                const auto& entry = requiredRow(legacy.Root, "patternEntries", "patternId", id[1]);
                if (sha256(canonical(entry.at("runtimePattern"), legacy.Before)) !=
                    entry.at("encounterRowSha256").text())
                    throw std::runtime_error(
                        "Stagger legacy source seal changed or uses an unsupported numeric encoding");
                auto owner = stageAddress(entry.at("runtimePattern"), id[2], ordinal);
                legacy.scalar(owner.Action->at("value"), change.fBefore / scale, change.fValue / scale);
                auto& encounter = document(documents, data, "Encounters/Valtan/ValtanEncounter.json");
                const auto& pattern = requiredRow(encounter.Root, "patterns", "patternId", id[1]);
                owner = stageAddress(pattern, id[2], ordinal);
                encounter.scalar(owner.Action->at("value"), change.fBefore / scale, change.fValue / scale);
                staggerPatterns.insert(id[1]);
                continue;
            }
            default:
                throw std::runtime_error("Unknown balance domain");
            }
            const auto* row = findRow(retail.Root, array, idKey, change.strId);
            const auto* field = row ? row->find(change.strField) : nullptr;
            if (field)
            {
                retail.scalar(*field, change.fBefore, change.fValue);
                continue;
            }
            if (base.empty())
                throw std::runtime_error("Missing Retail field: " + change.strId + "." + change.strField);
            auto& source = document(documents, data, base);
            if (change.eDomain == BALANCE_DOMAIN::DAMAGE)
                array = "profiles";
            const auto& sourceRow = requiredRow(source.Root, array, idKey, change.strId);
            source.scalar(sourceRow.at(change.strField), change.fBefore, change.fValue);
            updateReceipt(documents, data, "Data/" + base, prefix + change.strId, change.strField,
                          number(change.fValue));
        }
        // 같은 패턴의 숫자 변경을 모두 합친 뒤 바뀐 legacy seal과 활성 패턴 receipt의 stages만 다시 계산한다.
        if (!staggerPatterns.empty())
        {
            auto& legacy = document(documents, data, "Valtan/Valtan.legacy-compatibility.json");
            const auto nextLegacy = legacy.output();
            const auto parsedLegacy = Parser(nextLegacy).parse();
            auto& encounter = document(documents, data, "Encounters/Valtan/ValtanEncounter.json");
            const auto nextEncounter = encounter.output();
            const auto parsedEncounter = Parser(nextEncounter).parse();
            for (const auto& identity : staggerPatterns)
            {
                const auto& before = requiredRow(legacy.Root, "patternEntries", "patternId", identity);
                const auto& after = requiredRow(parsedLegacy, "patternEntries", "patternId", identity);
                legacy.replace(before.at("encounterRowSha256"),
                               quote(sha256(canonical(after.at("runtimePattern"), nextLegacy))));
                const auto& pattern = requiredRow(parsedEncounter, "patterns", "patternId", identity);
                if (pattern.at("selectionMode").text() == "AUDITION_ONLY")
                    continue;
                std::size_t index = 0;
                for (const auto& row : parsedEncounter.at("patterns").array())
                {
                    if (row.at("selectionMode").text() == "AUDITION_ONLY")
                        continue;
                    if (row.at("patternId").text() == identity)
                        break;
                    ++index;
                }
                updateReceipt(documents, data, "Data/Encounters/Valtan/ValtanEncounter.json", "pattern:" + identity,
                              "patterns[" + std::to_string(index) + "].stages",
                              canonical(pattern.at("stages"), nextEncounter));
            }
        }
    }

    // 게시 binding의 key/index 경로를 따라 현재 tree 값을 찾는다. 깊이·배열 범위 오류는 이전 주소의 유효성을 잃은 것으로 거부한다.
    const Json& jsonPath(const Json& root, const Json& path)
    {
        const Json* node = &root;
        if (path.array().empty() || path.array().size() > 32)
            throw std::runtime_error("Invalid numeric source path depth");
        for (const auto& step : path.array())
        {
            if (std::holds_alternative<std::string>(step.Value))
                node = &node->at(step.text());
            else
            {
                const double index = step.number();
                if (index < 0 || index != std::floor(index) || index >= node->array().size())
                    throw std::runtime_error("Numeric source index is no longer valid");
                node = &node->array()[static_cast<std::size_t>(index)];
            }
        }
        return *node;
    }

    // 숫자 저장이 손댈 수 있는 저작 정본과 게시 사본을 명시한다. binding의 임의 경로를 파일 쓰기 권한으로 쓰지 않는다.
    const std::set<std::string> BindingDocuments{
        "Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json",
        "Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json",
        "Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json",
        "Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.worldsequences.json",
        "Data/Valtan/Valtan.gameplay.json",
        "Data/Encounters/Valtan/ValtanEncounter.json"};

    // 허용된 package 상대 문서만 연다. Data 상대 캐시에 이미 있는 동일 물리 문서는 중복 생성하지 않는다.
    Document& boundDocument(Documents& documents, const fs::path& data, const std::string& relative)
    {
        if (!BindingDocuments.contains(relative))
            throw std::runtime_error("Numeric binding document is outside the allowlist");
        const auto path = (data.parent_path() / fs::path(relative)).lexically_normal();
        for (const auto& [key, existing] : documents)
            if (existing->Path.lexically_normal() == path)
                return *existing;
        // 게시된 불변 주소에는 설치 Map 사본도 포함된다. 이 연결에서는 Data 상대 경로와 구별되는
        // package 상대 이름을 캐시 키로 유지한다.
        const auto key = "@" + relative;
        auto& result = documents[key];
        if (!result)
            result = std::make_unique<Document>(path);
        return *result;
    }

    // binding의 schema/version과 항목 상한을 검사한다. 각 source의 현재 identity·hash는 저장 준비에서 별도로 확인한다.
    void validateBindings(const Json& root)
    {
        if (root.at("schema").text() != "lostark.numeric-source-bindings" || root.at("formatVersion").number() != 1 ||
            root.at("entries").array().size() > 4096 || root.at("files").array().size() > BindingDocuments.size())
            throw std::runtime_error("Invalid numeric source binding metadata");
    }

    // 같은 저작 피해 정의를 공유하는 alias에 요청 값을 확장한다. 서로 다른 새 값이나 현재 snapshot에 없는 연결은 거부한다.
    std::vector<BALANCE_NUMERIC_CHANGE> expandChanges(const std::vector<BALANCE_NUMERIC_CHANGE>& requested,
                                                      const std::vector<BALANCE_NUMERIC_ENTRY>& snapshot,
                                                      const std::string& metadata)
    {
        if (std::none_of(requested.begin(), requested.end(),
                         [](const auto& c) { return c.eDomain == BALANCE_DOMAIN::PATTERN_DAMAGE; }))
            return requested;
        if (metadata.empty())
            throw std::runtime_error("Pattern damage bindings are unavailable; publish numeric source bindings first");
        const auto bindings = Parser(metadata).parse();
        validateBindings(bindings);
        std::map<std::string, double> aliases;
        std::vector<BALANCE_NUMERIC_CHANGE> result;
        for (const auto& change : requested)
        {
            if (change.eDomain != BALANCE_DOMAIN::PATTERN_DAMAGE)
            {
                result.push_back(change);
                continue;
            }
            const Json* found = nullptr;
            for (const auto& entry : bindings.at("entries").array())
                if (entry.at("id").text() == change.strId && entry.at("field").text() == change.strField)
                {
                    if (found)
                        throw std::runtime_error("Duplicate numeric source binding");
                    found = &entry;
                }
            if (!found)
                throw std::runtime_error("Pattern damage field has no canonical source binding");
            const auto [previous, inserted] = aliases.emplace(found->at("alias").text(), change.fValue);
            if (!inserted && previous->second != change.fValue)
                throw std::runtime_error("Conflicting values for one shared pattern damage definition");
        }
        for (const auto& entry : bindings.at("entries").array())
        {
            const auto selected = aliases.find(entry.at("alias").text());
            if (selected == aliases.end())
                continue;
            const auto current = std::find_if(snapshot.begin(), snapshot.end(),
                                              [&](const auto& row)
                                              {
                                                  return row.eDomain == BALANCE_DOMAIN::PATTERN_DAMAGE &&
                                                         row.strId == entry.at("id").text() &&
                                                         row.strField == entry.at("field").text();
                                              });
            if (current == snapshot.end())
                throw std::runtime_error("Numeric source binding belongs to a different gameplay generation");
            result.push_back({current->eDomain, current->strId, current->strField, current->fValue, selected->second});
        }
        if (result.size() > 4096)
            throw std::runtime_error("Shared pattern damage expansion exceeds its bound");
        return result;
    }

    // 게시된 source/mirror의 hash와 stable guard를 현재 파일에 대조한 뒤 피해 token을 준비한다. 변경 후 binding hash와 receipt도 함께 갱신한다.
    void stagePatternDamage(Documents& documents, const fs::path& data,
                            const std::vector<BALANCE_NUMERIC_CHANGE>& changes, const std::string& cachedBindings)
    {
        const auto file = data / L"Balance" / L"NumericSourceBindings.json";
        const bool hasDamage = std::any_of(changes.begin(), changes.end(),
                                           [](const auto& c) { return c.eDomain == BALANCE_DOMAIN::PATTERN_DAMAGE; });
        if (!fs::exists(file))
        {
            if (hasDamage)
                throw std::runtime_error("Missing numeric source bindings");
            return;
        }
        auto& metadata = document(documents, data, "Balance/NumericSourceBindings.json");
        validateBindings(metadata.Root);
        if (hasDamage)
        {
            const auto cached = Parser(cachedBindings).parse();
            if (canonical(cached.at("entries"), cachedBindings) !=
                canonical(metadata.Root.at("entries"), metadata.Before))
                throw std::runtime_error(
                    "CONFLICT: numeric source ownership changed; restart the Server after publication");
            for (const auto& row : metadata.Root.at("files").array())
            {
                auto& source = boundDocument(documents, data, row.at("path").text());
                if (sha256(source.Before) != row.at("sha256").text())
                    throw std::runtime_error("CONFLICT: pattern source changed since numeric bindings were generated");
            }
            for (const auto& change : changes)
            {
                if (change.eDomain != BALANCE_DOMAIN::PATTERN_DAMAGE)
                    continue;
                const auto& binding = requiredRow(metadata.Root, "entries", "id", change.strId);
                if (binding.at("field").text() != change.strField)
                    throw std::runtime_error("Pattern source field changed");
                for (const char* group : {"sources", "mirrors"})
                    for (const auto& location : binding.at(group).array())
                    {
                        auto& source = boundDocument(documents, data, location.at("document").text());
                        for (const auto& guard : location.at("guards").array())
                            if (jsonPath(source.Root, guard.at("path")).text() != guard.at("value").text())
                                throw std::runtime_error("CONFLICT: stable pattern source identity moved");
                        source.scalar(jsonPath(source.Root, location.at("path")), change.fBefore, change.fValue);
                    }
            }
        }
        // 피해 외 숫자 변경도 source hash를 갱신한다. 무력화 수정이 피해 field와 같은 Valtan Encounter를 바꿀 수 있다.
        for (const auto& row : metadata.Root.at("files").array())
        {
            const auto path = (data.parent_path() / fs::path(row.at("path").text())).lexically_normal();
            for (const auto& [key, source] : documents)
                if (source->Path.lexically_normal() == path && !source->Replacements.empty())
                    metadata.replace(row.at("sha256"), quote(sha256(source->output())));
        }
        if (hasDamage)
        {
            const auto encounterPath = (data / L"Encounters" / L"Valtan" / L"ValtanEncounter.json").lexically_normal();
            const auto found = std::find_if(documents.begin(), documents.end(), [&](const auto& entry)
                                            { return entry.second->Path.lexically_normal() == encounterPath; });
            if (found != documents.end() && !found->second->Replacements.empty())
            {
                auto& source = *found->second;
                const auto next = source.output();
                const auto after = Parser(next).parse();
                std::size_t index = 0;
                for (const auto& pattern : after.at("patterns").array())
                {
                    if (pattern.at("selectionMode").text() == "AUDITION_ONLY")
                        continue;
                    const auto& before =
                        requiredRow(source.Root, "patterns", "patternId", pattern.at("patternId").text());
                    if (canonical(before.at("stages"), source.Before) != canonical(pattern.at("stages"), next))
                        updateReceipt(documents, data, "Data/Encounters/Valtan/ValtanEncounter.json",
                                      "pattern:" + pattern.at("patternId").text(),
                                      "patterns[" + std::to_string(index) + "].stages",
                                      canonical(pattern.at("stages"), next));
                    ++index;
                }
            }
        }
    }

    // 지역 Win32 handle 소유자다. 실패 예외가 전파돼도 파일·lock handle을 정리한다.
    struct Handle
    {
        HANDLE Value = INVALID_HANDLE_VALUE;

        ~Handle()
        {
            if (Value != INVALID_HANDLE_VALUE && Value != nullptr)
                CloseHandle(Value);
        }
    };

    // 패턴 저작과 같은 파일 lock을 사용해 canonical writer의 동시 저장을 거부한다. 소멸 시 잠금 다음 handle 순으로 정리한다.
    struct Admission
    {
        Handle File;
        OVERLAPPED Position{};
        bool Locked = false;

        explicit Admission(const fs::path& data)
        {
            const auto path = data.parent_path() / L"out" / L"ValtanPatternTransactions" / L"create-pattern.lock";
            fs::create_directories(path.parent_path());
            File.Value = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (File.Value == INVALID_HANDLE_VALUE ||
                !LockFileEx(File.Value, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY, 0, 1, 0, &Position))
                throw std::runtime_error("Balance save is busy: canonical authoring writer owns the source");
            Locked = true;
        }

        ~Admission()
        {
            if (Locked)
                UnlockFileEx(File.Value, 0, 1, 0, &Position);
        }
    };

    // 파일별 이전/다음 bytes, stage·backup 경로와 교체 여부다. 역순 보상에서 자기 변경인지 대조하는 근거다.
    struct DiskEdit
    {
        fs::path Path, Staged, Backup;
        std::string Before, After;
        bool Promoted = false, Existed = true;
    };

    // 새 임시 파일에 후보를 쓰고 FlushFileBuffers로 밀어낸다. 기존 이름을 덮어쓰지 않는다.
    void writeDurable(const fs::path& path, const std::string& bytes)
    {
        Handle file;
        file.Value = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file.Value == INVALID_HANDLE_VALUE)
            throw std::runtime_error("Cannot stage balance file: " + path.string());
        DWORD written = 0;
        if (!WriteFile(file.Value, bytes.data(), static_cast<DWORD>(bytes.size()), &written, nullptr) ||
            written != bytes.size() || !FlushFileBuffers(file.Value))
            throw std::runtime_error("Could not flush balance file: " + path.string());
    }

    // 모든 후보를 stage한 뒤 교체 직전마다 원본 freshness를 확인한다. 각 파일은 원자적으로 교체하되 전체 묶음은 실패 시 역순 보상한다.
    void persist(std::vector<DiskEdit>& edits, const Documents& documents)
    {
        static std::atomic<std::uint64_t> ordinal{0};
        const auto tag = L".balance." + std::to_wstring(GetCurrentProcessId()) + L"." +
                         std::to_wstring(GetTickCount64()) + L"." + std::to_wstring(++ordinal);
        // 이미 교체한 파일은 After, 아직 바꾸지 않은 파일은 Before와 비교한다. 외부 동시 저장을 자신의 교체로 오인하지 않는다.
        auto assertFresh = [&]()
        {
            for (const auto& [relative, source] : documents)
            {
                const auto found = std::find_if(edits.begin(), edits.end(),
                                                [&](const auto& edit) { return edit.Path == source->Path; });
                const auto& expected = found != edits.end() && found->Promoted ? found->After : source->Before;
                if (readFile(source->Path) != expected)
                    throw std::runtime_error("CONFLICT: source changed during balance save: " + relative);
            }
            for (const auto& edit : edits)
            {
                if (!edit.Existed && !edit.Promoted)
                {
                    if (fs::exists(edit.Path))
                        throw std::runtime_error("CONFLICT: new destination appeared during balance save");
                }
                else if (readFile(edit.Path) != (edit.Promoted ? edit.After : edit.Before))
                    throw std::runtime_error("CONFLICT: destination changed during balance save");
            }
        };
        try
        {
            for (auto& edit : edits)
            {
                edit.Staged = edit.Path;
                edit.Staged += tag + L".staged";
                edit.Backup = edit.Path;
                edit.Backup += tag + L".backup";
                writeDurable(edit.Staged, edit.After);
            }
            assertFresh();
            for (auto& edit : edits)
            {
                assertFresh();
                const bool replaced =
                    edit.Existed ? !!ReplaceFileW(edit.Path.c_str(), edit.Staged.c_str(), edit.Backup.c_str(), 0,
                                                  nullptr, nullptr)
                                 : !!MoveFileExW(edit.Staged.c_str(), edit.Path.c_str(), MOVEFILE_WRITE_THROUGH);
                if (!replaced)
                    throw std::runtime_error("Atomic balance replacement failed: " + edit.Path.string());
                edit.Promoted = true;
            }
            assertFresh();
        }
        catch (...)
        {
            // 현재 bytes가 자신이 쓴 After일 때만 되돌린다. 복구 API 실패나 외부 수정 시 backup을 남기며 완전 복구를 보장하지 않는다.
            for (auto it = edits.rbegin(); it != edits.rend(); ++it)
            {
                if (it->Promoted)
                {
                    try
                    {
                        if (readFile(it->Path) == it->After)
                        {
                            if (it->Existed)
                                ReplaceFileW(it->Path.c_str(), it->Backup.c_str(), nullptr, 0, nullptr, nullptr);
                            else
                                DeleteFileW(it->Path.c_str());
                        }
                    }
                    catch (...)
                    { /* 복구를 위해 durable backup을 남긴다. */
                    }
                }
                std::error_code error;
                if (!it->Staged.empty())
                    fs::remove(it->Staged, error);
            }
            throw;
        }
    }

    // 기존 저장 receipt가 가리킨 파일도 최신 hash와 대조한다. 승인된 목적지의 다음 hash만 새 receipt에 합친다.
    void addSaveReceipt(std::vector<DiskEdit>& edits, const fs::path& data, const fs::path& bootstrap,
                        const GameplayDataRevision& numericRevision, const std::string& numericReceipt,
                        const fs::path& runtimePointer)
    {
        const auto path = bootstrap.parent_path() / L"BalanceNumeric.save.receipt.json";
        const bool exists = fs::exists(path);
        const auto before = exists ? readFile(path) : std::string{};
        std::map<std::string, std::string> hashes;
        const std::set<std::string> allowed{
            "Data/Balance/PlayerProfiles.json",
            "Data/Balance/PlayerSkills.json",
            "Data/Balance/DamageProfiles.json",
            "Data/Balance/BossProfiles.json",
            "Data/Balance/Profiles/Retail.balanceprofile.json",
            "Data/Balance/Reference/Official/2026-08-05.balance-provenance.receipt.json",
            "Data/Valtan/Valtan.legacy-compatibility.json",
            "Data/Encounters/Valtan/ValtanEncounter.json",
            "Data/Valtan/Valtan.gameplay.json",
            "Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap",
            "Server/Bin/DataFiles/Gameplay/NumericBalance.active.json",
            "Data/KoukuSaydon/Gate1/KoukuSaydonComposition.json",
            "Data/Encounters/KoukuSaydon/KoukuSaydonEncounter.json",
            "Data/Balance/NumericSourceBindings.json",
            "Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.worldsequences.json",
            "Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.worldsequences.json"};
        if (exists)
        {
            const auto old = Parser(before).parse();
            if (old.at("schema").text() != "lostark.numeric-balance-save")
                throw std::runtime_error("Invalid numeric save receipt");
            for (const auto& file : old.at("files").array())
            {
                const auto relative = file.at("path").text();
                if (!allowed.contains(relative) || !hashes.emplace(relative, file.at("sha256").text()).second ||
                    sha256(readFile(data.parent_path() / fs::path(relative))) != file.at("sha256").text())
                    throw std::runtime_error("CONFLICT: previously saved numeric source changed: " + relative);
            }
        }
        for (const auto& edit : edits)
        {
            if (!runtimePointer.empty() && edit.Path == runtimePointer)
                continue;
            auto relative = edit.Path.lexically_relative(data.parent_path()).generic_string();
            if (!allowed.contains(relative))
            {
                // 검사·설치가 명시한 별도 server-data root는 허용한다. 이 물리 경로를
                // package 상대 저작 override로 기록하지는 않는다.
                if (edit.Path == bootstrap || edit.Path == bootstrap.parent_path() / L"NumericBalance.active.json")
                    continue;
                throw std::runtime_error("Numeric save destination is outside its source allowlist: " + relative);
            }
            hashes[relative] = sha256(edit.After);
        }
        const auto active = Parser(numericReceipt).parse();
        std::string after = "{\"schema\":\"lostark.numeric-balance-save\",\"numericRevision\":" +
                            quote(Format_GameplayDataRevision(numericRevision)) +
                            ",\"bootstrapContentSha256\":" + quote(active.at("bootstrapContentSha256").text()) +
                            ",\"files\":[";
        bool first = true;
        for (const auto& [relative, hash] : hashes)
        {
            if (!first)
                after += ',';
            first = false;
            after += "{\"path\":" + quote(relative) + ",\"sha256\":" + quote(hash) + '}';
        }
        after += "]}\n";
        edits.push_back({path, {}, {}, before, std::move(after), false, exists});
    }
}

// ServerApp 시작·catalog 교체에서 숫자 snapshot과 source binding을 읽는다. active 참조는 준비 완료 뒤 교체하지만 경로 멤버는 중간 변경될 수 있다.
bool LostArk::Server::CServerBalanceNumericStore::Initialize(std::shared_ptr<const CGameplayCatalog> active,
                                                             std::string& status)
{
    try
    {
        if (!active)
            throw std::runtime_error("Balance catalog is unavailable");
        auto entries = std::make_shared<std::vector<BALANCE_NUMERIC_ENTRY>>();
        GameplayDataRevision revision;
        if (!active->Build_NumericBalanceSnapshot(*entries, revision, status))
            return false;
        auto data = environment(L"LOSTARK_SERVER_DATA_ROOT");
        if (data.empty())
            data = moduleDirectory().parent_path() / L"DataFiles";
        m_BootstrapPath = fs::absolute(data / L"Gameplay" / L"Gameplay.bootstrap").lexically_normal();
        m_AuthoringDataRoot = authoringRoot(m_BootstrapPath);
        const auto bindings = m_AuthoringDataRoot / L"Balance" / L"NumericSourceBindings.json";
        m_SourceBindingsBytes.clear();
        if (!m_AuthoringDataRoot.empty() && fs::exists(bindings))
        {
            m_SourceBindingsBytes = readFile(bindings);
            validateBindings(Parser(m_SourceBindingsBytes).parse());
        }
        m_Active = std::move(active);
        m_Entries = std::move(entries);
        m_Revision = revision;
        status.clear();
        return true;
    }
    catch (const std::exception& error)
    {
        status = error.what();
        return false;
    }
}

// sequence/page를 검증하고 지역 candidate에 한 페이지를 만든다. 성공한 경우에만 호출자의 result를 교체한다.
bool LostArk::Server::CServerBalanceNumericStore::BuildSnapshot(std::uint32_t sequence, std::uint32_t page,
                                                                S2C_BALANCE_SNAPSHOT& result, std::string& status) const
{
    if (!m_Entries || !m_Revision.Is_Valid() || !sequence)
    {
        status = "Balance snapshot is unavailable";
        return false;
    }
    const auto count = (m_Entries->size() + MAX_BALANCE_PAGE_ENTRIES - 1) / MAX_BALANCE_PAGE_ENTRIES;
    if (!count || count > MAX_BALANCE_PAGES || page >= count)
    {
        status = "Balance snapshot page is outside its range";
        return false;
    }
    S2C_BALANCE_SNAPSHOT candidate;
    candidate.iRequestSequence = sequence;
    candidate.NumericRevision = m_Revision;
    candidate.iPageIndex = page;
    candidate.iPageCount = static_cast<std::uint32_t>(count);
    const auto begin = page * MAX_BALANCE_PAGE_ENTRIES;
    const auto end = (std::min)(begin + MAX_BALANCE_PAGE_ENTRIES, m_Entries->size());
    candidate.Entries.assign(m_Entries->begin() + begin, m_Entries->begin() + end);
    result = std::move(candidate);
    status.clear();
    return true;
}

// 검증 worker가 현재 revision과 field별 이전 값을 확인하고 독립 catalog를 재파싱한다. result 교체 전에는 원본을 건드리지 않는다.
bool LostArk::Server::CServerBalanceNumericStore::PreparePatch(const C2S_BALANCE_PATCH& request,
                                                               SERVER_BALANCE_PREPARED& result,
                                                               BALANCE_APPLY_RESULT& failure, std::string& status) const
{
    failure = BALANCE_APPLY_RESULT::INVALID_CHANGE;
    if (!m_Active || !m_Entries)
    {
        status = "Balance catalog is unavailable";
        return false;
    }
    if (request.BaseNumericRevision != m_Revision)
    {
        failure = BALANCE_APPLY_RESULT::STALE_REVISION;
        status = "Balance changed; reload the current snapshot";
        return false;
    }
    if (!request.iRequestSequence || request.Changes.empty() || request.Changes.size() > MAX_BALANCE_CHANGES)
    {
        status = "Invalid balance patch size or sequence";
        return false;
    }
    try
    {
        std::set<std::tuple<BALANCE_DOMAIN, std::string, std::string>> keys;
        for (const auto& change : request.Changes)
        {
            const auto found = std::find_if(m_Entries->begin(), m_Entries->end(),
                                            [&](const auto& entry)
                                            {
                                                return entry.eDomain == change.eDomain && entry.strId == change.strId &&
                                                       entry.strField == change.strField;
                                            });
            if (found == m_Entries->end() || !std::isfinite(change.fValue) || !std::isfinite(change.fBefore) ||
                found->fValue != change.fBefore || (found->isIntegral && std::floor(change.fValue) != change.fValue) ||
                !keys.emplace(change.eDomain, change.strId, change.strField).second)
            {
                status = "Unknown, duplicate, stale or non-finite balance field";
                return false;
            }
        }
        auto candidate = std::make_shared<CGameplayCatalog>();
        SERVER_BALANCE_PREPARED prepared;
        prepared.Changes = expandChanges(request.Changes, *m_Entries, m_SourceBindingsBytes);
        if (!candidate->Load_NumericBalancePatch(*m_Active, prepared.Changes, prepared.BootstrapBytes))
        {
            status = candidate->Get_Status();
            return false;
        }
        auto entries = std::make_shared<std::vector<BALANCE_NUMERIC_ENTRY>>();
        if (!candidate->Build_NumericBalanceSnapshot(*entries, prepared.NumericRevision, status))
            return false;
        prepared.Generation = std::move(candidate);
        prepared.Entries = std::move(entries);
        prepared.BaseNumericRevision = m_Revision;
        prepared.SourceBindingsBytes = m_SourceBindingsBytes;
        prepared.BaseBootstrapBytes = m_Active->Export_BootstrapBytes();
        GameplayDataRevision contentHash, nonNumericHash;
        if (!prepared.Generation->Build_NumericBalanceReceiptHashes(contentHash, nonNumericHash, status))
            return false;
        prepared.NumericReceiptBytes =
            "{\"schema\":\"lostark.numeric-balance-active\",\"parentGameplayRevision\":" +
            quote(Format_GameplayDataRevision(m_Active->Get_ActiveRevision())) +
            ",\"bootstrapContentSha256\":" + quote(Format_GameplayDataRevision(contentHash)) +
            ",\"numericRevision\":" + quote(Format_GameplayDataRevision(prepared.NumericRevision)) +
            ",\"nonNumericRowsSha256\":" + quote(Format_GameplayDataRevision(nonNumericHash)) + "}\n";
        prepared.BootstrapPath = m_BootstrapPath;
        prepared.AuthoringDataRoot = m_AuthoringDataRoot;
        result = std::move(prepared);
        status.clear();
        failure = BALANCE_APPLY_RESULT::APPLIED;
        return true;
    }
    catch (const std::exception& error)
    {
        status = error.what();
        return false;
    }
}

// 저장 worker가 latest disk를 읽어 field CAS와 source identity를 검증한다. 저작 정본·설치 bootstrap·receipt·선택적 활성 포인터를 한 보상 묶음으로 저장한다.
bool LostArk::Server::CServerBalanceNumericStore::PersistPrepared(const SERVER_BALANCE_PREPARED& prepared,
                                                                  std::string& status) const
{
    try
    {
        if (!prepared.Generation || !prepared.Entries || !prepared.NumericRevision.Is_Valid() ||
            prepared.BootstrapBytes.empty() || prepared.AuthoringDataRoot.empty())
            throw std::runtime_error(
                "Balance save requires the Server authoring Data directory; set LOSTARK_PROJECT_DATA_ROOT");
        // 정본 작성자 lock을 확보한 뒤 준비 당시 bootstrap과 최신 설치본을 다시 대조한다.
        Admission admission(prepared.AuthoringDataRoot);
        const auto diskBootstrap = readFile(prepared.BootstrapPath);
        GameplayDataRevision diskHash;
        if (!Try_Parse_GameplayDataRevision(sha256(diskBootstrap), diskHash))
            throw std::runtime_error("Cannot hash the published gameplay");
        LostArk::Server::CGameplayCatalog diskCatalog;
        if (!diskCatalog.Load_FromBootstrap(prepared.BootstrapPath, diskHash,
                                            prepared.Generation->Get_ActiveRevision()) ||
            normalize(diskCatalog.Export_BootstrapBytes()) != normalize(prepared.BaseBootstrapBytes))
            throw std::runtime_error("CONFLICT: published gameplay changed since the active generation");
        // 최신 원본의 stable field만 바꾸는 후보를 만든다. 기존 unknown property와 무관한 저작 bytes는 유지한다.
        Documents documents;
        stageSources(documents, prepared.AuthoringDataRoot, prepared.Changes);
        stagePatternDamage(documents, prepared.AuthoringDataRoot, prepared.Changes, prepared.SourceBindingsBytes);
        std::vector<DiskEdit> edits;
        for (const auto& [relative, source] : documents)
            if (!source->Replacements.empty())
                edits.push_back({source->Path, {}, {}, source->Before, source->output()});
        edits.push_back({prepared.BootstrapPath, {}, {}, diskBootstrap, prepared.BootstrapBytes});
        const auto receiptPath = prepared.BootstrapPath.parent_path() / L"NumericBalance.active.json";
        const bool receiptExists = fs::exists(receiptPath);
        const auto oldReceipt = receiptExists ? readFile(receiptPath) : std::string{};
        if (receiptExists && Parser(oldReceipt).parse().at("numericRevision").text() !=
                                 Format_GameplayDataRevision(prepared.BaseNumericRevision))
            throw std::runtime_error("CONFLICT: numeric restart receipt changed");
        edits.push_back({receiptPath, {}, {}, oldReceipt, prepared.NumericReceiptBytes, false, receiptExists});
        if (!prepared.RuntimeActivePointerPath.empty())
        {
            if (prepared.RuntimeActivePointerPath.filename() != L"active-generation.json" ||
                prepared.RuntimeActivePointerBaseBytes.empty() || prepared.RuntimeActivePointerBytes.empty() ||
                readFile(prepared.RuntimeActivePointerPath) != prepared.RuntimeActivePointerBaseBytes)
                throw std::runtime_error("CONFLICT: Debug active generation pointer changed");
            Parser(prepared.RuntimeActivePointerBytes).parse();
            edits.push_back({prepared.RuntimeActivePointerPath,
                             {},
                             {},
                             prepared.RuntimeActivePointerBaseBytes,
                             prepared.RuntimeActivePointerBytes});
        }
        // 모든 후보가 완성된 마지막 단계에서 기록을 합치고 실제 파일 교체를 시작한다.
        addSaveReceipt(edits, prepared.AuthoringDataRoot, prepared.BootstrapPath, prepared.NumericRevision,
                       prepared.NumericReceiptBytes, prepared.RuntimeActivePointerPath);
        persist(edits, documents);
        status.clear();
        return true;
    }
    catch (const std::exception& error)
    {
        status = error.what();
        return false;
    }
}

// RoomThread가 모든 방의 staged 숫자를 확정하는 시점에 같은 catalog·entry·revision 참조로 활성 조회 상태를 바꾼다.
void LostArk::Server::CServerBalanceNumericStore::CommitPrepared(const SERVER_BALANCE_PREPARED& prepared) noexcept
{
    m_Active = prepared.Generation;
    m_Entries = prepared.Entries;
    m_Revision = prepared.NumericRevision;
}
