#pragma once

#include "WorldSequenceDocument.h"
#include "WorldSequencePlayer.h"
#include "PhysicalResourceCatalog.h"
#include "CompositionResourceTree.h"
#include "EffectV2_Catalog.h"
#include "EffectAuthoringResourceTree.h"
#include "CompositionWorkbenchSession.h"

#include <array>
#include <filesystem>
#include <functional>
#include <set>

#ifdef _DEBUG
namespace Client
{
class CLevel_KakulSaydonArena;
struct WORLD_OBJECT_TRAVEL_DRAFT;

class CWorldObjectTool final : public ICompositionWorkbenchSession
{
public:
    ~CWorldObjectTool();
    void Open();
    // Select stable authoring IDs without reloading or saving an existing draft.
    bool Open_ObjectMotion(const std::string& objectId, const std::string& instanceId, std::string& status,
        const std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT>& previewPlacement = {});
    void Deactivate();
    void Update(f32_t seconds, bool_t active);
    void Render();
    void Render_QuickTransformTuning(const std::string& objectId, const std::string& instanceId);
    void Begin_WorkbenchFrame() override;
    void Render_WorkbenchPane(COMPOSITION_WORKBENCH_PANE pane) override;
    void End_WorkbenchFrame() override;
    bool Execute_CompositionEdit(COMPOSITION_EDIT_COMMAND command, std::string& status) override;
    bool Insert_CompositionTransfer(const COMPOSITION_TRANSFER& transfer, std::string& status) override;
    bool Can_AppendCompositionAnimationResource(const COMPOSITION_ANIMATION_RESOURCE& resource,
        bool replace, std::string& status) const override;
    bool Append_CompositionAnimationResource(const COMPOSITION_ANIMATION_RESOURCE& resource,
        bool replace, std::string& status) override;
    void On_WorkbenchDeactivated() override { Deactivate(); }
    bool_t Is_Open() const { return m_Open; }
    bool Consume_InteractionRequest();
    bool Consume_ServerPlayRequest(std::string& patternId, uint32_t& sourceRevision);
    bool Consume_ServerStopRequest() { const bool requested = m_ServerStopRequested; m_ServerStopRequested = false; return requested; }
    void Set_ServerPlayPreparationPending(bool pending) { m_ServerPreparationPending = pending; }
    void Set_ServerPlayStatus(std::string status) { m_ServerPlayStatus = std::move(status); }
    void Set_LinkedSaveCallbacks(std::function<bool(std::string&)> canSave,
        std::function<bool(bool, std::string&)> apply)
    { m_CanSaveLinked = std::move(canSave); m_ApplyLinkedSave = std::move(apply); }
    // Only a saved document may become a Workbench resource inventory.
    const CWorldSequenceDocument* Get_SavedDocument() const { return m_Ready ? &m_SavedDocument : nullptr; }
    /* The in-edit document the tool already previews with. Only the integrated
       authoring preview consumes it; product playback and publish keep the
       saved document above. */
    const CWorldSequenceDocument* Get_AuthoringDraftDocument() const
    { return m_Ready ? &Preview_Document() : nullptr; }
    uint64_t Get_SavedGeneration() const { return m_SavedGeneration; }
    /* publishRuntime=false saves the authoring sources and stops there, so the
       integrated cutscene Save cannot also run the Area publisher or the
       linked battle pattern publish. The default keeps the standalone Save. */
    bool Save_Source(bool publishRuntime = true);
    [[nodiscard]] bool Is_Dirty() const noexcept { return m_Dirty; }

private:
    bool Load_Source();
    COMPOSITION_TRANSFER Capture_CompositionObject(const std::string& objectId,
        const std::string& motionId, std::string& status) const;
    bool Paste_CompositionObject(const COMPOSITION_WORLD_OBJECT_TRANSFER& transfer,
        bool duplicate, std::string& status);
    bool Insert_CompositionEffects(const COMPOSITION_EFFECT_TRANSFER& transfer, std::string& status);
    bool Render_SaveButton();
    void Render_SaveStatus() const;
    void Render_ColliderPreview();
    bool Matches_SourceBaseline();
    void Start_Publish();
    void Poll_Publish();
    void Mark_Dirty();
    std::vector<uint32_t>& Emission_Origins(const WORLD_SEQUENCE_TEMPLATE& sequence);
    void Stop_Preview();
    bool Begin_Preview();
    const CWorldSequenceDocument& Preview_Document() const;
    bool Prepare_PreviewEffects(const CWorldSequenceDocument& document, const std::string& targetId);
    bool Begin_EffectPreview(CWorldSequenceDocument staged, const std::string& instanceId);
    void Play_Preview();
    void Play_AtPlayer();
    std::vector<std::string> Server_ColliderMotionIds() const;
    bool Refresh_ServerPlayPatterns(const std::vector<std::string>& motionIds);
    void Render_ServerPlayControls();
    const WORLD_SEQUENCE_INSTANCE* Preview_Instance() const;
    void Seek(f32_t clockMs);
    f32_t SpanMs() const;
    f32_t PreviewSpanMs() const;
    void Select_Object(const std::string& id);
    void Select_ResourceRow(const std::string& id, const std::vector<std::string>& visibleRows,
        bool control, bool shift);
    std::string Resource_Parent(const std::string& id) const;
    std::string Resource_Anchor(const std::string& id) const;
    std::string Resource_Label(const std::string& id) const;
    std::vector<std::string> Selected_ResourceRoots() const;
    bool Can_MoveResources(const std::vector<std::string>& ids, const std::string& parentId) const;
    bool Move_Resources(const std::vector<std::string>& ids, const std::string& parentId);
    bool Create_ResourceParent(const std::vector<std::string>& ids);
    void Reveal_Resource(const std::string& id);
    void Select_State(const std::string& id);
    std::vector<std::string> StateIds(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource) const;
    bool Create_Object();
    void Create_State();
    bool Build_State(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const std::string& stateName,
        WORLD_SEQUENCE_TEMPLATE& sequence, WORLD_SEQUENCE_INSTANCE& instance);
    void Change_ResourceAnchor(WORLD_SEQUENCE_OBJECT_RESOURCE& resource, const std::string& anchorKind);
    void Render_WindowMenu();
    void Render_Toolbar();
    void Render_Resources(bool fillPane = false);
    void Refresh_AnimationResources();
    void Render_AnimationResources();
    void Render_EffectResources();
    bool Append_SelectedEffect();
    bool Play_SelectedEffect();
    bool Build_SelectedEffectCandidate(CWorldSequenceDocument& staged, std::string& instanceId);
    const WORLD_SEQUENCE_INSTANCE* Effect_TargetInstance() const;
    void Render_EffectRows(WORLD_SEQUENCE_TEMPLATE& sequence);
    void Render_ColliderRows(WORLD_SEQUENCE_TEMPLATE& sequence);
    bool Append_ColliderTrack(WORLD_SEQUENCE_TEMPLATE& sequence);
    bool Duplicate_ColliderTrack(WORLD_SEQUENCE_TEMPLATE& sequence, size_t index);
    bool Append_SelectedAnimation();
    bool Duplicate_TimelineBox(WORLD_SEQUENCE_TEMPLATE& sequence, bool animation, size_t index);
    bool Stage_SelectedModel(CWorldSequenceDocument& candidate);
    bool Assign_SelectedModel();
    void Render_Detail();
    bool Render_TravelEditor(WORLD_SEQUENCE_INSTANCE& instance, WORLD_SEQUENCE_TEMPLATE& sequence);
    void Use_AuthoredMapPreview();
    bool Render_MapAnchor(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource);
    void Render_ObjectDetail(WORLD_SEQUENCE_OBJECT_RESOURCE& resource);
    const WORLD_SEQUENCE_OBJECT_RESOURCE* Preview_Group() const;
    void Render_GroupDetail(WORLD_SEQUENCE_OBJECT_RESOURCE& resource);
    void Render_GroupSequence(const WORLD_SEQUENCE_OBJECT_RESOURCE& resource, bool parentOverview = false);
    void Render_SelectedSequence();
    void Render_Sequence(WORLD_SEQUENCE_TEMPLATE& sequence);
    bool Animation_EndMs(const WORLD_SEQUENCE_TEMPLATE& sequence, bool preserveWindows, uint32_t& outEnd);
    bool Resize_Stage(WORLD_SEQUENCE_TEMPLATE& sequence, uint32_t durationMs);
    void Render_KeyEditor(WORLD_SEQUENCE_TEMPLATE& sequence);
    void Render_PhysicalResources();
    void Rebuild_PhysicalTree();

