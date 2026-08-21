#include "AnimationEffectCueDocument.h"

#include "DataJson.h"

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
    std::string Unified_AuthoredEffectAssetId(
        const std::string_view strProductEffectAssetId)
    {
        constexpr std::string_view LegacyToken = ".authored-baseline";
        constexpr std::string_view UnifiedSuffix = ".unified";
        std::string Candidate(strProductEffectAssetId);
        if (const size_t iLegacyToken = Candidate.find(LegacyToken);
            iLegacyToken != std::string::npos)
        {
            Candidate.erase(iLegacyToken, LegacyToken.size());
        }
        if (!Candidate.ends_with(UnifiedSuffix))
            Candidate += UnifiedSuffix;
        return Candidate;
    }

    bool Is_ExactAuthoredEffectMapping(
        const std::string_view strProductEffectAssetId,
        const std::string_view strAuthoredEffectAssetId)
    {
        return !strProductEffectAssetId.empty() &&
            (strProductEffectAssetId == strAuthoredEffectAssetId ||
             Unified_AuthoredEffectAssetId(strProductEffectAssetId) ==
                strAuthoredEffectAssetId);
    }

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

bool_t Client::CAnimationEffectCueDocument::Resolve_PreviewCandidates(
    const ANIMATION_SKILL_BINDING& Binding,
    const std::vector<ANIMATION_EFFECT_CUE>& ProductCues,
    const std::string_view strAuthoredEffectAssetId,
    std::vector<ANIMATION_EFFECT_PREVIEW_CANDIDATE>& OutCandidates,
    std::string& strOutStatus)
{
    if (LostArk::Shared::INVALID_SKILL_ID == Binding.iSkillId ||
        Binding.Stages.empty() || strAuthoredEffectAssetId.empty())
    {
        strOutStatus =
            "Effect preview candidate resolver received an invalid binding or Effect ID.";
        return false;
    }

    std::vector<ANIMATION_EFFECT_PREVIEW_CANDIDATE> Staged;
    size_t iBoundClipOrdinal = 0u;
    for (size_t iStage = 0u; iStage < Binding.Stages.size(); ++iStage)
    {
        const ANIMATION_SKILL_STAGE& Stage = Binding.Stages[iStage];
        for (size_t iClip = 0u; iClip < Stage.Clips.size();
            ++iClip, ++iBoundClipOrdinal)
        {
            const ANIMATION_SKILL_CLIP& Clip = Stage.Clips[iClip];
            for (const ANIMATION_EFFECT_CUE& Cue : ProductCues)
            {
                if (Cue.strClipName != Clip.strClipName ||
                    !Is_ExactAuthoredEffectMapping(
                        Cue.strEffectAssetId, strAuthoredEffectAssetId))
                {
                    continue;
                }
                Staged.push_back(
                    { iBoundClipOrdinal, iStage, iClip, Clip, Cue });
            }
        }
    }

    if (Staged.empty())
    {
        strOutStatus = "No Product animevents cue maps Effect '" +
            std::string(strAuthoredEffectAssetId) +
            "' to an ordered skillbinding clip.";
        return false;
    }

    OutCandidates = std::move(Staged);
    strOutStatus = "Resolved " + std::to_string(OutCandidates.size()) +
        " exact Product cue preview candidate(s).";
    return true;
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

    const std::string Text{
        std::istreambuf_iterator<char>(Input),
        std::istreambuf_iterator<char>() };
    ANIMATION_EFFECT_CUE_DOCUMENT Staged;
    std::vector<std::string> ReferencedClips;
    if (!Load_FromText(
            strAnimationAssetId, Text, {}, Staged, strOutStatus, false,
            &ReferencedClips) ||
        !Load_Projectiles(
            strAnimationAssetId, ReferencedClips, Staged.Projectiles,
            strOutStatus, false))
    {
        return false;
    }

    OutDocument = std::move(Staged);
    strOutStatus = "Loaded " + std::to_string(OutDocument.Cues.size()) +
        " admitted animation Effect cues for Product prewarm without a live model.";
	if (!OutDocument.UnavailableEffectAssetIds.empty())
	{
		strOutStatus += " Isolated " + std::to_string(
			OutDocument.UnavailableEffectAssetIds.size()) +
			" unavailable exact catalog target(s); first: " +
			OutDocument.UnavailableEffectAssetIds.front() + ".";
	}
    return true;
}

