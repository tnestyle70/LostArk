#include "ClassSelectionPresentation.h"
#include "ProjectDataRoot.h"
#include "MapAssetCatalog.h"
#include "GameInstance.h"
#include "RuntimeAssetRoot.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <locale>
#include <set>
#include <sstream>
#include <variant>

namespace Client
{
namespace
{
using Json = DATA_JSON_VALUE;
using Path = std::vector<std::variant<std::string, size_t>>;

std::string Text(const Json& value, const char* key)
{ const auto* child = value.Find(key); return child && child->Is_String() ? child->Get_String() : std::string{}; }

std::string Serialize(const Json& value)
{
    std::ostringstream stream;
    stream.imbue(std::locale::classic()); stream << std::setprecision(17);
    const auto write = [&](const auto& self, const Json& item) -> void {
        switch (item.Get_Type())
        {
        case DATA_JSON_TYPE::NULL_VALUE: stream << "null"; break;
        case DATA_JSON_TYPE::BOOLEAN: stream << (item.Get_Boolean() ? "true" : "false"); break;
        case DATA_JSON_TYPE::NUMBER: stream << item.Get_Number(); break;
        case DATA_JSON_TYPE::STRING: stream << '"' << CDataJson::Escape(item.Get_String()) << '"'; break;
        case DATA_JSON_TYPE::ARRAY:
            stream << '[';
            for (size_t i = 0; i < item.Get_Array().size(); ++i)
            { if (i) stream << ','; self(self, item.Get_Array()[i]); }
            stream << ']'; break;
        case DATA_JSON_TYPE::OBJECT:
            stream << '{';
            { bool first = true;
              for (const auto& [key, child] : item.Get_Object())
              { if (!first) stream << ','; first = false; stream << '"' << CDataJson::Escape(key) << "\":"; self(self, child); } }
            stream << '}'; break;
        }
    };
    write(write, value); return stream.str();
}

bool Equal(const Json& a, const Json& b)
{
    if (a.Get_Type() != b.Get_Type()) return false;
    switch (a.Get_Type())
    {
    case DATA_JSON_TYPE::NULL_VALUE: return true;
    case DATA_JSON_TYPE::BOOLEAN: return a.Get_Boolean() == b.Get_Boolean();
    case DATA_JSON_TYPE::NUMBER: return a.Get_Number() == b.Get_Number();
    case DATA_JSON_TYPE::STRING: return a.Get_String() == b.Get_String();
    case DATA_JSON_TYPE::ARRAY:
        if (a.Get_Array().size() != b.Get_Array().size()) return false;
        for (size_t i = 0; i < a.Get_Array().size(); ++i) if (!Equal(a.Get_Array()[i], b.Get_Array()[i])) return false;
        return true;
    case DATA_JSON_TYPE::OBJECT:
        if (a.Get_Object().size() != b.Get_Object().size()) return false;
        for (const auto& [key, child] : a.Get_Object())
        { const auto* other = b.Find(key); if (!other || !Equal(child, *other)) return false; }
        return true;
    }
    return false;
}

Json Set(const Json& object, const std::string& key, Json value)
{
    auto fields = object.Get_Object(); fields[key] = std::move(value);
    return Json::Object(std::move(fields), object.Get_ObjectInsertionOrder());
}

// Split the original curve before moving its visible interval. Inserting a
// Hermite value and its derivative preserves both halves of the original span.
Json EffectCurveSample(const std::vector<Json>& keys, const double time)
{
    const auto at = [](const Json& key) { return key.Find("timeMs")->Get_Number(); };
    const auto right = std::lower_bound(keys.begin(), keys.end(), time,
        [&](const Json& key, double value) { return at(key) < value; });
    if (right != keys.end() && at(*right) == time) return *right;
    const auto& a = right == keys.begin() ? keys.front() : *(right - 1);
    const auto& b = right == keys.end() ? keys.back() : *right;
    const double seconds = (at(b) - at(a)) * .001;
    const double u = seconds > 0. ? (time - at(a)) * .001 / seconds : 0.;
    Json result = Json::Object({{"timeMs", Json::Number(time)}});
    if (a.Find("sourceMs"))
        return Set(result, "sourceMs", Json::Number(a.Find("sourceMs")->Get_Number() +
            u * (b.Find("sourceMs")->Get_Number() - a.Find("sourceMs")->Get_Number())));
    if (a.Find("position"))
    {
        for (const auto* field : {"position", "scale"})
        {
            auto values = a.Find(field)->Get_Array(); const auto& other = b.Find(field)->Get_Array();
            for (size_t i = 0; i < values.size(); ++i)
                values[i] = Json::Number(values[i].Get_Number() + u * (other[i].Get_Number() - values[i].Get_Number()));
            result = Set(result, field, Json::Array(std::move(values)));
        }
        const auto quaternion = [](const Json& key) {
            const auto& values = key.Find("rotationQuaternion")->Get_Array();
            return XMQuaternionNormalize(XMVectorSet(static_cast<float>(values[0].Get_Number()),
                static_cast<float>(values[1].Get_Number()), static_cast<float>(values[2].Get_Number()),
                static_cast<float>(values[3].Get_Number())));
        };
        XMFLOAT4 rotation;
        XMStoreFloat4(&rotation, XMQuaternionNormalize(XMQuaternionSlerp(quaternion(a), quaternion(b), static_cast<float>(u))));
        return Set(result, "rotationQuaternion", Json::Array({Json::Number(rotation.x), Json::Number(rotation.y),
            Json::Number(rotation.z), Json::Number(rotation.w)}));
    }
    const std::string mode = seconds > 0. ? Text(a, "interpolation") : "CONSTANT";
    auto values = a.Find("value")->Get_Array(), tangents = values;
    for (size_t i = 0; i < values.size(); ++i)
    {
        const double p = values[i].Get_Number(), q = b.Find("value")->Get_Array()[i].Get_Number();
        double value = p, derivative = 0.;
        if (mode == "LINEAR") { value += u * (q - p); derivative = (q - p) / seconds; }
        else if (mode == "CUBIC")
        {
            const double leave = a.Find("leaveTangent")->Get_Array()[i].Get_Number();
            const double arrive = b.Find("arriveTangent")->Get_Array()[i].Get_Number();
            const double u2 = u * u, u3 = u2 * u;
            value = (2. * u3 - 3. * u2 + 1.) * p + (u3 - 2. * u2 + u) * seconds * leave +
                (-2. * u3 + 3. * u2) * q + (u3 - u2) * seconds * arrive;
            derivative = ((6. * u2 - 6. * u) * p + (-6. * u2 + 6. * u) * q) / seconds +
                (3. * u2 - 4. * u + 1.) * leave + (3. * u2 - 2. * u) * arrive;
        }
        values[i] = Json::Number(value); tangents[i] = Json::Number(derivative);
    }
    result = Set(Set(result, "value", Json::Array(std::move(values))), "interpolation", Json::String(mode));
    result = Set(result, "arriveTangent", Json::Array(tangents));
    return Set(result, "leaveTangent", Json::Array(std::move(tangents)));
}

bool RetimeEffectCurve(const Json& curve, const double oldStart, const double oldEnd,
    const double start, const double end, const double duration, Json& result)
{
    const auto& original = curve.Get_Array();
    auto keys = original;
    const auto at = [](const Json& key) { return key.Find("timeMs")->Get_Number(); };
    const bool parameter = original.front().Find("value") != nullptr;
    for (const double time : {0., oldStart, oldEnd, duration})
    {
        const auto insertion = std::lower_bound(keys.begin(), keys.end(), time,
            [&](const Json& key, double value) { return at(key) < value; });
        if (insertion == keys.end() || at(*insertion) != time)
            keys.insert(insertion, EffectCurveSample(original, time));
    }
    // The runtime holds a parameter's final value outside its authored keys.
    if (parameter && at(original.back()) < duration)
        for (auto& key : keys) if (at(key) == at(original.back())) key = Set(key, "interpolation", Json::String("CONSTANT"));
    keys.erase(std::remove_if(keys.begin(), keys.end(), [&](const Json& key) {
        return (start == 0. && at(key) < oldStart) || (end == duration && at(key) > oldEnd);
    }), keys.end());
    const auto map = [&](double time) {
        if (time == oldStart) return start;
        if (time == oldEnd) return end;
        if (time == 0.) return 0.;
        if (time == duration) return duration;
        if (time < oldStart) return time * start / oldStart;
        if (time <= oldEnd) return start + (time - oldStart) * (end - start) / (oldEnd - oldStart);
        return end + (time - oldEnd) * (duration - end) / (duration - oldEnd);
    };
    const auto scaleTangent = [](Json& key, const char* field, const double scale) {
        auto values = key.Find(field)->Get_Array();
        for (auto& value : values) value = Json::Number(value.Get_Number() * scale);
        key = Set(key, field, Json::Array(std::move(values)));
    };
    auto moved = keys;
    for (size_t i = 0; i < keys.size(); ++i)
    {
        const double time = map(at(keys[i]));
        if (!std::isfinite(time) || time < 0. || time > duration || (i && time <= at(moved[i - 1]))) return false;
        moved[i] = Set(moved[i], "timeMs", Json::Number(time));
        if (parameter)
        {
            scaleTangent(moved[i], "arriveTangent", i ? (at(keys[i]) - at(keys[i - 1])) / (time - map(at(keys[i - 1]))) : 0.);
            scaleTangent(moved[i], "leaveTangent", i + 1 < keys.size() ?
                (at(keys[i + 1]) - at(keys[i])) / (map(at(keys[i + 1])) - time) : 0.);
        }
    }
    const auto plateau = [&](const Json& key, double time) {
        // A new endpoint is a sample, not a second copy of a stable key ID.
        auto value = EffectCurveSample(std::vector<Json>{key}, time);
        return parameter ? Set(value, "interpolation", Json::String("CONSTANT")) : value;
    };
    if (at(moved.front()) > 0.) moved.insert(moved.begin(), plateau(moved.front(), 0.));
    if (at(moved.back()) < duration)
    {
        auto final = plateau(moved.back(), duration);
        if (parameter) moved.back() = Set(moved.back(), "interpolation", Json::String("CONSTANT"));
        moved.push_back(std::move(final));
    }
    result = Json::Array(std::move(moved)); return true;
}

const Json* At(const Json& root, const Path& path)
{
    const Json* result = &root;
    for (const auto& step : path)
    {
        if (const auto* name = std::get_if<std::string>(&step)) result = result->Find(*name);
        else
        {
            const size_t index = std::get<size_t>(step);
            result = result->Is_Array() && index < result->Get_Array().size() ? &result->Get_Array()[index] : nullptr;
        }
        if (!result) return nullptr;
    }
    return result;
}

Json Replace(const Json& value, const Path& path, size_t depth, const Json& replacement)
{
    if (depth == path.size()) return replacement;
    if (const auto* name = std::get_if<std::string>(&path[depth]))
        return Set(value, *name, depth + 1u == path.size() ? replacement : Replace(*value.Find(*name), path, depth + 1u, replacement));
    auto items = value.Get_Array(); const size_t index = std::get<size_t>(path[depth]);
    items[index] = Replace(items[index], path, depth + 1u, replacement);
    return Json::Array(std::move(items));
}

size_t Find(const Json* array, const char* key, const std::string& value)
{
    if (array && array->Is_Array())
        for (size_t i = 0; i < array->Get_Array().size(); ++i)
            if (Text(array->Get_Array()[i], key) == value) return i;
    return SIZE_MAX;
}

bool Resolve(const Json& manifest, const Json& world, const CWorldSequenceDocument& document,
    const CLASS_MOVIE_AUTHORING_BOX& box, bool& worldSource, Path& path)
{
    const size_t scene = Find(manifest.Find("scenes"), "classId", box.classId);
    if (scene == SIZE_MAX) return false;
    Path phasePath{std::string("scenes"), scene, std::string(box.loop ? "loop" : "intro")};
    const auto* phase = At(manifest, phasePath);
    if (!phase) return false;
    worldSource = false;
    const char* collection = box.kind == "Camera" ? "cameras" : box.kind == "Effect" ? "effects" : box.kind == "Light" ? "lights" : nullptr;
    const char* identity = box.kind == "Camera" ? "cameraId" : box.kind == "Effect" ? "effectId" : "lightId";
    if (collection)
    {
        const size_t index = Find(phase->Find(collection), identity, box.boxId);
        if (index == SIZE_MAX) return false;
        path = phasePath; path.emplace_back(std::string(collection)); path.emplace_back(index); return true;
    }
    if (box.kind == "Time Control")
    { path = phasePath; path.emplace_back(std::string("clockKeys")); return box.boxId == "movie-clock"; }
    if (box.kind == "Material")
    {
        const auto* tracks = phase->Find("materialTracks");
        if (!tracks || !tracks->Is_Array()) return false;
        for (size_t i = 0; i < tracks->Get_Array().size(); ++i)
        {
            const auto& track = tracks->Get_Array()[i]; const auto* curves = track.Find("curves");
            if (!curves || !curves->Is_Array()) continue;
            for (const auto& curve : curves->Get_Array())
                if (Text(track, "instanceId") + "." + Text(track, "slotId") + "." + Text(curve, "parameter") == box.boxId)
                { path = phasePath; path.emplace_back(std::string("materialTracks")); path.emplace_back(i); return true; }
        }
        return false;
    }
    const auto* instances = phase->Find("instanceIds");
    if (!instances || !instances->Is_Array()) return false;
    for (const auto& id : instances->Get_Array())
    {
        if (!id.Is_String()) continue;
        const auto* instance = document.Find_Instance(id.Get_String());
        const auto* sequence = instance ? document.Find_Template(instance->templateId) : nullptr;
        if (!sequence) continue;
        const size_t templateIndex = Find(world.Find("templates"), "sequenceId", instance->templateId);
        if (templateIndex == SIZE_MAX) continue;
        Path templatePath{std::string("templates"), templateIndex};
        const auto* raw = At(world, templatePath);
        auto accept = [&](const char* field, size_t index) {
            path = templatePath; path.emplace_back(std::string(field)); path.emplace_back(index); worldSource = true;
            return At(world, path) != nullptr;
        };
        if (box.kind == "World Model")
            for (const auto& track : sequence->tracks)
                if (id.Get_String() + ".transform." + track.slotId == box.boxId)
                    return accept("tracks", Find(raw->Find("tracks"), "slotId", track.slotId));
        if (box.kind == "Animation")
            for (size_t i = 0; i < sequence->animationTracks.size(); ++i)
                if (id.Get_String() + ".animation." + std::to_string(i) == box.boxId) return accept("animationTracks", i);
        if (box.kind == "Sound")
            for (const auto& track : sequence->soundTracks)
                if (id.Get_String() + "." + track.soundTrackId == box.boxId)
                    return accept("soundTracks", Find(raw->Find("soundTracks"), "soundTrackId", track.soundTrackId));
    }
    return false;
}

// Omitted clockKeys is the runtime identity clock. Expose a draft of that clock
// without changing the source until the user explicitly applies its row.
Json ReadRow(const Json& root, const Path& path, const std::string& kind)
{
    if (const auto* value = At(root, path)) return *value;
    if (kind == "Time Control" && !path.empty())
    {
        Path parent = path; parent.pop_back();
        const auto* phase = At(root, parent); const auto* duration = phase ? phase->Find("durationMs") : nullptr;
        if (duration && duration->Is_Number())
        {
            const auto key = [](double time) { return Json::Object({{"timeMs", Json::Number(time)}, {"sourceMs", Json::Number(time)}}); };
            return Json::Array({key(0.), key(duration->Get_Number())});
        }
    }
    return Json::Null();
}

std::string Identity(const Json& value)
{
    if (!value.Is_Object()) return {};
    std::string result;
    for (const auto* name : {"classId", "sequenceId", "objectId", "cameraId", "effectId", "lightId", "soundTrackId", "keyId", "instanceId", "slotId", "materialName", "parameter"})
    {
        const auto text = Text(value, name);
        if (!text.empty()) result += std::string(name) + "=" + text + "\n";
    }
    return result;
}

// Merge by stable authored identities. Key arrays without identities are one
// field: concurrent edits to those arrays conflict instead of guessing indices.
bool Merge(const Json& baseline, const Json& draft, const Json& latest, Json& out, std::string& status, const std::string& path)
{
    if (Equal(baseline, draft)) { out = latest; return true; }
    if (Equal(baseline, latest) || Equal(draft, latest)) { out = draft; return true; }
    if (baseline.Is_Object() && draft.Is_Object() && latest.Is_Object())
    {
        auto fields = latest.Get_Object();
        std::set<std::string> names;
        for (const auto& [name, unused] : baseline.Get_Object()) names.insert(name);
        for (const auto& [name, unused] : draft.Get_Object()) names.insert(name);
        for (const auto& name : names)
        {
            const auto* a = baseline.Find(name); const auto* b = draft.Find(name); const auto* c = latest.Find(name);
            if (a && b && Equal(*a, *b)) continue;
            if (!a)
            {
                if (!c || (b && Equal(*b, *c))) { if (b) fields[name] = *b; continue; }
                status = "Movie source field was added concurrently: " + path + "/" + name; return false;
            }
            if (!b)
            {
                if (!c || Equal(*a, *c)) { fields.erase(name); continue; }
                status = "Movie source field changed before deletion: " + path + "/" + name; return false;
            }
            if (!c) { status = "Edited movie source field was removed concurrently: " + path + "/" + name; return false; }
            Json merged;
            if (!Merge(*a, *b, *c, merged, status, path + "/" + name)) return false;
            fields[name] = std::move(merged);
        }
        out = Json::Object(std::move(fields), latest.Get_ObjectInsertionOrder()); return true;
    }
    if (baseline.Is_Array() && draft.Is_Array() && latest.Is_Array())
    {
        std::map<std::string, const Json*> a, b;
        std::set<std::string> live;
        bool stable = true;
        for (const auto& item : baseline.Get_Array()) { const auto id = Identity(item); stable &= !id.empty() && a.emplace(id, &item).second; }
        for (const auto& item : draft.Get_Array()) { const auto id = Identity(item); stable &= !id.empty() && b.emplace(id, &item).second; }
        for (const auto& item : latest.Get_Array()) { const auto id = Identity(item); stable &= !id.empty() && live.insert(id).second; }
        if (stable && a.size() == b.size() && std::all_of(a.begin(), a.end(), [&](const auto& item) { return b.contains(item.first); }))
        {
            auto items = latest.Get_Array();
            for (auto& item : items)
            {
                const auto id = Identity(item);
                if (a.contains(id) && !Merge(*a[id], *b[id], item, item, status, path + "/" + id)) return false;
            }
            for (const auto& [id, old] : a)
                if (!live.contains(id) && !Equal(*old, *b[id])) { status = "Edited movie row was removed concurrently: " + path + "/" + id; return false; }
            out = Json::Array(std::move(items)); return true;
        }
    }
    status = "Movie source field changed concurrently: " + path; return false;
}

bool Read(const std::filesystem::path& path, std::string& bytes, Json& json, std::string& status)
{
    std::error_code error; const auto size = std::filesystem::file_size(path, error);
    if (error || !size || size > 16u * 1024u * 1024u) { status = "Movie authoring source is unavailable or exceeds 16 MiB: " + path.generic_string(); return false; }
    std::ifstream file(path, std::ios::binary);
    bytes.assign(std::istreambuf_iterator<char>(file), {});
    if (!file || bytes.size() != size) { status = "Movie authoring source changed while reading."; return false; }
    return CDataJson::Parse(bytes, json, status);
}

std::array<std::filesystem::path, 2> SourcePaths(const std::string& area)
{
    return {CProjectDataRoot::Resolve(L"Camera/ClassSelection.cinematics.json"),
        CProjectDataRoot::Resolve(std::filesystem::path("Maps/Authoring") / area / (area + ".worldsequences.json"))};
}

bool SourcesMatch(const std::array<std::filesystem::path, 2>& paths,
    const Json& manifest, const Json& world, std::string& status)
{
    const Json* expected[]{&manifest, &world};
    for (size_t i = 0; i < 2; ++i)
    {
        std::string bytes; Json current;
        if (!Read(paths[i], bytes, current, status)) return false;
        if (!Equal(current, *expected[i]))
        { status = "Movie sources changed on disk. Save to reconcile the source before publishing; the draft is preserved."; return false; }
    }
    return true;
}

bool MatchesPublishedWorld(const CWorldSequenceDocument& source, const CWorldSequencePlayer::TARGET_SET& targets)
{
    const auto path = CMapAssetCatalog::Get_MapDataRoot() / (source.Get_AreaId() + ".worldsequences.json");
    if (!std::filesystem::is_regular_file(path)) return false;
    WORLD_SEQUENCE_PLACEMENT_MAP placements; WORLD_SEQUENCE_DEPLOY_MAP deploy;
    CWorldSequencePlayer::Collect_ValidationTargets(targets, placements, deploy);
    CWorldSequenceDocument published; std::string status;
    return published.Load(path, source.Get_AreaId(), placements, deploy, status) && source.Is_Equivalent(published);
}

struct WriterLock final
{
    HANDLE handle = INVALID_HANDLE_VALUE;
    ~WriterLock() { if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle); }
    bool Open(const std::filesystem::path& path)
    { handle = CreateFileW((path.wstring() + L".writer.lock").c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr); return handle != INVALID_HANDLE_VALUE; }
};

bool Write(const std::filesystem::path& path, const std::string& bytes)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file.write(bytes.data(), static_cast<std::streamsize>(bytes.size())); file.flush();
    return static_cast<bool>(file);
}
}

