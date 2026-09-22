#pragma once
#include "BoneAnimationDocument.h"
#include <memory>
#include <string>

namespace Client
{
class CBoneAnimationWorkbench final
{
public:
    bool Select(const std::string& asset, const std::shared_ptr<Engine::CModel>& model);
    void Render();
    void Update(float deltaSeconds);
    void Stop();
    bool Is_Active() const { return m_Override; }
    bool Is_Dirty() const { return m_Dirty || m_PoseDirty; }
    bool Consume_Installed() { const bool installed = m_Installed; m_Installed = false; return installed; }
    bool Consume_PreviewRequest() { const bool request = m_PreviewRequested; m_PreviewRequested = false; return request; }
    const std::string& Status() const { return m_Status; }
private:
    BONE_ANIMATION_CLIP* Selected();
    void Select_Bone(const std::string& name);
    void Read_Key();
    void Remember();
    bool Commit(CBoneAnimationDocument candidate);
    void Restore_History(bool redo);
    void Render_Source();
    void Render_Keys();
    void Render_Viewport();
    void Apply_Pose(bool requestOwnership = true);
    CBoneAnimationDocument m_Document;
    std::weak_ptr<Engine::CModel> m_Model;
    std::string m_Selected, m_Bone, m_Source, m_Status;
    char m_NewName[128] = "authored.flight";
    char m_BoneSearch[128]{};
    float m_Clock = 0.f;
    int m_SourceIn = 0, m_SourceOut = 1000, m_HoldDuration = 2000;
    float m_SourceRate = 1.f;
    float3_t m_KeyPosition{}, m_KeyDegrees{}, m_KeyScale{1.f, 1.f, 1.f};
    std::vector<CBoneAnimationDocument> m_Undo, m_Redo;
    bool m_PoseDirty = false, m_ShowSkeleton = false, m_PreviewRequested = false;
    bool m_Dirty = false, m_Playing = false, m_Override = false, m_Loop = true, m_Installed = false;
};
}
