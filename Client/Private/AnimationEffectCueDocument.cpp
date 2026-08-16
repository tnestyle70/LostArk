#include "AnimationEffectCueDocument.h"

#include "Effect_Catalog.h"
#include "ProjectDataRoot.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <sstream>
#include <tuple>
#include <unordered_set>

namespace
{
    bool Tokenize(
        const std::string& Line,
        std::vector<std::string>& Out,
        std::string& Error)
    {
        Out.clear();
        size_t Cursor = 0u;
        while (Cursor < Line.size())
        {
            while (Cursor < Line.size() &&
                std::isspace(static_cast<unsigned char>(Line[Cursor])))
                ++Cursor;
            if (Cursor == Line.size())
                break;
            std::string Token;
            bool InQuote = false;
            while (Cursor < Line.size())
            {
                const char Character = Line[Cursor++];
                if ('"' == Character)
                {
                    InQuote = !InQuote;
                    continue;
                }
                if ('\\' == Character && InQuote && Cursor < Line.size())
                {
                    Token.push_back(Line[Cursor++]);
                    continue;
                }
                if (!InQuote &&
                    std::isspace(static_cast<unsigned char>(Character)))
                    break;
                Token.push_back(Character);
            }
            if (InQuote)
            {
                Error = "Unterminated quote in animevents row.";
                return false;
            }
            Out.push_back(std::move(Token));
        }
        return true;
    }

    bool Parse_UInt(
        const std::string& Text,
        uint32_t& Out)
    {
        const char* Begin = Text.data();
        const char* End = Begin + Text.size();
        const auto Result = std::from_chars(Begin, End, Out);
        return Result.ec == std::errc{} && Result.ptr == End;
    }

    bool Parse_Float(
        const std::string& Text,
        float& Out)
    {
        char* pEnd = nullptr;
        Out = std::strtof(Text.c_str(), &pEnd);
        return pEnd == Text.c_str() + Text.size() && std::isfinite(Out);
    }

    std::map<std::string, std::string, std::less<>> Make_Fields(
        const std::vector<std::string>& Tokens,
        const size_t iFirst,
        bool& bValid)
    {
        std::map<std::string, std::string, std::less<>> Result;
        bValid = true;
        for (size_t iToken = iFirst; iToken < Tokens.size(); ++iToken)
        {
            const size_t Equal = Tokens[iToken].find('=');
            if (std::string::npos == Equal || 0u == Equal ||
                !Result.emplace(Tokens[iToken].substr(0u, Equal),
                    Tokens[iToken].substr(Equal + 1u)).second)
            {
                bValid = false;
                return {};
            }
        }
        return Result;
    }

    bool Read_Transform(
        const std::map<std::string, std::string, std::less<>>& Fields,
        Client::EFFECT_TRANSFORM_DESC& Out)
    {
        const auto Read = [&Fields](const char* pName, float& Value)
        {
            const auto Iterator = Fields.find(pName);
            return Fields.end() == Iterator ||
                Parse_Float(Iterator->second, Value);
        };
        return Read("px", Out.vPosition.x) &&
            Read("py", Out.vPosition.y) &&
            Read("pz", Out.vPosition.z) &&
            Read("rx", Out.vRotationDegrees.x) &&
            Read("ry", Out.vRotationDegrees.y) &&
            Read("rz", Out.vRotationDegrees.z) &&
            Read("sx", Out.vScale.x) &&
            Read("sy", Out.vScale.y) &&
            Read("sz", Out.vScale.z) &&
            Out.vScale.x > 0.f && Out.vScale.y > 0.f && Out.vScale.z > 0.f;
    }
}

bool_t Client::CAnimationEffectCueDocument::Load_ForProductPrewarm(
    const std::string& strAnimationAssetId,
    ANIMATION_EFFECT_CUE_DOCUMENT& OutDocument,
    std::string& strOutStatus)
{
    if (strAnimationAssetId.empty())
    {
        strOutStatus = "Animation asset ID is empty.";
        return false;
    }

    const std::filesystem::path Relative =
        std::filesystem::path(L"Animation") / L"Authored" /
        std::filesystem::path(strAnimationAssetId) /
        (std::filesystem::path(strAnimationAssetId).wstring() + L".animevents");
    const std::filesystem::path Path = CProjectDataRoot::Resolve(Relative);
    std::ifstream Input(Path, std::ios::binary);
    if (Path.empty() || !Input)
    {
        strOutStatus = "Missing animation event document: " + Path.string();
        return false;
    }

    std::string Line;
    if (!std::getline(Input, Line))
    {
        strOutStatus = "Animation event document is empty.";
        return false;
    }

    std::unordered_set<std::string> ReferencedClipSet;
    std::vector<std::string> Tokens;
    std::string Error;
    while (std::getline(Input, Line))
    {
        if (Line.empty())
            continue;
        if (!Tokenize(Line, Tokens, Error) || Tokens.size() < 2u)
        {
            strOutStatus = Error.empty() ?
                "Invalid animation event row while collecting referenced clips." :
                Error;
            return false;
        }
        if ("EFFECT" == Tokens[1] || "HIT" == Tokens[1])
            ReferencedClipSet.insert(Tokens[0]);
    }

    std::vector<std::string> ReferencedClips(
        ReferencedClipSet.begin(), ReferencedClipSet.end());
    if (!Load(strAnimationAssetId, ReferencedClips, OutDocument, strOutStatus))
        return false;

    strOutStatus = "Loaded " + std::to_string(OutDocument.Cues.size()) +
        " admitted animation Effect cues for Product prewarm without a live model.";
    return true;
}

