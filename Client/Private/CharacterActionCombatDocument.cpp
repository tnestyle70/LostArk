#include "CharacterActionCombatDocument.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"
#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace
{
    using namespace Client;
    using Json = DATA_JSON_VALUE;
    using Row = CHARACTER_ACTION_COMBAT_ROW;

    const Json& Field(const Json& value, const char* name)
    {
        const auto* found = value.Find(name);
        if (!found) throw std::runtime_error(std::string("Missing combat field: ") + name);
        return *found;
    }
    std::string Text(const Json& value, const char* name)
    {
        const auto& found = Field(value, name);
        if (!found.Is_String()) throw std::runtime_error(std::string("Invalid combat text: ") + name);
        return found.Get_String();
    }
    double Number(const Json& value, const char* name)
    {
        const auto& found = Field(value, name);
        if (!found.Is_Number() || !std::isfinite(found.Get_Number()))
            throw std::runtime_error(std::string("Invalid combat number: ") + name);
        return found.Get_Number();
    }
    std::uint32_t Integer(const Json& value, const char* name)
    {
        const double n = Number(value, name);
        if (n < 0.0 || n > UINT32_MAX || std::floor(n) != n)
            throw std::runtime_error(std::string("Invalid combat integer: ") + name);
        return static_cast<std::uint32_t>(n);
    }
    const Json::ARRAY& Array(const Json& value, const char* name)
    {
        const auto& found = Field(value, name);
        if (!found.Is_Array()) throw std::runtime_error(std::string("Invalid combat array: ") + name);
        return found.Get_Array();
    }
    bool Read(const std::filesystem::path& path, std::string& text)
    {
        std::ifstream stream(path, std::ios::binary);
        if (!stream) return false;
        text.assign(std::istreambuf_iterator<char>(stream), {});
        return !stream.bad();
    }
    bool Stable(const std::string& id)
    {
        return !id.empty() && id.size() <= 128u &&
            id.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.-") == std::string::npos;
    }
    void Validate(const Row& row)
    {
        if (!Stable(row.strColliderId) || !Stable(row.strLogicId) || !Stable(row.strResultId) ||
            (row.strResultKind != "DAMAGE" && row.strResultKind != "STAGGER" && row.strResultKind != "COUNTER") ||
            row.iSkillId == 0u || row.iAreaType < 1u || row.iAreaType > 3u ||
            row.iTimeMs > 600000u || row.iRepeatCount == 0u || row.iRepeatCount > 64u ||
            row.iRepeatMs > 100000u || (row.iRepeatCount > 1u && row.iRepeatMs == 0u) ||
            (row.bContact && row.iTimeMs != 0u) || row.iPushMs > 10000u || row.iMaxTargets > 64u ||
            !std::isfinite(row.fRange) || row.fRange <= 0.0 || row.fRange > 1000.0 ||
            !std::isfinite(row.fAngleDegrees) || row.fAngleDegrees < 0.0 || row.fAngleDegrees > 360.0 || std::floor(row.fAngleDegrees) != row.fAngleDegrees ||
            !std::isfinite(row.fWidth) || row.fWidth < 0.0 || row.fWidth > 1000.0 ||
            !std::isfinite(row.fHeight) || row.fHeight < 0.0 || row.fHeight > 1000.0 ||
            !std::isfinite(row.fOffset) || std::abs(row.fOffset) > 1000.0 ||
            !std::isfinite(row.fInner) || row.fInner < 0.0 || row.fInner >= row.fRange ||
            !std::isfinite(row.fPushRange) || std::abs(row.fPushRange) > 50.0 ||
            (row.iPushMs == 0u && row.fPushRange != 0.0) ||
            (row.iAreaType == 2u && row.fWidth <= 0.0) ||
            (row.iAreaType != 2u && row.fWidth != 0.0) ||
            (row.iAreaType != 3u && row.fAngleDegrees != 0.0) ||
            (row.iAreaType == 2u && row.fInner != 0.0) ||
            (row.iAreaType == 3u && row.fAngleDegrees <= 0.0))
            throw std::runtime_error("Invalid collider schedule, shape, or Result: " + row.strColliderId);
    }
    std::vector<Row> Decode(const Json& root)
    {
        if (Text(root, "schema") != "lostark.animation-hit-shapes" || Integer(root, "formatVersion") != 4u)
            throw std::runtime_error("Character combat requires imported HitShapes v4");
        std::vector<Row> rows;
        std::unordered_set<std::string> ids;
        for (const auto& skill : Array(root, "skills"))
        {
            const auto skillId = Integer(skill, "skillId");
            auto append = [&](const Json& owner, const std::uint32_t stage, const std::uint32_t projectile,
                const std::uint32_t projectileStart)
            {
                std::uint32_t repeatTotal = 0u;
                const auto& hits = Array(owner, "hits");
                if (hits.size() > (projectile == UINT32_MAX ? 192u : 48u))
                    throw std::runtime_error("Too many character colliders");
                for (const auto& hit : hits)
                {
                    Row row;
                    row.iSkillId = skillId; row.iStageIndex = stage;
                    row.iProjectileIndex = projectile; row.iProjectileStartMs = projectileStart;
                    row.strColliderId = Text(hit, "colliderId");
                    const auto& logic = Field(hit, "logic"); const auto& result = Field(hit, "result");
                    row.strLogicId = Text(logic, "logicId"); row.strResultId = Text(result, "resultId");
                    row.strResultKind = Text(result, "resultKind");
                    if (Text(logic, "logicType") != "DURATION" || Text(logic, "judgementKind") != "AREA_OVERLAP" ||
                        Text(result, "logicType") != "RESULT" || Text(logic, "colliderId") != row.strColliderId ||
                        Text(logic, "resultId") != row.strResultId || !ids.insert(row.strColliderId).second ||
                        !ids.insert(row.strLogicId).second || !ids.insert(row.strResultId).second)
                        throw std::runtime_error("Collider / Logic / Result identity join failed: " + row.strColliderId);
                    const bool isProjectile = projectile != UINT32_MAX;
                    row.iTimeMs = Integer(hit, isProjectile ? "atMs" : "timeMs");
                    row.iRepeatCount = Integer(hit, isProjectile ? "count" : "repeatCount");
                    row.iRepeatMs = Integer(hit, isProjectile ? "everyMs" : "repeatMs");
                    if (isProjectile)
                    {
                        const auto trigger = Text(hit, "trigger");
                        if (trigger != "CONTACT" && trigger != "TIMED") throw std::runtime_error("Unknown projectile trigger");
                        row.bContact = trigger == "CONTACT";
                    }
                    row.iAreaType = Integer(hit, "areaType"); row.iMaxTargets = Integer(hit, "maxTargets");
                    row.iPushMs = Integer(hit, "pushMs");
                    row.fRange = Number(hit, "range"); row.fAngleDegrees = Number(hit, "angle");
                    row.fWidth = Number(hit, "width"); row.fHeight = Number(hit, "height");
                    row.fOffset = Number(hit, "offset"); row.fInner = Number(hit, "inner");
                    row.fPushRange = Number(hit, "pushRange");
                    Validate(row);
                    if (!row.bContact) repeatTotal += row.iRepeatCount;
                    rows.push_back(std::move(row));
                }
                if (repeatTotal > 192u) throw std::runtime_error("Collider repeats exceed the bounded runtime mask");
            };
            auto stage = [&](const Json& owner, std::uint32_t stageIndex)
            {
                append(owner, stageIndex, UINT32_MAX, 0u);
                if (const auto* projectiles = owner.Find("projectiles"))
                {
                    if (!projectiles->Is_Array()) throw std::runtime_error("Invalid projectile array");
                    std::uint32_t index = 0u;
                    for (const auto& projectile : projectiles->Get_Array())
                        append(projectile, stageIndex, index++, Integer(projectile, "timeMs"));
                }
            };
            if (const auto* stages = skill.Find("stages"))
            {
                if (!stages->Is_Array()) throw std::runtime_error("Invalid combat stages");
                for (const auto& item : stages->Get_Array()) stage(item, Integer(item, "stageIndex"));
            }
            else stage(skill, 0u);
        }
        return rows;
    }
    Json ReplaceRows(const Json& value, const std::unordered_map<std::string, const Row*>& rows)
    {
        if (value.Is_Array())
        {
            Json::ARRAY values;
            for (const auto& item : value.Get_Array()) values.push_back(ReplaceRows(item, rows));
            // A collider's stable identity survives timeline moves and source order changes.
            if (!values.empty() && values.front().Find("colliderId") && values.front().Find("result"))
                std::stable_sort(values.begin(), values.end(), [](const Json& a, const Json& b)
                {
                    const bool projectile = a.Find("trigger") != nullptr;
                    const bool contactA = projectile && Text(a, "trigger") == "CONTACT";
                    const bool contactB = projectile && Text(b, "trigger") == "CONTACT";
                    if (contactA != contactB) return contactA;
                    return Number(a, projectile ? "atMs" : "timeMs") < Number(b, projectile ? "atMs" : "timeMs");
                });
            return Json::Array(std::move(values));
        }
        if (!value.Is_Object()) return value;
        auto fields = value.Get_Object();
        if (value.Find("colliderId") && value.Find("result"))
        {
            const auto& row = *rows.at(Text(value, "colliderId"));
            const bool projectile = row.iProjectileIndex != UINT32_MAX;
            auto number = [&](const char* name, double n) { fields.insert_or_assign(name, Json::Number(n)); };
            number(projectile ? "atMs" : "timeMs", row.iTimeMs);
            number(projectile ? "count" : "repeatCount", row.iRepeatCount);
            number(projectile ? "everyMs" : "repeatMs", row.iRepeatMs);
            number("areaType", row.iAreaType); number("range", row.fRange); number("angle", row.fAngleDegrees);
            number("width", row.fWidth); number("height", row.fHeight); number("offset", row.fOffset);
            number("inner", row.fInner); number("maxTargets", row.iMaxTargets);
            number("pushMs", row.iPushMs); number("pushRange", row.fPushRange);
        }
        else for (auto& [name, item] : fields) item = ReplaceRows(item, rows);
        return Json::Object(std::move(fields), value.Get_ObjectInsertionOrder());
    }
    void Serialize(const Json& value, std::ostringstream& output, int indent = 0)
    {
        if (value.Is_Object())
        {
            output << "{"; bool first = true;
            for (const auto& [name, item] : value.Get_Object())
            {
                output << (first ? "\n" : ",\n") << std::string(indent + 2, ' ') << '"' << CDataJson::Escape(name) << "\": ";
                Serialize(item, output, indent + 2); first = false;
            }
            if (!first) output << '\n' << std::string(indent, ' ');
            output << "}";
        }
        else if (value.Is_Array())
        {
            output << "["; bool first = true;
            for (const auto& item : value.Get_Array())
            {
                output << (first ? "\n" : ",\n") << std::string(indent + 2, ' ');
                Serialize(item, output, indent + 2); first = false;
            }
            if (!first) output << '\n' << std::string(indent, ' ');
            output << "]";
        }
        else if (value.Is_String()) output << '"' << CDataJson::Escape(value.Get_String()) << '"';
        else if (value.Is_Number()) output << std::setprecision(17) << value.Get_Number();
        else if (value.Is_Boolean()) output << (value.Get_Boolean() ? "true" : "false");
        else output << "null";
    }
}

