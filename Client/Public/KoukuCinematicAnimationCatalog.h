#pragma once

#include "KoukuSaydonCompositionDocument.h"

namespace Client
{
struct KOUKU_CINEMATIC_ANIMATION_KEY final
{
    std::string strId, strSlot, strStatus, strSourceClip, strNotice;
    std::uint32_t iTrackExport = 0u;
    double fNativeStartMs = 0.0, fLocalStartMs = 0.0;
    double fSourceInMs = 0.0, fSourceEndOffsetMs = 0.0;
    bool bSourceLoop = false, bSourceReverse = false;
    KOUKU_SAYDON_COMPOSITION_ANIMATION_OCCURRENCE Animation;
};

struct KOUKU_CINEMATIC_ANIMATION_GROUP final
{
    std::string strId, strSceneId, strDisplayName, strSourceScene, strGroupName;
    std::string strProfileId, strRuntimeProfileId, strModelAssetId, strBindingStatus;
    std::uint32_t iMatineeExport = 0u, iGroupExport = 0u;
    std::vector<KOUKU_CINEMATIC_ANIMATION_KEY> Keys;
};

class CKoukuCinematicAnimationCatalog final
{
public:
    // A bad document preserves the previous catalog. A bad key stays visible
    // with its diagnostic and cannot become an authored occurrence.
    static bool Load(std::vector<KOUKU_CINEMATIC_ANIMATION_GROUP>& groups, std::string& status);
    static bool Parse(std::string_view text,
        std::vector<KOUKU_CINEMATIC_ANIMATION_GROUP>& groups, std::string& status);
};
}
