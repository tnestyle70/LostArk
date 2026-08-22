#include "SkillGroundTargetPreview.h"

#include "GameInstance.h"
#include "RuntimeAssetRoot.h"
#include "Shader.h"
#include "Texture.h"
#include "VIBuffer_Rect.h"

#include <cmath>
#include <filesystem>

Client::CSkillGroundTargetPreview::CSkillGroundTargetPreview(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
	: CGameObject(std::move(pDevice), std::move(pContext))
{
}

Client::CSkillGroundTargetPreview::CSkillGroundTargetPreview(
	const CSkillGroundTargetPreview& prototype)
	: CGameObject(prototype)
{
}

Client::CSkillGroundTargetPreview::~CSkillGroundTargetPreview() = default;

HRESULT Client::CSkillGroundTargetPreview::Initialize_Prototype()
{
	return S_OK;
}

HRESULT Client::CSkillGroundTargetPreview::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)) ||
		FAILED(__super::Add_Component(
			ETOUI(LEVEL::STATIC), SHADER_TAG, TEXT("Com_Shader"), m_pShader)) ||
		FAILED(__super::Add_Component(
			ETOUI(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
			TEXT("Com_VIBuffer"), m_pRect)))
	{
		return E_FAIL;
	}
	return S_OK;
}

bool_t Client::CSkillGroundTargetPreview::Begin(
	const PLAYER_SKILL_DEFINITION& skill)
{
	using LostArk::Shared::SKILL_TARGET_INTENT_KIND;
	if (SKILL_TARGET_INTENT_KIND::GROUND_POINT != skill.eTargetIntent ||
		skill.RangePreview.strAssetId.empty() ||
		skill.TargetPreview.strAssetId.empty())
	{
		return false;
	}
	const std::filesystem::path rangePath =
		CRuntimeAssetRoot::Resolve(skill.RangePreview.strAssetId);
	const std::filesystem::path targetPath =
		CRuntimeAssetRoot::Resolve(skill.TargetPreview.strAssetId);
	if (rangePath.empty() || targetPath.empty())
		return false;
	shared_ptr<CTexture> stagedRange = CTexture::Create(
		m_pDevice, m_pContext, rangePath.c_str(), 1u);
	shared_ptr<CTexture> stagedTarget = CTexture::Create(
		m_pDevice, m_pContext, targetPath.c_str(), 1u);
	if (nullptr == stagedRange || nullptr == stagedTarget)
		return false;

	m_pRangeTexture = std::move(stagedRange);
	m_pTargetTexture = std::move(stagedTarget);
	m_RangeSpec = skill.RangePreview;
	m_TargetSpec = skill.TargetPreview;
	m_isTargetValid = false;
	/* Set_State publishes the first fully sampled caster/target pair. Keeping
	 this dormant avoids a one-frame quad at world origin on the key-down frame. */
	m_isActive = false;
	return true;
}

void Client::CSkillGroundTargetPreview::Set_State(
	const float3_t& casterPosition,
	const float3_t& targetPosition,
	const bool_t targetValid)
{
	if (nullptr == m_pRangeTexture || nullptr == m_pTargetTexture)
		return;
	m_CasterPosition = casterPosition;
	m_TargetPosition = targetPosition;
	m_isTargetValid = targetValid;
	m_isActive = true;
}

void Client::CSkillGroundTargetPreview::Clear()
{
	m_isActive = false;
	m_isTargetValid = false;
	m_pRangeTexture.reset();
	m_pTargetTexture.reset();
	m_RangeSpec = {};
	m_TargetSpec = {};
}

void Client::CSkillGroundTargetPreview::Late_Update(f32_t fTimeDelta)
{
	UNREFERENCED_PARAMETER(fTimeDelta);
	if (m_isActive && nullptr != m_pRangeTexture && nullptr != m_pTargetTexture)
	{
		CGameInstance::Get().Add_RenderObject(
			RENDERGROUP::BLEND,
			static_pointer_cast<CGameObject>(shared_from_this()));
	}
}

HRESULT Client::CSkillGroundTargetPreview::Render()
{
	if (!m_isActive)
		return S_OK;
	const float4_t& rangeTint = m_isTargetValid ?
		m_RangeSpec.vValidTint : m_RangeSpec.vInvalidTint;
	const float4_t& targetTint = m_isTargetValid ?
		m_TargetSpec.vValidTint : m_TargetSpec.vInvalidTint;
	if (FAILED(Render_Quad(m_pRangeTexture, m_CasterPosition,
			m_RangeSpec.fDiameter, rangeTint)) ||
		FAILED(Render_Quad(m_pTargetTexture, m_TargetPosition,
			m_TargetSpec.fDiameter, targetTint)))
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT Client::CSkillGroundTargetPreview::Render_Quad(
	const shared_ptr<CTexture>& texture,
	const float3_t& position,
	const f32_t diameter,
	const float4_t& tint)
{
	if (nullptr == texture || nullptr == m_pShader || nullptr == m_pRect ||
		!std::isfinite(diameter) || diameter <= 0.f)
	{
		return E_INVALIDARG;
	}
	float4x4_t world{};
	XMStoreFloat4x4(&world,
		XMMatrixScaling(diameter, diameter, 1.f) *
		XMMatrixRotationX(XM_PIDIV2) *
		XMMatrixTranslation(position.x, position.y + 0.035f, position.z));
	if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", &world)) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_ViewMatrix", CGameInstance::Get().Get_Transform(D3DTS::VIEW))) ||
		FAILED(m_pShader->Bind_Matrix(
			"g_ProjMatrix", CGameInstance::Get().Get_Transform(D3DTS::PROJ))) ||
		FAILED(m_pShader->Bind_RawValue("g_TintLinear", &tint, sizeof(tint))) ||
		FAILED(texture->Bind_ShaderResource(m_pShader, "g_CoverageTexture", 0u)) ||
		FAILED(m_pShader->Begin(0u)) ||
		FAILED(m_pRect->Bind_Resources()) ||
		FAILED(m_pRect->Render()))
	{
		return E_FAIL;
	}
	return S_OK;
}

unique_ptr<Client::CSkillGroundTargetPreview>
Client::CSkillGroundTargetPreview::Create(
	ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext)
{
	auto instance = unique_ptr<CSkillGroundTargetPreview>(
		new CSkillGroundTargetPreview(std::move(pDevice), std::move(pContext)));
	if (FAILED(instance->Initialize_Prototype()))
		return nullptr;
	return instance;
}

shared_ptr<CPrototype> Client::CSkillGroundTargetPreview::Clone(void* pArg)
{
	auto instance = shared_ptr<CSkillGroundTargetPreview>(
		new CSkillGroundTargetPreview(*this));
	if (FAILED(instance->Initialize(pArg)))
		return nullptr;
	return instance;
}