bool CClassSelectionPresentation::Validate_Authoring(const Json& manifest, const Json& world,
    std::vector<SCENE>& scenes, CWorldSequenceDocument& document, std::string& status) const
{
    const auto& area = m_Resources.Get_Document().Get_AreaId();
    if (area.empty() || !m_Targets.Is_Complete()) { status = "Enter Character Select before editing the movie."; return false; }
    WORLD_SEQUENCE_PLACEMENT_MAP placements; WORLD_SEQUENCE_DEPLOY_MAP deploy;
    CWorldSequencePlayer::Collect_ValidationTargets(m_Targets, placements, deploy);
    if (!Parse(manifest, area, scenes, status) || !document.Load_Text(Serialize(world), area, placements, deploy, status) ||
        !Validate_WorldExclusions(scenes, document, status)) return false;
    for (const auto& scene : scenes)
        for (const auto* phase : {&scene.intro, &scene.loop})
            for (const auto& id : phase->instanceIds)
            {
                const auto* instance = document.Find_Instance(id);
                const auto* sequence = instance ? document.Find_Template(instance->templateId) : nullptr;
                if (!instance || !instance->enabled || !sequence || instance->startDelayMs || instance->playbackSpeed != 1.f ||
                    sequence->durationMs != phase->durationMs || instance->motionEnd == WORLD_SEQUENCE_MOTION_END::NEXT)
                { status = "Movie phase and World sequence disagree: " + id; return false; }
            }
    return true;
}

