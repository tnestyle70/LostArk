// 객체별 행렬·속도 상태와 이동·회전·shader 전달 인터페이스를 구현한다.
#include "Transform.h"
// 전진 후보의 이동 가능 여부와 현재 행렬의 shader 바인딩을 직접 호출한다.
#include "Navigation.h"
#include "Shader.h"

NS_BEGIN(Engine)

CTransform::CTransform(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
    : CComponent { pDevice, pContext }
{
}

CTransform::~CTransform()
{
}

// 세 기저 행의 길이를 반환한다. 음수 scale의 부호나 shear 전체를 복원하는 분해 함수는 아니다.
const float3_t CTransform::Get_Scaled()
{
    return float3_t(
        XMVectorGetX(XMVector3Length(Get_State(STATE::RIGHT))),
        XMVectorGetX(XMVector3Length(Get_State(STATE::UP))),
        XMVectorGetX(XMVector3Length(Get_State(STATE::LOOK)))
    );
}

// Create가 호출하는 기본 준비 경로다. 실제 단위 행렬과 이동 설정은 Initialize에서 만든다.
HRESULT CTransform::Initialize_Prototype()
{
    return S_OK;
}

// GameObject가 Transform을 직접 만든 뒤 호출한다. descriptor가 있으면 속도를 복사하고 행렬은 항상 초기화한다.
// pArg가 없으면 현재 속도를 유지한다. 원형을 Clone하는 경로는 아래 Clone에서 지원하지 않는다.
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

// 현재 행렬을 shader 변수에 넘긴다. 유효한 shader/변수 이름이 전제이며 바인딩 HRESULT를 그대로 전달한다.
HRESULT CTransform::Bind_ShaderResource(shared_ptr<class CShader> pShader, const char_t* pConstantName)
{
    return pShader->Bind_Matrix(pConstantName, &m_WorldMatrix);
}

// 방향을 정규화한 뒤 새 축 길이를 적용한다. 영벡터 축을 복구하는 별도 처리는 없다.
void CTransform::Scale(f32_t fScaleX, f32_t fScaleY, f32_t fScaleZ)
{
    Set_State(STATE::RIGHT, XMVector3Normalize(Get_State(STATE::RIGHT)) * fScaleX);
    Set_State(STATE::UP, XMVector3Normalize(Get_State(STATE::UP)) * fScaleY);
    Set_State(STATE::LOOK, XMVector3Normalize(Get_State(STATE::LOOK)) * fScaleZ);
}

// 기존 기저에 배율을 누적한다. Scale과 달리 연속 호출하면 배율이 곱해진다.
void CTransform::Scaling(f32_t fScaleX, f32_t fScaleY, f32_t fScaleZ)
{
    Set_State(STATE::RIGHT, Get_State(STATE::RIGHT) * fScaleX);
    Set_State(STATE::UP, Get_State(STATE::UP) * fScaleY);
    Set_State(STATE::LOOK, Get_State(STATE::LOOK) * fScaleZ);
}

// Camera 등의 전진에 사용한다. 후보 위치를 먼저 계산하고 Navigation이 거부하면 위치 행은 유지한다.
void CTransform::Go_Straight(f32_t fTimeDelta, shared_ptr<class CNavigation> pNavigation)
{
    vector_t  vPosition = Get_State(STATE::POSITION);
    vector_t  vLook = Get_State(STATE::LOOK);

    vPosition += XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

    if (nullptr == pNavigation ||
        true == pNavigation->isMove(vPosition))
    {
        Set_State(STATE::POSITION, vPosition);
    }
}

// 후진/좌우 이동은 기저를 정규화하므로 scale 크기에 따라 속도가 변하지 않는다. 충돌 검사는 하지 않는다.
void CTransform::Go_Backward(f32_t fTimeDelta)
{
    vector_t  vPosition = Get_State(STATE::POSITION);
    vector_t  vLook = Get_State(STATE::LOOK);

    vPosition -= XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

    Set_State(STATE::POSITION, vPosition);
}

// 정규화한 RIGHT의 반대 방향으로 속도×초만큼 이동한다. 위치 갱신에 Navigation이나 충돌 판정을 거치지 않는다.
void CTransform::Go_Left(f32_t fTimeDelta)
{
    vector_t  vPosition = Get_State(STATE::POSITION);
    vector_t  vRight = Get_State(STATE::RIGHT);

    vPosition -= XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

    Set_State(STATE::POSITION, vPosition);
}

// 정규화한 RIGHT 방향으로 속도×초만큼 이동한다. 기저 방향과 위치의 좌표계는 이 Transform의 현재 행렬을 따른다.
void CTransform::Go_Right(f32_t fTimeDelta)
{
    vector_t  vPosition = Get_State(STATE::POSITION);
    vector_t  vRight = Get_State(STATE::RIGHT);

    vPosition += XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

    Set_State(STATE::POSITION, vPosition);
}

// 현재 축의 길이는 보존하되 방향은 단위 기저에서 다시 만든다. 입력 degree를 radian으로 바꿔 DirectXMath에 준다.
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

// X/Y/Z 각도를 pitch/yaw/roll 입력으로 해석해 quaternion을 만든다. 기존 방향에 더하는 동작은 아니다.
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

// Camera_Free 등이 프레임 delta로 호출한다. degree/초 * 초를 radian으로 변환해 기존 세 기저에 적용한다.
void CTransform::Turn(fvector_t vAxis, f32_t fTimeDelta)
{
    vector_t        vRight = Get_State(STATE::RIGHT);
    vector_t        vUp = Get_State(STATE::UP);
    vector_t        vLook = Get_State(STATE::LOOK);

    matrix_t        RotationMatrix = XMMatrixRotationAxis(
        vAxis, XMConvertToRadians(m_fRotationPerSec) * fTimeDelta);

    Set_State(STATE::RIGHT, XMVector3TransformNormal(vRight, RotationMatrix));
    Set_State(STATE::UP, XMVector3TransformNormal(vUp, RotationMatrix));
    Set_State(STATE::LOOK, XMVector3TransformNormal(vLook, RotationMatrix));
}

// vAt은 현재 위치와 같은 좌표계다. 목표 방향과 +Y의 외적으로 직교 기저를 만들고 축 길이를 다시 적용한다.
// 같은 위치나 +Y와 평행한 방향은 퇴화하므로 호출자가 피해야 한다. 위치 행은 유지한다.
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

// 목표와의 거리로 이동 여부만 정한다. 큰 delta에서는 목표를 지나칠 수 있으며 Navigation은 조회하지 않는다.
void CTransform::Chase(fvector_t vGoal, f32_t fTimeDelta, f32_t fLimitDistance)
{
    vector_t        vPosition = Get_State(STATE::POSITION);
    vector_t        vMoveDir = vGoal - vPosition;

    f32_t fDistance = XMVectorGetX(XMVector3Length(vMoveDir));

    if (fDistance >= fLimitDistance)
    {
        vPosition += XMVector3Normalize(vMoveDir) * m_fSpeedPerSec * fTimeDelta;
    }

    Set_State(STATE::POSITION, vPosition);
}

// GameObject가 소유할 Component를 만든다. 초기화 실패 시 지역 shared_ptr가 정리되며 nullptr를 반환한다.
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

// 현재 Transform은 GameObject::Initialize에서 직접 생성한다. Prototype 복제 요청은 지원하지 않는다.
shared_ptr<CPrototype> CTransform::Clone(void* pArg)
{
    return nullptr;
}

NS_END
