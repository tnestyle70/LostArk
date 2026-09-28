#include "ValtanCinematicEffectLibrary.h"
#include "ProjectDataRoot.h"
#include "WorldSequenceDocument.h"

bool Client::Load_ValtanCinematicEffectLibrary(
    VALTAN_CINEMATIC_EFFECT_LIBRARY& output, std::string& status)
{
    constexpr const char* areaId = "LV_LUT_HEARTRB_ED";
    VALTAN_CINEMATIC_EFFECT_LIBRARY staged;
    staged.SourcePath = CProjectDataRoot::Resolve(
        "Maps/Authoring/LV_LUT_HEARTRB_ED/LV_LUT_HEARTRB_ED.worldsequences.json");
    CWorldSequenceDocument document;
    // These cinematic actors are document-owned OBJECT_RESOURCE bindings;
    // never manufacture external Map/Deploy placements to admit another route.
    if (staged.SourcePath.empty() || !document.Load(staged.SourcePath, areaId, {}, {}, status))
    {
        status = "Cinematic Effect library kept its previous list: " + status;
        return false;
    }
    for (const auto& route : VALTAN_SOURCE_CINEMATIC_ROUTES)
    {
        VALTAN_CINEMATIC_EFFECT_GROUP group;
        group.strPatternId = route.patternId;
        group.strInstanceId = "world.sequence.instance.valtan.source-preview." + std::string(route.suffix);
        const auto* instance = document.Find_Instance(group.strInstanceId);
        const auto* sequence = instance ? document.Find_Template(instance->templateId) : nullptr;
        if (!instance || !instance->enabled || !sequence)
        {
            status = "Cinematic Effect library kept its previous list; source instance is absent or disabled: " +
                group.strInstanceId;
            return false;
        }
        group.strSequenceId = sequence->sequenceId;
        group.strDisplayName.assign(reinterpret_cast<const char*>(route.displayName.data()), route.displayName.size());
        group.strSourceDisplayName = sequence->displayName;
        group.iDurationMs = sequence->durationMs;
        for (const auto& track : sequence->effectTracks)
        {
            VALTAN_CINEMATIC_EFFECT_OCCURRENCE effect;
            effect.strTrackId = track.effectTrackId;
            effect.Key.strStableId = track.resourceId;
            effect.Key.eOwnerKind = track.resourceKind == "V1_EFFECT" ? EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT :
                track.resourceKind == "LEAF" ? EFFECT_RESOURCE_OWNER_KIND::V2_LEAF :
                track.resourceKind == "GROUP" ? EFFECT_RESOURCE_OWNER_KIND::V2_GROUP : EFFECT_RESOURCE_OWNER_KIND::END;
            if (!effect.Key.Is_Valid())
            {
                status = "Cinematic Effect library kept its previous list; unsupported resource: " + track.effectTrackId;
                return false;
            }
            effect.iStartMs = sequence->EffectStartMs(track);
            effect.iDurationMs = track.durationMs;
            effect.bLoopEffectToDuration = track.loopEffectToDuration;
            effect.bFitEffectToDuration = track.fitEffectToDuration;
            group.Effects.push_back(std::move(effect));
        }
        staged.Groups.push_back(std::move(group));
    }
    output = std::move(staged);
    status = "Saved cinematic sequence Effects loaded. Preview opens one Effect; Complete Play uses the published sequence.";
    return true;
}
