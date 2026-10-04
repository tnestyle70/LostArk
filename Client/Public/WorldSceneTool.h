#pragma once

#include "Client_Defines.h"

#ifdef _DEBUG
#include "DeployPropRuntime.h"
#include "MapPlacementEditSession.h"
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <vector>

NS_BEGIN(Client)

class IMapAuthoringHost;

// Independent F1 presentation/authoring view of the current Level's live models.
// Level containers and the existing model/animation runtimes remain their owners.
class CWorldSceneTool final
{
public:
    CWorldSceneTool();
    ~CWorldSceneTool();
    void Open();
    void Hide();
    bool Is_Open() const { return m_bOpen; }
    void Update(float deltaSeconds, bool visible);
    void Render();
    bool Consume_PickRequest();
    bool Consume_InteractionRequest();
    bool Consume_FocusRequest(float3_t& position, float& radius);
    void Set_Status(std::string status) { m_Status = std::move(status); }
    void Complete_MapPick(MAP_WORLD_MESH_PICK selected);
    void Complete_DeployPick(DEPLOY_WORLD_MESH_PICK selected);
    bool Try_GetSelectedHit(float3_t& position) const;
    bool Is_Publishing() const { return m_Edit.Is_Publishing(); }
    const std::string& Get_AreaId() const { return m_AreaId; }

private:
    IMapAuthoringHost* Host() const;
    void Refresh_Rows();
    void Select_Map(uint64_t placementId);
    void Select_Deploy(uint64_t placementId);
    void Render_Rows();
    void Render_MapDetails();
    bool Begin_Editing();
    void Render_AnimationControls();
    void Render_DeployDetails();
    void Update_AnimationPreview(float deltaSeconds);
    void Stop_AnimationPreview();
    void Stop_SelfMotionPreview();
    bool Begin_DeployPreview();
    bool Begin_SelfMotionPreview();

    struct ROW final { uint64_t id = 0; bool deploy = false; std::string label, search; };
    bool m_bOpen = false, m_bPickRequested = false, m_bInteraction = false;
    bool m_bFocusRequested = false;
    float3_t m_FocusPosition{};
    float m_FocusRadius = 8.f;
    uint32_t m_LevelIndex = UINT32_MAX;
    uint64_t m_RuntimeGeneration = UINT64_MAX;
    std::string m_AreaId, m_Status;
    std::array<char, 256> m_Search{};
    std::vector<ROW> m_Rows;
    std::vector<size_t> m_FilteredRows;
    bool m_bRowsDirty = true;
    bool m_bDeploySelected = false;
    std::optional<MAP_WORLD_MESH_PICK> m_MapSelection;
    std::optional<DEPLOY_WORLD_MESH_PICK> m_DeploySelection;
    CMapPlacementEditSession m_Edit;
    std::optional<MAP_PLACEMENT_RECORD> m_SelectedOriginal;
    std::vector<MAP_PLACEMENT_RECORD> m_Undo;
    std::optional<MAP_PLACEMENT_RECORD> m_PendingUndo;
    bool m_bUndoGestureSaved = false;

    // Animation methods live in WorldSceneTool_Animation.cpp. Only this tool's
    // Begin success grants it permission to Sample/End the borrowed preview.
    std::weak_ptr<CDeployPropObject> m_AnimationObject;
    uint64_t m_AnimationPlacementId = 0;
    std::string m_AnimationClip;
    bool m_bAnimationOwned = false, m_bAnimationPlaying = false, m_bAnimationLoop = true;
    float m_AnimationTimeSeconds = 0.f, m_AnimationDuration = 0.f, m_AnimationRate = 1.f;
    float3_t m_DeployPreviewPosition{};
    float3_t m_DeployPreviewRotationDegrees{};
    float m_DeployPreviewOpacity = 1.f;
    bool m_bRevealHidden = false;
    float4_t m_DeployPreviewBaseRotation = float4_t(0.f, 0.f, 0.f, 1.f);
    bool m_bDeployPoseDirty = false, m_bAnimationHasClip = false;
    float m_DeployPreviewUniformScale = 1.f;
    std::string m_AnimationAreaId;
    uint32_t m_AnimationLevelIndex = UINT32_MAX;

    bool m_bSelfMotionOwned = false, m_bSelfMotionPlaying = false;
    float m_SelfMotionTime = 0.f, m_SelfMotionRate = 1.f;
    float m_SavedSelfMotionTime = 0.f, m_SavedSelfMotionRate = 1.f;
    bool m_bSavedSelfMotionPaused = false;
    std::string m_SelfMotionAreaId;
    uint32_t m_SelfMotionLevelIndex = UINT32_MAX;
    uint64_t m_SelfMotionGeneration = UINT64_MAX;
    IMapAuthoringHost* m_pSelfMotionHost = nullptr; // Equality guard only; never dereferenced.
};

NS_END
#endif
