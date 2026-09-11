#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "WorldSequenceDocument.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END
NS_BEGIN(Client)

// Geometry presentation only. The existing WorldSequencePlayer owns every clock and pose.
class CWorldSequenceObject final : public CGameObject
{
public:
    struct DESC : CGameObject::GAMEOBJECT_DESC
    {
        uint32_t levelIndex = ETOUI(LEVEL::END);
        shared_ptr<Engine::CModel> modelPrototype;
        ComPtr<ID3D11ShaderResourceView> diffuseTexture;
    };
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Prototype_GameObject_WorldSequenceObject";
    static constexpr const wchar_t* LAYER_TAG = L"Layer_WorldSequenceObjects";
    static unique_ptr<CWorldSequenceObject> Create(ComPtr<ID3D11Device>, ComPtr<ID3D11DeviceContext>);
    HRESULT Initialize_Prototype() override { return S_OK; }
    HRESULT Initialize(void*) override;
    shared_ptr<CPrototype> Clone(void*) override;
    void Late_Update(f32_t) override;
    HRESULT Render() override;
    bool_t Sample(const float4x4_t& world, bool_t visible,
        const WORLD_SEQUENCE_ANIMATION_TRACK* animation, f32_t localMs, f32_t windowEndMs);
    bool_t Is_Visible() const { return m_Visible; }
    void Hide() { m_Visible = false; }
    const float4x4_t& Get_SampledWorld() const { return m_World; }
    const std::string& Get_RenderStatus() const { return m_RenderStatus; }
private:
    CWorldSequenceObject(ComPtr<ID3D11Device>, ComPtr<ID3D11DeviceContext>);
    shared_ptr<Engine::CModel> m_Model;
    shared_ptr<Engine::CShader> m_Shader;
    ComPtr<ID3D11ShaderResourceView> m_Diffuse;
    float4x4_t m_World{};
    f32_t m_SampleTimeSeconds = 0.f;
    bool_t m_Visible = false;
    std::string m_RenderStatus;
};
NS_END
