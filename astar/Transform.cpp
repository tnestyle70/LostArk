#include "Transform.h"
#include "Shader.h"

#include "NAvigation.h"

CTransform::CTransform(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent { pDevice, pContext }
{
}

_float3 CTransform::Get_Scaled() const
{
    return _float3(
        XMVectorGetX(XMVector3Length(Get_State(STATE::RIGHT))), 
        XMVectorGetX(XMVector3Length(Get_State(STATE::UP))),
        XMVectorGetX(XMVector3Length(Get_State(STATE::LOOK)))
    );
}

void CTransform::Set_Scale(_float fScaleX, _float fScaleY, _float fScaleZ)
{
    Set_State(STATE::RIGHT, XMVector3Normalize(Get_State(STATE::RIGHT)) * fScaleX);
    Set_State(STATE::UP, XMVector3Normalize(Get_State(STATE::UP)) * fScaleY);
    Set_State(STATE::LOOK, XMVector3Normalize(Get_State(STATE::LOOK)) * fScaleZ);
}

HRESULT CTransform::Initialize_Prototype()
{
    
    /*D3DXMatrixIdentity(&결과, );*/
    XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());    

    return S_OK;
}

HRESULT CTransform::Initialize(void* pArg)
{
    if (nullptr == pArg)
        return S_OK;

    TRANSFORM_DESC* pDesc = static_cast<TRANSFORM_DESC*>(pArg);

    m_fSpeedPerSec = pDesc->fSpeedPerSec;
    m_fRotationPerSec = pDesc->fRotationPerSec;

    return S_OK;
}

HRESULT CTransform::Bind_ShaderResource(CShader* pShader, const _char* pConstantName)
{
    return pShader->Bind_Matrix(pConstantName, &m_WorldMatrix);    
}

void CTransform::Go_Straight(_float fTimeDelta, CNavigation* pNavigation)
{
    _vector     vPosition = Get_State(STATE::POSITION);
    _vector     vLook = Get_State(STATE::LOOK);

    vPosition += XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

    if(nullptr == pNavigation || 
        true == pNavigation->isMove(vPosition))
        Set_State(STATE::POSITION, vPosition);

}

void CTransform::Go_Astar(_float fTimeDelta, CNavigation* pNavigation, _int iCellIndex)
{
    const list<_vector>*  pRoute = pNavigation->Make_Route(iCellIndex);

    if (nullptr != pRoute)
        m_Route = *pRoute;

    if (true == m_Route.empty())
        return;

    _vector     vGoalPos = m_Route.front();


    if (false == Chase(vGoalPos, fTimeDelta, 0.3f))
    {
        m_Route.pop_front();
    }
}

void CTransform::Go_Left(_float fTimeDelta)
{
    _vector     vPosition = Get_State(STATE::POSITION);
    _vector     vRight = Get_State(STATE::RIGHT);

    vPosition -= XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

    Set_State(STATE::POSITION, vPosition);
}

void CTransform::Go_Right(_float fTimeDelta)
{
    _vector     vPosition = Get_State(STATE::POSITION);
    _vector     vRight = Get_State(STATE::RIGHT);

    vPosition += XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

    Set_State(STATE::POSITION, vPosition);
}

void CTransform::Go_Backward(_float fTimeDelta)
{
    _vector     vPosition = Get_State(STATE::POSITION);
    _vector     vLook = Get_State(STATE::LOOK);

    vPosition -= XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

    Set_State(STATE::POSITION, vPosition);
}

void CTransform::Rotation(_fvector vAxis, _float fRadian)
{
    _float3     vScale = Get_Scaled();

    _vector vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f) * vScale.x;
    _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f) * vScale.y;
    _vector vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f) * vScale.z;

    _matrix RotationMatrix = XMMatrixRotationAxis(vAxis, fRadian);    

    vRight = XMVector3TransformNormal(vRight, RotationMatrix);
    vUp = XMVector3TransformNormal(vUp, RotationMatrix);
    vLook = XMVector3TransformNormal(vLook, RotationMatrix);

    Set_State(STATE::RIGHT, vRight);
    Set_State(STATE::UP, vUp);
    Set_State(STATE::LOOK, vLook);
}

void CTransform::Rotation(_float fRadianX, _float fRadianY, _float fRadianZ)
{
    _float3     vScale = Get_Scaled();

    _vector vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f) * vScale.x;
    _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f) * vScale.y;
    _vector vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f) * vScale.z;

    // _matrix RotationMatrix = XMMatrixRotationAxis(vAxis, fRadian);
    /* 각축에 대한 복합적인 회전을 하나의 복소수로 표현한 데이터.(사원수) */

    
    _vector vQuaternion = XMQuaternionRotationRollPitchYaw(fRadianX, fRadianY, fRadianZ);
    _matrix RotationMatrix = XMMatrixRotationQuaternion(vQuaternion);

    vRight = XMVector3TransformNormal(vRight, RotationMatrix);
    vUp = XMVector3TransformNormal(vUp, RotationMatrix);
    vLook = XMVector3TransformNormal(vLook, RotationMatrix);

    Set_State(STATE::RIGHT, vRight);
    Set_State(STATE::UP, vUp);
    Set_State(STATE::LOOK, vLook);
}

void CTransform::Turn(_fvector vAxis, _float fTimeDelta)
{
    _vector vRight = Get_State(STATE::RIGHT);
    _vector vUp = Get_State(STATE::UP);
    _vector vLook = Get_State(STATE::LOOK);

    _matrix RotationMatrix = XMMatrixRotationAxis(vAxis, m_fRotationPerSec * fTimeDelta);

    vRight = XMVector3TransformNormal(vRight, RotationMatrix);
    vUp = XMVector3TransformNormal(vUp, RotationMatrix);
    vLook = XMVector3TransformNormal(vLook, RotationMatrix);

    Set_State(STATE::RIGHT, vRight);
    Set_State(STATE::UP, vUp);
    Set_State(STATE::LOOK, vLook);
}

void CTransform::LookAt(_fvector vPoint)
{
    _vector     vLook = vPoint - Get_State(STATE::POSITION);
    _vector     vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
    _vector     vUp = XMVector3Cross(vLook, vRight);

    _float3     vScale = Get_Scaled();

    Set_State(STATE::RIGHT, XMVector3Normalize(vRight) * vScale.x);
    Set_State(STATE::UP, XMVector3Normalize(vUp) * vScale.y);
    Set_State(STATE::LOOK, XMVector3Normalize(vLook) * vScale.z);
}

_bool CTransform::Chase(_fvector vPoint, _float fTimeDelta, _float fMinDistance)
{
    _bool       isMove = { false };
    _vector     vPosition = Get_State(STATE::POSITION);

    _vector     vMoveDir = vPoint - vPosition;

    if(fMinDistance <= XMVectorGetX(XMVector3Length(vMoveDir)))
    {
        isMove = true;
        vPosition += XMVector3Normalize(vMoveDir) * m_fSpeedPerSec * fTimeDelta;
    }

    Set_State(STATE::POSITION, XMVectorSetW(vPosition, 1.f));    

    return isMove;
}

CTransform* CTransform::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTransform* pInstance = new CTransform(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CTransform");
        Safe_Release(pInstance);
    }
    return pInstance;
}

CComponent* CTransform::Clone(void* pArg)
{
    return nullptr;
}

void CTransform::Free()
{
    __super::Free();
}
