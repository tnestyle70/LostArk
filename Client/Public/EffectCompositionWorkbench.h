#pragma once

#include "EffectEditingSession.h"
#include "EffectCompositionModelPreview.h"
#include "EffectV2_Catalog.h"
#include "EffectV2_Runtime.h"
#include "EffectCompositionWorldResource.h"
#include <map>
#include <set>

namespace Client
{
class CEffect_Tool_V2;
class CEffectCompositionWorkbench final
{
public:
    CEffectCompositionWorkbench(ComPtr<ID3D11Device> device,
        ComPtr<ID3D11DeviceContext> context, CEffect_Tool_V2& resourceEditor);
    ~CEffectCompositionWorkbench();
    void Open() { m_Open = true; }
    bool Is_Open() const { return m_Open; }
    void Render();
    void Update(float dt, bool active);
    void Deactivate();
    void Set_Player(CKoukuSaydonPresentationPlayer* player) { m_Model.Set_Player(player); }
    bool Consume_InteractionRequest() { const bool value = m_Interaction; m_Interaction = false; return value; }
private:
    void Refresh_Inventory();
    void Load_WorldObjects();
    void Render_Toolbar();
    void Render_Resources();
    void Render_Detail();
    void Render_ModelResources();
    void Render_Timeline();
    bool Play(bool reset);
    void Stop();
    void Sample(bool seek);
    bool Resolve_Anchor(float4x4_t& pivot);
    bool Sample_Anchor(float seconds, float4x4_t& pivot, std::string& error) const;
    void Record_Anchor();
    void Reset_AnchorHistory();
    void Edited();
    void Capture_Context();
    void Restore_Context();
    uint32_t DurationMs() const;
    ComPtr<ID3D11Device> m_Device;
    ComPtr<ID3D11DeviceContext> m_Context;
    CEffect_Tool_V2& m_ResourceEditor;
    CEffectEditingSession m_Edit;
    CEffectCompositionModelPreview m_Model;
    std::vector<EFFECT_V2_RESOURCE_SUMMARY> m_Inventory;
    std::vector<EFFECT_COMPOSITION_WORLD_RESOURCE> m_WorldObjects;
    uint32_t m_WorldRevision = 0;
    std::string m_Status, m_WorldStatus, m_AnchorMember, m_InventorySelection;
    std::string m_DragChild;
    std::set<std::string> m_Muted;
    std::string m_Solo;
    std::shared_ptr<EFFECT_V2_PIVOT_HISTORY> m_AnchorHistory = std::make_shared<EFFECT_V2_PIVOT_HISTORY>();
    float m_LastRecordedSeconds = -1.f;
    float4x4_t m_LastRecordedPivot{};
    float4x4_t m_WorldPivot{};
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> m_PlaySnapshot;
    uint32_t m_Handle = 0, m_DragStart = 0, m_DragDuration = 0;
    int m_DragKind = 0;
    float m_DragX = 0.f;
    double m_ClockMs = 0.0;
    float m_Zoom = 80.f;
    int m_CreateType = 2;
    int m_AnchorKind = 0;
    char m_Name[256] = {}, m_Filter[192] = {};
    bool m_Open = false, m_Interaction = false, m_InventoryLoaded = false, m_WorldLoaded = false;
    bool m_Playing = false, m_Paused = false, m_Loop = false, m_PreviewDirty = false;
};
}
