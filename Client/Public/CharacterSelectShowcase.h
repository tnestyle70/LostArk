#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <memory>
#include <string>

NS_BEGIN(Client)

class CCharacter;
class CMapPlacementRuntime;

/* A Level-owned presentation stage. Objects still use the ordinary Layer
update/render path; this service never controls the replicated actor's pose. */
class CCharacterSelectShowcase final
{
public:
    CCharacterSelectShowcase();
    ~CCharacterSelectShowcase();
    CCharacterSelectShowcase(const CCharacterSelectShowcase&) = delete;
    CCharacterSelectShowcase& operator=(const CCharacterSelectShowcase&) = delete;

    /* Call once per Level Update. A new clone stays hidden until the next
    normal Object Update; false with an empty status is this pending frame.
    Hide preserves that preparation, while Clear removes all owned objects. */
    bool Show(const CMapPlacementRuntime& sourceMap,
        const std::shared_ptr<CCharacter>& approvedCharacter, std::string& status);
    void Hide();
    // End PREVIEW: restore the replica and remove its display-only clone.
    // Show recreates the default idle and preserves its normal warmup frame.
    void Leave();
    void Clear();
    std::shared_ptr<CCharacter> Get_Character() const;
    bool Is_Visible() const;
    float3_t Get_LightingTranslation() const;

private:
    struct STATE;
    std::unique_ptr<STATE> m_State;
};

NS_END