bool_t Client::CAnimationEffectCueDocument::Load(
    const std::string& strAnimationAssetId,
    const std::vector<std::string>& AvailableClips,
    ANIMATION_EFFECT_CUE_DOCUMENT& OutDocument,
    std::string& strOutStatus,
    const bool_t bFilterToAvailableClips)
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
	const std::string Text{
		std::istreambuf_iterator<char>(Input),
		std::istreambuf_iterator<char>() };
	ANIMATION_EFFECT_CUE_DOCUMENT Staged;
	if (!Load_FromText(
		strAnimationAssetId, Text, AvailableClips, Staged,
		strOutStatus, bFilterToAvailableClips))
	{
		return false;
	}
	/* The projectile document is optional; when it exists it must parse, so a
	broken one keeps the previous document instead of half a load. */
	if (!Load_Projectiles(strAnimationAssetId, AvailableClips, Staged.Projectiles,
		strOutStatus, bFilterToAvailableClips))
	{
		return false;
	}
	OutDocument = std::move(Staged);
	return true;
}

bool_t Client::CAnimationEffectCueDocument::Load_Projectiles(
	const std::string& strAnimationAssetId,
	const std::vector<std::string>& AvailableClips,
	std::vector<ANIMATION_PROJECTILE_CUE>& OutProjectiles,
	std::string& strOutStatus,
	const bool_t bFilterToAvailableClips)
{
	OutProjectiles.clear();
	const std::filesystem::path Relative =
		std::filesystem::path(L"Animation") / L"Authored" /
		std::filesystem::path(strAnimationAssetId) /
		(std::filesystem::path(strAnimationAssetId).wstring() + L".projectiles.json");
	const std::filesystem::path Path = CProjectDataRoot::Resolve(Relative);
	std::ifstream Input(Path, std::ios::binary);
	if (Path.empty() || !Input)
		return true;
	const std::string Text{
		std::istreambuf_iterator<char>(Input),
		std::istreambuf_iterator<char>() };
	DATA_JSON_VALUE Root;
	std::string Error;
	if (!CDataJson::Parse(Text, Root, Error) || !Root.Is_Object())
	{
		strOutStatus = "Animation projectile document is not valid JSON: " + Error;
		return false;
	}
	const DATA_JSON_VALUE* pSchema = Root.Find("schema");
	const DATA_JSON_VALUE* pVersion = Root.Find("formatVersion");
	const DATA_JSON_VALUE* pAsset = Root.Find("animationAssetId");
	const DATA_JSON_VALUE* pRows = Root.Find("projectiles");
	if (nullptr == pSchema || !pSchema->Is_String() ||
		"lostark.animation-projectiles" != pSchema->Get_String() ||
		nullptr == pVersion || !pVersion->Is_Number() || 1.0 != pVersion->Get_Number() ||
		nullptr == pAsset || !pAsset->Is_String() ||
		strAnimationAssetId != pAsset->Get_String() ||
		nullptr == pRows || !pRows->Is_Array())
	{
		strOutStatus = "Animation projectile document header is invalid.";
		return false;
	}
	const std::unordered_set<std::string> ClipSet(
		AvailableClips.begin(), AvailableClips.end());
	const auto Read_Number = [](const DATA_JSON_VALUE& Object, const char* pKey,
		double& OutValue)
	{
		const DATA_JSON_VALUE* pValue = Object.Find(pKey);
		if (nullptr == pValue || !pValue->Is_Number())
			return false;
		OutValue = pValue->Get_Number();
		return std::isfinite(OutValue);
	};
	const auto To_Cm = [](const double Meters)
	{
		return static_cast<int32_t>(std::lround(Meters * 100.0));
	};
	std::vector<ANIMATION_PROJECTILE_CUE> Staged;
	for (const DATA_JSON_VALUE& Row : pRows->Get_Array())
	{
		const DATA_JSON_VALUE* pClip = Row.Is_Object() ? Row.Find("clip") : nullptr;
		const DATA_JSON_VALUE* pKind = Row.Is_Object() ? Row.Find("kind") : nullptr;
		const DATA_JSON_VALUE* pOrigin = Row.Is_Object() ? Row.Find("origin") : nullptr;
		const DATA_JSON_VALUE* pHits = Row.Is_Object() ? Row.Find("hits") : nullptr;
		double StartMs = 0.0, Speed = 0.0, MinDistance = 0.0, MaxDistance = 0.0;
		double LifeMs = 0.0, Radius = 0.0, OffsetForward = 0.0, OffsetRight = 0.0;
		if (nullptr == pClip || !pClip->Is_String() ||
			nullptr == pKind || !pKind->Is_String() ||
			nullptr == pOrigin || !pOrigin->Is_String() ||
			("CASTER" != pOrigin->Get_String() && "AIM" != pOrigin->Get_String()) ||
			nullptr == pHits || !pHits->Is_Array() ||
			!Read_Number(Row, "offsetForward", OffsetForward) ||
			!Read_Number(Row, "offsetRight", OffsetRight) ||
			!Read_Number(Row, "startMs", StartMs) || StartMs < 0.0 ||
			!Read_Number(Row, "speed", Speed) || Speed < 0.0 ||
			!Read_Number(Row, "minDistance", MinDistance) || MinDistance < 0.0 ||
			!Read_Number(Row, "maxDistance", MaxDistance) || MaxDistance < 0.0 ||
			!Read_Number(Row, "lifeMs", LifeMs) || LifeMs <= 0.0 ||
			!Read_Number(Row, "radius", Radius) || Radius < 0.0)
		{
			strOutStatus = "Animation projectile row is invalid.";
			return false;
		}
		ANIMATION_PROJECTILE_CUE Cue;
		Cue.strClipName = pClip->Get_String();
		const std::string& Kind = pKind->Get_String();
		if ("MISSILE" == Kind) Cue.iKind = 0u;
		else if ("FIXAREA" == Kind) Cue.iKind = 1u;
		else if ("GRENADE" == Kind) Cue.iKind = 2u;
		else if ("TRACE" == Kind) Cue.iKind = 3u;
		else
		{
			strOutStatus = "Animation projectile kind is invalid.";
			return false;
		}
		Cue.iOrigin = "AIM" == pOrigin->Get_String() ? 1u : 0u;
		Cue.fOffsetForward = static_cast<f32_t>(OffsetForward);
		Cue.fOffsetRight = static_cast<f32_t>(OffsetRight);
		Cue.iStartMs = static_cast<uint32_t>(StartMs);
		Cue.fSpeed = static_cast<f32_t>(Speed);
		Cue.fMinDistance = static_cast<f32_t>(MinDistance);
		Cue.fMaxDistance = static_cast<f32_t>(MaxDistance);
		Cue.iLifeMs = static_cast<uint32_t>(LifeMs);
		Cue.fRadius = static_cast<f32_t>(Radius);
		for (const DATA_JSON_VALUE& Hit : pHits->Get_Array())
		{
			double AreaType = 0.0, Range = 0.0, Angle = 0.0, Width = 0.0;
			double Height = 0.0, Offset = 0.0, Inner = 0.0;
			if (!Hit.Is_Object() ||
				!Read_Number(Hit, "areaType", AreaType) || AreaType < 1.0 || AreaType > 3.0 ||
				!Read_Number(Hit, "range", Range) || Range <= 0.0 ||
				!Read_Number(Hit, "angle", Angle) || Angle < 0.0 || Angle > 360.0 ||
				!Read_Number(Hit, "width", Width) || Width < 0.0 ||
				!Read_Number(Hit, "height", Height) || Height < 0.0 ||
				!Read_Number(Hit, "offset", Offset) ||
				!Read_Number(Hit, "inner", Inner) || Inner < 0.0)
			{
				strOutStatus = "Animation projectile hit is invalid.";
				return false;
			}
			HIT_AREA_SHAPE Shape{};
			Shape.iAreaType = static_cast<int32_t>(AreaType);
			Shape.iAreaRange = To_Cm(Range);
			/* The wire reads a box width and a fan sweep from the same field,
			   as the official AreaAngle column does. */
			Shape.iAreaAngle = 2 == Shape.iAreaType ?
				To_Cm(Width) : static_cast<int32_t>(std::lround(Angle));
			Shape.iAreaHeight = To_Cm(Height);
			Shape.iAreaOffsetX = To_Cm(Offset);
			Shape.iAreaInner = To_Cm(Inner);
			Cue.Shapes.push_back(Shape);
		}
		if (bFilterToAvailableClips && !ClipSet.contains(Cue.strClipName))
			continue;
		if (!ClipSet.contains(Cue.strClipName))
		{
			strOutStatus = "Animation projectile row names an unknown clip: " +
				Cue.strClipName;
			return false;
		}
		Staged.push_back(std::move(Cue));
	}
	OutProjectiles = std::move(Staged);
	return true;
}