bool CClassSelectionPresentation::Commit_Authoring(std::vector<SCENE> scenes,
    const CWorldSequenceDocument& document, std::string& status)
{
    // Admission failure leaves the current movie and draft untouched. This edits
    // clocks/keys of existing resources; playback keeps the same World owner.
    if (!m_Resources.Replace_DocumentKeepingModels(document, m_Targets, status)) return false;
    Stop(); m_Scenes = std::move(scenes); m_Timelines.clear();
    if (m_Authoring) m_Authoring->appliedToPreview = true;
    m_Status = "Movie draft applied. Play or seek to preview the edited movie.";
    return true;
}

bool CClassSelectionPresentation::Prepare_Authoring(const std::vector<SCENE>& scenes,
    const CWorldSequenceDocument& document, std::string& status)
{
    CWorldSequencePlayer staged;
    if (!staged.Set_Document(document, m_Targets, status)) return false;
    for (const auto& scene : scenes)
        for (const auto* phase : {&scene.intro, &scene.loop})
        {
            for (const auto& id : phase->instanceIds)
                if (!staged.Prepare_InstanceResources(id, m_Targets))
                { status = "Movie model/clip admission failed: " + staged.Get_Status(); return false; }
            for (const auto& effect : phase->effects)
            {
                float seconds = 0.f;
                if (!CEffectPresentationService::Try_Get_PreparedProductDurationSeconds(effect.assetId, seconds))
                { status = "Movie Effect is not prepared. Open and Save the Effect first: " + effect.assetId; return false; }
            }
        }
    return true;
}