    int m_ColliderPreviewFrame = -1;
    bool m_Open = false;
    bool m_ResourcesOpen = true;
    bool m_SequencerOpen = true;
    bool m_DetailOpen = true;
    bool m_ResetLayoutRequested = false;
    bool m_InteractionRequested = false;
    bool m_CompositionResourceFocused = false;
    bool m_NativeResourceSelected = false;
    std::optional<COMPOSITION_EDIT_COMMAND> m_PendingCompositionEdit;
    COMPOSITION_TRANSFER m_PendingCompositionTransfer;
    bool m_PreviewAtCharacter = true;
    std::string m_ServerPlayScope, m_ServerPatternId, m_ServerPlayStatus, m_PendingServerPatternId;
    std::vector<std::pair<std::string, std::string>> m_ServerPatterns;
    uint32_t m_ServerSourceRevision = 0u, m_PendingServerSourceRevision = 0u;
    bool m_ServerStopRequested = false, m_ServerPreparationPending = false;
    // Read-only context from a Composition box, valid only while this Object is selected.
    std::optional<CWorldSequencePlayer::OBJECT_PLACEMENT> m_CompositionPreviewPlacement;
    std::string m_CompositionPreviewObjectId;
    bool m_Ready = false;
    bool m_Dirty = false;
    bool m_PreviewActive = false;
    bool m_PreviewDirty = false;
    bool m_Playing = false;
    CLevel_KakulSaydonArena* m_PreviewLevel = nullptr;
    f32_t m_ClockMs = 0.f;
    f32_t m_VerticalArcHeight = 2.f;
    std::shared_ptr<WORLD_OBJECT_TRAVEL_DRAFT> m_TravelDraft;
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
    // Temporary Play Effect candidate; Append alone commits it to authoring.
    std::optional<CWorldSequenceDocument> m_EffectPreviewDocument;
    std::optional<CWorldSequenceDocument> m_PendingEffectPreviewDocument;
    std::string m_PendingEffectInstance;
    std::vector<std::string> m_PreviewPreparationTargets;
    bool m_PreviewPreparationPending = false;
    bool m_PlayAfterPreparation = false;
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
    // Organizational selection never changes Motion bindings or model transforms.
    std::set<std::string> m_SelectedResources;
    std::string m_ResourceSelectionAnchor;
    std::string m_SelectedFolder;
    std::string m_ResourceAnchorKind = "WORLD";
    std::set<std::string> m_RevealResourceParents;
    std::vector<std::string> m_ParentEditSelection;
    std::string m_MoveResourceParent;
    std::string m_NewObjectParent;
    std::array<char, 128> m_NewParentName{};
    // Preview scope stays on the combined resource while the editor selects a member.
    std::string m_SelectedGroup;
    std::string m_SelectedInstance;
    size_t m_SelectedTrack = 0;
    size_t m_SelectedAnimationRow = 0;
    int m_SelectedBoxKind = 0; // 0 Transform, 1 Animation, 2 Effect, 3 Collider, 4 Stage.
    bool m_TimelineFitRequested = false;
    std::string m_StageResizeSequence;
    uint32_t m_StageResizeOriginalMs = 0, m_StageResizeDurationMs = 0;
    float m_StageResizePixelsPerMs = 1.f;
    size_t m_SelectedColliderRow = 0;
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
    std::vector<CEffectAuthoringResourceTree::RESOURCE> m_AuthoredEffectResources;
    bool m_SelectedEffectAuthored = false;
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
