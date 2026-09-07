#include "WorldSequenceObject.h"
#include "DeferredMaterialRenderUtils.h"
#include "MapAssetRenderUtils.h"
#include "GameInstance.h"
#include "Model.h"
#include "Shader.h"
#include <algorithm>
#include <cmath>

using namespace Client;
using namespace Engine;

CWorldSequenceObject::CWorldSequenceObject(ComPtr<ID3D11Device> device,
    ComPtr<ID3D11DeviceContext> context) : CGameObject(device, context) {}

unique_ptr<CWorldSequenceObject> CWorldSequenceObject::Create(
    ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> context)
{
    return unique_ptr<CWorldSequenceObject>(new CWorldSequenceObject(device, context));
}

HRESULT CWorldSequenceObject::Initialize(void* argument)
{
    if (!argument) return E_INVALIDARG;
    const auto& desc = *static_cast<const DESC*>(argument);
    if (!desc.modelPrototype || FAILED(__super::Initialize(argument))) return E_FAIL;
    m_Model = dynamic_pointer_cast<CModel>(desc.modelPrototype->Clone(nullptr));
    m_Diffuse = desc.diffuseTexture;
    if (!m_Model || FAILED(__super::Add_Component(desc.levelIndex,
        m_Model->Is_Skinned() ? L"Prototype_Component_Shader_VtxAnimMeshBinary" :
        L"Prototype_Component_Shader_VtxMeshBinary", L"Com_Shader", m_Shader))) return E_FAIL;
    // A newly authored skinned resource may have no animation track yet.
    // Its cloned rest pose still needs the same combined matrices as a sampled clip.
    if (m_Model->Is_Skinned()) m_Model->Refresh_BoneCombinedMatrices();
    XMStoreFloat4x4(&m_World, XMMatrixIdentity());
    return S_OK;
}

shared_ptr<CPrototype> CWorldSequenceObject::Clone(void* argument)
{
    auto object = shared_ptr<CWorldSequenceObject>(new CWorldSequenceObject(*this));
    return SUCCEEDED(object->Initialize(argument)) ? object : nullptr;
}

bool_t CWorldSequenceObject::Sample(const float4x4_t& world, const bool_t visible,
    const WORLD_SEQUENCE_ANIMATION_TRACK* animation, const f32_t localMs, const f32_t windowEndMs)
{
    for (const auto& row : world.m)
        for (const float component : row)
            if (!std::isfinite(component)) return false;
    if (animation)
    {
        uint32_t index = UINT32_MAX;
        for (uint32_t i = 0; i < m_Model->Get_NumAnimations(); ++i)
            if (animation->clipName == m_Model->Get_AnimationName(i)) { index = i; break; }
        f32_t position = 0.f, duration = 0.f;
        if (index == UINT32_MAX || !m_Model->Get_AnimationProgress(index, position, duration) || duration <= 0.f)
            return false;
        const f32_t ticksPerSecond = m_Model->Get_AnimationTickPerSecond(index);
        f32_t ticks = (std::max)(0.f, localMs - animation->startMs) * 0.001f *
            animation->playbackRate * ticksPerSecond;
        if (localMs >= windowEndMs && animation->holdLastFrame) ticks = duration;
        else if (animation->loop) ticks = std::fmod(ticks, duration);
        else if (ticks > duration) ticks = animation->holdLastFrame ? duration : 0.f;
        m_Model->Set_Animation(index, false);
        m_Model->Skip_Blend();
        if (!m_Model->Set_AnimTrackPosition(index, ticks)) return false;
        m_Model->Play_Animation(0.f);
    }
    m_World = world;
    m_Visible = visible;
    return true;
}

void CWorldSequenceObject::Late_Update(f32_t)
{
    if (m_Visible) CGameInstance::Get().Add_RenderObject(RENDERGROUP::NONBLEND,
        static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT CWorldSequenceObject::Render()
{
    if (!m_Visible) return S_OK;
    const auto failed = [this](const std::string& stage)
    { m_RenderStatus = "World Object render failed: " + stage; return E_FAIL; };
    if (FAILED(m_Shader->Bind_Matrix("g_WorldMatrix", &m_World)) ||
        FAILED(CGameInstance::Get().Bind_Transform(m_Shader, "g_ViewMatrix", D3DTS::VIEW)) ||
        FAILED(CGameInstance::Get().Bind_Transform(m_Shader, "g_ProjMatrix", D3DTS::PROJ))) return failed("world/view/projection binding");
    const bool_t animated = m_Model->Is_Skinned();
    MAP_ASSET_RENDER_PROFILE profile;
    profile.cullMode = MAP_ASSET_CULL_MODE::TWO_SIDED;
    if (!animated)
    {
        matrix_t basis = XMLoadFloat4x4(&m_World);
        basis.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);
        float4x4_t normal;
        XMStoreFloat4x4(&normal, XMMatrixTranspose(XMMatrixInverse(nullptr, basis)));
        if (FAILED(m_Shader->Bind_Matrix("g_WorldInvTransposeMatrix", &normal))) return failed("normal matrix binding");
    }
    for (uint32_t mesh = 0; mesh < m_Model->Get_NumMeshes(); ++mesh)
    {
        const HRESULT material = animated ? Bind_DeferredMaterialInputs(*m_Model, m_Shader, mesh, {}, nullptr, m_Diffuse) :
            CMapAssetRenderUtils::Bind_Material(m_Model, m_Shader, mesh, profile, 0.f, m_Diffuse);
        const auto meshLabel = " (mesh " + std::to_string(mesh) + ")";
        if (FAILED(material)) return failed("material binding" + meshLabel);
        if (animated && FAILED(m_Model->Bind_BoneMatrices(m_Shader, "g_BoneMatrices", mesh)))
            return failed("bone matrix binding" + meshLabel);
        if (FAILED(m_Shader->Begin(animated ? 0u : CMapAssetRenderUtils::Select_Pass(profile, false))))
            return failed("shader pass" + meshLabel);
        if (FAILED(m_Model->Render(mesh))) return failed("mesh submission" + meshLabel);
    }
    m_RenderStatus.clear();
    return S_OK;
}
