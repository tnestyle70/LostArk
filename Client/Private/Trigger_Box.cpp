//MapTool과 Level 진단의 배치 descriptor를 방향성 wire box로 표시한다.
#include "Trigger_Box.h"
//선 그리기 자원과 정점 제출은 Engine의 공용 구현에 맡긴다.
#include "WireBoxRenderer.h"

//렌더 그룹 등록과 현재 view/projection 행렬을 조회한다.
#include "GameInstance.h"

//descriptor 수치가 유한한지 검사한다.
#include <cmath>

CTrigger_Box::CTrigger_Box(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CTrigger_Box::~CTrigger_Box() = default;

//원형이 공용 선 렌더 자원을 만든다. 준비 실패는 원형 등록으로 넘기지 않는다.
HRESULT CTrigger_Box::Initialize_Prototype()
{
	m_pWireBoxRenderer = Engine::CWireBoxRenderer::Create(m_pDevice, m_pContext);
	if (nullptr == m_pWireBoxRenderer)
	{
		return E_FAIL;
	}

	return S_OK;
}

//Clone 입력이 없거나 기반 초기화·descriptor 검증이 실패하면 E_FAIL이다.
HRESULT CTrigger_Box::Initialize(void* pArg)
{
	if (nullptr == pArg ||
		FAILED(__super::Initialize(pArg)) ||
		!Apply_Descriptor(*static_cast<TRIGGER_BOX_DESC*>(pArg)))
	{
		return E_FAIL;
	}

	return S_OK;
}

//Layer의 늦은 갱신에서 저작 표시 여부를 확인한 뒤 자신을 렌더 목록에 넣는다.
void CTrigger_Box::Late_Update(f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	if (!m_bAuthoringVisible)
	{
		return;
	}

	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONLIGHT,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

//Client는 표시 색을 정하고, Engine은 이미 월드 좌표인 상자를 현재 카메라로 그린다.
HRESULT CTrigger_Box::Render()
{
	//숨겨진 상자는 정상적으로 건너뛰고, 보여야 하는 상자의 자원 누락만 실패로 처리한다.
	if (!m_bAuthoringVisible)
	{
		return S_OK;
	}

	if (nullptr == m_pWireBoxRenderer)
	{
		return E_FAIL;
	}

	//선택 강조를 우선하고, 미선택은 collision/trigger와 enabled 속성으로 색을 구분한다.
	float4_t vColor{};
	if (m_bSelected)
	{
		vColor = float4_t(1.f, 0.85f, 0.1f, 1.f);
	}
	else if (m_Desc.isCollisionBox)
	{
		if (m_Desc.isEnabled)
		{
			vColor = float4_t(0.15f, 0.55f, 1.f, 1.f);
		}
		else
		{
			vColor = float4_t(0.25f, 0.35f, 0.55f, 1.f);
		}
	}
	else
	{
		if (m_Desc.isEnabled)
		{
			vColor = float4_t(0.1f, 1.f, 0.35f, 1.f);
		}
		else
		{
			vColor = float4_t(1.f, 0.45f, 0.1f, 1.f);
		}
	}

	//NONLIGHT에서 호출된 현재 시점의 카메라 행렬을 Engine에 전달한다.
	const float4x4_t& ViewMatrix = *CGameInstance::Get().Get_Transform(D3DTS::VIEW);
	const float4x4_t& ProjectionMatrix = *CGameInstance::Get().Get_Transform(D3DTS::PROJ);
	return m_pWireBoxRenderer->Render(m_Bounds, vColor, ViewMatrix, ProjectionMatrix);
}

//검증을 통과한 값만 보관하고 방향성 경계를 다시 만든다.
bool_t CTrigger_Box::Apply_Descriptor(const TRIGGER_BOX_DESC& desc)
{
	if (!Is_ValidDescriptor(desc))
	{
		return false;
	}

	m_Desc = desc;
	Rebuild_Bounds();
	return true;
}

void CTrigger_Box::Set_AuthoringVisible(const bool_t isVisible)
{
	m_bAuthoringVisible = isVisible;
}

void CTrigger_Box::Set_Selected(const bool_t isSelected)
{
	m_bSelected = isSelected;
}

const std::string& CTrigger_Box::Get_PlacementId() const
{
	return m_Desc.placementId;
}

bool_t CTrigger_Box::Is_ValidDescriptor(const TRIGGER_BOX_DESC& desc)
{
	return !desc.placementId.empty() &&
		std::isfinite(desc.position.x) &&
		std::isfinite(desc.position.y) &&
		std::isfinite(desc.position.z) &&
		std::isfinite(desc.halfExtents.x) &&
		std::isfinite(desc.halfExtents.y) &&
		std::isfinite(desc.halfExtents.z) &&
		std::isfinite(desc.yawDegrees) &&
		desc.halfExtents.x > 0.f &&
		desc.halfExtents.y > 0.f &&
		desc.halfExtents.z > 0.f;
}

//월드 중심·축별 반크기를 복사하고 Y축 degree를 radian으로 변환한다.
void CTrigger_Box::Rebuild_Bounds()
{
	m_Bounds.Center = m_Desc.position;
	m_Bounds.Extents = m_Desc.halfExtents;

	const f32_t fYawRadians = XMConvertToRadians(m_Desc.yawDegrees);
	const vector_t vOrientation = XMQuaternionRotationRollPitchYaw(0.f, fYawRadians, 0.f);
	XMStoreFloat4(&m_Bounds.Orientation, vOrientation);
}

//Loader의 원형 등록용 단독 소유 객체다. 준비 실패 시 임시 객체가 자동 정리된다.
std::unique_ptr<CTrigger_Box> CTrigger_Box::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = std::unique_ptr<CTrigger_Box>(new CTrigger_Box(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		return nullptr;
	}

	return pInstance;
}

//원형의 Engine 렌더 자원을 공유하고 배치 입력을 적용한다. 실패는 nullptr로 Layer 추가를 막는다.
std::shared_ptr<CPrototype> CTrigger_Box::Clone(void* pArg)
{
	auto pInstance = std::shared_ptr<CTrigger_Box>(new CTrigger_Box(*this));
	if (FAILED(pInstance->Initialize(pArg)))
	{
		return nullptr;
	}

	return pInstance;
}
