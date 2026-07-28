#include "Transform.h"
#include "GameInstance.h"

CTransform::CTransform(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : CComponent { pDevice, pContext }
{
}

CTransform::~CTransform()
{
}

const float3_t CTransform::Get_Scaled()
{
    return float3_t(
        XMVectorGetX(XMVector3Length(Get_State(STATE::RIGHT))), 
        XMVectorGetX(XMVector3Length(Get_State(STATE::UP))),
        XMVectorGetX(XMVector3Length(Get_State(STATE::LOOK)))
    );
}

HRESULT CTransform::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CTransform::Initialize(void* pArg)
{
    if (nullptr != pArg)
    {
        auto    pDesc = static_cast<CTransform::TRANSFORM_DESC*>(pArg);

        m_fSpeedPerSec = pDesc->fSpeedPerSec;
        m_fRotationPerSec = pDesc->fRotationPerSec;
    }

    XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

    return S_OK;
}

HRESULT CTransform::Bind_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName)
{
    return pShader->Bind_Matrix(pConstantName, &m_WorldMatrix);    
}

void CTransform::Scale(f32_t fScaleX, f32_t fScaleY, f32_t fScaleZ)
{
    Set_State(STATE::RIGHT, XMVector3Normalize(Get_State(STATE::RIGHT)) * fScaleX);
    Set_State(STATE::UP, XMVector3Normalize(Get_State(STATE::UP)) * fScaleY);
    Set_State(STATE::LOOK, XMVector3Normalize(Get_State(STATE::LOOK)) * fScaleZ);
}

void CTransform::Scaling(f32_t fScaleX, f32_t fScaleY, f32_t fScaleZ)
{
    Set_State(STATE::RIGHT, Get_State(STATE::RIGHT) * fScaleX);
    Set_State(STATE::UP, Get_State(STATE::UP) * fScaleY);
    Set_State(STATE::LOOK, Get_State(STATE::LOOK) * fScaleZ);
}

void CTransform::Go_Straight(f32_t fTimeDelta, shared_ptr<class CNavigation> pNavigation)
{
    vector_t  vPosition = Get_State(STATE::POSITION);
    vector_t  vLook = Get_State(STATE::LOOK);

    vPosition += XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

    if(nullptr == pNavigation || 
        true == pNavigation->isMove(vPosition))
        Set_State(STATE::POSITION, vPosition);
}

void CTransform::Go_Backward(f32_t fTimeDelta)
{
    vector_t  vPosition = Get_State(STATE::POSITION);
    vector_t  vLook = Get_State(STATE::LOOK);

    vPosition -= XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

    Set_State(STATE::POSITION, vPosition);
}


void CTransform::Go_Left(f32_t fTimeDelta)
{
    vector_t  vPosition = Get_State(STATE::POSITION);
    vector_t  vRight = Get_State(STATE::RIGHT);

    vPosition -= XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

    Set_State(STATE::POSITION, vPosition);
}

void CTransform::Go_Right(f32_t fTimeDelta)
{
    vector_t  vPosition = Get_State(STATE::POSITION);
    vector_t  vRight = Get_State(STATE::RIGHT);

    vPosition += XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

    Set_State(STATE::POSITION, vPosition);
}

void CTransform::Rotation(fvector_t vAxis, f32_t fDegree)
{
    float3_t        vScale = Get_Scaled();
    vector_t        vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f) * vScale.x;
    vector_t        vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f) * vScale.y;
    vector_t        vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f) * vScale.z;

    matrix_t        RotationMatrix = XMMatrixRotationAxis(vAxis, XMConvertToRadians(fDegree));

    Set_State(STATE::RIGHT, XMVector3TransformNormal(vRight, RotationMatrix));
    Set_State(STATE::UP, XMVector3TransformNormal(vUp, RotationMatrix));
    Set_State(STATE::LOOK, XMVector3TransformNormal(vLook, RotationMatrix));
}

void CTransform::Rotation(f32_t fDegreeX, f32_t fDegreeY, f32_t fDegreeZ)
{
    float3_t        vScale = Get_Scaled();
    vector_t        vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f) * vScale.x;
    vector_t        vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f) * vScale.y;
    vector_t        vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f) * vScale.z;

    vector_t        vQuaternion = XMQuaternionRotationRollPitchYaw(
        XMConvertToRadians(fDegreeX),
        XMConvertToRadians(fDegreeY),
        XMConvertToRadians(fDegreeZ));

    matrix_t        RotationMatrix = XMMatrixRotationQuaternion(vQuaternion);

    Set_State(STATE::RIGHT, XMVector3TransformNormal(vRight, RotationMatrix));
    Set_State(STATE::UP, XMVector3TransformNormal(vUp, RotationMatrix));
    Set_State(STATE::LOOK, XMVector3TransformNormal(vLook, RotationMatrix));

}

void CTransform::Turn(fvector_t vAxis, f32_t fTimeDelta)
{
    vector_t        vRight = Get_State(STATE::RIGHT);
    vector_t        vUp = Get_State(STATE::UP);
    vector_t        vLook = Get_State(STATE::LOOK);

    matrix_t        RotationMatrix = XMMatrixRotationAxis(vAxis, XMConvertToRadians(m_fRotationPerSec) * fTimeDelta);

    Set_State(STATE::RIGHT, XMVector3TransformNormal(vRight, RotationMatrix));
    Set_State(STATE::UP, XMVector3TransformNormal(vUp, RotationMatrix));
    Set_State(STATE::LOOK, XMVector3TransformNormal(vLook, RotationMatrix));
}

void CTransform::LookAt(fvector_t vAt)
{
    vector_t vLook = XMVectorSetW(vAt, 1.f) - Get_State(STATE::POSITION);
    vector_t vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
    vector_t vUp = XMVector3Cross(vLook, vRight);

    float3_t        vScale = Get_Scaled();

    Set_State(STATE::RIGHT, XMVector3Normalize(vRight) * vScale.x);
    Set_State(STATE::UP, XMVector3Normalize(vUp) * vScale.y);
    Set_State(STATE::LOOK, XMVector3Normalize(vLook) * vScale.z);
}

void CTransform::Chase(fvector_t vGoal, f32_t fTimeDelta, f32_t fLimitDistance)
{
    vector_t        vPosition = Get_State(STATE::POSITION);
    vector_t        vMoveDir = vGoal - vPosition;

    f32_t fDistance = XMVectorGetX(XMVector3Length(vMoveDir));

    if(fDistance >= fLimitDistance)
        vPosition += XMVector3Normalize(vMoveDir) * m_fSpeedPerSec * fTimeDelta;    

    Set_State(STATE::POSITION, vPosition);
}

shared_ptr<CTransform> CTransform::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
{
    auto pInstance = shared_ptr<CTransform>(new CTransform(pDevice, pContext));

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CTransform");
        return nullptr;
    }

    return pInstance;
}


shared_ptr<CPrototype> CTransform::Clone(void* pArg)
{
    return nullptr;
}
