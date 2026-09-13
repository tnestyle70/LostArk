#include "KoukuCinematicAnimationCatalog.h"
#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <cmath>
#include <fstream>
#include <iterator>
#include <unordered_set>

namespace
{
using namespace Client;
std::string Text(const DATA_JSON_VALUE& row, const char* name)
{
    const auto* value = row.Find(name);
    return value && value->Is_String() ? value->Get_String() : std::string{};
}
bool Number(const DATA_JSON_VALUE& row, const char* name, double& number)
{
    const auto* value = row.Find(name);
    if (!value || !value->Is_Number() || !std::isfinite(value->Get_Number())) return false;
    number = value->Get_Number(); return true;
}
bool Unsigned(const DATA_JSON_VALUE& row, const char* name, std::uint32_t& number, const double max = 600000.0)
{
    double value = 0.0;
    if (!Number(row, name, value) || value < 0.0 || value > max || std::floor(value) != value) return false;
    number = static_cast<std::uint32_t>(value); return true;
}
}

bool Client::CKoukuCinematicAnimationCatalog::Load(
    std::vector<KOUKU_CINEMATIC_ANIMATION_GROUP>& groups, std::string& status)
{
    const auto path = CProjectDataRoot::Resolve(L"Animation/Reference/KoukuSaydon/KoukuSaydon.cinematicreference.json");
    std::error_code error;
    const auto bytes = std::filesystem::file_size(path, error);
    if (error || bytes > 8u * 1024u * 1024u)
    { status = "Cinematic reference is missing or exceeds 8 MiB; previous catalog preserved."; return false; }
    std::ifstream input(path, std::ios::binary);
    const std::string text((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    if (!input || text.size() != bytes)
    { status = "Cinematic reference read failed; previous catalog preserved."; return false; }
    return Parse(text, groups, status);
}

bool Client::CKoukuCinematicAnimationCatalog::Parse(std::string_view text,
    std::vector<KOUKU_CINEMATIC_ANIMATION_GROUP>& groups, std::string& status)
{
    DATA_JSON_VALUE root;
    if (!CDataJson::Parse(text, root, status)) return false;
    std::uint32_t version = 0u;
    const auto* rows = root.Find("groups");
    if (Text(root, "schema") != "lostark.kouku-cinematic-animation-reference" ||
        Text(root, "authority") != "REFERENCE_ONLY" || !Unsigned(root, "formatVersion", version) || version != 1u ||
        !rows || !rows->Is_Array() || rows->Get_Array().size() > 128u)
    { status = "Invalid cinematic reference header; previous catalog preserved."; return false; }
    std::vector<KOUKU_CINEMATIC_ANIMATION_GROUP> staged;
    std::unordered_set<std::string> groupIds, keyIds;
    std::size_t keyCount = 0u, holdouts = 0u;
    for (const auto& row : rows->Get_Array())
    {
        KOUKU_CINEMATIC_ANIMATION_GROUP group;
        group.strId = Text(row, "id"); group.strSceneId = Text(row, "sceneId");
        group.strDisplayName = Text(row, "displayName"); group.strSourceScene = Text(row, "sourceScene");
        group.strGroupName = Text(row, "groupName"); group.strProfileId = Text(row, "profileId");
        group.strRuntimeProfileId = Text(row, "runtimeProfileId");
        group.strModelAssetId = Text(row, "modelAssetId"); group.strBindingStatus = Text(row, "bindingError");
        const auto* keys = row.Find("keys");
        if (group.strId.empty() || !groupIds.insert(group.strId).second || group.strSceneId.empty() ||
            group.strDisplayName.empty() || !Unsigned(row, "matineeExport", group.iMatineeExport) ||
            !Unsigned(row, "groupExport", group.iGroupExport) || !keys || !keys->Is_Array() || keys->Get_Array().size() > 256u)
        { status = "Invalid cinematic group; previous catalog preserved."; return false; }
        for (const auto& source : keys->Get_Array())
        {
            KOUKU_CINEMATIC_ANIMATION_KEY key;
            key.strId = Text(source, "id"); key.strNotice = Text(source, "previewNotice"); key.strSlot = Text(source, "slot"); key.strStatus = Text(source, "error");
            const auto* native = source.Find("sourceKey");
            if (native && native->Is_Object())
            {
                key.strSourceClip = Text(*native, "animseqname");
                double seconds = 0.0;
                if (Number(*native, "animstartoffset", seconds)) key.fSourceInMs = seconds * 1000.0;
                if (Number(*native, "animendoffset", seconds)) key.fSourceEndOffsetMs = seconds * 1000.0;
                if (const auto* loop = native->Find("blooping"); loop && loop->Is_Boolean()) key.bSourceLoop = loop->Get_Boolean();
                if (const auto* reverse = native->Find("breverse"); reverse && reverse->Is_Boolean()) key.bSourceReverse = reverse->Get_Boolean();
            }
            if (key.strSourceClip.empty()) key.strStatus = "Missing native source key.";
            auto& animation = key.Animation;
            animation.strProfileId = group.strRuntimeProfileId; animation.strSourceStageId = "RAW";
            animation.strSourceSlotId = key.strId; animation.strRuntimeClip = Text(source, "runtimeClip");
            animation.strEndPolicy = Text(source, "endPolicy");
            double rate = 0.0;
            if (key.strId.empty() || !keyIds.insert(group.strId + "/" + key.strId).second ||
                !Unsigned(source, "trackExport", key.iTrackExport) ||
                !Number(source, "nativeStartMs", key.fNativeStartMs) ||
                !Number(source, "localStartMs", key.fLocalStartMs) || key.fLocalStartMs < 0.0 ||
                !Unsigned(source, "sourceStartMs", animation.iSourceStartMs) ||
                !Unsigned(source, "sourceEndMs", animation.iSourceEndMs) ||
                !Unsigned(source, "playMs", animation.iPlayMs) || animation.iPlayMs == 0u ||
                !Number(source, "playRate", rate) || rate < .01 || rate > 16.0)
                key.strStatus = "Invalid cinematic source key timing or identity.";
            animation.fPlayRate = static_cast<float>(rate);
            if (key.strStatus.empty() && (animation.strRuntimeClip.empty() || group.strProfileId.empty() ||
                animation.iSourceEndMs <= animation.iSourceStartMs ||
                (animation.strEndPolicy != "LOOP_TO_WINDOW" && animation.strEndPolicy != "HOLD_LAST_POSE")))
                key.strStatus = "Cinematic key has no admitted forward source window.";
            if (!key.strStatus.empty()) ++holdouts;
            group.Keys.push_back(std::move(key)); ++keyCount;
        }
        staged.push_back(std::move(group));
    }
    groups = std::move(staged);
    status = "Cinematic reference: " + std::to_string(groups.size()) + " actor groups / " +
        std::to_string(keyCount) + " keys / " + std::to_string(holdouts) + " diagnostic keys.";
    return true;
}
