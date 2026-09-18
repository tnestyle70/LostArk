#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "WorldSequenceDocument.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END
NS_BEGIN(Client)
class CPart_Equipment;

// Geometry presentation only. The existing WorldSequencePlayer owns every clock and pose.
class CWorldSequenceObject final : public CGameObject
{
public:
    // A product boss part drawn in this body's frame: a static piece on a
    // socket bone, or (empty socketBone) a plate skinned to the body's palette.
    struct PRESENTATION_PART
    {
        wstring_t modelPrototypeTag;
        wstring_t shaderPrototypeTag;
        std::string socketBone;
    };
    struct DESC : CGameObject::GAMEOBJECT_DESC
    {
        uint32_t levelIndex = ETOUI(LEVEL::END);
        shared_ptr<Engine::CModel> modelPrototype;
        ComPtr<ID3D11ShaderResourceView> diffuseTexture;
        std::vector<PRESENTATION_PART> presentationParts;
        std::string materialProfileId;
    };
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Prototype_GameObject_WorldSequenceObject";
    static constexpr const wchar_t* LAYER_TAG = L"Layer_WorldSequenceObjects";
    static unique_ptr<CWorldSequenceObject> Create(ComPtr<ID3D11Device>, ComPtr<ID3D11DeviceContext>);
    HRESULT Initialize_Prototype() override { return S_OK; }
    HRESULT Initialize(void*) override;
    shared_ptr<CPrototype> Clone(void*) override;
    void Late_Update(f32_t) override;
    HRESULT Render() override;
    HRESULT Render_Group(RENDERGROUP group) override;
    bool_t Sample(const float4x4_t& world, bool_t visible,
        const WORLD_SEQUENCE_ANIMATION_TRACK* animation, f32_t localMs, f32_t windowEndMs);
    bool_t Is_Visible() const { return m_Visible; }
    void Hide() { m_Visible = false; }
    // Return to the same rest-pose state as a new clone, without recreating it.
    bool_t Reset_ForReuse();
    const float4x4_t& Get_SampledWorld() const { return m_World; }
    const std::string& Get_RenderStatus() const
    { return m_RenderStatus.empty() ? m_TranslucentRenderStatus : m_RenderStatus; }
#ifdef _DEBUG
    // The visible pose owns this sample; offsets are metres in a normalized bone basis.
    bool_t Try_GetAttachmentWorld(const std::string& bone, float4x4_t& out) const;
#endif
private:
    CWorldSequenceObject(ComPtr<ID3D11Device>, ComPtr<ID3D11DeviceContext>);
    HRESULT Render_Translucent();
    shared_ptr<Engine::CModel> m_Model;
    shared_ptr<Engine::CModel> m_SaydonHatModel;
    shared_ptr<Engine::CShader> m_Shader;
    ComPtr<ID3D11ShaderResourceView> m_Diffuse;
    std::vector<shared_ptr<CPart_Equipment>> m_Parts;
    std::string m_MaterialProfileId;
    float4x4_t m_World{};
    f32_t m_SampleTimeSeconds = 0.f;
    bool_t m_Visible = false;
    bool_t m_HasTranslucentMeshes = false;
    std::string m_RenderStatus;
    std::string m_TranslucentRenderStatus;
};
NS_END