bool Client::CCharacterActionCombatDocument::Reload(std::string_view asset, std::string& status)
{
    try
    {
        if (!Stable(std::string(asset))) throw std::runtime_error("Invalid animation asset ID");
        const auto path = CProjectDataRoot::Resolve(std::filesystem::path("Animation/HitShapes") / (std::string(asset) + ".hitshapes.json"));
        std::string bytes; Json root;
        if (!Read(path, bytes) || !CDataJson::Parse(bytes, root, status)) throw std::runtime_error("Cannot read combat document: " + status);
        if (Text(root, "animationAssetId") != asset) throw std::runtime_error("Combat asset owner mismatch");
        auto rows = Decode(root);
        m_Path = path; m_Baseline = std::move(bytes); m_Rows = std::move(rows);
        status = "Character Collider / Logic / Result loaded";
        return true;
    }
    catch (const std::exception& error) { status = error.what(); return false; }
}

bool Client::CCharacterActionCombatDocument::Save_Atomic(const std::vector<Row>& rows, std::string& status)
{
    std::filesystem::path temporary;
    try
    {
        std::string bytes;
        if (m_Path.empty() || !Read(m_Path, bytes) || bytes != m_Baseline)
            throw std::runtime_error("Combat file changed outside this draft; preserve the draft and reload explicitly");
        if (rows.size() != m_Rows.size()) throw std::runtime_error("Combat rows must retain their imported identities");
        std::unordered_map<std::string, const Row*> changes;
        for (const auto& row : rows)
        {
            Validate(row);
            if (!changes.emplace(row.strColliderId, &row).second) throw std::runtime_error("Duplicate collider ID");
        }
        for (const auto& baseline : m_Rows)
        {
            Row identity = *changes.at(baseline.strColliderId);
            identity.iTimeMs = baseline.iTimeMs; identity.iRepeatCount = baseline.iRepeatCount; identity.iRepeatMs = baseline.iRepeatMs;
            identity.iAreaType = baseline.iAreaType; identity.iMaxTargets = baseline.iMaxTargets; identity.iPushMs = baseline.iPushMs;
            identity.fRange = baseline.fRange; identity.fAngleDegrees = baseline.fAngleDegrees; identity.fWidth = baseline.fWidth;
            identity.fHeight = baseline.fHeight; identity.fOffset = baseline.fOffset; identity.fInner = baseline.fInner; identity.fPushRange = baseline.fPushRange;
            if (identity != baseline) throw std::runtime_error("Collider / Logic / Result owner cannot be reassigned");
        }
        Json root;
        if (!CDataJson::Parse(bytes, root, status)) throw std::runtime_error(status);
        auto candidate = ReplaceRows(root, changes);
        auto admitted = Decode(candidate);
        // The existing balance owner supplies action/stage duration and combo cutoffs.
        std::string skillBytes; Json skills;
        if (!Read(CProjectDataRoot::Resolve("Balance/PlayerSkills.json"), skillBytes) || !CDataJson::Parse(skillBytes, skills, status))
            throw std::runtime_error("Cannot validate combat schedules against PlayerSkills");
        for (const auto& row : admitted)
        {
            const Json* owner = nullptr;
            for (const auto& skill : Array(skills, "skills")) if (Integer(skill, "skillId") == row.iSkillId) { owner = &skill; break; }
            if (!owner) throw std::runtime_error("Unknown combat skill ID");
            const auto& stages = Array(*owner, "comboStages");
            const Json* timing = owner;
            if (!stages.empty())
            {
                if (row.iStageIndex >= stages.size()) throw std::runtime_error("Unknown combat stage");
                timing = &stages[row.iStageIndex];
            }
            const auto limit = Integer(*timing, "actionDurationMs");
            const auto finalTime = static_cast<std::uint64_t>(row.iTimeMs) + static_cast<std::uint64_t>(row.iRepeatCount - 1u) * row.iRepeatMs;
            if (row.iProjectileIndex == UINT32_MAX && (finalTime > limit ||
                (Text(*owner, "skillKind") == "COMBO" && finalTime > Integer(*timing, "comboAdvanceMs"))))
                throw std::runtime_error("Collider exceeds its Server action/stage duration: " + row.strColliderId);
        }
        std::ostringstream encoded; Serialize(candidate, encoded); encoded << '\n';
        const auto output = encoded.str();
        temporary = m_Path; temporary += L".action-combat." + std::to_wstring(GetCurrentProcessId()) + L".tmp";
        HANDLE file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) throw std::runtime_error("Cannot create combat save temporary");
        DWORD written = 0u;
        const bool writtenOk = WriteFile(file, output.data(), static_cast<DWORD>(output.size()), &written, nullptr) &&
            written == output.size() && FlushFileBuffers(file);
        CloseHandle(file);
        if (!writtenOk) throw std::runtime_error("Cannot flush combat save temporary");
        std::string reopened; Json verified;
        if (!Read(temporary, reopened) || reopened != output || !CDataJson::Parse(reopened, verified, status))
            throw std::runtime_error("Combat save verification failed");
        (void)Decode(verified);
        if (!Read(m_Path, bytes) || bytes != m_Baseline) throw std::runtime_error("Combat baseline changed during Save");
        if (!MoveFileExW(temporary.c_str(), m_Path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            throw std::runtime_error("Cannot atomically replace combat document");
        m_Baseline = output; m_Rows = std::move(admitted);
        status = "Collider / Logic / Result saved; publish Gameplay and restart Server to apply combat";
        return true;
    }
    catch (const std::exception& error)
    {
        if (!temporary.empty()) { std::error_code ignored; std::filesystem::remove(temporary, ignored); }
        status = error.what(); return false;
    }
}