bool_t Client::CAnimationEffectCueDocument::Load_FromText(
	const std::string& strAnimationAssetId,
	const std::string_view Text,
	const std::vector<std::string>& AvailableClips,
	ANIMATION_EFFECT_CUE_DOCUMENT& OutDocument,
	std::string& strOutStatus,
	const bool_t bFilterToAvailableClips)
{
	return Load_FromText(
		strAnimationAssetId, Text, AvailableClips, OutDocument,
		strOutStatus, bFilterToAvailableClips, nullptr);
}

bool_t Client::CAnimationEffectCueDocument::Load_FromText(
	const std::string& strAnimationAssetId,
	const std::string_view Text,
	const std::vector<std::string>& AvailableClips,
	ANIMATION_EFFECT_CUE_DOCUMENT& OutDocument,
	std::string& strOutStatus,
	const bool_t bFilterToAvailableClips,
	std::vector<std::string>* pOutReferencedClips)
{
	if (strAnimationAssetId.empty() || Text.empty())
	{
		strOutStatus = "Animation asset ID or event text is empty.";
		return false;
	}
	std::istringstream Input{ std::string(Text) };

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

    const bool_t bDiscoverReferencedClips =
        nullptr != pOutReferencedClips;
    if (bDiscoverReferencedClips &&
        (!AvailableClips.empty() || bFilterToAvailableClips))
    {
        strOutStatus =
            "Animation event clip discovery arguments are incompatible.";
        return false;
    }
    const std::unordered_set<std::string> ClipSet(
        AvailableClips.begin(), AvailableClips.end());
    std::unordered_set<std::string> ReferencedClipSet;
    const auto Is_AvailableClip =
        [&ClipSet, &ReferencedClipSet, bDiscoverReferencedClips](
            const std::string& ClipName)
        {
            return bDiscoverReferencedClips ?
                ReferencedClipSet.contains(ClipName) :
                ClipSet.contains(ClipName);
        };
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
        if (bDiscoverReferencedClips &&
            ("EFFECT" == Tokens[1] || "HIT" == Tokens[1]))
        {
            ReferencedClipSet.insert(Tokens[0]);
        }
        if ("HIT" == Tokens[1])
        {
			if (bFilterToAvailableClips &&
				!Is_AvailableClip(Tokens[0]))
			{
				continue;
			}
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
                !Is_AvailableClip(Hit.strClipName))
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
		if (bFilterToAvailableClips &&
			!Is_AvailableClip(Tokens[0]))
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
            !Is_AvailableClip(Cue.strClipName))
        {
            strOutStatus =
				"Animation EFFECT cue failed clip/time/transform validation.";
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
            strOutStatus = "Duplicate Animation EFFECT cue.";
            return false;
        }
		if (!CEffectCatalog::Contains(Cue.strEffectAssetId))
		{
			Staged.UnavailableEffectAssetIds.push_back(
				Cue.strEffectAssetId);
			continue;
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
	std::sort(Staged.UnavailableEffectAssetIds.begin(),
		Staged.UnavailableEffectAssetIds.end());
	Staged.UnavailableEffectAssetIds.erase(std::unique(
		Staged.UnavailableEffectAssetIds.begin(),
		Staged.UnavailableEffectAssetIds.end()),
		Staged.UnavailableEffectAssetIds.end());
    if (nullptr != pOutReferencedClips)
    {
        std::vector<std::string> ReferencedClips(
            ReferencedClipSet.begin(), ReferencedClipSet.end());
        std::sort(ReferencedClips.begin(), ReferencedClips.end());
        *pOutReferencedClips = std::move(ReferencedClips);
    }
    OutDocument = std::move(Staged);
    strOutStatus = "Loaded " + std::to_string(OutDocument.Cues.size()) +
		" admitted animation Effect cues.";
	if (!OutDocument.UnavailableEffectAssetIds.empty())
	{
		strOutStatus += " Isolated " + std::to_string(
			OutDocument.UnavailableEffectAssetIds.size()) +
			" unavailable exact catalog target(s); first: " +
			OutDocument.UnavailableEffectAssetIds.front() + ".";
	}
    return true;
}

