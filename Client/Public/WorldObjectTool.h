#pragma once

#include "WorldSequenceDocument.h"
#include "PhysicalResourceCatalog.h"
#include "CompositionResourceTree.h"
#include "EffectV2_Catalog.h"

#include <array>
#include <filesystem>
#include <functional>
#include <set>

#ifdef _DEBUG
namespace Client
{
class CLevel_KakulSaydonArena;

class CWorldObjectTool final
{
public:
    ~CWorldObjectTool();
    void Open();
    // Select stable authoring IDs without reloading or saving an existing draft.
    bool Open_ObjectMotion(const std::string& objectId, const std::string& instanceId, std::string& status);
    void Deactivate();
    void Update(f32_t seconds, bool_t active);
    void Render();
    bool_t Is_Open() const { return m_Open; }
    bool Consume_InteractionRequest();
    void Set_LinkedSaveCallbacks(std::function<bool(std::string&)> canSave,
        std::function<bool(bool, std::string&)> apply)
    { m_CanSaveLinked = std::move(canSave); m_ApplyLinkedSave = std::move(apply); }
    // Only a saved document may become a Workbench resource inventory.
    const CWorldSequenceDocument* Get_SavedDocument() const { return m_Ready ? &m_SavedDocument : nullptr; }
    uint64_t Get_SavedGeneration() const { return m_SavedGeneration; }

private:
    bool Load_Source();
    bool Save_Source();
    bool Matches_SourceBaseline();
    void Start_Publish();
    void Poll_Publish();
    void Mark_Dirty();
    std::vector<uint32_t>& Emission_Origins(const WORLD_SEQUENCE_TEMPLATE& sequence);
    void Stop_Preview();
    bool Begin_Preview();
    const WORLD_SEQUENCE_INSTANCE* Preview_Instance() const;
    void Seek(f32_t clockMs);
    f32_t SpanMs() const;
    f32_t PreviewSpanMs() const;
    void Select_Object(const std::string& id);
    void Select_State(const std::string& id);
    std::vector<std::string> StateIds(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource) const;
    bool Create_Object();
    void Create_State();
    bool Build_State(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const std::string& stateName,
        WORLD_SEQUENCE_TEMPLATE& sequence, WORLD_SEQUENCE_INSTANCE& instance);
    void Change_ResourceAnchor(WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const std::string& anchorKind);
    void Render_WindowMenu();
    void Render_Toolbar();
    void Render_Resources();
    void Refresh_AnimationResources();
    void Render_AnimationResources();
    void Render_EffectResources();
    bool Append_SelectedEffect();
    void Render_EffectRows(WORLD_SEQUENCE_TEMPLATE& sequence);
    bool Append_SelectedAnimation();
    bool Stage_SelectedModel(CWorldSequenceDocument& candidate);
    bool Assign_SelectedModel();
    void Render_Detail();
    void Render_ObjectDetail(WORLD_SEQUENCE_OBJECT_RESOURCE& resource);
    const WORLD_SEQUENCE_OBJECT_RESOURCE* Preview_Group() const;
    void Render_GroupDetail(WORLD_SEQUENCE_OBJECT_RESOURCE& resource);
    void Render_GroupSequence(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource);
    void Render_Sequence(WORLD_SEQUENCE_TEMPLATE& sequence);
    void Render_KeyEditor(WORLD_SEQUENCE_TEMPLATE& sequence);
    void Render_PhysicalResources();
    void Rebuild_PhysicalTree();