bool CClassSelectionPresentation::Begin_Authoring(std::string& status)
{
    if (m_Authoring) return true;
    const auto paths = SourcePaths(m_Resources.Get_Document().Get_AreaId());
    auto staged = std::make_unique<AUTHORING_STATE>(); std::string bytes;
    if (!Read(paths[0], bytes, staged->manifest, status) || !Read(paths[1], bytes, staged->world, status) ||
        !Validate_Authoring(staged->manifest, staged->world, staged->scenes, staged->document, status)) return false;
    staged->manifestBase = staged->manifest; staged->worldBase = staged->world;
    staged->needsPublish = !MatchesPublishedWorld(staged->document, m_Targets);
    staged->status = "Movie source draft ready. Apply edits to preview them; opening a row preserves playback.";
    m_Authoring = std::move(staged); m_Timelines.clear(); return true;
}

bool CClassSelectionPresentation::Reload_Authoring(std::string& status)
{
    if (Is_AuthoringPublishPending()) { status = "Wait for the movie runtime publish to finish before reloading."; return false; }
    const auto paths = SourcePaths(m_Resources.Get_Document().Get_AreaId());
    auto staged = std::make_unique<AUTHORING_STATE>(); std::string bytes;
    if (!Read(paths[0], bytes, staged->manifest, status) || !Read(paths[1], bytes, staged->world, status)) return false;
    std::vector<SCENE> scenes; CWorldSequenceDocument document;
    if (!Validate_Authoring(staged->manifest, staged->world, scenes, document, status)) return false;
    staged->manifestBase = staged->manifest; staged->worldBase = staged->world;
    staged->scenes = scenes; staged->document = document;
    staged->needsPublish = (m_Authoring && m_Authoring->needsPublish) || !MatchesPublishedWorld(document, m_Targets);
    staged->generation = m_Authoring ? m_Authoring->generation + 1u : 1u;
    if (!Prepare_Authoring(scenes, document, status) || !Commit_Authoring(std::move(scenes), document, status)) return false;
    staged->appliedToPreview = true;
    m_Authoring = std::move(staged); status = "Movie sources loaded. Unsaved changes were replaced only after validation."; return true;
}

bool CClassSelectionPresentation::Get_RepeatMovie(const std::string& classId) const
{
    const auto& scenes = m_Authoring ? m_Authoring->scenes : m_Scenes;
    const auto scene = std::find_if(scenes.begin(), scenes.end(),
        [&](const SCENE& value) { return value.classId == classId; });
    return scene != scenes.end() && scene->repeatMovie;
}

bool CClassSelectionPresentation::Set_RepeatMovie(const std::string& classId,
    const bool repeat, std::string& status)
{
    if (!Begin_Authoring(status)) return false;
    if (Is_AuthoringPublishPending())
    { status = "Wait for the current Movie publish before changing repeat."; return false; }
    const auto* scenesValue = m_Authoring->manifest.Find("scenes");
    if (!scenesValue || !scenesValue->Is_Array())
    { status = "Movie scene source is unavailable."; return false; }
    auto rows = scenesValue->Get_Array();
    const auto row = std::find_if(rows.begin(), rows.end(),
        [&](const Json& value) { return Text(value, "classId") == classId; });
    const auto live = std::find_if(m_Scenes.begin(), m_Scenes.end(),
        [&](const SCENE& value) { return value.classId == classId; });
    const auto authored = std::find_if(m_Authoring->scenes.begin(), m_Authoring->scenes.end(),
        [&](const SCENE& value) { return value.classId == classId; });
    if (row == rows.end() || live == m_Scenes.end() || authored == m_Authoring->scenes.end())
    { status = "The Movie scene changed; its previous draft was preserved."; return false; }
    *row = Set(*row, "repeatMovie", Json::Boolean(repeat));
    auto manifest = Set(m_Authoring->manifest, "scenes", Json::Array(std::move(rows)));
    std::vector<SCENE> validated;
    if (!Parse(manifest, m_Resources.Get_Document().Get_AreaId(), validated, status)) return false;
    // This policy never changes admission, resources or the current playback pose.
    live->repeatMovie = authored->repeatMovie = repeat;
    m_Authoring->manifest = std::move(manifest);
    m_Authoring->dirty = !Equal(m_Authoring->manifest, m_Authoring->manifestBase) ||
        !Equal(m_Authoring->world, m_Authoring->worldBase);
    ++m_Authoring->generation;
    status = repeat ? "Movie repeat enabled. Save movie keeps this setting." :
        "Movie plays once and holds its last frame. Save movie keeps this setting.";
    m_Authoring->status = status;
    return true;
}

bool CClassSelectionPresentation::Get_AuthoringBox(const std::string& classId, const bool loop,
    const std::string& kind, const std::string& boxId, CLASS_MOVIE_AUTHORING_BOX& out, std::string& status)
{
    if (!Begin_Authoring(status)) return false;
    CLASS_MOVIE_AUTHORING_BOX staged{classId, kind, boxId, loop, m_Authoring->generation};
    bool world = false; Path path;
    if (!Resolve(m_Authoring->manifest, m_Authoring->world, m_Authoring->document, staged, world, path))
    { status = "This background belongs to the Map editor. Select a movie actor, camera, effect, material, light, sound or clock row."; return false; }
    staged.value = ReadRow(world ? m_Authoring->world : m_Authoring->manifest, path, kind);
    out = std::move(staged); status.clear(); return true;
}