bool_t Client::CAnimationEffectCueDocument::Load(
    const std::string& strAnimationAssetId,
    const std::vector<std::string>& AvailableClips,
    ANIMATION_EFFECT_CUE_DOCUMENT& OutDocument,
    std::string& strOutStatus)
{
    if (strAnimationAssetId.empty())
    {
        strOutStatus = "Animation asset ID is empty.";
        return false;
    }
    const std::filesystem::path Relative =
        std::filesystem::path(L"Animation") / L"Authored" /
        std::filesystem::path(strAnimationAssetId) /
        (std::filesystem::path(strAnimationAssetId).wstring() + L".animevents");
    const std::filesystem::path Path = CProjectDataRoot::Resolve(Relative);
    std::ifstream Input(Path, std::ios::binary);
    if (Path.empty() || !Input)
    {
        strOutStatus = "Missing animation event document: " + Path.string();
        return false;
    }

    std::string HeaderLine;
    if (!std::getline(Input, HeaderLine))
    {
        strOutStatus = "Animation event document is empty.";
        return false;
    }
    std::vector<std::string> Tokens;
    std::string Error;
    if (!Tokenize(HeaderLine, Tokens, Error) || Tokens.size() != 4u ||
        "LOSTARK_ANIM_EVENTS" != Tokens[0])
    {
        strOutStatus = Error.empty() ?
            "Invalid animation event header." : Error;
        return false;
    }
    uint32_t Version = 0u;
    uint32_t DeclaredRows = 0u;
    if (!Parse_UInt(Tokens[1], Version) || Version < 3u || Version > 5u ||
        Tokens[2] != strAnimationAssetId ||
        !Parse_UInt(Tokens[3], DeclaredRows))
    {
        strOutStatus = "Animation event header owner/version/count is invalid.";
        return false;
    }

    const std::unordered_set<std::string> ClipSet(
        AvailableClips.begin(), AvailableClips.end());
    std::unordered_set<std::string> Keys;
    ANIMATION_EFFECT_CUE_DOCUMENT Staged;
    Staged.iFormatVersion = Version;
    Staged.strAnimationAssetId = strAnimationAssetId;
    uint32_t ActualRows = 0u;
    std::string Line;
    while (std::getline(Input, Line))
    {
        if (Line.empty())
            continue;
        ++ActualRows;
        if (!Tokenize(Line, Tokens, Error) || Tokens.size() < 3u)
        {
            strOutStatus = Error.empty() ?
                "Invalid animation event row." : Error;
            return false;
        }
        if ("HIT" == Tokens[1])
        {
            bool HitFieldsValid = false;
            const auto HitFields = Make_Fields(Tokens, 2u, HitFieldsValid);
            if (!HitFieldsValid)
            {
                strOutStatus = "Animation HIT row has an invalid or duplicate field.";
                return false;
            }
            ANIMATION_HIT_CUE Hit;
            Hit.strClipName = Tokens[0];
            const auto Read_UInt = [&HitFields](const char* pName, uint32_t& Value)
            {
                const auto Iterator = HitFields.find(pName);
                return HitFields.end() == Iterator || Parse_UInt(Iterator->second, Value);
            };
            const auto Read_Int = [&HitFields](const char* pName, int32_t& Value)
            {
                const auto Iterator = HitFields.find(pName);
                if (HitFields.end() == Iterator)
                    return true;
                const char* Begin = Iterator->second.data();
                const char* End = Begin + Iterator->second.size();
                const auto Result = std::from_chars(Begin, End, Value);
                return Result.ec == std::errc{} && Result.ptr == End;
            };
            const auto Start = HitFields.find("startms");
            if (HitFields.end() == Start || !Parse_UInt(Start->second, Hit.iStartMs))
            {
                strOutStatus = "Animation HIT row has an invalid startms.";
                return false;
            }
            Hit.iEndMs = Hit.iStartMs;
            if (!Read_UInt("endms", Hit.iEndMs) ||
                !Read_UInt("rep", Hit.iRepeatCount) ||
                !Read_UInt("repms", Hit.iRepeatMs) ||
                !Read_Int("area", Hit.Shape.iAreaType) ||
                !Read_Int("ar", Hit.Shape.iAreaRange) ||
                !Read_Int("aa", Hit.Shape.iAreaAngle) ||
                !Read_Int("ah", Hit.Shape.iAreaHeight) ||
                !Read_Int("ax", Hit.Shape.iAreaOffsetX) ||
                !Read_Int("arem", Hit.Shape.iAreaInner) ||
                Hit.iEndMs < Hit.iStartMs || Hit.iRepeatCount < 1u ||
                !ClipSet.contains(Hit.strClipName))
            {
                strOutStatus = "Animation HIT row failed clip/time/shape validation.";
                return false;
            }
            Staged.Hits.push_back(std::move(Hit));
            continue;
        }
        if ("EFFECT" != Tokens[1])
            continue;
        bool FieldsValid = false;
        const auto Fields = Make_Fields(Tokens, 2u, FieldsValid);
        if (!FieldsValid)
        {
            strOutStatus = "Animation EFFECT row has an invalid or duplicate field.";
            return false;
        }
        const auto Payload = Fields.find("payload");
        const auto EffectRef = Fields.find("effectref");
        if (Fields.end() == Payload || Payload->second.empty() ||
            Fields.end() == EffectRef || "asset" != EffectRef->second)
        {
            continue;
        }

        ANIMATION_EFFECT_CUE Cue;
        Cue.strClipName = Tokens[0];
        Cue.strEffectAssetId = Payload->second;
        const auto Start = Fields.find("startms");
        if (Fields.end() == Start || !Parse_UInt(Start->second, Cue.iStartMs))
        {
            strOutStatus = "Animation EFFECT cue has an invalid startms.";
            return false;
        }
        Cue.iEndMs = Cue.iStartMs;
        const auto End = Fields.find("endms");
        if (Fields.end() != End && !Parse_UInt(End->second, Cue.iEndMs))
        {
            strOutStatus = "Animation EFFECT cue has an invalid endms.";
            return false;
        }
        const auto Anchor = Fields.find("anchor");
        if (Fields.end() != Anchor)
            Cue.strAnchorSlotId = Anchor->second;
        const auto Follow = Fields.find("follow");
        if (Fields.end() != Follow)
        {
            if ("follow" == Follow->second)
                Cue.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
            else if ("snapshot" == Follow->second)
                Cue.eFollowPolicy = EFFECT_FOLLOW_POLICY::SNAPSHOT;
            else
            {
                strOutStatus = "Animation EFFECT cue has an unknown follow policy.";
                return false;
            }
        }
        const auto Stop = Fields.find("stop");
        if (Fields.end() != Stop)
        {
            if ("natural" == Stop->second)
                Cue.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
            else if ("cue_end" == Stop->second)
                Cue.eStopPolicy = EFFECT_STOP_POLICY::CUE_END;
            else
            {
                strOutStatus = "Animation EFFECT cue has an unknown stop policy.";
                return false;
            }
        }
        if (!Read_Transform(Fields, Cue.LocalTransform) ||
            Cue.iEndMs < Cue.iStartMs || Cue.strAnchorSlotId.empty() ||
            !ClipSet.contains(Cue.strClipName) ||
            !CEffectCatalog::Contains(Cue.strEffectAssetId))
        {
            strOutStatus = "Animation EFFECT cue failed clip/effect/transform validation.";
            return false;
        }
        if (!CEffectCatalog::Admit_ProductCue(strAnimationAssetId,
                Cue.strClipName, Cue.iStartMs, Cue.strEffectAssetId,
                Cue.pProductAdmissionToken, Error))
        {
            strOutStatus = "Animation EFFECT cue Product admission failed: " +
                Error;
            return false;
        }
        if (EFFECT_STOP_POLICY::CUE_END == Cue.eStopPolicy &&
            Cue.iEndMs <= Cue.iStartMs)
        {
            strOutStatus = "cue_end requires endms greater than startms.";
            return false;
        }
        const std::string Key = Cue.strClipName + "\n" +
            std::to_string(Cue.iStartMs) + "\n" + Cue.strEffectAssetId +
            "\n" + Cue.strAnchorSlotId;
        if (!Keys.insert(Key).second)
        {
            strOutStatus = "Duplicate admitted Animation EFFECT cue.";
            return false;
        }
        Staged.Cues.push_back(std::move(Cue));
    }
    if (ActualRows != DeclaredRows)
    {
        strOutStatus = "Animation event row count does not match the header.";
        return false;
    }
    std::sort(Staged.Cues.begin(), Staged.Cues.end(),
        [](const ANIMATION_EFFECT_CUE& Left,
            const ANIMATION_EFFECT_CUE& Right)
        {
            return std::tie(Left.strClipName, Left.iStartMs,
                Left.strEffectAssetId, Left.strAnchorSlotId) <
                std::tie(Right.strClipName, Right.iStartMs,
                    Right.strEffectAssetId, Right.strAnchorSlotId);
        });
    OutDocument = std::move(Staged);
    strOutStatus = "Loaded " + std::to_string(OutDocument.Cues.size()) +
        " admitted animation Effect cues.";
    return true;
}