    bool m_Open = false;
    bool m_ResourcesOpen = true;
    bool m_SequencerOpen = true;
    bool m_DetailOpen = true;
    bool m_ResetLayoutRequested = false;
    bool m_InteractionRequested = false;
    bool m_PreviewAtCharacter = true;
    bool m_Ready = false;
    bool m_Dirty = false;
    bool m_PreviewActive = false;
    bool m_PreviewDirty = false;
    bool m_Playing = false;
    CLevel_KakulSaydonArena* m_PreviewLevel = nullptr;
    f32_t m_ClockMs = 0.f;
    f32_t m_VerticalArcHeight = 2.f;
    // Distribute on Ring preset inputs; rows are the saved truth, not these.
    int m_GroupCount = 1;
    float3_t m_EmissionStep{1.f, 0.f, 0.f};
    uint32_t m_EmissionDelayStepMs = 0u;
    float3_t m_SpacingMultiplier{1.f, 1.f, 1.f};
    int m_RingCount = 10;
    f32_t m_RingStartDegrees = 0.f;
    f32_t m_Zoom = 100.f;
    CWorldSequenceDocument m_Document;
    CWorldSequenceDocument m_SavedDocument;
    std::map<std::string, std::vector<uint32_t>> m_EmissionOrigins;
    std::set<std::string> m_EditedMotionIds;
    std::function<bool(std::string&)> m_CanSaveLinked;
    std::function<bool(bool, std::string&)> m_ApplyLinkedSave;
    bool m_LinkedSavePending = false;
    bool m_PublishLinkedPatterns = false;
    WORLD_SEQUENCE_PLACEMENT_MAP m_MapTargets;
    WORLD_SEQUENCE_DEPLOY_MAP m_DeployTargets;
    std::filesystem::path m_SourcePath;
    std::filesystem::path m_PlacementPath;
    std::filesystem::path m_DeployPath;
    std::string m_SourceBytes;
    std::string m_PlacementBytes;
    std::string m_DeployBytes;
    std::string m_Status;
    std::string m_PreviewStatus;
    std::string m_SelectedObject;
    // Preview scope stays on the combined resource while the editor selects a member.
    std::string m_SelectedGroup;
    std::string m_SelectedInstance;
    size_t m_SelectedTrack = 0;
    size_t m_SelectedKey = 0;
    uint64_t m_SavedGeneration = 0;
    std::array<char, 128> m_NewObjectName{};
    int m_NewObjectAnchor = 0;
    bool m_CreateObjectFailed = false;
    std::array<char, 128> m_NewStateName{};
    std::string m_PristinePatternId;
    struct ANIMATION_RESOURCE
    {
        std::string clipName;
        double durationMs = 0.;
    };
    std::vector<ANIMATION_RESOURCE> m_AnimationResources;
    std::string m_AnimationObjectId;
    std::string m_AnimationModelAssetId;
    std::string m_AnimationCandidateModelAssetId;
    std::string m_AnimationCandidateObjectId;
    std::string m_MaterialSourceObjectId;
    std::string m_MaterialSourceCandidate;
    std::string m_MaterialSourceStatus;
    bool m_AnimationCatalogReady = false;
    std::string m_AnimationResourceStatus;
    std::string m_SelectedAnimationClip;
    std::array<char, 256> m_AnimationSearch{};
    std::vector<EFFECT_V2_RESOURCE_SUMMARY> m_EffectResources;
    std::string m_SelectedEffectResource;
    EFFECT_V2_RESOURCE_KIND m_SelectedEffectKind = EFFECT_V2_RESOURCE_KIND::GROUP;
    std::string m_EffectResourceStatus;
    bool m_EffectInventoryLoaded = false;
    size_t m_SelectedEffectRow = 0;
    std::array<char, 256> m_EffectSearch{};
    std::array<char, 256> m_ObjectSearch{};
    std::array<char, 256> m_PhysicalSearch{};
    std::vector<PHYSICAL_RESOURCE_ASSET> m_PhysicalAssets;
    COMPOSITION_RESOURCE_TREE_NODE m_PhysicalTree;
    std::string m_PhysicalStatus;
    std::string m_SelectedPhysical;
    bool m_PhysicalScanned = false;
    bool m_PhysicalScanRunning = false;
    CPhysicalResourceScan m_PhysicalScan;
    int m_PhysicalSlot = 0;
    HANDLE m_PublishProcess = nullptr;
    std::filesystem::path m_PublishLog;
};
}
#endif