bool CClassSelectionPresentation::Apply_AuthoringBox(const CLASS_MOVIE_AUTHORING_BOX& before,
    const Json& replacement, std::string& status)
{
    if (!m_Authoring || before.generation != m_Authoring->generation)
    { status = "The movie changed after this row was opened. Reopen the row before applying."; return false; }
    bool world = false; Path path;
    if (!Resolve(m_Authoring->manifest, m_Authoring->world, m_Authoring->document, before, world, path) ||
        !Equal(before.value, ReadRow(world ? m_Authoring->world : m_Authoring->manifest, path, before.kind)) || Identity(before.value) != Identity(replacement))
    { status = "Movie row identity changed; the existing draft was preserved."; return false; }
    // A camera key edit has no model or Effect resource changes. Validate only
    // its row when the span is unchanged, preserving the admitted phase coverage.
    if (before.kind == "Camera" && !world)
    {
        std::vector<EFFECT_CAMERA_ROW> rows;
        if (!CEffectRecoveryCamera::Parse(Json::Object({{"cameras", Json::Array({replacement})}}), true, rows, status)) return false;
        const auto scene = std::find_if(m_Authoring->scenes.begin(), m_Authoring->scenes.end(),
            [&](const SCENE& value) { return value.classId == before.classId; });
        const auto current = std::find_if(m_Scenes.begin(), m_Scenes.end(),
            [&](const SCENE& value) { return value.classId == before.classId; });
        if (scene == m_Authoring->scenes.end() || current == m_Scenes.end())
        { status = "The camera movie is no longer available; the existing draft and playback were preserved."; return false; }
        auto& phase = before.loop ? scene->loop : scene->intro;
        auto& currentPhase = before.loop ? current->loop : current->intro;
        const auto row = std::find_if(phase.cameras.begin(), phase.cameras.end(),
            [&](const EFFECT_CAMERA_ROW& value) { return value.id == before.boxId; });
        const auto currentRow = std::find_if(currentPhase.cameras.begin(), currentPhase.cameras.end(),
            [&](const EFFECT_CAMERA_ROW& value) { return value.id == before.boxId; });
        const auto& candidate = rows.front();
        if (row == phase.cameras.end() || currentRow == currentPhase.cameras.end())
        { status = "The camera row is no longer available; the existing draft and playback were preserved."; return false; }
        if (!candidate.muted && !candidate.modelRelative && candidate.startMs == row->startMs &&
            candidate.cue.iDurationMs == row->cue.iDurationMs && candidate.startMs == currentRow->startMs &&
            candidate.cue.iDurationMs == currentRow->cue.iDurationMs)
        {
            auto manifest = Replace(m_Authoring->manifest, path, 0u, replacement);
            auto previous = *currentRow;
            *currentRow = candidate;
            if (m_Active && m_Scene == &*current && m_Looping == before.loop)
            {
                const float sampleMs = (std::min)(static_cast<float>(currentPhase.SourceTimeMs(m_ElapsedMs)),
                    std::nextafter(static_cast<float>(currentPhase.durationMs), 0.f));
                if (!Sample_Camera(currentPhase, sampleMs))
                { *currentRow = std::move(previous); status = m_Status; return false; }
            }
            *row = candidate;
            m_Authoring->manifest = std::move(manifest);
            m_Authoring->dirty = !Equal(m_Authoring->manifest, m_Authoring->manifestBase) ||
                !Equal(m_Authoring->world, m_Authoring->worldBase);
            ++m_Authoring->generation; m_Timelines.clear();
            status = "Camera keys applied. Playback time and pause state were preserved. Save movie keeps these keys.";
            m_Authoring->status = status;
            return true;
        }
        // Camera span edits still use complete movie admission below. They may
        // change phase coverage, so they keep the existing stop-and-apply flow.
    }
    Json manifest = m_Authoring->manifest, worldJson = m_Authoring->world;
    auto& target = world ? worldJson : manifest; target = Replace(target, path, 0u, replacement);
    std::vector<SCENE> scenes; CWorldSequenceDocument document;
    if (!Validate_Authoring(manifest, worldJson, scenes, document, status)) return false;
    if (before.kind == "Sound")
    {
        const auto* source = replacement.Find("sourceStartMs");
        if (source && source->Get_Number() > 0.)
        {
            uint32_t nativeDurationMs = 0u;
            const auto asset = CRuntimeAssetRoot::Resolve(Text(replacement, "assetId"));
            if (asset.empty() || !CGameInstance::Get().Get_SoundDurationMs(asset.wstring(), nativeDurationMs) ||
                source->Get_Number() >= nativeDurationMs)
            { status = "Audio source in must be inside the WAV; the current draft is preserved."; return false; }
        }
    }
    if (!Prepare_Authoring(scenes, document, status) || !Commit_Authoring(std::move(scenes), document, status)) return false;
    m_Authoring->scenes = m_Scenes; m_Authoring->document = m_Resources.Get_Document();
    m_Authoring->manifest = std::move(manifest); m_Authoring->world = std::move(worldJson);
    m_Authoring->dirty = !Equal(m_Authoring->manifest, m_Authoring->manifestBase) || !Equal(m_Authoring->world, m_Authoring->worldBase);
    ++m_Authoring->generation; status = "Movie row applied. Save keeps the edited sources."; return true;
}

