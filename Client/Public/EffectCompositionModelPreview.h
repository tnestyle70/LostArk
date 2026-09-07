#pragma once

#include "KoukuSaydonCompositionDocument.h"
#include "EffectV2_Target.h"

namespace Client
{
class CKoukuSaydonPresentationPlayer;

struct EFFECT_COMPOSITION_MODEL_ACTOR final
{
    std::string memberId, patternId, displayName, actorProfileId, targetPlacementId;
    std::uint32_t authoredOffsetMs = 0u;
    double effectiveOffsetMs = 0.0;
    std::string status;
};

struct EFFECT_COMPOSITION_MODEL_CLIP final
{
    std::string memberId, patternId, stageId, occurrenceId, runtimeClip, profileId, endPolicy;
    double startMs = 0.0;
    std::uint32_t durationMs = 0u, sourceStartMs = 0u;
    float playRate = 1.f;
    std::string status;
};

// Read-only view of saved Composition model rows. No authoring writer, product
// admission or second model runtime: the existing arena presentation owner is borrowed.
class CEffectCompositionModelPreview final
{
public:
    void Set_Player(CKoukuSaydonPresentationPlayer* player);
    bool Reload();
    bool Select_Pattern(const std::string& patternId);
    bool Select_Bundle(const std::string& bundleId);
    void Clear_Selection();
    bool Begin(std::uint32_t clockMs = 0u, bool paused = true);
    bool Sample(std::uint32_t clockMs, bool paused);
    void Stop();
    bool Is_Active() const;
    bool Resolve_Target(const std::string& memberId,
        EFFECT_V2_TARGET& target, EFFECT_V2_TARGET_VIEW& view) const;

    const KOUKU_SAYDON_COMPOSITION_DOCUMENT& Get_Document() const { return m_Document; }
    const std::vector<EFFECT_COMPOSITION_MODEL_ACTOR>& Actors() const { return m_Actors; }
    const std::vector<EFFECT_COMPOSITION_MODEL_CLIP>& Rows() const { return m_Rows; }
    const std::string& Selected_Id() const { return m_SelectedId; }
    bool Selected_IsBundle() const { return m_IsBundle; }
    std::uint32_t DurationMs() const { return m_DurationMs; }
    const std::string& Status() const { return m_Status; }
    bool Is_Loaded() const { return m_Loaded; }

private:
    void Build_Rows();
    void Append_Actor(const KOUKU_SAYDON_COMPOSITION_PATTERN* pattern,
        const std::string& patternId, const std::string& memberId, std::uint32_t offsetMs);
    CKoukuSaydonPresentationPlayer* m_Player = nullptr;
    KOUKU_SAYDON_COMPOSITION_DOCUMENT m_Document;
    std::vector<EFFECT_COMPOSITION_MODEL_ACTOR> m_Actors;
    std::vector<EFFECT_COMPOSITION_MODEL_CLIP> m_Rows;
    std::string m_SelectedId, m_Status;
    bool m_IsBundle = false, m_Loaded = false;
    std::uint32_t m_DurationMs = 0u;
    std::uint64_t m_PreviewGeneration = 0u;
};
}
