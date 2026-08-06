#include "Trigger_Box.h"

#include "GameInstance.h"

#include <cmath>

namespace
{
	void XM_CALLCONV Draw_WireOrientedBox(
		PrimitiveBatch<VertexPositionColor>* pBatch,
		const BoundingOrientedBox& bounds,
		FXMVECTOR color)
	{
		static constexpr WORD indices[] =
		{
			0, 1, 1, 2, 2, 3, 3, 0,
			4, 5, 5, 6, 6, 7, 7, 4,
			0, 4, 1, 5, 2, 6, 3, 7,
		};
		float3_t corners[BoundingOrientedBox::CORNER_COUNT] = {};
		bounds.GetCorners(corners);
		VertexPositionColor vertices[BoundingOrientedBox::CORNER_COUNT] = {};
		for (size_t index = 0;
			index < BoundingOrientedBox::CORNER_COUNT;
			++index)
		{
			vertices[index].position = corners[index];
			XMStoreFloat4(&vertices[index].color, color);
		}
		pBatch->DrawIndexed(
			D3D_PRIMITIVE_TOPOLOGY_LINELIST,
			indices,
			_countof(indices),
			vertices,
			_countof(vertices));
	}
}

CTrigger_Box::CTrigger_Box(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject { pDevice, pContext }
{
}

CTrigger_Box::~CTrigger_Box() = default;

HRESULT CTrigger_Box::Initialize_Prototype()
{
	m_pBatch = make_shared<PrimitiveBatch<VertexPositionColor>>(
		m_pContext.Get());
	m_pEffect = make_shared<BasicEffect>(m_pDevice.Get());
	if (nullptr == m_pBatch || nullptr == m_pEffect)
		return E_FAIL;
	m_pEffect->SetVertexColorEnabled(true);

	const void* pVertexShaderByteCode = nullptr;
	size_t byteCodeLength = {};
	m_pEffect->GetVertexShaderBytecode(
		&pVertexShaderByteCode,
		&byteCodeLength);
	if (FAILED(m_pDevice->CreateInputLayout(
		VertexPositionColor::InputElements,
		VertexPositionColor::InputElementCount,
		pVertexShaderByteCode,
		byteCodeLength,
		m_pInputLayout.GetAddressOf())))
	{
		return E_FAIL;
	}
	return S_OK;
}

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

void CTrigger_Box::Late_Update(f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	if (!m_isAuthoringVisible)
		return;
	CGameInstance::Get().Add_RenderObject(
		RENDERGROUP::NONLIGHT,
		static_pointer_cast<CGameObject>(shared_from_this()));
}

HRESULT CTrigger_Box::Render()
{
	if (!m_isAuthoringVisible || nullptr == m_pBatch ||
		nullptr == m_pEffect || nullptr == m_pInputLayout)
	{
		return m_isAuthoringVisible ? E_FAIL : S_OK;
	}

	m_pEffect->SetWorld(XMMatrixIdentity());
	m_pEffect->SetView(XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::VIEW)));
	m_pEffect->SetProjection(XMLoadFloat4x4(
		CGameInstance::Get().Get_Transform(D3DTS::PROJ)));
	m_pContext->IASetInputLayout(m_pInputLayout.Get());
	m_pEffect->Apply(m_pContext.Get());

	const vector_t color = m_isSelected ?
		XMVectorSet(1.f, 0.85f, 0.1f, 1.f) :
		(m_Desc.isEnabled ?
			XMVectorSet(0.1f, 1.f, 0.35f, 1.f) :
			XMVectorSet(1.f, 0.45f, 0.1f, 1.f));
	m_pBatch->Begin();
	Draw_WireOrientedBox(m_pBatch.get(), m_Bounds, color);
	m_pBatch->End();
	return S_OK;
}

bool_t CTrigger_Box::Apply_Descriptor(const TRIGGER_BOX_DESC& desc)
{
	if (!Is_ValidDescriptor(desc))
		return false;
	m_Desc = desc;
	Rebuild_Bounds();
	return true;
}

void CTrigger_Box::Set_AuthoringVisible(const bool_t isVisible)
{
	m_isAuthoringVisible = isVisible;
}

void CTrigger_Box::Set_Selected(const bool_t isSelected)
{
	m_isSelected = isSelected;
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

void CTrigger_Box::Rebuild_Bounds()
{
	m_Bounds.Center = m_Desc.position;
	m_Bounds.Extents = m_Desc.halfExtents;
	XMStoreFloat4(
		&m_Bounds.Orientation,
		XMQuaternionRotationRollPitchYaw(
			0.f,
			XMConvertToRadians(m_Desc.yawDegrees),
			0.f));
}

unique_ptr<CTrigger_Box> CTrigger_Box::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto pInstance = unique_ptr<CTrigger_Box>(
		new CTrigger_Box(pDevice, pContext));
	if (FAILED(pInstance->Initialize_Prototype()))
		return nullptr;
	return pInstance;
}

shared_ptr<CPrototype> CTrigger_Box::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CTrigger_Box>(new CTrigger_Box(*this));
	if (FAILED(pInstance->Initialize(pArg)))
		return nullptr;
	return pInstance;
}