bool CClassSelectionPresentation::Edit_AuthoringTiming(const CLASS_MOVIE_AUTHORING_BOX& before,
    double start, double end, const CLASS_MOVIE_TIMING_EDIT gesture, std::string& status)
{
    if (!m_Authoring || before.generation != m_Authoring->generation)
    { status = "The movie changed during this gesture. Select the box again."; return false; }
    bool world = false; Path path;
    if (!Resolve(m_Authoring->manifest, m_Authoring->world, m_Authoring->document, before, world, path) ||
        !Equal(before.value, ReadRow(world ? m_Authoring->world : m_Authoring->manifest, path, before.kind)))
    { status = "The selected source changed. Its current draft was preserved."; return false; }
    const auto timeline = Get_Timeline(before.classId, before.loop);
    const CLASS_MOVIE_TIMELINE_BOX* selected = nullptr;
    const CLASS_MOVIE_TIMELINE_ROW* selectedRow = nullptr;
    if (timeline) for (const auto& row : timeline->rows) if (row.kind == before.kind)
        for (const auto& box : row.boxes) if (box.id == before.boxId) { selected = &box; selectedRow = &row; }
    if (!selected || !std::isfinite(start) || !std::isfinite(end) || start < 0. || end <= start || end > timeline->sourceDurationMs)
    { status = "Box time must stay inside the movie phase with a positive duration."; return false; }

    Json replacement = before.value;
    if (before.kind == "Effect")
    {
        replacement = Set(Set(replacement, "startMs", Json::Number(start)), "endMs", Json::Number(end));
        if (gesture == CLASS_MOVIE_TIMING_EDIT::MOVE)
        {
            // Preserve age, root pose and parameter samples inside the visible
            // interval. A collapsed hidden tail is discarded; an expanded phase
            // endpoint holds the boundary sample. Trimming changes visibility only.
            const double oldStart = selected->sourceStartMs, oldEnd = selected->sourceEndMs;
            const double duration = timeline->sourceDurationMs;
            const auto retime = [&](Json& owner, const char* field) {
                Json keys;
                if (!RetimeEffectCurve(*owner.Find(field), oldStart, oldEnd, start, end, duration, keys))
                { status = "Effect move would collapse curve keys. The current draft is preserved."; return false; }
                owner = Set(owner, field, std::move(keys)); return true;
            };
            for (const auto* field : {"clockKeys", "rootKeys"})
                if (replacement.Find(field) && !retime(replacement, field)) return false;
            if (const auto* tracks = replacement.Find("parameterTracks"))
            {
                auto curves = tracks->Get_Array();
                for (auto& curve : curves) if (!retime(curve, "keys")) return false;
                replacement = Set(replacement, "parameterTracks", Json::Array(std::move(curves)));
            }
            if (const auto* delta = replacement.Find("loopAgeDeltaMs"); delta && delta->Get_Number() > 0.)
            {
                const auto& keys = replacement.Find("clockKeys")->Get_Array();
                replacement = Set(replacement, "loopAgeDeltaMs", Json::Number(
                    keys.back().Find("sourceMs")->Get_Number() - keys.front().Find("sourceMs")->Get_Number()));
            }
        }
        return Apply_AuthoringBox(before, replacement, status);
    }
    if (before.kind == "Sound")
    {
        // The gesture arrives in visual source time. Birth remains there, but
        // trimming consumes undilated WAV milliseconds, including instance rate.
        const double rate = selected->playbackRate;
        const double oldStart = replacement.Find("startMs")->Get_Number();
        const double storedStart = std::round(oldStart + (start - selected->sourceStartMs) * rate);
        const double actualStart = selected->sourceStartMs + (storedStart - oldStart) / rate;
        const double movieStart = Map_TimelineTime(before.classId, before.loop, actualStart, false);
        const double movieEnd = Map_TimelineTime(before.classId, before.loop, end, false);
        double sourceIn = selected->sourceOffsetMs;
        double duration = replacement.Find("durationMs")->Get_Number();
        if (gesture == CLASS_MOVIE_TIMING_EDIT::TRIM_START)
        {
            const double trimMs = std::round((movieStart - selected->movieStartMs) * rate);
            sourceIn += trimMs; duration -= trimMs;
        }
        else if (gesture == CLASS_MOVIE_TIMING_EDIT::TRIM_END)
            duration = std::round((movieEnd - selected->movieStartMs) * rate);
        if (storedStart < 0. || sourceIn < 0. || duration < 1. ||
            sourceIn + duration > CWorldSequenceDocument::MAX_DURATION_MS)
        { status = "Sound trim is outside its source window; the current draft is preserved."; return false; }
        replacement = Set(Set(Set(replacement, "startMs", Json::Number(storedStart)),
            "durationMs", Json::Number(duration)), "sourceStartMs", Json::Number(sourceIn));
        return Apply_AuthoringBox(before, replacement, status);
    }
    // Reorder uses a cursor insertion point; it never stores that fractional
    // position. Rounding it could make a valid drop near the phase end empty.
    if (before.kind != "Camera" || gesture != CLASS_MOVIE_TIMING_EDIT::MOVE)
    { start = std::round(start); end = std::round(end); }
    if (end <= start) { status = "This box requires at least one source millisecond."; return false; }
    if (before.kind == "Animation")
    {
        if (selected->loopAnimation || selected->nativeDurationMs <= 0.)
        { status = "Use Box Detail for looping clips. Timeline trim requires a finite native clip."; return false; }
        double sourceIn = selected->sourceOffsetMs, sourceOut = selected->sourceClipEndMs;
        if (gesture == CLASS_MOVIE_TIMING_EDIT::TRIM_START)
            sourceIn += (start - selected->sourceStartMs) * selected->playbackRate;
        if (gesture == CLASS_MOVIE_TIMING_EDIT::TRIM_END)
            sourceOut = sourceIn + (end - start) * selected->playbackRate;
        sourceIn = std::round(sourceIn); sourceOut = std::round(sourceOut);
        const double actualEnd = start + (sourceOut - sourceIn) / selected->playbackRate;
        if (sourceIn < 0. || sourceOut <= sourceIn || sourceOut > selected->nativeDurationMs + 1. ||
            actualEnd > timeline->sourceDurationMs + .001)
        { status = "Source In/Out must remain within the native animation."; return false; }
        for (const auto& other : selectedRow->boxes) if (other.id != selected->id &&
            start < other.sourceEndMs - .001 && actualEnd > other.sourceStartMs + .001)
        { status = "This edit overlaps another clip of the same actor and slot."; return false; }
        replacement = Set(Set(Set(replacement, "startMs", Json::Number(start)),
            "sourceStartMs", Json::Number(sourceIn)), "sourceEndMs", Json::Number(sourceOut));
        return Apply_AuthoringBox(before, replacement, status);
    }
    if (before.kind != "Camera")
    { status = "This track spans the phase. Edit its keys in Box Detail."; return false; }

    // Camera cuts must tile the phase. Move reorders whole cuts; edge trim
    // moves their shared boundary and scales each affected cut's local keys.
    Path collection = path; collection.pop_back();
    const auto* cameras = At(m_Authoring->manifest, collection);
    auto rows = cameras->Get_Array();
    std::sort(rows.begin(), rows.end(), [](const auto& a, const auto& b) {
        return a.Find("startMs")->Get_Number() < b.Find("startMs")->Get_Number(); });
    const auto found = std::find_if(rows.begin(), rows.end(), [&](const auto& row) { return Text(row, "cameraId") == before.boxId; });
    const size_t index = static_cast<size_t>(found - rows.begin());
    if (index == rows.size()) { status = "The camera cut is no longer available."; return false; }
    const auto resize = [&](Json& row, double nextStart, double nextDuration) {
        if (nextDuration < 1.) return false;
        const double previousDuration = row.Find("durationMs")->Get_Number();
        auto keys = row.Find("keys")->Get_Array();
        double previous = -1.;
        for (auto& key : keys)
        {
            const double next = std::round(key.Find("timeMs")->Get_Number() * nextDuration / previousDuration);
            if (next <= previous) return false;
            key = Set(key, "timeMs", Json::Number(next)); previous = next;
        }
        row = Set(Set(Set(row, "startMs", Json::Number(nextStart)), "durationMs", Json::Number(nextDuration)), "keys", Json::Array(std::move(keys)));
        row = Set(row, "source", Json::String("PROJECT_TUNED"));
        return true;
    };
    if (gesture == CLASS_MOVIE_TIMING_EDIT::MOVE)
    {
        size_t destination = 0;
        for (size_t i = 0; i < rows.size(); ++i) if (i != index && start >= rows[i].Find("startMs")->Get_Number()) ++destination;
        auto moved = rows[index]; rows.erase(rows.begin() + index);
        rows.insert(rows.begin() + (std::min)(destination, rows.size()), std::move(moved));
        double cursor = 0.;
        for (auto& row : rows) { row = Set(row, "startMs", Json::Number(cursor)); cursor += row.Find("durationMs")->Get_Number(); }
    }
    else
    {
        const bool left = gesture == CLASS_MOVIE_TIMING_EDIT::TRIM_START;
        if ((left && index == 0u) || (!left && index + 1u == rows.size()))
        { status = "The first and last camera boundaries stay at the phase endpoints."; return false; }
        const size_t a = left ? index - 1u : index, b = a + 1u;
        const double boundary = left ? start : end;
        const double begin = rows[a].Find("startMs")->Get_Number();
        const double finish = rows[b].Find("startMs")->Get_Number() + rows[b].Find("durationMs")->Get_Number();
        if (!resize(rows[a], begin, boundary - begin) || !resize(rows[b], boundary, finish - boundary))
        { status = "The cut is too short for its existing keys or crosses the neighboring cut."; return false; }
    }
    Json manifest = Replace(m_Authoring->manifest, collection, 0u, Json::Array(std::move(rows)));
    std::vector<SCENE> scenes; CWorldSequenceDocument document;
    if (!Validate_Authoring(manifest, m_Authoring->world, scenes, document, status) ||
        !Prepare_Authoring(scenes, document, status) || !Commit_Authoring(std::move(scenes), document, status)) return false;
    m_Authoring->manifest = std::move(manifest); m_Authoring->scenes = m_Scenes;
    m_Authoring->dirty = !Equal(m_Authoring->manifest, m_Authoring->manifestBase) || !Equal(m_Authoring->world, m_Authoring->worldBase);
    ++m_Authoring->generation;
    status = "Camera cut timing applied with complete phase coverage. Save keeps these changes.";
    return true;
}

bool CClassSelectionPresentation::Set_WorldExcluded(const std::string& classId,
    const std::string& objectId, const bool excluded, std::string& status)
{
    if (!Begin_Authoring(status)) return false;
    if (Is_AuthoringPublishPending()) { status = "Wait for the current Movie publish before editing its sources."; return false; }
    auto* scenesValue = m_Authoring->manifest.Find("scenes");
    if (!scenesValue || !scenesValue->Is_Array()) { status = "Movie scene source is unavailable."; return false; }
    auto rows = scenesValue->Get_Array();
    const auto row = std::find_if(rows.begin(), rows.end(), [&](const Json& value) { return Text(value, "classId") == classId; });
    const auto live = std::find_if(m_Scenes.begin(), m_Scenes.end(), [&](const SCENE& value) { return value.classId == classId; });
    const auto authored = std::find_if(m_Authoring->scenes.begin(), m_Authoring->scenes.end(),
        [&](const SCENE& value) { return value.classId == classId; });
    if (row == rows.end() || live == m_Scenes.end() || authored == m_Authoring->scenes.end())
    { status = "The Movie scene changed; its previous draft was preserved."; return false; }
    bool belongs = false;
    for (const auto* phase : {&authored->intro, &authored->loop})
        for (const auto& id : phase->instanceIds)
            if (const auto* instance = m_Authoring->document.Find_Instance(id))
                for (const auto& binding : instance->bindings) belongs |= binding.targetId == objectId;
    if (!belongs || !m_Authoring->document.Find_ObjectResource(objectId))
    { status = "The selected WORLD object does not belong to this Movie."; return false; }
    std::set<std::string> ids(authored->excludedWorldObjectIds.begin(), authored->excludedWorldObjectIds.end());
    if (excluded) ids.insert(objectId); else ids.erase(objectId);
    std::vector<Json> values;
    for (const auto& id : ids) values.push_back(Json::String(id));
    *row = Set(*row, "excludedWorldObjectIds", Json::Array(std::move(values)));
    const auto manifest = Set(m_Authoring->manifest, "scenes", Json::Array(std::move(rows)));
    std::vector<SCENE> scenes; CWorldSequenceDocument document;
    if (!Validate_Authoring(manifest, m_Authoring->world, scenes, document, status)) return false;
    // This edit changes only drawing. Do not Stop/rebuild actors or invalidate
    // Effect bone providers, the current camera, pause, or the Movie clock.
    live->excludedWorldObjectIds.assign(ids.begin(), ids.end());
    authored->excludedWorldObjectIds = live->excludedWorldObjectIds;
    m_Authoring->manifest = manifest;
    m_Authoring->dirty = !Equal(m_Authoring->manifest, m_Authoring->manifestBase) ||
        !Equal(m_Authoring->world, m_Authoring->worldBase);
    ++m_Authoring->generation;
    Apply_WorldInspection();
    status = excluded ? "Deleted from this Movie preview (Intro and Loop). Save Movie persists this change; Restore brings it back." :
        "WORLD item restored to this Movie preview. Save Movie persists this change.";
    m_Authoring->status = status;
    return true;
}

bool CClassSelectionPresentation::Save_Authoring(std::string& status, const bool publish)
{
    if (!Begin_Authoring(status)) return false;
    if (Is_AuthoringPublishPending()) { status = "Movie runtime publishing is still running."; return false; }
    const auto paths = SourcePaths(m_Resources.Get_Document().Get_AreaId());
    if (!m_Authoring->dirty && SourcesMatch(paths, m_Authoring->manifestBase, m_Authoring->worldBase, status))
    {
        m_Authoring->needsPublish = !MatchesPublishedWorld(m_Authoring->document, m_Targets);
        if (publish) return Publish_Authoring(status);
        else status = "Movie sources have no unsaved changes.";
        return true;
    }
    WriterLock locks[2];
    if (!locks[0].Open(paths[0]) || !locks[1].Open(paths[1])) { status = "Another movie source writer is active. The draft is preserved."; return false; }
    std::array<std::string, 2> baseline, output; std::array<Json, 2> latest, merged;
    for (size_t i = 0; i < 2; ++i) if (!Read(paths[i], baseline[i], latest[i], status)) return false;
    if (!Merge(m_Authoring->manifestBase, m_Authoring->manifest, latest[0], merged[0], status, "Movie") ||
        !Merge(m_Authoring->worldBase, m_Authoring->world, latest[1], merged[1], status, "World")) return false;
    if (!Equal(merged[1], latest[1]))
    {
        const auto* revision = latest[1].Find("revision");
        if (!revision || !revision->Is_Number() || revision->Get_Number() >= UINT32_MAX) { status = "World revision cannot advance."; return false; }
        merged[1] = Set(merged[1], "revision", Json::Number(revision->Get_Number() + 1.));
    }
    // Validate the exact emitted bytes, including integer/float lexical changes.
    for (size_t i = 0; i < 2; ++i)
    {
        output[i] = Serialize(merged[i]) + "\n";
        Json parsed;
        if (!CDataJson::Parse(output[i], parsed, status) || !Equal(parsed, merged[i]))
        { status = "Serialized movie source did not round-trip: " + status; return false; }
        merged[i] = std::move(parsed);
    }
    std::vector<SCENE> scenes; CWorldSequenceDocument document;
    if (!Validate_Authoring(merged[0], merged[1], scenes, document, status) || !Prepare_Authoring(scenes, document, status)) return false;
    const auto suffix = L".movie-" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(GetTickCount64());
    std::array<std::filesystem::path, 2> temporary, backup;
    std::array<bool, 2> changed{}, committed{};
    auto cleanup = [&] { for (const auto& path : temporary) if (!path.empty()) { std::error_code ec; std::filesystem::remove(path, ec); } };
    for (size_t i = 0; i < 2; ++i)
    {
        changed[i] = !Equal(merged[i], latest[i]); if (!changed[i]) continue;
        output[i] = Serialize(merged[i]) + "\n"; temporary[i] = paths[i].wstring() + suffix + L".tmp"; backup[i] = paths[i].wstring() + suffix + L".bak";
        if (!Write(temporary[i], output[i])) { cleanup(); status = "Cannot stage movie source save. The draft is preserved."; return false; }
    }
    bool success = true;
    for (size_t i = 0; i < 2 && success; ++i)
    {
        std::string fresh; Json parsed;
        success = Read(paths[i], fresh, parsed, status) && fresh == baseline[i];
    }
    for (size_t i = 0; i < 2 && success; ++i)
    {
        if (!changed[i]) continue;
        std::string fresh; Json parsed;
        success = Read(paths[i], fresh, parsed, status) && fresh == baseline[i];
        if (success) success = ReplaceFileW(paths[i].c_str(), temporary[i].c_str(), backup[i].c_str(), 0u, nullptr, nullptr) != FALSE;
        committed[i] = success;
        // ReplaceFile captures the actual replaced bytes. An external writer can
        // race the preceding freshness read; restore that newer backup on failure.
        if (success) success = Read(backup[i], fresh, parsed, status) && fresh == baseline[i];
    }
    if (success) success = Commit_Authoring(std::move(scenes), document, status);
    if (!success)
    {
        bool restored = true;
        for (size_t i = 2; i-- > 0;)
            if (committed[i])
            {
                std::string fresh; Json parsed;
                const bool own = Read(paths[i], fresh, parsed, status) && fresh == output[i];
                restored &= own && ReplaceFileW(paths[i].c_str(), backup[i].c_str(), nullptr, 0u, nullptr, nullptr) != FALSE;
            }
        cleanup(); status = restored ? "Movie save conflicted or failed; original sources and draft were preserved." :
            "Movie save failed; a concurrent writer prevented rollback. Backups remain beside the source files."; return false;
    }
    cleanup();
    m_Authoring->scenes = m_Scenes; m_Authoring->document = m_Resources.Get_Document();
    m_Authoring->manifest = merged[0]; m_Authoring->world = merged[1];
    m_Authoring->manifestBase = std::move(merged[0]); m_Authoring->worldBase = std::move(merged[1]);
    m_Authoring->dirty = false; ++m_Authoring->generation;
    m_Authoring->needsPublish = !MatchesPublishedWorld(m_Authoring->document, m_Targets);
    if (m_Authoring->needsPublish && publish) Start_AuthoringPublish();
    else if (m_Authoring->needsPublish) m_Authoring->status = "Movie sources saved. Publish applies saved World data for the next entry.";
    else m_Authoring->status = "Movie sources saved and applied. Camera, Effect, light and clock edits also persist on the next entry.";
    status = m_Authoring->status;
    return true;
}

bool CClassSelectionPresentation::Publish_Authoring(std::string& status)
{
    if (!Begin_Authoring(status)) return false;
    if (m_Authoring->dirty) { status = "Save the movie before publishing. The unsaved draft is preserved."; return false; }
    if (m_Authoring->publishProcess) { status = "Movie runtime publishing is still running."; return false; }
    const auto paths = SourcePaths(m_Resources.Get_Document().Get_AreaId());
    if (!SourcesMatch(paths, m_Authoring->manifestBase, m_Authoring->worldBase, status))
    { m_Authoring->status = status; return false; }
    m_Authoring->needsPublish = !MatchesPublishedWorld(m_Authoring->document, m_Targets);
    if (m_Authoring->needsPublish)
    {
        Start_AuthoringPublish(); status = m_Authoring->status;
        return m_Authoring->publishProcess != nullptr;
    }
    status = "Saved camera data is applied to Client playback. No World runtime changes are pending.";
    m_Authoring->status = status;
    return true;
}

void CClassSelectionPresentation::Start_AuthoringPublish()
{
    if (!m_Authoring || !m_Authoring->needsPublish || m_Authoring->publishProcess) return;
    auto& draft = *m_Authoring;
    const auto root = CProjectDataRoot::Get().parent_path();
    const auto script = root / L"Tools/MapPipeline/Publish-MapAuthoring.ps1";
    const auto paths = SourcePaths(m_Resources.Get_Document().Get_AreaId());
    if (!SourcesMatch(paths, draft.manifestBase, draft.worldBase, draft.status)) return;
    draft.needsPublish = !MatchesPublishedWorld(draft.document, m_Targets);
    if (!draft.needsPublish) { draft.status = "Saved Movie sources already match the published World."; return; }
    if (!std::filesystem::is_regular_file(script))
    { draft.status = "Movie sources saved; runtime publisher is missing. Save to retry."; return; }
    wchar_t temporary[MAX_PATH]{};
    if (!GetTempPathW(MAX_PATH, temporary)) { draft.status = "Movie sources saved; publish log folder is unavailable. Save to retry."; return; }
    draft.publishLog = std::filesystem::path(temporary) / (L"LostArk-ClassMovie-" + std::to_wstring(GetCurrentProcessId()) + L".log");
    SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    const HANDLE log = CreateFileW(draft.publishLog.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
        &security, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (log == INVALID_HANDLE_VALUE) { draft.status = "Movie sources saved; cannot create publisher log. Save to retry."; return; }
    const HANDLE input = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        &security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (input == INVALID_HANDLE_VALUE) { CloseHandle(log); draft.status = "Movie sources saved; cannot prepare publisher input. Save to retry."; return; }
    const auto area = m_Resources.Get_Document().Get_AreaId();
    std::wstring command = L"powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass -File \"" + script.wstring() +
        L"\" -ProjectRoot \"" + root.wstring() + L"\" -AreaId \"" + std::wstring(area.begin(), area.end()) + L"\" -Scope WorldSequences -Mode Publish";
    std::vector<wchar_t> arguments(command.begin(), command.end()); arguments.push_back(0);
    STARTUPINFOW startup{}; startup.cb = sizeof(startup); startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = log; startup.hStdError = log; startup.hStdInput = input;
    PROCESS_INFORMATION process{};
    const bool started = CreateProcessW(nullptr, arguments.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW,
        nullptr, root.c_str(), &startup, &process) != FALSE;
    CloseHandle(log); CloseHandle(input);
    if (!started) { draft.status = "Movie sources saved; cannot start publisher. Save to retry."; return; }
    CloseHandle(process.hThread); draft.publishProcess = process.hProcess;
    draft.status = "Movie sources saved and current preview applied; publishing World data for the next entry. Log: " + draft.publishLog.string();
}

void CClassSelectionPresentation::Poll_AuthoringPublish()
{
    if (!m_Authoring || !m_Authoring->publishProcess || WaitForSingleObject(m_Authoring->publishProcess, 0) == WAIT_TIMEOUT) return;
    DWORD code = 1; GetExitCodeProcess(m_Authoring->publishProcess, &code);
    CloseHandle(m_Authoring->publishProcess); m_Authoring->publishProcess = nullptr;
    if (code != 0)
    {
        m_Authoring->status = "Movie sources saved and current preview applied; runtime publish failed (" + std::to_string(code) +
            "). The previous published World remains. Save to retry. Log: " + m_Authoring->publishLog.string(); return;
    }
    m_Authoring->needsPublish = !MatchesPublishedWorld(m_Authoring->document, m_Targets);
    std::string freshness;
    if (!SourcesMatch(SourcePaths(m_Resources.Get_Document().Get_AreaId()),
        m_Authoring->manifestBase, m_Authoring->worldBase, freshness))
    {
        m_Authoring->needsPublish = true;
        m_Authoring->status = "Publisher finished, but " + freshness + " Log: " + m_Authoring->publishLog.string(); return;
    }
    if (m_Authoring->needsPublish)
    {
        m_Authoring->status = "Publisher finished, but the published World does not match the saved movie. Publish to retry. Log: " +
            m_Authoring->publishLog.string(); return;
    }
    m_Authoring->status = "Movie saved, applied and published for Play and the next entry. Log: " + m_Authoring->publishLog.string();
}
}
